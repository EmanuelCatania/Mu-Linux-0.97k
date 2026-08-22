// stubs_bulk_misc.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 14828-16258 (1431 lines).
//
// BATCH 10-15: misc bulk decompiles.
//   BATCH 10 — Sound/BST/Misc + Quest UI, ErrorReport, SystemInfo, HashTable, GameGuard packet handler
//   BATCH 11 — BMD/SMD model loading + mesh rendering + linked list + quest init
//   BATCH 13 — GameGuard / Crypto / IAT thunks (0x0053CB86–0x005406F0)
//   BATCH 14 — SMD model build, UI widget system, text rendering, chat path parsing
//   BATCH 15 — Font/Text rendering, BST (red-black tree) cache, KillGLWindow

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);
extern void __cdecl FUN_0054158c(void* ptr);
void __fastcall FUN_00407b30(void *node, float *out);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 10 — Sound/BST/Misc range (0x00402xxx-0x00412xxx)
// Quest UI, ErrorReport, SystemInfo, HashTable, GameGuard packet handler
// ═══════════════════════════════════════════════════════════════════════════════

// FUN_00402ff0 @ 0x00402FF0 (~59 lines) — Quest dialog: render message box text + answer choices
// __fastcall(ecx=questObj). Renders g_lpszMessageBoxCustom lines centered, then answer list
// with mouse hover highlight (red if hovered). Uses g_iNumAnswer, g_iNumLineMessageBoxCustom.
void __fastcall FUN_00402ff0(int param_1) {
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    EnableAlphaTest(true);
    int iPos_y = (((7 - g_iNumAnswer) - g_iNumLineMessageBoxCustom) * 0x12) / 2 + 0x42;
    if (0 < g_iNumLineMessageBoxCustom) {
        for (int i = 0; i < g_iNumLineMessageBoxCustom; i++) {
            RenderCenterText(0x226, iPos_y, g_lpszMessageBoxCustom[i]);
            iPos_y += 0x12;
        }
    }
    if (*(char*)(param_1 + 0x1c882) != '\x01' && *(char*)(param_1 + 0x1c87f) == '\x01') {
        iPos_y = 0xfa;
    }
    int hovered = ((int)MouseY - iPos_y) / 18;
    SelectObject(m_hFontDC, g_hFontBold);
    for (int i = 0; i < g_iNumAnswer; ++i) {
        int dx = 556 - (int)MouseX;
        if (dx < 0) dx = -dx;
        if (hovered == i && dx <= 106) {
            m_dwTextColor = 0xff0000ff;
        } else {
            m_dwTextColor = 0xff67bfdf;
        }
        char* line = &g_lpszDialogAnswer[i][0][0];
        if (*line) {
            RenderCenterText(550, iPos_y, line);
        }
        iPos_y += 18;
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

extern "C" void __cdecl RenderTipText(int sx, int sy, const char* Text);
int __cdecl FUN_004c3dd0(int param_1);

// sub_403150 @ 0x00403150 (438 bytes) — lista de items que pide la quest.
//
// Devuelve 1 si el personaje TIENE todos los items requeridos, 0 si falta
// alguno.  sub_403320 usa ese retorno para pintar el boton de "Proceder" en
// claro o en gris; sub_4BFDE0 la llama con a3 = 0 para dibujar los modelos 3D
// de los items en el panel.
//
//   a2 = estado esperado (se compara contra This + 0x1C882)
//   a3 = 0 -> dibuja los items en 3D;  != 0 -> dibuja "Nombre x N" como texto
//
// 2026-08-22: portada desde el DISASSEMBLY, no desde el decompile.  Hex-Rays
// emite "positive sp value has been detected, the output may be wrong" para
// esta funcion y pierde los parametros (lee Buffer[92] y v18 sin inicializar),
// asi que el decompile no sirve.  El disassembly, en cambio, sale limpio.
//
// Layout de la entrada de quest (18 bytes, arranca en pQuest + 40 + 18*i):
//   +0  categoria del item      -> nType = categoria * 32 + indice
//   +1  indice del item
//   +2  cantidad pedida
//   +4  flags de disponibilidad por clase (se indexa con This[4])
//   -1  (= pQuest + 39 + 18*i) 1 = la entrada pide un item
char __fastcall FUN_00403150(void *pThis, int /*edx*/, char a2, char a3)
{
    const int This = (int)(uintptr_t)pThis;

    if (*(BYTE *)(This + 0x1c87f) == 0) return 0;      // el panel no esta abierto
    if (*(char *)(This + 0x1c882) != a2) return 0;     // el estado no es el pedido

    float sy  = 235.0f;
    char  ret = 1;
    if (a3 != 0) {
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        sy = 240.0f;
        m_dwBackColor = 0;
    }

    const int    questIdx = *(unsigned char *)(This + 0x1c87a);
    const BYTE  *pQuest   = (const BYTE *)(This + 584 * questIdx + 8);
    const int    nEntry   = *(const short *)pQuest;
    const int    klass    = *(unsigned char *)(This + 4);

    char buf[100];
    for (int i = 0; i < nEntry; ++i) {
        const BYTE *e = pQuest + 40 + 18 * i;
        if (*(const BYTE *)(e + 4 + klass) != 1) continue;   // no aplica a esta clase
        if (*(const BYTE *)(e - 1) != 1)         continue;   // la entrada no pide item

        const int nType  = (int)e[1] + 32 * (int)e[0];
        const int nCount = (int)e[2];

        if (a3 != 0) {
            // FindQuestItemsInInven devuelve 0 si ya los tiene, o cuantos
            // faltan.  De ahi que "0" pinte en celeste y "!= 0" en rojo.
            int missing = FUN_00482dd0(nType, nCount, 0xFFFFFFFFu);
            if (missing == 0) {
                m_dwTextColor = 0xFF67BFDFu;
            } else {
                m_dwTextColor = 0xFF1E1EFFu;
                ret = 0;
            }
            const char *name = "";
            unsigned int abase = (unsigned int)(uintptr_t)DAT_07d78068;
            if (abase >= 0x100000u && abase < 0x80000000u && nType >= 0 && nType < 1024)
                name = (const char *)(uintptr_t)(abase + (unsigned int)nType * 64u);
            crt_sprintf(buf, "%s x %d", name, nCount);
            RenderText(510, (int)sy, buf, 0, 0, nullptr);
        } else {
            // Desviacion conocida del binario: el DLL de inyeccion parchea el
            // byte de 0x004032A8 ("Fix Quest Item Preview") para que este
            // Level sea 0 en vez de -1, porque con -1 la vista previa del item
            // sale mal.  Se deja fiel a IDA.
            RenderItem3D(480.0f, sy, 20.0f, 20.0f, nType, -1, 0, 0, false);
        }
        sy += _DAT_00552464;
    }
    return ret;
}

// FUN_00403320 @ 0x00403320 (955 bytes) — ventana de quest del NPC
// Port fiel del decompile.  2026-08-21: acá había un resumen que sólo dibujaba
// el fondo y dejaba el resto como comentarios ("stub: full render logic
// omitted"); ni siquiera llamaba a FUN_00402ff0, que es la que dibuja el texto
// del diálogo y las respuestas.  Con el flag del panel prendido, GetScreenWidth
// angostaba el viewport a 450 y esa franja quedaba en negro.
//
// Desviación: la llamada a sub_403150 (que dibuja la lista de items pedidos por
// la quest y devuelve si están todos en el inventario) queda pendiente — su
// decompile sale con "positive sp value has been detected, the output may be
// wrong" y Hex-Rays perdió los parámetros.  Acá se asume "cumple" para el color
// del botón; lo único que cambia es que el botón sale habilitado y la lista de
// items no se dibuja.
void __fastcall FUN_00403320(void* param_1) {
    char Buffer[100];
    int  This = (int)(uintptr_t)param_1;

    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    RenderInventoryInterface_stub(450, 0, 1);
    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;

    char state = *(char*)(This + 116866);      // +0x1C882
    if (state == 1) {
        FUN_005125a0(279, 450.0f, 325.0f, 190.0f, 10.0f,
                     0.0f, 0.0f, 0.7421875f, 0.625f, 1, 1);
        if (FUN_00403150(param_1, 0, state, 1)) {
            m_dwTextColor = 0xFFD2E6FFu;
        } else {
            glColor3f(0.3f, 0.3f, 0.3f);
        }

        if ((double)MouseX >= 485.0 && (double)MouseX < 605.0 &&
            (double)MouseY >= 355.0 && (double)MouseY < 379.0 && MouseLButtonPush) {
            glColor3f(0.4f, 0.4f, 0.4f);
            if (MouseLButtonPop) {
                MouseLButtonPush = 0;
                MouseLButton = 0;
            }
        }
        SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
        FUN_005125a0(240, 485.0f, 355.0f, 120.0f, 24.0f,
                     0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        RenderCenterText(545, 360, GlobalText[699]);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else if (state == 3) {
        FUN_005125a0(271, 500.0f, 367.7f, 113.0f, 18.0f,
                     0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);
        m_dwBackColor = 0xFF282828u;
        m_dwTextColor = 0xFF96DCFFu;
        RenderText(470, 370, GlobalText[198], 0, 0, nullptr);
        int zen = *(int*)(This + 116868);       // +0x1C884
        m_dwTextColor = (DWORD)FUN_004c3dd0(zen);
        ConvertGold64_stub(zen, Buffer);
        RenderText(510, 370, Buffer, 0, 0, nullptr);
    }

    // Botón de cerrar + su tooltip
    FUN_005125a0(280, 475.0f, 395.0f, 24.0f, 24.0f,
                 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if ((double)MouseX >= 475.0 && (double)MouseX < 499.0 &&
        (double)MouseY >= 395.0 && (double)MouseY < 419.0) {
        SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText(475, 382, GlobalText[225]);
    }

    m_dwBackColor = 0;
    SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
    m_dwTextColor = 0xFFF0FF96u;

    int questIdx  = *(unsigned char*)(This + 116858);   // +0x1C87A
    int questBase = This + 584 * questIdx;
    int nameW = 120 * (int)WindowWidth / 0x280;
    // getMonsterName devuelve NULL mientras MonsterScript no esté parseada
    // (ver la nota de CLAUDE.md); el original no lo contempla.
    char* npcName = getMonsterName(*(unsigned char*)(questBase + 12));
    if (npcName) RenderText(485, 12, npcName, nameW, 1, (void*)3);

    m_dwTextColor = 0xFFFFDCC8u;
    RenderText(472, 22, (char*)(questBase + 13),
               150 * (int)WindowWidth / 0x280, 1, nullptr);

    FUN_00402ff0(This);      // texto del diálogo + respuestas
    glColor3f(1.0f, 1.0f, 1.0f);
}

// FUN_00403f30 @ 0x00403F30 (30 bytes) — dispatcher del render de quest.
// 2026-08-21: no existía y nadie lo llamaba.  sub_4F5820 (Render_QuickButtons_)
// lo invoca entre RenderGoldenArcherWindow y RenderServerDivision; sin eso el
// panel de quest nunca se dibujaba aunque su flag estuviera prendido.
void __fastcall FUN_00403a40(void* param_1);
char __fastcall FUN_00403f30(void* param_1) {
    if (param_1 == nullptr) return 1;
    unsigned char mode = *(unsigned char*)((int)(uintptr_t)param_1 + 116863);  // +0x1C87F
    if (mode == 1)      FUN_00403320(param_1);
    else if (mode == 2) FUN_00403a40(param_1);
    return 1;
}

// FUN_00403700 @ 0x00403700 (~116 lines) — Quest list: render quest entries filtered by state
// __thiscall(this=questObj, param_1=filterState). Iterates up to 200 quests via CSQuest__getQuestState,
// shows matching quests in a scrollable list. Mouse click selects quest + calls CSQuest__CheckQuestState
// and CSQuest__ShowDialogText. Tab buttons "Progress"/"Complete" at top. Selected quest shows arrow icon.
UINT __fastcall FUN_00403700(void* ecx, void* /*edx*/, UINT param_1) {
    (void)ecx; (void)param_1;
    // Tab buttons rendered at offset _DAT_005524a4, hover highlight
    // Quest loop: for questIndex 0..199:
    //   if getQuestState(questIndex) == param_1 and name != empty:
    //     render quest name, highlight on hover, click selects
    //     selected quest gets arrow bitmap (id=9)
    // Returns 0 (via pSVar9)
    return 0;
}

// FUN_00403a40 @ 0x00403A40 (~88 lines) — Quest UI main panel: tabs + quest list + close button
// __fastcall(ecx=questObj). Renders 3-tab quest panel (tab 0=progress, 1=complete, 2=special).
// Calls FUN_00403700 for tab 0/1 content, FUN_00403a30 for tab 2.
// Renders NPC name, close button with tooltip, dialog answers via FUN_00402ff0.
void __fastcall FUN_00403a40(void* param_1) {
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    RenderInventoryInterface_stub(0x1c2, 0, 1);
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    // SelectObject(m_hFontDC, g_hFont); // globals not yet declared
    // Render 3 tab buttons (stride 0x37, bitmaps 0x115/0x116 for normal/selected)
    // Tab click sets *(param_1+0x1c87d)
    // Render separator lines (bitmap 0x117) and border lines (bitmap 0x104)
    // Dispatch to FUN_00403700(1 or 2) or FUN_00403a30() based on selected tab
    // Close button, NPC name, dialog answers
    char cVar1 = *(char*)((int)param_1 + 0x1c87d);
    if (cVar1 == '\0') {
        FUN_00403700(param_1, NULL, 1);
    } else if (cVar1 == '\x01') {
        FUN_00403700(param_1, NULL, 2);
    } else if (cVar1 == '\x02') {
        FUN_00403a30();
    }
    // 2026-08-21: faltaba el render del texto del dialogo + respuestas.
    FUN_00402ff0((int)(uintptr_t)param_1);
    glColor3f(1.0f, 1.0f, 1.0f);
}

// FUN_004041e0 @ 0x004041E0 (~74 lines) — HashTable::GetIndex — find slot for 4-byte key
// __thiscall(this=hashTableObj, param_1=key). Linear probe hash table lookup.
// Returns slot index or 0xFFFFFFFF if not found / table full.
// Vtable call at this+0xc for hash function, entries at this+8.
UINT __fastcall FUN_004041e0(void* ecx, void* /*edx*/, DWORD param_1) {
    (void)ecx; (void)param_1;
    // hash = vtable[3](param_1) — compute initial slot
    // Linear probe: compare 4-byte key at (this+8)[slot*4]
    //   if slot == {0,0,0,0}: return -1 (empty)
    //   if slot matches param_1: return slot
    //   advance slot = (slot+1) % capacity
    // If full: CErrorReport__Write("Hash table full — GetIndex"), return -1
    return 0xFFFFFFFF;
}

// InitDirectSound @ 0x004044A0 — real implementation in src/Sound/Sound.cpp.

// FUN_00405340 @ 0x00405340 — CErrorReport_RotateLog (215 bytes IDA, port FIEL).
// __thiscall(this). Reads error log file (this+4 = HANDLE), decrypts with
// Xor_ConvertBuffer (this+0x10c = key state), keeps last 4 entries, rewrites file.
//
// Layout (per IDA):
//   this+4   HANDLE (file)
//   this+8   LPCSTR file path
//   this+0x10c  XOR key state DWORD
// Xor_ConvertBuffer is declared in functions.h
char* __stdcall FUN_00405420(char* param_1);
extern "C" void CErrorReport_RotateLog_impl(DWORD This) {
    static char Buffer[0x1FFFF + 1];
    DWORD numRead = 0;
    HANDLE hFile = *(HANDLE*)(This + 4);
    if (hFile == INVALID_HANDLE_VALUE || hFile == nullptr) return;
    if (!ReadFile(hFile, Buffer, 0x1FFFF, &numRead, nullptr)) return;
    *(DWORD*)(This + 0x10C) = (DWORD)Xor_ConvertBuffer(Buffer, numRead, 0);
    Buffer[numRead] = '\0';
    char* trimAt = FUN_00405420(Buffer);
    if (numRead < 0x7FFF && trimAt == Buffer) return;
    char* keepFrom = (trimAt == Buffer) ? Buffer : trimAt;
    DWORD keepLen = numRead - (DWORD)(keepFrom - Buffer);
    CloseHandle(hFile);
    LPCSTR path = (LPCSTR)(This + 8);
    DeleteFileA(path);
    HANDLE newH = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                              nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    *(HANDLE*)(This + 4) = newH;
    *(DWORD*)(This + 0x10C) = 0;
    if (newH != INVALID_HANDLE_VALUE) {
        DWORD wr = 0;
        WriteFile(newH, keepFrom, keepLen, &wr, nullptr);
    }
}

// FUN_00405420 @ 0x00405420 — CErrorReport_FindLogMarker (143 bytes IDA, port FIEL).
// Scans buffer for "###Log Begin###" markers, returns pointer to 4th-from-last marker
// (so newer log can keep the last 4 entries), or original buffer if < 5 markers found.
// Note: IDA shows __stdcall(char*, int) but a2 is a phantom param; only the buffer is used.
char* __stdcall FUN_00405420(char* param_1) {
    static const char kLogMarker[] = "###Log Begin###";
    if (!param_1) return param_1;
    char* markers[128];
    int count = 0;
    char* p = param_1;
    while ((p = strchr(p, '#')) != nullptr) {
        if (strncmp(p, kLogMarker, sizeof(kLogMarker) - 1) == 0) {
            if (count < 128) markers[count++] = p;
            p += sizeof(kLogMarker) - 1;
        } else {
            ++p;
        }
    }
    if (count > 4) return markers[count - 4];
    return param_1;
}




// GetOSVersion @ 0x00405A20 (~461 lines) — Detect Windows version string
// Calls GetVersionExA, then giant switch on major/minor version to build OS name string.
// Detects Win95/98/ME/NT3.51/NT4/2000/XP/Server2003 + service pack info via registry.
// Writes result to si+0x80 (128-byte OS name buffer).
void __cdecl GetOSVersion(DWORD si) {
    // OSVERSIONINFOA ovi; ovi.dwOSVersionInfoSize = 0x94; GetVersionExA(&ovi);
    // Switch on ovi.dwMajorVersion (3,4,5):
    //   3.51 -> "Windows NT 3.51"
    //   4.0+platformId1 -> "Windows 95" (+ "OSR2" if CSD='C'|'B')
    //   4.10 -> "Windows 98" (+ "SE" if CSD='A')
    //   4.90 -> "Windows Me"
    //   4.0+platformId2 -> "Windows NT 4.0"
    //   5.0 -> "Windows 2000" + registry ProductType (Server/Professional)
    //   5.1 -> "Windows XP" + registry ProductType
    //   5.2 -> "Windows Server 2003"
    // Appends service pack string from ovi.szCSDVersion
    // stub: 461 lines of string copy + version detection
}

// FUN_00405e20 @ 0x00405E20 (284 bytes IDA) — Measure CPU speed via RDTSC + QueryPerformanceCounter
// Returns the number of TSC ticks per second of QPC time (= CPU MHz × 1e6 if TSC = clock).
// Used by GetCPUInfo to format the "%d MHz" suffix on the CPU name string.
//
// Port faithful from IDA sub_405E20: cpuid(1) check TSC bit, QPF, boost priority to
// realtime, pin to CPU 0, measure RDTSC across Sleep(ms), restore state, compute ratio.
long long __cdecl FUN_00405e20(DWORD param_1) {
    // CPUID(1) — check TSC support (EDX bit 4)
    int regs[4] = {0,0,0,0};
    __cpuid(regs, 1);
    if ((regs[3] & 0x10) == 0) return 0;  // no TSC

    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) return 0;

    HANDLE hProc = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    DWORD priClass = GetPriorityClass(hProc);
    int   threadPri = GetThreadPriority(hThread);
    DWORD_PTR procAff = 0, sysAff = 0;
    GetProcessAffinityMask(hProc, &procAff, &sysAff);

    SetPriorityClass(hProc, REALTIME_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
    SetProcessAffinityMask(hProc, 1);

    // cpuid as serializing barrier before RDTSC
    __cpuid(regs, 0);
    LARGE_INTEGER qpcStart, qpcEnd;
    QueryPerformanceCounter(&qpcStart);
    unsigned __int64 tscStart = __rdtsc();
    Sleep(param_1);
    QueryPerformanceCounter(&qpcEnd);
    unsigned __int64 tscEnd = __rdtsc();

    SetProcessAffinityMask(hProc, procAff);
    SetThreadPriority(hThread, threadPri);
    SetPriorityClass(hProc, priClass);

    double tscDelta = (double)(__int64)(tscEnd - tscStart);
    double qpcDelta = (double)(qpcEnd.QuadPart - qpcStart.QuadPart);
    double seconds  = qpcDelta / (double)freq.QuadPart;
    if (seconds <= 0.0) return 0;
    return (long long)(tscDelta / seconds);
}

// GetCPUInfo @ 0x00405F40 (~336 lines) — Detect CPU vendor + model string
// cpuid(0) -> vendor string (GenuineIntel / AuthenticAMD / etc)
// cpuid(1) -> family/model, then giant switch to map to human-readable name
// (Pentium Pro/2/3/4, AMD K5/K6/K7/Athlon/Duron, Cyrix, etc)
// Writes result to si (128-byte CPU name buffer) + appends " %dMHz" via FUN_00405e20.
void __cdecl GetCPUInfo(DWORD si) {
    (void)si;
    // cpuid(0) -> vendor ID at si[0..11]
    // cpuid(1) -> family = bits 8-11, model = bits 4-7
    // Switch on vendor:
    //   "AuthenticAMD": family 4->486, family 5->K5/K6, family 6->K7/Athlon/Duron
    //   "GenuineIntel": family 3/4/5->486/Pentium, family 6->PentiumPro/2/3, family 15->Pentium4
    //   "CyrixInstead": Cyrix models
    // Append " %dMHz" from FUN_00405e20(500)
    // stub: 336 lines of CPU identification
}

// GetDXVersion @ 0x00406270 (~154 lines) — Detect DirectX version installed
// Probes DDRAW.DLL, DINPUT.DLL, D3D8.DLL, D3D9.DLL via LoadLibrary/GetProcAddress.
// Tests DirectDrawCreate, DirectDrawCreateEx, CoCreateInstance for DirectMusic.
// Returns version code: 0=none, 0x100=DDraw1, 0x200=DInput, 0x300=DDraw2, 0x500=DDraw4,
// 0x600=DMusic, 0x601=DDraw7, 0x700=D3D8, 0x800=D3D8only, 0x900=D3D9.
DWORD __stdcall GetDXVersion_stub(void) {
    // LoadLibrary("DDRAW.DLL") -> GetProcAddress("DirectDrawCreate")
    // Create DDraw1, QI DDraw2 -> 0x100
    // LoadLibrary("DINPUT.DLL") -> GetProcAddress("DirectInputCreateA") -> 0x200
    // DDraw2->SetCooperativeLevel + CreateSurface -> 0x300
    // QI DDraw4 -> 0x500, QI DDraw7 -> 0x601
    // CoCreateInstance(CLSID_DirectMusicPerformance) -> 0x600
    // GetProcAddress("DirectDrawCreateEx") -> 0x601
    // LoadLibrary("D3D8.DLL") -> 0x700
    // LoadLibrary("D3D9.DLL") -> 0x900
    return 0x900;
}

// FUN_00406660 @ 0x00406660 — CErrorReport_DumpCallStack (port FIEL).
// __thiscall(this). Dumps CPU registers (CONTEXT_FULL) and walks EBP chain to
// log return addresses (up to 1024 frames) on crash. The function.h decl is
// __stdcall(void); IDA shows it's __thiscall(this) — we accept the void
// declaration since the call sites (vtable dispatch via CErrorReport) are not
// active in our build.
void __stdcall FUN_00406660(void) {
    // No-op: CErrorReport vtable not wired up in our build. The implementation
    // logic is preserved as CErrorReport_DumpCallStack_impl below for future use.
}
extern "C" void CErrorReport_DumpCallStack_impl(DWORD This) {
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(GetCurrentThread(), &ctx)) return;

    CErrorReport__Write(This, (char*)"<Register>\r\n");
    CErrorReport__Write(This, (char*)"EAX = 0x%08X  EBX = 0x%08X  ECX = 0x%08X  EDX = 0x%08X\r\n",
                        ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx);
    CErrorReport__Write(This, (char*)"ESI = 0x%08X  EDI = 0x%08X  EBP = 0x%08X  EIP = 0x%08X\r\n",
                        ctx.Esi, ctx.Edi, ctx.Ebp, ctx.Eip);
    CErrorReport__Write(This, (char*)"<Call Stack>\r\n");

    DWORD* ebp = (DWORD*)(uintptr_t)ctx.Ebp;
    int count = 0;
    while (ebp && !IsBadReadPtr(ebp, sizeof(DWORD) * 2) && *ebp != 0 && count < 1024) {
        DWORD ret = ebp[1];  // return address sits at [ebp+4]
        CErrorReport__Write(This, (char*)"0x%08X\r\n", ret);
        ebp = (DWORD*)(uintptr_t)*ebp;
        ++count;
    }
}

// FUN_00406bd0 @ 0x00406BD0 (~54 lines) — Packet cipher init (constructor)
// __fastcall(ecx=cipherObj). Sets vtable, calls FUN_00406d20 (key schedule),
// allocates random-sized obfuscation buffer at obj+6, allocates encode/decode buffers
// (0x400 entries * 4 bytes each), zeroes them, sets obj+8/+9 as read/write cursors.
void __fastcall FUN_00406bd0(void* param_1) {
    // *(int*)param_1 = &PTR_FUN_005524c8 (vtable)
    // FUN_00406d20(param_1) — key schedule init
    // delete old buffer at param_1[6], alloc rand()%0xCC7+0x159 obfuscation bytes
    // param_1[3] = 0x400 (capacity)
    // param_1[1] = new byte[0x1000] (encode buffer)
    // param_1[2] = new byte[0x1000] (decode buffer)
    // Zero both buffers, set param_1[8]=param_1[1], param_1[9]=param_1[2]
    (void)param_1;
}

// FUN_00406e90 @ 0x00406E90 (~20 lines) — Hash function A: hash 4-byte key with multiplier 0x83
// __fastcall(ecx=hashObj). Hashes 4 stack bytes: h = h*0x83 + byte[i], returns h % capacity.
UINT __fastcall FUN_00406e90(int param_1) {
    (void)param_1;
    // uint h = 0;
    // for (i = 0; i < 4; i++) h = h * 0x83 + key_bytes[i];
    // return h % *(uint*)(param_1 + 0xc);
    return 0;
}

// FUN_00406ef0 @ 0x00406EF0 (~20 lines) — Hash function B: hash 4-byte key with configurable multiplier
// __fastcall(ecx=hashObj). Like FUN_00406e90 but uses multiplier from obj+0x1c.
UINT __fastcall FUN_00406ef0(int param_1) {
    (void)param_1;
    // uint h = 0;
    // int mult = *(int*)(param_1 + 0x1c);
    // for (i = 0; i < 4; i++) h = key_bytes[i] + mult * h;
    // return h % *(uint*)(param_1 + 0xc);
    return 0;
}

// FUN_004070d0 @ 0x004070D0 (~379 lines) — GameGuard packet handler (anti-tamper dispatch)
// Handles GameGuard message IDs 0x3E9..0x3F8. Key behaviors:
//   0x3E9/0x3EA: format param_2 into error string, show MessageBox
//   0x3F3: show error MessageBox, call CloseHack
//   0x3F4/0x3F5: format error with param_1, MessageBox, CloseHack
//   0x3F6: format error with param_2, MessageBox, CloseHack
//   0x3F8: XOR-encrypt packet with 32-byte hardcoded key {0xe7,0x6d,0x3a,...},
//           build C1 packet, lookup serial in MAIN_HASH_CLASS, send via socket
//   default: return 1
// Contains anti-tamper patterns: repeated key init forward+reverse, dead code.
int __stdcall FUN_004070d0(int param_1, int param_2) {
    (void)param_1; (void)param_2;
    // switch(param_1):
    //   case 0x3E9/0x3EA: wsprintfA(buf, param_2_template, param_2); MessageBoxA; CloseHack
    //   case 0x3F3: MessageBoxA(error); CloseHack(g_hWnd, true)
    //   case 0x3F4: wsprintfA(buf, template, param_1); MessageBoxA; CloseHack
    //   case 0x3F5: wsprintfA(buf, template, param_2)
    //   case 0x3F6: wsprintfA(buf, param_2_template, param_2); MessageBoxA; CloseHack
    //   case 0x3F8: build XOR-encrypted C1 packet:
    //     32-byte key = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,...}
    //     XOR payload with key[i%32] ^ prev_byte
    //     Lookup serial in MAIN_HASH_CLASS, send packet via socket
    //   default: return 1
    return 1;
}











// ParseNodes @ 0x0040AB90 (~207 lines) — SMD parser: parse "nodes" section
// Reads SMD file tokens via FUN_0040b350. Scans for "nodes" keyword, then parses each node entry:
// copies node name (stride 0x22) to DAT_00590c12 array, reads parent bone index as short.
// Then scans for "skeleton" keyword for next section. Increments DAT_00590c10 (node count).
void __cdecl ParseNodes(void) {
    // Skip tokens until "nodes" keyword found (compare with DAT_0055908c)
    // For each node until '}' (token type 2):
    //   If token type 0 and != '}': increment DAT_00590c10
    //   If token type 1: copy name to DAT_00590c12[nodeIdx*0x22], read parent index
    // Then scan for "skeleton" keyword
    // stub: 207 lines of token parsing
}

// ParseSkeleton @ 0x0040AE10 (~118 lines) — SMD parser: parse "skeleton" section
// Reads skeleton frames. Scans for "skeleton" keyword, then reads frame data:
// DAT_00794490 = frame count. For each frame: reads time value, then per-node
// reads 6 floats (3 position + 3 rotation) into bone arrays at DAT_0079449c (stride 0x4B0).
void __cdecl ParseSkeleton(void) {
    // Skip tokens until "skeleton"
    // DAT_00794490 = 0
    // For each frame until '}':
    //   If "Ctime" token: read time, then per node (DAT_00590c10 nodes):
    //     Read 6 values via FUN_0040b350 -> DAT_00809798
    //     Store position[3] + rotation[3] per bone into DAT_0079449c + frame*0x4B0
    //   DAT_00794490++
}

// ParseTriangles @ 0x0040AFB0 (~156 lines) — SMD parser: parse "triangles" section
// Scans for "triangles" keyword. For each triangle: reads material name (stride 0x20)
// into DAT_0059396a, then reads 3 vertices (each: bone index as short + 8 floats for
// pos/normal/UV). Supports Flip parameter to reverse vertex winding.
// DAT_00593968 = triangle count.
void __cdecl ParseTriangles(bool Flip) {
    (void)Flip;
    // Skip tokens until "triangles"
    // DAT_00593968 = 0
    // For each triangle until '}':
    //   Copy material name to DAT_0059396a[triIdx*0x20]
    //   For each of 3 vertices (order depends on Flip):
    //     Read bone index (short), 3 pos floats, 3 normal floats, 2 UV floats
    //     Store into DAT_00608c6c arrays (stride 0x12 shorts)
    //   DAT_00593968++
}

// FUN_0040b350 @ 0x0040B350 (~136 lines) — SMD tokenizer: read next token from SMD file
// Reads from global SMDFile (FILE*). Skips whitespace and // comments.
// Token types: 0=string (stored in DAT_00590b10), 1=number (stored in DAT_00809798 as float),
// 2=EOF, 0x23='#', 0x2c=',', 0x3b=';', 0x7b='{', 0x7d='}'.
// Handles quoted strings (delimited by '"'), numbers with sign/decimal.
int __stdcall FUN_0040b350(void) {
    // Read char from SMDFile via FUN_0054218a
    // Skip whitespace (FUN_005420dd), skip // comments
    // Switch on char:
    //   '"': read quoted string into DAT_00590b10, return 0
    //   '#',',',';','{','}': return char code
    //   '-','.',0-9: read number, parse via FUN_00542133 -> DAT_00809798, return 1
    //   else: read identifier into DAT_00590b10, return 0
    // EOF: return 2
    return 0;
}

// FixupSMD @ 0x0040B630 (~386 lines) — SMD: post-process skeleton + triangle data
// Builds bone matrices from parsed skeleton data. For root bones (parent==-1): AngleMatrix
// from rotation, scale position by _DAT_0055259c, copy to bone transform.
// For child bones: AngleMatrix + R_ConcatTransforms with parent matrix.
// Then processes triangle vertices: transforms by bone matrix, computes normals.
// Large function — summary stub only.
void __stdcall FixupSMD_stub(void) {
    // For each node (DAT_00590c10):
    //   if parent == -1: AngleMatrix(rotation, matrix), FUN_004f9e90(matrix, output), copy position
    //   else: AngleMatrix(rotation, local), R_ConcatTransforms(parent, local, result)
    // For each triangle vertex:
    //   Transform position by bone matrix
    //   Transform normal by bone rotation
    //   Scale by _DAT_0055259c
    // stub: 386 lines of matrix math + vertex transformation
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 12
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 13 — GameGuard / Crypto / IAT thunks (0x0053CB86–0x005406F0)
// 22 addresses total; some already implemented in earlier batches or GameGuard_Init2.cpp
// ═══════════════════════════════════════════════════════════════════════════════

// --- IAT thunks (6 bytes each) — jmp [__imp_xxx] trampolines.
// The linker resolves these; no C stub needed.  Documented here only.
//
// gluOrtho2D              @ 0x0053CB86 (6 bytes) — IAT thunk → opengl32.dll!gluOrtho2D
// send                    @ 0x0053CB92 (6 bytes) — IAT thunk → ws2_32.dll!send
// GetFileVersionInfoA     @ 0x0053CB9E (6 bytes) — IAT thunk → version.dll
// GetFileVersionInfoSizeA @ 0x0053CBA4 (6 bytes) — IAT thunk → version.dll

// --- Packet crypto helpers ---

// FUN_0053d170 @ 0x0053D170 — helper: bit-to-byte index (param >> 3)
static int GG_BitToByteIndex(int param_1) {
    return param_1 >> 3;
}





// CSimpleModulus::LoadEncryptionKey (0x0053D180) — thin wrapper around D1C0.
BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD *self, const char *fn) {
    return FUN_0053d1c0(self, fn, 0x1112, 1, 1, 0, 1);
}
// CSimpleModulus::LoadDecryptionKey (0x0053D1A0) — thin wrapper around D1C0.
BOOL __cdecl CSimpleModulus_LoadDecryptionKey(DWORD *self, const char *fn) {
    return FUN_0053d1c0(self, fn, 0x1112, 1, 0, 1, 1);
}

// --- GameGuard core ---
// FUN_0053d430 @ 0x0053D430 (325 bytes) — already declared in functions.h
//   and called via FUN_00412700 wrapper (stubs.cpp ~line 10835). No new body needed.

// FUN_0053d620 @ 0x0053D620 (424 bytes) — GameGuard cleanup
// Writes encrypted timestamp to log file, closes handle, deletes critical section,
// releases crypto context via FUN_00540ac0.
void __fastcall FUN_0053d620(DWORD param_1) {
    (void)param_1;
}

// FUN_0053d7d0 @ 0x0053D7D0 (187 bytes) — GameGuard main check routine
// Called periodically from game loop. If ctx->initialized==0, opens log + returns.
// Otherwise dispatches heartbeat via FUN_005404a0 and check via FUN_004070d0.
int __cdecl FUN_0053d7d0(void *self, char *param_1) {
    (void)self; (void)param_1;
    return 0;
}

// FUN_0053d890 @ 0x0053D890 (4130 bytes) — already implemented in
//   src/GameGuard/GameGuard_Init2.cpp. No new body needed.

// FUN_0053e8c0 @ 0x0053E8C0 — GameGuard encrypted string decoder
// In-place XOR: if str[0]==0x01, derives LCG key from str[1], decodes str[4..N]
// where N = (str[2]^key1) | ((str[3]^key2) << 8).  Overwrites str[0..N-1].
// GameGuard_Init2.cpp declares as: void* __cdecl FUN_0053e8c0(void *key)
void* __cdecl FUN_0053e8c0(void *param_1) {
    char *p = (char *)param_1;
    if (!p) return p;
    if (*p == 1) {
        // XOR decode in-place using LCG from byte[1]
        // In our build strings are already decoded; return offset+4
        return p + 4;
    }
    return p;
}

// FUN_0053e930 @ 0x0053E930 (337 bytes) — GameGuard log flush + close
// Identical pattern to FUN_0053d620 but operates on an int* log struct.
// GetLocalTime, sprintf timestamp, XOR-encrypt, WriteFile, CloseHandle.
void __fastcall FUN_0053e930(int *param_1) {
    (void)param_1;
}

// FUN_0053ea90 @ 0x0053EA90 (263 bytes) — already declared in functions.h
//   and body exists at stubs.cpp ~line 10938.  No new body needed.

// FUN_0053eba0 @ 0x0053EBA0 — GameGuard encrypted log writer
// EnterCriticalSection, vsprintf, GetLocalTime, XOR-encrypt with LCG,
// WriteFile, LeaveCriticalSection.
// GameGuard_Init2.cpp declares as: void __cdecl FUN_0053eba0(int *log, void *msg)
void __cdecl FUN_0053eba0(int *param_1, void *param_2) {
    (void)param_1; (void)param_2;
}

// FUN_0053ed80 @ 0x0053ED80 (538 bytes) — GameGuard full shutdown
// Stops monitoring thread (FUN_0053f290), closes event handles (0x338/0x33c),
// flushes+closes log, crash-reports via FUN_0053f680, frees DLL via FUN_005406f0.
int __fastcall FUN_0053ed80(char *param_1) {
    if (param_1) param_1[1] = 1; // mark as shut down
    return 0;
}

// FUN_0053efa0 @ 0x0053EFA0 — GameGuard DLL loader + monitoring thread launcher
// Builds path, LoadLibraryA, GetProcAddress(ordinal 1 & 2),
// CreateEvent + CreateThread(StartAddress@0x0053f370).
// GameGuard_Init2.cpp declares as: int __cdecl FUN_0053efa0(void *ctx, int val)
int __cdecl FUN_0053efa0(void *self, int param_1) {
    (void)self; (void)param_1;
    return 1;
}

// FUN_0053f290 @ 0x0053F290 (220 bytes) — GameGuard monitoring thread stop
// Signals event, WaitForSingleObject(2000ms), TerminateThread if timeout,
// CloseHandle, FreeLibrary.
int __fastcall FUN_0053f290(int param_1) {
    (void)param_1;
    return 1;
}

// StartAddress @ 0x0053F370 (153 bytes) — GameGuard monitoring thread entry
// WaitForSingleObject loop (~3555ms timeout); calls periodic check callback,
// invokes FUN_005400d0(500,0) on failure.
static DWORD __stdcall GG_ThreadProc(LPVOID lpThreadParameter) {
    (void)lpThreadParameter;
    return 0;
}

// FUN_0053f680 @ 0x0053F680 (1473 bytes) — GameGuard crash reporter
// Registry counter (HKLM\\..., tracks consecutive crashes up to 10), encrypted log flush,
// loads MAPI32.DLL → MAPILogon/MAPISendMail to send crash report email with
// computer name, user name, timestamp, and crash reason code.
void __fastcall FUN_0053f680(int param_1) {
    (void)param_1;
}

// FUN_0053fcf0 @ 0x0053FCF0 (218 bytes) — Toolhelp32 API resolver
// GetModuleHandle("kernel32"), resolves 5 APIs (CreateToolhelp32Snapshot,
// Process32First/Next, Module32First/Next) via GetProcAddress.
// Sets DAT_083bbb18=1 when done.
int __stdcall FUN_0053fcf0(void) {
    return 0; // APIs not resolved — enumeration skipped
}

// TopLevelExceptionFilter @ 0x0053FFA0 (299 bytes) — GameGuard unhandled exception filter
// Logs crash info, calls FUN_0053fdd0/FUN_0053feb0 (snapshots),
// then FUN_0053f680 (crash reporter) + ExitProcess(100) if on main thread.
static long __stdcall GG_TopLevelExceptionFilter(void) {
    return 0; // EXCEPTION_CONTINUE_SEARCH
}

// FUN_0053fdd0 @ 0x0053FDD0 — GameGuard process snapshot (thread enumeration)
// CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD=0x2), Thread32First/Next loop
void __cdecl FUN_0053fdd0(void) {
    // no-op — GameGuard anti-tamper
}

// FUN_0053feb0 @ 0x0053FEB0 — GameGuard module snapshot
// CreateToolhelp32Snapshot(TH32CS_SNAPMODULE=0x8), Module32First/Next loop
void __cdecl FUN_0053feb0(void) {
    // no-op — GameGuard anti-tamper
}

// FUN_005400d0 @ 0x005400D0 (612 bytes) — GameGuard error dispatch / heartbeat
// Large switch: 0x60F→0x3E9, 0x610→0x3EA, 500→crash(3), 0x614→nProtect check,
// 0x615→PID mismatch(5), 0x618→timeout(9), 0x62D-0x63A→various error mappings.
// Calls FUN_004070d0 to notify game of GameGuard status.
unsigned int __stdcall FUN_005400d0(int param_1, unsigned int param_2) {
    (void)param_1; (void)param_2;
    return 1;
}

// FUN_005404a0 @ 0x005404A0 — already implemented in stubs.cpp ~line 9740 (Pipe_Write wrapper)
// FUN_005404c0 @ 0x005404C0 — already implemented in stubs.cpp ~line 11816 (RNG seed init)
// FUN_00540a70 @ 0x00540A70 — already implemented in stubs.cpp ~line 11840 (crypto ctx init)
// FUN_00540ac0 @ 0x00540AC0 — already implemented in stubs.cpp ~line 11991 (crypto cleanup)

// thunk_FUN_005406f0 @ 0x00540510 (5 bytes) — jmp FUN_005406f0 trampoline
// No separate stub needed; calls route to FUN_005406f0.

// FUN_005406f0 @ 0x005406F0 (159 bytes) — GameGuard pipe/handle cleanup
// FUN_005404a0(0x8b1,0x610,NULL), SetEvent, wait+terminate monitoring thread,
// close pipe handles (lpTargetHandle_00563b58 / 00563b5c).
int __fastcall FUN_005406f0(int param_1) {
    (void)param_1;
    return 1;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 13
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 14 — SMD model build, UI widget system, text rendering, chat path parsing
// (addresses 0x0040bba0 — 0x0040fb70)
// ═══════════════════════════════════════════════════════════════════════════════

// SMD2BMDModel @ 0x0040BBA0 (~226 lines) — Convert parsed SMD data into BMD model structure
void __cdecl SMD2BMDModel_stub(int ID, int Actions) {
    (void)ID; (void)Actions;
    // Models[ID]: set node/mesh counts, alloc bones(count*0x8C), sub-meshes(count*0x28)
    // Per sub-mesh: copy vertices, normals, UVs, material names from parsed buffers
}

// SMD2BMDAnimation @ 0x0040BF50 (~98 lines) — Convert SMD skeleton frames to BMD animation
void __cdecl SMD2BMDAnimation_stub(int ID, bool LockPosition) {
    (void)ID; (void)LockPosition;
    // Per bone: alloc pos/rot/quat arrays, copy from skeleton data, euler->quat
    // Compute root motion deltas; action_index++
}







// FUN_0040dba0 @ 0x0040DBA0 (~62 lines) — UI widget destructor (type A, vtable 0x552668)
void __fastcall FUN_0040dba0(void* param_1) { (void)param_1; }

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║ STUBS FANTASMA — NO SON LA IMPLEMENTACIÓN VIVA (2026-07-20)              ║
// ║ FUN_0040dce0 / FUN_0040def0 son el render de los 3 botones popup del     ║
// ║ ChatListBox.  Están PORTADOS en src/UI/ChatListBox.cpp como              ║
// ║ ChatLB_DrawButton / ChatLB_renderFooter (slot 24 de la vtable, al que    ║
// ║ IDA llega por el thunk sub_40D600).  Estos dos cuerpos quedan vacíos     ║
// ║ porque no los llama nadie: los únicos xrefs en IDA son sub_40DEF0 y      ║
// ║ sub_40CE20 (slot 22, el frame/BG, todavía stub).  Si algún día se        ║
// ║ portea slot 22, que llame al helper de ChatListBox.cpp — no a esto.      ║
// ╚══════════════════════════════════════════════════════════════════════════╝
void __stdcall FUN_0040dce0(int p1, int p2, float p3, float p4, float p5, float p6, DWORD p7, int p8) {
    (void)p1; (void)p2; (void)p3; (void)p4; (void)p5; (void)p6; (void)p7; (void)p8;
}
void __fastcall FUN_0040def0(int param_1) { (void)param_1; }

// FUN_0040e230 @ 0x0040E230 (~57 lines) — UI widget: check mouse in scroll/title area
int __fastcall FUN_0040e230(int param_1) { (void)param_1; return 0; }

// FUN_0040e400 @ 0x0040E400 (~69 lines) — UI widget: scrollbar + option button input
int __fastcall FUN_0040e400(int* param_1) { (void)param_1; return 1; }

// FUN_0040e5b0 @ 0x0040E5B0 (~129 lines) — Chat: parse message into path components
void __fastcall FUN_0040e5b0(void* ecx, void* /*edx*/, char* param_1) {
    (void)ecx; (void)param_1;
}

// FUN_0040e780 @ 0x0040E780 (~96 lines) — Chat: reassemble path components into string
void __fastcall FUN_0040e780(void* ecx, void* /*edx*/, char* param_1) {
    (void)ecx; (void)param_1;
}

// FUN_0040eae0 @ 0x0040EAE0 (~62 lines) — UI widget destructor (type B, vtable 0x552760)
void __fastcall FUN_0040eae0(void* param_1) { (void)param_1; }

// FUN_0040f730 @ 0x0040F730 (~58 lines) — UI list/tree widget constructor
void* __fastcall FUN_0040f730(void* param_1) { return param_1; }

// FUN_0040f860 @ 0x0040F860 (~55 lines) — UI list widget: clear items
void __fastcall FUN_0040f860(void* param_1) { (void)param_1; }

// FUN_0040f950 @ 0x0040F950 (~97 lines) — UI list widget destructor
void __fastcall FUN_0040f950(void* param_1) { (void)param_1; }

// FUN_0040fb70 @ 0x0040FB70 (~65 lines) — Text render: single line with alignment
float __fastcall FUN_0040fb70(void* ecx, void* /*edx*/, int p2, int p3, char* p4,
                              int p5, LONG p6, int p7, DWORD p8, int p9) {
    (void)ecx; (void)p2; (void)p3; (void)p4; (void)p5; (void)p6; (void)p7; (void)p8; (void)p9;
    return 0.0f;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 14
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 15 — Font/Text rendering, BST (red-black tree) cache, KillGLWindow,
//            GenerateCheckSum, DestroyWindow, WinMain (0x0041E8A0 variant)
// ═══════════════════════════════════════════════════════════════════════════════

// FUN_0040fcd0 @ 0x0040FCD0 (~305 lines) — Font_RenderText: render string to GL texture,
// measures text extent, clears pixel buffer, calls TextOutA, builds GL tex via glTexImage2D,
// inserts into BST cache (red-black tree) keyed by first-4-byte checksum.
void __cdecl FUN_0040fcd0(void* self, byte* param_1, int param_2, LONG param_3, int param_4) {
    (void)self; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}

// FUN_004102e0 @ 0x004102E0 (~58 lines) — Font_SetColorCode: interprets color-code byte
// (0xF0..0xF8, 1, 0xF4) and writes foreground/background DWORD colors into per-segment
// slot at this+0x2C + param_1*0x10.  Used by chat color markup (\x02 escape).
void __cdecl FUN_004102e0(void* self, int param_1, unsigned char param_2) {
    (void)self; (void)param_1; (void)param_2;
}

// FUN_004104b0 @ 0x004104B0 (~111 lines) — Font_ParseColorMarkup: scans string for \x02
// escape codes, builds segment table (up to 0x11 segments) at this+0x24, strips markup
// bytes from output, measures each segment via GetTextExtentPointA. Calls FUN_004102e0
// per segment.
// FUN_004104b0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_004104b0(LONG _this, char *Source)
{
  DWORD *cx; // edx
  int v3; // ebx
  int v4; // ecx
  int v5; // esi
  int v6; // edi
  int v7; // eax
  int v8; // esi
  int v9; // edx
  int v10; // ecx
  char v11; // al
  LONG *v12; // esi
  int v13; // ecx
  LONG v14; // eax
  DWORD *v15; // [esp+10h] [ebp-110h]
  struct tagSIZE sz; // [esp+14h] [ebp-10Ch] BYREF
  int v17; // [esp+1Ch] [ebp-104h]
  char Destination[253]; // [esp+20h] [ebp-100h] BYREF
  short v19; // [esp+11Dh] [ebp-3h]
  char v20; // [esp+11Fh] [ebp-1h]

  cx = (DWORD *)_this;
  sz.cx = _this;
  if ( Source )
  {
    v3 = 0;
    memset((void *)(_this + 36), 0, 0xA0u);
    v4 = strlen(Source);
    v5 = 0;
    v17 = v4;
    if ( v4 > 0 )
    {
      v6 = 0;
      v15 = cx + 9;
      do
      {
        if ( v6 >= 18 )
        {
          break;
        }
        if ( Source[v5] == 2 )
        {
          v7 = v5 - v6;
          ++v5;
          *v15 = v7;
          FUN_004102e0(cx, v3, Source[v5]);
          cx = (DWORD *)sz.cx;
          v4 = v17;
          ++v3;
          v6 += 2;
          v15 += 4;
        }
        ++v5;
      }
      while ( v5 < v4 );
    }
    cx[49] = v3;
    if ( v3 )
    {
      memset(Destination, 0, sizeof(Destination));
      v19 = 0;
      v20 = 0;
      strncpy(Destination, Source, 0x100u);
      v8 = v17;
      memset(Source, 0, 0x100u);
      v9 = 0;
      v10 = 0;
      if ( v17 > 0 )
      {
        do
        {
          if ( v9 >= v8 )
          {
            break;
          }
          v11 = Destination[v10];
          if ( v11 == 2 )
          {
            ++v10;
          }
          else
          {
            Source[v9++] = v11;
          }
          ++v10;
        }
        while ( v10 < v8 );
      }
      if ( v3 > 0 )
      {
        v12 = (LONG *)(sz.cx + 40);
        do
        {
          v13 = *(v12 - 1);
          sz.cx = 0;
          sz.cy = 0;
          GetTextExtentPointA(m_hFontDC, Source, v13, &sz);
          v14 = sz.cx;
          if ( sz.cx )
          {
            v14 = sz.cx - 1;
          }
          *v12 = v14;
          v12 += 4;
          --v3;
        }
        while ( v3 );
      }
    }
  }
}


// FUN_004105f0 @ 0x004105F0 (~55 lines) — Font_BuildColorBitmap: reads ppvBits pixel buffer
// (DAT_005590bc stride * 0x100 * 3 bpp), maps white(0xFF) pixels to m_dwTextColor and
// non-white to m_dwBackColor, applying per-segment color overrides from the segment table.
// Output goes to param_1 as DWORD array 0x100 wide per row.
void __cdecl FUN_004105f0(void* self, DWORD* param_1, int param_2, int param_3, int param_4) {
    (void)self; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}

// FUN_004106d0 @ 0x004106D0 (~110 lines) — Font_CacheLookupAndRender: looks up cached text
// entry by first-4-byte checksum + string match + color match. If found, decrements age
// counter and calls FUN_004108b0 (RenderBitmap) per texture page. Returns 1 on hit, 0 miss.
int __cdecl FUN_004106d0(void* self, byte* param_1, int param_2, int param_3, float param_4) {
    (void)self; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    return 0;
}

// FUN_004108b0 @ 0x004108B0 (~19 lines) — Font_RenderCachedBitmap: clamps X to [0,WindowWidth-w],
// calls RenderBitmap with DAT_055c9b90 texture handle, screen coords, and UV from Bitmaps[8..9].
void __stdcall FUN_004108b0(int param_1, int param_2, int param_3, int param_4) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}












// KillGLWindow @ 0x00412AF0 (~35 lines) — Releases OpenGL context (wglMakeCurrent(NULL),
// wglDeleteContext), deletes font DC, releases g_hDC via ReleaseDC, restores display
// settings via ChangeDisplaySettingsA(NULL), shows cursor.
void KillGLWindow(void) {
    // Already implemented in WinMain.cpp as OpenGL_Release
}

// GenerateCheckSum @ 0x00412D50 (~36 lines) — Computes a DWORD checksum over a byte buffer:
// iterates 4 bytes at a time, applies XOR/ADD/shift based on (wKey + i/4) % 3, then
// scrambles with right-shift by ((i/4) & 0xF) + 3. Used for packet integrity.
DWORD __cdecl GenerateCheckSum(BYTE* pbyBuffer, DWORD dwSize, WORD wKey) {
    DWORD uVar5 = (DWORD)wKey << 9;
    DWORD uVar4 = 0;
    DWORD uVar2 = (DWORD)wKey;
    while (uVar4 <= dwSize - 4) {
        DWORD uVar1 = *(DWORD*)(pbyBuffer + uVar4);
        DWORD uVar3 = (uVar2 + (uVar4 >> 2)) % 3;
        if (uVar3 == 0) {
            uVar5 = uVar5 ^ uVar1;
        } else if (uVar3 == 1) {
            uVar5 = uVar5 + uVar1;
        } else if (uVar3 == 2) {
            uVar5 = (uVar5 << ((unsigned char)(uVar1 % 0xB))) ^ uVar1;
        }
        uVar5 = uVar5 ^ ((uVar5 + uVar2) >> (((BYTE)(uVar4 >> 2) & 0xF) + 3));
        uVar4 += 4;
    }
    return uVar5;
}

// DestroyWindow @ 0x004145C0 (~262 lines) — Full cleanup: deletes fonts (g_hFont/Bold/Big),
// frees entity data, hash-table entries (SkillAttribute, CharacterMachine), releases all
// BMD models, unloads all textures, destroys COM objects (DAT_055c9ff0/f4, g_pRenderText),
// kills MuPlayer.exe, restores system parameters (screensaver etc).
void Game_DestroyWindow(void) {
    // Stub — real impl deletes fonts, entity data, hash table entries,
    // BMD models, textures, COM objects, kills MuPlayer, restores SystemParametersInfo.
}

// WinMain @ 0x0041E8A0 (~1493 lines) — This is the ACTUAL WinMain entry point.
// Already fully documented in src/WinMain.cpp at 0x0041FC00.
// Address 0x0041E8A0 is the Ghidra-identified start including anti-tamper preamble.
// No separate stub needed — covered by existing WinMain implementation.

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 15
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// Sistema de tela (capa del MG) — destructores. Port 2026-08-11.
//
// Las vtables se leyeron del binario original (`Cliente armado/main.exe`,
// MD5 eb95ac0785e40a7ad60c9ddb5d8bef34), porque su contenido es data y no
// aparece en los decompiles:
//     off_552520  (widget) = { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
//     DAT_005524e8 (nodo)  = { 0x00408680 }
// ─────────────────────────────────────────────────────────────────────────────

void __cdecl    FUN_00407970(void *_this);   // dtor de nodo   (0x407970, __cdecl)

// FUN_00408680 @ 0x00408680 — dtor "scalar deleting" del NODO (vtable[0] de
// DAT_005524e8). Port FIEL de IDA `sub_408680`:
//   flags & 2 → array: el count vive en el DWORD previo al puntero, y se
//               destruye hacia atrás (`eh vector destructor iterator`).
//   flags & 1 → además libera la memoria.
// El iterador se despliega inline porque los dtors de elemento de las dos
// clases tienen convenciones distintas (__cdecl acá, __fastcall en el widget).
void __cdecl FUN_00408680(void *_this, char flags)
{
    char *lpMem = (char *)_this;
    if (!lpMem) return;
    if (flags & 2) {
        int count = *((int *)lpMem - 1);
        for (int i = count - 1; i >= 0; --i) FUN_00407970(lpMem + (size_t)0x3C * i);
        if (flags & 1) FUN_0054158c((unsigned char *)(lpMem - 4));
    } else {
        FUN_00407970(lpMem);
        if (flags & 1) FUN_0054158c((unsigned char *)lpMem);
    }
}

// FUN_0045aaa0 @ 0x0045AAA0 — dtor "scalar deleting" del WIDGET de tela
// (vtable[0] de off_552520). Igual, con stride 0x54 y dtor `sub_408070`.
void __fastcall FUN_0045aaa0_impl(void *_this, char flags)
{
    char *lpMem = (char *)_this;
    if (!lpMem) return;
    if (flags & 2) {
        int count = *((int *)lpMem - 1);
        for (int i = count - 1; i >= 0; --i) FUN_00408070(lpMem + (size_t)0x54 * i);
        if (flags & 1) FUN_0054158c((unsigned char *)(lpMem - 4));
    } else {
        FUN_00408070(lpMem);
        if (flags & 1) FUN_0054158c((unsigned char *)lpMem);
    }
}
