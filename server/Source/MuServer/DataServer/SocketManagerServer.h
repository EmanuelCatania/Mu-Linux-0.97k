#pragma once

// Lo propio de este server para Common/SocketManager: el tope del buffer
// de envio pendiente y quien atiende los paquetes que llegan.

#define MAX_SIDE_PACKET_SIZE 262144

void DataServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size);

#define SocketProtocolCore DataServerProtocolCore
