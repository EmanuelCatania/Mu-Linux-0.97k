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
// 1: negociación (0.97.45).
// 2: apilado y precios en el catálogo (F3:F8, F3:F9); se quitan F3:E3 y F3:E4.
// 3: velocidad de ataque de los demás jugadores (0x12 expandido, F3:FA).
constexpr unsigned short MU_PROTOCOL_VERSION = 3;
