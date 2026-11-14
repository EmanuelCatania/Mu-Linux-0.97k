#!/usr/bin/env python3
"""Carga de modelos de NPC (Content/npc/models.toml) con el OpenNpc real.

Compila el bloque de OpenNpc que usa la tabla, junto con NpcModelTable.cpp y
SoundTable.cpp, con AccessModelWithTextures / OpenTexture / LoadWaveFile y el
flag DAT_0055a7c4 registrando cada llamada, y corre los modelos 0..1023.
Comprueba que no haya líneas rechazadas y que cada modelo de NPC exista.

Con --reference-ref REV compila además el switch de OpenNpc tal como estaba en
REV y exige la misma secuencia de llamadas. La tabla se generó así desde el
commit de T4 del refactor.
"""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from MonsterSpawnPointer import build

ROOT = Path(__file__).resolve().parents[1]

STUBS = r'''
#include <cstdio>
static int g;
struct FlagProxy {
    int value = 0;
    FlagProxy& operator=(char v) { value = v; std::printf("%d|F|%d\n", g, (int)v); return *this; }
    operator int() const { return value; }
};
static FlagProxy flag;
#define DAT_0055a7c4 flag
static void AccessModelWithTextures(int id, const char* path, const char* name, int idx)
{ std::printf("%d|A|%d|%s|%s|%d\n", g, id, path, name, idx); }
static void OpenTexture(int id, void* path, int filter, bool b)
{ std::printf("%d|T|%d|%s|%d|%d\n", g, id, (const char*)path, filter, (int)b); }
static void LoadWaveFile(int id, const char* path, int ch, char d3)
{ std::printf("%d|W|%d|%s|%d|%d\n", g, id, path, ch, (int)d3); }
static void DbgLogPublic(const char* msg) { std::printf("WARN|%s\n", msg); }
'''


def block(text, start):
    i = text.index(start)
    depth, k = 0, text.index("{", i)
    for j in range(k, len(text)):
        depth += {"{": 1, "}": -1}.get(text[j], 0)
        if depth == 0:
            return text[i:j + 1]


def inline(path, *headers):
    source = (ROOT / path).read_text(encoding="utf-8").replace('#include "stdafx.h"', "")
    source = source.replace('#include "functions.h"', "")
    source = source.replace('extern "C" void DbgLogPublic(const char* msg);', "")
    for header in headers:
        text = (ROOT / "src" / header).read_text(encoding="utf-8").replace("#pragma once", "")
        source = source.replace(f'#include "{header}"', text)
    return source


def current_harness():
    toml = (ROOT / "src/assets/data/TomlLines.h").read_text(encoding="utf-8").replace("#pragma once", "")
    # Los dos lectores van al mismo archivo: cada uno en su namespace para que sus
    # helpers (Warn, Read) no choquen. Los includes estándar quedan arriba.
    def isolate(text, name):
        text = text.replace('#include "assets/data/TomlLines.h"', "")
        text = re.sub(r"^#include <[^>]+>\s*$", "", text, flags=re.M)
        return f"namespace {name} {{\n{text}\n}}\n"
    sound = isolate(inline("src/audio/SoundTable.cpp", "audio/SoundTable.h"), "sound_tu")
    npc = isolate(inline("src/assets/model/NpcModelTable.cpp", "assets/model/NpcModelTable.h"), "npc_tu")
    body = block((ROOT / "src/assets/model/NPC_ModelLoad.cpp").read_text(encoding="utf-8"),
                 "    if (const NpcModel* npc = NpcModel_Get(Type)) {")
    return ("#include <map>\n#include <string>\n#include <utility>\n#include <vector>\n" + toml + STUBS +
            sound + npc + "using sound_tu::SoundTable_LoadNpc;\nusing npc_tu::NpcModel;\n"
            "using npc_tu::NpcModel_Get;\nstatic void Run(int Type) {\n" + body +
            "\n}\nint main() { for (g = 0; g < 1024; ++g) Run(g); }\n")


def reference_harness(revision):
    text = subprocess.check_output(["git", "show", f"{revision}:client/src/assets/model/NPC_ModelLoad.cpp"],
                                   cwd=ROOT).decode("utf-8")
    return STUBS + "static void Run(int Type) {\n" + block(text, "    switch (Type) {") + \
        "\n}\nint main() { for (g = 0; g < 1024; ++g) Run(g); }\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-npc-model-") as temp:
        data = Path(temp) / "data"
        for name in ("npc/models.toml", "audio/sound/sounds.toml"):
            (data / "Content" / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "bin/Client/Content" / name, data / "Content" / name)
        rows = build(Path(temp) / "current", current_harness(), args, cwd=data)
        warnings = [r for r in rows if r.startswith("WARN|")]
        if warnings:
            raise RuntimeError("los lectores rechazaron líneas:\n" + "\n".join(warnings))
        files = {p.name.lower() for p in (ROOT / "bin/Client").rglob("*.bmd")}
        # Sólo el modelo principal: las partes piden una variante 03 que el pack no
        # trae, igual que el binario (el loop va hasta 3 y la carga falla sin aviso).
        loads = [r.split("|") for r in rows if r.split("|")[1] == "A" and r.split("|")[0] == r.split("|")[2]]
        missing = sorted({f"{r[4]}{r[5]:0>2}.bmd" for r in loads
                          if f"{r[4]}{r[5]:0>2}.bmd".lower() not in files and f"{r[4]}{r[5]}.bmd".lower() not in files})
        if missing:
            raise RuntimeError(f"modelos que no están: {missing}")
        npcs = len({r.split("|")[0] for r in rows})
        print(f"NpcModel: {npcs} modelos de NPC, {len(rows)} llamadas, modelos presentes")
        if args.reference_ref:
            reference = build(Path(temp) / "reference", reference_harness(args.reference_ref), args)
            if reference != rows:
                for a, b in zip(reference, rows):
                    if a != b:
                        raise RuntimeError(f"difiere:\n  código {a}\n  tabla  {b}")
                raise RuntimeError(f"cantidad distinta: código {len(reference)} / tabla {len(rows)}")
            print(f"Equivalencia con {args.reference_ref}: OK ({len(rows)} llamadas)")


if __name__ == "__main__":
    main()
