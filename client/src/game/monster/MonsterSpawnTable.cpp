#include "stdafx.h"
#include "game/monster/MonsterSpawnTable.h"
#include "assets/data/TomlLines.h"

#include <array>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

namespace {

constexpr int kTypes = 256;
constexpr int kItemModelBase = 400;   // ITEM_MODEL_BASE
constexpr int kItemsPerSection = 32;  // ITEM_MAX_TYPE_VANILLA

void Warn(const std::string& message)
{
    DbgLogPublic(("MonsterSpawn: " + message).c_str());
}

// [sección, índice] -> modelo del item.
bool ParseItem(const std::string& text, int& model)
{
    std::vector<std::string> items;
    int section = 0, index = 0;
    if (!TomlLines::SplitArray(text, items) || items.size() != 2) return false;
    if (!TomlLines::ParseInt(items[0], section) || !TomlLines::ParseInt(items[1], index)) return false;
    if (section < 0 || index < 0 || index >= kItemsPerSection) return false;
    model = kItemModelBase + section * kItemsPerSection + index;
    return true;
}

bool ParseLoader(const std::string& text, SpawnLoader& loader)
{
    std::string name;
    if (!TomlLines::ParseString(text, name)) return false;
    if (name == "monster") loader = SpawnLoader::Monster;
    else if (name == "npc") loader = SpawnLoader::Npc;
    else if (name == "none") loader = SpawnLoader::None;
    else return false;
    return true;
}

bool ParseField(const std::string& key, const std::string& value, MonsterSpawn& spawn)
{
    if (key == "model") return TomlLines::ParseInt(value, spawn.Model);
    if (key == "load") return ParseLoader(value, spawn.Loader);
    if (key == "resource") return TomlLines::ParseInt(value, spawn.Resource);
    if (key == "extra") return TomlLines::ParseInt(value, spawn.ExtraResource);
    if (key == "scale") return TomlLines::ParseFloat(value, spawn.Scale);
    if (key == "right") return ParseItem(value, spawn.Right);
    if (key == "left") return ParseItem(value, spawn.Left);
    if (key == "right_level") return TomlLines::ParseInt(value, spawn.RightLevel);
    if (key == "left_level") return TomlLines::ParseInt(value, spawn.LeftLevel);
    if (key == "right_excellent") return TomlLines::ParseInt(value, spawn.RightExcellent);
    if (key == "blend_mesh") return TomlLines::ParseInt(value, spawn.BlendMesh);
    if (key == "blend_light") return TomlLines::ParseFloat(value, spawn.BlendLight);
    if (key == "subtype") return TomlLines::ParseInt(value, spawn.Subtype);
    if (key == "hidden_mesh") return TomlLines::ParseInt(value, spawn.HiddenMesh);
    if (key == "level") return TomlLines::ParseInt(value, spawn.State446);
    if (key == "blood") return TomlLines::ParseInt(value, spawn.Flag766);
    if (key == "init") return TomlLines::ParseString(value, spawn.Init);
    return false;
}

using SpawnTable = std::array<MonsterSpawn, kTypes>;

SpawnTable Read()
{
    SpawnTable table{};
    std::vector<TomlLines::Line> lines, bad;
    if (!TomlLines::Read("Content/monster/spawn.toml", lines, &bad)) {
        Warn("no se encontró Content/monster/spawn.toml; todos los tipos usan el modelo 270");
        return table;
    }
    for (const auto& line : bad)
        Warn("línea " + std::to_string(line.number) + " inválida: " + line.value);
    for (const auto& line : lines) {
        int type = -1;
        const bool inSpawn = line.section.rfind("spawn.", 0) == 0
                          && TomlLines::ParseInt(line.section.substr(6), type)
                          && type >= 0 && type < kTypes;
        if (!inSpawn || !ParseField(line.key, line.value, table[type]))
            Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
    }
    return table;
}

} // namespace

const MonsterSpawn& MonsterSpawn_Get(int type)
{
    static const SpawnTable table = Read();
    static const MonsterSpawn fallback{};
    return (type >= 0 && type < kTypes) ? table[type] : fallback;
}
