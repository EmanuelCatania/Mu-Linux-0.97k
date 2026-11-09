#include "MapSpawns.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>

#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

static const char* const kSectionNames[MAP_SPAWN_SECTIONS] = { "npc", "area", "spot", "invasion", "event" };

const char* MapSpawnSectionName(int section)
{
	return (section >= 0 && section < MAP_SPAWN_SECTIONS) ? kSectionNames[section] : "?";
}

// Las secciones 1 y 3 son rectángulos con cantidad; el resto, un punto.
static bool SectionIsBox(int section)
{
	return section == MAP_SPAWN_AREA || section == MAP_SPAWN_EVENT_AREA;
}

std::string MapSpawns_FolderName(int map, const std::string& mapName)
{
	char prefix[16];

	std::snprintf(prefix, sizeof(prefix), "%02d-", map);

	std::string name = prefix;

	for (unsigned char c : mapName)
	{
		if (std::isalnum(c))
		{
			name += (char)c;
		}
	}

	return name;
}

// Número de mapa del prefijo "NN-" de la carpeta; -1 si no tiene.
static int FolderMapNumber(const std::string& folder)
{
	size_t dash = folder.find('-');

	if (dash == 0 || dash == std::string::npos)
	{
		return -1;
	}

	for (size_t n = 0; n < dash; n++)
	{
		if (!std::isdigit((unsigned char)folder[n]))
		{
			return -1;
		}
	}

	return std::atoi(folder.substr(0, dash).c_str());
}

// Carpetas de mapa con Spawns.toml, ordenadas por número. Falla si dos carpetas
// dicen ser el mismo mapa.
static bool ListMapFolders(const std::string& mapsDir, std::map<int, fs::path>& out, std::string& error)
{
	std::error_code ec;

	if (!fs::is_directory(fs::u8path(mapsDir), ec))
	{
		return true;
	}

	for (const fs::directory_entry& entry : fs::directory_iterator(fs::u8path(mapsDir), ec))
	{
		if (!entry.is_directory(ec))
		{
			continue;
		}

		std::string folder = entry.path().filename().u8string();

		int map = FolderMapNumber(folder);

		if (map < 0 || !fs::is_regular_file(entry.path() / "Spawns.toml", ec))
		{
			continue;
		}

		if (out.count(map) != 0)
		{
			error = "dos carpetas para el mapa " + std::to_string(map) + ": '" + out[map].filename().u8string() + "' y '" + folder + "'";

			return false;
		}

		out[map] = entry.path();
	}

	return true;
}

static fs::path MapsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Maps";
}

static fs::path InvasionsFile(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Events" / "Invasions" / "Spawns.toml";
}

std::string MapSpawns_InvasionsPath(const std::string& dataDir)
{
	return InvasionsFile(dataDir).u8string();
}

std::string MapSpawns_MapsPath(const std::string& dataDir)
{
	return MapsDir(dataDir).u8string();
}

bool MapSpawns_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> folders;

	std::string error;

	std::error_code ec;

	// Con carpetas repetidas también hay datos: que MapSpawns_Load lo reporte en
	// vez de caer en silencio al .txt.
	return !ListMapFolders(MapsDir(dataDir).u8string(), folders, error) || !folders.empty() || fs::is_regular_file(InvasionsFile(dataDir), ec);
}

// Lee un entero del entry; `required` hace que su ausencia sea error.
static bool ReadInt(const toml::table& entry, const char* key, bool required, int defaultValue, int minValue, int maxValue, int& out, std::string& error)
{
	const toml::node* node = entry.get(key);

	if (node == nullptr)
	{
		if (required)
		{
			error = std::string("falta '") + key + "'";

			return false;
		}

		out = defaultValue;

		return true;
	}

	std::optional<int64_t> value = node->value<int64_t>();

	if (!value || *value < minValue || *value > maxValue)
	{
		error = std::string("'") + key + "' tiene que ser un entero entre " + std::to_string(minValue) + " y " + std::to_string(maxValue);

		return false;
	}

	out = (int)*value;

	return true;
}

// Lee [x, y] o [x, y, tx, ty] en `values`.
static bool ReadCoords(const toml::table& entry, const char* key, size_t count, int* values, std::string& error)
{
	const toml::array* array = entry.get_as<toml::array>(key);

	if (array == nullptr || array->size() != count)
	{
		error = std::string("falta '") + key + "' con " + std::to_string(count) + " coordenadas";

		return false;
	}

	for (size_t n = 0; n < count; n++)
	{
		std::optional<int64_t> value = array->get(n)->value<int64_t>();

		if (!value || *value < 0 || *value > 255)
		{
			error = std::string("'") + key + "': las coordenadas van de 0 a 255";

			return false;
		}

		values[n] = (int)*value;
	}

	return true;
}

