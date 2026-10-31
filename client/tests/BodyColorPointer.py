#!/usr/bin/env python3
"""Dos cuerpos de color reales; servicios de color y backend controlados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
using uint=unsigned;
BYTE* activeModel;BYTE* activeEntity;
unsigned calls,colorCalls,variant;
int currentType,currentFlags;
std::uint64_t digest;
void Hash(const void* bytes,std::size_t size) {const BYTE* p=static_cast<const BYTE*>(bytes);while(size--){digest^=*p++;digest*=1099511628211ull;}}
void PartObjectColor(int type,float scale,float alpha,float* color,char mode) {
    Require(variant==0 && type==currentType && reinterpret_cast<BYTE*>(color)==activeModel+72 && mode==((currentFlags>>8)&1));++colorCalls;
    color[0]=static_cast<float>(type)*0.125f;color[1]=scale;color[2]=alpha;
    Hash(&type,4);Hash(&scale,4);Hash(&alpha,4);Hash(&mode,1);
}
void Weapon_SetColorAlt(int type,float scale,float alpha,float* color) {
    Require(variant==1 && type==currentType && reinterpret_cast<BYTE*>(color)==activeModel+72);++colorCalls;
    color[0]=static_cast<float>(type)*0.25f;color[1]=scale;color[2]=alpha;
    Hash(&type,4);Hash(&scale,4);Hash(&alpha,4);
}
void BMD__RenderBody(void* model,int flags,float f1,int f2,float f3,float f4,float f5,int hidden,int rgba) {
    Require(model==activeModel && flags==((variant==0 && currentType==324 && !(currentFlags&16))?currentFlags&~256:currentFlags));
    const int expected=variant==0 && currentType==471?2:currentType==565?1:variant==0 && (currentType==431 || currentType==506 || currentType==608)?123:-1;
    Require(hidden==expected);++calls;
    Hash(&flags,4);Hash(&f1,4);Hash(&f2,4);Hash(&f3,4);Hash(&f4,4);Hash(&f5,4);Hash(&hidden,4);Hash(&rgba,4);Hash(activeModel+72,12);Hash(activeEntity+88,4);
}
'''
MAIN = r'''
template<class T> void Put(BYTE* p,T value) {std::memcpy(p,&value,sizeof(value));}
const int flags[]={0,16,256,272,64,320,72,328,-1,0x240,0x10000,0x10010,0x44,0x48,0x110,0x210};
void* Draw(unsigned path,BYTE* entity,int type,float scale,int flag,float alpha,int rgba) {
    if(path==0)return RenderPartObjectBodyColor(activeModel,ARG,type,scale,flag,alpha,rgba);
    return Entity_SetModelColorAlt(activeModel,ARG,type,scale,flag,alpha,rgba);
}
std::uint64_t Run(BYTE* modelMemory,BYTE* entityMemory) {
    digest=14695981039346656037ull;activeModel=modelMemory+16;activeEntity=entityMemory+16;
    for(currentType=-1;currentType<=1024;++currentType)for(unsigned state=0;state<16;++state)for(variant=0;variant<2;++variant) {
        std::memset(modelMemory,0xa5,224);std::memset(entityMemory,0x5a,480);calls=colorCalls=0;currentFlags=flags[state];
        Put(activeEntity+88,123);Put(activeEntity+100,static_cast<int>(state*137)-1);
        for(int i=0;i<3;++i)Put(activeEntity+104+i*4,static_cast<float>(static_cast<int>(state*7+i*3)-23)*0.125f);
        BYTE beforeModel[224],beforeEntity[480];std::memcpy(beforeModel,modelMemory,224);std::memcpy(beforeEntity,entityMemory,480);
        Require(Draw(variant,activeEntity,currentType,static_cast<float>(state)*0.125f,currentFlags,static_cast<float>(state)*0.25f,state%2?-1:1171)==nullptr);
        Require(calls==1 && colorCalls==((currentFlags&16)?0u:variant==0 && currentType==324?0u:1u));
        Require(std::memcmp(beforeModel,modelMemory,88)==0 && std::memcmp(beforeModel+100,modelMemory+100,124)==0);
        if(variant==0 && currentType==471)Put(beforeEntity+16+88,-1);
        Require(std::memcmp(beforeEntity,entityMemory,480)==0);
        if(currentFlags&16) {float color[3]={1,1,1};Require(std::memcmp(activeModel+72,color,12)==0);}
        Hash(modelMemory,224);Hash(entityMemory,480);
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,2048);Require(Run(mask&1?high:low,(mask&2?high:low)+1024)==expected);
            if(mask&1)for(unsigned i=0;i<224;++i)Require(low[i]==0x7b);
            if(mask&2)for(unsigned i=0;i<480;++i)Require(low[1024+i]==0x7b);
        }std::puts("cuatro combinaciones modelo/entidad altos y bajos; aliases intactos");
    }
    std::printf("Body color: 32832 casos, colores/hidden mesh/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None, legacy=False):
    source=read_source('src/Render/Render_LegacyBillboards.cpp',revision)
    if legacy:source=read_source('tests/fixtures/BodyColor-e3731d7c.cpp')
    body=function(source,'RenderPartObjectBodyColor');alt=function(source,'Entity_SetModelColorAlt')
    arg='reinterpret_cast<uintptr_t>(entity)' if 'uintptr_t entity' in body else 'static_cast<int>(reinterpret_cast<uintptr_t>(entity))'
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]
    if legacy:
        prefix+='constexpr unsigned ModelBytes=224,LightOffset=72;\n'
    else:
        prefix+=layouts()+'using vec34_t=float[3][4];\n#pragma pack(push,1)\n'+record(read_source('src/Entity/EntityTypes.h'),'OBJECT')+'\n#pragma pack(pop)\n'
        prefix+='OBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}\nconstexpr unsigned LightOffset=offsetof(BMD,BodyLight);\nstatic_assert(sizeof(OBJECT)==(sizeof(void*)==4?444:464));\nstatic_assert(LightOffset==(sizeof(void*)==4?72:96));\n'
    services=SERVICES.replace('activeModel+72','activeModel+LightOffset')
    main=MAIN.replace('ARG',arg).replace('224','ModelBytes').replace('activeModel+72','activeModel+LightOffset')
    main=main.replace('modelMemory,88','modelMemory,16+LightOffset').replace('beforeModel+100,modelMemory+100,124','beforeModel+28+LightOffset,modelMemory+28+LightOffset,ModelBytes-28-LightOffset')
    # Comparar contenido semántico entre layouts; las guardas completas se revisan arriba.
    main=main.replace('Hash(modelMemory,ModelBytes);','Hash(activeModel+LightOffset,12);')
    return prefix+services+body+alt+main


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');parser.add_argument('--differential',action='store_true');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-body-color-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.differential:
            legacy=build_and_run(directory/'legacy',harness(legacy=True),args)
            if legacy!=current:raise RuntimeError('Diferencia semántica frente al núcleo x86 congelado')
            print('Colores/argumentos/estado equivalentes al núcleo e3731d7c')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
