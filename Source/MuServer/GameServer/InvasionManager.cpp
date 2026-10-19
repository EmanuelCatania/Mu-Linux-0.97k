#include "stdafx.h"
#include "InvasionManager.h"
#include "FlyingDragons.h"
#include "ReadScript.h"
#include "Monster.h"
#include "MonsterSetBase.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "Protocol.h"
#include "ItemManager.h"
#ifdef _WIN32
#include "resource.h"
#endif
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "Util.h"

CInvasionManager gInvasionManager;

namespace
{
	const int TAMACHAN_MAP = 0;
	const int TAMACHAN_DROP_MIN_X = 156;
	const int TAMACHAN_DROP_MAX_X = 170;
	const int TAMACHAN_DROP_MIN_Y = 126;
	const int TAMACHAN_DROP_MAX_Y = 129;
	// Tipo de evento del opcode 0x0B que el cliente 0.98j interpreta como
	// Tamachan (ProtocolCore 0x444299): estado 1 lo hace aparecer en el
	// puente de Lorencia y estado 0 lo despide.
	const BYTE TAMACHAN_EVENT_INDEX = 2;
	const int TAMACHAN_DROPS_PER_TYPE = 3;
	const int TAMACHAN_DROP_INTERVAL_SECONDS = 60;
	const int TAMACHAN_DROP_STEP_SECONDS = 12;
	const int TAMACHAN_FIREWORKS_DISTANCE = 12;
	const int TAMACHAN_JEWEL_LIST[] =
	{
		GET_ITEM(12, 15), // Chaos
		GET_ITEM(14, 13), // Bless
		GET_ITEM(14, 14), // Soul
		GET_ITEM(14, 16), // Life
		GET_ITEM(14, 22), // Creation
	};
}

CInvasionManager::CInvasionManager()
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		lpInfo->Index = n;

		lpInfo->State = INVASION_STATE_BLANK;

		lpInfo->RemainTime = 0;

		lpInfo->TargetTime = 0;

		lpInfo->TickCount = GetTickCount();

		lpInfo->TamachanActive = 0;
		lpInfo->TamachanRainWave = -1;
		lpInfo->TamachanRainDropCount = 0;

		this->CleanMonster(lpInfo);
	}

	for (int n = 0; n < MAX_OBJECT; n++)
	{
		this->m_TamachanUserMap[n] = -1;
	}
}

CInvasionManager::~CInvasionManager()
{

}

void CInvasionManager::Init()
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		if (gServerInfo.m_InvasionManagerSwitch == 0)
		{
			this->SetState(&this->m_InvasionInfo[n], INVASION_STATE_BLANK);
		}
		else
		{
			this->SetState(&this->m_InvasionInfo[n], INVASION_STATE_EMPTY);
		}
	}

	gFlyingDragons.Init();
}

