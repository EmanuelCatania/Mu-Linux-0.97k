#!/usr/bin/env python3
"""Ejecuta CalculateAll con sus ocho cálculos reales y bases altas.

Sólo Apply del servidor y rand son servicios controlados. Compara campos
derivados, orden y bytes; no certifica protocolo, UI ni sesión del cliente.
"""
import argparse
import os
from pathlib import Path
import tempfile

from BaseAttackPointer import harness as base_harness
from SceneObjectPointer import build_and_run, function, read_source

WRAPPERS = r'''
std::vector<int> calcEvents;
BYTE beforeDerived[1412];
int randomValues[3], randomCount;
int CalcRandom() { Require(randomCount < 3); return randomValues[randomCount++]; }
void EnterCalc(const void* pointer, int stage) { Require(pointer == activeMachine); calcEvents.push_back(stage); }
int __fastcall Stats_CalcBase(BYTE* c) { EnterCalc(c, 1); return Stats_CalcBaseReal(c); }
const ITEM* Stats_CalcMagicDmgRange(BYTE* c) { EnterCalc(c, 2); return Stats_CalcMagicDmgRangeReal(c); }
int Stats_CalcAddStrength(short* c) { EnterCalc(c, 3); return Stats_CalcAddStrengthReal(c); }
int CalculateAttackSpeed(BYTE* c) { EnterCalc(c, 4); return CalculateAttackSpeedReal(c); }
const ITEM* Stats_CalcDefense(BYTE* c) { EnterCalc(c, 5); return Stats_CalcDefenseReal(c); }
int Stats_CalcDefenseRate(BYTE* c) { EnterCalc(c, 6); return Stats_CalcDefenseRateReal(c); }
int Stats_ExtraOptionEquip6(short* c) { EnterCalc(c, 7); return Stats_ExtraOptionEquip6Real(c); }
int Stats_ExtraOptionGlovesWings(BYTE* c) {
    EnterCalc(c, 8); const int result = Stats_ExtraOptionGlovesWingsReal(c);
    std::memcpy(beforeDerived, c, 1412); return result;
}
'''

