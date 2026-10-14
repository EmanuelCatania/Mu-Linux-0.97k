// Sound.cpp
// DirectSound8 audio system — WAV loading, buffer management, 3D audio.
//
// Ported from IDA Hex-Rays decompilations (mu97k-src-IDA/raw/):
//   004044A0_InitDirectSound.c    — DirectSoundCreate + primary buffer + 3D listener
//   00404650_CreateStaticBuffer.c — IDirectSound::CreateSoundBuffer + Lock/Fill/Unlock
//   00404A10_LoadWaveFile.c       — top-level loader (guards + metadata writeback)
//   00404A00_FillBuffer.c         — no-op in the shipped binary (empty return 0)
//   00404E90 / 004050A0           — CWaveFile::Open / CWaveFile::Read (MMIO)
//
// All state lives in globals.cpp: g_EnableSound, g_Enable3DSound, g_lpDS,
// g_lpDS3DListener, g_dwBufferBytes, wavefile, g_lpDSBuffer[420][4],
// g_lpDS3DBuffer[420][4], MaxBufferChannel[], BufferChannel[], Enable3DSound[],
// BufferName[][], SoundLoadCount.

#include "stdafx.h"
#include "Sound/SoundManager.h"
#pragma warning(disable: 4996)
#include "Sound/Sound.h"
#include "Platform/IAudio.h"
#include <dsound.h>
#include <mmsystem.h>

// We don't link dxguid.lib — define the GUIDs locally (they're constants).
#ifndef DSOUND_GUIDS_DEFINED_LOCAL
#define DSOUND_GUIDS_DEFINED_LOCAL
// dxguid.lib is listed in vcxproj, so these symbols resolve from it.
#endif

namespace Platform {
namespace {
class Win32Audio final : public IAudio {
public:
    int32_t Initialize(void* window) override;
    int32_t CreateBuffer(int Buffer, const char* strFileName, int MaxChannel, bool Enable) override;
    int32_t Play(int Buffer, uint32_t Object, bool bLooped) override;
    void Stop(int Buffer) override;
    int32_t ReleaseBuffer(int Buffer) override;
    void SetVolume(int slot, int channel, int32_t hundredthsDb) override;
    void SetPosition(int slot, int channel, float x, float y, float z) override;
    void Shutdown() override;
};
}
IAudio& GetAudio() { static Win32Audio audio; return audio; }
}
using Platform::Win32Audio;

// ============================================================================
// CWaveFile — see Sound.h for struct layout.
// ============================================================================

// CWaveFile::ctor  @ 0x00404e20
void CWaveFile_ctor(CWaveFile* self, BYTE is_streaming)
{
    self->vtable       = (void**)0x005524c0;
    self->hmmio        = (HMMIO)0;
    self->buffer_flags = 0;
    self->is_streaming = is_streaming;
}

