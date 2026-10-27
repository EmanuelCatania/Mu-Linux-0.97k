#!/usr/bin/env python3
"""CheckFullSet y cola reales; prefijo CHARACTER opaco x86, sin layout nativo."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
int EquipmentLevelSet;
'''
MAIN=r'''
bool Check(BYTE* entity){return CheckFullSet(ARG);}
std::uint64_t Run(BYTE* entityMemory,BYTE* machine){
    std::uint64_t digest=14695981039346656037ull;
    const unsigned short armors[]={271,276,0,65535};
    for(unsigned trial=0;trial<65536;++trial){
        std::memset(entityMemory,0xa5,960);std::memset(machine,0x5a,1504);
        BYTE* entity=entityMemory+16;auto* character=AsCharacter(entity);
        const BYTE cls=static_cast<BYTE>(trial);character->Class=cls;
        const unsigned profile=(trial/256)%16,mode=(trial/4096)%4;
        for(unsigned part=0;part<6;++part){
            auto& p=character->BodyPart[part];p.Type=static_cast<short>(ITEM_MODEL_BASE+part*16+3);p.Level=static_cast<BYTE>(9+(profile%3));
            if(profile==3)p.Level=static_cast<BYTE>(9+part%3);
            if(profile>=4 && profile<=9 && part==profile-4)p.Type=-1;
            if(profile==10 && part==3)++p.Type;
            if(profile==11 && part==4)p.Level=8;
            if(profile==12)p.Level=255;
            if(profile==13)p.Type=static_cast<short>(part==2?-17:15);
            if(profile==14)p.Level=static_cast<BYTE>(0xf0|(8+part%3));
            if(profile==15){p.Type=static_cast<short>(part==1?-1:ITEM_MODEL_BASE+part*32);p.Level=10;}
        }
        CharacterAttribute=mode==0?nullptr:machine+16;DAT_07cf1ffc=mode==2?machine+32:CharacterAttribute;
        const unsigned short armor=armors[trial/16384];std::memcpy(machine+16+740,&armor,2);
        bool expected=true;int minimum=15;const unsigned first=(cls&7)==3?2:1;
        const int set=(character->BodyPart[5].Type-ITEM_MODEL_BASE)%16;
        for(unsigned part=first;part<=5;++part){const auto& p=character->BodyPart[part];if(p.Type==-1 || (p.Level&15)<9 || (p.Type-ITEM_MODEL_BASE)%16!=set)expected=false;minimum=(std::min)(minimum,static_cast<int>(p.Level&15));}
        const int defense=((cls&7)==3 && expected && CharacterAttribute && CharacterAttribute==DAT_07cf1ffc && armor!=271 && armor!=276)?0:1;
        BYTE beforeEntity[960],beforeMachine[1504];std::memcpy(beforeEntity,entityMemory,960);std::memcpy(beforeMachine,machine,1504);
        EquipmentLevelSet=123;g_bAddDefense=-3;const bool result=Check(entity);
        Require(result==expected && EquipmentLevelSet==(expected?minimum:0) && g_bAddDefense==defense);
        Require(std::memcmp(beforeEntity,entityMemory,960)==0 && std::memcmp(beforeMachine,machine,1504)==0);
        for(unsigned value:{static_cast<unsigned>(result),static_cast<unsigned>(EquipmentLevelSet),static_cast<unsigned>(g_bAddDefense)}){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+2048);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,4096);Require(Run(mask&1?high:low,(mask&2?high:low)+2048)==expected);
            if(mask&1)for(unsigned i=0;i<960;++i)Require(low[i]==0x7b);
            if(mask&2)for(unsigned i=0;i<1504;++i)Require(low[2048+i]==0x7b);
        }std::puts("cuatro combinaciones personaje/ficha altos y bajos; aliases intactos");
    }std::printf("Full set: 65536 casos, set/nivel/defensa/readonly %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Render/Render_PlayerEquipment.cpp',revision)
    header=read_source('src/Entity/EntityTypes.h',revision)
    part=re.search(r'struct PART_t\s*\{.*?\};',header,re.S).group()
    # Sólo prefijo sin punteros; OBJECT es una reserva opaca de los bytes x86.
    character=header[header.index('struct CHARACTER {'):header.index('    PART_t   Weapon[2];')]
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#include <algorithm>\n#include <initializer_list>\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\nstruct OBJECT {BYTE opaque[444];};\n'
    prefix+=part+character+'BYTE remainder[292];};\nstatic_assert(sizeof(CHARACTER)==916);static_assert(offsetof(CHARACTER,Class)==444);static_assert(offsetof(CHARACTER,BodyPart)==480);\n'
    prefix+='\n'.join(re.findall(r'^inline (?:const )?CHARACTER\* AsCharacter[^\n]+',header,re.M))
    prefix+=re.search(r'constexpr int ITEM_MODEL_BASE\s*=\s*[^;]+;',read_source('src/Item/ItemDefines.h',revision)).group()
    globals_header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in ['DAT_07cf1ff4','DAT_07cf1ffc']:
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',globals_header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    prefix+='\n'+re.search(r'^#define CharacterAttribute[^\n]+',globals_header,re.M).group()+'\n'
    prefix+=re.search(r'^int g_bAddDefense\s*=\s*[^;]+;',source,re.M).group()+SERVICES
    code=function(source,'CheckFullSet_Tail')+function(source,'CheckFullSet')
    arg='reinterpret_cast<uintptr_t>(entity)' if 'CheckFullSet(uintptr_t' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(entity))'
    return prefix+code+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-full-set-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
