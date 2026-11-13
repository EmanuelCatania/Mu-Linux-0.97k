#pragma once

// Lo propio de este server para Common/SocketManagerUdp: a donde van los
// errores y quien atiende los paquetes que llegan.

#include "Util.h"

#define UdpLog(...) LogAdd(LOG_RED, __VA_ARGS__)

// Este server solo envia.
inline void UdpProtocolCore(BYTE, BYTE*, int)
{
}
