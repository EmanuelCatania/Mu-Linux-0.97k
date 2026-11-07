#include "stdafx.h"
#include "CustomPet.h"
#include "ItemManager.h"
#include "JsonLite.h"
#include "Map.h"
#include "Util.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

CCustomPet gCustomPet;

static_assert(sizeof(CATALOG_PET) == 24, "catalog pet");

namespace
{
	int ParsePetItem(const JsonValue* value)
	{
		if (value == nullptr)
		{
			return -1;
		}

		int section = -1, sub = -1;

		if (value->IsString())
		{
			if (sscanf(value->text.c_str(), "%d,%d", &section, &sub) != 2)
			{
				return -1;
			}
		}
		else if (value->IsArray() && value->items.size() == 2 && value->items[0].IsNumber() && value->items[1].IsNumber())
		{
			section = (int)value->items[0].number;

			sub = (int)value->items[1].number;
		}

		if (section < 0 || section >= MAX_ITEM_SECTION || sub < 0 || sub >= MAX_ITEM_TYPE + MAX_ITEM_EX_PER_SECTION)
		{
			return -1;
		}

		return GET_ITEM(section, sub);
	}

	bool ReadPet(int item, const JsonValue& root, CUSTOM_PET_INFO& out, std::string& error)
	{
		memset(&out, 0, sizeof(out));

		out.Item = item;

		out.Catalog.Item = (WORD)item;

		out.Catalog.BlendMesh = 0xFF;

		out.Catalog.Scale = (float)root.GetNumber("scale", 1.0);

		out.Catalog.Action = (BYTE)root.GetNumber("action", 0);

		double blend = root.GetNumber("blendMesh", -1);

		if (blend >= 0 && blend < 255)
		{
			out.Catalog.BlendMesh = (BYTE)blend;
		}

		const JsonValue* movement = root.Get("movement");

		if (movement == nullptr || !movement->IsObject())
		{
			error = "falta 'movement'";

			return false;
		}

		std::string type = movement->GetString("type", "");

		if (type == "follow")
		{
			out.Catalog.Movement = CATALOG_PET_MOVE_FOLLOW;
		}
		else if (type == "orbit")
		{
			out.Catalog.Movement = CATALOG_PET_MOVE_ORBIT;
		}
		else
		{
			error = "'movement.type' tiene que ser 'follow' u 'orbit'";

			return false;
		}

		out.Catalog.Radius = (float)movement->GetNumber("radius", 50.0);

		out.Catalog.Period = (float)movement->GetNumber("period", 4000.0);

		out.Catalog.Height = (float)movement->GetNumber("height", 20.0);

		if (out.Catalog.Period < 100.0f)
		{
			error = "'movement.period' tiene que ser de al menos 100 ms";

			return false;
		}

		if (const JsonValue* abilities = root.Get("abilities"))
		{
			if (!abilities->IsArray())
			{
				error = "'abilities' tiene que ser un array";

				return false;
			}

			for (const JsonValue& ability : abilities->items)
			{
				std::string kind = ability.GetString("type", "");

				if (kind == "pickup")
				{
					if (ability.GetString("what", "zen") != "zen")
					{
						error = "'pickup' por ahora sólo levanta 'zen'";

						return false;
					}

					out.Pickup.Enabled = true;

					out.Pickup.Range = (int)ability.GetNumber("range", 3);

					out.Pickup.Interval = (DWORD)ability.GetNumber("interval", 1000);

					out.Pickup.Delay = (DWORD)ability.GetNumber("delay", 1500);

					out.Catalog.Flags |= CATALOG_PET_FLAG_COLLECT;

					if (out.Pickup.Range < 1 || out.Pickup.Range > 8)
					{
						error = "'pickup.range' va de 1 a 8 casillas";

						return false;
					}

					out.Catalog.CollectRange = (BYTE)out.Pickup.Range;
				}
				else
				{
					error = "habilidad desconocida: " + kind;

					return false;
				}
			}
		}

		return true;
	}
}

CCustomPet::CCustomPet()
{
	memset(this->m_LastPickup, 0, sizeof(this->m_LastPickup));

	memset(this->m_TargetTime, 0, sizeof(this->m_TargetTime));

	for (int n = 0; n < MAX_OBJECT; n++)
	{
		this->m_Target[n] = -1;
	}
}

void CCustomPet::Load(const char* folder)
{
	this->m_Info.clear();

	this->m_Catalog.clear();

	std::error_code ec;

	if (!std::filesystem::is_directory(folder, ec))
	{
		return;
	}

	std::vector<std::filesystem::path> files;

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folder, ec))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".json")
		{
			files.push_back(entry.path());
		}
	}

	std::sort(files.begin(), files.end());

	for (const std::filesystem::path& file : files)
	{
		const std::string name = file.filename().string();

		std::ifstream stream(file, std::ios::binary);

		std::stringstream buffer;

		buffer << stream.rdbuf();

		JsonValue root;

		std::string error;

		if (!JsonParser::Parse(buffer.str(), root, error))
		{
			LogAdd(LOG_RED, "[CustomPet] %s: JSON inválido: %s", name.c_str(), error.c_str());

			continue;
		}

		const int item = ParsePetItem(root.Get("item"));

		ITEM_INFO* lpInfo = (item < 0) ? 0 : gItemManager.GetInfoPtr(item);

		if (lpInfo == 0)
		{
			LogAdd(LOG_RED, "[CustomPet] %s: 'item' no existe en Item.txt", name.c_str());

			continue;
		}

		if (lpInfo->Slot != 8)
		{
			LogAdd(LOG_RED, "[CustomPet] %s: el item %d no va en el slot del helper (Slot 8 en Item.txt)", name.c_str(), item);

			continue;
		}

		CUSTOM_PET_INFO info;

		if (!ReadPet(item, root, info, error))
		{
			LogAdd(LOG_RED, "[CustomPet] %s: %s", name.c_str(), error.c_str());

			continue;
		}

		this->m_Info[item] = info;

		this->m_Catalog.push_back(info.Catalog);

		LogAdd(LOG_BLUE, "[CustomPet] %s: pet del item %d%s", name.c_str(), item, info.Pickup.Enabled ? ", levanta zen" : "");
	}
}

