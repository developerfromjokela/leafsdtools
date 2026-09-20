#include "stdafx.h"
#include "ProdInf.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

const DWORD PROD_SIZE = 0x4312;
const DWORD PROD2_OFFSET = 0x8000;
const DWORD FULL_BLOCK_SIZE = PROD2_OFFSET + PROD_SIZE; // 0xC312
const DWORD IMMO_OFFSET = 0x183;
const DWORD CHECKSUM_OFFSET = 0x4310;

// PRV1
const DWORD PRV1_SIZE = 0x80;

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
    int button_x = x_right - w; // Top-left x
    int button_y = y_top;

    for (int i = 0; i < num_buttons; i++) {
        if (x >= button_x && x < button_x + w &&
            y >= button_y && y < button_y + h) {
            return i;
        }
        button_y += h + gap;
    }
    return -1;
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
    PrintToScreen(1, "\n\nRead + write PROD1+2 section.\n\n");
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

BOOL WriteBufferToFile(const WCHAR* filePath, const BYTE* buffer, DWORD size) {
    HANDLE file = CreateFile(
        filePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (file == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    DWORD bytesWritten = 0;
    BOOL success = WriteFile(file, buffer, size, &bytesWritten, NULL);
    CloseHandle(file);

    return (success && bytesWritten == size);
}

BOOL ReadBufferFromFile(const WCHAR* filePath, BYTE* buffer, DWORD expectedSize) {
    HANDLE file = CreateFile(
        filePath,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (file == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    DWORD fileSize = GetFileSize(file, NULL);
    if (fileSize != expectedSize) {
        CloseHandle(file);
        return FALSE;
    }

    DWORD bytesRead = 0;
    BOOL success = ReadFile(file, buffer, expectedSize, &bytesRead, NULL);
    CloseHandle(file);

    return (success && bytesRead == expectedSize);
}

void DisableProdImmo(BYTE* serial, BYTE* productId) {
	// QY8XXX IMMO check stored in CONF section
	BYTE immoEnabled = 0;

	int immoResult = GetSysInfConfiguration(0xF, &immoEnabled, 1);
	if (immoResult == 0) {
		PrintToScreen(1, "Current IMMO Check = %02X\n", (BYTE*)immoEnabled);
		PrintToScreen(1, "Setting IMMO Check = 00..");

		immoEnabled = 0;
		immoResult = WriteSysInfConfiguration(0xF, &immoEnabled, 1);
		if (immoResult == 0) {
			PrintToScreen(1, "OK!\n");
		} else {
			PrintToScreen(1, "FAIL! (ERR %d)\n", immoResult);
		}
	} else {
		PrintToScreen(1, "Could not retrieve current IMMO Status (ERR %d)\n", immoResult);
	}

    Sleep(500);
    WaitForScreenUntouch();
}

void DumpProd(BYTE* serial, BYTE* productId) {
    BYTE* buffer = NULL;
    LogError(L"Start PROD dump!", 0);

    buffer = (BYTE*)malloc(FULL_BLOCK_SIZE);
    if (buffer == NULL) {
        PrintToScreen(1, "Failed to allocate buffer!\n");
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    ZeroMemory(buffer, FULL_BLOCK_SIZE);

    int readResult = ReadSingleFlashBlock(2, FULL_BLOCK_SIZE, buffer);
    if (readResult > 0) {
        PrintToScreen(1, "Failed to read Flash block (%d)\n", readResult);
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    // Verify Signatures
    if (memcmp(buffer, "PROD", 4) != 0) {
        PrintToScreen(1, "PROD1 signature invalid!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    if (memcmp(buffer + PROD2_OFFSET, "PROD2", 5) != 0) {
        PrintToScreen(1, "PROD2 signature invalid!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    WCHAR pathProd1[128], pathProd2[128];
    wsprintf(pathProd1, L"\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
        productId[0], productId[1], productId[2], productId[3],
        serial[0], serial[1], serial[2], serial[3]);
    wsprintf(pathProd2, L"\\SystemSD\\prod2_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
        productId[0], productId[1], productId[2], productId[3],
        serial[0], serial[1], serial[2], serial[3]);

    PrintToScreen(1, "Dumping PROD and PROD2 to SD...\n");

    BOOL res1 = WriteBufferToFile(pathProd1, buffer, PROD_SIZE);
    BOOL res2 = WriteBufferToFile(pathProd2, buffer + PROD2_OFFSET, PROD_SIZE);

    if (res1 && res2) {
        PrintToScreen(1, "Dump successful!\nSaved PROD1 and PROD2 files.\n");
    } else {
        PrintToScreen(1, "Error writing PROD files to SD card!\n");
    }

    free(buffer);
    Sleep(500);
    WaitForScreenUntouch();
}

void WriteProd(BYTE* serial, BYTE* productId) {
    DWORD sum1, sum2;
    BYTE* buffer = NULL;

    LogError(L"Start PROD restore!", 0);

    buffer = (BYTE*)malloc(FULL_BLOCK_SIZE);
    if (buffer == NULL) {
        PrintToScreen(1, "Failed to allocate buffer!\n");
        Sleep(5000);
        WaitForScreenUntouch();
        return;
    }

    // Fill gaps between PRODs with 0xFF
	memset(buffer, 0xFF, FULL_BLOCK_SIZE);

    WCHAR pathProd1[128], pathProd2[128];
    wsprintf(pathProd1, L"\\SystemSD\\prod_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
        productId[0], productId[1], productId[2], productId[3],
        serial[0], serial[1], serial[2], serial[3]);
    wsprintf(pathProd2, L"\\SystemSD\\prod2_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
        productId[0], productId[1], productId[2], productId[3],
        serial[0], serial[1], serial[2], serial[3]);

    PrintToScreen(1, "Reading PROD and PROD2 from SD...\n");

    if (!ReadBufferFromFile(pathProd1, buffer, PROD_SIZE)) {
        PrintToScreen(1, "Could not read valid prod file from SD!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    if (!ReadBufferFromFile(pathProd2, buffer + PROD2_OFFSET, PROD_SIZE)) {
        PrintToScreen(1, "Could not read valid prod2 file from SD!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    // Validate Signatures
    if (memcmp(buffer, "PROD", 4) != 0) {
        PrintToScreen(1, "PROD1 file signature invalid!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    if (memcmp(buffer + PROD2_OFFSET, "PROD2", 5) != 0) {
        PrintToScreen(1, "PROD2 file signature invalid!\n");
        free(buffer);
        Sleep(500);
        WaitForScreenUntouch();
        return;
    }

    PrintToScreen(1, "Correcting Checksums...\n");
    
    sum1 = sumbytes(buffer, CHECKSUM_OFFSET);
    *(WORD*)(buffer + CHECKSUM_OFFSET) = (WORD)sum1;

    sum2 = sumbytes(buffer + PROD2_OFFSET, CHECKSUM_OFFSET);
    *(WORD*)(buffer + PROD2_OFFSET + CHECKSUM_OFFSET) = (WORD)sum2;

    PrintToScreen(1, "PROD1 CHECKSUM: %04X\n", *(WORD*)(buffer + CHECKSUM_OFFSET));
    PrintToScreen(1, "PROD2 CHECKSUM: %04X\n", *(WORD*)(buffer + PROD2_OFFSET + CHECKSUM_OFFSET));

    if (DRY_RUN) {
        wsprintf(pathProd1, L"\\SystemSD\\prodsnew_%02X%02X%02X%02X_%02X%02X%02X%02X.bin",
            productId[0], productId[1], productId[2], productId[3],
            serial[0], serial[1], serial[2], serial[3]);

        if (WriteBufferToFile(pathProd1, buffer, FULL_BLOCK_SIZE)) {
            PrintToScreen(1, "Dry run files saved successfully!\n");
        } else {
            PrintToScreen(1, "Dry run file write failed!\n");
        }
    } else {
        int writeResult = WriteSingleFlashBlock(2, FULL_BLOCK_SIZE, buffer);
        LogError(L"Write result", writeResult);
        if (writeResult == 0) {
            PrintToScreen(1, "PROD1+PROD2 restored to FLASH successfully!\n");
        } else {
            PrintToScreen(1, "Could not write block to FLASH! Err: %d\n", writeResult);
        }
    }

    free(buffer);
    Sleep(500);
    WaitForScreenUntouch();
}

void RunProdInfMenu() {
    InitProdInfMainMenu(true);
    if (!NEW_NAV) {
        PrintToScreen(1, "QY8 version!!\n");
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