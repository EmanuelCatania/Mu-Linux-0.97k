#!/usr/bin/env python3
"""Prueba casteo mágico real con SetAction y direcciones sobre 4 GiB.

Reutiliza fixtures escalares x86; velocidad de ataque y azar son servicios
controlados. No certifica CHARACTER nativo x64 ni efectos de una sesión.
"""
import argparse
import os
from pathlib import Path
import tempfile

from PlayerWalkPointer import harness as walk_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#ifndef _WIN32
using UINT = unsigned int;
#endif
int attackSpeedCalls;
void SetAttackSpeed() { Require(actionCalls == 0); ++attackSpeedCalls; }
'''

MAIN = r'''
void CallMagic(CHARACTER* character) { CALL_MAGIC; }
std::uint64_t RunMagic(CHARACTER* character, BYTE* models) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    activeCharacter = character; digest = 14695981039346656037ull; seed = 0x97c444a8;
    actionCalls = attackSpeedCalls = randomDraws = 0;
    CallMagic(nullptr); Require(actionCalls == 0 && attackSpeedCalls == 0 && randomDraws == 0);
    bool actions[256] = {}, classes[256] = {};
    const short types[] = {0, 390, 390, 297}, helpers[] = {-1, 818, 819, 820};
    const short limits[] = {-1, 0, 3, 4, 5, 82, 83, 86, 87, 91, 92, 256};
    const DWORD frames[] = {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u};
    const int random[] = {0, 1, 2, 3, 32767};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Class = BYTE(Input() % 256);
        classes[character->Class] = true;
        character->Helper.Type = Pick(helpers); character->SafeZone = BYTE(Input() % 2);
        character->SwordCount = BYTE(trial % 256); character->Object.CurrentAction = BYTE(Input() % 256);
        const DWORD frame = Pick(frames); std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        const short limit = Pick(limits);
        std::memset(models, 0xa5, 391 * 188);
        std::memcpy(models + character->Object.Type * 188 + 38, &limit, 2);
        randomValues[0] = Pick(random); randomDraws = actionCalls = attackSpeedCalls = 0;
        soundEvents.clear(); audio.calls = 0;
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        int expectedAction, expectedRandom = 0;
        const bool player = expected.Object.Type == 390;
        if(!player) {
            expectedAction = expected.SwordCount % 3 ? 4 : 3;
            expected.SwordCount = BYTE(expected.SwordCount + 1);
        } else if((expected.Helper.Type == 818 || expected.Helper.Type == 819) && !expected.SafeZone) expectedAction = 91;
        else if((expected.Class & 7) == 2) expectedAction = 86;
        else { expectedAction = 82 + randomValues[0] % 2; expectedRandom = 1; }
        if(expectedAction < limit && expected.Object.CurrentAction != expectedAction) {
            expected.Object.PriorAction = expected.Object.CurrentAction;
            std::memcpy(&expected.Object.PriorAnimationFrame, &frame, 4);
            expected.Object.CurrentAction = BYTE(expectedAction); expected.Object.AnimationFrame = 0.0f;
        }
        CallMagic(character);
        Require(actionCalls == 1 && requestedAction == expectedAction);
        Require(attackSpeedCalls == int(player) && randomDraws == expectedRandom);
        Require(soundEvents.empty() && audio.calls == 0);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        actions[requestedAction] = true;
        Hash(character, sizeof(*character)); Hash(&requestedAction, sizeof(requestedAction));
        Hash(&randomDraws, sizeof(randomDraws)); Hash(&attackSpeedCalls, sizeof(attackSpeedCalls));
    }
    for(int action : {3, 4, 82, 83, 86, 91}) Require(actions[action]);
    for(bool covered : classes) Require(covered);
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x74000000u)); Require(character && models);
    const auto expected = RunMagic(character, models);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highCharacter && highModels);
        Require(RunMagic(character, highModels) == expected);
        Require(RunMagic(highCharacter, models) == expected);
        Require(RunMagic(highCharacter, highModels) == expected);
        std::puts("magia sobre 4 GiB: cuatro combinaciones de personaje/modelos equivalentes");
    }
    std::printf("SetPlayerMagic: 32768 estados, seis acciones y contador de byte; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = walk_harness(revision).split("void CallWalk(CHARACTER* character)", 1)[0]
    magic = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "SetPlayerMagic").replace("rand()", "NextRandom()")
    arg = "character" if "CHARACTER* c" in magic else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + SERVICES + magic + MAIN.replace("CALL_MAGIC", f"SetPlayerMagic({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-magic-pointer-") as temp:
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
