#include "EventInvasions.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>

// Igual que MapSpawns.cpp: sin excepciones, los errores vuelven en parse_result.
#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

std::string EventInvasions_Path(const std::string& dataDir)
{
	return (fs::u8path(dataDir) / "Events" / "Invasions" / "Invasions.toml").u8string();
}

bool EventInvasions_HasData(const std::string& dataDir)
{
	std::error_code ec;

	return fs::is_regular_file(fs::u8path(EventInvasions_Path(dataDir)), ec);
}

// ---------------------------------------------------------------------------
// TOML

static const toml::array kEmptyArray;

static std::string Where(const std::string& file, const toml::node& node)
{
	return file + ":" + std::to_string(node.source().begin.line) + ": ";
}

// Entero opcional de una tabla; sin la clave queda `out` como estaba.
static bool ReadInt(const toml::table& table, const char* key, int minValue, int maxValue, int& out, const std::string& where, std::string& error)
{
	const toml::node* node = table.get(key);

	if (node == nullptr)
	{
		return true;
	}

	std::optional<int64_t> value = node->value<int64_t>();

	if (!value || *value < minValue || *value > maxValue)
	{
		error = where + "'" + key + "' tiene que ser un entero entre " + std::to_string(minValue) + " y " + std::to_string(maxValue);

		return false;
	}

	out = (int)*value;

	return true;
}

static bool CheckKeys(const toml::table& table, std::initializer_list<const char*> allowed, const std::string& where, std::string& error)
{
	for (const auto& [key, node] : table)
	{
		bool ok = false;

		for (const char* name : allowed)
		{
			ok = ok || key.str() == name;
		}

		if (!ok)
		{
			error = where + "campo desconocido '" + std::string(key.str()) + "'";

			return false;
		}
	}

	return true;
}

// "HH:MM:SS" (o "HH:MM", segundo 0); cada parte puede ser "*" = cualquiera.
static bool ParseTime(const std::string& text, InvasionSchedule& s)
{
	int* parts[3] = { &s.Hour, &s.Minute, &s.Second };

	const int limits[3] = { 23, 59, 59 };

	std::vector<std::string> fields;

	std::stringstream stream(text);

	std::string field;

	while (std::getline(stream, field, ':'))
	{
		fields.push_back(field);
	}

	if (fields.size() < 2 || fields.size() > 3)
	{
		return false;
	}

	s.Second = 0;

	for (size_t n = 0; n < fields.size(); n++)
	{
		if (fields[n] == "*")
		{
			*parts[n] = -1;

			continue;
		}

		if (fields[n].empty() || fields[n].size() > 2)
		{
			return false;
		}

		for (char c : fields[n])
		{
			if (!std::isdigit((unsigned char)c))
			{
				return false;
			}
		}

		*parts[n] = std::atoi(fields[n].c_str());

		if (*parts[n] > limits[n])
		{
			return false;
		}
	}

	return true;
}

static std::string FormatTime(const InvasionSchedule& s)
{
	const int parts[3] = { s.Hour, s.Minute, s.Second };

	std::string text;

	for (int n = 0; n < 3; n++)
	{
		char buffer[8];

		std::snprintf(buffer, sizeof(buffer), (parts[n] < 0) ? "*" : "%02d", parts[n]);

		text += (n > 0) ? ":" : "";

		text += buffer;
	}

	return text;
}

static bool ReadSchedule(const toml::node& item, InvasionSchedule& s, const std::string& where, std::string& error)
{
	const char* bad = "cada horario es \"HH:MM:SS\" (con * para cualquiera) o { time = \"HH:MM:SS\", dow, day, month, year }";

	if (std::optional<std::string> text = item.value<std::string>())
	{
		if (!ParseTime(*text, s))
		{
			error = where + bad;

			return false;
		}

		return true;
	}

	const toml::table* table = item.as_table();

	const toml::node* timeNode = (table != nullptr) ? table->get("time") : nullptr;

	std::optional<std::string> time = (timeNode != nullptr) ? timeNode->value<std::string>() : std::nullopt;

	if (!time || !ParseTime(*time, s))
	{
		error = where + bad;

		return false;
	}

	return CheckKeys(*table, { "time", "dow", "day", "month", "year" }, where, error)
		&& ReadInt(*table, "dow", 1, 7, s.DayOfWeek, where, error)
		&& ReadInt(*table, "day", 1, 31, s.Day, where, error)
		&& ReadInt(*table, "month", 1, 12, s.Month, where, error)
		&& ReadInt(*table, "year", 2000, 2100, s.Year, where, error);
}

