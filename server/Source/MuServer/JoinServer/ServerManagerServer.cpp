#include "stdafx.h"
#include "ServerManager.h"
#include "AccountManager.h"

void ServerManagerOnDelServer(WORD ServerCode)
{
	gAccountManager.ClearServerAccountInfo(ServerCode);
}