// CWaveFile::Open  @ 0x00404e90
// Parses RIFF/WAVE via MMIO, positions hmmio at start of 'data' chunk,
// writes data-chunk size into g_dwBufferBytes (DAT_00583da8).
int CWaveFile_Open(CWaveFile* self, LPSTR filename)
{
    if (self->is_streaming != 0) return 0;

    MMCKINFO ckRiff, ckChunk;
    self->hmmio = mmioOpenA(filename, NULL, MMIO_ALLOCBUF);
    if (!self->hmmio) return 0;

    if (mmioDescend(self->hmmio, &ckRiff, NULL, 0) != 0) goto fail;
    if (ckRiff.ckid    != mmioFOURCC('R','I','F','F')) goto fail;
    if (ckRiff.fccType != mmioFOURCC('W','A','V','E')) goto fail;

    ckChunk.ckid = mmioFOURCC('f','m','t',' ');
    if (mmioDescend(self->hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0) goto fail;
    if (mmioRead(self->hmmio, (HPSTR)&self->wFormatTag, 0x10) == -1) goto fail;

    if (self->wBitsPerSample == 8) self->buffer_flags = 0x80;
    if (self->wFormatTag != WAVE_FORMAT_PCM) goto fail;

    mmioAscend(self->hmmio, &ckChunk, 0);

    ckChunk.ckid = mmioFOURCC('d','a','t','a');
    if (mmioDescend(self->hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0) goto fail;

    self->remaining_size = (int)ckChunk.cksize;
    g_dwBufferBytes      = self->remaining_size;
    return 1;

fail:
    mmioClose(self->hmmio, 0);
    self->hmmio = (HMMIO)0;
    return 0;
}

// CWaveFile::Read  @ 0x004050a0
int CWaveFile_Read(CWaveFile* self, HPSTR buffer, UINT size)
{
    if (self->is_streaming != 0) return 0;

    if ((UINT)self->remaining_size < size) {
        for (UINT i = 0; i < size; ++i) buffer[i] = 0;
    }
    LONG read = mmioRead(self->hmmio, buffer, size);
    if (read < (LONG)size) return 0;

    self->remaining_size -= (int)size;
    return 1;
}

static void CWaveFile_Close(CWaveFile* self)
{
    if (self && self->hmmio) {
        mmioClose(self->hmmio, 0);
        self->hmmio = (HMMIO)0;
    }
}


// ============================================================================
// InitDirectSound  @ 0x004044A0
// ============================================================================
// IDA:
//   CoInitialize(NULL)
//   DirectSoundCreate(NULL, &g_lpDS, NULL)
//   SetCooperativeLevel(hWnd, DSSCL_PRIORITY=2)
//   CreateSoundBuffer(primary DSBCAPS_PRIMARYBUFFER|CTRL3D)
//   QueryInterface(IID_IDirectSound3DListener) → g_lpDS3DListener
//   primary->SetFormat(22050 Hz / 16-bit / stereo)
//   Zero g_lpDSBuffer[0..420][0..4]
//   g_EnableSound = 1
//
// The `if (!g_Enable3DSound)` guard in the original prevents re-init; in this
// build g_Enable3DSound is set by CreateStaticBuffer per-slot, not globally,
// so we use g_EnableSound as the re-init guard instead.
// ============================================================================
// IDA: InitDirectSound (0x004044A0)
int32_t Win32Audio::Initialize(void* window)
{
    if (g_EnableSound) return S_OK;   // already initialised

    CoInitialize(NULL);

    HRESULT hr = DirectSoundCreate(NULL, &g_lpDS, NULL);
    if (FAILED(hr)) return hr;

    hr = g_lpDS->SetCooperativeLevel(static_cast<HWND>(window), DSSCL_PRIORITY);
    if (FAILED(hr)) { g_lpDS->Release(); g_lpDS = NULL; return hr; }

    // Primary buffer — needed to get the 3D listener interface and to set
    // hardware mixing format.
    DSBUFFERDESC dsbd = {};
    dsbd.dwSize  = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;  // 0x11
    LPDIRECTSOUNDBUFFER lpDSBPrimary = NULL;
    hr = g_lpDS->CreateSoundBuffer(&dsbd, &lpDSBPrimary, NULL);
    if (FAILED(hr)) { g_lpDS->Release(); g_lpDS = NULL; return hr; }

    hr = lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener,
                                      (LPVOID*)&g_lpDS3DListener);
    if (FAILED(hr)) {
        lpDSBPrimary->Release();
        g_lpDS->Release(); g_lpDS = NULL;
        return hr;
    }

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 2;
    wfx.nSamplesPerSec  = 22050;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = 4;
    wfx.nAvgBytesPerSec = 88200;
    wfx.cbSize          = 0;
    hr = lpDSBPrimary->SetFormat(&wfx);
    // SetFormat failure is non-fatal on many drivers — proceed regardless.

    // Zero the per-slot buffer arrays (defensive; globals.cpp already does this).
    for (int i = 0; i < 420; ++i) {
        for (int j = 0; j < 4; ++j) {
            g_lpDSBuffer[i][j]   = NULL;
            g_lpDS3DBuffer[i][j] = NULL;
            Object3DSound[i][j]  = 0;
        }
        MaxBufferChannel[i] = 0;
        BufferChannel[i]    = 0;
        Enable3DSound[i]    = false;
    }

    if (lpDSBPrimary) lpDSBPrimary->Release();
    g_EnableSound = TRUE;
    return S_OK;
}


// ============================================================================
// CreateStaticBuffer  @ 0x00404650
// ============================================================================
// Allocates a CWaveFile on the heap, parses the header, then creates a
// secondary IDirectSoundBuffer filled with the WAV's sample data.
// For Enable!=0, also creates IDirectSound3DBuffer and sets an initial
// "far away" position (so the sound starts silent until SetPosition updates).
// Only slot [Buffer][0] is populated — the original FillBuffer (0x00404A00)
// that would have duplicated the buffer into slots [1..3] is a no-op in this
// binary, so multi-instance playback is not available.
// ============================================================================
// IDA: CreateStaticBuffer (0x00404650)
int32_t Win32Audio::CreateBuffer(int Buffer, const char* strFileName,
                                   int /*MaxChannel*/, bool Enable)
{
    if (!g_lpDS) return E_FAIL;

    // 1. Alloc + open wave file.
    CWaveFile* pWave = (CWaveFile*)operator_new(sizeof(CWaveFile));
    if (!pWave) return E_OUTOFMEMORY;
    CWaveFile_ctor(pWave, 0);
    wavefile = pWave;

    if (!CWaveFile_Open(pWave, (LPSTR)strFileName)) {
        operator_delete(pWave);
        wavefile = NULL;
        return E_FAIL;
    }

    // 2. Build WAVEFORMATEX from parsed header (first 18 bytes of CWaveFile).
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = pWave->wFormatTag;
    wfx.nChannels       = pWave->nChannels;
    wfx.nSamplesPerSec  = pWave->nSamplesPerSec;
    wfx.nAvgBytesPerSec = pWave->nAvgBytesPerSec;
    wfx.nBlockAlign     = pWave->nBlockAlign;
    wfx.wBitsPerSample  = pWave->wBitsPerSample;
    wfx.cbSize          = 0;

    DSBUFFERDESC dsbd = {};
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    // IDA: flags = (-Enable & 0x34) + 0xE2 →
    //   Enable=0 → 0xE2 = CTRLFREQUENCY|CTRLVOLUME|STATIC|CTRLPAN
    //   Enable=1 → 0xEE then clamped to 0xE2+0x34=0x116? Actually -1&0x34=0x34, +0xE2=0x116 (overflow to low 8).
    //   Properly interpreted: non-3D = 0xE2, 3D = 0xE2 XOR CTRLPAN_CTRL3D = 0x32+0xC0=...
    //   Simplest and correct: DSBCAPS_CTRL3D|DSBCAPS_STATIC for 3D, the non-3D flags for 2D.
    dsbd.dwFlags = Enable
        ? (DSBCAPS_CTRL3D | DSBCAPS_STATIC | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME)
        : (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_STATIC);
    dsbd.dwBufferBytes = g_dwBufferBytes;
    dsbd.lpwfxFormat   = &wfx;

    // 3. Create buffer in slot [Buffer][0].
    LPDIRECTSOUNDBUFFER* ppBuf = &g_lpDSBuffer[Buffer][0];
    HRESULT hr = g_lpDS->CreateSoundBuffer(&dsbd, ppBuf, NULL);
    if (FAILED(hr) || !*ppBuf) {
        CWaveFile_Close(pWave);
        operator_delete(pWave);
        wavefile = NULL;
        return hr;
    }

    // 4. If 3D, get the IDirectSound3DBuffer + initial position.
    if (Enable) {
        LPDIRECTSOUND3DBUFFER* pp3D = &g_lpDS3DBuffer[Buffer][0];
        hr = (*ppBuf)->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)pp3D);
        if (SUCCEEDED(hr) && *pp3D) {
            // Far-away deferred position so sample is inaudible until the
            // per-frame Sound_UpdatePositions repositions it near the listener.
            (*pp3D)->SetPosition(-2.0f, 0.0f, 2.0f, DS3D_DEFERRED);
        }
    }

    // 5. Lock → read PCM → unlock.
    HPSTR ptr1 = NULL; DWORD sz1 = 0;
    HPSTR ptr2 = NULL; DWORD sz2 = 0;
    hr = (*ppBuf)->Lock(0, g_dwBufferBytes, (LPVOID*)&ptr1, &sz1,
                              (LPVOID*)&ptr2, &sz2, 0);
    if (FAILED(hr)) goto fail_release;

    if (sz1 != g_dwBufferBytes) { (*ppBuf)->Unlock(ptr1, sz1, ptr2, sz2); goto fail_release; }

    if (!CWaveFile_Read(pWave, ptr1, g_dwBufferBytes)) {
        (*ppBuf)->Unlock(ptr1, sz1, ptr2, sz2);
        goto fail_release;
    }
    (*ppBuf)->Unlock(ptr1, sz1, ptr2, sz2);

    CWaveFile_Close(pWave);
    operator_delete(pWave);
    wavefile = NULL;
    return S_OK;

