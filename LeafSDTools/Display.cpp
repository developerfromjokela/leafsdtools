#include "stdafx.h"
#include "Font.h"

/*
	Handle Display&Drawing using GDI-Sub driver.
	Windows automotive build for LEAF does not include DirectDraw or Windows.
*/

// Global variables
HMODULE g_hGdisDll;
int g_GDISAPI;
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

typedef int (*GdisInitSurface_t)(GdisSurface* context, DWORD* output);
typedef int (*GdisConfigureContext_t)(DWORD surface, GdisRenderContext* context);
typedef int (*GdisLockSurface_t)(DWORD surface, DWORD* rect, DWORD flags, DWORD* output);
typedef int (*GdisLockSurface2_t)(DWORD surface, DWORD* rect, DWORD flags, DWORD flags2, DWORD* output);
typedef int (*GdisUnlockSurface_t)(DWORD surface);
typedef int (*GdisCreateContext_t)(DWORD surface, DWORD contextId, DWORD* output);
typedef int (*GdisGetContext_t)(DWORD contextId, DWORD param2);
typedef int (*GdisSetRenderParam_t)(DWORD contextId, int mode, DWORD param1);
typedef int (*GdisDestroySurface_t)(DWORD surface);


void custom_memset(void *ptr, size_t size) {
    unsigned char *byte_ptr = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        byte_ptr[i] = 0;
		Sleep(0.3);
    }
}

void custom_memset2(void *ptr, size_t size) {
    unsigned char *byte_ptr = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        byte_ptr[i] = 0;
    }
}


GdisSurface surfaces[2];
DWORD surfaceHandles[4];

int SetupRenderContext() {
    GdisCreateContext_t GdisCreateContext = (GdisCreateContext_t)((g_GDISAPI << 8 | 0x15) * -2 + -0x1ff);
    GdisConfigureContext_t GdisConfigureContext = (GdisConfigureContext_t)((g_GDISAPI << 8 | 6) * -2 + -0x1ff);
    GdisGetContext_t GdisGetContext = (GdisGetContext_t)((g_GDISAPI << 8 | 0x16) * -2 + -0x1ff);
    GdisSetRenderParam_t GdisSetRenderParam = (GdisSetRenderParam_t)((g_GDISAPI << 8 | 0x1C) * -2 + -0x1ff);

    if (!GdisCreateContext || !GdisConfigureContext || !GdisGetContext || !GdisSetRenderParam) {
        LogError(L"GDIS function not found", 0);
        return -1;
    }

    DWORD surface = surfaceHandles[g_surfaceIndex];
    //g_surfaceIndex = (g_surfaceIndex - 1) & 1;

    DWORD contextId;
    int result = GdisCreateContext(surface, 1, &contextId);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisCreateContext failed: 0x%08X", result);
        LogError(errorMsg, result);
        return result;
    }

    GdisRenderContext renderContext;
    custom_memset2(&renderContext, 0x34);
    renderContext.size = 0x34;
    renderContext.width = 800;
    renderContext.height = 0x1e0;
    renderContext.width2 = 800;
    renderContext.height2 = 0x1e0;
    renderContext.max_level = 0xff;
    renderContext.format = 7;
    renderContext.flags = 0x4000;

	if (LOG_SURFACES == 1)
		LogBufferContents(L"GdisRenderContext before init", (DWORD*)&renderContext, sizeof(GdisRenderContext) / 4);

    result = GdisConfigureContext(surface, &renderContext);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisConfigureContext failed: 0x%08X", result);
        LogError(errorMsg, result);
    }
	if (LOG_SURFACES == 1)
		LogError(L"GdisConfigureContext", result);

    result = GdisGetContext(contextId, 0);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisGetContext failed: 0x%08X", result);
        LogError(errorMsg, result);
    }
	if (LOG_SURFACES == 1)
		LogError(L"GdisGetContext", result);

    result = GdisSetRenderParam(contextId, 2, 1000);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisSetRenderParam failed: 0x%08X", result);
        LogError(errorMsg, result);
    }
	if (LOG_SURFACES == 1)
		LogError(L"GdisSetRenderParam", result);

    return 0;
}

int LockSurface(DWORD surface, DWORD* output) {
    GdisLockSurface_t GdisLockSurface = (GdisLockSurface_t)((g_GDISAPI << 8 | 0xB) * -2 + -0x1ff);
    if (!GdisLockSurface) {
        LogError(L"GdisLockSurface function not found", 0);
        return -1;
    }
    int result = GdisLockSurface(surface, NULL, 0, output);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisLockSurface failed: 0x%08X", result);
        LogError(errorMsg, result);
        return result;
    }
    return 0;
}

