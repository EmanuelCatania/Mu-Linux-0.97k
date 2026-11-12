#include "ItemExtras.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

#define TOML_EXCEPTIONS 0
#include "tomlplusplus/toml.hpp"

namespace fs = std::filesystem;

static const char* const kSectionNames[ITEM_EXTRAS_SECTIONS] =
{
	"swords", "axes", "maces", "spears", "bows", "staffs", "shields", "helms",
	"armors", "pants", "gloves", "boots", "wings", "helpers", "potions", "scrolls",
};

const char* ItemExtras_SectionName(int section)
{
	return (section >= 0 && section < ITEM_EXTRAS_SECTIONS) ? kSectionNames[section] : "items";
}

int ItemExtras_GetItem(int section, int index)
{
	return (index < 32) ? (section * 32 + index) : (512 + section * 480 + (index - 32));
}

static fs::path ItemsDir(const std::string& dataDir)
{
	return fs::u8path(dataDir) / "Items";
}

// "00-swords.toml" -> 0; -1 si no es un archivo de sección.
static int FileSection(const std::string& name)
{
	if (name.size() < 8 || !std::isdigit((unsigned char)name[0]) || !std::isdigit((unsigned char)name[1]) || name[2] != '-')
	{
		return -1;
	}

	if (name.compare(name.size() - 5, 5, ".toml") != 0)
	{
		return -1;
	}

	int section = (name[0] - '0') * 10 + (name[1] - '0');

	return (section < ITEM_EXTRAS_SECTIONS) ? section : -1;
}

static bool ListSectionFiles(const std::string& dataDir, std::map<int, fs::path>& out, std::string& error)
{
	std::error_code ec;

	if (!fs::is_directory(ItemsDir(dataDir), ec))
	{
		return true;
	}

	for (const fs::directory_entry& entry : fs::directory_iterator(ItemsDir(dataDir), ec))
	{
		if (!entry.is_regular_file(ec))
		{
			continue;
		}

		std::string name = entry.path().filename().u8string();

		int section = FileSection(name);

		if (section < 0)
		{
			continue;
		}

		if (out.count(section) != 0)
		{
			error = "dos archivos para la sección " + std::to_string(section) + ": '" + out[section].filename().u8string() + "' y '" + name + "'";

			return false;
		}

		out[section] = entry.path();
	}

	return true;
}

std::string ItemExtras_SectionPath(const std::string& dataDir, int section)
{
	std::map<int, fs::path> files;

	std::string error;

	if (ListSectionFiles(dataDir, files, error) && files.count(section) != 0)
	{
		return files[section].u8string();
	}

	char name[64];

	snprintf(name, sizeof(name), "%02d-%s.toml", section, ItemExtras_SectionName(section));

	return (ItemsDir(dataDir) / name).u8string();
}

bool ItemExtras_HasData(const std::string& dataDir)
{
	std::map<int, fs::path> files;

	std::string error;

	return !ListSectionFiles(dataDir, files, error) || !files.empty();
}

//**********************************************//
//******************* JSON *********************//
//**********************************************//

static void CanonicalJson(const JsonValue& v, std::string& out)
{
	char buf[64];

	switch (v.type)
	{
	case JsonValue::Null: out += "null"; break;
	case JsonValue::Bool: out += v.boolean ? "true" : "false"; break;
	case JsonValue::Number: snprintf(buf, sizeof(buf), "%.17g", v.number); out += buf; break;
	case JsonValue::String: out += '"'; out += v.text; out += '"'; break;
	case JsonValue::Array:
		out += '[';
		for (size_t n = 0; n < v.items.size(); n++)
		{
			if (n != 0) out += ',';
			CanonicalJson(v.items[n], out);
		}
		out += ']';
		break;
	case JsonValue::Object:
		out += '{';
		for (std::map<std::string, JsonValue>::const_iterator it = v.members.begin(); it != v.members.end(); ++it)
		{
			if (it != v.members.begin()) out += ',';
			out += '"'; out += it->first; out += "\":";
			CanonicalJson(it->second, out);
		}
		out += '}';
		break;
	}
}

static std::string Canonical(const JsonValue& v)
{
	std::string out;

	CanonicalJson(v, out);

	return out;
}

static JsonValue JsonNumber(double value)
{
	JsonValue v;

	v.type = JsonValue::Number;

	v.number = value;

	return v;
}

// Nodo de toml++ -> JsonValue (las fechas no se usan en estos datos).
static bool TomlToJson(const toml::node& node, JsonValue& out, std::string& error)
{
	out = JsonValue();

	if (const toml::table* t = node.as_table())
	{
		out.type = JsonValue::Object;

		for (auto&& [key, value] : *t)
		{
			if (!TomlToJson(value, out.members[std::string(key.str())], error))
			{
				return false;
			}
		}

		return true;
	}

	if (const toml::array* a = node.as_array())
	{
		out.type = JsonValue::Array;

		for (const toml::node& item : *a)
		{
			out.items.emplace_back();

			if (!TomlToJson(item, out.items.back(), error))
			{
				return false;
			}
		}

		return true;
	}

	if (std::optional<std::string> s = node.value<std::string>(); s && node.is_string())
	{
		out.type = JsonValue::String;

		out.text = *s;

		return true;
	}

	if (node.is_integer() || node.is_floating_point())
	{
		out = JsonNumber(*node.value<double>());

		return true;
	}

	if (const toml::value<bool>* b = node.as_boolean())
	{
		out.type = JsonValue::Bool;

		out.boolean = b->get();

		return true;
	}

	error = "tipo de valor no soportado (fecha u hora)";

	return false;
}

