// Terrain_RenderBlocks.cpp
//
// Extracted from stubs_game.cpp.  Owns terrain block culling/render and the
// clipped dynamic-light variant.  Function comments retain IDA provenance.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
// AddTerrainLightClip @ 0x004F7800 (~74 lines) — adds clamped light to terrain buffer
// Iterates a square region around (xf,yf). Per cell: falloff = (Range-dist)/Range.
// Adds Light * falloff to Buffer, clamps to [0.0, 1.0].
// AddTerrainLightClip (0x004F7800).
//
// 2026-08-23: quedo SIN CALLERS en nuestro arbol, y es correcto que asi sea — no
// es codigo muerto para borrar.  Hasta hoy `structs.h` aliaseaba
// `AddTerrainLight` (0x4F76C0) a esta funcion, y por eso toda la luz dinamica
// quedaba clampeada a 1.0.  En el binario esta variante tiene UN solo caller
// (0x4C0E59, dentro de una funcion que todavia no portamos); cuando se porte,
// debe llamar a esta y no a AddTerrainLight.
//
// Diferencia entre las dos: esta clampea a [0, 1]; 0x4F76C0 solo evita negativos
// y deja que la luz supere 1.0 (que es lo que produce el resplandor del fuego).
void __cdecl AddTerrainLightClip_stub(float xf, float yf, float Light[3], int Range, float Buffer[3]) {
    // 0x004F7800 — Add clamped light to terrain light buffer.
    // Iterates a square region of radius Range around (xf,yf) in grid coords.
    // Per cell: falloff = (Range - dist) / Range. Adds Light * falloff to Buffer, clamps [0,1].
    // Buffer is a 256x256 grid of float[3] (RGB), stride 0xC per cell.
    // _DAT_00552594 = world-to-grid scale factor.

    float gridX = xf * _DAT_00552594;
    float gridY = yf * _DAT_00552594;
    int centerX = (int)gridX;
    int centerY = (int)gridY;

    int yStart = centerY - Range;
    int yEnd   = centerY + Range;

    if (yStart > yEnd) return;

    float fY = (float)yStart;
    int iy = yStart;
    do {
        int xStart = centerX - Range;
        int xEnd   = centerX + Range;
        float fX = (float)xStart;
        if (xStart <= xEnd) {
            int ix = xStart;
            do {
                float dx = gridX - fX;
                float dy = gridY - fY;
                float dist = (float)sqrt(dx * dx + dy * dy);
                float falloff = ((float)Range - dist) / (float)Range;

                if (_DAT_00552580 < falloff) {
                    // Index into 256x256 terrain light buffer
                    int cellIdx = ((unsigned int)(iy) & 0xFF) * 0x100 + ((unsigned int)(ix) & 0xFF);
                    float* pCell = Buffer + cellIdx * 3;

                    for (int ch = 0; ch < 3; ch++) {
                        float val = falloff * Light[ch] + pCell[ch];
                        pCell[ch] = val;
                        if (val < _DAT_00552580) {
                            pCell[ch] = 0.0f;
                        } else if (val > _DAT_0055256c) {
                            pCell[ch] = 1.0f;
                        }
                    }
                }

                ix++;
                fX += _DAT_0055256c;  // 1.0f
            } while (ix <= xEnd);
        }
        fY += _DAT_0055256c;  // 1.0f
        iy++;
    } while (iy <= yEnd);
}

// RenderTerrainBlock @ 0x004F9720 (~33 lines) — renders a 4x4 terrain tile block
void __cdecl RenderTerrainBlock_stub(float xf, float yf, int xi, int yi, bool EditFlag) {
    // 0x004F9720 — renders a 4x4 terrain tile block
    // _DAT_00552504 = 0.5f (half-tile center), _DAT_0055256c = 1.0f (tile stride)
    // CameraTopViewEnable = CameraTopViewEnabled
    // RenderTerrainTile = FUN_004f8480 (declared with int params but actually takes floats via stack)
    // TestFrustrum2D = FUN_004f8ff0
    float startX = xf;
    int row = 0;
    do {
        float centerY = yf + _DAT_00552504;  // 0.5f
        int col = 0;
        xf = startX;
        do {
            unsigned short visible = FUN_004f8ff0(xf + _DAT_00552504, centerY, 0.0f);
            if (visible || CameraTopViewEnabled != 0) {
                // RenderTerrainTile(xf, yf, col+xi, row+yi, 1.0f, 1, EditFlag)
                // functions.h declares first 2 params as int; reinterpret float bits
                FUN_004f8480(*(int*)&xf, *(int*)&yf, col + xi, row + yi, 1.0f, 1, (int)EditFlag);
            }
            xf = xf + _DAT_0055256c;  // 1.0f
            col++;
        } while (col < 4);
        yf = yf + _DAT_0055256c;
        row++;
    } while (row < 4);
}

// RenderTerrainFrustrum @ 0x004F97E0 (~42 lines) — iterates visible terrain blocks
void __cdecl RenderTerrainFrustrum_stub(bool EditFlag) {
    // 0x004F97E0 — iterates visible terrain blocks in frustum bounds
    // FrustrumBoundMinX_1 = DAT_0839bc90, FrustrumBoundMinY_1 = DAT_0839bc94
    // FrustrumBoundMaxX_1 = DAT_0055a774, FrustrumBoundMaxY_1 = DAT_0055a778
    // _DAT_0055264c = 2.0f (block center offset), _DAT_00552650 = 4.0f (block stride)
    // TestFrustrum2D = FUN_004f8ff0, CameraTopViewEnable = CameraTopViewEnabled
    int yi = (int)DAT_0839bc94;  // FrustrumBoundMinY_1
    if (yi <= (int)DAT_0055a778) {  // FrustrumBoundMaxY_1
        float blockY = (float)yi;
        do {
            int maxX = (int)DAT_0055a774;  // FrustrumBoundMaxX_1
            if ((int)DAT_0839bc90 <= maxX) {
                float centerY = blockY + _DAT_0055264c;  // 2.0f
                int xi = (int)DAT_0839bc90;  // FrustrumBoundMinX_1
                float blockX = (float)xi;
                do {
                    unsigned short visible = FUN_004f8ff0(blockX + _DAT_0055264c, centerY, -40.0f);
                    if (visible || CameraTopViewEnabled != 0) {
                        RenderTerrainBlock_stub(blockX, blockY, xi, yi, EditFlag);
                    }
                    blockX = blockX + _DAT_00552650;  // 4.0f
                    xi += 4;
                    maxX = (int)DAT_0055a774;
                } while (xi <= maxX);
            }
            blockY = blockY + _DAT_00552650;  // 4.0f
            yi += 4;
        } while (yi <= (int)DAT_0055a778);
    }
}
