#!/usr/bin/env python3
"""Prueba copia de equipo y stats reales; direcciones altas con helper estable.

Catálogo, parada y mascotas controlados. El owner de DeleteBug sigue siendo
DWORD (B2): cambios de helper se comparan sólo en x86/direcciones bajas.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from CalculateAllPointer import harness as calculate_harness
from ItemAnglePointer import harness as item_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
CHARACTER* activeCharacter;
bool stableHelper;
int classCalculations, catalogParts[9];
std::vector<int> classEvents;
struct Catalog {
    int GetItemModel(int type) const { return type < 0 ? -1 : type + 400; }
    int GetItemBehavior(int type) const {
        if(type >= 600 && type <= 603) return 416 + type - 600;
        if(type == 700) return -1;
        return type >= 512 ? (type - 512) % 512 : type;
    }
    const void* GetPet(int type) const { return type >= 600 && type <= 603 ? this : nullptr; }
    void SetEntityPart(const void* owner, int part, int type) {
        Require(owner == activeCharacter && part >= 0 && part < 9);
        catalogParts[part] = type; classEvents.push_back(100 + part);
    }
} gContentCatalog;
void DeleteBug(DWORD owner) {
    Require(!stableHelper && owner == static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(activeCharacter)));
    classEvents.push_back(200);
}
void CreateBug(int type, void* position, void* owner, int flag) {
    Require(!stableHelper && position == activeCharacter->Object.Position && owner == activeCharacter && flag == 0);
    classEvents.push_back(1000 + type);
}
void SetPlayerStop(CHARACTER* c) { Require(c == activeCharacter); classEvents.push_back(300); }
void SetCharacterScale(CHARACTER* c) { Require(c == activeCharacter); classEvents.push_back(301); SetCharacterScaleReal(c); }
int CalculateAll(BYTE* c, int a, int b) {
    Require(c == activeMachine && a == 0 && b == 0); ++classCalculations; classEvents.push_back(302);
    return CalculateAllReal(c, a, b);
}
'''

MAIN = r'''
void CallClass(CHARACTER* character) { CALL_CLASS; }
std::uint64_t RunClass(CHARACTER* character, BYTE* machine, BYTE* attribute, BYTE* allocation, bool stable) {
    activeCharacter = character; activeMachine = machine; stableHelper = stable; seed = 0x97c45c13; std::uint64_t digest = 14695981039346656037ull;
    DAT_055c9e44 = reinterpret_cast<std::uintptr_t>(allocation); DAT_07d78068 = DAT_055c9e44;
    auto* items = reinterpret_cast<ITEM_ATTRIBUTE*>(allocation); std::memset(allocation, 0, (ITEM_MAX_EX + 1024) * 64);
    for(int i = 0; i < ITEM_MAX_EX; ++i) items[i].Durability = 16;
    std::vector<BYTE> beforeTable(allocation, allocation + (ITEM_MAX_EX + 1024) * 64);
    const short itemTypes[] = {-1, 0, 31, 128, 135, 143, 224, 226, 234, 235, 236, 237, 416, 417, 418, 419, 512, 600, 601, 602, 603, 700};
    const BYTE actions[] = {0, 33, 34, 91, 92, 132, 133, 140, 141, 255};
    const short previous[] = {-1, 816, 817, 818, 819, 999};
    for(int trial = 0; trial < 16384; ++trial) {
        std::memset(character, 0x5a, sizeof(*character)); std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64);
        character->Object.Type = trial % 3 ? 390 : 391; character->Object.CurrentAction = actions[Input() % 10];
        character->Class = BYTE(trial); character->Skin = BYTE(Input() % 2); character->Change = BYTE(Input() % 2);
        attribute[11] = BYTE(Input()); DAT_07cf1ff4 = attribute; DAT_07cf1ffc = character->Object.Type == 390 ? machine : nullptr;
        for(int slot = 0; slot < 12; ++slot) {
            auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*slot); std::memset(item, 0, sizeof(*item));
            item->Type = itemTypes[Input() % 22]; item->Durability = item->Type == -1 ? 0 : BYTE(Input());
            item->Level = int(Input() % 16) * 8; item->Option1 = BYTE(Input());
            item->DamageMin = static_cast<unsigned short>(Input()); item->DamageMax = static_cast<unsigned short>(Input());
            item->Defense = static_cast<unsigned short>(Input()); item->MagicDefense = static_cast<unsigned short>(Input()); item->WalkSpeed = static_cast<unsigned short>(Input());
        }
        const short pet = reinterpret_cast<ITEM*>(machine + 1080)->Type;
        const short helper = pet == -1 ? -1 : short(ItemModel(ItemBehaviorType(pet)));
        character->Helper.Type = stable ? helper : previous[Input() % 6];
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        std::vector<int> expectedEvents; int expectedParts[9]; for(int& value : expectedParts) value = -9999;
        const bool player = expected.Object.Type == 390;
        if(player) {
            const int sourceOffsets[] = {0, 68, 476, 544}; const int targetOffsets[] = {624, 648, 672, 696};
            for(int i = 0; i < 4; ++i) {
                const short type = reinterpret_cast<ITEM*>(machine + 536 + sourceOffsets[i])->Type;
                const short model = type == -1 ? -1 : short(ItemEntityModel(type)); std::memcpy(reinterpret_cast<BYTE*>(&expected) + targetOffsets[i], &model, 2);
            }
            expected.Helper.Type = helper;
            const int wear[] = {136, 204, 272, 340, 408, 0, 68, 476, 544};
            for(int part = 0; part < 9; ++part) { expectedParts[part] = reinterpret_cast<ITEM*>(machine + 536 + wear[part])->Type; expectedEvents.push_back(100 + part); }
            if(helper != character->Helper.Type) {
                expectedEvents.push_back(200); int bug = helper == 816 ? 816 : helper == 818 ? 195 : helper == 819 ? 267 : gContentCatalog.GetPet(pet) ? helper : 0;
                if(bug) expectedEvents.push_back(1000 + bug);
            }
            for(int i = 0; i < 2; ++i) { const auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*i); expected.Weapon[i].Level = BYTE((item->Level >> 3) & 15); expected.Weapon[i].Option1 = item->Option1; }
            expected.Wing.Level = BYTE((reinterpret_cast<ITEM*>(machine + 1012)->Level >> 3) & 15); expected.Helper.Level = BYTE((reinterpret_cast<ITEM*>(machine + 1080)->Level >> 3) & 15);
            const BYTE action = expected.Object.CurrentAction;
            if(!(action >= 133 && action <= 140) && !(action >= 34 && action <= 91)) expectedEvents.push_back(300);
            if(!expected.Change) {
                for(int i = 1; i <= 5; ++i) {
                    const auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*(i+1));
                    expected.BodyPart[i].Type = item->Type == -1 ? short((expected.Class & 7) + 4*(expected.Class >> 3) + 905 + 7*i) : short(ItemEntityModel(item->Type));
                    expected.BodyPart[i].Level = BYTE((item->Level >> 3) & 15); expected.BodyPart[i].Option1 = item->Option1;
                }
                SetCharacterScaleReal(&expected); expectedEvents.push_back(301); expectedEvents.push_back(302);
            }
        }
        for(int& value : catalogParts) value = -9999;
        classEvents.clear(); calcEvents.clear(); classCalculations = randomCount = applyCalls = 0;
        serverApply = trial % 11 == 0; serverRate = static_cast<unsigned short>(Input()); for(int& value : randomValues) value = int(Input() % 32768);
        BYTE beforeMachine[1412]; std::memcpy(beforeMachine, machine, 1412);
        CallClass(character);
        const bool recalculate = player && !expected.Change;
        Require(classEvents == expectedEvents && std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(std::memcmp(catalogParts, expectedParts, sizeof(expectedParts)) == 0 && classCalculations == int(recalculate));
        Require(randomCount == (recalculate ? 3 : 0) && applyCalls == (recalculate ? 5 : 0));
        if(!recalculate) Require(std::memcmp(beforeMachine, machine, 1412) == 0);
        else Require(calcEvents == std::vector<int>({1,2,3,4,5,6,7,8}));
        for(const BYTE* bytes : {reinterpret_cast<BYTE*>(character), machine}) {
            const int count = bytes == machine ? 1412 : int(sizeof(*character));
            for(int i = 0; i < count; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
        }
        for(int event : classEvents) { digest ^= unsigned(event); digest *= 1099511628211ull; }
    }
    Require(std::memcmp(allocation, beforeTable.data(), beforeTable.size()) == 0); return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u)); auto* machine = static_cast<BYTE*>(Reserve(0x75000000u));
    auto* attribute = static_cast<BYTE*>(Reserve(0x73000000u)); auto* items = static_cast<BYTE*>(Reserve(0x72000000u)); Require(character && machine && attribute && items);
    const auto stable = RunClass(character, machine, attribute, items, true), legacy = RunClass(character, machine, attribute, items, false);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highCharacter && highMachine);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(character, 0, sizeof(*character)); if(mask & 2) std::memset(machine, 0, 1412);
            Require(RunClass(mask & 1 ? highCharacter : character, mask & 2 ? highMachine : machine, attribute, items, true) == stable);
        }
        std::puts("clase sobre 4 GiB: cuatro combinaciones, solo helper estable; pool de mascotas pendiente B2");
    }
    std::printf("SetCharacterClass: 16384 estados por modo; estable %016llx, cambios bajos %016llx\n", static_cast<unsigned long long>(stable), static_cast<unsigned long long>(legacy));
}
'''


def harness(revision=None):
    prefix = calculate_harness(revision).split("int CallCalculate(BYTE* machine)", 1)[0]
    scalar = re.search(r"struct OBJECT\s*\{.*?\};", item_harness(revision), re.S).group()
    prefix = prefix.replace("struct OBJECT { BYTE data[444]; };", scalar, 1).replace("CalculateAll(", "CalculateAllReal(")
    header, globals_source = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    name = "DAT_07cf1ffc"
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group() + re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    prefix += function(read_source("src/Render/SMD_Parser.cpp", revision), "SetCharacterScale").replace("SetCharacterScale(", "SetCharacterScaleReal(", 1)
    prefix += SERVICES + function(read_source("src/Item/ItemDefines.h", revision), "GetItemSection")
    catalog = read_source("src/Item/ContentCatalog.h", revision)
    for name in ("ItemModel", "ItemBehaviorType", "ItemEntityModel"):
        prefix += function(catalog, name)
    character = function(read_source("src/Net/SecondPassword.cpp", revision), "SetCharacterClass")
    arg = "character" if "CHARACTER* character" in character else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + character + MAIN.replace("CALL_CLASS", f"SetCharacterClass({arg})")


def main():
    import sys
    if "Win32" not in sys.argv:
        print("Omitido en x64: banco atado al layout x86; lo cubren los bancos nativos de B2")
        return
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-character-class-") as temp:
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
