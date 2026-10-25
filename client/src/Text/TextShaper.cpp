#include "Text/TextShaper.h"
#include "Text/TextCore.h"
#include "Platform/SDL3/SDLFont.h"
#include <hb.h>
#include <SheenBidi/SheenBidi.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <map>
#include <unordered_map>

namespace TextEngine {
namespace {
std::string fallbackDirectory;

std::string Lower(std::string value)
{
    for (char& c : value) c = char(std::tolower(static_cast<unsigned char>(c)));
    return value;
}

// Escrituras que el juego ya mostraba (latín, griego, cirílico y los signos
// comunes): para ellas se apagan kerning y ligaduras opcionales, así el texto
// conserva exactamente los avances de antes. El resto usa los features por
// defecto de HarfBuzz, que son los que necesitan para verse bien.
bool KeepsLegacyMetrics(hb_script_t script)
{
    return script == HB_SCRIPT_LATIN || script == HB_SCRIPT_GREEK || script == HB_SCRIPT_CYRILLIC ||
           script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_UNKNOWN;
}

bool InheritsScript(hb_script_t script)
{
    return script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_UNKNOWN;
}

struct Codepoint { char32_t value; uint32_t offset, length; };

bool ToWindows1252Byte(char32_t codepoint, unsigned char& byte)
{
    for (int b = 0; b < 256; ++b)
        if (DecodeWindows1252(static_cast<unsigned char>(b)) == codepoint) { byte = static_cast<unsigned char>(b); return true; }
    return false;
}
} // namespace

// Fuentes del sistema para lo que Data/Font no cubre, en orden de prueba.
// Windows: las que trae el sistema para cada escritura. Linux: los nombres
// habituales de Noto, DejaVu y WenQuanYi, buscados en las carpetas de fuentes.
const std::vector<std::string>& SystemFallbackFonts()
{
    static const std::vector<std::string> fonts = [] {
        std::vector<std::string> result;
        std::error_code error;
#ifdef _WIN32
        const char* windows = std::getenv("WINDIR");
        const std::filesystem::path dir = std::filesystem::path(windows && *windows ? windows : "C:\\Windows") / "Fonts";
        static const char* const names[] = {
            "segoeui.ttf", "msyh.ttc", "msjh.ttc", "YuGothR.ttc", "meiryo.ttc", "msgothic.ttc",
            "malgun.ttf", "Nirmala.ttc", "Nirmala.ttf", "mangal.ttf", "LeelawUI.ttf", "ebrima.ttf", "seguihis.ttf",
            "seguisym.ttf", "simsun.ttc", "arialuni.ttf",
        };
        for (const char* name : names)
            if (std::filesystem::is_regular_file(dir / name, error)) result.push_back((dir / name).string());
#else
        static const char* const names[] = {
            "notosans-regular.ttf", "dejavusans.ttf", "notosanscjk-regular.ttc", "notosanscjksc-regular.otf",
            "notosanssc-regular.otf", "wqy-microhei.ttc", "wqy-zenhei.ttc", "droidsansfallbackfull.ttf",
            "notosansarabic-regular.ttf", "notosanshebrew-regular.ttf", "notosansdevanagari-regular.ttf",
            "notosansthai-regular.ttf", "notosanssymbols-regular.ttf", "notosanssymbols2-regular.ttf",
        };
        std::vector<std::filesystem::path> roots = { "/usr/share/fonts", "/usr/local/share/fonts" };
        if (const char* home = std::getenv("HOME")) {
            roots.emplace_back(std::filesystem::path(home) / ".local/share/fonts");
            roots.emplace_back(std::filesystem::path(home) / ".fonts");
        }
        std::map<std::string, std::string> found;
        for (const auto& root : roots) {
            std::filesystem::recursive_directory_iterator entry(root, std::filesystem::directory_options::skip_permission_denied, error), end;
            for (; !error && entry != end; entry.increment(error))
                if (entry->is_regular_file(error)) found.emplace(Lower(entry->path().filename().string()), entry->path().string());
            error.clear();
        }
        for (const char* name : names) {
            const auto it = found.find(name);
            if (it != found.end()) result.push_back(it->second);
        }
#endif
        return result;
    }();
    return fonts;
}

void SetFallbackFontDirectory(const std::string& directory) { fallbackDirectory = directory; }

struct FontStack::Cache {
    std::unordered_map<std::string, ShapedText> entries;
};

FontStack::~FontStack()
{
    for (auto* font : m_Fallbacks) delete font;
    delete m_Cache;
}

void FontStack::Reset(Platform::SDLFont* primary, int height, bool bold)
{
    for (auto* font : m_Fallbacks) delete font;
    m_Fallbacks.clear();
    m_FallbacksLoaded = false;
    m_SystemTried = 0;
    m_Primary = primary;
    m_Height = height;
    m_Bold = bold;
    if (m_Cache) m_Cache->entries.clear();
}

std::size_t FontStack::FallbackCount()
{
    LoadFallbacks();
    return m_Fallbacks.size();
}

// Cada archivo de Data/Font que no sea de la familia principal es un
// respaldo. Por familia se toma la variante Bold o Regular según la fuente
// lógica; el orden es alfabético por archivo, así es estable y predecible.
// Se abren con LoadFile: FreeType lee el archivo bajo demanda, así los 4 MB
// del respaldo CJK no se cargan en memoria por cada tamaño de fuente.
void FontStack::LoadFallbacks()
{
    if (m_FallbacksLoaded) return;
    m_FallbacksLoaded = true;
    std::filesystem::path base;
    if (!fallbackDirectory.empty()) base = fallbackDirectory;
    else {
        const char* root = std::getenv("MU_ASSET_ROOT");
        base = std::filesystem::path(root && *root ? root : "") / "Data/Font";
    }
    std::error_code error;
    std::map<std::string, std::map<std::string, std::string>> families; // familia -> estilo -> archivo
    for (const auto& entry : std::filesystem::directory_iterator(base, error)) {
        if (!entry.is_regular_file()) continue;
        const std::string ext = Lower(entry.path().extension().string());
        if (ext != ".ttf" && ext != ".otf") continue;
        const std::string stem = entry.path().stem().string();
        if (Lower(stem).rfind("liberationsans", 0) == 0) continue;
        const auto dash = stem.find('-');
        const std::string family = dash == std::string::npos ? stem : stem.substr(0, dash);
        const std::string style = dash == std::string::npos ? "regular" : Lower(stem.substr(dash + 1));
        families[family][style] = entry.path().generic_string();
    }
    for (const auto& [family, styles] : families) {
        auto pick = styles.find(m_Bold ? "bold" : "regular");
        if (pick == styles.end()) pick = styles.find("regular");
        if (pick == styles.end()) pick = styles.begin();
        auto* font = new Platform::SDLFont;
        std::string message;
        if (font->LoadFile(pick->second, m_Height, message)) m_Fallbacks.push_back(font);
        else delete font;
    }
}

Platform::SDLFont* FontStack::FontFor(char32_t codepoint)
{
    if (!m_Primary || m_Primary->HasCodepoint(codepoint)) return m_Primary;
    LoadFallbacks();
    for (auto* font : m_Fallbacks)
        if (font->HasCodepoint(codepoint)) return font;
    if (!fallbackDirectory.empty()) return m_Primary;
    const auto& system = SystemFallbackFonts();
    while (m_SystemTried < system.size()) {
        auto* font = new Platform::SDLFont;
        std::string message;
        if (!font->LoadFile(system[m_SystemTried++], m_Height, message)) { delete font; continue; }
        m_Fallbacks.push_back(font);
        if (font->HasCodepoint(codepoint)) return font;
    }
    return m_Primary;
}

const ShapedText& FontStack::Shape(std::string_view input)
{
    if (!m_Cache) m_Cache = new Cache;
    auto found = m_Cache->entries.find(std::string(input));
    if (found != m_Cache->entries.end()) return found->second;
    // La interfaz redibuja los mismos textos cada frame; los que cambian
    // (contadores, chat) se descartan en bloque cuando la caché se llena.
    if (m_Cache->entries.size() >= 4096) m_Cache->entries.clear();

    const std::string text = Sanitize(input);
    ShapedText result;
    std::vector<Codepoint> codepoints;
    bool mayBeRtl = false;
    for (std::size_t offset = 0; offset < text.size();) {
        char32_t value;
        const std::size_t length = Decode(text, offset, value);
        codepoints.push_back({ value, uint32_t(offset), uint32_t(length) });
        if (value >= 0x0590) mayBeRtl = true;
        offset += length;
    }

    // ── 1. Tramos de dirección, en orden visual ──
    struct Run { uint32_t begin, end; bool rtl; };
    std::vector<Run> runs;
    if (!mayBeRtl) {
        if (!text.empty()) runs.push_back({ 0, uint32_t(text.size()), false });
    } else {
        SBCodepointSequence sequence = { SBStringEncodingUTF8, text.data(), text.size() };
        SBAlgorithmRef algorithm = SBAlgorithmCreate(&sequence);
        SBUInteger offset = 0;
        while (algorithm && offset < text.size()) {
            // La dirección del párrafo sale de su primer carácter fuerte (P2-P3).
            SBParagraphRef paragraph = SBAlgorithmCreateParagraph(algorithm, offset, text.size() - offset, SBLevelDefaultLTR);
            if (!paragraph) break;
            const SBUInteger length = SBParagraphGetLength(paragraph);
            SBLineRef line = SBParagraphCreateLine(paragraph, offset, length);
            if (line) {
                const SBRun* items = SBLineGetRunsPtr(line);
                for (SBUInteger i = 0; i < SBLineGetRunCount(line); ++i)
                    runs.push_back({ uint32_t(items[i].offset), uint32_t(items[i].offset + items[i].length), (items[i].level & 1) != 0 });
                SBLineRelease(line);
            }
            SBParagraphRelease(paragraph);
            if (!length) break;
            offset += length;
        }
        if (algorithm) SBAlgorithmRelease(algorithm);
    }

    // ── 2. Dentro de cada tramo: segmentos de una escritura y una fuente ──
    hb_unicode_funcs_t* unicode = hb_unicode_funcs_get_default();
    hb_buffer_t* buffer = hb_buffer_create();
    int pen = 0;
    std::size_t index = 0; // codepoint actual (los tramos vienen en orden visual, se busca)
    for (const Run& run : runs) {
        struct Segment { uint32_t begin, end; hb_script_t script; Platform::SDLFont* font; bool legacy; };
        std::vector<Segment> segments;
        index = 0;
        while (index < codepoints.size() && codepoints[index].offset < run.begin) ++index;
        hb_script_t script = HB_SCRIPT_COMMON;
        Platform::SDLFont* font = nullptr;
        // Las marcas, los espacios y los signos comunes siguen a lo anterior;
        // si el tramo empieza con ellos toman lo del primer carácter con escritura.
        for (std::size_t i = index; i < codepoints.size() && codepoints[i].offset < run.end; ++i) {
            const hb_script_t s = hb_unicode_script(unicode, codepoints[i].value);
            if (!InheritsScript(s)) { script = s; break; }
        }
        for (std::size_t i = index; i < codepoints.size() && codepoints[i].offset < run.end; ++i) {
            const auto& cp = codepoints[i];
            hb_script_t s = hb_unicode_script(unicode, cp.value);
            const bool inherits = InheritsScript(s);
            if (inherits) s = script;
            Platform::SDLFont* f = font && (inherits || font->HasCodepoint(cp.value)) ? font : FontFor(cp.value);
            unsigned char legacyByte;
            const bool legacy = ToWindows1252Byte(cp.value, legacyByte);
            if (segments.empty() || s != segments.back().script || f != segments.back().font)
                segments.push_back({ cp.offset, cp.offset + cp.length, s, f, legacy });
            else {
                segments.back().end = cp.offset + cp.length;
                segments.back().legacy = segments.back().legacy && legacy;
            }
            script = s;
            font = f;
        }
        // En un tramo de derecha a izquierda los segmentos van en orden inverso.
        if (run.rtl) std::reverse(segments.begin(), segments.end());

        // ── 3. Shaping ──
        for (const Segment& segment : segments) {
            if (!segment.font) continue;
            auto* shapingFont = static_cast<hb_font_t*>(segment.font->ShapingFont());
            if (!shapingFont) continue;
            hb_buffer_clear_contents(buffer);
            hb_buffer_add_utf8(buffer, text.data(), int(text.size()), segment.begin, int(segment.end - segment.begin));
            hb_buffer_set_direction(buffer, run.rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
            hb_buffer_set_script(buffer, segment.script);
            hb_buffer_set_language(buffer, hb_language_get_default());
            hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
            // El texto 1252 dibujaba también el guion suave (U+00AD), que
            // HarfBuzz oculta por ser "ignorable". Sólo en esos tramos: en un
            // emoji compuesto el ZWJ sí tiene que quedar oculto.
            hb_buffer_set_flags(buffer, segment.legacy ? HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES : HB_BUFFER_FLAG_DEFAULT);
            static const hb_feature_t legacy[] = {
                { HB_TAG('k','e','r','n'), 0, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
                { HB_TAG('l','i','g','a'), 0, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
                { HB_TAG('c','l','i','g'), 0, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END },
            };
            const bool keep = KeepsLegacyMetrics(segment.script);
            hb_shape(shapingFont, buffer, keep ? legacy : nullptr, keep ? 3u : 0u);
            unsigned count = 0;
            const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, &count);
            const hb_glyph_position_t* position = hb_buffer_get_glyph_positions(buffer, &count);
            for (unsigned i = 0; i < count; ++i) {
                const auto& raster = segment.font->GlyphIndex(info[i].codepoint);
                // El avance es el del glifo rasterizado (igual que el texto de
                // antes) más lo que HarfBuzz haya corrido respecto del nominal
                // (marcas en cero, ajustes de posición de escrituras complejas).
                const hb_position_t nominal = hb_font_get_glyph_h_advance(shapingFont, info[i].codepoint);
                const int adjust = int((position[i].x_advance - nominal + (position[i].x_advance >= nominal ? 32 : -32)) / 64);
                ShapedGlyph glyph;
                glyph.font = segment.font;
                glyph.glyph = info[i].codepoint;
                glyph.x = pen + int((position[i].x_offset + 32) >> 6);
                glyph.y = int((position[i].y_offset + 32) >> 6);
                glyph.cluster = info[i].cluster;
                result.glyphs.push_back(glyph);
                pen += raster.advance + adjust;
            }
        }
    }
    hb_buffer_destroy(buffer);
    result.width = pen;
    return m_Cache->entries.emplace(std::string(input), std::move(result)).first->second;
}

} // namespace TextEngine
