#!/usr/bin/env python3
"""Prueba reacción al golpe real con animación, sonido y partículas registradas.

Reutiliza los fixtures escalares x86 de muerte. No ejecuta pools de partículas,
render ni dispositivo de audio; no certifica CHARACTER nativo x64.
"""
import argparse
import os
from pathlib import Path
import tempfile

from PlayerDiePointer import harness as die_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <array>
std::vector<std::array<float, 3>> sparks;
int Particle_Spawn(int type, float* position, float* angle, float* light, int flag, float alpha, int mode) {
    Require(type == 1221 && angle == activeCharacter->Object.Angle && light == activeCharacter->Object.Light);
    Require(flag == 0 && alpha == 1.0f && mode == 0);
    sparks.push_back({position[0], position[1], position[2]}); events.push_back(30000 + type); return 0;
}
'''

MAIN = r'''
void CallShock(CHARACTER* character, int hit) { CALL_SHOCK; }
std::uint64_t RunShock(CHARACTER* character, BYTE* models) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    g_EnableSound = TRUE; activeCharacter = character;
    for(int slot = 0; slot < 420; ++slot) { MaxBufferChannel[slot] = 4; Enable3DSound[slot] = true; }
    seed = 0x97c444b6; digest = 14695981039346656037ull;
    events.clear(); CallShock(nullptr, 100); Require(events.empty());
    bool particleCounts[6] = {};
    const short types[] = {0, 283, 284, 285, 331, 390, 390}, helpers[] = {-1, 0, 818, 819};
    const int subtypes[] = {0, 205, 206, 207, 208, 209};
    const BYTE classes[] = {0, 1, 2, 3, 7, 10, 255}, dead[] = {0, 0, 1, 15};
    const BYTE current[] = {0, 2, 3, 4, 5, 62, 63, 130, 255};
    const short limits[] = {-1, 0, 5, 6, 130, 131, 256};
    const DWORD frames[] = {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u};
    const int hits[] = {-2147483647 - 1, -1, 0, 1, 100, 2147483647};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Object.SubType = Pick(subtypes);
        character->Class = Pick(classes); character->Dead = Pick(dead); character->Helper.Type = Pick(helpers);
        character->Object.CurrentAction = Pick(current);
        const DWORD frame = Pick(frames); std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        for(int i = 0; i < 3; ++i) {
            character->Object.Position[i] = int(Input() % 1000) * 0.25f - 100.0f;
            character->Object.Angle[i] = i * -45.0f; character->Object.Light[i] = i * 0.25f;
        }
        const short limit = Pick(limits);
        const short sounds[] = {short(Input() % 3 ? 1 : -1), 2};
        std::memset(models, 0xa5, 391 * 188);
        std::memcpy(models + character->Object.Type * 188 + 38, &limit, 2);
        std::memcpy(models + character->Object.Type * 188 + 174, sounds, sizeof(sounds));
        const int hit = Pick(hits);
        for(int i = 0; i < 32; ++i) randomValues[i] = trial % 3 == 0 ? 0 : trial % 3 == 1 ? 1 : int(Input() & 0x7fff);
        int expectedDraws = 0;
        auto next = [&]() { Require(expectedDraws < 32); return randomValues[expectedDraws++]; };
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        std::vector<int> expectedEvents;
        std::vector<std::array<float, 3>> expectedSparks;
        const bool blocked = expected.Dead || expected.Helper.Type == 818 || expected.Helper.Type == 819 ||
            expected.Object.CurrentAction == 62 || expected.Object.CurrentAction == 63 || hit <= 0;
        int expectedAction = -1;
        if(!blocked) {
            if(expected.Object.Type == 390) { expectedAction = 130; expected.Movement = 0; }
            else if(expected.Object.CurrentAction < 3 || expected.Object.CurrentAction > 4) expectedAction = 5;
            if(expectedAction >= 0) {
                expectedEvents.push_back(20000 + expectedAction);
                if(expectedAction < limit && expected.Object.CurrentAction != expectedAction) {
                    expected.Object.PriorAction = expected.Object.CurrentAction;
                    std::memcpy(&expected.Object.PriorAnimationFrame, &frame, 4);
                    expected.Object.CurrentAction = BYTE(expectedAction); expected.Object.AnimationFrame = 0.0f;
                }
            }
            if(expected.Object.AnimationFrame == 0.0f) {
                const bool special = expected.Object.SubType >= 206 && expected.Object.SubType <= 208;
                if(expected.Object.Type == 390 && !special) {
                    const int voice = (expected.Class & 7) == 2 ? next() % 2 + 79 : next() % 3 + 75;
                    expectedEvents.push_back(10000 + voice);
                } else if(special) expectedEvents.push_back(10093);
                else if(expected.Object.Type != 284 && sounds[0] != -1) expectedEvents.push_back(10000 + sounds[next() % 2] + 170);
            }
            if(expected.Object.Type == 331) {
                for(int i = 0; i < 5; ++i) if(next() % 2 == 0) {
                    const float x = float(next() % 128 - 64) + expected.Object.Position[0];
                    const float z = float(next() % 50) + expected.Object.Position[2] + 200.0f;
                    expectedSparks.push_back({x, expected.Object.Position[1], z}); expectedEvents.push_back(31221);
                }
                expectedEvents.push_back(10105);
                particleCounts[expectedSparks.size()] = true;
            }
        }
        events.clear(); soundEvents.clear(); sparks.clear(); randomDraws = actionCalls = 0; requestedAction = -1;
        audio.calls = 0; audio.result = Input() % 2 ? HRESULT(0) : HRESULT(-2147467259);
        std::memset(BufferChannel, 0, sizeof(BufferChannel)); std::memset(Object3DSound, 0, sizeof(Object3DSound));
        CallShock(character, hit);
        Require(events == expectedEvents && sparks == expectedSparks && randomDraws == expectedDraws);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Require(actionCalls == int(expectedAction >= 0));
        if(actionCalls) Require(requestedAction == expectedAction);
        Require(audio.calls <= 2 && soundEvents.size() == std::size_t(audio.calls));
        unsigned bound = 0; int index = 0;
        for(int slot : {75, 76, 77, 79, 80, 93, 105, 171, 172}) {
            for(int ch = 0; ch < 4; ++ch) {
                const auto address = Address(Object3DSound[slot][ch]);
                if(address) { Require(slot != 105 && address == reinterpret_cast<std::uintptr_t>(character)); bound |= 1u << index; }
            }
            ++index;
        }
        Hash(character, sizeof(*character)); Hash(&randomDraws, sizeof(randomDraws));
        for(int event : events) Hash(&event, sizeof(event));
        for(const auto& spark : sparks) Hash(spark.data(), 12); Hash(&bound, sizeof(bound));
    }
    for(bool covered : particleCounts) Require(covered);
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x76000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x74000000u)); Require(character && models);
    const auto expected = RunShock(character, models);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highCharacter && highModels);
        std::memset(models, 0xcc, 391 * 188);
        Require(RunShock(character, highModels) == expected);
        Require(RunShock(highCharacter, models) == expected);
        Require(RunShock(highCharacter, highModels) == expected);
        std::puts("shock sobre 4 GiB: personaje/modelos, sonidos y partículas equivalentes");
    }
    std::printf("SetPlayerShock: 32768 estados, bloqueos y cero a cinco partículas; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = die_harness(revision).split("void CallDie(CHARACTER* character)", 1)[0]
    prefix = prefix.replace("randomValues[2]", "randomValues[32]").replace("Require(randomDraws < 2)", "Require(randomDraws < 32)")
    prefix = prefix.replace("owner == AsObject(activeCharacter) && loop == 0", "(owner == AsObject(activeCharacter) || (slot == 105 && !owner)) && loop == 0")
    shock = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "SetPlayerShock").replace("rand()", "NextRandom()")
    arg = "character" if "CHARACTER* c" in shock else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + SERVICES + shock + MAIN.replace("CALL_SHOCK", f"SetPlayerShock({arg}, hit)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-shock-pointer-") as temp:
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
