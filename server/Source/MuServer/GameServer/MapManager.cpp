#include "stdafx.h"
#include "MapManager.h"
#include "MapInfo.h"
#include "ReadScript.h"
#include "ServerInfo.h"
#include "Util.h"

CMapManager gMapManager;

CMapManager::CMapManager()
{
	this->m_MapManagerInfo.clear();
}

CMapManager::~CMapManager()
{

}

void CMapManager::Load(const char* path)
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

	this->m_MapManagerInfo.clear();

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

			MAP_MANAGER_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = lpReadScript->GetNumber();

			info.NonPK = lpReadScript->GetAsNumber();

			info.ViewRange = lpReadScript->GetAsNumber();

			info.ExperienceRate = lpReadScript->GetAsNumber();

			info.ItemDropRate = lpReadScript->GetAsNumber();

			info.ExcItemDropRate = lpReadScript->GetAsNumber();

			info.DeadGate = lpReadScript->GetAsNumber();

			info.RespawnInPlace = lpReadScript->GetAsNumber();

			info.FlyingDragons = lpReadScript->GetAsNumber();

			strcpy_s(info.MapName, lpReadScript->GetAsString());

			this->m_MapManagerInfo.insert(std::pair<int, MAP_MANAGER_INFO>(info.Index, info));
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

void CMapManager::LoadMaps(const char* path)
{
	std::vector<MapInfoDef> defs;

	std::string error;

	if (!MapInfo_Load(path, defs, error))
	{
		// Al arrancar es fatal, como el resto de los datos; en una recarga
		// quedan las reglas de antes.
		if (this->m_MapManagerInfo.empty())
		{
			ErrorMessageBox("[MapManager] %s", error.c_str());
		}
		else
		{
			LogAdd(LOG_RED, "[MapManager] Recarga cancelada, quedan las reglas de antes: %s", error.c_str());
		}

		return;
	}

	this->m_MapManagerInfo.clear();

	for (const MapInfoDef& def : defs)
	{
		MAP_MANAGER_INFO info;

		memset(&info, 0, sizeof(info));

		info.Index = def.Index;

		info.NonPK = def.NonPK;

		info.ViewRange = def.ViewRange;

		info.ExperienceRate = def.ExperienceRate;

		info.ItemDropRate = def.ItemDropRate;

		info.ExcItemDropRate = def.ExcItemDropRate;

		info.DeadGate = def.DeadGate;

		info.RespawnInPlace = def.RespawnInPlace;

		info.FlyingDragons = def.FlyingDragons;

		strcpy_s(info.MapName, def.Name.c_str());

		this->m_MapManagerInfo.insert(std::pair<int, MAP_MANAGER_INFO>(info.Index, info));
	}

	LogAdd(LOG_BLUE, "[MapManager] %d mapas desde %s", (int)defs.size(), path);
}

int CMapManager::GetMapNonPK(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return gServerInfo.m_NonPK;
	}
	else
	{
		return ((it->second.NonPK == -1) ? gServerInfo.m_NonPK : ((it->second.NonPK == 1) ? 1 : 0));
	}
}

int CMapManager::GetMapNonOutlaw(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 0;
	}
	else
	{
		return ((it->second.NonPK == -1) ? 0 : ((it->second.NonPK == 2) ? 1 : 0));
	}
}

int CMapManager::GetMapViewRange(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 15;
	}
	else
	{
		return ((it->second.ViewRange == -1) ? 15 : it->second.ViewRange);
	}
}

int CMapManager::GetMapExperienceRate(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 100;
	}
	else
	{
		return ((it->second.ExperienceRate == -1) ? 100 : it->second.ExperienceRate);
	}
}

int CMapManager::GetMapItemDropRate(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 100;
	}
	else
	{
		return ((it->second.ItemDropRate == -1) ? 100 : it->second.ItemDropRate);
	}
}

int CMapManager::GetMapExcItemDropRate(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 0;
	}
	else
	{
		return ((it->second.ExcItemDropRate == -1) ? 0 : it->second.ExcItemDropRate);
	}
}

int CMapManager::GetMapDeadGate(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 0;
	}
	else
	{
		return it->second.DeadGate;
	}
}

int CMapManager::GetMapRespawnInPlace(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 0;
	}
	else
	{
		return it->second.RespawnInPlace;
	}
}

int CMapManager::GetMapFlyingDragons(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return 0;
	}
	else
	{
		return it->second.FlyingDragons;
	}
}

const char* CMapManager::GetMapName(int index)
{
	std::map<int, MAP_MANAGER_INFO>::iterator it = this->m_MapManagerInfo.find(index);

	if (it == this->m_MapManagerInfo.end())
	{
		return "Not Found";
	}
	else
	{
		return it->second.MapName;
	}
}