//**********************************************//
//***************** Lectura TOML ***************//
//**********************************************//

static std::string Where(const fs::path& file, const std::string& key)
{
	return file.filename().u8string() + " [" + key + "]";
}

static bool ReadIntNode(const toml::node* node, int minValue, int maxValue, int& out)
{
	if (node == nullptr || !node->is_integer())
	{
		return false;
	}

	long long v = node->as_integer()->get();

	if (v < minValue || v > maxValue)
	{
		return false;
	}

	out = (int)v;

	return true;
}

// Nivel: número 0..15 o "*" (cualquiera). Sin la clave = cualquiera.
static bool ReadLevel(const toml::table& t, int& out)
{
	const toml::node* node = t.get("level");

	if (node == nullptr)
	{
		out = -1;

		return true;
	}

	if (node->is_string() && node->as_string()->get() == "*")
	{
		out = -1;

		return true;
	}

	return ReadIntNode(node, 0, 15, out);
}

static bool ReadItemPair(const toml::node* node, int& section, int& index)
{
	if (node == nullptr)
	{
		return false;
	}

	if (const toml::array* a = node->as_array())
	{
		return a->size() == 2 && ReadIntNode(a->get(0), 0, ITEM_EXTRAS_SECTIONS - 1, section) && ReadIntNode(a->get(1), 0, ITEM_EXTRAS_MAX_INDEX - 1, index);
	}

	if (node->is_string())
	{
		std::string text = node->as_string()->get();

		int s = 0;

		int i = 0;

		char tail = 0;

		if (sscanf(text.c_str(), "%d,%d%c", &s, &i, &tail) != 2 || s < 0 || s >= ITEM_EXTRAS_SECTIONS || i < 0 || i >= ITEM_EXTRAS_MAX_INDEX)
		{
			return false;
		}

		section = s;

		index = i;

		return true;
	}

	return false;
}

// Carpeta del modelo cuando el TOML no la dice: la del item en el cliente,
// Data\Item\NN-sección\III\ (el .bmd y sus texturas van juntos ahí).
static std::string DefaultFolder(int section, int index)
{
	char folder[64];

	snprintf(folder, sizeof(folder), "Item\\%02d-%s\\%03d\\", section, ItemExtras_SectionName(section), index);

	return folder;
}

static std::string ToBackslash(std::string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');

	return path;
}

static std::string ToSlash(std::string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	return path;
}

