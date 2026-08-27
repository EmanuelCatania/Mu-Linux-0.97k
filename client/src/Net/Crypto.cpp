// Crypto.cpp
// Packet encryption / decryption routines and hash table operations.
//
// FUN_00404330 @ 0x00404330 — Packet_DecryptByte
// FUN_00423710 @ 0x00423710 — Packet_EncryptByte
// FUN_00423760 @ 0x00423760 — Packet_EncryptDword
// FUN_00423c40 @ 0x00423c40 — Buffer_XorKey3
// FUN_00404400 @ 0x00404400 — Packet_EncryptBuffer (0x584 bytes)
// FUN_00409e20 @ 0x00409e20 — Packet_DecryptDword
// FUN_00403f80 @ 0x00403f80 — HashTable_Insert
// FUN_00404280 @ 0x00404280 — HashTable_GetValue
// FUN_0043dcc0 @ 0x0043dcc0 — Object_GetType (accessor: return *(param+8))
//
// Key global:
//   DAT_00559050 — 16-byte XOR key table (indices 0..15)

#include "stdafx.h"

void __fastcall FUN_00401af0(void*);
void  __fastcall FUN_00407950(void *node);
int   __cdecl    FUN_00407b90(int a1, int a3, const float *range);
void  __cdecl    FUN_00407c60(int a1, int a3, float rest);
char  __fastcall FUN_00407d10(int a1);
DWORD* __cdecl   FUN_00407e50(DWORD *node);
void  __fastcall FUN_004088b0(void *sys, int idx, short na, short nb, float rest_scaled, float dist, BYTE flags);
extern void __cdecl FUN_0054158c(void* ptr);
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
// -- Declaraciones de funciones que viven en otros modulos --------------
// Agregadas por el refactor B3: se declaraban localmente en el archivo del
// que se movieron estas funciones. Migrar a functions.h mas adelante.
void __fastcall FUN_00401960(int param_1);
void __fastcall FUN_004079b0(void *node, float x, float y, float z, int pinned);
void __fastcall FUN_00407b30(void *node, float *out);
float __fastcall FUN_00407b50(void *a, int b, float *c);
void __fastcall FUN_00407ef0(void *node, float p1, float p2, float p3, float radius, int boneIdx);
void __fastcall FUN_0053cbf0(int *param_1);
void __cdecl FUN_0053cc00_impl(int param_1);




// FUN_00404330 — Packet_DecryptByte
// Decrypts a single byte: *param_2 → *param_1
// Decrypt steps: b -= 0x23; b ^= key[b]; b += 0xb9
void __cdecl FUN_00404330(void *param_1_v,void *param_2_v)
{
  byte *param_1 = (byte*)param_1_v;
  byte *param_2 = (byte*)param_2_v;
  byte *pbVar1;
  byte bVar2;

  pbVar1 = (byte*)operator_new(1);
  bVar2 = *param_2;
  *pbVar1 = bVar2;
  bVar2 = bVar2 - 0x23;
  *pbVar1 = bVar2;
  bVar2 = (bVar2 ^ PacketXorKey16[0]) + 0xb9;
  *pbVar1 = bVar2;
  *param_1 = bVar2;
  operator_delete(pbVar1);
  return;
}


// FUN_00423710 — Packet_EncryptByte
// Encrypts a single byte (param_2) into param_1.
// Encrypt steps: b += 0x47; b ^= key[b]; randomize *param_2
void __cdecl FUN_00423710(void *param_1_v,void *param_2_v)
{
  byte *param_1 = (byte*)param_1_v;
  char *param_2 = (char*)param_2_v;
  char cVar1;
  byte *pbVar2;
  int iVar3;

  pbVar2 = (byte*)operator_new(1);
  cVar1 = *param_2;
  *pbVar2 = cVar1 + 0x47U;
  *pbVar2 = (cVar1 + 0x47U ^ PacketXorKey16[0]) + 0x23;
  iVar3 = _rand();
  *param_2 = (char)iVar3;
  *param_1 = *pbVar2;
  operator_delete(pbVar2);
  return;
}


// FUN_00423760 — Packet_EncryptDword
// Encrypts a 4-byte block (param_2) into param_1.
// Cada byte: b += 0x47; b ^= key[i%16]; chain-XOR con el byte siguiente;
// randomize source bytes with _rand().
void __cdecl FUN_00423760(void *param_1_v,void *param_2_v)
{
  undefined4 *param_1 = (undefined4*)param_1_v;
  undefined4 *param_2 = (undefined4*)param_2_v;
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  byte *pbVar5;
  uint uVar6;

  pbVar2 = (byte*)operator_new(4);
  uVar6 = 0;
  *(undefined4 *)pbVar2 = *param_2;
  pbVar5 = pbVar2;
  do {
    bVar1 = *pbVar5;
    uVar4 = uVar6 & 0x8000000f;
    *pbVar5 = bVar1 + 0x47;
    if ((int)uVar4 < 0) {
      uVar4 = (uVar4 - 1 | 0xfffffff0) + 1;
    }
    bVar1 = (PacketXorKey16[uVar4] ^ bVar1 + 0x47) + 0x23;
    *pbVar5 = bVar1;
    if (uVar6 < 3) {
      *pbVar5 = pbVar2[uVar6 + 1] ^ bVar1;
    }
    iVar3 = _rand();
    pbVar5[(int)param_2 - (int)pbVar2] = (byte)iVar3;
    uVar6 = uVar6 + 1;
    pbVar5 = pbVar5 + 1;
  } while (uVar6 < 4);
  *param_1 = *(undefined4 *)pbVar2;
  operator_delete(pbVar2);
  return;
}


// FUN_00423c40 — Buffer_XorKey3
// Aplica XOR a cada byte de param_1[0..param_2-1] con DAT_00559678[i % 3].
void __cdecl FUN_00423c40(int param_1,int param_2)
{
  int iVar1;

  iVar1 = 0;
  if (0 < param_2) {
    do {
      *(byte *)(iVar1 + param_1) = *(byte *)(iVar1 + param_1) ^ (&PacketXorKey3)[iVar1 % 3];
      iVar1 = iVar1 + 1;
    } while (iVar1 < param_2);
  }
  return;
}


// FUN_00404400 — Packet_EncryptBuffer
// Encrypts a 0x584-byte (1412-byte) buffer param_2 into param_1.
// Allocates temp heap buffer, applies chain-XOR with key table (16 bytes),
// then randomizes source bytes.
void __cdecl FUN_00404400(void *param_1_v,void *param_2_v)
{
  undefined4 *param_1 = (undefined4*)param_1_v;
  undefined4 *param_2 = (undefined4*)param_2_v;
  byte bVar1;
  byte *pbVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  byte *pbVar6;
  uint uVar7;

  pbVar2 = (byte*)operator_new(0x584);
  puVar5 = param_2;
  pbVar6 = pbVar2;
  for (iVar4 = 0x161; iVar4 != 0; iVar4 = iVar4 + -1) {
    *(undefined4 *)pbVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    pbVar6 = pbVar6 + 4;
  }
  uVar7 = 0;
  pbVar6 = pbVar2;
  do {
    bVar1 = *pbVar6;
    *pbVar6 = bVar1 + 0x47;
    uVar3 = uVar7 & 0x8000000f;
    if ((int)uVar3 < 0) {
      uVar3 = (uVar3 - 1 | 0xfffffff0) + 1;
    }
    bVar1 = (PacketXorKey16[uVar3] ^ bVar1 + 0x47) + 0x23;
    *pbVar6 = bVar1;
    if (uVar7 < 0x583) {
      *pbVar6 = pbVar2[uVar7 + 1] ^ bVar1;
    }
    iVar4 = _rand();
    pbVar6[(int)param_2 - (int)pbVar2] = (byte)iVar4;
    uVar7 = uVar7 + 1;
    pbVar6 = pbVar6 + 1;
  } while (uVar7 < 0x584);
  pbVar6 = pbVar2;
  for (iVar4 = 0x161; iVar4 != 0; iVar4 = iVar4 + -1) {
    *param_1 = *(undefined4 *)pbVar6;
    pbVar6 = pbVar6 + 4;
    param_1 = param_1 + 1;
  }
  operator_delete(pbVar2);
  return;
}


