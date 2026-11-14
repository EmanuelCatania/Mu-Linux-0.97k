#!/usr/bin/env python3
"""Definiciones de spawn de monstruos (Content/monster/spawn.toml).

Compila el lector real (src/game/monster/MonsterSpawnTable.cpp) contra el
spawn.toml del repo, vuelca los 256 tipos campo por campo y comprueba que no
haya líneas rechazadas y que cada `init` exista en Monster.cpp.

Con --reference-ref REV compila además MakeMonsterDefinitions tal como estaba
en REV (la tabla constexpr de Monster.cpp antes de pasar a datos) y exige el
mismo volcado. La tabla se generó así desde el commit de T4 del refactor.
"""
import argparse
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOML = ROOT / "bin/Client/Content/monster/spawn.toml"
FIELDS = ("Model", "Loader", "Resource", "ExtraResource", "Scale", "Right", "Left", "RightLevel",
          "LeftLevel", "RightExcellent", "BlendMesh", "BlendLight", "Subtype", "HiddenMesh",
          "State446", "Flag766")


def build(directory, source, args, cwd=None):
    directory.mkdir(parents=True)
    (directory / "main.cpp").write_text(source, encoding="utf-8")
    (directory / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.25)\nproject(MonsterSpawnPointer LANGUAGES CXX)\n"
        "add_executable(pointer_test main.cpp)\ntarget_compile_features(pointer_test PRIVATE cxx_std_17)\n"
        "if(MSVC)\n target_compile_options(pointer_test PRIVATE /utf-8)\nendif()\n", encoding="utf-8")
    command = ["cmake", "-S", str(directory), "-B", str(directory / "build")]
    if args.generator:
        command += ["-G", args.generator]
    if os.name == "nt":
        command += ["-A", args.architecture, "-T", "v143"]
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    subprocess.run(["cmake", "--build", str(directory / "build"), "--config", "Release"],
                   check=True, stdout=subprocess.DEVNULL)
    binary = directory / "build" / "pointer_test"
    if os.name == "nt":
        binary = directory / "build" / "Release" / "pointer_test.exe"
    return subprocess.check_output([str(binary)], cwd=cwd or directory, text=True,
                                   encoding="utf-8").splitlines()


def item_constants(defines):
    lines = [l for l in defines.splitlines()
             if re.match(r"constexpr (int|short) (ITEM_\w+|MODEL_\w+) = [\w\s+*()-]+;", l)]
    return "\n".join(lines) + "\n"


DUMP = r'''
#include <cstdio>
#include <cstring>
static unsigned Bits(float f) { unsigned u; std::memcpy(&u, &f, 4); return u; }
static void Dump(int type, int model, int loader, int resource, int extra, float scale, int right, int left,
                 int rl, int ll, int rx, int blend, float blendLight, int subtype, int hidden, int s446,
                 int f766, const char* init)
{
    std::printf("%d|%d|%d|%d|%d|%08x|%d|%d|%d|%d|%d|%d|%08x|%d|%d|%d|%d|%s\n", type, model, loader, resource,
                extra, Bits(scale), right, left, rl, ll, rx, blend, Bits(blendLight), subtype, hidden, s446,
                f766, init);
}
'''