int LockSurfaceAlt(DWORD surface, DWORD* output) {
    GdisLockSurface2_t GdisLockSurface = (GdisLockSurface2_t)((g_GDISAPI << 8 | 0xC) * -2 + -0x1ff);
    if (!GdisLockSurface) {
        LogError(L"GdisLockSurface (alt) function not found", 0);
        return -1;
    }
    int result = GdisLockSurface(surface, NULL, 0xFFFFFFFF, 0, output);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisLockSurface (alt) failed: 0x%08X", result);
        LogError(errorMsg, result);
        return result;
    }
    return 0;
}

int UnlockSurface(DWORD surface) {
    GdisUnlockSurface_t GdisUnlockSurface = (GdisUnlockSurface_t)((g_GDISAPI << 8 | 0xD) * -2 + -0x1ff);
    if (!GdisUnlockSurface) {
        LogError(L"GdisUnlockSurface function not found", 0);
        return -1;
    }
    int result = GdisUnlockSurface(surface);
    if (result != 0) {
        WCHAR errorMsg[64];
        wsprintf(errorMsg, L"GdisUnlockSurface failed: 0x%08X", result);
        LogError(errorMsg, result);
        return result;
    }
    return 0;
}

// unused, single buffer works good enough ;)
int SwapBuffers() {
    GdisLockSurface_t GdisLockSurface = (GdisLockSurface_t)((g_GDISAPI << 8 | 0xB) * -2 + -0x1ff);
    GdisUnlockSurface_t GdisUnlockSurface = (GdisUnlockSurface_t)((g_GDISAPI << 8 | 0xD) * -2 + -0x1ff);
    if (!GdisLockSurface || !GdisUnlockSurface) {
        LogError(L"SwapBuffers function not found", 0);
        return -1;
    }

    int frontIndex = g_surfaceIndex;
    int backIndex = (g_surfaceIndex - 1) & 1;
    DWORD frontBuffer, backBuffer;
    int result;

    result = LockSurface(surfaceHandles[frontIndex], &frontBuffer);
    if (result != 0) return result;
    result = LockSurface(surfaceHandles[backIndex], &backBuffer);
    if (result != 0) {
        UnlockSurface(surfaceHandles[frontIndex]);
        return result;
    }

    memcpy((void*)frontBuffer, (void*)backBuffer, 0xBB800);

    WCHAR swapMsg[64];
    wsprintf(swapMsg, L"Swapping buffers: front=0x%08X, back=0x%08X", frontBuffer, backBuffer);
    LogError(swapMsg, 0);

    result = UnlockSurface(surfaceHandles[backIndex]);
    if (result != 0) {
        UnlockSurface(surfaceHandles[frontIndex]);
        return result;
    }
    result = UnlockSurface(surfaceHandles[frontIndex]);
    if (result != 0) return result;

    return 0;
}

