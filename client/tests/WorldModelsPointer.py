#!/usr/bin/env python3
"""Cargas de OpenWorldModels por mapa (Content/map/maps.toml) con el código real.

Compila el tramo de OpenWorldModels que corre en cada entrada a un mapa (desde
SetMaxTextures(0x69) hasta los objetos del mapa, sin el bloque Data2 ni los
ajustes de campos de modelo) junto con los lectores reales de maps.toml y de
sounds.toml. AccessModel, OpenTexture, OpenJPG, LoadWaveFile y SetMaxTextures
registran cada llamada; se corre para los mapas 0..16.

Comprueba que los lectores no rechacen líneas. Con --reference-ref REV compila
además ese tramo tal como estaba en REV (el switch por mapa y la tabla de
Lorencia en código) y exige la misma secuencia de llamadas, mapa por mapa. Las
cargas se generaron así desde el commit de la primera parte de T7.
"""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from MonsterSpawnPointer import build

ROOT = Path(__file__).resolve().parents[1]

COMMON = r'''
#include <cstdio>
#include <cstdint>
#include <cstring>
enum { MAP_LORENCIA, MAP_DUNGEON, MAP_DEVIAS, MAP_NORIA, MAP_LOST_TOWER, MAP_EXILE, MAP_ARENA, MAP_ATLANS,
       MAP_TARKAN, MAP_DEVIL_SQUARE1, MAP_ICARUS, MAP_BLOOD_CASTLE1, MAP_BLOOD_CASTLE2, MAP_BLOOD_CASTLE3,
       MAP_BLOOD_CASTLE4, MAP_BLOOD_CASTLE5, MAP_BLOOD_CASTLE6 };
static bool IsBloodCastleMap(int m) { return m >= 11 && m <= 16; }
static void AccessModel(int slot, const char* dir, const char* name, int index)
{ std::printf("M|%d|%s|%s|%d\n", slot, dir, name, index); }
static void OpenTexture(int slot, const char* dir, int filter, char flag)
{ std::printf("T|%d|%s|%d|%d\n", slot, dir, filter, (int)flag); }
static void OpenJPG(const char* file, int slot, int a, int b, int c, int d)
{ std::printf("J|%s|%d|%d|%d|%d|%d\n", file, slot, a, b, c, d); }
static void LoadWaveFile(int id, const char* file, int ch, int d3)
{ std::printf("W|%d|%s|%d|%d\n", id, file, ch, d3); }
static void SetMaxTextures(int n) { std::printf("S|%d\n", n); }
static void crt_sprintf(char* out, const char* fmt, int v) { std::sprintf(out, fmt, v); }
namespace Platform {
inline int TruncateText = 0;
inline void TextFormatTruncated(char* out, size_t size, int, const char* fmt, int v) { std::snprintf(out, size, fmt, v); }
inline void TextCopy(char* dst, const char* src, size_t) { std::printf("C|%d|%s\n", (int)(intptr_t)dst, src); }
}
constexpr int TEXTURE_STRIDE = 1;
static char* g_BitmapsRaw = nullptr;
static char DAT_0055a7c4 = 1;   // sólo lo que corre en cada entrada al mapa
static int DAT_083a4100;
struct ModelStub { short NumBones; };
static ModelStub g_model{};
static ModelStub* AsModel(char*) { return &g_model; }
static uintptr_t DAT_05828d58 = 0;
constexpr uintptr_t BMD_STRIDE = 0;
'''


def cut(text, start, data2, end):
    """El tramo [start, end) sin el bloque Data2 (de `data2` a "Register data paths")."""
    a = text.index(start)
    b = text.index(data2, a)
    c = text.index("        // Register data paths for all 0xa0 object slots\n", b)
    d = text.index(end, c)
    return text[a:b] + text[c:d] + "    }\n"


