#!/usr/bin/env python3
"""Verifica brillo y recorrido real de items en el suelo sobre 4 GiB.

Terreno, azar y emisión de partículas controlados. Pool relocalizable del
banco con layout x86; no certifica pool nativo x64, render ni partículas reales.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemAnglePointer import harness as item_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
BYTE* DAT_07e12840;
OBJECT* expectedSparkle;
bool inPool;
int sparkleDraws, particleCalls;
int SparkleRandom() { ++sparkleDraws; return int(Input() % 32768); }
float lastSpark[3];
int Particle_Spawn(int type, float* position, float* angle, float* light, int flag, float alpha, int mode) {
    Require(type == 1230 && alpha == 1.0f && mode == 0 && flag == (particleCalls & 1));
    Require(light[0] == 1.0f && light[1] == 1.0f && light[2] == 1.0f);
    if(inPool) {
        const auto offset = reinterpret_cast<BYTE*>(angle) - DAT_07e12840;
        Require(offset >= 100 && offset < 1000 * 516 && offset % 516 == 100);
    } else Require(angle == expectedSparkle->Angle);
    std::memcpy(lastSpark, position, sizeof(lastSpark));
    Hash(position, 12); Hash(angle, 12); Hash(light, 12); Hash(&flag, sizeof(flag));
    ++particleCalls; return 0;
}
'''

MAIN = r'''
void CallSparkle(OBJECT* object) { CALL_SPARKLE; }
std::uint64_t RunSparkle(OBJECT* object) {
    inPool = false; expectedSparkle = object; seed = 0x97c50365; digest = 14695981039346656037ull;
    const int counters[] = {-97, -96, -49, -48, -47, -1, 0, 1, 47, 48, 49, 95, 96, 97};
    const float angles[] = {-180.0f, -0.0f, 0.0f, 45.0f, 90.0f, 180.0f, 270.0f};
    for(int trial = 0; trial < 16384; ++trial) {
        std::memset(object, 0x5a, sizeof(*object)); object->SubType = Pick(counters);
        for(int i = 0; i < 3; ++i) { object->Angle[i] = Pick(angles); object->Position[i] = float(int(Input() % 1024) - 512); }
        OBJECT expected; std::memcpy(&expected, object, sizeof(expected)); ++expected.SubType;
        const bool emit = object->SubType % 48 == 0;
        const auto beforeSeed = seed;
        float position[3] = {};
        if(emit) {
            float input[3] = {float(Input() % 32768 % 32 + 16), 0.0f, float(Input() % 32768 % 32 + 16)}, matrix[12];
            Matrix_BuildFromEuler(object->Angle, matrix); Vector_Rotate(input, matrix, position);
            for(int i = 0; i < 3; ++i) position[i] += object->Position[i];
        }
        const auto afterSeed = seed; seed = beforeSeed;
        particleCalls = sparkleDraws = 0; CallSparkle(object);
        Require(seed == afterSeed && particleCalls == (emit ? 2 : 0) && sparkleDraws == (emit ? 2 : 0));
        if(emit) Require(std::memcmp(lastSpark, position, sizeof(position)) == 0);
        Require(std::memcmp(object, &expected, sizeof(expected)) == 0);
        Hash(object, sizeof(*object)); Hash(&sparkleDraws, sizeof(sparkleDraws));
    }
    return digest;
}
std::uint64_t RunPool(BYTE* pool) {
    DAT_07e12840 = pool; inPool = true; seed = 0x97c50376; digest = 14695981039346656037ull;
    std::memset(pool, 0xa5, 1000 * 516);
    for(int i = 0; i < 1000; ++i) {
        auto* object = reinterpret_cast<OBJECT*>(pool + i * 516 + 72);
        reinterpret_cast<BYTE*>(object)[0] = i % 3 == 0;
        if(i % 3) continue;
        std::memset(object, 0, sizeof(*object)); reinterpret_cast<BYTE*>(object)[0] = 1;
        object->Type = short(399 + i % 600); object->SubType = i % 97 - 48;
        object->Position[0] = float(i * 8); object->Position[1] = float(i * 3); object->Position[2] = float(i % 500 - 100);
        const float velocity = float(i % 32 - 16); std::memcpy(pool + i * 516 + 288, &velocity, 4);
        for(int j = 0; j < 3; ++j) object->Angle[j] = float(i % 8 * 45);
    }
    for(int frame = 0; frame < 64; ++frame) {
        particleCalls = sparkleDraws = heightCalls = 0; MoveItems();
        Require(heightCalls == 334 && particleCalls == sparkleDraws);
        for(int i = 0; i < 1000; ++i) {
            for(int j = 0; j < 72; ++j) Require(pool[i * 516 + j] == 0xa5);
            if(i % 3) for(int j = 73; j < 516; ++j) Require(pool[i * 516 + j] == 0xa5);
        }
        Hash(pool, 1000 * 516); Hash(&particleCalls, sizeof(particleCalls)); Hash(&sparkleDraws, sizeof(sparkleDraws));
    }
    return digest;
}
int main() {
    auto* low = static_cast<BYTE*>(Reserve(0x76000000u)); Require(low);
    const auto direct = RunSparkle(reinterpret_cast<OBJECT*>(low)), pool = RunPool(low);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low, 0, 1000 * 516);
        Require(RunSparkle(reinterpret_cast<OBJECT*>(high)) == direct && RunPool(high) == pool);
        for(int i = 0; i < 1000 * 516; ++i) Require(low[i] == 0);
        std::puts("items sobre 4 GiB: brillo y pool alto; alias bajo intacto");
    }
    std::printf("brillo: 16384 estados %016llx; MoveItems: 1000 slots, 64 frames %016llx\n",
                static_cast<unsigned long long>(direct), static_cast<unsigned long long>(pool));
}
'''


def harness(revision=None):
    prefix = item_harness(revision).split("void CallItemAngle(OBJECT* object)", 1)[0]
    globals_source = read_source("src/globals.cpp", revision)
    for name in ("_DAT_005527d0", "_DAT_005528e4", "_DAT_00552a28"):
        prefix += re.search(rf"^float\s+{name}\s*=[^;]+;", globals_source, re.M).group() + "\n"
    sparkle = function(read_source("src/Entity/Entity_LegacyEffects.cpp", revision), "Entity_UpdateSparkleEffect").replace("_rand()", "SparkleRandom()")
    move = function(read_source("src/Util/Misc.cpp", revision), "MoveItems")
    arg = "object" if "OBJECT* object" in sparkle else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix + SERVICES + sparkle + move + MAIN.replace("CALL_SPARKLE", f"Entity_UpdateSparkleEffect({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-ground-items-") as temp:
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
