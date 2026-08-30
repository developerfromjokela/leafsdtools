#include "stdafx.h"
#include "SDPinControl.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

const char* sdLabels[] = {"SD UNLOCK", "SD LOCK", "EXIT"};


void RenderSDMenuOptions() {
    int y = 20;
    for (int i = 0; i < 3; i++) {
        if (RenderButtonWithState(780, y, 180, 45, sdLabels[i], true) != 0) {
            break;
        }
        y += 40 + 20; // Move down for next button
    }
}

int GetPressedSDButton(int x, int y, int x_right, int y_top, int w, int h, int gap, int num_buttons) {
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

void InitSDMainMenu(bool renderMenuOpts) {
	ResetTextRenderer();
	DrawBackground(0x0010);
	AdjustBoundaries(600);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	PrintToScreen(2, "S D   P I N");
	PrintToScreen(1, "\n\nLock & unlock SD card protection.\n\n");
	if (renderMenuOpts)
		RenderSDMenuOptions();
	Sleep(50);
}


void RunSDPinControl(bool lock) {
	if (NEW_NAV) {
		PrintToScreen(1, "\n\nSD Control is not supported on this device!");
		WaitForScreenUntouch();
		Sleep(3000);
		return;
	}
	LogError(L"Start SDPinControl", 0);
	if (lock) {
		PrintToScreen(1, "Lock");
	} else {
		PrintToScreen(1, "Unlock");
	}
	PrintToScreen(1, " SD\n");

	PrintToScreen(1, "Insert SD card into slot A (left slot) and touch the screen to continue.\n");
	LCDTouchEvent* evt = WaitForTouch(INFINITE);
	if (evt != NULL) {
		WaitForScreenUntouch();
		Sleep(200);
		BYTE pinCode[0x10] = {0};
		if (ReadFullSDPin((BYTE*) pinCode) != 0) {
			PrintToScreen(1, "Could not retrieve Unit SD Pin, cannot continue.\n");
			Sleep(4000);
		} else {
			char pinHex[50] = {0}; // Buffer for 16 bytes (up to 48 chars + null)
			int pos = 0; // Current position in pinHex
			for (int pinByte = 0; pinByte < 0x10; pinByte++) {
				if (isprint(pinCode[pinByte]) && pinByte > 3) {
					// Printable ASCII: append the character itself
					pinHex[pos++] = pinCode[pinByte];
				} else {
					// Non-printable: append two-digit hex
					pos += sprintf(&pinHex[pos], "%02X", pinCode[pinByte]);
				}
			}
			PrintToScreen(1, "Using PIN %s\n", pinHex);	
			PrintToScreen(1, "%sing", (lock ? "Lock" : "Unlock"));
			InitSDCards();
			Sleep(200);
			int lockResult = -1;
			for (int i = 0; i < 5; i++) {
				CheckMountStatus(1);
				PrintToScreen(1, ".");
				lockResult = ChangeSDLock(L"DSK8:", (BYTE*) pinCode, (lock ? 1 : 0));
				LogError(L"LockChangeResult", lockResult);	
				if (lockResult == 1) {
					PrintToScreen(1, "OK\n");
					PrintToScreen(1, "!NOTE! Please remove SD from Slot A before using SD un/lock function again");
					break;
				}
				Sleep(1000);
			}
			LogError(L"LockUnlock result", lockResult);
			if (lockResult != 1) {
				PrintToScreen(1, "FAIL\n\n");
				PrintToScreen(1, "SD lock/unlock failed\n(0x%08X, E: 0x%08X)\n", lockResult, GetLastError());
				PrintToScreen(1, "Please double-check the SD card and try again. Try restarting car if SD is good.\n\n");
				PrintToScreen(1, "!NOTE! if using microSD with adapter, try changing the SD adapter with another one. Some adapters are missing pins required for communication.");
			}
		}
		Sleep(7000);
	}
	WaitForScreenUntouch();
	Sleep(1000);
}


void RunSDPinControlMenu() {
    InitSDMainMenu(true);
	if (NEW_NAV) {
		PrintToScreen(1, "\n\nSD Control is not supported on this device!");
		WaitForScreenUntouch();
		Sleep(3000);
		return;
	}
    bool quitAction = false;

    while (!quitAction) {
		if (quitAction) {
			WaitForScreenUntouch();
			return;
		}
        RenderSDMenuOptions();

        int btn = -1;
        while (TRUE) {
            LCDTouchEvent* touchEvt = WaitForTouch(INFINITE);
            if (touchEvt != NULL) {
                if (btn == -1) {
                    btn = GetPressedSDButton(touchEvt->xCoord, touchEvt->yCoord, 780, 20, 180, 45, 20, 3);
					if (btn != -1 && NEW_NAV)
						break;
                }
            } else if (btn != -1 && !NEW_NAV) {
                // Once user has lifted their finger off / touch events have stopped, continue.
                break;
            }
        }
        InitSDMainMenu(false);
		WaitForScreenUntouch();
        switch (btn) {
            case 0:
                RunSDPinControl(false);
                break;
			case 1:
				RunSDPinControl(true);
				break;
            case 2:
                quitAction = true;
                break;
        }
    }

    WaitForScreenUntouch();
    return;
}