const CUSTOM_PET_INFO* CCustomPet::GetInfo(int item) const
{
	std::map<int, CUSTOM_PET_INFO>::const_iterator it = this->m_Info.find(item);

	return (it == this->m_Info.end()) ? 0 : &it->second;
}

void CCustomPet::MainProc(LPOBJ lpObj)
{
	if (this->m_Info.empty() || lpObj->Type != OBJECT_USER || lpObj->DieRegen != 0)
	{
		return;
	}

	CItem* lpPet = &lpObj->Inventory[8];

	if (lpPet->IsItem() == 0 || lpPet->m_IsValidItem == 0)
	{
		return;
	}

	const CUSTOM_PET_INFO* lpInfo = this->GetInfo(lpPet->m_Index);

	if (lpInfo == 0)
	{
		return;
	}

	if (lpInfo->Pickup.Enabled && (GetTickCount() - this->m_LastPickup[lpObj->Index]) >= lpInfo->Pickup.Interval)
	{
		this->m_LastPickup[lpObj->Index] = GetTickCount();

		this->PickupMoney(lpObj, lpInfo->Pickup);
	}
}

// Zen del mapa que el jugador podría levantar, dentro de `range` casillas.
// Misma regla que CMap::CheckItemGive, sin el aviso de "no es tuyo".
bool CCustomPet::CanTakeMoney(LPOBJ lpObj, int index, int range) const
{
	if (MAP_ITEM_RANGE(index) == 0)
	{
		return false;
	}

	CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

	if (lpItem->IsItem() == 0 || lpItem->m_Give == 1 || lpItem->m_Live == 0 || lpItem->GetKind() != GET_ITEM(14, 15))
	{
		return false;
	}

	int x = lpItem->m_X - lpObj->X;

	int y = lpItem->m_Y - lpObj->Y;

	if (x > range || x < -range || y > range || y < -range)
	{
		return false;
	}

	if (OBJECT_RANGE(lpItem->m_UserIndex) != 0 && lpItem->m_LootTime > GetTickCount() && lpItem->m_UserIndex != lpObj->Index)
	{
		if (lpItem->m_QuestItem != 0 || OBJECT_RANGE(lpObj->PartyNumber) == 0 || lpObj->PartyNumber != gObj[lpItem->m_UserIndex].PartyNumber)
		{
			return false;
		}
	}

	return true;
}

// El pet elige el zen más cercano y lo levanta recién después de `Delay`, el
// tiempo que tarda en llegar volando en el cliente (como el Collecter del
// 5.2).  De a uno por vez.
void CCustomPet::PickupMoney(LPOBJ lpObj, const CUSTOM_PET_PICKUP& pickup)
{
	if (MAP_RANGE(lpObj->Map) == 0 || lpObj->Interface.use != 0 || lpObj->Transaction == 1)
	{
		return;
	}

	int& target = this->m_Target[lpObj->Index];

	if (target >= 0 && !this->CanTakeMoney(lpObj, target, pickup.Range))
	{
		target = -1;
	}

	if (target < 0)
	{
		int best = -1, bestDistance = 0;

		for (int n = 0; n < MAX_MAP_ITEM; n++)
		{
			if (!this->CanTakeMoney(lpObj, n, pickup.Range))
			{
				continue;
			}

			CMapItem* lpItem = &gMap[lpObj->Map].m_Item[n];

			int distance = (lpItem->m_X - lpObj->X) * (lpItem->m_X - lpObj->X) + (lpItem->m_Y - lpObj->Y) * (lpItem->m_Y - lpObj->Y);

			if (best < 0 || distance < bestDistance)
			{
				best = n;

				bestDistance = distance;
			}
		}

		if (best >= 0)
		{
			target = best;

			this->m_TargetTime[lpObj->Index] = GetTickCount();
		}

		return;
	}

	if ((GetTickCount() - this->m_TargetTime[lpObj->Index]) < pickup.Delay)
	{
		return;
	}

	CMapItem* lpItem = &gMap[lpObj->Map].m_Item[target];

	if (gObjCheckMaxMoney(lpObj->Index, lpItem->m_BuyMoney) == 0)
	{
		return;
	}

	gMap[lpObj->Map].ItemGive(lpObj->Index, target);

	lpObj->Money += lpItem->m_BuyMoney;

	target = -1;

	PMSG_ITEM_GET_SEND pMsg;

	pMsg.header.setE(0x22, sizeof(pMsg));

	memset(pMsg.ItemInfo, 0, sizeof(pMsg.ItemInfo));

	pMsg.result = 0xFE;

	pMsg.ItemInfo[0] = SET_NUMBERHB(SET_NUMBERHW(lpObj->Money));

	pMsg.ItemInfo[1] = SET_NUMBERLB(SET_NUMBERHW(lpObj->Money));

	pMsg.ItemInfo[2] = SET_NUMBERHB(SET_NUMBERLW(lpObj->Money));

	pMsg.ItemInfo[3] = SET_NUMBERLB(SET_NUMBERLW(lpObj->Money));

	DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
}