void CInvasionManager::Load(const char* path)
{
	CReadScript* lpReadScript = new CReadScript;

	if (lpReadScript == NULL)
	{
		ErrorMessageBox(READ_SCRIPT_ALLOC_ERROR, path);

		return;
	}

	if (!lpReadScript->Load(path))
	{
		ErrorMessageBox(READ_SCRIPT_FILE_ERROR, path);

		delete lpReadScript;

		return;
	}

	for (int n = 0; n < MAX_INVASION; n++)
	{
		this->m_InvasionInfo[n].RespawnMessage = -1;

		this->m_InvasionInfo[n].DespawnMessage = -1;

		this->m_InvasionInfo[n].BossIndex = -1;

		this->m_InvasionInfo[n].BossMessage = -1;

		this->m_InvasionInfo[n].InvasionTime = 0;

		this->m_InvasionInfo[n].WarningTime = 0;

		this->m_InvasionInfo[n].WarningMsg = -1;

		this->m_InvasionInfo[n].TamachanRainWave = -1;
		this->m_InvasionInfo[n].TamachanRainDropCount = 0;

		this->m_InvasionInfo[n].StartTime.clear();

		this->m_InvasionInfo[n].RespawnInfo[0].clear();

		this->m_InvasionInfo[n].RespawnInfo[1].clear();

		this->m_InvasionInfo[n].RespawnInfo[2].clear();

		this->m_InvasionInfo[n].RespawnInfo[3].clear();

		this->m_InvasionInfo[n].RespawnInfo[4].clear();

		this->m_InvasionInfo[n].RespawnInfo[5].clear();

		this->m_InvasionInfo[n].RespawnInfo[6].clear();

		this->m_InvasionInfo[n].RespawnInfo[7].clear();

		this->m_InvasionInfo[n].RespawnInfo[8].clear();

		this->m_InvasionInfo[n].RespawnInfo[9].clear();

		this->m_InvasionInfo[n].RespawnInfo[10].clear();

		this->m_InvasionInfo[n].RespawnInfo[11].clear();

		this->m_InvasionInfo[n].RespawnInfo[12].clear();

		this->m_InvasionInfo[n].RespawnInfo[13].clear();

		this->m_InvasionInfo[n].RespawnInfo[14].clear();

		this->m_InvasionInfo[n].RespawnInfo[15].clear();

		this->m_InvasionInfo[n].RespawnInfo[16].clear();

		this->m_InvasionInfo[n].RespawnInfo[17].clear();

		this->m_InvasionInfo[n].RespawnInfo[18].clear();

		this->m_InvasionInfo[n].RespawnInfo[19].clear();

		this->m_InvasionInfo[n].MonsterInfo.clear();
	}

	try
	{
		eTokenResult token;

		while (true)
		{
			token = lpReadScript->GetToken();

			if (token == TOKEN_END || token == TOKEN_END_SECTION)
			{
				break;
			}

			int section = lpReadScript->GetNumber();

			while (true)
			{
				token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				if (section == 0)
				{
					INVASION_START_TIME info;

					int index = lpReadScript->GetNumber();

					info.Year = lpReadScript->GetAsNumber();

					info.Month = lpReadScript->GetAsNumber();

					info.Day = lpReadScript->GetAsNumber();

					info.DayOfWeek = lpReadScript->GetAsNumber();

					info.Hour = lpReadScript->GetAsNumber();

					info.Minute = lpReadScript->GetAsNumber();

					info.Second = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].StartTime.push_back(info);
				}
				else if (section == 1)
				{
					int index = lpReadScript->GetNumber();

					this->m_InvasionInfo[index].RespawnMessage = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].DespawnMessage = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].BossIndex = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].BossMessage = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].InvasionTime = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].WarningTime = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].WarningMsg = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].InvasionEffect = lpReadScript->GetAsNumber();

					strcpy_s(this->m_InvasionInfo[index].InvasionName, lpReadScript->GetAsString());

#ifdef _WIN32
					CreateSubMenuItem(ID_STARTINVASION, index, this->m_InvasionInfo[index].InvasionName);
#endif
				}
				else if (section == 2)
				{
					INVASION_RESPWAN_INFO info;

					int index = lpReadScript->GetNumber();

					info.Group = lpReadScript->GetAsNumber();

					info.Map = lpReadScript->GetAsNumber();

					info.Value = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].RespawnInfo[info.Group].push_back(info);
				}
				else if (section == 3)
				{
					INVASION_MONSTER_INFO info;

					int index = lpReadScript->GetNumber();

					info.Group = lpReadScript->GetAsNumber();

					info.MonsterClass = lpReadScript->GetAsNumber();

					info.RegenType = lpReadScript->GetAsNumber();

					info.RegenTime = lpReadScript->GetAsNumber();

					this->m_InvasionInfo[index].MonsterInfo.push_back(info);
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

void CInvasionManager::MainProc()
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		DWORD elapsed = GetTickCount() - lpInfo->TickCount;

		if (elapsed < 1000)
		{
			continue;
		}

		lpInfo->TickCount = GetTickCount();

		lpInfo->RemainTime = (int)difftime(lpInfo->TargetTime, time(0));

		switch (lpInfo->State)
		{
			case INVASION_STATE_BLANK:
			{
				this->ProcState_BLANK(lpInfo);

				break;
			}

			case INVASION_STATE_EMPTY:
			{
				this->ProcState_EMPTY(lpInfo);

				break;
			}

			case INVASION_STATE_START:
			{
				this->ProcState_START(lpInfo);

				break;
			}
		}
	}
}

void CInvasionManager::ProcState_BLANK(INVASION_INFO* lpInfo)
{

}

