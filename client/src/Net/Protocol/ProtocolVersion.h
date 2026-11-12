#pragma once

// Versión del protocolo de Mu-Linux (Fase 5, C1).  Misma constante que
// server/Source/MuServer/Common/ProtocolVersion.h: si se sube una, se sube la
// otra en el mismo cambio.
//
// El server la manda al final del F1/00 y el cliente al final del F1/01, los dos
// en big-endian.  Un F1/00 sin el campo viene de un server anterior a la
// negociación y cuenta como versión 0.  Si no coinciden, el cliente no manda la
// cuenta y muestra el cartel de versión incorrecta (ErrorMessage 0x1A, el mismo
// que el resultado 6 del login), que al confirmarlo cierra el juego.
constexpr unsigned short MU_PROTOCOL_VERSION = 1;
