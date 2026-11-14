#!/usr/bin/env python3
"""Propiedades de los mapas (Content/map/maps.toml) contra las reglas del código.

Compila el lector real (src/game/core/MapInfoTable.cpp) con el maps.toml del
repo, vuelca cada propiedad para los mapas -1..40 y los 256 valores de HeroTile,
y la compara con la expresión que el código tenía antes de pasar a datos:

    mount_flies     World == 8 || World == 10           (Dinorant a +90)
    auto_attack     World != 6                          (arena)
    weather         World en 0, 2, 3, 7, 9, 10          (Game_CharSelectTick)
    indoor_tiles    Lorencia: HeroTile == 4; Devias: HeroTile == 3 || >= 10
    ambient         el loop de Game_MainLoop que corta 0,1,2,3,5,6,7,20
    swimmable, music, music_safe_zone_only, mini_map    (constructor de CMapManager)

Un cambio deliberado en maps.toml tiene que venir con su cambio acá.
"""
import argparse
import shutil
import tempfile
from pathlib import Path

from MonsterSpawnPointer import build

ROOT = Path(__file__).resolve().parents[1]
TOML = ROOT / "bin/Client/Content/map/maps.toml"
MAPS = range(-1, 41)

# Qué mapa deja sonar cada sonido ambiental (el resto lo corta).
AMBIENT_KEEP = {0: {0, 2, 3}, 1: {0, 9}, 3: {1}, 2: {3}, 5: {4}, 6: {7}, 7: {8}, 20: {10}}
MUSIC = {0: "Lorencia.mp3", 1: "Dungeon.mp3", 2: "Devias.mp3", 3: "Noria.mp3", 5: "Dungeon.mp3"}


def expected(m):
    stopped = sorted(s for s, keep in AMBIENT_KEEP.items() if m not in keep)
    indoor = "".join("1" if (m == 0 and t == 4) or (m == 2 and (t == 3 or t >= 10)) else "0" for t in range(256))
    return "|".join(str(v) for v in (
        m, int(m == 7), int(m in (8, 10)), int(m != 6), int(m in (0, 2, 3, 7, 9, 10)),
        ",".join(map(str, stopped)), indoor, MUSIC.get(m, ""), int(m in (0, 2, 3)),
        2 if 0 <= m <= 16 else 0, ""))


HARNESS = r'''
#include <algorithm>
#include <cstdio>
static void DbgLogPublic(const char* msg) { std::printf("WARN|%s\n", msg); }
SOURCE
int main()
{
    for (int m = -1; m <= 40; ++m) {
        const MapInfo& info = MapInfo_Get(m);
        std::string stopped, indoor;
        std::vector<int> all = MapInfo_AllAmbient();
        std::sort(all.begin(), all.end());
        for (int sound : all)
            if (std::find(info.ambient.begin(), info.ambient.end(), sound) == info.ambient.end())
                stopped += (stopped.empty() ? "" : ",") + std::to_string(sound);
        for (int t = 0; t < 256; ++t) indoor += info.IsIndoor(t) ? '1' : '0';
        std::printf("%d|%d|%d|%d|%d|%s|%s|%s|%d|%d|%s\n", m, info.swimmable, info.mountFlies, info.autoAttack,
                    info.weather, stopped.c_str(), indoor.c_str(), info.music.c_str(), info.musicSafeZoneOnly,
                    (int)info.miniMap, info.name.c_str());
    }
}
'''


def harness():
    source = (ROOT / "src/game/core/MapInfoTable.cpp").read_text(encoding="utf-8")
    for header in ("game/core/MapInfoTable.h", "assets/data/TomlLines.h"):
        text = (ROOT / "src" / header).read_text(encoding="utf-8").replace("#pragma once", "")
        source = source.replace(f'#include "{header}"', text)
    source = source.replace('#include "stdafx.h"', "").replace('extern "C" void DbgLogPublic(const char* msg);', "")
    return HARNESS.replace("SOURCE", source)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-map-info-") as temp:
        data = Path(temp) / "data"
        (data / "Content/map").mkdir(parents=True)
        shutil.copyfile(TOML, data / "Content/map/maps.toml")
        rows = build(Path(temp) / "current", harness(), args, cwd=data)
        warnings = [r for r in rows if r.startswith("WARN|")]
        if warnings:
            raise RuntimeError("el lector rechazó líneas:\n" + "\n".join(warnings))
        for row, m in zip(rows, MAPS):
            if row != expected(m):
                raise RuntimeError(f"mapa {m} difiere:\n  tabla   {row[:200]}\n  código  {expected(m)[:200]}")
        if len(rows) != len(MAPS):
            raise RuntimeError("cantidad de mapas distinta")
        print(f"MapInfo: {len(rows)} mapas x 256 tiles, igual a las reglas que tenía el código")


if __name__ == "__main__":
    main()
