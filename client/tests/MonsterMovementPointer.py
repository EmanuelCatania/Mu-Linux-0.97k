#!/usr/bin/env python3
"""Prueba despacho real de movimiento con personaje/objeto altos.

Servicios de marcha, camino y terreno controlados; no prueba pathfinding
real, animación ni layout nativo x64. Conserva fixtures escalares x86.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from CharacterPositionPointer import harness as position_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
CHARACTER* movingCharacter;
std::vector<int> moveEvents;
unsigned int advanceResult, routeResult;
void SetPlayerWalk(CHARACTER* c) { Require(c == movingCharacter); moveEvents.push_back(1); c->Run = 42; }
unsigned int Entity_AdvancePath(void* pointer, char flag) {
    Require(pointer == movingCharacter && flag == 1); moveEvents.push_back(2);
    movingCharacter->PathCurrent = 17; return advanceResult;
}
void SetPlayerStop(CHARACTER* c) { Require(c == movingCharacter); moveEvents.push_back(3); c->Run = 0; }
void MoveCharacterPosition(CHARACTER* c) { Require(c == movingCharacter); moveEvents.push_back(4); c->Object.Position[2] += 1.0f; }
unsigned int Path_FindRoute(int x, int y, int tx, int ty, unsigned char* path, float reserved) {
    Require(x == movingCharacter->PositionX && y == movingCharacter->PositionY);
    Require(tx == movingCharacter->TargetX && ty == movingCharacter->TargetY && reserved == 0.0f);
    Require(path == reinterpret_cast<BYTE*>(movingCharacter) + 852);
    moveEvents.push_back(5); path[0] = 7; return routeResult;
}
'''

MAIN = r'''
void CallMonster(CHARACTER* character, OBJECT* object) { CALL_MONSTER; }
std::uint64_t RunMonster(CHARACTER* character, OBJECT* object, char* alias) {
    seed = 0x97c454cd; digest = 14695981039346656037ull; movingCharacter = character;
    const short types[] = {0, 272, 277, 390};
    const BYTE flags[] = {0, 1, 255}, directions[] = {0, 1, 2, 7, 8, 255};
    const unsigned int results[] = {0u, 1u, 256u, 257u, 0xffffffffu};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character)); std::memset(object, 0xa5, sizeof(*object));
        character->Dead = Pick(flags); character->Movement = Pick(flags);
        reinterpret_cast<BYTE*>(character)[848] = Pick(flags);
        reinterpret_cast<BYTE*>(character)[764] = Pick(directions);
        character->PositionX = int(Input() % 256); character->PositionY = int(Input() % 256);
        character->TargetX = trial % 3 ? BYTE(Input() % 256) : BYTE(character->PositionX);
        character->TargetY = trial % 3 ? BYTE(Input() % 256) : BYTE(character->PositionY);
        character->Object.Angle[2] = 37.0f; character->Object.Position[2] = -15.0f;
        object->Type = Pick(types);
        for(int i = 0; i < 3; ++i) object->Position[i] = float(int(Input() % 1024) - 512);
        const int heroMode = trial % 3;
        DAT_07abf5d8 = heroMode == 0 ? nullptr : heroMode == 1 ? reinterpret_cast<char*>(character) : alias;
        advanceResult = Pick(results); routeResult = Pick(results);
        CHARACTER expected; OBJECT expectedObject;
        std::memcpy(&expected, character, sizeof(expected)); std::memcpy(&expectedObject, object, sizeof(expectedObject));
        std::vector<int> expectedEvents; int expectedHeightCalls = 0;
        if(heroMode != 1) {
            if(expected.Dead) {
                if(object->Type == 272) {
                    expectedObject.Position[2] = object->Position[0] * 0.125f + object->Position[1] * 0.25f - 17.0f;
                    expectedHeightCalls = 1;
                }
            } else if(expected.Movement) {
                expectedEvents = {1, 2}; expected.Run = 42; expected.PathCurrent = 17;
                if(advanceResult & 255) {
                    expected.Movement = 0; expected.Run = 0; expectedEvents.push_back(3);
                    expected.Object.Angle[2] = (float(reinterpret_cast<BYTE*>(&expected)[764]) - _DAT_0055256c) * _DAT_00552844;
                }
                expectedEvents.push_back(4); expected.Object.Position[2] += 1.0f;
            } else if(!reinterpret_cast<BYTE*>(&expected)[848] && object->Type != 277 &&
                      (expected.PositionX != expected.TargetX || expected.PositionY != expected.TargetY)) {
                expectedEvents.push_back(5); expected.PathCurrent = 7;
                if(routeResult & 255) expected.Movement = 1;
            }
        }
        moveEvents.clear(); heightCalls = 0; CallMonster(character, object);
        Require(moveEvents == expectedEvents && heightCalls == expectedHeightCalls);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(std::memcmp(object, &expectedObject, sizeof(expectedObject)) == 0);
        Hash(character, sizeof(*character)); Hash(object, sizeof(*object)); Hash(&heightCalls, sizeof(heightCalls));
        for(int event : moveEvents) Hash(&event, sizeof(event));
        const int count = int(moveEvents.size()); Hash(&count, sizeof(count));
    }
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* object = static_cast<OBJECT*>(Reserve(0x75000000u)); Require(character && object);
    const auto expected = RunMonster(character, object, reinterpret_cast<char*>(std::uintptr_t(sizeof(void*) > 4 ? 0x176000000ull : 0x77000000ull)));
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highObject = static_cast<OBJECT*>(Reserve(std::uint64_t(0x175000000ull))); Require(highCharacter && highObject);
        for(int mask = 1; mask < 4; ++mask) {
            auto* c = mask & 1 ? highCharacter : character;
            char* alias = reinterpret_cast<char*>(mask & 1 ? character : highCharacter);
            Require(RunMonster(c, mask & 2 ? highObject : object, alias) == expected);
        }
        std::puts("movimiento sobre 4 GiB: cuatro combinaciones y Hero distinto con iguales bits bajos");
    }
    std::printf("MoveMonsterClient: 32768 estados; bytes/orden %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = position_harness(revision).split("void CallPosition(CHARACTER* character)", 1)[0]
    prefix = prefix.replace("SetPlayerStop(", "UnusedStop(").replace("MoveCharacterPosition(", "UnusedPosition(")
    globals_source = read_source("src/globals.cpp", revision)
    for name in ("_DAT_0055256c", "_DAT_00552844"):
        prefix += re.search(rf"^float\s+{name}\s*=[^;]+;", globals_source, re.M).group() + "\n"
    monster = function(read_source("src/Net/SecondPassword.cpp", revision), "MoveMonsterClient")
    args = "character, object" if "CHARACTER* character" in monster else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character)), static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix + SERVICES + monster + MAIN.replace("CALL_MONSTER", f"MoveMonsterClient({args})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-monster-movement-") as temp:
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
