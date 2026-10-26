#!/usr/bin/env python3
"""Verifica comparación real de cuatro floats con ambas direcciones altas.

Incluye NaN: conserva la semántica actual del port aunque difiera de IDA.
No prueba la preparación de poses ni el render que usa esta comparación.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
#include <cmath>
std::uint32_t seed;
std::uint32_t Input() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
int CallQuad(const float* a, const float* b) { return CALL_QUAD; }
std::uint64_t RunQuad(float* a, float* b) {
    seed = 0x97c4f9c7; std::uint64_t digest = 14695981039346656037ull;
    const std::uint32_t values[] = {0u, 0x80000000u, 1u, 0x80000001u, 0x3a83126eu, 0x3a83126fu, 0x3a831270u,
        0xba83126fu, 0x3f800000u, 0xbf800000u, 0x7f7fffffu, 0xff7fffffu, 0x7f800000u, 0xff800000u, 0x7fc00001u, 0xffc00001u};
    for(int trial = 0; trial < 65536; ++trial) {
        for(int i = 0; i < 4; ++i) {
            const auto va = values[Input() % 16], vb = trial % 3 == 0 ? va : values[Input() % 16];
            std::memcpy(a + i, &va, 4); std::memcpy(b + i, &vb, 4);
        }
        std::uint32_t beforeA[4], beforeB[4]; std::memcpy(beforeA, a, 16); std::memcpy(beforeB, b, 16);
        int expected = 1;
        for(int i = 0; i < 4; ++i) if(std::fabs(a[i] - b[i]) > _DAT_005528a8) expected = 0;
        const int actual = CallQuad(a, b);
        Require(actual == expected && CallQuad(b, a) == expected && CallQuad(a, a) == 1);
        Require(std::memcmp(beforeA, a, 16) == 0 && std::memcmp(beforeB, b, 16) == 0);
        digest ^= unsigned(actual); digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* a = static_cast<float*>(Reserve(0x76000000u));
    auto* b = static_cast<float*>(Reserve(0x75000000u)); Require(a && b);
    const auto expected = RunQuad(a, b);
    if(sizeof(void*) > 4) {
        auto* highA = static_cast<float*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highB = static_cast<float*>(Reserve(std::uint64_t(0x175000000ull))); Require(highA && highB);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(a, 0xa5, 16); if(mask & 2) std::memset(b, 0x5a, 16);
            Require(RunQuad(mask & 1 ? highA : a, mask & 2 ? highB : b) == expected);
        }
        std::puts("comparacion sobre 4 GiB: cuatro combinaciones de operandos equivalentes");
    }
    std::printf("Terrain_QuadEqual: 65536 pares, simetria y alias; retorno %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += re.search(r"^float\s+_DAT_005528a8\s*=[^;]+;", read_source("src/globals.cpp", revision), re.M).group()
    quad = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Terrain_QuadEqual")
    args = "a, b" if "const float* param_1" in quad else "static_cast<int>(reinterpret_cast<std::uintptr_t>(a)), static_cast<int>(reinterpret_cast<std::uintptr_t>(b))"
    return prefix + quad + MAIN.replace("CALL_QUAD", f"Terrain_QuadEqual({args}, 123, -456)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quad-equal-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de retorno con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
