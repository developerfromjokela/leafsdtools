#ifndef FLASH_H
#define FLASH_H

#define SD1_DISK L"DSK1:"
#define SD2_DISK L"DSK8:"

int GetProdSection(CHAR* modelName, BYTE* productId, BYTE* serial, BYTE* pin);
int ReadSingleFlashBlock(int block, DWORD size, BYTE* output);

// SD
int ReadFullSDPin(BYTE* pin);
int ChangeSDLock(LPCWSTR disk, BYTE* pin, int command);
int CheckMountStatus(WORD command);
void InitSDCards();
void SDController_CMD17();

#endif