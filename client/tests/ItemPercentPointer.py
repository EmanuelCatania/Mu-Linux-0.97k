#!/usr/bin/env python3
"""Prueba opciones porcentuales reales y retorno del ITEM sobre 4 GiB.

Preserva productos usados por el cliente, wrap de WORD y alias del destino.
No certifica todavía los cálculos completos de magia/defensa que propagan retorno.
"""
import argparse
import os
from pathlib import Path
import tempfile

from ItemSpecialPointer import harness as special_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uintptr_t CallPercent(short* value, int special, ITEM* item, unsigned short factor) { CALL_PERCENT; }
std::uint64_t RunPercent(BYTE* itemStorage, BYTE* valueStorage) {
    std::uint64_t digest = 14695981039346656037ull;
    const unsigned short factors[] = {0, 1, 2, 10, 100, 1000};
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(itemStorage, 0x5a, 100); std::memset(valueStorage, 0xa5, 40);
        auto* item = reinterpret_cast<ITEM*>(itemStorage + 16); auto* data = reinterpret_cast<BYTE*>(item);
        item->Type = trial % 3 == 0 ? -1 : 500; item->Durability = trial % 5 == 0 ? 0 : BYTE(255); item->SpecialNum = BYTE(trial % 10);
        for(int i = 0; i < 9; ++i) data[37+i] = i % 3 ? BYTE(trial & 255) : BYTE(trial + i);
        const int offsets[] = {0, 0, 18, 26, 46}; const int alias = trial % 5;
        auto* value = reinterpret_cast<unsigned short*>(alias ? data + offsets[alias] : valueStorage + 16); *value = static_cast<unsigned short>(trial);
        const int special = trial % 4 == 0 ? int(data[37 + trial % 9]) : trial % 4 == 1 ? -1 : trial % 4 == 2 ? 256 : trial & 255;
        const auto factor = factors[trial % 6];
        alignas(unsigned short) BYTE expectedItem[100], expectedValue[40]; std::memcpy(expectedItem, itemStorage, 100); std::memcpy(expectedValue, valueStorage, 40);
        BYTE* expectedData = expectedItem + 16;
        auto* expectedDest = reinterpret_cast<unsigned short*>(alias ? expectedData + offsets[alias] : expectedValue + 16);
        short expectedType; std::memcpy(&expectedType, expectedData, 2);
        if(expectedType != -1) {
            const int count = expectedData[36];
            for(int i = 0; i < count; ++i) if(int(expectedData[i + 37]) == special)
                *expectedDest = static_cast<unsigned short>(*expectedDest + static_cast<unsigned short>(factor * *expectedDest / 100));
        }
        const auto returned = CallPercent(reinterpret_cast<short*>(value), special, item, factor);
        Require(returned == reinterpret_cast<std::uintptr_t>(item));
        Require(std::memcmp(itemStorage, expectedItem, 100) == 0 && std::memcmp(valueStorage, expectedValue, 40) == 0);
        for(int i = 0; i < 100; ++i) { digest ^= itemStorage[i]; digest *= 1099511628211ull; }
        for(int i = 0; i < 40; ++i) { digest ^= valueStorage[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* item = static_cast<BYTE*>(Reserve(0x76000000u)); auto* value = static_cast<BYTE*>(Reserve(0x75000000u)); Require(item && value);
    const auto expected = RunPercent(item, value);
    if(sizeof(void*) > 4) {
        auto* highItem = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highValue = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highItem && highValue);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(item, 0, 100); if(mask & 2) std::memset(value, 0, 40);
            Require(RunPercent(mask & 1 ? highItem : item, mask & 2 ? highValue : value) == expected);
        }
        std::puts("porcentajes sobre 4 GiB: cuatro combinaciones y retorno completo del ITEM");
    }
    std::printf("PlusSpecialPercent: 65536 estados, alias/overflow/retorno; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = special_harness(revision).split("void CallPlus(unsigned short* value", 1)[0]
    percent = function(read_source("src/Core/Runtime_Small.cpp", revision), "PlusSpecialPercent")
    call = "return reinterpret_cast<std::uintptr_t>(PlusSpecialPercent(value, special, item, factor));" if "const ITEM* a3" in percent else "return static_cast<std::uint32_t>(PlusSpecialPercent(value, special, static_cast<int>(reinterpret_cast<std::uintptr_t>(item)), factor));"
    return prefix + percent + MAIN.replace("CALL_PERCENT", call)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-percent-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes/retorno con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
