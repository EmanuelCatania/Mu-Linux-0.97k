#pragma once
// Sound.h — DirectSound8 + MMIO: CWaveFile, Sound_LoadAndPlay, Sound_CreateBuffer.
#include <dsound.h>
#include <mmsystem.h>

struct CWaveFile   // 0x2C bytes
{
    void**   vtable;           // +0x00  → PTR_FUN_005524c0
    HMMIO    hmmio;            // +0x04
    WORD     wFormatTag;       // +0x08  1=PCM
    WORD     nChannels;        // +0x0A
    DWORD    nSamplesPerSec;   // +0x0C
    DWORD    nAvgBytesPerSec;  // +0x10
    WORD     nBlockAlign;      // +0x14
    WORD     wBitsPerSample;   // +0x16
    WORD     cbSize;           // +0x18
    WORD     _pad;             // +0x1A
    int      remaining_size;   // +0x20
    UINT     buffer_flags;     // +0x24   0x80 if 8-bit
    BYTE     is_streaming;     // +0x28
};

void    CWaveFile_ctor(CWaveFile* self, BYTE is_streaming);
int     CWaveFile_Open(CWaveFile* self, LPSTR filename);
int     CWaveFile_Read(CWaveFile* self, HPSTR buffer, UINT size);
int     Sound_CreateBuffer(int id, LPSTR filename, int max_inst, BYTE is_3d);
void    Sound_LoadAndPlay(int id, LPSTR filename, int max_inst, char is_3d);
