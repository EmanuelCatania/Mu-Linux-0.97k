#!/usr/bin/env python3
"""Prueba PlusSpecial y defensa reales con ITEM y destino sobre 4 GiB.

Extrae ITEM completo. Conserva lectura legacy en +45 y alias del destino;
no certifica cálculo completo de atributos ni interfaz de inventario.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void CallPlus(unsigned short* value, int special, ITEM* item) { CALL_PLUS; }
void ExpectedPlus(BYTE* item, unsigned short* value, int special) {
    short type; std::memcpy(&type, item, 2); if(type == -1) return;
    const int count = item[36];
    for(int i = 0; i < count; ++i) if(int(item[i + 37]) == special && item[26]) *value = static_cast<unsigned short>(*value + item[i + 45]);
}
std::uint64_t RunSpecial(BYTE* itemStorage, BYTE* valueStorage) {
    std::uint64_t digest = 14695981039346656037ull;
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(itemStorage, 0x5a, 100); std::memset(valueStorage, 0xa5, 40);
        auto* item = reinterpret_cast<ITEM*>(itemStorage + 16); auto* data = reinterpret_cast<BYTE*>(item);
        item->Type = trial % 3 == 0 ? -1 : 500; item->Durability = trial % 5 == 0 ? 0 : BYTE(255);
        item->SpecialNum = BYTE(trial % 10);
        for(int i = 0; i < 9; ++i) data[37 + i] = BYTE(trial + 17*i);
        for(int i = 0; i < 9; ++i) data[45 + i] = BYTE(trial / 256 + 31*i);
        const int offsets[] = {0, 0, 18, 26, 46}; const int alias = trial % 5;
        auto* value = reinterpret_cast<unsigned short*>(alias ? data + offsets[alias] : valueStorage + 16);
        *value = static_cast<unsigned short>(trial);
        const int special = trial % 4 == 0 ? int(data[37 + trial % 9]) : trial % 4 == 1 ? -1 : trial % 4 == 2 ? 256 : trial & 255;
        alignas(unsigned short) BYTE expectedItem[100], expectedValue[40]; std::memcpy(expectedItem, itemStorage, 100); std::memcpy(expectedValue, valueStorage, 40);
        auto* expectedDest = reinterpret_cast<unsigned short*>(alias ? expectedItem + 16 + offsets[alias] : expectedValue + 16);
        ExpectedPlus(expectedItem + 16, expectedDest, special); CallPlus(value, special, item);
        Require(std::memcmp(itemStorage, expectedItem, 100) == 0 && std::memcmp(valueStorage, expectedValue, 40) == 0);
        unsigned int defense = 0;
        if(item->Type != -1) {
            unsigned short base = item->Defense; ExpectedPlus(data, &base, 63);
            defense = static_cast<unsigned int>(static_cast<short>(base));
        }
        Require(Item_GetDefenseWithSpecial(123, reinterpret_cast<short*>(item)) == defense);
        Require(Item_GetDefenseWithSpecial(123, nullptr) == 0);
        Require(std::memcmp(itemStorage, expectedItem, 100) == 0);
        for(int i = 0; i < 100; ++i) { digest ^= itemStorage[i]; digest *= 1099511628211ull; }
        for(int i = 0; i < 40; ++i) { digest ^= valueStorage[i]; digest *= 1099511628211ull; }
        digest ^= defense; digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* item = static_cast<BYTE*>(Reserve(0x76000000u)); auto* value = static_cast<BYTE*>(Reserve(0x75000000u)); Require(item && value);
    const auto expected = RunSpecial(item, value);
    if(sizeof(void*) > 4) {
        auto* highItem = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highValue = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highItem && highValue);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(item, 0, 100); if(mask & 2) std::memset(value, 0, 40);
            Require(RunSpecial(mask & 1 ? highItem : item, mask & 2 ? highValue : value) == expected);
        }
        std::puts("opciones sobre 4 GiB: cuatro combinaciones ITEM/destino y cuatro alias equivalentes");
    }
    std::printf("PlusSpecial/defensa: 65536 estados, overflow/canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += "\n#ifndef _WIN32\nusing WORD = std::uint16_t;\n#endif\nusing std::uintptr_t;\n"
    structs = read_source("src/structs.h", revision)
    prefix += re.search(r"^#define MAX_SPECIAL_OPTION[^\n]+", structs, re.M).group() + "\n"
    if '#include "Item/ItemTypes.h"' in structs:
        prefix += read_source("src/Item/ItemTypes.h", revision)
    else:
        prefix += re.search(r"struct ITEM\b.*?\n\};", structs, re.S).group()
    prefix += "\nstatic_assert(sizeof(ITEM) == 68 && offsetof(ITEM, Durability) == 26 && offsetof(ITEM, SpecialNum) == 36);\n"
    prefix += "\n".join(re.findall(r"^inline (?:const )?ITEM\* AsItem[^\n]+", structs, re.M))
    plus = function(read_source("src/Core/Runtime_Small.cpp", revision), "PlusSpecial")
    # El extractor reconoce int; se conserva el unsigned del retorno de producción.
    defense = "unsigned " + function(read_source("src/Item/Item_SpecialOption.cpp", revision), "Item_GetDefenseWithSpecial")
    arg = "item" if "const ITEM* Item" in plus else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(item))"
    return prefix + plus + defense + MAIN.replace("CALL_PLUS", f"PlusSpecial(value, special, {arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-special-") as temp:
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