def reference_harness(monster, defines):
    start = monster.index("constexpr int KeepSpawnValue")
    end = monster.index("constexpr auto MonsterDefinitions = MakeMonsterDefinitions();")
    block = monster[start:end]
    names = re.findall(r"void (InitializeMonster\w+)\(CHARACTER\* character\)", block)
    # Los cuerpos de los inicializadores no importan para la tabla.
    for name in names:
        i = block.index(f"void {name}(CHARACTER* character)")
        j = block.index("{", i)
        depth, k = 1, j + 1
        while depth:
            depth += {"{": 1, "}": -1}.get(block[k], 0)
            k += 1
        # Un cuerpo distinto por función: con cuerpos vacíos el linker las funde
        # (ICF) y todas quedan con el mismo puntero.
        block = block[:j] + "{ static volatile int marker = " + str(names.index(name)) + "; marker = marker + 1; }" + block[k:]
    table = "\n".join(f'    if (d.Initialize == {n}) return "{n[len("Initialize"):]}";' for n in names)
    return ("#include <array>\nstruct CHARACTER;\n" + item_constants(defines) + DUMP + "namespace {\n" + block +
            "\nconstexpr auto MonsterDefinitions = MakeMonsterDefinitions();\n"
            "const char* InitName(const MonsterDefinition& d) {\n    if (!d.Initialize) return \"\";\n" + table +
            '\n    return "?";\n}\n}\nint main() {\n    for (int t = 0; t < 256; ++t) { const auto& d = MonsterDefinitions[t];\n'
            "        Dump(t, d.Model, (int)d.Loader, d.Resource, d.ExtraResource, d.Scale, d.Right, d.Left, d.RightLevel,\n"
            "             d.LeftLevel, d.RightExcellent, d.BlendMesh, d.BlendLight, d.Subtype, d.HiddenMesh, d.State446,\n"
            "             d.Flag766, InitName(d)); }\n}\n")


def current_harness():
    source = (ROOT / "src/game/monster/MonsterSpawnTable.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/game/monster/MonsterSpawnTable.h").read_text(encoding="utf-8")
    toml = (ROOT / "src/assets/data/TomlLines.h").read_text(encoding="utf-8")
    source = source.replace('#include "stdafx.h"', "")
    source = source.replace('#include "game/monster/MonsterSpawnTable.h"', header.replace("#pragma once", ""))
    source = source.replace('#include "assets/data/TomlLines.h"', toml.replace("#pragma once", ""))
    source = source.replace('extern "C" void DbgLogPublic(const char* msg);',
                            'void DbgLogPublic(const char* msg) { std::printf("WARN|%s\\n", msg); }')
    return "#include <cstdio>\n" + DUMP + source + r'''
int main()
{
    for (int t = 0; t < 256; ++t) {
        const MonsterSpawn& d = MonsterSpawn_Get(t);
        Dump(t, d.Model, (int)d.Loader, d.Resource, d.ExtraResource, d.Scale, d.Right, d.Left, d.RightLevel,
             d.LeftLevel, d.RightExcellent, d.BlendMesh, d.BlendLight, d.Subtype, d.HiddenMesh, d.State446,
             d.Flag766, d.Init.c_str());
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-monster-spawn-") as temp:
        data = Path(temp) / "data"
        (data / "Content/monster").mkdir(parents=True)
        shutil.copyfile(TOML, data / "Content/monster/spawn.toml")
        rows = build(Path(temp) / "current", current_harness(), args, cwd=data)
        warnings = [r for r in rows if r.startswith("WARN|")]
        if warnings:
            raise RuntimeError("el lector rechazó líneas:\n" + "\n".join(warnings))
        monster = (ROOT / "src/game/monster/Monster.cpp").read_text(encoding="utf-8")
        known = set(re.findall(r'\{\s*"(\w+)"\s*,\s*InitializeMonster\w+\s*\}', monster))
        unknown = sorted({r.split("|")[-1] for r in rows} - known - {""})
        if unknown:
            raise RuntimeError(f"init sin función en Monster.cpp: {unknown}")
        print(f"MonsterSpawn: 256 tipos volcados, {len(known)} inicializadores conocidos")
        if args.reference_ref:
            def show(path):
                return subprocess.check_output(["git", "show", f"{args.reference_ref}:client/{path}"],
                                               cwd=ROOT).decode("utf-8")
            reference = build(Path(temp) / "reference",
                              reference_harness(show("src/game/monster/Monster.cpp"),
                                                show("src/game/item/ItemDefines.h")), args)
            if reference != rows:
                for a, b in zip(reference, rows):
                    if a != b:
                        raise RuntimeError(f"difiere:\n  código {a}\n  tabla  {b}")
                raise RuntimeError("cantidad de filas distinta")
            print(f"Equivalencia con {args.reference_ref}: OK (256 tipos, {len(FIELDS)} campos + init)")


if __name__ == "__main__":
    main()
