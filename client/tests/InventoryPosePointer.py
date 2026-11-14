#!/usr/bin/env python3
"""Pose de los items 3D en las grillas (Content/item/inventory_pose.toml).

Compila el lector real (src/render/InventoryPoseTable.cpp) contra el .toml del
repo y vuelca, para cada modelo vanilla, los tres ángulos, la altura, las dos
escalas y el modo de ángulo; comprueba que no haya líneas rechazadas.

Con --reference-ref REV compila además MakeInventoryPoses tal como estaba en
REV (la tabla constexpr de BMD_LegacyDraw.cpp) y exige el mismo volcado, bit a
bit. La tabla se generó así desde el commit de T5 del refactor.
"""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from MonsterSpawnPointer import build, item_constants

ROOT = Path(__file__).resolve().parents[1]
TOML = ROOT / "bin/Client/Content/item/inventory_pose.toml"

DUMP = r'''
#include <cstdio>
#include <cstring>
static unsigned Bits(float f) { unsigned u; std::memcpy(&u, &f, 4); return u; }
template <class Pose> static void Dump(int type, const Pose& p)
{
    std::printf("%d|%08x|%08x|%08x|%08x|%08x|%08x|%d\n", type, Bits(p.Angle[0]), Bits(p.Angle[1]),
                Bits(p.Angle[2]), Bits(p.BodyHeight), Bits(p.Scale), Bits(p.NegativeLevelScale), (int)p.AngleMode);
}
'''


def inline(text, *headers):
    for header in headers:
        body = (ROOT / "src" / header).read_text(encoding="utf-8").replace("#pragma once", "")
        text = text.replace(f'#include "{header}"', body)
    return text


def current_harness():
    source = (ROOT / "src/render/InventoryPoseTable.cpp").read_text(encoding="utf-8")
    source = source.replace('#include "stdafx.h"', "")
    source = inline(source, "render/InventoryPoseTable.h", "assets/data/TomlLines.h", "app/core/ResourceLimits.h")
    source = source.replace('extern "C" void DbgLogPublic(const char* msg);',
                            'void DbgLogPublic(const char* msg) { std::printf("WARN|%s\\n", msg); }')
    return DUMP + source + "\nint main() { for (int t = 0; t < MODEL_MAX_VANILLA; ++t) Dump(t, InventoryPose_Get(t)); }\n"


def reference_harness(revision):
    def show(path):
        return subprocess.check_output(["git", "show", f"{revision}:client/{path}"], cwd=ROOT).decode("utf-8")
    source = show("src/render/BMD_LegacyDraw.cpp")
    start = source.index("enum class InventoryAngleMode")
    end = source.index("constexpr InventoryPoseDefinition DefaultInventoryPose{};")
    limits = show("src/app/core/ResourceLimits.h").replace("#pragma once", "")
    return ("#include <array>\n" + DUMP + limits + item_constants(show("src/game/item/ItemDefines.h")) +
            source[start:end] +
            "\nint main() { for (int t = 0; t < MODEL_MAX_VANILLA; ++t) Dump(t, InventoryPoses[t]); }\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-inventory-pose-") as temp:
        data = Path(temp) / "data"
        (data / "Content/item").mkdir(parents=True)
        shutil.copyfile(TOML, data / "Content/item/inventory_pose.toml")
        rows = build(Path(temp) / "current", current_harness(), args, cwd=data)
        warnings = [r for r in rows if r.startswith("WARN|")]
        if warnings:
            raise RuntimeError("el lector rechazó líneas:\n" + "\n".join(warnings))
        default = rows[0].split("|")[1:]
        posed = sum(1 for r in rows if r.split("|")[1:] != default)
        print(f"InventoryPose: {len(rows)} modelos, {posed} con pose distinta a la del modelo 0")
        if args.reference_ref:
            reference = build(Path(temp) / "reference", reference_harness(args.reference_ref), args)
            if reference != rows:
                for a, b in zip(reference, rows):
                    if a != b:
                        raise RuntimeError(f"difiere:\n  código {a}\n  tabla  {b}")
                raise RuntimeError("cantidad de modelos distinta")
            print(f"Equivalencia con {args.reference_ref}: OK ({len(rows)} modelos, bit a bit)")


if __name__ == "__main__":
    main()
