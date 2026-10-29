#!/usr/bin/env python3
"""Verifica daño físico real con máquina, atributos y reserva de items altas.

Todos los auxiliares de opciones/durabilidad son reales. No prueba
CalculateAll integrado, hover de inventario ni daño del servidor.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from BaseDefensePointer import harness as defense_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
int CallBase(BYTE* machine) { return CALL_BASE; }
std::uint64_t RunBase(BYTE* machine, BYTE* attribute, BYTE* allocation, int displacement) {
    seed = 0x97c47d41; std::uint64_t digest = 14695981039346656037ull;
    const auto reservation = reinterpret_cast<std::uintptr_t>(allocation);
    const auto base = reservation + displacement * 64;
    DAT_055c9e44 = reservation; DAT_07d78068 = base; DAT_07cf1ff4 = attribute;
    std::memset(allocation, 0xa5, (ITEM_MAX_EX + 1024) * 64);
    std::vector<BYTE> expectedTable(allocation, allocation + (ITEM_MAX_EX + 1024) * 64);
    const short types[] = {-32768, -1, 0, 31, 127, 128, 134, 135, 136, 142, 143, 144, 145, 159, 160, 192, 193, 8191, 8192, 32767};
    Require(CallBase(nullptr) == 0 && CallBase(reinterpret_cast<BYTE*>(std::uintptr_t(0x1000))) == 0);
    bool bowBoost = false, crossbowBoost = false;
    for(int trial = 0; trial < 16384; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64); attribute[11] = BYTE(Input());
        const unsigned short strength = static_cast<unsigned short>(Input()), vitality = static_cast<unsigned short>(Input()), energy = static_cast<unsigned short>(Input());
        std::memcpy(machine + 20, &strength, 2); std::memcpy(machine + 22, &vitality, 2); std::memcpy(machine + 26, &energy, 2);
        for(int offset : {536, 604, 1012, 1148}) {
            auto* item = reinterpret_cast<ITEM*>(machine + offset); std::memset(item, 0, sizeof(*item));
            item->Type = types[Input() % 20]; item->Durability = Input() % 5 == 0 ? 0 : BYTE(255);
            item->DamageMin = static_cast<unsigned short>(Input()); item->DamageMax = static_cast<unsigned short>(Input());
            item->Level = int(1 + Input() % 15) * 8; item->Option1 = BYTE(Input() % 64); item->SpecialNum = BYTE(Input() % 10);
            auto* data = reinterpret_cast<BYTE*>(item);
            for(int j = 0; j < 9; ++j) data[37+j] = j % 3 == 0 ? BYTE(60) : j % 3 == 1 ? BYTE(73) : BYTE(74);
            for(int j = 0; j < 9; ++j) data[45+j] = BYTE(Input());
        }
        auto* left = reinterpret_cast<ITEM*>(machine + 536); auto* right = reinterpret_cast<ITEM*>(machine + 604);
        const bool naked = trial % 16 == 0;
        if(naked) for(int offset : {536, 604, 1012, 1148}) reinterpret_cast<ITEM*>(machine + offset)->Type = -1;
        if(trial % 32 == 1) { left->Type = 135; right->Type = 128; left->Durability = right->Durability = 255; bowBoost = true; }
        if(trial % 32 == 2) { left->Type = 136; right->Type = 143; left->Durability = right->Durability = 255; crossbowBoost = true; }
        // Se prepara también la fila fallback de tipos inválidos.
        for(int row : {0, int(left->Type), int(right->Type), int(reinterpret_cast<ITEM*>(machine + 1012)->Type)}) {
            if(row < 0 || row >= ITEM_MAX_EX) continue;
            const BYTE durability = BYTE(1 + Input() % 255);
            reinterpret_cast<ITEM_ATTRIBUTE*>(base)[row].Durability = durability;
            expectedTable[displacement * 64 + row * 64 + offsetof(ITEM_ATTRIBUTE, Durability)] = durability;
        }
        DAT_055c9e44 = trial % 13 == 0 ? 0 : reservation;
        DAT_07d78068 = trial % 17 == 0 ? reservation + 1 : base;
        DAT_07cf1ff4 = trial % 19 == 0 ? nullptr : attribute;
        const bool valid = DAT_055c9e44 && DAT_07d78068 == base && DAT_07cf1ff4;
        BYTE before[1412], beforeAttribute[64]; std::memcpy(before, machine, 1412); std::memcpy(beforeAttribute, attribute, 64);
        const int returned = CallBase(machine);
        if(!valid) Require(returned == 0 && std::memcmp(before, machine, 1412) == 0);
        else {
            unsigned short minimum; std::memcpy(&minimum, machine + 60, 2); Require(returned == minimum);
            BYTE allowed[1412]; std::memcpy(allowed, before, 1412); std::memcpy(allowed + 60, machine + 60, 8);
            Require(std::memcmp(allowed, machine, 1412) == 0);
            if(naked) {
                const int cls = attribute[11] & 7; unsigned short lo, hi;
                if(cls == 2) { const unsigned short sum = static_cast<unsigned short>(strength + vitality); lo = sum / 7; hi = sum / 4; }
                else if(cls == 1) { lo = strength / 6; hi = strength / 4; }
                else if(cls == 3) { lo = static_cast<unsigned short>(strength / 6 + energy / 12); hi = static_cast<unsigned short>(energy / 8 + strength / 4); }
                else { lo = strength / 8; hi = strength / 4; }
                unsigned short actual[4]; std::memcpy(actual, machine + 60, 8);
                Require(actual[0] == lo && actual[1] == hi && actual[2] == lo && actual[3] == hi);
            }
        }
        Require(std::memcmp(beforeAttribute, attribute, 64) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        digest ^= unsigned(returned); digest *= 1099511628211ull;
    }
    Require(bowBoost && crossbowBoost && std::memcmp(allocation, expectedTable.data(), expectedTable.size()) == 0);
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* attribute = static_cast<BYTE*>(Reserve(0x75000000u)); auto* items = static_cast<BYTE*>(Reserve(0x74000000u)); Require(machine && attribute && items);
    const auto expected = RunBase(machine, attribute, items, 0); Require(RunBase(machine, attribute, items, 1023) == expected);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttribute = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highItems = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highAttribute && highItems);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(attribute, 0, 64); if(mask & 4) std::memset(items, 0, (ITEM_MAX_EX + 1024) * 64);
            Require(RunBase(mask & 1 ? highMachine : machine, mask & 2 ? highAttribute : attribute, mask & 4 ? highItems : items, 1023) == expected);
        }
        std::puts("ataque base sobre 4 GiB: ocho combinaciones y desplazamientos 0/1023 equivalentes");
    }
    std::printf("Stats_CalcBase: 16384 estados, guardas/equipo/canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = defense_harness(revision).split("const ITEM* CallDefense(BYTE* machine)", 1)[0].replace("0x10000", "0x100000")
    prefix += "\n#ifndef _WIN32\n#define __fastcall\n#define __forceinline inline\n#endif\n"
    header, globals_source = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    name = "DAT_055c9e44"
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
    prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    prefix += "\n".join(re.findall(r"^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;", read_source("src/Item/ItemDefines.h", revision), re.M))
    prefix += re.search(r"static __forceinline [^\n]*\bItemAttribute_Base\([^\{]*\{.*?^\}", header, re.M | re.S).group()
    base = function(read_source("src/Core/Runtime_Small.cpp", revision), "Stats_CalcBase")
    arg = "machine" if "BYTE* a1" in base else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + base + MAIN.replace("CALL_BASE", f"Stats_CalcBase({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-base-attack-") as temp:
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