fail_release:
    if (*ppBuf) { (*ppBuf)->Release(); *ppBuf = NULL; }
    if (g_lpDS3DBuffer[Buffer][0]) {
        g_lpDS3DBuffer[Buffer][0]->Release();
        g_lpDS3DBuffer[Buffer][0] = NULL;
    }
    CWaveFile_Close(pWave);
    operator_delete(pWave);
    wavefile = NULL;
    return E_FAIL;
}



static int SetHall(int /*Buffer*/) { return 1; }

// IDA: PlayBuffer (0x00404BC0)
int32_t Win32Audio::Play(int Buffer, uint32_t Object, bool bLooped)
{
    if (!g_EnableSound)   return S_OK;
    if (Buffer < 0)       return S_OK;
    if (Buffer >= 420)    return S_OK;

    // Guard del port (IDA no lo tiene): g_lpDSBuffer es [420][4], asi que un
    // BufferChannel fuera de rango indexaria memoria ajena.
    if (BufferChannel[Buffer] < 0 || BufferChannel[Buffer] >= 4) {
        BufferChannel[Buffer] = 0;
    }

    LPDIRECTSOUNDBUFFER pBuf = g_lpDSBuffer[Buffer][BufferChannel[Buffer]];
    if (!pBuf) return E_FAIL;   // IDA devuelve 0x80004005

    DWORD flags = bLooped ? DSBPLAY_LOOPING : 0;
    HRESULT hr = pBuf->Play(0, 0, flags);
    if (FAILED(hr)) return hr;

    if (Enable3DSound[Buffer]) {
        Object3DSound[Buffer][BufferChannel[Buffer]] = Object;
        SetHall(Buffer);
    }

    // OJO: NO incrementar BufferChannel aca.
    //
    // IDA 0x00404BC0 hace solo el wrap, sin INC previo:
    //     if (BufferChannel[Buffer] >= MaxBufferChannel[Buffer]) BufferChannel[Buffer] = 0;
    // O sea el canal queda clavado en 0 — que es el UNICO que existe, porque
    // FillBuffer (0x00404A00) es `return 0` y nunca crea los canales 1..3.
    //
    // Un port previo agrego un `BufferChannel[Buffer]++` aca ("el disassembly
    // muestra un INC, lo restauro") y eso mataba el audio: al rotar al canal 1
    // el puntero es NULL, la funcion retorna ANTES del wrap y el slot queda
    // trabado en 1 para siempre. Efecto: todo sonido cargado con MaxChannel>=2
    // (pasos 8-11, golpes 50-56, swing 40-42, arco 65-67, gritos 75-80, magias
    // 86-92) sonaba UNA vez y despues enmudecia; los de MaxChannel==1 (jewel 49,
    // herrero 120, ambientes) seguian andando porque 1 >= 1 los devolvia a 0.
    if (BufferChannel[Buffer] >= MaxBufferChannel[Buffer]) {
        BufferChannel[Buffer] = 0;
    }
    return S_OK;
}