// FUN_00409e20 — Packet_DecryptDword
// Decrypts a 4-byte block param_2 into param_1.
// Reverse chain-XOR then: b = (key[i%16] ^ (b-0x23)) + 0xb9
void __cdecl FUN_00409e20(void *param_1_v,void *param_2_v)
{
  undefined4 *param_1 = (undefined4*)param_1_v;
  undefined4 *param_2 = (undefined4*)param_2_v;
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;

  puVar2 = (undefined4*)operator_new(4);
  iVar5 = 4;
  *puVar2 = *param_2;
  uVar3 = 3;
  do {
    if (uVar3 < 3) {
      *(byte *)(uVar3 + (int)puVar2) =
           *(byte *)(uVar3 + (int)puVar2) ^ *(byte *)(uVar3 + 1 + (int)puVar2);
    }
    bVar1 = *(char *)(uVar3 + (int)puVar2) - 0x23;
    uVar4 = uVar3 & 0x8000000f;
    *(byte *)(uVar3 + (int)puVar2) = bVar1;
    if ((int)uVar4 < 0) {
      uVar4 = (uVar4 - 1 | 0xfffffff0) + 1;
    }
    *(byte *)(uVar3 + (int)puVar2) = (PacketXorKey16[uVar4] ^ bVar1) + 0xb9;
    uVar3 = uVar3 - 1;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  *param_1 = *puVar2;
  operator_delete((undefined *)puVar2);
  return;
}


// FUN_00403f80 — HashTable_Insert
// Inserta (clave=param_1, valor=return_addr) en la hash table (this).
// Busca un slot vacío o coincidente por sondeo lineal (stride 1 mod capacidad).
// Llama a FUN_00405540 con un mensaje de error si la tabla está llena.
void __cdecl FUN_00403f80(void *this_,void *param_1_v,void *param_2_v)
{
  undefined4 param_1 = *(undefined4*)param_1_v;
  undefined4 param_2 = *(undefined4*)param_2_v;
  uint uVar1;
  int iVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  byte *pbVar6;
  bool bVar7;
  bool bVar8;
  undefined4 unaff_retaddr = 0;
  uint uVar9;
  byte abStack_8 [8];

  // Ofuscación por HashTable: si la tabla no fue construida (vtable==NULL
  // or capacity==0) treat as no-op. CLAUDE.md marks these ops as non-game logic.
  if (this_ == nullptr || *(int *)this_ == 0 || *(int *)((int)this_ + 0xc) == 0) return;
  uVar1 = (**(code **)(*(int *)this_ + 0xc))(&param_2);
  abStack_8[0] = 0;
  abStack_8[1] = 0;
  abStack_8[2] = 0;
  abStack_8[3] = 0;
  uVar9 = 0;
  if (*(int *)((int)this_ + 0xc) != 0) {
    do {
      iVar2 = 4;
      bVar7 = false;
      iVar3 = 0;
      bVar8 = true;
      pbVar6 = (byte *)(*(int *)((int)this_ + 8) + uVar1 * 4);
      pbVar4 = (byte *)&param_1;
      pbVar5 = pbVar6;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar7 = *pbVar4 < *pbVar5;
        bVar8 = *pbVar4 == *pbVar5;
        pbVar4 = pbVar4 + 1;
        pbVar5 = pbVar5 + 1;
      } while (bVar8);
      if (!bVar8) {
        iVar3 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
      }
      if (iVar3 == 0) {
        *(undefined4 *)(*(int *)((int)this_ + 4) + uVar1 * 4) = unaff_retaddr;
        *(undefined4 *)(*(int *)((int)this_ + 8) + uVar1 * 4) = param_1;
        return;
      }
      iVar2 = 4;
      bVar7 = false;
      iVar3 = 0;
      bVar8 = true;
      pbVar4 = abStack_8;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar7 = *pbVar4 < *pbVar6;
        bVar8 = *pbVar4 == *pbVar6;
        pbVar4 = pbVar4 + 1;
        pbVar6 = pbVar6 + 1;
      } while (bVar8);
      if (!bVar8) {
        iVar3 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
      }
      if (iVar3 == 0) {
        *(undefined4 *)(*(int *)((int)this_ + 4) + uVar1 * 4) = unaff_retaddr;
        *(undefined4 *)(*(int *)((int)this_ + 8) + uVar1 * 4) = param_1;
        return;
      }
      uVar1 = (uVar1 + 1) % *(uint *)((int)this_ + 0xc);
      uVar9 = uVar9 + 1;
    } while (uVar9 < *(uint *)((int)this_ + 0xc));
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
  return;
}


// FUN_00404280 — HashTable_GetValue
// Busca param_1 en la hash table (this), devuelve el valor asociado o 0.
// Devuelve 0 (no encontrado) sin error; a diferencia de Insert, que avisa si está llena.
void* __cdecl FUN_00404280(void *this_,void *param_1_v)
{
  undefined4 param_1 = *(undefined4*)param_1_v;
  uint uVar1;
  int iVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  byte *pbVar6;
  bool bVar7;
  bool bVar8;
  uint uVar9;
  byte abStack_8 [4];
  undefined4 local_4;

  local_4 = param_1;
  if (this_ == nullptr || *(int *)this_ == 0 || *(int *)((int)this_ + 0xc) == 0) return (void*)0;
  uVar1 = (**(code **)(*(int *)this_ + 0xc))(&param_1);
  uVar9 = 0;
  undefined4 stack0xfffffff4 = local_4;
  if (*(int *)((int)this_ + 0xc) != 0) {
    do {
      iVar2 = 4;
      bVar7 = false;
      iVar3 = 0;
      bVar8 = true;
      pbVar6 = (byte *)(*(int *)((int)this_ + 8) + uVar1 * 4);
      pbVar4 = (byte*)&stack0xfffffff4;
      pbVar5 = pbVar6;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar7 = *pbVar4 < *pbVar5;
        bVar8 = *pbVar4 == *pbVar5;
        pbVar4 = pbVar4 + 1;
        pbVar5 = pbVar5 + 1;
      } while (bVar8);
      if (!bVar8) {
        iVar3 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
      }
      if (iVar3 == 0) {
        return (void*)0;
      }
      iVar2 = 4;
      bVar7 = false;
      iVar3 = 0;
      bVar8 = true;
      pbVar4 = abStack_8;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar7 = *pbVar4 < *pbVar6;
        bVar8 = *pbVar4 == *pbVar6;
        pbVar4 = pbVar4 + 1;
        pbVar6 = pbVar6 + 1;
      } while (bVar8);
      if (!bVar8) {
        iVar3 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
      }
      if (iVar3 == 0) {
        if (uVar1 == 0xffffffff) {
          return (void*)0;
        }
        return (void*)(uintptr_t)*(undefined4 *)(*(int *)((int)this_ + 4) + uVar1 * 4);
      }
      uVar1 = (uVar1 + 1) % *(uint *)((int)this_ + 0xc);
      uVar9 = uVar9 + 1;
    } while (uVar9 < *(uint *)((int)this_ + 0xc));
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
  return (void*)0;
}


// FUN_0043dcc0 @ 0x0043dcc0 — Object_GetType
// Devuelve *(undefined4*)(param_1 + 8) — el campo tipo/ID de un objeto.
int __fastcall FUN_0043dcc0(int param_1)
{
  return *(int *)(param_1 + 8);
}

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 3155-3436 (282 lines)
// CSimpleModulus encrypt/decrypt: FUN_0053cc30, FUN_0053cd20, FUN_0053cca0, FUN_0053ce30
// + helpers: CSimpleModulus_EncryptBlock, CSimpleModulus_DecryptBlock, CsmTrace, CsmWatchdog
// =============================================================================
// CSimpleModulus encryption (Mu Online client→server) — ported from IDA
// ═════════════════════════════════════════════════════════════════════════════
// Three-level function tree (all reachable from FUN_0053cc30):
//
//   FUN_0053cc30  CSimpleModulus::Encode(dst, src, len)     ← entry point
//       └─ FUN_0053cd20  Encrypt one 8-byte block → 11 bytes
//              ├─ FUN_0053cf90  Insert N-bit integer into bit-packed buffer
//              │       ├─ FUN_0053d170  bit-index → byte-index (>> 3)
//              │       └─ FUN_0053d0d0  in-buffer bit-shift
//              └─ (acceso a claves: this+20 EncKey[4], this+52 XorKey[4])
//
// Object layout (17 DWORDs, 68 bytes), matches g_SimpleModulusCS @ 0x05826c10:
//   +0  : header (la matemática del cifrado no lo usa; en el original es la vtable)
//   +4  : ModKey[0..3]  — modulus primes (always shared with Dec2.dat)
//   +20 : EncKey[0..3]  — multiplier (client→server)
//   +36 : DecKey[0..3]  — multiplicador (server→cliente, el mismo objeto cuando se mezclan)
//   +52 : XorKey[0..3]  — máscara de pre-XOR que se aplica al u16 en claro antes del módulo
//
// Cuando a FUN_0053cc30 se la llama con dst=0, sólo devuelve el tamaño de salida
// necesario (11 bytes por cada bloque de 8 bytes de entrada, redondeado hacia arriba). Con dst!=0 escribe
// los bytes encriptados en dst y devuelve igual la cantidad de bytes.
//
// Los llamadores pasan dst como `int` (de ahí la firma `int a1`) — lo casteamos de vuelta.
// El contexto siempre es g_SimpleModulusCS para codificar y g_SimpleModulusSC
// para decodificar; CC30 tiene CS hardcodeado (el original pasaba `this` por ecx, pero
// todos los llamadores ponen ecx=g_SimpleModulusCS antes de la llamada).
// ─────────────────────────────────────────────────────────────────────────────

// Forward decl — body below.
static void CSimpleModulus_EncryptBlock(DWORD *ctx, BYTE *dst, WORD *src, BYTE xorSeed);

// FUN_0053cc30 @ 0x0053cc30 — CSimpleModulus::Encode wrapper
// a1 = dst buffer (0 → size-only query)
// a2 = src plaintext buffer
// a3 = src length in bytes
// returns 11 * ceil(len / 8) (expanded encrypted size)
int __cdecl FUN_0053cc30(int a1, unsigned char *a2, int a3)
{
    int result = 11 * ((a3 + 7) / 8);
    if (!a1 || a3 <= 0) return result;

    BYTE *dst = (BYTE *)a1;
    BYTE *src = a2;
    int   remaining = a3;
    while (remaining > 0) {
        // Copia ≤8 bytes a un bloque scratch de 8 bytes rellenado con ceros, para que el
        // último bloque parcial no lea más allá del buffer de origen.
        BYTE  blockIn[8] = {0};
        int   take = remaining < 8 ? remaining : 8;
        for (int i = 0; i < take; ++i) blockIn[i] = src[i];

        CSimpleModulus_EncryptBlock(DAT_05826c10, dst, (WORD *)blockIn, (BYTE)take);

        src       += 8;
        dst       += 11;
        remaining -= 8;
    }
    return result;
}

