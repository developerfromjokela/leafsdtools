#include "stdafx.h"
#include "Font.h"
#include <ddraw.h>
#include <stdarg.h>

/*
	Handle Display&Drawing using DirectDraw for QY8XXX.
*/

// Function type definitions
typedef HRESULT (WINAPI *CreateSurfaceFunc)(LPDIRECTDRAW, BYTE *, LPDIRECTDRAWSURFACE *, IUnknown *);
typedef HRESULT (WINAPI *SetCooperativeLevelFunc)(LPDIRECTDRAW, HWND, DWORD);
typedef HRESULT (WINAPI *BltFunc)(LPDIRECTDRAWSURFACE, RECT *, LPDIRECTDRAWSURFACE, RECT *, DWORD, LPDDBLTFX);
typedef HRESULT (WINAPI *UnlockFunc)(LPDIRECTDRAWSURFACE, LPVOID);
typedef HRESULT (WINAPI *LockFunc)(LPDIRECTDRAWSURFACE, RECT *, BYTE *, DWORD, HANDLE);


DWORD g_surfaceIndex = 0;
DWORD g_xPos = 0;
DWORD g_yPos = 0;
DWORD g_maxX = 0;
DWORD g_maxY = 0;
DWORD g_width = 0;
DWORD g_height = 0;
DWORD g_charWidth = 0xB;
DWORD g_charHeight = 0x16;
DWORD g_offsetX = 0;
DWORD g_offsetY = 0;
DWORD g_background = 0;
DWORD surfaceHandles[2] = { 0xffffffff, 0xffffffff };

LPDIRECTDRAW g_ddContext;
LPDIRECTDRAWSURFACE g_ddSurface;
LPDIRECTDRAWSURFACE g_ddPrimarySurface;

