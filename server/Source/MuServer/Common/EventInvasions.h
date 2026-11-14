#pragma once

// Invasiones en TOML (Fase 5, datos por evento).
//
// Hoy las invasiones viven en Data/Event/InvasionManager.dat, partidas en cuatro
// tablas que se cruzan por índice (horarios, definición, mapas por grupo y
// monstruos por grupo). El formato nuevo junta todo lo de una invasión:
//
//     Data/Events/Invasions/Invasions.toml   qué invasiones hay y cuándo salen
//     Data/Events/Invasions/Spawns.toml      dónde aparece cada monstruo (MapSpawns)
//
// Si existe Invasions.toml, el GameServer lo usa y deja de leer el .dat. Este
// módulo no depende del GameServer: lo usan CInvasionManager y mu-data.

#include <string>
#include <vector>

#define EVENT_INVASION_MAX 30          // MAX_INVASION del GameServer
#define EVENT_INVASION_MAX_GROUP 20    // MAX_INVASION_RESPAWN_GROUP

// Un horario. -1 = cualquiera ("*" en el .dat). DayOfWeek: 1 = domingo ... 7 = sábado.
struct InvasionSchedule
{
	int Year = -1;
	int Month = -1;
	int Day = -1;
	int DayOfWeek = -1;
	int Hour = -1;
	int Minute = -1;
	int Second = -1;
};

// Un mapa del grupo. `Value` elige qué puntos de Spawns.toml usa (los que tienen
// el mismo `value` en ese mapa).
struct InvasionMap
{
	int Map = 0;
	int Value = 0;
};

// Un monstruo del grupo. RegenType 0 = no reaparece durante la invasión; si no,
// reaparece RegenTime segundos después de morir.
struct InvasionMonster
{
	int Monster = 0;
	int RegenType = 0;
	int RegenTime = 0;
};

// Al empezar, de cada grupo se elige UN mapa al azar y salen en él todos sus
// monstruos.
struct InvasionGroup
{
	std::vector<InvasionMap> Maps;
	std::vector<InvasionMonster> Monsters;
};

// Los valores por defecto son los del GameServer para una invasión sin
// definición (tabla 1 del .dat).
struct InvasionDef
{
	int Index = 0;
	bool Defined = false; // tiene nombre, mensajes y duración (tabla 1)
	std::string Name;
	int RespawnMessage = -1;
	int DespawnMessage = -1;
	int BossIndex = -1;
	int BossMessage = -1;
	int InvasionTime = 0; // minutos
	int WarningTime = 0;  // minutos
	int WarningMessage = -1;
	int InvasionEffect = 0;
	std::vector<InvasionSchedule> Schedule;
	std::vector<InvasionGroup> Groups; // la posición es el número de grupo
	std::string Source;
};

// Data/Events/Invasions/Invasions.toml dentro del directorio de datos.
std::string EventInvasions_Path(const std::string& dataDir);

bool EventInvasions_HasData(const std::string& dataDir);

// Lee Invasions.toml. El resultado queda ordenado por índice. Devuelve false con
// el motivo (archivo y línea) en `error`.
bool EventInvasions_Load(const std::string& path, std::vector<InvasionDef>& out, std::string& error);

// Lee InvasionManager.dat con un parser propio (no el CReadScript del
// GameServer): sirve para importar y para comparar contra el TOML.
bool EventInvasions_LoadLegacy(const std::string& path, std::vector<InvasionDef>& out, std::string& error);

bool EventInvasions_WriteToml(const std::string& path, const std::vector<InvasionDef>& defs, std::string& error);

// Compara dos listas; si difieren, deja en `diff` la primera diferencia.
bool EventInvasions_Equal(const std::vector<InvasionDef>& a, const std::vector<InvasionDef>& b, std::string& diff);
