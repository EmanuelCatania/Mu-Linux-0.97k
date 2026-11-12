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
#define MU_PROTOCOL_VERSION 3
