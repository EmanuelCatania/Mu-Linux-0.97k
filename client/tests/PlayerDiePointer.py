#!/usr/bin/env python3
"""Prueba muerte y wrapper reales con SetAction y PlayBuffer reales.

Usa los dobles escalares x86 del banco de marcha. CreateEffect registra
argumentos y orden; no ejecuta los pools de efectos ni un dispositivo de audio.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from PlayerWalkPointer import harness as walk_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
static_assert(offsetof(OBJECT, Live) == 0 && offsetof(OBJECT, Angle) == 28);
static_assert(offsetof(OBJECT, Light) == 232);
float* CreateEffect(int type, float* position, float* angle, float* light,
                    float* p4, float* p5, float* p6, float* p7, BYTE flag) {
    Require(position == activeCharacter->Object.Position && angle == activeCharacter->Object.Angle);
    Require(light == activeCharacter->Object.Light && !p4 && !p5 && !p7 && flag == 0);
    Require(reinterpret_cast<std::uintptr_t>(p6) == ~std::uintptr_t(0));
    events.push_back(type); return nullptr;
}
'''

MAIN = r'''
void CallDie(CHARACTER* character) { CALL_DIE; }
void CallWrapper(CHARACTER* character) { CALL_WRAPPER; }
std::uint64_t RunDie(CHARACTER* character, BYTE* models, bool wrapper) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    g_EnableSound = TRUE; activeCharacter = character;
    for(int slot = 0; slot < 420; ++slot) { MaxBufferChannel[slot] = 4; Enable3DSound[slot] = true; }
    digest = 14695981039346656037ull;
    events.clear(); CallDie(nullptr); CallWrapper(nullptr); Require(events.empty());
    for(short type : {0, 294, 295, 296, 299, 300, 301, 390})
    for(int subtype : {0, 205, 206, 207, 208, 209})
    for(BYTE classId : {0, 1, 2, 3, 7, 10, 255})
    for(short limit : {0, 6, 7, 131, 132, 256})
    for(BYTE current : {0, 6, 131, 255})
    for(DWORD frame : {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u})
    for(short sound : {-1, 1})
    for(HRESULT result : {HRESULT(0), HRESULT(-2147467259)}) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Live = current != 0;
        character->Object.Type = type; character->Object.SubType = subtype; character->Class = classId;
        character->Object.CurrentAction = current;
        std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        for(int i = 0; i < 3; ++i) {
            character->Object.Position[i] = type * 12.5f + i;
            character->Object.Angle[i] = i * -45.0f; character->Object.Light[i] = i * 0.25f;
        }
        std::memset(models, 0xa5, 391 * 188);
        std::memcpy(models + type * 188 + 38, &limit, 2);
        std::memcpy(models + type * 188 + 178, &sound, 2);
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        const bool special = type == 390 && subtype >= 206 && subtype <= 208;
        const bool explode = special || type == 295 || type == 300;
        const int action = type == 390 ? 131 : 6;
        std::vector<int> expectedEvents;
        if(explode) {
            expected.Object.Live = 0;
            if(type == 295) {
                for(int i = 0; i < 8; ++i) { expectedEvents.push_back(226); expectedEvents.push_back(227); }
            } else {
                expectedEvents.push_back(210);
                for(int i = 0; i < 10; ++i) expectedEvents.push_back(211);
            }
            expectedEvents.push_back(10094);
        } else {
            expectedEvents.push_back(20000 + action);
            if(action < limit && current != action) {
                expected.Object.PriorAction = current;
                std::memcpy(&expected.Object.PriorAnimationFrame, &frame, 4);
                expected.Object.CurrentAction = BYTE(action); expected.Object.AnimationFrame = 0.0f;
            }
        }
        if(expected.Object.AnimationFrame == 0.0f) {
            if(type == 390 && !special) expectedEvents.push_back((classId & 7) == 2 ? 10080 : 10078);
            else if(sound != -1) expectedEvents.push_back(10000 + sound + 170);
        }
        events.clear(); soundEvents.clear(); actionCalls = 0; requestedAction = -1;
        audio.calls = 0; audio.result = result;
        std::memset(BufferChannel, 0, sizeof(BufferChannel)); std::memset(Object3DSound, 0, sizeof(Object3DSound));
        if(wrapper) CallWrapper(character); else CallDie(character);
        Require(events == expectedEvents && std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(actionCalls == (explode ? 0 : 1));
        Require(soundEvents.size() == std::size_t(audio.calls));
        unsigned bound = 0; int index = 0;
        for(int slot : {78, 80, 94, 171}) {
            for(int ch = 0; ch < 4; ++ch) {
                const auto address = Address(Object3DSound[slot][ch]);
                if(address) { Require(address == reinterpret_cast<std::uintptr_t>(character)); bound |= 1u << index; }
            }
            ++index;
        }
        Hash(character, sizeof(*character));
        for(int event : events) Hash(&event, sizeof(event)); Hash(&bound, sizeof(bound));
    }
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x74000000u)); Require(character && models);
    const auto expected = RunDie(character, models, false);
    Require(RunDie(character, models, true) == expected);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highCharacter && highModels);
        Require(RunDie(character, highModels, false) == expected);
        Require(RunDie(highCharacter, models, false) == expected);
        Require(RunDie(highCharacter, highModels, false) == expected);
        Require(RunDie(highCharacter, highModels, true) == expected);
        std::puts("muerte sobre 4 GiB: personaje/modelos y wrapper conservan direcciones completas");
    }
    std::printf("SetPlayerDie: 129024 casos, animación/explosiones/sonidos; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = walk_harness(revision).split("void CallWalk(CHARACTER* character)", 1)[0]
    types = read_source("src/Entity/EntityTypes.h", revision)
    live = re.search(r"BYTE\s+Live\s*;", types).group()
    angle = re.search(r"vec3_t\s+Angle\s*;", types).group()
    light = re.search(r"vec3_t\s+Light\s*;", types).group()
    prefix = prefix.replace("BYTE prefix[2];", live + " BYTE prefix;", 1)
    prefix = prefix.replace("BYTE beforeAction[261 - 28];", angle + " BYTE beforeLight[232 - 40]; " + light + " BYTE beforeAction[261 - 244];", 1)
    prefix = prefix.replace("CHARACTER* activeCharacter;", "std::vector<int> events;\nCHARACTER* activeCharacter;", 1)
    prefix = prefix.replace("requestedAction = action;", "requestedAction = action; events.push_back(20000 + action);", 1)
    prefix = prefix.replace("soundEvents.push_back(slot);", "soundEvents.push_back(slot); events.push_back(10000 + slot);", 1)
    source = read_source("src/Net/SecondPassword.cpp", revision)
    die = function(source, "SetPlayerDie")
    wrapper = function(source, "mc_SetPlayerDie")
    main = MAIN
    for token, name, body, signature in (("CALL_DIE", "SetPlayerDie", die, "CHARACTER* c_in"),
                                        ("CALL_WRAPPER", "mc_SetPlayerDie", wrapper, "CHARACTER* c")):
        arg = "character" if signature in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
        main = main.replace(token, f"{name}({arg})")
    return prefix + SERVICES + die + wrapper + main


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-die-pointer-") as temp:
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
