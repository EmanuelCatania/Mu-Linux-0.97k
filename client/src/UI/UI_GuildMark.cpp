// UI_GuildMark.cpp
// Extraído de stubs_game.cpp; se conserva la trazabilidad IDA en los comentarios de las funciones.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_004F0100 y FUN_004F02F0 escriben DWORD ARGB en cada píxel y suben
// el resultado como GL_RGBA. El loader genérico del port deja las OZJ como RGB
// (3 bytes/píxel), por lo que el slot 34 necesita convertirse una vez en el
// destino RGBA que ya presupone el binario antes de renderizar una marca.
extern "C" bool GuildMark_EnsureRenderTarget(void)
{
    BITMAP_t& markTexture = Bitmaps[34];
    const int width = (int)markTexture.Width;
    const int height = (int)markTexture.Height;
    if (width <= 0 || height <= 0 || markTexture.Buffer == nullptr)
        return false;

    if ((unsigned char)markTexture.Components == 4)
        return true;

    const size_t pixelCount = (size_t)width * (size_t)height;
    BYTE* rgba = new BYTE[pixelCount * 4];
    const BYTE* rgb = markTexture.Buffer;
    for (size_t i = 0; i < pixelCount; ++i) {
        rgba[i * 4 + 0] = rgb[i * 3 + 0];
        rgba[i * 4 + 1] = rgb[i * 3 + 1];
        rgba[i * 4 + 2] = rgb[i * 3 + 2];
        rgba[i * 4 + 3] = 0xFF;
    }

    delete[] markTexture.Buffer;
    markTexture.Buffer = rgba;
    markTexture.Components = 4;
    return true;
}

// IDA: FUN_004F0100 — inicializa la paleta compartida antes de convertir los
// 64 nibbles de una marca a píxeles ARGB. El editor usa la misma tabla mediante
// FUN_004F02F0; por eso debe existir aun cuando no haya una marca de entidad.
extern "C" void GuildMark_InitializePalette(bool blend)
{
    const DWORD alpha = blend ? 0u : 0x80000000u;
    DAT_07e11f34[0]  = alpha;
    DAT_07e11f34[1]  = 0xFF000000u;
    DAT_07e11f34[2]  = 0xFF808080u;
    DAT_07e11f34[3]  = 0xFFFFFFFFu;
    DAT_07e11f34[4]  = 0xFF0000FFu;
    DAT_07e11f34[5]  = 0xFF0080FFu;
    DAT_07e11f34[6]  = 0xFF00FFFFu;
    DAT_07e11f34[7]  = 0xFF00FF80u;
    DAT_07e11f34[8]  = 0xFF00FF00u;
    DAT_07e11f34[9]  = 0xFF80FF00u;
    DAT_07e11f34[10] = 0xFFFFFF00u;
    DAT_07e11f34[11] = 0xFFFF8000u;
    DAT_07e11f34[12] = 0xFFFF0000u;
    DAT_07e11f34[13] = 0xFFFF0080u;
    DAT_07e11f34[14] = 0xFFFF00FFu;
    DAT_07e11f34[15] = 0xFF8000FFu;
}

// IDA: FUN_004F02F0 — renderiza el emblema de guild en pantalla.
void __cdecl RenderGuildMark_stub(float p1, float p2, float p3, float p4, int p5) {
    // IDA: FUN_004F02F0 — p5 indexa la paleta MarkColor[16] en DAT_07e11f34.
    // Bitmaps[34] es la textura temporal del emblema (Components, TextureNumber, Buffer).

    if (!GuildMark_EnsureRenderTarget()) return;

    float Width  = p3;
    float Height = p4;

    // Draw background box (texture 0x116) slightly larger
    GL_DrawTexture(0x116, p1 - _DAT_0055256c, p2 - _DAT_0055256c,
                 Width + _DAT_0055264c, Height + _DAT_0055264c,
                 0.0f, 0.0f, Width * _DAT_00552ae4, Height * _DAT_00552a20,
                 '\x01', '\x01');

    // IDA: el buffer se llena con las dimensiones físicas de Bitmaps[34], no
    // con p3/p4, que solamente determinan el tamaño del quad en pantalla.
    const int iVar2 = (int)Bitmaps[34].Width;
    const int iVar3 = (int)Bitmaps[34].Height;

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
