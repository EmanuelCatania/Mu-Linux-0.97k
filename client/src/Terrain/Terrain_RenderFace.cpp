// Terrain_RenderFace.cpp — port 1:1 desde IDA (2026-06-27)
// Cadena de dibujo de tiles texturados del terreno (eslabón #4-#5 de RenderTerrain):
//   RenderTerrainTile (FUN_004f8480) → RenderTerrainFace (FUN_004f7fb0) →
//   FaceTexture (UVs) + RenderFace / RenderFaceAlpha / RenderFaceBlend (draw).
//
// Antes: FUN_004f7fb0 era un no-op stub (stubs.cpp) y las 4 primitivas no existían,
// por eso el dev previo escribió una fallback flat-shaded en RenderTerrain.
//
// Direcciones/buffers verificados por bytes de operando en IDA:
//   FaceTexture        0x004F7DF0   RenderFace        0x004F7A90
//   RenderFaceAlpha    0x004F7B80   RenderFaceBlend   0x004F7CE0
//   RenderTerrainFace  0x004F7FB0   BindTexture       0x00511480 (GL_BindTextureSlot)
//   Bitmaps base       0x083A7CA0 (stride 0x38, BITMAP_t: Width@0x20 Height@0x24)
//   TerrainMappingAlpha  = DAT_0834b608 (float[256²])   (mislabel "TerrainHeight")
//   TerrainMappingLayer1 = DAT_080bb2b4 (BYTE[256²])
//   TerrainMappingLayer2 = DAT_080ab2b4 (BYTE[256²])
//   PrimaryTerrainLight  = DAT_081cb608 (float[256²][3])  ← 0x081CB608 (NO 0x07eab250;
//                          el macro PrimaryTerrainLight de structs.h apunta mal a
//                          0x07eab250 — buffer muerto. Lo leímos directo de DAT_081cb608,
//                          que FUN_004f95e0 puebla per-frame desde BackTerrainLight 0x0828b608).
//   TerrainVertex        = g_TilePickBuf[12]   (4 corners contiguos)
//   TerrainTextureCoord  = g_TerrainTexCoord[8] (4 UV pairs)
//   TerrainGrassWind     = DAT_07eab200 (float[256²])     (mislabel "water-wave heights")
//   TerrainGrassTexture[(yi&0xFF)+1] = DAT_0810b2cc[yi&0xFF]
//   WaterMove=DAT_07eeb214  WaterTextureNumber=DAT_0839bc8c  CurrentLayer=DAT_0814b2dc
//   TerrainFlag=DAT_0838bc44  unk_839BC86=DAT_0839bc86  World=g_GameSubState
//   TerrainIndex1..4 = DAT_07eab1ec/f0/f4/f8

#include "stdafx.h"


// ── Aliases locales que mapean los nombres IDA a nuestros globals ────────────
#define TER_FLAG    DAT_0838bc44
#define TER_IDX1    DAT_07eab1ec
#define TER_IDX2    DAT_07eab1f0
#define TER_IDX3    DAT_07eab1f4
#define TER_IDX4    DAT_07eab1f8
#define TER_ALPHA   DAT_0834b608                          // TerrainMappingAlpha (float)
#define TER_L1      DAT_080bb2b4                           // TerrainMappingLayer1 (BYTE)
#define TER_L2      DAT_080ab2b4                           // TerrainMappingLayer2 (BYTE)
#define TER_WIND    DAT_07eab200                           // TerrainGrassWind (float)
#define TER_VERT    ((float(*)[3])&g_TilePickBuf[0])       // TerrainVertex[4][3]
#define TER_TEX     ((float(*)[2])&g_TerrainTexCoord[0])   // TerrainTextureCoord[4][2]
#define TER_LIGHT   ((float(*)[3])&DAT_081cb608[0])        // PrimaryTerrainLight[N][3]
#define TER_NODRAW  DAT_0839bc86                           // unk_839BC86 (skip-draw flag)

// ── FaceTexture (FUN_004F7DF0) — computa UVs del tile en TerrainTextureCoord ──
void __cdecl FUN_004f7df0(int Texture, float xf, float yf, char Water, char Scale)
{
    BITMAP_t* bmp = &Bitmaps[Texture + 35];
    float Width, dHeight;
    if (Scale) { Width = 16.0f / bmp->Width; dHeight = 16.0f; }
    else       { Width = 64.0f / bmp->Width; dHeight = 64.0f; }
    float Height = dHeight / bmp->Height;
    float suf = Width * xf;
    float svf = Height * yf;
    float (*tc)[2] = TER_TEX;

    if (Water) {
        float k = Scale ? 0.0080000004f : 0.0020000001f;
        float w3 = TER_WIND[TER_IDX1] * k;
        float w4 = TER_WIND[TER_IDX2] * k;
        float yfa = w3 + svf;
        float xfb = DAT_07eeb214 + suf;          // WaterMove + suf
        tc[0][0] = xfb;          tc[0][1] = yfa;
        float ub = w4 + svf;
        float va = xfb + Width;
        tc[1][0] = va;           tc[1][1] = ub;
        tc[2][0] = va;           tc[2][1] = ub + Height;
        tc[3][0] = xfb;          tc[3][1] = yfa + Height;
    } else {
        tc[0][0] = suf;          tc[0][1] = svf;
        float u = suf + Width;
        tc[1][0] = u;            tc[1][1] = svf;
        tc[2][0] = u;            tc[2][1] = svf + Height;
        tc[3][0] = suf;          tc[3][1] = svf + Height;
    }
}

