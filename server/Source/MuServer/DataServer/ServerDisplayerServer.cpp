#include "stdafx.h"
#include "ServerDisplayerHooks.h"
#include "Log.h"
#include "ServerManager.h"
#include "SocketManager.h"

void ServerDisplayerAddLogs()
{
#ifdef _WIN32
	gLog.AddLog(GetPrivateProfileInt("Log", "LOG", 1, ".\\DataServer.ini") != 0, "LOG");

#ifdef MYSQL

	gLog.AddLog(GetPrivateProfileInt("Log", "LOG_QUERY", 1, ".\\DataServer.ini") != 0, "LOG_QUERY");

#endif
#else
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG", 1, "./DataServer.ini") != 0), "LOG");
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG_QUERY", 1, "./DataServer.ini") != 0), "LOG_QUERY");
#endif
}

#ifdef _WIN32

void ServerDisplayerWindowName(char* buff)
{
	wsprintf(buff, "MuEmu DataServer Ex097 (QueueSize : %d)", gSocketManager.GetQueueSize());
}

bool ServerDisplayerIsActive()
{
	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() == false)
		{
			continue;
		}

		if ((GetTickCount() - gServerManager[n].m_PacketTime) <= 60000)
		{
			return true;
		}
	}

	return false;
}

#endif