void CInvasionManager::ProcState_EMPTY(INVASION_INFO* lpInfo)
{
	if (lpInfo->RemainTime > 0 && lpInfo->RemainTime <= (lpInfo->WarningTime * 60))
	{
		int minutes = lpInfo->RemainTime / 60;

		if ((lpInfo->RemainTime % 60) == 0)
		{
			minutes--;
		}

		if (lpInfo->MinutesLeft != minutes)
		{
			lpInfo->MinutesLeft = minutes;

			gNotice.GCNoticeSendToAll(0, lpInfo->WarningMsg, lpInfo->InvasionName, (lpInfo->MinutesLeft + 1));
		}
	}

	if (lpInfo->RemainTime <= 0)
	{
		if (_stricmp(lpInfo->InvasionName, "Tamachan") == 0)
		{
			gNotice.GCNoticeSendToAll(0, "Tamachan ha aparecido en Lorencia.");
		}
		else if (lpInfo->RespawnMessage != -1)
		{
			gNotice.GCNoticeSendToAll(0, lpInfo->RespawnMessage, lpInfo->InvasionName);
		}

		this->SetState(lpInfo, INVASION_STATE_START);
	}
}

void CInvasionManager::ProcState_START(INVASION_INFO* lpInfo)
{
	if (lpInfo->RemainTime <= 0)
	{
		if (lpInfo->DespawnMessage != -1)
		{
			gNotice.GCNoticeSendToAll(0, lpInfo->DespawnMessage, lpInfo->InvasionName);
		}

		this->SetState(lpInfo, INVASION_STATE_EMPTY);
	}
	else
	{
		this->ProcTamachanJewelRain(lpInfo);
	}
}

void CInvasionManager::SetState(INVASION_INFO* lpInfo, int state)
{
	lpInfo->State = state;

	switch (lpInfo->State)
	{
		case INVASION_STATE_BLANK:
		{
			this->SetState_BLANK(lpInfo);

			break;
		}

		case INVASION_STATE_EMPTY:
		{
			this->SetState_EMPTY(lpInfo);

			break;
		}

		case INVASION_STATE_START:
		{
			this->SetState_START(lpInfo);

			break;
		}
	}
}

void CInvasionManager::SetState_BLANK(INVASION_INFO* lpInfo)
{

}

void CInvasionManager::SetState_EMPTY(INVASION_INFO* lpInfo)
{
	lpInfo->TamachanRainWave = -1;
	lpInfo->TamachanRainDropCount = 0;

	// Fin del Tamachan (por tiempo o porque Init() reinicio las invasiones):
	// se le avisa a Lorencia para que el cliente lo despida.
	if (lpInfo->TamachanActive != 0)
	{
		lpInfo->TamachanActive = 0;

		GCEventStateSendToAll(TAMACHAN_MAP, 0, TAMACHAN_EVENT_INDEX);
	}

	this->ClearMonster(lpInfo);

	this->CheckSync(lpInfo);
}

void CInvasionManager::SetState_START(INVASION_INFO* lpInfo)
{
	for (int n = 0; n < MAX_INVASION_RESPAWN_GROUP; n++)
	{
		if (lpInfo->RespawnInfo[n].empty() == 0)
		{
			INVASION_RESPWAN_INFO* lpRespawnInfo = &lpInfo->RespawnInfo[n][(GetLargeRand() % lpInfo->RespawnInfo[n].size())];

			for (std::vector<INVASION_MONSTER_INFO>::iterator it = lpInfo->MonsterInfo.begin(); it != lpInfo->MonsterInfo.end(); it++)
			{
				if (it->Group == lpRespawnInfo->Group)
				{
					this->SetMonster(lpInfo, lpRespawnInfo, &(*it));
				}
			}
		}
	}

	lpInfo->RemainTime = lpInfo->InvasionTime * 60;

	lpInfo->TargetTime = (int)(time(0) + lpInfo->RemainTime);

	lpInfo->TamachanRainWave = -1;
	lpInfo->TamachanRainDropCount = 0;

	if (this->IsTamachanInvasion(lpInfo) != 0)
	{
		// El Tamachan del 0.98j no es un monstruo: lo dibuja el cliente al
		// recibir el 0x0B con el evento 2.  Solo se avisa a quien esta en
		// Lorencia; el resto lo recibe al entrar (TamachanUserMapCheck).
		lpInfo->TamachanActive = 1;

		GCEventStateSendToAll(TAMACHAN_MAP, 1, TAMACHAN_EVENT_INDEX);
	}

	this->ProcTamachanJewelRain(lpInfo);
}

