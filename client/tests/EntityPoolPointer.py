#!/usr/bin/env python3
"""Verifica reserva, desplazamiento y búsquedas reales del pool de personajes.

Conserva el layout x86 de bytes. No prueba los campos puntero del CHARACTER
ni sus consumidores de combate/render, que requieren migraciones posteriores.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SUPPORT = r'''
BYTE* reservation;
unsigned selectedOffset;
void* AllocatePool(std::size_t size) {
    Require(size == 0x764d4);
    return reservation + 16;
}
int SelectedOffset() { return selectedOffset; }
'''

MAIN = r'''
void Run(std::uintptr_t address) {
    reservation = static_cast<BYTE*>(Reserve(address)); Require(reservation != nullptr);
    for(selectedOffset = 0; selectedOffset < 128; ++selectedOffset) {
        std::memset(reservation, 0xa5, 0x764d4 + 32);
        InitializePool();
        Require(DAT_055c9e48 == address + 16);
        Require(CharactersClient == address + 16 + selectedOffset * 916);
        auto* base = reinterpret_cast<BYTE*>(CharactersClient);
        // La inicialización existente limpia 401 slots; conservarla exactamente.
        for(std::size_t i = 0; i < 0x764d4 + 32; ++i) {
            const bool cleared = i >= 16 + selectedOffset * 916 &&
                i < 16 + selectedOffset * 916 + 0x166b5 * 4;
            Require(reservation[i] == (cleared ? 0 : 0xa5));
        }
        Require(Entity_FindById(42) == 400 && FindCharacterIndex(42) == 400);
        for(int slot = 0; slot < 400; ++slot) {
            base[slot * 916] = 1;
            const short key = short(slot - 200);
            std::memcpy(base + slot * 916 + 476, &key, sizeof(key));
        }
        for(int slot = 0; slot < 400; ++slot) {
            Require(Entity_FindById(slot - 200) == slot);
            Require(FindCharacterIndex(slot - 200) == slot);
        }
        // Un duplicado inactivo no debe ocultar una coincidencia posterior.
        const short duplicate = 199;
        std::memcpy(base + 476, &duplicate, sizeof(duplicate)); base[0] = 0;
        Require(Entity_FindById(199) == 399 && FindCharacterIndex(199) == 399);
        Require(Entity_FindById(1000) == 400 && FindCharacterIndex(1000) == 400);
        for(int i = 0; i < 16; ++i)
            Require(reservation[i] == 0xa5 && reservation[16 + 0x764d4 + i] == 0xa5);
    }
}
int main() {
    CharactersClient = 0; Require(Entity_FindById(0) == 400);
    Run(0x68000000u);
    if(sizeof(void*) > 4) {
        Run(std::uint64_t(0x168000000ull));
        std::puts("pool sobre 4 GiB: direcciones completas y búsquedas correctas");
    }
    std::puts("128 desplazamientos: inicialización, 400 claves, inactivos, ausentes y canarios correctos");
}
'''


def harness(revision=None):
    header = read_source("src/globals.h", revision)
    globals_source = read_source("src/globals.cpp", revision)
    declarations = ""
    for name in ("DAT_055c9e48", "CharactersClient"):
        declarations += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        declarations += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    # Extraer también el alias de producción: no crear almacenamiento paralelo.
    alias = re.search(r"^#define DAT_07abf5d0[^\n]+", read_source("src/structs.h", revision), re.M).group()
    entry = read_source("src/Platform/ClientEntry.cpp", revision)
    start = re.search(r"^[ \t]*DAT_055c9e48\s*=", entry, re.M).start()
    end = entry.index(";", entry.index("memset(", start)) + 1
    initialize = entry[start:end].replace("malloc(0x764d4)", "AllocatePool(0x764d4)")
    initialize = initialize.replace("rand()", "SelectedOffset()")
    lookup = function(read_source("src/Entity/Entity_Lookup.cpp", revision), "Entity_FindById")
    legacy = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "FindCharacterIndex")
    return (PREAMBLE.split("BYTE* reservation;")[0].replace("0x10000", "0x100000")
            + declarations + "\n" + alias + "\n" + SUPPORT
            + "void InitializePool() {" + initialize + "}\n" + lookup + legacy + MAIN)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-entity-pool-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
