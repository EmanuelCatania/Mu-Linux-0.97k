#!/usr/bin/env python3
"""Iluminación real con oráculo aritmético; vista legacy o BMD/owner nativos."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
using uint=unsigned;
using undefined1=unsigned char;
using undefined4=std::uint32_t;
using float10=long double;
float10 fsin(float10 value){return std::sin(value);}
float WorldTime;int SceneFlag;
float terrain[3];float expectedX,expectedY;unsigned terrainCalls;
void RequestTerrainLight(float x,float y,float* rgb){Require(x==expectedX && y==expectedY);++terrainCalls;std::memcpy(rgb,terrain,12);}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
std::uint64_t Run(BYTE* entityMemory,BYTE* modelMemory,BYTE* owner){
    std::uint64_t digest=14695981039346656037ull;
    const short types[]={325,225,162,0,390,869};const int scenes[]={2,4,5,0,10};
    for(unsigned trial=0;trial<32768;++trial){
        std::memset(entityMemory,0xa5,480);std::memset(modelMemory,0x5a,224);std::memset(owner,0x6b,160);
        BYTE *entity=entityMemory+16,*model=modelMemory+16;
        const short type=types[trial%6];Put(entity+2,type);
        const unsigned char contrast=(trial/6)%4==0?0:(trial/6)%4==1?1:(trial/6)%4==2?128:255;entity[220]=contrast;
        const unsigned flags=(trial/24)%2?0x200u:0u;Put(owner+120,flags);
        Put(entity+252,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(owner)));
        expectedX=static_cast<float>(static_cast<int>(trial%1024)-512)*0.25f;expectedY=-expectedX;
        Put(entity+16,expectedX);Put(entity+20,expectedY);
        float light[3];for(int axis=0;axis<3;++axis){light[axis]=static_cast<float>((trial+axis*7)%17)*0.125f;Put(entity+232+axis*4,light[axis]);}
        SceneFlag=scenes[(trial/48)%5];WorldTime=static_cast<float>(trial%4096)-1024.0f;
        for(int axis=0;axis<3;++axis)terrain[axis]=(trial/240)%3==0?0.0f:(trial/240)%3==1?static_cast<float>(axis+1)*0.125f:axis==1?0.5f:0.0f;
        BYTE beforeEntity[480],beforeModel[224],beforeOwner[160];std::memcpy(beforeEntity,entityMemory,480);std::memcpy(beforeModel,modelMemory,224);std::memcpy(beforeOwner,owner,160);
        terrainCalls=0;Entity_GetLightScale(ENTITY_ARG,MODEL_ARG);
        float color[3];
        if(type==325){for(float& c:color)c=0.6f;Require(terrainCalls==0);}
        else if(type==225 && flags&0x200){const float c=static_cast<float>((fsin(static_cast<float10>(WorldTime)*static_cast<float10>(_DAT_00552a08))+static_cast<float10>(_DAT_0055256c))*static_cast<float10>(_DAT_00552504));for(float& component:color)component=c;Require(terrainCalls==0);}
        else{
            Require(terrainCalls==1);beforeModel[16+68]=contrast;
            const bool zero=terrain[0]==0 && terrain[1]==0 && terrain[2]==0;
            const bool fallback=zero && ((SceneFlag!=2 && SceneFlag!=4) || type==162);
            for(int axis=0;axis<3;++axis){const float component=fallback?1.0f:terrain[axis];color[axis]=(contrast?component:component*_DAT_005524f4)+light[axis];}
        }
        std::memcpy(beforeModel+16+72,color,12);
        Require(std::memcmp(beforeModel,modelMemory,224)==0 && std::memcmp(beforeEntity,entityMemory,480)==0 && std::memcmp(beforeOwner,owner,160)==0);
        for(BYTE value:beforeModel){digest^=value;digest*=1099511628211ull;}digest^=terrainCalls;digest*=1099511628211ull;
    }return digest;
}
int main(){
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024,low+2048);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,2048);Require(Run(mask&1?high:low,(mask&2?high:low)+1024,low+2048)==expected);
            if(mask&1)for(unsigned i=0;i<480;++i)Require(low[i]==0x7b);
            if(mask&2)for(unsigned i=0;i<224;++i)Require(low[1024+i]==0x7b);
        }std::puts("cuatro combinaciones entidad/modelo altos y bajos; owner32 bajo, aliases intactos");
    }std::printf("Light scale: 32768 casos, colores/flags/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None,native=False):
    code=function(read_source('src/Terrain/Terrain_Light.cpp',revision),'Entity_GetLightScale')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES
    if 'AsObject(' in code:
        from ObjectLayoutContract import object_types,record
        from LegacyModelLayout import VIEW
        prefix+="\nusing WORD=std::uint16_t;using vec3_t=float[3];using vec34_t=float[3][4];\n"+object_types()+"\nOBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}\n"
        prefix+=("\n#pragma pack(push,1)\n"+record(read_source('src/Model/ModelTypes.h'),'BMD')+"\n#pragma pack(pop)\ninline BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}") if native else VIEW

    header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in sorted(set(re.findall(r'_DAT_[0-9a-f]+',code))):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    arg='reinterpret_cast<uintptr_t>(PTR)' if 'uintptr_t param_1' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(PTR))'
    main=MAIN.replace('static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(owner))','reinterpret_cast<std::uintptr_t>(owner)') if 'AsObject(' in code else MAIN
    if native:
        main=main.replace('224','256').replace('beforeModel[16+68]','beforeModel[16+offsetof(BMD,LightEnable)]').replace('beforeModel+16+72','beforeModel+16+offsetof(BMD,BodyLight)').replace('mask<4','mask<8').replace('low+2048)==expected','(mask&4?high:low)+2048)==expected').replace('cuatro combinaciones entidad/modelo altos y bajos; owner32 bajo','ocho combinaciones entidad/modelo/owner nativos altos y bajos')
    return prefix+code+main.replace('ENTITY_ARG',arg.replace('PTR','entity')).replace('MODEL_ARG',arg.replace('PTR','model'))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--native-layout',action='store_true');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and args.native_layout:parser.error('La referencia usa la vista legacy x86')
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-light-scale-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(native=args.native_layout),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
