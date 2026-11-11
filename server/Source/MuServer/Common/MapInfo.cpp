#include "MapInfo.h"
#include "MapSpawns.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>

#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

// Valores de `pk`, en el orden de NonPK (0, 1, 2).
static const char* const kPkNames[] = { "allowed", "disabled", "no_outlaws" };

static fs::path MapsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Maps";
}

// Carpetas de mapa por número; con `withFile`, sólo las que tienen ese archivo.
// Falla si dos carpetas dicen ser el mismo mapa.
static bool ListFolders(const std::string& dataDir, const char* withFile, std::map<int, fs::path>& out, std::string& error)
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

		if (map < 0 || (withFile != nullptr && !fs::is_regular_file(entry.path() / withFile, ec)))
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

bool MapInfo_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> folders;

	std::string error;

	// Con carpetas repetidas también hay datos: que MapInfo_Load lo reporte en
	// vez de caer en silencio al .txt.
	return !ListFolders(dataDir, "Map.toml", folders, error) || !folders.empty();
}

std::string MapInfo_Path(const std::string& dataDir, int map, const std::string& name)
{
	std::map<int, fs::path> folders;

	std::string error;

	if (ListFolders(dataDir, nullptr, folders, error) && folders.count(map) != 0)
	{
		return (folders[map] / "Map.toml").u8string();
	}

	return (MapsDir(dataDir) / fs::u8path(MapSpawns_FolderName(map, name)) / "Map.toml").u8string();
}

// ---------------------------------------------------------------------------
// Map.toml

static bool ReadInt(const toml::table& table, const char* key, int minValue, int maxValue, int& out, std::string& error)
{
	const toml::node* node = table.get(key);

	if (node == nullptr)
	{
		return true;
	}

	std::optional<int64_t> value = node->value<int64_t>();

	if (!value || *value < minValue || *value > maxValue)
	{
		error = std::to_string(node->source().begin.line) + ": '" + key + "' tiene que ser un entero entre " + std::to_string(minValue) + " y " + std::to_string(maxValue);

		return false;
	}

	out = (int)*value;

	return true;
}

static bool ReadBool(const toml::table& table, const char* key, int& out, std::string& error)
{
	const toml::node* node = table.get(key);

	if (node == nullptr)
	{
		return true;
	}

	std::optional<bool> value = node->value<bool>();

	if (!value)
	{
		error = std::to_string(node->source().begin.line) + ": '" + key + "' tiene que ser true o false";

		return false;
	}

	out = *value ? 1 : 0;

	return true;
}

static bool LoadMapFile(const fs::path& path, int map, MapInfoDef& def, std::string& error)
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

	def = MapInfoDef();

	def.Index = map;

	def.Source = file;

	for (auto&& [key, value] : table)
	{
		std::string_view k = key.str();

		bool known = k == "name" || k == "pk" || k == "view_range" || k == "experience_rate" || k == "item_drop_rate" || k == "exc_item_drop_rate" || k == "dead_gate" || k == "respawn_in_place" || k == "flying_dragons";

		if (!known)
		{
			error = file + ":" + std::to_string(value.source().begin.line) + ": campo desconocido '" + std::string(k) + "'";

			return false;
		}
	}

	std::optional<std::string> name = table["name"].value<std::string>();

	if (!name || name->empty() || name->size() >= 32)
	{
		error = file + ": falta 'name' (texto de 1 a 31 caracteres)";

		return false;
	}

	def.Name = *name;

	if (const toml::node* pk = table.get("pk"))
	{
		std::optional<std::string> value = pk->value<std::string>();

		def.NonPK = -2;

		for (int n = 0; value && n < 3; n++)
		{
			if (*value == kPkNames[n])
			{
				def.NonPK = n;
			}
		}

		if (def.NonPK == -2)
		{
			error = file + ":" + std::to_string(pk->source().begin.line) + ": 'pk' vale \"allowed\", \"disabled\" o \"no_outlaws\" (sin 'pk', lo que diga NonPK en la configuración del server)";

			return false;
		}
	}

	std::string why;

	bool ok = ReadInt(table, "view_range", 0, 255, def.ViewRange, why) && ReadInt(table, "experience_rate", 0, 1000000, def.ExperienceRate, why) && ReadInt(table, "item_drop_rate", 0, 1000000, def.ItemDropRate, why) && ReadInt(table, "exc_item_drop_rate", 0, 1000000, def.ExcItemDropRate, why) && ReadInt(table, "dead_gate", 0, 65535, def.DeadGate, why) && ReadBool(table, "respawn_in_place", def.RespawnInPlace, why) && ReadBool(table, "flying_dragons", def.FlyingDragons, why);

	if (!ok)
	{
		error = file + ":" + why;

		return false;
	}

	return true;
}

