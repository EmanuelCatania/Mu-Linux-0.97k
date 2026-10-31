#!/usr/bin/env python3
"""Prueba SetAction real con objetos/modelos bajos y sobre 4 GiB.

El doble OBJECT conserva sólo los campos usados por la función en sus offsets
x86. No representa el layout nativo x64 ni prueba carga/render de modelos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SUPPORT = r'''
#ifndef _WIN32
using WORD = std::uint16_t;
#endif
struct OBJECT {
    BYTE prefix[2];
    TYPE_FIELD
    BYTE middle[0x105 - 4];
    CURRENT_FIELD
    PRIOR_FIELD
    BYTE padding;
    FRAME_FIELD
    PRIOR_FRAME_FIELD
    BYTE suffix[0x1bc - 0x110];
};
static_assert(sizeof(void*) != 4 || (sizeof(OBJECT) == 444));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, Type) == 2));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, CurrentAction) == 261));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, PriorAction) == 262));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, AnimationFrame) == 264));
static_assert(sizeof(void*) != 4 || (offsetof(OBJECT, PriorAnimationFrame) == 268));
OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
OBJECT* AsObject(std::uintptr_t p) { return reinterpret_cast<OBJECT*>(p); }
std::uint64_t digest;
void Hash(const void* data, std::size_t size) {
    const auto* bytes = static_cast<const BYTE*>(data);
    while(size--) { digest ^= *bytes++; digest *= 1099511628211ull; }
}
'''

MAIN = r'''
void* Call(OBJECT* object, int action) { return CALL_ACTION; }
void Cast(OBJECT* object) { CALL_CAST; }
std::uint64_t RunCasting(BYTE* models, OBJECT* object) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    digest = 14695981039346656037ull;
    for(short type : {0, 1, 302, 390})
    for(short limit : {0, 5, 6, 87, 88})
    for(BYTE current : {0, 5, 87, 255})
    for(DWORD frame : {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u}) {
        std::memset(models, 0xa5, 391 * 188);
        std::memcpy(models + type * 188 + 38, &limit, 2);
        std::memset(object, 0x5a, sizeof(*object));
        object->Type = type; object->CurrentAction = current;
        std::memcpy(&object->AnimationFrame, &frame, 4);
        OBJECT expected; std::memcpy(&expected, object, sizeof(expected));
        const BYTE action = type == 390 ? 87 : 5;
        if(action < limit && current != action) {
            expected.PriorAction = current;
            std::memcpy(&expected.PriorAnimationFrame, &frame, 4);
            expected.CurrentAction = action; expected.AnimationFrame = 0.0f;
        }
        Cast(object);
        Require(std::memcmp(object, &expected, sizeof(expected)) == 0);
        Hash(object, sizeof(*object));
    }
    return digest;
}
void RunAll(BYTE* models, BYTE* pool) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    CharactersClient = reinterpret_cast<std::uintptr_t>(pool);
    const short limit = 150;
    std::memcpy(models + 390 * 188 + 38, &limit, 2);
    for(int action : {76, 77, 99, 170}) {
        std::memset(pool, 0x5a, 400 * 916);
        for(int slot = 0; slot < 400; ++slot) {
            auto* object = reinterpret_cast<OBJECT*>(pool + slot * 916);
            pool[slot * 916] = BYTE(slot % 2);
            object->Type = slot % 3 ? 390 : 99;
            object->CurrentAction = 7; object->AnimationFrame = 1.25f;
        }
        std::vector<BYTE> expected(pool, pool + 400 * 916);
        for(int slot = 0; slot < 400; ++slot) {
            if(slot % 2 == 0 || slot % 3 == 0) continue;
            const DWORD angles[] = {0, 0, 1127481344};
            std::memcpy(expected.data() + slot * 916 + 28, angles, sizeof(angles));
            if(action < limit || action == 76 || action == 77) {
                auto* object = reinterpret_cast<OBJECT*>(expected.data() + slot * 916);
                object->PriorAction = 7; object->PriorAnimationFrame = 1.25f;
                object->CurrentAction = BYTE(action); object->AnimationFrame = 0.0f;
            }
        }
        Characters_SetActionAll(action);
        Require(std::memcmp(pool, expected.data(), expected.size()) == 0);
    }
}
std::uint64_t Run(BYTE* models, OBJECT* object) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    digest = 14695981039346656037ull;
    for(short type = 0; type < 8; ++type)
    for(short limit : {-1, 0, 1, 76, 77, 255, 32767})
    for(int action : {-1, 0, 1, 75, 76, 77, 78, 254, 255, 256, 32767, 32768})
    for(BYTE current : {0, 1, 76, 77, 255})
    for(DWORD frame : {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u}) {
        std::memset(models, 0xa5, 8 * 188);
        std::memcpy(models + type * 188 + 38, &limit, 2);
        std::memset(object, 0x5a, sizeof(*object));
        object->Type = type; object->CurrentAction = current;
        std::memcpy(&object->AnimationFrame, &frame, 4);
        OBJECT expected; std::memcpy(&expected, object, sizeof(expected));
        const bool accepted = action < limit || action == 76 || action == 77;
        if(accepted && current != action) {
            expected.PriorAction = current;
            std::memcpy(&expected.PriorAnimationFrame, &frame, 4);
            expected.CurrentAction = BYTE(action); expected.AnimationFrame = 0.0f;
        }
        const auto* result = Call(object, action);
        Require(result == (accepted ? object : nullptr));
        Require(std::memcmp(object, &expected, sizeof(expected)) == 0);
        for(int i = 0; i < 8 * 188; ++i) {
            BYTE value = 0xa5;
            if(i == type * 188 + 38 || i == type * 188 + 39)
                value = reinterpret_cast<const BYTE*>(&limit)[i - type * 188 - 38];
            Require(models[i] == value);
        }
        Hash(object, sizeof(*object)); Hash(&accepted, sizeof(accepted));
    }
    Require(Call(nullptr, 76) == nullptr);
    DAT_05828d58 = 0;
    OBJECT unchanged; std::memcpy(&unchanged, object, sizeof(unchanged));
    Require(Call(object, 76) == nullptr && std::memcmp(object, &unchanged, sizeof(unchanged)) == 0);
    return digest;
}
int main() {
    auto* models = static_cast<BYTE*>(Reserve(0x70000000u));
    auto* object = static_cast<OBJECT*>(Reserve(0x71000000u)); Require(models && object);
    auto* pool = static_cast<BYTE*>(Reserve(0x72000000u)); Require(pool != nullptr);
    RunAll(models, pool);
    const auto expected = Run(models, object);
    const auto casting = RunCasting(models, object);
    if(sizeof(void*) > 4) {
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x170000000ull)));
        auto* highObject = static_cast<OBJECT*>(Reserve(std::uint64_t(0x171000000ull)));
        Require(highModels && highObject);
        auto* highPool = static_cast<BYTE*>(Reserve(std::uint64_t(0x172000000ull))); Require(highPool != nullptr);
        RunAll(highModels, highPool);
        // Evitar que una dirección truncada lea o escriba una copia equivalente.
        std::memset(models, 0xcc, 8 * 188);
        Require(Run(highModels, object) == expected);
        Require(Run(models, highObject) == expected);
        Require(Run(highModels, highObject) == expected);
        Require(RunCasting(highModels, object) == casting);
        Require(RunCasting(models, highObject) == casting);
        Require(RunCasting(highModels, highObject) == casting);
        std::puts("objeto/modelos sobre 4 GiB: cuatro combinaciones equivalentes");
    }
    std::printf("SetAction: 13440 casos, bytes/retorno %016llx\n", static_cast<unsigned long long>(expected));
    std::puts("Characters_SetActionAll: 400 slots, activos/tipos y acciones válidas/inválidas correctos");
    std::printf("SetPlayerTeleport: 320 casos; bytes %016llx\n", static_cast<unsigned long long>(casting));
}
'''


def harness(revision=None):
    header = read_source("src/globals.h", revision)
    globals_source = read_source("src/globals.cpp", revision)
    declaration = re.search(r"extern [^;\n]*\bDAT_05828d58\s*;", header).group()
    definition = re.search(r"^[^/\n]*\bDAT_05828d58\s*=\s*[^;]+;", globals_source, re.M).group()
    declaration += re.search(r"extern [^;\n]*\bCharactersClient\s*;", header).group()
    definition += re.search(r"^[^/\n]*\bCharactersClient\s*=\s*[^;]+;", globals_source, re.M).group()
    all_actions = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "Characters_SetActionAll")
    types = read_source("src/Entity/EntityTypes.h", revision)
    support = SUPPORT
    for token, name, kind in (("TYPE_FIELD", "Type", "short"),
                              ("CURRENT_FIELD", "CurrentAction", "BYTE"),
                              ("PRIOR_FIELD", "PriorAction", "BYTE"),
                              ("FRAME_FIELD", "AnimationFrame", "float"),
                              ("PRIOR_FRAME_FIELD", "PriorAnimationFrame", "float")):
        support = re.sub(rf"\b{token}\b", re.search(rf"\b{kind}\s+{name}\s*;", types).group(), support)
    action = function(read_source("src/Entity/Entity_ActionLegacy.cpp", revision), "SetAction")
    casting = function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "SetPlayerTeleport")
    call = "SetAction(object, action)" if "OBJECT* entity_ptr" in action else "SetAction(static_cast<int>(reinterpret_cast<std::uintptr_t>(object)), action)"
    cast_arg = "object" if "OBJECT* param_1" in casting else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return (PREAMBLE.split("BYTE* reservation;")[0].replace("0x10000", "0x100000") + declaration + definition
            + support + action + all_actions + casting
            + MAIN.replace("CALL_ACTION", call).replace("CALL_CAST", f"SetPlayerTeleport({cast_arg})"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-action-pointer-") as temp:
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