// Nombre de la lista en cada tipo de archivo. En las carpetas de mapa van las
// cuatro secciones atadas a un mapa; las invasiones (sección 3) cruzan mapas y
// viven aparte, en Events/Invasions/Spawns.toml, con el mapa en cada entrada.
static const char* ListName(int section, bool invasions)
{
	if (invasions)
	{
		return (section == MAP_SPAWN_EVENT_AREA) ? "invasion" : nullptr;
	}

	return (section == MAP_SPAWN_EVENT_AREA) ? nullptr : kSectionNames[section];
}

// `map` = número de la carpeta, o -1 para el archivo de invasiones (el mapa va
// en cada entrada).
static bool LoadSpawnFile(const fs::path& path, int map, std::vector<MapSpawnEntry> (&sections)[MAP_SPAWN_SECTIONS], std::string& error)
{
	std::string file = path.u8string();

	bool invasions = (map < 0);

	toml::parse_result result = toml::parse_file(file);

	if (!result)
	{
		const toml::parse_error& e = result.error();

		error = file + ":" + std::to_string(e.source().begin.line) + ": " + std::string(e.description());

		return false;
	}

	const toml::table& root = result.table();

	for (auto&& [key, node] : root)
	{
		bool known = false;

		for (int s = 0; s < MAP_SPAWN_SECTIONS; s++)
		{
			const char* name = ListName(s, invasions);

			known = known || (name != nullptr && key.str() == name);
		}

		if (!known)
		{
			std::string where = file + ":" + std::to_string(node.source().begin.line) + ": ";

			if (!invasions && key.str() == "event_area")
			{
				error = where + "los puntos de las invasiones cruzan mapas: van en Events/Invasions/Spawns.toml, con `map` en cada entrada";
			}
			else
			{
				error = where + "sección desconocida '" + std::string(key.str()) + (invasions ? "' (vale invasion)" : "' (valen npc, area, spot y event)");
			}

			return false;
		}
	}

	for (int s = 0; s < MAP_SPAWN_SECTIONS; s++)
	{
		const char* listName = ListName(s, invasions);

		if (listName == nullptr)
		{
			continue;
		}

		const toml::node* sectionNode = root.get(listName);

		if (sectionNode == nullptr)
		{
			continue;
		}

		const toml::array* array = sectionNode->as_array();

		if (array == nullptr)
		{
			error = file + ":" + std::to_string(sectionNode->source().begin.line) + ": '" + listName + "' tiene que ser una lista";

			return false;
		}

		for (const toml::node& item : *array)
		{
			const toml::table* entry = item.as_table();

			std::string where = file + ":" + std::to_string(item.source().begin.line);

			if (entry == nullptr)
			{
				error = where + ": cada elemento de '" + listName + "' es una tabla { monster = ..., ... }";

				return false;
			}

			bool box = SectionIsBox(s);

			for (auto&& [key, value] : *entry)
			{
				std::string_view k = key.str();

				bool allowed = k == "monster" || k == "dir" || k == "dis" || (box ? k == "box" || k == "count" : k == "at") || (s == MAP_SPAWN_EVENT_AREA && k == "value") || (invasions && k == "map");

				if (!allowed)
				{
					error = where + ": campo desconocido '" + std::string(k) + "' en '" + listName + "'";

					return false;
				}
			}

			MapSpawnEntry spawn;

			spawn.Section = s;

			spawn.Map = map;

			spawn.Source = where;

			int coords[4] = { 0, 0, 0, 0 };

			std::string why;

			bool ok = (!invasions || ReadInt(*entry, "map", true, 0, 0, 255, spawn.Map, why)) && ReadInt(*entry, "monster", true, 0, 0, 65535, spawn.Monster, why) && ReadInt(*entry, "dir", false, -1, -1, 7, spawn.Dir, why) && ReadInt(*entry, "dis", false, 0, 0, 255, spawn.Dis, why) && ReadCoords(*entry, box ? "box" : "at", box ? 4 : 2, coords, why);

			if (ok && box)
			{
				ok = ReadInt(*entry, "count", true, 0, 0, 10000, spawn.Count, why) && ReadInt(*entry, "value", false, 0, INT32_MIN, INT32_MAX, spawn.Value, why);
			}

			if (!ok)
			{
				error = where + ": " + why;

				return false;
			}

			spawn.X = coords[0];

			spawn.Y = coords[1];

			spawn.TX = coords[2];

			spawn.TY = coords[3];

			sections[s].push_back(spawn);
		}
	}

	return true;
}

