#include "stdafx.h"
#include "UserSRAM.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

const char* sramLabels[] = {"DUMP TO SD", "RESTORE VFLASH", "RESTORE SRAM", "EXIT"};


void RenderSRAMMenuOptions() {
    int y = 20;
    for (int i = 0; i < 4; i++) {
        if (RenderButtonWithState(780, y, 180, 45, sramLabels[i], true) != 0) {
            break;
        }
        y += 40 + 20; // Move down for next button
    }
}

int GetPressedSRAMButton(int x, int y, int x_right, int y_top, int w, int h, int gap, int num_buttons) {
    int button_x = x_right - w; // Top-left x (e.g., 800 - 150 = 650)
    int button_y = y_top;

    for (int i = 0; i < num_buttons; i++) {
        // Check if (x, y) is within button bounds
        if (x >= button_x && x < button_x + w &&
            y >= button_y && y < button_y + h) {
            return i; // Return button index (0 to 4)
        }
        button_y += h + gap; // Move to next button
    }
    return -1; // No button pressed
}

void InitUserSRAMMainMenu(bool renderMenuOpts) {
	ResetTextRenderer();
	DrawBackground(0x0010);
	AdjustBoundaries(600);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "U s e r   D A T A");
	PrintToScreen(1, "\n\nRead + write SRAM and VFlash data.\n\n");
	if (renderMenuOpts)
		RenderSRAMMenuOptions();
	Sleep(50);
}

bool DumpBlockToFile(HANDLE flashDevice, FILE* output, BYTE* buffer, DWORD* ioControlInput, DWORD block) {
	ioControlInput[0] = block;
	ioControlInput[1] = 0;
	DWORD bytesReturned;
	// Call DeviceIoControl with dynamically allocated input buffer
	BOOL devIOResult = DeviceIoControl(flashDevice, 0x80112000, ioControlInput, 8,
								  buffer, 0x10000, &bytesReturned, NULL);
	if (!devIOResult)
	{
		PrintToScreen(1, "DeviceIoControl failed!");
		return false;
	}

	// Write data to the file
	size_t bytesWritten = fwrite(buffer, 1, 0x10000, output);
	if (bytesWritten != 0x10000)
	{
		PrintToScreen(1, "File write failed!");
		return false;
	}
	return true;
}

bool DumpBlocksToFile(HANDLE flashDevice, char* fileName, DWORD start, DWORD end) {
	FILE* file = NULL;
	DWORD bufferSize = 64 * 1024;
	DWORD ioControlInputSize = 8;
	BYTE* buffer = NULL;
	DWORD* ioControlInput = NULL;

	// Allocate buffers dynamically
	buffer = (BYTE*)malloc(bufferSize);
	if (buffer == NULL)
	{
		PrintToScreen(1, "Failed to allocate buffer!");
		return false;
	}

	ioControlInput = (DWORD*)malloc(ioControlInputSize);
	if (ioControlInput == NULL)
	{
		PrintToScreen(1, "Failed to allocate ioControlInput!");
		free(buffer);
		return false;
	}

	// Initialize buffers to zero
	ZeroMemory(buffer, bufferSize);
	ZeroMemory(ioControlInput, ioControlInputSize);

	// Open the destination file in binary write mode
	file = fopen(fileName, "wb");
	if (file == NULL)
	{
		PrintToScreen(1, "Could not open file on SD for dumping");
		free(buffer);
		free(ioControlInput);
		return false;
	}

	PrintToScreen(1, ">> 0000000 / %u", (end-start) * bufferSize);

	DWORD counter = start;
	DWORD byteCounter = 0;
	do
	{
		if (!DumpBlockToFile(flashDevice, file, buffer, ioControlInput, counter)) {
			PrintToScreen(1, "\nFailed to read block! quitting..");
		}
		byteCounter += bufferSize;
		PrintToScreen(1, "\r >> %u / %u", byteCounter, (end-start) * bufferSize);
		counter++;
	} while (counter < end);

	fclose(file);
	free(buffer);
	free(ioControlInput);
	PrintToScreen(1, "\nDone. Read %u bytes\n", byteCounter);
	return true;
}

