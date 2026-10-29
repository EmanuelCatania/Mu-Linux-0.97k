#!/usr/bin/env python3
"""Prueba defensa base real con máquina, atributos e items altos.

Opciones/porcentajes/durabilidad reales. Apply del servidor controlado;
CHARACTER se usa como fixture escalar sólo para los alias previos del port.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from DefenseRatePointer import harness as rate_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
const ITEM* CallDefense(BYTE* machine) { return CALL_DEFENSE; }
std::uint64_t RunDefense(BYTE* machine, BYTE* attribute, ITEM_ATTRIBUTE* items) {
    activeMachine = machine; seed = 0x97c47dfe; std::uint64_t digest = 14695981039346656037ull;
    DAT_07d78068 = reinterpret_cast<std::uintptr_t>(items);
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64); std::memset(items, 0, 32 * 64);
        attribute[11] = BYTE(Input()); const unsigned short dex = static_cast<unsigned short>(Input()); std::memcpy(machine + 22, &dex, 2);
        DAT_07cf1ff4 = trial % 5 == 0 ? nullptr : attribute;
        for(int offset : {604, 672, 740, 808, 876, 944, 1216, 1284}) {
            auto* item = reinterpret_cast<ITEM*>(machine + offset); std::memset(item, 0, sizeof(*item));
            item->Type = Input() % 5 == 0 ? -1 : short(Input() % 32); item->Durability = BYTE(Input());
            item->Level = int(Input() % 16) * 8; item->Option1 = BYTE(Input() % 64); item->SuccessfulBlocking = BYTE(Input());
            item->SpecialNum = BYTE(Input() % 10); auto* data = reinterpret_cast<BYTE*>(item);
            for(int j = 0; j < 9; ++j) data[37+j] = j % 2 ? BYTE(70) : BYTE(62);
            for(int j = 0; j < 9; ++j) data[45+j] = BYTE(Input());
            if(item->Type >= 0) items[item->Type].Durability = BYTE(1 + Input() % 255);
        }
        serverApply = trial % 11 == 0; serverRate = static_cast<unsigned short>(Input()); applyCalls = 0;
        BYTE expected[1412], beforeItems[32 * 64], beforeAttribute[64]; std::memcpy(expected, machine, 1412); std::memcpy(beforeItems, items, sizeof(beforeItems)); std::memcpy(beforeAttribute, attribute, 64);
        const ITEM* expectedReturn = nullptr;
        if(serverApply) std::memcpy(expected + 76, &serverRate, 2);
        else if(DAT_07cf1ff4) {
            unsigned short defense = (attribute[11] & 7) == 2 ? dex / 4 : dex / 3;
            auto* shield = reinterpret_cast<ITEM*>(machine + 604);
            if(shield->Type != -1 && shield->Durability) {
                const float reduction = CalcDurabilityPercent(shield->Durability, items[shield->Type].Durability, shield->Level, shield->Option1);
                defense = static_cast<unsigned short>(defense + shield->SuccessfulBlocking - static_cast<unsigned short>(double(shield->SuccessfulBlocking) * double(reduction)));
                unsigned short bonus = 0; ExpectedPlus(reinterpret_cast<BYTE*>(shield), &bonus, 62);
                defense = static_cast<unsigned short>(defense + static_cast<unsigned short>(bonus - static_cast<unsigned short>(double(bonus) * double(reduction))));
                PlusSpecialPercent(reinterpret_cast<short*>(&defense), 70, shield, 10);
            }
            for(int offset : {672, 740, 808, 876, 944, 1284, 1216})
                PlusSpecialPercent(reinterpret_cast<short*>(&defense), 70, reinterpret_cast<ITEM*>(machine + offset), 10);
            expectedReturn = reinterpret_cast<ITEM*>(machine + 1216); std::memcpy(expected + 76, &defense, 2);
        }
        Require(CallDefense(machine) == expectedReturn && applyCalls == 1 && std::memcmp(machine, expected, 1412) == 0);
        Require(std::memcmp(items, beforeItems, sizeof(beforeItems)) == 0 && std::memcmp(attribute, beforeAttribute, 64) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        digest ^= expectedReturn != nullptr; digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* attribute = static_cast<BYTE*>(Reserve(0x75000000u)); auto* items = static_cast<ITEM_ATTRIBUTE*>(Reserve(0x74000000u)); Require(machine && attribute && items);
    const auto expected = RunDefense(machine, attribute, items);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttribute = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highItems = static_cast<ITEM_ATTRIBUTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highAttribute && highItems);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(attribute, 0, 64); if(mask & 4) std::memset(items, 0, 32 * 64);
            Require(RunDefense(mask & 1 ? highMachine : machine, mask & 2 ? highAttribute : attribute, mask & 4 ? highItems : items) == expected);
        }
        std::puts("defensa base sobre 4 GiB: ocho combinaciones y retorno completo");
    }
    std::printf("Stats_CalcDefense: 32768 estados; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = rate_harness(revision).split("int CallRate(BYTE* machine)", 1)[0]
    prefix = prefix.replace("std::memcpy(activeMachine + 78, &serverRate, 2);", "std::memcpy(activeMachine + 76, &serverRate, 2);")
    types = read_source("src/Entity/EntityTypes.h", revision)
    prefix += "\nusing vec3_t = float[3];\nstruct OBJECT { BYTE data[444]; };\n"
    for name in ("PART_t", "CHARACTER"):
        prefix += re.search(rf"struct {name}\s*\{{.*?\}};", types, re.S).group()
    prefix += "\nstatic_assert(sizeof(CHARACTER) == 916 && offsetof(CHARACTER, Weapon) == 624 && offsetof(CHARACTER, BodyPart) == 480);\n"
    prefix += "\nCHARACTER* AsCharacter(void* p) { return static_cast<CHARACTER*>(p); }\nCHARACTER* AsCharacter(std::uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }\n"
    prefix += function(read_source("src/Core/Runtime_Small.cpp", revision), "PlusSpecialPercent")
    defense = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Stats_CalcDefense")
    arg = "machine" if "BYTE* param_1" in defense else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + defense + MAIN.replace("CALL_DEFENSE", f"Stats_CalcDefense({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-base-defense-") as temp:
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
