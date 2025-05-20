#include "stdafx.h"
#include "Flash.h"


/*

  SD Card handling functions

*/

int ChangeSDLock(LPCWSTR disk, BYTE* pin, int command) {
	if (pin == NULL)
		return false;
    int result = 0xbad32;


    HANDLE hDevice = CreateFileW(disk,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (hDevice != INVALID_HANDLE_VALUE) {
		DWORD bytesReturned = 0;
        DWORD ioctlCode = (command == 0) ? 0x80032020 : 0x80032014;

        result = DeviceIoControl(hDevice, ioctlCode, pin, 0x10, NULL, 0, &bytesReturned, NULL);

        CloseHandle(hDevice);
    }

    return result;
}

int CheckMountStatus(WORD command) {
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    BOOL success = FALSE;
    DWORD result = 0xFFFFFFFF;
    DWORD output = 0;
    DWORD bytesReturned = 0;

    // Open device handle for SDC1: (SDC = SD Controller?)
    hDevice = CreateFileW(
        L"SDC1:",                    // Device name
        GENERIC_READ | GENERIC_WRITE, // Read/write access
        0,                           // No sharing
        NULL,                        // No security attributes
        OPEN_EXISTING,               // Open existing device
        0,                           // No special attributes
        NULL                         // No template file
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
		LogError(L"CheckMountStatus invalid handle", command);
        return result; // Return 0xFFFFFFFF on failure
    }

    // Send IOCTL command
    success = DeviceIoControl(
        hDevice,                     // Device handle
        0x80032004,                  // Custom IOCTL code
        &command,                    // Input buffer (16-bit command)
        sizeof(command),             // Input buffer size
        &output,                     // Output buffer
        sizeof(output),              // Output buffer size
        &bytesReturned,              // Bytes returned
        NULL                         // No overlapped I/O
    );

	LogError(L"CheckMountStatus", output);

    if (success) {
        // Process output: return inverted LSB
        result = (~output & 1);
    }

	LogError(L"CheckMountStatus result", result);

    // Clean up
    CloseHandle(hDevice);
    return result;
}

/*
	This function signals the SD Card controller to initialise SD card slots.
	After inserting/removing a card, it is necessary to issue this command.
*/
void InitSDCards()
{
    HANDLE hDevice = CreateFileW(L"SDC1:",
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

    if (hDevice != INVALID_HANDLE_VALUE) {
        short index = 0;
        for (int i = 0; i < 2; ++i)
        {
            BOOL result = DeviceIoControl(hDevice,
                                          0x80032004,
                                          &index,
                                          sizeof(index),
                                          NULL,
                                          0,
                                          NULL,
                                          NULL);
            if (!result)
                break;

            index++;
        }

        BYTE cmd = 0x11;
        DeviceIoControl(hDevice,
                        0x80032000,
                        &cmd,
                        sizeof(cmd),
                        NULL,
                        0,
                        NULL,
                        NULL);

        CloseHandle(hDevice);
    }
}

void SDController_CMD17() {
    HANDLE hDevice = CreateFileW(L"SDC1:",
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

    if (hDevice != INVALID_HANDLE_VALUE) {

        BYTE cmd = 0x11;
        BOOL cmdResult = DeviceIoControl(hDevice,
                        0x80032000,
                        &cmd,
                        sizeof(cmd),
                        NULL,
                        0,
                        NULL,
                        NULL);

		LogError(L"SDController_CMD0x11 result", cmdResult);
        CloseHandle(hDevice);
	} else {
		LogError(L"SDController_CMD0x11, SDC1 handle is invalid", (long)hDevice);
	}
}



/*

	NAND handling functions

*/

int ReadSingleFlashBlock(int block, DWORD size, BYTE* output) {
	LogError(L"ReadSingleFlashBLK: Opening flash block", block);
	LogError(L"ReadSingleFlashBLK: Reading size:", size);
	HANDLE hFMD1 = CreateFileW(L"FMD1:",0xc0000000,0,0,0,0,0);
	if ( hFMD1 != INVALID_HANDLE_VALUE ) {
		DWORD param[2] = { 5, 0 };
		LogBufferContents(L"BFR: ", param, 4);
		param[0] = block;
		LogBufferContents(L"AFTR: ", param, 4);
		DWORD nRet = 0;
		if (DeviceIoControl(hFMD1,0x80112000, param, 8, &output, size, &nRet, 0)) {
			if ( nRet == size ) {
				CloseHandle(hFMD1);
				return 0;
			}
			return 3;
		}
		return 2;
	}
	return 1;
}

BOOL WriteProdDataToFile(int block, BYTE* prodData, DWORD dataSize) {
    WCHAR filePath[128];
    wsprintf(filePath, L"\\SystemSD\\%03dprod.bin", block);

    // Open or create the file
    HANDLE hFile = CreateFile(
        filePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        LogError(L"Failed to open prod.bin, Error code:", error);
        return FALSE;
    }

    // Write prodData to the file
    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        hFile,
        prodData,
        dataSize,
        &bytesWritten,
        NULL
    );

    if (!success || bytesWritten != dataSize) {
        // Handle write error
        DWORD error = GetLastError();
        LogError(L"Failed to write prodbin. Error code: %d, Bytes written: %d\n",
                error);
		LogError(L"Bytes written:", bytesWritten);
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);

	LogError(L"Saved prodbin to \SystemSD\??prod.bin", 0);
    return TRUE;
}

int GetProdSection(CHAR* modelName, BYTE* productId, BYTE* serial, BYTE* pin) {
	if (modelName == NULL && serial == NULL && productId == NULL && pin == NULL) {
		LogError(L"GetProd: invalid args!", 3);
	    return 3;
	}

	BYTE prodData[0x2D1];
	int flashBlockReadResult = ReadSingleFlashBlock(5, 0x2D1, (BYTE*)prodData);
	if (flashBlockReadResult != 0) {
		LogError(L"GetProd: Reading PROG from flash failed", flashBlockReadResult);
		return 1;
	}

	if (productId != NULL)
		memcpy(productId, ((BYTE*)prodData) + 0x18, 4);

	if (modelName != NULL)
		memcpy(modelName, ((BYTE*)prodData) + 0x38, 8);

	if (serial != NULL)
		memcpy(serial, ((BYTE*)prodData) + 0x48, 4);

	if (pin != NULL)
		memcpy(pin, ((BYTE*)prodData) + 0x58, 4);

	return 0;
}

int ReadFullSDPin(BYTE* pin) {
	if (pin == NULL) {
		LogError(L"ReadFullPin: invalid arg!", 3);
	    return 3;
	}
	BYTE prodData[0x2D1];
	int flashBlockReadResult = ReadSingleFlashBlock(5, 0x2D1, (BYTE*)prodData);
	if (flashBlockReadResult != 0) {
		LogError(L"GetProd: Reading PROG from flash failed", flashBlockReadResult);
		return 1;
	}

	memcpy(pin, ((BYTE*)prodData) + 0xf8, 0x10);
	return 0;
}