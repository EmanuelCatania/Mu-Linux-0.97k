// UI_GuildMark.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// RenderGuildMark @ 0x004F02F0 (~87 lines) — renders guild emblem at screen position
// Fills Bitmaps[0x1e9] pixel buffer with MarkColor[param_5], uploads via glTexImage2D.
// If param_5==0: draws cross pattern in gray. Then RenderBitmap.
void __cdecl RenderGuildMark_stub(float p1, float p2, float p3, float p4, int p5) {
    // 0x004F02F0 — Render guild emblem at screen position (p1,p2), size (p3 x p4).
    // p5 = color index into MarkColor[16] palette at DAT_07e11f34.
    // Bitmaps[0x22] = guild mark texture slot (Components, TextureNumber, Buffer).

    float Width  = p3;
    float Height = p4;

    // Draw background box (texture 0x116) slightly larger
    GL_DrawTexture(0x116, p1 - _DAT_0055256c, p2 - _DAT_0055256c,
                 Width + _DAT_0055264c, Height + _DAT_0055264c,
                 0.0f, 0.0f, Width * _DAT_00552ae4, Height * _DAT_00552a20,
                 '\x01', '\x01');

    int iVar2 = (int)p3;  // pixel width
    int iVar3 = (int)p4;  // pixel height

    DWORD* pixelBuf = (DWORD*)Bitmaps[0x22].Buffer;
    // 2026-08-25: `DAT_07e11f34` ahora es el array de 16 que realmente es, asi
    // que se indexa directo (antes `(&DAT_07e11f34)[p5]` sobre un unico DWORD
    // leia hasta 60 bytes del vecino).
    if (p5 < 0 || p5 > 15) return;
    DWORD color = DAT_07e11f34[p5];  // MarkColor[p5]

    if (p5 == 0) {
        // No guild — fill black, draw gray cross pattern
        DWORD* pFill = pixelBuf;
        int rowsLeft = iVar3;
        if (iVar3 > 0) {
            do {
                DWORD* pEnd = pFill;
                if (iVar2 > 0) {
                    pEnd = pFill + iVar2;
                    for (int c = iVar2; c != 0; c--) {
                        *pFill = 0xFF000000;  // opaque black
                        pFill++;
                    }
                }
                rowsLeft--;
                pFill = pEnd;
            } while (rowsLeft != 0);
        }
        // Diagonal cross (top-left to bottom-right, stride 9)
        DWORD* pCross = pixelBuf;
        for (int i = 8; i != 0; i--) {
            *pCross = 0xFF808080;  // gray
            pCross += 9;
        }
        // Diagonal cross (top-right to bottom-left, stride 7)
        pCross = pixelBuf;
        for (int i = 8; i != 0; i--) {
            pCross += 7;
            *pCross = 0xFF808080;  // gray
        }
    } else {
        // Fill with guild color
        DWORD* pFill = pixelBuf;
        int rowsLeft = iVar3;
        if (iVar3 > 0) {
            do {
                int colsLeft = iVar2;
                DWORD* pRow = pFill;
                if (iVar2 > 0) {
                    for (; colsLeft != 0; colsLeft--) {
                        *pRow = color;
                        pRow++;
                    }
                    pFill += iVar2;
                }
                rowsLeft--;
            } while (rowsLeft != 0);
        }
    }

    // Upload pixel buffer as GL texture
    glBindTexture(0xDE1, Bitmaps[0x22].TextureNumber);  // GL_TEXTURE_2D
    glTexImage2D(0xDE1, 0, (int)(BYTE)Bitmaps[0x22].Components,
                 iVar2, iVar3, 0, 0x1908, 0x1401, (void*)Bitmaps[0x22].Buffer);  // GL_RGB, GL_UNSIGNED_BYTE

    // Draw the guild mark texture (texture slot 0x22)
    GL_DrawTexture(0x22, p1, p2, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
}
