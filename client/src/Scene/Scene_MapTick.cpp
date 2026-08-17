// Scene_MapTick.cpp
// Per-frame map/tile update and walk-mode tile dispatch.
//
// FUN_004f64d0 @ 0x004f64d0 — Scene_MapTick
//
// Manages the map-tile hash table for the current player position.
// Uses HashTable at DAT_055c9bc8 and player position at DAT_07cf1ffc.
// Dispatches to FUN_004c4650 (walk map) or FUN_004c8d70 (alt map)
// based on boundary conditions.

#include "stdafx.h"


// FUN_004f64d0 — Scene_MapTick
// Updates the tile hash table entry for the local player's tile position.
// If the player has moved outside the current map tile bounds, reloads the map:
//   - Checks bounds: x in [DAT_07ea5284..0x27f], y in [DAT_07ea5288..0x1df]
//   - Calls FUN_004c4650 or FUN_004c8d70 to reload walk/alt map
// Also handles DAT_0055a3e4 as a pending map-change request.
void FUN_004f64d0(void)
{
  char cVar1;
  uint uVar2;
  void *pvVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  // (was: int iStack0000000c — phantom outgoing-stack arg slot; resolved into FUN_004c9730 3rd param)

  FUN_00511600();
  puVar5 = (undefined4*)DAT_07cf1ffc;
  DAT_07e11d6e = 0;
  // 2026-05-08: BUG-FIX — DAT_07eaa134 (B-key/repair-mode flag) se queda
  // pegada en != 0 (probablemente corrupción adicional similar a
  // DAT_07d78068). Eso dispara el dispatch a FUN_004c8d70 (RenderRepairInfo)
  // en lugar de FUN_004c4650 (tooltip normal), y el cursor (FUN_004bffa0)
  // dibuja el sprite de reparación en vez de flecha.
  // Hasta encontrar el writer real, forzamos a 0 si no hay un repair NPC
  // efectivamente abierto (DAT_07eaa138 = RepairEnable counter).
  if (DAT_07eaa134 != 0 && DAT_07eaa138 == 0) {
    DAT_07eaa134 = 0;
  } else if (DAT_07eaa134 != 0) {
    DAT_07eaa134 = 1;
  }
  if (DAT_07eaa160 == 0) {
    if ((((DAT_07eaa13c == 1) && (_DAT_00552cac <= (float)DAT_083a427c)) &&
        ((float)DAT_083a427c < _DAT_00552c24)) &&
       ((_DAT_00552c14 <= (float)DAT_083a4278 && ((float)DAT_083a4278 < _DAT_00552ca8)))) {
      uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
      if (uVar2 == 0xffffffff) {
        pvVar3 = operator_new(0x585);
        *(undefined1 *)((int)pvVar3 + 0x584) = 1;
        FUN_00403f80(&DAT_055c9bc8,pvVar3,puVar5);
      }
      else {
        puVar4 = (undefined4 *)FUN_00404280(&DAT_055c9bc8,puVar5);
        cVar1 = *(char *)(puVar4 + 0x161);
        *(byte *)(puVar4 + 0x161) = cVar1 + 1U;
        if ((byte)(cVar1 + 1U) < 2) {
          FUN_00404370(puVar5,puVar4);
        }
      }
      FUN_004c4650((void*)0x109,(void*)0x78,DAT_07e91350,'\0');
      puVar5 = (undefined4*)DAT_07cf1ffc;
      uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
      if (uVar2 != 0xffffffff) {
        puVar4 = (undefined4 *)FUN_00404280(&DAT_055c9bc8,puVar5);
        cVar1 = *(char *)(puVar4 + 0x161);
        *(char *)(puVar4 + 0x161) = cVar1 + -1;
        if ((char)(cVar1 + -1) == '\0') {
          FUN_00404400(puVar4,puVar5);
        }
      }
    }
    goto LAB_004f6824;
  }
  if (((DAT_083a427c < DAT_07ea5288) || (0x27f < DAT_083a427c)) ||
     ((DAT_083a4278 < DAT_07ea5284 || ((0x1df < DAT_083a4278 || (DAT_07eaa134 == 0)))))) {
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) {
      pvVar3 = operator_new(0x585);
      *(undefined1 *)((int)pvVar3 + 0x584) = 1;
      FUN_00403f80(&DAT_055c9bc8,pvVar3,puVar5);
    }
    else {
      uVar2 = HashTable_GetIndex(&DAT_055c9bc8,puVar5);
      if (uVar2 == 0xffffffff) {
        puVar4 = (undefined4 *)0x0;
      }
      else {
        puVar4 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
      }
      cVar1 = *(char *)(puVar4 + 0x161);
      *(byte *)(puVar4 + 0x161) = cVar1 + 1U;
      if ((byte)(cVar1 + 1U) < 2) {
        FUN_00404370(puVar5,puVar4);
      }
    }
    FUN_004c4650((void*)(uintptr_t)DAT_07ea840c,(void*)(uintptr_t)DAT_07ea8408,(void*)(uintptr_t)DAT_07eaa160,(int)DAT_07ea9844);
    puVar5 = (undefined4*)DAT_07cf1ffc;
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) goto LAB_004f6824;
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,puVar5);
    if (uVar2 == 0xffffffff) goto LAB_004f6614;
    puVar4 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
  }
  else {
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) {
      pvVar3 = operator_new(0x585);
      *(undefined1 *)((int)pvVar3 + 0x584) = 1;
      FUN_00403f80(&DAT_055c9bc8,pvVar3,puVar5);
    }
    else {
      uVar2 = HashTable_GetIndex(&DAT_055c9bc8,puVar5);
      if (uVar2 == 0xffffffff) {
        puVar4 = (undefined4 *)0x0;
      }
      else {
        puVar4 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
      }
      cVar1 = *(char *)(puVar4 + 0x161);
      *(byte *)(puVar4 + 0x161) = cVar1 + 1U;
      if ((byte)(cVar1 + 1U) < 2) {
        FUN_00404370(puVar5,puVar4);
      }
    }
    FUN_004c8d70((void*)(uintptr_t)DAT_07ea840c,(int)DAT_07ea8408,(void*)(uintptr_t)DAT_07eaa160);
    puVar5 = (undefined4*)DAT_07cf1ffc;
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) goto LAB_004f6824;
    uVar2 = HashTable_GetIndex(&DAT_055c9bc8,puVar5);
    if (uVar2 == 0xffffffff) {
LAB_004f6614:
      puVar4 = (undefined4 *)0x0;
    }
    else {
      puVar4 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
    }
  }
  cVar1 = *(char *)(puVar4 + 0x161);
  *(char *)(puVar4 + 0x161) = cVar1 + -1;
  if ((char)(cVar1 + -1) == '\0') {
    FUN_00404400(puVar4,puVar5);
  }
LAB_004f6824:
  // 2026-05-05: clamp DAT_0055a3e4 to valid skill slot range (0..19) before
  // calling tooltip. Otherwise garbage values like 0x2A2A cause OOB reads
  // inside the tooltip code that crash on hover.
  if ((int)DAT_0055a3e4 >= 0 && (int)DAT_0055a3e4 < 20) {
    // Skill tooltip render. Args resolved from IDA: (float Y, int X, int hoveredSkillIdx).
    // Ghidra mis-decoded the 3rd arg as a stack write; it is the real 3rd param.
    FUN_004c9730(*(float*)&DAT_07ea840c, (int)DAT_07ea8408, (int)DAT_0055a3e4);
  }
  return;
}