static bool ReadItemTable(const fs::path& file, int section, const std::string& key, const toml::table& t, ItemExtraDef& d, std::string& error)
{
	const std::string where = Where(file, key);

	d.Source = where;

	for (auto&& [k, value] : t)
	{
		const std::string name(k.str());

		if (name == "model")
		{
			const toml::table* m = value.as_table();

			const toml::node* folder = (m != nullptr) ? m->get("folder") : nullptr;

			const toml::node* model = (m != nullptr) ? m->get("name") : nullptr;

			if (m == nullptr || model == nullptr || !model->is_string() || (folder != nullptr && !folder->is_string()) || m->size() != ((folder != nullptr) ? 2u : 1u))
			{
				error = where + ": 'model' tiene que ser { name = \"...\" } o { folder = \"...\", name = \"...\" }";

				return false;
			}

			// Sin carpeta: la del item (Item/NN-sección/III/).
			d.ModelFolder = (folder != nullptr) ? ToBackslash(folder->as_string()->get()) : DefaultFolder(section, (int)std::strtol(key.c_str(), nullptr, 10));

			d.ModelName = model->as_string()->get();
		}
		else if (name == "glow")
		{
			const toml::array* a = value.as_array();

			int rgb[3] = { 0, 0, 0 };

			if (a == nullptr || a->size() != 3 || !ReadIntNode(a->get(0), 0, 255, rgb[0]) || !ReadIntNode(a->get(1), 0, 255, rgb[1]) || !ReadIntNode(a->get(2), 0, 255, rgb[2]))
			{
				error = where + ": 'glow' tiene que ser [R, G, B] con valores de 0 a 255";

				return false;
			}

			d.Glow = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
		}
		else if (name == "bow")
		{
			std::string type = value.is_string() ? value.as_string()->get() : "";

			if (type != "bow" && type != "crossbow")
			{
				error = where + ": 'bow' tiene que ser \"bow\" o \"crossbow\"";

				return false;
			}

			d.Bow = (type == "bow") ? 0 : 1;
		}
		else if (name == "wing")
		{
			const toml::table* w = value.as_table();

			if (w == nullptr)
			{
				error = where + ": 'wing' tiene que ser una tabla";

				return false;
			}

			d.HasWing = true;

			for (auto&& [wk, wv] : *w)
			{
				const std::string wname(wk.str());

				const toml::array* pair = wv.as_array();

				if (wname == "defense" && ReadIntNode(&wv, -100000, 100000, d.WingDefenseA))
				{
					continue;
				}

				if ((wname == "inc_damage" || wname == "dec_damage") && pair != nullptr && pair->size() == 2)
				{
					int* a = (wname == "inc_damage") ? &d.WingIncDamageA : &d.WingDecDamageA;

					int* b = (wname == "inc_damage") ? &d.WingIncDamageB : &d.WingDecDamageB;

					if (ReadIntNode(pair->get(0), -100000, 100000, *a) && ReadIntNode(pair->get(1), -100000, 100000, *b))
					{
						continue;
					}
				}

				error = where + ": 'wing." + wname + "' no es válido (defense = N, inc_damage = [A, B], dec_damage = [A, B])";

				return false;
			}
		}
		else if (name == "inventory" || name == "effects" || name == "tooltip" || name == "pet")
		{
			JsonValue* target = (name == "inventory") ? &d.Inventory : (name == "effects") ? &d.Effects : (name == "tooltip") ? &d.Tooltip : &d.Pet;

			std::string inner;

			if (!TomlToJson(value, *target, inner))
			{
				error = where + ": '" + name + "': " + inner;

				return false;
			}

			const bool needsObject = (name == "inventory" || name == "pet");

			if (needsObject ? !target->IsObject() : !target->IsArray())
			{
				error = where + ": '" + name + "' tiene que ser " + (needsObject ? "una tabla" : "un array");

				return false;
			}
		}
		else if (name == "stack")
		{
			// stack = 255  es lo mismo que  stack = [ { max = 255 } ].
			int max = 0;

			if (ReadIntNode(&value, 0, 255, max))
			{
				ItemExtraStack s;

				s.MaxStack = max;

				d.Stack.push_back(s);

				continue;
			}

			const toml::array* list = value.as_array();

			if (list == nullptr)
			{
				error = where + ": 'stack' tiene que ser un número o una lista de { level, max, create }";

				return false;
			}

			for (const toml::node& entry : *list)
			{
				const toml::table* e = entry.as_table();

				ItemExtraStack s;

				if (e == nullptr || !ReadLevel(*e, s.Level) || !ReadIntNode(e->get("max"), 0, 255, s.MaxStack))
				{
					error = where + ": cada 'stack' necesita max (0..255) y level opcional (0..15 o \"*\")";

					return false;
				}

				if (e->get("create") != nullptr && !ReadItemPair(e->get("create"), s.CreateSection, s.CreateIndex))
				{
					error = where + ": 'create' tiene que ser \"S,I\" o [S, I]";

					return false;
				}

				for (auto&& [ek, ev] : *e)
				{
					const std::string ename(ek.str());

					if (ename != "level" && ename != "max" && ename != "create")
					{
						error = where + ": clave desconocida en 'stack': " + ename;

						return false;
					}
				}

				d.Stack.push_back(s);
			}
		}
		else if (name == "value")
		{
			const toml::array* list = value.as_array();

			if (list == nullptr)
			{
				error = where + ": 'value' tiene que ser una lista de { level, buy, sell }";

				return false;
			}

			for (const toml::node& entry : *list)
			{
				const toml::table* e = entry.as_table();

				ItemExtraValue v;

				if (e == nullptr || !ReadLevel(*e, v.Level))
				{
					error = where + ": cada 'value' necesita level opcional (0..15 o \"*\")";

					return false;
				}

				if ((e->get("buy") != nullptr && !ReadIntNode(e->get("buy"), 0, 2000000000, v.Buy)) || (e->get("sell") != nullptr && !ReadIntNode(e->get("sell"), 0, 2000000000, v.Sell)))
				{
					error = where + ": 'buy' y 'sell' tienen que ser números de 0 a 2.000.000.000";

					return false;
				}

				for (auto&& [ek, ev] : *e)
				{
					const std::string ename(ek.str());

					if (ename != "level" && ename != "buy" && ename != "sell")
					{
						error = where + ": clave desconocida en 'value': " + ename;

						return false;
					}
				}

				d.Value.push_back(v);
			}
		}
		else
		{
			error = where + ": clave desconocida '" + name + "' (model, glow, bow, wing, inventory, effects, tooltip, pet, stack, value)";

			return false;
		}
	}

	(void)section;

	return true;
}

static bool LoadSectionFile(const fs::path& file, int section, std::vector<ItemExtraDef>& out, std::string& error)
{
	toml::parse_result result = toml::parse_file(file.u8string());

	if (!result)
	{
		const toml::parse_error& e = result.error();

		error = file.filename().u8string() + ":" + std::to_string(e.source().begin.line) + ": " + std::string(e.description());

		return false;
	}

	const toml::table& root = result.table();

	for (auto&& [k, value] : root)
	{
		const std::string key(k.str());

		char* end = nullptr;

		long index = std::strtol(key.c_str(), &end, 10);

		if (key.empty() || *end != '\0' || index < 0 || index >= ITEM_EXTRAS_MAX_INDEX)
		{
			error = file.filename().u8string() + ": '" + key + "' no es un índice de item (0 a 511)";

			return false;
		}

		const toml::table* t = value.as_table();

		if (t == nullptr)
		{
			error = Where(file, key) + ": cada item es una tabla, por ejemplo [" + key + "]";

			return false;
		}

		ItemExtraDef d;

		d.Section = section;

		d.Index = (int)index;

		if (!ReadItemTable(file, section, key, *t, d, error))
		{
			return false;
		}

		out.push_back(d);
	}

	return true;
}

static bool DefLess(const ItemExtraDef& a, const ItemExtraDef& b)
{
	return (a.Section != b.Section) ? (a.Section < b.Section) : (a.Index < b.Index);
}

