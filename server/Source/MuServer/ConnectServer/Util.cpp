#include "stdafx.h"
#include "Util.h"
#include "ClientManager.h"
#include "SocketManager.h"

int gClientCount = 0;

void ConnectServerTimeoutProc()
{
	for (int n = 0; n < MAX_CLIENT; n++)
	{
		if (gClientManager[n].CheckState() != false)
		{
			if (gClientManager[n].CheckOnlineTime() == false)
			{
				gSocketManager.Disconnect(n);
			}
		}
	}
}

int GetFreeClientIndex()
{
	int index = -1;

	int count = gClientCount;

	if (SearchFreeClientIndex(&index, 0, MAX_CLIENT, 10000) != 0)
	{
		return index;
	}

	for (int n = 0; n < MAX_CLIENT; n++)
	{
		if (gClientManager[count].CheckState() == false)
		{
			return count;
		}
		else
		{
			count = (((++count) >= MAX_CLIENT) ? 0 : count);
		}
	}

	return -1;
}

int SearchFreeClientIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime)
{
	DWORD CurOnlineTime = 0;

	DWORD MaxOnlineTime = 0;

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gClientManager[n].CheckState() == false && gClientManager[n].CheckAlloc() != false)
		{
			if ((CurOnlineTime = (GetTickCount() - gClientManager[n].m_OnlineTime)) > MinTime && CurOnlineTime > MaxOnlineTime)
			{
				(*index) = n;

				MaxOnlineTime = CurOnlineTime;
			}
		}
	}

	return (((*index) == -1) ? 0 : 1);
}

