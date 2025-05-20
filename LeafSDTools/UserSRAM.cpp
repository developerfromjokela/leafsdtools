#include "stdafx.h"
#include "UserSRAM.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

const char* sramLabels[] = {"READ ALL", "READ TCU APN", "***", "EXIT"};

const BYTE connInfoSramAddresses[] = {
	0x4a,
	0x53
};

void RenderSRAMMenuOptions() {
    int y = 20;
    for (int i = 0; i < 4; i++) {
        if (RenderButtonWithState(780, y, 150, 36, sramLabels[i], true) != 0) {
            break;
        }
        y += 36 + 10; // Move down for next button
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
	AdjustBoundaries(770);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "U s e r   S R A M");
	PrintToScreen(1, "\n\nRead, write and modify SRAM data. Choose command\n\n");
	if (renderMenuOpts)
		RenderSRAMMenuOptions();
	Sleep(50);
}

typedef DWORD (WINAPI *PFN_CApi_GetSramAllData)(void);
typedef DWORD (WINAPI *PFN_CApi_GetSramVirtualAddress)(int, void**);
typedef DWORD (WINAPI *PFN_CApi_CheckSramArea)(int);

void DumpAllSRAM(WCHAR* baseName) {
    LogError(L"Starting DumpAllSRAM", 0);
    PrintToScreen(1, "Loading COMMONDLL\n");

    // Handle for the DLL
    HMODULE hDll = NULL;
    PFN_CApi_GetSramVirtualAddress pfnGetSramVirtualAddress = NULL;
    PFN_CApi_CheckSramArea pfnCheckSramArea = NULL;
    WCHAR logMessage[256];
    void* sramAddress = NULL;
    UINT size = 0;

    // Step 1: Load the DLL
    hDll = LoadLibrary(L"commondll.dll");
    if (hDll == NULL) {
        wsprintf(logMessage, L"Failed to load commondll.dll. Error: %d", GetLastError());
        LogError(logMessage, 1);
        PrintToScreen(1, "Failed to load commondll.dll. Error: %d\n", GetLastError());
        return;
    }

    // Step 2: Get function addresses
    pfnGetSramVirtualAddress = (PFN_CApi_GetSramVirtualAddress)GetProcAddress(hDll, L"CApi_GetSramVirtualAddress");
    pfnCheckSramArea = (PFN_CApi_CheckSramArea)GetProcAddress(hDll, L"CApi_CheckSramArea");
    if (pfnGetSramVirtualAddress == NULL || pfnCheckSramArea == NULL) {
        wsprintf(logMessage, L"Failed to get function addresses. Error: %d", GetLastError());
        LogError(logMessage, 1);
        PrintToScreen(1, "Failed to get function addresses. Error: %d\n", GetLastError());
        FreeLibrary(hDll);
        return;
    }

    // Step 3: Iterate through all possible selectors (0 to 0x100)
    const DWORD REQUESTED_SIZE = 0x3FE0; // 16,128 bytes
    for (BYTE selector = 0; selector <= 0x83; selector++) {
        wsprintf(logMessage, L"Processing selector 0x%02X", selector);
        LogError(logMessage, 0);

		PrintToScreen(1, "\r >> %u / %u: 0,0", selector, 0xFF);

		Sleep(200);
        // Call CApi_GetSramVirtualAddress
        size = pfnGetSramVirtualAddress((int)selector, &sramAddress);
        wsprintf(logMessage, L"CApi_GetSramVirtualAddress(0x%02X) returned size: %u, address: 0x%08X", selector, size, (DWORD)sramAddress);
        LogError(logMessage, 0);

		PrintToScreen(1, "\r >> %u / %u: %u,0x%08X", selector, 0xFF, size, (DWORD)sramAddress);

        // Check if size is non-zero and address is valid
        if (size == 0 || sramAddress == NULL) {
            wsprintf(logMessage, L"CApi_GetSramVirtualAddress failed (size: %u, address: 0x%08X)", size, (DWORD)sramAddress);
            LogError(logMessage, 1);
            continue;
        }

        // Call CApi_CheckSramArea
        UINT checkResult = pfnCheckSramArea((int)selector);
        wsprintf(logMessage, L"CApi_CheckSramArea(0x%02X) returned: %u", selector, checkResult);
        LogError(logMessage, 0);

        // Handle valid SRAM area
        if (checkResult == 1) {
            wsprintf(logMessage, L"Valid SRAM pointer: 0x%08X, size: %u bytes", (DWORD)sramAddress, size);
            LogError(logMessage, 0);

            // Determine dump size (smaller of returned size or REQUESTED_SIZE)
            DWORD dumpSize = (size <= REQUESTED_SIZE) ? size : REQUESTED_SIZE;
            if (IsBadReadPtr(sramAddress, dumpSize) == 0) {
                wsprintf(logMessage, L"Pointer 0x%08X is readable for %u bytes", (DWORD)sramAddress, dumpSize);
                LogError(logMessage, 0);

                // Log first DWORD for debugging
                DWORD* pData = (DWORD*)sramAddress;
                wsprintf(logMessage, L"First DWORD at pointer: 0x%08X", *pData);
                LogError(logMessage, 0);

                LPVOID tempBuffer = LocalAlloc(LPTR, dumpSize);
                if (tempBuffer == NULL) {
                    wsprintf(logMessage, L"Failed to allocate temporary buffer for %u bytes. Error: %d", dumpSize, GetLastError());
                    LogError(logMessage, 1);
                    PrintToScreen(1, "Failed to allocate temporary buffer for %u bytes. Error: %d\n", dumpSize, GetLastError());
                    continue;
                }

                // Copy SRAM data to temporary buffer
                memcpy(tempBuffer, sramAddress, dumpSize);

                // Generate file name with baseName and selector
                WCHAR sramFileName[260] = {0};
                wsprintf(sramFileName, L"%s_sram_0x%02X.bin", baseName, selector);

                // Dump the temporary buffer to a file
                HANDLE hFile = CreateFile(sramFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE) {
                    wsprintf(logMessage, L"Failed to create dump file %s. Error: %d", sramFileName, GetLastError());
                    LogError(logMessage, 1);
                    PrintToScreen(1, "Failed to create dump file %s. Error: %d\n", sramFileName, GetLastError());
                    LocalFree(tempBuffer);
                    continue;
                }

                DWORD bytesWritten;
                BOOL success = WriteFile(hFile, tempBuffer, dumpSize, &bytesWritten, NULL);
                CloseHandle(hFile);

                if (success && bytesWritten == dumpSize) {
                    wsprintf(logMessage, L"Successfully dumped %u bytes to %s", dumpSize, sramFileName);
                    LogError(logMessage, 0);
                } else {
                    wsprintf(logMessage, L"Failed to dump data to %s. Bytes written: %u, Error: %d", sramFileName, bytesWritten, GetLastError());
                    LogError(logMessage, 1);
                    PrintToScreen(1, "Failed to dump data to %s. Bytes written: %u, Error: %d\n", sramFileName, bytesWritten, GetLastError());
                }

                // Free the temporary buffer
                LocalFree(tempBuffer);
            } else {
                wsprintf(logMessage, L"Pointer 0x%08X is not readable for %u bytes", (DWORD)sramAddress, dumpSize);
                LogError(logMessage, 1);
                PrintToScreen(1, "\nPointer 0x%08X is not readable for %u bytes\n", (DWORD)sramAddress, dumpSize);
            }
        } else {
            wsprintf(logMessage, L"Invalid SRAM area for selector 0x%02X", selector);
            LogError(logMessage, 1);
            PrintToScreen(1, "\nInvalid SRAM area for selector 0x%02X\n", selector);
        }

        Sleep(100); // Brief delay to avoid overwhelming the UI or hardware
    }

    // Step 4: Finalize
    wsprintf(logMessage, L"Completed dumping all SRAM regions");
    LogError(logMessage, 0);
    PrintToScreen(1, "Completed dumping all SRAM regions\n");
    FreeLibrary(hDll);
    Sleep(4000);
	WaitForScreenUntouch();
}

