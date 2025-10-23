#ifndef READNAND_H
#define READNAND_H

#ifdef QY8XXX
#define MAX_BLOCKS 0x1ff
#else
#define MAX_BLOCKS 0x86
#endif

void RunReadNAND(BOOL unattend);

#endif