bool ItemExtras_Load(const std::string& dataDir, std::vector<ItemExtraDef>& out, std::string& error)
{
	out.clear();

	std::map<int, fs::path> files;

	if (!ListSectionFiles(dataDir, files, error))
	{
		return false;
	}

	for (const std::pair<const int, fs::path>& file : files)
	{
		if (!LoadSectionFile(file.second, file.first, out, error))
		{
			return false;
		}
	}

	std::stable_sort(out.begin(), out.end(), DefLess);

	for (size_t n = 1; n < out.size(); n++)
	{
		if (out[n].Section == out[n - 1].Section && out[n].Index == out[n - 1].Index)
		{
			error = out[n].Source + ": el item está dos veces";

			return false;
		}
	}

	return true;
}

//**********************************************//
//************** Formatos viejos ***************//
//**********************************************//

// Partes una línea de los .txt viejos: '//' es comentario, la coma separa como
// un espacio ("12,015" = sección e índice) y las comillas agrupan.
static void SplitLine(const std::string& raw, std::vector<std::string>& tokens)
{
	tokens.clear();

	std::string token;

	bool quoted = false;

	bool hasToken = false;

	for (size_t n = 0; n < raw.size(); n++)
	{
		char c = raw[n];

		if (!quoted && c == '/' && n + 1 < raw.size() && raw[n + 1] == '/')
		{
			break;
		}

		if (c == '"')
		{
			quoted = !quoted;

			hasToken = true;

			continue;
		}

		if (!quoted && (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ','))
		{
			if (hasToken)
			{
				tokens.push_back(token);
			}

			token.clear();

			hasToken = false;

			continue;
		}

		token += c;

		hasToken = true;
	}

	if (hasToken)
	{
		tokens.push_back(token);
	}
}

static bool ParseNumber(const std::string& text, int& out)
{
	if (text == "*")
	{
		out = -1;

		return true;
	}

	char* end = nullptr;

	long v = std::strtol(text.c_str(), &end, 10);

	if (text.empty() || *end != '\0')
	{
		return false;
	}

	out = (int)v;

	return true;
}

static bool ParseFloat(const std::string& text, float& out)
{
	char* end = nullptr;

	out = std::strtof(text.c_str(), &end);

	return !text.empty() && *end == '\0';
}

// Recorre las filas de un .txt viejo hasta 'end'. `row` devuelve false con el
// motivo para cortar.
template <typename F> static bool ForEachRow(const fs::path& path, bool& exists, std::string& error, F row)
{
	std::ifstream file(path, std::ios::binary);

	exists = (bool)file;

	if (!file)
	{
		return true;
	}

	std::string raw;

	int line = 0;

	std::vector<std::string> tokens;

	while (std::getline(file, raw))
	{
		line++;

		SplitLine(raw, tokens);

		if (tokens.empty())
		{
			continue;
		}

		if (tokens[0] == "end")
		{
			break;
		}

		std::string reason;

		if (!row(tokens, reason))
		{
			error = path.filename().u8string() + ":" + std::to_string(line) + ": " + reason;

			return false;
		}
	}

	return true;
}

static ItemExtraDef& DefFor(std::vector<ItemExtraDef>& out, std::map<int, size_t>& byItem, int section, int index)
{
	int key = section * ITEM_EXTRAS_MAX_INDEX + index;

	std::map<int, size_t>::iterator it = byItem.find(key);

	if (it != byItem.end())
	{
		return out[it->second];
	}

	ItemExtraDef d;

	d.Section = section;

	d.Index = index;

	byItem[key] = out.size();

	out.push_back(d);

	return out.back();
}

static bool ValidItem(int section, int index)
{
	return section >= 0 && section < ITEM_EXTRAS_SECTIONS && index >= 0 && index < ITEM_EXTRAS_MAX_INDEX;
}

// Carpeta que le daba el GameServer a un modelo del Encoder: las partes de
// armadura (secciones 7..11) por sub-índice bajo Player, el resto por índice
// (GET_ITEM) bajo Item.
static std::string LegacyFolder(int section, int index)
{
	char folder[64];

	if (section >= 7 && section < 12)
	{
		snprintf(folder, sizeof(folder), "Player\\Custom\\%d\\", index);
	}
	else
	{
		snprintf(folder, sizeof(folder), "Item\\Custom\\%d\\", ItemExtras_GetItem(section, index));
	}

	return folder;
}

// "3,0", "12,032" o [12, 32].
static bool JsonItemKey(const JsonValue* value, int& section, int& index)
{
	if (value == nullptr)
	{
		return false;
	}

	if (value->IsArray() && value->items.size() == 2 && value->items[0].IsNumber() && value->items[1].IsNumber())
	{
		section = (int)value->items[0].number;

		index = (int)value->items[1].number;

		return ValidItem(section, index);
	}

	if (value->IsString())
	{
		char tail = 0;

		return sscanf(value->text.c_str(), "%d,%d%c", &section, &index, &tail) == 2 && ValidItem(section, index);
	}

	return false;
}

// Comentario de arriba de un JSON ("// ..." antes del '{').
static std::string LeadingComment(const std::string& text)
{
	std::istringstream in(text);

	std::string line;

	std::string comment;

	while (std::getline(in, line))
	{
		size_t first = line.find_first_not_of(" \t\r");

		if (first == std::string::npos)
		{
			continue;
		}

		if (line.compare(first, 2, "//") != 0)
		{
			break;
		}

		std::string body = line.substr(first + 2);

		size_t start = body.find_first_not_of(' ');

		size_t last = body.find_last_not_of(" \t\r");

		comment += (start == std::string::npos) ? "" : body.substr(start, last - start + 1);

		comment += '\n';
	}

	return comment;
}

