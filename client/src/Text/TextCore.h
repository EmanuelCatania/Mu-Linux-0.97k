#pragma once
#include <cstddef>
#include <string>
#include <string_view>

// Núcleo de texto Unicode del cliente. Todo es UTF-8; Windows-1252 sólo
// aparece al convertir desde o hacia los formatos y paquetes viejos.
//
// Unidades: los offsets y largos son siempre en BYTES del UTF-8. "Carácter"
// es un code point y "cluster" es un grapheme cluster extendido (UAX #29):
// lo que el jugador ve como una sola letra, por ejemplo "é" con marca
// combinada, una sílaba hangul o un emoji compuesto con ZWJ.
namespace TextEngine {

constexpr char32_t ReplacementCharacter = 0xFFFD;

// ── Windows-1252 ────────────────────────────────────────────────────────────
// Los bytes 0x81, 0x8D, 0x8F, 0x90 y 0x9D no tienen carácter en 1252 y se
// conservan como el control C1 del mismo valor, igual que SDLFont::DecodeByte.
char32_t DecodeWindows1252(unsigned char byte);
std::string FromWindows1252(std::string_view text);
// Lo que no existe en 1252 se escribe como `replacement`, una vez por cluster.
std::string ToWindows1252(std::string_view text, char replacement = '?');

// ── UTF-8 ───────────────────────────────────────────────────────────────────
// Decodifica el code point en `offset`. Devuelve los bytes consumidos (1 a 4);
// una secuencia inválida (mal formada, sobrelarga, surrogate o > U+10FFFF)
// consume 1 byte y da ReplacementCharacter. Con offset al final devuelve 0.
std::size_t Decode(std::string_view text, std::size_t offset, char32_t& codepoint);
// Escribe el UTF-8 de `codepoint` (los inválidos se escriben como U+FFFD).
void Append(std::string& out, char32_t codepoint);
bool IsValid(std::string_view text);
// Copia `text` reemplazando cada secuencia inválida por U+FFFD.
std::string Sanitize(std::string_view text);

// ── Conteo y cortes ─────────────────────────────────────────────────────────
std::size_t CodepointCount(std::string_view text);
std::size_t ClusterCount(std::string_view text);
// Inicio del cluster siguiente / anterior a `offset` (cursor y borrado).
std::size_t NextCluster(std::string_view text, std::size_t offset);
std::size_t PreviousCluster(std::string_view text, std::size_t offset);
// Mayor prefijo, terminado en un límite de cluster, con a lo sumo
// `maxBytes` bytes / `maxClusters` clusters. Devuelve su largo en bytes.
std::size_t FitBytes(std::string_view text, std::size_t maxBytes);
std::size_t FitClusters(std::string_view text, std::size_t maxClusters);
// Copia a un buffer C de `capacity` bytes (NUL incluido) sin partir un
// cluster. Devuelve los bytes escritos sin el NUL.
std::size_t CopyTruncated(char* out, std::size_t capacity, std::string_view text);

// ── Normalización y comparación ─────────────────────────────────────────────
std::string NormalizeNFC(std::string_view text);
// NFC + case folding: dos textos son "el mismo nombre" si su clave coincide.
// Distingue acentos ("añañe" != "anane") y no mayúsculas ("Añañe" == "añañe").
std::string CaseFoldKey(std::string_view text);
int CompareIgnoreCase(std::string_view left, std::string_view right);

} // namespace TextEngine
