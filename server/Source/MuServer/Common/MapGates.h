#pragma once

// Gates en TOML (Fase 5, sección 2).
//
// Hoy viven en Data/Move/Gate.txt, todas juntas. El formato nuevo pone cada
// gate en la carpeta del mapa donde está:
//
//     Data/Maps/00-Lorencia/Gates.toml
//
// El número de cada gate se escribe a mano y no cambia: lo usan el cliente
// (Gate.bmd), Move.txt y el dead_gate de los Map.toml. Si existe algún
// Gates.toml, el GameServer los usa y deja de leer Gate.txt. Este módulo no
// depende del GameServer: lo usan CGate y mu-data.

#include <string>
#include <vector>

// Una línea de Gate.txt. -1 = "*" en el .txt.
struct GateDef
{
	int Index = 0;
	int Flag = 0;          // 0 = destino, 1 = entrada, 2 = salida (como en el cliente)
	int Map = 0;
	int X = 0;
	int Y = 0;
	int TX = 0;
	int TY = 0;
	int Target = 0;        // gate a la que lleva una entrada; 0 = ninguna
	int Dir = 0;
	int MinLevel = -1;
	int MaxLevel = -1;
	int MinReset = -1;
	int MaxReset = -1;
	int AccountLevel = 0;
	std::string Source;    // archivo:línea, para los mensajes de error
};

// true si alguna carpeta Maps/NN-Nombre tiene Gates.toml.
bool MapGates_HasData(const std::string& dataDir);

// Lee los Gates.toml de Maps/. El resultado queda ordenado por número de gate.
// Falla si un número se repite. Devuelve false con el motivo (archivo y línea)
// en `error`.
bool MapGates_Load(const std::string& dataDir, std::vector<GateDef>& out, std::string& error);

// Lee Gate.txt con un parser propio (no el CReadScript del GameServer): sirve
// para importar y para comparar contra el TOML.
bool MapGates_LoadLegacy(const std::string& path, std::vector<GateDef>& out, std::string& error);

// Ruta del Gates.toml de un mapa: la carpeta que ya exista para ese número o una
// nueva con MapSpawns_FolderName.
std::string MapGates_Path(const std::string& dataDir, int map, const std::string& mapName);

// Escribe el Gates.toml de un mapa con sus gates.
bool MapGates_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<GateDef>& gates, std::string& error);

// Compara dos listas (ordenadas por número); si difieren, deja en `diff` la
// primera diferencia.
bool MapGates_Equal(const std::vector<GateDef>& a, const std::vector<GateDef>& b, std::string& diff);

// Gates de entrada cuyo destino no existe, para avisar (el server las acepta,
// pero no llevan a ningún lado).
std::vector<std::string> MapGates_DanglingTargets(const std::vector<GateDef>& gates);
