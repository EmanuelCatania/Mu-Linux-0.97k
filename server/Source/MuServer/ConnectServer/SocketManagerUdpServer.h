#pragma once

// Lo propio de este server para Common/SocketManagerUdp: a donde van los
// errores y quien atiende los paquetes que llegan.

#include "ServerList.h"
#include "Util.h"

#define UdpLog(...) LogAdd(LOG_RED, __VA_ARGS__)

inline void UdpProtocolCore(BYTE head, BYTE* lpMsg, int size)
{
	gServerList.ServerProtocolCore(head, lpMsg, size);
}