// FUN_0053cd20 @ 0x0053CD20 — Encrypt one 8-byte block into 11 output bytes.
// ctx       : objeto CSimpleModulus (this) — tablas de claves en +20 y +52.
// dst       : buffer de salida de 11 bytes (CC30 ya lo puso en cero al entrar).
// src       : 4 plaintext u16 words = 8 bytes (little-endian).
// xorSeed   : byte de personalización del checksum (largo original del bloque).
//
// Algorithm per word w[i], i=0..3:
//   tmp[i]  = (EncKey[i] * (prevLow ^ (w[i] ^ XorKey[i]))) % ModKey[i]
//   prevLow = tmp[i] & 0xFFFF
// Post-procesa con una cadena de XOR sobre los 16 bits bajos (mezcla cruzada),
// y después empaqueta cada tmp[i] como {16 bits + 2 bits altos} en dst, en los offsets de bit
// 0,22,44,66,88 — en total 4 × 18 = 72 bits usados más 2 bits para el checksum
// en el offset 88.. y 16 bits del DWORD de checksum en el bit 0 del último campo.
// Byte final = XOR-suma de los 8 bytes de entrada con (xorSeed ^ 0x3D).
static void CSimpleModulus_EncryptBlock(DWORD *ctx, BYTE *dst, WORD *src, BYTE xorSeed)
{
    // dst must be pre-zeroed (11 bytes).
    for (int i = 0; i < 11; ++i) dst[i] = 0;

    DWORD packed[4];   // intermediate encrypted DWORDs (packed[i] = tmp)
    DWORD prevLow = 0;

    // ── Pass 1: modular multiply per 16-bit word ─────────────────────────────
    DWORD *modKey = ctx + 1;    // this+4  (ModKey[0..3])
    DWORD *encKey = ctx + 5;    // this+20 (EncKey[0..3])
    DWORD *xorKey = ctx + 13;   // this+52 (XorKey[0..3])
    for (int i = 0; i < 4; ++i) {
        DWORD w    = src[i] ^ xorKey[i];
        DWORD tmp  = (DWORD)((unsigned __int64)encKey[i] * (prevLow ^ w) % modKey[i]);
        packed[i]  = tmp;
        prevLow    = tmp & 0xFFFF;
    }

    // ── Pass 2: XorKey chain into lower 3 words (IDA sub_53CD20 post-loop).
    // packed[3] stays unchanged. For i=2,1,0:
    //   packed[i] = packed[i+1].lo_original ^ XorKey[i] ^ packed[i]
    {
        DWORD lo3 = packed[3] & 0xFFFF;
        DWORD lo2 = packed[2] & 0xFFFF;   // captured before packed[2] is overwritten
        DWORD lo1 = packed[1] & 0xFFFF;   // captured before packed[1] is overwritten
        packed[2] = lo3 ^ xorKey[2] ^ packed[2];
        packed[1] = lo2 ^ xorKey[1] ^ packed[1];
        packed[0] = lo1 ^ xorKey[0] ^ packed[0];
    }

    // ── Pass 3: bit-pack into dst using FUN_0053cf90 ─────────────────────────
    //   Por cada packed[i]: 16 bits arrancando en bitOff, después 2 bits en bitOff+22.
    //   bitOff avanza de a 18 por palabra (CF90 devuelve a2+a5).
    int bitOff = 0;
    for (int i = 0; i < 4; ++i) {
        bitOff = FUN_0053cf90((int)dst, bitOff, (int)&packed[i], 0,  16);
        bitOff = FUN_0053cf90((int)dst, bitOff, (int)&packed[i], 22, 2);
    }

    // ── Pass 4: checksum byte ────────────────────────────────────────────────
    BYTE chk = (BYTE)0xF8;  // -8 in signed char
    for (int i = 0; i < 8; ++i) chk ^= ((BYTE *)src)[i];

    // Escribe [chk ^ xorSeed ^ 0x3D] en los bits (bitOff..bitOff+8), y después
    // [chk] at bits (bitOff+8..bitOff+16).
    WORD  tail;
    ((BYTE *)&tail)[0] = (BYTE)(chk ^ xorSeed ^ 0x3D);
    ((BYTE *)&tail)[1] = chk;
    FUN_0053cf90((int)dst, bitOff, (int)&tail, 0, 16);
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_0053cca0 @ 0x0053CCA0 — CSimpleModulus::Decode
// Inversa de CC30 (codificar). Los paquetes server→cliente se decodifican con ésta.
// 11 bytes de entrada → 8 de salida por bloque. Devuelve el total de bytes decodificados
// (o sólo el tamaño cuando dst==0). El bloque lo decodifica FUN_0053ce30.
//
// Context = DAT_05826c58 (g_SimpleModulusSC) loaded from Data\Dec2.dat.
// Layout: ctx[1+i]=ModKey, ctx[9+i]=DecKey, ctx[13+i]=XorKey (DWORD indices).
// ─────────────────────────────────────────────────────────────────────────────
static int CSimpleModulus_DecryptBlock(DWORD *ctx, BYTE *dst, BYTE *src);

int __cdecl FUN_0053cca0(int a1, int a2, int a3, int a4)
{
    BYTE *dst = (BYTE *)a1;
    BYTE *src = (BYTE *)a2;
    int   srcLen = a3;
    // (a4 unused — IDA shows 4-arg signature; functions.h still declares 4 ints)
    (void)a4;

    if (!dst) return 8 * ((srcLen + 10) / 11);
    int outBytes = 0;
    int srcCursor = 0;
    while (srcCursor < srcLen) {
        int n = CSimpleModulus_DecryptBlock(DAT_05826c58, dst, src);
        if (n < 0) return n;            // checksum mismatch — propagate -1
        outBytes  += n;
        srcCursor += 11;
        src       += 11;
        dst       += 8;
    }
    return outBytes;
}

// FUN_0053ce30 @ 0x0053CE30 — Decrypt one 11-byte block to 8 bytes.
// ctx : CSimpleModulus context (decode flavor).
// dst : 8-byte output (caller must guarantee 8 bytes available).
// src : 11-byte input.
// Devuelve la cantidad de bytes de texto plano válidos (0..8) recuperados del
// checksum tail, or -1 on checksum mismatch.
extern "C" void DbgLogPublic(const char* msg);
static void CsmTrace(const char *fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt);
    _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    DbgLogPublic(buf);
}

// Watchdog: detects ANY future trample of DAT_05826c58 (Dec2 keys).
// Silencioso mientras mod0 sea estable; loguea una vez por cambio. Se mantiene después de arreglar el bug
// (DAT_055ca160 size mismatch) to catch any regression early.
extern "C" void CsmWatchdog(const char *tag)
{
    static DWORD s_known = 0;
    DWORD cur = DAT_05826c58[1];
    if (s_known == 0 && cur != 0) {
        s_known = cur;  // baseline
        return;
    }
    if (s_known != 0 && cur != s_known) {
        char buf[160];
        _snprintf_s(buf, sizeof(buf), _TRUNCATE,
            "CSM-WD[%s] ★TRAMPLED★ mod0 was %08X now %08X",
            tag, s_known, cur);
        DbgLogPublic(buf);
        s_known = cur;  // suppress repeats for the same trampled state
    }
}

static int CSimpleModulus_DecryptBlock(DWORD *ctx, BYTE *dst, BYTE *src)
{
    // Tracing deshabilitado después de confirmar que el decode de C3 funciona. Para reactivarlo,
    // poner trace=true y subir el tope de s_traceCount.
    static int s_traceCount = 0;
    bool trace = false;
    (void)s_traceCount;

    DWORD packed[4] = {0};

    // ── Pass 1: bit-unpack 4×{16+2}-bit packed[i] from src ────────────────
    int bitOff = 0;
    for (int i = 0; i < 4; ++i) {
        FUN_0053cf90((int)&packed[i], 0,  (int)src, bitOff,      16);
        bitOff += 16;
        FUN_0053cf90((int)&packed[i], 22, (int)src, bitOff,      2);
        bitOff += 2;
    }
    int tailBitOff = bitOff;  // 4*18 = 72

    if (trace) {
        CsmTrace("CSM block #%d src[0..10]=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                 s_traceCount,
                 src[0],src[1],src[2],src[3],src[4],src[5],src[6],src[7],src[8],src[9],src[10]);
        CsmTrace("CSM keys: Mod=[%08X %08X %08X %08X]",
                 ctx[1],ctx[2],ctx[3],ctx[4]);
        CsmTrace("CSM keys: Dec=[%08X %08X %08X %08X]",
                 ctx[9],ctx[10],ctx[11],ctx[12]);
        CsmTrace("CSM keys: Xor=[%08X %08X %08X %08X]",
                 ctx[13],ctx[14],ctx[15],ctx[16]);
        CsmTrace("CSM after pass1: packed=[%08X %08X %08X %08X]",
                 packed[0],packed[1],packed[2],packed[3]);
    }

    // ── Pass 2: reverse XOR chain across packed[0..2] ────────────────────
    // Encode did: packed[2]=lo3_old^XorKey[2]^packed[2]; etc. (using OLD lo's)
    // El decode revierte yendo de alto→bajo y usando los lo RECIÉN restaurados.
    // Per IDA sub_53CE30 líneas 56-67 (XorKey en this+15..13).
    DWORD *xorKey = ctx + 13;     // ctx[13..16]
    {
        DWORD prevLo = packed[3] & 0xFFFF;
        packed[2] = prevLo ^ xorKey[2] ^ packed[2];
        prevLo    = packed[2] & 0xFFFF;
        packed[1] = prevLo ^ xorKey[1] ^ packed[1];
        prevLo    = packed[1] & 0xFFFF;
        packed[0] = prevLo ^ xorKey[0] ^ packed[0];
    }

    if (trace) {
        CsmTrace("CSM after pass2: packed=[%08X %08X %08X %08X]",
                 packed[0],packed[1],packed[2],packed[3]);
    }

    // ── Pass 3: inverse modular multiply per word ────────────────────────
    // outWord[i] = prevLo ^ XorKey[i] ^ ((packed[i] * DecKey[i]) % ModKey[i])
    // prevLo para la iteración siguiente = lo(packed[i]) (ANTES de multiplicar).
    DWORD *modKey = ctx + 1;      // ctx[1..4]
    DWORD *decKey = ctx + 9;      // ctx[9..12]
    WORD  *outW   = (WORD *)dst;
    WORD   prevLo = 0;
    for (int i = 0; i < 4; ++i) {
        DWORD m   = (DWORD)(((unsigned __int64)packed[i] * decKey[i]) % modKey[i]);
        WORD  w   = (WORD)(xorKey[i] ^ m);
        outW[i]   = (WORD)(prevLo ^ w);
        prevLo    = (WORD)packed[i];
    }

    // ── Pass 4: read+verify checksum tail ────────────────────────────────
    DWORD tail = 0;
    FUN_0053cf90((int)&tail, 0, (int)src, tailBitOff, 16);
    BYTE  byteSeed = (BYTE)(tail & 0xFF);
    BYTE  byteChk  = (BYTE)((tail >> 8) & 0xFF);
    // Recupera el byteSeed original = (chk_lo ^ chk_hi ^ 0x3D) → deshacer para obtener xorSeed
    BYTE  xorSeed  = (BYTE)(byteSeed ^ byteChk ^ 0x3D);

    // Recompute checksum over the 8 output bytes
    BYTE chkCalc = 0xF8;
    for (int i = 0; i < 8; ++i) chkCalc ^= dst[i];

    if (trace) {
        CsmTrace("CSM out[0..7]=%02X %02X %02X %02X %02X %02X %02X %02X  chkCalc=%02X chkRecv=%02X xorSeed=%02X",
                 dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7],chkCalc,byteChk,xorSeed);
    }

    if (chkCalc != byteChk) return -1;
    return (int)xorSeed;          // 1..8 = number of valid plaintext bytes
}

