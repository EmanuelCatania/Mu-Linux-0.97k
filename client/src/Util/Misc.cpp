// Misc.cpp
// Miscellaneous utility and validation functions.
//
// Packet_IsValidSockType @ 0x00406b10 — Packet_IsValidSockType
// CheckSpecialText @ 0x00406B30 (IDA: FUN_00406B30; name from 5.2).
// CPhysicsManager::Move @ 0x00409C40 (IDA: FUN_00409C40).
// CPhysicsManager::Render @ 0x00409CF0 (IDA: FUN_00409CF0).
// Object_SetRectFields @ 0x0040c690 — Object_SetRectFields
// Object_ClearMembers @ 0x0040e590 — Object_ClearMembers
// Packet_ParseReceived @ 0x00402fd0 — Packet_ParseReceived
// MoveItems @ 0x00503760 — Entity_UpdateGravity

#include "stdafx.h"


// Packet_IsValidSockType — Packet_IsValidSockType
// Returns 1 if param_2 is socket type 1 (TCP) or 0xf (UDP-compatible),
// otherwise returns 0.
int __cdecl Packet_IsValidSockType(int param_1,int param_2)
{
  if ((param_2 != 1) && (param_2 != 0xf)) {
    return 0;
  }
  return 1;
}


// CheckSpecialText — returns true when the text contains a disallowed character.
// Scans string param_1 for characters invalid in player names/chat.
// Returns 1 (non-zero low byte) if an invalid character is found,
// 0 (in the high byte return) if all characters are valid.
// Also handles DBCS (double-byte) character sequences.
bool __cdecl CheckSpecialText(const byte *param_1)
{
  byte bVar1;
  byte bVar2;
  int  cVar3;
  int iVar4;

  bVar1 = *param_1;
  do {
    if (bVar1 == 0) {
      return 0; // valid: no invalid chars found
    }
    cVar3 = mbclen(param_1);
    iVar4 = cVar3;
    if (iVar4 == 1) {
      bVar1 = *param_1;
      iVar4 = (int)bVar1;
      if ((((bVar1 < 0x30) || ((0x39 < bVar1 && (bVar1 < 0x41)))) ||
          ((0x5a < bVar1 && (bVar1 < 0x61)))) || (0x7a < bVar1)) goto LAB_00406bbe;
    }
    else {
      bVar1 = *param_1;
      if ((bVar1 < 0x81) || (200 < bVar1)) {
LAB_00406bbe:
        return 1; // found invalid character
      }
      bVar2 = param_1[1];
      iVar4 = (int)bVar2;
      if (((((bVar2 < 0x41) || (0x5a < bVar2)) && ((bVar2 < 0x61 || (0x7a < bVar2)))) &&
          ((bVar2 < 0x81 || (bVar2 == 0xff)))) ||
         (((((0xa0 < bVar1 && (bVar1 < 0xb0)) && (0xa0 < bVar2)) ||
           (((bVar1 == 0xc6 && (0x52 < bVar2)) && (bVar2 < 0xa1)))) ||
          ((0xc6 < bVar1 && (bVar2 < 0xa1)))))) goto LAB_00406bbe;
      param_1 = param_1 + 1;
    }
    bVar1 = param_1[1];
    param_1 = param_1 + 1;
  } while( true );
}


// CPhysicsManager::Move — advances every registered physics object.
// Updates the floating-point random key _DAT_00590af0 with a random delta
// in ±0.1 range, clamped to [-0.2, 1.0].
// Then iterates a linked list from *(param_1+8)+8, calling Sound_UpdateChannel3D_Tick
// on each node until sentinel *(param_1+0xc) is reached.
void __cdecl CPhysicsManager_Move(void* physics_manager, float fTime)
{
  const int param_1 = (int)(uintptr_t)physics_manager;
  undefined4 *puVar1;
  int iVar2;

  iVar2 = _rand();
  _DAT_00590af0 = _DAT_00590af0 + (float)(iVar2 % 200 + -100) * _DAT_00552500;
  if (_DAT_00590af0 < _DAT_00552570) {
    _DAT_00590af0 = -0.2;
  }
  else if (_DAT_0055256c <= _DAT_00590af0) {
    _DAT_00590af0 = 1.0;
  }
  puVar1 = *(undefined4 **)(*(int *)(param_1 + 8) + 8);
  if (puVar1 != *(undefined4 **)(param_1 + 0xc)) {
    do {
      if (puVar1 == (undefined4 *)0x0) {
        return;
      }
      Sound_UpdateChannel3D_Tick((int *)*puVar1, fTime);
      puVar1 = (undefined4 *)puVar1[2];
    } while (*(undefined4 **)(param_1 + 0xc) != puVar1);
  }
  return;
}


// CPhysicsManager::Render — renders every registered physics object.
void __cdecl CPhysicsManager_Render(void* physics_manager)
{
  const int param_1 = (int)(uintptr_t)physics_manager;
  undefined4 *puVar1;

  puVar1 = *(undefined4 **)(*(int *)(param_1 + 8) + 8);
  if (puVar1 != *(undefined4 **)(param_1 + 0xc)) {
    do {
      if (puVar1 == (undefined4 *)0x0) {
        return;
      }
      // IDA: vtable[3](*i, 0) -- thiscall; se llama como __fastcall(ecx, edx).
      void* obj = (void*)(uintptr_t)*puVar1;
      (*(void (__fastcall **)(void*, void*, int))(*(int *)obj + 0xc))(obj, nullptr, 0);
      puVar1 = (undefined4 *)puVar1[2];
    } while (*(undefined4 **)(param_1 + 0xc) != puVar1);
  }
  return;
}


