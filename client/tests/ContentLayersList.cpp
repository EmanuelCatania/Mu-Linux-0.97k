// Listado de carpetas a través de las capas (Fase 5, E2): lo que ve el código
// que recorre "Data/<carpeta>" tiene que coincidir con lo que después abre.
#include "platform/ContentLayers.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using namespace Platform::ContentLayers;

static int failures = 0;
#define CHECK(condition) do { if (!(condition)) { std::printf("FALLA linea %d: %s\n", __LINE__, #condition); ++failures; } } while (0)

static void Put(const fs::path& path, const char* text = "x")
{
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}

static const Entry* Find(const std::vector<Entry>& list, const char* name)
{
    for (const Entry& entry : list) if (entry.name == name) return &entry;
    return nullptr;
}

static std::string Read(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main()
{
    std::error_code error;
    const fs::path root = fs::temp_directory_path() / "mu_content_layers_list_test";
    fs::remove_all(root, error);
    fs::create_directories(root);
    fs::current_path(root);

    // Sin capas: la carpeta tal cual, ordenada, sin subcarpetas.
    Put("Data/Music/Noria.mp3", "data");
    Put("Data/Music/atlans.mp3", "data");
    Put("Data/Music/sub/dentro.mp3");
    Init();
    {
        const auto list = List("Data/Music");
        CHECK(list.size() == 2);
        CHECK(list.size() == 2 && list[0].name == "atlans.mp3" && list[1].name == "Noria.mp3");
        CHECK(list.size() == 2 && list[0].layer == 0 && Read(list[0].path) == "data");
        CHECK(List("Data/NoExiste").empty());
    }

    // Con Content, un pack y el server.
    Put("Content/alias.toml",
        "[alias]\n"
        "\"Music\" = \"audio/music/map\"\n"
        "\"Music/intro.mp3\" = \"audio/music/event/intro.mp3\"\n");
    Put("Content/audio/music/map/Lorencia.mp3", "content");
    Put("Content/audio/music/map/noria.mp3", "content");       // tapa al de Data
    Put("Content/audio/music/event/intro.mp3", "evento");      // alias de un archivo
    Put("Content/Music/suelto.mp3", "content-sin-alias");      // ruta vieja dentro de la capa
    Put("Content/monster/01-Bull/monster.toml", "source = \"Monster\"\n");
    Put("Content/monster/01-Bull/Monster01.bmd", "registro");
    Put("Data/Monster/Monster01.bmd", "data");
    Put("Data/Monster/Monster02.bmd", "data");
    Put("Packs/hd/Music/Lorencia.mp3", "pack");                // tapa al de Content
    Put("Packs/hd/Music/extra.mp3", "pack");
    Put("Servers/abc/audio/music/map/Atlans.mp3", "server");   // tapa a todos
    Init();
    SetServer("abc");
    {
        const auto list = List("Data\\Music\\");
        CHECK(list.size() == 6);
        const Entry* atlans = Find(list, "Atlans.mp3");
        CHECK(atlans && atlans->layer == 0 && Read(atlans->path) == "server");
        CHECK(!Find(list, "atlans.mp3"));
        const Entry* lorencia = Find(list, "Lorencia.mp3");
        CHECK(lorencia && lorencia->layer == 1 && Read(lorencia->path) == "pack");
        const Entry* extra = Find(list, "extra.mp3");
        CHECK(extra && extra->layer == 1);
        const Entry* noria = Find(list, "noria.mp3");
        CHECK(noria && noria->layer == 2 && Read(noria->path) == "content");
        CHECK(!Find(list, "Noria.mp3"));
        const Entry* intro = Find(list, "intro.mp3");
        CHECK(intro && intro->layer == 2 && Read(intro->path) == "evento");
        const Entry* loose = Find(list, "suelto.mp3");
        CHECK(loose && Read(loose->path) == "content-sin-alias");
        CHECK(!Find(list, "dentro.mp3"));
        // Lo listado es lo mismo que se abre: el primer candidato que existe.
        for (const Entry& entry : list) {
            std::string opened;
            for (const auto& candidate : Candidates(("Data/Music/" + entry.name).c_str()))
                if (fs::is_regular_file(candidate.path, error)) { opened = candidate.path; break; }
            CHECK(!opened.empty() && fs::equivalent(opened, entry.path, error));
        }
    }
    {
        // Registro de entidades: el archivo vive en la carpeta de la entidad.
        const auto list = List("Data/Monster");
        CHECK(list.size() == 2);
        const Entry* first = Find(list, "Monster01.bmd");
        CHECK(first && Read(first->path) == "registro");
        const Entry* second = Find(list, "Monster02.bmd");
        CHECK(second && second->layer == 3 && Read(second->path) == "data");
    }
    {
        // Fuera de Data no hay capas.
        const auto list = List("Packs/hd/Music");
        CHECK(list.size() == 2 && list[0].layer == 0);
    }
    {
        int data = 0, content = 0, pack = 0, server = 0, total = 0;
        Walk("Data", [&](const std::string& path) {
            const std::string generic = fs::path(path).generic_string();
            if (generic.rfind("Data/", 0) == 0) ++data;
            else if (generic.rfind("Content/", 0) == 0) ++content;
            else if (generic.rfind("Packs/", 0) == 0) ++pack;
            else if (generic.rfind("Servers/", 0) == 0) ++server;
            ++total;
            return true;
        });
        CHECK(data == 5 && content == 7 && pack == 2 && server == 1 && total == 15);
        int visited = 0;
        Walk("Data", [&](const std::string&) { return ++visited < 3; });
        CHECK(visited == 3);
        int only = 0;
        Walk("Packs", [&](const std::string&) { ++only; return true; });
        CHECK(only == 2);
    }

    fs::current_path(fs::temp_directory_path());
    fs::remove_all(root, error);
    if (failures) { std::printf("%d fallas\n", failures); return 1; }
    std::printf("ContentLayersList: ok\n");
    return 0;
}
