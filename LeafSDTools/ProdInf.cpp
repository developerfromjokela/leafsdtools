#include "stdafx.h"
#include "ProdInf.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"


const DWORD PROD_SIZE = 0x2D2;
const bool DRY_RUN = false;
const int LABELS_COUNT = 4;

const char* prodLabels[] = {"DUMP TO SD", "RESTORE FROM SD", "DISABLE IMMO", "EXIT"};

void RenderProdMenuOptions() {
    int y = 20;
    for (int i = 0; i < LABELS_COUNT; i++) {
        if (RenderButtonWithState(780, y, 180, 45, prodLabels[i], true) != 0) {
            break;
        }
        y += 40 + 20; // Move down for next button
    }
}

int GetPressedProdButton(int x, int y, int x_right, int y_top, int w, int h, int gap, int num_buttons) {
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

void InitProdInfMainMenu(bool renderMenuOpts) {
	ResetTextRenderer();
	DrawBackground(0x0010);
	AdjustBoundaries(600);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "P R O D   I N F");
	PrintToScreen(1, "\n\nRead + write PROD section.\n\n");
	if (renderMenuOpts)
		RenderProdMenuOptions();
	Sleep(50);
}

DWORD sumbytes(const BYTE *data, UINT nLen)
{
   DWORD sum = 0;
   for ( UINT i=0; i<nLen ;i++ ) {
      sum += data[i];
   }
   return sum;
}


int DumpSingleBlockToFile(HANDLE file, BYTE* buffer, int block, DWORD bufferSize) {
	LogError(L"Start DumpSingleBlockToFile", 0);
	int readResult = ReadSingleFlashBlock(block, bufferSize, buffer);
	if (readResult > 0) {
		LogError(L"Failure result!", readResult);
		switch (readResult) {
			case 1: {
				PrintToScreen(1, "Could not open Flash Device!\n");
				break;
			}
			case 2: {
				PrintToScreen(1, "DeviceIoControl Failed!\n");
				break;
			}
			case 3: {
				PrintToScreen(1, "Could not read all data!\n");
				break;
			}
			default: {
				PrintToScreen(1, "Unknown error\n");
				break;
			}
		}
		return 0;
	}

	// Verify PROD signature before trusting this data
	if (*(DWORD*)buffer != 'DORP') {
		LogError(L"PROD signature mismatch!", *(DWORD*)buffer);
		PrintToScreen(1, "PROD signature invalid! Aborting dump.\n");
		return 0;
	}
	LogError(L"Start writing to file!", bufferSize);

	// Write data to the file
	DWORD bytesWritten = 0;
	BOOL success = WriteFile(
		file,
		buffer,
		bufferSize,
		&bytesWritten,
		NULL
	);
	if (!success || bytesWritten != bufferSize)
	{
		if (bytesWritten != bufferSize) {
			PrintToScreen(1, "Write size mismatch!\n");
		}
		PrintToScreen(1, "File write failed!\n");
		return 0;
	}
	PrintToScreen(1, "Block %d read successfully!\n", block);
	return 1;
}


