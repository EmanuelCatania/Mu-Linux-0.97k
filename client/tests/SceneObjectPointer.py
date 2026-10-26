#!/usr/bin/env python3
"""Aísla el tick real de objetos y verifica el transporte de su dirección.

Extrae las funciones de producción, sin copiar su lógica. Los servicios de
render/audio se sustituyen por dobles; no valida el juego completo ni layouts
nativos x64. Las reservas conservan los offsets x86 de esta etapa.

Windows: python tests/SceneObjectPointer.py --architecture Win32
Windows x64: python tests/SceneObjectPointer.py --architecture x64
Linux: python3 tests/SceneObjectPointer.py --generator Ninja
Comparación x86: agregar --reference-ref <commit-base>.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def read_source(path, revision=None):
    if revision:
        return subprocess.check_output(
            ["git", "show", f"{revision}:{path}"], cwd=ROOT).decode("utf-8")
    return (ROOT / path).read_text(encoding="utf-8")


def function(source, name):
    # Los comentarios no deben aportar llaves ni falsas definiciones.
    source = re.sub(r"//[^\n]*|/\*.*?\*/", "", source, flags=re.S)
    found = list(re.finditer(
        rf"(?:void|float\*|BYTE\*)\s+(?:__cdecl\s+)?{name}\([^{{;]*\)\s*\{{", source))
    if len(found) != 1:
        raise ValueError(f"Se esperaba una definición de {name}, hay {len(found)}")
    start = found[0].start()
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


PREAMBLE = r'''
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
using DWORD = std::uint32_t;
using WORD = std::uint16_t;
using BYTE = std::uint8_t;
#define __cdecl
#endif
using undefined4 = std::uint32_t;
struct OBJECT;
OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
OBJECT* AsObject(std::uintptr_t p) { return reinterpret_cast<OBJECT*>(p); }
struct Map { int world; int GetCurrentMap() const { return world; } } gMapManager;
float WorldTime;
int HeroTile, SceneFlag, CameraWalkCut, CurrentCameraCount, DAT_07e11d30;
DWORD DAT_05828d58;
alignas(16) char hero[0x1bc];
char* Hero = hero;
float terrain[3];
float* PrimaryTerrainLight[] = {terrain};
float _DAT_005524f4 = 0.1f, _DAT_00552874 = 0.05f;
float _DAT_00552580 = 0.0f, _DAT_0055256c = 1.0f;
std::uint64_t digest, events;
void Hash(const void* memory, std::size_t size) {
    const auto* p = static_cast<const unsigned char*>(memory);
    while(size--) { digest ^= *p++; digest *= 1099511628211ull; }
}
void Event(unsigned kind) { ++events; Hash(&kind, sizeof(kind)); }
void PlayBuffer(int id, std::uintptr_t, int) { Event(1000 + id); }
void AddTerrainLight(float x, float y, float* light, int range, float*) {
    Event(1); Hash(&x, 4); Hash(&y, 4); Hash(light, 12); Hash(&range, 4);
}
void BMD__PlayAnimation(void*, float* frame, float*, void*, float speed) {
    Event(2); *frame += speed; Hash(&speed, 4);
}
template<class... Args> void CreateEffect(int type, float* pos, float* angle,
                                        float* light, Args...) {
    Event(3); Hash(&type, 4); Hash(pos, 12); Hash(angle, 12); Hash(light, 12);
}
template<class... Args> void Particle_Spawn(int type, float* pos, float* angle,
                                          float* light, Args...) {
    Event(4); Hash(&type, 4); Hash(pos, 12); Hash(angle, 12); Hash(light, 12);
}
float TurnAngle2(float from, float to, float step) {
    Event(5); Hash(&to, 4); Hash(&step, 4); return from;
}
void* Reserve(std::uintptr_t address, std::size_t size) {
#ifdef _WIN32
    return VirtualAlloc(reinterpret_cast<void*>(address), size,
                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void* p = mmap(reinterpret_cast<void*>(address), size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    return p == MAP_FAILED ? nullptr : p;
#endif
}
#define Models DAT_05828d58
#define EditFlag DAT_07e11d30
#define LODWORD(x) (*reinterpret_cast<std::uint32_t*>(&(x)))
'''

MAIN = r'''
template<class T> void Put(char* p, int offset, T value) {
    std::memcpy(p + offset, &value, sizeof(value));
}
std::uint64_t Run(char* object, char* models) {
    digest = 14695981039346656037ull;
    events = 0;
    for(int world = 0; world < 18; ++world)
    for(int type = 0; type <= 200; ++type)
    for(int state = 0; state < 5; ++state) {
        std::memset(object, 0, 0x1bc);
        std::memset(models, 0, 201 * 188);
        std::memset(hero, 0, sizeof(hero));
        gMapManager.world = world;
        HeroTile = state == 0 ? 3 : state == 1 ? 4 : state == 2 ? 10 : 0;
        SceneFlag = state == 0 ? 2 : state == 1 ? 4 : 5;
        CameraWalkCut = state & 1;
        CurrentCameraCount = 37;
        DAT_07e11d30 = state == 3;
        WorldTime = state == 0 ? 512.0f : state == 1 ? 4001.0f : 9999.0f;
        Put(object, 2, static_cast<short>(type));
        Put(object, 16, 100.0f); Put(object, 20, 200.0f);
        Put(object, 36, state == 0 ? 0.0f : state == 1 ? 90.0f : 270.0f);
        Put(object, 48, 90.0f);
        Put(object, 52, 130.0f); Put(object, 56, 205.0f);
        Put(object, 104, 0.8f); Put(object, 128, 6.0f);
        Put(object, 192, 0.05f); Put(object, 196, 0.03f);
        Put(object, 204, 0.1f); Put(object, 261, static_cast<BYTE>(17));
        Put(object, 353, static_cast<BYTE>(state & 1));
        Put(object, 356, state >= 3 ? 0.0f : 1.0f);
        Put(object, 360, state == 4 ? 0.0f : state == 2 ? 0.001f : 0.5f);
        Put(hero, 16, state == 3 ? 1000.0f : 110.0f); Put(hero, 20, 205.0f);
        Put(hero, 261, static_cast<BYTE>(17));
        std::srand(17 + world * 1000 + type * 4 + state);
        CALL_TICK
        Hash(object, 0x1bc);
        Hash(models, 201 * 188);
    }
    Hash(&events, sizeof(events));
    return digest;
}
int main() {
    auto* models = static_cast<char*>(Reserve(0x10000000u, 0x10000));
    auto* low = static_cast<char*>(Reserve(0x20000000u, 0x10000));
    if(!models || !low) { std::fputs("No se pudo reservar memoria baja\n", stderr); return 1; }
    InitializeModels(models);
    const auto lowDigest = Run(low, models);
    std::printf("casos=18090 digest=%016llx\n", static_cast<unsigned long long>(lowDigest));
    if(sizeof(void*) > 4) {
        auto* high = static_cast<char*>(Reserve(std::uint64_t(0x120000000ull), 0x10000));
        if(!high || reinterpret_cast<std::uintptr_t>(high) <= 0xffffffffull) {
            std::fputs("No se pudo reservar un objeto sobre 4 GiB\n", stderr); return 1;
        }
        const auto highDigest = Run(high, models);
        if(highDigest != lowDigest) {
            std::fputs("Diferencia entre objeto bajo y alto\n", stderr); return 1;
        }
        std::puts("objeto sobre 4 GiB: mismos bytes y eventos que el objeto bajo");
        auto* highModels = static_cast<char*>(Reserve(std::uint64_t(0x110000000ull), 0x10000));
        if(!highModels) { std::fputs("No se pudo reservar Models sobre 4 GiB\n", stderr); return 1; }
        InitializeModels(highModels);
        if(Run(low, highModels) != lowDigest || Run(high, highModels) != lowDigest) {
            std::fputs("Diferencia con Models sobre 4 GiB\n", stderr); return 1;
        }
        std::puts("modelos sobre 4 GiB: dos inicializaciones y ambos objetos equivalentes");
    }
}
'''


def harness(revision=None):
    header = read_source("src/globals.h", revision)
    globals_source = read_source("src/globals.cpp", revision)
    declaration = re.search(r"extern [^;\n]*\bDAT_05828d58\s*;", header).group()
    definition = re.search(r"^[^/\n]*\bDAT_05828d58\s*=\s*[^;]+;", globals_source, re.M).group()
    initializers = []
    for path in ("src/Platform/ClientEntry.cpp", "src/Model/Model_Items.cpp"):
        initializers.append(re.search(r"^[ \t]*DAT_05828d58\s*=\s*[^;]+;",
                                      read_source(path, revision), re.M).group())
    validate = ('if(DAT_05828d58 != reinterpret_cast<std::uintptr_t>(models)) '
                '{ std::fputs("Base Models truncada\\n", stderr); std::exit(1); }')
    initialize = ('\nvoid* DAT_06f42a58;\nvoid InitializeModels(void* models) {'
                  'DAT_06f42a58 = models;' + initializers[0] + validate
                  + 'DAT_05828d58 = 0;' + initializers[1] + validate + '}\n')
    preamble = PREAMBLE.replace("DWORD DAT_05828d58;", declaration + definition)
    scene = read_source("src/Scene/Scene_ObjectUpdate.cpp", revision)
    tick = function(scene, "FUN_004fdc00")
    alpha = function(read_source("src/Render/Particle.cpp", revision), "Alpha")
    physics = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision),
                       "Effect_PhysicsTick")
    ambient = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision),
                       "FUN_0046c7f0")
    angles = function(read_source("src/Math/Math_LegacyTransforms.cpp", revision), "AngleMatrix")
    rotate = function(read_source("src/Math/Math_3D.cpp", revision), "Vector_Rotate")
    start = scene.index("struct SceneFlamePreset {")
    end = scene.index("} // namespace", start)
    flames = scene[start:end]
    call = "FUN_004fdc00(reinterpret_cast<OBJECT*>(object));"
    if "float o)" in tick:
        call = ("float bits; const auto address = static_cast<DWORD>("
                "reinterpret_cast<std::uintptr_t>(object)); "
                "std::memcpy(&bits, &address, 4); FUN_004fdc00(bits);")
    return preamble + initialize + alpha + physics + angles + rotate + ambient + flames + tick + MAIN.replace("CALL_TICK", call)


def build_and_run(directory, source, args):
    directory.mkdir()
    (directory / "main.cpp").write_text(source, encoding="utf-8")
    (directory / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.25)\n"
        "project(SceneObjectPointer LANGUAGES CXX)\n"
        "add_executable(pointer_test main.cpp)\n"
        "target_compile_features(pointer_test PRIVATE cxx_std_17)\n"
        "if(MSVC)\n target_compile_options(pointer_test PRIVATE /fp:precise /utf-8)\n"
        "else()\n target_compile_options(pointer_test PRIVATE -fno-strict-aliasing)\n"
        "endif()\n", encoding="utf-8")
    command = ["cmake", "-S", str(directory), "-B", str(directory / "build")]
    if args.generator:
        command += ["-G", args.generator]
    if os.name == "nt":
        command += ["-A", args.architecture, "-T", "v143"]
    subprocess.run(command, check=True)
    subprocess.run(["cmake", "--build", str(directory / "build"), "--config", "Release"],
                   check=True)
    binary = directory / "build" / "pointer_test"
    if os.name == "nt":
        binary = directory / "build" / "Release" / "pointer_test.exe"
    return subprocess.check_output([str(binary)], text=True, cwd=directory)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia float heredada sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-object-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError(f"Diferencia con {args.reference_ref}: {reference!r} / {current!r}")
            print(f"Equivalencia de bytes/eventos con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
