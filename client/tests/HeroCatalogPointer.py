#!/usr/bin/env python3
"""Prueba reaplicación del catálogo; buffers altos sin pet custom.

Catálogo y callbacks de mascotas controlados. Cambios de pet custom sólo con
direcciones bajas: DeleteBug conserva owner DWORD y queda pendiente de B2.
"""
import argparse
import os
from pathlib import Path
import tempfile

from CharacterClassPointer import harness as class_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void CallHero(CHARACTER* character) { CALL_HERO; }
std::uint64_t RunHero(CHARACTER* character, BYTE* machine, bool noCustom) {
    activeCharacter = character; activeMachine = machine; stableHelper = noCustom; seed = 0x97c09720; std::uint64_t digest = 14695981039346656037ull;
    const short types[] = {-1, 0, 31, 128, 135, 416, 417, 418, 419, 512, 600, 601, 602, 603, 700};
    const short plainPets[] = {-1, 416, 417, 418, 419, 700};
    DAT_07cf1ffc = nullptr; classEvents.clear(); CallHero(nullptr); Require(classEvents.empty());
    for(int trial = 0; trial < 16384; ++trial) {
        std::memset(character, 0x5a, sizeof(*character)); std::memset(machine, 0xa5, 1412);
        character->Object.Type = trial % 3 ? 390 : 391;
        for(int slot = 0; slot < 12; ++slot) {
            const short type = types[Input() % 15]; std::memcpy(machine + 536 + 68*slot, &type, 2);
        }
        if(noCustom) { const short pet = plainPets[Input() % 6]; std::memcpy(machine + 1080, &pet, 2); }
        DAT_07cf1ffc = trial % 5 == 0 ? nullptr : machine;
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        BYTE beforeMachine[1412]; std::memcpy(beforeMachine, machine, 1412);
        std::vector<int> expectedEvents; int expectedParts[9]; for(int& value : expectedParts) value = -9999;
        if(DAT_07cf1ffc && expected.Object.Type == 390) {
            const int wear[] = {136,204,272,340,408,0,68,476,544};
            for(int part = 0; part < 9; ++part) {
                short type; std::memcpy(&type, machine + 536 + wear[part], 2); expectedParts[part] = type; expectedEvents.push_back(100 + part);
            }
            short pet; std::memcpy(&pet, machine + 1080, 2);
            if(pet != -1 && gContentCatalog.GetPet(pet)) {
                expected.Helper.Type = short(ItemModel(ItemBehaviorType(pet)));
                expectedEvents.push_back(200); expectedEvents.push_back(1000 + expected.Helper.Type);
            }
        }
        classEvents.clear(); for(int& value : catalogParts) value = -9999;
        CallHero(character);
        Require(classEvents == expectedEvents && std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(std::memcmp(catalogParts, expectedParts, sizeof(expectedParts)) == 0 && std::memcmp(machine, beforeMachine, 1412) == 0);
        for(int i = 0; i < int(sizeof(*character)); ++i) { digest ^= reinterpret_cast<BYTE*>(character)[i]; digest *= 1099511628211ull; }
        for(int part : catalogParts) { digest ^= unsigned(part); digest *= 1099511628211ull; }
        for(int event : classEvents) { digest ^= unsigned(event); digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u)); auto* machine = static_cast<BYTE*>(Reserve(0x75000000u)); Require(character && machine);
    const auto plain = RunHero(character, machine, true), legacy = RunHero(character, machine, false);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highCharacter && highMachine);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(character, 0, sizeof(*character)); if(mask & 2) std::memset(machine, 0, 1412);
            Require(RunHero(mask & 1 ? highCharacter : character, mask & 2 ? highMachine : machine, true) == plain);
        }
        std::puts("catalogo sobre 4 GiB: cuatro combinaciones sin pet custom; pool pendiente B2");
    }
    std::printf("Hero_ApplyCatalogEquipment: 16384 estados por modo; sin pet %016llx, cambios bajos %016llx\n", static_cast<unsigned long long>(plain), static_cast<unsigned long long>(legacy));
}
'''


def harness(revision=None):
    prefix = class_harness(revision).split("void CallClass(CHARACTER* character)", 1)[0]
    hero = function(read_source("src/Net/SecondPassword.cpp", revision), "Hero_ApplyCatalogEquipment")
    arg = "character" if "CHARACTER* character" in hero else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + hero + MAIN.replace("CALL_HERO", f"Hero_ApplyCatalogEquipment({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if getattr(args, "architecture", "x64") == "x64":
        print("Omitido en x64: banco atado al layout x86; lo cubren los bancos nativos de B2")
        return
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-hero-catalog-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes/eventos con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
