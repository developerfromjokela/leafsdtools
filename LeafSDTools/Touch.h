#ifndef TOUCH_H
#define TOUCH_H

typedef struct {
	WORD xCoord;
	WORD yCoord;
	BYTE status;
} LCDTouchEvent;

LCDTouchEvent* WaitForTouch(DWORD timeout);
void WaitForScreenUntouch();
#endif