#ifndef DISPLAY_H
#define DISPLAY_H

// Common

#define COLOR_WHITE      0xFFFF // RGB(255,255,255)
#define COLOR_LIGHT_GRAY 0xC618 // RGB(192,192,192)
#define COLOR_DARK_GRAY  0x630C // RGB(128,128,128)
#define COLOR_BLACK      0x0000 // RGB(0,0,0)

// Debug parameters
#define LOG_PIXELS 0
#define LOG_SURFACES 0
// Function pointers


typedef struct {
    DWORD size;              // Size of the structure (0x34)
	DWORD reserved2;      // Zero-initialized reserved fields (7 fields)
    DWORD reserved3;      // Zero-initialized reserved fields (7 fields)
    DWORD width;             // Width of the surface (800)
    DWORD height;            // Height of the surface (800)
	DWORD reserved1;      // Zero-initialized reserved fields (7 fields)
	DWORD reserved5;      // Zero-initialized reserved fields (7 fields)
    DWORD width2;            // Possibly redundant width or stride (0x1e0 = 480)
    DWORD height2;           // Possibly redundant height or format-related (0x1e0 = 480)
	DWORD reserved4;      // Zero-initialized reserved fields (7 fields)
    DWORD max_level;         // Maximum level or depth (0xff = 255)
    DWORD format;            // Surface format (7, likely an enum or flag)
    DWORD flags;             // Configuration flags (0x4000)
} GdisRenderContext;


/*

0, 0x08, 8: 0x3c
4, 0x0c, 12:  0x1e0
8, 0x10, 16:  800
12, 0x14, 20: 0x353635
16, 0x18, 24:  0x10
20, 0x1c, 28:  local_18 * 10 + 0x10
24, 0x20, 32:  0x81
32, 0x28, 40: bffr
40, 0x30, 48: len

*/
typedef struct {
    DWORD size;         // 0, 0x3c
    DWORD height;       // 4, 0x1e0 (480)
    DWORD width;        // 8, 800
    DWORD pixelFormat;  // 12, 0x353635 (RGB565)
    DWORD unknown1;     // 16, 0x10
    DWORD unknown2;     // 20, 0x10 or 0x1a
    DWORD flags;        // 24, 0x81
	DWORD unknown3;     // 28
    DWORD buffer;   // 32, 0x640
	DWORD unknown4; // 36
    DWORD bufferSize;   // 40, e.g., 0x56000000
	DWORD reserved2c;    // 0x2c: Reserved (0)
    DWORD reserved30;    // 0x30: Reserved (0)
    DWORD reserved34;    // 0x34: Reserved (0)
    DWORD reserved38;    // 0x38: Reserved (0)
} GdisSurface;


// Functions
int SetupRenderContext();
int LockSurface(DWORD surface, DWORD* output);
int LockSurfaceAlt(DWORD surface, DWORD* output);
int UnlockSurface(DWORD surface);
int SwapBuffers();
int InitGraphic();
int DestroySurfaces();
int ScrollScreen();
void AdjustTextPosition(int x, int y);
void AdjustBoundaries(int x);
int RenderChar(byte c, int x, int y, WORD color, int scale);
int RenderStringWithScale(const byte* str, int scale);
int RenderString(const byte* str);
void PrintScreenWithScale(const byte* str, int scale);
void PrintScreen(const byte* str);
int DrawBackground(DWORD color);
void PrintToScreen(int scale, const char* format, ...);
void DrawRect(int x, int y, int w, int h, WORD color, WORD* pixels);
void DrawChar(byte c, int x, int y, WORD color, int scale, WORD* pixels);
int ClearArea(int x, int y, int w, int h);
int RenderBtnText(int x, int y, const char* text, WORD color, int scale, WORD* pixels);
int RenderButton(int x_right, int y_top, int w, int h, const char* text);
int RenderButtonWithState(int x_right, int y_top, int w, int h, const char* text, bool state);
void ResetTextRenderer();



#endif // DISPLAY_H