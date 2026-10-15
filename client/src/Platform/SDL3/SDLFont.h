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
    // MU_ASSET_ROOT o <directorio del ejecutable>/assets; permite una raÃ­z explÃ­cita.
    bool LoadFamily(const char* family, bool bold, bool italic, int height,
                    int width, bool underline, bool strikeOut, std::string& error,
                    const char* assetRoot = nullptr);
    static uint32_t DecodeByte(uint8_t byte); // recursos del cliente: Windows-1252
    const FontGlyph& Glyph(uint32_t codepoint);
    FontMetrics Metrics() const;
    int Measure(const uint32_t* text, size_t count);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
}
