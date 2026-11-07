#!/usr/bin/env python3
"""Verifica los buffers de las tres funciones XOR usando sus cuerpos reales.

Mismas opciones de compilación que SceneObjectPointer.py. Compara vectores,
canarios, ida/vuelta y archivos de muestra; no interpreta los layouts de disco.
La comparación --reference-ref se ejecuta en x86. En x64 exige memoria alta.
"""
import argparse
import json
import os
from pathlib import Path
import tempfile

from SceneObjectPointer import ROOT, build_and_run, function, read_source


PREAMBLE = r'''
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
#define __cdecl
#endif
using byte = unsigned char;
char DAT_00559bb4[3] = {char(0xfc), char(0xcf), char(0xab)};
char bBuxCode[3] = {char(0xfc), char(0xcf), char(0xab)};
BYTE DAT_0055a770[3] = {0, 0, 0};
void* Reserve(std::uintptr_t address) {
#ifdef _WIN32
    return VirtualAlloc(reinterpret_cast<void*>(address), 0x100000,
                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void* p = mmap(reinterpret_cast<void*>(address), 0x100000,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    return p == MAP_FAILED ? nullptr : p;
#endif
}
'''

MAIN = r'''
using Cipher = void (*)(BYTE*, int);
std::uint64_t digest = 14695981039346656037ull;
void Require(bool ok) {
    if(!ok) { std::fputs("Buffer, vector o canario incorrecto\n", stderr); std::exit(1); }
}
void Check(BYTE* memory, Cipher cipher, const BYTE* key, const std::vector<BYTE>& input) {
    const auto n = input.size();
    Require(n + 32 < 0x100000);
    std::memset(memory, 0xa5, n + 32);
    BYTE* buffer = memory + 16;
    std::memcpy(buffer, input.data(), n);
    cipher(buffer, static_cast<int>(n));
    for(std::size_t i = 0; i < n; ++i) {
        Require(buffer[i] == BYTE(input[i] ^ key[i % 3]));
        digest ^= buffer[i]; digest *= 1099511628211ull;
    }
    for(int i = 0; i < 16; ++i) Require(memory[i] == 0xa5 && memory[n + 16 + i] == 0xa5);
    cipher(buffer, static_cast<int>(n));
    Require(std::memcmp(buffer, input.data(), n) == 0);
    // El contrato original no accede a memoria cuando Size <= 0.
    cipher(nullptr, 0); cipher(nullptr, -1);
}
std::uint64_t Run(BYTE* memory, const std::vector<std::vector<BYTE>>& inputs) {
    digest = 14695981039346656037ull;
    for(const auto& input : inputs) {
        Check(memory, Call0, reinterpret_cast<const BYTE*>(DAT_00559bb4), input);
        Check(memory, Call1, reinterpret_cast<const BYTE*>(bBuxCode), input);
        Check(memory, CallTerrain, DAT_0055a770, input);
        // Además de la clave cero del port, ejercitar XOR efectivo en Terrain.
        DAT_0055a770[0] = 0xd4; DAT_0055a770[1] = 0x12; DAT_0055a770[2] = 0xab;
        Check(memory, CallTerrain, DAT_0055a770, input);
        DAT_0055a770[0] = DAT_0055a770[1] = DAT_0055a770[2] = 0;
    }
    return digest;
}
int main() {
    std::vector<std::vector<BYTE>> inputs;
    for(int n : {0, 1, 2, 3, 4, 9, 20, 40, 64, 300, 584, 1024, 2560, 32768, 65539, 300000}) {
        std::vector<BYTE> bytes(n);
        for(int i = 0; i < n; ++i) bytes[i] = BYTE(i * 37 + 11);
        inputs.push_back(bytes);
    }
    const char* paths[] = {SAMPLE_PATHS};
    for(const auto* path : paths) {
        std::ifstream stream(path, std::ios::binary);
        Require(stream.good());
        inputs.emplace_back(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }
    auto* low = static_cast<BYTE*>(Reserve(0x30000000u));
    Require(low != nullptr);
    const auto expected = Run(low, inputs);
    std::printf("muestras=%zu digest=%016llx\n", inputs.size(), static_cast<unsigned long long>(expected));
    if(sizeof(void*) > 4) {
        auto* high = static_cast<BYTE*>(Reserve(std::uint64_t(0x130000000ull)));
        Require(high && reinterpret_cast<std::uintptr_t>(high) > 0xffffffffull);
        Require(Run(high, inputs) == expected);
        std::puts("buffers sobre 4 GiB: vectores, canarios e ida/vuelta correctos");
    }
}
'''


def harness(revision=None):
    scene = read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    code0 = function(scene, "BuxConvert_0")
    code1 = function(scene, "BuxConvert_1")
    terrain = function(read_source("src/Net/Net_Bux.cpp", revision), "BuxConvert")
    call0 = "BuxConvert_0(p, n);"
    call_terrain = "BuxConvert(p, n);"
    if "BuxConvert_0(int buf" in code0:
        call0 = "BuxConvert_0(static_cast<int>(reinterpret_cast<std::uintptr_t>(p)), n);"
    if "BuxConvert(int data" in terrain:
        call_terrain = "BuxConvert(static_cast<int>(reinterpret_cast<std::uintptr_t>(p)), n);"
    wrappers = (
        f"void Call0(BYTE* p, int n) {{ {call0} }}\n"
        "void Call1(BYTE* p, int n) { BuxConvert_1(p, n); }\n"
        f"void CallTerrain(BYTE* p, int n) {{ {call_terrain} }}\n")
    samples = [ROOT / "bin/Client/Data/Local" / name
               for name in ("item.bmd", "skill.bmd", "quest.bmd")]
    for sample in samples:
        if not sample.is_file():
            raise FileNotFoundError(f"Falta archivo de muestra: {sample}")
    paths = ", ".join(json.dumps(str(p)) for p in samples)
    return PREAMBLE + code0 + code1 + terrain + wrappers + MAIN.replace("SAMPLE_PATHS", paths)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia de buffers int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-bux-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError(f"Diferencia con {args.reference_ref}: {reference!r} / {current!r}")
            print(f"Equivalencia de bytes con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