int InitGraphic() {

    g_hGdisDll = LoadLibrary(L"coredll.dll");
    if (!g_hGdisDll) {
        LogError(L"LoadLibrary failed for coredll.dll", GetLastError());
        return -0x7fffbffb;
    }

    typedef int (*QueryAPISetID_t)(const char*);
    QueryAPISetID_t QueryAPISetID = (QueryAPISetID_t)GetProcAddress(g_hGdisDll, L"QueryAPISetID");
    if (!QueryAPISetID) {
        LogError(L"GetProcAddress failed for QueryAPISetID", GetLastError());
        FreeLibrary(g_hGdisDll);
        return -0x7fffbffb;
    }

    g_GDISAPI = QueryAPISetID("GDIS");
    if (g_GDISAPI == -1) {
        LogError(L"QueryAPISetID failed for GDIS", 0);
        FreeLibrary(g_hGdisDll);
        return -0x7fffbffb;
    }

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

    // Initialize surfaces
    GdisInitSurface_t GdisInitSurface = (GdisInitSurface_t)((g_GDISAPI << 8 | 3) * -2 + -0x1ff);
    if (!GdisInitSurface) {
        LogError(L"GdisInitSurface function not found", 0);
        FreeLibrary(g_hGdisDll);
        return -0x7fffbffb;
    }

    for (int i = 0; i < 2; i++) {
        custom_memset2(&surfaces[i], 0x3c);
        surfaces[i].size = 0x3c;
        surfaces[i].height = 0x1e0;
        surfaces[i].width = 800;
        surfaces[i].pixelFormat = 0x353635;
        surfaces[i].unknown1 = 0x10;
        surfaces[i].unknown2 = 0x10 + (i * 10);
        surfaces[i].flags = 0x81;
        WCHAR surfaceMsg[64];
		if (LOG_SURFACES == 1) {
			wsprintf(surfaceMsg, L"GdisSurface[%d] before init", i);
			LogBufferContents(surfaceMsg, (DWORD*)&surfaces[i], sizeof(GdisSurface) / 4);
		}

        int result = GdisInitSurface(&surfaces[i], &surfaceHandles[i]);
        if (result != 0) {
            wsprintf(surfaceMsg, L"GdisInitSurface failed for surface %d: 0x%08X", i, result);
            LogError(surfaceMsg, result);
            surfaceHandles[i] = 0xffffffff;
            FreeLibrary(g_hGdisDll);
            return result;
        }

		if (LOG_SURFACES == 1) {
			 wsprintf(surfaceMsg, L"surfaceHandles[%d]=0x%08X", i, surfaceHandles[i]);
             LogError(surfaceMsg, 0);
             wsprintf(surfaceMsg, L"GdisSurface[%d] after init", i);
             LogBufferContents(surfaceMsg, (DWORD*)&surfaces[i], sizeof(GdisSurface) / 4);
		}

		// Frame buffer and size assigned! Zero it out to "black"
		// without locking surface, program crashes after writing surface with zer
		// using normal memset.
		LARGE_INTEGER physAddr;
        physAddr.QuadPart = (LONGLONG)surfaces[i].buffer;	
		LPVOID virtualAddr = MmMapIoSpace(physAddr, surfaces[i].bufferSize, FALSE);
		if (!virtualAddr) {
			LogError(L"Failed to map framebuffer", 0);
		} else {
			__try {
				memset(virtualAddr, 0, surfaces[i].bufferSize);
				LogError(L"Framebuffer cleared successfully", 0);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				LogError(L"Exception during framebuffer clear", GetLastError());
			}

			MmUnmapIoSpace(virtualAddr, surfaces[i].bufferSize);
		}
    }

	SetupRenderContext();

    // Set globals
    g_width = surfaces[0].width;
    g_height = surfaces[0].height;
    g_maxX = g_width / g_charWidth - 1;
    g_maxY = g_height / g_charHeight - 2;
    g_offsetX = 6;
    g_offsetY = 6;

    // Crash! But we write background later so doesn't matter ;)
/*	if (surfaceHandles[g_surfaceIndex] != 0xffffffff) {
			 WCHAR surfaceMsg[64];
			 DWORD fb;
             if (LockSurface(surfaceHandles[g_surfaceIndex], &fb) != 0) {
                 LogError(L"RenderChar: LockSurfaceAlt failed for clearing framebuffer", 0);
			 } else {
				WORD* pixels = (WORD*)fb;
				custom_memset(pixels, 400);
				if (UnlockSurface(surfaceHandles[g_surfaceIndex]) != 0) {
                  LogError(L"RenderChar: UnlockSurface failed", 0);
				} else {
				    wsprintf(surfaceMsg, L"Cleared framebuffer for surface %d: buffer=0x%08X, size=0x%08X, using locking and unlocking", 0, surfaces[0].buffer, surfaces[i].bufferSize);
                    LogError(surfaceMsg, 0);
				}
			 }
    }*/
    return 0;
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

int DestroySurfaces() {
    GdisDestroySurface_t GdisDestroySurface = (GdisDestroySurface_t)((g_GDISAPI << 8 | 4) * -2 + -0x1ff);
    if (!GdisDestroySurface) {
        LogError(L"GdisDestroySurface function not found", 0);
        return -1;
    }

    for (int i = 0; i < 2; i++) {
        if (surfaceHandles[i] != 0xffffffff) {
            int result = GdisDestroySurface(surfaceHandles[i]);
            if (result != 0) {
                WCHAR errorMsg[64];
                wsprintf(errorMsg, L"GdisDestroySurface failed for surface %d: 0x%08X", i, result);
                LogError(errorMsg, result);
                return result;
            }
			if (LOG_SURFACES == 1) {
				WCHAR destroyMsg[64];
				wsprintf(destroyMsg, L"Destroyed surface %d: handle=0x%08X", i, surfaceHandles[i]);
				LogError(destroyMsg, 0);
			}
            surfaceHandles[i] = 0xffffffff;
        }
    }
	if (g_hGdisDll)
		FreeLibrary(g_hGdisDll);
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

	// Fill the scrolled area with bg
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

int RenderBtnText(int x, int y, const char* text, WORD color, int scale, WORD* pixels) {
    int x_offset = x;
    for (int i = 0; text[i] != '\0'; i++) {
		DrawChar((byte)text[i], x_offset, y, color, scale, pixels, true);
        x_offset += g_charWidth * scale;
    }
    return 0;
}


int RenderButton(int x_right, int y_top, int w, int h, const char* text) {
    // Lock surface once
	int scale = 1;
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"render_button: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;

    // Calculate top-left corner
    int x = x_right - w; // e.g., 800 - 150 = 650
    int y = y_top;       // e.g., 0

    // Outer border (2px)
    DrawRect(x, y, w, 2, COLOR_WHITE, pixels);           // Top
    DrawRect(x, y, 2, h, COLOR_WHITE, pixels);           // Left
    DrawRect(x, y + h - 2, w, 2, COLOR_BLACK, pixels);   // Bottom
    DrawRect(x + w - 2, y, 2, h, COLOR_BLACK, pixels);   // Right

    // Inner border (2px)
    DrawRect(x + 2, y + 2, w - 4, 2, COLOR_LIGHT_GRAY, pixels); // Top
    DrawRect(x + 2, y + 2, 2, h - 4, COLOR_LIGHT_GRAY, pixels); // Left
    DrawRect(x + 2, y + h - 4, w - 4, 2, COLOR_DARK_GRAY, pixels); // Bottom
    DrawRect(x + w - 4, y + 2, 2, h - 4, COLOR_DARK_GRAY, pixels); // Right

    // Inner fill
    DrawRect(x + 4, y + 4, w - 8, h - 8, COLOR_LIGHT_GRAY, pixels);

    // Center text
    int text_len = strlen(text);
    int text_width = text_len * g_charWidth * scale;
    int text_height = g_charHeight * scale;
    int text_x = x + (int)((w - text_width) / 2.0);
    int text_y = y + (int)((h - text_height) / 2.0);

    if (RenderBtnText(text_x, text_y, text, COLOR_BLACK, scale, pixels) != 0) goto error;

    // Unlock surface
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
    // Lock surface once
	int scale = 1;
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"render_button: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;

    // Calculate top-left corner
    int x = x_right - w;
    int y = y_top;

    // Outer border (2px)
    DrawRect(x, y, w, 2, COLOR_WHITE, pixels);           // Top
    DrawRect(x, y, 2, h, COLOR_WHITE, pixels);           // Left
    DrawRect(x, y + h - 2, w, 2, COLOR_BLACK, pixels);   // Bottom
    DrawRect(x + w - 2, y, 2, h, COLOR_BLACK, pixels);   // Right

    // Inner border (2px)
    DrawRect(x + 2, y + 2, w - 4, 2, COLOR_LIGHT_GRAY, pixels); // Top
    DrawRect(x + 2, y + 2, 2, h - 4, COLOR_LIGHT_GRAY, pixels); // Left
    DrawRect(x + 2, y + h - 4, w - 4, 2, COLOR_DARK_GRAY, pixels); // Bottom
    DrawRect(x + w - 4, y + 2, 2, h - 4, COLOR_DARK_GRAY, pixels); // Right

    // Inner fill
    DrawRect(x + 4, y + 4, w - 8, h - 8, state ? COLOR_LIGHT_GRAY : COLOR_DARK_GRAY, pixels);

    // Center text
    int text_len = strlen(text);
    int text_width = text_len * g_charWidth * scale;
    int text_height = g_charHeight * scale;
    int text_x = x + (int)((w - text_width) / 2.0);
    int text_y = y + (int)((h - text_height) / 2.0);

    if (RenderBtnText(text_x, text_y, text, state ? COLOR_BLACK : COLOR_WHITE, scale, pixels) != 0) goto error;

    // Unlock surface
    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"render_button: UnlockSurface failed", 0);
        return -1;
    }
    return 0;

error:
    UnlockSurface(surfaceHandles[0]);
    return -1;
}

int ClearArea(int x, int y, int w, int h) {
    // Lock surface
    DWORD fb;
    if (LockSurfaceAlt(surfaceHandles[0], &fb) != 0) {
        LogError(L"clear_area: LockSurfaceAlt failed", 0);
        return -1;
    }
    WORD* pixels = (WORD*)fb;

    // Clear the specified area
    DrawRect(x, y, w, h, g_background, pixels);

    // Unlock surface
    if (UnlockSurface(surfaceHandles[0]) != 0) {
        LogError(L"clear_area: UnlockSurface failed", 0);
        return -1;
    }
    return 0;
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



void AdjustTextPosition(int x, int y) {
    if (x != -1)
	  g_xPos = x;
	if (y != -1)
	  g_yPos = y;
}

void AdjustBoundaries(int x) {
    g_maxX = x / g_charWidth - 1;
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
    //SwapBuffers();
    RenderStringWithScale(str, scale);
	SetupRenderContext();
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
	SetupRenderContext();
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