static const BYTE PixelData[32] = {
    0x20, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00,
    0x00, 0xF8, 0x00, 0x00,
    0xE0, 0x07, 0x00, 0x00,
    0x1F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

int InitGraphic() {
    HRESULT hr = 0;

    // Initialize globals
    g_xPos = 0;
    g_yPos = 0;
    g_maxX = 0;
    g_maxY = 0;
    g_width = 0;
    g_height = 0;
    g_charWidth = 0xB;
    g_charHeight = 0x16;
    g_offsetX = 0;
    g_offsetY = 0;
    g_surfaceIndex = 0;


	if (LOG_SURFACES == 1)
		SerialLog("Running DirectDrawCreate");

    // Create DirectDraw object
    hr = DirectDrawCreate(NULL, &g_ddContext, NULL);
    if (FAILED(hr)) {
        LogError(L"DirectDrawCreate failed", hr);
        return -0x7fffbffb;
    }

	if (LOG_SURFACES == 1)
		SerialLog("Running SetCooperativeLevel");

	// Using raw vtable access to execute correct function, must run immediately after boot, or else will fail/crash.
    //hr = g_ddContext->SetCooperativeLevel(0, 0);
    SetCooperativeLevelFunc setCoopLevel = (SetCooperativeLevelFunc)(*(DWORD *)(*(DWORD *)g_ddContext + 0x44));
    hr = setCoopLevel(g_ddContext, 0, 0);
    if (FAILED(hr)) {
        g_ddContext->Release();
        g_ddContext = NULL;
        LogError(L"SetCooperativeLevel failed", hr);
        return -0x7fffbffb;
    }


    BYTE surfaceDesc[0x6c];
	memset(surfaceDesc, 0, 0x6c);

    surfaceDesc[0] = 0x6c;
    surfaceDesc[4] = DDSD_CAPS;
    surfaceDesc[100] = 0x40;


	memset(&g_ddPrimarySurface, 0, sizeof(LPDIRECTDRAWSURFACE));

	if (LOG_SURFACES == 1)
		SerialLog("Running CreateSurface");

	
	// Default defined CreateSurface not working :(
    //hr = g_ddContext->CreateSurface(&surfaceDesc, &g_ddSurface, NULL);
    CreateSurfaceFunc createSurface = (CreateSurfaceFunc)(*(DWORD *)(*(DWORD *)g_ddContext + 0x14));
	hr = createSurface(g_ddContext, surfaceDesc, &g_ddPrimarySurface, NULL);
    if (FAILED(hr)) {
        g_ddContext->Release();
        g_ddContext = NULL;
        LogError(L"CreateSurface primary failed", hr);
        return -0x7fffbffb;
    }

	memset(surfaceDesc, 0, 0x6c);

	surfaceDesc[4] = 7;
	surfaceDesc[5] = 0x10;
	surfaceDesc[6] = 0;
	surfaceDesc[7] = 0;
	surfaceDesc[0] = 0x6c;
	surfaceDesc[1] = 0;
	surfaceDesc[2] = 0;
	surfaceDesc[3] = 0;
	surfaceDesc[100] = 0x10;
	surfaceDesc[0x65] = 1;
	surfaceDesc[0x66] = 0;
	surfaceDesc[0x67] = 0;
	surfaceDesc[0x18] = 0;
	surfaceDesc[0x19] = 0;
	surfaceDesc[0x1a] = 0;
	surfaceDesc[0x1b] = 0;
	surfaceDesc[0xc] = 0x20;
	surfaceDesc[0xd] = 3;
	surfaceDesc[0xe] = 0;
	surfaceDesc[0xf] = 0;
	surfaceDesc[8] = 0xe0;
	surfaceDesc[9] = 1;
	surfaceDesc[10] = 0;
	surfaceDesc[0xb] = 0;

	memcpy(surfaceDesc + 0x44, PixelData, 32); // Copy pixel data


	memset(&g_ddSurface, 0, sizeof(LPDIRECTDRAWSURFACE));

	if (LOG_SURFACES == 1)
		SerialLog("Running CreateSurface");

	// Default defined CreateSurface not working :(
    //hr = g_ddContext->CreateSurface(&surfaceDesc, &g_ddSurface, NULL);
	hr = createSurface(g_ddContext, surfaceDesc, &g_ddSurface, NULL);
    if (FAILED(hr)) {
        g_ddContext->Release();
        g_ddContext = NULL;
        LogError(L"CreateSurface secondary failed", hr);
        return -0x7fffbffb;
    }

	// Blt operation (vtable offset 0x6c)
    BltFunc blt = (BltFunc)(*(DWORD *)(*(DWORD *)g_ddSurface + 0x6c));
    RECT rect = {0, 0, 800, 480};
    hr = blt(g_ddSurface, &rect, g_ddPrimarySurface, &rect, 0x400, 0);
    if (FAILED(hr)) {
        g_ddSurface->Release();
        g_ddSurface = NULL;
		g_ddPrimarySurface->Release();
        g_ddPrimarySurface = NULL;
        g_ddContext->Release();
        g_ddContext = NULL;
        LogError(L"BLTFX failed", hr);
        return -0x7fffbffb;
    }

    // Set globals
    g_width = 800;
    g_height = 480;
    g_maxX = g_width / g_charWidth - 1;
    g_maxY = g_height / g_charHeight - 2;
    g_offsetX = 6;
    g_offsetY = 6;
    surfaceHandles[0] = (DWORD)g_ddSurface;
	surfaceHandles[1] = (DWORD)g_ddPrimarySurface;

    return 0;
}

int DestroySurfaces() {
	if (LOG_SURFACES == 1)
		SerialLog("Destroying surfaces");
    if (g_ddSurface) {
        g_ddSurface->Release();
        g_ddSurface = NULL;
    }
	if (g_ddSurface) {
        g_ddPrimarySurface->Release();
        g_ddPrimarySurface = NULL;
    }
    if (g_ddContext) {
        g_ddContext->Release();
        g_ddContext = NULL;
    }
    surfaceHandles[0] = 0xffffffff;
    surfaceHandles[1] = 0xffffffff;
    return 0;
}

int LockSurface(DWORD surface, DWORD* output) {
    int lockResult = 0;
    BYTE surfaceDesc[0x6c];

    if (LOG_SURFACES == 1)
        SerialLog("LockSurface");

    memset(surfaceDesc, 0, 0x6c);
    surfaceDesc[0] = 0x6c; // dwSize = 108
    surfaceDesc[1] = 0;
    surfaceDesc[2] = 0;
    surfaceDesc[3] = 0;

    // Lock surface (vtable offset 0x4c)
    LockFunc lockSurface = (LockFunc)(*(DWORD *)(*(DWORD *)surface + 0x4c));
    lockResult = lockSurface((LPDIRECTDRAWSURFACE)surface, NULL, surfaceDesc, 8, 0);
    if (lockResult != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"DirectDraw Lock failed: 0x%08X", lockResult);
        LogError(errorMsg, lockResult);
        return lockResult;
    }

    *output = *(DWORD *)(surfaceDesc + 0x20);
    return lockResult;
}

int LockSurfaceAlt(DWORD surface, DWORD* output) {
	if (LOG_SURFACES == 1)
		SerialLog("LockSurface");
    return LockSurface(surface, output);
}

int UnlockSurface(DWORD surface) {
    // Logging
    if (LOG_SURFACES == 1)
        SerialLog("UnlockSurface");

    // Unlock surface (vtable offset 0x68)
    UnlockFunc unlockSurface = (UnlockFunc)(*(DWORD *)(*(DWORD *)surface + 0x68));
    HRESULT hr = unlockSurface((LPDIRECTDRAWSURFACE)surface, 0);
    if (hr != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"DirectDraw Unlock failed: 0x%08X", hr);
        LogError(errorMsg, hr);
        return hr;
    }

    return 0;
}

