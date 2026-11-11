#pragma once

struct MAP_MANAGER_INFO
{
	int Index;
	int NonPK;
	int ViewRange;
	int ExperienceRate;
	int ItemDropRate;
	int ExcItemDropRate;
	int DeadGate;
	int RespawnInPlace;
	int FlyingDragons;
	char MapName[32];
};

class CMapManager
{
public:

	CMapManager();

	~CMapManager();

	void Load(const char* path);

	// Fase 5: las reglas de cada mapa desde Data/Maps/NN-Nombre/Map.toml
	// (`path` es Data/). Llena la misma tabla que Load con MapManager.txt.
	void LoadMaps(const char* path);

	int GetMapNonPK(int index);

	int GetMapNonOutlaw(int index);

	int GetMapViewRange(int index);

	int GetMapExperienceRate(int index);

	int GetMapItemDropRate(int index);

	int GetMapExcItemDropRate(int index);

	int GetMapDeadGate(int index);

	int GetMapRespawnInPlace(int index);

	int GetMapFlyingDragons(int index);

	const char* GetMapName(int index);

public:

	std::map<int, MAP_MANAGER_INFO> m_MapManagerInfo;
};

extern CMapManager gMapManager;
