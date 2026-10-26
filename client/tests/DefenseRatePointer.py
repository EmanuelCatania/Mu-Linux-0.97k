#!/usr/bin/env python3
"""Verifica tasa de defensa real con tres bases altas e items reales.

Apply del servidor controlado. No certifica snapshot de red, CalculateAll,
inventario visible ni layouts nativos con punteros.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemSpecialPointer import harness as special_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int g_bAddDefense, EquipmentLevelSet;
BYTE* activeMachine;
bool serverApply;
unsigned short serverRate;
int applyCalls;
struct ServerStats {
    bool Apply(void* pointer) {
        Require(pointer == activeMachine); ++applyCalls;
        if(serverApply) std::memcpy(activeMachine + 78, &serverRate, 2);
        return serverApply;
    }
} gServerCharacterStats;
std::uint32_t seed;
std::uint32_t Input() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
'''

MAIN = r'''
int CallRate(BYTE* machine) { return CALL_RATE; }
std::uint64_t RunRate(BYTE* machine, BYTE* attribute, ITEM_ATTRIBUTE* items) {
    activeMachine = machine; seed = 0x97c47e16; std::uint64_t digest = 14695981039346656037ull;
    DAT_07d78068 = reinterpret_cast<std::uintptr_t>(items);
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64); std::memset(items, 0, 32 * sizeof(*items));
        attribute[11] = BYTE(Input() % 256); const unsigned short vit = static_cast<unsigned short>(Input());
        std::memcpy(machine + 22, &vit, 2);
        DAT_07cf1ff4 = trial % 5 == 0 ? nullptr : attribute;
        g_bAddDefense = int(Input() % 2); EquipmentLevelSet = 9 + int(Input() % 4);
        for(int i = 1; i <= 7; ++i) {
            auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*i); std::memset(item, 0, sizeof(*item));
            item->Type = short(Input() % 32); item->Durability = BYTE(Input() % 256);
            item->Defense = static_cast<unsigned short>(Input()); item->Level = int(Input() % 16) * 8;
            item->Option1 = BYTE(Input() % 64); item->SpecialNum = BYTE(Input() % 10);
            auto* data = reinterpret_cast<BYTE*>(item);
            for(int j = 0; j < 9; ++j) data[37+j] = j % 2 ? BYTE(Input() % 256) : BYTE(63);
            for(int j = 0; j < 9; ++j) data[45+j] = BYTE(Input());
            items[item->Type].Durability = BYTE(1 + Input() % 255);
        }
        serverApply = trial % 11 == 0; serverRate = static_cast<unsigned short>(Input()); applyCalls = 0;
        BYTE expected[1412], beforeItems[32 * 64], beforeAttribute[64];
        std::memcpy(expected, machine, sizeof(expected)); std::memcpy(beforeItems, items, sizeof(beforeItems)); std::memcpy(beforeAttribute, attribute, 64);
        int expectedReturn = 0;
        if(serverApply) std::memcpy(expected + 78, &serverRate, 2);
        else if(DAT_07cf1ff4) {
            const int cls = attribute[11] & 7;
            int value = cls == 2 ? vit / 10 : cls == 1 ? vit / 3 : cls == 0 ? vit / 4 : vit / 5;
            for(int i = 1; i <= 7; ++i) {
                auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*i);
                if(!item->Durability) continue;
                unsigned short defense = item->Defense; ExpectedPlus(reinterpret_cast<BYTE*>(item), &defense, 63);
                const unsigned int legacyDefense = static_cast<unsigned int>(static_cast<short>(defense));
                const float reduction = CalcDurabilityPercent(item->Durability, items[item->Type].Durability, item->Level, i == 7 ? 0 : item->Option1);
                value += int(legacyDefense) - int(double(static_cast<unsigned short>(legacyDefense)) * double(reduction));
            }
            unsigned short stored = static_cast<unsigned short>(value);
            if(g_bAddDefense) {
                const double bonus = EquipmentLevelSet == 10 ? 0.050000001 : EquipmentLevelSet == 11 ? 0.1 : 0;
                stored = static_cast<unsigned short>(static_cast<short>(stored) + int(double(stored) * bonus));
            }
            std::memcpy(expected + 78, &stored, 2); expectedReturn = static_cast<short>(stored);
        }
        const int returned = CallRate(machine);
        Require(returned == expectedReturn && applyCalls == 1 && std::memcmp(machine, expected, sizeof(expected)) == 0);
        Require(std::memcmp(items, beforeItems, sizeof(beforeItems)) == 0 && std::memcmp(attribute, beforeAttribute, 64) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        digest ^= unsigned(returned); digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* attribute = static_cast<BYTE*>(Reserve(0x75000000u));
    auto* items = static_cast<ITEM_ATTRIBUTE*>(Reserve(0x74000000u)); Require(machine && attribute && items);
    const auto expected = RunRate(machine, attribute, items);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttribute = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highItems = static_cast<ITEM_ATTRIBUTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highAttribute && highItems);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(attribute, 0, 64); if(mask & 4) std::memset(items, 0, 32 * 64);
            Require(RunRate(mask & 1 ? highMachine : machine, mask & 2 ? highAttribute : attribute, mask & 4 ? highItems : items) == expected);
        }
        std::puts("defensa sobre 4 GiB: ocho combinaciones CharacterMachine/atributos/items equivalentes");
    }
    std::printf("Stats_CalcDefenseRate: 32768 estados, bytes/retorno %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = special_harness(revision).split("void CallPlus(unsigned short* value", 1)[0]
    # El oráculo de opciones respeta las lecturas legacy y sus posibles alias.
    from ItemSpecialPointer import MAIN as SPECIAL_MAIN
    prefix += function(SPECIAL_MAIN, "ExpectedPlus")
    structs = read_source("src/structs.h", revision)
    prefix += "\n" + "\n".join(re.findall(r"^#define\s+(?:MAX_CLASS|MAX_RESISTANCE)\s+[^\n]+", structs, re.M)) + "\n"
    prefix += re.search(r"struct ITEM_ATTRIBUTE\b.*?\n\};", structs, re.S).group()
    prefix += "\nstatic_assert(sizeof(ITEM_ATTRIBUTE) == 64);\n"
    header, globals_source = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("DAT_07cf1ff4", "DAT_07d78068"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    prefix += function(read_source("src/Core/Runtime_Small.cpp", revision), "CalcDurabilityPercent")
    rate = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Stats_CalcDefenseRate")
    arg = "machine" if "BYTE* param_1" in rate else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + SERVICES + rate + MAIN.replace("CALL_RATE", f"Stats_CalcDefenseRate({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-defense-rate-") as temp:
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