static bool LoadJsonFolder(const fs::path& folder, bool pets, std::vector<ItemExtraDef>& out, std::map<int, size_t>& byItem, std::string& error)
{
	std::error_code ec;

	if (!fs::is_directory(folder, ec))
	{
		return true;
	}

	std::vector<fs::path> files;

	for (const fs::directory_entry& entry : fs::directory_iterator(folder, ec))
	{
		if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
		{
			files.push_back(entry.path());
		}
	}

	std::sort(files.begin(), files.end());

	for (const fs::path& file : files)
	{
		std::ifstream stream(file, std::ios::binary);

		std::stringstream buffer;

		buffer << stream.rdbuf();

		const std::string text = buffer.str();

		const std::string name = (pets ? "Custom/Pets/" : "Custom/Items/") + file.filename().u8string();

		JsonValue root;

		std::string inner;

		if (!JsonParser::Parse(text, root, inner))
		{
			error = name + ": JSON inválido: " + inner;

			return false;
		}

		int section = 0;

		int index = 0;

		if (!root.IsObject() || !JsonItemKey(root.Get("item"), section, index))
		{
			error = name + ": falta 'item' o no es válido";

			return false;
		}

		ItemExtraDef& d = DefFor(out, byItem, section, index);

		std::string comment = LeadingComment(text);

		if (!comment.empty())
		{
			d.Comment += comment;
		}

		if (pets)
		{
			d.Pet = root;

			d.Pet.members.erase("item");

			continue;
		}

		for (const std::pair<const std::string, JsonValue>& member : root.members)
		{
			if (member.first == "item")
			{
				continue;
			}

			if (member.first == "inventory")
			{
				d.Inventory = member.second;
			}
			else if (member.first == "effects")
			{
				d.Effects = member.second;
			}
			else if (member.first == "tooltip")
			{
				d.Tooltip = member.second;
			}
			else
			{
				error = name + ": clave desconocida '" + member.first + "'";

				return false;
			}
		}
	}

	return true;
}

static JsonValue PoseJson(float x, float y, float rx, float ry, float rz, float scale)
{
	JsonValue pose;

	pose.type = JsonValue::Object;

	JsonValue& position = pose.members["position"];

	position.type = JsonValue::Array;

	position.items = { JsonNumber(x), JsonNumber(y) };

	JsonValue& rotation = pose.members["rotation"];

	rotation.type = JsonValue::Array;

	rotation.items = { JsonNumber(rx), JsonNumber(ry), JsonNumber(rz) };

	pose.members["scale"] = JsonNumber(scale);

	return pose;
}

