#include "stdafx.h"
#include "Util.h"

// ConnectServer, DataServer y JoinServer. El GameServer tiene la suya, que
// ademas informa el indice del usuario.

void ConsoleProtocolLog(int type, BYTE* lpMsg, int size)
{
	BYTE head, subhead;

	BYTE header = lpMsg[0];

	if (header == 0xC1 || header == 0xC3)
	{
		head = lpMsg[2];
	}
	else if (header == 0xC2 || header == 0xC4)
	{
		head = lpMsg[3];
	}

	subhead = ((header == 0xC1) ? lpMsg[3] : lpMsg[4]);

	gConsole.Output(type, "[%s] Header: 0x%02X, Head: 0x%02X, SubHead: 0x%02X, Size: %d", (type == CON_PROTO_TCP_RECV) ? "RECV" : "SEND", header, head, subhead, size);
}
