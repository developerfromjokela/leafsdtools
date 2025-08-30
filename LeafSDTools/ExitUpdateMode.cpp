#include "stdafx.h"
#include "ExitUpdateMode.h"
#include "Display.h"
#include "Touch.h"

typedef int (WINAPI *LPCAPISetBootMode)(int param_1);

void RunExitUpdate() {
	LogError(L"Start ExitUpdate", 0);
	ResetTextRenderer();
	DrawBackground(0x0010);
	WaitForScreenUntouch();
	Sleep(50);
	WaitForScreenUntouch();
	Sleep(50);

	PrintToScreen(2, "E X I T   U P D A T E  M O D E");
	PrintToScreen(1, "\n\nYou can exit update mode if stock updater is unavailable.\n\nTouch the screen to continue\nor wait 10 sec to return\n");

	LCDTouchEvent* evt = WaitForTouch(10000);
	if (evt != NULL) {
		WaitForScreenUntouch();
		Sleep(50);
		WaitForScreenUntouch();
		Sleep(50);

		HINSTANCE hDll = NULL;
		LPCAPISetBootMode pSetBootMode = NULL;

		// Load the DLL
		hDll = LoadLibrary(TEXT("COMMONDLL.DLL"));
		if (hDll == NULL)
		{
			PrintToScreen(1, "Failed to load COMMONDLL.DLL. Error: %d\n", GetLastError());
			Sleep(5000);
			return;
		}

		// Get the function address
		pSetBootMode = (LPCAPISetBootMode)GetProcAddress(hDll, TEXT("CApi_SetBootMode"));
		if (pSetBootMode == NULL)
		{
			PrintToScreen(1, "Failed to get address of CApi_SetBootMode. Error: %d\n", GetLastError());
			FreeLibrary(hDll);
			Sleep(5000);
			return;
		}

		pSetBootMode(0);

		FreeLibrary(hDll);

		CleanupLog();

		Sleep(100);

	    DrawBackground(0x2de0);
		AdjustTextPosition(0, 0);
		Sleep(100);
		PrintToScreen(4, "U   P   D   A   T   E    Q   U   I   T   !\n\n\n");
		PrintToScreen(1, "Please turn off power and re-insert your stock SD card!");
		Sleep(100);

		// Infinite loop
		while (TRUE) {
			Sleep(500);
		}
	}
	WaitForScreenUntouch();
}