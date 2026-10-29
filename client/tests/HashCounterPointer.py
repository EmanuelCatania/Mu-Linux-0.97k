#!/usr/bin/env python3
"""Contadores del nodo compartido: MapTick completo y bloques reales de intro/conexión."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using uint=unsigned;using undefined4=std::uint32_t;using undefined1=BYTE;using byte=BYTE;
void* DAT_07cf1ffc;int MAIN_HASH_CLASS;char DAT_07e11d6e,DAT_07eaa134;DWORD DAT_07eaa13c,DAT_07eaa160,DAT_083a427c,DAT_083a4278,DAT_07ea5284,DAT_07ea5288,DAT_07ea840c,DAT_07ea8408,DAT_07ea9844,DAT_0055a3e4;char DAT_07e91350[68];
#define MouseX DAT_083a427c
#define InventoryStartX DAT_07ea5288
float _DAT_00552cac=100,_DAT_00552c24=200,_DAT_00552c14=50,_DAT_00552ca8=150;
BYTE *node,*memory;void* wantedKey;unsigned wantedOffset,factoryCalls;bool missing;int rightEdge;std::vector<DWORD> events;
namespace Ui{constexpr int WindowRight=1;}
struct Window{int PanelOrigin(int id){Require(id==Ui::WindowRight);events.push_back(7);return rightEdge;}}gWindow;
void GL_ResetState(){events.push_back(0);}uint HashTable_GetIndex(void* context,void* key){Require(context==&MAIN_HASH_CLASS && key==DAT_07cf1ffc);events.push_back(1);return missing?0xffffffffu:3;}
void* AntiTamper_HashNode(){++factoryCalls;events.push_back(2);return node;}
void HashTable_Insert(void* context,void* value,void* key){Require(context==&MAIN_HASH_CLASS && value==node && key==wantedKey && node[wantedOffset]==1);events.push_back(3);}
DWORD Number(void* p){return static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(p));}
void RenderItemInfo(void* a,void* b,void* c,int d){events.push_back(4);events.push_back(Number(a));events.push_back(Number(b));events.push_back(c==DAT_07e91350?0xffffffffu:Number(c));events.push_back(static_cast<DWORD>(d));}
void RenderRepairInfo(void* a,int b,void* c){events.push_back(5);events.push_back(Number(a));events.push_back(static_cast<DWORD>(b));events.push_back(Number(c));}
void RenderSkillTooltip(float y,int x,int skill){DWORD bits;std::memcpy(&bits,&y,4);events.push_back(6);events.push_back(bits);events.push_back(static_cast<DWORD>(x));events.push_back(skill);}
'''
MAIN=r'''
void Hash(std::uint64_t& h,const BYTE* p,unsigned n){for(unsigned i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* buffer,BYTE* keys){memory=buffer;node=buffer+16;DAT_07cf1ffc=keys+16;std::uint64_t hash=14695981039346656037ull;const DWORD xs[]={0,99,100,199,200,319,320,639,640,799,800},ys[]={0,49,50,149,150,199,200,479,480},skills[]={0xffffffffu,0,1,19,20,0x7fffffffu,0x80000000u},yBits[]={0,0x80000000u,0x41c80000,0x7fc12345};
 for(unsigned trial=0;trial<8192;++trial){std::memset(buffer,0xa5,1445);std::memset(keys,0x6d,1444);BYTE expectedMemory[1445],beforeKeys[1444];std::memcpy(expectedMemory,buffer,1445);std::memcpy(beforeKeys,keys,1444);DAT_07e11d6e=static_cast<char>(0xff);DAT_07eaa134=static_cast<char>(trial%4==0?0:trial%4==1?1:trial%4==2?2:-1);DAT_07eaa13c=(trial/3)%3;DAT_07eaa160=(trial/7)%3;DAT_083a427c=xs[(trial/5)%11];DAT_083a4278=ys[(trial/11)%9];DAT_07ea5284=(trial/13)%2?200:0;DAT_07ea5288=(trial/17)%2?320:0;DAT_07ea840c=yBits[(trial/19)%4];DAT_07ea8408=trial*12345u;DAT_07ea9844=trial%3==0?0:trial%3==1?1:0xffffffffu;DAT_0055a3e4=skills[(trial/23)%7];rightEdge=(trial/29)%2?640:800;missing=trial%2;wantedKey=DAT_07cf1ffc;wantedOffset=1412;factoryCalls=0;std::vector<DWORD> expected={0};const char normalized=DAT_07eaa134?1:0;bool render=false,repair=false;
  if(!DAT_07eaa160)render=DAT_07eaa13c==1 && DAT_083a427c>=100 && DAT_083a427c<200 && DAT_083a4278>=50 && DAT_083a4278<150;else{render=true;bool outside=static_cast<int>(DAT_083a427c)<static_cast<int>(DAT_07ea5288);if(!outside){expected.push_back(7);outside=static_cast<int>(DAT_083a427c)>=rightEdge;}repair=!outside && DAT_083a4278>=DAT_07ea5284 && DAT_083a4278<=479 && normalized;}
  if(render){expected.push_back(1);if(missing){expected.push_back(2);expected.push_back(3);expectedMemory[16+1412]=1;}if(!DAT_07eaa160)expected.insert(expected.end(),{4,0x109,0x78,0xffffffffu,0});else if(repair)expected.insert(expected.end(),{5,DAT_07ea840c,DAT_07ea8408,DAT_07eaa160});else expected.insert(expected.end(),{4,DAT_07ea840c,DAT_07ea8408,DAT_07eaa160,DAT_07ea9844});}if(DAT_0055a3e4<20)expected.insert(expected.end(),{6,DAT_07ea840c,DAT_07ea8408,DAT_0055a3e4});events.clear();Scene_MapTick();Require(events==expected && DAT_07e11d6e==0 && DAT_07eaa134==normalized && factoryCalls==unsigned(render && missing) && std::memcmp(buffer,expectedMemory,1445)==0 && std::memcmp(keys,beforeKeys,1444)==0);Hash(hash,buffer,1445);for(auto value:events)Hash(hash,reinterpret_cast<BYTE*>(&value),4);
  for(unsigned block=0;block<4;++block){std::memset(buffer,0x6c,1445);std::memcpy(expectedMemory,buffer,1445);wantedOffset=block<2?4:1;expectedMemory[16+wantedOffset]=1;factoryCalls=0;events.clear();Block(block,DAT_07cf1ffc);Require(factoryCalls==1 && events==std::vector<DWORD>({2,3}) && std::memcmp(buffer,expectedMemory,1445)==0 && std::memcmp(keys,beforeKeys,1444)==0);Hash(hash,buffer,1445);}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){if(mask&1)std::memset(low,0x7b,1445);if(mask&2)std::memset(low+4096,0x7b,1444);Require(Run((mask&1)?high:low,(mask&2)?high+4096:low+4096)==expected);if(mask&1)for(unsigned i=0;i<1445;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<1444;++i)Require(low[4096+i]==0x7b);}std::puts("nodo/clave altos; bytes de contador, ramas/eventos y aliases intactos");}std::printf("Hash counter: 8192 ticks/32768 bloques %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    wrappers=[]
    for path,var in [('src/Scene/Scene_Intro.cpp','pvVar5'),('src/Net/Net_Connect.cpp','pvVar4')]:
        source=read_source(path,revision)
        pattern=rf'{var}\s*=\s*AntiTamper_HashNode\(\);\s*[^\n]+\n\s*HashTable_Insert\([^\n]+;'
        blocks=re.findall(pattern,source)
        if len(blocks)!=2:raise ValueError(f'{path}: se esperaban dos bloques, hay {len(blocks)}')
        for block in blocks:
            n=len(wrappers)
            setup='undefined4* puVar1=static_cast<undefined4*>(key);wantedKey=key;' if var=='pvVar5' else ('byte m_SendSerial=0,m_ReceiveSerial=0;wantedKey='+('&m_SendSerial;' if n==2 else '&m_ReceiveSerial;'))
            wrappers.append(f'void Block{n}(void* key){{void* {var};{setup}{block}}}')
    dispatcher='void Block(unsigned which,void* key){switch(which){'+''.join(f'case {n}:Block{n}(key);break;' for n in range(4))+'}}'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Scene/Scene_MapTick.cpp',revision),'Scene_MapTick')+'\n'.join(wrappers)+dispatcher+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-hash-counter-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
