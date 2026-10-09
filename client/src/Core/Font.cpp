// Font.cpp — CFont. Ver Font.h.

#include "stdafx.h"
#include "Core/Font.h"
#include "Config/UserSettings.h"

CFont gFont;

// Alto de la fuente base. Lo leen el render de texto y el layout de los paneles.
extern int FontHeight;

void CFont::Create(DWORD windowWidth)
{
    const UserFontSettings& cfg = gUserSettings.GetFont();

    if (cfg.present) {
        // DESVIACION (DLL, Font.cpp): la fuente sale de Config.ini [Font]. El
        // alto es fijo (no depende de la resolución), con tope 25; la fuente
        // común puede ir en negrita (FontBold) y la grande mide el doble.
        FontHeight = cfg.height;
        const DWORD pitch = DEFAULT_PITCH;
        m_Fonts[FONT_NORMAL] = CreateFontA(FontHeight, cfg.width, 0, 0,
                                           cfg.bold ? FW_BOLD : FW_NORMAL,
                                           cfg.italic, cfg.underline, cfg.strikeOut,
                                           cfg.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           cfg.quality, pitch, cfg.faceName);
        m_Fonts[FONT_BOLD]   = CreateFontA(FontHeight, cfg.width, 0, 0, FW_BOLD,
                                           cfg.italic, cfg.underline, cfg.strikeOut,
                                           cfg.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           cfg.quality, pitch, cfg.faceName);
        m_Fonts[FONT_BIG]    = CreateFontA(FontHeight * 2, cfg.width, 0, 0, FW_BOLD,
                                           cfg.italic, cfg.underline, cfg.strikeOut,
                                           cfg.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           cfg.quality, pitch, cfg.faceName);
        return;
    }

    // Sin [Font] en Config.ini: como el binario (WinMain, paso 15). El alto
    // depende de la resolución.
    int fontSize = 0x0c;                              // 640x480
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

void CFont::CreateTextSurface(HDC hdc, LONG width, LONG height)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = width;
    bmi.bmiHeader.biHeight      = -height;   // de arriba hacia abajo
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    m_hTextBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &m_pTextBits, nullptr, 0);
    m_hTextDC     = CreateCompatibleDC(hdc);
    SelectObject(m_hTextDC, m_hTextBitmap);
    SetBkMode(m_hTextDC, TRANSPARENT);
}

void CFont::Reload(DWORD windowWidth)
{
    for (HFONT& font : m_Fonts) {
        if (font) DeleteObject(font);
        font = nullptr;
    }
    Create(windowWidth);
    if (m_hTextDC) SelectObject(m_hTextDC, m_Fonts[FONT_NORMAL]);
}

void CFont::Release()
{
    for (HFONT& font : m_Fonts) {
        if (font) DeleteObject(font);
        font = nullptr;
    }
    if (m_hTextDC)     DeleteDC(m_hTextDC);
    if (m_hTextBitmap) DeleteObject(m_hTextBitmap);
    m_hTextDC     = NULL;
    m_hTextBitmap = NULL;
    m_pTextBits   = nullptr;
}
