#!/usr/bin/env python3
"""Verifica rango mágico real con máquina y tabla de atributos altas.

Opciones y durabilidad reales; Apply del servidor controlado. No certifica
CalculateAll, protocolo, hover de inventario ni interfaz visible.
"""
import argparse
import os
from pathlib import Path
import tempfile

from DefenseRatePointer import harness as rate_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
const ITEM* CallMagic(BYTE* machine) { return CALL_MAGIC; }
std::uint64_t RunMagic(BYTE* machine, ITEM_ATTRIBUTE* items) {
    activeMachine = machine; seed = 0x97c47dae; std::uint64_t digest = 14695981039346656037ull;
    DAT_07d78068 = reinterpret_cast<std::uintptr_t>(items);
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(items, 0, 32 * 64);
        const unsigned short energy = static_cast<unsigned short>(Input()); std::memcpy(machine + 26, &energy, 2);
        for(int offset : {536, 1012, 1148}) {
            auto* item = reinterpret_cast<ITEM*>(machine + offset); std::memset(item, 0, sizeof(*item));
            item->Type = Input() % 5 == 0 ? -1 : short(Input() % 32); item->Durability = BYTE(Input());
            item->Level = int(Input() % 16) * 8; item->Option1 = BYTE(Input() % 64); item->SpecialNum = BYTE(Input() % 10);
            auto* data = reinterpret_cast<BYTE*>(item);
            for(int j = 0; j < 9; ++j) data[37+j] = j % 4 == 0 ? BYTE(60) : j % 4 == 1 ? BYTE(61) : j % 4 == 2 ? BYTE(75) : BYTE(76);
            for(int j = 0; j < 9; ++j) data[45+j] = BYTE(Input());
            if(item->Type >= 0) items[item->Type].Durability = BYTE(1 + Input() % 255);
        }
        serverApply = trial % 11 == 0; serverRate = static_cast<unsigned short>(Input()); serverMaximum = static_cast<unsigned short>(Input()); applyCalls = 0;
        BYTE expected[1412], beforeItems[32 * 64]; std::memcpy(expected, machine, 1412); std::memcpy(beforeItems, items, sizeof(beforeItems));
        const ITEM* expectedReturn = nullptr;
        if(serverApply) { std::memcpy(expected + 70, &serverRate, 2); std::memcpy(expected + 72, &serverMaximum, 2); }
        else {
            unsigned short minimum = energy / 9, maximum = energy / 4;
            auto* wings = reinterpret_cast<ITEM*>(machine + 1012); auto* weapon = reinterpret_cast<ITEM*>(machine + 536); auto* ring = reinterpret_cast<ITEM*>(machine + 1148);
            auto scaledBonus = [&](ITEM* item, int special, int option) {
                unsigned short bonus = 0; ExpectedPlus(reinterpret_cast<BYTE*>(item), &bonus, special);
                const float reduction = CalcDurabilityPercent(item->Durability, items[item->Type].Durability, item->Level, option);
                return static_cast<unsigned short>(bonus - static_cast<unsigned short>(double(bonus) * double(reduction)));
            };
            if(wings->Type != -1) {
                const auto bonus = scaledBonus(wings, 61, 0); minimum = static_cast<unsigned short>(minimum + bonus); maximum = static_cast<unsigned short>(maximum + bonus);
            }
            if(weapon->Type != -1 && weapon->Durability) {
                const auto bonus = scaledBonus(weapon, weapon->Type == 31 ? 60 : 61, weapon->Option1);
                minimum = static_cast<unsigned short>(minimum + bonus); maximum = static_cast<unsigned short>(maximum + bonus);
                ExpectedPlus(reinterpret_cast<BYTE*>(weapon), &minimum, 75); ExpectedPlus(reinterpret_cast<BYTE*>(weapon), &maximum, 75);
                PlusSpecialPercent(reinterpret_cast<short*>(&minimum), 76, weapon, 2); PlusSpecialPercent(reinterpret_cast<short*>(&maximum), 76, weapon, 2);
            }
            if(ring->Type != -1 && ring->Durability) {
                ExpectedPlus(reinterpret_cast<BYTE*>(ring), &minimum, 75); ExpectedPlus(reinterpret_cast<BYTE*>(ring), &maximum, 75);
                PlusSpecialPercent(reinterpret_cast<short*>(&minimum), 76, ring, 2); PlusSpecialPercent(reinterpret_cast<short*>(&maximum), 76, ring, 2);
            }
            expectedReturn = ring; std::memcpy(expected + 70, &minimum, 2); std::memcpy(expected + 72, &maximum, 2);
        }
        const auto* returned = CallMagic(machine);
        Require(returned == expectedReturn && applyCalls == 1 && std::memcmp(machine, expected, 1412) == 0);
        Require(std::memcmp(items, beforeItems, sizeof(beforeItems)) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        digest ^= returned != nullptr; digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* items = static_cast<ITEM_ATTRIBUTE*>(Reserve(0x75000000u)); Require(machine && items);
    const auto expected = RunMagic(machine, items);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highItems = static_cast<ITEM_ATTRIBUTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highMachine && highItems);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(items, 0, 32 * 64);
            Require(RunMagic(mask & 1 ? highMachine : machine, mask & 2 ? highItems : items) == expected);
        }
        std::puts("magia sobre 4 GiB: cuatro combinaciones y retorno de anillo completo");
    }
    std::printf("Stats_CalcMagicDmgRange: 32768 estados; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = rate_harness(revision).split("int CallRate(BYTE* machine)", 1)[0]
    prefix = prefix.replace("unsigned short serverRate;", "unsigned short serverRate, serverMaximum;")
    prefix = prefix.replace("std::memcpy(activeMachine + 78, &serverRate, 2);", "{ std::memcpy(activeMachine + 70, &serverRate, 2); std::memcpy(activeMachine + 72, &serverMaximum, 2); }")
    prefix += function(read_source("src/Core/Runtime_Small.cpp", revision), "PlusSpecialPercent")
    magic = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Stats_CalcMagicDmgRange")
    arg = "machine" if "BYTE* param_1" in magic else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + magic + MAIN.replace("CALL_MAGIC", f"Stats_CalcMagicDmgRange({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-magic-range-") as temp:
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
