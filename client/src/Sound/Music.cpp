// Music.cpp
// BGM (mp3) via el proceso externo MuPlayer.exe.
//
// IDA: FUN_004127F0 — StopMp3
// IDA: FUN_00412890 — PlayMp3
//
// Las dos comparan el nombre recibido contra el track en curso (MusicCurrentTrack).
// StopMp3 manda WM_CLOSE a la ventana "MuPlayer"; PlayMp3 ademas la lanza con
// WinExec si no existe.
//
// Globals:
//   MusicCurrentTrack — IDA: DAT_055C9D04, track en reproduccion (buffer)
//   m_MusicOnOff  — @ 0x055C9E3C, flag on/off (registro "MusicOnOff", default 0)
//   DAT_055ca018  — Destroy @ 0x055CA018, la app se esta cerrando (lo pone WndProc)
//
// MuPlayer.exe vive en bin/Client/ (36 KB, del patcher oficial de Webzen
// 00.95.14; usa MCIWndCreateA de MSVFW32). Sin ese exe al lado del cliente,
// PlayMp3 sale en el primer fopen y no suena nada.
//
// El flag "MusicOnOff" del registro arranca en 0 (fiel a IDA), y el toggle del
// menu de opciones que lo escribe todavia no esta portado, asi que por ahora
// hay que ponerlo a mano:
//   reg add "HKCU\SOFTWARE\Webzen\Mu\Config" /v MusicOnOff /t REG_DWORD /d 1 /f

#include "stdafx.h"

// NOTA(refactor B3): `qmemcpy` viene del decompile de Ghidra y el proyecto lo
// define suelto en 14 archivos. Conviene centralizarlo en un header comun.
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif


// IDA: FUN_004127F0
// Detiene el BGM si `name` coincide con el track en reproduccion.
// Port fiel del raw de IDA; el decompile de Ghidra traia el strcmp expandido
// a mano byte a byte, que es la misma comparacion.
void __cdecl Music_StopTrack(DWORD param_1_d, int bEnforce)
{
    const char* Name = (const char*)(uintptr_t)param_1_d;
    if (Name == NULL) return;   // guard del port: nuestra tabla de nombres puede estar vacia

    if ((m_MusicOnOff || bEnforce) && MusicCurrentTrack[0] && strcmp(Name, MusicCurrentTrack) == 0)
    {
        FUN_00405540(&DAT_055c9bf0, s_StopMp3_cmd_0055911c);
        HWND hWnd = FindWindowA(NULL, s_MuPlayer_00559110);
        if (hWnd)
        {
            SendMessageA(hWnd, WM_CLOSE, 0, 0);
            MusicCurrentTrack[0] = 0;
        }
    }
}


// IDA: FUN_00412890
// Arranca `name` lanzando MuPlayer.exe como proceso externo.
//   - mismo track ya sonando        -> no hace nada
//   - otro track sonando            -> lo corta (WM_CLOSE) y sale
//   - nada sonando / sin ventana    -> WinExec("MuPlayer.exe <name>")
// Port fiel del raw de IDA. El decompile de Ghidra perdia el 2do argumento del
// sprintf ("MuPlayer.exe %s" sin el nombre), asi que la linea de comandos salia
// con basura.
void __cdecl Music_PlayTrack(DWORD param_1_d, int bEnforce)
{
    const char* Name = (const char*)(uintptr_t)param_1_d;
    if (Name == NULL) return;   // guard del port: nuestra tabla de nombres puede estar vacia

    CHAR CmdLine[256];

    if (DAT_055ca018 != 0 || (!m_MusicOnOff && !bEnforce))
        return;

    if (MusicCurrentTrack[0])
    {
        if (strcmp(Name, MusicCurrentTrack) == 0)
            return;

        HWND hWnd = FindWindowA(NULL, s_MuPlayer_00559110);
        if (hWnd)
        {
            SendMessageA(hWnd, WM_CLOSE, 0, 0);
            MusicCurrentTrack[0] = 0;
            return;
        }
    }

    FILE* fp = FUN_0054173f(s_MuPlayer_exe_00559154, DAT_005580ac);
    if (fp == NULL) return;
    FUN_0054150f(fp);

    fp = FUN_0054173f(Name, DAT_005580ac);
    if (fp == NULL) return;
    FUN_0054150f(fp);

    if (FindWindowA(NULL, s_MuPlayer_00559110) == NULL)
    {
        FUN_00405540(&DAT_055c9bf0, s_PlayMp3_cmd_00559140);
        crt_sprintf(CmdLine, s_MuPlayer_exe__s_00559130, Name);
        WinExec(CmdLine, 0);
        strcpy_s(MusicCurrentTrack, sizeof(MusicCurrentTrack), Name);
    }
}


// ── FUN_00412180 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00412180 @ 0x00412180 (~66 lines) — ListBox_HandleInput2: identical structure to
// FUN_00411a20 (key 7/0xC/0xD/0xE dispatch, scroll adjust, selection tracking).
// Separate vtable variant for a different list widget class.
int __fastcall FUN_00412180(int* param_1) {
    (void)param_1;
    return 0;
}

// ── FUN_004124d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004124d0 @ 0x004124D0 (24 bytes) — memcpy 0x4a dwords (296 bytes)
// FUN_004124d0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_004124d0(void *a1, const void *a2)
{
  if ( a1 )
  {
    qmemcpy(a1, a2, 0x128u);
  }
}

// ── FUN_004124f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004124f0 @ 0x004124F0 — GG module ~dtor
void __fastcall FUN_004124f0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00412510((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00412510 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00412510 @ 0x00412510 (~45 lines) — ListBox_Destructor_A: clears item linked list
// via FUN_00411360 loop, frees list sentinel nodes, resets counts, then delegates to
// base class destructor (FUN_00410de0 + FUN_00410d90). Sets vtable to PTR_FUN_00552668.
void __fastcall FUN_00412510(DWORD* param_1) {
    (void)param_1;
}

// ── FUN_004125f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004125f0 @ 0x004125F0 — GG module2 ~dtor
void __fastcall FUN_004125f0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00412610((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00412610 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00412610 @ 0x00412610 (~45 lines) — ListBox_Destructor_B: same structure as
// FUN_00412510 but sets vtable to PTR_FUN_00552760. Second list-box class variant.
void __fastcall FUN_00412610(DWORD* param_1) {
    (void)param_1;
}

// ── FUN_00412700 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00412700 @ 0x00412700 (12 bytes) — GameGuard string cleanup
void FUN_00412700(void) { FUN_0053d430((BYTE *)&lpWindowName_0055910c); }

// ── FUN_00412710 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00412710 @ 0x00412710 (12 bytes)
void FUN_00412710(void) {}

// ── FUN_00412780 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00412780 @ 0x00412780 (10 bytes) — cleanup hash class
void FUN_00412780(void) { FUN_00406bd0((void *)0x055ca0a0); }

// ── FUN_00412790 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00412790 @ 0x00412790 (12 bytes)
void FUN_00412790(void) {}

// ── FUN_004127c0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004127c0 @ 0x004127C0 (10 bytes) — init error report
void FUN_004127c0(void) { FUN_00405240_init((void *)0x055c9bf0); }

// ── FUN_004127d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004127d0 @ 0x004127D0 (12 bytes)
void FUN_004127d0(void) {}
