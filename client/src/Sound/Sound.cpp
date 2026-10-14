#include "stdafx.h"
#include "Platform/IAudio.h"
#include "Sound/SoundManager.h"

// IDA: InitDirectSound (0x004044A0)
HRESULT __cdecl InitDirectSound(HWND window) { return Platform::GetAudio().Initialize(window); }
// IDA: CreateStaticBuffer (0x00404650)
HRESULT __cdecl CreateStaticBuffer(int slot, const char* path, int channels, bool spatial) {
    return Platform::GetAudio().CreateBuffer(slot, path, channels, spatial);
}

// ============================================================================
// LoadWaveFile  @ 0x00404A10
// ============================================================================
// Top-level loader — guards, calls CreateStaticBuffer + FillBuffer, then
// records metadata. FillBuffer is a no-op in the shipped binary.
// ============================================================================
void __cdecl LoadWaveFile(int Buffer, const char* strFileName,
                          int MaxChannel, bool Enable)
{
    if (!g_EnableSound)               return;
    if (Buffer < 0)                   return;
    if (Buffer >= 420)                return;   // bounds check
    if (MaxBufferChannel[Buffer] > 0) return;   // already loaded

    if (!g_Enable3DSound) Enable = false;       // global 3D disable

    if (FAILED(CreateStaticBuffer(Buffer, strFileName, MaxChannel, Enable))) {
        return;
    }
    // FillBuffer (0x00404A00) would duplicate the buffer into channels [1..3]
    // for polyphony. The shipped binary has it as an empty stub returning 0,
    // so MaxChannel is effectively always 1 in practice.

    BufferChannel[Buffer]    = 0;
    MaxBufferChannel[Buffer] = MaxChannel;
    Enable3DSound[Buffer]    = Enable;
    strncpy(BufferName[Buffer], strFileName, 0x3F);
    BufferName[Buffer][0x3F] = 0;
    ++SoundLoadCount;

    // DESVIACION (DLL): volumen de efectos de Config.ini [Sound] SoundLevel.
    gSound.ApplySoundVolume(Buffer);
}


// ============================================================================
// LoadWaveFile @ 0x00404A10 (IDA: FUN_00404A10; name from 5.2).
// (Monster_Data, Model_Gates, stubs) which pass the 4th arg as char.
// Forwards to LoadWaveFile with bool cast.
// ============================================================================
void __cdecl LoadWaveFile(int Buffer, const char* strFileName,
                          int MaxChannel, char Enable)
{
    LoadWaveFile(Buffer, strFileName, MaxChannel, Enable != 0);
}
