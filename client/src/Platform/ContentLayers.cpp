#include "Platform/ContentLayers.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Platform::ContentLayers {
namespace {
namespace fs = std::filesystem;

struct State {
    std::string server;                    // "Servers/<id>" o vacío
    std::vector<std::string> packs;        // "Packs/<nombre>"
    bool content = false;                  // existe Content/
    // Alias en minúsculas y con '/', ordenados del prefijo más largo al más corto.
    std::vector<std::pair<std::string, std::string>> aliases;
    // Alias de archivo generados por el registro (ver ReadEntities):
    // "monster/monster01.bmd" -> "monster/01-BullFighter/Monster01.bmd".
    std::unordered_map<std::string, std::string> files;
    int entities = 0;
};

std::mutex mutex;
std::shared_ptr<const State> current = std::make_shared<State>();

std::string Lower(std::string text)
{
    for (char& c : text) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return text;
}

// Separadores a '/', sin barras repetidas ni al principio y al final.
std::string Normalize(const std::string& path)
{
    std::string out;
    for (char c : path) {
        if (c == '\\') c = '/';
        if (c == '/' && (out.empty() || out.back() == '/')) continue;
        out += c;
    }
    while (!out.empty() && out.back() == '/') out.pop_back();
    return out;
}

std::string Trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

std::string Unquote(std::string text)
{
    text = Trim(text);
    if (text.size() >= 2 && (text.front() == '"' || text.front() == '\'') && text.back() == text.front())
        return text.substr(1, text.size() - 2);
    return text;
}

// Content/alias.toml: sección [alias] con líneas "vieja" = "nueva", rutas
// relativas al interior de Data y de cada capa.
void ReadAliases(State& state)
{
    std::ifstream file("Content/alias.toml");
    std::string line, section;
    while (std::getline(file, line)) {
        const auto hash = line.find('#');
        if (hash != std::string::npos && line.find('"') > hash) line.erase(hash);
        line = Trim(line);
        if (line.empty()) continue;
        if (line.front() == '[') { section = Lower(Trim(line.substr(1, line.find(']') - 1))); continue; }
        const auto equal = line.find('=');
        if (section != "alias" || equal == std::string::npos) continue;
        const std::string from = Lower(Normalize(Unquote(line.substr(0, equal))));
        const std::string to = Normalize(Unquote(line.substr(equal + 1)));
        if (!from.empty() && !to.empty()) state.aliases.emplace_back(from, to);
    }
    std::stable_sort(state.aliases.begin(), state.aliases.end(),
        [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });
}

bool IsDirectory(const fs::path& path)
{
    std::error_code error;
    return fs::is_directory(path, error);
}

// `source = "..."` antes de la primera sección de un .toml de entidad.
std::string ReadSource(const fs::path& toml)
{
    std::ifstream file(toml);
    std::string line;
    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.empty() || line.front() == '#') continue;
        if (line.front() == '[') break;
        const auto equal = line.find('=');
        if (equal != std::string::npos && Lower(Trim(line.substr(0, equal))) == "source")
            return Normalize(Unquote(line.substr(equal + 1)));
    }
    return {};
}

