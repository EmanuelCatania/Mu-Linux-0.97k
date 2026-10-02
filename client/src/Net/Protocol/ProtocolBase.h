// ProtocolBase.h — espejo de los encabezados de paquete del server.
//
// Origen: Mu-Linux-0.97k @ 25ceccb, Source/MuServer/GameServer/ProtocolDefines.h
// (ConnectServer/ProtocolDefines.h es idéntico). Sólo se copian los campos: los
// métodos set()/setE() del server no cambian el layout.
//
// Incluir después de stdafx.h (usa BYTE/WORD/DWORD de windows.h).
// Estos headers sólo documentan el protocolo y lo verifican con static_assert:
// todavía no los usa ningún parseo (eso es la Fase 2).
#pragma once

#include <cstddef>

namespace Proto {

// ProtocolDefines.h:18 — C1/C3 con head.
struct PBMSG_HEAD
{
    BYTE type;      // 0xC1 o 0xC3
    BYTE size;
    BYTE head;
};
static_assert(sizeof(PBMSG_HEAD) == 3, "PBMSG_HEAD");

// ProtocolDefines.h:39 — C1/C3 con head + subhead.
struct PSBMSG_HEAD
{
    BYTE type;
    BYTE size;
    BYTE head;
    BYTE subh;
};
static_assert(sizeof(PSBMSG_HEAD) == 4, "PSBMSG_HEAD");

// ProtocolDefines.h:63 — C2/C4 (tamaño de 16 bits, big-endian: size[0] = HB).
struct PWMSG_HEAD
{
    BYTE type;      // 0xC2 o 0xC4
    BYTE size[2];
    BYTE head;
};
static_assert(sizeof(PWMSG_HEAD) == 4, "PWMSG_HEAD");

// ProtocolDefines.h:86 — C2/C4 con head + subhead.
struct PSWMSG_HEAD
{
    BYTE type;
    BYTE size[2];
    BYTE head;
    BYTE subh;
};
static_assert(sizeof(PSWMSG_HEAD) == 5, "PSWMSG_HEAD");

} // namespace Proto