// ── FUN_00403ea0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_00403ea0 @ 0x00403EA0 (73 bytes) — Quest class full init
void __fastcall FUN_00403ea0(void *param_1_raw) {
    int *param_1 = (int *)param_1_raw;
    *param_1 = (int)&PTR_LAB_005524b8;
    if (g_csQuest == 0) g_csQuest = (DWORD)param_1;
    *(BYTE *)(param_1 + 1) = 0xff;
    *(BYTE *)((int)param_1 + 0x1c87a) = 0;
    *(BYTE *)((int)param_1 + 0x1c87b) = 0;
    *(BYTE *)((int)param_1 + 0x1c87d) = 0;
    *(BYTE *)((int)param_1 + 0x1c87e) = 0;
    *(BYTE *)((int)param_1 + 0x1c87f) = 0;
    *(short *)(param_1 + 0x7220) = 0;
    // *param_1 = (int)&PTR_FUN_005524b4; // final vtable
}

// ── FUN_00403ef0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 17-byte ─────────────────────────────────────────────────────────────────

// FUN_00403ef0 @ 0x00403EF0 (17 bytes) — quest vtable init
void __fastcall FUN_00403ef0(int *param_1) {
    *param_1 = (int)&PTR_LAB_005524b8;
    g_csQuest = 0;
}

// ── FUN_00403f10 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 30-byte: virtual destructors (deinit + conditional delete) ──────────────

