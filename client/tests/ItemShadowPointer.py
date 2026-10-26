#!/usr/bin/env python3
"""Verifica la tabla sombra de items con item.bmd y el lector/escritor reales.

La base primaria todavía queda bajo 4 GiB: su migración no es parte de este
cambio. La conversión de nombres se sustituye por un validador de direcciones.
"""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE as IO_PREAMBLE
from SceneObjectPointer import ROOT, build_and_run, function, read_source

EXTRA = r'''
using UINT = unsigned int;
DWORD DAT_01c5e200 = 0x01c5e200;
const char s__s___File_corrupted__00559bd4[] = "archivo corrupto";
BYTE* primary;
BYTE* shadowReservation;
void* AllocateShadow(std::size_t n) { Require(n == 0x8000); return shadowReservation + 16; }
'''

TEXT = r'''
namespace TextEngine {
void ConvertWindows1252InPlace(char* p, int size) {
    const auto address = reinterpret_cast<std::uintptr_t>(p);
    const auto base = reinterpret_cast<std::uintptr_t>(primary);
    const auto shadow = reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*>(DAT_07cf1ff0));
    Require(size == 30 && ((address >= base && address + size <= base + 0x8000) ||
                          (address >= shadow && address + size <= shadow + 0x8000)));
}
}
'''

MAIN = r'''
std::vector<BYTE> Read(const char* path) {
    std::ifstream f(path, std::ios::binary); Require(f.good());
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}
void Run(std::uintptr_t address, const std::vector<BYTE>& file) {
    shadowReservation = static_cast<BYTE*>(Reserve(address));
    Require(shadowReservation != nullptr);
    std::memset(shadowReservation, 0xa5, 0x8000 + 32);
    std::memset(primary - 16, 0xa5, 0x8000 + 32);
    InitializeShadow();
    Require(reinterpret_cast<void*>(DAT_07cf1ff0) == shadowReservation + 16);
    Item_LoadBMD(SAMPLE_PATH);
    const auto* shadow = reinterpret_cast<const BYTE*>(DAT_07cf1ff0);
    for(int i = 0; i < 0x8000; ++i) {
        const BYTE plain = BYTE(file[i] ^ BYTE(DAT_00559bb4[i % 64 % 3]));
        Require(primary[i] == plain);
        Require(shadow[i] == BYTE(i % 64 == 39 ? plain * 2 : plain));
    }
    Item_SaveBMD("items-roundtrip.bmd");
    Require(Read("items-roundtrip.bmd") == file);
    for(int i = 0; i < 16; ++i) {
        Require(shadowReservation[i] == 0xa5 && shadowReservation[0x8000 + 16 + i] == 0xa5);
        Require(primary[i - 16] == 0xa5 && primary[0x8000 + i] == 0xa5);
    }
}
int main() {
    const auto file = Read(SAMPLE_PATH); Require(file.size() == 0x8004);
    primary = static_cast<BYTE*>(Reserve(0x60000000u)); Require(primary != nullptr);
    primary += 16;
    // Contrato primario heredado, expresamente fuera del alcance de esta prueba.
    DAT_07d78068 = static_cast<int>(reinterpret_cast<std::uintptr_t>(primary));
    Run(0x61000000u, file);
    if(sizeof(void*) > 4) {
        Run(std::uint64_t(0x161000000ull), file);
        std::puts("sombra de items sobre 4 GiB: reserva, lector/escritor y canarios correctos");
    }
    std::puts("item.bmd: 512 registros de 64 bytes, sombra y checksum sin cambios");
}
'''


def harness(revision=None):
    header = read_source("src/globals.h", revision)
    globals_source = read_source("src/globals.cpp", revision)
    declarations = ""
    for name in ("DAT_07d78068", "DAT_07cf1ff0"):
        declarations += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        declarations += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    allocation = re.search(r"^[ \t]*DAT_07cf1ff0\s*=\s*[^;]+;",
                           read_source("src/Platform/ClientEntry.cpp", revision), re.M).group()
    allocation = allocation.replace("malloc(0x8000)", "AllocateShadow(0x8000)")
    data = read_source("src/Item/Item_Data.cpp", revision)
    scene = read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    sample = ROOT / "bin/Client/Data/Local/item.bmd"
    if not sample.is_file():
        raise FileNotFoundError(sample)
    return (IO_PREAMBLE.split("BYTE* reservation;")[0] + EXTRA + declarations + TEXT
            + "void InitializeShadow() {" + allocation + "}\n"
            + function(scene, "BuxConvert_0") + function(data, "Item_LoadBMD")
            + function(data, "Item_SaveBMD") + MAIN.replace("SAMPLE_PATH", json.dumps(str(sample))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-shadow-") as temp:
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
