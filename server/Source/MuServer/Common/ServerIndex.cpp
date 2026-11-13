#include "stdafx.h"
#include "ServerManager.h"
#include "Util.h"

// DataServer y JoinServer: los GameServer conectados (gServerManager).

int gServerCount = 0;

int GetFreeServerIndex()
{
	int index = -1;

	int count = gServerCount;

	if (SearchFreeServerIndex(&index, 0, MAX_SERVER, 10000) != 0)
	{
		return index;
	}

	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[count].CheckState() == false)
		{
			return count;
		}
		else
		{
			count = (((++count) >= MAX_SERVER) ? 0 : count);
		}
	}

	return -1;
}

int SearchFreeServerIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime)
{
	DWORD CurOnlineTime = 0;

	DWORD MaxOnlineTime = 0;

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gServerManager[n].CheckState() == false && gServerManager[n].CheckAlloc() != false)
		{
			if ((CurOnlineTime = (GetTickCount() - gServerManager[n].m_OnlineTime)) > MinTime && CurOnlineTime > MaxOnlineTime)
			{
				(*index) = n;

				MaxOnlineTime = CurOnlineTime;
			}
		}
	}

	return (((*index) == -1) ? 0 : 1);
}

CServerManager* FindServerByCode(WORD ServerCode)
{
	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != false && gServerManager[n].m_ServerCode == ServerCode)
		{
			return &gServerManager[n];
		}
	}

	return 0;
}
