// Sound_DS3D.cpp
// DirectSound8 playback + per-frame 3D positional update.
//
// PlayBuffer        @ 0x00404BC0 — start playback on a loaded slot (also PlayBuffer)
// Sound_UpdatePos   @ 0x00404CD0 — per-frame 3D listener-relative SetPosition
// SetHall           @ 0x00404BB0 — stub in original (returns 1)
//
// State lives in globals.cpp. See src/Sound/Sound.cpp for the loader side.

#include "stdafx.h"
#include "Platform/ITimer.h"
#include "Platform/IAudio.h"




// ============================================================================
// SetHall @ 0x00404BB0 — empty stub in the shipped binary (returns 1).
// Originally intended to apply a reverb/environment preset via IKsPropertySet
// (EAX), but the final release left this as a no-op.
// ============================================================================



// ============================================================================
// PlayBuffer / PlayBuffer  @ 0x00404BC0
// ============================================================================
// Plays the secondary buffer for slot [Buffer][BufferChannel[Buffer]].
//   Buffer  — sound ID (the same index passed to LoadWaveFile).
//   Object  — entity pointer (3D positional anchor), stored for per-frame update.
//   bLooped — non-zero → DSBPLAY_LOOPING.
//
// Returns S_OK (0) on success, or the HRESULT from Play on failure. Matches
// IDA 00404BC0_PlayBuffer.c semantics (including the channel-wrap reset).
// ============================================================================
// IDA: PlayBuffer (0x00404BC0)
HRESULT __cdecl PlayBuffer(int Buffer, DWORD Object, BOOL bLooped)
{
    if (!g_EnableSound || Buffer < 0 || Buffer >= 420) return S_OK;
    if (BufferChannel[Buffer] < 0 || BufferChannel[Buffer] >= 4) BufferChannel[Buffer] = 0;
    const HRESULT result = Platform::GetAudio().Play(Buffer, BufferChannel[Buffer], bLooped != 0);
    if (FAILED(result)) return result;
    if (Enable3DSound[Buffer]) Object3DSound[Buffer][BufferChannel[Buffer]] = Object;
    // IDA: PlayBuffer (0x00404BC0), sólo wrap: FillBuffer no crea canales adicionales.
    if (BufferChannel[Buffer] >= MaxBufferChannel[Buffer]) BufferChannel[Buffer] = 0;
    return S_OK;
}



// ============================================================================
// Sound_UpdatePositions @ 0x00404CD0
// ============================================================================
// Builds a rotation matrix from the player's yaw, then for each active 3D
// sound slot, computes the listener-space XZ offset from the bound entity,
// scales by the DS3D unit conversion, and calls SetPosition on each active
// 3D buffer.
//
// Requires: g_EnableSound AND g_Enable3DSound both true, plus at least one
// slot with Enable3DSound[i] && MaxBufferChannel[i] > 0 (i.e. a 3D sound is
// loaded and playing).
// ============================================================================
// Matrix_BuildFromEuler (EulerToMatrix3x4) and Vector_Rotate (Vec3_TransformByMatrix)
// are declared in functions.h with the canonical signatures.

// IDA: FUN_00404CD0
void Sound_Update3DPositions(void)
{
    if (!g_EnableSound || !g_Enable3DSound) return;

    float eulerAngles[3];
    float rotMatrix[12];
    eulerAngles[0] = 0.0f;
    eulerAngles[1] = 0.0f;
    eulerAngles[2] = Ff(DAT_083a42c0);      // player yaw (CameraAngle[2] bits)
    Matrix_BuildFromEuler(eulerAngles, rotMatrix);

    const float unitScale = _DAT_005524bc;  // world→DS3D unit conversion

    for (int slot = 0; slot < 420; ++slot) {
        if (!Enable3DSound[slot])          continue;
        if (MaxBufferChannel[slot] <= 0)   continue;

        for (int ch = 0; ch < MaxBufferChannel[slot]; ++ch) {
            DWORD entityPtr = Object3DSound[slot][ch];
            if (entityPtr == 0) continue;

            // Player world pos (DAT_07abf5d8) minus entity world pos.
            float offset[3];
            offset[0] = *(float*)(DAT_07abf5d8 + 0x10) - *(float*)(entityPtr + 0x10);
            offset[1] = *(float*)(DAT_07abf5d8 + 0x14) - *(float*)(entityPtr + 0x14);
            offset[2] = *(float*)(DAT_07abf5d8 + 0x18) - *(float*)(entityPtr + 0x18);

            // Rotate into listener space.
            Vector_Rotate(offset, rotMatrix, offset);
            offset[0] *= unitScale;
            offset[1] *= unitScale;
            offset[2] *= unitScale;

            // IDA: Set3DSoundPosition (0x00404CD0): XY rotado pasa a XZ del audio, altura cero.
            Platform::GetAudio().SetPosition(slot, ch, -offset[0], 0.0f, -offset[1]);
        }
    }
}



// Sound_StopBuffer @ 0x00404C60 — stop a slot's currently-playing channel.
// For non-zero Buffer, also rewind to position 0. Uses g_lpDSBuffer[Buffer][0]
// (only channel 0 is populated in this binary's pipeline — FillBuffer is a no-op).
// IDA: FUN_00404C60
void __cdecl Sound_StopBuffer(int Buffer) { if (g_EnableSound && Buffer >= 0) Platform::GetAudio().Stop(Buffer); }

// waveIO__dtor @ 0x00404E40 — CWaveFile ~dtor (calls waveIO__CloseWaveFile)


// IDA signature is __thiscall(this); function.h decl is __stdcall(void).
// No live callers in our build (vtable dispatch via CErrorReport disabled),
// El logger activo del cliente es debug.log.
void __stdcall FUN_00405340(void) {
    // No-op — CErrorReport vtable not yet active.
}

