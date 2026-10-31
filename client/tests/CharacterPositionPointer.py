#!/usr/bin/env python3
"""Prueba desplazamiento con velocidad y matrices reales y personaje alto.

Terreno y fabs son servicios controlados; OBJECT conserva offsets escalares
x86. Verifica el port actual, incluidas sus diferencias de altura con IDA.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from CharacterMoveSpeedPointer import harness as speed_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int heightCalls, fabsCalls;
float heightX, heightY;
float RequestTerrainHeight(float x, float y) {
    ++heightCalls; heightX = x; heightY = y;
    return x * 0.125f + y * 0.25f - 17.0f;
}
float10 Math_Fabs(float value) { ++fabsCalls; return std::fabs(value); }
'''

MAIN = r'''
void CallPosition(CHARACTER* character) { CALL_POSITION; }
std::uint64_t RunPosition(CHARACTER* character) {
    digest = 14695981039346656037ull; seed = 0x97c454ba;
    const short types[] = {0, 272, 273, 390};
    const short helpers[] = {-1, 818, 819, 820}, wings[] = {-1, 789, 790};
    const int worlds[] = {-1, 0, 7, 8, 9, 10, 11};
    const float angles[] = {-360.0f, -90.0f, 0.0f, 45.0f, 90.0f, 180.0f, 359.0f};
    const float timers[] = {-3.0f, -0.0f, 0.0f, 0.5f, 1.5f, 3.0f};
    const WORD speeds[] = {0, 1, 12, 15, 16, 255, 65535};
    const BYTE runs[] = {0, 39, 40, 41, 255};
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Object.Type = Pick(types); character->Object.Kind = BYTE(Input() % 3);
        character->Helper.Type = Pick(helpers); character->Wing.Type = Pick(wings);
        character->SafeZone = BYTE(Input() % 2); character->Run = Pick(runs);
        character->MoveSpeed = Pick(speeds); reinterpret_cast<BYTE*>(character)[120] = BYTE(Input() % 4);
        character->Object.Timer = Pick(timers); gMapManager.world = Pick(worlds);
        for(int i = 0; i < 3; ++i) {
            character->Object.Position[i] = float(int(Input() % 65536) - 32768) * 0.125f;
            character->Object.Angle[i] = Pick(angles);
        }
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        float velocity[3] = {0.0f, -float(CharacterMoveSpeed(&expected)), 0.0f}, matrix[12], rotated[3];
        Matrix_BuildFromEuler(expected.Object.Angle, matrix); Vector_Rotate(velocity, matrix, rotated);
        for(int i = 0; i < 3; ++i) expected.Object.Position[i] = rotated[i] + expected.Object.Position[i];
        const float x = expected.Object.Position[0], y = expected.Object.Position[1];
        float height = x * 0.125f + y * 0.25f - 17.0f;
        // Gate del port: no introducir aquí la corrección World/SafeZone de IDA.
        if(expected.Helper.Type == 819) height += gMapManager.world == 8 || gMapManager.world == 10 ? _DAT_00552848 : _DAT_0055284c;
        expected.Object.Position[2] = height;
        if(expected.Object.Type == 272) {
            const float bob = float(sin(expected.Object.Timer));
            expected.Object.Position[2] = expected.Object.Position[2] - bob * 70.0f + 70.0f;
        }
        expected.Object.Timer = expected.Object.Timer + _DAT_00552934;
        heightCalls = fabsCalls = 0; CallPosition(character);
        Require(heightCalls == 1 && fabsCalls == int(expected.Object.Type == 272));
        Require(heightX == x && heightY == y);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Hash(character, sizeof(*character)); Hash(&heightCalls, sizeof(heightCalls)); Hash(&fabsCalls, sizeof(fabsCalls));
    }
    return digest;
}
int main() {
    auto* low = static_cast<CHARACTER*>(Reserve(0x76000000u)); Require(low);
    const auto expected = RunPosition(low);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low, 0, sizeof(*low)); Require(RunPosition(high) == expected);
        for(std::size_t i = 0; i < sizeof(*low); ++i) Require(reinterpret_cast<BYTE*>(low)[i] == 0);
        std::puts("desplazamiento sobre 4 GiB: personaje alto y alias bajo intacto");
    }
    std::printf("MoveCharacterPosition: 65536 estados, bytes/terreno %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = speed_harness(revision).split("float10 CallSpeed(CHARACTER* character)", 1)[0]
    types = read_source("src/Entity/EntityTypes.h", revision)
    angle = re.search(r"vec3_t\s+Angle\s*;", types).group()
    timer = re.search(r"float\s+Timer\s*;", types).group()
    prefix = prefix.replace("BYTE beforeKind[132 - 28];", angle + " BYTE beforeTimer[128 - 40]; " + timer, 1)
    prefix += "\nstatic_assert(sizeof(void*) != 4 || (offsetof(OBJECT, Angle) == 28 && offsetof(OBJECT, Timer) == 128));\n"
    globals_source = read_source("src/globals.cpp", revision)
    for name in ("_DAT_00552848", "_DAT_0055284c", "_DAT_00552934"):
        prefix += re.search(rf"^float\s+{name}\s*=[^;]+;", globals_source, re.M).group() + "\n"
    position = function(read_source("src/Net/SecondPassword.cpp", revision), "MoveCharacterPosition")
    arg = "character" if "CHARACTER* character" in position else "static_cast<int>(reinterpret_cast<std::uintptr_t>(character))"
    return prefix + SERVICES + position + MAIN.replace("CALL_POSITION", f"MoveCharacterPosition({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if getattr(args, "architecture", "x64") == "x64":
        print("Omitido en x64: banco atado al layout x86; lo cubren los bancos nativos de B2")
        return
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-position-pointer-") as temp:
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