static bool ReadGroup(const toml::table& table, InvasionGroup& group, const std::string& file, std::string& error)
{
	std::string where = Where(file, table);

	if (!CheckKeys(table, { "maps", "monsters" }, where, error))
	{
		return false;
	}

	const toml::array* maps = table.get_as<toml::array>("maps");

	const toml::array* monsters = table.get_as<toml::array>("monsters");

	if ((table.get("maps") != nullptr && maps == nullptr) || (table.get("monsters") != nullptr && monsters == nullptr))
	{
		error = where + "'maps' y 'monsters' son listas";

		return false;
	}

	for (const toml::node& item : (maps != nullptr) ? *maps : kEmptyArray)
	{
		InvasionMap map;

		std::string at = Where(file, item);

		if (std::optional<int64_t> value = item.value<int64_t>())
		{
			map.Map = (int)*value;
		}
		else if (const toml::table* entry = item.as_table())
		{
			map.Map = -1;

			if (!CheckKeys(*entry, { "map", "value" }, at, error)
				|| !ReadInt(*entry, "map", 0, 255, map.Map, at, error)
				|| !ReadInt(*entry, "value", 0, 255, map.Value, at, error))
			{
				return false;
			}
		}
		else
		{
			map.Map = -1;
		}

		if (map.Map < 0 || map.Map > 255)
		{
			error = at + "cada mapa es un número (0 a 255) o { map = N, value = N }";

			return false;
		}

		group.Maps.push_back(map);
	}

	for (const toml::node& item : (monsters != nullptr) ? *monsters : kEmptyArray)
	{
		InvasionMonster monster;

		std::string at = Where(file, item);

		monster.Monster = -1;

		if (std::optional<int64_t> value = item.value<int64_t>())
		{
			monster.Monster = (int)*value;
		}
		else if (const toml::table* entry = item.as_table())
		{
			if (!CheckKeys(*entry, { "monster", "regen_type", "regen_time" }, at, error)
				|| !ReadInt(*entry, "monster", 0, 65535, monster.Monster, at, error)
				|| !ReadInt(*entry, "regen_type", 0, 255, monster.RegenType, at, error)
				|| !ReadInt(*entry, "regen_time", 0, 86400, monster.RegenTime, at, error))
			{
				return false;
			}
		}

		if (monster.Monster < 0 || monster.Monster > 65535)
		{
			error = at + "cada monstruo es un número o { monster = N, regen_type = N, regen_time = segundos }";

			return false;
		}

		group.Monsters.push_back(monster);
	}

	return true;
}

