// Scene_CharSelect_Nav.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 13751-14826 (1076 lines).
//
// Char-select slot navigation helpers:
//   FUN_004011d0 (CharSlot_FindFirstActive)
//   FUN_00401650 (CharSlot navigation)
//   FUN_004017e0 (slot-list scroll)
//   FUN_00401960 (slot-list navigation forward)
//   FUN_00401af0 (slot-list navigation back)
//   ... and related slot scroll/select helpers.
//
// Manejan el scroll del panel de char-select / lista de clases y la elección de slot.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);
extern "C" float __cdecl CalcDurabilityPercent(BYTE dur, BYTE maxDur, int Level, int option);
extern "C" void __cdecl PlusSpecial(unsigned short *Value, int Special, DWORD Item);
// -- Declaraciones de funciones que viven en otros modulos --------------
// Agregadas por el refactor B3: se declaraban localmente en el archivo del
// que se movieron estas funciones. Migrar a functions.h mas adelante.
int __cdecl LevelConvert(BYTE Level);
int __cdecl Net_Disconnect(int ctx);



extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif

// IDA Hex-Rays intrinsic shims (mirror of stubs.cpp shims).
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SHIBYTE(x)           (*(((char*)&(x))+1))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif

// ── Char-select slot navigation helpers ────────────────────────────────────────

// FUN_004011d0 @ 0x004011d0 — CharSlot_FindFirstActive(this, slot_list, dir)
// Escanea slot_list buscando la primera entrada distinta de cero en la dirección dir.
// Devuelve el handle del slot, o decrementa el índice de vista y llama a FUN_00401730.
short __fastcall FUN_004011d0(void *pThis, short *param_1, int param_2)
{
    int iVar1 = 0;
    char *pcVar2 = (char *)(*(BYTE *)((int)pThis + 4) + 0x2c + (int)param_1);
    if (0 < *param_1) {
        do {
            if (*pcVar2 != '\0') {
                return param_1[param_2 + iVar1 * 9 + 0x18];
            }
            iVar1++;
            pcVar2 += 0x12;
        } while (iVar1 < *param_1);
    }
    *(char *)((int)pThis + 0x1c87a) = *(char *)((int)pThis + 0x1c87a) - 1;
    FUN_00401730(pThis, -1);
    return *(short *)((int)pThis + 0x1c880);
}

// FUN_004016e0 @ 0x004016e0 — CharSlot_DecodePrevState(this, dir)
// Decodifica el estado de selección anterior desde el campo de bits empaquetado en +0x1c848.
void __fastcall FUN_004016e0(void *pThis, int param_1)
{
    uint uVar2;
    if (param_1 == -1) {
        uVar2 = 0;
    } else {
        uVar2 = (uint)(*(BYTE *)((int)pThis + 0x1c87a) >> 2);
    }
    int iVar3 = *(BYTE *)((int)pThis + 0x1c87a) - (int)uVar2;
    BYTE bVar1;
    if (iVar3 == 0) {
        bVar1 = *(BYTE *)(uVar2 + 0x1c848 + (int)pThis);
    } else {
        bVar1 = *(BYTE *)(uVar2 + 0x1c848 + (int)pThis) >> (((BYTE)iVar3 & 0xf) << 1);
    }
    if (param_1 == -1) {
        *(BYTE *)((int)pThis + 0x1c882) = bVar1 & 3;
    }
}

// FUN_00401650 @ 0x00401650 — CharSlot_FindEquipped(this, slot_list)
// Busca el primer slot de personaje equipado y dispara la navegación de FUN_004011d0.
uint __fastcall FUN_00401650(void *pThis, short *param_1)
{
    BYTE *in_EAX = 0;
    int iVar3 = 0;
    if (0 < *param_1) {
        BYTE *pbVar4 = (BYTE *)(param_1 + 0x14);
        do {
            in_EAX = pbVar4 + (uint)*(BYTE *)((int)pThis + 4) + (-0x28 - (int)param_1);
            if (((in_EAX + 0x2c)[(int)param_1] == 1) &&
                ((in_EAX = (BYTE *)(pbVar4[-1] - 1), in_EAX == (BYTE *)0x0))) {
                int iVar2 = FUN_00482dd0(
                    (uint)pbVar4[1] + (uint)*pbVar4 * 0x20,
                    (uint)pbVar4[2], 0xffffffff);
                if (iVar2 != 0) {
                    short sVar1 = FUN_004011d0(pThis, param_1, 1);
                    *(short *)((int)pThis + 0x1c880) = sVar1;
                    return (uint)((unsigned int)(ULONG_PTR)pThis & 0xffffff00);
                }
                in_EAX = (BYTE *)0x0;
            }
            iVar3++;
            pbVar4 += 0x12;
        } while (iVar3 < *param_1);
    }
    return ((uint)(ULONG_PTR)in_EAX >> 8) << 8 | 1;
}

// FUN_00401730 @ 0x00401730 — CharSlot_Navigate(this, dir)
// Máquina de estados de navegación de la pantalla de selección de personaje.
void __fastcall FUN_00401730(void *pThis, char param_1)
{
    short *psVar1 = (short *)((int)pThis + (uint)*(BYTE *)((int)pThis + 0x1c87a) * 0x248 + 8);
    if (param_1 == -1) {
        FUN_004016e0(pThis, -1);
    } else {
        *(char *)((int)pThis + 0x1c882) = param_1;
    }
    char cVar2 = *(char *)((int)pThis + 0x1c882);
    if (cVar2 == '\x01') {
        uint uVar5 = FUN_00401650(pThis, psVar1);
        if ((char)uVar5 != '\0') {
            short sVar3 = FUN_004011d0(pThis, psVar1, 2);
            *(short *)((int)pThis + 0x1c880) = sVar3;
            *(BYTE *)((int)pThis + 0x1c882) = 1;
        }
    } else if (cVar2 == '\x02') {
        short sVar3 = FUN_004011d0(pThis, psVar1, 3);
        *(short *)((int)pThis + 0x1c880) = sVar3;
    } else if (cVar2 == '\x03') {
        uint uVar4 = FUN_00401230(pThis, psVar1, '\0');
        if ((char)uVar4 != '\0') {
            short sVar3 = FUN_004011d0(pThis, psVar1, 0);
            *(short *)((int)pThis + 0x1c880) = sVar3;
        }
    }
}

// FUN_00401230 @ 0x00401230 — CharSlot_Verify(this, slot_list, flag)
// Escanea la lista de slots de personaje buscando uno equipado que coincida, y actualiza
// this+0x1c880 (handle seleccionado) y this+0x1c882 (state=5 si lo encontró).
// Heavy HashTable obfuscation stripped — core logic preserved.
// Devuelve 0 (byte bajo) si encontró y seleccionó, 1 si no lo encontró.
uint __fastcall FUN_00401230(void *pThis, short *param_1, char param_2)
{
    int iVar8 = 0;
    int local_8 = 0;
    if (0 < *param_1) {
        int local_10 = 0;
        char *local_c = (char *)((int)param_1 + 0x2b);
        do {
            if ((uint)*(BYTE *)((uint)*(BYTE *)((int)pThis + 4) + local_10 + 0x2c + (int)param_1) ==
                *(BYTE *)((int)pThis + 5) + 1) {
                iVar8 = 0;
                char cVar1 = *local_c;
                if (0 < param_1[1]) {
                    short *puVar12 = (short *)(param_1 + 0xa6);
                    do {
                        if ((*(char *)((int)puVar12 + -3) == cVar1) ||
                            (*(char *)((int)puVar12 + -3) == -1)) {
                            ushort uVar3 = *(ushort *)((BYTE *)DAT_07cf1ff4 + 0xe);
                            if (*puVar12 != 0 && uVar3 < *puVar12) {
                                *(short *)((int)pThis + 0x1c880) = param_1[iVar8 * 8 + 0xaa];
                                *(BYTE *)((int)pThis + 0x1c882) = 5;
                                return (uint)(ULONG_PTR)pThis & 0xffffff00;
                            }
                            if (puVar12[1] != 0 && puVar12[1] < uVar3) {
                                short sVar4 = param_1[iVar8 * 8 + 0xaa];
                                *(BYTE *)((int)pThis + 0x1c882) = 5;
                                *(short *)((int)pThis + 0x1c880) = sVar4;
                                return (uint)(ULONG_PTR)pThis & 0xffffff00;
                            }
                            if (*(int *)(puVar12 + 2) != 0) {
                                if (param_2 == '\0') {
                                    *(int *)((int)pThis + 0x1c884) = *(int *)(puVar12 + 2);
                                } else {
                                    int iVar5 = ((int *)DAT_07cf1ffc)[0x152];
                                    int iVar6 = *(int *)(puVar12 + 2);
                                    *(int *)((int)pThis + 0x1c884) = iVar6;
                                    if (iVar5 < iVar6) {
                                        short sVar4 = param_1[iVar8 * 8 + 0xaa];
                                        *(BYTE *)((int)pThis + 0x1c882) = 5;
                                        *(short *)((int)pThis + 0x1c880) = sVar4;
                                        return (uint)(ULONG_PTR)pThis & 0xffffff00;
                                    }
                                }
                            }
                        }
                        iVar8++;
                        puVar12 += 8;
                    } while (iVar8 < param_1[1]);
                }
            }
            local_10 += 0x12;
            local_c += 0x12;
            local_8++;
        } while (local_8 < *param_1);
    }
    return ((uint)(ULONG_PTR)iVar8 >> 8) << 8 | 1;
}


// ─────────────────────────────────────────────────────────────────────────────
// Math utilities
// ─────────────────────────────────────────────────────────────────────────────

// FUN_00512a10 @ 0x00512A10 — Math_Fmin(a, b) → min(a,b)
// FUN_00512a10 (IDA-activated, was Ghidra stub)
double __cdecl FUN_00512a10(float a1, float a2)
{
  if ( a1 <= (double)a2 )
  {
    return a1;
  }
  else
  {
    return a2;
  }
}


// FUN_00512a30 @ 0x00512A30 — Math_Fmax(a, b) → max(a,b)
// FUN_00512a30 (IDA-activated, was Ghidra stub)
double __cdecl FUN_00512a30(float a1, float a2)
{
  if ( a1 <= (double)a2 )
  {
    return a2;
  }
  else
  {
    return a1;
  }
}


// ─────────────────────────────────────────────────────────────────────────────
// Vec3 math helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_004f9cb0 @ 0x004F9CB0 — Vec3_Multiply(a, b, out) — component-wise multiply
void __cdecl FUN_004f9cb0(float *param_1, float *param_2, float *param_3)
{
    *param_3     = *param_1 * *param_2;
    param_3[1]   = param_1[1] * param_2[1];
    param_3[2]   = param_1[2] * param_2[2];
}

// FUN_004f9d20 @ 0x004F9D20 — Vec3_Cross(a, b, out) — standard cross product
void __cdecl FUN_004f9d20(float *param_1, float *param_2, float *param_3)
{
    *param_3     = param_2[2] * param_1[1] - param_1[2] * param_2[1];
    param_3[1]   = param_1[2] * *param_2  - *param_1  * param_2[2];
    param_3[2]   = *param_1   * param_2[1] - *param_2  * param_1[1];
}

// ─────────────────────────────────────────────────────────────────────────────
// GuildMark helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_004fa5a0 @ 0x004FA5A0 — GuildMark_ResetTarget
// Limpia los tres slots de destino de marca de guild (todos → 0xffffffff) y resetea el
// float distance register to -1.0 (0xbf800000).
void FUN_004fa5a0(void)
{
    _DAT_0055a7bc = -1.0f;  // 0xbf800000 in IEEE 754
    DAT_0055a7b0  = 0xffffffff;
    DAT_0055a7b4  = 0xffffffff;
    DAT_0055a7b8  = 0xffffffff;
}

// ─────────────────────────────────────────────────────────────────────────────
// Entity / Character helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_00444b30 @ 0x00444B30 — Entity_SetIdleAction
// Setea la animación de idle de una entidad: DK (tipo 0x186) → acción 0x57, el resto → 5.
void __cdecl FUN_00444b30(int param_1)
{
    if (*(short *)(param_1 + 2) == 0x186) {
        FUN_0043e820(param_1, 0x57);
    } else {
        FUN_0043e820(param_1, 5);
    }
}

// FUN_0047e350 @ 0x0047E350 — CharData_CalcNextLevelExp
// Calcula la experiencia que falta para el nivel siguiente y la guarda en
// el struct de datos del personaje, en el offset +0x34.
// Formula: (lvl+9)*lvl²*10, plus cubic correction term if lvl > 255.
void __fastcall FUN_0047e350(int param_1)
{
    uint uVar1 = (uint)*(ushort *)(param_1 + 0xe);
    int  iVar2 = (uVar1 + 9) * uVar1 * uVar1 * 10;
    *(int *)(param_1 + 0x34) = iVar2;
    if (uVar1 > 0xff) {
        *(uint *)(param_1 + 0x34) = iVar2 +
            (uVar1 - 0xf6) * (uVar1 - 0xff) * (uVar1 - 0xff) * 1000;
    }
}

// FUN_0047e3a0 @ 0x0047E3A0 — HotkeyBar_FindFreeSlot
// Scans 12 hotkey bar slots (stride 0x44) starting at CharData+0x232.
// Returns slot index with low byte = 1 on success, or raw index (no flag) if full.
// FUN_0047e3a0 (IDA-activated, was Ghidra stub)
char __cdecl FUN_0047e3a0(char *_this)
{
  int v1; // eax
  BYTE *i; // ecx

  v1 = 0;
  for ( i = (BYTE *)(_this + 562); *i; i += 68 )
  {
    if ( ++v1 >= 12 )
    {
      return 0;
    }
  }
  return 1;
}


// FUN_0045acc0 @ 0x0045ACC0 — Character_FindByKey_WithClear
// Scans entity array (stride 0x394, 400 entries) for an entity whose
// la clave (short en +0x1dc) coincide con param_1. Limpia el byte +0x2e8 de cada
// entidad durante el escaneo. Devuelve el índice coincidente, o 400 si no lo encontró.
// FUN_0045acc0 (IDA-activated, was Ghidra stub)
int __cdecl FUN_0045acc0(int a1)
{
  int v1; // esi
  int result; // eax
  int i; // ecx

  v1 = 0;
  result = 400;
  for ( i = 0; i < 366400; i += 916 )
  {
    if ( *(BYTE *)(i + CharactersClient) )
    {
      if ( *(short *)(i + CharactersClient + 476) == a1 )
      {
        result = v1;
      }
    }
    *(BYTE *)(i + CharactersClient + 744) = 0;
    ++v1;
  }
  return result;
}


