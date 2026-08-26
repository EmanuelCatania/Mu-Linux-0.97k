// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// FUN_004f8740 @ 0x004F8740 (79 lines) — Terrain_RenderTileQuad
// Renders a single terrain quad as GL_TRIANGLE_FAN with per-vertex lighting.
// p1/p2 = grid X/Y, p3 = tile size, p4 = index step, p5 = texcoord array ptr,
// p6 = enable lighting, p7 = alpha value.
void __cdecl FUN_004f8740(float p1, float p2, float p3, int p4, int p5, char p6, float p7) {
    // BUG-FIX 2026-04-29: guard contra DAT_07eab24c (BackTerrainHeight) no
    // inicializado. Crash AV en
    // 0x410E4597 venía de cursor billboard FUN_004f8bb0 dereferenciando
    // el buffer NULL.
    if (DAT_07eab24c == 0 || (uintptr_t)DAT_07eab24c < 0x100000) return;
    int iX = (int)p1;
    int iY = (int)p2;
    if (iX < 0 || iY < 0 || iX >= 0xff || iY >= 0xff) return;

    int idx0 = iY * 0x100 + iX;
    int idx1 = idx0 + p4;
    int idx3 = iX + (iY + p4) * 0x100;
    int idx2 = p4 + idx3;

    float scale = _DAT_005524f0;
    float verts[4][3];
    verts[0][0] = p1 * scale;
    verts[0][1] = p2 * scale;
    verts[0][2] = *(float *)(DAT_07eab24c + idx0 * 4); // BackTerrainHeight
    verts[1][0] = verts[0][0] + p3 * scale;
    verts[1][1] = verts[0][1];
    verts[1][2] = *(float *)(DAT_07eab24c + idx1 * 4);
    verts[2][0] = verts[1][0];
    verts[2][1] = verts[0][1] + p3 * scale;
    verts[2][2] = *(float *)(DAT_07eab24c + idx2 * 4);
    verts[3][0] = verts[0][0];
    verts[3][1] = verts[2][1];
    verts[3][2] = *(float *)(DAT_07eab24c + idx3 * 4);

    float light[4][3];
    if (p6 != '\0') {
        int indices[4] = { idx0, idx1, idx2, idx3 };
        for (int v = 0; v < 4; v++) {
            // 2026-08-23: leia DAT_07eab250, que es un DWORD muerto y NO es
            // PrimaryTerrainLight (ver globals.h:837).  El buffer real es
            // DAT_081cb608, el mismo que resetea Terrain_Water por frame.
            float *src = &DAT_081cb608[indices[v] * 3];
            light[v][0] = src[0]; light[v][1] = src[1]; light[v][2] = src[2];
        }
    }

    glBegin(6); // GL_TRIANGLE_FAN
    for (int i = 0; i < 4; i++) {
        if (p6 != '\0') {
            if (p7 == 1.0f) {
                glColor3fv((GLfloat *)&light[i]);
            } else {
                glColor4f(light[i][0], light[i][1], light[i][2], p7);
            }
        }
        // IDA 0x004F8740 walks `a5` as 4 records of 3 floats each (stride 12),
        // consuming the first two components as UVs. Callers such as
        // FUN_004f8980/FUN_004f8bb0 pass rotated quad data in that layout.
        const float *tc = (const float *)(p5 + i * 12);
        glTexCoord2f(tc[0], tc[1]);
        glVertex3fv((GLfloat *)&verts[i]);
    }
    glEnd();
}

// FUN_004f8980 @ 0x004F8980 (116 lines) — Terrain_RenderTexturedObject
// Renders a rotated textured object on terrain by tiling into sub-quads.
// p1 = texture index, p2/p3 = grid position, p4 = rotation angle.
// Uses AngleMatrix + VectorRotate to rotate sub-tile corners, then draws each
// with FUN_004f8740 (Particle_DrawTile).
// Bitmaps[idx * 0xE + 8] = width, Bitmaps[idx * 0xE + 9] = height.
// _DAT_00552b9c = 1/64 (UV step), _DAT_0055256c = 1.0 (tile step).
void __cdecl FUN_004f8980(int p1, int p2, int p3, float p4)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    float angles[3] = { 0.0f, 0.0f, p4 };
    float matrix[3][4];
    AngleMatrix(angles, matrix);
    FUN_00511480(p1); // BindTexture

    // Bitmap dimensions
    float* bmpData = (float*)((char*)Bitmaps + p1 * 0xE * sizeof(float));
    float bmpW = bmpData[8];
    float bmpH = bmpData[9];
    float uvStepX = _DAT_00552cb4 / bmpW;  // 64.0 / width
    float uvStepY = _DAT_00552cb4 / bmpH;  // 64.0 / height

    float tileW = bmpW * _DAT_00552b9c;  // width * (1/64)
    float tileH = bmpH * _DAT_00552b9c;  // height * (1/64)

    float fy = 0.0f;
    while (fy < tileH) {
        float fx = 0.0f;
        float uvY0 = fy * uvStepY;
        float uvY1 = (fy + _DAT_0055256c) * uvStepY;

        while (fx < tileW) {
            float uvX0 = fx * uvStepX;
            float uvX1 = (fx + _DAT_0055256c) * uvStepX;
            float uvRot[4][3] = {
                { uvX0, uvY0, 0.0f },
                { uvX1, uvY0, 0.0f },
                { uvX1, uvY1, 0.0f },
                { uvX0, uvY1, 0.0f }
            };
            for (int i = 0; i < 4; ++i) {
                float in[3] = {
                    uvRot[i][0] - _DAT_00552504,
                    uvRot[i][1] - _DAT_00552504,
                    0.0f
                };
                float out[3];
                out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
                out[1] = in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2];
                out[2] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];
                uvRot[i][0] = out[0] + _DAT_00552504;
                uvRot[i][1] = out[1] + _DAT_00552504;
                uvRot[i][2] = out[2];
            }

            FUN_004f8740((float)p2 + fx, (float)p3 + fy, 1.0f, 1, (int)uvRot, '\x01', 1.0f);
            fx += _DAT_0055256c;
        }
        fy += _DAT_0055256c;
    }
}


// RenderTerrainAlphaBitmap @ 0x004F8BB0 (~105 lines) — Terrain decal overlay
// Renders rotated alpha texture on terrain (spell circles, shadows, blood splats).
void __cdecl RenderTerrainAlphaBitmap(int tex, float x, float y, float sx, float sy, float *light, float alpha, float size) {
    (void)tex; (void)x; (void)y; (void)sx; (void)sy; (void)light; (void)alpha; (void)size;
    // Full implementation requires:
    //   1. glColor3fv(light) or glColor4f(light[0..2], alpha)
    //   2. AngleMatrix from 'size' (rotation angle)
    //   3. BindTexture(tex)
    //   4. 2D tile loop over bounding area based on max(sx,sy)
    //   5. Per-tile: compute UV + VectorRotate for rotation
    //   6. Call FUN_004f8740 (terrain quad renderer) per tile
    // Documented at 0x004F8BB0 — terrain decal system
}