int SwapBuffers() {
    return 0;
}

int SetupRenderContext() {
    return 0;
}

int ScrollScreen() {
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[g_surfaceIndex], &fb) != 0) {
        LogError(L"ScrollScreen: LockSurfaceAlt failed", 0);
        return -1;
    }

    WORD* pixels = (WORD*)fb;
    size_t rowSize = g_charHeight * g_width * 2;
    memmove(pixels, pixels + g_charHeight * g_width, g_width * g_height * 2 - rowSize);

    // Fill scrolled area with background
    WORD* dest = pixels + (g_height - g_charHeight) * g_width;
    size_t pixelCount = g_charHeight * g_width;
    for (size_t i = 0; i < pixelCount; i++) {
        dest[i] = g_background;
    }

    if (UnlockSurface(surfaceHandles[g_surfaceIndex]) != 0) {
        LogError(L"ScrollScreen: UnlockSurface failed", 0);
        return -1;
    }
    return 0;
}

void AdjustTextPosition(int x, int y) {
    if (x != -1)
        g_xPos = x;
    if (y != -1)
        g_yPos = y;
}

void AdjustBoundaries(int x) {
    g_maxX = x / g_charWidth - 1;
}

int RenderChar(byte c, int x, int y, WORD color, int scale, bool transparent) {
    if (c < 0x20) c = 0x20;
    if (scale < 1) scale = 1;

    WCHAR logMsg[128];
    if (LOG_PIXELS == 1) {
        wsprintf(logMsg, L"RenderChar: c=0x%02X, x=%d, y=%d, color=0x%04X, scale=%d", c, x, y, color, scale);
        LogError(logMsg, 0);
    }

    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"RenderChar: LockSurfaceAlt failed", 0);
        return -1;
    }

    WORD* pixels = (WORD*)fb;
    DrawChar(c, x, y, color, scale, pixels, transparent);

    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"RenderChar: UnlockSurface failed", 0);
        return -1;
    }
    return 0;
}

int RenderStringWithScale(const byte* str, int scale) {
    int origY = g_yPos;
    while (*str) {
        if (*str == '\r') {
            g_xPos = 0;
            str++;
            continue;
        }
        if (*str == '\n') {
            g_xPos = 0;
            g_yPos++;
            if (g_yPos >= g_maxX) {
                g_yPos = g_maxX - 1;
                ScrollScreen();
            }
            str++;
            continue;
        }

        int x = g_xPos * g_charWidth + g_offsetX;
        int y = g_yPos * g_charHeight + g_offsetY;
        RenderChar(*str, x, y, 0xFFFF, scale, (scale > 1));

        g_xPos++;
        if (g_xPos >= g_maxX - 1) {
            g_xPos = 0;
            g_yPos++;
            if (g_yPos >= g_maxY) {
                g_yPos = g_maxY - 1;
                ScrollScreen();
            }
        }
        str++;
    }
    return 0;
}

int RenderString(const byte* str) {
    return RenderStringWithScale(str, 1);
}

void PrintScreenWithScale(const byte* str, int scale) {
    if (LOG_PIXELS == 1) {
        LogError(L"PrintScreen, surfaceindex:", g_surfaceIndex);
        LogError(L"PrintScreen, scale:", scale);
    }
    RenderStringWithScale(str, scale);
}

void PrintScreen(const byte* str) {
    PrintScreenWithScale(str, 1);
}

int DrawBackground(DWORD color) {
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"DrawBackground: LockSurfaceAlt failed", 0);
        return -1;
    }
    g_background = color;
    WORD* pixels = (WORD*)fb;
    for (DWORD y = 0; y < g_height; y++) {
        for (DWORD x = 0; x < g_width; x++) {
            pixels[y * g_width + x] = color;
        }
    }
    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"DrawBackground: UnlockSurface failed", 0);
        return -1;
    }
    return 0;
}

void PrintToScreen(int scale, const char* format, ...) {
    char buffer[2052];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    PrintScreenWithScale((byte*)buffer, scale);
    va_end(args);
}

void DrawRect(int x, int y, int w, int h, WORD color, WORD* pixels) {
    for (int py = y; py < y + h && py < g_height; py++) {
        if (py < 0) continue;
        for (int px = x; px < x + w && px < g_width; px++) {
            if (px < 0) continue;
            pixels[py * g_width + px] = color;
        }
    }
}