// IDA: Sound_StopBuffer (0x00404C60)
void Win32Audio::Stop(int Buffer) {
    if (!g_EnableSound || Buffer < 0) return;
    LPDIRECTSOUNDBUFFER pBuf = g_lpDSBuffer[Buffer][0];
    if (pBuf) {
        pBuf->Stop();
        if (Buffer != 0) pBuf->SetCurrentPosition(0);
    }
}

// IDA: Sound_ReleaseBuffer (0x00404AD0)
int32_t Win32Audio::ReleaseBuffer(int Buffer)
{
  if (!g_EnableSound) return 0;
  if (Buffer < 0 || Buffer >= 420) return 0;

  int channels = MaxBufferChannel[Buffer];
  if (channels < 0) channels = 0;
  if (channels > 4) channels = 4;

  for (int ch = 0; ch < channels; ++ch) {
    LPDIRECTSOUNDBUFFER v4 = g_lpDSBuffer[Buffer][ch];
    if (v4) {
      v4->Release();
      g_lpDSBuffer[Buffer][ch] = nullptr;
    }
    if (Enable3DSound[Buffer]) {
      LPDIRECTSOUND3DBUFFER v5 = g_lpDS3DBuffer[Buffer][ch];
      if (v5) {
        v5->Release();
        g_lpDS3DBuffer[Buffer][ch] = nullptr;
      }
    }
  }
  MaxBufferChannel[Buffer] = 0;
  Object3DSound[Buffer][0] = 0;
  Object3DSound[Buffer][1] = 0;
  Object3DSound[Buffer][2] = 0;
  Object3DSound[Buffer][3] = 0;
  if (SoundLoadCount > 0) --SoundLoadCount;
  return 0;
}