// CErrorReport::WriteDebugInfoStr @ 0x00405500 (IDA: CErrorReport__WriteDebugInfoStr).
// (declared in Ghidra as CErrorReport::WriteDebugInfoStr — writes debug string)
// This is a thin wrapper that calls CErrorReport__Write; implementation is in the
// vtable dispatch. We stub it as a pass-through.
void __cdecl CErrorReport_WriteDebugInfoStr(DWORD This, char *fmt) {
    CErrorReport__Write(This, fmt);
}

// IDA: CErrorReport::Write (0x00405540)
void __cdecl CErrorReport_Write(void*,const char*,...)            {} // debug log — kept as stub

// CErrorReport::WriteLogBegin @ 0x00405590 (IDA: CErrorReport__WriteLogBegin).
void __fastcall CErrorReport_WriteLogBegin(DWORD This) {
    CErrorReport__Write(This, (char *)"========Log Begin========");
}

// CErrorReport::WriteCurrentTime @ 0x004055A0 (IDA: FUN_004055A0).
// Logs current local date/time via CErrorReport_Write (debug log sink at DAT_055C9BF0).
// If param_1 != 0, logs an additional data block from DAT_00558128.
void CErrorReport_WriteCurrentTime(int param_1) {
    const auto local_10 = Platform::GetTimer().LocalTime();
    CErrorReport_Write(&DAT_055c9bf0, "%4d %02d %02d %02d %02d"); // date+time format
    if (param_1 != 0) {
        CErrorReport_Write(&DAT_055c9bf0, DAT_00558128);
    }
}

// CErrorReport::WriteSystemInfo @ 0x00405620 (IDA: CErrorReport__WriteSystemInfo).
// Logs OS name, CPU name, RAM (MB), DirectX version to error report.
// si points to 264-byte SystemInfo struct: si[0..127]=CPU, si[128..255]=OS, si[256..259]=RAMbytes, si[260..]=DirectX.
void __fastcall CErrorReport_WriteSystemInfo(void* This_v, void* /*edx*/, void* si_v) {
    DWORD This = (DWORD)(uintptr_t)This_v;
    DWORD si   = (DWORD)(uintptr_t)si_v;
    CErrorReport__Write(This, (char*)"<System information>\r\n");
    CErrorReport__Write(This, (char*)"OS \t\t\t: %s\r\n", (const char*)(si + 128));
    CErrorReport__Write(This, (char*)"CPU \t\t\t: %s\r\n", (const char*)si);
    CErrorReport__Write(This, (char*)"RAM \t\t\t: %dMB\r\n", *(DWORD*)(si + 256) / 1024 / 1024 + 1);
    CErrorReport__Write(This, (char*)"\r\n");  // Separator
    CErrorReport__Write(This, (char*)"Direct-X \t\t: %s\r\n", (const char*)(si + 260));
}

// CErrorReport::WriteOpenGLInfo @ 0x004056B0 (IDA: FUN_004056B0).
// Logs GL vendor/renderer/version + max texture size + max viewport.
void __fastcall CErrorReport_WriteOpenGLInfo(void* This_v) {
    DWORD This = (DWORD)(uintptr_t)This_v;
    GLint maxTex = 0;
    GLint maxView[2] = {0, 0};
    CErrorReport__Write(This, (char*)"<OpenGL information>\r\n");
    const char* s;
    s = (const char*)glGetString(0x1F00);  // GL_VENDOR
    CErrorReport__Write(This, (char*)"Vendor \t\t\t: %s\r\n", s ? s : "(null)");
    s = (const char*)glGetString(0x1F01);  // GL_RENDERER
    CErrorReport__Write(This, (char*)"Render \t\t\t: %s\r\n", s ? s : "(null)");
    s = (const char*)glGetString(0x1F02);  // GL_VERSION
    CErrorReport__Write(This, (char*)"Version \t\t: %s\r\n", s ? s : "(null)");
    glGetIntegerv(0x0D33, &maxTex);          // GL_MAX_TEXTURE_SIZE
    CErrorReport__Write(This, (char*)"Max texture size \t: %d\r\n", maxTex);
    glGetIntegerv(0x0D3A, maxView);          // GL_MAX_VIEWPORT_DIMS
    CErrorReport__Write(This, (char*)"Max viewport \t\t: %d x %d\r\n", maxView[0], maxView[1]);
}

// CErrorReport::WriteImeInfo @ 0x00405760 (IDA: CErrorReport__WriteImeInfo).
// Logs IME description, IME file, keyboard layout name.
void __fastcall CErrorReport_WriteImeInfo(void* This_v, void* /*edx*/, HWND hWnd) {
    DWORD This = (DWORD)(uintptr_t)This_v;
    char lpszTemp[256];
    CErrorReport__Write(This, (char*)"<IME information>\r\n");
    HIMC hImc = ImmGetContext(hWnd);
    if (hImc) {
        HKL hKl = GetKeyboardLayout(0);
        ImmGetDescriptionA(hKl, lpszTemp, sizeof(lpszTemp));
        CErrorReport__Write(This, (char*)"IME Name \t\t: %s\r\n", lpszTemp);
        ImmGetIMEFileNameA(hKl, lpszTemp, sizeof(lpszTemp));
        CErrorReport__Write(This, (char*)"IME File Name \t\t: %s\r\n", lpszTemp);
        ImmReleaseContext(hWnd, hImc);
    }
    if (GetKeyboardLayoutNameA(lpszTemp)) {
        CErrorReport__Write(This, (char*)"Keyboard type \t\t: %s\r\n", lpszTemp);
    }
}
