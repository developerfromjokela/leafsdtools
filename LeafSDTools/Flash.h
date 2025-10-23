#ifndef FLASH_H
#define FLASH_H

#define SD1_DISK L"DSK1:"
#define SD2_DISK L"DSK8:"

#define LOG_FLASH_WRITING false

#define FLASH_CONTROL_IO_SIZE 0x10

// Flash
int GetProdSection(CHAR* modelName, BYTE* productId, BYTE* productId2, BYTE* serial, BYTE* pin);
int ReadSingleFlashBlock(int block, DWORD size, BYTE* output);
bool WriteSingleBlockFromFile(HANDLE flashDevice, FILE* file, BYTE* buffer, DWORD* ioControlInput, DWORD block, DWORD blockSize);

// SD
int ReadFullSDPin(BYTE* pin);
int ChangeSDLock(LPCWSTR disk, BYTE* pin, int command);
int CheckMountStatus(WORD command);
void InitSDCards();
void SDController_CMD17();

#endif