#!/usr/bin/env python3
"""Verifica daño mágico real con máquina y salidas altas, incluidos alias.

Usa los 64 registros declarados en SkillAttribute.Raw, sin certificar
protocolo, carga de skills ni corregir índices fuera de ese rango.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void CallDamage(const BYTE* machine, int type, int* low, int* high) { CALL_DAMAGE; }
std::uint64_t RunDamage(BYTE* machine, BYTE* minStorage, BYTE* maxStorage) {
    std::uint64_t digest = 14695981039346656037ull;
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(machine, 0x5a, 128); std::memset(minStorage, 0xa5, 32); std::memset(maxStorage, 0x3c, 32);
        for(int i = 0; i < 2560; ++i) SkillAttribute.Raw[i] = char((i + trial) & 255);
        const unsigned short minimum = static_cast<unsigned short>(trial * 17), maximum = static_cast<unsigned short>(65535 - trial * 31);
        std::memcpy(machine + 70, &minimum, 2); std::memcpy(machine + 72, &maximum, 2);
        const int type = trial % 64 + 256 * (trial % 9 - 4), mode = trial % 4;
        int* low = mode == 2 ? reinterpret_cast<int*>(machine + 72) : reinterpret_cast<int*>(minStorage + 8);
        int* high = mode == 1 ? low : mode == 3 ? reinterpret_cast<int*>(machine + 68) : reinterpret_cast<int*>(maxStorage + 8);
        BYTE expectedMachine[128], expectedMin[32], expectedMax[32], beforeSkills[2560];
        std::memcpy(expectedMachine, machine, 128); std::memcpy(expectedMin, minStorage, 32); std::memcpy(expectedMax, maxStorage, 32); std::memcpy(beforeSkills, SkillAttribute.Raw, 2560);
        const BYTE damage = BYTE(SkillAttribute.Raw[40 * (type & 255) + 33]);
        const int expectedLow = int(damage) + minimum;
        BYTE* lowDest = mode == 2 ? expectedMachine + 72 : expectedMin + 8;
        std::memcpy(lowDest, &expectedLow, 4);
        unsigned short afterMaximum; std::memcpy(&afterMaximum, expectedMachine + 72, 2);
        const int expectedHigh = (damage >> 1) + damage + afterMaximum;
        BYTE* highDest = mode == 1 ? lowDest : mode == 3 ? expectedMachine + 68 : expectedMax + 8;
        std::memcpy(highDest, &expectedHigh, 4);
        CallDamage(machine, type, low, high);
        Require(std::memcmp(machine, expectedMachine, 128) == 0 && std::memcmp(minStorage, expectedMin, 32) == 0 && std::memcmp(maxStorage, expectedMax, 32) == 0);
        Require(std::memcmp(SkillAttribute.Raw, beforeSkills, 2560) == 0);
        for(int i = 0; i < 128; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        for(const BYTE* bytes : {minStorage, maxStorage}) for(int i = 0; i < 32; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* low = static_cast<BYTE*>(Reserve(0x75000000u)); auto* high = static_cast<BYTE*>(Reserve(0x74000000u)); Require(machine && low && high);
    const auto expected = RunDamage(machine, low, high);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highLow = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highHigh = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highLow && highHigh);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(machine, 0, 128); if(mask & 2) std::memset(low, 0, 32); if(mask & 4) std::memset(high, 0, 32);
            Require(RunDamage(mask & 1 ? highMachine : machine, mask & 2 ? highLow : low, mask & 4 ? highHigh : high) == expected);
        }
        std::puts("danio magico sobre 4 GiB: ocho combinaciones y tres alias equivalentes");
    }
    std::printf("GetMagicSkillDamage: 32768 estados, 64 skills y canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += re.search(r"struct _SkillAttrEntry\s*\{.*?\};", read_source("src/globals.h", revision), re.S).group()
    prefix += "\n_SkillAttrEntry SkillAttribute{};\nstatic_assert(sizeof(SkillAttribute.Raw) == 64 * 40);\n"
    damage = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "GetMagicSkillDamage")
    arg = "machine" if "const BYTE* This" in damage else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + damage + MAIN.replace("CALL_DAMAGE", f"GetMagicSkillDamage({arg}, type, low, high)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-magic-skill-") as temp:
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
