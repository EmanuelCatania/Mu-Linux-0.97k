// ProtocolBase.h — espejo de los encabezados de paquete del server.
//
// Origen: Mu-Linux-0.97k @ 25ceccb, Source/MuServer/GameServer/ProtocolDefines.h
// (ConnectServer/ProtocolDefines.h es idéntico). Sólo se copian los campos: los
// métodos set()/setE() del server no cambian el layout.
//
// Tipos de ancho fijo, sin dependencia de Windows. Los handlers conservan
// el padding y el orden de bytes del contrato existente.
#pragma once

#include <cstddef>
#include <cstdint>
#include <climits>
#include <limits>

static_assert(CHAR_BIT == 8, "El protocolo requiere bytes de 8 bits");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "El catálogo requiere float IEEE 754 de 32 bits");

namespace Proto {

// Common/TextLimits.h del server (0.97.40): largos de los campos de texto, en
// bytes de UTF-8.
constexpr std::size_t kCharacterNameSize = 40;   // 10 grapheme clusters
constexpr std::size_t kGuildNameSize     = 32;   // 8 grapheme clusters
constexpr std::size_t kChatMessageSize   = 200;

// ProtocolDefines.h:18 — C1/C3 con head.
struct PBMSG_HEAD
{
    std::uint8_t type;      // 0xC1 o 0xC3
    std::uint8_t size;
    std::uint8_t head;
};
static_assert(sizeof(PBMSG_HEAD) == 3, "PBMSG_HEAD");

// ProtocolDefines.h:39 — C1/C3 con head + subhead.
struct PSBMSG_HEAD
{
    std::uint8_t type;
    std::uint8_t size;
    std::uint8_t head;
    std::uint8_t subh;
};
static_assert(sizeof(PSBMSG_HEAD) == 4, "PSBMSG_HEAD");

// ProtocolDefines.h:63 — C2/C4 (tamaño de 16 bits, big-endian: size[0] = HB).
struct PWMSG_HEAD
{
    std::uint8_t type;      // 0xC2 o 0xC4
    std::uint8_t size[2];
    std::uint8_t head;
};
static_assert(sizeof(PWMSG_HEAD) == 4, "PWMSG_HEAD");

// ProtocolDefines.h:86 — C2/C4 con head + subhead.
struct PSWMSG_HEAD
{
    std::uint8_t type;
    std::uint8_t size[2];
    std::uint8_t head;
    std::uint8_t subh;
};
static_assert(sizeof(PSWMSG_HEAD) == 5, "PSWMSG_HEAD");

} // namespace Proto
