#include "ItemDrops.h"
#include "MapSpawns.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <tuple>

#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

static const char* const kFile = "Drops.toml";

static const char* const kOptionNames[5] = { "level", "skill", "luck", "add", "exc" };

static int* OptionField(ItemDropDef& d, int n)
{
	int* fields[5] = { &d.LevelOption, &d.SkillOption, &d.LuckOption, &d.AddOption, &d.ExcOption };

	return fields[n];
}

static int OptionValue(const ItemDropDef& d, int n)
{
	const int values[5] = { d.LevelOption, d.SkillOption, d.LuckOption, d.AddOption, d.ExcOption };

	return values[n];
}

static fs::path MapsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Maps";
}

std::string ItemDrops_GlobalPath(const std::string& dataDir)
{
	return (fs::u8path(dataDir) / "Items" / kFile).u8string();
}

// Carpetas de mapa por número; con `withFile`, sólo las que tienen ese archivo.
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

bool ItemDrops_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> folders;

	std::string error;

	std::error_code ec;

	return fs::is_regular_file(fs::u8path(ItemDrops_GlobalPath(dataDir)), ec) || !ListFolders(dataDir, true, folders, error) || !folders.empty();
}

std::string ItemDrops_MapPath(const std::string& dataDir, int map, const std::string& mapName)
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
// Drops.toml

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

// Lee [a, b] con cada valor entre min y max.
static bool ReadPair(const toml::table& t, const char* key, bool required, int minValue, int maxValue, int& a, int& b, std::string& error)
{
	const toml::node* node = t.get(key);

	if (node == nullptr)
	{
		if (required)
		{
			error = std::string("falta '") + key + "'";
		}

		return !required;
	}

	const toml::array* array = node->as_array();

	std::optional<int64_t> x = (array && array->size() == 2) ? array->get(0)->value<int64_t>() : std::nullopt;

	std::optional<int64_t> y = (array && array->size() == 2) ? array->get(1)->value<int64_t>() : std::nullopt;

	if (!x || !y || *x < minValue || *x > maxValue || *y < minValue || *y > maxValue)
	{
		error = std::string("'") + key + "' es [a, b] con valores entre " + std::to_string(minValue) + " y " + std::to_string(maxValue);

		return false;
	}

	a = (int)*x;

	b = (int)*y;

	return true;
}

static bool LoadDropFile(const fs::path& path, int map, std::vector<ItemDropDef>& out, std::string& error)
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
		if (key.str() != "drop")
		{
			error = file + ":" + std::to_string(value.source().begin.line) + ": sección desconocida '" + std::string(key.str()) + "' (vale drop)";

			return false;
		}
	}

	const toml::node* list = table.get("drop");

	if (list == nullptr)
	{
		return true;
	}

	const toml::array* array = list->as_array();

	if (array == nullptr)
	{
		error = file + ":" + std::to_string(list->source().begin.line) + ": 'drop' tiene que ser una lista";

		return false;
	}

	for (const toml::node& item : *array)
	{
		std::string where = file + ":" + std::to_string(item.source().begin.line);

		const toml::table* entry = item.as_table();

		if (entry == nullptr)
		{
			error = where + ": cada elemento de 'drop' es una tabla { item = [sección, índice], ... }";

			return false;
		}

		for (auto&& [key, value] : *entry)
		{
			std::string_view k = key.str();

			bool known = k == "item" || k == "level" || k == "grade" || k == "options" || k == "monster" || k == "monster_level" || k == "rate";

			if (!known)
			{
				error = where + ": campo desconocido '" + std::string(k) + "'";

				return false;
			}
		}

		ItemDropDef d;

		d.Map = map;

		d.Source = where;

		std::string why;

		bool ok = ReadPair(*entry, "item", true, 0, 511, d.Section, d.Index, why) && ReadInt(*entry, "level", 0, 15, d.Level, why) && ReadInt(*entry, "grade", 0, 255, d.Grade, why) && ReadInt(*entry, "monster", 0, 65535, d.Monster, why) && ReadPair(*entry, "monster_level", false, -1, 1000, d.MonsterLevelMin, d.MonsterLevelMax, why);

		if (ok && d.Section > 15)
		{
			why = "la sección del item va de 0 a 15";

			ok = false;
		}

		if (ok)
		{
			if (!entry->contains("rate"))
			{
				why = "falta 'rate' (de 1.000.000; -1 = siempre)";

				ok = false;
			}
			else
			{
				ok = ReadInt(*entry, "rate", -1, 1000000, d.Rate, why);
			}
		}

		if (ok && entry->contains("options"))
		{
			const toml::table* options = entry->get_as<toml::table>("options");

			if (options == nullptr)
			{
				why = "'options' es una tabla { level = ..., skill = ..., luck = ..., add = ..., exc = ... }";

				ok = false;
			}

			for (auto it = (options != nullptr) ? options->begin() : entry->end(); ok && options != nullptr && it != options->end(); ++it)
			{
				std::string_view k = it->first.str();

				int n = 0;

				while (n < 5 && k != kOptionNames[n])
				{
					n++;
				}

				if (n == 5)
				{
					why = "opción desconocida '" + std::string(k) + "' (valen level, skill, luck, add y exc)";

					ok = false;
				}
				else
				{
					ok = ReadInt(*options, kOptionNames[n], 0, 255, *OptionField(d, n), why);
				}
			}
		}

		if (!ok)
		{
			error = where + ": " + why;

			return false;
		}

		out.push_back(d);
	}

	return true;
}

