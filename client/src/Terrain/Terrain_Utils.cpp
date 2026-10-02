// Terrain_Utils.cpp
// IDA: Terrain_GetTileIndex (0x004F6C40)
// RequestTerrainHeight @ 0x004f7500  — Terrain_HeightAt
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
//   SceneFlag — current game state (5 = in-game)
//   DAT_080cb2cc — terrain height map float[256][256]
//   _DAT_00552580 — float constant 0.0

#include "stdafx.h"

int __cdecl Terrain_GetTileIndex(uint param_1,uint param_2)

{
  return (param_2 & 0xff) * 0x100 + (param_1 & 0xff);
}


// IDA: RequestTerrainHeight (0x004F7500)
// Bilinear interpolation of terrain height at world (xf, yf).
float __cdecl RequestTerrainHeight(float xf, float yf)
{
    // Desviación: IDA retorna si SceneFlag != 5, pero Recv_JoinMapServer llama a
    // CreateCharacterPointer antes de que la escena pase a 5. Acá alcanza con que
    // el mundo esté cargado (World válido y DAT_080cb2cc con el height map).
    if ((int)World < 0) return 0.0f;

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


// TestFrustrum2D @ 0x004f8ff0
//
// Terrain_PointInQuad — tests whether a 2D world point (param_1, param_2)
// lies inside (or on the boundary of) the current terrain quad, using a
// signed-area (cross-product) test against param_3.
//
// The terrain quad is stored as 4 projected vertices in FrustrumX (X) and
// FrustrumY (Y), filled by CreateFrustrum2D (Login_CameraUpdate).
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
//   SceneFlag  — current game state
//   FrustrumX  — quad vertex X array (4 floats)
//   FrustrumY  — quad vertex Y array (4 floats)

undefined2 __cdecl TestFrustrum2D(float param_1,float param_2,float param_3)

{
  // 004F8FF0 TestFrustrum2D returns a boolean.  The prior reconstruction
  // returned diagnostic bit flags for a failed edge; callers use this as a
  // boolean and therefore treated every rejected block as visible.
  if (SceneFlag != 5)
    return 1;

  for (int i = 0, previous = 3; i < 4; previous = i++) {
    const float cross = (FrustrumY[previous] - param_2) *
                        (FrustrumX[i] - param_1) -
                        (FrustrumX[previous] - param_1) *
                        (FrustrumY[i] - param_2);
    if (!(cross > param_3))
      return 0;
  }
  return 1;
}
