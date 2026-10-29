#!/usr/bin/env python3
"""Aísla CheckTarget real y conserva su dirección de personaje y de pool.

El doble CHARACTER expone sólo TargetPosition en el offset x86 verificado en
IDA. No representa el layout nativo x64. El render de terreno se sustituye
por servicios que comprueban sus argumentos y permiten hit/miss controlados.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SUPPORT = r'''
using vec3_t = float[3];
struct CHARACTER {
    BYTE prefix[0x314];
    TARGET_FIELD
    BYTE suffix[0x394 - 0x314 - 12];
};
static_assert(sizeof(CHARACTER) == 0x394);
static_assert(offsetof(CHARACTER, TargetPosition) == 788);
CHARACTER* AsCharacter(CHARACTER* p) { return p; }
CHARACTER* AsCharacter(std::uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }
int SelectedCharacter;
DWORD TargetX, TargetY, DAT_080ab288, DAT_080ab28c;
float DAT_083a4130, DAT_083a4134, DAT_083a4138;
bool terrainHit;
int terrainCalls, tileCalls;
float Float(DWORD bits) { float f; std::memcpy(&f, &bits, 4); return f; }
DWORD Bits(float f) { DWORD bits; std::memcpy(&bits, &f, 4); return bits; }
void RenderTerrain(char mode) { Require(mode == 1); ++terrainCalls; }
int RenderTerrainTile(int x, int y, int gx, int gy, float size, int a, int b) {
    Require(DWORD(x) == DAT_080ab288 && DWORD(y) == DAT_080ab28c);
    Require(gx == int(Float(DAT_080ab288)) && gy == int(Float(DAT_080ab28c)));
    Require(size == 1.0f && a == 1 && b == 1); ++tileCalls; return terrainHit;
}
std::uint64_t digest;
void Hash(const void* data, std::size_t n) {
    const auto* bytes = static_cast<const BYTE*>(data);
    for(std::size_t i = 0; i < n; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
}
'''

MAIN = r'''
std::uint64_t Run(BYTE* pool, CHARACTER* character) {
    CharactersClient = static_cast<decltype(CharactersClient)>(reinterpret_cast<std::uintptr_t>(pool));
    std::memset(pool, 0xa5, 400 * 916);
    for(int slot = 0; slot < 400; ++slot) {
        const float position[] = {slot * 123.5f - 20000.0f, slot * -31.25f + 9000.0f, slot * 0.125f};
        std::memcpy(pool + slot * 916 + 16, position, 12);
    }
    std::vector<BYTE> unchanged(pool, pool + 400 * 916);
    digest = 14695981039346656037ull;
    for(int test = 0; test < 410; ++test) {
        std::memset(character, 0x5a, sizeof(*character));
        std::vector<BYTE> expected(reinterpret_cast<BYTE*>(character), reinterpret_cast<BYTE*>(character) + sizeof(*character));
        SelectedCharacter = test < 400 ? test : -1;
        terrainHit = (test % 2) != 0;
        terrainCalls = tileCalls = 0;
        DAT_080ab288 = Bits(-12.75f); DAT_080ab28c = Bits(234.5f);
        DAT_083a4130 = (test - 405) * 99.75f;
        DAT_083a4134 = (test - 402) * -150.25f; DAT_083a4138 = 345.125f;
        TargetX = 0x12345678; TargetY = 0x98765432;
        const bool hit = test < 400 || terrainHit;
        float position[3] = {DAT_083a4130, DAT_083a4134, DAT_083a4138};
        if(test < 400) std::memcpy(position, pool + test * 916 + 16, 12);
        if(hit) std::memcpy(expected.data() + 788, position, 12);
        const bool result = CALL_TARGET;
        Require(result == hit && terrainCalls == (test >= 400) && tileCalls == (test >= 400));
        Require(std::memcmp(character, expected.data(), expected.size()) == 0);
        Require(TargetX == (hit ? DWORD(int(position[0] * 0.01f)) : 0x12345678));
        Require(TargetY == (hit ? DWORD(int(position[1] * 0.01f)) : 0x98765432));
        Require(std::memcmp(pool, unchanged.data(), unchanged.size()) == 0);
        Hash(character, sizeof(*character)); Hash(&TargetX, 4); Hash(&TargetY, 4);
        Hash(&result, sizeof(result)); Hash(&terrainCalls, sizeof(terrainCalls));
    }
    return digest;
}
int main() {
    auto* pool = static_cast<BYTE*>(Reserve(0x6a000000u));
    auto* character = static_cast<CHARACTER*>(Reserve(0x6b000000u)); Require(pool && character);
    const auto expected = Run(pool, character);
    if(sizeof(void*) > 4) {
        auto* highPool = static_cast<BYTE*>(Reserve(std::uint64_t(0x16a000000ull)));
        auto* highCharacter = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x16b000000ull)));
        Require(highPool && highCharacter);
        // Un truncamiento no debe poder leer una copia idéntica en la dirección baja.
        std::memset(pool, 0xcc, 400 * 916);
        Require(Run(highPool, character) == expected);
        Require(Run(pool, highCharacter) == expected);
        Require(Run(highPool, highCharacter) == expected);
        std::puts("pool y personaje sobre 4 GiB: cuatro combinaciones equivalentes");
    }
    std::printf("CheckTarget: 400 objetivos y 10 hit/miss de terreno; bytes/eventos %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    header = read_source("src/globals.h", revision)
    globals_source = read_source("src/globals.cpp", revision)
    declaration = re.search(r"extern [^;\n]*\bCharactersClient\s*;", header).group()
    definition = re.search(r"^[^/\n]*\bCharactersClient\s*=\s*[^;]+;", globals_source, re.M).group()
    alias = re.search(r"^#define DAT_07abf5d0[^\n]+", read_source("src/structs.h", revision), re.M).group()
    field = re.search(r"vec3_t\s+TargetPosition\s*;", read_source("src/Entity/EntityTypes.h", revision)).group()
    target = function(read_source("src/Combat/Combat_Targeting.cpp", revision), "CheckTarget")
    call = "CheckTarget(character)" if "CHARACTER* c" in target else "CheckTarget(static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(character)))"
    return (PREAMBLE.split("BYTE* reservation;")[0].replace("0x10000", "0x100000")
            + declaration + definition + "\n" + alias + "\n"
            + SUPPORT.replace("TARGET_FIELD", field) + target + MAIN.replace("CALL_TARGET", call))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-target-pointer-") as temp:
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
