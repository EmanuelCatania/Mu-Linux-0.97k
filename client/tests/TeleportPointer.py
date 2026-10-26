#!/usr/bin/env python3
"""Prueba inicio/fin de teleport reales junto a SetAction real.

El doble OBJECT conserva los campos usados en offsets x86. Efectos y audio
son servicios que validan direcciones, argumentos y orden; no prueban render.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ActionPointer import harness as action_harness
from SceneObjectPointer import build_and_run, function, read_source

OBJECT_RECORD = r'''
using vec3_t = float[3];
struct OBJECT {
    BYTE prefix[2];
    TYPE_FIELD
    BYTE beforePosition[12];
    POSITION_FIELD
    ANGLE_FIELD
    BYTE beforeTeleport[124 - 40];
    TELEPORT_FIELD
    BYTE beforeLight[232 - 125];
    LIGHT_FIELD
    BYTE beforeAction[261 - 244];
    CURRENT_FIELD
    PRIOR_FIELD
    BYTE padding;
    FRAME_FIELD
    PRIOR_FRAME_FIELD
    BYTE beforeAlpha[356 - 272];
    ALPHA_FIELD
    BYTE suffix[444 - 360];
};
static_assert(offsetof(OBJECT, Position) == 16);
static_assert(offsetof(OBJECT, Angle) == 28);
static_assert(offsetof(OBJECT, Light) == 232);
static_assert(offsetof(OBJECT, Teleport) == 124);
static_assert(offsetof(OBJECT, AlphaTarget) == 356);
void* __cdecl SetAction(OBJECT*, int);
'''

SERVICES = r'''
OBJECT* activeObject;
int serviceCalls;
void Event(int expected) {
    Require(++serviceCalls == expected);
    Hash(&expected, sizeof(expected)); Hash(activeObject, sizeof(*activeObject));
}
void SetAttackSpeed() { Event(1); }
void* __cdecl SetAction(OBJECT* object, int action) {
    Require(object == activeObject && action == 87); Event(2);
    return SetActionReal(object, action);
}
float* CreateEffect(int type, float* position, float* angle, float* light,
                    float* p4, float* p5, float* p6, float* p7, BYTE flag) {
    Require(type == 1176 && position == activeObject->Position && angle == activeObject->Angle);
    Require(light == activeObject->Light && !p4 && !p5 && !p7 && flag == 0);
    Require(reinterpret_cast<std::uintptr_t>(p6) == 0xffffffffu);
    Event(3); Hash(position, 12); Hash(angle, 12); Hash(light, 12); return nullptr;
}
void PlayBuffer(int id, std::uintptr_t owner, int flag) {
    Require(id == 88 && owner == 0 && flag == 0); Event(4);
}
'''

MAIN = r'''
void Begin(OBJECT* object) { CALL_BEGIN; }
void End(OBJECT* object) { CALL_END; }
std::uint64_t Run(BYTE* models, OBJECT* object) {
    DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);
    digest = 14695981039346656037ull;
    activeObject = object; serviceCalls = 0;
    Begin(nullptr); End(nullptr); Require(serviceCalls == 0);
    for(short type = 0; type < 8; ++type)
    for(short limit : {0, 76, 87, 88, 200})
    for(BYTE current : {0, 87, 255})
    for(DWORD frame : {0u, 0x80000000u, 0x3fa00000u, 0x7fc00001u})
    for(DWORD alpha : {0u, 0x3f000000u, 0x7fc00001u})
    for(BYTE teleport : {0, 1, 3, 255})
    for(int mode = 0; mode < 3; ++mode) {
        std::memset(models, 0xa5, 8 * 188);
        std::memcpy(models + type * 188 + 38, &limit, 2);
        std::memset(object, 0x5a, sizeof(*object));
        object->Type = type; object->CurrentAction = current; object->Teleport = teleport;
        std::memcpy(&object->AnimationFrame, &frame, 4);
        std::memcpy(&object->AlphaTarget, &alpha, 4);
        for(int i = 0; i < 3; ++i) {
            object->Position[i] = type * 15.25f + i; object->Angle[i] = i * -12.75f;
            object->Light[i] = i * 0.25f;
        }
        const int steps = mode == 2 ? 2 : 1;
        for(int step = 0; step < steps; ++step) {
            const bool end = mode == 1 || step == 1;
            OBJECT expected; std::memcpy(&expected, object, sizeof(expected));
            if(87 < limit && expected.CurrentAction != 87) {
                expected.PriorAction = expected.CurrentAction;
                std::memcpy(&expected.PriorAnimationFrame, &expected.AnimationFrame, 4);
                expected.CurrentAction = 87; expected.AnimationFrame = 0.0f;
            }
            expected.AlphaTarget = end ? 1.0f : 0.0f;
            expected.Teleport = end ? 3 : 1;
            if(end) expected.AnimationFrame = 5.0f;
            serviceCalls = 0;
            if(end) End(object); else Begin(object);
            Require(serviceCalls == 4 && std::memcmp(object, &expected, sizeof(expected)) == 0);
            Hash(object, sizeof(*object));
        }
    }
    return digest;
}
int main() {
    auto* models = static_cast<BYTE*>(Reserve(0x74000000u));
    auto* object = static_cast<OBJECT*>(Reserve(0x75000000u)); Require(models && object);
    const auto expected = Run(models, object);
    if(sizeof(void*) > 4) {
        auto* highModels = static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull)));
        auto* highObject = static_cast<OBJECT*>(Reserve(std::uint64_t(0x175000000ull)));
        Require(highModels && highObject);
        std::memset(models, 0xcc, 8 * 188);
        Require(Run(highModels, object) == expected);
        Require(Run(models, highObject) == expected);
        Require(Run(highModels, highObject) == expected);
        std::puts("teleport con objeto/modelos sobre 4 GiB: cuatro combinaciones equivalentes");
    }
    std::printf("teleport: 17280 casos, bytes/eventos %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = action_harness(revision).split("void* Call(OBJECT* object, int action)", 1)[0]
    types = read_source("src/Entity/EntityTypes.h", revision)
    record = OBJECT_RECORD
    for token, name, kind in (("TYPE_FIELD", "Type", "short"),
                              ("POSITION_FIELD", "Position", "vec3_t"),
                              ("ANGLE_FIELD", "Angle", "vec3_t"),
                              ("LIGHT_FIELD", "Light", "vec3_t"),
                              ("TELEPORT_FIELD", "Teleport", "BYTE"),
                              ("CURRENT_FIELD", "CurrentAction", "BYTE"),
                              ("PRIOR_FIELD", "PriorAction", "BYTE"),
                              ("FRAME_FIELD", "AnimationFrame", "float"),
                              ("PRIOR_FRAME_FIELD", "PriorAnimationFrame", "float"),
                              ("ALPHA_FIELD", "AlphaTarget", "float")):
        record = re.sub(rf"\b{token}\b", re.search(rf"\b{kind}\s+{name}\s*;", types).group(), record)
    prefix = re.sub(r"struct OBJECT\s*\{.*?\};", lambda _: record, prefix, count=1, flags=re.S)
    prefix = prefix.replace("void* __cdecl SetAction(OBJECT* entity_ptr", "void* __cdecl SetActionReal(OBJECT* entity_ptr")
    source = read_source("src/Entity/Entity_LegacyTeleport.cpp", revision)
    begin, end = (function(source, name) for name in ("CreateTeleportBegin", "CreateTeleportEnd"))
    main = MAIN
    for token, name, body in (("CALL_BEGIN", "CreateTeleportBegin", begin), ("CALL_END", "CreateTeleportEnd", end)):
        arg = "object" if "OBJECT* o" in body else "static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(object))"
        main = main.replace(token, f"{name}({arg})")
    return prefix + SERVICES + begin + end + main


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia unsigned int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-teleport-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes/eventos con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
