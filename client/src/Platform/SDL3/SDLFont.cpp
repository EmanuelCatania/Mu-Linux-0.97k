#include "Platform/SDL3/SDLFont.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_MODULE_H
#include <algorithm>
#include <limits>
#include <unordered_map>

namespace Platform {
namespace {
std::vector<uint8_t> ReadTable(FT_Face face, FT_ULong tag)
{
    FT_ULong size = 0;
    if (FT_Load_Sfnt_Table(face, tag, 0, nullptr, &size)) return {};
    std::vector<uint8_t> bytes(size);
    if (FT_Load_Sfnt_Table(face, tag, 0, bytes.data(), &size)) return {};
    return bytes;
}
unsigned U16(const std::vector<uint8_t>& bytes, size_t offset)
{
    return (unsigned(bytes[offset]) << 8) | bytes[offset + 1];
}
// VDMX describe la celda rasterizada, no sólo el EM. Sin perfiles de otra fuente.
bool SelectVDMX(FT_Face face, int height, int& ppem, FontMetrics& metrics)
{
    const auto bytes = ReadTable(face, FT_MAKE_TAG('V', 'D', 'M', 'X'));
    if (bytes.size() < 6) return false;
    const unsigned ratios = U16(bytes, 4);
    if (6 + size_t(ratios) * 6 > bytes.size()) return false;
    for (unsigned i = 0; i < ratios; ++i) {
        const size_t ratio = 6 + i * 4;
        if (!((bytes[ratio + 1] == 1 && bytes[ratio + 2] <= 1 && bytes[ratio + 3] >= 1) ||
              (bytes[ratio + 1] == 0 && bytes[ratio + 2] == 0 && bytes[ratio + 3] == 0))) continue;
        const size_t group = U16(bytes, 6 + ratios * 4 + i * 2);
        if (group + 4 > bytes.size()) return false;
        const unsigned count = U16(bytes, group);
        if (group + 4 + size_t(count) * 6 > bytes.size()) return false;
        bool found = false;
        for (unsigned j = 0; j < count; ++j) {
            const size_t at = group + 4 + j * 6;
            const int top = int16_t(U16(bytes, at + 2));
            const int bottom = int16_t(U16(bytes, at + 4));
            if (top - bottom > height) break;
            if (top <= bottom || U16(bytes, at) == 0) continue;
            ppem = U16(bytes, at);
            metrics = { top, top - bottom };
            found = true;
            // La primera coincidencia exacta conserva el tamaño GDI del banco.
            if (metrics.height == height) break;
        }
        return found;
    }
    return false;
}
}

struct SDLFont::Impl {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
    std::vector<uint8_t> bytes;
    FontMetrics metrics;
    std::unordered_map<uint32_t, FontGlyph> glyphs;
    ~Impl() {
        if (face) FT_Done_Face(face);
        if (library) FT_Done_FreeType(library);
    }
};
SDLFont::SDLFont() : impl(new Impl) {}
SDLFont::~SDLFont() = default;

bool SDLFont::Load(std::vector<uint8_t> bytes, int height, std::string& error)
{
    error.clear();
    if (height <= 0 || height > 4096 || bytes.empty() || bytes.size() > size_t(std::numeric_limits<FT_Long>::max())) {
        error = "Tamaño o archivo de fuente inválido";
        return false;
    }
    std::unique_ptr<Impl> next(new Impl);
    next->bytes = std::move(bytes);
    if (FT_Init_FreeType(&next->library)) { error = "No se pudo iniciar FreeType"; return false; }
    unsigned interpreter = 35;
    if (FT_Property_Set(next->library, "truetype", "interpreter-version", &interpreter)) {
        error = "No se pudo seleccionar el intérprete TrueType"; return false;
    }
    if (FT_New_Memory_Face(next->library, next->bytes.data(), FT_Long(next->bytes.size()), 0, &next->face) ||
        !FT_IS_SCALABLE(next->face) || FT_Select_Charmap(next->face, FT_ENCODING_UNICODE)) {
        error = "La fuente no es escalable o no tiene mapa Unicode"; return false;
    }
    auto face = next->face;
    const auto os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
    const int span = os2 ? os2->usWinAscent + os2->usWinDescent : face->ascender - face->descender;
    if (span <= 0 || face->units_per_EM == 0) { error = "Métricas de fuente inválidas"; return false; }
    int ppem = std::max(1, (height * int(face->units_per_EM) + span / 2) / span);
    const bool vdmx = SelectVDMX(face, height, ppem, next->metrics);
    if (FT_Set_Pixel_Sizes(face, 0, ppem)) { error = "No se pudo establecer el tamaño de fuente"; return false; }
    if (!vdmx) {
        const int ascent = os2 ? os2->usWinAscent : face->ascender;
        next->metrics = { (ascent * height + span / 2) / span, height };
    }
    impl.swap(next);
    return true;
}

const FontGlyph& SDLFont::Glyph(uint32_t codepoint)
{
    const auto existing = impl->glyphs.find(codepoint);
    if (existing != impl->glyphs.end()) return existing->second;
    FontGlyph glyph;
    if (impl->face && !FT_Load_Char(impl->face, codepoint, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO)) {
        const auto slot = impl->face->glyph;
        const auto& bitmap = slot->bitmap;
        glyph.advance = int((slot->advance.x + 32) / 64);
        glyph.left = slot->bitmap_left;
        glyph.top = slot->bitmap_top;
        glyph.width = int(bitmap.width);
        glyph.height = int(bitmap.rows);
        glyph.coverage.resize(size_t(glyph.width) * glyph.height);
        for (int y = 0; y < glyph.height; ++y) {
            const auto row = bitmap.buffer + (bitmap.pitch >= 0 ? y : glyph.height - 1 - y) * std::abs(bitmap.pitch);
            for (int x = 0; x < glyph.width; ++x)
                glyph.coverage[size_t(y) * glyph.width + x] = (row[x / 8] & (0x80 >> (x % 8))) ? 255 : 0;
        }
    }
    return impl->glyphs.emplace(codepoint, std::move(glyph)).first->second;
}
FontMetrics SDLFont::Metrics() const { return impl->metrics; }
int SDLFont::Measure(const uint32_t* text, size_t count)
{
    int width = 0;
    if (!text) return width;
    for (size_t i = 0; i < count; ++i) {
        const int advance = Glyph(text[i]).advance;
        if (advance > std::numeric_limits<int>::max() - width) return std::numeric_limits<int>::max();
        width += advance;
    }
    return width;
}
}