bool RestoreBlocksFromFile(HANDLE flashDevice, char* fileName, DWORD start, DWORD end) {
	FILE* backupFile = fopen(fileName, "rb");
	DWORD bufferSize = 64 * 1024;
	
	bool backupFileValid = false;
	long fileSize = 0;
	DWORD byteCounter = 0;

	if (backupFile != NULL) {
		fseek(backupFile, 0, SEEK_END);
		fileSize = ftell(backupFile);
		LogError(L"FileSize:", fileSize);
		LogError(L"Expected FileSize:", (end-start)*0x10000);
		backupFileValid = (fileSize >= (end-start)*0x10000);
	}

	if (!backupFileValid) {
		PrintToScreen(1, "Valid backup file not found! Cannot continue.\n");
		fclose(backupFile);
		return false;
	} else {
		BYTE* buffer = (BYTE*)malloc(bufferSize);
		if (buffer == NULL)
		{
			fclose(backupFile);
			PrintToScreen(1, "Failed to allocate buffer!");
			return false;
		}

		ZeroMemory(buffer, 0x10000);

		DWORD* ioControlInput = (DWORD*)malloc(FLASH_CONTROL_IO_SIZE);
		if (ioControlInput == NULL)
		{
			PrintToScreen(1, "Failed to allocate ioControlInput!");
			free(buffer);
			fclose(backupFile);
			return false;
		}
		ZeroMemory(ioControlInput, FLASH_CONTROL_IO_SIZE);

		fseek(backupFile, 0, SEEK_SET);

		PrintToScreen(1, ">> 0000000 / %u", (end-start) * bufferSize);

		DWORD counter = start;
		do
		{
			if (!WriteSingleBlockFromFile(flashDevice, backupFile, buffer, ioControlInput, counter)) {
				PrintToScreen(1, "\nFailed to write block! skip..");
				break;
			}
			byteCounter += bufferSize;
			PrintToScreen(1, "\r >> %u / %u", byteCounter, (end-start) * bufferSize);
			counter++;
		} while (counter < end);

		free(buffer);
		free(ioControlInput);

		PrintToScreen(1, "\nWrote %u bytes\n", byteCounter);
	}

	fclose(backupFile);
	return ((end-start)*bufferSize) == byteCounter;
}

void DumpUserData(BYTE* serial, BYTE* productId) {
	LogError(L"Start SRAM read!", 0);

	// Open the flash device (FMD1:)
	HANDLE hDevice = CreateFileW(L"FMD1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		PrintToScreen(1, "Could not open flash device!");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	char fileName[260] = {0};
	sprintf(fileName, "\\SystemSD\\sram_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Reading SRAM to file:\n %s\n", fileName);
	Sleep(1000);

	if (DumpBlocksToFile(hDevice, fileName, 0x47, 0x57)) {
		memset(fileName, 0, 260);
		sprintf(fileName, "\\SystemSD\\vflash_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
			((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
			((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
		PrintToScreen(1, "Reading VFlash to file:\n %s\n", fileName);
		Sleep(1000);
		DumpBlocksToFile(hDevice, (char*)fileName, 0x57, 0x85);
	}

	CloseHandle(hDevice);
    Sleep(5000);
	WaitForScreenUntouch();
}

void RestoreVFlash(BYTE* serial, BYTE* productId) {
	LogError(L"Start VFlash write!", 0);

	// Open the flash device (FMD1:)
	HANDLE hDevice = CreateFileW(L"FMD1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		PrintToScreen(1, "Could not open flash device!");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	char fileName[260] = {0};
	sprintf(fileName, "\\SystemSD\\vflash_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Restoring VFlash from file:\n %s\n", fileName);
	Sleep(1000);

	RestoreBlocksFromFile(hDevice, (char*) fileName, 0x57, 0x85);

	CloseHandle(hDevice);
    Sleep(5000);
	WaitForScreenUntouch();
}

void RestoreSRAM(BYTE* serial, BYTE* productId) {
	LogError(L"Start SRAM write!", 0);

	// Open the flash device (FMD1:)
	HANDLE hDevice = CreateFileW(L"FMD1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		PrintToScreen(1, "Could not open flash device!");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	char fileName[260] = {0};
	sprintf(fileName, "\\SystemSD\\sram_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Restoring SRAM from file:\n %s\n", fileName);
	Sleep(1000);

	RestoreBlocksFromFile(hDevice, (char*) fileName, 0x47, 0x57);

	CloseHandle(hDevice);
    Sleep(5000);
	WaitForScreenUntouch();
}



void RunUserSRAM() {
    InitUserSRAMMainMenu(true);
    BYTE serial[4] = {0};
    BYTE productId[4] = {0};
    LogError(L"GetProdSection!", 0);
    int prodResult = GetProdSection(NULL, (BYTE*)productId, (BYTE*)serial, NULL);
    if (prodResult != 0) {
        LogError(L"GetProdSection fail!", prodResult);
        PrintToScreen(1, "Could not read device section: %u\n", prodResult);
    } else {
        bool quitAction = false;

        while (!quitAction) {
            RenderSRAMMenuOptions();

            int btn = -1;
            while (TRUE) {
                LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
                if (touchEvt != NULL) {
                    if (btn == -1) {
                        btn = GetPressedSRAMButton(touchEvt->xCoord, touchEvt->yCoord, 780, 20, 180, 45, 20, 4);
                    }
                } else if (btn != -1) {
                    // Once user has lifted their finger off / touch events have stopped, continue.
                    break;
                }
            }
            InitUserSRAMMainMenu(false);
			WaitForScreenUntouch();
            switch (btn) {
                case 0:
                    DumpUserData((BYTE*)serial, (BYTE*)productId);
                    break;
				case 1:
					RestoreVFlash((BYTE*)serial, (BYTE*)productId);
					break;
				case 2:
					RestoreSRAM((BYTE*)serial, (BYTE*)productId);
					break;
                case 3:
                    quitAction = true;
                    break;
            }
        }

        WaitForScreenUntouch();
        return;
    }
    Sleep(500);
    WaitForScreenUntouch();
}