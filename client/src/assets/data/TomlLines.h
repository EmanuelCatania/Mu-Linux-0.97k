#pragma once

// Lector mínimo de los .toml de datos del cliente: secciones [nombre] y líneas
// clave = valor, con comentarios #. Alcanza para tablas planas; no entiende
// tablas en línea ni strings de varias líneas.

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace TomlLines {

inline std::string Trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\r");
    return text.substr(first, last - first + 1);
}

// Saca el comentario: desde el primer # que no esté entre comillas.
inline std::string StripComment(const std::string& line)
{
    bool quoted = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') quoted = !quoted;
        else if (line[i] == '#' && !quoted) return line.substr(0, i);
    }
    return line;
}

struct Line {
    int         number;    // línea del archivo, para los avisos
    std::string section;   // sin corchetes; vacío antes de la primera sección
    std::string key;
    std::string value;     // tal cual, sin el comentario
};

// Devuelve false si el archivo no se pudo abrir. `bad` recibe las líneas que no
// son ni sección ni clave = valor.
inline bool Read(const char* path, std::vector<Line>& lines, std::vector<Line>* bad = nullptr)
{
    std::ifstream file(path);
    if (!file) return false;
    std::string text, section;
    int number = 0;
    while (std::getline(file, text)) {
        ++number;
        text = Trim(StripComment(text));
        if (text.empty()) continue;
        const auto equal = text.find('=');
        if (text.front() == '[' && equal == std::string::npos) {
            section = Trim(text.substr(1, text.find(']') - 1));
            continue;
        }
        if (equal == std::string::npos) {
            if (bad) bad->push_back({number, section, {}, text});
            continue;
        }
        lines.push_back({number, section, Trim(text.substr(0, equal)), Trim(text.substr(equal + 1))});
    }
    return true;
}

inline bool ParseInt(const std::string& text, int& value)
{
    if (text.empty()) return false;
    char* end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (*end != '\0') return false;
    value = static_cast<int>(parsed);
    return true;
}

inline bool ParseFloat(const std::string& text, float& value)
{
    if (text.empty()) return false;
    char* end = nullptr;
    value = std::strtof(text.c_str(), &end);
    return *end == '\0';
}

inline bool ParseBool(const std::string& text, bool& value)
{
    if (text == "true") { value = true; return true; }
    if (text == "false") { value = false; return true; }
    return false;
}

inline bool ParseString(const std::string& text, std::string& value)
{
    if (text.size() < 2 || text.front() != '"' || text.back() != '"') return false;
    value = text.substr(1, text.size() - 2);
    return value.find('"') == std::string::npos;
}

// [a, "b", c] -> {"a", "\"b\"", "c"}; los elementos no pueden llevar comas.
inline bool SplitArray(const std::string& text, std::vector<std::string>& items)
{
    if (text.size() < 2 || text.front() != '[' || text.back() != ']') return false;
    items.clear();
    const std::string inner = text.substr(1, text.size() - 2);
    size_t start = 0;
    while (start <= inner.size()) {
        const auto comma = inner.find(',', start);
        const std::string item = Trim(inner.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!item.empty()) items.push_back(item);
        else if (comma != std::string::npos) return false;
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return true;
}

} // namespace TomlLines
