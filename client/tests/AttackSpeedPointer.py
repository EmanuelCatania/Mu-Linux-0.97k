#!/usr/bin/env python3
"""Verifica velocidad de ataque real con CharacterMachine y atributos altos.

Opciones de ITEM reales; Apply del servidor controlado. No certifica
CalculateAll, protocolo ni interfaz de equipo.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemSpecialPointer import harness as special_harness, MAIN as SPECIAL_MAIN
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
BYTE* activeMachine;
bool serverApply;
unsigned short serverAttack, serverMagic;
int applyCalls;
struct ServerStats {
    bool Apply(void* pointer) {
        Require(pointer == activeMachine); ++applyCalls;
        if(serverApply) { std::memcpy(activeMachine + 56, &serverAttack, 2); std::memcpy(activeMachine + 68, &serverMagic, 2); }
        return serverApply;
    }
} gServerCharacterStats;
std::uint32_t seed;
std::uint32_t Input() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
'''

MAIN = r'''
int CallSpeed(BYTE* machine) { return CALL_SPEED; }
std::uint64_t RunSpeed(BYTE* machine, BYTE* attribute) {
    activeMachine = machine; seed = 0x97c47dd8; std::uint64_t digest = 14695981039346656037ull;
    const short types[] = {-1, 0, 31, 134, 135, 136, 142, 143, 144, 191, 192, 255};
    const BYTE durabilities[] = {0, 1, 255};
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64);
        attribute[11] = BYTE(Input() % 256); const unsigned short dex = static_cast<unsigned short>(trial);
        std::memcpy(machine + 22, &dex, 2); machine[40] = BYTE(Input() % 256);
        DAT_07cf1ff4 = trial % 5 == 0 ? nullptr : attribute;
        for(int offset : {536, 604, 876, 1080, 1148}) {
            auto* item = reinterpret_cast<ITEM*>(machine + offset); std::memset(item, 0, sizeof(*item));
            item->Type = types[Input() % 12]; item->Durability = durabilities[Input() % 3]; item->WeaponSpeed = BYTE(Input());
            item->SpecialNum = BYTE(Input() % 10); auto* bytes = reinterpret_cast<BYTE*>(item);
            for(int j = 0; j < 9; ++j) bytes[37+j] = j % 2 ? BYTE(Input() % 256) : BYTE(77);
            for(int j = 0; j < 9; ++j) bytes[45+j] = BYTE(Input());
        }
        serverApply = trial % 11 == 0; serverAttack = static_cast<unsigned short>(Input()); serverMagic = static_cast<unsigned short>(Input()); applyCalls = 0;
        BYTE expected[1412], beforeAttribute[64]; std::memcpy(expected, machine, sizeof(expected)); std::memcpy(beforeAttribute, attribute, 64);
        if(serverApply) { std::memcpy(expected + 56, &serverAttack, 2); std::memcpy(expected + 68, &serverMagic, 2); }
        else if(DAT_07cf1ff4) {
            const int cls = attribute[11] & 7;
            unsigned short attack = cls == 2 ? dex / 50 : cls == 1 || cls == 3 ? dex / 15 : dex / 20;
            unsigned short magic = cls == 2 ? dex / 50 : cls == 1 || cls == 3 ? dex / 20 : dex / 10;
            const auto* left = reinterpret_cast<ITEM*>(machine + 536); const auto* right = reinterpret_cast<ITEM*>(machine + 604);
            const auto* pants = reinterpret_cast<ITEM*>(machine + 876);
            auto valid = [](const ITEM* item) { return item->Durability && item->Type >= 0 && item->Type < 192 && item->Type != 135 && item->Type != 143; };
            int weaponBonus = valid(left) ? valid(right) ? (int(left->WeaponSpeed) + right->WeaponSpeed) / 2 : left->WeaponSpeed : valid(right) ? right->WeaponSpeed : 0;
            const int pantsBonus = pants->Durability && pants->Type != -1 ? pants->WeaponSpeed : 0;
            const int buffBonus = machine[40] & 1 ? 20 : 0;
            attack = static_cast<unsigned short>(attack + weaponBonus + pantsBonus + buffBonus);
            magic = static_cast<unsigned short>(magic + weaponBonus + pantsBonus + buffBonus);
            for(int offset : {536, 604, 1148, 1080}) {
                ExpectedPlus(machine + offset, &attack, 77); ExpectedPlus(machine + offset, &magic, 77);
            }
            std::memcpy(expected + 56, &attack, 2); std::memcpy(expected + 68, &magic, 2);
        }
        Require(CallSpeed(machine) == 0 && applyCalls == 1);
        Require(std::memcmp(machine, expected, sizeof(expected)) == 0 && std::memcmp(attribute, beforeAttribute, 64) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* attribute = static_cast<BYTE*>(Reserve(0x75000000u)); Require(machine && attribute);
    const auto expected = RunSpeed(machine, attribute);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttribute = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highMachine && highAttribute);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(attribute, 0, 64);
            Require(RunSpeed(mask & 1 ? highMachine : machine, mask & 2 ? highAttribute : attribute) == expected);
        }
        std::puts("velocidad de ataque sobre 4 GiB: cuatro combinaciones CharacterMachine/atributos equivalentes");
    }
    std::printf("CalculateAttackSpeed: 65536 estados, bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = special_harness(revision).split("void CallPlus(unsigned short* value", 1)[0]
    prefix += function(SPECIAL_MAIN, "ExpectedPlus")
    header, globals_source = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    name = "DAT_07cf1ff4"
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
    prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    speed = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "CalculateAttackSpeed")
    arg = "machine" if "BYTE* param_1" in speed else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + SERVICES + speed + MAIN.replace("CALL_SPEED", f"CalculateAttackSpeed({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-attack-speed-") as temp:
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
