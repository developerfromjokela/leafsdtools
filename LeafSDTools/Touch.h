#ifndef TOUCH_H
#define TOUCH_H

#define IOCTL_GET_TOUCH_EVENT 0x802f2040 

typedef struct {
	WORD xCoord;
	WORD yCoord;
	BYTE status;
} LCDTouchEvent;


typedef struct {
    UINT16 x;         // Offset 0: X coordinate (16-bit)
    UINT16 y;         // Offset 2: Y coordinate (16-bit)
    BYTE status;      // Offset 4: Status (0 or 1)
    BYTE touch_id;    // Offset 5: Touch ID (1, 2, 3, or 4)
    BYTE pressure;   // Offset 6: Pressure (1 = light touch, 2 = pressure on display)
    BYTE gesture_id;  // Offset 7: Gesture ID (0)
    UINT16 reserved2; // Offset 8-9: Reserved
} QY8XTouchEvent;

LCDTouchEvent* WaitForTouch(DWORD timeout);
void WaitForScreenUntouch();
#endif