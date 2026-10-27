#!/usr/bin/env python3
"""Sprite y dos recorridos reales del pool: direcciones nativas y bytes x86.

Backend gráfico/antilag controlados. Registros444 y texturas56 opacos;
no certifica layout nativo, owners, replay SIM, raster ni texturas reales.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <algorithm>
struct OBJECT;
char* spritePool; char* g_BitmapsRaw;
#define DAT_07c85890 (*reinterpret_cast<char(*)[1002*444]>(spritePool))
bool blocked; int settingsCalls;
struct Settings { bool GetAntilag(int id) { Require(id==ANTILAG_STATIC_EFFECTS); ++settingsCalls; return blocked; } } gUserSettings;
struct Event { int kind,type; std::uint32_t width,height,rotation; std::vector<BYTE> data; };
std::vector<Event> events;
std::uint32_t Bits(float value) { std::uint32_t bits; std::memcpy(&bits,&value,4); return bits; }
void GL_SetBlendAdditive() { events.push_back({0,0,0,0,0,{}}); }
void GL_SetBlendSrcAlpha() { events.push_back({1,0,0,0,0,{}}); }
void GL_SetBlendSrcOver(char mode) { Require(mode==1); events.push_back({2,0,0,0,0,{}}); }
void RenderSprite_0(int type,float* pos,float width,float height,float* light,float rotation,float u,float v,float uw,float vh) {
    const auto offset=reinterpret_cast<std::uintptr_t>(pos)-reinterpret_cast<std::uintptr_t>(spritePool);
    Require(offset>=16 && offset<1002*444 && (offset-16)%444==0 && reinterpret_cast<BYTE*>(light)==reinterpret_cast<BYTE*>(pos)+216);
    Require(u==0.0f && v==0.0f && uw==1.0f && vh==1.0f);
    std::vector<BYTE> data(24); std::memcpy(data.data(),pos,12); std::memcpy(data.data()+12,light,12);
    events.push_back({3,type,Bits(width),Bits(height),Bits(rotation),data});
}
'''

MAIN = r'''
template<class T> void Put(BYTE* ptr,T value) { std::memcpy(ptr,&value,sizeof(value)); }
template<class T> T Get(const BYTE* ptr) { T value; std::memcpy(&value,ptr,sizeof(value)); return value; }
const float fades[]={-2.0f,-1.0f,-0.0f,0.0f,0.05f,0.1f,0.2f,0.3f,0.9f,1.0f,1.1f,2.0f};
const float scales[]={0.0f,-0.0f,0.25f,0.5f,1.0f,2.0f,-1.0f};
void Fill(BYTE* sprite,unsigned value) {
    const short type=static_cast<short>((value*97u)%11000);
    Put(sprite+2,type); Put(sprite+12,scales[(value/12)%7]); Put(sprite+264,fades[value%12]); sprite[352]=value%3==0?0:value%3==1?1:255;
    for(int axis=0;axis<3;++axis) { Put(sprite+16+axis*4,static_cast<float>(static_cast<int>((value+axis*17)%256)-128)); Put(sprite+232+axis*4,static_cast<float>((value+axis*7)%17)*0.125f); }
    Put(sprite+36,static_cast<float>(value%360));
}
Event Expected(const BYTE* sprite) {
    const int type=Get<short>(sprite+2); const float original=Get<float>(sprite+264);
    const float fade=sprite[352]?(std::min)(1.0f,original+0.1f):(std::max)(0.2f,original-0.1f);
    const float factor=Get<float>(sprite+12)*fade;
    const auto* texture=reinterpret_cast<const BYTE*>(g_BitmapsRaw)+type*56;
    std::vector<BYTE> data(24); std::memcpy(data.data(),sprite+16,12); std::memcpy(data.data()+12,sprite+232,12);
    return {3,type,Bits(factor*Get<float>(texture+32)),Bits(factor*Get<float>(texture+36)),Bits(Get<float>(sprite+36)),data};
}
void Check(const std::vector<Event>& expected,std::uint64_t& digest) {
    Require(events.size()==expected.size());
    for(std::size_t i=0;i<events.size();++i) {
        const auto &a=events[i],&b=expected[i];
        if(a.kind!=b.kind || a.type!=b.type || a.width!=b.width || a.height!=b.height || a.rotation!=b.rotation || a.data!=b.data)
            std::fprintf(stderr,"sprite event %zu kind %d/%d type %d/%d width %08x/%08x height %08x/%08x\n",i,a.kind,b.kind,a.type,b.type,a.width,b.width,a.height,b.height);
        Require(a.kind==b.kind && a.type==b.type && a.width==b.width && a.height==b.height && a.rotation==b.rotation && a.data==b.data);
        for(auto value:{static_cast<std::uint32_t>(a.kind),static_cast<std::uint32_t>(a.type),a.width,a.height,a.rotation}) { digest^=value; digest*=1099511628211ull; }
        for(BYTE value:a.data) { digest^=value; digest*=1099511628211ull; }
    }
}
void Draw(const BYTE* object) { Render_DrawSprite(ARG); }
std::uint64_t RunSprite(BYTE* memory,BYTE* textureMemory) {
    constexpr std::size_t poolBytes=1002*444+32,tableBytes=11000*56+80;
    spritePool=reinterpret_cast<char*>(memory+16); g_BitmapsRaw=reinterpret_cast<char*>(textureMemory+64);
    std::memset(textureMemory,0xa5,tableBytes);
    for(int i=0;i<11000;++i) { Put(reinterpret_cast<BYTE*>(g_BitmapsRaw)+i*56+32,static_cast<float>((i*13)%2049)); Put(reinterpret_cast<BYTE*>(g_BitmapsRaw)+i*56+36,static_cast<float>((i*29+1)%1025)); }
    std::vector<BYTE> beforeTextures(textureMemory,textureMemory+tableBytes);
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,0x5a,476); BYTE* sprite=memory+16; Fill(sprite,trial);
        BYTE before[476]; std::memcpy(before,memory,476);
        blocked=trial%5==0; settingsCalls=0; events.clear();
        const std::vector<Event> expected=blocked?std::vector<Event>():std::vector<Event>{Expected(sprite)};
        Draw(blocked?nullptr:sprite);
        Require(settingsCalls==1 && std::memcmp(before,memory,476)==0); Check(expected,digest);
    }
    for(unsigned frame=0;frame<128;++frame) {
        std::memset(memory,0x6b,poolBytes); blocked=frame%7==0;
        std::vector<Event> expected; int active=0;
        for(int i=0;i<1002;++i) {
            BYTE* sprite=memory+16+i*444; sprite[0]=(i+frame)%5==0?0:static_cast<BYTE>(i%3==0?1:i%3==1?128:255);
            if(!sprite[0]) continue;
            Fill(sprite,frame*1002+i); const int blend=(i+frame)%5; Put(sprite+4,blend);
            if(blend<3) expected.push_back({blend,0,0,0,0,{}});
            if(!blocked) expected.push_back(Expected(sprite)); ++active;
        }
        std::vector<BYTE> before(memory,memory+poolBytes),after=before;
        for(int i=0;i<1002;++i) after[16+i*444]=0;
        for(int path=0;path<2;++path) {
            std::memcpy(memory,before.data(),poolBytes); events.clear(); settingsCalls=0;
            if(path==0) Render_DrawSpritePool(); else Character_UpdateAll();
            Require(settingsCalls==active && std::memcmp(after.data(),memory,poolBytes)==0); Check(expected,digest);
        }
    }
    Require(std::memcmp(beforeTextures.data(),textureMemory,tableBytes)==0); return digest;
}
int main() {
    auto* pool=static_cast<BYTE*>(Reserve(0x26000000u)); auto* textures=static_cast<BYTE*>(Reserve(0x24000000u)); Require(pool && textures);
    const auto expected=RunSprite(pool,textures);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull))); auto* highTextures=static_cast<BYTE*>(Reserve(std::uint64_t(0x124000000ull))); Require(high && highTextures);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(pool,0,1002*444+32); if(mask&2) std::memset(textures,0,11000*56+80);
            Require(RunSprite(mask&1?high:pool,mask&2?highTextures:textures)==expected);
            if(mask&1) for(int i=0;i<1002*444+32;++i) Require(pool[i]==0);
            if(mask&2) for(int i=0;i<11000*56+80;++i) Require(textures[i]==0);
        }
        std::puts("cuatro combinaciones pool/texturas altas y bajas, aliases intactos");
    }
    std::printf("Sprite draw: 32768 objetos y 128 frames por recorrido, blend/fade/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0].replace("0x10000","0x100000")
    prefix+='\n#define Require(...) do { if(!(__VA_ARGS__)) { std::fprintf(stderr,"sprite assertion line %d\\n",__LINE__); std::exit(1); } } while(false)\n'
    prefix+=re.search(r"enum eAntilag\s*\{.*?\};",read_source("src/Config/UserSettings.h",revision),re.S).group()+SERVICES
    entities=read_source("src/Entity/EntityTypes.h",revision)
    helpers=re.findall(r"^inline (?:const )?OBJECT\* AsObject[^\n]+",entities,re.M)
    if len(helpers)!=3: raise ValueError("Se esperaban tres overloads de AsObject")
    prefix+="\n".join(helpers)
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("_DAT_005524f4","_DAT_005526e4","_DAT_0055256c"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    for name in ("DAT_083a7cc0","DAT_083a7cc4"):
        prefix+="\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",header,re.M).group()+"\n"
    source=read_source("src/Sound/Sound_Queue.cpp",revision)
    draw=function(source,"Render_DrawSprite"); prefix+=draw+function(source,"Render_DrawSpritePool")
    prefix+=function(read_source("src/Render/Particle_Legacy.cpp",revision),"Character_UpdateAll")
    arg="reinterpret_cast<const OBJECT*>(object)" if "const OBJECT*" in draw.split("{",1)[0] else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix+MAIN.replace("ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-sprite-draw-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