static bool ReadInvasion(const toml::table& table, InvasionDef& def, const std::string& file, std::string& error)
{
	std::string where = Where(file, table);

	def.Source = file + ":" + std::to_string(table.source().begin.line);

	if (!CheckKeys(table, { "index", "name", "schedule", "duration", "warning", "effect", "boss", "messages", "groups" }, where, error))
	{
		return false;
	}

	def.Index = -1;

	if (!ReadInt(table, "index", 0, EVENT_INVASION_MAX - 1, def.Index, where, error))
	{
		return false;
	}

	if (def.Index < 0)
	{
		error = where + "falta 'index' (0 a " + std::to_string(EVENT_INVASION_MAX - 1) + ")";

		return false;
	}

	if (const toml::node* name = table.get("name"))
	{
		std::optional<std::string> text = name->value<std::string>();

		if (!text || text->size() > 31)
		{
			error = where + "'name' es un texto de hasta 31 caracteres";

			return false;
		}

		def.Defined = true;

		def.Name = *text;
	}

	if (!ReadInt(table, "duration", 0, 1440, def.InvasionTime, where, error)
		|| !ReadInt(table, "warning", 0, 1440, def.WarningTime, where, error)
		|| !ReadInt(table, "effect", 0, 255, def.InvasionEffect, where, error)
		|| !ReadInt(table, "boss", -1, 65535, def.BossIndex, where, error))
	{
		return false;
	}

	if (const toml::node* node = table.get("messages"))
	{
		const toml::table* messages = node->as_table();

		std::string at = Where(file, *node);

		if (messages == nullptr)
		{
			error = at + "'messages' es { respawn, despawn, boss, warning }";

			return false;
		}

		if (!CheckKeys(*messages, { "respawn", "despawn", "boss", "warning" }, at, error)
			|| !ReadInt(*messages, "respawn", -1, 65535, def.RespawnMessage, at, error)
			|| !ReadInt(*messages, "despawn", -1, 65535, def.DespawnMessage, at, error)
			|| !ReadInt(*messages, "boss", -1, 65535, def.BossMessage, at, error)
			|| !ReadInt(*messages, "warning", -1, 65535, def.WarningMessage, at, error))
		{
			return false;
		}
	}

	if (const toml::node* node = table.get("schedule"))
	{
		const toml::array* schedule = node->as_array();

		if (schedule == nullptr)
		{
			error = Where(file, *node) + "'schedule' es una lista de horarios";

			return false;
		}

		for (const toml::node& item : *schedule)
		{
			InvasionSchedule s;

			if (!ReadSchedule(item, s, Where(file, item), error))
			{
				return false;
			}

			def.Schedule.push_back(s);
		}
	}

	if (const toml::node* node = table.get("groups"))
	{
		const toml::array* groups = node->as_array();

		if (groups == nullptr || groups->size() > EVENT_INVASION_MAX_GROUP)
		{
			error = Where(file, *node) + "'groups' es una lista de hasta " + std::to_string(EVENT_INVASION_MAX_GROUP) + " grupos";

			return false;
		}

		for (const toml::node& item : *groups)
		{
			const toml::table* group = item.as_table();

			if (group == nullptr)
			{
				error = Where(file, item) + "cada grupo es { maps = [...], monsters = [...] }";

				return false;
			}

			def.Groups.emplace_back();

			if (!ReadGroup(*group, def.Groups.back(), file, error))
			{
				return false;
			}
		}
	}

	return true;
}

bool EventInvasions_Load(const std::string& path, std::vector<InvasionDef>& out, std::string& error)
{
	out.clear();

	toml::parse_result result = toml::parse_file(path);

	if (!result)
	{
		const toml::parse_error& e = result.error();

		error = path + ":" + std::to_string(e.source().begin.line) + ": " + std::string(e.description());

		return false;
	}

	const toml::table& root = result.table();

	for (const auto& [key, node] : root)
	{
		if (key.str() != "invasion")
		{
			error = Where(path, node) + "sección desconocida '" + std::string(key.str()) + "' (vale [[invasion]])";

			return false;
		}
	}

	const toml::node* list = root.get("invasion");

	const toml::array* array = (list != nullptr) ? list->as_array() : nullptr;

	if (list != nullptr && array == nullptr)
	{
		error = Where(path, *list) + "'invasion' va como [[invasion]]";

		return false;
	}

	std::map<int, InvasionDef> byIndex;

	for (const toml::node& item : (array != nullptr) ? *array : kEmptyArray)
	{
		const toml::table* table = item.as_table();

		InvasionDef def;

		if (table == nullptr)
		{
			error = Where(path, item) + "cada invasión es una tabla [[invasion]]";

			return false;
		}

		if (!ReadInvasion(*table, def, path, error))
		{
			return false;
		}

		if (byIndex.count(def.Index) != 0)
		{
			error = def.Source + ": el índice " + std::to_string(def.Index) + " ya lo usa " + byIndex[def.Index].Source;

			return false;
		}

		byIndex[def.Index] = def;
	}

	for (auto& [index, def] : byIndex)
	{
		out.push_back(def);
	}

	return true;
}

// ---------------------------------------------------------------------------
// InvasionManager.dat

namespace
{
	struct DatToken
	{
		std::string Text;
		bool Quoted = false;
		int Line = 0;
	};
}

