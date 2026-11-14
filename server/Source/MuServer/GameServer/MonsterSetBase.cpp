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
	this->m_Groups.clear();

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

	this->m_Groups.clear();

	for (const MapSpawnEntry& e : entries)
	{
		SpawnGroup& g = this->m_Groups[FindSpawnGroup(this->m_Groups, e, true)];

		g.Entries.push_back(e);

		g.RowStart.push_back(this->m_count);

		this->AddEntry(e);

		g.RowCount.push_back(this->m_count - g.RowStart.back());
	}

	LogAdd(LOG_BLUE, "[MonsterSetBase] %d spawns desde %s (%d entradas)", this->m_count, path, (int)entries.size());

	return true;
}

// El archivo de una entrada: su `Source` es "archivo:línea".
static std::string SpawnEntryFile(const MapSpawnEntry& e)
{
	size_t colon = e.Source.rfind(':');

	return (colon == std::string::npos) ? e.Source : e.Source.substr(0, colon);
}

// MapSpawns_Load intercala las entradas de los distintos archivos (las ordena
// por sección, como la tabla de MonsterSetBase.txt), así que el grupo de cada
// entrada se busca por nombre de archivo.
int CMonsterSetBase::FindSpawnGroup(std::vector<SpawnGroup>& groups, const MapSpawnEntry& e, bool create)
{
	std::string file = SpawnEntryFile(e);

	for (size_t i = 0; i < groups.size(); i++)
	{
		if (groups[i].File == file)
		{
			return (int)i;
		}
	}

	if (!create)
	{
		return -1;
	}

	groups.emplace_back();

	groups.back().File = file;

	return (int)groups.size() - 1;
}

// Dos versiones de un archivo de spawns dan la misma tabla si sus entradas
// coinciden campo a campo (los comentarios y los números de línea no cuentan).
static bool SameSpawnEntries(const std::vector<MapSpawnEntry>& a, const std::vector<MapSpawnEntry>& b)
{
	if (a.size() != b.size())
	{
		return false;
	}

	for (size_t i = 0; i < a.size(); i++)
	{
		const MapSpawnEntry& x = a[i];
		const MapSpawnEntry& y = b[i];

		if (x.Section != y.Section || x.Monster != y.Monster || x.Map != y.Map || x.Dis != y.Dis || x.X != y.X || x.Y != y.Y || x.TX != y.TX || x.TY != y.TY || x.Dir != y.Dir || x.Count != y.Count || x.Value != y.Value)
		{
			return false;
		}
	}

	return true;
}

void CMonsterSetBase::AddEntry(const MapSpawnEntry& e)
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

bool CMonsterSetBase::ReloadMaps(const char* path, std::vector<int>& remap, std::vector<std::pair<int, int>>& changed, std::vector<std::string>& files, std::string& error)
{
	std::vector<MapSpawnEntry> entries;

	if (!MapSpawns_Load(path, entries, error))
	{
		return false;
	}

	// Primero los archivos nuevos completos, para saber cuáles cambiaron.
	std::vector<SpawnGroup> groups;

	for (const MapSpawnEntry& e : entries)
	{
		groups[FindSpawnGroup(groups, e, true)].Entries.push_back(e);
	}

	// keep[i] = grupo viejo cuyas filas se conservan para groups[i], o -1.
	std::vector<int> keep(groups.size(), -1);

	files.clear();

	for (size_t i = 0; i < groups.size(); i++)
	{
		for (size_t o = 0; o < this->m_Groups.size(); o++)
		{
			if (this->m_Groups[o].File == groups[i].File && SameSpawnEntries(this->m_Groups[o].Entries, groups[i].Entries))
			{
				keep[i] = (int)o;
			}
		}

		if (keep[i] < 0)
		{
			files.push_back(groups[i].File);
		}
	}

	// Un archivo que desapareció también es un cambio: sus monstruos se van.
	for (const SpawnGroup& o : this->m_Groups)
	{
		bool kept = false;

		for (const SpawnGroup& g : groups)
		{
			kept = kept || (g.File == o.File);
		}

		if (!kept)
		{
			files.push_back(o.File + " (borrado)");
		}
	}

	// La tabla nueva se arma aparte (son 400 KB) y se copia al final, en el
	// mismo orden de entradas que LoadMaps.
	CMonsterSetBase* next = new CMonsterSetBase;

	remap.assign(this->m_count, -1);

	changed.clear();

	for (SpawnGroup& g : groups)
	{
		g.Entries.clear();
	}

	for (const MapSpawnEntry& e : entries)
	{
		int i = FindSpawnGroup(groups, e, false);

		SpawnGroup& g = groups[i];

		size_t k = g.Entries.size();

		g.Entries.push_back(e);

		g.RowStart.push_back(next->m_count);

		if (keep[i] >= 0)
		{
			const SpawnGroup& old = this->m_Groups[keep[i]];

			for (int n = 0; n < old.RowCount[k] && next->m_count < MAX_MSB_MONSTER; n++)
			{
				next->m_MonsterSetBaseInfo[next->m_count] = this->m_MonsterSetBaseInfo[old.RowStart[k] + n];

				remap[old.RowStart[k] + n] = next->m_count++;
			}
		}
		else
		{
			next->AddEntry(e);

			if (!changed.empty() && changed.back().second == g.RowStart.back())
			{
				changed.back().second = next->m_count;
			}
			else
			{
				changed.emplace_back(g.RowStart.back(), next->m_count);
			}
		}

		g.RowCount.push_back(next->m_count - g.RowStart.back());
	}

	memcpy(this->m_MonsterSetBaseInfo, next->m_MonsterSetBaseInfo, sizeof(this->m_MonsterSetBaseInfo));

	this->m_count = next->m_count;

	this->m_Groups = groups;

	delete next;

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
