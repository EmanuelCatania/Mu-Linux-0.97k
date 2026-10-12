#include "stdafx.h"
#include "ContentCatalog.h"
#include "CustomPet.h"
#include "CustomWing.h"
#include "ItemManager.h"
#include "MonsterManager.h"
#include "ReadScript.h"
#include "Util.h"
#include "JsonLite.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

CContentCatalog gContentCatalog;

// Registros más chicos que el límite del socket (MAX_MAIN_PACKET_SIZE = 8192).
#define CATALOG_MAX_PACKET_SIZE 4096

static_assert(sizeof(PMSG_CATALOG_HEAD) == 14, "catalog head");
static_assert(sizeof(CATALOG_ITEM) == 264, "catalog item");
static_assert(sizeof(CATALOG_MONSTER) == 96, "catalog monster");
static_assert(sizeof(CATALOG_MAP_FOG) == 8, "catalog map fog");
static_assert(sizeof(PMSG_CATALOG_END_SEND) == 12, "catalog end");
static_assert(sizeof(CATALOG_EFFECT) == 132, "catalog effect");
static_assert(sizeof(CATALOG_TOOLTIP) == 64, "catalog tooltip");

bool IsSafeModelPath(const char* path);

namespace
{
	template <size_t N> void CopyText(char (&dst)[N], const char* src)
	{
		memset(dst, 0, N);

		if (src != 0)
		{
			strncpy(dst, src, N - 1);
		}
	}

	WORD ClampWord(int value)
	{
		return (WORD)((value < 0) ? 0 : ((value > 0xFFFF) ? 0xFFFF : value));
	}

	BYTE ClampByte(int value)
	{
		return (BYTE)((value < 0) ? 0 : ((value > 0xFF) ? 0xFF : value));
	}

	DWORD Fnv1a(DWORD hash, const BYTE* data, size_t size)
	{
		for (size_t n = 0; n < size; n++)
		{
			hash = (hash ^ data[n]) * 16777619u;
		}

		return hash;
	}

	// Carpeta del CustomItem del Encoder: las partes de armadura (secciones
	// 7..11) van por sub-índice bajo Player, el resto por índice bajo Item.
	void LegacyItemFolder(int index, char* folder, int size)
	{
		if (index >= GET_ITEM(7, 0) && index < GET_ITEM(12, 0))
		{
			snprintf(folder, size, "Player\\Custom\\%d\\", GET_ITEM_SUB(index));
		}
		else
		{
			snprintf(folder, size, "Item\\Custom\\%d\\", index);
		}
	}

	CReadScript* OpenLegacy(const char* folder, const char* name)
	{
		char path[MAX_PATH];

		snprintf(path, sizeof(path), "%s%s", folder, name);

		CReadScript* lpReadScript = new CReadScript;

		if (!lpReadScript->Load(path))
		{
			delete lpReadScript;

			return 0;
		}

		LogAdd(LOG_BLUE, "[ContentCatalog] Fusionando %s", path);

		return lpReadScript;
	}
}

CContentCatalog::CContentCatalog()
{
	this->m_Built = false;
}