// FUN_0045ad10 @ 0x0045AD10 — Characters_SetActionAll
// Setea la acción dada en todas las entidades vivas de tipo DK (0x186).
// Resetea el Angle a (0, 0, 180°) antes de aplicar la acción.
// FUN_0045ad10 (IDA-activated, was Ghidra stub)
void __cdecl FUN_0045ad10(int Action)
{
  int i; // esi
  DWORD v2; // eax
  DWORD v3; // [esp-8h] [ebp-14h]

  for ( i = 0; i < 366400; i += 916 )
  {
    v2 = i + CharactersClient;
    if ( *(BYTE *)(i + CharactersClient) )
    {
      if ( *(WORD *)(v2 + 2) == 390 )
      {
        v3 = i + CharactersClient;
        *(DWORD *)(v2 + 28) = 0;
        *(DWORD *)(v2 + 32) = 0;
        *(DWORD *)(v2 + 36) = 1127481344;
        SetAction(v3, Action);
      }
    }
  }
}


// FUN_0045ad60 @ 0x0045AD60 — Characters_FreeAllBMDBuffers
// Libera los buffers de heap de BMD por entidad (puntero en entity+0x114) de todo el array,
// y después libera el buffer BMD extra compartido (DAT_07abf164).
// FUN_0045ad60 (IDA-activated, was Ghidra stub)
void FUN_0045ad60()
{
  int i; // edi
  LPVOID *v1; // esi

  for ( i = 0; i < 366400; i += 916 )
  {
    v1 = (LPVOID *)(i + CharactersClient + 276);
    if ( *v1 )
    {
      delete__(*v1);
      *v1 = 0;
    }
  }
  if ( DAT_07abf164 )
  {
    delete__(DAT_07abf164);
    DAT_07abf164 = 0;
  }
}


// FUN_0045c720 @ 0x0045C720 — Character_UpdateEquipSlotAnimations
// Para personajes de tipo DK: valida el estado de animación y actualiza los cinco
// equipment-slot model indices, selecting the correct animated variant based
// on move_type_flags (bits 0-2 = weapon style, bits 3+ = sub-variant offset).
void __cdecl FUN_0045c720(int param_1)
{
    if (*(short *)(param_1 + 2) != 0x186) return;

    BYTE v1 = *(BYTE *)(param_1 + 0x105);
    bool v2 = true;
    if (v1 >= 0x85u && v1 <= 0x8Cu) {
        v2 = false;
    }
    if ((v1 < 0x22u || v1 > 0x5Bu) && v2) {
        SetPlayerStop((void *)param_1);
    }

    int v3 = *(BYTE *)(param_1 + 0x1bc) & 7;
    int v4 = *(unsigned char *)(param_1 + 0x1bc) >> 3;
    if (*(short *)(param_1 + 504) == v3 + 912) {
        *(BYTE *)(param_1 + 506) = 0;
        *(WORD *)(param_1 + 504) = (WORD)(v3 + 4 * v4 + 912);
        *(BYTE *)(param_1 + 507) = 0;
    }
    if (*(short *)(param_1 + 552) == v3 + 919) {
        *(BYTE *)(param_1 + 530) = 0;
        *(WORD *)(param_1 + 528) = (WORD)(v3 + 4 * v4 + 919);
        *(BYTE *)(param_1 + 531) = 0;
    }
    if (*(short *)(param_1 + 576) == v3 + 926) {
        *(BYTE *)(param_1 + 554) = 0;
        *(WORD *)(param_1 + 552) = (WORD)(v3 + 4 * v4 + 926);
        *(BYTE *)(param_1 + 555) = 0;
    }
    if (*(short *)(param_1 + 600) == v3 + 933) {
        *(BYTE *)(param_1 + 578) = 0;
        *(WORD *)(param_1 + 576) = (WORD)(v3 + 4 * v4 + 933);
        *(BYTE *)(param_1 + 579) = 0;
    }
    if (*(short *)(param_1 + 624) == v3 + 940) {
        *(BYTE *)(param_1 + 602) = 0;
        *(WORD *)(param_1 + 600) = (WORD)(v3 + 4 * v4 + 940);
        *(BYTE *)(param_1 + 603) = 0;
    }
    SetCharacterScale((int)param_1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sound
// ─────────────────────────────────────────────────────────────────────────────

// FUN_00451a90 @ 0x00451A90 — Sound_PlayFootstep
// Plays a terrain-appropriate footstep sound.
//   World 2 (Lost Tower): tile < 10 and != 3 → snd 10
//   World 0 or 3 (Lorencia/Devias): tile == 0 → snd 9
//   World 7 (Devil Square), alive → snd 11
//   Default → snd 8
// FUN_00451a90 (IDA-activated, was Ghidra stub)
char FUN_00451a90()
{
  char result; // al

  result = *(BYTE *)(Hero + 261);
  if ( result != 30 && result != 31 )
  {
    if ( World == 2 )
    {
      if ( HeroTile != 3 && HeroTile < 10 )
      {
        return PlayBuffer(10, 0, 0);
      }
    }
    else if ( World && World != 3 )
    {
      if ( World == 7 && !*(BYTE *)(Hero + 846) )
      {
        return PlayBuffer(11, 0, 0);
      }
    }
    else if ( !HeroTile )
    {
      return PlayBuffer(9, 0, 0);
    }
    return PlayBuffer(8, 0, 0);
  }
  return result;
}


// ─────────────────────────────────────────────────────────────────────────────
// Network helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_0043daf0 @ 0x0043DAF0 — NetContext_ClearSendBuffer
// Pone en cero la cola grande de paquetes de envío (0x96258 DWORDs = ~600 KB) y resetea
// los dos contadores de longitud en los offsets +0x4014 y +0x4018.
int __fastcall FUN_0043daf0(int param_1)
{
    DWORD *puVar2 = (DWORD *)(param_1 + 0x401c);
    for (int i = 0x96258; i != 0; i--) *puVar2++ = 0;
    *(DWORD *)(param_1 + 0x4014) = 0;
    *(DWORD *)(param_1 + 0x4018) = 0;
    return param_1;
}

// FUN_0043ddd0 @ 0x0043DDD0 — Net_FlushSendBuffer
// Manda la cola de salida pendiente vía send(); maneja WSAEWOULDBLOCK
// con elegancia (devuelve 1). Llama a FUN_0043dc90 ante errores fatales de socket (devuelve 0).
// Devuelve 1 si tuvo éxito o si el bloqueo no es fatal.
int __fastcall FUN_0043ddd0(int param_1)
{
    int iVar2 = 0;
    if (0 < *(int *)(param_1 + 0x200c)) {
        int iVar1;
        do {
            iVar1 = send(*(SOCKET *)(param_1 + 8),
                         (char *)(iVar2 + param_1 + 0xc),
                         *(int *)(param_1 + 0x200c) - iVar2, 0);
            if (iVar1 == -1) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) return 1;
                FUN_0043dc90(param_1);
                return 0;
            }
            if (iVar1 < 1) { FUN_0043dc90(param_1); return 0; }
            if (*(int *)(param_1 + 0x4014) != 0) FUN_0043de60();
            iVar2 += iVar1;
            iVar1 = *(int *)(param_1 + 0x200c) - iVar1;
            *(int *)(param_1 + 0x200c) = iVar1;
        } while (0 < iVar1);
    }
    return 1;
}

// FUN_0043de70 @ 0x0043DE70 — Net_Recv
// Lee los datos entrantes del socket al buffer de recepción del contexto, y después despacha
// todos los paquetes completos vía FUN_0043df90. Los paquetes parciales los maneja corriendo
// los bytes restantes al frente del buffer.
// Returns 0 on clean dispatch, 1 on no data / WSAEWOULDBLOCK, 3 on incomplete header.
int __fastcall FUN_0043de70(void *param_1)
{
    int iVar2 = recv(*(SOCKET *)((int)param_1 + 8),
                     (char *)(*(int *)((int)param_1 + 0x4010) + 0x2010 + (int)param_1),
                     0x2000 - *(int *)((int)param_1 + 0x4010), 0);
    if (iVar2 == 0) { DbgLogPublic("NET: Net_Recv recv=0 (peer closed)"); return 1; }
    if (iVar2 == -1) {
        int err = WSAGetLastError();
        char dbg[96];
        wsprintfA(dbg, "NET: Net_Recv recv=-1 WSA=%d", err);
        DbgLogPublic(dbg);
        return 1;
    }
    // MuEmu: decrypt the fresh chunk in place (no-op for vanilla servers).
    // Esto tiene que pasar ANTES de que el parser de headers C1/C2/C3/C4 vea nada,
    // porque cada byte del cable — headers incluidos — viene encriptado.
    {
        BYTE *pNew = (BYTE*)((int)param_1 + 0x2010) + *(int *)((int)param_1 + 0x4010);
        MuEmu::DecryptRecv(pNew, iVar2);
    }
    {
        char dbg[256];
        BYTE *p = (BYTE*)((int)param_1 + 0x2010) + *(int *)((int)param_1 + 0x4010);
        int nShow = iVar2 > 24 ? 24 : iVar2;
        char hexbuf[128]; hexbuf[0] = 0;
        for (int hi = 0; hi < nShow; ++hi) {
            char tmp[8]; wsprintfA(tmp, "%02X ", p[hi]);
            lstrcatA(hexbuf, tmp);
        }
        wsprintfA(dbg, "NET: Net_Recv got %d bytes (post-decrypt) [%s]", iVar2, hexbuf);
        DbgLogPublic(dbg);
    }

    iVar2 = *(int *)((int)param_1 + 0x4010) + iVar2;
    *(int *)((int)param_1 + 0x4010) = iVar2;
    if (iVar2 < 3) return 3;

    int iOff = 0;
    while (true) {
        BYTE *pBase = (BYTE *)((int)param_1 + 0x2010);
        BYTE  hdr   = pBase[iOff];
        BYTE *pPkt  = pBase + iOff;
        uint  uLen;
        if ((hdr == 0xc1) || (hdr == 0xc3)) {
            uLen = pBase[iOff + 1];
        } else if ((hdr == 0xc2) || (hdr == 0xc4)) {
            uLen = (uint)pBase[iOff + 1] * 0x100 + pBase[iOff + 2];
        } else {
            // Después de MuEmu::DecryptRecv los bytes nuevos ya son texto plano,
            // así que cualquier cosa que no arranque con C1/C2/C3/C4 es un error real
            // de protocolo — se descarta el buffer y se sale.
            char dbg[96];
            wsprintfA(dbg, "NET: Net_Recv UNKNOWN hdr=%02X at off=%d → discard buffer", hdr, iOff);
            DbgLogPublic(dbg);
            *(int *)((int)param_1 + 0x4010) = 0;
            return 0;
        }
        if (uLen == 0) return 0;
        if (*(int *)((int)param_1 + 0x4010) < (int)uLen) break;

        {
            char dbg[80];
            wsprintfA(dbg, "NET: Net_Recv enqueue hdr=%02X len=%u", hdr, uLen);
            DbgLogPublic(dbg);
        }
        FUN_0043df90((int)param_1, (int)pPkt, uLen);
        if (*(int *)((int)param_1 + 0x4014) != 0) FUN_0043de60();
        iOff += uLen;
        int rem = *(int *)((int)param_1 + 0x4010) - uLen;
        *(int *)((int)param_1 + 0x4010) = rem;
        if (rem < 1) return 0;
    }

    if (iOff < 1) return 0;
    uint uRem = (uint)*(int *)((int)param_1 + 0x4010);
    if ((int)uRem < 1) return 0;

    // Corre los bytes restantes al frente del buffer
    BYTE *pDst = (BYTE *)((int)param_1 + 0x2010);
    BYTE *pSrc = pDst + iOff;
    for (uint i = uRem >> 2; i != 0; i--, pDst += 4, pSrc += 4)
        *(DWORD *)pDst = *(DWORD *)pSrc;
    for (uint i = uRem & 3; i != 0; i--)
        *pDst++ = *pSrc++;
    return 0;
}

