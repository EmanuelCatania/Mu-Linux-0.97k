#pragma once

// Estado del usuario fuera de los datos del juego (Fase 5, E1).
//
// Config.ini, Macro.txt, Cache/, Screenshots/ y debug.log van a User/, al lado
// de Data/.  En Linux el lanzador ya corre el cliente desde la carpeta del
// usuario (MU_CLIENT_ROOT, ~/.local/share/mu97k): ahí no cambia nada.
//
// El Config.ini y el Data/Macro.txt que traen los paquetes quedan como
// plantillas: en el primer arranque se COPIAN a User/, y desde entonces el
// cliente sólo lee y escribe en User/.  Extraer una versión nueva encima pisa
// la plantilla y no las preferencias.  Cache/ y Screenshots/ son del usuario:
// se MUEVEN a User/ si todavía están en la raíz.

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include "Platform/IFileSystem.h"
#include "Platform/Text.h"

namespace Platform {

inline const char* UserDir()
{
    static const char* const dir = std::getenv("MU_CLIENT_ROOT") ? "" : "User/";
    return dir;
}

inline void UserPath(char* out, std::size_t capacity, const char* name)
{
    TextFormatSecure(out, capacity, "%s%s", UserDir(), name);
}

template <std::size_t N>
inline void UserPath(char (&out)[N], const char* name) { UserPath(out, N, name); }

// Macros de chat: en Linux siguen en Data\Macro.txt como hasta ahora.
inline const char* UserMacroPath()
{
    return *UserDir() ? "User/Macro.txt" : "Data\\Macro.txt";
}

namespace UserPathDetail {
inline bool FileExists(const char* path)
{
    FILE* f = std::fopen(path, "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

inline void CopyIfMissing(const char* from, const char* to)
{
    if (FileExists(to)) return;
    FILE* in = std::fopen(from, "rb");
    if (!in) return;
    FILE* out = std::fopen(to, "wb");
    if (out) {
        char buffer[8192];
        std::size_t n;
        while ((n = std::fread(buffer, 1, sizeof(buffer), in)) > 0) std::fwrite(buffer, 1, n, out);
        std::fclose(out);
    }
    std::fclose(in);
}
} // namespace UserPathDetail

// Crea User/ y trae lo que todavía está en la raíz.  Idempotente; se llama
// antes de abrir debug.log y antes de leer Config.ini.
inline void PrepareUserDir()
{
    static bool done = false;
    if (done) return;
    done = true;
    if (!*UserDir()) return;
    GetFileSystem().EnsureDirectory("User");
    UserPathDetail::CopyIfMissing("Config.ini", "User/Config.ini");
    UserPathDetail::CopyIfMissing("Data/Macro.txt", "User/Macro.txt");
    if (!GetFileSystem().Exists("User/Cache") && GetFileSystem().Exists("Cache"))
        std::rename("Cache", "User/Cache");
    if (!GetFileSystem().Exists("User/Screenshots") && GetFileSystem().Exists("Screenshots"))
        std::rename("Screenshots", "User/Screenshots");
}

} // namespace Platform
