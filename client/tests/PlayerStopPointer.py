#!/usr/bin/env python3
"""Aísla SetPlayerStop con tabla de items, terreno y PlayBuffer reales.

CHARACTER y PART_t se extraen de producción. OBJECT es un doble escalar con
los offsets x86 usados. Audio, mapa y azar son servicios controlados; no se
certifica el layout nativo x64 ni una sesión completa dentro del juego.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from SoundPointer import harness as sound_harness
from SceneObjectPointer import build_and_run, function, read_source

OBJECT_RECORD = r'''
#ifndef _WIN32
using WORD = std::uint16_t;
#define __forceinline inline
#endif
struct OBJECT {
    BYTE prefix[2];
    TYPE_FIELD
    SUBTYPE_FIELD
    BYTE beforePosition[8];
    POSITION_FIELD
    BYTE beforeAction[261 - 28];
    CURRENT_FIELD
    PRIOR_FIELD
    BYTE padding;
    FRAME_FIELD
    PRIOR_FRAME_FIELD
    BYTE suffix[444 - 272];
};
PART_RECORD
CHARACTER_RECORD
static_assert(sizeof(void*) != 4 || (sizeof(CHARACTER) == 916 && sizeof(PART_t) == 24));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, Type) == 2 && offsetof(OBJECT, SubType) == 4));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, CurrentAction) == 261 && offsetof(OBJECT, PriorAction) == 262));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, AnimationFrame) == 264 && offsetof(OBJECT, PriorAnimationFrame) == 268));
static_assert(sizeof(void*) != 4 || (offsetof(CHARACTER, Class) == 444 && offsetof(CHARACTER, Weapon) == 624));
static_assert(sizeof(void*) != 4 || (offsetof(CHARACTER, Wing) == 672 && offsetof(CHARACTER, Helper) == 696));
static_assert(sizeof(void*) != 4 || (offsetof(CHARACTER, Run) == 768 && offsetof(CHARACTER, SafeZone) == 846));
CHARACTER* AsCharacter(void* p) { return static_cast<CHARACTER*>(p); }
CHARACTER* AsCharacter(std::uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }
OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
OBJECT* AsObject(std::uintptr_t p) { return reinterpret_cast<OBJECT*>(p); }
using uint = unsigned int;
'''

SERVICES = r'''
int SceneFlag;
BYTE DAT_0838bc70[0x10000];
struct Map {
    int world; bool swimming;
    int GetCurrentMap() const { return world; }
    bool IsSwimmable(int value) const { Require(value == world); return swimming; }
} gMapManager;
int randomValues[2], randomDraws, terrainCalls;
int NextRandom() { Require(randomDraws < 2); return randomValues[randomDraws++]; }
int Terrain_GetTileIndex(uint x, uint y) { ++terrainCalls; return TileIndexReal(x, y); }
std::uint32_t seed;
std::uint32_t Input() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
template<class T, std::size_t N> T Pick(const T (&values)[N]) { return values[Input() % N]; }
'''

MAIN = r'''
void Call(CHARACTER* character) { CALL_STOP; }
std::uint64_t Run(CHARACTER* character, BYTE* models, BYTE* items, unsigned displacement) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    DAT_055c9e44 = reinterpret_cast<std::uintptr_t>(items);
    DAT_07d78068 = DAT_055c9e44 + displacement * 64;
    Require(ItemAttribute_Base() == DAT_07d78068);
    auto* table = reinterpret_cast<BYTE*>(DAT_07d78068);
    std::memset(items, 0, (ITEM_MAX_EX + 0x400) * 64);
    std::memset(models, 0xa5, 391 * 188);
    std::memset(DAT_0838bc70, 0, sizeof(DAT_0838bc70));
    g_EnableSound = TRUE;
    for(int slot = 0; slot < 420; ++slot) { MaxBufferChannel[slot] = 4; Enable3DSound[slot] = true; }
    seed = 0x97c4430c; digest = 14695981039346656037ull;
    bool actions[256] = {};
    const short types[] = {390, 390, 0, 302};
    const short helpers[] = {-1, 818, 819, 820}, wings[] = {-1, 700};
    const short weapons[] = {-32768, -1, 399, 400, 431, 495, 496, 497, 498, 527, 528, 534,
                            535, 536, 542, 543, 544, 545, 546, 547, 559, 560, 591, 592, 8590, 8591, 32767};
    const short right[] = {-1, 528, 534, 535, 545};
    const int worlds[] = {0, 7, 10, 11, 16, 17}, subtypes[] = {0, 205, 206, 207, 208, 209};
    const DWORD frames[] = {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u};
    const int random[] = {0, 1, 16, 31};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Object.SubType = Pick(subtypes);
        character->Class = BYTE(Input() % 256); character->SafeZone = BYTE(Input() % 2);
        character->Helper.Type = Pick(helpers); character->Wing.Type = Pick(wings);
        character->Weapon[0].Type = Pick(weapons); character->Weapon[1].Type = Pick(right);
        character->Object.Position[0] = int(Input() % 2000) * 12.5f - 10000.0f;
        character->Object.Position[1] = int(Input() % 2000) * -9.75f + 1500.0f;
        character->Object.CurrentAction = BYTE(Input() % 256);
        const DWORD frame = Pick(frames); std::memcpy(&character->Object.AnimationFrame, &frame, 4);
        character->Run = BYTE(Input() % 255 + 1);
        gMapManager.world = Pick(worlds); gMapManager.swimming = Input() % 2 != 0;
        SceneFlag = Input() % 2 ? 5 : 4;
        const int tile = TileIndexReal(uint(int(character->Object.Position[0] * 0.0099999998f)),
                                      uint(int(character->Object.Position[1] * 0.0099999998f)));
        DAT_0838bc70[tile] = BYTE(Input() % 2);
        const int row = character->Weapon[0].Type - 399;
        if(row >= 1 && row < ITEM_MAX_EX) table[row * 64 - 34] = BYTE(Input() % 2);
        const short sounds[] = {short(Input() % 3 ? 1 : -1), 2};
        std::memcpy(models + character->Object.Type * 188 + 170, sounds, sizeof(sounds));
        randomValues[0] = Pick(random); randomValues[1] = int(Input() % 2);
        randomDraws = terrainCalls = 0;
        audio.calls = 0; audio.slot = audio.channel = -7; audio.loop = false;
        audio.result = Input() % 2 ? HRESULT(0) : HRESULT(-2147467259);
        std::memset(BufferChannel, 0, sizeof(BufferChannel));
        std::memset(Object3DSound, 0, sizeof(Object3DSound));
        CHARACTER before; std::memcpy(&before, character, sizeof(before));
        Call(character);
        Require(character->Run == 0 && randomDraws >= 1 && randomDraws <= 2);
        const auto action = character->Object.CurrentAction; actions[action] = true;
        if(action == before.Object.CurrentAction) {
            Require(std::memcmp(&character->Object.AnimationFrame, &before.Object.AnimationFrame, 4) == 0);
            Require(character->Object.PriorAction == before.Object.PriorAction);
            Require(std::memcmp(&character->Object.PriorAnimationFrame, &before.Object.PriorAnimationFrame, 4) == 0);
        } else {
            Require(character->Object.PriorAction == before.Object.CurrentAction);
            Require(std::memcmp(&character->Object.PriorAnimationFrame, &before.Object.AnimationFrame, 4) == 0);
            DWORD bits; std::memcpy(&bits, &character->Object.AnimationFrame, 4); Require(bits == 0);
        }
        for(int offset = 0; offset < int(sizeof(CHARACTER)); ++offset) {
            if(offset == int(offsetof(CHARACTER, Run)) || offset == 261 || offset == 262 || (offset >= 264 && offset < 272)) continue;
            Require(reinterpret_cast<BYTE*>(character)[offset] == reinterpret_cast<BYTE*>(&before)[offset]);
        }
        bool bound = false;
        for(int slot = 170; slot < 175; ++slot) for(int ch = 0; ch < 4; ++ch) {
            const auto address = Address(Object3DSound[slot][ch]);
            if(address) { Require(address == reinterpret_cast<std::uintptr_t>(character)); bound = true; }
        }
        Require(audio.calls <= 1);
        if(audio.calls) Require(audio.slot >= 171 && audio.slot <= 172 && audio.channel == 0 && !audio.loop);
        Hash(character, sizeof(*character)); Hash(&randomDraws, sizeof(randomDraws));
        Hash(&terrainCalls, sizeof(terrainCalls)); Hash(&audio.calls, sizeof(audio.calls));
        Hash(&audio.slot, sizeof(audio.slot)); Hash(&audio.channel, sizeof(audio.channel)); Hash(&bound, sizeof(bound));
    }
    for(int action : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 78}) Require(actions[action]);
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x7a000000u));
    auto* items = static_cast<BYTE*>(Reserve(0x7b000000u));
    auto* models = static_cast<BYTE*>(Reserve(0x7c000000u)); Require(character && items && models);
    const auto expected = Run(character, models, items, 0);
    Require(Run(character, models, items, 1023) == expected);
    if(sizeof(void*) > 4) {
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x17a000000ull)));
        auto* highItems = static_cast<BYTE*>(Reserve(std::uint64_t(0x17b000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x17c000000ull))); Require(highCharacter && highItems && highModels);
        for(int mask = 1; mask < 8; ++mask)
            Require(Run(mask & 1 ? highCharacter : character, mask & 2 ? highModels : models,
                        mask & 4 ? highItems : items, 1023) == expected);
        std::puts("reposo sobre 4 GiB: ocho combinaciones de personaje/modelos/items equivalentes");
    }
    std::printf("SetPlayerStop: 32768 estados, 14 acciones y sonidos; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = sound_harness(revision).split("HRESULT CallPlay(", 1)[0].replace("0x10000", "0x100000")
    types = read_source("src/Entity/EntityTypes.h", revision)
    record = OBJECT_RECORD
    for token, name, kind in (("TYPE_FIELD", "Type", "short"), ("SUBTYPE_FIELD", "SubType", "int"),
                              ("POSITION_FIELD", "Position", "vec3_t"), ("CURRENT_FIELD", "CurrentAction", "BYTE"),
                              ("PRIOR_FIELD", "PriorAction", "BYTE"), ("FRAME_FIELD", "AnimationFrame", "float"),
                              ("PRIOR_FRAME_FIELD", "PriorAnimationFrame", "float")):
        record = re.sub(rf"\b{token}\b", re.search(rf"\b{kind}\s+{name}\s*;", types).group(), record)
    for token, name in (("PART_RECORD", "PART_t"), ("CHARACTER_RECORD", "CHARACTER")):
        record = record.replace(token, re.search(rf"struct {name}\s*\{{.*?\}};", types, re.S).group())
    prefix = re.sub(r"struct OBJECT\s*\{.*?\};", lambda _: record, prefix, count=1, flags=re.S)
    header, globals_source = (read_source(path, revision) for path in ("src/globals.h", "src/globals.cpp"))
    declarations = "\n".join(re.findall(r"^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;", read_source("src/Item/ItemDefines.h", revision), re.M))
    for name in ("DAT_05828d58", "DAT_055c9e44", "DAT_07d78068"):
        declarations += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        declarations += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    declarations += re.search(r"static __forceinline [^\n]*\bItemAttribute_Base\([^\{]*\{.*?^\}", header, re.M | re.S).group()
    terrain = function(read_source("src/Terrain/Terrain_Utils.cpp", revision), "Terrain_GetTileIndex").replace("Terrain_GetTileIndex(", "TileIndexReal(", 1)
    stop = function(read_source("src/Net/SecondPassword.cpp", revision), "SetPlayerStop").replace("rand()", "NextRandom()")
    arg = "character" if "CHARACTER* c" in stop else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + declarations + terrain + SERVICES + stop + MAIN.replace("CALL_STOP", f"SetPlayerStop({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-stop-pointer-") as temp:
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
