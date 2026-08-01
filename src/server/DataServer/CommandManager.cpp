#include "stdafx.h"
#include "CommandManager.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CCommandManager gCommandManager;

CCommandManager::CCommandManager()
{

}

CCommandManager::~CCommandManager()
{

}

void CCommandManager::GDGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg, int index)
{
	SDHP_GLOBAL_POST_SEND pMsg;

	pMsg.header.set(0x05, 0x03, sizeof(pMsg));

	pMsg.type = lpMsg->type;

	memcpy(pMsg.name, lpMsg->name, sizeof(pMsg.name));

	memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));

	memcpy(pMsg.serverName, lpMsg->serverName, sizeof(pMsg.serverName));

	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}
}

void CCommandManager::GDCommandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index)
{
	SDHP_COMMAND_RESET_SEND pMsg;

	pMsg.header.set(0x05, 0x04, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account, lpMsg->account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpMsg->name, sizeof(pMsg.name));

#ifndef MYSQL
	gQueryManager.ExecQuery("EXEC WZ_GetResetInfo '%s','%s'", lpMsg->account, lpMsg->name);
#else
	gQueryManager.ExecResultQuery("CALL WZ_GetResetInfo('%s', '%s')", lpMsg->account, lpMsg->name);
#endif

	gQueryManager.Fetch();

	pMsg.ResetDay = gQueryManager.GetAsInteger("ResetDay");

	pMsg.ResetWek = gQueryManager.GetAsInteger("ResetWek");

	pMsg.ResetMon = gQueryManager.GetAsInteger("ResetMon");

	gQueryManager.Close();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

void CCommandManager::GDCommandGrandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index)
{
	SDHP_COMMAND_RESET_SEND pMsg;

	pMsg.header.set(0x05, 0x05, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account, lpMsg->account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpMsg->name, sizeof(pMsg.name));

#ifndef MYSQL
	gQueryManager.ExecQuery("EXEC WZ_GetGrandResetInfo '%s','%s'", lpMsg->account, lpMsg->name);
#else
	gQueryManager.ExecResultQuery("CALL WZ_GetGrandResetInfo('%s', '%s')", lpMsg->account, lpMsg->name);
#endif

	gQueryManager.Fetch();

	pMsg.ResetDay = gQueryManager.GetAsInteger("GrandResetDay");

	pMsg.ResetWek = gQueryManager.GetAsInteger("GrandResetWek");

	pMsg.ResetMon = gQueryManager.GetAsInteger("GrandResetMon");

	gQueryManager.Close();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

void CCommandManager::GDGlobalPostLinkRecv(
	SDHP_GLOBAL_POST_LINK_RECV* lpMsg,
	int index,
	int size)
{
	if (lpMsg == NULL || size != sizeof(SDHP_GLOBAL_POST_LINK_RECV) ||
		lpMsg->header.size != sizeof(SDHP_GLOBAL_POST_LINK_RECV) ||
		lpMsg->type > 2 || lpMsg->linkLength == 0 ||
		lpMsg->linkStart + lpMsg->linkLength >= sizeof(lpMsg->message) ||
		lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
		lpMsg->name[sizeof(lpMsg->name) - 1] != 0 ||
		lpMsg->serverName[sizeof(lpMsg->serverName) - 1] != 0)
	{
		return;
	}

	SDHP_GLOBAL_POST_LINK_SEND pMsg;
	pMsg.header.set(0x05, 0x06, sizeof(pMsg));
	pMsg.type = lpMsg->type;
	memcpy(pMsg.name, lpMsg->name, sizeof(pMsg.name));
	memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));
	memcpy(pMsg.serverName, lpMsg->serverName, sizeof(pMsg.serverName));
	pMsg.linkStart = lpMsg->linkStart;
	pMsg.linkLength = lpMsg->linkLength;
	memcpy(pMsg.ItemInfo, lpMsg->ItemInfo, sizeof(pMsg.ItemInfo));

	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}
}