def reference_harness(revision):
    text = subprocess.check_output(["git", "show", f"{revision}:client/src/game/terrain/Terrain_LegacyLoad.cpp"],
                                   cwd=ROOT).decode("utf-8")
    body = cut(text, "    SetMaxTextures(0x69);\n", "        // Dynamic map: load from per-map object file\n",
               "        // Map-specific post-load fixups\n")
    return (COMMON + "struct MapStub { int map; int GetCurrentMap() const { return map; } } gMapManager;\n"
            "static void Run() {\n" + body + "}\n"
            "int main() { for (int m = 0; m <= 16; ++m) { gMapManager.map = m; std::printf(\"=|%d\\n\", m); Run(); } }\n")


def inline(path, *headers):
    text = (ROOT / path).read_text(encoding="utf-8")
    for header in headers:
        body = (ROOT / "src" / header).read_text(encoding="utf-8").replace("#pragma once", "")
        text = text.replace(f'#include "{header}"', body)
    text = text.replace('#include "stdafx.h"', "").replace('#include "functions.h"', "")
    text = text.replace('extern "C" void DbgLogPublic(const char* msg);', "")
    text = text.replace('#include "assets/data/TomlLines.h"', "")
    text = re.sub(r"^#include <[^>]+>\s*$", "", text, flags=re.M)
    return text


def current_harness():
    text = (ROOT / "src/game/terrain/Terrain_LegacyLoad.cpp").read_text(encoding="utf-8")
    body = cut(text, "    // Cargas propias de cada mapa", "        // Dynamic map: load from per-map object file\n",
               "        // Map-specific post-load fixups\n")
    toml = (ROOT / "src/assets/data/TomlLines.h").read_text(encoding="utf-8").replace("#pragma once", "")
    maps = inline("src/game/core/MapInfoTable.cpp", "game/core/MapInfoTable.h")
    sound = inline("src/audio/SoundTable.cpp", "audio/SoundTable.h")
    return ("#include <algorithm>\n#include <bitset>\n#include <map>\n#include <string>\n#include <utility>\n"
            "#include <vector>\n" + COMMON + toml +
            "static void DbgLogPublic(const char* msg) { std::printf(\"WARN|%s\\n\", msg); }\n" +
            "namespace maps_tu {\n" + maps + "\n}\nnamespace sound_tu {\n" + sound + "\n}\n"
            "using namespace maps_tu;\nusing sound_tu::SoundTable_LoadMap;\n"
            "struct MapStub { int map; int GetCurrentMap() const { return map; }\n"
            "    const MapInfo& Info(int m) const { return MapInfo_Get(m); } } gMapManager;\n"
            "static void Run() {\n" + body + "}\n"
            "int main() { for (int m = 0; m <= 16; ++m) { gMapManager.map = m; std::printf(\"=|%d\\n\", m); Run(); } }\n")


def normalize(rows):
    return [r.replace("\\", "/") for r in rows]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-world-models-") as temp:
        data = Path(temp) / "data"
        for name in ("map/maps.toml", "audio/sound/sounds.toml"):
            (data / "Content" / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "bin/Client/Content" / name, data / "Content" / name)
        rows = normalize(build(Path(temp) / "current", current_harness(), args, cwd=data))
        warnings = [r for r in rows if r.startswith("WARN|")]
        if warnings:
            raise RuntimeError("los lectores rechazaron líneas:\n" + "\n".join(warnings))
        maps = sum(1 for r in rows if r.startswith("=|"))
        print(f"WorldModels: {maps} mapas, {len(rows) - maps} llamadas")
        if args.reference_ref:
            reference = normalize(build(Path(temp) / "reference", reference_harness(args.reference_ref), args))
            if reference != rows:
                for a, b in zip(reference, rows):
                    if a != b:
                        raise RuntimeError(f"difiere:\n  código {a}\n  tabla  {b}")
                raise RuntimeError(f"cantidad distinta: código {len(reference)} / tabla {len(rows)}")
            print(f"Equivalencia con {args.reference_ref}: OK ({len(rows) - maps} llamadas)")


if __name__ == "__main__":
    main()
