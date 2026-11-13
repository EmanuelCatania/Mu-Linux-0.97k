#pragma once

// Lo propio de este server para Common/IpManager: de donde sale el maximo de
// conexiones por IP (0 = no se acepta ninguna).

#include "ServerInfo.h"

inline int IpManagerMaxConnection()
{
	return gServerInfo.m_MaxIpConnection;
}