static bool Tokenize(const std::string& path, std::vector<DatToken>& tokens, std::string& error)
{
	std::ifstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo abrir " + path;

		return false;
	}

	std::string line;

	int number = 0;

	while (std::getline(file, line))
	{
		number++;

		size_t n = 0;

		while (n < line.size())
		{
			char c = line[n];

			if (std::isspace((unsigned char)c))
			{
				n++;
			}
			else if (c == '/' && n + 1 < line.size() && line[n + 1] == '/')
			{
				break;
			}
			else if (c == '"')
			{
				size_t close = line.find('"', n + 1);

				if (close == std::string::npos)
				{
					error = path + ":" + std::to_string(number) + ": comillas sin cerrar";

					return false;
				}

				tokens.push_back({ line.substr(n + 1, close - n - 1), true, number });

				n = close + 1;
			}
			else
			{
				size_t start = n;

				while (n < line.size() && !std::isspace((unsigned char)line[n]) && line[n] != '"')
				{
					n++;
				}

				tokens.push_back({ line.substr(start, n - start), false, number });
			}
		}
	}

	return true;
}

static bool DatNumber(const DatToken& token, int& out)
{
	if (token.Quoted || token.Text.empty())
	{
		return false;
	}

	if (token.Text == "*")
	{
		out = -1;

		return true;
	}

	char* end = nullptr;

	double value = std::strtod(token.Text.c_str(), &end);

	if (end == nullptr || *end != '\0')
	{
		return false;
	}

	out = (int)value;

	return true;
}