// ── RenderFace (FUN_004F7A90) — quad opaco (base) ────────────────────────────
void __cdecl FUN_004f7a90(int Texture)
{
    GL_ResetState();                  // DisableAlphaBlend
    GL_BindTextureSlot(Texture + 35);      // BindTexture
    if (TER_NODRAW) return;
    float (*v)[3]  = TER_VERT;
    float (*tc)[2] = TER_TEX;
    float (*L)[3]  = TER_LIGHT;
    glBegin(6u);                     // GL_TRIANGLE_FAN
    glTexCoord2f(tc[0][0], tc[0][1]); glColor3fv(L[TER_IDX1]); glVertex3fv(v[0]);
    glTexCoord2f(tc[1][0], tc[1][1]); glColor3fv(L[TER_IDX2]); glVertex3fv(v[1]);
    glTexCoord2f(tc[2][0], tc[2][1]); glColor3fv(L[TER_IDX3]); glVertex3fv(v[2]);
    glTexCoord2f(tc[3][0], tc[3][1]); glColor3fv(L[TER_IDX4]); glVertex3fv(v[3]);
    glEnd();
}

// ── RenderFaceAlpha (FUN_004F7B80) — overlay capa2 con alpha por vértice ──────
void __cdecl FUN_004f7b80(int Texture)
{
    GL_SetBlendSrcOver('\x01');            // EnableAlphaTest(1)
    GL_BindTextureSlot(Texture + 35);      // BindTexture
    if (TER_NODRAW) return;
    float (*v)[3]  = TER_VERT;
    float (*tc)[2] = TER_TEX;
    float (*L)[3]  = TER_LIGHT;
    glBegin(6u);
    glTexCoord2f(tc[0][0], tc[0][1]); glColor4f(L[TER_IDX1][0], L[TER_IDX1][1], L[TER_IDX1][2], TER_ALPHA[TER_IDX1]); glVertex3fv(v[0]);
    glTexCoord2f(tc[1][0], tc[1][1]); glColor4f(L[TER_IDX2][0], L[TER_IDX2][1], L[TER_IDX2][2], TER_ALPHA[TER_IDX2]); glVertex3fv(v[1]);
    glTexCoord2f(tc[2][0], tc[2][1]); glColor4f(L[TER_IDX3][0], L[TER_IDX3][1], L[TER_IDX3][2], TER_ALPHA[TER_IDX3]); glVertex3fv(v[2]);
    glTexCoord2f(tc[3][0], tc[3][1]); glColor4f(L[TER_IDX4][0], L[TER_IDX4][1], L[TER_IDX4][2], TER_ALPHA[TER_IDX4]); glVertex3fv(v[3]);
    glEnd();
}

// ── RenderFaceBlend (FUN_004F7CE0) — overlay de agua (modula por alpha gris) ──
void __cdecl FUN_004f7ce0(int Texture)
{
    GL_SetBlendAdditive();                  // EnableAlphaBlend
    GL_BindTextureSlot(Texture + 35);      // BindTexture
    if (TER_NODRAW) return;
    float (*v)[3]  = TER_VERT;
    float (*tc)[2] = TER_TEX;
    glBegin(6u);
    glTexCoord2f(tc[0][0], tc[0][1]); glColor3f(TER_ALPHA[TER_IDX1], TER_ALPHA[TER_IDX1], TER_ALPHA[TER_IDX1]); glVertex3fv(v[0]);
    glTexCoord2f(tc[1][0], tc[1][1]); glColor3f(TER_ALPHA[TER_IDX2], TER_ALPHA[TER_IDX2], TER_ALPHA[TER_IDX2]); glVertex3fv(v[1]);
    glTexCoord2f(tc[2][0], tc[2][1]); glColor3f(TER_ALPHA[TER_IDX3], TER_ALPHA[TER_IDX3], TER_ALPHA[TER_IDX3]); glVertex3fv(v[2]);
    glTexCoord2f(tc[3][0], tc[3][1]); glColor3f(TER_ALPHA[TER_IDX4], TER_ALPHA[TER_IDX4], TER_ALPHA[TER_IDX4]); glVertex3fv(v[3]);
    glEnd();
}

