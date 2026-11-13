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
// 4: evento de rechazo F3:FB.
// 5: lista de /move en el catálogo (F3:F5); se quita F3:E5.
// 6: vida de monstruos en el 0x13 y el 0x15; F3:E2 sólo cuando cambia.
// 7: CharSet expandido (ExtItem[9] en 0x12, F3:13 y F3:00); se quitan F3:EB y F3:ED.
// 8: stats en un bloque View* dentro de F3:03, F3:05 y F3:06; se quitan F3:E0 y F3:E1.
// 9: horarios de eventos e invasiones en el C2:91; se quitan F3:E6 y F3:F2.
// 10: detalle de los buffs propios en el 0x07 expandido; se quita F3:F1.
constexpr unsigned short MU_PROTOCOL_VERSION = 12;