bool MapSpawns_Load(const std::string& dataDir, std::vector<MapSpawnEntry>& out, std::string& error)
{
	std::map<int, fs::path> folders;

	if (!ListMapFolders(MapsDir(dataDir).u8string(), folders, error))
	{
		return false;
	}

	// Una lista por sección y, dentro, los mapas en orden: así el resultado
	// queda como en MonsterSetBase.txt (sección por sección, mapa por mapa).
	std::vector<MapSpawnEntry> sections[MAP_SPAWN_SECTIONS];

	for (const auto& [map, folder] : folders)
	{
		if (!LoadSpawnFile(folder / "Spawns.toml", map, sections, error))
		{
			return false;
		}
	}

	std::error_code ec;

	if (fs::is_regular_file(InvasionsFile(dataDir), ec))
	{
		if (!LoadSpawnFile(InvasionsFile(dataDir), -1, sections, error))
		{
			return false;
		}

		// En el archivo de invasiones el mapa es un campo: se ordena por mapa
		// conservando el orden del archivo dentro de cada uno.
		std::stable_sort(sections[MAP_SPAWN_EVENT_AREA].begin(), sections[MAP_SPAWN_EVENT_AREA].end(), [](const MapSpawnEntry& a, const MapSpawnEntry& b)
		{
			return a.Map < b.Map;
		});
	}

	out.clear();

	for (int s = 0; s < MAP_SPAWN_SECTIONS; s++)
	{
		out.insert(out.end(), sections[s].begin(), sections[s].end());
	}

	return true;
}

// ---------------------------------------------------------------------------
// MonsterSetBase.txt

namespace
{
	struct LegacyToken
	{
		std::string Text;
		int Line;
	};
}

static std::string Trim(const std::string& text)
{
	size_t begin = text.find_first_not_of(" \t\r\n");

	size_t end = text.find_last_not_of(" \t\r\n");

	return (begin == std::string::npos) ? std::string() : text.substr(begin, end - begin + 1);
}

bool MapSpawns_LoadLegacy(const std::string& path, std::vector<MapSpawnEntry>& out, std::string& error)
{
	std::ifstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo abrir " + path;

		return false;
	}

	std::vector<LegacyToken> tokens;

	std::map<int, std::string> comments;

	std::string line;

	for (int number = 1; std::getline(file, line); number++)
	{
		size_t comment = line.find("//");

		if (comment != std::string::npos)
		{
			comments[number] = Trim(line.substr(comment + 2));

			line.erase(comment);
		}

		std::istringstream words(line);

		std::string word;

		while (words >> word)
		{
			tokens.push_back({ word, number });
		}
	}

	size_t pos = 0;

	auto number = [&](int& value) -> bool
	{
		if (pos >= tokens.size())
		{
			error = path + ": el archivo termina en medio de una línea";

			return false;
		}

		const LegacyToken& token = tokens[pos++];

		char* end = nullptr;

		long parsed = std::strtol(token.Text.c_str(), &end, 10);

		if (end == token.Text.c_str() || *end != '\0')
		{
			error = path + ":" + std::to_string(token.Line) + ": se esperaba un número y hay '" + token.Text + "'";

			return false;
		}

		value = (int)parsed;

		return true;
	};

	out.clear();

	while (pos < tokens.size() && tokens[pos].Text != "end")
	{
		int section;

		if (!number(section))
		{
			return false;
		}

		if (section < 0 || section >= MAP_SPAWN_SECTIONS)
		{
			error = path + ":" + std::to_string(tokens[pos - 1].Line) + ": sección " + std::to_string(section) + " desconocida";

			return false;
		}

		while (pos < tokens.size() && tokens[pos].Text != "end")
		{
			MapSpawnEntry spawn;

			spawn.Section = section;

			int line = tokens[pos].Line;

			spawn.Source = path + ":" + std::to_string(line);

			spawn.Comment = comments.count(line) ? comments[line] : std::string();

			bool ok = number(spawn.Monster) && number(spawn.Map) && number(spawn.Dis) && number(spawn.X) && number(spawn.Y);

			if (ok && SectionIsBox(section))
			{
				ok = number(spawn.TX) && number(spawn.TY);
			}

			ok = ok && number(spawn.Dir);

			if (ok && SectionIsBox(section))
			{
				ok = number(spawn.Count) && (section != MAP_SPAWN_EVENT_AREA || number(spawn.Value));
			}

			if (!ok)
			{
				return false;
			}

			out.push_back(spawn);
		}

		pos++; // el "end" de la sección
	}

	return true;
}

