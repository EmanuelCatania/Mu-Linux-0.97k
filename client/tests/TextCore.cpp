// Contrato del núcleo de texto. Las cadenas usan escapes \u para no depender
// de la página de códigos con la que el compilador lee el fuente.
#include "Text/TextCore.h"
#include <cstdio>
#include <string>

namespace {
int failures = 0;
void Check(bool ok, const char* what)
{
    if (!ok) { std::printf("FALLA: %s\n", what); ++failures; }
}
std::string U8(const char* text) { return text; }
}

int main()
{
    using namespace Text;
    // ── Windows-1252 ida y vuelta ──
    std::string all1252;
    for (int b = 1; b < 256; ++b) all1252.push_back(static_cast<char>(b));
    Check(ToWindows1252(FromWindows1252(all1252)) == all1252, "1252 ida y vuelta de los 255 bytes");
    Check(FromWindows1252("\x80") == U8(u8"€"), "0x80 es el euro");
    Check(FromWindows1252("A\xF1" "a") == U8(u8"Aña"), "acento de un byte pasa a dos");
    Check(ToWindows1252(U8(u8"你好")) == "??", "CJK sin equivalente da un reemplazo por cluster");
    Check(ToWindows1252(U8(u8"ñ")) == "\xF1", "n + tilde combinada se compone a 0xF1");
    Check(ToWindows1252(U8(u8"\U0001F468‍\U0001F469‍\U0001F467")) == "?", "familia ZWJ es un solo reemplazo");

    // ── Validación ──
    Check(IsValid(U8(u8"ñ你\U0001F44D")), "UTF-8 válido");
    Check(!IsValid("\xC3"), "secuencia cortada");
    Check(!IsValid("\xC0\x80"), "sobrelarga");
    Check(!IsValid("\xED\xA0\x80"), "surrogate");
    Check(!IsValid("\xF4\x90\x80\x80"), "mayor a U+10FFFF");
    Check(Sanitize("a\xFF" "b") == U8(u8"a�b"), "inválido se reemplaza, no se descarta");
    Check(IsValid(U8(u8"�")), "U+FFFD escrito es válido");

    // ── Conteo ──
    const std::string family = U8(u8"\U0001F468‍\U0001F469‍\U0001F467");
    const std::string thumbs = U8(u8"\U0001F44D\U0001F3FD");
    Check(CodepointCount(family) == 5 && ClusterCount(family) == 1, "familia: 5 code points, 1 cluster");
    Check(ClusterCount(thumbs) == 1, "pulgar con tono de piel es un cluster");
    Check(ClusterCount(U8(u8"안녕하세요")) == 5, "coreano: 5 sílabas");
    Check(ClusterCount(U8(u8"각")) == 1, "jamo conjunto es una sílaba");
    Check(ClusterCount(U8(u8"नमस्ते")) == 3, "devanagari namaste: 3 clusters");
    Check(ClusterCount("\r\n") == 1, "CR LF es un cluster");
    Check(ClusterCount("") == 0, "vacío");

    // ── Cursor y cortes ──
    const std::string mixed = "a" + family + "b";
    Check(NextCluster(mixed, 1) == 1 + family.size(), "el cursor salta la familia entera");
    Check(PreviousCluster(mixed, 1 + family.size()) == 1, "el borrado se lleva la familia entera");
    Check(PreviousCluster(mixed, 3) == 1, "desde el medio de un cluster vuelve a su inicio");
    Check(FitBytes(mixed, 5) == 1, "no parte la familia por bytes");
    Check(FitBytes(mixed, mixed.size()) == mixed.size(), "entra completo");
    Check(FitClusters(mixed, 2) == 1 + family.size(), "dos clusters");
    char buffer[4];
    Check(CopyTruncated(buffer, sizeof(buffer), U8(u8"ññ")) == 2 && buffer[2] == 0, "copia sin partir la segunda ñ");

    // ── Normalización y mayúsculas ──
    Check(NormalizeNFC(U8(u8"ã")) == U8(u8"ã"), "NFC compone");
    Check(CompareIgnoreCase(U8(u8"Añañe"), U8(u8"aÑAñE")) == 0, "Añañe == aÑAñE");
    Check(CompareIgnoreCase(U8(u8"añañe"), "anane") != 0, "añañe != anane");
    Check(CompareIgnoreCase(U8(u8"Привет"), U8(u8"пРИВЕТ")) == 0, "cirílico sin mayúsculas");
    Check(CompareIgnoreCase(U8(u8"Straße"), "STRASSE") == 0, "ß se pliega a ss");
    Check(CaseFoldKey(U8(u8"ñ")) == CaseFoldKey(U8(u8"Ñ")), "forma descompuesta == compuesta");

    if (failures) { std::printf("%d fallas\n", failures); return 1; }
    std::puts("TextCore: 1252, validación, clusters, cortes, NFC y case folding correctos");
    return 0;
}
