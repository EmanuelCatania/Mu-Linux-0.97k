// UI_LegacyWidgets.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

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

// STUBS FANTASMA — NO SON LA IMPLEMENTACIÓN VIVA.
// FUN_0040dce0 / FUN_0040def0 son el render de los 3 botones popup del
// ChatListBox.  Están PORTADOS en src/UI/ChatListBox.cpp como
// ChatLB_DrawButton / ChatLB_renderFooter (slot 24 de la vtable, al que IDA
// llega por el thunk sub_40D600); el slot 22 (sub_40CE20, ChatLB_renderBg)
// también usa ChatLB_DrawButton.  Estos dos cuerpos quedan vacíos porque no
// los llama nadie.
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
// FUN_004104b0 (IDA-activated)
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
          GetTextExtentPointA(gFont.GetTextDC(), Source, v13, &sz);
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

// Game_DestroyWindow @ 0x004145C0 (IDA `DestroyWindow`, 1039 bytes) — la limpieza
// de salida.  Su unico llamador es el final de WinMain (0x42207B), despues del
// bucle de mensajes.
//
// Se porta SOLO lo que tiene efecto observable o es trivialmente seguro.  Lo
// que sigue queda fuera A PROPOSITO:
//
//  - Los dos bloques de hash-table (re-encriptado de SkillAttribute y de
//    CharacterMachine): anti-tamper, fuera por politica del proyecto.
//
//  - `SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_iScreenSaverOldValue, 0, 0)`
//    y el `SystemParametersInfoA(0x61, 0, ...)`.  **Portarlos seria un bug, no
//    una mejora**: el lado de ARRANQUE que guarda el valor viejo es el port de
//    IDA gated por `IDA_PORT_00422074`, que nunca se compila (quedo en
//    docs/codigo-muerto/ida-ports/FUN_00422074.cpp), asi que nunca
//    desactivamos el salvapantallas y `g_iScreenSaverOldValue` vale 0.
//    Restaurar ese 0 le DESACTIVARIA el salvapantallas al usuario de forma
//    permanente.  Si algun dia se porta ese arranque, estas dos lineas vuelven
//    junto con el.
//
//  - Los frees masivos (`BMD::Release` de los modelos 160..962, `UnloadImage`
//    de las 1450 texturas, ModelsDump, RendomMemoryDump, SkillAttribute,
//    CharacterMachine, GateAttribute).  El proceso termina inmediatamente
//    despues, asi que el SO los reclama igual; y nuestros pools no tienen los
//    mismos tamanos que el binario, con lo cual un recorrido a ciegas puede
//    crashear al cerrar.  Un crash de salida es peor que una limpieza que no
//    hace falta.
//
//  - `g_pRenderText`: en este arbol es un objeto stub (`g_RenderTextStubObj`),
//    no un objeto con vtable real, asi que no tiene destructor que llamar.
void Game_DestroyWindow(void) {
    // Fuentes GDI (IDA L1-12).
    gFont.Release();

    // Los dos widgets de lista, por el slot 0 de su vtable con flag 1
    // ("scalar deleting destructor": ademas libera la memoria).  IDA L~200.
    typedef void (__fastcall *FnDtor)(DWORD*, int /*edx*/, char);
    if (DAT_055c9ff0) {
        DWORD* obj = (DWORD*)DAT_055c9ff0;
        void** vt  = (void**)*obj;
        if (vt && vt[0]) ((FnDtor)vt[0])(obj, 0, 1);
        DAT_055c9ff0 = 0;
    }
    if (DAT_055c9ff4) {
        DWORD* obj = (DWORD*)DAT_055c9ff4;
        void** vt  = (void**)*obj;
        if (vt && vt[0]) ((FnDtor)vt[0])(obj, 0, 1);
        DAT_055c9ff4 = 0;
    }

    // Mata el reproductor de musica.  ESTA es la parte con efecto observable:
    // MuPlayer.exe es un proceso EXTERNO, asi que si no se cierra la musica
    // sigue sonando despues de que el cliente termino.  IDA manda WM_DESTROY
    // (2); Music.cpp usa WM_CLOSE para lo mismo desde StopMp3.
    {
        HWND hPlayer = FindWindowA(NULL, s_MuPlayer_00559110);
        if (hPlayer) SendMessageA(hPlayer, WM_DESTROY, 0, 0);
    }
}

// WinMain @ 0x0041E8A0 (~1493 lines) — This is the ACTUAL WinMain entry point.
// Already fully documented in src/WinMain.cpp at 0x0041FC00.
// Address 0x0041E8A0 is the Ghidra-identified start including anti-tamper preamble.
// No separate stub needed — covered by existing WinMain implementation.

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 15
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// Sistema de tela (capa del MG) — destructores.
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
        if (flags & 1) operator_delete((unsigned char *)(lpMem - 4));
    } else {
        FUN_00407970(lpMem);
        if (flags & 1) operator_delete((unsigned char *)lpMem);
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
        if (flags & 1) operator_delete((unsigned char *)(lpMem - 4));
    } else {
        FUN_00408070(lpMem);
        if (flags & 1) operator_delete((unsigned char *)lpMem);
    }
}
