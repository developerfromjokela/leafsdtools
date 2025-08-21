#include "stdafx.h"
#include "WriteNAND.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"


const char* flashLabels[] = {"FLASH ALL", "BL IPL", "PROD", "BL BASE", "BL EBOOT", "BL ENH", "APP", "User DATA", "START", "CANCEL"};

bool flashBlocks[10] = {false, false, false, false, false, false, false, false, false, false};


void RenderFlashMenuOptions() {
    int y = 15;
    for (int i = 0; i < 10; i++) {
        if (RenderButtonWithState(780, y, 150, 36, flashLabels[i], flashBlocks[i]) != 0) {
            break;
        }
        y += 36 + 10; // Move down for next button
    }
}

int GetPressedFlashButton(int x, int y, int x_right, int y_top, int w, int h, int gap, int num_buttons) {
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

DWORD DataSum(const BYTE *data, UINT nLen) 
{
   DWORD sum = 0;
   for ( UINT i=0; i<nLen ;i++ ) {
      sum += data[i];
   }
   return sum;
}

void RunWriteNAND() {
	ResetTextRenderer();
	DrawBackground(0x0010);
	AdjustBoundaries(620);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "W r i t e   N A N D");
	PrintToScreen(1, "\n\nWrite backup file to NAND. (SIMULATION, does not write to flash yet!)\n");
	PrintToScreen(1, "!NOTE! Be careful, wrongful flash may render the device inoperable\n\n"); 
	memset(flashBlocks, 0, sizeof(flashBlocks));
	RenderFlashMenuOptions();
	Sleep(50);
	BYTE serial[4] = {0};
	BYTE productId[4] = {0};
	LogError(L"GetProdSection!", 0);
	int prodResult = GetProdSection(NULL, (BYTE*)productId, (BYTE*)serial, NULL);
	if (prodResult != 0) {
		LogError(L"GetProdSection fail!", prodResult);
		PrintToScreen(1, "Could not read device section: %u \n", prodResult);
	} else {
		char fileName[260] = {0};
		sprintf(fileName, "\\SystemSD\\nand_%02X%02X%02X%02X_%02X%02X%02X%02X.bin", 
			((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
			((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
		FILE* backupFile = fopen(fileName, "rb");
		bool backupFileValid = false;
		long fileSize = 0;
		if (backupFile != NULL) {
			fseek(backupFile, 0, SEEK_END);
			fileSize = ftell(backupFile);
			fclose(backupFile);
			backupFileValid = (fileSize > 8192);
		}

		if (!backupFileValid) {
			PrintToScreen(1, "Valid NAND Backup file not found! Cannot continue.\n");
			for (int i = 0; i < 5; i++) {
				PrintToScreen(1, "\rExiting in.. %u", 5-i);
				Sleep(1000);
			}
		} else {
			PrintToScreen(1, "Valid NAND backup found! Size: %u bytes\n", fileSize);
			flashBlocks[9] = true;

			bool startFlash = false;
			bool quitAction = false;
			while (!startFlash && !quitAction) {
				RenderFlashMenuOptions();
				WaitForScreenUntouch();
				int btn = -1;
				while (TRUE) {
					LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
					if (touchEvt != NULL) {
						if (btn == -1) {
							btn = GetPressedFlashButton(touchEvt->xCoord, touchEvt->yCoord, 780, 15, 150, 36, 10, 10);
						}
					} else if (btn != -1) {
						// Once user has lifted their finger off / touch events have stopped, continue.
						break;
					}
				}

				switch (btn) {
					case 8:
						if (flashBlocks[btn]) {
							quitAction = false;
							startFlash = true;
						}
						break;
					case 9:
						startFlash = false;
						quitAction = true;
						break;
					default:
						if (btn == 0) {
							if (!flashBlocks[0]) {
								memset(flashBlocks, 0, sizeof(flashBlocks)-2);
								flashBlocks[0] = true;
							} else {
								flashBlocks[0] = false;
							}
							flashBlocks[0] = true;
						} else {
							if (flashBlocks[0]) {
								flashBlocks[0] = false;
							}
							flashBlocks[btn] = !flashBlocks[btn];
						}
						bool flashEnabled = false;
						for (int i3 = 0; i3 < (sizeof(flashBlocks)-2); i3++) {
							if (flashBlocks[i3]) {
								flashEnabled = true;
								break;
							}
						}
						flashBlocks[8] = flashEnabled;
						break;
				}
			}

			if (startFlash) {

				flashBlocks[9] = false;
				flashBlocks[10] = false;

				RenderFlashMenuOptions();
				Sleep(400);

				if (!flashBlocks[0]) {
					char* flashStringFormat = "\r >> %s %u / %u block";
					// Only flash specific blocks
					if (flashBlocks[1]) {
						PrintToScreen(1, "\nBOOT IPL NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "Boot IPL", 0, 4, 0);
						for (int page = 0; page < 4; page++) {
							Sleep(350);
							PrintToScreen(1, flashStringFormat, "Boot IPL", page+1, 4);
						}*/

						Sleep(500);
					}

					if (flashBlocks[2]) {
						PrintToScreen(1, "\nPROD NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "PROD", 0, 4);
						for (int page = 0; page < 4; page++) {
							Sleep(350);
							PrintToScreen(1, flashStringFormat, "PROD", page+1, 4);
						}*/
						Sleep(500);
					}

					if (flashBlocks[3]) {
						PrintToScreen(1, "\nBASE NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "Boot BASE", 0, 1);
						Sleep(350);
						PrintToScreen(1, flashStringFormat, "Boot BASE", 1, 1);*/
						Sleep(500);
					}
					if (flashBlocks[4]) {
						PrintToScreen(1, "\nEBOOT NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "Boot EBOOT", 0, 1);
						Sleep(350);
						PrintToScreen(1, flashStringFormat, "Boot EBOOT", 1, 1);*/
						Sleep(500);
					}
					if (flashBlocks[5]) {
						PrintToScreen(1, "\nBOOT ENH NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "Boot ENHANCE", 0, 1);
						Sleep(350);
						PrintToScreen(1, flashStringFormat, "Boot ENHANCE", 1, 1);*/
						Sleep(500);
					}
					if (flashBlocks[6]) {
						PrintToScreen(1, "\nAPP NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "APP X/W1", 0, 54);
						for (int page = 0; page < 54; page++) {
							Sleep(350);
							PrintToScreen(1, flashStringFormat, "APP X/W1", page+1, 54);
						}
						PrintToScreen(1, "\n");
						PrintToScreen(1, flashStringFormat, "PS + MEM", 0, 1);
						Sleep(350);
						PrintToScreen(1, flashStringFormat, "PS + MEM", 1, 1);*/
						Sleep(500);
					}

					if (flashBlocks[7]) {
						PrintToScreen(1, "\nUSER DATA NOT IMPLEMENTED YET!\n");
						/*PrintToScreen(1, flashStringFormat, "NAVI DATA", 0, 1);
						for (int page = 0; page < 75; page++) {
							Sleep(350);
							PrintToScreen(1, flashStringFormat, "NAVI & USER DATA", page+1, 2175);
						}*/
						Sleep(500);
					}
				} else {
					// Only flash 64kB blocks
					PrintToScreen(1, "\nFlashing all data (excluding first 8 blocks)\n");

					FILE *backupFile = fopen(fileName, "rb");
					if (backupFile == NULL) {
						PrintToScreen(1, "Could not open backup file!");
						Sleep(5000);
						return;
					}

					// Seek to offset 0x10000
					fseek(backupFile, 0x10000, SEEK_SET);

					/*HANDLE hDevice = CreateFileW(L"FMD1:",
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

					if (hDevice == INVALID_HANDLE_VALUE) {
						fclose(backupFile);
						LogError(L"Backup handle invalid!", 1);
						PrintToScreen(1, "Could not open flash device!");
						Sleep(5000);
						return;
					}*/

					BYTE* buffer = NULL;

					// Allocate buffers dynamically
					buffer = (BYTE*)malloc(0x10000);
					if (buffer == NULL)
					{
						fclose(backupFile);
						PrintToScreen(1, "Failed to allocate buffer!");
						Sleep(5000);
						return;
					}


					ZeroMemory(buffer, 0x10000);

					int blockIndex = 8;
					int blockCounter = 0;
					BYTE dummyOut[2] = {0};
					BOOL success = TRUE;

					while (blockIndex < 0x87)  // Loop from page 8 to 134 (0x86)
					{
						size_t readBytes = fread(buffer, 1, 0x10000, backupFile);
						if (readBytes == 0)
							break;

						LogError(L"Read bytes from backup", readBytes);

						if (readBytes < 0x10000) {
							LogError(L"BLOCK ERROR", blockIndex);
							LogError(L"Block size was not expected", readBytes);
							break;
						}

						PrintToScreen(1, "\r >> %u / %u blocks", blockIndex-7, 0x7F);

						DWORD blockSum = DataSum(buffer, 0x10000);

						LogError(L"BLOCK READ", blockIndex);
						LogError(L"BLOCK SUM", blockSum);

						/*// First DeviceIoControl: clear flash area
						struct {
							DWORD page;
							DWORD cmd;
						} flashSelect = { blockIndex, 1 };

						PrintToScreen(1, "\r >> %u / %u blocks", blockIndex-7, 0x7F);

						if (!DeviceIoControl(hDevice, 0x8011200C, &flashSelect, sizeof(flashSelect),
											 0, 0, NULL, NULL)) {
							success = FALSE;
							break;
						}

						// Second DeviceIoControl: write to flash area
						struct {
							DWORD page;
							DWORD size;
							void *data;
							void *unused;
						} flashWrite = { blockIndex, 0x10000, buffer, dummyOut };

						if (!DeviceIoControl(hDevice, 0x80112004, &flashWrite, sizeof(flashWrite),
											 0, 0, NULL, NULL)) {
							success = FALSE;
							break;
						}*/

						blockIndex++;
						blockCounter++;
					}

					//CloseHandle(hDevice);
					fclose(backupFile);
					free(buffer);

					if (!success) {
						PrintToScreen(1, "\nFailed to flash (simulation)!\n");
					} else {
						PrintToScreen(1, "\nFlash complete (simulation)!\n");
					}
				}

				for (int i2 = 0; i2 < 5; i2++) {
					PrintToScreen(1, "\rExiting in.. %u", 5-i2);
					Sleep(1000);
				}
				WaitForScreenUntouch();
				return;
			}
			WaitForScreenUntouch();
			return;

		}
	}
	Sleep(4000);
	WaitForScreenUntouch();
}