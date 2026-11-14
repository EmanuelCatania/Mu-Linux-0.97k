#include "stdafx.h"
#include "render/InventoryPoseTable.h"
#include "assets/data/TomlLines.h"
#include "app/core/ResourceLimits.h"

#include <array>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

namespace {

using PoseTable = std::array<InventoryPose, MODEL_MAX_VANILLA>;

void Warn(const std::string& message)
{
    DbgLogPublic(("InventoryPose: " + message).c_str());
}

// "primero-último" o "uno"; los dos dentro de la tabla.
bool ParseRange(const std::string& key, int& first, int& last)
{
    std::string text;
    if (!TomlLines::ParseString(key, text)) return false;
    const auto dash = text.find('-');
    if (dash == std::string::npos) {
        if (!TomlLines::ParseInt(text, first)) return false;
        last = first;
    } else if (!TomlLines::ParseInt(text.substr(0, dash), first) ||
               !TomlLines::ParseInt(text.substr(dash + 1), last)) {
        return false;
    }
    return first >= 0 && first <= last && last < MODEL_MAX_VANILLA;
}

bool ApplyAngle(const std::string& value, int first, int last, PoseTable& table)
{
    std::vector<std::string> items;
    float angle[3];
    if (!TomlLines::SplitArray(value, items) || items.size() < 3 || items.size() > 4) return false;
    for (int i = 0; i < 3; ++i)
        if (!TomlLines::ParseFloat(items[i], angle[i])) return false;
    InventoryAngleMode mode = InventoryAngleMode::Fixed;
    if (items.size() == 4) {
        std::string name;
        if (!TomlLines::ParseString(items[3], name)) return false;
        if (name == "two_hand") mode = InventoryAngleMode::ItemAttribute;
        else if (name == "level") mode = InventoryAngleMode::Level;
        else return false;
    }
    for (int type = first; type <= last; ++type) {
        for (int i = 0; i < 3; ++i) table[type].Angle[i] = angle[i];
        table[type].AngleMode = mode;
    }
    return true;
}

bool ApplySize(const std::string& value, int first, int last, PoseTable& table)
{
    std::vector<std::string> items;
    float height = 0.0f, scale = 0.0f;
    if (!TomlLines::SplitArray(value, items) || items.size() != 2) return false;
    if (!TomlLines::ParseFloat(items[0], height) || !TomlLines::ParseFloat(items[1], scale)) return false;
    for (int type = first; type <= last; ++type) {
        table[type].BodyHeight = height;
        table[type].Scale = scale;
        table[type].NegativeLevelScale = scale;
    }
    return true;
}

bool ApplyNegativeScale(const std::string& value, int first, int last, PoseTable& table)
{
    float scale = 0.0f;
    if (!TomlLines::ParseFloat(value, scale)) return false;
    for (int type = first; type <= last; ++type) table[type].NegativeLevelScale = scale;
    return true;
}

PoseTable Read()
{
    PoseTable table{};
    std::vector<TomlLines::Line> lines, bad;
    if (!TomlLines::Read("Content/item/inventory_pose.toml", lines, &bad)) {
        Warn("no se encontró Content/item/inventory_pose.toml; todos los items usan la pose por defecto");
        return table;
    }
    for (const auto& line : bad)
        Warn("línea " + std::to_string(line.number) + " inválida: " + line.value);
    for (const auto& line : lines) {
        int first = 0, last = 0;
        bool ok = ParseRange(line.key, first, last);
        if (ok) {
            if (line.section == "angle") ok = ApplyAngle(line.value, first, last, table);
            else if (line.section == "size") ok = ApplySize(line.value, first, last, table);
            else if (line.section == "negative_level_scale") ok = ApplyNegativeScale(line.value, first, last, table);
            else ok = false;
        }
        if (!ok) Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
    }
    return table;
}

} // namespace

const InventoryPose& InventoryPose_Get(int model)
{
    static const PoseTable table = Read();
    static const InventoryPose fallback{};
    return (model >= 0 && model < MODEL_MAX_VANILLA) ? table[model] : fallback;
}
