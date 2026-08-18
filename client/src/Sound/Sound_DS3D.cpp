// Sound_DS3D.cpp
// DirectSound8 playback + per-frame 3D positional update.
//
// PlayBuffer        @ 0x00404BC0 — start playback on a loaded slot (also FUN_00404bc0)
// Sound_UpdatePos   @ 0x00404CD0 — per-frame 3D listener-relative SetPosition
// SetHall           @ 0x00404BB0 — stub in original (returns 1)
//
// State lives in globals.cpp. See src/Sound/Sound.cpp for the loader side.

#include "stdafx.h"
#include <dsound.h>



// ============================================================================
// SetHall @ 0x00404BB0 — empty stub in the shipped binary (returns 1).
// Originally intended to apply a reverb/environment preset via IKsPropertySet
// (EAX), but the final release left this as a no-op.
// ============================================================================
static int SetHall(int /*Buffer*/) { return 1; }


// ============================================================================
// PlayBuffer / FUN_00404bc0  @ 0x00404BC0
// ============================================================================
// Plays the secondary buffer for slot [Buffer][BufferChannel[Buffer]].
//   Buffer  — sound ID (the same index passed to LoadWaveFile).
//   Object  — entity pointer (3D positional anchor), stored for per-frame update.
//   bLooped — non-zero → DSBPLAY_LOOPING.
//
// Returns S_OK (0) on success, or the HRESULT from Play on failure. Matches
// IDA 00404BC0_PlayBuffer.c semantics (including the channel-wrap reset).
// ============================================================================
HRESULT __cdecl PlayBuffer(int Buffer, DWORD Object, BOOL bLooped)
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

// Alias retained — other code calls FUN_00404bc0 directly via functions.h.
HRESULT __cdecl FUN_00404bc0(int Buffer, DWORD Object, BOOL bLooped)
{
    return PlayBuffer(Buffer, Object, bLooped);
}


// ============================================================================
// Sound_UpdatePositions / FUN_00404CD0  @ 0x00404CD0
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
// FUN_004f9db0 (EulerToMatrix3x4) and FUN_004fa0b0 (Vec3_TransformByMatrix)
// are declared in functions.h with the canonical signatures.

void FUN_00404cd0(void)
{
    if (!g_EnableSound || !g_Enable3DSound) return;

    float eulerAngles[3];
    float rotMatrix[12];
    eulerAngles[0] = 0.0f;
    eulerAngles[1] = 0.0f;
    eulerAngles[2] = Ff(DAT_083a42c0);      // player yaw (CameraAngle[2] bits)
    FUN_004f9db0(eulerAngles, rotMatrix);

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
            FUN_004fa0b0(offset, rotMatrix, offset);
            offset[0] *= unitScale;
            offset[1] *= unitScale;
            offset[2] *= unitScale;

            LPDIRECTSOUND3DBUFFER p3D = g_lpDS3DBuffer[slot][ch];
            if (p3D) {
                // Note: Y is force to 0 and X/Z are negated (IDA decompile),
                // matching the original's coordinate-system convention.
                p3D->SetPosition(-offset[0], 0.0f, -offset[2], DS3D_DEFERRED);
            }
        }
    }
}

// ── FUN_00404e60_impl — helper local movido desde stubs_bulk_small.cpp (refactor B3) ──
static void __cdecl FUN_00404e60_impl(int param_1) {
    *(int *)param_1 = (int)&PTR_FUN_005524c0;
    if (*(HANDLE *)(param_1 + 4) != NULL) {
        mmioClose((HMMIO)*(HANDLE *)(param_1 + 4), 0);
    }
}

// ── FUN_00404c60 — movida desde stubs_render_helpers.cpp (refactor B3) ──
// FUN_00404c60 / StopBuffer @ 0x00404C60 — stop a slot's currently-playing channel.
// For non-zero Buffer, also rewind to position 0. Uses g_lpDSBuffer[Buffer][0]
// (only channel 0 is populated in this binary's pipeline — FillBuffer is a no-op).
void __cdecl FUN_00404c60(int Buffer) {
    if (!g_EnableSound || Buffer < 0) return;
    LPDIRECTSOUNDBUFFER pBuf = g_lpDSBuffer[Buffer][0];
    if (pBuf) {
        pBuf->Stop();
        if (Buffer != 0) pBuf->SetCurrentPosition(0);
    }
}

