#include "stdafx.h"
#include "MonsterSetBase.h"
#include "Map.h"
#include "MapSpawns.h"
#include "ReadScript.h"
#include "Util.h"

CMonsterSetBase gMonsterSetBase;

CMonsterSetBase::CMonsterSetBase()
{
	this->m_count = 0;
}

CMonsterSetBase::~CMonsterSetBase()
{

}

void CMonsterSetBase::Load(const char* path)
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

	this->m_count = 0;

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

				MONSTER_SET_BASE_INFO info;

				memset(&info, 0, sizeof(info));

				info.Type = section;

				info.MonsterClass = lpReadScript->GetNumber();

				info.Map = lpReadScript->GetAsNumber();

				info.Dis = lpReadScript->GetAsNumber();

				info.X = lpReadScript->GetAsNumber();

				info.Y = lpReadScript->GetAsNumber();

				if (section == 1 || section == 3)
				{
					info.TX = lpReadScript->GetAsNumber();

					info.TY = lpReadScript->GetAsNumber();
				}

				info.Dir = lpReadScript->GetAsNumber();

				int count = 1;

				if (section == 1 || section == 3)
				{
					count = lpReadScript->GetAsNumber();

					if (section == 3)
					{
						info.Value = lpReadScript->GetAsNumber();
					}
				}

				this->AddLine(info, count);
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

void CMonsterSetBase::AddLine(MONSTER_SET_BASE_INFO info, int count)
{
	if (info.Type == 2)
	{
		info.X = (info.X - 3) + GetLargeRand() % 7;

		info.Y = (info.Y - 3) + GetLargeRand() % 7;
	}

	if (info.Type == 1 || info.Type == 3)
	{
		for (int n = 0; n < count; n++)
		{
			this->SetInfo(info);
		}
	}
	else
	{
		this->SetInfo(info);
	}
}

bool CMonsterSetBase::LoadMaps(const char* path)
{
	std::vector<MapSpawnEntry> entries;

	std::string error;

	if (!MapSpawns_Load(path, entries, error))
	{
		ErrorMessageBox("[MonsterSetBase] %s", error.c_str());

		return false;
	}

	this->m_count = 0;

	for (const MapSpawnEntry& e : entries)
	{
		MONSTER_SET_BASE_INFO info;

		memset(&info, 0, sizeof(info));

		info.Type = e.Section;

		info.MonsterClass = e.Monster;

		info.Map = e.Map;

		info.Dis = e.Dis;

		info.X = e.X;

		info.Y = e.Y;

		info.TX = e.TX;

		info.TY = e.TY;

		info.Dir = e.Dir;

		info.Value = e.Value;

		this->AddLine(info, e.Count);
	}

	LogAdd(LOG_BLUE, "[MonsterSetBase] %d spawns desde %s (%d entradas)", this->m_count, path, (int)entries.size());

	return true;
}

void CMonsterSetBase::SetInfo(MONSTER_SET_BASE_INFO info)
{
	if (this->m_count < 0 || this->m_count >= MAX_MSB_MONSTER)
	{
		return;
	}

	info.Dir = ((info.Dir == -1) ? (GetLargeRand() % 8) : info.Dir);

	this->m_MonsterSetBaseInfo[this->m_count++] = info;
}

bool CMonsterSetBase::GetPosition(int index, short map, short* ox, short* oy)
{
	if (index < 0 || index >= MAX_MSB_MONSTER)
	{
		return 0;
	}

	MONSTER_SET_BASE_INFO* lpInfo = &this->m_MonsterSetBaseInfo[index];

	if (lpInfo->Type == 0 || lpInfo->Type == 4)
	{
		(*ox) = lpInfo->X;

		(*oy) = lpInfo->Y;

		return 1;
	}
	else if (lpInfo->Type == 1 || lpInfo->Type == 3)
	{
		return this->GetBoxPosition(map, lpInfo->X, lpInfo->Y, lpInfo->TX, lpInfo->TY, ox, oy);
	}
	else if (lpInfo->Type == 2)
	{
		return this->GetBoxPosition(map, (lpInfo->X - 3), (lpInfo->Y - 3), (lpInfo->X + 3), (lpInfo->Y + 3), ox, oy);
	}

	return 0;
}

bool CMonsterSetBase::GetBoxPosition(int map, int x, int y, int tx, int ty, short* ox, short* oy)
{
	for (int n = 0; n < 100; n++)
	{
		int subx = tx - x;

		int suby = ty - y;

		subx = ((subx < 1) ? 1 : subx);

		suby = ((suby < 1) ? 1 : suby);

		subx = x + (GetLargeRand() % subx);

		suby = y + (GetLargeRand() % suby);

		if (gMap[map].CheckAttr(subx, suby, 1) == 0 && gMap[map].CheckAttr(subx, suby, 4) == 0 && gMap[map].CheckAttr(subx, suby, 8) == 0)
		{
			(*ox) = subx;

			(*oy) = suby;

			return 1;
		}
	}

	return 0;
}

void CMonsterSetBase::SetBoxPosition(int index, int map, int x, int y, int tx, int ty)
{
	if (index < 0 || index >= MAX_MSB_MONSTER)
	{
		return;
	}

	MONSTER_SET_BASE_INFO* lpInfo = &this->m_MonsterSetBaseInfo[index];

	lpInfo->Map = map;

	lpInfo->X = x;

	lpInfo->Y = y;

	lpInfo->TX = tx;

	lpInfo->TY = ty;
}
