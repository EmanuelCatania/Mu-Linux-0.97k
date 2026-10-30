#!/usr/bin/env python3
"""Prueba SetCharacterScale real con la dirección del personaje sobre 4 GiB.

El doble CHARACTER conserva sólo los campos usados en offsets x86. PART_t
se extrae completo de producción. No certifica un CHARACTER nativo x64.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
from LegacyEntityLayout import opaque_cloth_fields

SUPPORT = r'''
struct OBJECT {
    BYTE prefix[12];
    SCALE_FIELD
    BYTE suffix[444 - 16];
};
PART_RECORD
struct CHARACTER {
    OBJECT Object;
    CLASS_FIELD
    SKIN_FIELD
    BYTE padding[0x1e0 - 0x1be];
    BODY_FIELD
    BYTE middle[0x34f - 0x270];
    CHANGE_FIELD
    BYTE suffix[916 - 0x350];
};
static_assert(sizeof(PART_t) == 24);
static_assert(sizeof(CHARACTER) == 916);
static_assert(offsetof(OBJECT, Scale) == 12);
static_assert(offsetof(CHARACTER, Class) == 444);
static_assert(offsetof(CHARACTER, Skin) == 445);
static_assert(offsetof(CHARACTER, BodyPart) == 480);
static_assert(offsetof(CHARACTER, Change) == 847);
CHARACTER* AsCharacter(void* p) { return static_cast<CHARACTER*>(p); }
CHARACTER* AsCharacter(std::uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }
std::uint64_t digest;
void Hash(const void* data, std::size_t size) {
    const auto* bytes = static_cast<const BYTE*>(data);
    while(size--) { digest ^= *bytes++; digest *= 1099511628211ull; }
}
'''

MAIN = r'''
void Call(CHARACTER* character) { CALL_SCALE; }
std::uint64_t Run(CHARACTER* character) {
    digest = 14695981039346656037ull;
    for(int classId = 0; classId < 256; ++classId)
    for(BYTE skin : {0, 1, 128, 255})
    for(BYTE change : {0, 1, 128, 255})
    for(short armor : {-32768, -1, 623, 624, 625, 626, 627, 633, 634, 635, 636, 637, 638, 32767})
    for(DWORD scale : {0u, 0x80000000u, 0x3f800000u, 0x7fc00001u}) {
        std::memset(character, 0x5a, sizeof(*character));
        character->Class = BYTE(classId); character->Skin = skin; character->Change = change;
        character->BodyPart[1].Type = armor;
        std::memcpy(&character->Object.Scale, &scale, 4);
        CHARACTER expected; std::memcpy(&expected, character, sizeof(expected));
        if(!change) {
            // Valores y límites de IDA SetCharacterScale 0x0045C050.
            const bool head = armor == 624 || armor == 626 || (armor >= 634 && armor <= 637);
            expected.BodyPart[0].Type = head ? short((classId & 7) + 4 * (classId >> 3) + 912) : short(-1);
            DWORD bits = scale;
            if((classId & 7) <= 1) bits = skin ? 0x3f6e147b : 0x3f666666;
            else if((classId & 7) == 2) bits = skin ? 0x3f5c28f6 : 0x3f6147ae;
            else if((classId & 7) == 3) bits = 0x3f733333;
            std::memcpy(&expected.Object.Scale, &bits, 4);
        }
        Call(character);
        Require(std::memcmp(character, &expected, sizeof(expected)) == 0);
        Hash(character, sizeof(*character));
    }
    return digest;
}
int main() {
    auto* character = static_cast<CHARACTER*>(Reserve(0x73000000u)); Require(character != nullptr);
    const auto expected = Run(character);
    if(sizeof(void*) > 4) {
        auto* high = static_cast<CHARACTER*>(Reserve(std::uint64_t(0x173000000ull))); Require(high != nullptr);
        Require(Run(high) == expected);
        std::puts("personaje sobre 4 GiB: mismos bytes de escala y partes");
    }
    std::printf("SetCharacterScale: 229376 casos; bytes %016llx\n", static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    types = read_source("src/Entity/EntityTypes.h", revision)
    support = SUPPORT.replace("PART_RECORD", opaque_cloth_fields(re.search(r"struct PART_t\s*\{.*?\};", types, re.S).group()))
    for token, name, kind in (("SCALE_FIELD", "Scale", "float"),
                              ("CLASS_FIELD", "Class", "BYTE"),
                              ("SKIN_FIELD", "Skin", "BYTE"),
                              ("CHANGE_FIELD", "Change", "BYTE")):
        support = re.sub(rf"\b{token}\b", re.search(rf"\b{kind}\s+{name}\s*;", types).group(), support)
    support = support.replace("BODY_FIELD", re.search(r"PART_t\s+BodyPart\[6\]\s*;", types).group())
    scale = function(read_source("src/Render/SMD_Parser.cpp", revision), "SetCharacterScale")
    call = "SetCharacterScale(character)" if "CHARACTER* param_1" in scale else "SetCharacterScale(static_cast<int>(reinterpret_cast<std::uintptr_t>(character)))"
    return PREAMBLE.split("BYTE* reservation;")[0] + support + scale + MAIN.replace("CALL_SCALE", call)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-scale-pointer-") as temp:
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
