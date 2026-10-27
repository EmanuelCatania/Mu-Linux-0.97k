#!/usr/bin/env python3
"""SetActionClass, Gesture y paquete real; registros opacos, SetAction controlado."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <array>
struct OBJECT;struct CHARACTER;
std::uintptr_t Hero;
BYTE* activeObject;
unsigned actionCalls;int requestedAction;
std::vector<std::array<BYTE,5>> packets;
void SetAction(OBJECT* object,int action){Require(reinterpret_cast<BYTE*>(object)==activeObject);++actionCalls;requestedAction=action;activeObject[261]=static_cast<BYTE>(action);}
struct Network{void SendC1(const BYTE* data,int size){Require(size==5);std::array<BYTE,5> packet;std::memcpy(packet.data(),data,5);packets.push_back(packet);}}gNetwork;
'''
MAIN=r'''
void Invoke(BYTE* character,BYTE* object,int action,int kind){SetActionClass(CLASS_ARG,OBJECT_ARG,action,kind);}
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
std::uint64_t Run(BYTE* classMemory,BYTE* objectMemory,BYTE* heroMemory){
    std::uint64_t digest=14695981039346656037ull;
    const int actions[]={-1,0,1,92,93,99,107,113,121,122,123,124,125,126,127,128,129,130,255,256,511};const int kinds[]={-1,0,255,256,0x12345678};
    const float angles[]={-720,-360,-180,-22.5f,-0.0f,0.0f,22.5f,45,90,337.5f,360,720};const BYTE directions[]={2,2,6,1,1,1,2,2,3,1,1,1};
    for(unsigned trial=0;trial<65536;++trial){
        std::memset(classMemory,0xa5,948);std::memset(objectMemory,0x5a,948);std::memset(heroMemory,0x6b,948);
        BYTE* character=classMemory+16;BYTE* object=(trial/2048)%2?character:objectMemory+16;BYTE* hero=trial%17==0?object:heroMemory+16;
        const BYTE cls=static_cast<BYTE>(trial/256),cur=static_cast<BYTE>(trial);
        character[444]=cls;object[261]=cur;hero[444]=cls;hero[261]=cur;
        Put(hero+36,angles[(trial/1024)%12]);Hero=trial%37==0?0:reinterpret_cast<std::uintptr_t>(hero);
        const int action=actions[(trial/256)%21],kind=kinds[(trial/512)%5];const bool allowed=cur>=1&&cur<=12;
        const int wanted=action+((cls&7)==2 && (action<123 || action>128)?1:0);
        BYTE beforeClass[948],beforeObject[948],beforeHero[948];std::memcpy(beforeClass,classMemory,948);std::memcpy(beforeObject,objectMemory,948);std::memcpy(beforeHero,heroMemory,948);
        if(allowed){if(object==character)beforeClass[16+261]=static_cast<BYTE>(wanted);else beforeObject[16+261]=static_cast<BYTE>(wanted);}
        packets.clear();actionCalls=0;activeObject=object;
        Invoke(character,object,action,kind);
        Require(actionCalls==static_cast<unsigned>(allowed) && (!allowed || requestedAction==wanted));
        const std::array<BYTE,5> packet={0xc1,5,0x18,directions[(trial/1024)%12],static_cast<BYTE>(kind)};
        Require(packets.size()==(allowed&&Hero?1u:0u));if(!packets.empty())Require(packets[0]==packet);
        Require(std::memcmp(classMemory,beforeClass,948)==0 && std::memcmp(objectMemory,beforeObject,948)==0 && std::memcmp(heroMemory,beforeHero,948)==0);
        digest^=actionCalls;digest*=1099511628211ull;if(allowed){digest^=static_cast<unsigned>(requestedAction);digest*=1099511628211ull;}
        for(auto p:packets)for(BYTE value:p){digest^=value;digest*=1099511628211ull;}
        if(Hero){
            hero[261]=cur;activeObject=hero;actionCalls=0;packets.clear();Gesture(action,static_cast<BYTE>(kind));
            Require(actionCalls==static_cast<unsigned>(allowed) && packets.size()==(allowed?2u:1u));for(auto p:packets)Require(p==packet);
            digest^=static_cast<unsigned>(packets.size());digest*=1099511628211ull;
        }
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+2048,low+4096);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,6144);Require(Run(mask&1?high:low,(mask&2?high:low)+2048,(mask&4?high:low)+4096)==expected);
            for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<948;++i)Require(low[slot*2048+i]==0x7b);
        }std::puts("ocho combinaciones clase/objeto/heroe altos y bajos; aliases intactos");
    }std::printf("Class action: 65536 casos, elf/gesto/paquete %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/UI/Chat_Gestures.cpp',revision);header=read_source('src/Entity/EntityTypes.h',revision)
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES
    prefix+='\n'.join(re.findall(r'^inline (?:const )?(?:OBJECT|CHARACTER)\* As(?:Object|Character)[^\n]+',header,re.M))
    code=function(source,'SendRequestAction')+function(source,'SetActionClass')+function(source,'Gesture')
    current='CHARACTER* character' in code
    c='reinterpret_cast<CHARACTER*>(character)' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(character))'
    o='reinterpret_cast<OBJECT*>(object)' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(object))'
    return prefix+code+MAIN.replace('CLASS_ARG',c).replace('OBJECT_ARG',o)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-class-action-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
