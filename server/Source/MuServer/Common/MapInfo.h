#pragma once

// Reglas de cada mapa en TOML (Fase 5, sección 2).
//
// Hoy viven en Data/MapManager.txt, una línea por mapa. El formato nuevo pone
// las de cada mapa en su carpeta, junto a sus spawns:
//
//     Data/Maps/00-Lorencia/Map.toml
//
// El número de mapa sale del prefijo de la carpeta. Si hay algún Map.toml, el
// GameServer los usa y deja de leer MapManager.txt. Este módulo no depende del
// GameServer: lo usan CMapManager y mu-data.

#include <string>
#include <vector>

// Una línea de MapManager.txt. -1 = "*" en el .txt: el valor por defecto del
// server (ver los Get* de CMapManager).
struct MapInfoDef
{
	int Index = 0;
	std::string Name;
	int NonPK = -1;           // 0 = se puede matar, 1 = sin PK, 2 = no entran outlaws
	int ViewRange = -1;
	int ExperienceRate = -1;
	int ItemDropRate = -1;
	int ExcItemDropRate = -1;
	int DeadGate = -1;
	int RespawnInPlace = 0;   // 0 o distinto de 0
	int FlyingDragons = 0;    // 0 o distinto de 0
	std::string Source;       // archivo (y línea), para los mensajes de error
};

// true si alguna carpeta Maps/NN-Nombre tiene Map.toml.
bool MapInfo_HasData(const std::string& dataDir);

// Lee los Map.toml de Maps/. El resultado queda ordenado por mapa. Devuelve
// false con el motivo (archivo y línea) en `error`.
bool MapInfo_Load(const std::string& dataDir, std::vector<MapInfoDef>& out, std::string& error);

// Lee MapManager.txt con un parser propio (no el CReadScript del GameServer):
// sirve para importar y para comparar contra el TOML.
bool MapInfo_LoadLegacy(const std::string& path, std::vector<MapInfoDef>& out, std::string& error);

// Ruta del Map.toml de un mapa: la carpeta que ya exista para ese número (tenga
// lo que tenga adentro) o una nueva con MapSpawns_FolderName.
std::string MapInfo_Path(const std::string& dataDir, int map, const std::string& name);

bool MapInfo_WriteToml(const std::string& path, const MapInfoDef& def, std::string& error);

// Compara dos listas; si difieren, deja en `diff` la primera diferencia.
bool MapInfo_Equal(const std::vector<MapInfoDef>& a, const std::vector<MapInfoDef>& b, std::string& diff);
