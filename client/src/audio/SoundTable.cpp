#include "stdafx.h"
#include "audio/SoundTable.h"
#include "assets/data/TomlLines.h"
#include "functions.h"

#include <map>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

namespace {

struct SoundEntry {
    int         id;
    std::string file;
    int         channels;
    bool        spatial;
};

using SoundSections = std::map<std::string, std::vector<SoundEntry>>;

void Warn(const std::string& message)
{
    DbgLogPublic(("SoundTable: " + message).c_str());
}

// Formato de cada línea:  id = ["archivo.wav", canales]  o  [..., canales, true]
bool ParseEntry(const TomlLines::Line& line, SoundEntry& entry)
{
    std::vector<std::string> items;
    if (!TomlLines::ParseInt(line.key, entry.id) || !TomlLines::SplitArray(line.value, items))
        return false;
    if (items.size() < 2 || items.size() > 3) return false;
    if (!TomlLines::ParseString(items[0], entry.file) || !TomlLines::ParseInt(items[1], entry.channels))
        return false;
    entry.spatial = false;
    return entry.channels > 0 && (items.size() == 2 || TomlLines::ParseBool(items[2], entry.spatial));
}

SoundSections Read()
{
    SoundSections sections;
    std::vector<TomlLines::Line> lines, bad;
    if (!TomlLines::Read("Content/audio/sound/sounds.toml", lines, &bad)) {
        Warn("no se encontró Content/audio/sound/sounds.toml; no se cargan sonidos con id");
        return sections;
    }
    for (const auto& line : bad)
        Warn("línea " + std::to_string(line.number) + " inválida: " + line.value);
    for (const auto& line : lines) {
        SoundEntry entry;
        if (line.section.empty() || !ParseEntry(line, entry)) {
            Warn("línea " + std::to_string(line.number) + " inválida: " + line.key + " = " + line.value);
            continue;
        }
        sections[line.section].push_back(entry);
    }
    return sections;
}

const SoundSections& Sections()
{
    static const SoundSections sections = Read();
    return sections;
}

void LoadSection(const std::string& name)
{
    const auto found = Sections().find(name);
    if (found == Sections().end()) return;
    for (const SoundEntry& entry : found->second) {
        const std::string path = "Data\\Sound\\" + entry.file;
        LoadWaveFile(entry.id, path.c_str(), entry.channels, entry.spatial ? '\x01' : '\0');
    }
}

} // namespace

void SoundTable_LoadStartup()
{
    LoadSection("sound");
}

void SoundTable_LoadMonster(int type)
{
    LoadSection("monster." + std::to_string(type));
}

void SoundTable_LoadNpc(int model)
{
    LoadSection("npc." + std::to_string(model));
}

void SoundTable_LoadMap(int map)
{
    LoadSection("map." + std::to_string(map));
}