// FUN_0043e010 @ 0x0043E010 — NetContext_AllocSlot
// Scans 300 packet slots (stride 0x2008) looking for a free slot (flag==1).
// Si tiene éxito lo marca como usado (flag=0) y devuelve el puntero a su región de datos (+0x4024).
// Returns NULL if no free slot is available.
int __fastcall FUN_0043e010(int param_1)
{
    int iVar1 = 0;
    int *piVar2 = (int *)(param_1 + 0x401c);
    do {
        if (*piVar2 == 1) {
            int slotBase = param_1 + iVar1 * 0x2008;
            *(DWORD *)(slotBase + 0x401c) = 0;
            return slotBase + 0x4024;
        }
        iVar1++;
        piVar2 = (int *)((int)piVar2 + 0x2008);
    } while (iVar1 < 300);
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Effect physics
// ─────────────────────────────────────────────────────────────────────────────

// FUN_0046ca00 @ 0x0046CA00 — Effect_PhysicsTick
// Actualización física por frame de una partícula de efecto a nivel del piso (p.ej. una moneda o un drop).
// Si el héroe está atacando y el efecto está en rango, lo atrae hacia
// el héroe con una velocidad proporcional al delta y un factor de fricción que decae.
// FUN_0046ca00 (IDA-activated, was Ghidra stub)
void __cdecl FUN_0046ca00(DWORD Object)
{
  unsigned char v2; // al
  double v3; // st7
  double v4; // st6
  double v5; // st7
  float v6; // [esp+4h] [ebp-1Ch]
  float v7; // [esp+8h] [ebp-18h]
  float v8; // [esp+Ch] [ebp-14h]
  float v9; // [esp+10h] [ebp-10h]
  float Objecta; // [esp+24h] [ebp+4h]
  float Objectb; // [esp+24h] [ebp+4h]

  v2 = *(BYTE *)(Hero + 261);
  if ( v2 >= 0xDu && v2 <= 0x21u && *(float *)(Object + 192) < 0.1 )
  {
    v3 = *(float *)(Hero + 16) - *(float *)(Object + 16);
    v4 = *(float *)(Hero + 20) - *(float *)(Object + 20);
    Objecta = v4;
    if ( sqrt(v4 * Objecta + v3 * v3) < 50.0 )
    {
      *(DWORD *)(Object + 200) = 0;
      *(float *)(Object + 192) = v3 * -0.40000001;
      *(float *)(Object + 196) = Objecta * -0.40000001;
      *(float *)(Object + 44) = v3 * -4.0;
      *(float *)(Object + 40) = Objecta * -4.0;
      PlayBuffer(94, Object, 0);
    }
  }
  v5 = *(float *)(Object + 192) * 0.60000002;
  *(float *)(Object + 192) = v5;
  Objectb = *(float *)(Object + 196) * 0.60000002;
  *(float *)(Object + 196) = Objectb;
  v6 = *(float *)(Object + 200) * 0.60000002;
  *(float *)(Object + 200) = v6;
  v7 = *(float *)(Object + 40) * 0.60000002;
  *(float *)(Object + 40) = v7;
  v8 = *(float *)(Object + 44) * 0.60000002;
  *(float *)(Object + 44) = v8;
  v9 = *(float *)(Object + 48) * 0.60000002;
  *(float *)(Object + 48) = v9;
  *(float *)(Object + 16) = v5 + *(float *)(Object + 16);
  *(float *)(Object + 20) = Objectb + *(float *)(Object + 20);
  *(float *)(Object + 24) = v6 + *(float *)(Object + 24);
  *(float *)(Object + 28) = v7 + *(float *)(Object + 28);
  *(float *)(Object + 32) = v8 + *(float *)(Object + 32);
  *(float *)(Object + 36) = v9 + *(float *)(Object + 36);
}


// ─────────────────────────────────────────────────────────────────────────────
// Floating labels
// ─────────────────────────────────────────────────────────────────────────────

// FUN_004793f0 @ 0x004793F0 — FloatingLabel_Add
// Agrega al pool una etiqueta flotante de daño/curación (base DAT_07c82cd0, stride 0x70,
// hasta el límite del pool en 0x7c8588f). Elige el slot con la "edad" más chica
// value. Special range 0x4b5–0x4d8 gets a randomised lifetime (50+rand%32).
// param_1 = label text or ID, param_2 = world position (float[3]),
// param_3 = color/type, param_4 = screen offset (float[3]), param_5 = extra data.
// FUN_004793f0 (IDA-activated, was Ghidra stub)
int __cdecl FUN_004793f0(int a1, DWORD *a2, int a3, DWORD *a4, int a5)
{
  BYTE *v5; // edx
  int v6; // ecx
  DWORD *v7; // esi
  int result; // eax

  v5 = (BYTE *)a1;
  v6 = 9999;
  v7 = (DWORD *)&DAT_07c82cd0;
  while ( *(BYTE *)v7 )
  {
    result = v7[14];
    if ( v6 > result )
    {
      v6 = v7[14];
      v5 = (BYTE *)v7;
    }
    v7 += 28;
    if ( (int)v7 >= (int)&DAT_07c85890 )
    {
      *v5 = 0;
      return result;
    }
  }
  *(BYTE *)v7 = 1;
  v7[1] = a1;
  v7[4] = *a2;
  v7[5] = a2[1];
  v7[6] = a2[2];
  v7[10] = *a4;
  v7[11] = a4[1];
  v7[12] = a4[2];
  result = a5;
  v7[9] = a3;
  v7[13] = 1065353216;
  v7[3] = a5;
  if ( a1 >= 1205 && (a1 <= 1206 || a1 == 1240) )
  {
    result = rand() % 32 + 50;
    v7[14] = result;
  }
  return result;
}


// ─────────────────────────────────────────────────────────────────────────────
// SkillAttribute loaders
// ─────────────────────────────────────────────────────────────────────────────

// FUN_004797b0 @ 0x004797B0 — SkillAttribute_LoadNames (text file)
// Parsea un archivo de texto: lee pares de índice de skill + nombre vía GetToken y guarda
// cada string de nombre en SkillAttribute[index*300 + 4].
void __cdecl FUN_004797b0(const char *param_1)
{
    DAT_07d7806c = (FILE *)fopen(param_1, (const char *)DAT_005580ac);
    if (DAT_07d7806c == NULL) return;

    int iVar2;
    while ((iVar2 = GetToken()) != 2) {
        if (iVar2 == 1) {
            long long lVar7 = (longlong)_DAT_083a40f8;   // IDA: (__int64)TokenNumber  (_DAT_083a40f8)
            GetToken();
            // Copia TokenString a la entrada de SkillAttribute en el índice lVar7
            size_t uLen = strlen(TokenString) + 1;
            char  *pSrc = TokenString;
            char  *pDst = (char *)((int)&SkillAttribute + (int)lVar7 * 300 + 4);
            for (size_t i = uLen >> 2; i; i--, pDst += 4, pSrc += 4)
                *(DWORD *)pDst = *(DWORD *)pSrc;
            for (size_t i = uLen & 3; i; i--)
                *pDst++ = *pSrc++;
        }
    }
}

// FUN_00479830 @ 0x00479830 — LoadGlobalTextBin (NOT SkillAttribute_LoadBin!)
// Mal nombrada por Ghidra. IDA confirma que es el loader del pool de strings localizados:
// lee Data\Local\Text.bmd (300000 bytes), lo desencripta con BuxConvert_0 y llena
// el array GlobalText[] de 1000 × 300 bytes. La implementación real vive en
// src/Local/Text_Data.cpp como LoadTextData_Bin(); acá ruteamos el nombre FUN_ hacia allá
// para que cualquier cosa enlazada contra el símbolo del decompile funcione.
void __cdecl FUN_00479830(const char *param_1)
{
    LoadTextData_Bin(param_1);
}

// FUN_00479950 @ 0x00479950 — SkillAttribute_SaveBin (binary file)
// Guarda la tabla SkillAttribute a un archivo binario, encriptando con XOR cada
// entrada de 300 bytes con BuxConvert_0 antes de escribirla.
void __cdecl FUN_00479950(const char *param_1)
{
    return;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    FILE *pFile = fopen(param_1, (const char *)&DAT_005597d4);
    BYTE *Buffer = (BYTE *)operator_new(300);

    char *pcVar3 = (char *)((int)&SkillAttribute + 4);
    do {
        BYTE *pBuf = Buffer;
        char *pcVar4 = pcVar3;
        for (int i = 0x4b; i != 0; i--, pcVar4 += 4, pBuf += 4)
            *(DWORD *)pBuf = *(DWORD *)pcVar4;
        BuxConvert_0((int)Buffer, 300);
        FUN_005430f0((char *)Buffer, 300, 1, (int *)pFile);
        pcVar3 += 300;
    } while ((int)pcVar3 < 0x7d73104);

    operator_delete(Buffer);
    fclose(pFile);
}

// ─────────────────────────────────────────────────────────────────────────────
// Terrain helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_004f7060 @ 0x004F7060 — Terrain_SpawnAmbientObjects
// Itera la tabla de atributos del terreno (base DAT_081cb2ed, stride 8) y
// spawns ambient world objects (type 8) at non-zero flag positions.
void FUN_004f7060(void)
{
    // BUG-FIX 2026-06-27: el while original usaba el bound de DIRECCIÓN ABSOLUTA
    // literal del binario fuente (136099341 = 0x081CB60D). En nuestro build
    // &DAT_081cb2ed vive en otra dirección, así que el loop caminaba memoria
    // ajena hasta una página no mapeada → AV (crash @0x005762E6, addr 0x021FA007),
    // disparado al wirear sub_4F7060 dentro del port 1:1 de RenderTerrain.
    // IDA: base 0x081CB2ED, bound 0x081CB60D → (0x320)/8 = 100 iteraciones.
    BYTE *pbVar1 = (BYTE *)&DAT_081cb2ed;
    for (int i = 0; i < 100; i++, pbVar1 += 8) {
        if (pbVar1[-5] != 0) {
            float scale = (float)((double)pbVar1[1] * 1.4117647);
            FUN_004f8980(8, (int)pbVar1[-1], (int)*pbVar1, scale);
        }
    }
}

// FUN_004f9a30 @ 0x004F9A30 — Terrain_WaterWaveUpdate
// Simulación de olas de agua por frame: mezcla los valores de altura adyacentes del
// buffer de olas anterior en el buffer actual, para una grilla de terreno de 256×256.
// param_1 selects between the two ping-pong buffers (0 or 1).
// FUN_004f9a30 (IDA-activated, was Ghidra stub)
int __cdecl FUN_004f9a30(int a1)
{
  char *v1; // ebx
  int v2; // esi
  int result; // eax
  int v4; // edi
  char *v5; // edx
  char *v6; // ecx
  int v7; // eax
  char *v8; // [esp+14h] [ebp+4h]

  v1 = (char *)&DAT_0814b2e0 + 0x40000 * a1;
  v2 = 257;
  v8 = (char *)&DAT_0814b2e0 + 0x40000 * (a1 ^ 1);
  do
  {
    result = v2 + 254;
    if ( !__OFSUB__(v2, v2 + 254) )
    {
      v4 = 254;
      v5 = &v1[4 * v2];
      v6 = &v8[4 * v2 + 1024];
      v2 += 254;
      do
      {
        v7 = ((*(DWORD *)v6 + *((DWORD *)v6 - 255) + *((DWORD *)v6 - 257) + *((DWORD *)v6 - 512)) >> 1)
           - *(DWORD *)v5;
        v6 += 4;
        v5 += 4;
        result = v7 - (v7 >> 4);
        --v4;
        *((DWORD *)v5 - 1) = result;
      }
      while ( v4 );
    }
    v2 += 2;
  }
  while ( v2 < 65280 );
  return result;
}


// ─────────────────────────────────────────────────────────────────────────────
// Collision / intersection helpers
// ─────────────────────────────────────────────────────────────────────────────

// FUN_00512a50 @ 0x00512A50 — Collision_PointInPolygon
// Tests whether a 3D point (param_1,param_2,param_3) lies inside the polygon
// formado por param_4 vértices (los punteros están en el array param_5..param_8).
// param_9 elige el plano de proyección (1=YZ, 2=XZ, 4=XY y las variantes en sentido horario).
// Umbral param_10: si es > _DAT_00552580 (0.0f), param_9 se corre 3 bits a la izquierda.
// Devuelve 1 si está adentro, 0 si está afuera.
// FUN_00512a50 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00512a50(float a1, float a2, float a3, int a4, int a5, int a6, int a7, int a8, int a9, float a10)
{
  int v10; // esi
  int v11; // edx
  int v12; // edi
  int v14; // esi
  int v15; // edx
  int v16; // edi
  int v17; // esi
  int v18; // edi
  float *v19; // eax
  int v20; // esi
  int v21; // edx
  int v22; // edi
  int v23; // esi
  int v24; // edx
  int v25; // edi
  int v26; // esi
  int v27; // edi
  float *v28; // eax
  int v29[4]; // [esp+0h] [ebp-10h]

  if ( a10 > 0.0 )
  {
    a9 *= 8;
  }
  v29[0] = a5;
  v29[2] = a7;
  v29[3] = a8;
  v29[1] = a6;
  switch ( a9 )
  {
    case 1:
      v10 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v11 = 4 * a4 - 4;
      do
      {
        v12 = 4 * v10;
        if ( (*(float *)(*(int *)((char *)v29 + v11) + 8) - a3) * (*(float *)(v29[v10] + 4) - a2)
           - (*(float *)(*(int *)((char *)v29 + v11) + 4) - a2) * (*(float *)(v29[v10] + 8) - a3) <= 0.0 )
        {
          return 0;
        }
        ++v10;
        v11 = v12;
      }
      while ( v10 < a4 );
      return 1;
    case 2:
      v14 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v15 = 4 * a4 - 4;
      while ( 1 )
      {
        v16 = 4 * v14;
        if ( (*(float *)(v29[v14] + 8) - a3) * (**(float **)((char *)v29 + v15) - a1)
           - (*(float *)(*(int *)((char *)v29 + v15) + 8) - a3) * (*(float *)v29[v14] - a1) <= 0.0 )
        {
          break;
        }
        ++v14;
        v15 = v16;
        if ( v14 >= a4 )
        {
          return 1;
        }
      }
      return 0;
    case 4:
      v17 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v18 = 4 * a4 - 4;
      while ( 1 )
      {
        v19 = *(float **)((char *)v29 + v18);
        v18 = 4 * v17;
        if ( (v19[1] - a2) * (*(float *)v29[v17] - a1) - (*(float *)(v29[v17] + 4) - a2) * (*v19 - a1) <= 0.0 )
        {
          break;
        }
        if ( ++v17 >= a4 )
        {
          return 1;
        }
      }
      return 0;
    case 8:
      v20 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v21 = 4 * a4 - 4;
      while ( 1 )
      {
        v22 = 4 * v20;
        if ( (*(float *)(*(int *)((char *)v29 + v21) + 8) - a3) * (*(float *)(v29[v20] + 4) - a2)
           - (*(float *)(*(int *)((char *)v29 + v21) + 4) - a2) * (*(float *)(v29[v20] + 8) - a3) >= 0.0 )
        {
          break;
        }
        ++v20;
        v21 = v22;
        if ( v20 >= a4 )
        {
          return 1;
        }
      }
      return 0;
    case 16:
      v23 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v24 = 4 * a4 - 4;
      while ( 1 )
      {
        v25 = 4 * v23;
        if ( (*(float *)(v29[v23] + 8) - a3) * (**(float **)((char *)v29 + v24) - a1)
           - (*(float *)(*(int *)((char *)v29 + v24) + 8) - a3) * (*(float *)v29[v23] - a1) >= 0.0 )
        {
          break;
        }
        ++v23;
        v24 = v25;
        if ( v23 >= a4 )
        {
          return 1;
        }
      }
      return 0;
    case 32:
      v26 = 0;
      if ( a4 <= 0 )
      {
        return 1;
      }
      v27 = 4 * a4 - 4;
      break;
    default:
      return 1;
  }
  while ( 1 )
  {
    v28 = *(float **)((char *)v29 + v27);
    v27 = 4 * v26;
    if ( (v28[1] - a2) * (*(float *)v29[v26] - a1) - (*(float *)(v29[v26] + 4) - a2) * (*v28 - a1) >= 0.0 )
    {
      break;
    }
    if ( ++v26 >= a4 )
    {
      return 1;
    }
  }
  return 0;
}


// FUN_005130f0 @ 0x005130F0 — AABB_IntersectOBB
// Separating-axis test between an OBB (param_4..param_15) and an AABB defined
// by two corner vectors (param_2, param_3) projected onto axis param_1.
// Devuelve 1 si se superponen, 0 si están separados.
int __cdecl FUN_005130f0(float *param_1, float *param_2, float *param_3,
                          float param_4,  float param_5,  float param_6,
                          float param_7,  float param_8,  float param_9,
                          float param_10, float param_11, float param_12,
                          float param_13, float param_14, float param_15)
{
    float fVar2 = *param_1 * *param_2 + param_2[2] * param_1[2] + param_2[1] * param_1[1];
    float fVar3 = *param_1 * *param_3 + param_3[2] * param_1[2] + param_3[1] * param_1[1];
    float fMax  = FUN_00512a30(fVar2, fVar3);
    float fMin  = FUN_00512a10(fVar2, fVar3);

    float fV3 = param_6  * param_1[2] + param_4  * *param_1 + param_5  * param_1[1];
    float fV5 = param_9  * param_1[2] + param_7  * *param_1 + param_8  * param_1[1];
    float fV6 = param_12 * param_1[2] + param_10 * *param_1 + param_11 * param_1[1];
    float fV7 = param_15 * param_1[2] + param_13 * *param_1 + param_14 * param_1[1];

    float fOMin = fV3, fOMax = fV3;
    if (fV5 <= _DAT_00552580) fOMin += fV5; else fOMax = fV3 + fV5;
    if (fV6 <= _DAT_00552580) fOMin += fV6; else fOMax += fV6;
    if (fV7 <= _DAT_00552580) fOMin += fV7; else fOMax += fV7;

    if (fMin >= fOMax) return 0;
    if (fOMin >= fMax) return 0;
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────

// ── FUN_00401010 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 10-byte: simple wrappers & field ops ────────────────────────────────────

// FUN_00401010 @ 0x00401010 (10 bytes) — calls quest table init
void FUN_00401010(void) { FUN_00403ea0((void *)&DAT_00567500); }

// ── FUN_00401020 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 12-byte: CRT atexit wrappers ────────────────────────────────────────────
// 2026-05-07: these registered shutdown callbacks at literal source-binary
// direcciones (0x004XXXXX). En nuestro build el linker coloca el código en
// offsets distintos, así que dispararlas con atexit crashearía al cerrar el programa.
// Since the registered targets were stubs/no-ops anyway, neuter the wrappers.

// FUN_00401020 @ 0x00401020 (12 bytes)
void FUN_00401020(void) {}

// ── FUN_00401120 — movida desde stubs_misc2.cpp (refactor B3) ──
// FUN_00401120 @ 0x00401120 — Quest_DecryptBuf: XOR-decrypt quest record buffer.
// 3-byte repeating XOR key at DAT_00558090.
void __cdecl FUN_00401120(int buf, int size) {
    for (int i = 0; i < size; i++)
        *(byte *)(buf + i) ^= (byte)DAT_00558090[i % 3];
}

// ── FUN_004017e0 — movida desde stubs_helpers.cpp (refactor B3) ──
// FUN_004017e0 @ 0x004017e0 — CharSelect_SetServer
// Setea el server activo (DAT_005615dc = param_1), formatea el nombre del server
// en el buffer de la línea de display (DAT_083a4348) y arma la lista de nombres de personaje.
// Usa SeparateTextIntoLines para cortar por palabras el string de nivel+nombre de cada slot.
void __fastcall FUN_004017e0(int param_1)
{
    char local_48[72];

    // 2026-08-21: los accesos a la tabla de dialogos ahora van por la struct
    // DIALOG_SCRIPT (ver globals.h).  Antes eran cuatro globals escalares
    // sueltos indexados con aritmetica de puntero tipado -> lecturas fuera de
    // rango.  El de m_lpszText acertaba de casualidad: `DAT_07cf5608` es
    // DWORD* y `+ param_1 * 0x100` da los 0x400 bytes correctos.
    if (param_1 < 0 || param_1 >= DIALOG_SCRIPT_COUNT) return;   // guard de port
    const DIALOG_SCRIPT *dlg = &g_DialogScript[param_1];

    g_iCurrentDialogScript = param_1;
    DAT_083a4324 = SeparateTextIntoLines(
        dlg->m_lpszText,
        (char *)&DAT_083a44c4, 7, 0x26);

    // Zero the name display buffer (0x5f DWORD-slots = 0x17c bytes)
    int *puVar8 = (int *)DAT_083a4348;
    for (int i = 0x5f; i != 0; i--)
        *puVar8++ = 0;

    int iVar3 = 0;
    DAT_083a7c0c = 0;

    int charCount = dlg->m_iNumAnswer;
    if (charCount > 10) charCount = 10;   // la tabla tiene 10 slots de respuesta
    if (charCount > 0) {
        char *pbVar6 = (char *)DAT_083a4348;
        int iVar7 = 0;
        do {
            iVar3 = iVar7 + 1;
            wsprintfA((LPSTR)local_48, s__d___s_005580b0, iVar3,
                      dlg->m_lpszAnswer[iVar7]);
            int iVar2 = SeparateTextIntoLines(local_48, pbVar6, 1, 0x26);
            if (iVar2 < 0) {
                ((char *)DAT_083a4348)[(iVar2 + iVar7) * 0x26] = 0;
            }
            DAT_083a7c0c = DAT_083a7c0c + 1;
            pbVar6 += 0x26;
            iVar7 = iVar3;
        } while (iVar3 < charCount);
    }

    if (charCount == 0) {
        // Sin respuestas: el binario ofrece la de cerrar, GlobalText[609]
        // (disasm 0x4018B9: `push offset GlobalText+2C9Ah`, y 0x2C9AC/300 = 609).
        // 2026-08-21: el port usaba &DAT_07d566d0, que no sale de IDA.
        wsprintfA((LPSTR)local_48, s__d___s_005580b0, iVar3 + 1, GlobalText[609]);
        // Copy string to DAT_083a4348
        uint uVar4 = (uint)strlen(local_48) + 1;
        DAT_083a7c0c = 1;
        char *pbVar6 = local_48;
        char *pbVar9 = (char *)DAT_083a4348;
        for (uint u5 = uVar4 >> 2; u5 != 0; u5--)
        { *(int *)pbVar9 = *(int *)pbVar6; pbVar6 += 4; pbVar9 += 4; }
        for (uint u4 = uVar4 & 3; u4 != 0; u4--)
        { *pbVar9++ = *pbVar6++; }
    }

    SetErrorMessage(0);
}

// ── FUN_00401960 — movida desde stubs_externs.cpp (refactor B3) ──
// Declaraciones adelantadas de los helpers definidos más abajo en este archivo
void __fastcall FUN_00401960(int);
void __fastcall FUN_00401af0(void*);
void __fastcall FUN_00408cb0(int*, float);

// FUN_00402f40 @ 0x00402F40 — Packet_ParseType2 (char-select click handler type 2)
// Variante más simple: actualiza el flag de hover, llama a FUN_00401af0 para procesar el click en el personaje,
// y después chequea un hotspot secundario para el envío de keepalive / reintento.
// El tipo de retorno es undefined4, según la declaración de functions.h.
unsigned int __cdecl FUN_00402f40(void *param_1) {
    if ((0x1c1 < DAT_083a427c) && (DAT_083a427c < 0x280) &&
        (-1 < DAT_083a4278) && (DAT_083a4278 < 0x1b1))
        DAT_07d78094 = 1;
    FUN_00401af0(param_1);
    if ((0x1da < DAT_083a427c) && (DAT_083a427c < 499) &&
        (0x18a < DAT_083a4278) && (DAT_083a4278 < 0x1a3) &&
        (IsClickPushed())) {
        DAT_083a4124 = '\0';
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        FUN_00401960((int)param_1);
    }
    return 1;
}

// ── FUN_00401af0 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
// FUN_00401af0 @ 0x00401AF0 — CharSelect_ClickHandler(state_ptr)
// Full char-select row click handler: computes row from mouse Y, resolves
// char slot, builds XOR-encrypted selection packet and sends.
// Slot types from DAT_07cf5760[server*0x100+slot]: 1/3→send packet, 2→FUN_00401960
// Paquete cliente->server de quest.  IDA sub_401AF0 arma en los dos sitios
// (L152-199 y L399-446) exactamente el mismo buffer:
//     *(DWORD*)v103 = 0x01C10003   -> len=3, packet = C1 ?? A2
//     v103[4] = 0xA2               -> opcode
//     append  *(BYTE*)(this + 116858)   (indice de quest actual)
//     append  1
// = [C1][05][A2][questIndex][01].  El wrapper anti-tamper lo mete en un frame
// C3 (`local_914[0] = 0xC3`), que es lo que corresponde: HackPacketCheck.txt
// da Encrypt = 1 para el indice 162 (0xA2), o sea C3/C4 con serial.
// El server (Protocol.cpp case 0xA2 -> CGQuestStateRecv) sólo lee QuestIndex y
// avanza el estado él mismo; el byte 1 del final lo ignora.
//
// 2026-08-21: el port armaba `{0xC1,1,0,0xA2,0}` con un XOR a mano y lo mandaba
// por un sendPkt propio — ni el opcode quedaba en su lugar ni el indice de
// quest viajaba.  Ahora usa el sender estandar del proyecto.
static void Quest_SendState(void *pThis)
{
    BYTE pkt[5];
    pkt[0] = 0xC1;
    pkt[1] = 0x05;
    pkt[2] = 0xA2;
    pkt[3] = *(BYTE *)((int)pThis + 0x1c87a);   // indice de quest actual
    pkt[4] = 0x01;
    Net_SendSmallPacket(pkt, 5);
}

void __fastcall FUN_00401af0(void *param_1)
{
    // Compute Y-base of char list
    int yBase;
    if (*(char *)((int)param_1 + 0x1c882) != '\x01' &&
        *(char *)((int)param_1 + 0x1c87f) == '\x01') {
        yBase = 0xfa;
    } else {
        yBase = ((7 - ((int)DAT_083a7c0c + (int)DAT_083a4324)) * 0x12) / 2
                + 0x42 + (int)DAT_083a4324 * 0x12;
    }

    if ((int)DAT_083a4278 < 0)  return;
    if ((int)DAT_083a7c0c * 0x12 <= (int)DAT_083a4278 - yBase)  return;

    uint distU = 0x236U - DAT_083a427c;
    int  distS = (int)distU < 0 ? -(int)distU : (int)distU;
    if (0x6a < distS)  return;

    if (!IsClickPushed())  return;
    DAT_083a4124 = 0;

    int slot = ((int)DAT_083a4278 - yBase) / 0x12;
    DAT_083a42c4 = 0;
    DAT_07e11d28 = 0;
    DAT_00559bec = 6;

    if (slot < 0) {
        DAT_083a4124 = 0;
        DAT_083a42c4 = 0;
        return;
    }

    char local_d29 = '\0';

    // Lee el tipo de slot de la tabla
    // m_iReturnForAnswer[slot] del dialogo activo (IDA sub_401AF0 0x401BE4:
    // `mov eax, [eax*4 + 0x07CF5760]` con eax = curScript*0x100 + slot).
    int curScript = g_iCurrentDialogScript;
    int slotType  = 0;
    if (curScript >= 0 && curScript < DIALOG_SCRIPT_COUNT && slot >= 0 && slot < 10)
        slotType = g_DialogScript[curScript].m_iReturnForAnswer[slot];


    if (slotType == 1) {
        short *slotData = (short *)((int)param_1
                          + (uint)*(BYTE *)((int)param_1 + 0x1c87a) * 0x248 + 8);
        uint ok = FUN_00401230(param_1, slotData, '\x01');
        if ((char)ok == '\0') {
            local_d29 = '\x01';
            FUN_004017e0(*(int *)((int)param_1 + 0x1c880));
            goto done;
        }
        Quest_SendState(param_1);
    } else if (slotType == 2) {
        DAT_083a4124 = 0;
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        FUN_00401960((int)param_1);
    } else if (slotType == 3) {
        DAT_083a4124 = 0;
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        Quest_SendState(param_1);
    }

done:
    FUN_00404bc0(0x1c, 0, 0);
    // m_iLinkForAnswer[slot] = indice del dialogo siguiente (IDA sub_401AF0
    // L387: `v64 = g_DialogScript[g_iCurrentDialogScript].m_iLinkForAnswer[v100];`
    // y solo encadena `if (v64 > 0 && !v98)`).
    {
        int cur = g_iCurrentDialogScript;
        if (cur >= 0 && cur < DIALOG_SCRIPT_COUNT && slot >= 0 && slot < 10) {
            int link = g_DialogScript[cur].m_iLinkForAnswer[slot];
            if (link > 0 && local_d29 == '\0') FUN_004017e0(link);
        }
    }
}

// ── FUN_0043ce50 — movida desde stubs_game.cpp (refactor B3) ──
// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 16
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 17 — Other addresses: entity, combat, rendering, IME, chat, particles
// ═══════════════════════════════════════════════════════════════════════════════

// FUN_0043ce50 @ 0x0043CE50 (~217 lines) — builds C1 packet opcode 0x81 (action request)
void __cdecl FUN_0043ce50(unsigned char param_1, int param_2) {
    // 0x0043CE50 — Build and send C1 packet opcode 0x81 (action request)
    // param_1 = action sub-type (unused in payload beyond opcode), param_2 = 4-byte payload data
    // Packet format: [C1][len][01][81][...payload XOR-encrypted...]
    // Usa la clave XOR de 32 bytes (la misma que la encriptación del login).
    // Anti-tamper: local_d58[0..0x1f] re-initialized repeatedly — compiler artifact, skipped.
    // Envío por socket vía DAT_055ca168, con cola de WSAEWOULDBLOCK en DAT_055ca16c.

    static const unsigned char xorKey[32] = {
        0xe7, 0x6d, 0x3a, 0x89, 0xbc, 0xb2, 0x9f, 0x73,
        0x23, 0xa8, 0xfe, 0xb6, 0x49, 0x5d, 0x39, 0x5d,
        0x8a, 0xcb, 0x63, 0x8d, 0xea, 0x7d, 0x2b, 0x5f,
        0xc3, 0xb1, 0xe9, 0x83, 0x29, 0x51, 0xe8, 0x56
    };

    // Build packet buffer (max 0x401 bytes)
    unsigned char pktBuf[0x401];
    memset(pktBuf, 0, sizeof(pktBuf));

    // Header: C1 packet, opcode area
    pktBuf[0] = 0xC1;      // packet type
    pktBuf[2] = 0x01;      // sequence/sub-opcode
    pktBuf[3] = 0x81;      // opcode: action request

    // Encripta con XOR el byte del índice 3 con la clave
    {
        unsigned int idx = 3;
        unsigned int keyIdx = idx & 0x1f;
        pktBuf[idx] = pktBuf[idx] ^ xorKey[keyIdx] ^ pktBuf[idx - 2];
    }

    // Escribe el payload de 4 bytes en el offset 4, y después lo encripta con XOR
    unsigned short payloadLen = 4;
    if (payloadLen + 4 < 0x401) {
        *(int*)(pktBuf + payloadLen) = param_2;
        for (unsigned int i = payloadLen; i < payloadLen + 4; i++) {
            unsigned int keyIdx = i & 0x1f;
            pktBuf[i] = pktBuf[i] ^ xorKey[keyIdx] ^ pktBuf[i - 2];
        }
        payloadLen = 8;
    }

    // Set packet length field
    if (pktBuf[0] == 0xC1) {
        pktBuf[1] = (unsigned char)payloadLen;
    } else if (pktBuf[0] == 0xC2) {
        *(unsigned short*)(pktBuf + 1) = payloadLen;
    }

    Net_SendSmallPacket(pktBuf, payloadLen);
}

// ── FUN_0043d1d0 — movida desde stubs_externs.cpp (refactor B3) ──
void* __cdecl FUN_0043d1d0(void *ctx, void *chardata) { return nullptr; }

// ── FUN_0043d3e0 — movida desde stubs_helpers.cpp (refactor B3) ──
// FUN_0043d3e0 @ 0x0043D3E0 — HashTable_LockRead (3-arg: ctx, key, *out)
// STUB: HashTable obfuscation helper.
void __cdecl FUN_0043d3e0(int a, int* b) { (void)a; (void)b; }

// ── FUN_0043d8a0 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_0043d8a0 @ 0x0043D8A0 — HashTable_Insert_Short (__thiscall this, param_1)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_0043d8a0(void *ctx, void *out) {
    // STUB: HashTable insert (1-byte value) with obfuscation — cannot implement safely
    (void)ctx; (void)out;
}

// ── FUN_0043de60 — movida desde stubs_render_helpers.cpp (refactor B3) ──
// FUN_0043de60 @ 0x0043DE60 — NOP (empty function in original binary).
// IDA live/raw: nullsub_2 (`retn 8`), not a send-queue drain.
void FUN_0043de60(void) {}

// ── FUN_0043df90 — movida desde stubs_linker.cpp (refactor B3) ──
// ═════════════════════════════════════════════════════════════════════════════
// Tanda 20 — stubs para el linker (cuerpos vacíos de funciones que se llaman pero todavía no están decompiladas)
// ═════════════════════════════════════════════════════════════════════════════

// FUN_0043df90 @ 0x0043DF90 (38 lines) — Net_EnqueuePacket: copies packet into 300-slot queue
// Cola en this+0x401c, cada slot = 0x2008 bytes (flag de 4 bytes, largo de 4 bytes, 0x2000 de datos).
// Returns: 0=success, 1=queue full, 2=packet too large.
void __cdecl FUN_0043df90(int param_1, int param_2, int param_3) {
    // param_1 = net context ptr, param_2 = packet data ptr, param_3 = packet length
    if ((int)param_3 > 0x2000) return; // packet too large

    int *piSlot = (int *)(param_1 + 0x401c);
    for (int i = 0; i < 300; i++) {
        if (*piSlot == 0) {
            // Mark slot as occupied
            *(int *)(param_1 + i * 0x2008 + 0x401c) = 1;
            // Store length
            *(int *)(param_1 + i * 0x2008 + 0x4020) = param_3;
            // Copy packet data (DWORD-aligned copy + trailing bytes)
            DWORD *pDst = (DWORD *)(param_1 + i * 0x2008 + 0x4024);
            DWORD *pSrc = (DWORD *)param_2;
            unsigned int dwords = (unsigned int)param_3 >> 2;
            for (unsigned int d = 0; d < dwords; d++) { *pDst++ = *pSrc++; }
            unsigned int trail = (unsigned int)param_3 & 3;
            BYTE *bDst = (BYTE *)pDst; BYTE *bSrc = (BYTE *)pSrc;
            for (unsigned int b = 0; b < trail; b++) { *bDst++ = *bSrc++; }
            return;
        }
        piSlot += 0x802; // stride 0x2008 bytes = 0x802 ints
    }
    // queue full — packet dropped
}

// ── FUN_0043e050 — movida desde stubs_externs.cpp (refactor B3) ──
// ── Missing function stubs (all LNK2019 unresolved externals) ─────────────────
// Movement / pathfinding
// FUN_0043e050 @ 0x0043E050 — CreateAngle(x1, y1, x2, y2)
// Calcula el ángulo (en grados, 0..360) del punto (x1,y1) al (x2,y2).
// Usa atan2 para el caso general; trata aparte los casos de dx o dy cercanos a cero.
// Constants: _DAT_00552868 ~ 0.0001 (epsilon), _DAT_00552860 = 57.29578 (180/pi),
//            _DAT_00552848 = 90.0, _DAT_005524ec = 180.0, _DAT_0055286c = 360.0,
//            _DAT_00552580 = 0.0, _DAT_00552864 = 270.0
float __cdecl FUN_0043e050(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    if (fabsf(dx) < _DAT_00552868) {
        // Nearly vertical — return 0 or 180
        if (dy < _DAT_00552580) {
            return _DAT_00552580; // 0 degrees
        }
        return _DAT_005524ec; // 180 degrees
    }
    if (fabsf(dy) >= _DAT_00552868) {
        // General case — PORT FIEL 1:1 (0x43E0DA): el IDA usa atan2(dy/dx, 1.0)
        // (= atan del RATIO, rango [-90,90]) y RECONSTRUYE el cuadrante con +180 si
        // dx<0. Antes usábamos atan2f(dy,dx) (dos args, que YA resuelve el cuadrante)
        // Y ADEMÁS sumábamos +180 → doble corrección de 180° cuando dx<0 → el hada
        // (y todo facing hacia la izquierda) apuntaba al revés. dx≠0 aquí (guard arriba).
        float angle = atan2f(dy / dx, 1.0f) * _DAT_00552860 + _DAT_00552848;
        if (dx < _DAT_00552580) {
            angle += _DAT_005524ec; // +180 if dx negative (reconstrucción de cuadrante)
        }
        return angle;
    }
    // Nearly horizontal — return 90 or 270
    if (dx < _DAT_00552580) {
        return _DAT_00552864; // 270 degrees
    }
    return _DAT_00552848; // 90 degrees
}

// ── FUN_0043e120 — movida desde stubs_misc2.cpp (refactor B3) ──
// FUN_0043e120 @ 0x0043E120 — Angle_Clamp(current, target, maxDelta)
// Clampea la diferencia angular entre el ángulo actual y el objetivo a ±maxDelta,
// y la devuelve como el ángulo nuevo módulo 360. Todos los ángulos en unidades del juego (0..0x167).
int __cdecl FUN_0043e120(int param_1, int param_2, int param_3) {
    unsigned int uVar5 = (unsigned int)(param_1 - param_2);
    int iVar3 = (int)((uVar5 ^ ((int)uVar5 >> 0x1f)) - ((int)uVar5 >> 0x1f)); // abs
    int iVar2 = param_1;
    if (param_2 < param_1) {
        unsigned int uVar1 = (unsigned int)(param_2 - param_1) + 0x168u;
        unsigned int uVar4 = (unsigned int)((int)uVar1 >> 0x1f);
        if (iVar3 < (int)((uVar1 ^ uVar4) - uVar4)) {
            iVar2 = param_3;
            if (iVar3 <= param_3) iVar2 = iVar3;
            iVar2 = -iVar2;
        } else {
            iVar2 = iVar3;
            if (param_3 < iVar3) iVar2 = param_3;
        }
    }
    if (param_1 < param_2) {
        unsigned int uVar1 = uVar5 + 0x168u;
        unsigned int uVar4 = (unsigned int)((int)uVar1 >> 0x1f);
        if (iVar3 < (int)((uVar1 ^ uVar4) - uVar4)) {
            iVar2 = iVar3;
            if (param_3 < iVar3) iVar2 = param_3;
        } else {
            if (iVar3 <= param_3) param_3 = iVar3;
            iVar2 = -param_3;
        }
    }
    return (iVar2 + 0x168 + param_1) % 0x168;
}

// ── FUN_0043e1b0 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_0043e1b0 @ 0x0043E1B0 — TurnAngle2(curAngle, tgtAngle, step)
// Avanza curAngle hacia tgtAngle a lo sumo 'step' grados, manejando la vuelta de 360.
// Devuelve tgtAngle directo si está dentro del rango de step; si no, curAngle +/- step.
//
// BUG-FIX 2026-04-26 (audit #1): el decomp IDA original tenía 5 flags FPU x87
// sin reconstruir (`v5/v7/v9/v12/v14`) → branches indefinidos. Reescrito con
// math estándar "smooth turn-toward with 360° wrap", preservando la semántica
// observable: snap si |delta| <= step, sino avanzar `step` grados por el camino
// más corto (con wrap 0/360 respetado).
float __cdecl FUN_0043e1b0(float a1, float a2, float a3)
{
  if ( a1 < 0.0f ) a1 += 360.0f;
  if ( a2 < 0.0f ) a2 += 360.0f;

  // Shortest signed delta in (-180, +180]
  float delta = a2 - a1;
  if      ( delta >  180.0f ) delta -= 360.0f;
  else if ( delta < -180.0f ) delta += 360.0f;

  // Snap if target within step
  if ( delta >= 0.0f )
  {
    if ( delta <= a3 ) return a2;
    float next = a1 + a3;
    if ( next >= 360.0f ) next -= 360.0f;
    return next;
  }
  else
  {
    if ( -delta <= a3 ) return a2;
    float next = a1 - a3;
    if ( next < 0.0f ) next += 360.0f;
    return next;
  }
}

// ── FUN_00444410 — movida desde stubs_game.cpp (refactor B3) ──
// SetPlayerAttack @ 0x00444410 (1627 bytes) — port FIEL desde IDA (2026-05-02).
// Setea la animación de ataque + el sonido de la entidad según:
//   - Entity type (c+2): non-player (39/40/51/302/default) vs player (390)
//   - Para el jugador: helper (c+696)=818/819 → a distancia, si no las armas izquierda/derecha
//     (c+624 LH, c+648 RH) determine animation 34..89.
// Calls: SetAction (FUN_0043e820), CreateEffect (FUN_00460dc0), PlayBuffer,
//   SetAttackSpeed (FUN_00443e70). All implemented.
//
// functions.h declara 4 argumentos pero IDA usa sólo 1 (DWORD c). Los extra se ignoran.
void __cdecl FUN_00444410(int c_entity, int /*type*/, int /*flag*/, int /*extra*/) {
    DWORD c = (DWORD)c_entity;
    if (c == 0) return;
    short v1 = *(short*)(c + 2);

    if (v1 != 390) {
        // Non-player entities
        switch (v1) {
        case 39:
            FUN_00460dc0(209, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            PlayBuffer(16, c, 0);
            break;
        case 40:
            FUN_0043e820((int)c, 1);
            PlayBuffer(16, c, 0);
            break;
        case 51:
            FUN_00460dc0(1196, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            PlayBuffer(91, 0, 0);
            break;
        case 302: {
            int r = rand() % 8;
            if (r <= 2) {
                FUN_0043e820((int)c, (r <= 0) ? 9 : 8);
            } else {
                FUN_0043e820((int)c, rand() % 2 + 3);
            }
            break;
        }
        default:
            if (*(unsigned char*)(c + 771) % 3) {
                FUN_0043e820((int)c, 4);
            } else {
                FUN_0043e820((int)c, 3);
            }
            ++*(unsigned char*)(c + 771);
            break;
        }
        goto LABEL_85;
    }

    // Player path
    FUN_00443e70();  // SetAttackSpeed
    short v2 = *(short*)(c + 696);  // Helper.Type

    // Helper path (818/819 = pet/fairy ranged)
    if ((v2 != 818 && v2 != 819) || *(unsigned char*)(c + 846)) {
        // No helper → weapon-based
        short v5 = *(short*)(c + 624);  // Helmet/LH? Actually weapon left
        if (v5 != -1 || *(unsigned short*)(c + 648) != 0xFFFF) {
            // Sword (400-495) — ItemAttribute[v5-399].TwoHand at offset -34
            if (v5 >= 400 && v5 < 496) {
                if (*((unsigned char*)&((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[v5 - 399] - 34)) {
                    if (v5 == 431) {
                        FUN_0043e820((int)c, 81);
                    } else {
                        FUN_0043e820((int)c, *(unsigned char*)(c + 771) % 3 + 39);
                    }
                } else {
                    short v6 = *(short*)(c + 648);
                    if (v6 < 400 || v6 >= 496) {
                        FUN_0043e820((int)c, (*(unsigned char*)(c + 771) & 1) + 35);
                    } else {
                        switch (*(unsigned char*)(c + 771) & 3) {
                        case 0: FUN_0043e820((int)c, 35); break;
                        case 1: FUN_0043e820((int)c, 37); break;
                        case 2: FUN_0043e820((int)c, 36); break;
                        case 3: FUN_0043e820((int)c, 38); break;
                        }
                    }
                }
                goto LABEL_85;
            }
            short v7 = *(short*)(c + 648);
            if (v7 >= 400 && v7 < 496) {
                FUN_0043e820((int)c, rand() % 2 + 37);
                goto LABEL_85;
            }
            // Spear (560-591)
            if (v5 >= 560 && v5 < 592) {
                if (*((unsigned char*)&((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[v5 - 399] - 34)) {
                    FUN_0043e820((int)c, rand() % 2 + 84);
                } else {
                    FUN_0043e820((int)c, rand() % 2 + 35);
                }
                goto LABEL_85;
            }
            // Mace 497/498
            if (v5 == 497 || v5 == 498) {
                FUN_0043e820((int)c, 42);
                goto LABEL_85;
            }
            // Mace/staff (496-527)
            if (v5 >= 496 && v5 < 528) {
                FUN_0043e820((int)c, *(unsigned char*)(c + 771) % 3 + 43);
                goto LABEL_85;
            }
            // Bow/crossbow type detection on right hand
            if ((v7 >= 528 && v7 < 535) || v7 == 545) {
                FUN_0043e820((int)c, (*(unsigned short*)(c + 672) == 0xFFFF) ? 46 : 48);
                goto LABEL_85;
            }
            if ((v5 >= 536 && v5 < 543) || (v5 >= 544 && v5 < 545) || v5 == 546) {
                FUN_0043e820((int)c, (*(unsigned short*)(c + 672) == 0xFFFF) ? 47 : 49);
                goto LABEL_85;
            }
        }
        FUN_0043e820((int)c, 34);
        goto LABEL_85;
    }

    // Helper-active path (helper attack animations)
    {
        short v3 = *(short*)(c + 624);
        if (v3 < 496) {
            // fall through to LABEL_11 (right-hand weapon check)
        } else if (v3 < 501) {
            FUN_0043e820((int)c, 52);
            goto LABEL_85;
        } else if (v3 >= 528) {
            // continue to LABEL_11
        } else {
            FUN_0043e820((int)c, 53);
            goto LABEL_85;
        }
        // LABEL_11: weapon checks
        short v4 = *(short*)(c + 648);
        if ((v4 >= 528 && v4 < 535) || v4 == 545) {
            FUN_0043e820((int)c, 54);
        } else if ((v3 >= 536 && v3 < 543) || (v3 >= 544 && v3 < 545) || v3 == 546) {
            FUN_0043e820((int)c, 55);
        } else if (v3 != -1 &&
                   *((unsigned char*)&((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[v3 - 399] - 34)) {
            FUN_0043e820((int)c, 51);
        } else {
            FUN_0043e820((int)c, 50);
        }
    }

LABEL_85:
    if (*(float*)(c + 264) == 0.0f) {
        if (*(short*)(c + 2) == 390) {
            int v13 = *(int*)(c + 4);
            if (v13 < 206 || v13 > 208) {
                short v14 = *(short*)(c + 648);
                if ((v14 >= 528 && v14 < 535) || v14 == 545) {
                    PlayBuffer(65, c, 0);
                } else {
                    short v15 = *(short*)(c + 624);
                    if ((v15 >= 536 && v15 < 543) || (v15 >= 544 && v15 < 545) || v15 == 546) {
                        PlayBuffer(66, c, 0);
                    } else if (v15 == 410 || v15 == 496) {
                        PlayBuffer(42, c, 0);
                    } else if (v15 != -1 || v14 != -1) {
                        PlayBuffer(rand() % 2 + 40, c, 0);
                    }
                }
            } else {
                int v17 = *(int*)(c + 4);
                if (v17 >= 206 && v17 <= 208) {
                    PlayBuffer(rand() % 2 + 40, c, 0);
                }
                short modelType = *(short*)(c + 2);
                int modelBase = (int)(uintptr_t)DAT_05828d58 + 188 * modelType;
                if (*(unsigned short*)(modelBase + 174) != 0xFFFF) {
                    int idx = rand() % 2 + 94 * modelType;
                    PlayBuffer(*(short*)((char*)(uintptr_t)DAT_05828d58 + 2 * idx + 174) + 170, c, 0);
                }
            }
        } else {
            int v17 = *(int*)(c + 4);
            if (v17 >= 206 && v17 <= 208) {
                PlayBuffer(rand() % 2 + 40, c, 0);
            }
            short modelType = *(short*)(c + 2);
            int modelBase = (int)(uintptr_t)DAT_05828d58 + 188 * modelType;
            if (*(unsigned short*)(modelBase + 174) != 0xFFFF) {
                int idx = rand() % 2 + 94 * modelType;
                PlayBuffer(*(short*)((char*)(uintptr_t)DAT_05828d58 + 2 * idx + 174) + 170, c, 0);
            }
        }
    }
    ++*(unsigned char*)(c + 771);
}

// ── FUN_00444a80 — movida desde stubs_game.cpp (refactor B3) ──
// SetPlayerMagic @ 0x00444a80 (38 líneas) — setea la animación de casteo en la entidad del héroe
// Si el tipo de entidad != 0x186: alterna la acción 3/4 según el contador de combo % 3
// If entity type == 0x186 (special): SetAttackSpeed, class-specific action (0x52/0x53 random, 0x56 swim, 0x5b certain classes)
// Declarada en functions.h como FUN_00444a80(int param_1)
void __cdecl FUN_00444a80(int param_1) {
    DWORD c = (DWORD)param_1;
    if (c == 0) return;
    short entityType = *(short*)(c + 2);
    if (entityType != 0x186) {
        int action;
        if ((UINT)*(BYTE*)(c + 0x303) % 3 == 0) {
            action = 3;
        } else {
            action = 4;
        }
        // SetAction(entity, action) — FUN_0043e820
        FUN_0043e820((int)c, action);
        *(char*)(c + 0x303) = *(char*)(c + 0x303) + 1;
        return;
    }
    // Special entity type 0x186: SetAttackSpeed then class-based action
    FUN_00443e70();  // SetAttackSpeed
    short charClass = *(short*)(c + 0x2b8);
    if ((charClass == 0x332 || charClass == 0x333) && *(char*)(c + 0x34e) == '\0') {
        FUN_0043e820((int)c, 0x5b);
        return;
    }
    if ((*(BYTE*)(c + 0x1bc) & 7) == 2) {
        FUN_0043e820((int)c, 0x56);
        return;
    }
    int r = rand();
    UINT v = r & 0x80000001;
    if ((int)v < 0) {
        v = (v - 1 | 0xFFFFFFFE) + 1;
    }
    FUN_0043e820((int)c, (int)(v + 0x52));
}

// ── FUN_00444b60 — movida desde stubs_misc2.cpp (refactor B3) ──
// FUN_00444b60 @ 0x00444B60 — SetPlayerShock(DWORD c, int Hit)
// Reproduce la reacción de "me pegaron" (anim 130 para el jugador, anim 5 para los monstruos) más
// a hit-grunt sound (PlayBuffer). Port FIEL desde IDA decompile (546 bytes).
//
// 2026-05-08: portada como parte de la importación de bugfixes del DLL companion. El parche
// `IgnoreRandomStuck` (Patchs.cpp:291) saltea el *shock aleatorio del 50%* que se tira en
// ReceiveAttackDamage cuando la entidad es el jugador (tipo 390) — ése es un
// arreglo de gameplay, no de esta función. Los llamadores que quieran el comportamiento
// IgnoreRandom deberían gatear con `*(WORD*)(c+2) != 390` antes de llamar.
//
// Entity offsets:
//   +2    short  entity_type        (390 = player)
//   +4    int    sub_class
//   +0x105 byte  current_action     (62/63 = currently dying anim → skip)
//   +0x10 float[3] world position
//   +0x1C float[3] world angle
//   +0x1BC byte   move_type_flags   (& 7 == 2 = swimming)
//   +0x2B8 short  weapon_type       (818/819 = certain mounts that skip shock)
//   +0x2EC byte   alive_flag        (se limpia en el shock del jugador, per IDA)
//   +0x2FD byte   dead_flag         (set ⇒ skip)
// Forward decls (signatures match functions.h / existing impls — return type
// de FUN_0043e820 en algunos headers es `void*`, así que delegamos vía el global
// header rather than re-declaring locally).

void __cdecl FUN_00444b60(int c, int Hit)
{
    if (!c) return;
    if (*(BYTE*)(c + 765)) return;            // already dead
    short v2 = *(short*)(c + 696);
    if (v2 == 818 || v2 == 819) return;      // mount-skip
    BYTE v3 = *(BYTE*)(c + 261);
    if (v3 == 62 || v3 == 63 || Hit <= 0) return; // dying anim or no damage

    short etype = *(short*)(c + 2);
    if (etype == 390) {
        // Player: shock anim, clear alive flag
        (void)FUN_0043e820(c, 130);
        *(BYTE*)(c + 748) = 0;
    } else if (v3 < 3 || v3 > 4) {
        // Monstruos/NPCs (que no estén en las anims de caminar/correr 3-4): anim de shock genérica
        (void)FUN_0043e820(c, 5);
    }

    // Sonido: sólo cuando no está a mitad de una animación (frame == 0)
    if (*(float*)(c + 264) == 0.0f) {
        if (etype == 390) {
            int sub = *(int*)(c + 4);
            if (sub < 206 || sub > 208) {
                if ((*(BYTE*)(c + 444) & 7) == 2) {
                    // Swimming: alternate splash sound 79 or 80
                    int v7 = rand() % 2;
                    FUN_00404bc0(v7 + 79, c, 0);
                    goto label_26;
                }
                int v6 = rand() % 3 + 75;     // generic player hit grunt
                FUN_00404bc0(v6, c, 0);
                goto label_26;
            }
        }
        int v8 = *(int*)(c + 4);
        if (v8 >= 206 && v8 <= 208) {
            FUN_00404bc0(93, c, 0);           // Lorencia skeleton-NPC hit sound
            goto label_26;
        }
        if (etype != 284) {
            // Monster-table sound lookup: Models[etype].Sound[rand()%2]
            DWORD modelsBase = (DWORD)DAT_05828d58;
            short* sndTbl = (short*)(modelsBase + 188 * etype + 174);
            if ((unsigned short)*sndTbl != 0xFFFF) {
                int v6 = sndTbl[rand() % 2] + 170;
                FUN_00404bc0(v6, c, 0);
            }
        }
    }
label_26:
    // Special case: monster type 331 — shower of sparkles around victim
    if (etype == 331) {
        for (int i = 5; i > 0; --i) {
            // 50% chance per spark
            unsigned r = (unsigned)rand() & 0x80000001u;
            bool fire = (r == 0) || (((int)r < 0) && (((char)r - 1) | (int)0xFFFFFFFE) == -1);
            if (fire) {
                int v12 = rand() % 128;
                float pos[3];
                pos[0] = (float)(v12 - 64) + *(float*)(c + 16);
                pos[1] = *(float*)(c + 20);
                pos[2] = (float)(rand() % 50) + *(float*)(c + 24) + 200.0f;
                FUN_00475220(1221, pos, (float*)(c + 28), (float*)(c + 232), 0, 1.0f, 0);
            }
        }
        FUN_00404bc0(105, 0, 0);
    }
}

// ── FUN_0045ac80 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_0045ac80 @ 0x0045AC80 — Entity_GetIndex: search entity array by network ID
// Devuelve el índice de slot 0-399, o 400 si no lo encontró.
int __cdecl FUN_0045ac80(int param_1)
{
    char *pcVar2 = (char*)(uintptr_t)DAT_07abf5d0;
    for (int iVar1 = 0; iVar1 <= 399; iVar1++, pcVar2 += 0x394) {
        if (*pcVar2 != '\0' && *(short*)(pcVar2 + 0x1dc) == (short)param_1)
            return iVar1;
    }
    return 400;
}

// ── FUN_0045c8c0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// ChangeCharacterExt @ 0x0045C8C0 (1063 bytes) — apply equipment visuals to char-select preview.
// Decodes a 10-byte CharSet[] payload (helmet/armor/pants/gloves/boots type+level packed bits) and
// escribe campos WORD/BYTE repartidos por el slot de entidad en +624..+651 (slots de cabeza/alas) y
// +504..+603 (5 body parts × 24-byte stride: type, lvl, exc-flag).
// Side effects:
//   • DeleteBug(c)       — limpia las mariposas de FX de clase viejas atadas al personaje
//   • CreateBug(...)     — spawn new wing FX (type 816/195/267/819) per Equipment[4]&3
//   • SetCharacterScale  — recompute height/weight after equipment changes
// Guard: sólo corre cuando la clase de la entidad (+2) == 390 (jugador). 0xFF en un slot de equipo = "vacío"
// (writes -1/0 sentinel). Bit-pattern (Equipment[2/3/4]>>4) + ((Equipment[8] >> N) & 1) == 31
// indica un set "antiguo/especial", que usa entity+444 (el nibble de skin de clase) como ID de sprite.
// Ported verbatim from IDA reference 0045C8C0_ChangeCharacterExt.c.
void __cdecl FUN_0045c8c0(int Key, BYTE *Equipment) {
    // DAT_07abf5d0 guarda la dirección base del array de entidades como entero.
    // 916 (0x394) = entity stride. Slot at DAT_07abf5d0 + 916*Key.
    DWORD c = DAT_07abf5d0 + 916 * Key;
    if (*(WORD*)(c + 2) != 390) return;

    // Helmet (slot 0)
    if (Equipment[0] == 0xFF) {
        *(WORD*)(c + 624) = (WORD)-1;
        *(BYTE*)(c + 627) = 0;
    } else {
        *(WORD*)(c + 624) = (WORD)(Equipment[0] + 400);
    }
    // Armor (slot 1)
    BYTE Type = Equipment[1];
    if (Type == 0xFF) {
        *(WORD*)(c + 648) = (WORD)-1;
        *(BYTE*)(c + 651) = 0;
    } else {
        *(WORD*)(c + 648) = (WORD)(Type + 400);
    }
    // Right-hand weapon (slot at +672 / 0x2A0)
    if (((Equipment[4] >> 2) & 3) == 3) {
        if ((Equipment[8] & 7) != 0) {
            *(WORD*)(c + 672) = (WORD)((Equipment[8] & 7) + 786);
        } else {
            *(WORD*)(c + 0x2A0) = (WORD)-1;
            *(BYTE*)(c + 0x2A3) = 0;
        }
    } else {
        *(WORD*)(c + 672) = (WORD)(((Equipment[4] >> 2) & 3) + 784);
    }

    // Wings: clear class-FX, then spawn type-specific
    FUN_004fffa0(c);  // DeleteBug
    Type = Equipment[4] & 3;
    if (Type == 3) {
        if ((Equipment[9] & 1) == 1) {
            *(WORD*)(c + 696) = 819;
            float* pos = (float*)(c + 16);
            FUN_004fffd0(267, (void*)pos, (void*)(uintptr_t)c, 0);
        } else {
            *(WORD*)(c + 696) = (WORD)-1;
            *(BYTE*)(c + 699) = 0;
        }
    } else {
        *(WORD*)(c + 696) = (WORD)(Type + 816);
        float* pos = (float*)(c + 16);
        if (Type == 0) {
            FUN_004fffd0(816, (void*)pos, (void*)(uintptr_t)c, 0);
        } else if (Type == 2) {
            FUN_004fffd0(195, (void*)pos, (void*)(uintptr_t)c, 0);
        } else if (Type == 3) {
            // inalcanzable — ya se atrapó arriba; se deja por paridad con IDA
            FUN_004fffd0(267, (void*)pos, (void*)(uintptr_t)c, 0);
        }
    }

    // 19-bit packed level field (Equipment[5..7] bigendian)
    int Level = Equipment[7] + ((Equipment[6] + (Equipment[5] << 8)) << 8);
    *(BYTE*)(c + 626) = (BYTE)LevelConvert(Equipment[7] & 7);          // weapon[0] exc lvl
    *(BYTE*)(c + 650) = (BYTE)LevelConvert((Level >> 3) & 7);          // weapon[1] exc lvl
    char Change = *(char*)(c + 847);  // class change flag
    *(BYTE*)(c + 674) = 0;
    *(BYTE*)(c + 698) = 0;
    if (Change) return;  // class-changed chars get default body parts; skip detailed slots

    // 5 body-part slots at c+504, +528, +552, +576, +600 (stride 24)
    {
        short v7 = Equipment[2] >> 4;
        if ((BYTE)v7 + 16 * (Equipment[8] >> 7) == 31) {
            BYTE v8 = *(BYTE*)(c + 444);
            v7 = v8 >> 3;
            *(WORD*)(c + 504) = (WORD)((v8 & 7) + 4 * (BYTE)v7 + 912);
        } else {
            v7 = (BYTE)v7 + 16 * (Equipment[8] >> 7);
            *(WORD*)(c + 504) = (WORD)((BYTE)v7 + 624);
        }
    }
    {
        short v9 = Equipment[2] & 0xF;
        if ((BYTE)v9 + 16 * ((Equipment[8] & 0x40) != 0) == 31) {
            BYTE b = *(BYTE*)(c + 444);
            *(WORD*)(c + 528) = (WORD)((b & 7) + 4 * (b >> 3) + 919);
        } else {
            v9 = (BYTE)v9 + 16 * ((Equipment[8] & 0x40) != 0);
            *(WORD*)(c + 528) = (WORD)((BYTE)v9 + 656);
        }
    }
    {
        char v10 = Equipment[3] >> 4;
        if (v10 + 16 * ((Equipment[8] & 0x20) != 0) == 31) {
            BYTE b = *(BYTE*)(c + 444);
            *(WORD*)(c + 552) = (WORD)((b & 7) + 4 * (b >> 3) + 926);
        } else {
            *(WORD*)(c + 552) = (WORD)((BYTE)(v10 + 16 * ((Equipment[8] & 0x20) != 0)) + 688);
        }
    }
    {
        char v11 = Equipment[3] & 0xF;
        if (v11 + 16 * ((Equipment[8] & 0x10) != 0) == 31) {
            BYTE b = *(BYTE*)(c + 444);
            *(WORD*)(c + 576) = (WORD)((b & 7) + 4 * (b >> 3) + 933);
        } else {
            *(WORD*)(c + 576) = (WORD)((BYTE)(v11 + 16 * ((Equipment[8] & 0x10) != 0)) + 720);
        }
    }
    {
        char v12 = Equipment[4] >> 4;
        if (v12 + 16 * ((Equipment[8] & 8) != 0) == 31) {
            BYTE b = *(BYTE*)(c + 444);
            *(WORD*)(c + 600) = (WORD)((b & 7) + 4 * (b >> 3) + 940);
        } else {
            *(WORD*)(c + 600) = (WORD)((BYTE)(v12 + 16 * ((Equipment[8] & 8) != 0)) + 752);
        }
    }
    *(BYTE*)(c + 506) = (BYTE)LevelConvert((Level >> 6) & 7);
    *(BYTE*)(c + 530) = (BYTE)LevelConvert((Level >> 9) & 7);
    *(BYTE*)(c + 554) = (BYTE)LevelConvert((Level >> 12) & 7);
    *(BYTE*)(c + 578) = (BYTE)LevelConvert((Level >> 15) & 7);
    *(BYTE*)(c + 602) = (BYTE)LevelConvert((Level >> 18) & 7);
    *(BYTE*)(c + 507) = Equipment[9] >> 7;
    *(BYTE*)(c + 531) = (Equipment[9] & 0x40) != 0;
    *(BYTE*)(c + 555) = (Equipment[9] & 0x20) != 0;
    *(BYTE*)(c + 579) = (Equipment[9] & 0x10) != 0;
    *(BYTE*)(c + 603) = (Equipment[9] & 8) != 0;
    *(BYTE*)(c + 627) = (Equipment[9] & 4) != 0;
    *(BYTE*)(c + 651) = (Equipment[9] & 2) != 0;
    SetCharacterScale((int)c);
}

// ── FUN_0046c7f0 — movida desde stubs_game.cpp (refactor B3) ──
// FUN_0046c7f0 @ 0x0046C7F0 (~176 lines) — directional hit particles with blood
// AngleMatrix + VectorRotate para la dirección del impacto. Offset aleatorio por eje.
// param_1: 0=blood (red/green, type 0x4AB + AddTerrainLight), 1=hit spark (type 0x4C4), 2=hit spark variant
// param_2: puntero base de la entidad (posición en +0x10/+0x14/+0x18, ángulos en +0x1C, luz en +0xE8)
// param_3/4/5: direction angles for AngleMatrix
// FUN_0046c7f0 @ 0x0046C7F0 — Object_SpawnAmbientFX(kind, o, dx, dy, dz)
// Port FIEL de IDA `sub_46C7F0`. Es el spawner de fuego/humo de los objetos
// del mundo: lo llama `MoveObjects` (0x4FDC00) para los braseros, la forja del
// herrero, las chimeneas y los faroles.
//
//   kind 0 → llama de fuego: partícula 1195 (subtipo aleatorio 0..3, 50% de
//            las veces) + `AddTerrainLight` con tinte naranja (radio 4).
//   kind 1 → HUMO: partícula 1220, subtipo 0, 50% de las veces.
//   kind 2 → HUMO: partícula 1220, subtipo 2, 50% de las veces.
//
// (dx,dy,dz) es un desplazamiento LOCAL que se rota por los ángulos del objeto
// (`o+28`) — así el efecto sale del punto correcto del modelo (la boca de la
// chimenea, la punta del farol). Luego se le suma la posición del objeto
// (`o+16`) y un jitter de ±8 por eje.
//
// 2026-08-11: la versión anterior era una reinterpretación como "blood/spark"
// que (a) construía la matriz con (dx,dy,dz) COMO SI FUERAN ÁNGULOS y rotaba
// un offset (0,0,0) — o sea el desplazamiento se perdía y todo salía en el
// origen del objeto — y (b) usaba tipos de partícula 0x4AB/0x4C4 en lugar de
// 1195/1220. Resultado: ningún humo en el mundo.
void __cdecl FUN_0046c7f0(int param_1, int param_2, float param_3, float param_4, float param_5) {
    float *a2 = (float *)param_2;
    if (!a2) return;

    float *v5 = a2 + 7;          // o+28  = Angle[3]
    float in2[3][4];
    AngleMatrix(v5, in2);

    float in1[3] = { param_3, param_4, param_5 };
    float out[3];
    FUN_004fa0b0(in1, (float *)in2, out);      // VectorRotate

    out[0] += a2[4];             // o+16 = Position[0]
    out[1] += a2[5];
    out[2] += a2[6];
    out[0] += (float)(rand() % 16 - 8);
    out[1] += (float)(rand() % 16 - 8);
    out[2] += (float)(rand() % 16 - 8);

    if (param_1 == 0) {
        float v10 = (float)(rand() % 6 + 6) * 0.1f;
        float Light[3] = { v10, v10 * 0.60000002f, v10 * 0.40000001f };
        if ((rand() % 2) == 0)
            FUN_00475220(1195, out, v5, Light, rand() % 4, 1.0f, 0);
        AddTerrainLight(out[0], out[1], Light, 4, PrimaryTerrainLight[0]);
    } else if (param_1 == 1) {
        if ((rand() % 2) == 0)
            FUN_00475220(1220, out, v5, a2 + 58, 0, 1.0f, 0);   // o+232 = Light
    } else if (param_1 == 2) {
        if ((rand() % 2) == 0)
            FUN_00475220(1220, out, v5, a2 + 58, 2, 1.0f, 0);
    }
}

// ── FUN_00479790 — movida desde stubs_misc2.cpp (refactor B3) ──
// FUN_00479790 @ 0x00479790 — Effect_UpdateAll
// Marca todos los slots activos del pool de personajes/efectos (DAT_07c85890, stride 0x1BC)
// para el tick de render, poniendo el byte [+0x160] = 1 en cada entrada activa (flag de activo distinto de cero).
// Se llama una vez por frame antes del loop de render, para que cada slot se procese exactamente una vez.
void FUN_00479790(void)
{
    // Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba el dirty-mark.
    char *pcVar1 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar1 += 0x1bc) {
        if (*pcVar1 != '\0')
            pcVar1[0x160] = '\x01';
    }
}

// ── FUN_00479910 — movida desde stubs_misc2.cpp (refactor B3) ──
// ── Item data helper stubs ────────────────────────────────────────────────────
// FUN_00479910 @ 0x00479910 — XOR-cipher buffer in-place (3-byte key: FC CF AB)
void __cdecl FUN_00479910(int buf, int len) {
    for (int i = 0; i < len; i++)
        *(BYTE *)(buf + i) ^= (BYTE)DAT_00559bb4[i % 3];
}

// ── FUN_0047dae0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47DAE0 @ 0x0047DAE0 (618 bytes) — Stats_CalcMagicDmgRange.
// Calcula el daño mágico mínimo/máximo en this[+70]/this[+72]:
//   min = Energy/9
//   max = Energy>>2 (= Energy/4)
//   + el special 61 de las Alas (this+1012) (escalado por durabilidad)
//   + el special 60 (o 61 para wand=31) de WeaponL (this+536) — escalado por durabilidad
//   + special 75 (flat add) on WeaponL
//   + special 76 (percent +2%) on WeaponL
//   + el special 75 + el special 76 (2%) del Ring1 (this+1148)
int __cdecl FUN_0047dae0(int param_1) {
    unsigned short energy = *(unsigned short*)(param_1 + 26);
    unsigned short *minPtr = (unsigned short*)(param_1 + 70);
    unsigned short *maxPtr = (unsigned short*)(param_1 + 72);
    *minPtr = energy / 9;
    *maxPtr = energy >> 2;

    DWORD itemRing1 = (DWORD)(uintptr_t)(param_1 + 1148);
    DWORD weaponL  = (DWORD)(uintptr_t)(param_1 + 536);

    // Wings (this+1012) magic damage bonus, durability scaled
    short wingsType = *(short*)(param_1 + 1012);
    if (wingsType != -1) {
        ITEM_ATTRIBUTE* attr = &((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[wingsType];
        float durP = CalcDurabilityPercent(*(unsigned char*)(param_1 + 1038),
                                           attr->Durability,
                                           *(int*)(param_1 + 1016), 0);
        unsigned short minBonus = 0, maxBonus = 0;
        PlusSpecial(&minBonus, 61, (DWORD)(uintptr_t)(param_1 + 1012));
        PlusSpecial(&maxBonus, 61, (DWORD)(uintptr_t)(param_1 + 1012));
        unsigned short minRed = (unsigned short)((double)minBonus * (double)durP);
        unsigned short maxRed = (unsigned short)((double)maxBonus * (double)durP);
        *minPtr += minBonus - minRed;
        *maxPtr += maxBonus - maxRed;
    }

    // WeaponL (this+536) magic damage bonus
    if (*(unsigned short*)weaponL != 0xFFFF) {
        unsigned char dur = *(unsigned char*)(weaponL + 26);
        if (dur != 0) {
            short type = *(short*)weaponL;
            ITEM_ATTRIBUTE* attr = &((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[type];
            float durP = CalcDurabilityPercent(dur, attr->Durability,
                                               *(int*)(weaponL + 4),
                                               *(unsigned char*)(weaponL + 27));
            int specId = (type == 31) ? 60 : 61;  // Wand uses special 60
            unsigned short minBonus = 0, maxBonus = 0;
            PlusSpecial(&minBonus, specId, weaponL);
            PlusSpecial(&maxBonus, specId, weaponL);
            *minPtr += (unsigned short)(minBonus - (unsigned short)((double)minBonus * (double)durP));
            *maxPtr += (unsigned short)(maxBonus - (unsigned short)((double)maxBonus * (double)durP));
            PlusSpecial(minPtr, 75, weaponL);
            PlusSpecial(maxPtr, 75, weaponL);
            FUN_0047cf40((short*)minPtr, 76, (int)weaponL, 2);
            FUN_0047cf40((short*)maxPtr, 76, (int)weaponL, 2);
        }
    }

    // Ring1 (this+1148) magic damage bonus
    if (*(unsigned short*)itemRing1 != 0xFFFF) {
        if (*(unsigned char*)(itemRing1 + 26) != 0) {
            PlusSpecial(minPtr, 75, itemRing1);
            PlusSpecial(maxPtr, 75, itemRing1);
            FUN_0047cf40((short*)minPtr, 76, (int)itemRing1, 2);
            return FUN_0047cf40((short*)maxPtr, 76, (int)itemRing1, 2);
        }
    }
    return (int)itemRing1;
}

// ── FUN_0047dd50 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47DD50 @ 0x0047DD50 (39 bytes) — Stats_CalcAddStrength (or similar).
// Computes a derived stat from CharacterMachine fields:
//   this[29] = 5*this[7] + (this[10] >> 2) + (this[11] * 3) / 2
// donde this es un puntero WORD; los offsets +14, +20, +22 y +58 van en bytes.
int __cdecl FUN_0047dd50(short *param_1) {
    int v = 5 * (unsigned short)param_1[7]
          + ((unsigned short)param_1[10] >> 2)
          + ((unsigned short)param_1[11] * 3) / 2;
    param_1[29] = (short)v;
    return v;
}

// ── FUN_0047dd80 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// CHARACTER_MACHINE::CalculateAttackSpeed @ 0x0047DD80 (598 bytes).
// Calcula la velocidad de ataque/magia en this[+0x38] / this[+0x44] (o +56/+68
// for non-class-1/2/3 = elf/etc).
//   Formulas based on (CharAttribute[+11] & 7) class:
//     class 2 (Wizard): atk = Dex/50, mag = Dex/50
//     class 1/3 (DK/MG): atk = Dex/15, mag = Dex/20
//     el resto: atk = (this+22)/20, mag = (this+22)/10
//   Después los bonus de velocidad de arma de WeaponL/R (slots 0/1).
//   Pants (slot 5 = this+876) speed bonus.
//   Flag de estado (this+40 bit 0): +20 a los dos.
//   PlusSpecial(77) on WeaponL/R, Ring1, Helmet/byte 1080.
int __cdecl FUN_0047dd80(int param_1) {
    DWORD ca = (DWORD)DAT_07cf1ff4;
    if (ca == 0) return 0;
    char* charAttr = (char*)(uintptr_t)ca;
    int charClass = *(unsigned char*)(charAttr + 11) & 7;

    unsigned short *atkPtr;
    unsigned short *magPtr;
    if (charClass == 2) {
        atkPtr = (unsigned short*)(param_1 + 0x38);
        magPtr = (unsigned short*)(param_1 + 0x44);
        unsigned short dex = *(unsigned short*)(param_1 + 0x16);
        *atkPtr = dex / 50;
        *magPtr = dex / 50;
    } else if (charClass == 1 || charClass == 3) {
        atkPtr = (unsigned short*)(param_1 + 0x38);
        magPtr = (unsigned short*)(param_1 + 0x44);
        unsigned short dex = *(unsigned short*)(param_1 + 0x16);
        *atkPtr = dex / 15;
        *magPtr = dex / 20;
    } else {
        atkPtr = (unsigned short*)(param_1 + 56);
        magPtr = (unsigned short*)(param_1 + 68);
        unsigned short v = *(unsigned short*)(param_1 + 22);
        *atkPtr = v / 20;
        *magPtr = v / 10;
    }

    DWORD weaponR = (DWORD)(uintptr_t)(param_1 + 604);  // WeaponR
    DWORD weaponL = (DWORD)(uintptr_t)(param_1 + 536);  // WeaponL
    DWORD pants   = (DWORD)(uintptr_t)(param_1 + 876);  // Pants

    // Skip dword_7E91388/pPickedItem hover preview for non-runtime simplicity.

    bool weaponLValid = false, weaponRValid = false, pantsValid = false;
    if (*(unsigned char*)(weaponL + 26)) {
        short t = *(short*)weaponL;
        if (t != 135 && t != 143 && t >= 0 && t < 192) weaponLValid = true;
    }
    if (*(unsigned char*)(weaponR + 26)) {
        short t = *(short*)weaponR;
        if (t != 135 && t != 143 && t >= 0 && t < 192) weaponRValid = true;
    }
    if (*(unsigned char*)(pants + 26) && *(unsigned short*)pants != 0xFFFF) {
        pantsValid = true;
    }

    int magBonus = 0;
    if (weaponLValid) {
        if (weaponRValid) {
            *atkPtr += (*(unsigned char*)(weaponL + 22) + *(unsigned char*)(weaponR + 22)) / 2;
            magBonus = (*(unsigned char*)(weaponL + 22) + *(unsigned char*)(weaponR + 22)) / 2;
        } else {
            *atkPtr += *(unsigned char*)(weaponL + 22);
            magBonus = *(unsigned char*)(weaponL + 22);
        }
        *magPtr += (unsigned short)magBonus;
    } else if (weaponRValid) {
        *atkPtr += *(unsigned char*)(weaponR + 22);
        magBonus = *(unsigned char*)(weaponR + 22);
        *magPtr += (unsigned short)magBonus;
    }

    if (pantsValid) {
        *atkPtr += *(unsigned char*)(pants + 22);
        *magPtr += *(unsigned char*)(pants + 22);
    }

    if ((*(unsigned char*)(param_1 + 40) & 1) != 0) {
        *atkPtr += 20;
        *magPtr += 20;
    }

    PlusSpecial(atkPtr, 77, (DWORD)(uintptr_t)(param_1 + 536));
    PlusSpecial(magPtr, 77, (DWORD)(uintptr_t)(param_1 + 536));
    PlusSpecial(atkPtr, 77, (DWORD)(uintptr_t)(param_1 + 604));
    PlusSpecial(magPtr, 77, (DWORD)(uintptr_t)(param_1 + 604));
    PlusSpecial(atkPtr, 77, (DWORD)(uintptr_t)(param_1 + 1148));
    PlusSpecial(magPtr, 77, (DWORD)(uintptr_t)(param_1 + 1148));
    DWORD helm = (DWORD)(uintptr_t)(param_1 + 1080);
    PlusSpecial(atkPtr, 77, helm);
    PlusSpecial(magPtr, 77, helm);
    return 0;
}

// ── FUN_0047dfe0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47DFE0 @ 0x0047DFE0 (369 bytes) — Stats_CalcDefense.
// Calcula el stat de defensa en this[+76]:
//   class==2 (Wizard): defense = Vit/4 (this[+22]>>2)
//   else: defense = Vit/3
//   + el Escudo (this+604), con la defensa escalada por durabilidad
//   + los bonus porcentuales de cada slot de equipo (special 70)
int __cdecl FUN_0047dfe0(int param_1) {
    DWORD ca = (DWORD)DAT_07cf1ff4;
    if (ca == 0) return 0;
    char* charAttr = (char*)(uintptr_t)ca;
    int charClass = *(unsigned char*)(charAttr + 11) & 7;

    unsigned short *defPtr = (unsigned short*)(param_1 + 76);
    if (charClass == 2) {
        *defPtr = (unsigned short)(*(unsigned short*)(param_1 + 22) >> 2);
    } else {
        *defPtr = (unsigned short)(*(unsigned short*)(param_1 + 22) / 3);
    }

    short v3 = *(short*)(param_1 + 604);  // WeaponR/Shield item type
    if (v3 != -1) {
        unsigned char dur = *(unsigned char*)(param_1 + 630);  // shield durability
        if (dur != 0) {
            unsigned char maxDur = ((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[v3].Durability;
            int level = *(int*)(param_1 + 608);
            unsigned char opt = *(unsigned char*)(param_1 + 631);
            float durP = CalcDurabilityPercent(dur, maxDur, level, opt);
            unsigned char defenseBase = *(unsigned char*)(param_1 + 620);
            unsigned short reduction = (unsigned short)((double)defenseBase * (double)durP);
            *defPtr += defenseBase - reduction;

            // Shield "Plus Defense" special (62)
            unsigned short Value = 0;
            PlusSpecial(&Value, 62, (DWORD)(uintptr_t)(param_1 + 604));
            *defPtr += (unsigned short)(Value - (unsigned short)((double)Value * (double)durP));

            FUN_0047cf40((short*)defPtr, 70, param_1 + 604, 10);
        }
    }
    // Apply percent defense bonuses from all equipment slots
    FUN_0047cf40((short*)defPtr, 70, param_1 + 672,  10);  // Pendant
    FUN_0047cf40((short*)defPtr, 70, param_1 + 740,  10);  // Armor
    FUN_0047cf40((short*)defPtr, 70, param_1 + 808,  10);  // Pants
    FUN_0047cf40((short*)defPtr, 70, param_1 + 876,  10);  // Gloves
    FUN_0047cf40((short*)defPtr, 70, param_1 + 944,  10);  // Boots
    FUN_0047cf40((short*)defPtr, 70, param_1 + 1284, 10);  // Necklace
    return FUN_0047cf40((short*)defPtr, 70, param_1 + 1216, 10);  // Ring2
}

// ── FUN_0047e160 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47E160 @ 0x0047E160 (383 bytes) — Stats_CalcCritBase / DefRate.
// Calcula el stat de tasa de defensa en this[+78]:
//   class 0 (Knight): Vit/4   (>>2)
//   class 1 (DK?):    Vit/3
//   class 2 (Wiz):    Vit/10
//   class 3+ (DL?):   Vit/5
// Then iterates 7 equipment slots checking durability-scaled rate bonus.
// Más un +5% / +10% si g_bAddDefense y EquipmentLevelSet == 10/11.
int __cdecl FUN_0047e160(int param_1) {
    DWORD ca = (DWORD)DAT_07cf1ff4;
    if (ca == 0) return 0;
    char* charAttr = (char*)(uintptr_t)ca;
    int charClass = *(unsigned char*)(charAttr + 11) & 7;
    unsigned short vit = *(unsigned short*)(param_1 + 22);

    int v4;
    if (charClass == 2)       v4 = vit / 10;
    else if (charClass == 1)  v4 = vit / 3;
    else if (charClass != 0)  v4 = vit / 5;
    else                       v4 = vit >> 2;
    *(short*)(param_1 + 78) = (short)v4;

    int totalReduction = 0;
    for (int i = 1; i <= 7; ++i) {
        int slotPtr = param_1 + 68 * i;
        if (*(unsigned char*)(slotPtr + 562)) {  // slot's [+26] = 0x22A; 8*68+0x22 hmm let me recalc
            unsigned int v7 = FUN_0047cfb0((DWORD)slotPtr, (short*)(slotPtr + 536));
            ITEM_ATTRIBUTE* v8 = &((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[*(short*)(slotPtr + 536)];
            float durP;
            if (i == 7) {
                durP = CalcDurabilityPercent(*(unsigned char*)(param_1 + 1038),
                                             v8->Durability,
                                             *(int*)(param_1 + 1016), 0);
            } else {
                durP = CalcDurabilityPercent(*(unsigned char*)(slotPtr + 562),
                                             v8->Durability,
                                             *(int*)(slotPtr + 540),
                                             *(unsigned char*)(slotPtr + 563));
            }
            int reduction = (int)((double)(unsigned short)v7 * (double)durP);
            totalReduction += (int)v7 - reduction;
        }
    }
    *(short*)(param_1 + 78) += (short)totalReduction;

    // Bonus de set completo (IDA L68-83). g_bAddDefense la escribe CheckFullSet
    // (0x451B20); EquipmentLevelSet es el MIN level de las 5 piezas del set.
    short defBefore = *(short*)(param_1 + 78);   // DEFRATE probe
    if (g_bAddDefense) {
        double v11 = 0.0;
        if (EquipmentLevelSet == 10)      v11 = 0.050000001;
        else if (EquipmentLevelSet == 11) v11 = 0.1;
        *(short*)(param_1 + 78) = (short)(*(short*)(param_1 + 78)
                                          + (int)((double)*(unsigned short*)(param_1 + 78) * v11));
    }

    // ── DEFRATE (temporal): verificación del bonus de set completo del +10/+11.
    // Loguea SÓLO cuando cambia algo, así se ve el antes/después al sacarse una
    // pieza del set. Esperado con set completo +11: bonus=+10%; al sacar una
    // pieza, EquipmentLevelSet cae a 0 y el bonus desaparece.
    {
        static int  s_add = -1, s_els = -1;
        static short s_before = -1, s_after = -1;
        short defAfter = *(short*)(param_1 + 78);
        if (g_bAddDefense != s_add || EquipmentLevelSet != s_els ||
            defBefore != s_before || defAfter != s_after) {
            s_add = g_bAddDefense; s_els = EquipmentLevelSet;
            s_before = defBefore;  s_after = defAfter;
            char db[160];
            _snprintf_s(db, sizeof(db), _TRUNCATE,
                "DEFRATE addDef=%d EquipLevelSet=%d defRate %d -> %d (bonus %+d)",
                g_bAddDefense, EquipmentLevelSet, defBefore, defAfter,
                (int)defAfter - (int)defBefore);
            DbgLogPublic(db);
        }
    }
    return *(short*)(param_1 + 78);
}

// ── FUN_0047e2e0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47E2E0 @ 0x0047E2E0 (46 bytes) — Stats_CalcExtraOption1.
// Iterates 6 equipment slots (CharacterMachine + 672 .. + 1012, stride 68
// bytes = tamaño de ITEM). Por cada slot con Option1 (byte +26) seteado, llama a
// FUN_0047cfe0 (accesor de item +0x14). Guarda el último resultado válido en
// CharacterMachine + 80 (this[40] si es puntero WORD).
int __cdecl FUN_0047e2e0(short *param_1) {
    short *slot = param_1 + 336;  // CharacterMachine + 672 (Pendant)
    short last = 0;
    for (int n = 0; n < 6; ++n) {
        if (*((unsigned char*)slot + 26)) {
            last = FUN_0047cfe0(slot);
            param_1[40] = last;  // CharacterMachine + 80
        }
        slot += 34;  // +68 bytes (next ITEM)
    }
    return (int)(unsigned char)last;
}

// ── FUN_0047e310 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// sub_47E310 @ 0x0047E310 (63 bytes) — Stats_CalcExtraOption2.
// Lee el Option1 (byte +26 dentro de ITEM) de los slots de Guantes y Alas. Por cada uno:
//   Guantes (Option1 en +970, ITEM en +944): this+82 = FUN_0047d000(guantes)
//   Alas    (Option1 en +1038, ITEM en +1012): this+82 += FUN_0047d000(alas)
// Orden: primero los guantes (reemplaza), después las alas (suma).
int __cdecl FUN_0047e310(int param_1) {
    short v = 0;
    if (*(unsigned char*)(param_1 + 970)) {
        *(short*)(param_1 + 82) = FUN_0047d000((short*)(param_1 + 944));
    }
    if (*(unsigned char*)(param_1 + 1038)) {
        v = FUN_0047d000((short*)(param_1 + 1012));
        *(short*)(param_1 + 82) += v;
    }
    return (int)(unsigned char)v;
}

// ── FUN_004f9c70 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_004f9c70 @ 0x004F9C70 — Terrain_QuadEqual(ptr1, ptr2_int) → compares 4 floats within epsilon.
// Firma de Ghidra: (int param_1, float *param_2) pero en functions.h está declarada como (int,int,int,int).
// Callers pass two int args (the float ptr pair). Match declared signature.
int __cdecl FUN_004f9c70(int param_1, int param_2, int param_3, int param_4) {
    // Sólo se usan los dos primeros parámetros: ptr1 = param_1, ptr2 = param_2
    float *p2 = (float*)param_2;
    int iVar1 = 0;
    int iVar2 = param_1 - param_2;
    do {
        float diff = *(float*)(iVar2 + (int)p2) - *p2;
        if (diff < 0.0f) diff = -diff;
        if (_DAT_005528a8 < diff)
            return 0;
        iVar1++;
        p2++;
    } while (iVar1 < 4);
    (void)param_3; (void)param_4;
    return 1;
}

// ── FUN_004f9ce0 — movida desde stubs_helpers.cpp (refactor B3) ──
// FUN_004f9ce0 @ 0x004F9CE0 — VectorMA(va, scale, vb, vc): vc = va + scale*vb
// IDA-ported: 3-vector multiply-add (Quake-style). Was stub copying in_rel.
void __cdecl FUN_004f9ce0(float *va, float scale, float *vb, float *vc) {
    vc[0] = scale * vb[0] + va[0];
    vc[1] = scale * vb[1] + va[1];
    vc[2] = scale * vb[2] + va[2];
}

// ── FUN_004f9d60 — movida desde stubs_helpers.cpp (refactor B3) ──
// FUN_004f9d60 @ 0x004F9D60 — Vec3_Normalize(vec[3])
// Normalises a 3-component float vector in-place.
void __cdecl FUN_004f9d60(float *vec) {
    float len = sqrtf(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
    if (len > 0.0f) { vec[0] /= len; vec[1] /= len; vec[2] /= len; }
}

// ── FUN_00513260 — movida desde stubs_mouse_hover.cpp (refactor B3) ──
// FUN_00513260 @ 0x00513260 — Entity_ViewportCheck(viewport, projection)
// Testea si la entidad descrita por 12 dwords (que el llamador copió de entity+0x130) está dentro
// del viewport actual, usando los punteros de matriz dados. Devuelve 1 si es visible, 0 si se descarta.
unsigned int __cdecl FUN_00513260(float *viewport, float *projection) { return 1; } // STUB: frustum cull
