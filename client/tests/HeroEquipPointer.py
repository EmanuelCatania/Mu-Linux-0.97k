#!/usr/bin/env python3
"""Monitores reales de equipo: identidad nativa/cache y diagnóstico de wear slots.

Reconstrucción de clase/animaciones y logging controlados, sin mutar el equipo.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <string>
struct CHARACTER;
std::uintptr_t DAT_07abf5d8;
void* DAT_07cf1ffc;
int SceneFlag;
CHARACTER* expectedHero;
int expectedPrevOp,expectedPrevSub,currentOp,currentSub;
unsigned formatCalls;
std::vector<unsigned> events;
namespace Platform{
constexpr int TruncateText=0;
template<std::size_t N>int TextFormatTruncated(char(&out)[N],std::size_t size,int,const char* format,int prevOp,int prevSub,int op,int sub){
    Require(size==N && prevOp==expectedPrevOp && prevSub==expectedPrevSub && op==currentOp && sub==currentSub);++formatCalls;return std::snprintf(out,N,format,prevOp,prevSub,op,sub);
}
}
void DbgLogPublic(const char* text){if(std::strstr(text,"durante el paquete"))events.push_back(0);else if(std::strstr(text,"de golpe"))events.push_back(1);else Require(false);}
void SetCharacterClass(CHARACTER* c){Require(c==expectedHero);events.push_back(2);}
void Character_UpdateEquipSlotAnimations(CHARACTER* c){Require(c==expectedHero);events.push_back(3);}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
template<unsigned Instance>std::uint64_t Run(BYTE* first,BYTE* second,BYTE* machineMemory){
    std::uint64_t digest=14695981039346656037ull;std::memset(first,0x5a,948);std::memset(second,0x5a,948);std::memset(first+4096,0x6b,948);std::memset(machineMemory,0xa5,1600);BYTE* machine=machineMemory+16;
    for(int slot=0;slot<12;++slot){Put(machine+536+slot*68,static_cast<short>(100+slot));Put(machine+540+slot*68,0x01020304u+slot);}
    expectedPrevOp=expectedPrevSub=-1;
    for(unsigned frame=0;frame<24;++frame){
        if(frame==2)Put(machine+536,static_cast<short>(200));
        if(frame==10)Put(machine+540+2*68,0x12345678u);
        if(frame==13)for(int slot=0;slot<12;++slot)Put(machine+536+slot*68,static_cast<short>(-1));
        if(frame==14)for(int slot=0;slot<12;++slot){Put(machine+536+slot*68,static_cast<short>(100+slot));Put(machine+540+slot*68,0x01020304u+slot);}
        if(frame==16){for(int slot=0;slot<12;++slot)Put(machine+536+slot*68,static_cast<short>(-1));Put(machine+536,static_cast<short>(100));}
        if(frame==21)Put(machine+540+2*68,0xabcdef12u);
        BYTE* hero=(frame>=4 && frame<17?second:first)+16;expectedHero=reinterpret_cast<CHARACTER*>(hero);
        DAT_07abf5d8=frame==12?0:reinterpret_cast<std::uintptr_t>(hero);DAT_07cf1ffc=frame==6||frame==18?nullptr:machine;
        SceneFlag=frame==9 || frame==20 || frame==21?2:5;
        BYTE* argument=frame==8?first+4096+16:hero;
        currentOp=0x40+frame;currentSub=frame^0x55;events.clear();formatCalls=0;
        BYTE beforeFirst[948],beforeSecond[948],beforeAlien[948],beforeMachine[1600];std::memcpy(beforeFirst,first,948);std::memcpy(beforeSecond,second,948);std::memcpy(beforeAlien,first+4096,948);std::memcpy(beforeMachine,machineMemory,1600);
        Subject<Instance>::EquipWipe_Tick(currentOp,currentSub);Subject<Instance>::HeroEquipWatchdog(ARG);
        std::vector<unsigned> expected;
        if(frame==13)expected={0,1};
        if(frame==0 || frame==2 || frame==4 || frame==10 || frame==13 || frame==14 || frame==16 || frame==17 || frame==22){expected.push_back(2);expected.push_back(3);}
        Require(events==expected && formatCalls==(frame==13?1u:0u));
        Require(std::memcmp(first,beforeFirst,948)==0 && std::memcmp(second,beforeSecond,948)==0 && std::memcmp(first+4096,beforeAlien,948)==0 && std::memcmp(machineMemory,beforeMachine,1600)==0);
        for(auto event:events){digest^=event;digest*=1099511628211ull;}for(BYTE value:beforeMachine){digest^=value;digest*=1099511628211ull;}
        expectedPrevOp=currentOp;expectedPrevSub=currentSub;
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* second=static_cast<BYTE*>(Reserve(sizeof(void*)>4?std::uintptr_t(0x126000000ull):std::uintptr_t(0x24000000u)));auto* machine=static_cast<BYTE*>(Reserve(0x22000000u));Require(low&&second&&machine);
    const auto expected=Run<0>(low,second,machine);
    if(sizeof(void*)>4){auto* highMachine=static_cast<BYTE*>(Reserve(std::uint64_t(0x122000000ull)));Require(highMachine && reinterpret_cast<std::uintptr_t>(second)>0xffffffffull);
        Require((reinterpret_cast<std::uintptr_t>(low)&0xffffffffull)==(reinterpret_cast<std::uintptr_t>(second)&0xffffffffull));
        Require(Run<1>(second,low,machine)==expected && Run<2>(low,second,highMachine)==expected && Run<3>(second,low,highMachine)==expected);
        std::puts("cuatro combinaciones heroe/ficha, cambio de identidad con mismos32bits: cache/logs/readonly iguales");
    }std::printf("Hero equip: 24 frames por instancia, cache/diagnostico %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Render/Render_PlayerEquipment.cpp',revision);header=read_source('src/Entity/EntityTypes.h',revision)
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES
    prefix+='\n'.join(re.findall(r'^inline (?:const )?CHARACTER\* AsCharacter[^\n]+',header,re.M))
    monitor=function(source,'HeroEquipWatchdog');wipe=function(source,'EquipWipe_Tick')
    prefix+='\ntemplate<unsigned Instance>struct Subject{static '+wipe+'\nstatic '+monitor+'\n};\n'
    arg='reinterpret_cast<CHARACTER*>(argument)' if 'CHARACTER* c' in monitor.split('{',1)[0] else 'static_cast<int>(reinterpret_cast<uintptr_t>(argument))'
    return prefix+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-hero-equip-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
