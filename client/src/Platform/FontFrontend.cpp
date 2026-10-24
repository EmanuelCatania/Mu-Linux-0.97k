// Font.cpp — CFont. Ver Font.h.

#include "stdafx.h"
#include "Render/Backend/GL/LegacyGL.h"
#include <algorithm>
#ifndef _WIN32
#include <strings.h>
#define _stricmp strcasecmp
#endif
#include "Core/Font.h"
#include "Config/UserSettings.h"
#include "Text/TextCore.h"
#ifdef MU_SDL_FONTS
#include "Platform/SDL3/SDLFont.h"
#include "Text/TextCore.h"
#include "Text/TextShaper.h"
#include "Text/TextAtlas.h"
#include <stdexcept>
namespace {
Platform::SDLFont portableFonts[MAX_FONT_TYPES];
TextEngine::FontStack portableStacks[MAX_FONT_TYPES];
}
#endif

CFont gFont;

HGDIOBJ CFont::SelectFont(HDC dc, HGDIOBJ font)
{
#ifdef _WIN32
    return ::SelectObject(dc, font);
#else
    if (dc != m_hTextDC) return nullptr;
    for (int i = 0; i < MAX_FONT_TYPES; ++i) {
        if (font == m_Fonts[i]) {
            auto previous = m_Fonts[m_SelectedFont];
            m_SelectedFont = i;
            return previous;
        }
    }
    return nullptr;
#endif
}
HFONT CFont::CurrentFont(HDC dc) const
{
#ifdef _WIN32
    return (HFONT)::GetCurrentObject(dc, OBJ_FONT);
#else
    return dc == m_hTextDC ? m_Fonts[m_SelectedFont] : nullptr;
#endif
}


void CFont::Create(DWORD windowWidth)
{
    const UserFontSettings& cfg = gUserSettings.GetFont();
#ifndef _WIN32
    m_Height = cfg.present ? cfg.height : windowWidth == 800 ? 13 : windowWidth == 1024 ? 14 : windowWidth >= 1280 ? 15 : 12;
    CreatePortableFonts(windowWidth);
    for (int i = 0; i < MAX_FONT_TYPES; ++i)
        m_Fonts[i] = reinterpret_cast<HFONT>(&portableFonts[i]);
    m_SelectedFont = FONT_NORMAL;
#else
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
        CreatePortableFonts(windowWidth);
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
    CreatePortableFonts(windowWidth);
#endif
}

void CFont::CreateTextSurface(HDC hdc, LONG width, LONG height)
{
    m_SurfaceWidth = width;
    m_SurfaceHeight = height;
#ifdef MU_SDL_FONTS
    // El DC conserva temporalmente la selección HFONT de los callers Windows.
    // FreeType escribe en memoria propia, nunca en una superficie GDI.
#ifdef _WIN32
    m_hTextDC = CreateCompatibleDC(hdc);
#else
    m_hTextDC = reinterpret_cast<HDC>(this);
#endif
    m_Mask.assign(size_t((width * 3 + 3) & ~3) * height, 0);
    m_pTextBits = m_Mask.data();
    return;
#else
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
#endif
}

void CFont::Reload(DWORD windowWidth)
{
    for (HFONT& font : m_Fonts) {
#ifdef _WIN32
        if (font) DeleteObject(font);
#endif
        font = nullptr;
    }
    Create(windowWidth);
    if (m_hTextDC) SelectFont(m_hTextDC, m_Fonts[FONT_NORMAL]);
}

void CFont::Release()
{
    for (HFONT& font : m_Fonts) {
#ifdef _WIN32
        if (font) DeleteObject(font);
#endif
        font = nullptr;
    }
#ifdef _WIN32
    if (m_hTextDC)     DeleteDC(m_hTextDC);
    if (m_hTextBitmap) DeleteObject(m_hTextBitmap);
#else
    m_Mask.clear();
#endif
    m_hTextDC     = NULL;
    m_hTextBitmap = NULL;
    m_pTextBits   = nullptr;
}