bool CInvasionManager::IsTamachanInvasion(INVASION_INFO* lpInfo)
{
	return (_stricmp(lpInfo->InvasionName, "Tamachan") == 0);
}

bool CInvasionManager::IsTamachanActive()
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		if (this->IsTamachanInvasion(lpInfo) != 0 && lpInfo->State == INVASION_STATE_START)
		{
			return 1;
		}
	}

	return 0;
}

void CInvasionManager::TamachanUserReset(int aIndex)
{
	if (OBJECT_RANGE(aIndex) != 0)
	{
		this->m_TamachanUserMap[aIndex] = -1;
	}
}

void CInvasionManager::TamachanUserMapCheck(LPOBJ lpObj)
{
	if (lpObj->Type != OBJECT_USER || OBJECT_RANGE(lpObj->Index) == 0)
	{
		return;
	}

	int lastMap = this->m_TamachanUserMap[lpObj->Index];

	this->m_TamachanUserMap[lpObj->Index] = lpObj->Map;

	// Solo al ENTRAR a Lorencia: el cliente borra al Tamachan cuando sale del
	// mapa, pero dentro del mapa lo mantiene, y reenviar el estado 1 lo haria
	// reaparecer en el punto de inicio (este hook tambien corre al cambiar
	// anillos, al teletransportarse dentro del mapa, etc).
	if (lpObj->Map != TAMACHAN_MAP || lastMap == TAMACHAN_MAP)
	{
		return;
	}

	if (this->IsTamachanActive() != 0)
	{
		GCEventStateSend(lpObj->Index, 1, TAMACHAN_EVENT_INDEX);
	}
}

void CInvasionManager::ProcTamachanJewelRain(INVASION_INFO* lpInfo)
{
	if (this->IsTamachanInvasion(lpInfo) == 0)
	{
		return;
	}

	if (lpInfo->InvasionTime <= 0)
	{
		return;
	}

	int elapsed = (lpInfo->InvasionTime * 60) - lpInfo->RemainTime;

	if (elapsed < 0)
	{
		return;
	}

	int wave = (elapsed / TAMACHAN_DROP_INTERVAL_SECONDS);

	if (wave < 0 || wave >= lpInfo->InvasionTime)
	{
		return;
	}

	if (lpInfo->TamachanRainWave != wave)
	{
		lpInfo->TamachanRainWave = wave;
		lpInfo->TamachanRainDropCount = 0;
	}

	int jewelCount = (sizeof(TAMACHAN_JEWEL_LIST) / sizeof(TAMACHAN_JEWEL_LIST[0]));
	int dropsPerMinute = (TAMACHAN_DROPS_PER_TYPE * jewelCount);
	int elapsedInMinute = (elapsed % TAMACHAN_DROP_INTERVAL_SECONDS);
	int expectedDrops = (elapsedInMinute / TAMACHAN_DROP_STEP_SECONDS) + 1;

	if (expectedDrops > dropsPerMinute)
	{
		expectedDrops = dropsPerMinute;
	}

	if (lpInfo->TamachanRainDropCount < expectedDrops)
	{
		this->DropTamachanJewelSingle(lpInfo->TamachanRainDropCount);

		lpInfo->TamachanRainDropCount++;
	}
}

void CInvasionManager::DropTamachanJewelSingle(int dropSequence)
{
	int dropOwnerIndex = this->GetTamachanDropOwnerIndex(TAMACHAN_MAP);

	if (OBJECT_USER_RANGE(dropOwnerIndex) == 0)
	{
		return;
	}

	int jewelCount = (sizeof(TAMACHAN_JEWEL_LIST) / sizeof(TAMACHAN_JEWEL_LIST[0]));
	int jewel = (dropSequence % jewelCount);
	int x = TAMACHAN_DROP_MIN_X + (GetLargeRand() % ((TAMACHAN_DROP_MAX_X - TAMACHAN_DROP_MIN_X) + 1));
	int y = TAMACHAN_DROP_MIN_Y + (GetLargeRand() % ((TAMACHAN_DROP_MAX_Y - TAMACHAN_DROP_MIN_Y) + 1));

	GDCreateItemSend(dropOwnerIndex, (BYTE)TAMACHAN_MAP, (BYTE)x, (BYTE)y, TAMACHAN_JEWEL_LIST[jewel], 0, 0, 0, 0, 0, -1, 0);

	int fireworksSource = this->GetTamachanFireworksSourceIndex(TAMACHAN_MAP, x, y);

	if (OBJECT_USER_RANGE(fireworksSource) != 0)
	{
		GCFireworksSend(&gObj[fireworksSource], x, y);
	}
}