// Registro (Fase 5): cada entidad es una carpeta Content/<tipo>/<nombre>/ con
// un <tipo>.toml que dice qué carpeta de Data reemplaza (`source = "Monster"`).
// Cada archivo de la carpeta pasa a responder por "<source>/<archivo>", así el
// código sigue pidiendo "Data\Monster\Monster01.bmd". Si dos entidades traen
// el mismo archivo gana la primera en orden alfabético.
void ReadEntities(State& state)
{
    std::error_code error;
    std::vector<fs::path> types, folders;
    for (fs::directory_iterator it("Content", error), end; !error && it != end; it.increment(error))
        if (it->is_directory(error)) types.push_back(it->path());
    for (const auto& type : types) {
        const std::string toml = type.filename().string() + ".toml";
        for (fs::directory_iterator it(type, error), end; !error && it != end; it.increment(error))
            if (it->is_directory(error) && fs::is_regular_file(it->path() / toml, error)) folders.push_back(it->path());
    }
    std::sort(folders.begin(), folders.end(),
        [](const fs::path& a, const fs::path& b) { return Lower(a.generic_string()) < Lower(b.generic_string()); });
    for (const auto& folder : folders) {
        const std::string source = ReadSource(folder / (folder.parent_path().filename().string() + ".toml"));
        if (source.empty()) continue;
        // Ruta relativa a Content/, como la usan las capas.
        const std::string target = folder.parent_path().filename().string() + "/" + folder.filename().string();
        for (fs::directory_iterator it(folder, error), end; !error && it != end; it.increment(error)) {
            if (!it->is_regular_file(error)) continue;
            const std::string name = it->path().filename().string();
            if (Lower(it->path().extension().string()) == ".toml") continue;
            state.files.emplace(Lower(source + "/" + name), target + "/" + name);
        }
        ++state.entities;
    }
}
} // namespace

void Init()
{
    auto state = std::make_shared<State>();
    std::error_code error;
    if (IsDirectory("Packs")) {
        for (fs::directory_iterator it("Packs", error), end; !error && it != end; it.increment(error))
            if (it->is_directory(error)) state->packs.push_back("Packs/" + it->path().filename().string());
        std::sort(state->packs.begin(), state->packs.end(),
            [](const std::string& a, const std::string& b) { return Lower(a) < Lower(b); });
    }
    state->content = IsDirectory("Content");
    if (state->content) { ReadAliases(*state); ReadEntities(*state); }
    std::lock_guard<std::mutex> lock(mutex);
    state->server = current->server;
    current = state;
}

void SetServer(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex);
    std::string server;
    if (!id.empty() && IsDirectory(fs::path("Servers") / id)) server = "Servers/" + id;
    if (server == current->server) return;
    auto state = std::make_shared<State>(*current);
    state->server = server;
    current = state;
}

std::vector<Candidate> Candidates(const char* path)
{
    std::shared_ptr<const State> state;
    {
        std::lock_guard<std::mutex> lock(mutex);
        state = current;
    }
    if (!path || (state->server.empty() && state->packs.empty() && !state->content)) return {};
    const std::string normalized = Normalize(path);
    // Sólo "Data/...": relativas al directorio del cliente.
    if (normalized.size() < 6 || Lower(normalized.substr(0, 5)) != "data/") return {};
    const std::string inner = normalized.substr(5);
    const std::string lower = Lower(inner);

    std::string aliased;
    if (const auto file = state->files.find(lower); file != state->files.end()) aliased = file->second;
    if (aliased.empty()) for (const auto& [from, to] : state->aliases) {
        if (lower.compare(0, from.size(), from) != 0) continue;
        if (lower.size() > from.size() && lower[from.size()] != '/') continue;
        aliased = to + inner.substr(from.size());
        break;
    }

    std::vector<Candidate> out;
    int layer = 0;
    auto add = [&](const std::string& root) {
        if (!aliased.empty()) out.push_back({ root + "/" + aliased, layer });
        out.push_back({ root + "/" + inner, layer });
        ++layer;
    };
    if (!state->server.empty()) add(state->server);
    for (const auto& pack : state->packs) add(pack);
    if (state->content) add("Content");
    out.push_back({ path, layer });
    return out;
}

std::string Describe()
{
    std::shared_ptr<const State> state;
    {
        std::lock_guard<std::mutex> lock(mutex);
        state = current;
    }
    std::string text = "Capas:";
    if (!state->server.empty()) text += " " + state->server;
    for (const auto& pack : state->packs) text += " " + pack;
    if (state->content)
        text += " Content(" + std::to_string(state->aliases.size()) + " alias, " +
            std::to_string(state->entities) + " entidades, " + std::to_string(state->files.size()) + " archivos)";
    text += " Data";
    return text;
}
} // namespace Platform::ContentLayers
