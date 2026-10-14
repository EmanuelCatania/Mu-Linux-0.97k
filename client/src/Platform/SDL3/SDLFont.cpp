#include "Platform/SDL3/SDLFont.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_MODULE_H
#include <algorithm>
#include <limits>
#include <unordered_map>
#include "Platform/IFileSystem.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <cctype>

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
    bool underline = false;
    bool strikeOut = false;
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
    if (impl->face && (impl->underline || impl->strikeOut) && glyph.advance > 0) {
        const int underlineY = 1;
        const int strikeY = -impl->metrics.ascent / 3;
        const int left = std::min(0, glyph.left);
        const int right = std::max(glyph.advance, glyph.left + glyph.width);
        const int top = std::min(-glyph.top, impl->strikeOut ? strikeY : underlineY);
        const int bottom = std::max(glyph.height - glyph.top, impl->underline ? underlineY + 1 : strikeY + 1);
        std::vector<uint8_t> decorated(size_t(right - left) * (bottom - top), 0);
        for (int y = 0; y < glyph.height; ++y)
            for (int x = 0; x < glyph.width; ++x)
                decorated[size_t(y - glyph.top - top) * (right - left) + x + glyph.left - left] = glyph.coverage[size_t(y) * glyph.width + x];
        for (int x = 0; x < glyph.advance; ++x) {
            if (impl->underline) decorated[size_t(underlineY - top) * (right - left) + x - left] = 255;
            if (impl->strikeOut) decorated[size_t(strikeY - top) * (right - left) + x - left] = 255;
        }
        glyph.left = left; glyph.top = -top; glyph.width = right - left; glyph.height = bottom - top;
        glyph.coverage.swap(decorated);
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

namespace {
std::string LowerFamily(std::string value)
{
    for (char& c : value) c = char(std::tolower(static_cast<unsigned char>(c)));
    return value;
}
std::vector<uint8_t> ReadFont(const std::string& path)
{
    FILE* stream = GetFileSystem().Open(path.c_str(), "rb");
    if (!stream) return {};
    std::vector<uint8_t> bytes;
    uint8_t block[8192];
    size_t count;
    while ((count = std::fread(block, 1, sizeof(block), stream)) != 0) {
        if (bytes.size() + count > 64 * 1024 * 1024) { bytes.clear(); break; }
        bytes.insert(bytes.end(), block, block + count);
    }
    if (std::ferror(stream)) bytes.clear();
    std::fclose(stream);
    return bytes;
}
// Se inspeccionan nombres de familia y estilo del archivo, no nombres de archivo.
// La búsqueda se cachea y no necesita fontconfig ni paquetes FreeType del sistema.
const std::unordered_map<std::string, std::string>& InstalledFonts()
{
    static const auto fonts = [] {
        std::unordered_map<std::string, std::string> result;
        std::vector<std::filesystem::path> roots;
#ifdef _WIN32
        if (const char* root = std::getenv("WINDIR")) roots.emplace_back(std::filesystem::path(root) / "Fonts");
        if (const char* root = std::getenv("LOCALAPPDATA")) roots.emplace_back(std::filesystem::path(root) / "Microsoft/Windows/Fonts");
#else
        roots.emplace_back("/usr/share/fonts");
        roots.emplace_back("/usr/local/share/fonts");
        if (const char* root = std::getenv("HOME")) {
            roots.emplace_back(std::filesystem::path(root) / ".fonts");
            roots.emplace_back(std::filesystem::path(root) / ".local/share/fonts");
        }
        if (const char* root = std::getenv("XDG_DATA_HOME")) roots.emplace_back(std::filesystem::path(root) / "fonts");
#endif
        FT_Library library = nullptr;
        if (FT_Init_FreeType(&library)) return result;
        std::vector<std::string> paths;
        for (const auto& root : roots) {
            std::error_code error;
            std::filesystem::recursive_directory_iterator entry(root, std::filesystem::directory_options::skip_permission_denied, error), end;
            for (; !error && entry != end; entry.increment(error)) {
                const auto extension = LowerFamily(entry->path().extension().string());
                if (extension == ".ttf" || extension == ".otf") paths.push_back(entry->path().string());
            }
        }
        std::sort(paths.begin(), paths.end());
        for (const auto& path : paths) {
            auto bytes = ReadFont(path);
            FT_Face face = nullptr;
            if (!bytes.empty() && !FT_New_Memory_Face(library, bytes.data(), FT_Long(bytes.size()), 0, &face)) {
                if (FT_IS_SCALABLE(face) && face->family_name) {
                    std::string key = LowerFamily(face->family_name);
                    key += (face->style_flags & FT_STYLE_FLAG_BOLD) ? "/b" : "/r";
                    key += (face->style_flags & FT_STYLE_FLAG_ITALIC) ? "i" : "n";
                    result.emplace(key, path);
                }
                FT_Done_Face(face);
            }
        }
        FT_Done_FreeType(library);
        return result;
    }();
    return fonts;
}
}

uint32_t SDLFont::DecodeByte(uint8_t byte)
{
    static const uint16_t extended[32] = {
        0x20ac,0x81,0x201a,0x192,0x201e,0x2026,0x2020,0x2021,
        0x2c6,0x2030,0x160,0x2039,0x152,0x8d,0x17d,0x8f,
        0x90,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,
        0x2dc,0x2122,0x161,0x203a,0x153,0x9d,0x17e,0x178
    };
    return byte >= 0x80 && byte < 0xa0 ? extended[byte - 0x80] : byte;
}

bool SDLFont::LoadFamily(const char* family, bool bold, bool italic, int height,
    int width, bool underline, bool strikeOut, std::string& error, const char* assetRoot)
{
    if (width < -4096 || width > 4096) { error = "Ancho de fuente inválido"; return false; }
    const std::string requested = family && *family ? family : "Arial";
    // System es bitmap en GDI: el default portable es Arial o su respaldo.
    const std::string name = LowerFamily(requested) == "system" ? "arial" : LowerFamily(requested);
    std::string path;
    if (name != "liberation sans") {
        const auto& installed = InstalledFonts();
        const auto found = installed.find(name + (bold ? "/b" : "/r") + (italic ? "i" : "n"));
        if (found != installed.end()) path = found->second;
    }
    auto bytes = path.empty() ? std::vector<uint8_t>{} : ReadFont(path);
    bool fallback = bytes.empty();
    if (fallback) {
        const char* root = assetRoot && *assetRoot ? assetRoot : std::getenv("MU_ASSET_ROOT");
        std::filesystem::path base;
        if (root && *root) base = root;
        else {
            const char* executable = SDL_GetBasePath();
            base = std::filesystem::path(executable ? executable : ".") / "assets";
        }
        const char* style = bold ? (italic ? "BoldItalic" : "Bold") : (italic ? "Italic" : "Regular");
        path = (base / "fonts/LiberationSans" / (std::string("LiberationSans-") + style + ".ttf")).string();
        bytes = ReadFont(path);
    }
    if (!Load(std::move(bytes), height, error)) {
        error += ": " + path;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fuente: %s", error.c_str());
        return false;
    }
    impl->underline = underline;
    impl->strikeOut = strikeOut;
    if (width != 0) {
        width = std::abs(width);
        const auto os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(impl->face, ft_sfnt_os2));
        const auto average = os2 ? FT_MulFix(os2->xAvgCharWidth, impl->face->size->metrics.x_scale) : 0;
        if (average > 0) {
            FT_Matrix transform = { FT_Fixed((int64_t(width) * 64 * 65536) / average), 0, 0, 65536 };
            FT_Set_Transform(impl->face, &transform, nullptr);
        }
    }
    SDL_Log("Fuente %s -> %s, altura %d%s", requested.c_str(), path.c_str(), height, fallback ? " (respaldo)" : "");
    // Log persistente junto al resto de logs del cliente, también si no hay consola.
    if (FILE* log = std::fopen("fonts.log", "a")) {
        std::fprintf(log, "%s -> %s; altura=%d; bold=%d; italic=%d%s\n", requested.c_str(), path.c_str(), height, bold, italic, fallback ? "; respaldo" : "");
        std::fclose(log);
    }
    return true;
}
}
