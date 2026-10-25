#include "stdafx.h"
#include "Text/TextAtlas.h"
#ifdef MU_SDL_FONTS
#include "Text/TextShaper.h"
#include "Platform/SDL3/SDLFont.h"
#include "Render/BufferedDraw.h"
#include "Config/UserSettings.h"
#include <unordered_map>
#include <vector>

namespace TextAtlas {
namespace {
constexpr int kPageSize = 512;
// Con el texto de la UI (tres fuentes, alfabeto latino) entra en una página.
// Si se llenan todas, se descartan juntas y se vuelven a llenar con lo que se
// sigue usando: más simple que un LRU por glifo y casi nunca ocurre.
constexpr int kMaxPages = 8;

struct Entry { int page, x, y, w, h, left, top; };
struct Page { Gfx::Texture texture = 0; int shelfX = 0, shelfY = 0, shelfH = 0; };

struct Key {
    const Platform::SDLFont* font; uint32_t glyph;
    bool operator==(const Key& o) const { return font == o.font && glyph == o.glyph; }
};
struct KeyHash {
    size_t operator()(const Key& k) const { return std::hash<const void*>()(k.font) ^ (size_t(k.glyph) * 0x9E3779B1u); }
};

std::vector<Page> pages;
std::unordered_map<Key, Entry, KeyHash> entries;

bool NewPage()
{
    if ((int)pages.size() >= kMaxPages) return false;
    Page page;
    page.texture = Gfx::CreateTexture();
    Gfx::BindTexture(page.texture);
    Gfx::TexParameter(Gfx::TexParam::MinFilter, Gfx::TexValue::Nearest);
    Gfx::TexParameter(Gfx::TexParam::MagFilter, Gfx::TexValue::Nearest);
    Gfx::TexParameter(Gfx::TexParam::WrapS, Gfx::TexValue::ClampToEdge);
    Gfx::TexParameter(Gfx::TexParam::WrapT, Gfx::TexValue::ClampToEdge);
    std::vector<unsigned char> clear(size_t(kPageSize) * kPageSize * 4, 0);
    Gfx::TexImage2D(4, kPageSize, kPageSize, Gfx::Format::RGBA, clear.data());
    pages.push_back(page);
    return true;
}

// Lugar en la página actual por estantes (filas de la altura del glifo más alto).
bool Place(int w, int h, int& page, int& x, int& y)
{
    if (w + 2 > kPageSize || h + 2 > kPageSize) return false;
    if (pages.empty() && !NewPage()) return false;
    Page* p = &pages.back();
    if (p->shelfX + w + 2 > kPageSize) { p->shelfY += p->shelfH; p->shelfX = 0; p->shelfH = 0; }
    if (p->shelfY + h + 2 > kPageSize) {
        if (!NewPage()) return false;
        p = &pages.back();
    }
    page = int(pages.size()) - 1;
    x = p->shelfX + 1; y = p->shelfY + 1;
    p->shelfX += w + 2;
    if (h + 2 > p->shelfH) p->shelfH = h + 2;
    return true;
}

const Entry* Find(Platform::SDLFont* font, uint32_t glyph)
{
    const Key key{ font, glyph };
    const auto found = entries.find(key);
    if (found != entries.end()) return &found->second;
    const auto& raster = font->GlyphIndex(glyph);
    Entry entry{ -1, 0, 0, raster.width, raster.height, raster.left, raster.top };
    if (raster.width > 0 && raster.height > 0) {
        if (!Place(raster.width, raster.height, entry.page, entry.x, entry.y)) {
            Release();
            if (!Place(raster.width, raster.height, entry.page, entry.x, entry.y)) return nullptr;
        }
        // Blanco con la cobertura en alfa: el color lo pone el vértice (MODULATE).
        std::vector<unsigned char> rgba(size_t(raster.width) * raster.height * 4, 255);
        for (size_t i = 0; i < raster.coverage.size(); ++i) rgba[i * 4 + 3] = raster.coverage[i];
        Gfx::BindTexture(pages[entry.page].texture);
        Gfx::SetUnpackAlignment(1);
        Gfx::TexSubImage2D(entry.x, entry.y, entry.w, entry.h, Gfx::Format::RGBA, rgba.data());
    }
    return &entries.emplace(key, entry).first->second;
}
} // namespace

void Draw(const TextEngine::ShapedText& text, float x, float baseline,
          unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (text.glyphs.empty()) return;
    // Las subidas ligan texturas: hacerlas dentro del Push para que la ligadura
    // vuelva a la que espera la caché de estado del juego (FUN_00511480).
    const int previousAlignment = Gfx::UnpackAlignment();
    Gfx::PushAttrib(Gfx::AttribEnable | Gfx::AttribTexture);
    // Resolver primero todos los glifos: subir uno nuevo puede descartar las
    // páginas, y entonces las entradas ya calculadas dejarían de valer.
    std::vector<const Entry*> resolved;
    resolved.reserve(text.glyphs.size());
    const size_t pageCount = pages.size();
    for (const auto& glyph : text.glyphs) resolved.push_back(glyph.font ? Find(glyph.font, glyph.glyph) : nullptr);
    if (pages.size() < pageCount) {
        resolved.clear();
        for (const auto& glyph : text.glyphs) resolved.push_back(glyph.font ? Find(glyph.font, glyph.glyph) : nullptr);
    }
    Gfx::Enable(Gfx::Cap::Texture2D);
    Gfx::TexEnvMode(Gfx::TexEnv::Modulate);
    const float originX = std::floor(x), originY = std::floor(baseline);
    // Contorno o sombra: el mismo texto en negro corrido de a un píxel, debajo
    // del color. El alfa es el del texto, así no se ve más fuerte que él.
    // (y va hacia arriba: la sombra cae a y - 1.)
    const auto pass = [&](float dx, float dy, unsigned char pr, unsigned char pg, unsigned char pb) {
    for (int page = 0; page < (int)pages.size(); ++page) {
        bool any = false;
        for (size_t i = 0; i < text.glyphs.size(); ++i) {
            const Entry* e = resolved[i];
            if (!e || e->page != page) continue;
            if (!any) {
                Gfx::BindTexture(pages[page].texture);
                BufferedDraw::Color4ub(pr, pg, pb, a);
                BufferedDraw::Begin(BufferedDraw::Primitive::Quads);
                any = true;
            }
            const float x0 = originX + dx + text.glyphs[i].x + e->left;
            const float y1 = originY + dy + text.glyphs[i].y + e->top;
            const float x1 = x0 + e->w, y0 = y1 - e->h;
            const float u0 = float(e->x) / kPageSize, u1 = float(e->x + e->w) / kPageSize;
            const float v0 = float(e->y) / kPageSize, v1 = float(e->y + e->h) / kPageSize;
            BufferedDraw::TexCoord2f(u0, v1); BufferedDraw::Vertex2f(x0, y0);
            BufferedDraw::TexCoord2f(u1, v1); BufferedDraw::Vertex2f(x1, y0);
            BufferedDraw::TexCoord2f(u1, v0); BufferedDraw::Vertex2f(x1, y1);
            BufferedDraw::TexCoord2f(u0, v0); BufferedDraw::Vertex2f(x0, y1);
        }
        if (any) BufferedDraw::End();
    }
    };
    if (gUserSettings.GetTextOutline()) {
        for (int oy = -1; oy <= 1; ++oy)
            for (int ox = -1; ox <= 1; ++ox)
                if (ox || oy) pass(float(ox), float(oy), 0, 0, 0);
    } else if (gUserSettings.GetTextShadow()) {
        pass(1.0f, -1.0f, 0, 0, 0);
    }
    pass(0.0f, 0.0f, r, g, b);
    Gfx::PopAttrib();
    Gfx::SetUnpackAlignment(previousAlignment);
}

unsigned long long Bytes() { return (unsigned long long)pages.size() * kPageSize * kPageSize * 4; }

void Release()
{
    for (auto& page : pages) if (page.texture) Gfx::DeleteTexture(page.texture);
    pages.clear();
    entries.clear();
}
} // namespace TextAtlas
#else
namespace TextAtlas {
void Draw(const TextEngine::ShapedText&, float, float, unsigned char, unsigned char, unsigned char, unsigned char) {}
unsigned long long Bytes() { return 0; }
void Release() {}
}
#endif
