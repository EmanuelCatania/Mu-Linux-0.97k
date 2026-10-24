#pragma once
#include "Text/TextCore.h"
#include <algorithm>
#include <cstring>
#include <string>

// Borde de texto de la red (Fase 4, texto Unicode).
//
// DESVIACION: el texto del cliente está en UTF-8, pero el protocolo con el
// server MuEmu todavía viaja en Windows-1252 (nombres, chat, avisos, guilds,
// lista de servers). Todo texto que entra de un paquete pasa por FromServer y
// todo texto que sale a un paquete por ToServer. Cuando cambie el protocolo a
// UTF-8 estas dos funciones pasan a copiar sin convertir.
namespace NetText {

// `length` bytes 1252 del paquete (corta en el primer NUL) a UTF-8 en un
// buffer C de `capacity` bytes, sin partir un carácter si no entra.
inline std::size_t FromServer(char* out, std::size_t capacity, const void* source, std::size_t length)
{
    if (!out || !capacity) return 0;
    const char* text = static_cast<const char*>(source);
    std::size_t used = 0;
    while (text && used < length && text[used]) ++used;
    return TextEngine::CopyTruncated(out, capacity, TextEngine::FromWindows1252(std::string_view(text ? text : "", used)));
}

// UTF-8 del cliente a 1252 para el paquete: exactamente `length` bytes,
// rellenados con ceros (como el memcpy/strncpy de los campos fijos).
inline void ToServer(void* destination, std::size_t length, const char* text)
{
    if (!destination || !length) return;
    const std::string legacy = TextEngine::ToWindows1252(text ? text : "");
    std::memset(destination, 0, length);
    std::memcpy(destination, legacy.data(), (std::min)(length, legacy.size()));
}

// Igual que ToServer pero devuelve el texto 1252 (mensajes de largo variable).
inline std::string ToServer(const char* text)
{
    return TextEngine::ToWindows1252(text ? text : "");
}

} // namespace NetText
