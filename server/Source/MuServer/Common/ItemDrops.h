#pragma once

// Drops especiales en TOML (Fase 5, sección 2).
//
// Hoy viven en Data/Item/ItemDrop.txt, con el mapa como columna. El formato
// nuevo los parte así:
//
//     Data/Items/Drops.toml              los que caen en cualquier mapa
//     Data/Maps/03-Noria/Drops.toml      los que caen sólo en ese mapa
//
// Si existe alguno, el GameServer los usa y deja de leer ItemDrop.txt. Este
// módulo no depende del GameServer: lo usan CItemDrop y mu-data.
//
// Estos drops se prueban antes que el drop normal de cada monstruo (un item al
// azar de nivel cercano al del monstruo, que sale de Item.txt): si cae uno, el
// monstruo no tira nada más.

#include <map>
#include <string>
#include <vector>

// Una línea de ItemDrop.txt. -1 = "*" en el .txt.
struct ItemDropDef
{
	int Section = 0;
	int Index = 0;
	int Level = 0;
	int Grade = 0;
	int LevelOption = -1;       // índices de ItemOptionRate.txt (-1 = ninguno)
	int SkillOption = -1;
	int LuckOption = -1;
	int AddOption = -1;
	int ExcOption = -1;
	int Map = -1;               // -1 = cualquier mapa
	int Monster = -1;           // -1 = cualquier monstruo
	int MonsterLevelMin = -1;
	int MonsterLevelMax = -1;
	int Rate = 0;               // de 1.000.000; -1 = siempre
	std::string Comment;
	std::string Source;         // archivo:línea, para los mensajes de error
};

// Rutas dentro del directorio de datos (Data/).
std::string ItemDrops_GlobalPath(const std::string& dataDir);

// true si existe Items/Drops.toml o algún Maps/NN-Nombre/Drops.toml.
bool ItemDrops_HasData(const std::string& dataDir);

// Lee Items/Drops.toml y los Drops.toml de los mapas (en ese orden). Devuelve
// false con el motivo (archivo y línea) en `error`.
bool ItemDrops_Load(const std::string& dataDir, std::vector<ItemDropDef>& out, std::string& error);

// Lee ItemDrop.txt con un parser propio (no el CReadScript del GameServer):
// sirve para importar y para comparar contra el TOML.
bool ItemDrops_LoadLegacy(const std::string& path, std::vector<ItemDropDef>& out, std::string& error);

// Ruta del Drops.toml de un mapa: la carpeta que ya exista para ese número o
// una nueva con MapSpawns_FolderName.
std::string ItemDrops_MapPath(const std::string& dataDir, int map, const std::string& mapName);

// Escribe un Drops.toml: `map` = -1 para el global (Items/Drops.toml), si no el
// del mapa (sin la columna de mapa).
bool ItemDrops_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<ItemDropDef>& drops, std::string& error);

// Compara dos listas sin importar el orden (el server elige al azar entre los
// que corresponden, así que el orden no cambia nada). Si difieren, deja en
// `diff` la primera diferencia.
bool ItemDrops_Equal(const std::vector<ItemDropDef>& a, const std::vector<ItemDropDef>& b, std::string& diff);
