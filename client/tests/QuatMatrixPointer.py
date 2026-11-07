#!/usr/bin/env python3
"""Prueba matriz real de cuaterniones con buffers altos y solapados.

Compara los bytes con x86; no valida la carga/animación de modelos completos.
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
void CallMatrix(const float* q, float* m) { CALL_MATRIX; }
std::uint64_t RunMatrix(float* input, float* output) {
    seed = 0x97c4fa27; std::uint64_t digest = 14695981039346656037ull;
    const std::uint32_t values[] = {0u, 0x80000000u, 1u, 0x80000001u, 0x3f000000u, 0xbf000000u,
        0x3f800000u, 0xbf800000u, 0x3f3504f3u, 0xbf3504f3u, 0x40000000u, 0xc0000000u};
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(input, 0x5a, 80); std::memset(output, 0xa5, 80);
        const int alias = trial % 4;
        float* q = input + 2;
        float* m = alias == 0 ? output + 2 : q + alias - 1;
        for(int i = 0; i < 4; ++i) { const auto bits = values[Input() % 12]; std::memcpy(q + i, &bits, 4); }
        BYTE beforeInput[80], beforeOutput[80]; std::memcpy(beforeInput, input, 80); std::memcpy(beforeOutput, output, 80);
        CallMatrix(q, m);
        BYTE allowedInput[80], allowedOutput[80]; std::memcpy(allowedInput, beforeInput, 80); std::memcpy(allowedOutput, beforeOutput, 80);
        BYTE* allowed = alias == 0 ? allowedOutput + 8 : allowedInput + (reinterpret_cast<BYTE*>(m) - reinterpret_cast<BYTE*>(input));
        for(int i : {0, 1, 2, 4, 5, 6, 8, 9, 10}) std::memcpy(allowed + i * 4, m + i, 4);
        Require(std::memcmp(input, allowedInput, 80) == 0 && std::memcmp(output, allowedOutput, 80) == 0);
        if(trial % 256 == 0) {
            const float identity[] = {0.0f, 0.0f, 0.0f, 1.0f}; std::memcpy(input + 2, identity, 16);
            CallMatrix(input + 2, output + 2);
            Require(output[2] == 1.0f && output[7] == 1.0f && output[12] == 1.0f);
            for(int i : {1, 2, 4, 6, 8, 9}) Require(output[2 + i] == 0.0f);
        }
        for(const BYTE* bytes : {reinterpret_cast<BYTE*>(input), reinterpret_cast<BYTE*>(output)})
            for(int i = 0; i < 80; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* input = static_cast<float*>(Reserve(0x76000000u));
    auto* output = static_cast<float*>(Reserve(0x75000000u)); Require(input && output);
    const auto expected = RunMatrix(input, output);
    if(sizeof(void*) > 4) {
        auto* highInput = static_cast<float*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highOutput = static_cast<float*>(Reserve(std::uint64_t(0x175000000ull))); Require(highInput && highOutput);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(input, 0, 80); if(mask & 2) std::memset(output, 0, 80);
            Require(RunMatrix(mask & 1 ? highInput : input, mask & 2 ? highOutput : output) == expected);
        }
        std::puts("matriz sobre 4 GiB: cuatro combinaciones y tres solapamientos equivalentes");
    }
    std::printf("QuatToMatrix: 65536 casos, identidad y canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += re.search(r"^float\s+_DAT_00552cf0\s*=[^;]+;", read_source("src/globals.cpp", revision), re.M).group()
    matrix = function(read_source("src/Render/Render_LegacyTransforms.cpp", revision), "QuatToMatrix")
    args = "q, m" if "const float* q" in matrix else "static_cast<int>(reinterpret_cast<std::uintptr_t>(q)), static_cast<int>(reinterpret_cast<std::uintptr_t>(m))"
    return prefix + matrix + MAIN.replace("CALL_MATRIX", f"QuatToMatrix({args}, 123, -456)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quat-matrix-") as temp:
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
