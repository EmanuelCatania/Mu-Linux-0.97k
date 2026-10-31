#!/usr/bin/env python3
"""Entrada real sobre fixtures x86; colores y Setup nativos se prueban por separado."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from BmdSetupPointer import SERVICES
from SceneObjectPointer import build_and_run, function, read_source

EXTRA=r'''
using byte=BYTE;
std::uintptr_t DAT_05828d58;
float DAT_06970acc[12],DAT_06970afc[12],DAT_06970c4c[12],DAT_06970c7c[12];
int currentMap;bool blocked;
struct Map {int GetCurrentMap(){return currentMap;}}gMapManager;
struct Settings {bool GetAntilag(int id){Require(id==ANTILAG_GLOW);Record(12,{static_cast<unsigned>(blocked)});return blocked;}}gUserSettings;
struct Catalog {int LogicModel(int type){Record(13,{static_cast<unsigned>(type)});return type;}}gContentCatalog;
namespace Gfx {void Color3f(float r,float g,float b){Record(14,{Bits(r),Bits(g),Bits(b)});}void Color4f(float r,float g,float b,float a){Record(15,{Bits(r),Bits(g),Bits(b),Bits(a)});}}
void GL_ResetState(){Record(16,{});}void EnableAlphaTest(bool enabled){Require(enabled);Record(17,{});}
void BMD__RenderBodyShadow(void* model,int blend,int hidden){Require(model==activeModel);Record(18,{static_cast<unsigned>(blend),static_cast<unsigned>(hidden)});}
void BMD__TransformPosition(void* model,float(*)[4],float* in,float* out,bool translate){Require(model==activeModel && !translate && in[0]==0 && in[1]==0 && in[2]==0);Record(19,{});out[0]=1.25f;out[1]=-0.5f;out[2]=2.0f;}
int CreateSprite(unsigned short type,float* pos,float scale,float* light,int owner,float rotation,int mode){
    Require(type==1176 && static_cast<unsigned>(owner)==reinterpret_cast<std::uintptr_t>(activeEntity) && rotation==0 && mode==0);
    Record(20,{type,Bits(scale),Bits(rotation),static_cast<unsigned>(mode)});Hash(pos,12);Hash(light,12);return 1;
}
void PartObjectColor(int type,float scale,float alpha,float* color,char flag){Require(reinterpret_cast<BYTE*>(color)==activeModel+72);Record(21,{static_cast<unsigned>(type),Bits(scale),Bits(alpha),static_cast<unsigned>(flag)});color[0]=type*0.001f;color[1]=scale;color[2]=alpha;}
void Weapon_SetColorAlt(int type,float scale,float alpha,float* color){Require(reinterpret_cast<BYTE*>(color)==activeModel+72);Record(22,{static_cast<unsigned>(type),Bits(scale),Bits(alpha)});color[0]=type*0.002f;color[1]=scale;color[2]=alpha;}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
std::uint64_t Run(BYTE* memory,BYTE* entityMemory){
    constexpr unsigned tableBytes=1200*188+32;
    DAT_05828d58=reinterpret_cast<std::uintptr_t>(memory+16);activeEntity=entityMemory+16;
    std::memset(memory,0xa5,tableBytes);eventDigest=14695981039346656037ull;events=0;
    const int models[]={419,431,471,506,546,570,607,608,671,676,703,708,735,740,767,772,790,865,958,543,957,0};
    for(unsigned trial=0;trial<8208;++trial){
        const short type=static_cast<short>(static_cast<int>(trial%1026)-1);const int model=models[(trial/7)%22];
        activeModel=memory+16+model*188;std::memset(activeModel,0x5a,188);std::memset(entityMemory,0xa5,480);
        Put(activeEntity+2,type);Put(activeEntity+88,-1);Put(activeEntity+100,static_cast<int>(trial%3));
        for(int axis=0;axis<3;++axis){Put(activeEntity+16+axis*4,static_cast<float>(static_cast<int>((trial+axis*3)%65)-32)*0.25f);Put(activeEntity+104+axis*4,static_cast<float>((trial+axis*7)%17)*0.125f);Put(activeEntity+232+axis*4,static_cast<float>((trial+axis*11)%9)*0.125f);}
        Put(activeEntity+120,static_cast<unsigned>((trial/8)%4));Put(activeEntity+360,static_cast<float>(trial%9)*0.125f);
        activeEntity[140]=(trial/32)%3==0?0:(trial/32)%3==1?1:255;
        currentMap=(trial/64)%3==0?7:(trial/64)%3==1?10:0;blocked=(trial/128)%2;
        WorldTime=static_cast<float>(static_cast<int>(trial%8192)-1024);DAT_083a45d4=static_cast<float>(trial%17)*0.0625f;sample={0.25f,0.75f};
        float light[]={0.125f,0.5f,1.0f},beforeLight[3];std::memcpy(beforeLight,light,12);
        std::vector<BYTE> before(memory,memory+tableBytes);
        RenderPartObjectEffect(static_cast<int>(reinterpret_cast<std::uintptr_t>(activeEntity)),model,light,static_cast<float>(trial%5)*0.25f,((trial/16)%16)<<3,static_cast<BYTE>((trial/256)%4),0,(trial/512)%2?0x100:((trial/1024)%2?0x400:2));
        const auto offset=static_cast<unsigned>(activeModel-memory);std::memcpy(before.data()+offset+72,activeModel+72,12);before[offset+136]=activeModel[136];
        if(type==833)before[16+833*188+136]=255;
        Require(std::memcmp(memory,before.data(),tableBytes)==0 && std::memcmp(light,beforeLight,12)==0);
        for(unsigned i=0;i<16;++i)Require(entityMemory[i]==0xa5 && entityMemory[464+i]==0xa5);
        Hash(activeModel,188);Hash(entityMemory,480);
    }Hash(&events,4);return eventDigest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* entity=static_cast<BYTE*>(Reserve(0x24000000u));Require(low&&entity);
    const auto expected=Run(low,entity);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0,1200*188+32);Require(Run(high,entity)==expected);for(unsigned i=0;i<1200*188+32;++i)Require(low[i]==0);std::puts("tabla de modelos alta: bytes/eventos iguales; entidad y owners32 bajos, alias intacto");}
    std::printf("Effect model: 8208 casos, pipeline/bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    # Esta entrada aún usa registros188/444 y owners32. Mantener su frontera x86
    # explícita; BmdSetupPointer/Differential ejecutan la preparación nativa real.
    setup=function(read_source('tests/fixtures/BMDSetup-b0a02c59.cpp'),'BMD_SetupRenderByType')
    body=read_source('tests/fixtures/BodyColor-e3731d7c.cpp')
    color=function(body,'RenderPartObjectBodyColor')+function(body,'Entity_SetModelColorAlt')
    effect=function(read_source('src/Render/Entity_DrawSetup.cpp',revision),'RenderPartObjectEffect')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')
    prefix+=re.search(r'enum eAntilag\s*\{.*?\};',read_source('src/Config/UserSettings.h',revision),re.S).group()+SERVICES+EXTRA
    header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    names=set(re.findall(r'_DAT_[0-9a-f]+',setup+effect));names.add('DAT_083a45d4')
    for name in sorted(names):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    return prefix+setup+color+effect+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-effect-model-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
