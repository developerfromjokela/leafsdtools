#include "stdafx.h"


typedef void (WINAPI *LPCAPIDebugShellOutput)(char* param_1);


HANDLE hLogFile = INVALID_HANDLE_VALUE;

HINSTANCE hDll = NULL;
LPCAPIDebugShellOutput pDebugShellOutput = NULL;


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

void CheckDebugShellCommand() {
	// Load the DLL

	if (hDll == NULL)
	{
		hDll = LoadLibrary(TEXT("DEBUGSHELLDLL.DLL"));
		if (hDll == NULL)
			return;
	} else {
		return;
	}

	// Get the function address
	if (pDebugShellOutput == NULL)
	{
		pDebugShellOutput = (LPCAPIDebugShellOutput)GetProcAddress(hDll, TEXT("DebugShellOutput"));
	}
}

void SerialLog(char* message) {
	CheckDebugShellCommand();
	if (pDebugShellOutput != NULL) {
		pDebugShellOutput(message);
	}
}

void LogError(LPCWSTR message, HRESULT hr) {
    CheckDebugShellCommand();
    SYSTEMTIME st;
    GetSystemTime(&st);

    WCHAR buffer[256];
    wsprintf(buffer, L"[%04d-%02d-%02d %02d:%02d:%02d] %s: HRESULT = 0x%08X\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message, hr);

	// Out to serial port with conversion to string
    if (pDebugShellOutput != NULL) {
        static char output_buffer[256];
        char mb_str[256];
        if (WideCharToMultiByte(CP_ACP, 0, buffer, -1, mb_str, 256, NULL, NULL) != 0) {
            sprintf(output_buffer, "%s \r\n", mb_str);
            pDebugShellOutput(output_buffer);
        }
    }

    if (hLogFile != INVALID_HANDLE_VALUE) {
        char ansiBuffer[256];
        WideCharToMultiByte(CP_ACP, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), NULL, NULL);
        DWORD bytesWritten;
        WriteFile(hLogFile, ansiBuffer, strlen(ansiBuffer), &bytesWritten, NULL);
        FlushFileBuffers(hLogFile);
    }
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

/*
Flush and close log buffer
*/
void CleanupLog() {
    if (hLogFile != INVALID_HANDLE_VALUE) {
		FlushFileBuffers(hLogFile);
        CloseHandle(hLogFile);
        hLogFile = INVALID_HANDLE_VALUE;
    }
	if (hDll != NULL) {
		FreeLibrary(hDll);
	}
}