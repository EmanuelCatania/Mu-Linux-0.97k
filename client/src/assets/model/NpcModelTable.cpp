#include "stdafx.h"
#include "assets/model/NpcModelTable.h"
#include "assets/data/TomlLines.h"

#include <map>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

namespace {

void Warn(const std::string& message)
{
    DbgLogPublic(("NpcModel: " + message).c_str());
}

bool ParseField(const std::string& key, const std::string& value, NpcModel& npc)
{
    if (key == "model") {
        std::vector<std::string> items;
        return TomlLines::SplitArray(value, items) && items.size() == 2
            && TomlLines::ParseString(items[0], npc.Model) && TomlLines::ParseInt(items[1], npc.Index);
    }
    if (key == "part_count") return TomlLines::ParseInt(value, npc.PartCount);
    if (key == "texture") return TomlLines::ParseBool(value, npc.Texture);
    if (key == "marks_loaded") return TomlLines::ParseBool(value, npc.MarksLoaded);
    if (key.rfind("part.", 0) == 0 && key.size() > 5) {
        int base = 0;
        if (!TomlLines::ParseInt(value, base)) return false;
        npc.Parts.emplace_back(key.substr(5), base);
        return true;
    }
    return false;
}

std::map<int, NpcModel> Read()
{
    std::map<int, NpcModel> models;
    std::vector<TomlLines::Line> lines, bad;
    if (!TomlLines::Read("Content/npc/models.toml", lines, &bad)) {
        Warn("no se encontró Content/npc/models.toml; no se cargan modelos de NPC");
        return models;
    }
    for (const auto& line : bad)
        Warn("línea " + std::to_string(line.number) + " inválida: " + line.value);
    for (const auto& line : lines) {
        int model = -1;
        const bool inNpc = line.section.rfind("npc.", 0) == 0
                        && TomlLines::ParseInt(line.section.substr(4), model) && model >= 0;
        if (!inNpc || !ParseField(line.key, line.value, models[model]))
            Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
    }
    for (auto it = models.begin(); it != models.end();) {
        if (it->second.Model.empty()) {
            Warn("[npc." + std::to_string(it->first) + "] sin model");
            it = models.erase(it);
        } else {
            ++it;
        }
    }
    return models;
}

} // namespace

const NpcModel* NpcModel_Get(int model)
{
    static const std::map<int, NpcModel> models = Read();
    const auto found = models.find(model);
    return found == models.end() ? nullptr : &found->second;
}