bool EventInvasions_LoadLegacy(const std::string& path, std::vector<InvasionDef>& out, std::string& error)
{
	out.clear();

	std::vector<DatToken> tokens;

	if (!Tokenize(path, tokens, error))
	{
		return false;
	}

	// Columnas de cada tabla, contando el índice.
	const size_t columns[4] = { 8, 10, 4, 5 };

	std::map<int, InvasionDef> byIndex;

	size_t pos = 0;

	auto fail = [&](size_t at, const std::string& text)
	{
		int line = (at < tokens.size()) ? tokens[at].Line : (tokens.empty() ? 0 : tokens.back().Line);

		error = path + ":" + std::to_string(line) + ": " + text;

		return false;
	};

	while (pos < tokens.size() && tokens[pos].Text != "end")
	{
		int section;

		if (!DatNumber(tokens[pos], section) || section < 0 || section > 3)
		{
			return fail(pos, "se esperaba el número de tabla (0 a 3)");
		}

		pos++;

		while (true)
		{
			if (pos >= tokens.size())
			{
				return fail(pos, "falta 'end' de la tabla " + std::to_string(section));
			}

			if (!tokens[pos].Quoted && tokens[pos].Text == "end")
			{
				pos++;

				break;
			}

			if (pos + columns[section] > tokens.size())
			{
				return fail(pos, "fila incompleta");
			}

			int v[10] = {};

			int line = tokens[pos].Line;

			for (size_t c = 0; c < columns[section]; c++)
			{
				bool name = (section == 1 && c == 9);

				if (name ? !tokens[pos + c].Quoted : !DatNumber(tokens[pos + c], v[c]))
				{
					return fail(pos + c, name ? "el nombre va entre comillas" : "se esperaba un número");
				}

				if (tokens[pos + c].Line != line)
				{
					return fail(pos + c, "fila incompleta");
				}
			}

			if (v[0] < 0 || v[0] >= EVENT_INVASION_MAX)
			{
				return fail(pos, "índice de invasión fuera de rango (0 a " + std::to_string(EVENT_INVASION_MAX - 1) + ")");
			}

			InvasionDef& def = byIndex[v[0]];

			if (def.Source.empty())
			{
				def.Index = v[0];

				def.Source = path + ":" + std::to_string(line);
			}

			if (section == 0)
			{
				InvasionSchedule s;

				s.Year = v[1];
				s.Month = v[2];
				s.Day = v[3];
				s.DayOfWeek = v[4];
				s.Hour = v[5];
				s.Minute = v[6];
				s.Second = v[7];

				def.Schedule.push_back(s);
			}
			else if (section == 1)
			{
				def.Defined = true;
				def.RespawnMessage = v[1];
				def.DespawnMessage = v[2];
				def.BossIndex = v[3];
				def.BossMessage = v[4];
				def.InvasionTime = v[5];
				def.WarningTime = v[6];
				def.WarningMessage = v[7];
				def.InvasionEffect = v[8];
				def.Name = tokens[pos + 9].Text;
			}
			else
			{
				int group = v[1];

				if (group < 0 || group >= EVENT_INVASION_MAX_GROUP)
				{
					return fail(pos + 1, "grupo fuera de rango (0 a " + std::to_string(EVENT_INVASION_MAX_GROUP - 1) + ")");
				}

				if ((int)def.Groups.size() <= group)
				{
					def.Groups.resize(group + 1);
				}

				if (section == 2)
				{
					def.Groups[group].Maps.push_back({ v[2], v[3] });
				}
				else
				{
					def.Groups[group].Monsters.push_back({ v[2], v[3], v[4] });
				}
			}

			pos += columns[section];
		}
	}

	for (auto& [index, def] : byIndex)
	{
		out.push_back(def);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Escritura

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

static std::string ScheduleText(const InvasionSchedule& s)
{
	std::string time = Quote(FormatTime(s));

	if (s.Year < 0 && s.Month < 0 && s.Day < 0 && s.DayOfWeek < 0)
	{
		return time;
	}

	std::string text = "{ time = " + time;

	text += (s.DayOfWeek >= 0) ? ", dow = " + std::to_string(s.DayOfWeek) : "";
	text += (s.Day >= 0) ? ", day = " + std::to_string(s.Day) : "";
	text += (s.Month >= 0) ? ", month = " + std::to_string(s.Month) : "";
	text += (s.Year >= 0) ? ", year = " + std::to_string(s.Year) : "";

	return text + " }";
}

bool EventInvasions_WriteToml(const std::string& path, const std::vector<InvasionDef>& defs, std::string& error)
{
	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# Invasiones: cuándo salen, cuánto duran y qué monstruos traen.\n"
		<< "#\n"
		<< "# index      número de la invasión (el de /invasionstart)\n"
		<< "# schedule   horarios \"HH:MM:SS\" (con * para cualquiera), o\n"
		<< "#            { time = \"HH:MM:SS\", dow, day, month, year } para fijar el día;\n"
		<< "#            dow: 1 = domingo ... 7 = sábado\n"
		<< "# duration   minutos que dura; warning: minutos de aviso antes de empezar\n"
		<< "# effect     1 = dragones rojos volando, 3 = dragones dorados\n"
		<< "# boss       monstruo jefe (su muerte manda el mensaje `boss`)\n"
		<< "# messages   números de mensaje de Message_*.txt (sin uno, no se avisa)\n"
		<< "# groups     al empezar, de cada grupo se elige un mapa al azar y salen ahí\n"
		<< "#            todos sus monstruos. Un mapa es un número o { map, value }:\n"
		<< "#            `value` elige qué puntos de Spawns.toml usa en ese mapa. Un\n"
		<< "#            monstruo es un número o { monster, regen_type, regen_time }:\n"
		<< "#            con regen_type 0 no reaparece hasta el final; si no, reaparece\n"
		<< "#            regen_time segundos después de morir.\n"
		<< "#\n"
		<< "# Dónde aparece cada monstruo está en Spawns.toml, en esta misma carpeta.\n";

	for (const InvasionDef& def : defs)
	{
		file << "\n[[invasion]]\nindex = " << def.Index << "\n";

		if (def.Defined)
		{
			file << "name = " << Quote(def.Name) << "\n";
		}

		if (!def.Schedule.empty())
		{
			file << "schedule = [";

			for (size_t n = 0; n < def.Schedule.size(); n++)
			{
				file << ((n > 0) ? ", " : "") << ScheduleText(def.Schedule[n]);
			}

			file << "]\n";
		}

		if (def.Defined)
		{
			file << "duration = " << def.InvasionTime << "\n";
		}

		if (def.WarningTime != 0)
		{
			file << "warning = " << def.WarningTime << "\n";
		}

		if (def.InvasionEffect != 0)
		{
			file << "effect = " << def.InvasionEffect << "\n";
		}

		if (def.BossIndex != -1)
		{
			file << "boss = " << def.BossIndex << "\n";
		}

		const std::pair<const char*, int> messages[4] = { { "respawn", def.RespawnMessage }, { "despawn", def.DespawnMessage }, { "boss", def.BossMessage }, { "warning", def.WarningMessage } };

		std::string list;

		for (const auto& [name, value] : messages)
		{
			if (value != -1)
			{
				list += (list.empty() ? "" : ", ") + std::string(name) + " = " + std::to_string(value);
			}
		}

		if (!list.empty())
		{
			file << "messages = { " << list << " }\n";
		}

		if (!def.Groups.empty())
		{
			file << "groups = [\n";

			for (const InvasionGroup& group : def.Groups)
			{
				file << "  { maps = [";

				for (size_t n = 0; n < group.Maps.size(); n++)
				{
					const InvasionMap& m = group.Maps[n];

					file << ((n > 0) ? ", " : "");

					if (m.Value == 0)
					{
						file << m.Map;
					}
					else
					{
						file << "{ map = " << m.Map << ", value = " << m.Value << " }";
					}
				}

				file << "], monsters = [";

				for (size_t n = 0; n < group.Monsters.size(); n++)
				{
					const InvasionMonster& m = group.Monsters[n];

					file << ((n > 0) ? ", " : "");

					if (m.RegenType == 0 && m.RegenTime == 0)
					{
						file << m.Monster;
					}
					else
					{
						file << "{ monster = " << m.Monster << ", regen_type = " << m.RegenType << ", regen_time = " << m.RegenTime << " }";
					}
				}

				file << "] },\n";
			}

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

// ---------------------------------------------------------------------------
// Comparación

bool EventInvasions_Equal(const std::vector<InvasionDef>& a, const std::vector<InvasionDef>& b, std::string& diff)
{
	if (a.size() != b.size())
	{
		diff = "distinta cantidad de invasiones: " + std::to_string(a.size()) + " y " + std::to_string(b.size());

		return false;
	}

	for (size_t i = 0; i < a.size(); i++)
	{
		const InvasionDef& x = a[i];

		const InvasionDef& y = b[i];

		std::string at = "invasión " + std::to_string(x.Index) + " (" + x.Source + " / " + y.Source + "): ";

		auto field = [&](const char* name, int p, int q)
		{
			if (p != q)
			{
				diff = at + name + " " + std::to_string(p) + " y " + std::to_string(q);
			}

			return p == q;
		};

		if (!field("index", x.Index, y.Index) || !field("definida", x.Defined, y.Defined)
			|| !field("respawn_message", x.RespawnMessage, y.RespawnMessage) || !field("despawn_message", x.DespawnMessage, y.DespawnMessage)
			|| !field("boss", x.BossIndex, y.BossIndex) || !field("boss_message", x.BossMessage, y.BossMessage)
			|| !field("duration", x.InvasionTime, y.InvasionTime) || !field("warning", x.WarningTime, y.WarningTime)
			|| !field("warning_message", x.WarningMessage, y.WarningMessage) || !field("effect", x.InvasionEffect, y.InvasionEffect)
			|| !field("horarios", (int)x.Schedule.size(), (int)y.Schedule.size()) || !field("grupos", (int)x.Groups.size(), (int)y.Groups.size()))
		{
			return false;
		}

		if (x.Name != y.Name)
		{
			diff = at + "nombre '" + x.Name + "' y '" + y.Name + "'";

			return false;
		}

		for (size_t n = 0; n < x.Schedule.size(); n++)
		{
			const InvasionSchedule& p = x.Schedule[n];

			const InvasionSchedule& q = y.Schedule[n];

			if (p.Year != q.Year || p.Month != q.Month || p.Day != q.Day || p.DayOfWeek != q.DayOfWeek || p.Hour != q.Hour || p.Minute != q.Minute || p.Second != q.Second)
			{
				diff = at + "horario " + std::to_string(n) + ": " + ScheduleText(p) + " y " + ScheduleText(q);

				return false;
			}
		}

		for (size_t g = 0; g < x.Groups.size(); g++)
		{
			const InvasionGroup& p = x.Groups[g];

			const InvasionGroup& q = y.Groups[g];

			std::string group = at + "grupo " + std::to_string(g) + ": ";

			bool same = p.Maps.size() == q.Maps.size() && p.Monsters.size() == q.Monsters.size();

			for (size_t n = 0; same && n < p.Maps.size(); n++)
			{
				same = p.Maps[n].Map == q.Maps[n].Map && p.Maps[n].Value == q.Maps[n].Value;
			}

			for (size_t n = 0; same && n < p.Monsters.size(); n++)
			{
				same = p.Monsters[n].Monster == q.Monsters[n].Monster && p.Monsters[n].RegenType == q.Monsters[n].RegenType && p.Monsters[n].RegenTime == q.Monsters[n].RegenTime;
			}

			if (!same)
			{
				diff = group + "distintos mapas o monstruos";

				return false;
			}
		}
	}

	return true;
}
