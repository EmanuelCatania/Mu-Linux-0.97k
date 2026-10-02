// Scene_MapTick.cpp
// Per-frame map/tile update and walk-mode tile dispatch.
//
// Scene_MapTick @ 0x004f64d0 — Scene_MapTick
//
// Manages the map-tile hash table for the current player position.
// Uses HashTable at MAIN_HASH_CLASS and player position at DAT_07cf1ffc.
// Dispatches to RenderItemInfo (walk map) or RenderRepairInfo (alt map)
// based on boundary conditions.

#include "stdafx.h"


// Scene_MapTick — Scene_MapTick
// Updates the tile hash table entry for the local player's tile position.
// If the player has moved outside the current map tile bounds, reloads the map:
//   - Checks bounds: x in [DAT_07ea5284..0x27f], y in [DAT_07ea5288..0x1df]
//   - Calls RenderItemInfo or RenderRepairInfo to reload walk/alt map
// Also handles DAT_0055a3e4 as a pending map-change request.
void Scene_MapTick(void)
{
  uint uVar2;
  void *pvVar3;
  undefined4 *puVar5;
  // (was: int iStack0000000c — phantom outgoing-stack arg slot; resolved into RenderSkillTooltip 3rd param)

  GL_ResetState();
  puVar5 = (undefined4*)DAT_07cf1ffc;
  DAT_07e11d6e = 0;
  // IDA Scene_MapTick L32-38: RepairEnable_0 sólo se NORMALIZA a 1; nunca se
  // apaga acá.  El port tenía (2026-05-08) un "fix" que lo ponía en 0 cada
  // frame si RepairEnable (0x07EAA138) valía 0 -- y RepairEnable vale 0
  // siempre que la tienda está abierta (sub_4E6550), así que el modo reparación
  // del herrero duraba un frame y el click levantaba el item.
  if (DAT_07eaa134 != 0) {
    DAT_07eaa134 = 1;
  }
  if (DAT_07eaa160 == 0) {
    if ((((DAT_07eaa13c == 1) && (_DAT_00552cac <= (float)DAT_083a427c)) &&
        ((float)DAT_083a427c < _DAT_00552c24)) &&
       ((_DAT_00552c14 <= (float)DAT_083a4278 && ((float)DAT_083a4278 < _DAT_00552ca8)))) {
      uVar2 = HashTable_GetIndex(&MAIN_HASH_CLASS,DAT_07cf1ffc);
      if (uVar2 == 0xffffffff) {
        pvVar3 = AntiTamper_HashNode();
        *(undefined1 *)((int)pvVar3 + 0x584) = 1;
        HashTable_Insert(&MAIN_HASH_CLASS,pvVar3,puVar5);
      }
      RenderItemInfo((void*)0x109,(void*)0x78,DAT_07e91350,'\0');
      puVar5 = (undefined4*)DAT_07cf1ffc;
    }
    goto LAB_004f6824;
  }
  if (((DAT_083a427c < DAT_07ea5288) || (0x27f < DAT_083a427c)) ||
     ((DAT_083a4278 < DAT_07ea5284 || ((0x1df < DAT_083a4278 || (DAT_07eaa134 == 0)))))) {
    uVar2 = HashTable_GetIndex(&MAIN_HASH_CLASS,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) {
      pvVar3 = AntiTamper_HashNode();
      *(undefined1 *)((int)pvVar3 + 0x584) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS,pvVar3,puVar5);
    }
    RenderItemInfo((void*)(uintptr_t)DAT_07ea840c,(void*)(uintptr_t)DAT_07ea8408,(void*)(uintptr_t)DAT_07eaa160,(int)DAT_07ea9844);
    puVar5 = (undefined4*)DAT_07cf1ffc;
  }
  else {
    uVar2 = HashTable_GetIndex(&MAIN_HASH_CLASS,DAT_07cf1ffc);
    if (uVar2 == 0xffffffff) {
      pvVar3 = AntiTamper_HashNode();
      *(undefined1 *)((int)pvVar3 + 0x584) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS,pvVar3,puVar5);
    }
    RenderRepairInfo((void*)(uintptr_t)DAT_07ea840c,(int)DAT_07ea8408,(void*)(uintptr_t)DAT_07eaa160);
    puVar5 = (undefined4*)DAT_07cf1ffc;
  }
LAB_004f6824:
  // 2026-05-05: clamp DAT_0055a3e4 to valid skill slot range (0..19) before
  // calling tooltip. Otherwise garbage values like 0x2A2A cause OOB reads
  // inside the tooltip code that crash on hover.
  if ((int)DAT_0055a3e4 >= 0 && (int)DAT_0055a3e4 < 20) {
    // Skill tooltip render. Args resolved from IDA: (float Y, int X, int hoveredSkillIdx).
    // Ghidra mis-decoded the 3rd arg as a stack write; it is the real 3rd param.
    RenderSkillTooltip(*(float*)&DAT_07ea840c, (int)DAT_07ea8408, (int)DAT_0055a3e4);
  }
  return;
}
