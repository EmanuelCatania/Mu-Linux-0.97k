#!/usr/bin/env python3
"""Verifica seguimiento de cabeza real con personajes/pool sobre 4 GiB.

OBJECT es fixture escalar x86; azar controlado. No valida animación visual ni
layout nativo x64. CreateAngle y FarAngle se extraen de producción.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from CharacterPositionPointer import harness as position_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void CallHead(CHARACTER* character) { CALL_HEAD; }
void CallLook(CHARACTER* character, const CHARACTER* target) { CALL_LOOK; }
void ExpectedLook(CHARACTER& character, const CHARACTER* target) {
    if(!target) return;
    OBJECT& o = character.Object;
    const auto& t = target->Object;
    const float angle = CreateAngle(o.Position[0], o.Position[1], t.Position[0], t.Position[1]);
    if(double(FarAngle(o.Angle[2], angle, 1)) < double(_DAT_00552848)) {
        o.HeadTargetAngle[0] = o.Angle[2] - angle;
        o.HeadTargetAngle[1] = (t.Position[2] - (o.Position[2] + _DAT_00552598)) * _DAT_005526e4;
        for(int i = 0; i < 2; ++i) if(o.HeadTargetAngle[i] < _DAT_00552580) o.HeadTargetAngle[i] += _DAT_0055286c;
    } else o.HeadTargetAngle[0] = o.HeadTargetAngle[1] = 0.0f;
}
std::uint64_t RunHead(CHARACTER* character, CHARACTER* pool) {
    CharactersClient = reinterpret_cast<std::uintptr_t>(pool);
    std::memset(pool, 0, 400 * sizeof(*pool));
    seed = 0x97c43e94; digest = 14695981039346656037ull;
    const BYTE actions[] = {0, 0, 1, 2, 2, 3, 6, 255};
    const float facings[] = {-180.0f, -0.0f, 0.0f, 89.0f, 90.0f, 180.0f, 270.0f, 359.0f};
    const int draws[] = {0, 1, 31, 32, 64, 127, 128, 32767};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        const int index = trial % 400;
        auto* target = &pool[index]; std::memset(target, 0xa5, sizeof(*target));
        character->TargetCharacter = trial % 7 == 0 ? -1 : short(index);
        character->Object.CurrentAction = Pick(actions); character->Object.Angle[2] = Pick(facings);
        for(int i = 0; i < 3; ++i) {
            character->Object.Position[i] = float(int(Input() % 1024) - 512);
            target->Object.Position[i] = float(int(Input() % 1024) - 512);
        }
        // Coordenadas coincidentes, verticales y horizontales además de cuadrantes.
        if(trial % 4 == 0) target->Object.Position[0] = character->Object.Position[0];
        if(trial % 5 == 0) target->Object.Position[1] = character->Object.Position[1];
        for(int& value : randomValues) value = Pick(draws);
        CHARACTER expected, targetBefore;
        std::memcpy(&expected, character, sizeof(expected)); std::memcpy(&targetBefore, target, sizeof(targetBefore));
        int expectedDraws = 0;
        const BYTE action = expected.Object.CurrentAction;
        if(action == 0) {
            expectedDraws = 1;
            if((randomValues[0] & 31) == 0) {
                expected.Object.HeadTargetAngle[0] = float(randomValues[1] % 128 - 64);
                expected.Object.HeadTargetAngle[1] = float(randomValues[2] % 48 - 16);
                for(int i = 0; i < 2; ++i) if(expected.Object.HeadTargetAngle[i] < _DAT_00552580) expected.Object.HeadTargetAngle[i] += _DAT_0055286c;
                expectedDraws = 3;
            }
        } else if(action != 6) {
            if(action == 2 && expected.TargetCharacter != -1) ExpectedLook(expected, target);
            else expected.Object.HeadTargetAngle[0] = expected.Object.HeadTargetAngle[1] = 0.0f;
        }
        randomDraws = 0; CallHead(character);
        Require(randomDraws == expectedDraws && std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(std::memcmp(target, &targetBefore, sizeof(targetBefore)) == 0);
        Hash(character, sizeof(*character)); Hash(&randomDraws, sizeof(randomDraws));
        const CHARACTER* directTarget = trial % 3 == 0 ? nullptr : target;
        ExpectedLook(expected, directTarget); CallLook(character, directTarget);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(std::memcmp(target, &targetBefore, sizeof(targetBefore)) == 0);
        Hash(character, sizeof(*character));
    }
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* pool = static_cast<CHARACTER*>(Reserve(0x75000000u)); Require(character && pool);
    const auto expected = RunHead(character, pool);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highPool = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x175000000ull))); Require(highCharacter && highPool);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(character, 0, sizeof(*character));
            if(mask & 2) std::memset(pool, 0, 400 * sizeof(*pool));
            Require(RunHead(mask & 1 ? highCharacter : character, mask & 2 ? highPool : pool) == expected);
        }
        std::puts("cabeza sobre 4 GiB: cuatro combinaciones personaje/pool equivalentes");
    }
    std::printf("MoveHead/LookAtTarget: 32768 estados, 400 objetivos; bytes/azar %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = position_harness(revision).split("void CallPosition(CHARACTER* character)", 1)[0]
    types = read_source("src/Entity/EntityTypes.h", revision)
    head = " ".join(re.search(rf"vec3_t\s+{name}\s*;", types).group() for name in ("HeadAngle", "HeadTargetAngle"))
    prefix = prefix.replace("BYTE beforeTimer[128 - 40];", head + " BYTE beforeTimer[128 - 64];", 1)
    prefix = prefix.replace("randomValues[2]", "randomValues[4]").replace("randomDraws < 2", "randomDraws < 4")
    prefix += "\nstatic_assert(sizeof(void*) != 4 || (offsetof(OBJECT, HeadTargetAngle) == 52 && offsetof(CHARACTER, TargetCharacter) == 784));\n"
    globals_source = read_source("src/globals.cpp", revision)
    for name in ("_DAT_00552598", "_DAT_005526e4", "_DAT_00552580", "_DAT_0055286c", "_DAT_00552868", "_DAT_00552860", "_DAT_005524ec", "_DAT_00552864"):
        if not re.search(rf"\bfloat\s+{name}\s*=", prefix):
            prefix += re.search(rf"^float\s+{name}\s*=[^;]+;", globals_source, re.M).group() + "\n"
    header = read_source("src/globals.h", revision)
    prefix += re.search(r"extern [^;\n]*\bCharactersClient\s*;", header).group()
    prefix += re.search(r"^[^/\n]*\bCharactersClient\s*=\s*[^;]+;", globals_source, re.M).group()
    prefix += function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "CreateAngle")
    prefix += function(read_source("src/Path/Path_Legacy.cpp", revision), "FarAngle")
    source = read_source("src/Entity/Entity_HeadMotion.cpp", revision)
    look, move = function(source, "LookAtTarget"), function(source, "MoveHead").replace("rand()", "NextRandom()")
    call_head = "character" if "CHARACTER* character" in move else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    call_look = "&character->Object, target" if "OBJECT* o" in look else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(character)), static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(target))"
    return prefix + look + move + MAIN.replace("CALL_HEAD", f"MoveHead({call_head})").replace("CALL_LOOK", f"LookAtTarget({call_look})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-head-pointer-") as temp:
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