int CInvasionManager::GetTamachanDropOwnerIndex(int map)
{
	int fallbackIndex = -1;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		LPOBJ lpUser = &gObj[n];

		if (lpUser->Type != OBJECT_USER || lpUser->Connected != OBJECT_ONLINE)
		{
			continue;
		}

		if (fallbackIndex == -1)
		{
			fallbackIndex = n;
		}

		if (lpUser->Map == map)
		{
			return n;
		}
	}

	return fallbackIndex;
}

int CInvasionManager::GetTamachanFireworksSourceIndex(int map, int x, int y)
{
	int mapFallback = -1;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		LPOBJ lpUser = &gObj[n];

		if (lpUser->Type != OBJECT_USER || lpUser->Connected != OBJECT_ONLINE || lpUser->Map != map)
		{
			continue;
		}

		if (mapFallback == -1)
		{
			mapFallback = n;
		}

		if (abs(lpUser->X - x) <= TAMACHAN_FIREWORKS_DISTANCE && abs(lpUser->Y - y) <= TAMACHAN_FIREWORKS_DISTANCE)
		{
			return n;
		}
	}

	return mapFallback;
}

void CInvasionManager::CheckSync(INVASION_INFO* lpInfo)
{
	if (lpInfo->StartTime.empty() != 0)
	{
		this->SetState(lpInfo, INVASION_STATE_BLANK);

		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<INVASION_START_TIME>::iterator it = lpInfo->StartTime.begin(); it != lpInfo->StartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(lpInfo, INVASION_STATE_BLANK);

		return;
	}

	lpInfo->RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	lpInfo->TargetTime = (int)ScheduleTime.GetTime();
}

int CInvasionManager::GetState(int index)
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return INVASION_STATE_BLANK;
	}

	return this->m_InvasionInfo[index].State;
}

char* CInvasionManager::GetInvasionName(int index)
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return NULL;
	}

	return this->m_InvasionInfo[index].InvasionName;
}

int CInvasionManager::GetCurrentRemainTime(int index)
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return 0;
	}

	return this->m_InvasionInfo[index].RemainTime;
}

int CInvasionManager::GetRemainTime(int index)
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return 0;
	}

	INVASION_INFO* lpInfo = &this->m_InvasionInfo[index];

	if (lpInfo->StartTime.empty() != 0)
	{
		return 0;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<INVASION_START_TIME>::iterator it = lpInfo->StartTime.begin(); it != lpInfo->StartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		return 0;
	}

	int RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	return (((RemainTime % 60) == 0) ? (RemainTime / 60) : ((RemainTime / 60) + 1));
}

bool CInvasionManager::AddMonster(INVASION_INFO* lpInfo, int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if (this->GetMonster(lpInfo, aIndex) != 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) == 0)
		{
			lpInfo->MonsterIndex[n] = aIndex;

			return 1;
		}
	}

	return 0;
}

bool CInvasionManager::DelMonster(INVASION_INFO* lpInfo, int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	int* index = this->GetMonster(lpInfo, aIndex);

	if (index == 0)
	{
		return 0;
	}

	(*index) = -1;

	return 1;
}

int* CInvasionManager::GetMonster(INVASION_INFO* lpInfo, int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (lpInfo->MonsterIndex[n] == aIndex)
		{
			return &lpInfo->MonsterIndex[n];
		}
	}

	return 0;
}

void CInvasionManager::CleanMonster(INVASION_INFO* lpInfo)
{
	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		lpInfo->MonsterIndex[n] = -1;
	}
}

void CInvasionManager::ClearMonster(INVASION_INFO* lpInfo)
{
	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) != 0)
		{
			gObjDel(lpInfo->MonsterIndex[n]);

			lpInfo->MonsterIndex[n] = -1;
		}
	}
}

