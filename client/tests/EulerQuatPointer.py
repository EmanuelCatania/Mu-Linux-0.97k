#!/usr/bin/env python3
"""Verifica conversión real Euler a cuaternión con buffers altos/solapados.

No certifica lectura BMD, registros con punteros ni preparación de poses.
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
void CallEuler(const float* input, float* output) { CALL_EULER; }
std::uint64_t RunEuler(float* input, float* output) {
    seed = 0x97c4fa1d; std::uint64_t digest = 14695981039346656037ull;
    const float angles[] = {-10000000000.0f, -1000.0f, -6.283185f, -3.1415927f, -1.0f,
                             -0.00000001f, -0.0f, 0.0f, 0.00000001f, 1.0f, 3.1415927f, 6.283185f, 1000.0f, 10000000000.0f};
    for(int trial = 0; trial < 65536; ++trial) {
        std::memset(input, 0x5a, 64); std::memset(output, 0xa5, 64);
        float* a = input + 2; float* q = trial % 4 == 0 ? output + 2 : a + trial % 4 - 1;
        for(int i = 0; i < 3; ++i) a[i] = angles[Input() % 14];
        BYTE beforeInput[64], beforeOutput[64]; std::memcpy(beforeInput, input, 64); std::memcpy(beforeOutput, output, 64);
        CallEuler(a, q);
        BYTE allowedInput[64], allowedOutput[64]; std::memcpy(allowedInput, beforeInput, 64); std::memcpy(allowedOutput, beforeOutput, 64);
        BYTE* allowed = trial % 4 == 0 ? allowedOutput + 8 : allowedInput + (reinterpret_cast<BYTE*>(q) - reinterpret_cast<BYTE*>(input));
        std::memcpy(allowed, q, 16);
        Require(std::memcmp(input, allowedInput, 64) == 0 && std::memcmp(output, allowedOutput, 64) == 0);
        const double norm = double(q[0])*q[0] + double(q[1])*q[1] + double(q[2])*q[2] + double(q[3])*q[3];
        Require(std::fabs(norm - 1.0) < 0.00001);
        for(const BYTE* bytes : {reinterpret_cast<BYTE*>(input), reinterpret_cast<BYTE*>(output)})
            for(int i = 0; i < 64; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* input = static_cast<float*>(Reserve(0x76000000u));
    auto* output = static_cast<float*>(Reserve(0x75000000u)); Require(input && output);
    const auto expected = RunEuler(input, output);
    if(sizeof(void*) > 4) {
        auto* highInput = static_cast<float*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highOutput = static_cast<float*>(Reserve(std::uint64_t(0x175000000ull))); Require(highInput && highOutput);
        for(int mask = 1; mask < 4; ++mask) {
            if(mask & 1) std::memset(input, 0, 64); if(mask & 2) std::memset(output, 0, 64);
            Require(RunEuler(mask & 1 ? highInput : input, mask & 2 ? highOutput : output) == expected);
        }
        std::puts("Euler sobre 4 GiB: cuatro combinaciones y tres solapamientos equivalentes");
    }
    std::printf("EulerToQuat: 65536 casos, norma y canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0] + "\n#include <cmath>\n"
    prefix += re.search(r"^float\s+_DAT_00552ce0\s*=[^;]+;", read_source("src/globals.cpp", revision), re.M).group()
    euler = function(read_source("src/Render/Render_LegacyTransforms.cpp", revision), "EulerToQuat")
    args = "input, output" if "const float* param_1" in euler else "static_cast<int>(reinterpret_cast<std::uintptr_t>(input)), static_cast<int>(reinterpret_cast<std::uintptr_t>(output))"
    return prefix + euler + MAIN.replace("CALL_EULER", f"EulerToQuat({args}, 123, -456)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-euler-quat-") as temp:
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
