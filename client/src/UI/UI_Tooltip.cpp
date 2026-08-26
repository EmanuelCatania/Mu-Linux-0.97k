// UI_Tooltip.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" int Text_MeasureOrthoWidth(const char* text);

// RenderTipText @ 0x0047F7F0 (~62 lines) — renders tooltip box with text
// Measures text, draws black border + bg quad, renders white text, restores blend.
void __cdecl RenderTipText_stub(int sx, int sy, char* Text) {
    // 0x0047F7F0 — renders tooltip box with text
    // Measures text extent, draws black background box with 1px border lines,
    // renders semi-transparent fill, then white text on top. Restores prior blend mode.

    // Measure text extent.
    // 2026-07-20: IDA divide sz.cx/sz.cy por g_fScreenRate_x/y en sus SEIS usos
    // para pasar de pixeles a unidades de layout.  En nuestro build ese global
    // esta desincronizado del ortho/viewport reales (sale de g_ScreenW, que es
    // una variable distinta de DAT_0056156c), asi que usamos la escala derivada
    // del viewport de OpenGL -- ver Text_PixelToOrthoScale en stubs_externs.cpp.
    SIZE textSize = { 0, 0 };
    SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);  // g_hFont
    int textLen = lstrlenA(Text);
    GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, Text, textLen, &textSize);
    {
        int wOrtho = Text_MeasureOrthoWidth(Text);
        if (wOrtho > 0 && textSize.cx > 0) {
            float k = (float)wOrtho / (float)textSize.cx;   // 1 / (viewport/ortho)
            textSize.cx = wOrtho;
            textSize.cy = (LONG)((float)textSize.cy * k);
        }
    }

    // Save current blend mode for restoration
    int savedBlendType = DAT_083a412c;  // _AlphaBlendType

    // Enable alpha test
    FUN_00511680('\x01');  // EnableAlphaTest(true)

    // Draw black border + background
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

    float y = (float)sy - _DAT_00552540;
    float x = (float)sx - _DAT_0055264c;

    // Top border line (horizontal)
    FUN_005124c0(x, y,
                 (float)textSize.cx + _DAT_00552650, 1.0f);
    // Left border line (vertical)
    FUN_005124c0(x, y,
                 1.0f, (float)textSize.cy + _DAT_00552650);
    // Right border line (vertical)
    FUN_005124c0((float)textSize.cx + x + _DAT_00552540, y,
                 1.0f, (float)textSize.cy + _DAT_00552650);
    // Bottom border line (horizontal)
    FUN_005124c0(x, (float)textSize.cy + y + _DAT_00552540,
                 (float)textSize.cx + _DAT_00552650, 1.0f);

    // Semi-transparent black fill
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);  // 0x3f4ccccd ≈ 0.8
    FUN_005124c0((float)sx - _DAT_0055256c, (float)sy - _DAT_0055264c,
                 (float)textSize.cx + _DAT_0055264c,
                 (float)textSize.cy + _DAT_0055264c);

    // White text
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(0xDE1);  // GL_TEXTURE_2D

    // Set text colors
    m_dwBackColor = 0;
    DAT_00559c78 = 0xffffffff;  // m_dwTextColor = white

    // Render text (original uses 8-param thiscall via DAT_055c9ff8; using 5-param stub)
    // Full call: CUIRenderText__RenderText(this=DAT_055c9ff8, sx, sy, Text, 0, 0, 1, NULL, 0x280)
    FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, sx, sy, Text, 0);

    // Restore prior blend mode via switch
    switch (savedBlendType) {
        case 1:  FUN_00511890();    break;  // EnableLightMap
        case 2:  FUN_00511680('\x01'); break;  // EnableAlphaTest(true)
        case 3:  FUN_00511710();    break;  // EnableAlphaBlend
        case 4:  FUN_00511790();    break;  // EnableAlphaBlendMinus
        case 5:  EnableAlphaBlend2_stub();  break;  // EnableAlphaBlend2
        default: FUN_00511600();    break;  // DisableAlphaBlend
    }
}