// ── RenderTerrainFace (FUN_004F7FB0) — dispatcher de capas/overlays ──────────
void __cdecl FUN_004f7fb0(float xf, float yf, int xi, int yi, float lodf)
{
    (void)xi; (void)yi; (void)lodf;   // los TerrainVertex/Index ya los fijó RenderTerrainTile

    if (TER_FLAG == 2) {
        // ── capa de billboards de pasto/arena, movida por el viento ────────
        // 2026-08-23: estuvo inerte porque `unk_55A76C` (DAT_0055a76c) estaba
        // inicializado en 0.  Nadie lo escribe, pero es constante de .data y en
        // el binario vale 1 — con 0 esta pasada no corria en ningun mapa y se
        // perdia el pasto de Lorencia/Noria y la arena volando de Tarkan.
        if (TER_ALPHA[TER_IDX1] <= 0.0f && TER_ALPHA[TER_IDX2] <= 0.0f &&
            TER_ALPHA[TER_IDX3] <= 0.0f && TER_ALPHA[TER_IDX4] <= 0.0f &&
            !DAT_0814b2dc && (World < 11 || World > 16))   // CurrentLayer
        {
            int slot = (int)(unsigned char)TER_L1[TER_IDX1] + 50;
            BITMAP_t* bmp = &Bitmaps[slot];
            if (bmp->Buffer) {
                GL_BindTextureSlot(slot);                      // BindTexture
                float yfa = bmp->Height + bmp->Height;
                float u = xf * 0.25f + DAT_0810b2cc[(unsigned char)yi]; // TerrainGrassTexture[(yi&0xFF)+1]
                float (*tc)[2] = TER_TEX;
                tc[0][0] = u;          tc[0][1] = 0.0f;
                tc[1][0] = u + 0.25f;  tc[1][1] = 0.0f;
                tc[2][0] = u + 0.25f;  tc[2][1] = 1.0f;
                tc[3][0] = u;          tc[3][1] = 1.0f;
                float (*v)[3] = TER_VERT;
                v[3][0] = v[0][0];  v[3][1] = v[0][1];    // copy corner0 x,y → corner3
                float oldZ0 = v[0][2];
                v[0][2] = v[0][2] + yfa;
                v[3][2] = oldZ0;
                v[1][2] = v[2][2] + yfa;
                v[0][0] = v[0][0] - 50.0f;
                v[1][0] = v[2][0] - 50.0f;
                v[0][1] = v[0][1] + TER_WIND[TER_IDX1];
                v[1][1] = v[2][1] + TER_WIND[TER_IDX2];
                float (*L)[3] = TER_LIGHT;
                glBegin(7u);                              // GL_QUADS
                glTexCoord2f(tc[0][0], tc[0][1]); glColor3fv(L[TER_IDX1]); glVertex3fv(v[0]);
                glTexCoord2f(tc[1][0], tc[1][1]); glColor3fv(L[TER_IDX2]); glVertex3fv(v[1]);
                glTexCoord2f(tc[2][0], tc[2][1]); glColor3fv(L[TER_IDX3]); glVertex3fv(v[2]);
                glTexCoord2f(tc[3][0], tc[3][1]); glColor3fv(L[TER_IDX4]); glVertex3fv(v[3]);
                glEnd();
            }
        }
        return;
    }

    // ── main pass (TerrainFlag 0) ──
    char water = 0;
    char hasL2;
    int tex;
    if (TER_ALPHA[TER_IDX1] < 1.0f || TER_ALPHA[TER_IDX2] < 1.0f ||
        TER_ALPHA[TER_IDX3] < 1.0f || TER_ALPHA[TER_IDX4] < 1.0f)
    {
        unsigned char l1 = TER_L1[TER_IDX1];
        hasL2 = 1;
        tex = l1;
        if (l1 == 5) water = 1;
    } else {
        hasL2 = 0;
        tex = (unsigned char)TER_L2[TER_IDX1];
    }
    FUN_004f7df0(tex, xf, yf, water, 0);     // FaceTexture
    FUN_004f7a90(tex);                       // RenderFace

    if (World == 7 && (unsigned char)TER_L2[TER_IDX1] == 5 &&
        (TER_ALPHA[TER_IDX1] > 0.0f || TER_ALPHA[TER_IDX2] > 0.0f ||
         TER_ALPHA[TER_IDX3] > 0.0f || TER_ALPHA[TER_IDX4] > 0.0f))
    {
        int t = (int)DAT_0839bc8c + 30;      // WaterTextureNumber + 30
        FUN_004f7df0(t, xf, yf, 0, 1);       // FaceTexture
        FUN_004f7ce0(t);                     // RenderFaceBlend
    }
    else if (hasL2 &&
             (TER_ALPHA[TER_IDX1] > 0.0f || TER_ALPHA[TER_IDX2] > 0.0f ||
              TER_ALPHA[TER_IDX3] > 0.0f || TER_ALPHA[TER_IDX4] > 0.0f))
    {
        int t2 = (unsigned char)TER_L2[TER_IDX1];
        if (t2 != 5) water = 0;
        FUN_004f7df0(t2, xf, yf, water, 0);  // FaceTexture
        FUN_004f7b80(t2);                    // RenderFaceAlpha
    }
}
