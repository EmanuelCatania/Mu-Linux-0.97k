// Terrain_Utils.cpp
// FUN_004f6c40 @ 0x004f6c40  — Grid_TileIndex
// FUN_004f7500 @ 0x004f7500  — Terrain_HeightAt
//
// Grid_TileIndex:
//   Converts (x, y) grid coordinates to a flat tile index:
//   index = (y & 0xFF) * 256 + (x & 0xFF)
//
// Terrain_HeightAt:
//   Bilinear-interpolates the terrain height at a given (x, y) position.
//   Height map is a 256x256 float array at DAT_080cb2cc.
//   Only active in game state 5 (in-game); returns 0.0 otherwise.
//   Parameters arrive on the x87 FPU stack (extraout_ST0 = x frac,
//   extraout_ST1 = y frac); Ghidra cannot represent this in the
//   function signature.
//
// Globals:
//   DAT_005615c0 — current game state (5 = in-game)
//   DAT_080cb2cc — terrain height map float[256][256]
//   _DAT_00552580 — float constant 0.0

#include "stdafx.h"

int __cdecl FUN_004f6c40(uint param_1,uint param_2)

{
  return (param_2 & 0xff) * 0x100 + (param_1 & 0xff);
}


// FUN_004f7500 @ 0x004F7500 — RequestTerrainHeight(xf, yf)
// Per IDA decomp (raw/004F7500_RequestTerrainHeight.c, 177 bytes).
//
// Bilinear interpolation of terrain height at world (xf, yf).
//
// BUG-FIX 2026-04-28: el Ghidra decomp inferiría argumentos vía x87 FPU stack
// (`__ftol()` lee ST0/ST1) — eso solo funciona si el caller compiló con x87,
// pero MSVC en Release usa SSE/SSE2 → ftol leía basura → return 0 → todos los
// hero/entity quedaban con z=0 (heroPos.z=0.0 en el log). Cambiamos la firma a
// (xf, yf) explícitos como el IDA original y actualizamos los call-sites.
float __cdecl FUN_004f7500(float xf, float yf)
{
    // BUG-FIX 2026-04-28: el IDA original tiene guard `if (g_GameState != 5)`
    // pero Recv_JoinMapServer llama CreateCharacterPointer ANTES de que el state
    // pase a 5 (en MuEmu el F3/03 llega rápido y el state machine está aún en 3
    // o 4). Resultado: hero.z spawn = 0. Relajamos el guard — ahora es seguro
    // mientras el world esté cargado (DAT_0055a7ac válido y DAT_080cb2cc con
    // height map real). Si el array está en 0 retornamos 0 (mismo resultado).
    if ((int)DAT_0055a7ac < 0) return 0.0f;

    float gx = xf * 0.01f;
    float gy = yf * 0.01f;
    int   ix = (int)gx;
    int   iy = (int)gy;
    int   ix1 = (ix + 1) & 0xff;
    int   iy1 = (iy + 1) & 0xff;
    int   ix0 = ix & 0xff;
    int   iy0 = iy & 0xff;
    float fx = gx - (float)ix;
    float fy = gy - (float)iy;

    // Bilinear: lerp(lerp(h00, h01, fy), lerp(h10, h11, fy), fx)
    float h00 = DAT_080cb2cc[iy0 * 0x100 + ix0];
    float h10 = DAT_080cb2cc[iy0 * 0x100 + ix1];
    float h01 = DAT_080cb2cc[iy1 * 0x100 + ix0];
    float h11 = DAT_080cb2cc[iy1 * 0x100 + ix1];

    float a = (h01 - h00) * fy + h00;
    float b = (h11 - h10) * fy + h10;
    return (b - a) * fx + a;
}


// FUN_004f8ff0 @ 0x004f8ff0
//
// Terrain_PointInQuad — tests whether a 2D world point (param_1, param_2)
// lies inside (or on the boundary of) the current terrain quad, using a
// signed-area (cross-product) test against param_3.
//
// The terrain quad is stored as 4 projected vertices in DAT_07eeb228 (X) and
// DAT_07eeb218 (Y), filled by FUN_004f8eb0 (Login_CameraUpdate).
// Iterates the 4 edges; if any cross product < param_3, returns a flag-encoded
// short indicating outside/on boundary.  Only active in game state 5.
//
// Returns:
//   encoded short with flags:
//     bit 8  — fVar1 < param_3 (outside edge)
//     bit 10 — NaN result
//     bit 14 — fVar1 == param_3 (on edge)
//   (short)1 if not in game state 5 or all edges pass.
//
// Globals:
//   DAT_005615c0  — current game state
//   DAT_07eeb228  — quad vertex X array (4 floats)
//   DAT_07eeb218  — quad vertex Y array (4 floats)

undefined2 __cdecl FUN_004f8ff0(float param_1,float param_2,float param_3)

{
  // 004F8FF0 TestFrustrum2D returns a boolean.  The prior reconstruction
  // returned diagnostic bit flags for a failed edge; callers use this as a
  // boolean and therefore treated every rejected block as visible.
  if (DAT_005615c0 != 5)
    return 1;

  for (int i = 0, previous = 3; i < 4; previous = i++) {
    const float cross = (DAT_07eeb218[previous] - param_2) *
                        (DAT_07eeb228[i] - param_1) -
                        (DAT_07eeb228[previous] - param_1) *
                        (DAT_07eeb218[i] - param_2);
    if (!(cross > param_3))
      return 0;
  }
  return 1;
}
