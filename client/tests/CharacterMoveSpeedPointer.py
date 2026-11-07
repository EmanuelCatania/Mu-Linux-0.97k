#!/usr/bin/env python3
"""Verifica velocidad real, efectos sobre Run y dirección del personaje.

OBJECT es un fixture escalar con offsets x86; no certifica layout nativo x64,
desplazamiento, terreno ni animación de una sesión de juego.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from PlayerStopPointer import harness as stop_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
using float10 = long double;
SPEED_FUNCTION
float10 CallSpeed(CHARACTER* character) { return CALL_SPEED; }
std::uint64_t RunSpeed(CHARACTER* character) {
    digest = 14695981039346656037ull;
    const short wings[] = {-1, 0, 788, 789, 790, 32767};
    const short helpers[] = {-1, 817, 818, 819, 820, 32767};
    const BYTE runs[] = {0, 1, 39, 40, 41, 255};
    int cases = 0;
    auto check = [&](short type, BYTE kind, short wing, short helper, BYTE safe, BYTE run, BYTE flags, WORD speed) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = type; character->Object.Kind = kind;
        character->Wing.Type = wing; character->Helper.Type = helper;
        character->SafeZone = safe; character->Run = run; character->MoveSpeed = speed;
        reinterpret_cast<BYTE*>(character)[120] = flags;
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        double value = speed;
        if(type == 390 && kind == 1) {
            const bool grounded = safe != 0 || (wing == -1 && helper != 818 && helper != 819);
            if(grounded) value = run < 40 ? 12.0 : 15.0;
            else { expected.Run = 40; value = wing == 789 ? 16.0 : 15.0; }
        }
        if(flags & 2) value *= 0.5;
        const float10 actual = CallSpeed(character);
        Require(actual == static_cast<float10>(value));
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Hash(character, sizeof(*character)); Hash(&value, sizeof(value)); ++cases;
    };
    // Cruce de todos los gates, fronteras de equipo y bytes de estado relevantes.
    for(short type : {0, 389, 390, 391})
    for(BYTE kind : {BYTE(0), BYTE(1), BYTE(2), BYTE(255)})
    for(short wing : wings) for(short helper : helpers)
    for(BYTE safe : {BYTE(0), BYTE(1), BYTE(255)})
    for(BYTE run : runs)
    for(BYTE flags : {BYTE(0), BYTE(1), BYTE(2), BYTE(3), BYTE(253), BYTE(255)})
        check(type, kind, wing, helper, safe, run, flags, 65535);
    // Los 65536 valores del WORD, sin gate de jugador y con/sin penalización.
    for(unsigned speed = 0; speed < 65536; ++speed)
        for(BYTE flags : {BYTE(0), BYTE(2)}) check(0, 1, -1, -1, 0, 0, flags, WORD(speed));
    Require(cases == 193280);
    return digest;
}
int main() {
    auto* low = static_cast<CHARACTER*>(Reserve(0x76000000u)); Require(low);
    const auto expected = RunSpeed(low);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low, 0, sizeof(*low));
        Require(RunSpeed(high) == expected);
        for(std::size_t i = 0; i < sizeof(*low); ++i) Require(reinterpret_cast<BYTE*>(low)[i] == 0);
        std::puts("velocidad sobre 4 GiB: personaje alto independiente de su alias bajo");
    }
    std::printf("CharacterMoveSpeed: 193280 estados y todos los WORD de velocidad; bytes/retorno %016llx\n",
                static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = stop_harness(revision).split("void Call(CHARACTER* character)", 1)[0]
    kind = re.search(r"BYTE\s+Kind\s*;", read_source("src/Entity/EntityTypes.h", revision)).group()
    prefix = prefix.replace("BYTE beforeAction[261 - 28];", "BYTE beforeKind[132 - 28]; " + kind + " BYTE beforeAction[261 - 133];", 1)
    prefix += "\nstatic_assert(sizeof(void*) != 4 || (offsetof(OBJECT, Kind) == 132 && offsetof(CHARACTER, MoveSpeed) == 762));\n"
    speed = function(read_source("src/Terrain/Terrain.cpp", revision), "CharacterMoveSpeed")
    arg = "character" if "CHARACTER* c" in speed else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + MAIN.replace("SPEED_FUNCTION", speed).replace("CALL_SPEED", f"CharacterMoveSpeed({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-speed-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes/retorno con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