void DrawChar(byte c, int x, int y, WORD color, int scale, WORD* pixels, bool transparent) {
    int charIndex = c - 0x20;
    if (charIndex > 96) {
        charIndex = 31;
    }
    if (LOG_PIXELS == 1) {
        LogError(L"CHARINDEX", charIndex);
    }

    for (int row = 0; row < 22; row++) {
        WORD bits = font[charIndex][row];
        for (int col = 15; col >= 5; col--) {
            bool isPixelSet = bits & (1 << col);
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int px = x + (15 - col) * scale + sx;
                    int py = y + row * scale + sy;
                    if (px >= 0 && px < g_width && py >= 0 && py < g_height) {
                        if (isPixelSet) {
                            pixels[py * g_width + px] = color;
                        } else if (!transparent) {
                            pixels[py * g_width + px] = g_background;
                        }
                    }
                }
            }
        }
    }
}

int ClearArea(int x, int y, int w, int h) {
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"clear_area: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;
    DrawRect(x, y, w, h, g_background, pixels);
    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"clear_area: UnlockSurface failed", 0);
        return -1;
    }
    return 0;
}

int RenderBtnText(int x, int y, const char* text, WORD color, int scale, WORD* pixels) {
    int x_offset = x;
    for (int i = 0; text[i] != '\0'; i++) {
        DrawChar((byte)text[i], x_offset, y, color, scale, pixels, true);
        x_offset += g_charWidth * scale;
    }
    return 0;
}

int RenderButton(int x_right, int y_top, int w, int h, const char* text) {
    int scale = 1;
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"render_button: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;

    int x = x_right - w;
    int y = y_top;

    DrawRect(x, y, w, 2, COLOR_WHITE, pixels);           // Top
    DrawRect(x, y, 2, h, COLOR_WHITE, pixels);           // Left
    DrawRect(x, y + h - 2, w, 2, COLOR_BLACK, pixels);   // Bottom
    DrawRect(x + w - 2, y, 2, h, COLOR_BLACK, pixels);   // Right

    DrawRect(x + 2, y + 2, w - 4, 2, COLOR_LIGHT_GRAY, pixels); // Top
    DrawRect(x + 2, y + 2, 2, h - 4, COLOR_LIGHT_GRAY, pixels); // Left
    DrawRect(x + 2, y + h - 4, w - 4, 2, COLOR_DARK_GRAY, pixels); // Bottom
    DrawRect(x + w - 4, y + 2, 2, h - 4, COLOR_DARK_GRAY, pixels); // Right

    DrawRect(x + 4, y + 4, w - 8, h - 8, COLOR_LIGHT_GRAY, pixels);

    int text_len = strlen(text);
    int text_width = text_len * g_charWidth * scale;
    int text_height = g_charHeight * scale;
    int text_x = x + (int)((w - text_width) / 2.0);
    int text_y = y + (int)((h - text_height) / 2.0);

    if (RenderBtnText(text_x, text_y, text, COLOR_BLACK, scale, pixels) != 0) goto error;

    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"render_button: UnlockSurface failed", 0);
        return -1;
    }
    return 0;

error:
    UnlockSurface(surfaceHandles[0]);
    return -1;
}

int RenderButtonWithState(int x_right, int y_top, int w, int h, const char* text, bool state) {
    int scale = 1;
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"render_button: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;

    int x = x_right - w;
    int y = y_top;

    DrawRect(x, y, w, 2, COLOR_WHITE, pixels);           // Top
    DrawRect(x, y, 2, h, COLOR_WHITE, pixels);           // Left
    DrawRect(x, y + h - 2, w, 2, COLOR_BLACK, pixels);   // Bottom
    DrawRect(x + w - 2, y, 2, h, COLOR_BLACK, pixels);   // Right

    DrawRect(x + 2, y + 2, w - 4, 2, COLOR_LIGHT_GRAY, pixels); // Top
    DrawRect(x + 2, y + 2, 2, h - 4, COLOR_LIGHT_GRAY, pixels); // Left
    DrawRect(x + 2, y + h - 4, w - 4, 2, COLOR_DARK_GRAY, pixels); // Bottom
    DrawRect(x + w - 4, y + 2, 2, h - 4, COLOR_DARK_GRAY, pixels); // Right

    DrawRect(x + 4, y + 4, w - 8, h - 8, state ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY, pixels);

    int text_len = strlen(text);
    int text_width = text_len * g_charWidth * scale;
    int text_height = g_charHeight * scale;
    int text_x = x + (int)((w - text_width) / 2.0);
    int text_y = y + (int)((h - text_height) / 2.0);

    if (RenderBtnText(text_x, text_y, text, state ? COLOR_BLACK : COLOR_WHITE, scale, pixels) != 0) goto error;

    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"render_button: UnlockSurface failed", 0);
        return -1;
    }
    return 0;

error:
    UnlockSurface(surfaceHandles[0]);
    return -1;
}

void ResetTextRenderer() {
    g_xPos = 0;
    g_yPos = 0;
    g_maxX = 0;
    g_maxY = 0;
    g_maxX = g_width / g_charWidth - 1;
    g_maxY = g_height / g_charHeight - 2;
    g_offsetX = 6;
    g_offsetY = 6;
}
