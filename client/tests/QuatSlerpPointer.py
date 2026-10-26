#!/usr/bin/env python3
"""Verifica interpolación real con tres buffers altos y factor float exacto.

Conserva mutación del segundo cuaternión y casos de alias del port actual.
No certifica animación completa ni equivalencia general entre CRT.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint32_t seed;
std::uint32_t Input() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
void CallSlerp(const float* a, float* b, float t, float* output) { CALL_SLERP; }
std::uint64_t RunSlerp(float* a, float* b, float* output) {
    seed = 0x97c4fa35; std::uint64_t digest = 14695981039346656037ull;
    const float quats[][4] = {{0,0,0,1}, {0,0,0,-1}, {1,0,0,0}, {-1,0,0,0}, {0,1,0,0}, {0,-1,0,0},
        {0,0,1,0}, {0,0,-1,0}, {.5f,.5f,.5f,.5f}, {-.5f,-.5f,-.5f,-.5f}, {.70710677f,0,0,.70710677f}, {0,.70710677f,0,.70710677f}};
    const std::uint32_t factors[] = {0u,0x80000000u,1u,0x80000001u,0x3e800000u,0x3f000000u,0x3f400000u,
                                    0x3f800000u,0xbf800000u,0x40000000u,0x7fc00001u};
    for(int trial = 0; trial < 32768; ++trial) {
        std::memset(a, 0x5a, 64); std::memset(b, 0xa5, 64); std::memset(output, 0x3c, 64);
        float* q1 = a + 2; float* q2 = b + 2;
        const int alias = trial % 5;
        float* dest = alias == 0 ? output + 2 : alias < 3 ? q1 + alias - 1 : q2 + alias - 3;
        std::memcpy(q1, quats[Input() % 12], 16); std::memcpy(q2, quats[Input() % 12], 16);
        const auto bits = factors[Input() % 11]; float t; std::memcpy(&t, &bits, 4);
        const float dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
        float expectedQ2[4]; std::memcpy(expectedQ2, q2, 16);
        if(dot < 0) for(float& value : expectedQ2) value = -value;
        BYTE beforeA[64], beforeB[64], beforeOutput[64];
        std::memcpy(beforeA, a, 64); std::memcpy(beforeB, b, 64); std::memcpy(beforeOutput, output, 64);
        CallSlerp(q1, q2, t, dest);
        BYTE allowedA[64], allowedB[64], allowedOutput[64];
        std::memcpy(allowedA, beforeA, 64); std::memcpy(allowedB, beforeB, 64); std::memcpy(allowedOutput, beforeOutput, 64);
        std::memcpy(allowedB + 8, expectedQ2, 16);
        BYTE* allowed = alias == 0 ? allowedOutput + 8 : alias < 3 ? allowedA + 8 + 4*(alias-1) : allowedB + 8 + 4*(alias-3);
        std::memcpy(allowed, dest, 16);
        Require(std::memcmp(a, allowedA, 64) == 0 && std::memcmp(b, allowedB, 64) == 0 && std::memcmp(output, allowedOutput, 64) == 0);
        if(alias == 0 && std::isfinite(t) && t >= 0 && t <= 1) {
            double norm = 0; for(int i = 0; i < 4; ++i) norm += double(dest[i])*dest[i];
            Require(std::fabs(norm - 1) < 0.00001);
        }
        for(const BYTE* bytes : {reinterpret_cast<BYTE*>(a), reinterpret_cast<BYTE*>(b), reinterpret_cast<BYTE*>(output)})
            for(int i = 0; i < 64; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* a = static_cast<float*>(Reserve(0x76000000u)); auto* b = static_cast<float*>(Reserve(0x75000000u));
    auto* output = static_cast<float*>(Reserve(0x74000000u)); Require(a && b && output);
    const auto expected = RunSlerp(a, b, output);
    if(sizeof(void*) > 4) {
        auto* highA = static_cast<float*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highB = static_cast<float*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highOutput = static_cast<float*>(Reserve(std::uint64_t(0x174000000ull))); Require(highA && highB && highOutput);
        for(int mask = 1; mask < 8; ++mask) {
            if(mask & 1) std::memset(a, 0, 64); if(mask & 2) std::memset(b, 0, 64); if(mask & 4) std::memset(output, 0, 64);
            Require(RunSlerp(mask & 1 ? highA : a, mask & 2 ? highB : b, mask & 4 ? highOutput : output) == expected);
        }
        std::puts("slerp sobre 4 GiB: ocho combinaciones y cuatro solapamientos equivalentes");
    }
    std::printf("QuatSlerp: 32768 casos, factor/mutacion/canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0] + "\n#include <cmath>\n"
    prefix += re.search(r"^float\s+_DAT_00552d00\s*=[^;]+;", read_source("src/globals.cpp", revision), re.M).group()
    slerp = function(read_source("src/Render/Render_LegacyTransforms.cpp", revision), "QuatSlerp")
    call = "QuatSlerp(a, b, t, output);" if "const float* q1" in slerp else "int bits; std::memcpy(&bits, &t, 4); QuatSlerp(static_cast<int>(reinterpret_cast<std::uintptr_t>(a)), static_cast<int>(reinterpret_cast<std::uintptr_t>(b)), bits, static_cast<int>(reinterpret_cast<std::uintptr_t>(output)));"
    return prefix + slerp + MAIN.replace("CALL_SLERP", call)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quat-slerp-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
