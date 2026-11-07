#!/usr/bin/env python3
"""Verifica opciones de guantes/alas reales con CharacterMachine alto.

Buffer escalar del banco y accessor reales; no certifica CalculateAll ni el
resto del cálculo de atributos o su interfaz.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
int CallOptions(BYTE* machine) { return CALL_OPTIONS; }
std::uint64_t RunOptions(BYTE* storage) {
    std::uint64_t digest = 14695981039346656037ull;
    BYTE* machine = storage + 16;
    for(unsigned value = 0; value < 65536; ++value)
    for(int mode = 0; mode < 4; ++mode) {
        std::memset(storage, 0x5a, 1444);
        const short gloveType = value % 7 == 0 ? -1 : 500, wingType = value % 11 == 0 ? -1 : 600;
        const unsigned short glove = static_cast<unsigned short>(value), wing = static_cast<unsigned short>(65535 - value);
        std::memcpy(machine + 944, &gloveType, 2); std::memcpy(machine + 1012, &wingType, 2);
        std::memcpy(machine + 968, &glove, 2); std::memcpy(machine + 1036, &wing, 2);
        machine[970] = mode & 1 ? BYTE(255) : BYTE(0); machine[1038] = mode & 2 ? BYTE(1) : BYTE(0);
        BYTE expected[1444]; std::memcpy(expected, storage, sizeof(expected));
        unsigned short result; std::memcpy(&result, machine + 82, 2);
        if(mode & 1) result = gloveType == -1 ? 0 : glove;
        const unsigned short extra = mode & 2 && wingType != -1 ? wing : 0;
        if(mode & 2) result = static_cast<unsigned short>(result + extra);
        std::memcpy(expected + 16 + 82, &result, 2);
        const int returned = CallOptions(machine);
        Require(returned == int(static_cast<unsigned char>(extra)));
        Require(std::memcmp(storage, expected, sizeof(expected)) == 0);
        for(int i = 0; i < 1444; ++i) { digest ^= storage[i]; digest *= 1099511628211ull; }
        digest ^= unsigned(returned); digest *= 1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low = static_cast<BYTE*>(Reserve(0x76000000u)); Require(low);
    const auto expected = RunOptions(low);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low, 0, 1444); Require(RunOptions(high) == expected);
        for(int i = 0; i < 1444; ++i) Require(low[i] == 0);
        std::puts("opciones sobre 4 GiB: CharacterMachine alto y alias bajo intacto");
    }
    std::printf("guantes/alas: 262144 estados, overflow/retorno/canarios; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += function(read_source("src/Core/Runtime_Small.cpp", revision), "FUN_0047d000")
    options = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Stats_ExtraOptionGlovesWings")
    arg = "machine" if "BYTE* param_1" in options else "static_cast<int>(reinterpret_cast<std::uintptr_t>(machine))"
    return prefix + options + MAIN.replace("CALL_OPTIONS", f"Stats_ExtraOptionGlovesWings({arg})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-gloves-wings-") as temp:
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
