#!/usr/bin/env python3
"""Prueba la reserva y el lector/escritor de gates con su código de producción.

Comprueba la muestra real Gate.bmd, sus 100 registros de nueve bytes, canarios
y salida idéntica. En x64 la tabla debe conservar una dirección sobre 4 GiB.
No cambia ni reconstruye el layout persistente; usa el struct ya existente.
"""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile

from SceneObjectPointer import ROOT, build_and_run, function, read_source

PREAMBLE = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
using BYTE = unsigned char;
using DWORD = std::uint32_t;
using CHAR = char;
#define __cdecl
#endif
void Require(bool ok) {
    if(!ok) { std::fputs("Tabla, archivo o canario incorrecto\n", stderr); std::exit(1); }
}
void* Reserve(std::uintptr_t address) {
#ifdef _WIN32
    return VirtualAlloc(reinterpret_cast<void*>(address), 0x10000,
                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void* p = mmap(reinterpret_cast<void*>(address), 0x10000, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    return p == MAP_FAILED ? nullptr : p;
#endif
}
char DAT_00559bb4[3] = {char(0xfc), char(0xcf), char(0xab)};
const char DAT_005597d4[] = "wb", DAT_005580ac[] = "rb";
const char s__s___File_not_exist__00558094[] = "falta archivo";
int DAT_055c9bf0;
FILE* crt_fopen(const char* p, const char* mode) { auto* f = std::fopen(p, mode); Require(f); return f; }
std::size_t crt_fwrite(void* p, int n, int c, int* f) { return std::fwrite(p, n, c, reinterpret_cast<FILE*>(f)); }
std::size_t crt_fread(void* p, int n, int c, int* f) { return std::fread(p, n, c, reinterpret_cast<FILE*>(f)); }
int crt_fclose(FILE* f) { return std::fclose(f); }
void* operator_new(int n) { auto* p = std::malloc(n); Require(p); return p; }
void operator_delete(void* p) { std::free(p); }
template<class... Args> int crt_sprintf(Args...) { std::abort(); }
template<class... Args> void CErrorReport_Write(Args...) { std::abort(); }
struct Window {
    template<class... Args> void ShowError(Args...) { std::abort(); }
    void RequestCloseApplication() { std::abort(); }
} gWindow;
namespace Platform { Window& GetWindow() { return gWindow; } }
BYTE* reservation;
void* AllocateTable(std::size_t n) { Require(n == 900); return reservation + 16; }
'''

MAIN = r'''
std::vector<BYTE> Read(const char* path) {
    std::ifstream f(path, std::ios::binary);
    Require(f.good());
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}
void Run(std::uintptr_t address, const std::vector<BYTE>& file) {
    reservation = static_cast<BYTE*>(Reserve(address));
    Require(reservation != nullptr);
    std::memset(reservation, 0xa5, 932);
    InitializeTable();
    Require(reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*>(GateAttribute)) == address + 16);
    Gate_LoadBMD(SAMPLE_PATH);
    const auto* bytes = reinterpret_cast<const BYTE*>(GateAttribute);
    for(int i = 0; i < 900; ++i) Require(bytes[i] == BYTE(file[i] ^ BYTE(DAT_00559bb4[i % 9 % 3])));
    Gate_SaveBMD("roundtrip.bmd");
    Require(Read("roundtrip.bmd") == file);
    for(int i = 0; i < 16; ++i) Require(reservation[i] == 0xa5 && reservation[916 + i] == 0xa5);
}
int main() {
    const auto file = Read(SAMPLE_PATH);
    Require(file.size() == 900);
    Run(0x40000000u, file);
    if(sizeof(void*) > 4) {
        Run(std::uint64_t(0x140000000ull), file);
        std::puts("tabla sobre 4 GiB: reserva, registros, canarios e ida/vuelta correctos");
    }
    std::puts("Gate.bmd: 100 registros de nueve bytes, salida identica a la muestra");
}
'''


def harness(revision=None):
    structs = read_source("src/structs.h", revision)
    record = re.search(r"struct GATE_ATTRIBUTE\s*\{.*?\};", structs, re.S).group()
    asserts = (
        'static_assert(sizeof(GATE_ATTRIBUTE) == 9);\n'
        'static_assert(offsetof(GATE_ATTRIBUTE, Target) == 6);\n'
        'static_assert(offsetof(GATE_ATTRIBUTE, RequireLevel) == 8);\n')
    declaration = re.search(r"extern [^;\n]*\bGateAttribute\s*;",
                            read_source("src/globals.h", revision)).group()
    definition = re.search(r"^[^/\n]*\bGateAttribute\s*=\s*[^;]+;",
                           read_source("src/globals.cpp", revision), re.M).group()
    allocation = re.search(r"^[ \t]*GateAttribute\s*=\s*[^;]+;",
                           read_source("src/Platform/ClientEntry.cpp", revision), re.M).group()
    allocation = allocation.replace("malloc(900)", "AllocateTable(900)")
    initialize = "void InitializeTable() { " + allocation + " }\n"
    gate = read_source("src/Item/Gate_Data.cpp", revision)
    cipher = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "BuxConvert_0")
    sample = ROOT / "bin/Client/Data/Gate.bmd"
    if not sample.is_file():
        raise FileNotFoundError(sample)
    return (PREAMBLE + record + asserts + declaration + definition + initialize + cipher
            + function(gate, "Gate_SaveBMD") + function(gate, "Gate_LoadBMD")
            + MAIN.replace("SAMPLE_PATH", json.dumps(str(sample))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-gate-pointer-") as temp:
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