// IDA: FreeDirectSound (0x00404B80)
void Win32Audio::Shutdown(void) {
    if (!g_EnableSound) return;

    for (int i = 0; i < 420; ++i) {
        for (int c = 0; c < 4; ++c) {
            if (g_lpDS3DBuffer[i][c]) { g_lpDS3DBuffer[i][c]->Release(); g_lpDS3DBuffer[i][c] = NULL; }
            if (g_lpDSBuffer  [i][c]) { g_lpDSBuffer  [i][c]->Release(); g_lpDSBuffer  [i][c] = NULL; }
        }
        MaxBufferChannel[i] = 0;
        BufferChannel[i]    = 0;
        Enable3DSound[i]    = false;
    }
    if (g_lpDS3DListener) { g_lpDS3DListener->Release(); g_lpDS3DListener = NULL; }
    if (g_lpDS)           { g_lpDS->Release();           g_lpDS = NULL; }

    g_EnableSound = FALSE;
}


static void __cdecl FUN_00404e60_impl(int param_1) {
    *(int *)param_1 = (int)&PTR_FUN_005524c0;
    if (*(HANDLE *)(param_1 + 4) != NULL) {
        mmioClose((HMMIO)*(HANDLE *)(param_1 + 4), 0);
    }
}

void __fastcall waveIO__dtor(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00404e60_impl(ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

void Win32Audio::SetVolume(int slot, int channel, int32_t hundredthsDb) {
    if (g_lpDSBuffer[slot][channel]) g_lpDSBuffer[slot][channel]->SetVolume(hundredthsDb);
}
void Win32Audio::SetPosition(int slot, int channel, float x, float y, float z) {
    if (g_lpDS3DBuffer[slot][channel])
        g_lpDS3DBuffer[slot][channel]->SetPosition(x, y, z, DS3D_DEFERRED);
}