// ── FUN_00404e40 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00404e40 @ 0x00404E40 — CWaveFile ~dtor (calls FUN_00404e60)
void __fastcall FUN_00404e40(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00404e60_impl(ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00405260 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00405260 @ 0x00405260 — CErrorReport ~dtor
void __fastcall FUN_00405260(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00405280((HANDLE *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00405280 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 11-byte: vtable + chain ─────────────────────────────────────────────────

// FUN_00405280 @ 0x00405280 (11 bytes)
// FUN_00405280 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00405280(HANDLE *_this)
{
  *_this = (HANDLE)DAT_005524c4;
  CloseHandle(_this[1]);
  return FUN_00405290((int)_this);
}

// ── FUN_00405340 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// IDA signature is __thiscall(this); function.h decl is __stdcall(void).
// No live callers in our build (vtable dispatch via CErrorReport disabled),
// so this is a NOP placeholder. The truncation logic ported below as
// CErrorReport_RotateLog_impl(this) for when the vtable is wired up.
void __stdcall FUN_00405340(void) {
    // No-op — CErrorReport vtable not yet active.
}

// ── FUN_00405500 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 52-byte ─────────────────────────────────────────────────────────────────

// FUN_00405500 @ 0x00405500 — CErrorReport::WriteDebugInfoStr (52 bytes)
// (declared in Ghidra as CErrorReport::WriteDebugInfoStr — writes debug string)
// This is a thin wrapper that calls CErrorReport__Write; implementation is in the
// vtable dispatch. We stub it as a pass-through.
void __cdecl FUN_00405500(DWORD This, char *fmt) {
    CErrorReport__Write(This, fmt);
}

// ── FUN_00405540 — movida desde stubs_render_helpers.cpp (refactor B3) ──
void __cdecl FUN_00405540(void*,const char*,...)            {} // debug log — kept as stub

// ── FUN_00405590 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00405590 @ 0x00405590 (15 bytes) — log begin marker
void __fastcall FUN_00405590(DWORD This) {
    CErrorReport__Write(This, (char *)"========Log Begin========");
}

// ── FUN_004055a0 — movida desde stubs_render_helpers.cpp (refactor B3) ──
// FUN_004055a0 @ 0x004055A0 — Log_Timestamp(verbose).
// Logs current local date/time via FUN_00405540 (debug log sink at DAT_055C9BF0).
// If param_1 != 0, logs an additional data block from DAT_00558128.
void FUN_004055a0(int param_1) {
    _SYSTEMTIME local_10;
    GetLocalTime(&local_10);
    FUN_00405540(&DAT_055c9bf0, "%4d %02d %02d %02d %02d"); // date+time format
    if (param_1 != 0) {
        FUN_00405540(&DAT_055c9bf0, DAT_00558128);
    }
}

// ── FUN_00405620 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00405620 @ 0x00405620 — CErrorReport::WriteSystemInfo (137 bytes IDA, port FIEL).
// Logs OS name, CPU name, RAM (MB), DirectX version to error report.
// si points to 264-byte SystemInfo struct: si[0..127]=CPU, si[128..255]=OS, si[256..259]=RAMbytes, si[260..]=DirectX.
void __fastcall FUN_00405620(void* This_v, void* /*edx*/, void* si_v) {
    DWORD This = (DWORD)(uintptr_t)This_v;
    DWORD si   = (DWORD)(uintptr_t)si_v;
    CErrorReport__Write(This, (char*)"<System information>\r\n");
    CErrorReport__Write(This, (char*)"OS \t\t\t: %s\r\n", (const char*)(si + 128));
    CErrorReport__Write(This, (char*)"CPU \t\t\t: %s\r\n", (const char*)si);
    CErrorReport__Write(This, (char*)"RAM \t\t\t: %dMB\r\n", *(DWORD*)(si + 256) / 1024 / 1024 + 1);
    CErrorReport__Write(This, (char*)"\r\n");  // Separator
    CErrorReport__Write(This, (char*)"Direct-X \t\t: %s\r\n", (const char*)(si + 260));
}

// ── FUN_004056b0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_004056b0 @ 0x004056B0 — CErrorReport::WriteOpenGLInfo (173 bytes IDA, port FIEL).
// Logs GL vendor/renderer/version + max texture size + max viewport.
void __fastcall FUN_004056b0(void* This_v) {
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

// ── FUN_00405760 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00405760 @ 0x00405760 — CErrorReport::WriteImeInfo (175 bytes IDA, port FIEL).
// Logs IME description, IME file, keyboard layout name.
void __fastcall FUN_00405760(void* This_v, void* /*edx*/, HWND hWnd) {
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
