#pragma once
#include "Net/Protocol/ProtocolBase.h"
#include "Text/TextCore.h"
#include <cstring>
#include <string>

// Borde de texto de la red (0.97.40).
//
// El cliente y el server manejan UTF-8. Los campos de texto de los paquetes
// tienen largo fijo y pueden ir llenos sin terminador; estas funciones copian
// entre esos campos y los buffers C del cliente sin dejar UTF-8 roto: lo que
// llega del server se sanea y lo que sale se corta en un borde de grapheme
// cluster.
namespace NetText {

// Largos de los campos, en bytes (Common/TextLimits.h del server).
constexpr std::size_t CharacterNameSize = Proto::kCharacterNameSize;
constexpr std::size_t GuildNameSize     = Proto::kGuildNameSize;
constexpr std::size_t ChatMessageSize   = Proto::kChatMessageSize;
constexpr std::size_t CharacterNameClusters = 10;
constexpr std::size_t GuildNameClusters     = 8;

// `length` bytes del paquete (corta en el primer NUL) a un buffer C de
// `capacity` bytes, sin partir un carácter si no entra.
inline std::size_t FromServer(char* out, std::size_t capacity, const void* source, std::size_t length)
{
    if (!out || !capacity) return 0;
    const char* text = static_cast<const char*>(source);
    std::size_t used = 0;
    while (text && used < length && text[used]) ++used;
    const std::string_view raw(text ? text : "", used);
    if (TextEngine::IsValid(raw)) return TextEngine::CopyTruncated(out, capacity, raw);
    return TextEngine::CopyTruncated(out, capacity, TextEngine::Sanitize(raw));
}

// Texto del cliente al campo del paquete: exactamente `length` bytes,
// cortado en un borde de cluster y rellenado con ceros.
inline void ToServer(void* destination, std::size_t length, const char* text)
{
    if (!destination || !length) return;
    const std::string_view value(text ? text : "");
    const std::size_t used = TextEngine::FitBytes(value, length);
    std::memset(destination, 0, length);
    std::memcpy(destination, value.data(), used);
}

} // namespace NetText