bool ItemExtras_LoadLegacy(const std::string& dataDir, std::vector<ItemExtraDef>& out, std::string& error)
{
	out.clear();

	std::map<int, size_t> byItem;

	const fs::path data = fs::u8path(dataDir);

	const fs::path encoder = data / "Custom" / "Encoder";

	bool exists = false;

	// CustomItem: Sección,Índice Skill "Modelo". Gana la primera fila del item.
	if (!ForEachRow(encoder / "CustomItem.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0, skill = 0;

		if (t.size() < 4 || !ParseNumber(t[0], s) || !ParseNumber(t[1], i) || !ParseNumber(t[2], skill) || !ValidItem(s, i))
		{
			why = "se espera Sección,Índice Skill \"Modelo\"";

			return false;
		}

		ItemExtraDef& d = DefFor(out, byItem, s, i);

		if (d.ModelName.empty())
		{
			d.ModelFolder = LegacyFolder(s, i);

			d.ModelName = t[3];
		}

		return true;
	}))
	{
		return false;
	}

	// CustomGlow: Sección,Índice R G B. Gana la primera fila.
	if (!ForEachRow(encoder / "CustomGlow.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0, r = 0, g = 0, b = 0;

		if (t.size() < 5 || !ParseNumber(t[0], s) || !ParseNumber(t[1], i) || !ParseNumber(t[2], r) || !ParseNumber(t[3], g) || !ParseNumber(t[4], b) || !ValidItem(s, i))
		{
			why = "se espera Sección,Índice R G B";

			return false;
		}

		ItemExtraDef& d = DefFor(out, byItem, s, i);

		if (d.Glow < 0)
		{
			d.Glow = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
		}

		return true;
	}))
	{
		return false;
	}

	// CustomBow: Índice(sección 4) Tipo Efecto "Modelo". El efecto nunca se usó.
	if (!ForEachRow(encoder / "CustomBow.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int i = 0, type = 0, effect = 0;

		if (t.size() < 4 || !ParseNumber(t[0], i) || !ParseNumber(t[1], type) || !ParseNumber(t[2], effect) || !ValidItem(4, i))
		{
			why = "se espera Índice Tipo Efecto \"Modelo\"";

			return false;
		}

		ItemExtraDef& d = DefFor(out, byItem, 4, i);

		if (d.Bow < 0)
		{
			d.Bow = (type == 0) ? 0 : 1;
		}

		if (d.ModelName.empty())
		{
			d.ModelFolder = LegacyFolder(4, i);

			d.ModelName = t[3];
		}

		return true;
	}))
	{
		return false;
	}

	// ItemPosition: Sección,Índice X Y RotX RotY RotZ Escala. Gana la última.
	if (!ForEachRow(encoder / "ItemPosition.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0;

		float v[6] = {};

		bool ok = t.size() >= 8 && ParseNumber(t[0], s) && ParseNumber(t[1], i) && ValidItem(s, i);

		for (int n = 0; ok && n < 6; n++)
		{
			ok = ParseFloat(t[2 + n], v[n]);
		}

		if (!ok)
		{
			why = "se espera Sección,Índice X Y RotX RotY RotZ Escala";

			return false;
		}

		DefFor(out, byItem, s, i).Inventory = PoseJson(v[0], v[1], v[2], v[3], v[4], v[5]);

		return true;
	}))
	{
		return false;
	}

	// CustomWing: Sección,Índice DefA IncA IncB DecA DecB "Modelo" (el modelo
	// nunca se usó: sale de Item.txt).
	if (!ForEachRow(data / "Item" / "CustomWing.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0;

		int c[5] = {};

		bool ok = t.size() >= 7 && ParseNumber(t[0], s) && ParseNumber(t[1], i) && ValidItem(s, i);

		for (int n = 0; ok && n < 5; n++)
		{
			ok = ParseNumber(t[2 + n], c[n]);
		}

		if (!ok)
		{
			why = "se espera Sección,Índice DefA IncA IncB DecA DecB \"Modelo\"";

			return false;
		}

		ItemExtraDef& d = DefFor(out, byItem, s, i);

		d.HasWing = true;

		d.WingDefenseA = c[0];

		d.WingIncDamageA = c[1];

		d.WingIncDamageB = c[2];

		d.WingDecDamageA = c[3];

		d.WingDecDamageB = c[4];

		return true;
	}))
	{
		return false;
	}

	// ItemStack: Sección,Índice Level MaxStack CreateIndex [Sub].
	if (!ForEachRow(data / "Item" / "ItemStack.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0;

		ItemExtraStack st;

		bool ok = t.size() >= 5 && ParseNumber(t[0], s) && ParseNumber(t[1], i) && ParseNumber(t[2], st.Level) && ParseNumber(t[3], st.MaxStack) && ParseNumber(t[4], st.CreateSection) && ValidItem(s, i);

		if (ok && st.CreateSection != -1)
		{
			ok = t.size() >= 6 && ParseNumber(t[5], st.CreateIndex) && ValidItem(st.CreateSection, st.CreateIndex);
		}

		if (!ok)
		{
			why = "se espera Sección,Índice Level MaxStack CreateIndex";

			return false;
		}

		DefFor(out, byItem, s, i).Stack.push_back(st);

		return true;
	}))
	{
		return false;
	}

	// ItemValue: Sección,Índice Level BuyValue SellValue.
	if (!ForEachRow(data / "Item" / "ItemValue.txt", exists, error, [&](const std::vector<std::string>& t, std::string& why)
	{
		int s = 0, i = 0;

		ItemExtraValue v;

		if (t.size() < 5 || !ParseNumber(t[0], s) || !ParseNumber(t[1], i) || !ParseNumber(t[2], v.Level) || !ParseNumber(t[3], v.Buy) || !ParseNumber(t[4], v.Sell) || !ValidItem(s, i))
		{
			why = "se espera Sección,Índice Level BuyValue SellValue";

			return false;
		}

		DefFor(out, byItem, s, i).Value.push_back(v);

		return true;
	}))
	{
		return false;
	}

	if (!LoadJsonFolder(data / "Custom" / "Items", false, out, byItem, error) || !LoadJsonFolder(data / "Custom" / "Pets", true, out, byItem, error))
	{
		return false;
	}

	std::stable_sort(out.begin(), out.end(), DefLess);

	return true;
}

std::vector<std::string> ItemExtras_LegacyFiles(const std::string& dataDir)
{
	static const char* const kFiles[] =
	{
		"Custom/Encoder/CustomItem.txt", "Custom/Encoder/CustomGlow.txt", "Custom/Encoder/CustomBow.txt", "Custom/Encoder/ItemPosition.txt",
		"Item/CustomWing.txt", "Item/ItemStack.txt", "Item/ItemValue.txt",
	};

	std::vector<std::string> out;

	std::error_code ec;

	const fs::path data = fs::u8path(dataDir);

	for (const char* file : kFiles)
	{
		if (fs::is_regular_file(data / fs::u8path(file), ec))
		{
			out.push_back(file);
		}
	}

	for (const char* folder : { "Custom/Items", "Custom/Pets" })
	{
		if (!fs::is_directory(data / folder, ec))
		{
			continue;
		}

		for (const fs::directory_entry& entry : fs::directory_iterator(data / folder, ec))
		{
			if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
			{
				out.push_back(std::string(folder) + "/" + entry.path().filename().u8string());
			}
		}
	}

	std::sort(out.begin(), out.end());

	return out;
}

void ItemExtras_ReadItemNames(const std::string& itemTxtPath, std::vector<std::string>& names)
{
	names.assign(8192, std::string());

	std::ifstream file(fs::u8path(itemTxtPath), std::ios::binary);

	std::string raw;

	int section = -1;

	std::vector<std::string> tokens;

	while (std::getline(file, raw))
	{
		// El nombre es el primer campo entre comillas.
		size_t open = raw.find('"');

		size_t close = (open == std::string::npos) ? std::string::npos : raw.find('"', open + 1);

		SplitLine(raw, tokens);

		if (tokens.empty())
		{
			continue;
		}

		if (tokens[0] == "end")
		{
			section = -1;

			continue;
		}

		int number = 0;

		if (section < 0)
		{
			if (tokens.size() == 1 && ParseNumber(tokens[0], number) && number >= 0 && number < ITEM_EXTRAS_SECTIONS)
			{
				section = number;
			}

			continue;
		}

		if (close != std::string::npos && ParseNumber(tokens[0], number) && ValidItem(section, number))
		{
			names[ItemExtras_GetItem(section, number)] = raw.substr(open + 1, close - open - 1);
		}
	}
}

