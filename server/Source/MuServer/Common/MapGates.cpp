#include "MapGates.h"
#include "MapSpawns.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>

#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

static const char* const kFile = "Gates.toml";

// Valores de `type`, en el orden de Flag (0, 1, 2).
static const char* const kTypeNames[3] = { "destination", "entrance", "exit" };

static fs::path MapsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Maps";
}

// Carpetas de mapa por número; con `withFile`, sólo las que tienen Gates.toml.
// Falla si dos carpetas dicen ser el mismo mapa.
static bool ListFolders(const std::string& dataDir, bool withFile, std::map<int, fs::path>& out, std::string& error)
{
	std::error_code ec;

	if (!fs::is_directory(MapsDir(dataDir), ec))
	{
		return true;
	}

	for (const fs::directory_entry& entry : fs::directory_iterator(MapsDir(dataDir), ec))
	{
		if (!entry.is_directory(ec))
		{
			continue;
		}

		std::string folder = entry.path().filename().u8string();

		int map = MapSpawns_FolderMap(folder);

		if (map < 0 || (withFile && !fs::is_regular_file(entry.path() / kFile, ec)))
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

bool MapGates_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> folders;

	std::string error;

	return !ListFolders(dataDir, true, folders, error) || !folders.empty();
}

std::string MapGates_Path(const std::string& dataDir, int map, const std::string& mapName)
{
	std::map<int, fs::path> folders;

	std::string error;

	if (ListFolders(dataDir, false, folders, error) && folders.count(map) != 0)
	{
		return (folders[map] / kFile).u8string();
	}

	return (MapsDir(dataDir) / fs::u8path(MapSpawns_FolderName(map, mapName)) / kFile).u8string();
}

// ---------------------------------------------------------------------------
// Gates.toml

static bool ReadInt(const toml::table& t, const char* key, int minValue, int maxValue, int& out, std::string& error)
{
	const toml::node* node = t.get(key);

	if (node == nullptr)
	{
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

// Lee una lista de `count` enteros entre min y max.
static bool ReadList(const toml::table& t, const char* key, size_t count, int minValue, int maxValue, int* values, std::string& error)
{
	const toml::array* array = t.get_as<toml::array>(key);

	bool ok = array != nullptr && array->size() == count;

	for (size_t n = 0; ok && n < count; n++)
	{
		std::optional<int64_t> v = array->get(n)->value<int64_t>();

		ok = v && *v >= minValue && *v <= maxValue;

		if (ok)
		{
			values[n] = (int)*v;
		}
	}

	if (!ok)
	{
		error = std::string("'") + key + "' es una lista de " + std::to_string(count) + " enteros entre " + std::to_string(minValue) + " y " + std::to_string(maxValue);
	}

	return ok;
}

static bool LoadGateFile(const fs::path& path, int map, std::vector<GateDef>& out, std::string& error)
{
	std::string file = path.u8string();

	toml::parse_result result = toml::parse_file(file);

	if (!result)
	{
		const toml::parse_error& e = result.error();

		error = file + ":" + std::to_string(e.source().begin.line) + ": " + std::string(e.description());

		return false;
	}

	const toml::table& table = result.table();

	for (auto&& [key, value] : table)
	{
		if (key.str() != "gate")
		{
			error = file + ":" + std::to_string(value.source().begin.line) + ": sección desconocida '" + std::string(key.str()) + "' (vale gate)";

			return false;
		}
	}

	const toml::node* list = table.get("gate");

	if (list == nullptr)
	{
		return true;
	}

	const toml::array* array = list->as_array();

	if (array == nullptr)
	{
		error = file + ":" + std::to_string(list->source().begin.line) + ": 'gate' tiene que ser una lista";

		return false;
	}

	for (const toml::node& item : *array)
	{
		std::string where = file + ":" + std::to_string(item.source().begin.line);

		const toml::table* entry = item.as_table();

		if (entry == nullptr)
		{
			error = where + ": cada elemento de 'gate' es una tabla { index = ..., area = [x, y, tx, ty], ... }";

			return false;
		}

		for (auto&& [key, value] : *entry)
		{
			std::string_view k = key.str();

			bool known = k == "index" || k == "type" || k == "area" || k == "target" || k == "dir" || k == "level" || k == "reset" || k == "account";

			if (!known)
			{
				error = where + ": campo desconocido '" + std::string(k) + "'";

				return false;
			}
		}

		GateDef g;

		g.Map = map;

		g.Source = where;

		std::string why;

		int area[4] = { 0, 0, 0, 0 };

		int level[2] = { -1, -1 };

		int reset[2] = { -1, -1 };

		bool ok = true;

		if (!entry->contains("index"))
		{
			why = "falta 'index' (el número de la gate, el mismo del cliente)";

			ok = false;
		}

		ok = ok && ReadInt(*entry, "index", 0, 65535, g.Index, why) && ReadList(*entry, "area", 4, 0, 255, area, why) && ReadInt(*entry, "target", 0, 65535, g.Target, why) && ReadInt(*entry, "dir", -1, 255, g.Dir, why) && ReadInt(*entry, "account", 0, 255, g.AccountLevel, why);

		ok = ok && (!entry->contains("level") || ReadList(*entry, "level", 2, -1, 1000, level, why)) && (!entry->contains("reset") || ReadList(*entry, "reset", 2, -1, 1000000, reset, why));

		if (ok)
		{
			std::optional<std::string> type = (*entry)["type"].value<std::string>();

			g.Flag = -1;

			for (int n = 0; type && n < 3; n++)
			{
				if (*type == kTypeNames[n])
				{
					g.Flag = n;
				}
			}

			if (g.Flag < 0)
			{
				why = "falta 'type' (vale \"destination\", \"entrance\" o \"exit\")";

				ok = false;
			}
		}

		if (!ok)
		{
			error = where + ": " + why;

			return false;
		}

		g.X = area[0];
		g.Y = area[1];
		g.TX = area[2];
		g.TY = area[3];
		g.MinLevel = level[0];
		g.MaxLevel = level[1];
		g.MinReset = reset[0];
		g.MaxReset = reset[1];

		out.push_back(g);
	}

	return true;
}

static bool CheckUnique(std::vector<GateDef>& gates, std::string& error)
{
	std::stable_sort(gates.begin(), gates.end(), [](const GateDef& a, const GateDef& b) { return a.Index < b.Index; });

	for (size_t n = 1; n < gates.size(); n++)
	{
		if (gates[n].Index == gates[n - 1].Index)
		{
			error = gates[n].Source + ": la gate " + std::to_string(gates[n].Index) + " ya está en " + gates[n - 1].Source;

			return false;
		}
	}

	return true;
}

bool MapGates_Load(const std::string& dataDir, std::vector<GateDef>& out, std::string& error)
{
	std::map<int, fs::path> folders;

	if (!ListFolders(dataDir, true, folders, error))
	{
		return false;
	}

	out.clear();

	for (const auto& [map, folder] : folders)
	{
		if (!LoadGateFile(folder / kFile, map, out, error))
		{
			return false;
		}
	}

	return CheckUnique(out, error);
}

// ---------------------------------------------------------------------------
// Gate.txt

bool MapGates_LoadLegacy(const std::string& path, std::vector<GateDef>& out, std::string& error)
{
	std::ifstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo abrir " + path;

		return false;
	}

	out.clear();

	std::string raw;

	int line = 0;

	while (std::getline(file, raw))
	{
		line++;

		std::string where = path + ":" + std::to_string(line);

		size_t slash = raw.find("//");

		if (slash != std::string::npos)
		{
			raw.erase(slash);
		}

		std::istringstream in(raw);

		std::vector<std::string> tokens;

		std::string token;

		while (in >> token)
		{
			tokens.push_back(token);
		}

		if (tokens.empty())
		{
			continue;
		}

		if (tokens[0] == "end")
		{
			break;
		}

		if (tokens.size() != 14)
		{
			error = where + ": se esperan 14 columnas (Index Flag Map X Y TX TY Target Dir MinLvl MaxLvl MinRes MaxRes AccountLvl)";

			return false;
		}

		int v[14];

		for (int n = 0; n < 14; n++)
		{
			if (tokens[n] == "*")
			{
				v[n] = -1;

				continue;
			}

			char* end = nullptr;

			v[n] = (int)std::strtol(tokens[n].c_str(), &end, 10);

			if (*end != '\0')
			{
				error = where + ": '" + tokens[n] + "' no es un número";

				return false;
			}
		}

		GateDef g;

		g.Index = v[0]; g.Flag = v[1]; g.Map = v[2];
		g.X = v[3]; g.Y = v[4]; g.TX = v[5]; g.TY = v[6];
		g.Target = v[7]; g.Dir = v[8];
		g.MinLevel = v[9]; g.MaxLevel = v[10];
		g.MinReset = v[11]; g.MaxReset = v[12];
		g.AccountLevel = v[13];
		g.Source = where;

		std::string why;

		if (g.Index < 0 || g.Index > 65535) why = "el número de gate va de 0 a 65535";
		else if (g.Flag < 0 || g.Flag > 2) why = "Flag vale 0, 1 o 2";
		else if (g.Map < 0 || g.Map > 255) why = "el mapa va de 0 a 255";
		else if (g.X < 0 || g.X > 255 || g.Y < 0 || g.Y > 255 || g.TX < 0 || g.TX > 255 || g.TY < 0 || g.TY > 255) why = "las coordenadas van de 0 a 255";
		else if (g.Target < 0 || g.Dir < -1 || g.Dir > 255 || g.AccountLevel < 0) why = "Target >= 0, Dir de 0 a 255 y AccountLvl >= 0";
		else if (g.MinLevel < -1 || g.MaxLevel < -1 || g.MinReset < -1 || g.MaxReset < -1) why = "los niveles y resets van de 0 en adelante (o *)";

		if (!why.empty())
		{
			error = where + ": " + why;

			return false;
		}

		out.push_back(g);
	}

	return CheckUnique(out, error);
}

// ---------------------------------------------------------------------------
// Escribir y comparar

bool MapGates_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<GateDef>& gates, std::string& error)
{
	std::error_code ec;

	fs::create_directories(fs::u8path(path).parent_path(), ec);

	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# " << mapName << " (mapa " << map << "): gates.\n"
		<< "#\n"
		<< "# index    número de la gate. No se cambia: lo usan el cliente (Gate.bmd),\n"
		<< "#          Move.txt y el dead_gate de los Map.toml\n"
		<< "# type     \"entrance\" (pisarla lleva a `target`), \"exit\" (donde aparece\n"
		<< "#          quien llega) o \"destination\" (sólo destino: /move, revivir)\n"
		<< "# area     [x, y, tx, ty]\n"
		<< "# target   gate a la que lleva una entrada\n"
		<< "# dir      hacia dónde mira el personaje al llegar (0 a 7)\n"
		<< "# level    [mínimo, máximo] de nivel para usarla; -1 = sin límite\n"
		<< "# reset    [mínimo, máximo] de resets; sin la clave, sin límite\n"
		<< "# account  nivel de cuenta mínimo; sin la clave, 0\n"
		<< "\ngate = [\n";

	for (const GateDef& g : gates)
	{
		file << "  { index = " << g.Index << ", type = \"" << kTypeNames[g.Flag] << "\", area = [" << g.X << ", " << g.Y << ", " << g.TX << ", " << g.TY << "]";

		if (g.Target != 0)
		{
			file << ", target = " << g.Target;
		}

		if (g.Dir != 0)
		{
			file << ", dir = " << g.Dir;
		}

		if (g.MinLevel != -1 || g.MaxLevel != -1)
		{
			file << ", level = [" << g.MinLevel << ", " << g.MaxLevel << "]";
		}

		if (g.MinReset != -1 || g.MaxReset != -1)
		{
			file << ", reset = [" << g.MinReset << ", " << g.MaxReset << "]";
		}

		if (g.AccountLevel != 0)
		{
			file << ", account = " << g.AccountLevel;
		}

		file << " },\n";
	}

	file << "]\n";

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}

bool MapGates_Equal(const std::vector<GateDef>& a, const std::vector<GateDef>& b, std::string& diff)
{
	auto key = [](const GateDef& g)
	{
		return std::vector<int>{ g.Index, g.Flag, g.Map, g.X, g.Y, g.TX, g.TY, g.Target, g.Dir, g.MinLevel, g.MaxLevel, g.MinReset, g.MaxReset, g.AccountLevel };
	};

	for (size_t n = 0; n < a.size() && n < b.size(); n++)
	{
		if (key(a[n]) != key(b[n]))
		{
			diff = "gate " + std::to_string(a[n].Index) + " (" + a[n].Source + ") contra gate " + std::to_string(b[n].Index) + " (" + b[n].Source + ")";

			return false;
		}
	}

	if (a.size() != b.size())
	{
		diff = std::to_string(a.size()) + " gates contra " + std::to_string(b.size());

		return false;
	}

	return true;
}

std::vector<std::string> MapGates_DanglingTargets(const std::vector<GateDef>& gates)
{
	std::set<int> indexes;

	for (const GateDef& g : gates)
	{
		indexes.insert(g.Index);
	}

	std::vector<std::string> out;

	for (const GateDef& g : gates)
	{
		if (g.Target != 0 && indexes.count(g.Target) == 0)
		{
			out.push_back(g.Source + ": la gate " + std::to_string(g.Index) + " lleva a la " + std::to_string(g.Target) + ", que no existe");
		}
	}

	return out;
}
