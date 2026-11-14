#pragma once

#include "MapSpawns.h"

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

	// Un archivo de spawns (Data/Maps/NN-Nombre/Spawns.toml o el de las
	// invasiones) y las filas de la tabla que salieron de él.
	struct SpawnGroup
	{
		std::string File;
		std::vector<MapSpawnEntry> Entries;
		std::vector<int> RowStart; // filas de cada entrada (una por `count`)
		std::vector<int> RowCount;
	};

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

	// Una entrada de un Spawns.toml, con AddLine.
	void AddEntry(const MapSpawnEntry& e);

	// El grupo del archivo de `e`; con `create` lo agrega si no está.
	static int FindSpawnGroup(std::vector<SpawnGroup>& groups, const MapSpawnEntry& e, bool create);

	bool GetPosition(int index, short map, short* ox, short* oy);

	bool GetBoxPosition(int map, int x, int y, int tx, int ty, short* ox, short* oy);

	void SetBoxPosition(int index, int map, int x, int y, int tx, int ty);

	// Recarga en caliente desde Data/ (Fase 5): sólo cambian las filas de los
	// archivos de spawns que cambiaron. Las de los demás quedan tal cual,
	// incluido el azar que se resolvió al cargarlas, y se mueven de índice si
	// hace falta. Si el TOML tiene un error no toca nada y lo devuelve.
	//   remap[viejo] = índice nuevo, o -1 si el archivo de esa fila cambió.
	//   changed      = rangos [inicio, fin) de filas nuevas, para crearlas.
	bool ReloadMaps(const char* path, std::vector<int>& remap, std::vector<std::pair<int, int>>& changed, std::vector<std::string>& files, std::string& error);

public:

	MONSTER_SET_BASE_INFO m_MonsterSetBaseInfo[MAX_MSB_MONSTER];

	int m_count;

	// Vacío cuando la tabla salió de MonsterSetBase.txt.
	std::vector<SpawnGroup> m_Groups;
};

extern CMonsterSetBase gMonsterSetBase;
