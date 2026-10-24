#include "stdafx.h"
#include "ReadNAND.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

void RunReadNAND(BOOL unattend) {
	ResetTextRenderer();
	DrawBackground(0x0010);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "R e a d   N A N D");
	PrintToScreen(1, "\n\nTouch the screen to continue\nor wait 10 sec to return\n");
	LogError(L"WaitForTouch!", 0);
	LCDTouchEvent* evt = WaitForTouch(10000);
	if (evt != NULL || unattend) {
		int counter = 0;
		BYTE serial[4] = {0};
		BYTE productId[4] = {0};
		LogError(L"GetProdSection!", 0);
		int prodResult = GetProdSection(NULL, (BYTE*)productId, NULL, (BYTE*)serial, NULL, NULL, NULL);
		if (prodResult != 0) {
			LogError(L"GetProdSection fail!", prodResult);
		    PrintToScreen(1, "Could not read device section: %u \n", prodResult);
		} else {
			LogError(L"Start read!", prodResult);
			char fileName[260] = {0};
			sprintf(fileName, "\\SystemSD\\nand_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
				((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
				((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
			PrintToScreen(1, "Reading NAND flash to file:\n %s\n", fileName);
			Sleep(3000);

			FILE* file = NULL;
			BOOL devIOResult = FALSE;
			DWORD bytesReturned;
			DWORD bufferSize = 64 * 1024; // 64 KB buffer size for main region, header uses 8KB
			DWORD ioControlInputSize = 8; // 8-byte input buffer
			BYTE* buffer = NULL;
			DWORD* ioControlInput = NULL;
			HANDLE hDevice = INVALID_HANDLE_VALUE;

			if (NEW_NAV)
				bufferSize = 0x20000;

			// Allocate buffers dynamically
			buffer = (BYTE*)malloc(bufferSize);
			if (buffer == NULL)
			{
				PrintToScreen(1, "Failed to allocate buffer!");
				Sleep(5000);
				WaitForScreenUntouch();
				return;
			}

			ioControlInput = (DWORD*)malloc(ioControlInputSize);
			if (ioControlInput == NULL)
			{
				PrintToScreen(1, "Failed to allocate ioControlInput!");
				free(buffer);
				Sleep(5000);
				WaitForScreenUntouch();
				return;
			}

			// Initialize buffers to zero
			ZeroMemory(buffer, bufferSize);
			ZeroMemory(ioControlInput, ioControlInputSize);

			// Open the flash device (FMD1:)
			hDevice = CreateFileW(L"FMD1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				PrintToScreen(1, "Could not open flash device!");
				free(buffer);
				free(ioControlInput);
				Sleep(5000);
				WaitForScreenUntouch();
				return;
			}

			DWORD maxSize = (NEW_NAV ? ((MAX_BLOCKS+1)*bufferSize) : ((MAX_BLOCKS-6) * bufferSize));

			// The dump is written in full to the FAT volume of the SD card. If the
			// volume is too small (for example, a previous dump is still on it), the
			// write fails only at the very end. Check the free space up front.
			ULARGE_INTEGER freeBytes;
			freeBytes.QuadPart = 0;
			if (GetDiskFreeSpaceEx(L"\\SystemSD\\", &freeBytes, NULL, NULL)) {
				LogError(L"SD free MB", (HRESULT)(freeBytes.QuadPart >> 20));
				if (freeBytes.QuadPart < (ULONGLONG)maxSize) {
					PrintToScreen(1, "\nNot enough free space on SD:\n%u MB free, %u MB needed.\nDelete old nand_*.bin dumps and retry.",
						(DWORD)(freeBytes.QuadPart >> 20), maxSize >> 20);
					CloseHandle(hDevice);
					free(buffer);
					free(ioControlInput);
					Sleep(8000);
					WaitForScreenUntouch();
					return;
				}
			}

			// Open the destination file in binary write mode
			file = fopen(fileName, "wb");
			if (file == NULL)
			{
				PrintToScreen(1, "Could not open file on SD for dumping");
				CloseHandle(hDevice);
				free(buffer);
				free(ioControlInput);
				Sleep(5000);
				WaitForScreenUntouch();
				return;
			}

			PrintToScreen(1, ">> 0000000 / %u", maxSize);

			DWORD counter = 0;
			DWORD byteCounter = 0;
			DWORD blockSize = bufferSize;
			bool readFailed = false;
			do
			{

				if (!NEW_NAV) {
					blockSize = (counter < 8 ? 8192 : bufferSize);
				}

				ioControlInput[0] = counter;
				ioControlInput[1] = 0;
				// Call DeviceIoControl with dynamically allocated input buffer
				devIOResult = DeviceIoControl(hDevice, (NEW_NAV ? 0x1112000 : 0x80112000), ioControlInput, ioControlInputSize,
											  buffer, blockSize, &bytesReturned, NULL);
				if (!devIOResult)
				{
					readFailed = true;
					LogError(L"ReadNAND: DeviceIoControl failed, block", counter);
					LogError(L"ReadNAND: GetLastError", GetLastError());
					PrintToScreen(1, "\nDeviceIoControl failed! block %u, E: 0x%08X", counter, GetLastError());
					break;
				}

				// Write data to the file
				size_t bytesWritten = fwrite(buffer, 1, blockSize, file);
				if (bytesWritten != blockSize)
				{
					readFailed = true;
					LogError(L"ReadNAND: fwrite failed, block", counter);
					LogError(L"ReadNAND: GetLastError", GetLastError());
					PrintToScreen(1, "\nFile write failed! block %u (SD full?)", counter);
					break;
				}

				byteCounter += blockSize;

				PrintToScreen(1, "\r >> %u / %u", byteCounter, maxSize);

				counter++;
			} while (counter <= MAX_BLOCKS);

			if (fclose(file) != 0 && !readFailed) {
				readFailed = true;
				LogError(L"ReadNAND: fclose failed", GetLastError());
			}
			CloseHandle(hDevice);
			free(buffer);
			free(ioControlInput);
			if (readFailed)
				PrintToScreen(1, "\nFAILED after %u bytes, dump is incomplete", byteCounter);
			else
				PrintToScreen(1, "\nDone. Read %u bytes", byteCounter);
		}
		Sleep(5000);
	}
	Sleep(1000);
	WaitForScreenUntouch();
}