//**********************************************//
//******************* Escritura ****************//
//**********************************************//

static std::string TomlString(const std::string& text)
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

static std::string TomlKey(const std::string& key)
{
	bool bare = !key.empty();

	for (char c : key)
	{
		bare = bare && (std::isalnum((unsigned char)c) || c == '_' || c == '-');
	}

	return bare ? key : TomlString(key);
}

static std::string TomlNumber(double value)
{
	char buf[64];

	if (value == std::floor(value) && std::fabs(value) < 1e15)
	{
		snprintf(buf, sizeof(buf), "%.0f", value);
	}
	else
	{
		// El más corto que vuelve al mismo double: así leer y escribir no
		// cambia nada (check-items compara los valores exactos).
		for (int precision = 1; precision <= 17; precision++)
		{
			snprintf(buf, sizeof(buf), "%.*g", precision, value);

			if (std::strtod(buf, nullptr) == value)
			{
				break;
			}
		}
	}

	return buf;
}

static std::string TomlInline(const JsonValue& v)
{
	switch (v.type)
	{
	case JsonValue::Bool: return v.boolean ? "true" : "false";
	case JsonValue::Number: return TomlNumber(v.number);
	case JsonValue::String: return TomlString(v.text);
	case JsonValue::Array:
	{
		std::string out = "[";

		for (size_t n = 0; n < v.items.size(); n++)
		{
			out += (n == 0) ? "" : ", ";

			out += TomlInline(v.items[n]);
		}

		return out + "]";
	}
	case JsonValue::Object:
	{
		std::string out = "{ ";

		bool first = true;

		for (const std::pair<const std::string, JsonValue>& member : v.members)
		{
			out += first ? "" : ", ";

			out += TomlKey(member.first) + " = " + TomlInline(member.second);

			first = false;
		}

		return out + (first ? "}" : " }");
	}
	default: return "\"\"";
	}
}

// Array en varias líneas, un elemento por línea (efectos, tooltip).
static std::string TomlMultiline(const JsonValue& v)
{
	if (v.items.size() <= 1)
	{
		return TomlInline(v);
	}

	std::string out = "[\n";

	for (const JsonValue& item : v.items)
	{
		out += "\t" + TomlInline(item) + ",\n";
	}

	return out + "]";
}

static std::string LevelText(int level)
{
	return (level < 0) ? "" : ("level = " + std::to_string(level));
}

static void WriteItem(std::ofstream& file, const ItemExtraDef& d, const std::vector<std::string>& names)
{
	const int item = ItemExtras_GetItem(d.Section, d.Index);

	const std::string& name = (item >= 0 && item < (int)names.size()) ? names[item] : std::string();

	if (!name.empty())
	{
		file << "# " << name << "\n";
	}

	std::istringstream comment(d.Comment);

	std::string line;

	while (std::getline(comment, line))
	{
		file << (line.empty() ? "#" : ("# " + line)) << "\n";
	}

	char key[8];

	snprintf(key, sizeof(key), "%03d", d.Index);

	file << "[" << key << "]\n";

	if (!d.ModelName.empty())
	{
		if (d.ModelFolder == DefaultFolder(d.Section, d.Index))
		{
			file << "model = { name = " << TomlString(d.ModelName) << " }\n";
		}
		else
		{
			file << "model = { folder = " << TomlString(ToSlash(d.ModelFolder)) << ", name = " << TomlString(d.ModelName) << " }\n";
		}
	}

	if (d.Bow >= 0)
	{
		file << "bow = \"" << ((d.Bow == 0) ? "bow" : "crossbow") << "\"\n";
	}

	if (d.Glow >= 0)
	{
		file << "glow = [" << ((d.Glow >> 16) & 0xFF) << ", " << ((d.Glow >> 8) & 0xFF) << ", " << (d.Glow & 0xFF) << "]\n";
	}

	if (d.HasWing)
	{
		file << "wing = { defense = " << d.WingDefenseA << ", inc_damage = [" << d.WingIncDamageA << ", " << d.WingIncDamageB << "], dec_damage = [" << d.WingDecDamageA << ", " << d.WingDecDamageB << "] }\n";
	}

	if (d.Inventory.IsObject())
	{
		file << "inventory = " << TomlInline(d.Inventory) << "\n";
	}

	if (d.Tooltip.IsArray())
	{
		file << "tooltip = " << TomlMultiline(d.Tooltip) << "\n";
	}

	if (d.Effects.IsArray())
	{
		file << "effects = " << TomlMultiline(d.Effects) << "\n";
	}

	if (d.Stack.size() == 1 && d.Stack[0].Level < 0 && d.Stack[0].CreateSection < 0)
	{
		file << "stack = " << d.Stack[0].MaxStack << "\n";
	}
	else if (!d.Stack.empty())
	{
		file << "stack = [\n";

		for (const ItemExtraStack& s : d.Stack)
		{
			std::string fields = LevelText(s.Level);

			fields += (fields.empty() ? "" : ", ") + std::string("max = ") + std::to_string(s.MaxStack);

			if (s.CreateSection >= 0)
			{
				fields += ", create = \"" + std::to_string(s.CreateSection) + "," + std::to_string(s.CreateIndex) + "\"";
			}

			file << "\t{ " << fields << " },\n";
		}

		file << "]\n";
	}

	if (!d.Value.empty())
	{
		file << "value = [\n";

		for (const ItemExtraValue& v : d.Value)
		{
			std::string fields = LevelText(v.Level);

			if (v.Buy >= 0)
			{
				fields += (fields.empty() ? "" : ", ") + std::string("buy = ") + std::to_string(v.Buy);
			}

			if (v.Sell >= 0)
			{
				fields += (fields.empty() ? "" : ", ") + std::string("sell = ") + std::to_string(v.Sell);
			}

			file << "\t{ " << fields << " },\n";
		}

		file << "]\n";
	}

	if (d.Pet.IsObject())
	{
		file << "\n[" << key << ".pet]\n";

		for (const std::pair<const std::string, JsonValue>& member : d.Pet.members)
		{
			file << TomlKey(member.first) << " = " << (member.second.IsArray() ? TomlMultiline(member.second) : TomlInline(member.second)) << "\n";
		}
	}

	file << "\n";
}

