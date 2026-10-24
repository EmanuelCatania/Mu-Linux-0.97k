#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Platform { class SDLFont; }

// Layout de una línea de texto UTF-8 (C3 del plan Unicode): orden
// bidireccional, escritura, fuente de respaldo y shaping con HarfBuzz.
// El resultado es una lista de glifos en orden VISUAL con su posición en
// píxeles relativa al origen (x a la derecha, y hacia arriba desde la base).
namespace TextEngine {

struct ShapedGlyph {
    Platform::SDLFont* font = nullptr; // cara que tiene el glifo
    uint32_t glyph = 0;                // id de glifo dentro de esa cara
    int x = 0;                         // izquierda del origen del glifo
    int y = 0;                         // desplazamiento vertical (marcas)
    uint32_t cluster = 0;              // byte del texto donde empieza su cluster
};

struct ShapedText {
    std::vector<ShapedGlyph> glyphs;
    int width = 0;                     // avance total
};

// Cadena de fuentes de una fuente lógica: la principal (la de [Font]) y las de
// respaldo de Data/Font, cargadas al mismo alto, en el orden en que se prueban.
class FontStack {
public:
    explicit FontStack(Platform::SDLFont* primary = nullptr) : m_Primary(primary) {}
    void Reset(Platform::SDLFont* primary, int height, bool bold);
    Platform::SDLFont* Primary() const { return m_Primary; }
    // Primera cara de la cadena que tiene el carácter; la principal si ninguna.
    Platform::SDLFont* FontFor(char32_t codepoint);
    const ShapedText& Shape(std::string_view utf8);
    // Para tests y diagnóstico: cuántas fuentes de respaldo se encontraron.
    std::size_t FallbackCount();
private:
    void LoadFallbacks();
    Platform::SDLFont* m_Primary = nullptr;
    int m_Height = 0;
    bool m_Bold = false;
    bool m_FallbacksLoaded = false;
    std::vector<Platform::SDLFont*> m_Fallbacks;
    struct Cache;
    Cache* m_Cache = nullptr;
public:
    ~FontStack();
    FontStack(const FontStack&) = delete;
    FontStack& operator=(const FontStack&) = delete;
};

// Carpeta de las fuentes de respaldo (por defecto Data/Font, relativa al
// cliente o a MU_ASSET_ROOT). Se puede fijar para los tests.
void SetFallbackFontDirectory(const std::string& directory);

} // namespace TextEngine