// Object_SetRectFields — Object_SetRectFields
// Sets two consecutive fields at this+0x2c and this+0x30.
void __cdecl Object_SetRectFields(void *this_,undefined4 param_1,undefined4 param_2)
{
  *(undefined4 *)((int)this_ + 0x2c) = param_1;
  *(undefined4 *)((int)this_ + 0x30) = param_2;
  return;
}


// Object_ClearMembers — Object_ClearMembers
// Zeroes 0x140 dwords (1280 bytes) starting at param_1 + 200 (0xc8).
void __cdecl Object_ClearMembers(int param_1)
{
  int iVar1;
  undefined4 *puVar2;

  puVar2 = (undefined4 *)(param_1 + 200);
  for (iVar1 = 0x140; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  return;
}


// Packet_ParseReceived — Packet_ParseReceived
// Dispatches an incoming packet from param_1 based on the sub-type byte
// at param_1+0x1c87f. Type 1: FUN_00402850. Type 2: Packet_ParseType2.
// Returns 1 (non-zero) on both paths.
undefined4 __cdecl Packet_ParseReceived(void *param_1)
{
  int iVar1;
  undefined4 uVar2;

  if (*(byte *)((int)param_1 + 0x1c87f) == 1) {
    iVar1 = FUN_00402850(param_1);
    (void)iVar1;
  }
  else {
    iVar1 = *(byte *)((int)param_1 + 0x1c87f) - 2;
    if (iVar1 == 0) {
      uVar2 = Packet_ParseType2(param_1);
      (void)uVar2;
      return 1;
    }
  }
  return 1;
}


// IDA: MoveItems (0x00503760)
// Iterates the entity-gravity pool (per-slot offset +0x18 inside the
// 1000-slot ground-items pool DAT_07e12840, stride 0x204). Per active slot:
// advances Z by velocity, decays velocity by _DAT_005527d0. Checks terrain
// height via RequestTerrainHeight; if entity is above terrain + offset, adjusts Y or
// Z velocity. Calls ItemAngle and Entity_UpdateSparkleEffect (FUN_00503650).
//
// 2026-05-08: AUTO-SKIP removed. Walker now uses the properly-sized pool
// `DAT_07e12840` (1000 × 0x204) with an explicit slot count instead of the
// literal end-bound `< 0x7e907f8`. Per-slot pfVar2 = slot_base + 0x18 (the
// gravity-field anchor that the orphan DAT_07e12858 used to alias).
void MoveItems(void)
{
  // 2026-08-21: el walker estaba corrido 72 bytes.  Tomaba `DAT_07e12840` como
  // si fuera `Items + 72` (leía el flag activo en slot+0), pero en nuestro build
  // ese símbolo ES la base del item — es lo que asumen Net_Process (0x20) y
  // Entity_Render (que escriben/leen active en ip+72).  Resultado: el flag activo
  // salía siempre 0 y la función no hacía NADA: los items no caían al suelo, no
  // giraban al caer y no soltaban destellos.
  //
  // IDA MoveItems (0x503760) trabaja sobre `v0 = &Items[0][96]` (la Z), así que
  // los offsets equivalentes desde la base del item son:
  //   ip+72  active   ·  ip+74  modelo  ·  ip+88/92  X,Y  ·  ip+96  Z
  //   ip+100/104  Angle[0]/Angle[1]     ·  ip+288  velocidad Z
  for (int slotIdx = 0; slotIdx < 1000; ++slotIdx) {
    unsigned char *ip = (unsigned char *)DAT_07e12840 + slotIdx * 0x204;
    if (ip[72] == 0) continue;

    float *pZ  = (float *)(ip + 96);
    float *pVz = (float *)(ip + 288);

    *pZ  = *pVz + *pZ;
    *pVz = *pVz - _DAT_005527d0;

    float10 fVar3 = (float10)RequestTerrainHeight(*(float *)(ip + 88), *(float *)(ip + 92));
    short  sVar1  = *(short *)(ip + 74);
    fVar3 = fVar3 + (float10)_DAT_0055284c;
    if ((399 < sVar1) && (sVar1 < 0x250)) {
      fVar3 = fVar3 + (float10)_DAT_005528e4;
    }
    if (fVar3 < (float10)*pZ) {
      // Todavía en el aire: gira mientras cae.
      if ((sVar1 < 0x250) || (0x26f < sVar1)) {
        *(float *)(ip + 100) = *pVz * _DAT_00552a28;
      }
      else {
        *(float *)(ip + 104) = *pVz * _DAT_00552a28;
      }
    }
    else {
      // Tocó el suelo: se apoya sobre el terreno.
      *pZ = (float)fVar3;
      ItemAngle((int)(ip + 72));
    }
    Entity_UpdateSparkleEffect((int)(ip + 72));
  }
}