// FUN_00403f10 @ 0x00403F10 — Quest ~dtor
void __fastcall FUN_00403f10(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00403ef0((int *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00404040 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_00404040 @ 0x00404040 — HashTable_Remove (__thiscall this, char *key)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_00404040(void *ctx, void *key) {
    // STUB: HashTable remove with obfuscation — cannot implement safely
    (void)ctx; (void)key;
}

// ── FUN_00404370 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_00404370 @ 0x00404370 — HashTable_CopyEncrypt(param_1, param_2)
// Copia 0x584 bytes de param_2 a un buffer nuevo, lo codifica con XOR (resta 0x23,
// XOR con la tabla DAT_00559050, suma 0xb9) y copia el resultado de vuelta a param_1.
void __cdecl FUN_00404370(void *vparam_1, void *vparam_2) {
    unsigned int *param_1 = (unsigned int*)vparam_1;
    unsigned int *param_2 = (unsigned int*)vparam_2;
    unsigned int *puVar2 = (unsigned int*)operator_new(0x584);
    unsigned int *puVar6 = puVar2;
    for (int iVar3 = 0x161; iVar3 != 0; iVar3--) { *puVar6 = *param_2++; puVar6++; }
    unsigned int uVar4 = 0x583;
    int iVar3 = 0x584;
    do {
        if (uVar4 < 0x583) {
            ((unsigned char*)puVar2)[uVar4] ^= ((unsigned char*)puVar2)[uVar4 + 1];
        }
        unsigned char bVar1 = ((unsigned char*)puVar2)[uVar4] - 0x23;
        ((unsigned char*)puVar2)[uVar4] = bVar1;
        unsigned int uVar5 = uVar4 & 0x8000000fu;
        if ((int)uVar5 < 0) uVar5 = (uVar5 - 1 | 0xfffffff0u) + 1;
        ((unsigned char*)puVar2)[uVar4] = (PacketXorKey16[uVar5] ^ bVar1) + 0xb9;
        uVar4--;
        iVar3--;
    } while (iVar3 != 0);
    puVar6 = puVar2;
    for (iVar3 = 0x161; iVar3 != 0; iVar3--) { *param_1 = *puVar6++; param_1++; }
    operator_delete((unsigned char*)puVar2);
}

// ── FUN_00408e30 — movida desde stubs_externs.cpp (refactor B3) ──
int  __cdecl    FUN_00408e30(DWORD *a1);

extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

// sub_402850 @ 0x00402850 (1774 bytes) — input de la ventana de quest del NPC.
// IDA, sacando el ruido anti-tamper del armado del paquete:
//
//   if (MouseX in [450,640) && MouseY in [0,433))  MouseOnWindow = 1;
//   sub_401AF0(This);
//   if (MouseX in [485,605) && MouseY in [355,379) && MouseLButtonPush) {
//       send([C1][05][A2][questIndex][01]);
//       MouseLButtonPush = 0; MouseLButton = 0; PlayBuffer(28, 0, 0);
//   }
//   if (MouseX in [475,499) && MouseY in [395,419) && MouseLButtonPush) {
//       MouseLButtonPush = 0; MouseUpdateTime = 0; MouseUpdateTimeMax = 6;
//       CSQuest::clearQuest(This);
//   }
//   result = MouseLButtonPop;  if (MouseLButtonPop) MouseLButtonPop = 0;
//
// 2026-08-22: acá había un resumen con SÓLO la rama del botón de cerrar.
// Faltaba la del botón de aceptar/continuar la quest — el que sub_403320
// dibuja en (485,355) 120x24 con GlobalText[699] ("Proceder con la quest").
// O sea el botón se veía y hasta se pintaba al pasar el mouse (ese feedback
// está en sub_403320), pero el click no mandaba nada y la quest no avanzaba.
int __cdecl FUN_00402850(void *param_1) {
    if ((0x1c1 < DAT_083a427c) && (DAT_083a427c < 0x280) &&
        (-1 < DAT_083a4278) && (DAT_083a4278 < 0x1b1))
        DAT_07d78094 = 1;                       // MouseOnWindow

    FUN_00401af0(param_1);

    // Botón de aceptar / continuar la quest.
    if ((484 < DAT_083a427c) && (DAT_083a427c < 605) &&
        (354 < DAT_083a4278) && (DAT_083a4278 < 379) &&
        (DAT_083a4124 != 0)) {
        // Mismo paquete que las respuestas del diálogo (ver Quest_SendState en
        // Scene_CharSelect_Nav.cpp): el server sólo lee QuestIndex y avanza el
        // estado él mismo.
        BYTE pkt[5];
        pkt[0] = 0xC1;
        pkt[1] = 0x05;
        pkt[2] = 0xA2;
        pkt[3] = *(BYTE *)((int)param_1 + 0x1c87a);   // índice de quest actual
        pkt[4] = 0x01;
        Net_SendSmallPacket(pkt, 5);

        DAT_083a4124 = 0;                       // MouseLButtonPush
        DAT_083a42c4 = 0;                       // MouseLButton
        PlayBuffer(28, 0, 0);
    }

    // Botón de cerrar.
    if ((0x1da < DAT_083a427c) && (DAT_083a427c < 499) &&
        (0x18a < DAT_083a4278) && (DAT_083a4278 < 0x1a3) &&
        (IsClickPushed())) {
        DAT_083a4124 = 0;
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        FUN_00401960((int)param_1);
    }
    if (DAT_083a413c != 0) DAT_083a413c = 0;
    return 1;
}

// ── FUN_00408ff0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 10
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 11 — BMD/SMD model loading + mesh rendering + linked list + quest init
// ═══════════════════════════════════════════════════════════════════════════════

// FUN_00408ff0 @ 0x00408FF0 (~43 lines) — BMD mesh render: prepare vertices + draw front/back faces
// __fastcall(ecx=meshObj). Aloca un buffer temporal de vértices (count*0xC), llama a FUN_00407b30 para transformar
// cada vértice, setea el modo de alpha según los flags de obj+0x14 (0=deshabilitar, 0x1000=alphaTest),
// y después llama dos veces a FUN_004090b0, para la cara frontal (textura obj+0xC) y la trasera (textura obj+0x10).
void __fastcall FUN_00408ff0(void* param_1) {
    // Port FIEL de IDA `sub_408FF0` — arma el buffer de vértices transformando
    // cada nodo y dibuja las dos caras.
    int *thiz = (int *)param_1;
    if (!thiz) return;
    int cols  = thiz[10];   // +0x28
    int rows  = thiz[11];   // +0x2c
    int count = thiz[12];   // +0x30
    int nodes = thiz[13];   // +0x34
    if (cols < 2 || rows < 2 || count <= 0 || !nodes) return;

    float *verts = (float *)operator_new(12 * count);
    if (!verts) return;

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            FUN_00407b30((void *)(nodes + 60 * (c + r * cols)),   // stride 0x3C
                         &verts[3 * (c + r * cols)]);

    // ── CLOTHDBG (temporal): bbox de los vértices ya transformados + handles
    // GL de las dos texturas. Si el bbox está lejos del pj o es degenerado, el
    // problema son las posiciones de los nodos (física / init); si está bien,
    // es textura o blend.
    {
        static DWORD s_lastCl = 0;
        DWORD nowCl = GetTickCount();
        if (nowCl - s_lastCl > 1000) {
            s_lastCl = nowCl;
            float mn[3] = { 1e30f, 1e30f, 1e30f }, mx[3] = { -1e30f, -1e30f, -1e30f };
            for (int k = 0; k < count; ++k)
                for (int a = 0; a < 3; ++a) {
                    float v = verts[3 * k + a];
                    if (v < mn[a]) mn[a] = v;
                    if (v > mx[a]) mx[a] = v;
                }
            int tf, tb; memcpy(&tf, &thiz[3], 4); memcpy(&tb, &thiz[4], 4);
            unsigned glF = (tf >= 0 && tf < 2048) ? (unsigned)(&DAT_083a7ccc)[tf * 0xe] : 0xFFFFFFFFu;
            unsigned glB = (tb >= 0 && tb < 2048) ? (unsigned)(&DAT_083a7ccc)[tb * 0xe] : 0xFFFFFFFFu;
            // ¿cae en pantalla? Projection (0x5113F0) devuelve coords 640x480.
            float ctr[3] = { (mn[0]+mx[0])*0.5f, (mn[1]+mx[1])*0.5f, (mn[2]+mx[2])*0.5f };
            int psx = -9999, psy = -9999;
            Camera_ProjectWorldToScreen(ctr, &psx, &psy);
            char clb[300];
            _snprintf_s(clb, sizeof(clb), _TRUNCATE,
                "CLOTHDBG v0=(%.1f,%.1f,%.1f) min=(%.1f,%.1f,%.1f) max=(%.1f,%.1f,%.1f) "
                "proj=(%d,%d) flags=0x%X texF=%d(gl=%u) texB=%d(gl=%u)",
                verts[0], verts[1], verts[2], mn[0], mn[1], mn[2], mx[0], mx[1], mx[2],
                psx, psy, (unsigned)thiz[5], tf, glF, tb, glB);
            DbgLogPublic(clb);
        }
    }

    unsigned flags = (unsigned)thiz[5] & 0x3000u;                 // +0x14
    if (flags == 0)          GL_ResetState();                      // DisableAlphaBlend
    else if (flags == 0x1000) GL_SetBlendSrcOver('');               // EnableAlphaTest

    glColor3f(1.0f, 1.0f, 1.0f);
    float texFront, texBack;
    memcpy(&texFront, &thiz[3], 4);   // +0x0C
    memcpy(&texBack,  &thiz[4], 4);   // +0x10
    FUN_004090b0(param_1, 0, 1, texFront, (int)verts);
    FUN_004090b0(param_1, 0, 0, texBack,  (int)verts);

    FUN_0054158c((unsigned char *)verts);
}

// ── FUN_004090b0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_004090b0 @ 0x004090B0 (~54 lines) — BMD mesh: emit quads for one face side
// __thiscall(this=meshObj, param_1=windingOrder, param_2=textureId, param_3=vertexBuf).
// Binds texture, glBegin(GL_QUADS=7), nested loop over rows/cols emitting 4 vertices per quad
// vía FUN_004091d0. El orden de winding se invierte según param_1 (0=atrás, 1=adelante).
void __fastcall FUN_004090b0(void* ecx, void* /*edx*/, int param_1, float param_2, int param_3) {
    // Port FIEL de IDA `sub_4090B0`. `param_2` es un GLfloat que en realidad
    // lleva el ID de textura en sus BITS (IDA: `BindTexture(SLODWORD(t))`),
    // igual que el `t` de sub_4091D0 lleva el índice de fila.
    int *thiz = (int *)ecx;
    if (!thiz || !param_3) return;
    int cols = thiz[10];   // +0x28
    int rows = thiz[11];   // +0x2c
    int texId; memcpy(&texId, &param_2, 4);

    GL_BindTextureSlot(texId);
    glBegin(GL_QUADS);
    if (param_1) {
        // cara delantera: (c,r) (c+1,r) (c+1,r+1) (c,r+1)
        for (int r = 0; r < rows - 1; ++r)
            for (int c = 0; c < cols - 1; ++c) {
                FUN_004091d0(ecx, 0, param_3, c,     (float)r);
                FUN_004091d0(ecx, 0, param_3, c + 1, (float)r);
                FUN_004091d0(ecx, 0, param_3, c + 1, (float)(r + 1));
                FUN_004091d0(ecx, 0, param_3, c,     (float)(r + 1));
            }
    } else {
        // cara trasera: winding invertido — (c,r) (c,r+1) (c+1,r+1) (c+1,r)
        for (int r = 0; r < rows - 1; ++r)
            for (int c = 0; c < cols - 1; ++c) {
                FUN_004091d0(ecx, 0, param_3, c,     (float)r);
                FUN_004091d0(ecx, 0, param_3, c,     (float)(r + 1));
                FUN_004091d0(ecx, 0, param_3, c + 1, (float)(r + 1));
                FUN_004091d0(ecx, 0, param_3, c + 1, (float)r);
            }
    }
    glEnd();
}

// ── FUN_004091d0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_004091d0 @ 0x004091D0 (~21 lines) — BMD mesh: emit single vertex with texcoord
// __thiscall. Calcula la UV desde la posición en la grilla: u = col/(cols-1), v = row/(rows-1), clampeado a 0.99.
// Emite glTexCoord2f + glVertex3f desde el buffer de vértices en param_1 + (row*cols+col)*0xC.
void __fastcall FUN_004091d0(void* ecx, void* /*edx*/, int param_1, int param_2, float param_3) {
    // Port FIEL de IDA `sub_4091D0`. param_1 = buffer de vértices (stride 12),
    // param_2 = columna, param_3 = fila (llega como float con valor entero).
    int *thiz = (int *)ecx;
    if (!thiz || !param_1) return;
    int cols = thiz[10];   // +0x28
    int rows = thiz[11];   // +0x2c
    int row  = (int)param_3;
    if (cols < 2 || rows < 2) return;

    float v = (float)row / (float)(rows - 1);
    if (v > 0.99f) v = 0.99f;               // IDA clampea a 0.99
    float u = (float)param_2 / (float)(cols - 1);

    const float *vert = (const float *)(param_1 + 12 * (param_2 + row * cols));
    glTexCoord2f(u, v);
    glVertex3f(vert[0], vert[1], vert[2]);
}

// ── FUN_00409250 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_00409250 @ 0x00409250 — VerletNode_AddToSystem: allocate node, init, insert into doubly-linked list.
// La cabeza de la lista está en this+0x50 (nodo de 0xc bytes: [0]=datos, [4]=next, [8]=prev). El contador en this+0x48.
// La entrada nueva se inserta justo después del centinela de cabecera.
void __cdecl FUN_00409250(void *widget, float p1, float p2, float p3, float radius, int boneIdx)
{
    char *thiz = (char*)widget;
    // allocate and construct VerletNode (0x24 bytes)
    void *node_raw = operator_new(0x24);
    void *node = (node_raw == NULL) ? NULL : (void *)FUN_00407e50((DWORD *)node_raw);
    FUN_00407ef0(node, p1, p2, p3, radius, boneIdx);

    // aloca la entrada de la lista enlazada (0xc bytes: [+0]=node_ptr, [+4]=next, [+8]=prev)
    int *entry = (int *)operator_new(0xc);
    if (entry != NULL) {
        entry[0] = 0;
        entry[1] = 0;
        entry[2] = 0;
        entry[0] = (int)node;

        // 2026-08-11 FIX (crash 0xC0000005 escribiendo a 0xCDCDCDD5 = heap sin
        // inicializar + 8, al crear la capa del MG): esta rutina trataba
        // `thiz + 0x50` como si el centinela estuviera EMBEBIDO ahí, y además
        // asumía `+4 = next / +8 = prev`. Las dos cosas están al revés.
        // IDA `sub_409250` L27-32:
        //     result[1] = *(_DWORD *)(this[20] + 4);
        //     *(_DWORD *)(*(_DWORD *)(this[20] + 4) + 8) = result;
        //     result[2] = this[20];
        //     *(_DWORD *)(this[20] + 4) = result;
        //     this[18] = this[18] + 1;
        // `this[20]` es el VALOR del puntero al centinela de cola que guardó
        // `sub_407FE0` en +0x50, y el layout es **+4 = prev, +8 = next**
        // (coherente con el ctor: `head[8] = tail`, `tail[4] = head`).
        // Se inserta al FINAL, antes de la cola.
        int tail = *(int *)(thiz + 0x50);      // this[20]
        int prev = *(int *)(tail + 4);         // tail->prev
        entry[1] = prev;                       // entry->prev = tail->prev
        *(int *)(prev + 8) = (int)entry;       // prev->next  = entry
        entry[2] = tail;                       // entry->next = tail
        *(int *)(tail + 4) = (int)entry;       // tail->prev  = entry
        *(int *)(thiz + 0x48) += 1;            // this[18]++
    }
}

// ── FUN_00409310 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
int   __cdecl    FUN_00409310(DWORD *thiz);                       // colisión con anclas
void  __fastcall FUN_00407e10(void *a, float b, int c, int d);
void  __fastcall FUN_00407e30(void *a, float *b);
void  __fastcall FUN_00407b30(void *node, float *out);

// FUN_00408e30 @ 0x00408E30 — Cloth_Solve: una iteración del solver.
// Port FIEL de IDA `sub_408E30`.
//   1. Recorre la lista de anclas y refresca su posición de MUNDO desde el
//      hueso al que están atadas (`TransformPosition`). Ese es el bucle que
//      hace que la tela siga al personaje.
//   2. `sub_409310` — colisión de cada nodo contra cada ancla.
//   3. Springs de igualdad (flags & 1).
//   4. Flush de correcciones acumuladas (`sub_407D10`) sobre toda la grilla.
//   5. Springs de rango (flags & 4): si alguno no converge, devuelve 0 para
//      que `sub_408900` vuelva a iterar.
int __cdecl FUN_00408e30(DWORD *a1)
{
  float Position[3];
  float WorldPosition[3];

  // El original no chequea BoneTransform: la tela sólo se crea para entidades
  // con el modelo ya cargado. Guard defensivo — sin él, `Matrix` sería
  // `48*boneIdx` (una dirección de dos dígitos) y el frame moriría en un AV.
  if (!a1 || !a1[1] || !*(DWORD *)(a1[1] + 276))
    return 1;

  // ── 1. anclas → posición de mundo desde el hueso ──────────────────────
  for (int i = *(int *)(a1[19] + 8); (int)a1[20] != i && i; i = *(int *)(i + 8))
  {
    DWORD *v5 = *(DWORD **)i;
    FUN_00407e30(v5, Position);
    // IDA: rota el vector local (x,y,z) → (z,-y,x) antes de transformar.
    float v6 = Position[0];
    Position[1] = -Position[1];
    Position[0] = Position[2];
    Position[2] = v6;
    FUN_004409a0(
        (void *)(DAT_05828d58 + 0xbc * (int)*(short *)(a1[1] + 2)),
        (float *)(*(DWORD *)(a1[1] + 276) + 48 * v5[4]),
        Position,
        WorldPosition,
        1);
    FUN_00407e10(v5, WorldPosition[0], *(int *)&WorldPosition[1], *(int *)&WorldPosition[2]);
  }

  FUN_00409310(a1);

  // ── 2. springs de igualdad ────────────────────────────────────────────
  for (int v7 = 0, v8 = 0; v7 < (int)a1[14]; ++v7, v8 += 16)
  {
    float *v9 = (float *)(a1[15] + v8);
    if ((*(BYTE *)&v9[3] & 1) != 0)
      FUN_00407c60(a1[13] + 60 * *(short *)v9, a1[13] + 60 * *((short *)v9 + 1), v9[2]);
  }

  // ── 3. flush de correcciones ──────────────────────────────────────────
  for (int j = 0; j < (int)a1[11]; ++j)
  {
    int v11 = a1[10];
    for (int k = 0; k < v11; ++k)
    {
      FUN_00407d10(a1[13] + 60 * (k + j * v11));
      v11 = a1[10];
    }
  }

  // ── 4. springs de rango ───────────────────────────────────────────────
  for (int v16 = 0, v3 = 0; v16 < (int)a1[14]; ++v16, v3 += 16)
  {
    float *v13 = (float *)(a1[15] + v3);
    int v14 = *((short *)v13 + 1);
    if (v14 >= (int)a1[10]
        && (*(BYTE *)&v13[3] & 4) != 0
        && !FUN_00407b90(a1[13] + 60 * v14, a1[13] + 60 * *(short *)v13, v13 + 1))
    {
      return 0;
    }
  }
  return 1;
}

// ── FUN_004093a0 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_004093a0 @ 0x004093A0 — Widget_Ctor: llama a FUN_00407fe0 y después setea la vtable.
void* __fastcall FUN_004093a0(void *param_1)
{
    FUN_00407fe0(param_1);
    // vtable = &PTR_LAB_00552548 — skipped in re-impl
    return param_1;
}

// ── FUN_004093c0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004093c0 @ 0x004093C0 (11 bytes)
void __fastcall FUN_004093c0(void *This) {
    *(int *)This = (int)&PTR_LAB_005524e8;
    FUN_00408070(This);
}

// ── FUN_004093e0 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_004093e0 @ 0x004093E0 — SpringMesh_Create: builds triangle-mesh spring system from BMD face data.
// Por cada vértice de cara: inicializa la posición del nodo desde el buffer de vértices de huesos (DAT_0584621c), lo marca como fijo si el tipo coincide.
// Por cada arista de cara (3 por triángulo): calcula el largo de reposo vía FUN_00407b50, marca si es horizontal/vertical,
// y agrega el resorte vía FUN_004088b0 a this->springs (stride 0x10; na,nb,rest_scaled,rest,flags).
// Layout: this+4=entity, +8=type, +0x14=flags, +0x30=node_count, +0x34=node_array*, +0x38=spring_count,
//         +0x3c=spring_array*, +0x54=slot_index, +0x5c=model_type.
void __cdecl FUN_004093e0(void *widget, int entity, short *slot, int type, int radius, int flags)
{
    char *thiz = (char*)widget;
    *(int *)(thiz + 8)  = type;
    *(int *)(thiz + 4)  = entity;
    *(short**)(thiz + 0x54) = slot;
    *(unsigned int *)(thiz + 0x14) |= (unsigned int)radius;   // radius repurposed as flag mask

    int model_type = (flags == -1) ? (int)*(short *)(entity + 2) : flags;
    *(int *)(thiz + 0x5c) = model_type;

    // model data: DAT_05828d58 + model_type * 0xbc; face list at +0x28
    int model_base = *(int *)(&DAT_05828d58) + model_type * 0xbc;
    int face_list  = *(int *)(model_base + 0x28);
    int face_entry = face_list + (int)slot * 0x28;

    // node count from face vertex list header (+4)
    int node_count = (int)*(short *)(face_entry + 4);
    *(int *)(thiz + 0x30) = node_count;

    // allocate node array: [count_header][node0..nodeN], each node 0x3c bytes
    int *raw = (int*)operator_new(node_count * 0x3c + 4);
    int *nodes;
    if (raw == NULL) {
        nodes = NULL;
    } else {
        nodes = raw + 1;
        *raw  = node_count;
        FUN_00541ec1(nodes, 0x3c, node_count, (void*)FUN_00407950);
    }
    *(int **)(thiz + 0x34) = nodes;

    // spring count from face header (+0xa = short)
    int spring_count = (int)*(short *)(face_entry + 0xa);
    *(int *)(thiz + 0x38) = spring_count * 3;

    // allocate spring array: spring_count * 0x30 bytes
    void *springs = operator_new(spring_count * 0x30);
    *(void **)(thiz + 0x3c) = springs;

    // init node positions from bone vertex buffer
    // buffer de vértices: (float*)&DAT_0584621c, stride de 3 floats por entrada * 15000 por slot
    float *verts = (float*)&DAT_0584621c;
    int vert_base = (int)slot * 15000;
    int *vert_indices = (int *)*(int *)(face_entry + 0x10);  // vertex index list

    for (int i = 0; i < node_count; i++) {
        int vi = vert_base + i;
        float x = verts[vi * 3];
        float y = verts[vi * 3 + 1];
        float z = verts[vi * 3 + 2];
        int pinned = ((int)*(short *)(vert_indices + i * 4) == type) ? 1 : 0;
        FUN_004079b0((char*)nodes + i * 0x3c, x, y, z, pinned);
    }

    // build springs from face triangle edges
    int *face_ptr  = (int *)*(int *)(face_entry + 0x1c);
    unsigned int sp = 0;
    for (int fi = 0; fi < spring_count; fi++, face_ptr = (int*)((char*)face_ptr + 0x24)) {
        short *tri = (short*)face_ptr;
        for (int ei = 0; ei < 3; ei++) {
            short na = tri[ei + 1];
            short nb = tri[(ei + 1) % 3 + 1];
            int pA = (int)nodes + na * 0x3c;
            int pB = (int)nodes + nb * 0x3c;
            float delta[3];
            float dist = FUN_00407b50((void*)pA, pB, delta);
            float posA[3], posB[3];
            FUN_00407b30((void*)pA, posA);
            FUN_00407b30((void*)pB, posB);
            BYTE edge_flags = (BYTE)(2 | (fabsf(posA[0] - posB[0]) <= _DAT_00552560 ? 4 : 1));
            FUN_004088b0(widget, (int)sp, na, nb, dist * _DAT_00552504, dist, edge_flags);
            sp++;
        }
    }
    *(unsigned int *)(thiz + 0x38) = sp;
}

// ── FUN_00409ad0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00409ad0 @ 0x00409AD0 (~42 lines) — CSQuest constructor: init doubly-linked list
// __fastcall(ecx=questObj). Aloca los nodos centinela de cabeza y cola (0xC cada uno, con punteros next/prev),
// los enlaza, pone count=0, setea la vtable a PTR_FUN_00552568 y llama a FUN_00403a30 (reset de quests).
void* __fastcall FUN_00409ad0(void* param_1) {
    // head = new Node{0,0,0}; tail = new Node{0,0,0}
    // param_1[2] = head; param_1[3] = tail
    // head->next = tail; tail->prev = head
    // param_1[1] = 0 (count)
    // *param_1 = &PTR_FUN_00552568 (vtable)
    // FUN_00403a30() — quest state reset
    return param_1;
}

// ── FUN_00409b60 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00409b60 @ 0x00409B60 — SoundWidgetB ~dtor
void __fastcall FUN_00409b60(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00409b80((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00409b80 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00409b80 @ 0x00409B80 (~39 lines) — CSQuest destructor: clear list + free sentinels
// __fastcall(ecx=questObj). Setea la vtable, llama a FUN_00409d20 (limpia todos los nodos),
// y después libera la cadena de nodos entre la cabeza y la cola, y los propios centinelas.
void __fastcall FUN_00409b80(void* param_1) {
    // *param_1 = &PTR_FUN_00552568 (vtable)
    // FUN_00409d20(param_1) — clear all quest nodes
    // Recorre la cadena head->next y borra cada nodo
    // Re-link head<->tail, count=0
    // delete tail, delete head
}

// ── FUN_00409d20 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00409d20 @ 0x00409D20 (~34 lines) — CSQuest: clear all quest nodes from linked list
// __fastcall(ecx=questObj). Itera desde head->next hasta tail, llama a FUN_004086e0 sobre los datos
// de cada nodo y después invoca el destructor vía la vtable. Libera todos los nodos intermedios.
void __fastcall FUN_00409d20(int param_1) {
    // Recorre desde (param_1+8)->next hasta llegar al centinela (param_1+0xC):
    //   FUN_004086e0(*node) — cleanup node data
    //   (*node->vtable[0])(1) — destructor with free
    // Free all intermediate nodes
    // Re-link head<->tail, count=0
}

// ── FUN_00409db0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_00409db0 @ 0x00409DB0 (110 bytes) — LinkedList: destroy all nodes + sentinels
void __fastcall FUN_00409db0(int *param_1) {
    *(int *)(*(int *)(param_1[2] + 4) + 8) = 0;
    void *pvVar1 = *(void **)(param_1[1] + 8);
    while (pvVar1 != NULL) {
        void *next = *(void **)((int)pvVar1 + 8);
        operator_delete(pvVar1);
        pvVar1 = next;
    }
    *(int *)(param_1[1] + 8) = param_1[2];
    *(int *)(param_1[2] + 4) = param_1[1];
    *param_1 = 0;
    if ((void *)param_1[2] != NULL) operator_delete((void *)param_1[2]);
    if ((void *)param_1[1] != NULL) operator_delete((void *)param_1[1]);
}

// ── FUN_00409ea0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00409ea0 @ 0x00409EA0 (10 bytes) — calls sound device init
void FUN_00409ea0(void) { FUN_0040a600((void *)&DAT_00590b00); }

// ── FUN_00409eb0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00409eb0 @ 0x00409EB0 (12 bytes)
void FUN_00409eb0(void) {}

// ── FUN_00409ed0 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
// FUN_00409ed0 @ 0x00409ED0 — WidgetB_Ctor: set vtable + zero fields.
void* __fastcall FUN_00409ed0(void *param_1)
{
    FUN_00409f20((int)param_1);
    // vtable set skipped
    return param_1;
}

// ── FUN_00409ef0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00409ef0 @ 0x00409EF0 — WidgetC ~dtor
void __fastcall FUN_00409ef0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00409f10((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00409f10 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
// FUN_00409f10 @ 0x00409F10 — WidgetB_SetVtable: sólo setea el puntero de vtable.
void __fastcall FUN_00409f10(void *param_1)
{
    // vtable = &PTR_FUN_00552574 — skipped in re-impl
    (void)param_1;
}

// ── FUN_00409f20 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
// ── FUN_00409f20/ed0/f10/0040a660 — second widget type ctor chain ────────────

// FUN_00409f20 @ 0x00409F20 — WidgetB_ZeroFields: clear 4 fields (+4,+8,+0x18,+0x1c).
void __fastcall FUN_00409f20(int param_1)
{
    *(short *)(param_1 + 4) = 0;
    *(int *)(param_1 + 8)   = 0;
    *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x1c) = 0;
}

// ── FUN_00409f30 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00409f30 @ 0x00409F30 (~64 lines) — BMD: build visible triangle list for mesh part
// __thiscall(this=renderCtx, param_1=vertBuf, param_2=meshData, param_3=entityPtr, param_4=skipAlpha).
// Checks entity visibility distance (param_3+0x168 >= threshold). Skips mesh parts at indices
// guardados en param_3+0x58/0x64. Cuenta los triángulos visibles por sub-mesh, aloca el buffer de resultado
// (count * 0x1E), y después llama a FUN_0040a1c0 por cada sub-mesh para poblar los datos de triángulos.
// Devuelve 1 si lo armó, 0 si la entidad está demasiado lejos o los índices de mesh son inválidos.
int __fastcall FUN_00409f30(void* ecx, void* /*edx*/, int param_1, int param_2, int param_3, char param_4) {
    (void)ecx; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    // if (*(float*)(param_3+0x168) < umbral) return 0
    // Saltea las sub-meshes en los índices sVar1, sVar2 (de param_3+0x58, +0x64)
    // Count total triangles across valid sub-meshes
    // this+0x18 = 0; this+0x1C = new byte[totalTris * 0x1E]
    // Por cada sub-mesh válida: FUN_0040a1c0(this, meshIdx, param_1, triCount, triData)
    // return 1
    return 0;
}

// ── FUN_0040a0a0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_0040a0a0 @ 0x0040A0A0 (100 bytes) — sound object init + open file
void __fastcall FUN_0040a0a0(void *This, int /*edx*/, int param_1, int param_2, int param_3, char param_4) {
    *(DWORD *)((int)This + 0x10) = 0x3cf5c28f;
    *(DWORD *)((int)This + 0x14) = 0xbf800000;
    *(float *)((int)This + 0xc) = -1.0f;
    FUN_004f9d60((float *)((int)This + 0xc));
    int iVar1 = FUN_00409f30(This, NULL, param_1, param_2, param_3, param_4);
    if (iVar1 != 0) {
        FUN_0040a300(This, NULL, param_1);
        operator_delete(*(void **)((int)This + 0x1c));
    }
}

// ── FUN_0040a110 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040a110 @ 0x0040A110 (~25 lines) — BMD: add triangle edge to shadow volume
// __thiscall(this=shadowCtx). Chequea la adyacencia en param_6 + param_4*0x24 + param_5*2 + 0x1A.
// Si la arista es de borde (adj == -1 o la cara adyacente mira hacia atrás), appendea un registro de arista (10 bytes)
// con 3 índices de vértice + 2 índices de vértice de arista al buffer de sombra en this+0x1C.
void __fastcall FUN_0040a110(void* ecx, void* /*edx*/, short param_1, short param_2, short param_3,
                             int param_4, int param_5, int param_6) {
    (void)ecx; (void)param_1; (void)param_2; (void)param_3;
    (void)param_4; (void)param_5; (void)param_6;
    // int slot = param_6 + param_4 * 0x24
    // short adj = *(short*)(slot + 0x1A + param_5*2)
    // if (adj == -1 || face[adj] mira hacia atrás):
    //   buf = this->shadowBuf + this->shadowCount * 10
    //   buf[0..1] = param_1, buf[2..3] = param_2, buf[4..5] = param_3
    //   buf[6..7] = edgeVtx0, buf[8..9] = edgeVtx1
    //   this->shadowCount++
}

// ── FUN_0040a1c0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040a1c0 @ 0x0040A1C0 (~70 lines) — BMD: backface cull + edge extraction for shadow volume
// __thiscall(this=shadowCtx, param_1=meshPartIdx, param_2=vertBuf, param_3=triCount, param_4=triData).
// Por cada triángulo: calcula la normal de la cara vía FaceNormalize y el producto punto con la dirección de la luz (this+0xC..0x14).
// Marca la cara como frontal (0) o trasera (1). Después, por cada triángulo frontal, llama a
// FUN_0040a110 tres veces (una por arista) para extraer las aristas de silueta del volumen de sombra.
void __fastcall FUN_0040a1c0(void* ecx, void* /*edx*/, short param_1, int param_2, short param_3, int param_4) {
    (void)ecx; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    // Por cada triángulo (param_3 iteraciones):
    //   Carga 3 vértices del vertBuf en (vtxIdx + param_1*15000) * 0xC
    //   FaceNormalize(v0, v1, v2, normal)
    //   dot = normal . lightDir (this+0xC,+0x10,+0x14)
    //   face_flag[tri] = (dot < threshold) ? 0 : 1
    // Por cada triángulo frontal:
    //   FUN_0040a110(edge0), FUN_0040a110(edge1), FUN_0040a110(edge2)
}

// ── FUN_0040a300 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040a300 @ 0x0040A300 (~110 lines) — BMD: build shadow volume geometry from silhouette edges
// __thiscall(this=shadowCtx, param_1=vertBuf). Allocates output vertex buffer (count * 0x48).
// Por cada arista de silueta: extruye los vértices de la arista en la dirección -luz (VectorMA),
// y arma 6 vértices (2 triángulos) por arista = un quad extruido. La altura se clampea a un mínimo.
void __fastcall FUN_0040a300(void* ecx, void* /*edx*/, int param_1) {
    (void)ecx; (void)param_1;
    // this->quadCount = 0
    // this->quadBuf = new byte[this->edgeCount * 0x48]
    // Por cada arista de silueta:
    //   v0 = vertBuf[(edge.vtx0 + edge.meshPart*15000) * 0xC]
    //   v1 = vertBuf[(edge.vtx1 + edge.meshPart*15000) * 0xC]
    //   Clamp v0.z, v1.z to minimum height (_DAT_00552584)
    //   extrude0 = v0 + (-height/this->extrudeScale) * lightDir vía VectorMA
    //   extrude1 = v1 + (-height/this->extrudeScale) * lightDir vía VectorMA
    //   Emit 6 vertices: v0, extrude0, v1, v1, extrude0, extrude1
}

// ── FUN_004236c0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_004236c0 @ 0x004236C0 (90 bytes) — BST node remove (unlink + delete)
void __fastcall FUN_004236c0(void *This, int /*edx*/, int *param_1) {
    int iVar1 = *(int *)(*param_1 + 0x10);
    if (iVar1 == 0) {
        *(int *)((int)This + 8) = 0;
    } else if (*(int *)(iVar1 + 8) == *param_1) {
        *(int *)(iVar1 + 8) = 0;
    } else {
        *(int *)(iVar1 + 0xc) = 0;
    }
    *(int *)((int)This + 4) = *(int *)((int)This + 4) - 1;
    if ((void *)*param_1 != NULL) {
        operator_delete((void *)*param_1);
    }
    *param_1 = 0;
}

// ── FUN_00423c30 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00423c30 @ 0x00423C30 (10 bytes) — disconnect socket context
void FUN_00423c30(void) { FUN_0043dc90(((int)(uintptr_t)DAT_055ca160)); }

// ── FUN_00423db0 — movida desde stubs_externs.cpp (refactor B3) ──
// FUN_00423db0 @ 0x00423DB0 — Net_ResetTrade: clears trade/shop globals, re-enables party markers.
void __cdecl FUN_00423db0(void) {
    DAT_05826d30 = 0;
    DAT_00559684 = 0xffffffff;
    lpString_05826bfc[0] = '\0';
    // Re-enable party target markers for all live entities
    unsigned char *puVar3 = (unsigned char*)(DAT_07abf5d0 + 0x2e9);
    char *pPlayer = DAT_07abf5d8;
    int iVar4 = 400;
    do {
        if (puVar3[-0x2e9] != '\0') {
            short sVar1 = *(short*)(pPlayer + 0x1da);
            *puVar3 = 0;
            if ((sVar1 != -1) && (*(short*)((char*)puVar3 - 0x10f) == sVar1))
                *puVar3 = 1;
        }
        puVar3 += 0x394;
        iVar4--;
    } while (iVar4 != 0);
}

// ── FUN_0043dc90 — movida desde stubs_linker.cpp (refactor B3) ──
// FUN_0043dc90 @ 0x0043DC90 (11 lines) — NetContext_Disconnect
// Clears connected flag, closes socket, invalidates handle.
void __cdecl FUN_0043dc90(int ctx) {
    DAT_055ca164 = 0; // g_bGameServerConnected
    closesocket(*(SOCKET *)(ctx + 8));
    *(int *)(ctx + 8) = -1; // INVALID_SOCKET
}

// ── FUN_0053cbb0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0053cbb0 @ 0x0053CBB0 (18 bytes) — GameGuard class init
void __fastcall FUN_0053cbb0(int *param_1) {
    *param_1 = (int)&PTR_FUN_0055389c;
    FUN_0053cc00_impl((int)param_1);
}

// ── FUN_0053cbd0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0053cbd0 @ 0x0053CBD0 — GameGuard ~dtor
void __fastcall FUN_0053cbd0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0053cbf0((int *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_0053cbf0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0053cbf0 @ 0x0053CBF0 (7 bytes)
void __fastcall FUN_0053cbf0(int *param_1) { *param_1 = (int)&PTR_FUN_0055389c; }

// ── FUN_0053ce30 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0053ce30 @ 0x0053CE30 (350 bytes) — Packet crypto decrypt block
// 4-round Feistel cipher: extracts 16-bit + 2-bit fields via FUN_0053cf90,
// applies reverse XOR chain using context keys (this+0x04..0x3c),
// y después multiplica-módulo + XOR de salida. Devuelve el byte de checksum o 0xFFFFFFFF si falla.
unsigned int __cdecl FUN_0053ce30(void *self, unsigned short *param_1, int param_2) {
    (void)self; (void)param_2;
    if (param_1) { param_1[0] = 0; param_1[1] = 0; param_1[2] = 0; param_1[3] = 0; }
    return 0;
}

// ── FUN_0053cf90 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0053cf90 @ 0x0053CF90 (305 bytes) — bit-field insert.
// Copia el rango de bits [a4..a4+a5) del buffer de origen (a3) al destino (a1)
// en el offset de bit a2. Usa un buffer temporal + FUN_0053d0d0 para el corrimiento entre bytes.
// Ported from IDA sub_53CF90.
int __stdcall FUN_0053cf90(int a1, unsigned int a2, int a3, unsigned int a4, int a5)
{
    int  lastByteIdx = FUN_0053d170((int)a4 + a5 - 1);
    int  firstByteIdx= FUN_0053d170((int)a4);
    unsigned int v6  = (unsigned int)(1 + lastByteIdx - firstByteIdx);

    // Aloca un scratch con 1 byte extra para que el corrimiento a la derecha de abajo no
    // desborde cuando se corren v6 + 1 bytes.
    char *lpMem = (char *)operator_new(v6 + 1);
    memset(lpMem, 0, v6 + 1);
    memcpy(lpMem, (const void *)(a3 + firstByteIdx), v6);

    int tail = (a4 + a5) % 8;
    if (tail) {
        lpMem[v6 - 1] = (char)(lpMem[v6 - 1] & (-1 << (8 - tail)));
    }

    // Alinea a la izquierda los bits extraídos al inicio del buffer scratch,
    // y después los alinea a la derecha al offset de bit del destino.
    FUN_0053d0d0((BYTE *)lpMem, v6,     -(a4 % 8));
    FUN_0053d0d0((BYTE *)lpMem, v6 + 1,  (a2 % 8));

    int    nBytes = (int)v6 + ((a2 % 8) > (a4 % 8) ? 1 : 0);
    BYTE  *v9     = (BYTE *)(a1 + FUN_0053d170(a2));
    if (nBytes > 0) {
        int off = (int)((BYTE *)lpMem - v9);
        do {
            *v9 = (BYTE)(*v9 | v9[off]);
            ++v9;
            --nBytes;
        } while (nBytes);
    }
    delete__(lpMem);
    return a2 + a5;
}

// ── FUN_0053d0d0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0053d0d0 @ 0x0053D0D0 (155 bytes) — Byte-array bitwise shift for crypto.
// Positive a3 → right-shift; negative a3 → left-shift.  Shifts bits across
// adjacent bytes (1-byte ripple).  Ported from IDA sub_53D0D0.
void __stdcall FUN_0053d0d0(unsigned char *a1, int a2, int a3)
{
    if (a3 == 0) return;
    if (a3 > 0) {
        // right-shift by a3 bits
        if (a2 - 1 > 0) {
            int      v6 = a2 - 1;
            BYTE    *v5 = &a1[a2 - 1];
            while (v6 > 0) {
                *v5 = (BYTE)((*v5 >> a3) | (a1[v6 - 1] << (8 - a3)));
                --v5;
                if (--v6 <= 0) break;
            }
            *v5 = (BYTE)(*v5 >> a3);
        } else {
            a1[0] = (BYTE)(a1[0] >> a3);
        }
    } else {
        // left-shift by |a3| bits
        int   v8 = -a3;
        BYTE *v7 = a1;
        if (a2 - 1 > 0) {
            int v9 = a2 - 1;
            do {
                *v7 = (BYTE)((*v7 << v8) | (v7[1] >> (a3 + 8)));
                ++v7;
                --v9;
            } while (v9);
        }
        *v7 = (BYTE)(*v7 << v8);
    }
}

// ── FUN_0053d170 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 10-byte: FUN_0053d170 — shift right 3 ───────────────────────────────────

// FUN_0053d170 @ 0x0053D170 (10 bytes)
// FUN_0053d170 (IDA-activated, was Ghidra stub)
int __cdecl FUN_0053d170(int a1)
{
  return a1 >> 3;
}

// ── FUN_0053d1c0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0053d1c0 @ 0x0053D1C0 (381 bytes) — Crypto key file loader.
// Reads a CSimpleModulus key file:
//   header = [int16 magic][int32 expected_size]
// y después hasta 4 grupos de clave de 16 bytes, con XOR contra DAT_00562e48, en el
// objeto destino en los offsets +4, +20, +36 y +52. Los flags a4..a7 eligen
// cuál de los cuatro grupos trae el archivo (LoadEncryptionKey pasa
// 1,1,0,1 → Mod,Enc,skip Dec,Xor; LoadDecryptionKey passes 1,0,1,1 →
// Mod, saltea Enc, Dec, Xor). Devuelve 1 si tuvo éxito, 0 ante cualquier error de E/S o de formato.
int __cdecl FUN_0053d1c0(void *self, const char *filename, short magic,
                          int a4, int a5, int a6, int a7)
{
    DWORD *_this = (DWORD *)self;
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    BYTE hdrBuf[6] = {0};
    DWORD nRead = 0;
    if (!ReadFile(hFile, hdrBuf, 6, &nRead, NULL) || nRead != 6) {
        CloseHandle(hFile);
        return 0;
    }
    short mg = (short)(hdrBuf[0] | (hdrBuf[1] << 8));
    int   sz = (int)(hdrBuf[2] | (hdrBuf[3] << 8) |
                     (hdrBuf[4] << 16) | (hdrBuf[5] << 24));
    int expected = 16 * (a4 + a5 + a6 + a7) + 6;
    if (mg != magic || sz != expected) {
        CloseHandle(hFile);
        return 0;
    }

    DWORD buf[4];
    char dbg[256];
    _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
        "CSM load %s: XORkey=[%08X %08X %08X %08X] thisAddr=%p",
        filename, DAT_00562e48[0], DAT_00562e48[1], DAT_00562e48[2], DAT_00562e48[3], _this);
    DbgLogPublic(dbg);
    if (a4) {  // ModKey → this+4
        ReadFile(hFile, buf, 16, &nRead, NULL);
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "  ModKey raw=[%08X %08X %08X %08X]", buf[0],buf[1],buf[2],buf[3]);
        DbgLogPublic(dbg);
        for (int i = 0; i < 4; ++i) _this[1 + i] = buf[i] ^ DAT_00562e48[i];
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "  ModKey dec=[%08X %08X %08X %08X]",
            _this[1],_this[2],_this[3],_this[4]);
        DbgLogPublic(dbg);
    }
    if (a5) {  // EncKey → this+20
        ReadFile(hFile, buf, 16, &nRead, NULL);
        for (int i = 0; i < 4; ++i) _this[5 + i] = buf[i] ^ DAT_00562e48[i];
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "  EncKey dec=[%08X %08X %08X %08X]",
            _this[5],_this[6],_this[7],_this[8]);
        DbgLogPublic(dbg);
    }
    if (a6) {  // DecKey → this+36
        ReadFile(hFile, buf, 16, &nRead, NULL);
        for (int i = 0; i < 4; ++i) _this[9 + i] = buf[i] ^ DAT_00562e48[i];
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "  DecKey dec=[%08X %08X %08X %08X]",
            _this[9],_this[10],_this[11],_this[12]);
        DbgLogPublic(dbg);
    }
    if (a7) {  // XorKey → this+52
        ReadFile(hFile, buf, 16, &nRead, NULL);
        for (int i = 0; i < 4; ++i) _this[13 + i] = buf[i] ^ DAT_00562e48[i];
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "  XorKey dec=[%08X %08X %08X %08X]",
            _this[13],_this[14],_this[15],_this[16]);
        DbgLogPublic(dbg);
    }
    CloseHandle(hFile);
    return 1;
}

// -- FUN_0053cc00_impl: helper local, movido desde stubs_bulk_small.cpp (B3) --
static void __cdecl FUN_0053cc00_impl(int param_1) {
    *(int *)(param_1 + 0x14) = 0; *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x1c) = 0; *(int *)(param_1 + 0x20) = 0;
    *(int *)(param_1 + 4) = 0;    *(int *)(param_1 + 8) = 0;
    *(int *)(param_1 + 0xc) = 0;  *(int *)(param_1 + 0x10) = 0;
    *(int *)(param_1 + 0x24) = 0; *(int *)(param_1 + 0x28) = 0;
    *(int *)(param_1 + 0x2c) = 0; *(int *)(param_1 + 0x30) = 0;
}
