#include "stdafx.h"
#include "ServerManager.h"
#include "CharacterManager.h"

void ServerManagerOnDelServer(WORD ServerCode)
{
	gCharacterManager.ClearServerCharacterInfo(ServerCode);
}
