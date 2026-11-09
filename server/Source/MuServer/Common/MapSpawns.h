#pragma once

// Spawns de monstruos y NPCs por carpeta de mapa (Fase 5, sección 2).
//
// Hoy todos los spawns viven en Data/Monster/MonsterSetBase.txt, con el número
// de mapa como columna. El formato nuevo los parte por mapa:
//
//     Data/Maps/00-Lorencia/Spawns.toml
//     Data/Maps/02-Devias/Spawns.toml
//     ...
//
// El número de mapa sale del prefijo de la carpeta. Este módulo no depende del
// GameServer: lo usan el GameServer (CMonsterSetBase::LoadMaps) y la
// herramienta mu-data (importar desde el .txt y comparar las dos tablas).

#include <string>
#include <vector>

// Las cinco secciones de MonsterSetBase.txt, con el nombre que usa el TOML.
enum MapSpawnSection
{
	MAP_SPAWN_NPC = 0,        // npc:        fijo en un punto
	MAP_SPAWN_AREA = 1,       // area:       `count` repartidos en un rectángulo
	MAP_SPAWN_SPOT = 2,       // spot:       uno, a ±3 tiles del punto
	MAP_SPAWN_EVENT_AREA = 3, // event_area: rectángulo de evento, con `value`
	MAP_SPAWN_EVENT = 4,      // event:      punto fijo de evento
	MAP_SPAWN_SECTIONS = 5,
};

const char* MapSpawnSectionName(int section);

// Una línea de MonsterSetBase.txt, sin expandir `count` ni aplicar el azar
// (la dirección -1 y el ±3 de `spot` los resuelve CMonsterSetBase al cargar).
struct MapSpawnEntry
{
	int Section = 0;
	int Monster = 0;
	int Map = 0;
	int Dis = 0;
	int X = 0;
	int Y = 0;
	int TX = 0;
	int TY = 0;
	int Dir = -1;
	int Count = 1;
	int Value = 0;
	std::string Comment; // el nombre que trae el .txt como comentario
	std::string Source;  // archivo:línea, para los mensajes de error
};

// true si mapsDir tiene al menos una carpeta NN-Nombre con Spawns.toml.
bool MapSpawns_HasData(const std::string& mapsDir);

// Lee todos los Spawns.toml. El resultado queda en el orden de
// MonsterSetBase.txt: por sección, después por mapa, y dentro de cada mapa en el
// orden del archivo. Devuelve false con el motivo (archivo y línea) en `error`.
bool MapSpawns_Load(const std::string& mapsDir, std::vector<MapSpawnEntry>& out, std::string& error);

// Lee MonsterSetBase.txt con un parser propio (no el CReadScript del
// GameServer): sirve para importar y para comparar contra el TOML.
bool MapSpawns_LoadLegacy(const std::string& path, std::vector<MapSpawnEntry>& out, std::string& error);

// Escribe un Spawns.toml con las entradas de un mapa (en su orden).
bool MapSpawns_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<MapSpawnEntry>& entries, std::string& error);

// Nombre de carpeta para un mapa: "00-Lorencia", "04-LostTower".
std::string MapSpawns_FolderName(int map, const std::string& mapName);
