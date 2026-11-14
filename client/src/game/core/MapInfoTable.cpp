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
        int map = -1;
        const bool inMap = line.section.rfind("map.", 0) == 0
                        && TomlLines::ParseInt(line.section.substr(4), map) && map >= 0;
        if (!inMap || !ParseField(line.key, line.value, table.maps[map]))
            Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
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

const std::vector<int>& MapInfo_AllAmbient()
{
    return Table().allAmbient;
}
