#include "platform/ContentLayers.h"
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
    bool system = false;                   // existe System/
    bool legacy = false;                   // existe Data/ (instalación vieja)
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
    // (carpeta de la entidad, nombre del .toml) a cualquier profundidad dentro
    // de cada tipo: monster/01-BullFighter/, item/00-swords/000/.
    std::vector<std::pair<fs::path, std::string>> folders;
    for (fs::directory_iterator type("Content", error), end; !error && type != end; type.increment(error)) {
        if (!type->is_directory(error)) continue;
        const std::string toml = Lower(type->path().filename().string() + ".toml");
        for (fs::recursive_directory_iterator it(type->path(), fs::directory_options::skip_permission_denied, error), stop;
             !error && it != stop; it.increment(error))
            if (it->is_regular_file(error) && Lower(it->path().filename().string()) == toml)
                folders.emplace_back(it->path().parent_path(), it->path().filename().string());
    }
    std::sort(folders.begin(), folders.end(), [](const auto& a, const auto& b) {
        return Lower(a.first.generic_string()) < Lower(b.first.generic_string());
    });
    for (const auto& [folder, toml] : folders) {
        const std::string source = ReadSource(folder / toml);
        if (source.empty()) continue;
        // Ruta relativa a Content/, como la usan las capas.
        std::string target = Normalize(folder.lexically_relative("Content").generic_string());
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
    state->system = IsDirectory("System");
    state->legacy = IsDirectory("Data");
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
    if (!path || (state->server.empty() && state->packs.empty() && !state->content && !state->system)) return {};
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
    // La base: System/ y, si quedó, el Data/ de una instalación vieja. Comparten
    // capa: las dos son "el clásico" para la regla de las -hd.
    if (state->system) out.push_back({ "System/" + inner, layer });
    if (state->legacy || !state->system) out.push_back({ path, layer });
    return out;
}

namespace {
std::shared_ptr<const State> Current()
{
    std::lock_guard<std::mutex> lock(mutex);
    return current;
}

std::string Real(const std::string& path, const Resolver& resolve)
{
    return resolve ? resolve(path) : path;
}

// Último componente de una ruta con '/'.
std::string Leaf(const std::string& path)
{
    const auto slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

// Si `key` es "<folder>/<archivo>" sin más carpetas, el nombre del archivo.
bool DirectChild(const std::string& key, const std::string& folder)
{
    return key.size() > folder.size() + 1 && key.compare(0, folder.size(), folder) == 0 &&
        key[folder.size()] == '/' && key.find('/', folder.size() + 1) == std::string::npos;
}
} // namespace

std::vector<Entry> List(const char* dir, const Resolver& resolve)
{
    std::vector<Entry> out;
    if (!dir) return out;
    const auto state = Current();
    std::unordered_map<std::string, size_t> seen;
    auto addFile = [&](const std::string& name, const std::string& path, int layer) {
        if (seen.emplace(Lower(name), out.size()).second) out.push_back({ name, path, layer });
    };
    auto addFolder = [&](const std::string& folder, int layer) {
        const std::string real = Real(folder, resolve);
        if (real.empty()) return;
        std::error_code error;
        for (fs::directory_iterator it(real, error), end; !error && it != end; it.increment(error)) {
            if (!it->is_regular_file(error)) { error.clear(); continue; }
            const std::string name = it->path().filename().string();
            addFile(name, Normalize(real) + "/" + name, layer);
        }
    };
    auto addMoved = [&](const std::string& path, int layer) {
        const std::string real = Real(path, resolve);
        std::error_code error;
        if (!real.empty() && fs::is_regular_file(real, error)) addFile(Leaf(path), real, layer);
    };

    const std::string normalized = Normalize(dir);
    const std::string lowerDir = Lower(normalized);
    const bool layered = (lowerDir == "data" || lowerDir.compare(0, 5, "data/") == 0) &&
        (!state->server.empty() || !state->packs.empty() || state->content || state->system);
    int layer = 0;
    if (layered) {
        const std::string inner = normalized.size() > 5 ? normalized.substr(5) : std::string();
        const std::string lower = Lower(inner);
        // Archivos de esta carpeta que viven en otra: registro de entidades y
        // alias de un archivo solo. Ganan sobre la carpeta, como en Candidates.
        std::vector<std::string> moved;
        if (!lower.empty()) {
            for (const auto& [from, to] : state->files) if (DirectChild(from, lower)) moved.push_back(to);
            for (const auto& [from, to] : state->aliases) if (DirectChild(from, lower)) moved.push_back(to);
            std::sort(moved.begin(), moved.end());
        }
        std::string aliased;
        for (const auto& [from, to] : state->aliases) {
            if (lower.empty() || lower.compare(0, from.size(), from) != 0) continue;
            if (lower.size() > from.size() && lower[from.size()] != '/') continue;
            aliased = to + inner.substr(from.size());
            break;
        }
        auto addRoot = [&](const std::string& root) {
            for (const std::string& path : moved) addMoved(root + "/" + path, layer);
            if (!aliased.empty()) addFolder(root + "/" + aliased, layer);
            addFolder(inner.empty() ? root : root + "/" + inner, layer);
            ++layer;
        };
        if (!state->server.empty()) addRoot(state->server);
        for (const auto& pack : state->packs) addRoot(pack);
        if (state->content) addRoot("Content");
        if (state->system) addFolder(inner.empty() ? std::string("System") : "System/" + inner, layer);
    }
    if (!layered || state->legacy || !state->system) addFolder(dir, layer);
    std::sort(out.begin(), out.end(), [](const Entry& a, const Entry& b) {
        const std::string la = Lower(a.name), lb = Lower(b.name);
        return la != lb ? la < lb : a.name < b.name;
    });
    return out;
}

void Walk(const char* root, const std::function<bool(const std::string&)>& visit, const Resolver& resolve)
{
    if (!root || !visit) return;
    const auto state = Current();
    std::vector<std::string> roots;
    if (Lower(Normalize(root)) == "data") {
        if (!state->server.empty()) roots.push_back(state->server);
        roots.insert(roots.end(), state->packs.begin(), state->packs.end());
        if (state->content) roots.push_back("Content");
        if (state->system) roots.push_back("System");
        if (state->legacy || !state->system) roots.push_back(root);
    } else {
        roots.push_back(root);
    }
    for (const std::string& folder : roots) {
        const std::string real = Real(folder, resolve);
        if (real.empty()) continue;
        std::error_code error;
        fs::recursive_directory_iterator it(real, fs::directory_options::skip_permission_denied, error), end;
        for (; !error && it != end; it.increment(error)) {
            if (!it->is_regular_file(error)) { error.clear(); continue; }
            if (!visit(it->path().string())) return;
        }
    }
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
    if (state->system) text += " System";
    if (state->legacy) text += " Data(instalación vieja: migrar con tools/migrar_data.py)";
    return text;
}
} // namespace Platform::ContentLayers
