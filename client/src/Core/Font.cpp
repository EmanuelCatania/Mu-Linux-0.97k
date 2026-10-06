// Font.cpp — CFont. Ver Font.h.

#include "stdafx.h"
#include "Core/Font.h"

CFont gFont;

// Alto de la fuente base. Lo leen el render de texto y el layout de los paneles.
extern int FontHeight;

void CFont::Create(DWORD windowWidth)
{
    // IDA WinMain (paso 15): el alto depende de la resolución.
    int fontSize = 0x0c;                          // 640x480
    if      (windowWidth == 0x320) fontSize = 0x0d;   // 800
    else if (windowWidth == 0x400) fontSize = 0x0e;   // 1024
    else if (windowWidth >= 0x500) fontSize = 0x0f;   // 1280 o más
    FontHeight = fontSize;

    // DESVIACION: el binario usa 129 (HANGEUL_CHARSET) porque su Text.bmd es
    // coreano. El nuestro está en Windows-1252, y con HANGEUL_CHARSET la GDI
    // toma los bytes 0x81..0xFE como primer byte de un par DBCS y se come el
    // carácter siguiente ("da? o" en vez de "daño"). En el original sale de
    // g_dwCharSet según el idioma; para datos en 1252 corresponde DEFAULT_CHARSET.
    const DWORD charSet = DEFAULT_CHARSET;

    m_Fonts[FONT_NORMAL] = CreateFontA(fontSize, 0, 0, 0, 400, 0, 0, 0, charSet, 0, 0, 0, 0, "Arial");
    m_Fonts[FONT_BOLD]   = CreateFontA(fontSize, 0, 0, 0, 700, 0, 0, 0, charSet, 0, 0, 0, 0, "Arial");
    // IDA WinMain 0x41F151: el doble de la base (FontHeight - 1), bold.
    m_Fonts[FONT_BIG]    = CreateFontA(2 * (fontSize - 1), 0, 0, 0, 700, 0, 0, 0, charSet, 0, 0, 0, 0, "Arial");
}

void CFont::Release()
{
    for (HFONT& font : m_Fonts) {
        if (font) DeleteObject(font);
        font = nullptr;
    }
}