void DisableProdImmo(BYTE* serial, BYTE* productId) {
	HANDLE file = NULL;
	BYTE* buffer = NULL;
	DWORD sum = 0;
	DWORD readBytes = 0;
	BOOL success = FALSE;
	LogError(L"Start DisableProdImmo!", 0);

	buffer = (BYTE*)malloc(PROD_SIZE);
	if (buffer == NULL)
	{
		PrintToScreen(1, "Failed to allocate buffer!\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	LogError(L"Buffer allocated!", PROD_SIZE);

	// Initialize buffers to zero
	ZeroMemory(buffer, PROD_SIZE);
	LogError(L"Buffer zeroed!", PROD_SIZE);

	WCHAR fileName[128];
	char debugLogFileName[64];
	wsprintf(fileName, L"\\SystemSD\\immo_bak_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	sprintf(debugLogFileName, "\\SystemSD\\immo_bak_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Making PROD backup to file:\n %s\n", debugLogFileName);
	Sleep(1000);


	file = CreateFile(
		fileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (file == INVALID_HANDLE_VALUE)
	{
        free(buffer);
		CloseHandle(file);
		PrintToScreen(1, "Could not open file on SD for backup\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	LogError(L"File opened!", 0);


	int result = DumpSingleBlockToFile(file, buffer, (NEW_NAV ? 2 : 5), PROD_SIZE);
	LogError(L"Dump to single block!", 0);
	LogError(L"Dump result", result ? 1 : 0);

	CloseHandle(file);
	LogError(L"Close file", 0);

	if (result != 1) {
		free(buffer);
		LogError(L"Free buff", 0);
		Sleep(500);
		WaitForScreenUntouch();
		return;
	}

	PrintToScreen(1, "IMMO Byte (old): %02X\n", *(BYTE*)(buffer+0x193));

	// Flip IMMO byte to 0
    *(BYTE*)(buffer+0x193) = 0;

	PrintToScreen(1, "IMMO Byte (new): %02X\n", *(BYTE*)(buffer+0x193));

	// Save changes

	PrintToScreen(1, "Correcting Checksum\n");
	sum = sumbytes(buffer, 0x2D0);

	PrintToScreen(1, "CHECKSUM (old, WORD): %04X\n", *(WORD*)(buffer+0x2D0));
    *(WORD*)(buffer+0x2D0) = (WORD)sum;

	PrintToScreen(1, "CHECKSUM (raw sum): %08X\n", sum);
	PrintToScreen(1, "CHECKSUM (written, WORD): %04X\n", *(WORD*)(buffer+0x2D0));

	if (DRY_RUN) {
		// Dry Run to new file, not writing to flash!
		ZeroMemory(&fileName, 128);
		ZeroMemory(&debugLogFileName, 64);
		wsprintf(fileName, L"\\SystemSD\\immonew_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
			((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
			((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
		sprintf(debugLogFileName, "\\SystemSD\\immonew_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
			((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
			((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);

		HANDLE dumpFile = CreateFile(
			fileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (dumpFile == INVALID_HANDLE_VALUE)
		{
			free(buffer);
			PrintToScreen(1, "Could not open file on SD for dumping\n");
			Sleep(5000);
			WaitForScreenUntouch();
			return;
		}

		readBytes = 0;
		success = WriteFile(
			dumpFile,
			buffer,
			PROD_SIZE,
			&readBytes,
			NULL
		);
		if (!success || readBytes != PROD_SIZE)
		{
			if (readBytes != PROD_SIZE) {
				PrintToScreen(1, "Write size mismatch!\n");
			}
			PrintToScreen(1, "File write failed!\n");
		} else {
			PrintToScreen(1, "Dry run PROD saved to file:\n %s\n", debugLogFileName);
		}
		CloseHandle(dumpFile);
	} else {
		// Write to FLASH!
		int writeResult = WriteSingleFlashBlock((NEW_NAV ? 2 : 5), PROD_SIZE, buffer);
		LogError(L"Write result", writeResult);
		switch (writeResult) {
			case 0: {
				PrintToScreen(1, "PROD written successfully!\n");
				break;
			}
			case 1: {
				PrintToScreen(1, "Could not open Flash Device!\n");
				break;
			}
			case 2: {
				PrintToScreen(1, "Could not write block to the end! Size mismatch!\n");
				break;
			}
			case 3: {
				PrintToScreen(1, "Could not write block to FLASH!\n");
				break;
			}
			case 4: {
				PrintToScreen(1, "Could not erase block on FLASH!\n");
				break;
			}
			default: {
				PrintToScreen(1, "Unknown error (%d)\n", writeResult);
				break;
			}
		}
	}

	free(buffer);
	LogError(L"Free buff", 0);
    Sleep(500);
	WaitForScreenUntouch();
}

void DumpProd(BYTE* serial, BYTE* productId) {
	HANDLE file = NULL;
	BYTE* buffer = NULL;
	LogError(L"Start PROD read!", 0);

	buffer = (BYTE*)malloc(PROD_SIZE);
	if (buffer == NULL)
	{
		PrintToScreen(1, "Failed to allocate buffer!\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	LogError(L"Buffer allocated!", PROD_SIZE);

	// Initialize buffers to zero
	ZeroMemory(buffer, PROD_SIZE);
	LogError(L"Buffer zeroed!", PROD_SIZE);

	WCHAR fileName[128];
	char debugLogFileName[64];
	wsprintf(fileName, L"\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	sprintf(debugLogFileName, "\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Reading PROD to file:\n %s\n", debugLogFileName);
	Sleep(1000);


	file = CreateFile(
		fileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (file == INVALID_HANDLE_VALUE)
	{
        free(buffer);
		CloseHandle(file);
		PrintToScreen(1, "Could not open file on SD for dumping\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	LogError(L"File opened!", 0);


	int result = DumpSingleBlockToFile(file, buffer, (NEW_NAV ? 2 : 5), PROD_SIZE);
	LogError(L"Dump to single block!", 0);
	LogError(L"Dump result", result ? 1 : 0);

	CloseHandle(file);
	LogError(L"Close file", 0);
	free(buffer);
	LogError(L"Free buff", 0);
    Sleep(500);
	WaitForScreenUntouch();
}

void WriteProd(BYTE* serial, BYTE* productId) {
	HANDLE file = NULL;
	bool backupFileValid = false;
	DWORD fileSize = 0;
	DWORD sum;
	BYTE* buffer = NULL;

	LogError(L"Start PROD write!", 0);

    buffer = (BYTE*)malloc(PROD_SIZE);
	if (buffer == NULL)
	{
		PrintToScreen(1, "Failed to allocate buffer!\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	// Initialize buffers to zero
	ZeroMemory(buffer, PROD_SIZE);

	WCHAR fileName[128];
	char debugLogFileName[64];
	wsprintf(fileName, L"\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	sprintf(debugLogFileName, "\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
		((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
		((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
	PrintToScreen(1, "Reading PROD from file:\n %s\n", debugLogFileName);
	Sleep(1000);

	file = CreateFile(
		fileName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (file == INVALID_HANDLE_VALUE)
	{
        free(buffer);
		PrintToScreen(1, "Could not open PROD file\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}

	fileSize = GetFileSize(file, NULL);
	backupFileValid = fileSize == PROD_SIZE;

	if (!backupFileValid) {
		CloseHandle(file);
		PrintToScreen(1, "Valid PROD file not found! Cannot continue.\n");
		for (int i = 0; i < 5; i++) {
			PrintToScreen(1, "\rExiting in.. %u", 5-i);
			Sleep(1000);
		}
	} else {
		DWORD readBytes = 0;
		BOOL success = ReadFile(file, buffer, PROD_SIZE, &readBytes, NULL);
		// close backup file
		CloseHandle(file);

		if (!success || readBytes != PROD_SIZE) {
            free(buffer);
			if (!success) {
				PrintToScreen(1, "File read failed!\n");
			} else {
				PrintToScreen(1, "File size doesn't match PROD size\n");
			}
            Sleep(5000);
			WaitForScreenUntouch();
			return;
		}

		if (*(DWORD*)buffer != 'DORP') {
			LogError(L"PROD signature mismatch in backup file!", *(DWORD*)buffer);
			free(buffer);
			PrintToScreen(1, "Backup file PRODsignature invalid! Aborting write.\n");
			Sleep(5000);
			WaitForScreenUntouch();
			return;
		}

		PrintToScreen(1, "Correcting Checksum\n");
		sum = sumbytes(buffer, 0x2D0);

		PrintToScreen(1, "CHECKSUM (old, WORD): %04X\n", *(WORD*)(buffer+0x2D0));
        *(WORD*)(buffer+0x2D0) = (WORD)sum;

		PrintToScreen(1, "CHECKSUM (raw sum): %08X\n", sum);
		PrintToScreen(1, "CHECKSUM (written, WORD): %04X\n", *(WORD*)(buffer+0x2D0));

		if (DRY_RUN) {
			// Dry Run to new file, not writing to flash!
			ZeroMemory(&fileName, 128);
			ZeroMemory(&debugLogFileName, 64);
			wsprintf(fileName, L"\\SystemSD\\prodnew_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
				((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
				((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
			sprintf(debugLogFileName, "\\SystemSD\\prodnew_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
				((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
				((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);

			HANDLE dumpFile = CreateFile(
				fileName,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);
			if (dumpFile == INVALID_HANDLE_VALUE)
			{
				free(buffer);
				PrintToScreen(1, "Could not open file on SD for dumping\n");
				Sleep(5000);
				WaitForScreenUntouch();
				return;
			}

			readBytes = 0;
			success = WriteFile(
				dumpFile,
				buffer,
				PROD_SIZE,
				&readBytes,
				NULL
			);
			if (!success || readBytes != PROD_SIZE)
			{
				if (readBytes != PROD_SIZE) {
					PrintToScreen(1, "Write size mismatch!\n");
				}
				PrintToScreen(1, "File write failed!\n");
			} else {
				PrintToScreen(1, "Dry run PROD saved to file:\n %s\n", debugLogFileName);
			}
			CloseHandle(dumpFile);
		} else {
			// Write to FLASH!
			int writeResult = WriteSingleFlashBlock((NEW_NAV ? 2 : 5), PROD_SIZE, buffer);
			LogError(L"Write result", writeResult);
			switch (writeResult) {
				case 0: {
					PrintToScreen(1, "PROD written successfully!\n");
					break;
				}
				case 1: {
					PrintToScreen(1, "Could not open Flash Device!\n");
					break;
				}
				case 2: {
					PrintToScreen(1, "Could not write block to the end! Size mismatch!\n");
					break;
				}
				case 3: {
					PrintToScreen(1, "Could not write block to FLASH!\n");
					break;
				}
				case 4: {
					PrintToScreen(1, "Could not erase block on FLASH!\n");
					break;
				}
				default: {
					PrintToScreen(1, "Unknown error (%d)\n", writeResult);
					break;
				}
			}
		}
	}
    free(buffer);
    Sleep(500);
	WaitForScreenUntouch();
}


void RunProdInfMenu() {
    InitProdInfMainMenu(true);
	if (NEW_NAV) {
		PrintToScreen(1, "Sorry, this functionality is not yet working for QY8XXX!\n");
		Sleep(5000);
		WaitForScreenUntouch();
		return;
	}
    BYTE serial[4] = {0};
    BYTE productId[4] = {0};
    LogError(L"GetProdSection!", 0);
    int prodResult = GetProdSection(NULL, (BYTE*)productId, NULL, (BYTE*)serial, NULL, NULL, NULL);
    if (prodResult != 0) {
        LogError(L"GetProdSection fail!", prodResult);
        PrintToScreen(1, "Could not read device section: %u\n", prodResult);
    } else {
        bool quitAction = false;

        while (!quitAction) {
            RenderProdMenuOptions();

            int btn = -1;
            while (TRUE) {
                LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
                if (touchEvt != NULL) {
                    if (btn == -1) {
                        btn = GetPressedProdButton(touchEvt->xCoord, touchEvt->yCoord, 780, 20, 180, 45, 20, LABELS_COUNT);
						if (btn != -1 && NEW_NAV)
							break;
                    }
                } else if (btn != -1 && !NEW_NAV) {
                    // Once user has lifted their finger off / touch events have stopped, continue.
                    break;
                }
            }
            InitProdInfMainMenu(false);
			WaitForScreenUntouch();
            switch (btn) {
                case 0:
                    DumpProd((BYTE*)serial, (BYTE*)productId);
                    break;
				case 1:
					WriteProd((BYTE*)serial, (BYTE*)productId);
					break;
				case 2:
					DisableProdImmo((BYTE*)serial, (BYTE*)productId);
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