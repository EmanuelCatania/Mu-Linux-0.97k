#include "stdafx.h"
#include "game/core/MapInfoTable.h"
#include "assets/data/TomlLines.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

namespace {

struct MapTable {
    std::map<int, MapInfo> maps;
    std::vector<int>       allAmbient;
};

void Warn(const std::string& message)
{
    DbgLogPublic(("MapInfo: " + message).c_str());
}

// ["4", "10-255"]: tiles sueltos o rangos.
bool ParseTiles(const std::string& value, std::bitset<256>& tiles)
{
    std::vector<std::string> items;
    if (!TomlLines::SplitArray(value, items)) return false;
    for (const std::string& item : items) {
        std::string text;
        int first = 0, last = 0;
        if (!TomlLines::ParseString(item, text)) return false;
        const auto dash = text.find('-');
        if (dash == std::string::npos) {
            if (!TomlLines::ParseInt(text, first)) return false;
            last = first;
        } else if (!TomlLines::ParseInt(text.substr(0, dash), first) ||
                   !TomlLines::ParseInt(text.substr(dash + 1), last)) {
            return false;
        }
        if (first < 0 || first > last || last > 255) return false;
        for (int tile = first; tile <= last; ++tile) tiles.set(tile);
    }
    return true;
}

bool ParseInts(const std::string& value, std::vector<int>& out)
{
    std::vector<std::string> items;
    if (!TomlLines::SplitArray(value, items)) return false;
    out.clear();
    for (const std::string& item : items) {
        int number = 0;
        if (!TomlLines::ParseInt(item, number)) return false;
        out.push_back(number);
    }
    return true;
}

bool ParseMiniMap(const std::string& value, MapMiniMap& mode)
{
    std::string name;
    if (!TomlLines::ParseString(value, name)) return false;
    if (name == "none") mode = MapMiniMap::None;
    else if (name == "mini") mode = MapMiniMap::MiniMap;
    else if (name == "full") mode = MapMiniMap::FullMap;
    else return false;
    return true;
}

bool ParseField(const std::string& key, const std::string& value, MapInfo& info)
{
    if (key == "name") return TomlLines::ParseString(value, info.name);
    if (key == "music") return TomlLines::ParseString(value, info.music);
    if (key == "music_safe_zone_only") return TomlLines::ParseBool(value, info.musicSafeZoneOnly);
    if (key == "swimmable") return TomlLines::ParseBool(value, info.swimmable);
    if (key == "mount_flies") return TomlLines::ParseBool(value, info.mountFlies);
    if (key == "auto_attack") return TomlLines::ParseBool(value, info.autoAttack);
    if (key == "weather") return TomlLines::ParseBool(value, info.weather);
    if (key == "ambient") return ParseInts(value, info.ambient);
    if (key == "indoor_tiles") return ParseTiles(value, info.indoorTiles);
    if (key == "mini_map") return ParseMiniMap(value, info.miniMap);
    if (key == "object_folder") return TomlLines::ParseInt(value, info.objectFolder) && info.objectFolder >= 0;
    if (key == "load_from") return TomlLines::ParseInt(value, info.loadFrom) && info.loadFrom >= 0;
    return false;
}

// [[map.N.load]]: cada clave es una carga; una tabla puede traer varias.
//   model   = [slot, "Data/ObjectN/", "nombre", índice]
//   texture = [slot, "ObjectN/"]
//   jpg     = ["archivo.jpg", slot, "clamp" | "repeat"]
bool ParseLoad(const std::string& key, const std::string& value, MapLoad& load)
{
    std::vector<std::string> items;
    if (!TomlLines::SplitArray(value, items)) return false;
    if (key == "model") {
        load.kind = MapLoad::Kind::Model;
        return items.size() == 4 && TomlLines::ParseInt(items[0], load.slot) &&
               TomlLines::ParseString(items[1], load.path) && TomlLines::ParseString(items[2], load.name) &&
               TomlLines::ParseInt(items[3], load.index);
    }
    if (key == "texture") {
        load.kind = MapLoad::Kind::Texture;
        return items.size() == 2 && TomlLines::ParseInt(items[0], load.slot) &&
               TomlLines::ParseString(items[1], load.path);
    }
    if (key == "jpg") {
        std::string wrap;
        load.kind = MapLoad::Kind::Jpg;
        if (items.size() != 3 || !TomlLines::ParseString(items[0], load.path) ||
            !TomlLines::ParseInt(items[1], load.slot) || !TomlLines::ParseString(items[2], wrap))
            return false;
        load.repeat = wrap == "repeat";
        return wrap == "repeat" || wrap == "clamp";
    }
    return false;
}

MapTable Read()
{
    MapTable table;
    std::vector<TomlLines::Line> lines, bad;
    if (!TomlLines::Read("Content/map/maps.toml", lines, &bad)) {
        Warn("no se encontró Content/map/maps.toml; todos los mapas usan los valores por defecto");
        return table;
    }
    for (const auto& line : bad)
        Warn("línea " + std::to_string(line.number) + " inválida: " + line.value);
    for (const auto& line : lines) {
        // "map.N", "map.N.objects" o "map.N.load"
        int map = -1;
        std::string part;
        bool ok = line.section.rfind("map.", 0) == 0;
        if (ok) {
            const std::string rest = line.section.substr(4);
            const auto dot = rest.find('.');
            ok = TomlLines::ParseInt(rest.substr(0, dot), map) && map >= 0;
            if (dot != std::string::npos) part = rest.substr(dot + 1);
        }
        if (ok) {
            MapInfo& info = table.maps[map];
            if (part.empty()) {
                ok = line.entry == 0 && ParseField(line.key, line.value, info);
            } else if (part == "objects") {
                int slot = 0;
                std::string name;
                ok = line.entry == 0 && TomlLines::ParseInt(line.key, slot) && slot >= 0 &&
                     TomlLines::ParseString(line.value, name);
                if (ok) info.objects.emplace_back(slot, name);
            } else if (part == "load") {
                MapLoad load;
                // Las claves de una misma [[map.N.load]] se aplican en el orden del archivo.
                ok = line.entry > 0 && ParseLoad(line.key, line.value, load);
                if (ok) info.loads.push_back(load);
            } else {
                ok = false;
            }
        }
        if (!ok) Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
    }
    // load_from: las cargas de otro mapa (Blood Castle 2..6 usan las del 1).
    for (auto& entry : table.maps) {
        MapInfo& info = entry.second;
        if (info.loadFrom < 0) continue;
        const auto source = table.maps.find(info.loadFrom);
        if (source == table.maps.end() || source->second.loadFrom >= 0 || !info.loads.empty())
            Warn("[map." + std::to_string(entry.first) + "] load_from inválido");
        else
            info.loads = source->second.loads;
    }
    for (const auto& entry : table.maps)
        for (int sound : entry.second.ambient)
            if (std::find(table.allAmbient.begin(), table.allAmbient.end(), sound) == table.allAmbient.end())
                table.allAmbient.push_back(sound);
    return table;
}

const MapTable& Table()
{
    static const MapTable table = Read();
    return table;
}

} // namespace

const MapInfo& MapInfo_Get(int map)
{
    static const MapInfo fallback{};
    const auto found = Table().maps.find(map);
    return found == Table().maps.end() ? fallback : found->second;
}

int MapInfo_ObjectFolder(int map)
{
    const int folder = MapInfo_Get(map).objectFolder;
    return folder >= 0 ? folder : map + 1;
}

const std::vector<int>& MapInfo_AllAmbient()
{
    return Table().allAmbient;
}
