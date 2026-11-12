// mu-data: herramientas para los datos del server (Fase 5, datos por mapa).
//
//   mu-data import-spawns <MonsterSetBase.txt> <MapManager.txt> <Data> [--force]
//       Parte MonsterSetBase.txt en un Spawns.toml por mapa
//       (<Data>/Maps/NN-Nombre/Spawns.toml) y saca las invasiones, que cruzan
//       mapas, a <Data>/Events/Invasions/Spawns.toml. Los nombres de carpeta
//       salen de MapManager.txt. Sin --force no pisa archivos existentes.
//
//   mu-data check-spawns <MonsterSetBase.txt> <Data>
//       Lee las dos fuentes y compara la tabla que arma cada una, entrada por
//       entrada, en orden de sección y mapa. Sale con 0 si son idénticas.
//
//   mu-data validate <Data>
//       Lee los Spawns.toml y avisa del primer error, con archivo y línea.

#include "MapSpawns.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path Utf8Path(const std::string& value)
{
#if defined(__cpp_char8_t)
	return fs::path(std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size()));
#else
	return fs::u8path(value);
#endif
}

static std::string PathToUtf8(const fs::path& path)
{
	auto value = path.u8string();
#if defined(__cpp_char8_t)
	return std::string(reinterpret_cast<const char*>(value.data()), value.size());
#else
	return value;
#endif
}

static int Fail(const std::string& message)
{
	std::fprintf(stderr, "mu-data: %s\n", message.c_str());

	return 1;
}

// Nombres de mapa de MapManager.txt: el índice es la primera columna y el
// nombre, el texto entre comillas.
static bool ReadMapNames(const std::string& path, std::map<int, std::string>& names, std::string& error)
{
	std::ifstream file(Utf8Path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo abrir " + path;

		return false;
	}

	std::string line;

	while (std::getline(file, line))
	{
		size_t comment = line.find("//");

		if (comment != std::string::npos)
		{
			line.erase(comment);
		}

		size_t open = line.find('"');

		size_t close = (open == std::string::npos) ? open : line.find('"', open + 1);

		int index;

		if (close == std::string::npos || std::sscanf(line.c_str(), "%d", &index) != 1)
		{
			continue;
		}

		names[index] = line.substr(open + 1, close - open - 1);
	}

	return true;
}

// Crea la carpeta de `file` y se niega a pisarlo sin --force.
static bool PrepareOutput(const fs::path& file, bool force, std::string& error)
{
	std::error_code ec;

	if (!force && fs::exists(file, ec))
	{
		error = PathToUtf8(file) + " ya existe (usar --force para pisarlo)";

		return false;
	}

	fs::create_directories(file.parent_path(), ec);

	if (ec)
	{
		error = "no se pudo crear " + PathToUtf8(file.parent_path()) + ": " + ec.message();

		return false;
	}

	return true;
}

static int ImportSpawns(const std::string& txt, const std::string& mapManager, const std::string& dataDir, bool force)
{
	std::string error;

	std::vector<MapSpawnEntry> entries;

	if (!MapSpawns_LoadLegacy(txt, entries, error))
	{
		return Fail(error);
	}

	std::map<int, std::string> names;

	if (!ReadMapNames(mapManager, names, error))
	{
		return Fail(error);
	}

	// Las invasiones (sección 3) van a su propio archivo; el resto, por mapa.
	std::map<int, std::vector<MapSpawnEntry>> byMap;

	size_t invasions = 0;

	for (const MapSpawnEntry& e : entries)
	{
		if (e.Section == MAP_SPAWN_EVENT_AREA)
		{
			invasions++;
		}
		else
		{
			byMap[e.Map].push_back(e);
		}
	}

	for (const auto& [map, list] : byMap)
	{
		std::string name = names.count(map) ? names[map] : ("Map" + std::to_string(map));

		fs::path file = Utf8Path(MapSpawns_MapsPath(dataDir)) / Utf8Path(MapSpawns_FolderName(map, name)) / "Spawns.toml";

		if (!PrepareOutput(file, force, error) || !MapSpawns_WriteToml(PathToUtf8(file), map, name, list, error))
		{
			return Fail(error);
		}

		std::printf("%-48s %4zu entradas\n", PathToUtf8(file).c_str(), list.size());
	}

	if (invasions > 0)
	{
		fs::path file = Utf8Path(MapSpawns_InvasionsPath(dataDir));

		if (!PrepareOutput(file, force, error) || !MapSpawns_WriteInvasions(PathToUtf8(file), entries, names, error))
		{
			return Fail(error);
		}

		std::printf("%-48s %4zu entradas\n", PathToUtf8(file).c_str(), invasions);
	}

	std::printf("%zu entradas: %zu en %zu mapas y %zu de invasiones\n", entries.size(), entries.size() - invasions, byMap.size(), invasions);

	return 0;
}

