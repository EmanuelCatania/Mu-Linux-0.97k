// Paridad del layout nuevo con el texto 1252 de antes: mismo glifo, mismo
// bitmap y mismo avance para todo Windows-1252, en los tamaños y pesos que usa
// el cliente. Si MU_TEST_FALLBACK_FONTS apunta a una carpeta con fuentes Noto,
// además verifica respaldo, bidi y shaping de escrituras complejas.
#include "Text/TextCore.h"
#include "Text/TextShaper.h"
#include "Platform/SDL3/SDLFont.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
int failures = 0;
void Check(bool ok, const std::string& what)
{
    if (!ok) { std::printf("FALLA: %s\n", what.c_str()); ++failures; }
}

std::vector<uint8_t> Read(const std::string& path)
{
    std::vector<uint8_t> bytes;
    if (FILE* f = std::fopen(path.c_str(), "rb")) {
        uint8_t block[65536]; size_t n;
        while ((n = std::fread(block, 1, sizeof(block), f)) > 0) bytes.insert(bytes.end(), block, block + n);
        std::fclose(f);
    }
    return bytes;
}

// Ancho y posiciones como los calcula el camino 1252 (SDLFont::Measure).
void CheckLegacy(Platform::SDLFont& font, TextEngine::FontStack& stack, const std::string& legacy, const std::string& label)
{
    int pen = 0;
    std::vector<int> positions;
    for (unsigned char c : legacy) {
        positions.push_back(pen);
        pen += font.Glyph(Platform::SDLFont::DecodeByte(c)).advance;
    }
    const auto& shaped = stack.Shape(TextEngine::FromWindows1252(legacy));
    Check(shaped.width == pen, label + ": ancho " + std::to_string(shaped.width) + " != " + std::to_string(pen));
    Check(shaped.glyphs.size() == legacy.size(), label + ": cantidad de glifos");
    for (size_t i = 0; i < shaped.glyphs.size() && i < legacy.size(); ++i) {
        const auto& g = shaped.glyphs[i];
        const auto& before = font.Glyph(Platform::SDLFont::DecodeByte(static_cast<unsigned char>(legacy[i])));
        const auto& after = g.font->GlyphIndex(g.glyph);
        Check(g.x == positions[i] && g.y == 0, label + ": posicion del glifo " + std::to_string(i));
        Check(after.coverage == before.coverage && after.left == before.left && after.top == before.top,
              label + ": bitmap del glifo " + std::to_string(i));
    }
}
}

int main()
{
    const std::string fontDir = MU_TEST_FONT_DIR;
    const char* styles[] = { "Regular", "Bold" };
    const std::string words[] = {
        "Bienvenido 'mago'!", "Da\xF1o m\xE1gico: 12~34", "AVAWAToTaVA fi fl ff", "\x80 100.000 Zen",
        "\x93" "Comillas\x94 \x96 guiones \x97", "MAY\xDA" "SCULAS y min\xFA" "sculas",
    };
    for (const char* style : styles) {
        for (int height = 12; height <= 30; height += 3) {
            Platform::SDLFont font;
            std::string error;
            Check(font.Load(Read(fontDir + "/LiberationSans-" + style + ".ttf"), height, error), "carga " + std::string(style));
            TextEngine::SetFallbackFontDirectory(fontDir);
            TextEngine::FontStack stack(&font);
            stack.Reset(&font, height, style[0] == 'B');
            const std::string label = std::string(style) + " " + std::to_string(height);
            std::string all;
            for (int b = 32; b < 256; ++b) all.push_back(static_cast<char>(b));
            CheckLegacy(font, stack, all, label + " 1252 completo");
            for (const auto& w : words) CheckLegacy(font, stack, w, label + " '" + w + "'");
        }
    }

    // ── Escrituras complejas con fuentes de respaldo (opcional) ──
    if (const char* fallback = std::getenv("MU_TEST_FALLBACK_FONTS")) {
        Platform::SDLFont font;
        std::string error;
        font.Load(Read(fontDir + "/LiberationSans-Regular.ttf"), 15, error);
        TextEngine::SetFallbackFontDirectory(fallback);
        TextEngine::FontStack stack(&font);
        stack.Reset(&font, 15, false);
        std::printf("fuentes de respaldo: %zu\n", stack.FallbackCount());
        // Árabe: las cuatro letras se unen y el orden visual se invierte.
        const auto& arabic = stack.Shape(u8"مرحبا");
        Check(!arabic.glyphs.empty() && arabic.glyphs.front().cluster > arabic.glyphs.back().cluster,
              "arabe en orden visual de derecha a izquierda");
        Check(arabic.glyphs.front().font != &font, "arabe con fuente de respaldo");
        // Mezcla: latín antes y después, el árabe queda en el medio invertido.
        const auto& mixed = stack.Shape(u8"abc مرحبا xyz");
        Check(mixed.glyphs.front().cluster == 0 && mixed.glyphs.back().cluster == 17,
              "bidi: latin a los costados");
        // Devanagari: el conjunto stt usa menos glifos que code points.
        const std::string namaste = u8"नमस्ते";
        const auto& hindi = stack.Shape(namaste);
        Check(!hindi.glyphs.empty() && hindi.glyphs.size() < TextEngine::CodepointCount(namaste), "devanagari con conjunto");
        // CJK sin glifo en Liberation: toma el respaldo.
        const auto& cjk = stack.Shape(u8"你好");
        Check(cjk.glyphs.size() == 2 && cjk.glyphs[0].font != &font && cjk.width > 0, "CJK con respaldo");
        // Coreano, japonés, tailandés y emoji también salen del respaldo.
        for (const char* text : { u8"한국어", u8"ひらがな", u8"สวัสดี", u8"😀🎮" }) {
            const auto& shaped = stack.Shape(text);
            bool fallback = !shaped.glyphs.empty();
            for (const auto& glyph : shaped.glyphs) fallback = fallback && glyph.font != &font && glyph.glyph != 0;
            Check(fallback && shaped.width > 0, std::string("respaldo para ") + text);
        }
    }

    if (failures) { std::printf("%d fallas\n", failures); return 1; }
    std::puts("TextShaper: 1252 identico al camino anterior en todos los tamanos y pesos");
    return 0;
}
