// Render_SpriteHelpers.cpp
//
// Extracted from stubs_game.cpp.  Owns the additive blend setup and sprite/
// digit-atlas draw helpers.  Function comments retain IDA provenance.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
// EnableAlphaBlend2 @ 0x00511810 (~28 lines) — OpenGL additive blending
void __stdcall EnableAlphaBlend2_stub(void) {
    // 0x00511810 — OpenGL additive blending (ONE_MINUS_DST_COLOR, ONE)
    // 2026-04-30 BUG-FIX: previously cached state in DAT_07eaa160/164/168
    // — those addresses are CheckInventory + adjacent ITEM ptrs, NOT GL
    // state.  IDA shows the real cache is at DAT_083a412c (AlphaBlendType),
    // DAT_083a411d (AlphaTestEnable), DAT_083a4125 (TextureEnable).
    // Writing 2/3/5 to CheckInventory was crashing Scene_MapTick when it
    // dereferenced CheckInventory as ITEM*.
    if (DAT_083a412c != 5) {           // AlphaBlendType
        DAT_083a412c = 5;
        glEnable(0x0BE2);              // GL_BLEND
        glBlendFunc(0x0301, 1);        // GL_ONE_MINUS_DST_COLOR, GL_ONE
    }
    FUN_00511570();                    // DisableCullFace
    FUN_00511530();                    // DisableDepthMask
    if (DAT_083a411d) {                // AlphaTestEnable
        DAT_083a411d = 0;
        glDisable(0x0BC0);             // GL_ALPHA_TEST
    }
    if (!DAT_083a4125) {               // TextureEnable
        DAT_083a4125 = 1;
        glEnable(0x0DE1);              // GL_TEXTURE_2D
    }
    if (FogEnable) {
        glEnable(0x0B60);              // GL_FOG
    }
}

// RenderSpriteUV @ 0x00511FB0 (~67 lines) — renders textured billboard quad with custom UVs
void __cdecl RenderSpriteUV_stub(int Texture, float Position[3], float Width, float Height, float (*UV)[2], float Light[3][4], float Alpha) {
    // 0x00511FB0 — Render textured billboard quad with custom UVs.
    // Binds texture, transforms Position through CameraMatrix (DAT_083a4140),
    // then draws GL_QUADS with 4 vertices: color from Light[][4] + Alpha, texcoord from UV[][2],
    // vertex position offset from transformed Position.
    //
    // Light[3][4]: 4 vertices, each with 3 color channels. Alpha is the 4th color component.
    // UV: pointer to 4 pairs of (u,v) texture coordinates.
    // Width/Height: used as pointers to vertex position data (Ghidra reinterprets float params
    // as pointers due to the calling convention; in practice they point to per-vertex data).
    //
    // NOTE: Ghidra shows phantom registers and reinterprets Width/Height as pointers.
    // The actual binary passes per-vertex position/UV/color arrays after the initial params.
    // This implementation follows the Ghidra decompilation structure.

    FUN_00511480(Texture);  // GL_BindTexture

    // VectorTransform: transform Position by CameraMatrix (4x3 matrix at DAT_083a4140)
    float transformed[3];
    float* camMat = (float*)&DAT_083a4140;
    // VectorTransform(in, matrix, out) — 3x4 matrix multiply
    transformed[0] = Position[0] * camMat[0] + Position[1] * camMat[1] + Position[2] * camMat[2] + camMat[3];
    transformed[1] = Position[0] * camMat[4] + Position[1] * camMat[5] + Position[2] * camMat[6] + camMat[7];
    transformed[2] = Position[0] * camMat[8] + Position[1] * camMat[9] + Position[2] * camMat[10] + camMat[11];

    glBegin(7);  // GL_QUADS

    // Ghidra decompile iterates 4 vertices. Per-vertex data:
    //   Color: Light[vertex][0..2] + Alpha
    //   TexCoord: UV[vertex][0..1]
    //   Vertex: transformed position (offset per vertex from transformed[])
    // Due to Ghidra reinterpreting Width/Height as data pointers and phantom regs,
    // the exact per-vertex offset layout is from the caller's stack.
    // Standard billboard quad: 4 corners offset by +/-Width, +/-Height from center.

    float* pColor = (float*)&Light[0][0];
    float* pUV = (float*)UV;
    float halfW = Width * 0.5f;
    float halfH = Height * 0.5f;

    // Vertex offsets for a billboard quad (CW: TL, TR, BR, BL)
    float offsets[4][2] = {
        { -halfW, -halfH },
        {  halfW, -halfH },
        {  halfW,  halfH },
        { -halfW,  halfH }
    };

    for (int i = 0; i < 4; i++) {
        glColor4f(pColor[i * 3 + 0], pColor[i * 3 + 1], pColor[i * 3 + 2], Alpha);
        glTexCoord2f(pUV[i * 2 + 0], pUV[i * 2 + 1]);
        float vtx[3] = {
            transformed[0] + offsets[i][0],
            transformed[1] + offsets[i][1],
            transformed[2]
        };
        glVertex3fv(vtx);
    }

    glEnd();
}

// RenderNumber2D @ 0x005122F0 (~40 lines) — renders integer as digit sprites
// _itoa to string, then per digit: RenderBitmap sub-rect from digit atlas (texture 1).
double __cdecl RenderNumber2D_stub(float x, float y, int Num, float Width, float Height) {
    // 0x005122F0 — renders integer as digit sprites using texture atlas
    // _DAT_00552504 = 0.5f, _DAT_00552928 = 0.7f (approx), _DAT_005526dc = 0.0625f
    // RenderBitmap = FUN_005125a0, texture 1 = digit atlas
    char buf[32];
    _itoa(Num, buf, 10);
    // strlen inline (Ghidra pattern: decrement 0xFFFFFFFF counter)
    int len = (int)strlen(buf);
    int i = 0;
    // Center the number string: shift x left by len * Width * 0.5f
    x = x - (float)len * Width * _DAT_00552504;  // 0.5f
    if (len > 0) {
        float step = Width * _DAT_00552928;  // ~0.7f digit spacing
        do {
            // Each digit: sub-rect from atlas row, u = (digit * 0.0625f), v = 0, uW = 0.0625f, vH = 0.5f
            float u = (float)(buf[i] - '0') * _DAT_005526dc;  // 0.0625f
            FUN_005125a0(1, x, y, Width, Height, u, 0.0f, 0.0625f, 0.5f, true, true);
            x = step + x;
            i++;
        } while (i < len);
    }
    return (double)x;
}
