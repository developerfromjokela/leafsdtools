#include "stdafx.h"
#include "Touch.h"

LCDTouchEvent* WaitForTouch(DWORD timeout) {
    HANDLE hTouch = CreateFileW(L"LCD1:", 
                                 0xC0000000, // dwDesiredAccess
                                 0x0,        // dwShareMode
                                 NULL,       // lpSecurityAttributes
                                 0x3,        // dwCreationDisposition
                                 0x0,        // dwFlagsAndAttributes
                                 NULL);      // hTemplateFile

	if (hTouch == INVALID_HANDLE_VALUE) {
		LogError(L"Cannot open LCD1", 1);
		return NULL;
	}

	LCDTouchEvent* touchEvent = (LCDTouchEvent*)LocalAlloc(LPTR, sizeof(LCDTouchEvent));

	if (!touchEvent) {
		LogError(L"Cannot allocate for LCDTouchEvent", 1);
		return NULL;
	}

	HANDLE evtHandle = OpenEventW(0x1F0003, FALSE, L"EVENT_LCD_TOUCH_KEY");

	if (!evtHandle) {
        LogError(L"OpenEventW failed", GetLastError());
        CloseHandle(hTouch);
        return NULL;
    }

	BYTE response[6] = {0};
	while (true) {
		DWORD waitSinglObj = WaitForSingleObject(evtHandle, timeout);
		 if (waitSinglObj != WAIT_OBJECT_0) {
            break;
        }
		DWORD nRet = 0;
		bool clickItm = DeviceIoControl(hTouch,0x800b205c,NULL,0,(BYTE*) response, 6,&nRet, 0);
		int clRslt = 0;
		if (clickItm) {
			clRslt = 1;
		}

		if (clickItm) {
			if (response[4] == 0x30) {
				break;
			}
			touchEvent->xCoord = *(WORD*)&response;
			touchEvent->yCoord = 480-(*(WORD*)&response[2]);
			touchEvent->status = response[4];
			return touchEvent;
		}
	}

	CloseHandle(evtHandle);
	CloseHandle(hTouch);
	return NULL;
}

void WaitForScreenUntouch() {
	while (WaitForTouch(100) != NULL) {
		Sleep(100);
	}
}