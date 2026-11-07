#!/usr/bin/env python3
"""Dos condiciones reales de terreno ambiental; tablas completas y cortocircuito."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using std::uintptr_t;
using Mapping=unsigned char[MAP_SIZE];Mapping* mapping;
#define TerrainMappingLayer1 (*mapping)
char* DAT_0838bc70;
bool Spawn(int iVar7){return (SPAWN);}
bool Transition(short sVar5,int iVar8){return (TRANSITION);}
'''
MAIN=r'''
constexpr unsigned size=65536+32;
std::uint64_t Run(BYTE* mapMemory,BYTE* wallMemory){
 for(unsigned i=0;i<size;++i){mapMemory[i]=BYTE(i*37+5);wallMemory[i]=BYTE(i*13+7);}
 mapping=reinterpret_cast<Mapping*>(mapMemory+16);DAT_0838bc70=reinterpret_cast<char*>(wallMemory+16);
 std::vector<BYTE> mapBefore(mapMemory,mapMemory+size),wallBefore(wallMemory,wallMemory+size);
 const short types[]={-32768,-1,0,177,178,181,191,32767};const BYTE values[]={0,5,8,255},walls[]={0,1,5,7,8,127,128,255};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 auto check=[&](unsigned index,BYTE layer,BYTE wall,short type){
  (*mapping)[index]=layer;DAT_0838bc70[index]=char(wall);mapBefore[index+16]=layer;wallBefore[index+16]=wall;
  const bool a=Spawn(int(index)),b=Transition(type,int(index));
  Require(a==(layer==5)&&b==((type==181&&layer!=5)||(type==177&&wall>=8))&&(*mapping)[index]==layer&&BYTE(DAT_0838bc70[index])==wall);
  coverage|=a?1:2;coverage|=b?4:8;hash^=unsigned(a)+2*unsigned(b);hash*=1099511628211ull;++cases;
 };
 for(unsigned index=0;index<65536;++index)for(unsigned profile=0;profile<4;++profile)for(short type:types)check(index,values[(index+profile)%4],values[(index+profile*3)%4],type);
 for(unsigned index:{0u,1u,255u,256u,32767u,32768u,65534u,65535u})for(unsigned layer=0;layer<256;++layer)for(BYTE wall:walls)for(short type:types)check(index,BYTE(layer),wall,type);
 Require(cases==2228224&&coverage==15&&std::memcmp(mapMemory,mapBefore.data(),size)==0&&std::memcmp(wallMemory,wallBefore.data(),size)==0);
 // Las tablas no pertinentes nunca deben evaluarse.
 mapping=nullptr;DAT_0838bc70=nullptr;Require(!Transition(178,2147483647));
 DAT_0838bc70=reinterpret_cast<char*>(wallMemory+16);for(unsigned wall=0;wall<256;++wall){DAT_0838bc70[0]=char(wall);Require(Transition(177,0)==(wall>=8));}DAT_0838bc70[0]=char(wallBefore[16]);
 mapping=reinterpret_cast<Mapping*>(mapMemory+16);DAT_0838bc70=nullptr;for(unsigned layer=0;layer<256;++layer){(*mapping)[0]=BYTE(layer);Require(Transition(181,0)==(layer!=5));}(*mapping)[0]=mapBefore[16];
 Require(std::memcmp(mapMemory,mapBefore.data(),size)==0&&std::memcmp(wallMemory,wallBefore.data(),size)==0);
 return hash;
}
BYTE* Table(uintptr_t base){for(unsigned i=0;i<2;++i)Require(Reserve(base+i*0x10000)==reinterpret_cast<void*>(base+i*0x10000));return reinterpret_cast<BYTE*>(base);}
int main(){
 auto* low=Table(0x26000000u);auto* wall=Table(0x28000000u);const auto expected=Run(low,wall);
 if(sizeof(void*)>4){
  auto* high=Table(uintptr_t(0x126000000ull));auto* highWall=Table(uintptr_t(0x128000000ull));
  std::memset(low,0x7b,size);Require(Run(high,wall)==expected);for(unsigned i=0;i<size;++i)Require(low[i]==0x7b);
  std::memset(wall,0x6d,size);Require(Run(low,highWall)==expected);for(unsigned i=0;i<size;++i)Require(wall[i]==0x6d);
  std::memset(low,0x7b,size);std::memset(wall,0x6d,size);Require(Run(high,highWall)==expected);for(unsigned i=0;i<size;++i)Require(low[i]==0x7b&&wall[i]==0x6d);
  Require(Run(Table(0x8e000000u),Table(0x90000000u))==expected);
  std::puts("tablas bajas,altas ybit31;aliases,canarios ycortocircuito intactos");
 }
 std::printf("Ambient terrain:2228224 pares/ruta %016llx\n",(unsigned long long)expected);
}
'''
def harness(revision=None):
    source=read_source('src/Render/Ambient_Particles.cpp',revision)
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S);matches=[]
    for start in re.finditer(r'\bif\s*\(',source):
        first=start.end();end=first;depth=1
        while depth:depth+=(source[end]=='(')-(source[end]==')');end+=1
        expression=source[first:end-1]
        if 'TerrainMappingLayer1' in expression:matches.append(expression)
    if len(matches)!=2:raise ValueError('Condiciones de terreno inesperadas')
    header=read_source('src/globals.h',revision);count=re.search(r'extern unsigned char\s+TerrainMappingLayer1\[([^\]]+)\]',header).group(1)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('MAP_SIZE',count).replace('SPAWN',matches[0]).replace('TRANSITION',matches[1])+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-terrain-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