void CContentCatalog::LoadLegacyItems(const char* folder)
{
	this->m_ItemPose.clear();

	this->m_MapFog.clear();

	this->m_Built = false;

	// Un ala de CustomWing.txt sin columna Behavior se comporta como un ala de
	// segunda (12,3): así se equipa, acepta joyas y anima como ala, que es lo
	// que el DLL resolvía con chequeos de CustomWing repartidos por el cliente.
	for (int n = 0; n < MAX_CUSTOM_WING; n++)
	{
		CUSTOM_WING_INFO* wing = gCustomWing.GetInfo(n);

		ITEM_INFO* lpInfo = (wing != 0) ? gItemManager.GetInfoPtr(wing->ItemIndex) : 0;

		if (lpInfo != 0 && lpInfo->Behavior < 0)
		{
			lpInfo->Behavior = GET_ITEM(12, 3);
		}
	}

	CReadScript* lpReadScript;

	// CustomItem: Index(sec,sub) Skill "Modelo".  La columna de Item.txt manda.
	if ((lpReadScript = OpenLegacy(folder, "CustomItem.txt")) != 0)
	{
		try
		{
			while (true)
			{
				eTokenResult token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				int index = lpReadScript->GetNumber();

				index = SafeGetItem(GET_ITEM(index, lpReadScript->GetAsNumber()));

				lpReadScript->GetAsNumber();

				const char* model = lpReadScript->GetAsString();

				ITEM_INFO* lpInfo = gItemManager.GetInfoPtr(index);

				if (lpInfo != 0 && lpInfo->ModelName[0] == 0 && IsSafeModelPath(model))
				{
					LegacyItemFolder(index, lpInfo->ModelFolder, sizeof(lpInfo->ModelFolder));

					strcpy_s(lpInfo->ModelName, model);
				}
			}
		}
		catch (...)
		{
			ErrorMessageBox(lpReadScript->GetError());
		}

		delete lpReadScript;
	}

	// CustomGlow: Index(sec,sub) R G B.
	if ((lpReadScript = OpenLegacy(folder, "CustomGlow.txt")) != 0)
	{
		try
		{
			while (true)
			{
				eTokenResult token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				int index = lpReadScript->GetNumber();

				index = SafeGetItem(GET_ITEM(index, lpReadScript->GetAsNumber()));

				int r = lpReadScript->GetAsNumber();

				int g = lpReadScript->GetAsNumber();

				int b = lpReadScript->GetAsNumber();

				ITEM_INFO* lpInfo = gItemManager.GetInfoPtr(index);

				if (lpInfo != 0 && lpInfo->GlowColor < 0)
				{
					lpInfo->GlowColor = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
				}
			}
		}
		catch (...)
		{
			ErrorMessageBox(lpReadScript->GetError());
		}

		delete lpReadScript;
	}

	// CustomBow: Index(sub de la sección 4) Tipo(0 arco, 1 ballesta) SkillShot "Modelo".
	// El tipo pasa a ser el comportamiento: el arco o la ballesta vanilla.
	if ((lpReadScript = OpenLegacy(folder, "CustomBow.txt")) != 0)
	{
		try
		{
			while (true)
			{
				eTokenResult token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				int index = SafeGetItem(GET_ITEM(4, lpReadScript->GetNumber()));

				int type = lpReadScript->GetAsNumber();

				lpReadScript->GetAsNumber();

				const char* model = lpReadScript->GetAsString();

				ITEM_INFO* lpInfo = gItemManager.GetInfoPtr(index);

				if (lpInfo == 0)
				{
					continue;
				}

				if (lpInfo->Behavior < 0)
				{
					lpInfo->Behavior = (type == 0) ? GET_ITEM(4, 0) : GET_ITEM(4, 8);
				}

				if (lpInfo->ModelName[0] == 0 && IsSafeModelPath(model))
				{
					LegacyItemFolder(index, lpInfo->ModelFolder, sizeof(lpInfo->ModelFolder));

					strcpy_s(lpInfo->ModelName, model);
				}
			}
		}
		catch (...)
		{
			ErrorMessageBox(lpReadScript->GetError());
		}

		delete lpReadScript;
	}

	// ItemPosition: Index(sec,sub) X Y RotX RotY RotZ Scale (pose de inventario).
	if ((lpReadScript = OpenLegacy(folder, "ItemPosition.txt")) != 0)
	{
		try
		{
			while (true)
			{
				eTokenResult token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				int index = lpReadScript->GetNumber();

				index = SafeGetItem(GET_ITEM(index, lpReadScript->GetAsNumber()));

				ITEM_POSE_INFO pose;

				pose.Valid = true;

				pose.PositionX = lpReadScript->GetAsFloatNumber();

				pose.PositionY = lpReadScript->GetAsFloatNumber();

				pose.RotationX = lpReadScript->GetAsFloatNumber();

				pose.RotationY = lpReadScript->GetAsFloatNumber();

				pose.RotationZ = lpReadScript->GetAsFloatNumber();

				pose.Scale = lpReadScript->GetAsFloatNumber();

				if (index >= 0)
				{
					this->m_ItemPose[index] = pose;
				}
			}
		}
		catch (...)
		{
			ErrorMessageBox(lpReadScript->GetError());
		}

		delete lpReadScript;
	}

	// MapFog: Mapa DiaR DiaG DiaB NocheR NocheG NocheB.
	if ((lpReadScript = OpenLegacy(folder, "MapFog.txt")) != 0)
	{
		try
		{
			while (true)
			{
				eTokenResult token = lpReadScript->GetToken();

				if (token == TOKEN_END || token == TOKEN_END_SECTION)
				{
					break;
				}

				CATALOG_MAP_FOG fog;

				memset(&fog, 0, sizeof(fog));

				fog.Map = ClampByte(lpReadScript->GetNumber());

				for (int n = 0; n < 3; n++)
				{
					fog.DayRGB[n] = ClampByte(lpReadScript->GetAsNumber());
				}

				for (int n = 0; n < 3; n++)
				{
					fog.NightRGB[n] = ClampByte(lpReadScript->GetAsNumber());
				}

				this->m_MapFog.push_back(fog);
			}
		}
		catch (...)
		{
			ErrorMessageBox(lpReadScript->GetError());
		}

		delete lpReadScript;
	}
}

