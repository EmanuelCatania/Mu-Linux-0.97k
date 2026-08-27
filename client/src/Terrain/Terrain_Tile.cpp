// Terrain_Tile.cpp
// Tile attribute flag operations on the 256×256 tile map at DAT_0838bc70.
//
// IDA: FUN_004f6ef0 @ 0x004f6ef0 — Terrain_SetTileAttributeBits
// IDA: FUN_004f6f10 @ 0x004f6f10 — Terrain_ClearTileAttributeBits
// IDA: FUN_004f6f30 @ 0x004f6f30 — Terrain_UpdateTileAttributeRect
//
// The tile map is a flat byte[256][256] array.
// Each byte holds per-tile attribute flags (walkable, safe zone, etc.).
// Access: map[y * 256 + x]

#include "stdafx.h"


// IDA: FUN_004f6ef0 — Terrain_SetTileAttributeBits
// Sets flag bits param_3 at tile (param_1, param_2).
void __cdecl Terrain_SetTileAttributeBits(int param_1,int param_2,int param_3)
{
  DAT_0838bc70[param_2 * 0x100 + param_1] = (char)(DAT_0838bc70[param_2 * 0x100 + param_1] | (byte)param_3);
  return;
}


// IDA: FUN_004f6f10 — Terrain_ClearTileAttributeBits
// Clears flag bits param_3 at tile (param_1, param_2).
void __cdecl Terrain_ClearTileAttributeBits(int param_1,int param_2,int param_3)
{
  DAT_0838bc70[param_2 * 0x100 + param_1] = (char)(DAT_0838bc70[param_2 * 0x100 + param_1] & ~(byte)param_3);
  return;
}


// IDA: FUN_004f6f30 — Terrain_UpdateTileAttributeRect
// Sets or clears flag param_5 across a rectangular region of tiles.
// Region: x in [param_1, param_1+param_3), y in [param_2, param_2+param_4).
// param_6 == 0: clear flags; param_6 != 0: set flags.
void __cdecl Terrain_UpdateTileAttributeRect(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6)
{
  int iVar1;
  int iVar2;

  if (0 < param_4) {
    do {
      iVar1 = param_3;
      iVar2 = param_1;
      if (0 < param_3) {
        do {
          if (param_6 == '\0') {
            Terrain_ClearTileAttributeBits(iVar2,param_2,param_5);
          }
          else {
            Terrain_SetTileAttributeBits(iVar2,param_2,param_5);
          }
          iVar1 = iVar1 + -1;
          iVar2 = iVar2 + 1;
        } while (iVar1 != 0);
      }
      param_2 = param_2 + 1;
      param_4 = param_4 + -1;
    } while (param_4 != 0);
  }
  return;
}
