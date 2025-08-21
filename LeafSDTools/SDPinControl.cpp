#include "stdafx.h"
#include "SDPinControl.h"
#include "Flash.h"
#include "Logger.h"
#include "Display.h"
#include "Touch.h"

void RunSDPinControl(bool lock) {
	LogError(L"Start SDPinControl", 0);
	ResetTextRenderer();
	DrawBackground(0x0010);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);
	if (lock) {
		PrintToScreen(2, "L o c k   S D");
	} else {
		PrintToScreen(2, "U n l o c k   S D");
	}
	PrintToScreen(1, "\n\nTouch the screen to continue\nor wait 10 sec to return\n");
	LCDTouchEvent* evt = WaitForTouch(10000);
	if (evt != NULL) {
		WaitForScreenUntouch();
		Sleep(50);
		WaitForScreenUntouch();
		Sleep(50);
		PrintToScreen(1, "Insert SD card into slot A (left slot) and touch the screen to continue.\n");
		evt = WaitForTouch(INFINITE);
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
	}
	WaitForScreenUntouch();
	Sleep(1000);
}