bool MapInfo_Load(const std::string& dataDir, std::vector<MapInfoDef>& out, std::string& error)
{
	std::map<int, fs::path> folders;

	if (!ListFolders(dataDir, "Map.toml", folders, error))
	{
		return false;
	}

	out.clear();

	for (const auto& [map, folder] : folders)
	{
		MapInfoDef def;

		if (!LoadMapFile(folder / "Map.toml", map, def, error))
		{
			return false;
		}

		out.push_back(def);
	}

	return true;
}

// ---------------------------------------------------------------------------
// MapManager.txt

bool MapInfo_LoadLegacy(const std::string& path, std::vector<MapInfoDef>& out, std::string& error)
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

		size_t comment = raw.find("//");

		if (comment != std::string::npos)
		{
			raw.erase(comment);
		}

		// Tokens separados por espacios; el nombre va entre comillas.
		std::vector<std::string> tokens;

		size_t pos = 0;

		while (pos < raw.size())
		{
			if (std::isspace((unsigned char)raw[pos]))
			{
				pos++;

				continue;
			}

			if (raw[pos] == '"')
			{
				size_t close = raw.find('"', pos + 1);

				if (close == std::string::npos)
				{
					error = where + ": comillas sin cerrar";

					return false;
				}

				tokens.push_back(raw.substr(pos + 1, close - pos - 1));

				pos = close + 1;

				continue;
			}

			size_t begin = pos;

			while (pos < raw.size() && !std::isspace((unsigned char)raw[pos]) && raw[pos] != '"')
			{
				pos++;
			}

			tokens.push_back(raw.substr(begin, pos - begin));
		}

		if (tokens.empty())
		{
			continue;
		}

		if (tokens[0] == "end")
		{
			break;
		}

		if (tokens.size() != 10)
		{
			error = where + ": se esperan 10 columnas (Index NonPK View ExperienceRate ItemDropRate ExcDropRate DeadGate SpawnInPlace FlyingDragons \"Nombre\")";

			return false;
		}

		int values[9];

		for (int n = 0; n < 9; n++)
		{
			const std::string& t = tokens[n];

			if (t == "*")
			{
				values[n] = -1;

				continue;
			}

			char* end = nullptr;

			long v = std::strtol(t.c_str(), &end, 10);

			if (t.empty() || *end != '\0')
			{
				error = where + ": '" + t + "' no es un número";

				return false;
			}

			if (v < -1)
			{
				error = where + ": '" + t + "' no puede ser negativo (* = el valor por defecto)";

				return false;
			}

			values[n] = (int)v;
		}

		MapInfoDef def;

		def.Index = values[0];
		def.NonPK = values[1];
		def.ViewRange = values[2];
		def.ExperienceRate = values[3];
		def.ItemDropRate = values[4];
		def.ExcItemDropRate = values[5];
		def.DeadGate = values[6];
		def.RespawnInPlace = values[7];
		def.FlyingDragons = values[8];
		def.Name = tokens[9];
		def.Source = where;

		if (def.Index < 0 || def.Index > 255)
		{
			error = where + ": el número de mapa va de 0 a 255";

			return false;
		}

		if (def.NonPK < -1 || def.NonPK > 2)
		{
			error = where + ": NonPK vale *, 0, 1 o 2";

			return false;
		}

		if (def.Name.empty() || def.Name.size() >= 32)
		{
			error = where + ": el nombre va de 1 a 31 caracteres";

			return false;
		}

		for (const MapInfoDef& other : out)
		{
			if (other.Index == def.Index)
			{
				error = where + ": el mapa " + std::to_string(def.Index) + " ya está en " + other.Source;

				return false;
			}
		}

		out.push_back(def);
	}

	std::sort(out.begin(), out.end(), [](const MapInfoDef& a, const MapInfoDef& b) { return a.Index < b.Index; });

	return true;
}

