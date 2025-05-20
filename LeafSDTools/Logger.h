#ifndef LOGGER_H
#define LOGGER_H

int InitLogFile(LPCWSTR path);
void LogError(LPCWSTR message, HRESULT hr);
void LogBufferContents(LPCWSTR prefix, DWORD* buffer, int size);
void CleanupLog();
bool IsLoggingEnabled();

#endif