static std::string Describe(const MapSpawnEntry& e)
{
	std::ostringstream text;

	text << MapSpawnSectionName(e.Section) << " monster=" << e.Monster << " map=" << e.Map << " dis=" << e.Dis << " x=" << e.X << " y=" << e.Y << " tx=" << e.TX << " ty=" << e.TY << " dir=" << e.Dir << " count=" << e.Count << " value=" << e.Value << " (" << e.Source << ")";

	return text.str();
}

static bool Same(const MapSpawnEntry& a, const MapSpawnEntry& b)
{
	// TX/TY y Count sólo existen en las secciones con rectángulo.
	bool box = a.Section == MAP_SPAWN_AREA || a.Section == MAP_SPAWN_EVENT_AREA;

	return a.Section == b.Section && a.Monster == b.Monster && a.Map == b.Map && a.Dis == b.Dis && a.X == b.X && a.Y == b.Y && a.Dir == b.Dir && a.Value == b.Value && (!box || (a.TX == b.TX && a.TY == b.TY && a.Count == b.Count));
}

static int CheckSpawns(const std::string& txt, const std::string& dataDir)
{
	std::string error;

	std::vector<MapSpawnEntry> legacy, maps;

	if (!MapSpawns_LoadLegacy(txt, legacy, error) || !MapSpawns_Load(dataDir, maps, error))
	{
		return Fail(error);
	}

	// El .txt repite secciones (un bloque 0 por mapa, varios bloques 3 de
	// eventos) y no siempre va en orden de mapa; los TOML quedan por sección y
	// después por mapa. Se compara en ese orden, conservando el del archivo
	// dentro de cada mapa: cambia en qué posición de la tabla cae cada spawn,
	// no qué spawns hay ni en qué mapa.
	bool reordered = false;

	for (size_t n = 1; n < legacy.size(); n++)
	{
		const MapSpawnEntry& a = legacy[n - 1];

		const MapSpawnEntry& b = legacy[n];

		reordered = reordered || (b.Section < a.Section) || (b.Section == a.Section && b.Map < a.Map);
	}

	std::stable_sort(legacy.begin(), legacy.end(), [](const MapSpawnEntry& a, const MapSpawnEntry& b)
	{
		return (a.Section != b.Section) ? (a.Section < b.Section) : (a.Map < b.Map);
	});

	size_t count = (legacy.size() < maps.size()) ? legacy.size() : maps.size();

	for (size_t n = 0; n < count; n++)
	{
		if (!Same(legacy[n], maps[n]))
		{
			std::printf("Distintas en la entrada %zu:\n  txt:  %s\n  toml: %s\n", n, Describe(legacy[n]).c_str(), Describe(maps[n]).c_str());

			return 1;
		}
	}

	if (legacy.size() != maps.size())
	{
		std::printf("Distinta cantidad: %zu en el txt, %zu en los toml\n", legacy.size(), maps.size());

		return 1;
	}

	size_t monsters = 0;

	for (const MapSpawnEntry& e : maps)
	{
		monsters += (e.Section == MAP_SPAWN_AREA || e.Section == MAP_SPAWN_EVENT_AREA) ? e.Count : 1;
	}

	std::printf("Idénticas: %zu entradas (%zu monstruos y NPCs al expandir count)\n", maps.size(), monsters);

	if (reordered)
	{
		std::printf("El .txt no está ordenado por sección y mapa: con los TOML cambia la posición de algunos spawns en la tabla (no cuáles ni dónde).\n");
	}

	return 0;
}

static int Validate(const std::string& dataDir)
{
	std::string error;

	std::vector<MapSpawnEntry> entries;

	if (!MapSpawns_Load(dataDir, entries, error))
	{
		return Fail(error);
	}

	std::map<int, size_t> perMap;

	for (const MapSpawnEntry& e : entries)
	{
		perMap[e.Map]++;
	}

	for (const auto& [map, n] : perMap)
	{
		std::printf("mapa %2d: %4zu entradas\n", map, n);
	}

	std::printf("OK: %zu entradas en %zu mapas\n", entries.size(), perMap.size());

	return 0;
}

int main(int argc, char** argv)
{
	std::vector<std::string> args(argv + 1, argv + argc);

	bool force = false;

	for (size_t n = 0; n < args.size(); n++)
	{
		if (args[n] == "--force")
		{
			force = true;

			args.erase(args.begin() + n--);
		}
	}

	if (args.size() == 4 && args[0] == "import-spawns")
	{
		return ImportSpawns(args[1], args[2], args[3], force);
	}

	if (args.size() == 3 && args[0] == "check-spawns")
	{
		return CheckSpawns(args[1], args[2]);
	}

	if (args.size() == 2 && args[0] == "validate")
	{
		return Validate(args[1]);
	}

	std::fprintf(stderr,
		"uso:\n"
		"  mu-data import-spawns <MonsterSetBase.txt> <MapManager.txt> <Data> [--force]\n"
		"  mu-data check-spawns <MonsterSetBase.txt> <Data>\n"
		"  mu-data validate <Data>\n");

	return 2;
}
