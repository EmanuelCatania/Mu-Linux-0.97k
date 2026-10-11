// Font.cpp — CFont. Ver Font.h.

#include "stdafx.h"
#include <algorithm>
#include "Core/Font.h"
#include "Config/UserSettings.h"

CFont gFont;

void CFont::Create(DWORD windowWidth)
{
    const UserFontSettings& cfg = gUserSettings.GetFont();

    if (cfg.present) {
        // DESVIACION (DLL, Font.cpp): la fuente sale de Config.ini [Font]. El
        // alto es fijo (no depende de la resolución), con tope 25; la fuente
        // común puede ir en negrita (FontBold) y la grande mide el doble.
        m_Height = cfg.height;
        const DWORD pitch = DEFAULT_PITCH;
        m_Fonts[FONT_NORMAL] = CreateFontA(m_Height, cfg.width, 0, 0,
                                           cfg.bold ? FW_BOLD : FW_NORMAL,
                                           cfg.italic, cfg.underline, cfg.strikeOut,
                                           cfg.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           cfg.quality, pitch, cfg.faceName);
        m_Fonts[FONT_BOLD]   = CreateFontA(m_Height, cfg.width, 0, 0, FW_BOLD,
                                           cfg.italic, cfg.underline, cfg.strikeOut,
                                           cfg.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           cfg.quality, pitch, cfg.faceName);
        m_Fonts[FONT_BIG]    = CreateFontA(m_Height * 2, cfg.width, 0, 0, FW_BOLD,
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
    m_Height = fontSize;

    // DESVIACION: el binario usa 129 (HANGEUL_CHARSET) porque su Text.bmd es
    // coreano. El nuestro está en Windows-1252, y con HANGEUL_CHARSET la GDI
    // toma los bytes 0x81..0xFE como primer byte de un par DBCS y se come el
    // carácter siguiente ("da? o" en vez de "daño"). En el original sale de
    // g_dwCharSet según el idioma; para datos en 1252 corresponde DEFAULT_CHARSET.
    const DWORD charSet = DEFAULT_CHARSET;

    m_Fonts[FONT_NORMAL] = CreateFontA(fontSize, 0, 0, 0, 400, 0, 0, 0, charSet, 0, 0, 0, 0, "Arial");
    m_Fonts[FONT_BOLD]   = CreateFontA(fontSize, 0, 0, 0, 700, 0, 0, 0, charSet, 0, 0, 0, 0, "Arial");
    // IDA WinMain 0x41F151: el doble de la base (m_Height - 1), bold.
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

namespace {
int CALLBACK CollectFace(const LOGFONTA* lf, const TEXTMETRICA*, DWORD, LPARAM param)
{
    // Las familias con '@' son las variantes verticales de las fuentes asiáticas.
    if (lf->lfFaceName[0] && lf->lfFaceName[0] != '@')
        reinterpret_cast<std::vector<std::string>*>(param)->push_back(lf->lfFaceName);
    return 1;
}
}

int CFont::GetFaceCount()
{
    if (m_Faces.empty()) {
        LOGFONTA lf = {};
        lf.lfCharSet = DEFAULT_CHARSET;
        HDC dc = GetDC(NULL);
        EnumFontFamiliesExA(dc, &lf, CollectFace, (LPARAM)&m_Faces, 0);
        ReleaseDC(NULL, dc);
        std::sort(m_Faces.begin(), m_Faces.end(), [](const std::string& a, const std::string& b) {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        });
        m_Faces.erase(std::unique(m_Faces.begin(), m_Faces.end(), [](const std::string& a, const std::string& b) {
            return _stricmp(a.c_str(), b.c_str()) == 0;
        }), m_Faces.end());
    }
    return (int)m_Faces.size();
}

const char* CFont::GetFaceName(int index)
{
    return index >= 0 && index < GetFaceCount() ? m_Faces[index].c_str() : "";
}

int CFont::FindFace(const char* name)
{
    const int count = GetFaceCount();
    for (int i = 0; i < count; ++i)
        if (_stricmp(m_Faces[i].c_str(), name) == 0) return i;
    return -1;
}