int CInvasionManager::GetMonsterCount(INVASION_INFO* lpInfo)
{
	int count = 0;

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) != 0)
		{
			count++;
		}
	}

	return count;
}

void CInvasionManager::SetMonster(INVASION_INFO* lpInfo, INVASION_RESPWAN_INFO* lpRespawnInfo, INVASION_MONSTER_INFO* lpMonsterInfo)
{
	for (int n = 0; n < gMonsterSetBase.m_count; n++)
	{
		MONSTER_SET_BASE_INFO* lpMsbInfo = &gMonsterSetBase.m_MonsterSetBaseInfo[n];

		if (lpMsbInfo->Type != 3 || lpMsbInfo->MonsterClass != lpMonsterInfo->MonsterClass || lpMsbInfo->Map != lpRespawnInfo->Map || lpMsbInfo->Value != lpRespawnInfo->Value)
		{
			continue;
		}

		int index = gObjAddMonster(lpMsbInfo->Map);

		if (OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[index];

		if (gObjSetPosMonster(index, n) == 0)
		{
			gObjDel(index);

			continue;
		}

		if (gObjSetMonster(index, lpMsbInfo->MonsterClass) == 0)
		{
			gObjDel(index);

			continue;
		}

		lpObj->MaxRegenTime = ((lpMonsterInfo->RegenType == 0) ? ((lpInfo->InvasionTime * 60) * 1000) : lpMonsterInfo->RegenTime * 1000);

		if (this->AddMonster(lpInfo, index) == 0)
		{
			gObjDel(index);

			continue;
		}

		if (lpObj->Class == lpInfo->BossIndex)
		{
			LogAdd(LOG_EVENT, "[Invasion Manager] (%s) Boss Position (Map: %d, X: %d, Y: %d", lpInfo->InvasionName, lpObj->Map, lpObj->X, lpObj->Y);
		}

		if (gServerInfo.m_FlyingDragonsOnlyBossMapSpawn != 0)
		{
			if (lpObj->Class == lpInfo->BossIndex)
			{
				gFlyingDragons.FlyingDragonsAdd(lpObj->Map, lpInfo->InvasionTime * 60, lpInfo->InvasionEffect);
			}
		}
		else
		{
			gFlyingDragons.FlyingDragonsAdd(lpObj->Map, lpInfo->InvasionTime * 60, lpInfo->InvasionEffect);
		}
	}
}

void CInvasionManager::MonsterDieProc(LPOBJ lpObj, LPOBJ lpTarget)
{
	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if (OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		if (this->GetState(lpInfo->Index) != INVASION_STATE_START)
		{
			continue;
		}

		if (this->GetMonster(lpInfo, lpObj->Index) == 0)
		{
			continue;
		}

		if (lpObj->Class == lpInfo->BossIndex)
		{
			if (gServerInfo.m_FlyingDragonsKillBossDisappear != 0)
			{
				gFlyingDragons.FlyingDragonsBossDieProc(lpObj->Map);
			}

			if (lpInfo->BossMessage != -1)
			{
				gNotice.GCNoticeSendToAll(0, lpInfo->BossMessage, lpTarget->Name, lpObj->Name);
			}
		}
	}
}

bool CInvasionManager::StartInvasion(int InvasionIndex)
{
	if (!gServerInfo.m_InvasionManagerSwitch || InvasionIndex < 0 || InvasionIndex >= MAX_INVASION) return false;
	if (!m_InvasionInfo[InvasionIndex].InvasionName[0] || m_InvasionInfo[InvasionIndex].State == INVASION_STATE_START) return false;

	// One dated occurrence, including rollover through midnight.
	time_t theTime = time(NULL) + 120;
	tm aTime;
	localtime_s(&aTime, &theTime);
	int hour = aTime.tm_hour;
	int minute = aTime.tm_min;

	INVASION_START_TIME info;

	info.Year = aTime.tm_year + 1900;

	info.Month = aTime.tm_mon + 1;

	info.Day = aTime.tm_mday;

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_InvasionInfo[InvasionIndex].StartTime.push_back(info);

	LogAdd(LOG_EVENT, "[Set Invasion Start][%s] At %2d:%2d:00", this->m_InvasionInfo[InvasionIndex].InvasionName, hour, minute);

	this->SetState(&this->m_InvasionInfo[InvasionIndex], INVASION_STATE_EMPTY);
	return true;
}

