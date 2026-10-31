#!/usr/bin/env python3
"""Prueba SetPlayerWalk con personaje, items, modelos, SetAction y audio reales.

Reutiliza los dobles escalares y servicios del banco de reposo. No representa
un layout CHARACTER nativo x64 ni prueba terreno, mapa o audio de una sesión.
"""
import argparse
import os
from pathlib import Path
import tempfile

from PlayerStopPointer import harness as stop_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
CHARACTER* activeCharacter;
int requestedAction, actionCalls;
std::vector<int> soundEvents;
void* __cdecl SetAction(OBJECT* object, int action) {
    Require(object == AsObject(activeCharacter)); requestedAction = action; ++actionCalls;
    return SetActionReal(object, action);
}
HRESULT __cdecl PlayBuffer(int slot, const OBJECT* owner, BOOL loop) {
    Require(owner == AsObject(activeCharacter) && loop == 0); soundEvents.push_back(slot);
    return PlayBufferReal(slot, owner, loop);
}
'''

MAIN = r'''
void CallWalk(CHARACTER* character) { CALL_WALK; }
std::uint64_t RunWalk(CHARACTER* character, BYTE* models, BYTE* items, unsigned displacement) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    DAT_055c9e44 = reinterpret_cast<std::uintptr_t>(items);
    DAT_07d78068 = DAT_055c9e44 + displacement * 64;
    Require(ItemAttribute_Base() == DAT_07d78068);
    auto* table = reinterpret_cast<BYTE*>(DAT_07d78068);
    std::memset(items, 0, (ITEM_MAX_EX + 0x400) * 64);
    std::memset(models, 0xa5, 391 * 188);
    g_EnableSound = TRUE; activeCharacter = character;
    for(int slot = 0; slot < 420; ++slot) { MaxBufferChannel[slot] = 4; Enable3DSound[slot] = true; }
    seed = 0x97c44393; digest = 14695981039346656037ull;
    bool actions[256] = {};
    const short types[] = {390, 390, 0, 297};
    const short helpers[] = {-1, 818, 819, 820}, wings[] = {-1, 700};
    const short weapons[] = {-32768, -1, 399, 400, 431, 495, 496, 497, 498, 527, 528, 530, 534,
                            535, 536, 542, 543, 544, 545, 546, 547, 559, 560, 591, 592, 32767};
    const short right[] = {-1, 400, 431, 528, 534, 535, 545};
    const int worlds[] = {0, 7, 8, 10, 11, 16, 17}, subtypes[] = {0, 205, 206, 207, 208, 209};
    const short limits[] = {0, 2, 13, 30, 76, 77, 88, 100, 256};
    const DWORD frames[] = {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u};
    const int random[] = {0, 1, 16, 32, 63, 64};
    const BYTE stamina[] = {0, 1, 38, 39, 40, 41, 255}, levels[] = {0, 4, 5, 9, 255};
    const short parts[] = {-1, 0, 600};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Object.SubType = Pick(subtypes);
        character->Class = BYTE(Input() % 256); character->SafeZone = BYTE(Input() % 2);
        character->Helper.Type = Pick(helpers); character->Wing.Type = Pick(wings);
        character->Weapon[0].Type = Pick(weapons); character->Weapon[1].Type = Pick(right);
        for(int i : {4, 5}) { character->BodyPart[i].Type = Pick(parts); character->BodyPart[i].Level = Pick(levels); }
        character->Object.CurrentAction = BYTE(Input() % 256);
        const DWORD frame = Pick(frames); std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        character->Run = Pick(stamina);
        gMapManager.world = Pick(worlds); gMapManager.swimming = Input() % 2 != 0;
        const int row = character->Weapon[0].Type - 399;
        if(row >= 1 && row < ITEM_MAX_EX) table[row * 64 - 34] = BYTE(Input() % 2);
        const short sounds[] = {short(Input() % 3 ? 1 : -1), 2};
        std::memcpy(models + character->Object.Type * 188 + 170, sounds, sizeof(sounds));
        const short limit = Pick(limits); std::memcpy(models + character->Object.Type * 188 + 38, &limit, 2);
        const int role = int(Input() % 3);
        Hero = reinterpret_cast<char*>(character) + (role ? sizeof(CHARACTER) : 0);
        if(role == 2 && sizeof(void*) > 4) {
            // Otra entidad puede tener los mismos 32 bits bajos que el personaje.
            Hero = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(character) ^ std::uint64_t(0x100000000ull));
        }
        randomValues[0] = Pick(random); randomValues[1] = int(Input() % 2); randomDraws = 0;
        audio.calls = 0; audio.slot = audio.channel = -7; audio.loop = false;
        audio.result = Input() % 2 ? HRESULT(0) : HRESULT(-2147467259);
        std::memset(BufferChannel, 0, sizeof(BufferChannel)); std::memset(Object3DSound, 0, sizeof(Object3DSound));
        requestedAction = -1; actionCalls = 0; soundEvents.clear();
        CHARACTER before; std::memcpy(&before, character, sizeof(before));
        CallWalk(character);
        Require(actionCalls == 1 && requestedAction >= 0 && requestedAction < 256);
        actions[requestedAction] = true;
        Require(randomDraws <= 2 && (before.Object.Type == 297 ? randomDraws == 0 : randomDraws >= 1));
        // IDA 0x00443930: Hero usa 1/64; cualquier otra entidad usa 1/16.
        std::vector<int> expectedSounds;
        int expectedDraws = 0;
        if(before.Object.Type == 297) expectedSounds.push_back(94);
        else {
            expectedDraws = 1;
            const bool special = before.Object.SubType >= 206 && before.Object.SubType <= 208;
            if((randomValues[0] & (role == 0 ? 63 : 15)) == 0 && (before.Object.Type != 390 || special)) {
                if(special) expectedSounds.push_back(93);
                if(sounds[0] != -1) { expectedDraws = 2; expectedSounds.push_back(171 + randomValues[1]); }
            }
        }
        Require(randomDraws == expectedDraws && soundEvents == expectedSounds);
        if(before.SafeZone) Require(character->Run == 0);
        else Require(character->Run == before.Run || (before.Run < 40 && character->Run == before.Run + 1));
        const bool accepted = requestedAction < limit || requestedAction == 76 || requestedAction == 77;
        if(!accepted || requestedAction == before.Object.CurrentAction) {
            Require(character->Object.CurrentAction == before.Object.CurrentAction);
            Require(std::memcmp(&character->Object.AnimationFrame, &before.Object.AnimationFrame, 4) == 0);
            Require(character->Object.PriorAction == before.Object.PriorAction);
            Require(std::memcmp(&character->Object.PriorAnimationFrame, &before.Object.PriorAnimationFrame, 4) == 0);
        } else {
            Require(character->Object.CurrentAction == BYTE(requestedAction));
            Require(character->Object.PriorAction == before.Object.CurrentAction);
            Require(std::memcmp(&character->Object.PriorAnimationFrame, &before.Object.AnimationFrame, 4) == 0);
            DWORD bits; std::memcpy(&bits, &character->Object.AnimationFrame, 4); Require(bits == 0);
        }
        for(int offset = 0; offset < int(sizeof(CHARACTER)); ++offset) {
            if(offset == int(offsetof(CHARACTER, Run)) || offset == 261 || offset == 262 || (offset >= 264 && offset < 272)) continue;
            Require(reinterpret_cast<BYTE*>(character)[offset] == reinterpret_cast<BYTE*>(&before)[offset]);
        }
        unsigned bound = 0; int index = 0;
        for(int slot : {93, 94, 171, 172}) {
            for(int ch = 0; ch < 4; ++ch) {
                const auto address = Address(Object3DSound[slot][ch]);
                if(address) { Require(address == reinterpret_cast<std::uintptr_t>(character)); bound |= 1u << index; }
            }
            ++index;
        }
        Require(audio.calls <= 2 && soundEvents.size() == std::size_t(audio.calls));
        for(int slot : soundEvents) Require(slot == 93 || slot == 94 || slot == 171 || slot == 172);
        Hash(character, sizeof(*character)); Hash(&requestedAction, sizeof(requestedAction));
        Hash(&randomDraws, sizeof(randomDraws)); Hash(&audio.calls, sizeof(audio.calls));
        for(int slot : soundEvents) Hash(&slot, sizeof(slot)); Hash(&bound, sizeof(bound));
    }
    for(int action : {2, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                      28, 29, 30, 31, 32, 33, 76, 77, 79, 80}) Require(actions[action]);
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x79000000u));
    auto* items = static_cast<BYTE*>(Reserve(0x7b000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x7c000000u)); Require(character && items && models);
    const auto expected = RunWalk(character, models, items, 0);
    Require(RunWalk(character, models, items, 1023) == expected);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x179000000ull)));
        auto* highItems = static_cast<BYTE*>(Reserve(std::uint64_t(0x17b000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x17c000000ull))); Require(highCharacter && highItems && highModels);
        for(int mask = 1; mask < 8; ++mask)
            Require(RunWalk(mask & 1 ? highCharacter : character, mask & 2 ? highModels : models,
                            mask & 4 ? highItems : items, 1023) == expected);
        std::puts("marcha sobre 4 GiB: ocho combinaciones e identidad completa de Hero correctas");
    }
    std::printf("SetPlayerWalk: 32768 estados, 26 acciones; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = stop_harness(revision).split("void Call(CHARACTER* character)", 1)[0]
    prefix = prefix.replace("HRESULT __cdecl PlayBuffer(", "HRESULT __cdecl PlayBufferReal(", 1)
    prefix = prefix.replace("struct PART_t {", "HRESULT __cdecl PlayBuffer(int, const OBJECT*, BOOL);\nstruct PART_t {", 1)
    action = function(read_source("src/Entity/Entity_ActionLegacy.cpp", revision), "SetAction").replace("SetAction(", "SetActionReal(", 1)
    walk = function(read_source("src/Net/SecondPassword.cpp", revision), "SetPlayerWalk").replace("rand()", "NextRandom()")
    arg = "character" if "CHARACTER* param_1" in walk else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + action + SERVICES + walk + MAIN.replace("CALL_WALK", f"SetPlayerWalk({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-walk-pointer-") as temp:
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