MAIN = r'''
int CallCalculate(BYTE* machine) { return CALL_CALCULATE; }
unsigned short ReadWord(const BYTE* data, int offset) { unsigned short value; std::memcpy(&value, data + offset, 2); return value; }
void WriteWord(BYTE* data, int offset, unsigned short value) { std::memcpy(data + offset, &value, 2); }
std::uint64_t RunCalculate(BYTE* machine, BYTE* attribute, BYTE* allocation) {
    activeMachine = machine; seed = 0x97c47e3c; std::uint64_t digest = 14695981039346656037ull;
    DAT_055c9e44 = reinterpret_cast<std::uintptr_t>(allocation);
    DAT_07d78068 = DAT_055c9e44 + 1023 * 64; auto* items = reinterpret_cast<ITEM_ATTRIBUTE*>(DAT_07d78068);
    std::memset(allocation, 0, (ITEM_MAX_EX + 1024) * 64);
    for(int i = 0; i < 192; ++i) items[i].Durability = BYTE(1 + i % 255);
    std::vector<BYTE> beforeTable(allocation, allocation + (ITEM_MAX_EX + 1024) * 64);
    calcEvents.clear(); randomCount = applyCalls = 0;
    Require(CallCalculate(nullptr) == 0 && calcEvents.empty() && randomCount == 0 && applyCalls == 0);
    for(int trial = 0; trial < 16384; ++trial) {
        std::memset(machine, 0x5a, 1412); std::memset(attribute, 0, 64); attribute[11] = BYTE(Input());
        DAT_07cf1ff4 = trial % 7 == 0 ? nullptr : attribute;
        for(int offset : {14, 20, 22, 26, 1374, 1376, 1378, 1382, 1384, 1388, 1390}) WriteWord(machine, offset, static_cast<unsigned short>(Input()));
        if(trial % 4 == 0) { WriteWord(machine, 1374, 100); WriteWord(machine, 1376, 99); }
        if(trial % 4 == 1) { WriteWord(machine, 1374, 0); WriteWord(machine, 1376, 0); }
        machine[40] = BYTE(Input());
        for(int slot = 0; slot < 12; ++slot) {
            auto* item = reinterpret_cast<ITEM*>(machine + 536 + 68*slot); std::memset(item, 0, sizeof(*item));
            item->Type = Input() % 5 == 0 ? -1 : short(Input() % 192); item->Durability = item->Type == -1 ? 0 : BYTE(Input());
            item->Level = int(Input() % 16) * 8; item->Option1 = BYTE(Input() % 64);
            item->DamageMin = static_cast<unsigned short>(Input()); item->DamageMax = static_cast<unsigned short>(Input());
            item->Defense = static_cast<unsigned short>(Input()); item->MagicDefense = static_cast<unsigned short>(Input());
            item->WalkSpeed = static_cast<unsigned short>(Input()); item->WeaponSpeed = BYTE(Input()); item->SuccessfulBlocking = BYTE(Input());
            item->SpecialNum = BYTE(Input() % 10); auto* data = reinterpret_cast<BYTE*>(item);
            const BYTE ids[] = {60, 61, 62, 63, 70, 73, 74, 75, 76, 77};
            for(int j = 0; j < 9; ++j) data[37+j] = ids[Input() % 10];
            for(int j = 0; j < 9; ++j) data[45+j] = BYTE(Input());
        }
        g_bAddDefense = int(Input() % 2); EquipmentLevelSet = 9 + int(Input() % 4);
        serverApply = trial % 11 == 0; serverRate = static_cast<unsigned short>(Input());
        for(int& value : randomValues) value = int(Input() % 32768);
        if(trial % 3 == 0) { randomValues[0] = 0; randomValues[1] = 99; randomValues[2] = 100; }
        BYTE beforeAttribute[64]; std::memcpy(beforeAttribute, attribute, 64);
        calcEvents.clear(); randomCount = applyCalls = 0;
        const int returned = CallCalculate(machine);
        Require(calcEvents == std::vector<int>({1,2,3,4,5,6,7,8}) && randomCount == 3 && applyCalls == 5);
        BYTE expected[1412]; std::memcpy(expected, beforeDerived, 1412);
        WriteWord(expected, 1396, static_cast<unsigned short>(ReadWord(beforeDerived, 1388) - ReadWord(beforeDerived, 1378)));
        WriteWord(expected, 1398, static_cast<unsigned short>(ReadWord(beforeDerived, 1390) - ReadWord(beforeDerived, 1378)));
        const int minimum = ReadWord(beforeDerived, 1374);
        const int range = int(ReadWord(beforeDerived, 1376)) - minimum + 1;
        WriteWord(expected, 1400, static_cast<unsigned short>(minimum + randomValues[0] % (range < 1 ? 1 : range) - ReadWord(beforeDerived, 78)));
        const auto rawCrit = static_cast<unsigned short>(ReadWord(beforeDerived, 58) - ReadWord(beforeDerived, 1384));
        const auto rawDodge = static_cast<unsigned short>(ReadWord(beforeDerived, 76) - ReadWord(beforeDerived, 1382));
        WriteWord(expected, 1402, rawCrit > 100 ? 100 : rawCrit); WriteWord(expected, 1404, rawDodge > 100 ? 100 : rawDodge);
        expected[1406] = randomValues[1] % 100 < ReadWord(expected, 1402); expected[1407] = randomValues[2] % 100 < ReadWord(expected, 1404);
        Require(returned == randomValues[2] / 100 && std::memcmp(machine, expected, 1412) == 0 && std::memcmp(attribute, beforeAttribute, 64) == 0);
        for(int i = 0; i < 1412; ++i) { digest ^= machine[i]; digest *= 1099511628211ull; }
        digest ^= unsigned(returned); digest *= 1099511628211ull;
    }
    Require(std::memcmp(allocation, beforeTable.data(), beforeTable.size()) == 0);
    return digest;
}
int main() {
    auto* machine = static_cast<BYTE*>(Reserve(0x76000000u)); auto* attribute = static_cast<BYTE*>(Reserve(0x75000000u)); auto* items = static_cast<BYTE*>(Reserve(0x74000000u)); Require(machine && attribute && items);
    const auto expected = RunCalculate(machine, attribute, items);
    if(sizeof(void*) > 4) {
        auto* highMachine = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttribute = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highItems = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highAttribute && highItems);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(machine, 0, 1412); if(mask & 2) std::memset(attribute, 0, 64); if(mask & 4) std::memset(items, 0, (ITEM_MAX_EX + 1024) * 64);
            Require(RunCalculate(mask & 1 ? highMachine : machine, mask & 2 ? highAttribute : attribute, mask & 4 ? highItems : items) == expected);
        }
        std::puts("CalculateAll sobre 4 GiB: ocho combinaciones, ocho calculos reales");
    }
    std::printf("CalculateAll: 16384 estados, orden/azar/derivados; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = base_harness(revision).split("int CallBase(BYTE* machine)", 1)[0]
    core, scene = read_source("src/Core/Runtime_Small.cpp", revision), read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    for name in ("FUN_0047cfe0", "FUN_0047d000"):
        prefix += function(core, name)
    names = ("Stats_CalcBase", "Stats_CalcMagicDmgRange", "Stats_CalcAddStrength", "CalculateAttackSpeed", "Stats_CalcDefense", "Stats_CalcDefenseRate", "Stats_ExtraOptionEquip6", "Stats_ExtraOptionGlovesWings")
    for name in ("Stats_CalcMagicDmgRange", "Stats_CalcAddStrength", "CalculateAttackSpeed", "Stats_ExtraOptionEquip6", "Stats_ExtraOptionGlovesWings"):
        prefix += function(scene, name)
    for name in names:
        prefix = prefix.replace(name + "(", name + "Real(")
    calculate = function(read_source("src/Net/SecondPassword.cpp", revision), "CalculateAll").replace("rand()", "CalcRandom()")
    arg = "machine" if "BYTE* characterMachine" in calculate else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + WRAPPERS + calculate + MAIN.replace("CALL_CALCULATE", f"CalculateAll({arg}, 123, -456)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-calculate-all-") as temp:
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
