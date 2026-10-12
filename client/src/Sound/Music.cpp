// Music.cpp — elección del tema de fondo.
//
// IDA: PlayMp3 y StopMp3 (0x004127F0). Las dos comparan el nombre recibido con
// el tema en curso (m_CurrentTrack) y respetan m_MusicEnabled (Config.ini
// [Sound] EnableMusic, o el registro) y Destroy (DAT_055ca018, el cliente se
// está cerrando).
//
// DESVIACION: el binario reproducía con un proceso externo, MuPlayer.exe
// (WinExec para empezar, WM_CLOSE para cortar). Acá lo hace CSound dentro del
// cliente (Sound/SoundManager.h). El tema suena una vez y no se repite mientras
// siga siendo el mismo, igual que antes.

#include "stdafx.h"
#include "Sound/SoundManager.h"

// IDA: StopMp3 (0x004127F0) — corta el tema si `Name` es el que está sonando.
void CSound::StopTrack(const char* Name, int bEnforce)
{
    if (Name == NULL) return;   // guard del port: la tabla de nombres puede venir vacía

    if ((m_MusicEnabled || bEnforce) && m_CurrentTrack[0] && strcmp(Name, m_CurrentTrack) == 0)
    {
        StopMusic();
        m_CurrentTrack[0] = 0;
    }
}

// IDA: PlayMp3 — empieza `Name`.
//   - mismo tema ya sonando  -> no hace nada
//   - otro tema sonando      -> lo corta y sale (el próximo frame arranca el nuevo)
//   - nada sonando           -> si el archivo existe, lo reproduce
void CSound::PlayTrack(const char* Name, int bEnforce)
{
    if (Name == NULL) return;   // guard del port: la tabla de nombres puede venir vacía

    if (DAT_055ca018 != 0 || (!m_MusicEnabled && !bEnforce))
        return;
    // DESVIACION: música parada desde el menú de opciones (CSound).
    if (IsMusicStoppedByUser()) {
        m_CurrentTrack[0] = 0;
        return;
    }

    if (m_CurrentTrack[0])
    {
        if (strcmp(Name, m_CurrentTrack) == 0)
            return;

        StopMusic();
        m_CurrentTrack[0] = 0;
        return;
    }

    FILE* fp = crt_fopen(Name, DAT_005580ac);
    if (fp == NULL) return;
    crt_fclose(fp);

    if (PlayMusic(Name))
        strcpy_s(m_CurrentTrack, sizeof(m_CurrentTrack), Name);
}


// IDA: PlayMp3 / StopMp3 (0x004127F0). Puentes para los callers heredados.
void __cdecl Music_PlayTrack(DWORD name, int enforce)
{
    gSound.PlayTrack((const char*)(uintptr_t)name, enforce);
}

void __cdecl Music_StopTrack(DWORD name, int enforce)
{
    gSound.StopTrack((const char*)(uintptr_t)name, enforce);
}

// FUN_00412180 @ 0x00412180 (~66 lines) — ListBox_HandleInput2: identical structure to
// FUN_00411a20 (key 7/0xC/0xD/0xE dispatch, scroll adjust, selection tracking).
// Separate vtable variant for a different list widget class.
int __fastcall FUN_00412180(int* param_1) {
    (void)param_1;
    return 0;
}

// FUN_004124d0 @ 0x004124D0 (24 bytes) — memcpy 0x4a dwords (296 bytes)
// FUN_004124d0 (IDA-activated)
void __cdecl FUN_004124d0(void *a1, const void *a2)
{
  if ( a1 )
  {
    qmemcpy(a1, a2, 0x128u);
  }
}

// FUN_004124f0 @ 0x004124F0 — GG module ~dtor
void __fastcall FUN_004124f0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00412510((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_00412510 @ 0x00412510 (~45 lines) — ListBox_Destructor_A: clears item linked list
// via FUN_00411360 loop, frees list sentinel nodes, resets counts, then delegates to
// base class destructor (FUN_00410de0 + FUN_00410d90). Sets vtable to PTR_FUN_00552668.
void __fastcall FUN_00412510(DWORD* param_1) {
    (void)param_1;
}

// FUN_004125f0 @ 0x004125F0 — GG module2 ~dtor
void __fastcall FUN_004125f0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00412610((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_00412610 @ 0x00412610 (~45 lines) — ListBox_Destructor_B: same structure as
// FUN_00412510 but sets vtable to PTR_FUN_00552760. Second list-box class variant.
void __fastcall FUN_00412610(DWORD* param_1) {
    (void)param_1;
}

// FUN_00412700 @ 0x00412700 (12 bytes) — NO es "string cleanup": es el wrapper que
// arranca GameGuard, `return PreInitNPGameMon("Mu")`.
//
// En el binario NO lo llama nadie desde codigo: su unico xref es de DATOS, desde la
// tabla de inicializadores dinamicos del CRT en 0x00558010. O sea corre ANTES de
// WinMain, via el thunk FUN_004126F0, que ademas registra el release con atexit.
//
// Nuestro build no replica esa tabla, asi que esta funcion queda sin callers y
// GameGuard nunca arranca, que es lo buscado.
void FUN_00412700(void) { FUN_0053d430((BYTE *)&g_GameGuardGameName); }

// FUN_00412710 @ 0x00412710 (12 bytes)
void FUN_00412710(void) {}

// FUN_00412780 @ 0x00412780 (10 bytes) — cleanup hash class
void FUN_00412780(void) { PacketCipher_Initialize((void *)0x055ca0a0); }

// FUN_00412790 @ 0x00412790 (12 bytes)
void FUN_00412790(void) {}

// FUN_004127c0 @ 0x004127C0 (10 bytes) — init error report
void FUN_004127c0(void) { FUN_00405240_init((void *)0x055c9bf0); }

// FUN_004127d0 @ 0x004127D0 (12 bytes)
void FUN_004127d0(void) {}
