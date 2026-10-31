#!/usr/bin/env python3
"""Preparación real BMD: color, argumentos de draw y direcciones completas.

Backend y muestras de materiales controlados. No certifica raster ni BMD nativo.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record

SERVICES = r'''
#include <cmath>
#include <initializer_list>
using uint=unsigned int;
using undefined4=std::uint32_t;
using float10=long double;
using ::intptr_t;
using ::uintptr_t;
float WorldTime;
float10 fsin(float10 value) { return std::sin(value); }
BYTE* activeModel; BYTE* activeEntity;
std::uint64_t eventDigest;
unsigned events;
void Hash(const void* bytes,std::size_t size) {
    const BYTE* p=static_cast<const BYTE*>(bytes);
    while(size--) {eventDigest^=*p++;eventDigest*=1099511628211ull;}
}
void Record(unsigned kind,std::initializer_list<std::uint32_t> data) {
    ++events;Hash(&kind,4);for(auto value:data) Hash(&value,4);
    Hash(activeModel+72,12);
}
std::uint32_t Bits(float value) {std::uint32_t result;std::memcpy(&result,&value,4);return result;}
void BMD__BeginRender() {Record(1,{});}
void BMD__RenderMesh(void* model,float layer,int animation,float scale,int height,float x,float y,float z,unsigned color) {
    Require(model==activeModel);
    Record(2,{Bits(layer),static_cast<unsigned>(animation),Bits(scale),static_cast<unsigned>(height),Bits(x),Bits(y),Bits(z),color});
}
void BMD__RenderBody(void* model,int flags,float f1,int f2,float f3,float f4,float f5,int f6,int rgba) {
    Require(model==activeModel);
    Record(3,{static_cast<unsigned>(flags),Bits(f1),static_cast<unsigned>(f2),Bits(f3),Bits(f4),Bits(f5),static_cast<unsigned>(f6),static_cast<unsigned>(rgba)});
}
namespace BufferedDraw {
void Color3f(float r,float g,float b) {Record(4,{Bits(r),Bits(g),Bits(b)});}
void Color4f(float r,float g,float b,float a) {Record(5,{Bits(r),Bits(g),Bits(b),Bits(a)});}
}
namespace BMDDevice {void EndModel() {Record(6,{});}}
struct Sample {float u,v;} sample;
const Sample& MaterialVisuals_DrawSample(const void* owner,int model) {
    Require(owner==activeEntity && (model==607 || model==608));Record(7,{static_cast<unsigned>(model)});return sample;
}
'''
MAIN = r'''
template<class T> void Put(BYTE* p,T value) {std::memcpy(p,&value,sizeof(value));}
std::uint64_t Run(BYTE* modelMemory,BYTE* entityMemory) {
    activeModel=modelMemory+16;activeEntity=entityMemory+16;
    eventDigest=14695981039346656037ull;events=0;
    for(int type=-1;type<=1024;++type) for(unsigned state=0;state<8;++state) {
        std::memset(modelMemory,0xa5,224);std::memset(entityMemory,0x5a,480);
        for(int i=0;i<3;++i) Put(activeModel+72+i*4,static_cast<float>(i+state)*0.125f);
        Put(activeEntity+88,static_cast<int>(state%3)-1);Put(activeEntity+100,static_cast<int>(state*137)-1);
        for(int i=0;i<3;++i) Put(activeEntity+104+i*4,static_cast<float>(static_cast<int>(state*7+i*3)-23)*0.125f);
        Put(activeEntity+360,static_cast<float>(state)*0.125f);
        WorldTime=(state==0?0.0f:state==1?-0.0f:state==2?-32.0f:static_cast<float>(state*513));
        sample={static_cast<float>(state)*0.125f,static_cast<float>(7-state)*0.125f};
        const float alpha=static_cast<float>(state)*0.25f;const unsigned flags=state%2?0xffffffffu:state*513u;
        BYTE beforeModel[224],beforeEntity[480];std::memcpy(beforeModel,modelMemory,224);std::memcpy(beforeEntity,entityMemory,480);
        const unsigned prior=events;
        BMD_SetupRenderByType(activeModel,ENTITY_ARG,type,alpha,flags);
        Require(events>prior && std::memcmp(beforeEntity,entityMemory,480)==0);
        Require(std::memcmp(beforeModel,modelMemory,88)==0 && std::memcmp(beforeModel+100,modelMemory+100,124)==0);
        const bool plain=type!=671 && type!=735 && type!=703 && type!=767 && type!=676 && type!=740 && type!=708 && type!=772 && type!=790 && type!=419 && type!=570 && type!=546 && type!=431 && type!=506 && type!=471 && type!=607 && type!=608;
        if(plain) Require(std::memcmp(beforeModel,modelMemory,224)==0);
        Hash(modelMemory,224);
    }
    Hash(&events,4);return eventDigest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
    const auto expected=Run(low,low+1024);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask) {
            std::memset(low,0x7b,2048);Require(Run(mask&1?high:low,(mask&2?high:low)+1024)==expected);
            if(mask&1) for(unsigned i=0;i<224;++i) Require(low[i]==0x7b);
            if(mask&2) for(unsigned i=0;i<480;++i) Require(low[1024+i]==0x7b);
        }
        std::puts("cuatro combinaciones modelo/entidad altos y bajos; aliases intactos");
    }
    std::printf("BMD setup: 8208 casos, colores/eventos/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Render/BMD_SetupRender.cpp',revision)
    code=function(source,'BMD_SetupRenderByType')
    types=layouts()+'\nusing vec34_t=float[3][4];\n#pragma pack(push,1)\n'+record(read_source('src/Entity/EntityTypes.h'),'OBJECT')+'\n#pragma pack(pop)\nOBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}\nconstexpr unsigned ModelBytes=sizeof(BMD)+36,EntityBytes=sizeof(OBJECT)+36;\n'
    types=types.replace('constexpr unsigned ModelBytes=sizeof(void*)==4?224:256;','')
    services=SERVICES.replace('Hash(activeModel+72,12)','Hash(activeModel+offsetof(BMD,BodyLight),12)')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+types+services
    header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in sorted(set(re.findall(r'_DAT_[0-9a-f]+',code))):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    arg='reinterpret_cast<uintptr_t>(activeEntity)'
    if 'uintptr_t param_2' not in code:arg='static_cast<int>(reinterpret_cast<uintptr_t>(activeEntity))'
    main=MAIN.replace('ENTITY_ARG',arg);main=re.sub(r'\b224\b','ModelBytes',main);main=re.sub(r'\b480\b','EntityBytes',main)
    main=main.replace('activeModel+72','activeModel+offsetof(BMD,BodyLight)')
    for off,field in [('88','HiddenMesh'),('100','BlendMesh'),('104','BlendMeshLight'),('360','Alpha')]:main=main.replace('activeEntity+'+off,'activeEntity+offsetof(OBJECT,'+field+')')
    main=main.replace('modelMemory,88','modelMemory,16+offsetof(BMD,BodyLight)').replace('beforeModel+100','beforeModel+16+offsetof(BMD,BodyLight)+12').replace('modelMemory+100','modelMemory+16+offsetof(BMD,BodyLight)+12').replace(',124)',',ModelBytes-16-offseTOF(BMD,BodyLight)-12)').replace('offseTOF','offsetof')
    return prefix+code+main


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-bmd-setup-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