#ifdef _WIN32
namespace {
int CALLBACK CollectFace(const LOGFONTA* lf, const TEXTMETRICA*, DWORD, LPARAM param)
{
    // Las familias con '@' son las variantes verticales de las fuentes asiáticas.
    if (lf->lfFaceName[0] && lf->lfFaceName[0] != '@')
        reinterpret_cast<std::vector<std::string>*>(param)->push_back(lf->lfFaceName);
    return 1;
}
}

#endif
int CFont::GetFaceCount()
{
    if (m_Faces.empty()) {
#ifdef MU_SDL_FONTS
        m_Faces.emplace_back("Liberation Sans");
#endif
#ifdef _WIN32
        LOGFONTA lf = {};
        lf.lfCharSet = DEFAULT_CHARSET;
        HDC dc = GetDC(NULL);
        EnumFontFamiliesExA(dc, &lf, CollectFace, (LPARAM)&m_Faces, 0);
        ReleaseDC(NULL, dc);
#else
        auto installed = Platform::SDLFont::InstalledFamilies();
        m_Faces.insert(m_Faces.end(), installed.begin(), installed.end());
#endif
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

void CFont::CreatePortableFonts(DWORD windowWidth)
{
    ++m_Generation;
#ifdef MU_SDL_FONTS
    const auto& cfg = gUserSettings.GetFont();
    for (int i = 0; i < MAX_FONT_TYPES; ++i) {
        const int height = i == FONT_BIG ? (cfg.present ? m_Height * 2 : (m_Height - 1) * 2) : m_Height;
        std::string error;
        if (!portableFonts[i].LoadFamily(cfg.present ? cfg.faceName : "Arial",
            i != FONT_NORMAL || (cfg.present && cfg.bold), cfg.present && cfg.italic, height,
            cfg.present ? cfg.width : 0, cfg.present && cfg.underline, cfg.present && cfg.strikeOut, error))
            throw std::runtime_error(error);
        portableStacks[i].Reset(&portableFonts[i], height, i != FONT_NORMAL || (cfg.present && cfg.bold));
    }
    // Las entradas del atlas apuntan a las caras anteriores.
    TextAtlas::Release();
#endif
    (void)windowWidth;
}

int CFont::SelectedFont(HDC dc) const
{
    const auto selected = CurrentFont(dc);
    for (int i = 0; i < MAX_FONT_TYPES; ++i) if (selected == m_Fonts[i]) return i;
    return FONT_NORMAL;
}

const TextEngine::ShapedText* CFont::Shape(HDC dc, LPCSTR text, int count)
{
#ifdef MU_SDL_FONTS
    if (!text || count < 0) return nullptr;
    // El texto del cliente está en UTF-8 (Fase 4); lo arma el layout.
    return &portableStacks[SelectedFont(dc)].Shape(std::string_view(text, size_t(count)));
#else
    (void)dc; (void)text; (void)count;
    return nullptr;
#endif
}

BOOL CFont::MeasureText(HDC dc, LPCSTR text, int count, SIZE* size)
{
    RenderProfile::TextScope profileText(RenderProfile::TextWork::Measure);
#ifdef MU_SDL_FONTS
    if (!size || count < 0 || (!text && count)) return FALSE;
    *size = {};
    if (!count) return TRUE;
    size->cx = Shape(dc, text, count)->width;
    size->cy = portableFonts[SelectedFont(dc)].Metrics().height;
    return TRUE;
#else
    // DESVIACION (Fase 4): el texto está en UTF-8; GDI ANSI recibe 1252.
    const std::string legacy = TextEngine::ToWindows1252(std::string_view(text ? text : "", text ? size_t(count) : 0));
    return GetTextExtentPointA(dc, legacy.c_str(), (int)legacy.size(), size);
#endif
}
BOOL CFont::MeasureText32(HDC dc, LPCSTR text, int count, SIZE* size)
{
#ifdef MU_SDL_FONTS
    return MeasureText(dc, text, count, size);
#else
    RenderProfile::TextScope profileText(RenderProfile::TextWork::Measure);
    const std::string legacy = TextEngine::ToWindows1252(std::string_view(text ? text : "", text ? size_t(count) : 0));
    return GetTextExtentPoint32A(dc, legacy.c_str(), (int)legacy.size(), size);
#endif
}
BOOL CFont::TextMetrics(HDC dc, TEXTMETRICA* metrics)
{
#ifdef MU_SDL_FONTS
    if (!metrics) return FALSE;
    *metrics = {};
    const auto value = portableFonts[SelectedFont(dc)].Metrics();
    metrics->tmHeight = value.height;
    metrics->tmAscent = value.ascent;
    metrics->tmDescent = value.height - value.ascent;
    return TRUE;
#else
    return GetTextMetricsA(dc, metrics);
#endif
}
void CFont::SetMaskColor(DWORD color)
{
    m_MaskColor = color;
#ifdef _WIN32
    SetTextColor(m_hTextDC, color);
#endif
}
void CFont::BeginTextMask()
{
    RenderProfile::TextScope profileText(RenderProfile::TextWork::Raster);
#ifdef MU_SDL_FONTS
    std::fill(m_Mask.begin(), m_Mask.end(), 0);
#endif
}
BOOL CFont::DrawTextMask(HDC dc, int x, int y, LPCSTR text, int count)
{
    RenderProfile::TextScope profileText(RenderProfile::TextWork::Raster);
#ifdef MU_SDL_FONTS
    if (!m_pTextBits || !text || count < 0) return FALSE;
    auto& font = portableFonts[SelectedFont(dc)];
    const int baseline = y + font.Metrics().ascent;
    const int stride = (m_SurfaceWidth * 3 + 3) & ~3;
    const TextEngine::ShapedText* shaped = count ? Shape(dc, text, count) : nullptr;
    for (size_t i = 0; shaped && i < shaped->glyphs.size(); ++i) {
        const auto& placed = shaped->glyphs[i];
        const auto& glyph = placed.font->GlyphIndex(placed.glyph);
        for (int row = 0; row < glyph.height; ++row) {
            const int dy = baseline - placed.y - glyph.top + row;
            if (dy < 0 || dy >= m_SurfaceHeight) continue;
            for (int col = 0; col < glyph.width; ++col) {
                const int dx = x + placed.x + glyph.left + col;
                if (dx < 0 || dx >= m_SurfaceWidth || !glyph.coverage[size_t(row) * glyph.width + col]) continue;
                auto* pixel = static_cast<unsigned char*>(m_pTextBits) + dy * stride + dx * 3;
                pixel[0] = (m_MaskColor >> 16) & 255;
                pixel[1] = (m_MaskColor >> 8) & 255;
                pixel[2] = m_MaskColor & 255;
            }
        }
    }
    return TRUE;
#else
    const std::string legacy = TextEngine::ToWindows1252(std::string_view(text ? text : "", text ? size_t(count) : 0));
    return TextOutA(dc, x, y, legacy.c_str(), (int)legacy.size());
#endif
}
BOOL CFont::BuildGlyphLists(HDC dc, unsigned first, unsigned count, unsigned base)
{
    RenderProfile::TextScope profileText(RenderProfile::TextWork::Raster);
#ifdef MU_SDL_FONTS
    auto& font = portableFonts[SelectedFont(dc)];
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    for (unsigned i = 0; i < count; ++i) {
        const auto& glyph = font.Glyph(Platform::SDLFont::DecodeByte(static_cast<unsigned char>(first + i)));
        const int stride = (glyph.width + 7) / 8;
        std::vector<unsigned char> bitmap(size_t(stride) * glyph.height, 0);
        for (int y = 0; y < glyph.height; ++y)
            for (int x = 0; x < glyph.width; ++x)
                if (glyph.coverage[size_t(y) * glyph.width + x])
                    bitmap[size_t(glyph.height - 1 - y) * stride + x / 8] |= 0x80 >> (x % 8);
        glNewList(base + i, GL_COMPILE);
        glBitmap(glyph.width, glyph.height, GLfloat(-glyph.left), GLfloat(glyph.height - glyph.top),
                 GLfloat(glyph.advance), 0, bitmap.empty() ? nullptr : bitmap.data());
        glEndList();
    }
    glPopClientAttrib();
    return TRUE;
#else
    return wglUseFontBitmapsA(dc, first, count, base);
#endif
}
