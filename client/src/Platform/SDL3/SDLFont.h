#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Platform {
// Un avance por glifo: mediciÃ³n y ambos renderers consumen el mismo resultado.
struct FontGlyph {
    int advance = 0;
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    std::vector<uint8_t> coverage; // filas de arriba hacia abajo, 0..255
};
struct FontMetrics {
    int ascent = 0;
    int height = 0;
};
class SDLFont {
public:
    SDLFont();
    ~SDLFont();
    SDLFont(const SDLFont&) = delete;
    SDLFont& operator=(const SDLFont&) = delete;
    // Altura positiva de celda, como CreateFont; los bytes pertenecen al objeto.
    bool Load(std::vector<uint8_t> bytes, int height, std::string& error);
    // Igual que Load, pero FreeType lee el archivo bajo demanda en vez de
    // cargarlo entero (fuentes del sistema de 10-20 MB usadas como respaldo).
    bool LoadFile(const std::string& path, int height, std::string& error);
    // Respaldo en Data/Font, relativo a la carpeta del cliente o a MU_ASSET_ROOT.
    bool LoadFamily(const char* family, bool bold, bool italic, int height,
                    int width, bool underline, bool strikeOut, std::string& error,
                    const char* assetRoot = nullptr);
    static std::vector<std::string> InstalledFamilies();
    // Rasterizado con 256 niveles de gris en vez de 1 bit. Lo toma cada fuente
    // al abrirse: se fija antes de crearlas (Config.ini [Font] FontQuality).
    static void SetAntialias(bool enabled);
    static uint32_t DecodeByte(uint8_t byte); // recursos del cliente: Windows-1252
    const FontGlyph& Glyph(uint32_t codepoint);
    // Por id de glifo de la fuente (lo que devuelve el shaping).
    const FontGlyph& GlyphIndex(uint32_t glyph);
    bool HasCodepoint(uint32_t codepoint) const;
    // hb_font_t* de HarfBuzz sobre esta cara; vive mientras viva la fuente.
    void* ShapingFont();
    FontMetrics Metrics() const;
    int Measure(const uint32_t* text, size_t count);
private:
    struct Impl;
    bool Open(std::unique_ptr<Impl> next, const char* path, int height, std::string& error);
    std::unique_ptr<Impl> impl;
};
}
