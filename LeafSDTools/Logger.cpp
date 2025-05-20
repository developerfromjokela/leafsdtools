#include "stdafx.h"


HANDLE hLogFile = INVALID_HANDLE_VALUE;

int InitLogFile(LPCWSTR path) {
    if (hLogFile != INVALID_HANDLE_VALUE) return -1;
    for (int i = 0; i < 30*4; i++) {
        DWORD dwAttrib = GetFileAttributes(path);
        BOOL fileExists = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
        hLogFile = CreateFile(
            path,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            fileExists ? OPEN_EXISTING : CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (hLogFile != INVALID_HANDLE_VALUE) {
            SetFilePointer(hLogFile, 0, NULL, FILE_END);
            return 0;
        }
        Sleep(250);
    }
	return 1;
}

bool IsLoggingEnabled() {
	return (hLogFile != INVALID_HANDLE_VALUE);
}


void LogError(LPCWSTR message, HRESULT hr) {
    if (hLogFile == INVALID_HANDLE_VALUE) return;
    SYSTEMTIME st;
    GetSystemTime(&st);
    WCHAR buffer[256];
    wsprintf(buffer, L"[%04d-%02d-%02d %02d:%02d:%02d] %s: HRESULT = 0x%08X\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message, hr);
    DWORD bytesWritten;
    char ansiBuffer[256];
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), NULL, NULL);
    WriteFile(hLogFile, ansiBuffer, strlen(ansiBuffer), &bytesWritten, NULL);
	FlushFileBuffers(hLogFile);
}

void LogBufferContents(LPCWSTR prefix, DWORD* buffer, int size) {
    if (hLogFile == INVALID_HANDLE_VALUE) return;
    WCHAR bufferLog[512];
    wsprintf(bufferLog, L"%s: ", prefix);
    for (int i = 0; i < size && i < 20; i++) {
        WCHAR temp[32];
        wsprintf(temp, L"[0x%X]=0x%X ", i * 4, buffer[i]);
        wcscat(bufferLog, temp);
    }
    LogError(bufferLog, 0);
}

void CleanupLog(void) {
    if (hLogFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hLogFile);
        hLogFile = INVALID_HANDLE_VALUE;
    }
}