// ---------------------------------------------------------------------------
// Escribir y comparar

static std::string Quote(const std::string& text)
{
	std::string out = "\"";

	for (char c : text)
	{
		if (c == '"' || c == '\\')
		{
			out += '\\';
		}

		out += c;
	}

	return out + "\"";
}

bool MapInfo_WriteToml(const std::string& path, const MapInfoDef& def, std::string& error)
{
	std::error_code ec;

	fs::create_directories(fs::u8path(path).parent_path(), ec);

	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# " << def.Name << " (mapa " << def.Index << "): reglas del mapa.\n"
		<< "#\n"
		<< "# name                nombre del mapa en los avisos del server\n"
		<< "# pk                  \"allowed\", \"disabled\" o \"no_outlaws\"; sin pk, lo que\n"
		<< "#                     diga NonPK en la configuración del server\n"
		<< "# view_range          tiles que ve un jugador (sin la clave, 15)\n"
		<< "# experience_rate     % de experiencia (sin la clave, 100)\n"
		<< "# item_drop_rate      % de drop de items (sin la clave, 100)\n"
		<< "# exc_item_drop_rate  % de items excelentes (sin la clave, 0)\n"
		<< "# dead_gate           gate (Gate.txt) al que se va al morir\n"
		<< "# respawn_in_place    revivir donde se murió en vez de en dead_gate\n"
		<< "# flying_dragons      salen los dragones voladores del evento\n"
		<< "\n"
		<< "name = " << Quote(def.Name) << "\n";

	if (def.NonPK >= 0 && def.NonPK <= 2)
	{
		file << "pk = \"" << kPkNames[def.NonPK] << "\"\n";
	}

	const struct { const char* key; int value; } numbers[] =
	{
		{ "view_range", def.ViewRange },
		{ "experience_rate", def.ExperienceRate },
		{ "item_drop_rate", def.ItemDropRate },
		{ "exc_item_drop_rate", def.ExcItemDropRate },
		{ "dead_gate", def.DeadGate },
	};

	for (const auto& n : numbers)
	{
		if (n.value >= 0)
		{
			file << n.key << " = " << n.value << "\n";
		}
	}

	file << "respawn_in_place = " << (def.RespawnInPlace != 0 ? "true" : "false") << "\n"
		<< "flying_dragons = " << (def.FlyingDragons != 0 ? "true" : "false") << "\n";

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}

bool MapInfo_Equal(const std::vector<MapInfoDef>& a, const std::vector<MapInfoDef>& b, std::string& diff)
{
	if (a.size() != b.size())
	{
		diff = std::to_string(a.size()) + " mapas contra " + std::to_string(b.size());

		return false;
	}

	for (size_t i = 0; i < a.size(); i++)
	{
		const MapInfoDef& x = a[i];
		const MapInfoDef& y = b[i];

		std::string field;

		// Los valores sin sentido (-1 en un número) se escriben sin clave y se
		// leen como -1; los dos flags sólo importan como cero o distinto de cero.
		auto num = [](int v) { return v < 0 ? -1 : v; };

		if (x.Index != y.Index) field = "número de mapa";
		else if (x.Name != y.Name) field = "name";
		else if (x.NonPK != y.NonPK) field = "pk";
		else if (num(x.ViewRange) != num(y.ViewRange)) field = "view_range";
		else if (num(x.ExperienceRate) != num(y.ExperienceRate)) field = "experience_rate";
		else if (num(x.ItemDropRate) != num(y.ItemDropRate)) field = "item_drop_rate";
		else if (num(x.ExcItemDropRate) != num(y.ExcItemDropRate)) field = "exc_item_drop_rate";
		else if (num(x.DeadGate) != num(y.DeadGate)) field = "dead_gate";
		else if ((x.RespawnInPlace != 0) != (y.RespawnInPlace != 0)) field = "respawn_in_place";
		else if ((x.FlyingDragons != 0) != (y.FlyingDragons != 0)) field = "flying_dragons";

		if (!field.empty())
		{
			diff = "mapa " + std::to_string(x.Index) + " (" + x.Source + " / " + y.Source + "): " + field;

			return false;
		}
	}

	return true;
}
