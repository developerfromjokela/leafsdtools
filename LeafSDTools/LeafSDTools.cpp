#include "stdafx.h"
#include "LeafSDTools.h"
#include "Flash.h"
#include "Touch.h"
#include "ReadNAND.h"
#include "WriteNAND.h"
#include "UserSRAM.h"
#include "SDPinControl.h"

#define TOOL_VERSION "101-beta"

const char* labels[] = {"Read NAND", "Write NAND", "SRAM", "SD lock", "SD unlock"};

char* wchar_to_ascii(const wchar_t* wchar_str) {
    if (!wchar_str) return NULL;

    // Get required buffer size (in bytes) for the converted string
    int size_needed = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
    if (size_needed == 0) return NULL; // Error handling

    // Allocate buffer for char string
    char* ascii_str = new char[size_needed];
    if (!ascii_str) return NULL; // Check allocation failure

    // Perform the conversion, replacing unmappable characters with '?'
    WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, ascii_str, size_needed, "?", NULL);

    return ascii_str;
}

void RenderMenuOptions() {

    int y = 20;

    for (int i = 0; i < 5; i++) {
        if (RenderButton(780, y, 180, 70, labels[i]) != 0) {
            break;
        }
        y += 70 + 10; // Move down for next button
    }
}

int GetPressedButton(int x, int y, int x_right, int y_top, int w, int h, int gap, int num_buttons) {
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

void ClearMenuOptionArea() {
	ClearArea(620, 75, 180, 480);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	 if (InitGraphic() != 0) {
       LogError(L"Display failed", 1);
       return 1;
    }
	InitSDCards();
    __try {

		while (TRUE) {
			Sleep(50);
			DrawBackground(0x001F);
			AdjustBoundaries(770);
			Sleep(50);
			// welcome banner
			PrintToScreen(2, "L e a f   S D   T o o l ");
			PrintToScreen(1, "\n\nVersion %s\n", TOOL_VERSION);

			CHAR model[8] = {0};
			BYTE serial[4] = {0};
			BYTE productId[4] = {0};
			BYTE pin[4] = {0};


			if (!IsLoggingEnabled()) {
				PrintToScreen(1, "Mounting SD..");

				if (InitLogFile(L"\\SystemSD\\leafsdtools.log") != 0) {
					PrintToScreen(1, "FAIL\n");
				} else {
					PrintToScreen(1, "OK\n");
				}
				LogError(L"Start log", 0);
			}

			int prodResult = GetProdSection((CHAR*)model, (BYTE*)productId, (BYTE*)serial, (BYTE*)pin);
			if (prodResult == 0) {
				PrintToScreen(1, "MODEL: %s\n", model);
				PrintToScreen(1, "Product ID / ID Code: %02X %02X %02X %02X\n", ((BYTE*)productId)[0], ((BYTE*)productId)[1], ((BYTE*)productId)[2], ((BYTE*)productId)[3]);
				PrintToScreen(1, "SERIAL: %02X %02X %02X %02X\n", ((BYTE*)serial)[0], ((BYTE*)serial)[1], ((BYTE*)serial)[2], ((BYTE*)serial)[3]);
				PrintToScreen(1, "SD Card PIN: %02X %02X %02X %02X\n", ((BYTE*)pin)[0], ((BYTE*)pin)[1], ((BYTE*)pin)[2], ((BYTE*)pin)[3]);			
			} else {
				PrintToScreen(1, "Could not read device section: 0x%04X \n", prodResult);
			}

			Sleep(400);
			RenderMenuOptions();
			int btn = -1;
			while (TRUE) {
				LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
				if (touchEvt != NULL) {
					if (btn == -1) {
						btn = GetPressedButton(touchEvt->xCoord, touchEvt->yCoord, 780, 20, 180, 70, 10, 5);
					}
				} else if (btn != -1) {
					// Once user has lifted their finger off / touch events have stopped, continue.
					break;
				}
			}


			switch (btn) {
			case 0:
				RunReadNAND();
				break;
			case 1:
				RunWriteNAND();
				break;
			case 2:
				RunUserSRAM();
				break;
			case 3:
				RunSDPinControl(true);
				break;
			case 4:
				RunSDPinControl(false);
				break;
			default:
				continue;
			}

			ResetTextRenderer();
		}

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        WCHAR errorMsg[256];
	    DrawBackground(0xF000);
		AdjustTextPosition(0, 0);
		Sleep(100);
	    wsprintf(errorMsg, L"Exception occurred: 0x%08X", GetExceptionCode());
		PrintToScreen(4, "F   A   T   A   L       E   R   R\n\n\n");
		PrintToScreen(1, wchar_to_ascii(errorMsg));
		Sleep(100);
        LogError(errorMsg, GetExceptionCode());
    }
	DestroySurfaces();
    CleanupLog();
	while (TRUE) {
		Sleep(500);
	}
    return 0;
}