// Function to dump SRAM data to a file
void DumpSramDataToFile(void* ptr, DWORD size, const WCHAR* filename) {
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        WCHAR logMessage[256];
        wsprintf(logMessage, L"Failed to create dump file %s. Error: %d", filename, GetLastError());
        LogError(logMessage, 1);
        PrintToScreen(1, "Failed to create dump file %s. Error: %d\n", filename, GetLastError());
        return;
    }

    DWORD bytesWritten;
    BOOL success = WriteFile(hFile, ptr, size, &bytesWritten, NULL);
    CloseHandle(hFile);

    WCHAR logMessage[256];
    if (success && bytesWritten == size) {
        wsprintf(logMessage, L"Successfully dumped %u bytes to %s", size, filename);
        LogError(logMessage, 0);
        PrintToScreen(1, "Successfully dumped %u bytes to %s\n", size, filename);
    } else {
        wsprintf(logMessage, L"Failed to dump data to %s. Bytes written: %u, Error: %d", filename, bytesWritten, GetLastError());
        LogError(logMessage, 1);
        PrintToScreen(1, "Failed to dump data to %s. Bytes written: %u, Error: %d\n", filename, bytesWritten, GetLastError());
    }
}

void DumpTCUAPNSram(WCHAR* baseName) {
    LogError(L"Starting DumpTCUAPNSram", 0);
    HMODULE hDll = NULL;
    PFN_CApi_GetSramVirtualAddress pfnGetSramVirtualAddress = NULL;
    PFN_CApi_CheckSramArea pfnCheckSramArea = NULL;
    DWORD result = 0;
    WCHAR logMessage[256];
    void* sramAddress = NULL;

    // Step 1: Load commondll.dll
    hDll = LoadLibrary(L"commondll.dll");
    if (hDll == NULL) {
        LogError(L"Failed to load commondll.dll. Error:", GetLastError());
        PrintToScreen(1, "Failed to load commondll.dll. Error: %d\n", GetLastError());
        return;
    }

    // Step 2: Get function addresses
    pfnGetSramVirtualAddress = (PFN_CApi_GetSramVirtualAddress)GetProcAddress(hDll, L"CApi_GetSramVirtualAddress");
    pfnCheckSramArea = (PFN_CApi_CheckSramArea)GetProcAddress(hDll, L"CApi_CheckSramArea");
    if (pfnGetSramVirtualAddress == NULL || pfnCheckSramArea == NULL) {
        LogError(L"Failed to get func address. Error:", GetLastError());
        PrintToScreen(1, "Failed to get func address. Error: %d\n", GetLastError());
        FreeLibrary(hDll);
        return;
    }

    const DWORD SRAM_SIZE = 0x3FE0; // 16,128 bytes
    for (int i = 0; i < sizeof(connInfoSramAddresses); i++) {
        WORD address = connInfoSramAddresses[i];
        LogError(L"Using address", address);
        PrintToScreen(1, "Attempting address 0x%02X\n", address);
        DWORD checkResult = pfnCheckSramArea(address);
        LogError(L"CheckSramArea Returned:", checkResult);
        PrintToScreen(1, "CheckSRAMArea Returned: 0x%08X\n", checkResult);
        result = pfnGetSramVirtualAddress(address, &sramAddress);
        LogError(L"Returned:", result);
        LogError(L"Address:", (DWORD)sramAddress);
        PrintToScreen(1, "Returned: 0x%08X, address: 0x%08X\n", result, (DWORD)sramAddress);

        // Check if pointer is readable for 0x3FE0 bytes and dump to file
        if (IsBadReadPtr(sramAddress, SRAM_SIZE) == 0) {
            wsprintf(logMessage, L"Pointer 0x%08X is readable for 0x%X bytes", result, SRAM_SIZE);
            LogError(logMessage, 0);
            PrintToScreen(1, "Pointer 0x%08X is readable for 0x%X bytes\n", result, SRAM_SIZE);
           
            // Log first DWORD for debugging
            DWORD* pData = (DWORD*)sramAddress;
            LogError(L"First DWORD at pointer: ", *pData);
            PrintToScreen(1, "First DWORD at pointer: 0x%08X\n", *pData);

            WCHAR sramFileName[260] = {0};
            wsprintf(sramFileName, L"%s_tcuapn_0x%02X.bin", baseName, address);

            // Dump the data to a file
            DumpSramDataToFile(sramAddress, SRAM_SIZE, sramFileName);
        } else {
            wsprintf(logMessage, L"Pointer 0x%08X is not readable for 0x%X bytes", result, SRAM_SIZE);
            LogError(logMessage, 1);
            PrintToScreen(1, "Pointer 0x%08X is not readable for 0x%X bytes\n", result, SRAM_SIZE);
        }

        Sleep(1000);
    }

    FreeLibrary(hDll);
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
        WCHAR sramBaseFileName[260] = {0};
        wsprintf(sramBaseFileName, L"\\SystemSD\\%02X%02X%02X%02X_%02X%02X%02X%02X", 
            ((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3],
            ((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);

        bool quitAction = false;

        while (!quitAction) {
            RenderSRAMMenuOptions();

            int btn = -1;
            while (TRUE) {
                LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
                if (touchEvt != NULL) {
                    if (btn == -1) {
                        btn = GetPressedSRAMButton(touchEvt->xCoord, touchEvt->yCoord, 780, 20, 150, 36, 10, 4);
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
                    DumpAllSRAM(sramBaseFileName);
                    break;
                case 1:
                    DumpTCUAPNSram(sramBaseFileName);
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