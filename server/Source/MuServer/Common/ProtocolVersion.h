#pragma once

// Versión del protocolo de Mu-Linux (Fase 5, C1).
//
// El server la manda al final del F1/00 (PMSG_CONNECT_CLIENT_SEND) y el cliente
// al final del F1/01 (PMSG_CONNECT_ACCOUNT_RECV), los dos en big-endian como el
// resto de los campos de dos bytes del protocolo.  Si no coinciden, el server
// rechaza el login con el código 6 (versión incorrecta) y el cliente, al recibir
// el F1/00, muestra el mismo mensaje sin mandar la cuenta.
//
// No se mantienen dos protocolos: cada cambio del wire que rompa compatibilidad
// sube este número, y cliente y server se actualizan juntos.  Un F1/01 sin el
// campo (cliente anterior a la negociación) cuenta como versión 0.
//
// El cliente tiene la misma constante en client/src/Net/Protocol/ProtocolVersion.h.
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
#define MU_PROTOCOL_VERSION 10
