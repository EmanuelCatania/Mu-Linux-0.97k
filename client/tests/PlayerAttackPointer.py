#!/usr/bin/env python3
"""Compara ataques reales y tablas sobre 4 GiB con la referencia x86.

Usa fixtures escalares x86; efectos, velocidad y azar son servicios controlados.
No certifica los layouts nativos x64 ni una sesión de juego.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from PlayerDiePointer import harness as die_harness, SERVICES as EFFECTS
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
int attackSpeedCalls;
void SetAttackSpeed() { ++attackSpeedCalls; events.push_back(40000); }
void CallAttack(CHARACTER* character) { CALL_ATTACK; }
std::uint64_t RunAttack(CHARACTER* character, BYTE* models, BYTE* items) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    DAT_07d78068 = reinterpret_cast<std::uintptr_t>(items + 65536);
    activeCharacter = character; g_EnableSound = TRUE;
    for(int slot = 0; slot < 420; ++slot) { MaxBufferChannel[slot] = 4; Enable3DSound[slot] = true; }
    seed = 0x97c44441; digest = 14695981039346656037ull;
    events.clear(); CallAttack(nullptr); Require(events.empty());
    bool actions[256] = {}, effects[2] = {};
    const short types[] = {0, 39, 40, 51, 302, 390, 390, 390};
    const short weapons[] = {-1, 399, 400, 410, 431, 495, 496, 497, 498, 500, 501, 527,
                             528, 534, 535, 536, 542, 543, 544, 545, 546, 547, 559, 560, 591, 592};
    const short helpers[] = {-1, 818, 819, 820}, wings[] = {-1, 700};
    const int subtypes[] = {0, 205, 206, 207, 208, 209};
    const short limits[] = {0, 1, 4, 34, 40, 50, 86, 256};
    const DWORD frames[] = {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u};
    for(int trial = 0; trial < 131072; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Object.SubType = Pick(subtypes);
        character->Weapon[0].Type = Pick(weapons); character->Weapon[1].Type = Pick(weapons);
        character->Helper.Type = Pick(helpers); character->Wing.Type = Pick(wings);
        character->SafeZone = BYTE(Input() % 2); character->SwordCount = BYTE(trial % 256);
        character->Object.CurrentAction = BYTE(Input() % 256);
        const DWORD frame = Pick(frames); std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        for(int i = 0; i < 3; ++i) {
            character->Object.Position[i] = trial * 0.125f + i;
            character->Object.Angle[i] = i * 45.0f; character->Object.Light[i] = i * 0.25f;
        }
        const short limit = Pick(limits), voice = Input() % 2 ? -1 : 1;
        std::memset(models, 0xa5, 391 * 188);
        std::memcpy(models + character->Object.Type * 188 + 38, &limit, 2);
        std::memcpy(models + character->Object.Type * 188 + 174, &voice, 2);
        std::memcpy(models + character->Object.Type * 188 + 176, &voice, 2);
        std::memset(items, Input() % 2, 0x40000);
        for(int& value : randomValues) value = int(Input() % 32768);
        randomDraws = actionCalls = attackSpeedCalls = 0; events.clear(); soundEvents.clear();
        const BYTE count = character->SwordCount; const short type = character->Object.Type;
        CallAttack(character);
        Require(attackSpeedCalls == int(type == 390));
        const int increment = type != 390 && type != 39 && type != 40 && type != 51 && type != 302 ? 2 : 1;
        Require(character->SwordCount == BYTE(count + increment));
        if(actionCalls) { Require(actionCalls == 1); actions[requestedAction] = true; }
        for(int event : events) { if(event == 209) effects[0] = true; if(event == 1196) effects[1] = true; }
        Hash(character, sizeof(*character)); Hash(&randomDraws, sizeof(randomDraws));
        for(int event : events) Hash(&event, sizeof(event));
        const auto size = std::uint32_t(events.size()); Hash(&size, sizeof(size));
    }
    for(int action : {1, 3, 4, 8, 9, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
                       46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 81, 84, 85}) Require(actions[action]);
    Require(effects[0] && effects[1]); return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x74000000u));
    auto* items = static_cast<BYTE*>(Reserve(0x75000000u)); Require(character && models && items);
    const auto expected = RunAttack(character, models, items);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull)));
        auto* highItems = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highCharacter && highModels && highItems);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 2) std::memset(models, 0, 391 * 188);
            if(mask & 4) std::memset(items, 0xff, 0x40000);
            Require(RunAttack(mask & 1 ? highCharacter : character, mask & 2 ? highModels : models,
                              mask & 4 ? highItems : items) == expected);
        }
        std::puts("ataques sobre 4 GiB: ocho combinaciones personaje/modelos/items equivalentes");
    }
    std::printf("SetPlayerAttack: 131072 estados, 30 acciones y efectos; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = die_harness(revision).split("void CallDie(CHARACTER* character)", 1)[0]
    # El banco reutilizado necesita cuatro sorteos y el sonido global del tipo 51.
    prefix = prefix.replace("randomValues[2]", "randomValues[8]").replace("randomDraws < 2", "randomDraws < 8")
    prefix = prefix.replace("owner == AsObject(activeCharacter) && loop == 0", "(owner == AsObject(activeCharacter) || (slot == 91 && owner == nullptr)) && loop == 0")
    prefix = prefix[:prefix.index(EFFECTS)]
    structs = read_source("src/structs.h", revision)
    record = "\n".join(re.findall(r"^#define\s+(?:MAX_CLASS|MAX_RESISTANCE)\s+[^\n]+", structs, re.M))
    record += "\n" + re.search(r"struct ITEM_ATTRIBUTE\b.*?\n\};", structs, re.S).group()
    record += "\nstatic_assert(sizeof(ITEM_ATTRIBUTE) == 64 && offsetof(ITEM_ATTRIBUTE, TwoHand) == 30);\n"
    attack = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "SetPlayerAttack").replace("rand()", "NextRandom()")
    arg = "character" if "CHARACTER* character" in attack else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    # La definición de SetAttackSpeed precede a la función extraída.
    services, main = MAIN.split("void CallAttack", 1)
    return prefix + record + EFFECTS + services + attack + "void CallAttack" + main.replace("CALL_ATTACK", f"SetPlayerAttack({arg}, 0, 0, 0)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-attack-pointer-") as temp:
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
