#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

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
// Convierte en el lugar un campo C de `capacity` bytes que trae texto
// Windows-1252 (recursos y paquetes viejos) a UTF-8, cortando sin partir un
// cluster si no entra. Devuelve los bytes escritos sin el NUL.
std::size_t ConvertWindows1252InPlace(char* field, std::size_t capacity);
// Para archivos de texto que el usuario edita (NPCName.txt, macros): si ya es
// UTF-8 válido se deja (el ASCII también lo es); si no, se toma como 1252.
std::size_t NormalizeTextFileInPlace(char* field, std::size_t capacity);

// ── Normalización y comparación ─────────────────────────────────────────────
std::string NormalizeNFC(std::string_view text);
// NFC + case folding: dos textos son "el mismo nombre" si su clave coincide.
// Distingue acentos ("añañe" != "anane") y no mayúsculas ("Añañe" == "añañe").
std::string CaseFoldKey(std::string_view text);
int CompareIgnoreCase(std::string_view left, std::string_view right);

// ── Cortes de línea (UAX #14, libunibreak) ─────────────────────────────────
// `result[i]` dice si una línea puede terminar justo antes del byte `i`
// (0 < i <= size): 0 = no, 1 = se puede, 2 = salto obligatorio. El vector
// mide size + 1; los bytes del medio de un carácter siempre dan 0.
std::vector<unsigned char> LineBreaks(std::string_view text);
// Largo del mayor prefijo de a lo sumo `maxBytes` bytes que termina en un
// punto de corte (incluye los espacios del final); un salto de línea del
// texto corta antes aunque entre todo. 0 si no hay ningún corte posible.
std::size_t FitLineBreak(std::string_view text, std::size_t maxBytes);

// Nombre de personaje o de guild (0.97.40), con la misma regla que
// TextName::IsValid del server: UTF-8 válido en NFC, de 1 a `maxClusters`
// clusters, sólo letras, marcas y dígitos de cualquier escritura y sin una
// marca combinante al principio.
bool IsNameText(std::string_view text, std::size_t maxClusters);

} // namespace TextEngine
