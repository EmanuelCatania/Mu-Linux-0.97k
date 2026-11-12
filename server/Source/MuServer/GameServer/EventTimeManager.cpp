#include "stdafx.h"
#include "EventTimeManager.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

#include "DevilSquare.h"
#include "BloodCastle.h"
#include "InvasionManager.h"
#include "BonusManager.h"
#include "GoldenArcherBingo.h"

CEventTimeManager gEventTimeManager;

CEventTimeManager::CEventTimeManager()
{

}

CEventTimeManager::~CEventTimeManager()
{

}

void CEventTimeManager::GCEventTimeSend(int aIndex)
{
	BYTE send[4096];

	PMSG_EVENT_TIME_SEND pMsg;

	pMsg.header.set(0xF3, 0xE6, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_EVENT_TIME info;

	int state;

	int i;

	if (gServerInfo.m_DevilSquareEvent)
	{
		strcpy_s(info.name, "Devil Square");

		info.time = 0;

		state = gDevilSquare.GetState(0);

		if (state == DS_STATE_START)
		{
			info.status = EVENT_STATE_START;
		}
		else if (state == DS_STATE_EMPTY)
		{
			info.status = EVENT_STATE_STAND;

			info.time = gDevilSquare.GetCurrentRemainTime(0);
		}
		else if (state == DS_STATE_STAND)
		{
			info.status = EVENT_STATE_OPEN;
		}
		else
		{
			info.status = EVENT_STATE_BLANK;
		}

		memcpy(&send[size], &info, sizeof(info));

		size += sizeof(info);

		pMsg.count++;
	}

	if (gServerInfo.m_BloodCastleEvent)
	{
		strcpy_s(info.name, "Blood Castle");

		info.time = 0;

		state = gBloodCastle.GetState(0);

		if (state == BC_STATE_START)
		{
			info.status = EVENT_STATE_START;
		}
		else if (state == BC_STATE_EMPTY)
		{
			info.status = EVENT_STATE_STAND;

			info.time = gBloodCastle.GetCurrentRemainTime(0);
		}
		else if (state == BC_STATE_STAND)
		{
			info.status = EVENT_STATE_OPEN;
		}
		else
		{
			info.status = EVENT_STATE_BLANK;
		}

		memcpy(&send[size], &info, sizeof(info));

		size += sizeof(info);

		pMsg.count++;
	}

	if (gServerInfo.m_InvasionManagerSwitch)
	{
		for (i = 0; i < MAX_INVASION; i++)
		{
			state = gInvasionManager.GetState(i);

			if (state == INVASION_STATE_BLANK)
			{
				continue;
			}

			strcpy_s(info.name, gInvasionManager.GetInvasionName(i));

			if (state == INVASION_STATE_EMPTY)
			{
				info.status = EVENT_STATE_STAND;

				info.time = gInvasionManager.GetCurrentRemainTime(i);
			}
			else
			{
				info.status = EVENT_STATE_START;

				info.time = 0;
			}

			memcpy(&send[size], &info, sizeof(info));

			size += sizeof(info);

			pMsg.count++;
		}
	}

	if (gServerInfo.m_BonusManagerSwitch)
	{
		for (i = 0; i < MAX_BONUS; i++)
		{
			state = gBonusManager.GetState(i);

			if (state == BONUS_STATE_BLANK)
			{
				continue;
			}

			strcpy_s(info.name, gBonusManager.GetBonusName(i));

			if (state == BONUS_STATE_EMPTY)
			{
				info.status = EVENT_STATE_STAND;

				info.time = gBonusManager.GetCurrentRemainTime(i);
			}
			else
			{
				info.status = EVENT_STATE_START;

				info.time = 0;
			}

			memcpy(&send[size], &info, sizeof(info));

			size += sizeof(info);

			pMsg.count++;
		}
	}

	if (gServerInfo.m_GoldenArcherBingoEvent)
	{
		strcpy_s(info.name, gGoldenArcherBingo.GetEventName());

		info.time = 0;

		state = gGoldenArcherBingo.GetState();

		if (state == BINGO_STATE_START)
		{
			info.status = EVENT_STATE_START;
		}
		else if (state == BINGO_STATE_EMPTY)
		{
			info.status = EVENT_STATE_STAND;

			info.time = gGoldenArcherBingo.GetCurrentRemainTime();
		}
		else if (state == BINGO_STATE_STAND)
		{
			info.status = EVENT_STATE_OPEN;
		}
		else
		{
			info.status = EVENT_STATE_BLANK;
		}

		memcpy(&send[size], &info, sizeof(info));

		size += sizeof(info);

		pMsg.count++;
	}

	if (pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);

		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send, &pMsg, sizeof(pMsg));

		DataSend(aIndex, send, size);
	}

	this->GCInvasionDetailSend(aIndex);
}

void CEventTimeManager::GCInvasionDetailSend(int aIndex)
{
	if (gServerInfo.m_InvasionManagerSwitch == 0)
	{
		return;
	}

	BYTE send[4096];

	PMSG_INVASION_DETAIL_SEND pMsg;

	pMsg.header.set(0xF3, 0xF2, 0);

	pMsg.count = 0;

	int size = sizeof(pMsg);

	int alive[MAX_MAP];

	int total[MAX_MAP];

	for (int i = 0; i < MAX_INVASION; i++)
	{
		if (gInvasionManager.GetMapStatus(i, alive, total) == 0)
		{
			continue;
		}

		PMSG_INVASION_DETAIL info;

		memset(&info, 0, sizeof(info));

		strcpy_s(info.name, gInvasionManager.GetInvasionName(i));

		int start = size;

		size += sizeof(info);

		for (int map = 0; map < MAX_MAP; map++)
		{
			if (total[map] == 0 || (size + (int)sizeof(PMSG_INVASION_DETAIL_MAP)) > (int)sizeof(send))
			{
				continue;
			}

			PMSG_INVASION_DETAIL_MAP entry;

			entry.map = map;

			entry.alive = alive[map];

			entry.total = total[map];

			memcpy(&send[size], &entry, sizeof(entry));

			size += sizeof(entry);

			info.maps++;
		}

		memcpy(&send[start], &info, sizeof(info));

		pMsg.count++;
	}

	// Sin invasiones en curso no se manda nada: el cliente sólo muestra el
	// detalle de las filas que el F3:E6 marca como en curso.
	if (pMsg.count == 0)
	{
		return;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
}