bool ItemDrops_Load(const std::string& dataDir, std::vector<ItemDropDef>& out, std::string& error)
{
	out.clear();

	std::error_code ec;

	fs::path global = fs::u8path(ItemDrops_GlobalPath(dataDir));

	if (fs::is_regular_file(global, ec) && !LoadDropFile(global, -1, out, error))
	{
		return false;
	}

	std::map<int, fs::path> folders;

	if (!ListFolders(dataDir, true, folders, error))
	{
		return false;
	}

	for (const auto& [map, folder] : folders)
	{
		if (!LoadDropFile(folder / kFile, map, out, error))
		{
			return false;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// ItemDrop.txt

bool ItemDrops_LoadLegacy(const std::string& path, std::vector<ItemDropDef>& out, std::string& error)
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

		std::string comment;

		size_t slash = raw.find("//");

		if (slash != std::string::npos)
		{
			comment = raw.substr(slash + 2);

			raw.erase(slash);
		}

		// "12,015" es sección e índice: la coma separa igual que un espacio.
		std::replace(raw.begin(), raw.end(), ',', ' ');

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
			error = where + ": se esperan 14 columnas (Sección,Índice Level Grade Level Skill Luck Option Exe MapNum Monster MonsterLvlMin MonsterLvlMax Rate)";

			return false;
		}

		int values[14];

		for (int n = 0; n < 14; n++)
		{
			const std::string& t = tokens[n];

			if (t == "*")
			{
				values[n] = -1;

				continue;
			}

			char* end = nullptr;

			long v = std::strtol(t.c_str(), &end, 10);

			if (*end != '\0')
			{
				error = where + ": '" + t + "' no es un número";

				return false;
			}

			values[n] = (int)v;
		}

		ItemDropDef d;

		d.Section = values[0];
		d.Index = values[1];
		d.Level = values[2];
		d.Grade = values[3];
		d.LevelOption = values[4];
		d.SkillOption = values[5];
		d.LuckOption = values[6];
		d.AddOption = values[7];
		d.ExcOption = values[8];
		d.Map = values[9];
		d.Monster = values[10];
		d.MonsterLevelMin = values[11];
		d.MonsterLevelMax = values[12];
		d.Rate = values[13];
		d.Source = where;

		// El comentario se conserva (suele ser el nombre del item), sin espacios
		// a los costados.
		size_t first = comment.find_first_not_of(" \t\r");

		size_t last = comment.find_last_not_of(" \t\r");

		d.Comment = (first == std::string::npos) ? std::string() : comment.substr(first, last - first + 1);

		bool ok = d.Section >= 0 && d.Section <= 15 && d.Index >= 0 && d.Index <= 511;

		std::string why = "el item va de 0,0 a 15,511";

		if (ok && (d.Level < 0 || d.Level > 15))
		{
			ok = false;

			why = "el nivel del item va de 0 a 15";
		}
		else if (ok && (d.Grade < 0 || d.Grade > 255))
		{
			ok = false;

			why = "Grade va de 0 a 255";
		}
		else if (ok && (d.Map < -1 || d.Map > 255 || d.Monster < -1 || d.Rate < -1 || d.Rate > 1000000 || d.MonsterLevelMin < -1 || d.MonsterLevelMax < -1))
		{
			ok = false;

			why = "valor fuera de rango (MapNum 0-255, Rate -1 a 1000000, el resto * o >= 0)";
		}

		for (int n = 0; ok && n < 5; n++)
		{
			if (OptionValue(d, n) < -1 || OptionValue(d, n) > 255)
			{
				ok = false;

				why = "los índices de ItemOptionRate van de 0 a 255 (o *)";
			}
		}

		if (!ok)
		{
			error = where + ": " + why;

			return false;
		}

		out.push_back(d);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Escribir y comparar

static void WriteEntry(std::ofstream& file, const ItemDropDef& d)
{
	file << "  { item = [" << d.Section << ", " << d.Index << "]";

	if (d.Level != 0)
	{
		file << ", level = " << d.Level;
	}

	if (d.Grade != 0)
	{
		file << ", grade = " << d.Grade;
	}

	bool any = false;

	for (int n = 0; n < 5; n++)
	{
		if (OptionValue(d, n) >= 0)
		{
			file << (any ? ", " : ", options = { ") << kOptionNames[n] << " = " << OptionValue(d, n);

			any = true;
		}
	}

	if (any)
	{
		file << " }";
	}

	if (d.Monster >= 0)
	{
		file << ", monster = " << d.Monster;
	}

	if (d.MonsterLevelMin >= 0 || d.MonsterLevelMax >= 0)
	{
		file << ", monster_level = [" << d.MonsterLevelMin << ", " << d.MonsterLevelMax << "]";
	}

	file << ", rate = " << d.Rate << " },";

	if (!d.Comment.empty())
	{
		file << " # " << d.Comment;
	}

	file << "\n";
}

bool ItemDrops_WriteToml(const std::string& path, int map, const std::string& mapName, const std::vector<ItemDropDef>& drops, std::string& error)
{
	std::error_code ec;

	fs::create_directories(fs::u8path(path).parent_path(), ec);

	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	if (map < 0)
	{
		file << "# Drops especiales que caen en cualquier mapa. Los de un solo mapa van en\n"
			<< "# Maps/NN-Nombre/Drops.toml.\n";
	}
	else
	{
		file << "# " << mapName << " (mapa " << map << "): drops especiales que caen sólo en este mapa.\n";
	}

	file << "#\n"
		<< "# item           [sección, índice] del item (Item.txt)\n"
		<< "# level          nivel del item (+N); sin la clave, 0\n"
		<< "# grade          opciones excelentes fijas, si `exc` no da ninguna\n"
		<< "# options        índices de ItemOptionRate.txt para level, skill, luck,\n"
		<< "#                add y exc; sin la clave, esa opción no sale\n"
		<< "# monster        sólo ese monstruo (Monster.txt); sin la clave, cualquiera\n"
		<< "# monster_level  [mínimo, máximo] del nivel del monstruo; -1 = sin límite\n"
		<< "# rate           chance de 1.000.000 por monstruo muerto; -1 = siempre\n"
		<< "#\n"
		<< "# Se prueban antes que el drop normal del monstruo (un item al azar de\n"
		<< "# nivel cercano al suyo): si cae uno de estos, el monstruo no tira nada más.\n"
		<< "\ndrop = [\n";

	for (const ItemDropDef& d : drops)
	{
		WriteEntry(file, d);
	}

	file << "]\n";

	if (!file)
	{
		error = "error escribiendo " + path;

		return false;
	}

	return true;
}

static auto DropKey(const ItemDropDef& d)
{
	return std::make_tuple(d.Section, d.Index, d.Level, d.Grade, d.LevelOption, d.SkillOption, d.LuckOption, d.AddOption, d.ExcOption, d.Map, d.Monster, d.MonsterLevelMin, d.MonsterLevelMax, d.Rate);
}

bool ItemDrops_Equal(const std::vector<ItemDropDef>& a, const std::vector<ItemDropDef>& b, std::string& diff)
{
	std::vector<ItemDropDef> x = a, y = b;

	auto less = [](const ItemDropDef& p, const ItemDropDef& q) { return DropKey(p) < DropKey(q); };

	std::sort(x.begin(), x.end(), less);

	std::sort(y.begin(), y.end(), less);

	for (size_t i = 0; i < x.size() && i < y.size(); i++)
	{
		if (DropKey(x[i]) != DropKey(y[i]))
		{
			diff = "item " + std::to_string(x[i].Section) + "," + std::to_string(x[i].Index) + " (" + x[i].Source + ") contra " + std::to_string(y[i].Section) + "," + std::to_string(y[i].Index) + " (" + y[i].Source + ")";

			return false;
		}
	}

	if (x.size() != y.size())
	{
		diff = std::to_string(x.size()) + " drops contra " + std::to_string(y.size());

		return false;
	}

	return true;
}
