#include "stdafx.h"
#include "ServerDisplayerHooks.h"
#include "Log.h"
#include "ServerList.h"
#include "SocketManager.h"

void ServerDisplayerAddLogs()
{
#ifdef _WIN32
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG", 1, ".\\ConnectServer.ini") != 0), "LOG");
#else
	gLog.AddLog((GetPrivateProfileInt("Log", "LOG", 1, "./ConnectServer.ini") != 0), "LOG");
#endif
}

#ifdef _WIN32

void ServerDisplayerWindowName(char* buff)
{
	wsprintf(buff, "MuEmu ConnectServer Ex097 (QueueSize : %d)", gSocketManager.GetQueueSize());
}

bool ServerDisplayerIsActive()
{
	return (gServerList.CheckJoinServerState() != false);
}

#endif
