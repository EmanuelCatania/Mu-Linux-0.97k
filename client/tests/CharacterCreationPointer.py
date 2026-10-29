#!/usr/bin/env python3
"""Verifica búsqueda/retorno y fábrica de monstruos con direcciones altas.

Constructor, limpieza, recursos, catálogo, nombres y joints controlados.
Pool alto: sólo reutilización/centinela; monstruos altos: reutilización sin
joints y acciones de modelo nulas. Los layouts con punteros quedan para B2.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <array>
#include <string>
#ifndef _WIN32
using WORD = std::uint16_t;
#endif
using std::uintptr_t;
using vec3_t = float[3];
RECORDS
static_assert(sizeof(OBJECT) == 444 && sizeof(CHARACTER) == 916 && sizeof(PART_t) == 24);
static_assert(offsetof(OBJECT, Owner) == 252 && offsetof(OBJECT, BoneMatrices) == 276);
CHARACTER* AsCharacter(void* p) { return static_cast<CHARACTER*>(p); }
CHARACTER* AsCharacter(std::uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }
OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
OBJECT* AsObject(std::uintptr_t p) { return reinterpret_cast<OBJECT*>(p); }
CONSTANTS
GLOBALS
#define DAT_07abf5d0 CharactersClient
DWORD HeroIndex;
struct Map { int world; int GetCurrentMap() const { return world; } } gMapManager;
BYTE* expectedSlot;
int expectedType, expectedX, expectedY, ctorCalls, cleanupCalls, jointCalls;
float expectedRotation;
bool safeMode;
std::vector<int> events;
std::string savedName;
void SeedCharacter(BYTE* slot, int type, int x, int y, float rotation) {
    auto* c = AsCharacter(slot); c->Object.Live = 1; c->Object.Type = short(type);
    c->Object.Scale = 1.0f; c->Object.Position[0] = x * 100.0f; c->Object.Position[1] = y * 100.0f; c->Object.Position[2] = 0.0f;
    c->Object.Angle[0] = c->Object.Angle[1] = 0; c->Object.Angle[2] = rotation;
    c->Object.Owner = c->Object.BoneMatrices = 0; c->Object.LightEnable = c->Object.EnableShadow = 1;
    c->Object.CurrentAction = 255; c->Object.PriorAction = 13; c->Object.AnimationFrame = 1.25f;
    c->Class = BYTE(type % 4); c->Skin = BYTE(x & 1); c->Change = 0;
}
void CreateCharacterPointer(unsigned char* slot, int type, int x, int y, float rotation) {
    Require(slot == expectedSlot && type == expectedType && x == expectedX && y == expectedY && rotation == expectedRotation);
    ++ctorCalls; events.push_back(30000 + type); SeedCharacter(slot, type, x, y, rotation);
}
void DeleteCloth(int c, int o, int flag) {
    Require(!safeMode && std::uint32_t(c) == std::uint32_t(reinterpret_cast<std::uintptr_t>(expectedSlot)) && c == o && flag == 0);
    ++cleanupCalls; events.push_back(40000);
}
void OpenMonsterModel(int id) { events.push_back(10000 + id); }
void OpenNpc(int id) { events.push_back(20000 + id); }
float RequestTerrainHeight(float x, float y) { return x * 0.125f + y * 0.25f + 17.0f; }
void Joint_Create(int type, float* start, float* end, float* angle, int subtype, int owner, float scale, int key, int flag) {
    Require(!safeMode && type == 1258 && start == AsObject(expectedSlot)->Position && end == start && angle == AsObject(expectedSlot)->Angle);
    Require((subtype == 2 || subtype == 3) && std::uint32_t(owner) == std::uint32_t(reinterpret_cast<std::uintptr_t>(expectedSlot)) && scale == 30.0f && key == -1 && flag == 0);
    ++jointCalls; events.push_back(50000 + subtype);
}
CATALOG_RECORD
struct Catalog {
    int mode = 0; mutable CatalogMonster monster{};
    const CatalogMonster* GetMonster(int type) const {
        if(!mode) return nullptr;
        monster.Present = true; monster.Model = mode == 1 ? 500 + (type & 31) : -1;
        monster.Scale = mode == 1 ? 1.25f : 0.75f; monster.Kind = BYTE(type & 1);
        std::strcpy(monster.Name, "custom"); return &monster;
    }
} gContentCatalog;
const char* getMonsterName(int type) { return type % 3 ? "vanilla" : ""; }
namespace EntityName {
void Set(const void* owner, const char* name) { Require(owner == expectedSlot); savedName = name ? name : ""; }
}
'''

MAIN = r'''
std::uintptr_t CallCreate(int key, int type, BYTE x, BYTE y, float rotation) { CALL_CREATE; }
std::uint64_t HashBytes(std::uint64_t digest, const void* p, int size) {
    const auto* bytes = static_cast<const BYTE*>(p); for(int i = 0; i < size; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; } return digest;
}
void PreparePool(BYTE* pool) {
    std::memset(pool, 0xa5, 401 * 916);
    for(int i = 0; i < 400; ++i) { auto* c = AsCharacter(pool + 916*i); c->Object.Live = 1; c->Key = WORD(i); }
}
std::uint64_t RunPool(BYTE* pool, bool safe) {
    CharactersClient = reinterpret_cast<std::uintptr_t>(pool); safeMode = safe; std::uint64_t digest = 14695981039346656037ull;
    const int count = safe ? 404 : 804;
    for(int trial = 0; trial < count; ++trial) {
        PreparePool(pool); int index, key; bool allocate = false, sentinel = false;
        if(trial < 400) { index = trial; key = index; }
        else if(trial < 404) {
            if(trial == 400) { index = 0; key = 7777; AsCharacter(pool)->Key = 7777; AsCharacter(pool + 399*916)->Key = 7777; }
            else if(trial == 401) { index = 400; key = 9999; sentinel = true; }
            else { index = 3; key = trial == 402 ? 32768 : -32768; AsCharacter(pool + 3*916)->Key = 32768; }
        } else { index = trial - 404; key = 5000; allocate = true; AsCharacter(pool + 916*index)->Object.Live = 0; }
        expectedSlot = pool + index * 916; expectedType = 270 + trial % 256; expectedX = BYTE(trial); expectedY = BYTE(255 - trial); expectedRotation = float(trial % 8 * 45);
        std::vector<BYTE> expected(pool, pool + 401*916);
        if(!sentinel) { SeedCharacter(expected.data() + index*916, expectedType, expectedX, expectedY, expectedRotation); if(allocate) AsCharacter(expected.data() + index*916)->Key = WORD(key); }
        events.clear(); ctorCalls = cleanupCalls = 0;
        const auto result = CallCreate(key, expectedType, BYTE(expectedX), BYTE(expectedY), expectedRotation);
        Require(result == reinterpret_cast<std::uintptr_t>(expectedSlot) && ctorCalls == (sentinel ? 0 : 1) && cleanupCalls == int(allocate));
        Require(std::memcmp(pool, expected.data(), expected.size()) == 0);
        digest = HashBytes(digest, expectedSlot, 916); digest = HashBytes(digest, &index, 4);
    }
    return digest;
}
std::uint64_t RunMonsters(BYTE* pool, BYTE* models, bool safe) {
    CharactersClient = reinterpret_cast<std::uintptr_t>(pool); DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models); safeMode = safe;
    std::vector<BYTE> canonical(1024*188, 0xa5);
    for(int i = 0; i < 1024; ++i) { const short limit = 256; std::memcpy(canonical.data() + i*188 + 38, &limit, 2); std::memset(canonical.data() + i*188 + 40, 0, 8); }
    std::uint64_t digest = 14695981039346656037ull; int cases = 0;
    const int worlds[] = {0, 8, 9, 10, 11, 12, 15, 16, 17};
    for(unsigned index = 0; index < 259; ++index)
    for(int world : worlds) for(int mode = 0; mode < 3; ++mode) {
        const unsigned type = index < 256 ? index : index == 256 ? 256u : index == 257 ? 511u : 0xffffffffu;
        const auto& def = type < MonsterDefinitions.size() ? MonsterDefinitions[type] : DefaultMonsterDefinition;
        if(safe && mode != 1 && def.Initialize == InitializeMonsterJoints) continue;
        std::memset(pool, 0xa5, 401*916); for(int i = 0; i < 400; ++i) AsCharacter(pool + 916*i)->Object.Live = 0;
        const int slot = int(index % 400), key = 5000 + int(index); expectedSlot = pool + slot*916;
        const bool allocate = !safe && index % 2 == 0;
        if(allocate) for(int i = 0; i < slot; ++i) { AsCharacter(pool + 916*i)->Object.Live = 1; AsCharacter(pool + 916*i)->Key = WORD(i); }
        else { AsCharacter(expectedSlot)->Object.Live = 1; AsCharacter(expectedSlot)->Key = WORD(key); }
        expectedType = mode == 1 ? 500 + (int(type) & 31) : def.Model;
        const int x = int(index * 13 % 512) - 128, y = int(index * 7 % 512) - 128;
        expectedX = BYTE(x); expectedY = BYTE(y); expectedRotation = 0;
        std::memcpy(models, canonical.data(), canonical.size()); auto expectedModels = canonical;
        if(mode != 1 && def.Initialize == InitializeMonster77) expectedModels[61236] = 0;
        gMapManager.world = world; gContentCatalog.mode = mode; HeroIndex = 0x12345678u;
        events.clear(); savedName.clear(); ctorCalls = cleanupCalls = jointCalls = 0;
        char* created = CreateMonster(type, x, y, key, -123);
        Require(created == reinterpret_cast<char*>(expectedSlot) && ctorCalls == 1 && cleanupCalls == int(allocate));
        Require(std::memcmp(models, expectedModels.data(), expectedModels.size()) == 0);
        Require(AsCharacter(created)->MonsterIndex == BYTE(type) && WORD(AsCharacter(created)->TargetCharacter) == WORD(HeroIndex));
        const char* name = getMonsterName(int(type)); if(mode == 1 && !name[0]) name = "custom"; Require(savedName == name);
        if(mode != 1 && def.Initialize == InitializeMonsterJoints) Require(jointCalls == 2); else Require(jointCalls == 0);
        digest = HashBytes(digest, created, 916); for(int event : events) digest = HashBytes(digest, &event, 4);
        digest = HashBytes(digest, savedName.data(), int(savedName.size())); ++cases;
    }
    digest = HashBytes(digest, &cases, 4); return digest;
}
int main() {
    auto* pool = static_cast<BYTE*>(Reserve(0x76000000u)); auto* models = static_cast<BYTE*>(Reserve(0x75000000u)); Require(pool && models);
    const auto reused = RunPool(pool, true), allPool = RunPool(pool, false);
    const auto safeMonsters = RunMonsters(pool, models, true), allMonsters = RunMonsters(pool, models, false);
    if(sizeof(void*) > 4) {
        auto* highPool = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highPool && highModels);
        std::memset(pool, 0, 401*916); Require(RunPool(highPool, true) == reused);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(pool, 0, 401*916); if(mask & 2) std::memset(models, 0, 1024*188);
            Require(RunMonsters(mask & 1 ? highPool : pool, mask & 2 ? highModels : models, true) == safeMonsters);
        }
        std::puts("creacion sobre 4 GiB: retorno/reutilizacion/centinela y cuatro combinaciones sin limpieza/joints; datos de acciones nulos");
    }
    std::printf("CreateCharacter: seguros %016llx, todos bajos %016llx; CreateMonster: seguros %016llx, todos bajos %016llx\n",
                static_cast<unsigned long long>(reused), static_cast<unsigned long long>(allPool), static_cast<unsigned long long>(safeMonsters), static_cast<unsigned long long>(allMonsters));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    types = read_source("src/Entity/EntityTypes.h", revision)
    obj = re.search(r"struct OBJECT\s*\{.*?\};", types, re.S).group()
    # Son campos opacos de 32 bits en este fixture; no se certifican como punteros.
    obj = re.sub(r"OBJECT\*\s+Owner", "std::uint32_t Owner", obj)
    obj = re.sub(r"vec34_t\*\s+BoneMatrices", "std::uint32_t BoneMatrices", obj)
    obj = re.sub(r"OBJECT\*\s+(ScenePrevious|SceneNext)\b", r"std::uint32_t \1", obj)
    records = "\n#pragma pack(push, 1)\n" + obj
    for name in ("PART_t", "CHARACTER"):
        records += re.search(rf"struct {name}\s*\{{.*?\}};", types, re.S).group()
    records += "\n#pragma pack(pop)\n"
    constants = "\n".join(re.findall(r"^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;", read_source("src/Item/ItemDefines.h", revision), re.M))
    header, globals_source = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    definitions = ""
    for name in ("CharactersClient", "DAT_05828d58"):
        definitions += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        definitions += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    catalog = re.search(r"struct CatalogMonster\s*\{.*?\};", read_source("src/Item/ContentCatalog.h", revision), re.S).group()
    prefix += SERVICES.replace("RECORDS", records).replace("CONSTANTS", constants).replace("GLOBALS", definitions).replace("CATALOG_RECORD", catalog)
    prefix += function(read_source("src/Entity/Entity_ActionLegacy.cpp", revision), "SetAction")
    prefix += function(read_source("src/Render/SMD_Parser.cpp", revision), "SetCharacterScale")
    monster = read_source("src/Monster/Monster.cpp", revision)
    factory = monster[monster.index("// IDA: CreateCharacter (0x0045BFA0)"):]
    call = "return reinterpret_cast<std::uintptr_t>(CreateCharacter(key, type, x, y, rotation));" if "CHARACTER* __cdecl CreateCharacter" in factory else "return CreateCharacter(key, type, x, y, rotation);"
    return prefix + factory + MAIN.replace("CALL_CREATE", call)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia unsigned int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-character-creation-") as temp:
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
