#include "MapZones.h"
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

static const char* const kFile = "Zones.toml";

static fs::path MapsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Maps";
}

// Carpetas de mapa por número; con `withFile`, sólo las que tienen Zones.toml.
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

bool MapZones_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> folders;

	std::string error;

	return !ListFolders(dataDir, true, folders, error) || !folders.empty();
}

std::string MapZones_Path(const std::string& dataDir, int map, const std::string& mapName)
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
// Zones.toml

// Lee una lista de `count` enteros entre min y max desde un nodo.
static bool ReadList(const toml::node* node, const char* key, size_t count, int minValue, int maxValue, int* values, std::string& error)
{
	const toml::array* array = node ? node->as_array() : nullptr;

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

static bool LoadRects(const toml::table& table, const char* key, const std::string& file, int map, std::vector<ZoneRect>& out, std::string& error)
{
	const toml::node* list = table.get(key);

	if (list == nullptr)
	{
		return true;
	}

	const toml::array* array = list->as_array();

	if (array == nullptr)
	{
		error = file + ":" + std::to_string(list->source().begin.line) + ": '" + key + "' es una lista de rectángulos [x, y, tx, ty]";

		return false;
	}

	for (const toml::node& item : *array)
	{
		ZoneRect r;

		r.Map = map;

		r.Source = file + ":" + std::to_string(item.source().begin.line);

		int v[4] = { 0, 0, 0, 0 };

		std::string why;

		if (!ReadList(&item, key, 4, 0, 255, v, why))
		{
			error = r.Source + ": cada elemento de '" + key + "' es [x, y, tx, ty] con valores entre 0 y 255";

			return false;
		}

		r.X = v[0];
		r.Y = v[1];
		r.TX = v[2];
		r.TY = v[3];

		out.push_back(r);
	}

	return true;
}

static bool LoadNpcMoves(const toml::table& table, const std::string& file, int map, std::vector<NpcMoveDef>& out, std::string& error)
{
	const toml::node* list = table.get("npc_move");

	if (list == nullptr)
	{
		return true;
	}

	const toml::array* array = list->as_array();

	if (array == nullptr)
	{
		error = file + ":" + std::to_string(list->source().begin.line) + ": 'npc_move' tiene que ser una lista";

		return false;
	}

	for (const toml::node& item : *array)
	{
		std::string where = file + ":" + std::to_string(item.source().begin.line);

		const toml::table* entry = item.as_table();

		if (entry == nullptr)
		{
			error = where + ": cada elemento de 'npc_move' es una tabla { npc = ..., at = [x, y], to = [mapa, x, y], ... }";

			return false;
		}

		for (auto&& [key, value] : *entry)
		{
			std::string_view k = key.str();

			bool known = k == "npc" || k == "at" || k == "to" || k == "level" || k == "reset" || k == "account" || k == "pk_move";

			if (!known)
			{
				error = where + ": campo desconocido '" + std::string(k) + "'";

				return false;
			}
		}

		NpcMoveDef d;

		d.Map = map;

		d.Source = where;

		std::string why;

		int at[2] = { 0, 0 }, to[3] = { 0, 0, 0 }, level[2] = { -1, -1 }, reset[2] = { -1, -1 };

		bool ok = true;

		if (!entry->contains("npc"))
		{
			why = "falta 'npc' (el número de Monster.txt)";

			ok = false;
		}

		ok = ok && ReadInt(*entry, "npc", 0, 65535, d.Npc, why) && ReadList(entry->get("at"), "at", 2, 0, 255, at, why) && ReadList(entry->get("to"), "to", 3, 0, 255, to, why) && ReadInt(*entry, "account", 0, 255, d.AccountLevel, why);

		ok = ok && (!entry->contains("level") || ReadList(entry->get("level"), "level", 2, -1, 1000, level, why)) && (!entry->contains("reset") || ReadList(entry->get("reset"), "reset", 2, -1, 1000000, reset, why));

		if (ok && entry->contains("pk_move"))
		{
			std::optional<bool> pk = (*entry)["pk_move"].value<bool>();

			if (!pk)
			{
				why = "'pk_move' tiene que ser true o false";

				ok = false;
			}
			else
			{
				d.PkMove = *pk ? 1 : 0;
			}
		}

		if (!ok)
		{
			error = where + ": " + why;

			return false;
		}

		d.X = at[0];
		d.Y = at[1];
		d.MoveMap = to[0];
		d.MoveX = to[1];
		d.MoveY = to[2];
		d.MinLevel = level[0];
		d.MaxLevel = level[1];
		d.MinReset = reset[0];
		d.MaxReset = reset[1];

		out.push_back(d);
	}

	return true;
}

bool MapZones_Load(const std::string& dataDir, MapZonesData& out, std::string& error)
{
	std::map<int, fs::path> folders;

	if (!ListFolders(dataDir, true, folders, error))
	{
		return false;
	}

	out = MapZonesData();

	for (const auto& [map, folder] : folders)
	{
		std::string file = (folder / kFile).u8string();

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
			std::string_view k = key.str();

			if (k != "safe" && k != "pk_free" && k != "npc_move")
			{
				error = file + ":" + std::to_string(value.source().begin.line) + ": sección desconocida '" + std::string(k) + "' (valen safe, pk_free y npc_move)";

				return false;
			}
		}

		if (!LoadRects(table, "safe", file, map, out.Safe, error) || !LoadRects(table, "pk_free", file, map, out.PkFree, error) || !LoadNpcMoves(table, file, map, out.NpcMove, error))
		{
			return false;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Los .txt de Data/Custom

// Lee las líneas de un .txt: tokens (con `*` = -1) y el comentario de `//`.
// Un archivo que no existe cuenta como vacío.
static bool ReadLegacyLines(const std::string& path, size_t columns, const char* header, std::vector<std::vector<int>>& rows, std::vector<std::string>& comments, std::vector<std::string>& sources, std::string& error)
{
	std::error_code ec;

	if (!fs::exists(fs::u8path(path), ec))
	{
		return true;
	}

	std::ifstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo abrir " + path;

		return false;
	}

	std::string raw;

	int line = 0;

	while (std::getline(file, raw))
	{
		line++;

		std::string where = path + ":" + std::to_string(line);

		std::string comment;

		size_t slash = raw.find("//");

		if (slash != std::string::npos)
		{
			comment = raw.substr(slash + 2);

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

		if (tokens.size() != columns)
		{
			error = where + ": se esperan " + std::to_string(columns) + " columnas (" + header + ")";

			return false;
		}

		std::vector<int> row;

		for (const std::string& t : tokens)
		{
			if (t == "*")
			{
				row.push_back(-1);

				continue;
			}

			char* end = nullptr;

			long v = std::strtol(t.c_str(), &end, 10);

			if (*end != '\0' || v < -1)
			{
				error = where + ": '" + t + "' no es un número válido (* o >= 0)";

				return false;
			}

			row.push_back((int)v);
		}

		size_t first = comment.find_first_not_of(" \t\r");

		size_t last = comment.find_last_not_of(" \t\r");

		rows.push_back(row);

		comments.push_back(first == std::string::npos ? std::string() : comment.substr(first, last - first + 1));

		sources.push_back(where);
	}

	return true;
}

static bool LoadLegacyRects(const std::string& path, std::vector<ZoneRect>& out, std::string& error)
{
	std::vector<std::vector<int>> rows;

	std::vector<std::string> comments, sources;

	if (!ReadLegacyLines(path, 5, "Map X Y TX TY", rows, comments, sources, error))
	{
		return false;
	}

	for (size_t n = 0; n < rows.size(); n++)
	{
		const std::vector<int>& v = rows[n];

		if (*std::max_element(v.begin(), v.end()) > 255 || *std::min_element(v.begin(), v.end()) < 0)
		{
			error = sources[n] + ": el mapa y las coordenadas van de 0 a 255";

			return false;
		}

		ZoneRect r;

		r.Map = v[0]; r.X = v[1]; r.Y = v[2]; r.TX = v[3]; r.TY = v[4];

		r.Comment = comments[n];

		r.Source = sources[n];

		out.push_back(r);
	}

	return true;
}

bool MapZones_LoadLegacy(const std::string& safeZone, const std::string& pkFree, const std::string& npcMove, MapZonesData& out, std::string& error)
{
	out = MapZonesData();

	if (!LoadLegacyRects(safeZone, out.Safe, error) || !LoadLegacyRects(pkFree, out.PkFree, error))
	{
		return false;
	}

	std::vector<std::vector<int>> rows;

	std::vector<std::string> comments, sources;

	if (!ReadLegacyLines(npcMove, 13, "Npc NpcMap NpcX NpcY MoveMap MoveX MoveY MinLvl MaxLvl MinRes MaxRes VIPLvl PKMove", rows, comments, sources, error))
	{
		return false;
	}

	for (size_t n = 0; n < rows.size(); n++)
	{
		const std::vector<int>& v = rows[n];

		NpcMoveDef d;

		d.Npc = v[0]; d.Map = v[1]; d.X = v[2]; d.Y = v[3];
		d.MoveMap = v[4]; d.MoveX = v[5]; d.MoveY = v[6];
		d.MinLevel = v[7]; d.MaxLevel = v[8]; d.MinReset = v[9]; d.MaxReset = v[10];
		d.AccountLevel = v[11]; d.PkMove = v[12];
		d.Comment = comments[n];
		d.Source = sources[n];

		bool ok = d.Npc >= 0 && d.Map >= 0 && d.Map <= 255 && d.MoveMap >= 0 && d.MoveMap <= 255;

		for (int c : { d.X, d.Y, d.MoveX, d.MoveY })
		{
			ok = ok && c >= 0 && c <= 255;
		}

		ok = ok && d.AccountLevel >= 0 && (d.PkMove == 0 || d.PkMove == 1);

		if (!ok)
		{
			error = sources[n] + ": valor fuera de rango (mapas y coordenadas de 0 a 255, VIPLvl >= 0, PKMove 0 o 1)";

			return false;
		}

		out.NpcMove.push_back(d);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Escribir y comparar

std::vector<int> MapZones_Maps(const MapZonesData& data)
{
	std::set<int> maps;

	for (const ZoneRect& r : data.Safe) maps.insert(r.Map);
	for (const ZoneRect& r : data.PkFree) maps.insert(r.Map);
	for (const NpcMoveDef& d : data.NpcMove) maps.insert(d.Map);

	return std::vector<int>(maps.begin(), maps.end());
}

static void WriteRects(std::ofstream& file, const char* key, int map, const std::vector<ZoneRect>& rects)
{
	bool first = true;

	for (const ZoneRect& r : rects)
	{
		if (r.Map != map)
		{
			continue;
		}

		if (first)
		{
			file << "\n" << key << " = [\n";

			first = false;
		}

		file << "  [" << r.X << ", " << r.Y << ", " << r.TX << ", " << r.TY << "],";

		if (!r.Comment.empty())
		{
			file << " # " << r.Comment;
		}

		file << "\n";
	}

	if (!first)
	{
		file << "]\n";
	}
}

bool MapZones_WriteToml(const std::string& path, int map, const std::string& mapName, const MapZonesData& data, std::string& error)
{
	std::error_code ec;

	fs::create_directories(fs::u8path(path).parent_path(), ec);

	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# " << mapName << " (mapa " << map << "): zonas.\n"
		<< "#\n"
		<< "# safe      [x, y, tx, ty] donde nadie recibe daño\n"
		<< "# pk_free   [x, y, tx, ty] donde matar no te vuelve PK\n"
		<< "# npc_move  hablar con el NPC `npc` parado en `at` = [x, y] lleva a\n"
		<< "#           `to` = [mapa, x, y]. Opcionales: level y reset = [mínimo,\n"
		<< "#           máximo] (-1 = sin límite), account (nivel de cuenta mínimo)\n"
		<< "#           y pk_move (si pueden usarlo los PK; sin la clave, no)\n";

	WriteRects(file, "safe", map, data.Safe);

	WriteRects(file, "pk_free", map, data.PkFree);

	bool first = true;

	for (const NpcMoveDef& d : data.NpcMove)
	{
		if (d.Map != map)
		{
			continue;
		}

		if (first)
		{
			file << "\nnpc_move = [\n";

			first = false;
		}

		file << "  { npc = " << d.Npc << ", at = [" << d.X << ", " << d.Y << "], to = [" << d.MoveMap << ", " << d.MoveX << ", " << d.MoveY << "]";

		if (d.MinLevel != -1 || d.MaxLevel != -1)
		{
			file << ", level = [" << d.MinLevel << ", " << d.MaxLevel << "]";
		}

		if (d.MinReset != -1 || d.MaxReset != -1)
		{
			file << ", reset = [" << d.MinReset << ", " << d.MaxReset << "]";
		}

		if (d.AccountLevel != 0)
		{
			file << ", account = " << d.AccountLevel;
		}

		if (d.PkMove != 0)
		{
			file << ", pk_move = true";
		}

		file << " },";

		if (!d.Comment.empty())
		{
			file << " # " << d.Comment;
		}

		file << "\n";
	}

	if (!first)
	{
		file << "]\n";
	}

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}

template <typename T, typename Key>
static bool SameList(const char* what, std::vector<T> a, std::vector<T> b, Key key, std::string& diff)
{
	auto less = [&](const T& x, const T& y) { return key(x) < key(y); };

	std::sort(a.begin(), a.end(), less);

	std::sort(b.begin(), b.end(), less);

	for (size_t n = 0; n < a.size() && n < b.size(); n++)
	{
		if (key(a[n]) != key(b[n]))
		{
			diff = std::string(what) + ": " + a[n].Source + " contra " + b[n].Source;

			return false;
		}
	}

	if (a.size() != b.size())
	{
		diff = std::string(what) + ": " + std::to_string(a.size()) + " contra " + std::to_string(b.size());

		return false;
	}

	return true;
}

bool MapZones_Equal(const MapZonesData& a, const MapZonesData& b, std::string& diff)
{
	auto rect = [](const ZoneRect& r) { return std::vector<int>{ r.Map, r.X, r.Y, r.TX, r.TY }; };

	auto move = [](const NpcMoveDef& d)
	{
		return std::vector<int>{ d.Npc, d.Map, d.X, d.Y, d.MoveMap, d.MoveX, d.MoveY, d.MinLevel, d.MaxLevel, d.MinReset, d.MaxReset, d.AccountLevel, d.PkMove };
	};

	return SameList("safe", a.Safe, b.Safe, rect, diff) && SameList("pk_free", a.PkFree, b.PkFree, rect, diff) && SameList("npc_move", a.NpcMove, b.NpcMove, move, diff);
}
