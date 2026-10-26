#!/usr/bin/env python3
"""Reproducción real de efectos: identidad/generación/interpolación nativas.

Registro, BeginStep, AddSprite y RenderBlend reales; continuidad/alpha/mundo
y CreateSprite controlados. Tick productor, pool/render y owners nativos no
certificados; OBJECT fixture escalar x86 y owner de Sprite sigue siendo int.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemResetPointer import harness as reset_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int SceneFlag, mapWorld;
struct Map { int GetCurrentMap() { return mapWorld; } } gMapManager;
const void* activeObject;
bool continuousFlag;
float alphaValue;
int ownerCalls, alphaCalls;
std::vector<Sprite> observed;
bool RenderInterpolation_OwnerContinuous(const void* object) { Require(object==activeObject); ++ownerCalls; return continuousFlag; }
float RenderInterpolation_Alpha() { ++alphaCalls; return alphaValue; }
int CreateSprite(unsigned short type,float* position,float scale,float* light,int owner,float rotation,int mode) {
    Sprite value; value.type=type; value.scale=scale; value.owner=owner; value.rotation=rotation; value.mode=mode;
    std::memcpy(value.position,position,12); std::memcpy(value.light,light,12); observed.push_back(value); return 0;
}
'''

MAIN = r'''
void Draw(const OBJECT* object) { WorldEffects_Draw(ARG); }
void Hash(std::uint64_t& digest,const void* p,std::size_t size) {
    const auto* bytes=static_cast<const BYTE*>(p); for(std::size_t i=0;i<size;++i) { digest^=bytes[i]; digest*=1099511628211ull; }
}
void HashSprite(std::uint64_t& digest,const Sprite& value) {
    Hash(digest,&value.type,2); Hash(digest,value.position,12); Hash(digest,&value.scale,4); Hash(digest,value.light,12);
    Hash(digest,&value.owner,4); Hash(digest,&value.rotation,4); Hash(digest,&value.mode,4);
}
void Collect(Frame& frame,int count,unsigned trial,bool old) {
    recording=&frame;
    for(int row=0;row<count;++row) {
        unsigned short type=static_cast<unsigned short>(trial*73+row);
        int owner=trial%2?-1:0, mode=static_cast<int>(trial%4);
        if(old && (trial+row)%3==0) ++type;
        if(old && (trial/3+row)%3==0) ++owner;
        if(old && (trial/9+row)%3==0) ++mode;
        float position[3],light[3];
        for(int axis=0;axis<3;++axis) { position[axis]=(trial%2048)*0.5f+row*3.0f+axis-(old?(axis+1)*20.0f:0.0f); light[axis]=old?0.0f:axis*0.25f; }
        float beforePosition[3], beforeLight[3]; std::memcpy(beforePosition,position,12); std::memcpy(beforeLight,light,12);
        Require(WorldEffects_AddSprite(type,position,old?0.5f:1.25f,light,owner,old?45.0f:-0.0f,mode)==0);
        Require(std::memcmp(beforePosition,position,12)==0 && std::memcmp(beforeLight,light,12)==0);
    }
    recording=nullptr;
}
std::uint64_t RunWorld(BYTE* memory) {
    OBJECT* object=reinterpret_cast<OBJECT*>(memory+16); activeObject=object;
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    const float alphas[]={-0.5f,-0.0f,0.25f,1.0f,1.5f};
    const short types[]={0,3,19,23,244,-1,32767,-32768};
    for(unsigned trial=0;trial<32768;++trial) {
        recording=nullptr; frames.clear(); previousFrames.clear(); SceneObjects::LiveNodes().clear(); SceneObjects::Generations().clear();
        Require(WorldEffects_AddSprite(0,nullptr,0,nullptr,0,0,0)==0 && frames.empty());
        std::memset(memory,0x5a,sizeof(OBJECT)+32); object->Live=(trial/7)%3==0?0:1; object->Type=types[trial%8];
        SceneObjects::Register(object); const auto oldGeneration=SceneObjects::Generation(object);
        SceneFlag=static_cast<int>(trial%3)+2; mapWorld=static_cast<int>(trial%18);
        scene=SceneFlag+static_cast<int>((trial/3)%2); world=mapWorld;
        const bool keepPrevious=scene==SceneFlag;
        Frame& previous=frames[object]; previous.generation=oldGeneration+(trial/17)%2; previous.type=object->Type+static_cast<int>((trial/19)%2);
        Collect(previous,static_cast<int>((trial/4)%4),trial,true); const Frame previousCopy=previous;
        WorldEffects_BeginStep(); Require(recording==nullptr && frames.empty() && (previousFrames.find(object)!=previousFrames.end())==keepPrevious);
        if(trial%17==0) { SceneObjects::Unregister(object); SceneObjects::Register(object); }
        const auto generation=SceneObjects::Generation(object);
        Frame& frame=frames[object]; frame.generation=generation+(trial/13)%2; frame.type=object->Type+static_cast<int>((trial/11)%3==0);
        Collect(frame,static_cast<int>(trial%4),trial,false); const Frame currentCopy=frame;
        const bool present=trial%29!=0, registered=trial%31!=0;
        if(!present) frames.erase(object); if(!registered) SceneObjects::Unregister(object);
        if(trial%19==0) ++SceneFlag; if(trial%23==0) ++mapWorld;
        continuousFlag=(trial/5)%2!=0; alphaValue=alphas[(trial/7)%5];
        const bool visible=scene==SceneFlag && world==mapWorld && present && registered && currentCopy.generation==generation && object->Live && currentCopy.type==object->Type;
        const bool continuous=visible && continuousFlag && keepPrevious && previousCopy.generation==currentCopy.generation && previousCopy.type==currentCopy.type && previousCopy.sprites.size()==currentCopy.sprites.size();
        std::vector<Sprite> expected;
        int expectedAlphaCalls=0;
        if(visible) {
            expected=currentCopy.sprites;
            if(continuous) for(std::size_t row=0;row<expected.size();++row) {
                const auto& old=previousCopy.sprites[row]; auto& now=expected[row];
                if(old.type==now.type && old.owner==now.owner && old.mode==now.mode) {
                    expectedAlphaCalls+=3; coverage|=4;
                    for(int axis=0;axis<3;++axis) now.position[axis]=alphaValue<=0?old.position[axis]:alphaValue>=1?now.position[axis]:old.position[axis]+(now.position[axis]-old.position[axis])*alphaValue;
                } else coverage|=8;
            }
        }
        BYTE before[sizeof(OBJECT)+32]; std::memcpy(before,memory,sizeof(before)); observed.clear(); ownerCalls=alphaCalls=0;
        Draw(object);
        Require(std::memcmp(before,memory,sizeof(before))==0 && observed.size()==expected.size());
        Require(ownerCalls==(visible && !currentCopy.sprites.empty()?1:0) && alphaCalls==expectedAlphaCalls);
        for(std::size_t row=0;row<expected.size();++row) {
            const auto& a=observed[row]; const auto& b=expected[row];
            Require(a.type==b.type && a.owner==b.owner && a.mode==b.mode && std::memcmp(a.position,b.position,12)==0 && std::memcmp(a.light,b.light,12)==0 && std::memcmp(&a.scale,&b.scale,4)==0 && std::memcmp(&a.rotation,&b.rotation,4)==0);
            HashSprite(digest,a);
        }
        coverage|=observed.empty()?1:2; Hash(digest,&ownerCalls,sizeof(ownerCalls)); Hash(digest,&alphaCalls,sizeof(alphaCalls));
        observed.clear(); ownerCalls=alphaCalls=0; Draw(reinterpret_cast<const OBJECT*>(std::uintptr_t(0x123)));
        Require(observed.empty() && ownerCalls==0 && alphaCalls==0);
    }
    Require(coverage==15); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunWorld(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,sizeof(OBJECT)+32); Require(RunWorld(high)==expected);
        for(std::size_t i=0;i<sizeof(OBJECT)+32;++i) Require(low[i]==0);
        std::puts("identidad/generacion sobre 4 GiB, alias bajo intacto y puntero no registrado sin lectura");
    }
    std::printf("WorldEffects_Draw: 32768 estados, guardas/interpolacion/sprites %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=reset_harness(revision).split("int ItemObjectAttributeImpl(",1)[0]
    prefix+=read_source("src/Scene/ObjectRegistry.h",revision).replace("#pragma once","")
    source=read_source("src/Render/WorldEffects.cpp",revision)
    for name in ("Sprite","Frame"):
        prefix+=re.search(rf"struct {name}\s*\{{.*?\n\}};",source,re.S).group()
    for pattern in (r"^std::unordered_map<const void \*, Frame> frames, previousFrames;",r"^Frame \*recording[^;]+;",r"^int scene[^;]+;"):
        prefix+=re.search(pattern,source,re.M).group()
    prefix+=SERVICES+function(read_source("src/Game/RenderInterpolationMath.cpp",revision),"RenderBlend")
    for name in ("WorldEffects_BeginStep","WorldEffects_AddSprite","WorldEffects_Draw"):
        prefix+=function(source,name)
    arg="object" if "const OBJECT* entity" in function(source,"WorldEffects_Draw") else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix+MAIN.replace("ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-world-effects-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
