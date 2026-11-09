#pragma once

#define MAX_MSB_MONSTER 10000

struct MONSTER_SET_BASE_INFO
{
	int Type;
	int MonsterClass;
	int Map;
	int Dis;
	int X;
	int Y;
	int Dir;
	int TX;
	int TY;
	int Value;
};

class CMonsterSetBase
{
public:

	CMonsterSetBase();

	~CMonsterSetBase();

	void Load(const char* path);

	// Spawns por carpeta de mapa (Data/Maps/NN-Nombre/Spawns.toml) y de las
	// invasiones (Data/Events/Invasions/Spawns.toml); `path` es Data/. Arma la
	// misma tabla que Load con MonsterSetBase.txt.
	bool LoadMaps(const char* path);

	// Una línea de MonsterSetBase.txt: aplica el azar de `spot` y de la
	// dirección, y repite `count` veces las secciones con rectángulo.
	void AddLine(MONSTER_SET_BASE_INFO info, int count);

	void SetInfo(MONSTER_SET_BASE_INFO info);

	bool GetPosition(int index, short map, short* ox, short* oy);

	bool GetBoxPosition(int map, int x, int y, int tx, int ty, short* ox, short* oy);

	void SetBoxPosition(int index, int map, int x, int y, int tx, int ty);

public:

	MONSTER_SET_BASE_INFO m_MonsterSetBaseInfo[MAX_MSB_MONSTER];

	int m_count;
};

extern CMonsterSetBase gMonsterSetBase;
