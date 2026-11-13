#pragma once

// Lo propio de este server para Common/SocketManagerUdp: a donde van los
// errores y quien atiende los paquetes que llegan.

#include "Log.h"

#define UdpLog(...) gLog.Output(LOG_CONNECT, __VA_ARGS__)

// Este server solo envia.
inline void UdpProtocolCore(BYTE, BYTE*, int)
{
}
