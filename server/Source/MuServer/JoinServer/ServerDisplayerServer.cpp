#include "stdafx.h"
#include "ServerDisplayerHooks.h"
#include "Log.h"
#include "AccountManager.h"
#include "JoinServerProtocol.h"
#include "ServerManager.h"
#include "SocketManager.h"

void ServerDisplayerAddLogs()
{
#ifdef _WIN32
	gLog.AddLog(GetPrivateProfileInt("Log", "LOG", 1, ".\\JoinServer.ini") != 0, "LOG");

	gLog.AddLog(GetPrivateProfileInt("Log", "LOG_ACCOUNT", 1, ".\\JoinServer.ini") != 0, "LOG_ACCOUNT");

#ifdef MYSQL

	gLog.AddLog(GetPrivateProfileInt("Log", "LOG_QUERY", 1, ".\\JoinServer.ini") != 0, "LOG_QUERY");

#endif
#else
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG", 1, "./JoinServer.ini") != 0), "LOG");
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG_ACCOUNT", 1, "./JoinServer.ini") != 0), "LOG_ACCOUNT");
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG_QUERY", 1, "./JoinServer.ini") != 0), "LOG_QUERY");
#endif
}

#ifdef _WIN32

void ServerDisplayerWindowName(char* buff)
{
	wsprintf(buff, "MuEmu JoinServer Ex097 (QueueSize : %d) (AccountCount : %d/%d)", gSocketManager.GetQueueSize(), gAccountManager.GetAccountCount(), MAX_ACCOUNT);
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
