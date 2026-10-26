#!/usr/bin/env python3
"""Preparación/tick reales de materiales con identidad de objeto nativa.

Resets y scopes reales, RNG/mapa controlados; OBJECT escalar de offsets x86.
No certifica render, pool de efectos ni layout nativo completo OBJECT/BMD.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemResetPointer import harness as reset_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int worldValue;
struct Map { int GetCurrentMap() { return worldValue; } } gMapManager;
int randomValues[16], randomIndex=0;
int TestRand() { Require(randomIndex<16); return randomValues[randomIndex++]; }
#define rand TestRand
'''

MAIN = r'''
void Prepare(OBJECT* object,int model) { MaterialVisuals_PrepareWeaponObject(PREPAREARG,model); }
void Tick(const OBJECT* object) { MaterialVisuals_TickObjectDraw(TICKARG); }
void Hash(std::uint64_t& digest,const void* p,std::size_t size) {
    const auto* bytes=static_cast<const BYTE*>(p); for(std::size_t i=0;i<size;++i) { digest^=bytes[i]; digest*=1099511628211ull; }
}
void HashSample(std::uint64_t& digest,const MaterialVisualSample& value) {
    Hash(digest,&value.model,sizeof(value.model)); Hash(digest,value.attribute,sizeof(value.attribute));
    Hash(digest,&value.u,4); Hash(digest,&value.v,4); Hash(digest,&value.alpha,4); Hash(digest,&value.action,1);
}
std::uint64_t RunMaterials(BYTE* memory) {
    OBJECT* object=reinterpret_cast<OBJECT*>(memory+16); std::uint64_t digest=14695981039346656037ull;
    const short types[]={239,244,0,23,244};
    const int models[]={414,565,567,606,607,608,413,564,566,605,0,-1,65535};
    const DWORD alphas[]={0u,0x80000000u,0x3f000000u,0x3f800000u,0x7fc00001u,0xbf800000u};
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,0x5a,sizeof(OBJECT)+32); objects.clear();
        object->Type=types[trial%5]; object->CurrentAction=static_cast<BYTE>(trial);
        const DWORD alpha=alphas[(trial/5)%6], gravity=trial%3==0?0x7fc00001u:trial%3==1?0x80000000u:0x3f800000u;
        std::memcpy(&object->Alpha,&alpha,4); std::memcpy(&object->Gravity,&gravity,4);
        const int model=models[(trial/30)%13]; const short beforeType=object->Type; const BYTE beforeAction=object->CurrentAction;
        float beforeAlpha; std::memcpy(&beforeAlpha,&alpha,4);
        for(int i=0;i<16;++i) randomValues[i]=static_cast<int>((trial*40503u+i*73u)&32767);
        randomIndex=0; draws=0; MaterialVisualSample outer; outer.model=999; current=&outer;
        Prepare(object,model);
        Require(current==&outer && objects.size()==1 && draws==0 && object->Type==beforeType);
        const auto found=objects.find(object); Require(found!=objects.end()); const MaterialVisualSample& actual=found->second.sample;
        MaterialVisualSample expected; expected.model=model; expected.alpha=beforeAlpha; expected.action=beforeAction;
        int expectedDraws=0;
        if(model==414 || model==567) expected.attribute[0]=randomValues[expectedDraws++];
        else if(model==565 || model==606) { expected.attribute[0]=randomValues[expectedDraws++]; expected.attribute[1]=randomValues[expectedDraws++]; }
        if(beforeAlpha>_DAT_005524f8 && (model==607 || model==608)) {
            expected.v=static_cast<float>(randomValues[expectedDraws++]%10)*_DAT_005524f4;
            expected.u=static_cast<float>(randomValues[expectedDraws++]%10)*_DAT_005524f4;
        }
        Require(randomIndex==expectedDraws && actual.model==expected.model && std::memcmp(actual.attribute,expected.attribute,sizeof(expected.attribute))==0);
        Require(std::memcmp(&actual.u,&expected.u,4)==0 && std::memcmp(&actual.v,&expected.v,4)==0 && std::memcmp(&actual.alpha,&alpha,4)==0 && actual.action==beforeAction);
        const DWORD expectedGravity=beforeType==244?gravity:0; Require(std::memcmp(&object->Gravity,&expectedGravity,4)==0);
        Hash(digest,object,sizeof(*object)); HashSample(digest,actual); Hash(digest,&randomIndex,sizeof(randomIndex));
        // El tick no cambia el objeto ni la muestra: sólo consume RNG en su gate.
        const short tickTypes[]={23,19,20,3,4,0,239}; object->Type=tickTypes[trial%7];
        std::memcpy(&object->Alpha,&alpha,4); object->RenderType=trial%3==0?0x80:0;
        object->HiddenMesh=trial%5==0?-2:trial%5==1?-1:0;
        worldValue=trial%2==0?4:0; DAT_07e11d30=trial%4==0?0:1;
        OBJECT before; std::memcpy(&before,object,sizeof(before)); MaterialVisualSample sampleBefore=actual;
        randomIndex=0;
        const bool expectedTick=worldValue==4 && !(object->Alpha<_DAT_005524f8) && object->RenderType!=0x80 && !(DAT_07e11d30==0 && object->HiddenMesh==-2) && object->Type!=0 && object->Type!=239;
        Tick(object); Require(randomIndex==(expectedTick?1:0) && std::memcmp(object,&before,sizeof(before))==0);
        Require(std::memcmp(&actual,&sampleBefore,sizeof(actual))==0 && current==&outer);
        for(int i=0;i<16;++i) Require(memory[i]==0x5a && memory[16+sizeof(OBJECT)+i]==0x5a);
        Hash(digest,&randomIndex,sizeof(randomIndex)); current=nullptr;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunMaterials(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,sizeof(OBJECT)+32); Require(RunMaterials(high)==expected);
        for(std::size_t i=0;i<sizeof(OBJECT)+32;++i) Require(low[i]==0);
        // Dos direcciones con los mismos bits bajos conservan muestras distintas.
        auto* a=reinterpret_cast<OBJECT*>(low+16); auto* b=reinterpret_cast<OBJECT*>(high+16);
        std::memset(a,0,sizeof(*a)); std::memset(b,0,sizeof(*b)); a->Type=b->Type=239; a->Alpha=b->Alpha=1;
        randomIndex=draws=0; objects.clear(); current=nullptr; Prepare(a,565); Prepare(b,606);
        Require(objects.size()==2 && objects.at(a).sample.model==565 && objects.at(b).sample.model==606 && randomIndex==4 && draws==0);
        std::puts("objeto sobre 4 GiB, alias intacto y dos claves con iguales 32 bits bajos");
    }
    std::printf("Materiales de objetos: 32768 estados, scopes/RNG/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=reset_harness(revision).split("int ItemObjectAttributeImpl(",1)[0]
    fake="const MaterialVisualSample* MaterialVisuals_Current() { return currentSample; }"
    if fake not in prefix:
        raise ValueError("No se encontró el getter controlado del fixture")
    prefix=prefix.replace(fake,"")+"\n#include <unordered_map>\n"
    header=read_source("src/Render/MaterialVisuals.h",revision)
    source=read_source("src/Render/MaterialVisuals.cpp",revision)
    prefix+=re.search(r"class MaterialVisualScope\s*\{.*?\n\};",header,re.S).group()
    prefix+=re.search(r"struct ObjectSample\s*\{.*?\};",source,re.S).group()
    prefix+=re.search(r"^const MaterialVisualSample \*current[^;]+;",source,re.M).group()
    prefix+=re.search(r"^std::unordered_map<const void \*, ObjectSample> objects;",source,re.M).group()
    prefix+=re.search(r"^const MaterialVisualSample \*MaterialVisuals_Current\([^\n]+",source,re.M).group()
    prefix+="\n".join(re.findall(r"^MaterialVisualScope::[^\n]+",source,re.M))
    globals_header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("_DAT_005524f8","DAT_07e11d30"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",globals_header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    prefix+=SERVICES
    reset=read_source("src/Entity/Entity_Reset.cpp",revision)
    for name in ("ItemObjectAttributeImpl","ItemObjectAttributeForRender"):
        prefix+=function(reset,name)
    prepare=function(source,"MaterialVisuals_PrepareWeaponObject"); tick=function(source,"MaterialVisuals_TickObjectDraw")
    prefix+=function(source,"MaterialVisuals_Prepare")+prepare+tick
    legacy="static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix+MAIN.replace("PREPAREARG","object" if "OBJECT* entity" in prepare else legacy).replace("TICKARG","object" if "const OBJECT* entity" in tick else legacy)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-material-object-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
