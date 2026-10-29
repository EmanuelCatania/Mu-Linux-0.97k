#!/usr/bin/env python3
"""Verifica orientación real de items: todos los short y objeto sobre 4 GiB.

Usa fixture escalar OBJECT con offsets x86. No valida layouts nativos x64 ni
render de items. Mantiene los bit patterns de producción sin reinterpretarlos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from CharacterPositionPointer import harness as position_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void CallItemAngle(OBJECT* object) { CALL_ITEM; }
std::uint64_t RunItems(OBJECT* object) {
    digest = 14695981039346656037ull;
    for(unsigned bits = 0; bits < 65536; ++bits)
    for(DWORD scale : {0u, 0x80000000u, 0x3f800000u, 0x7fc00001u}) {
        std::memset(object, 0x5a, sizeof(*object));
        std::memcpy(&object->Type, &bits, 2); std::memcpy(&object->Scale, &scale, 4);
        OBJECT before; std::memcpy(&before, object, sizeof(before));
        CallItemAngle(object);
        // Sólo Angle y, para determinados tipos, Scale pueden escribirse.
        OBJECT allowed; std::memcpy(&allowed, &before, sizeof(allowed));
        std::memcpy(allowed.Angle, object->Angle, sizeof(allowed.Angle));
        std::memcpy(&allowed.Scale, &object->Scale, sizeof(allowed.Scale));
        Require(std::memcmp(object, &allowed, sizeof(allowed)) == 0);
        const short type = object->Type;
        if(type != 419 && type != 952 && type != 957 && type != 958)
            Require(std::memcmp(&object->Scale, &scale, 4) == 0);
        if(type == 419) Require(object->Angle[0] == 60.0f && object->Scale == 0.7f);
        if(type == 958) Require(object->Angle[0] == 115.0f && object->Angle[1] == 75.0f && object->Angle[2] == 8.0f && object->Scale == 0.4f);
        if(type < 0) Require(object->Angle[0] == 0.0f && object->Angle[1] == 0.0f && object->Angle[2] == -45.0f);
        Hash(object, sizeof(*object));
    }
    return digest;
}
int main() {
    auto* low = static_cast<OBJECT*>(Reserve(0x76000000u)); Require(low);
    const auto expected = RunItems(low);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<OBJECT*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low, 0, sizeof(*low)); Require(RunItems(high) == expected);
        for(std::size_t i = 0; i < sizeof(*low); ++i) Require(reinterpret_cast<BYTE*>(low)[i] == 0);
        std::puts("orientacion sobre 4 GiB: objeto alto y alias bajo intacto");
    }
    std::printf("ItemAngle: 262144 casos, todos los tipos short; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = position_harness(revision).split("void CallPosition(CHARACTER* character)", 1)[0]
    scale = re.search(r"float\s+Scale\s*;", read_source("src/Entity/EntityTypes.h", revision)).group()
    prefix = prefix.replace("BYTE beforePosition[8];", "BYTE beforeScale[4]; " + scale, 1)
    prefix += "\nstatic_assert(offsetof(OBJECT, Scale) == 12 && offsetof(OBJECT, Angle) == 28);\n"
    item = function(read_source("src/Entity/Entity_LegacyEffects.cpp", revision), "ItemAngle")
    arg = "object" if "OBJECT* object" in item else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix + item + MAIN.replace("CALL_ITEM", f"ItemAngle({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-angle-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