void CContentCatalog::LoadLegacyMonsters(const char* folder)
{
	this->m_Built = false;

	CReadScript* lpReadScript;

	// CustomMonster: Index Tipo(0 NPC, 1 monstruo) Golden Escala "Carpeta" "Modelo".
	if ((lpReadScript = OpenLegacy(folder, "CustomMonster.txt")) == 0)
	{
		return;
	}

	try
	{
		while (true)
		{
			eTokenResult token = lpReadScript->GetToken();

			if (token == TOKEN_END || token == TOKEN_END_SECTION)
			{
				break;
			}

			int index = lpReadScript->GetNumber();

			int kind = lpReadScript->GetAsNumber();

			int golden = lpReadScript->GetAsNumber();

			float scale = lpReadScript->GetAsFloatNumber();

			char modelFolder[64] = { 0 };

			strcpy_s(modelFolder, lpReadScript->GetAsString());

			const char* model = lpReadScript->GetAsString();

			MONSTER_INFO* lpInfo = gMonsterManager.GetInfo(index);

			if (lpInfo == 0 || lpInfo->ModelName[0] != 0 || !IsSafeModelPath(modelFolder) || !IsSafeModelPath(model))
			{
				continue;
			}

			lpInfo->Kind = kind;

			lpInfo->Golden = golden;

			lpInfo->Scale = scale;

			strcpy_s(lpInfo->ModelFolder, modelFolder);

			strcpy_s(lpInfo->ModelName, model);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

const ITEM_POSE_INFO* CContentCatalog::GetPose(int index) const
{
	std::map<int, ITEM_POSE_INFO>::const_iterator it = this->m_ItemPose.find(index);

	return (it == this->m_ItemPose.end()) ? 0 : &it->second;
}

void CContentCatalog::BuildSection(BYTE subh, const BYTE* records, int recordSize, int count, std::vector<std::vector<BYTE>>& packets)
{
	int perPacket = (CATALOG_MAX_PACKET_SIZE - (int)sizeof(PMSG_CATALOG_HEAD)) / recordSize;

	int totalChunks = (count + perPacket - 1) / perPacket;

	if (totalChunks == 0)
	{
		totalChunks = 1;
	}

	for (int chunk = 0; chunk < totalChunks; chunk++)
	{
		int first = chunk * perPacket;

		int amount = ((count - first) < perPacket) ? (count - first) : perPacket;

		if (amount < 0)
		{
			amount = 0;
		}

		int size = (int)sizeof(PMSG_CATALOG_HEAD) + (amount * recordSize);

		std::vector<BYTE> packet(size);

		PMSG_CATALOG_HEAD pMsg;

		pMsg.header.set(0xF3, subh, size);

		pMsg.version = CATALOG_VERSION;

		pMsg.recordSize = (WORD)recordSize;

		pMsg.count = (WORD)amount;

		pMsg.chunk = (WORD)chunk;

		pMsg.totalChunks = (WORD)totalChunks;

		memcpy(&packet[0], &pMsg, sizeof(pMsg));

		if (amount > 0)
		{
			memcpy(&packet[sizeof(pMsg)], records + (first * recordSize), amount * recordSize);
		}

		packets.push_back(packet);
	}
}

void CContentCatalog::Build()
{
	this->m_Packets.clear();

	std::vector<CATALOG_ITEM> items;

	const std::map<int, ITEM_INFO>& itemMap = gItemManager.GetInfoMap();

	for (std::map<int, ITEM_INFO>::const_iterator it = itemMap.begin(); it != itemMap.end(); it++)
	{
		const ITEM_INFO& info = it->second;

		if (info.Index < 0 || info.Index >= MAX_ITEM)
		{
			continue;
		}

		CATALOG_ITEM entry;

		memset(&entry, 0, sizeof(entry));

		entry.Index = (WORD)info.Index;

		entry.Behavior = (WORD)((info.Behavior >= 0) ? info.Behavior : info.Index);

		entry.Slot = (BYTE)info.Slot;

		entry.Skill = ClampByte(info.Skill);

		entry.Width = ClampByte(info.Width);

		entry.Height = ClampByte(info.Height);

		entry.TwoHand = ClampByte(info.TwoHand);

		entry.Durability = info.Durability;

		entry.MagicDurability = info.MagicDurability;

		CopyText(entry.Name, info.Name);

		entry.Level = ClampWord(info.Level);

		entry.DamageMin = ClampWord(info.DamageMin);

		entry.DamageMax = ClampWord(info.DamageMax);

		entry.AttackSpeed = ClampWord(info.AttackSpeed);

		entry.WalkSpeed = ClampWord(info.WalkSpeed);

		entry.Defense = ClampWord(info.Defense);

		entry.MagicDefense = ClampWord(info.MagicDefense);

		entry.DefenseSuccessRate = ClampWord(info.DefenseSuccessRate);

		entry.MagicDamageRate = ClampWord(info.MagicDamageRate);

		entry.RequireLevel = ClampWord(info.RequireLevel);

		entry.RequireStrength = ClampWord(info.RequireStrength);

		entry.RequireDexterity = ClampWord(info.RequireDexterity);

		entry.RequireEnergy = ClampWord(info.RequireEnergy);

		entry.RequireVitality = ClampWord(info.RequireVitality);

		for (int n = 0; n < 4; n++)
		{
			entry.RequireClass[n] = ClampByte(info.RequireClass[n]);

			entry.Resistance[n] = ClampByte(info.Resistance[n]);
		}

		entry.Value = (DWORD)info.Value;

		entry.BuyMoney = (DWORD)info.BuyMoney;

		if (info.ModelName[0] != 0)
		{
			entry.Flags |= CATALOG_ITEM_HAS_MODEL;

			CopyText(entry.ModelFolder, info.ModelFolder);

			CopyText(entry.ModelName, info.ModelName);
		}

		if (info.EntityName[0] != 0)
		{
			entry.Flags |= CATALOG_ITEM_HAS_ENTITY;

			CopyText(entry.EntityFolder, info.EntityFolder);

			CopyText(entry.EntityName, info.EntityName);
		}

		if (info.GlowColor >= 0)
		{
			entry.Flags |= CATALOG_ITEM_HAS_GLOW;

			entry.GlowColor = (DWORD)info.GlowColor;
		}

		CUSTOM_WING_INFO* wing = gCustomWing.GetInfoByItem(info.Index);

		if (wing != 0)
		{
			entry.CustomWing = (BYTE)(wing->Index + 1);

			entry.WingDefenseConstA = ClampWord(wing->DefenseConstA);

			entry.WingIncDamageConstA = ClampWord(wing->IncDamageConstA);

			entry.WingIncDamageConstB = ClampWord(wing->IncDamageConstB);

			entry.WingDecDamageConstA = ClampWord(wing->DecDamageConstA);

			entry.WingDecDamageConstB = ClampWord(wing->DecDamageConstB);
		}

		const ITEM_POSE_INFO* pose = this->GetPose(info.Index);

		if (pose != 0 && pose->Valid)
		{
			entry.Flags |= CATALOG_ITEM_HAS_POSE;

			entry.PositionX = pose->PositionX;

			entry.PositionY = pose->PositionY;

			entry.RotationX = pose->RotationX;

			entry.RotationY = pose->RotationY;

			entry.RotationZ = pose->RotationZ;

			entry.Scale = pose->Scale;
		}

		items.push_back(entry);
	}

	std::vector<CATALOG_MONSTER> monsters;

	for (int n = 0; n < MAX_MONSTER_INFO; n++)
	{
		MONSTER_INFO* lpInfo = gMonsterManager.GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		CATALOG_MONSTER entry;

		memset(&entry, 0, sizeof(entry));

		entry.Index = (WORD)lpInfo->Index;

		entry.Kind = (BYTE)((lpInfo->Kind < 0) ? 0xFF : lpInfo->Kind);

		entry.Golden = ClampByte(lpInfo->Golden);

		entry.Scale = lpInfo->Scale;

		entry.Level = ClampWord(lpInfo->Level);

		CopyText(entry.Name, lpInfo->Name);

		if (lpInfo->ModelName[0] != 0)
		{
			entry.Flags |= CATALOG_MONSTER_HAS_MODEL;

			CopyText(entry.ModelFolder, lpInfo->ModelFolder);

			CopyText(entry.ModelName, lpInfo->ModelName);
		}

		monsters.push_back(entry);
	}

	this->BuildSection(0xE7, items.empty() ? 0 : (const BYTE*)&items[0], sizeof(CATALOG_ITEM), (int)items.size(), this->m_Packets);

	this->BuildSection(0xE8, monsters.empty() ? 0 : (const BYTE*)&monsters[0], sizeof(CATALOG_MONSTER), (int)monsters.size(), this->m_Packets);

	this->BuildSection(0xE9, this->m_MapFog.empty() ? 0 : (const BYTE*)&this->m_MapFog[0], sizeof(CATALOG_MAP_FOG), (int)this->m_MapFog.size(), this->m_Packets);

	this->BuildSection(0xEC, this->m_Effects.empty() ? 0 : (const BYTE*)&this->m_Effects[0], sizeof(CATALOG_EFFECT), (int)this->m_Effects.size(), this->m_Packets);

	this->BuildSection(0xEF, this->m_Tooltips.empty() ? 0 : (const BYTE*)&this->m_Tooltips[0], sizeof(CATALOG_TOOLTIP), (int)this->m_Tooltips.size(), this->m_Packets);

	const std::vector<CATALOG_PET>& pets = gCustomPet.GetCatalog();

	this->BuildSection(0xEE, pets.empty() ? 0 : (const BYTE*)&pets[0], sizeof(CATALOG_PET), (int)pets.size(), this->m_Packets);

	DWORD hash = 2166136261u;

	for (size_t n = 0; n < this->m_Packets.size(); n++)
	{
		hash = Fnv1a(hash, &this->m_Packets[n][0], this->m_Packets[n].size());
	}

	PMSG_CATALOG_END_SEND pMsg;

	pMsg.header.set(0xF3, 0xEA, sizeof(pMsg));

	pMsg.itemCount = (WORD)items.size();

	pMsg.monsterCount = (WORD)monsters.size();

	pMsg.hash = hash;

	std::vector<BYTE> end(sizeof(pMsg));

	memcpy(&end[0], &pMsg, sizeof(pMsg));

	this->m_Packets.push_back(end);

	this->m_Built = true;

	LogAdd(LOG_BLUE, "[ContentCatalog] %d items, %d monstruos, %d fog, %d efectos, %d paquetes", (int)items.size(), (int)monsters.size(), (int)this->m_MapFog.size(), (int)this->m_Effects.size(), (int)this->m_Packets.size());
}

void CContentCatalog::Send(int aIndex)
{
	if (!this->m_Built)
	{
		this->Build();
	}

	for (size_t n = 0; n < this->m_Packets.size(); n++)
	{
		DataSend(aIndex, &this->m_Packets[n][0], (DWORD)this->m_Packets[n].size());
	}
}

namespace
{
	// "12,32", "12,032" o [12, 32] -> índice de item; -1 si no se entiende.
	int ParseItemKey(const JsonValue* value)
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

	bool ReadVector(const JsonValue& effect, const char* key, float* out, int size, std::string& error)
	{
		const JsonValue* value = effect.Get(key);

		if (value == nullptr)
		{
			return true;
		}

		if (!value->IsArray() || (int)value->items.size() != size)
		{
			error = std::string("'") + key + "' tiene que ser un array de " + std::to_string(size) + " números";

			return false;
		}

		for (int n = 0; n < size; n++)
		{
			if (!value->items[n].IsNumber())
			{
				error = std::string("'") + key + "' tiene que ser numérico";

				return false;
			}

			out[n] = (float)value->items[n].number;
		}

		return true;
	}

	bool ReadEffect(int item, const JsonValue& effect, CATALOG_EFFECT& out, std::string& error)
	{
		memset(&out, 0, sizeof(out));

		out.Item = (WORD)item;

		out.Bitmap = 0xFFFF;

		out.Scale = 1.0f;

		out.Color[0] = out.Color[1] = out.Color[2] = 1.0f;

		if (!effect.IsObject())
		{
			error = "cada efecto tiene que ser un objeto";

			return false;
		}

		std::string on = effect.GetString("on", "equipped");

		if (on == "equipped")
		{
			out.On = CATALOG_EFFECT_ON_EQUIPPED;
		}
		else
		{
			error = "'on' no soportado: " + on + " (por ahora sólo 'equipped')";

			return false;
		}

		std::string type = effect.GetString("type", "");

		if (type == "sprite")
		{
			out.Type = CATALOG_EFFECT_SPRITE;
		}
		else if (type == "particle")
		{
			out.Type = CATALOG_EFFECT_PARTICLE;

			double particle = effect.GetNumber("particle", -1);

			if (particle < 0 || particle > 0xFFFF)
			{
				error = "una partícula necesita 'particle' (tipo vanilla)";

				return false;
			}

			out.Particle = (WORD)particle;

			out.SubType = (BYTE)effect.GetNumber("subtype", 0);
		}
		else
		{
			error = "'type' tiene que ser 'sprite' o 'particle'";

			return false;
		}

		std::string texture = effect.GetString("texture", "");

		double bitmap = effect.GetNumber("bitmap", -1);

		if (!texture.empty())
		{
			if (texture.size() >= sizeof(out.Texture) || !IsSafeModelPath(texture.c_str()))
			{
				error = "ruta de textura inválida: " + texture;

				return false;
			}

			strcpy_s(out.Texture, texture.c_str());
		}
		else if (bitmap >= 0 && bitmap < 0xFFFF)
		{
			out.Bitmap = (WORD)bitmap;
		}
		else if (out.Type == CATALOG_EFFECT_SPRITE)
		{
			error = "un sprite necesita 'texture' o 'bitmap'";

			return false;
		}

		const JsonValue* bones = effect.Get("bones");

		if (bones == nullptr || !bones->IsArray() || bones->items.empty() || bones->items.size() > CATALOG_EFFECT_MAX_BONES)
		{
			error = "'bones' tiene que ser un array de 1 a " + std::to_string(CATALOG_EFFECT_MAX_BONES) + " huesos";

			return false;
		}

		for (size_t n = 0; n < bones->items.size(); n++)
		{
			if (!bones->items[n].IsNumber() || bones->items[n].number < 0 || bones->items[n].number > 199)
			{
				error = "hueso inválido en 'bones' (0..199)";

				return false;
			}

			out.Bones[n] = (BYTE)bones->items[n].number;
		}

		out.BoneCount = (BYTE)bones->items.size();

		double chance = effect.GetNumber("chance", 1);

		out.Chance = (BYTE)((chance < 1) ? 1 : ((chance > 255) ? 255 : chance));

		out.Scale = (float)effect.GetNumber("scale", 1.0);

		if (!ReadVector(effect, "color", out.Color, 3, error) || !ReadVector(effect, "offset", out.Offset, 3, error))
		{
			return false;
		}

		if (const JsonValue* pulse = effect.Get("pulse"))
		{
			if (!pulse->IsObject())
			{
				error = "'pulse' tiene que ser un objeto";

				return false;
			}

			out.PulseSpeed = (float)pulse->GetNumber("speed", 0.0);

			out.PulseScale = (float)pulse->GetNumber("scale", 0.0);

			out.PulseColor = (float)pulse->GetNumber("color", 0.0);
		}

		return true;
	}
}

namespace
{
	// UTF-8 (el JSON) a Latin-1 (lo que dibuja el cliente).  Lo que no entra
	// en Latin-1 queda como '?'.
	bool Utf8ToLatin1(const std::string& in, char* out, size_t size)
	{
		size_t n = 0;

		for (size_t i = 0; i < in.size(); )
		{
			unsigned char c = (unsigned char)in[i];

			unsigned int code;

			if (c < 0x80)
			{
				code = c;

				i += 1;
			}
			else if ((c & 0xE0) == 0xC0 && i + 1 < in.size())
			{
				code = ((c & 0x1F) << 6) | ((unsigned char)in[i + 1] & 0x3F);

				i += 2;
			}
			else
			{
				code = '?';

				i += ((c & 0xF0) == 0xE0) ? 3 : (((c & 0xF8) == 0xF0) ? 4 : 1);
			}

			if (n + 1 >= size)
			{
				return false;
			}

			out[n++] = (char)((code < 256) ? code : '?');
		}

		out[n] = 0;

		return true;
	}

	bool ReadTooltip(int item, const JsonValue& lines, std::vector<CATALOG_TOOLTIP>& out, std::string& error)
	{
		static const char* colors[] = { "white", "blue", "red", "gold", "green", "darkred", "purple", "darkblue", "darkgold" };

		if (!lines.IsArray() || lines.items.size() > CATALOG_TOOLTIP_MAX_LINES)
		{
			error = "tiene que ser un array de hasta " + std::to_string(CATALOG_TOOLTIP_MAX_LINES) + " líneas";

			return false;
		}

		for (const JsonValue& line : lines.items)
		{
			CATALOG_TOOLTIP entry;

			memset(&entry, 0, sizeof(entry));

			entry.Item = (WORD)item;

			std::string text = line.IsString() ? line.text : line.GetString("text", "");

			std::string color = line.IsString() ? "white" : line.GetString("color", "white");

			if (text.empty() || !Utf8ToLatin1(text, entry.Text, sizeof(entry.Text)))
			{
				error = "cada línea necesita un texto de hasta " + std::to_string(sizeof(entry.Text) - 1) + " caracteres";

				return false;
			}

			entry.Color = 0xFF;

			for (int n = 0; n < 9; n++)
			{
				if (color == colors[n])
				{
					entry.Color = (BYTE)n;
				}
			}

			if (entry.Color == 0xFF)
			{
				error = "color desconocido: " + color + " (white, blue, red, gold, green, darkred, purple, darkblue, darkgold)";

				return false;
			}

			out.push_back(entry);
		}

		return true;
	}
}

void CContentCatalog::LoadItemJson(const char* folder)
{
	this->m_Effects.clear();

	this->m_Tooltips.clear();

	this->Invalidate();

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
			LogAdd(LOG_RED, "[ContentCatalog] %s: JSON inválido: %s", name.c_str(), error.c_str());

			continue;
		}

		const int item = ParseItemKey(root.Get("item"));

		if (item < 0 || gItemManager.GetInfoPtr(item) == 0)
		{
			LogAdd(LOG_RED, "[ContentCatalog] %s: 'item' no existe en Item.txt", name.c_str());

			continue;
		}

		std::vector<CATALOG_EFFECT> effects;

		bool valid = true;

		if (const JsonValue* list = root.Get("effects"))
		{
			if (!list->IsArray())
			{
				LogAdd(LOG_RED, "[ContentCatalog] %s: 'effects' tiene que ser un array", name.c_str());

				continue;
			}

			for (size_t n = 0; n < list->items.size(); n++)
			{
				CATALOG_EFFECT effect;

				if (!ReadEffect(item, list->items[n], effect, error))
				{
					LogAdd(LOG_RED, "[ContentCatalog] %s: efecto %d: %s", name.c_str(), (int)n, error.c_str());

					valid = false;

					break;
				}

				effects.push_back(effect);
			}
		}

		// Pose en el inventario: { "position": [x, y], "rotation": [x, y, z], "scale": s }.
		// Reemplaza la de ItemPosition.txt y vale también para un vanilla.
		ITEM_POSE_INFO pose = {};

		bool hasPose = false;

		if (valid)
		{
			if (const JsonValue* inventory = root.Get("inventory"))
			{
				float position[2] = { 0.0f, 0.0f };

				float rotation[3] = { 0.0f, 0.0f, 0.0f };

				if (!inventory->IsObject() || !ReadVector(*inventory, "position", position, 2, error) || !ReadVector(*inventory, "rotation", rotation, 3, error))
				{
					LogAdd(LOG_RED, "[ContentCatalog] %s: 'inventory': %s", name.c_str(), error.empty() ? "tiene que ser un objeto" : error.c_str());

					valid = false;
				}
				else
				{
					pose.Valid = true;

					pose.PositionX = position[0];

					pose.PositionY = position[1];

					pose.RotationX = rotation[0];

					pose.RotationY = rotation[1];

					pose.RotationZ = rotation[2];

					pose.Scale = (float)inventory->GetNumber("scale", 0.0);

					hasPose = true;
				}
			}
		}

		// Líneas propias del tooltip: "texto" o { "text": "...", "color": "gold" }.
		std::vector<CATALOG_TOOLTIP> tooltip;

		if (valid)
		{
			if (const JsonValue* lines = root.Get("tooltip"))
			{
				if (!ReadTooltip(item, *lines, tooltip, error))
				{
					LogAdd(LOG_RED, "[ContentCatalog] %s: 'tooltip': %s", name.c_str(), error.c_str());

					valid = false;
				}
			}
		}

		// Un archivo con errores no aplica nada: mejor sin efecto que a medias.
		if (!valid)
		{
			continue;
		}

		this->m_Tooltips.insert(this->m_Tooltips.end(), tooltip.begin(), tooltip.end());

		if (hasPose)
		{
			this->m_ItemPose[item] = pose;
		}

		this->m_Effects.insert(this->m_Effects.end(), effects.begin(), effects.end());

		LogAdd(LOG_BLUE, "[ContentCatalog] %s: item %d, %d efectos, %d líneas de tooltip%s", name.c_str(), item, (int)effects.size(), (int)tooltip.size(), hasPose ? ", pose de inventario" : "");
	}
}
