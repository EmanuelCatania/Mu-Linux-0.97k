#pragma once

// Zonas de cada mapa en TOML (Fase 5, sección 2).
//
// Hoy viven en tres archivos de Data/Custom, con el mapa como columna:
//
//     CustomSafeZone.txt    rectángulos donde nadie recibe daño
//     CustomPkFree.txt      rectángulos donde matar no te vuelve PK
//     CustomNpcMove.txt     NPCs que llevan al personaje a otro lugar
//
// El formato nuevo junta las tres listas de un mapa en su carpeta:
//
//     Data/Maps/06-Arena/Zones.toml
//
// Si existe algún Zones.toml, el GameServer usa esos para las tres cosas y deja
// de leer los .txt. Este módulo no depende del GameServer: lo usan CSafeZone,
// CPKFree, CCustomNpcMove y mu-data.

#include <string>
#include <vector>

// Un rectángulo de CustomSafeZone.txt o CustomPkFree.txt.
struct ZoneRect
{
	int Map = 0;
	int X = 0;
	int Y = 0;
	int TX = 0;
	int TY = 0;
	std::string Comment;
	std::string Source;
};

// Una línea de CustomNpcMove.txt. -1 = "*" en el .txt.
struct NpcMoveDef
{
	int Npc = 0;
	int Map = 0;
	int X = 0;
	int Y = 0;
	int MoveMap = 0;
	int MoveX = 0;
	int MoveY = 0;
	int MinLevel = -1;
	int MaxLevel = -1;
	int MinReset = -1;
	int MaxReset = -1;
	int AccountLevel = 0;
	int PkMove = 0;
	std::string Comment;
	std::string Source;
};

struct MapZonesData
{
	std::vector<ZoneRect> Safe;
	std::vector<ZoneRect> PkFree;
	std::vector<NpcMoveDef> NpcMove;
};

// true si alguna carpeta Maps/NN-Nombre tiene Zones.toml.
bool MapZones_HasData(const std::string& dataDir);

// Lee los Zones.toml de Maps/, mapa por mapa y en el orden de cada archivo.
// Devuelve false con el motivo (archivo y línea) en `error`.
bool MapZones_Load(const std::string& dataDir, MapZonesData& out, std::string& error);

// Lee los tres .txt con un parser propio (no el CReadScript del GameServer):
// sirve para importar y para comparar contra el TOML. Un archivo que no existe
// cuenta como vacío.
bool MapZones_LoadLegacy(const std::string& safeZone, const std::string& pkFree, const std::string& npcMove, MapZonesData& out, std::string& error);

// Ruta del Zones.toml de un mapa: la carpeta que ya exista para ese número o una
// nueva con MapSpawns_FolderName.
std::string MapZones_Path(const std::string& dataDir, int map, const std::string& mapName);

// Escribe el Zones.toml de un mapa con lo que en `data` es de ese mapa.
bool MapZones_WriteToml(const std::string& path, int map, const std::string& mapName, const MapZonesData& data, std::string& error);

// Mapas que tienen algo en `data`, ordenados.
std::vector<int> MapZones_Maps(const MapZonesData& data);

// Compara las tres listas sin importar el orden entre mapas. Si difieren, deja
// en `diff` la primera diferencia.
bool MapZones_Equal(const MapZonesData& a, const MapZonesData& b, std::string& diff);
