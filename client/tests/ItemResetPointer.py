#!/usr/bin/env python3
"""Resets reales de materiales con puntero nativo y fixture de offsets x86.

Owner/BoneMatrices son opacos uint32_t; no certifica layout OBJECT x64,
render, catálogo de materiales ni igualdad de las funciones CRT entre SO.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
from LegacyEntityLayout import opaque_cloth_fields

SERVICES = r'''
using float10=long double;
using undefined4=std::uint32_t;
using longlong=std::int64_t;
using ulonglong=std::uint64_t;
using uint=unsigned;
double fsin(double value) { return std::sin(value); }
OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
OBJECT* AsObject(int p) { return reinterpret_cast<OBJECT*>(static_cast<std::uintptr_t>(static_cast<std::uint32_t>(p))); }
int draws=0;
int _rand() { Require(draws<2); return draws++ == 0 ? 12345 : 23456; }
MaterialVisualSample sample;
const MaterialVisualSample* currentSample=nullptr;
const MaterialVisualSample* MaterialVisuals_Current() { return currentSample; }
'''

MAIN = r'''
int Call(OBJECT* object, bool render) { return render ? ItemObjectAttributeForRender(ARG) : ItemObjectAttribute(ARG); }
std::uint64_t Run(BYTE* memory) {
    std::uint64_t digest=14695981039346656037ull;
    auto* object=reinterpret_cast<OBJECT*>(memory+16);
    for(unsigned type=0;type<65536;++type)
    for(int mode=0;mode<4;++mode)
    for(float time : {-100000.0f, 0.0f, 0.125f, 100000.0f}) {
        std::memset(memory,0x5a,sizeof(OBJECT)+32); std::memcpy(&object->Type,&type,2);
        WorldTime=time; draws=0;
        sample.model=static_cast<short>(type)+(mode==3?1:0);
        sample.attribute[0]=12345; sample.attribute[1]=23456;
        currentSample=mode>=2?&sample:nullptr;
        OBJECT before; std::memcpy(&before,object,sizeof(before));
        const int result=Call(object,mode!=0);
        const int count=object->Type==414 || object->Type==567 ? 1 : object->Type==565 || object->Type==606 ? 2 : 0;
        Require(draws==(mode==0?count:0));
        // Sólo estos campos y el slot legacy +120 pueden modificarse.
        OBJECT allowed; std::memcpy(&allowed,&before,sizeof(allowed));
#define ALLOW(field) std::memcpy(&allowed.field,&object->field,sizeof(object->field))
        ALLOW(BlendMesh); ALLOW(HiddenMesh); ALLOW(Light); ALLOW(LightEnable);
        ALLOW(AlphaEnable); ALLOW(EnableShadow); ALLOW(Velocity); ALLOW(CollisionRange);
        ALLOW(PriorAnimationFrame); ALLOW(AnimationFrame); ALLOW(PriorAction); ALLOW(CurrentAction);
        ALLOW(Gravity); ALLOW(Alpha); ALLOW(BlendMeshLight); ALLOW(BlendMeshTexCoordU);
        ALLOW(BlendMeshTexCoordV); ALLOW(Scale);
#undef ALLOW
        std::memcpy(reinterpret_cast<BYTE*>(&allowed)+120,reinterpret_cast<BYTE*>(object)+120,4);
        Require(std::memcmp(&allowed,object,sizeof(allowed))==0);
        Require(object->LightEnable==1 && object->AlphaEnable==0 && object->EnableShadow==0);
        Require(object->Alpha==1.0f && object->Velocity==0.0f && object->CollisionRange==50.0f);
        Require(object->Gravity==0.0f && object->AnimationFrame==0.0f && object->PriorAnimationFrame==0.0f);
        Require(object->CurrentAction==0 && object->PriorAction==0);
        if(object->Type<0) Require(object->Scale==0.8f && object->BlendMesh==-1 && object->HiddenMesh==-1 && result==0);
        const int first=mode==0 || mode==2 ? 12345 : 0, second=mode==0 || mode==2 ? 23456 : 0;
        if(object->Type==414) Require(object->BlendMeshTexCoordU==static_cast<float>(first%10)*_DAT_005524f4 && result==first/10);
        if(object->Type==565 || object->Type==606) {
            Require(object->BlendMeshTexCoordU==static_cast<float>(first%10)*_DAT_005524f4);
            Require(object->BlendMeshTexCoordV==static_cast<float>(second%10)*_DAT_005524f4 && result==second/10);
        }
        for(int i=0;i<16;++i) Require(memory[i]==0x5a && memory[16+sizeof(OBJECT)+i]==0x5a);
        for(std::size_t i=0;i<sizeof(OBJECT);++i) { digest^=reinterpret_cast<BYTE*>(object)[i]; digest*=1099511628211ull; }
        for(int value : {result,draws}) for(int shift=0;shift<32;shift+=8) { digest^=static_cast<BYTE>(static_cast<unsigned>(value)>>shift); digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low);
    const auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,sizeof(OBJECT)+32); Require(Run(high)==expected);
        for(std::size_t i=0;i<sizeof(OBJECT)+32;++i) Require(low[i]==0);
        std::puts("objeto sobre 4 GiB y alias bajo intacto");
    }
    std::printf("ItemObjectAttribute: 1048576 estados, bytes/retorno/azar %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0] + "\n#include <cmath>\n"
    types = read_source("src/Entity/EntityTypes.h", revision)
    obj = re.search(r"struct OBJECT\s*\{.*?\};", types, re.S).group()
    obj = re.sub(r"OBJECT\*\s+Owner", "std::uint32_t Owner", obj)
    obj = re.sub(r"vec34_t\*\s+BoneMatrices", "std::uint32_t BoneMatrices", obj)
    # Este banco aísla consumidores legacy: tampoco certifica enlaces nativos.
    obj = re.sub(r"OBJECT\*\s+(ScenePrevious|SceneNext)\b", r"std::uint32_t \1", obj)
    obj = opaque_cloth_fields(obj)
    prefix += "\nusing vec3_t=float[3];\n#pragma pack(push,1)\n" + obj + "\n#pragma pack(pop)\nstatic_assert(sizeof(OBJECT)==444);\n"
    prefix += re.search(r"struct MaterialVisualSample\s*\{.*?\n\};", read_source("src/Render/MaterialVisuals.h", revision), re.S).group()
    prefix += SERVICES
    source = read_source("src/Entity/Entity_Reset.cpp", revision)
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in sorted(set(re.findall(r"_DAT_\w+|\bWorldTime\b", source))):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    for name in ("ItemObjectAttributeImpl", "ItemObjectAttribute", "ItemObjectAttributeForRender"):
        prefix += function(source, name)
    arg = "object" if "ItemObjectAttribute(OBJECT*" in source else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix + MAIN.replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-reset-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia de bytes/eventos con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