// ---------------------------------------------------------------------------
// Escritura

// Una entrada como tabla en línea, con el nombre del monstruo como comentario.
static void WriteEntry(std::ofstream& file, const MapSpawnEntry& e, bool withMap, const std::string& mapName)
{
	file << "  { ";

	if (withMap)
	{
		file << "map = " << e.Map << ", ";
	}

	file << "monster = " << e.Monster;

	if (SectionIsBox(e.Section))
	{
		file << ", box = [" << e.X << ", " << e.Y << ", " << e.TX << ", " << e.TY << "], count = " << e.Count;

		if (e.Section == MAP_SPAWN_EVENT_AREA)
		{
			file << ", value = " << e.Value;
		}
	}
	else
	{
		file << ", at = [" << e.X << ", " << e.Y << "]";
	}

	if (e.Dir != -1)
	{
		file << ", dir = " << e.Dir;
	}

	if (e.Dis != 0)
	{
		file << ", dis = " << e.Dis;
	}

	file << " },";

	if (!e.Comment.empty() || !mapName.empty())
	{
		file << " #";

		if (!e.Comment.empty())
		{
			file << " " << e.Comment;
		}

		if (!mapName.empty())
		{
			file << (e.Comment.empty() ? " " : " - ") << mapName;
		}
	}

	file << "\n";
}

bool MapSpawns_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<MapSpawnEntry>& entries, std::string& error)
{
	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# " << mapName << " (mapa " << map << "): monstruos y NPCs.\n"
		<< "#\n"
		<< "# npc    fijo en `at` = [x, y]\n"
		<< "# area   `count` monstruos repartidos en `box` = [x, y, tx, ty]\n"
		<< "# spot   uno, a 3 tiles o menos de `at`\n"
		<< "# event  como npc, para los eventos del mapa (Blood Castle, Devil Square)\n"
		<< "#\n"
		<< "# Los puntos de las invasiones no van acá: cruzan mapas y viven en\n"
		<< "# Events/Invasions/Spawns.toml.\n"
		<< "#\n"
		<< "# dir: 0 a 7, o -1 (o sin dir) para una al azar. dis: distancia que se\n"
		<< "# mueve el monstruo desde su lugar (0 si no se mueve).\n";

	for (int s = 0; s < MAP_SPAWN_SECTIONS; s++)
	{
		if (s == MAP_SPAWN_EVENT_AREA)
		{
			continue;
		}

		bool first = true;

		for (const MapSpawnEntry& e : entries)
		{
			if (e.Section != s)
			{
				continue;
			}

			if (first)
			{
				file << "\n" << kSectionNames[s] << " = [\n";

				first = false;
			}

			WriteEntry(file, e, false, std::string());
		}

		if (!first)
		{
			file << "]\n";
		}
	}

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}

bool MapSpawns_WriteInvasions(const std::string& path, const std::vector<MapSpawnEntry>& entries, const std::map<int, std::string>& mapNames, std::string& error)
{
	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# Puntos de aparición de las invasiones (Death King, Red Dragon, dorados...).\n"
		<< "#\n"
		<< "# Cruzan mapas, así que no están en ninguna carpeta de Maps: cada entrada\n"
		<< "# dice en qué `map` aparece. Las invasiones en sí (horarios, jefes,\n"
		<< "# mensajes) siguen en Event/InvasionManager.dat; ahí, la tabla 2 elige por\n"
		<< "# mapa y por `value` qué puntos de esta lista usa cada una.\n"
		<< "#\n"
		<< "# `count` monstruos repartidos en `box` = [x, y, tx, ty]. dir: 0 a 7, o -1\n"
		<< "# (o sin dir) para una al azar. dis: distancia que se mueve el monstruo.\n"
		<< "\ninvasion = [\n";

	for (const MapSpawnEntry& e : entries)
	{
		if (e.Section != MAP_SPAWN_EVENT_AREA)
		{
			continue;
		}

		auto name = mapNames.find(e.Map);

		WriteEntry(file, e, true, (name != mapNames.end()) ? name->second : std::string());
	}

	file << "]\n";

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}
