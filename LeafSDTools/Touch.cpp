#include "stdafx.h"
#include "Touch.h"

POINT pointerInfo;
BOOL posResult = false;
SHORT pointResult = 0;
SHORT prevPointResult = 0;
int timeoutCounter = 0;
LCDTouchEvent touchEvent;
QY8XTouchEvent qy8touch = {0};


LCDTouchEvent* WaitForTouch(DWORD timeout) {


	ZeroMemory(&touchEvent, sizeof(LCDTouchEvent));
	ZeroMemory(&qy8touch, sizeof(QY8XTouchEvent));

	if (NEW_NAV) {
		HANDLE evtHandle = OpenEventW(0x1F0003, FALSE, L"EVENT_TPL_TOUCH_KEY");

		if (!evtHandle) {
			LogError(L"OpenEventW failed", GetLastError());
			return NULL;
		}

		// Open the touch panel device
		HANDLE hTouch = CreateFileW(L"TPL1:", 
								 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
								 NULL);
    	if (hTouch == INVALID_HANDLE_VALUE) {
			LogError(L"Cannot open TPL1", 1);
			return NULL;
		}

		while (true) {
			if (WaitForSingleObject(evtHandle, timeout) != WAIT_OBJECT_0) {
				break;
			}
 
			// While data comes in, pause! data comes until finger is released / no more events
		    while (WaitForSingleObject(evtHandle, 200) != WAIT_OBJECT_0) {
			   Sleep(100);
			}


			DWORD bytesReturned = 0;
			BOOL success = DeviceIoControl(
				hTouch,
				IOCTL_GET_TOUCH_EVENT,
				NULL,
				0,
				&qy8touch,
				sizeof(qy8touch),
				&bytesReturned,
				NULL
			);

			if (success) {
				SerialLog("Touch data: \r\n");
				/*LogError(L"  X: ", qy8touch.x);
				LogError(L"  Y: ", 480-qy8touch.y);
				LogError(L"  ID: ", qy8touch.touch_id);
				LogError(L"  ST: ", qy8touch.status);
				LogError(L"  R1: ", qy8touch.reserved1);
				LogError(L"  R2: ", qy8touch.reserved2);
				LogError(L"  GST: ", qy8touch.gesture_id);*/
				touchEvent.xCoord = qy8touch.x;
				touchEvent.yCoord = 480-qy8touch.y;
				touchEvent.status = 1;
				CloseHandle(hTouch);
				CloseHandle(evtHandle);
				return &touchEvent;
			} else {
				LogError(L"DeviceIoControl failed: ", GetLastError());
			}
		}

		CloseHandle(hTouch);
		CloseHandle(evtHandle);

		return NULL;
	}


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
		SerialLog("TOUCH EVT!\r\n");
		
		bool clickItm = DeviceIoControl(hTouch,0x800b205c,NULL,0,(BYTE*) response, 6,&nRet, 0);

		if (clickItm) {
			if (response[4] == 0x30) {
				break;
			}
			touchEvent.xCoord = *(WORD*)&response;
			touchEvent.yCoord = 480-(*(WORD*)&response[2]);
			touchEvent.status = response[4];
			CloseHandle(evtHandle);
			CloseHandle(hTouch);
			return &touchEvent;
		}
	}

	CloseHandle(evtHandle);
	CloseHandle(hTouch);
	return NULL;
}

void WaitForScreenUntouch() {
	if (!NEW_NAV) {
		while (WaitForTouch(100) != NULL) {
			Sleep(100);
		}
	}
}