bool ItemExtras_WriteSection(const std::string& path, int section, const std::vector<ItemExtraDef>& items, const std::vector<std::string>& names, std::string& error)
{
	std::error_code ec;

	fs::create_directories(fs::u8path(path).parent_path(), ec);

	std::ofstream file(fs::u8path(path), std::ios::binary);

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	file << "# Items de la sección " << section << " (" << ItemExtras_SectionName(section) << "): lo que no está en Item/Item.txt.\n";
	file << "# Cada tabla es un item por su índice: [020] es el item " << section << ",20.\n";
	file << "#\n";
	file << "#   model     = { name = \"Sword21\" }                          modelo propio en Data/Item/NN-sección/III/ del cliente\n";
	file << "#               (o { folder = \"Player/Custom/21/\", name = \"...\" } para otra carpeta bajo Data/)\n";
	file << "#   bow       = \"bow\" | \"crossbow\"                                 arco o ballesta custom\n";
	file << "#   glow      = [R, G, B]                                            color del brillo\n";
	file << "#   wing      = { defense = A, inc_damage = [A, B], dec_damage = [A, B] }   ala custom\n";
	file << "#   inventory = { position = [x, y], rotation = [x, y, z], scale = s }      pose en el inventario\n";
	file << "#   tooltip   = [ { text = \"...\", color = \"gold\" } ]                líneas propias del tooltip\n";
	file << "#   effects   = [ { on = \"equipped\", type = \"sprite\", ... } ]        efectos sobre los huesos\n";
	file << "#   stack     = 255  o  [ { level = 0, max = 255, create = \"14,22\" } ]   apilado (sólo server)\n";
	file << "#   value     = [ { level = 0, buy = 100, sell = 20 } ]              precio (sólo server)\n";
	file << "#   [NNN.pet]                                                        pet (el pickup es sólo server)\n";
	file << "\n";

	for (const ItemExtraDef& d : items)
	{
		if (d.Section == section)
		{
			WriteItem(file, d, names);
		}
	}

	if (!file)
	{
		error = "no se pudo escribir " + path;

		return false;
	}

	return true;
}

//**********************************************//
//******************* Comparar *****************//
//**********************************************//

static std::string Describe(const ItemExtraDef& d)
{
	std::ostringstream out;

	out << "model=" << d.ModelFolder << d.ModelName << " glow=" << d.Glow << " bow=" << d.Bow;

	out << " wing=" << d.HasWing << ":" << d.WingDefenseA << "," << d.WingIncDamageA << "," << d.WingIncDamageB << "," << d.WingDecDamageA << "," << d.WingDecDamageB;

	out << " inventory=" << Canonical(d.Inventory) << " effects=" << Canonical(d.Effects) << " tooltip=" << Canonical(d.Tooltip) << " pet=" << Canonical(d.Pet);

	out << " stack=";

	for (const ItemExtraStack& s : d.Stack)
	{
		out << "(" << s.Level << "," << s.MaxStack << "," << s.CreateSection << "," << s.CreateIndex << ")";
	}

	out << " value=";

	for (const ItemExtraValue& v : d.Value)
	{
		out << "(" << v.Level << "," << v.Buy << "," << v.Sell << ")";
	}

	return out.str();
}

bool ItemExtras_Equal(const std::vector<ItemExtraDef>& a, const std::vector<ItemExtraDef>& b, std::string& diff)
{
	std::map<int, std::string> left;

	std::map<int, std::string> right;

	for (const ItemExtraDef& d : a)
	{
		left[ItemExtras_GetItem(d.Section, d.Index)] = Describe(d);
	}

	for (const ItemExtraDef& d : b)
	{
		right[ItemExtras_GetItem(d.Section, d.Index)] = Describe(d);
	}

	for (const ItemExtraDef& d : a)
	{
		const int item = ItemExtras_GetItem(d.Section, d.Index);

		if (right.count(item) == 0)
		{
			diff = "el item " + std::to_string(d.Section) + "," + std::to_string(d.Index) + " falta en el segundo";

			return false;
		}

		if (right[item] != left[item])
		{
			diff = "el item " + std::to_string(d.Section) + "," + std::to_string(d.Index) + " difiere:\n  " + left[item] + "\n  " + right[item];

			return false;
		}
	}

	for (const ItemExtraDef& d : b)
	{
		if (left.count(ItemExtras_GetItem(d.Section, d.Index)) == 0)
		{
			diff = "el item " + std::to_string(d.Section) + "," + std::to_string(d.Index) + " falta en el primero";

			return false;
		}
	}

	return true;
}
