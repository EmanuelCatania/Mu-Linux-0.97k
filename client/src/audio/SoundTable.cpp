#include "stdafx.h"
#include "audio/SoundTable.h"
#include "functions.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
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

std::string Trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\r");
    return text.substr(first, last - first + 1);
}

// Saca el comentario: desde el primer # que no esté entre comillas.
std::string StripComment(const std::string& line)
{
    bool quoted = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') quoted = !quoted;
        else if (line[i] == '#' && !quoted) return line.substr(0, i);
    }
    return line;
}

void Warn(const std::string& message)
{
    DbgLogPublic(("SoundTable: " + message).c_str());
}

// Formato de cada línea:  id = ["archivo.wav", canales]  o  [..., canales, true]
bool ParseEntry(const std::string& line, SoundEntry& entry)
{
    const auto equal = line.find('=');
    const auto open = line.find('[', equal);
    const auto close = line.rfind(']');
    if (equal == std::string::npos || open == std::string::npos || close == std::string::npos || close < open)
        return false;

    char* end = nullptr;
    const std::string key = Trim(line.substr(0, equal));
    entry.id = static_cast<int>(std::strtol(key.c_str(), &end, 10));
    if (key.empty() || *end != '\0') return false;

    const std::string list = line.substr(open + 1, close - open - 1);
    const auto quote1 = list.find('"');
    const auto quote2 = list.find('"', quote1 + 1);
    if (quote1 == std::string::npos || quote2 == std::string::npos) return false;
    entry.file = list.substr(quote1 + 1, quote2 - quote1 - 1);

    std::vector<std::string> rest;
    std::string tail = list.substr(quote2 + 1);
    size_t start = 0;
    while (start <= tail.size()) {
        const auto comma = tail.find(',', start);
        const std::string item = Trim(tail.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!item.empty()) rest.push_back(item);
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    if (rest.empty() || rest.size() > 2) return false;
    entry.channels = static_cast<int>(std::strtol(rest[0].c_str(), &end, 10));
    if (*end != '\0' || entry.channels <= 0) return false;
    entry.spatial = rest.size() == 2 && rest[1] == "true";
    return rest.size() == 1 || rest[1] == "true" || rest[1] == "false";
}

SoundSections Read()
{
    SoundSections sections;
    std::ifstream file("Content/audio/sound/sounds.toml");
    if (!file) {
        Warn("no se encontró Content/audio/sound/sounds.toml; no se cargan sonidos con id");
        return sections;
    }
    std::string line, section;
    int number = 0;
    while (std::getline(file, line)) {
        ++number;
        line = Trim(StripComment(line));
        if (line.empty()) continue;
        if (line.front() == '[' && line.find('=') == std::string::npos) {
            section = Trim(line.substr(1, line.find(']') - 1));
            sections[section];
            continue;
        }
        SoundEntry entry;
        if (section.empty() || !ParseEntry(line, entry)) {
            Warn("línea " + std::to_string(number) + " inválida: " + line);
            continue;
        }
        sections[section].push_back(entry);
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
