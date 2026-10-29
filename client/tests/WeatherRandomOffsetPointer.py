#!/usr/bin/env python3
"""Emisión real de lluvia/nubes con desplazamientos numéricos, RNG controlado."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using uint=unsigned int;
char* DAT_07abf5d8;int DAT_083a3ff0;float DAT_081cb608[1];
const float _DAT_00552900=300.0f,_DAT_00552ab8=1000.0f;
unsigned trial,randomAt;std::vector<DWORD> actual;
DWORD Bits(float f){DWORD b;std::memcpy(&b,&f,4);return b;}
int Random(unsigned n){return n%3==0 ? (trial%4==0?0:trial%4==1?1:trial%4==2?40:10) : trial==5000?0x7fffffff:int(trial+n*997);}
int ControlledRand(){Require(randomAt<7);int r=Random(randomAt++);actual.insert(actual.end(),{1,DWORD(r)});return r;}
struct Map{int world;int GetCurrentMap(){actual.push_back(2);return world;}}gMapManager;
void CreateEffect(int type,float* p,float* a,float* l,float* sub,float* owner,float* key,float* extra,int flag){
 Require(!owner&&!extra&&!flag&&reinterpret_cast<std::uintptr_t>(key)==0xffffffffu);
 actual.insert(actual.end(),{3,DWORD(type),DWORD(reinterpret_cast<std::uintptr_t>(sub))});
 for(float* v:{p,a,l})for(int i=0;i<3;++i)actual.push_back(Bits(v[i]));
}
void PlayBuffer(int id,int owner,int flag){Require(!owner&&!flag);actual.insert(actual.end(),{4,DWORD(id)});}
void AddTerrainLight(float x,float y,float* c,int radius,float* light){Require(light==DAT_081cb608);actual.insert(actual.end(),{5,Bits(x),Bits(y),Bits(c[0]),Bits(c[1]),Bits(c[2]),DWORD(radius)});}
#define _rand ControlledRand
void Emit(){int iVar12;uint uVar8;DWORD* puStack_44;FRAME
BLOCK
}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory){
 DAT_07abf5d8=reinterpret_cast<char*>(memory+16);
 std::uint64_t hash=14695981039346656037ull;unsigned count=0,coverage=0;
 for(int world:{-1,0,10,11})for(int rain:{0,1})for(trial=0;trial<=5000;++trial){
  for(unsigned i=0;i<128;++i)memory[i]=BYTE(i*37+trial);
  const float pos[]={float(int(trial)-2500)*.125f,float(int(trial)*3-512)*.25f,float(int(trial%32)-16)*.5f};
  std::memcpy(DAT_07abf5d8+16,pos,12);BYTE before[128];std::memcpy(before,memory,128);
  gMapManager.world=world;DAT_083a3ff0=rain;randomAt=0;actual.clear();std::vector<DWORD> wanted;unsigned used=0;
  auto random=[&](){int r=Random(used++);wanted.insert(wanted.end(),{1,DWORD(r)});return r;};
  auto effect=[&](int type,int sub,float x,float y,float z){wanted.insert(wanted.end(),{3,DWORD(type),DWORD(sub),Bits(x),Bits(y),Bits(z),0,0,0,Bits(1),Bits(1),Bits(1)});};
  if(rain){
   if(random()%40==0){int x=random()%600-200;int y=random()%400+200;effect(191,3,float(x)+pos[0],float(y)+pos[1],pos[2]+300.0f);wanted.insert(wanted.end(),{4,46});coverage|=1;}
   else coverage|=2;
   wanted.insert(wanted.end(),{5,Bits(pos[0]),Bits(pos[1]),Bits(-.3f),Bits(-.3f),Bits(-.2f),16});
  }
  wanted.push_back(2);
  if(world==10){if(random()%10==0){int x=random()%5000-2500;int y=random()%5000-2500;effect(1150,0,float(x)+pos[0],float(y)+pos[1],pos[2]-1000.0f);coverage|=4;}else coverage|=8;}
  Emit();Require(actual==wanted&&randomAt==used&&std::memcmp(memory,before,128)==0);
  for(DWORD value:actual){hash^=value;hash*=1099511628211ull;}++count;
 }
 Require(count==40008&&coverage==15);
 std::printf("Weather random offsets: %u casos;RNG,orden,efectos,luz y owner readonly %016llx\n",count,static_cast<unsigned long long>(hash));
 return hash;
}
int main(){
 BYTE* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){
  BYTE* signBit=static_cast<BYTE*>(Reserve(0x8e000000u));Require(signBit);Require(Run(signBit)==expected);
  BYTE* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);
  std::memset(low,0x7b,128);Require(Run(high)==expected);
  for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);
  std::puts("Hero bajo,bit31 y alto;alias inferior y128bytes del owner intactos");
 }
}
'''
def harness(revision=None):
    source=read_source('src/Render/Weather.cpp',revision)
    begin=source.index('\n',source.index('// ── Weather-mode:'))+1
    end=source.index('    // Get terrain tile',begin)
    frame=re.search(r'    float\s+__fr\[12\][\s\S]*?&fStack_c\s*= __fr\[11\];',source).group()
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('FRAME',frame).replace('BLOCK',source[begin:end])+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-weather-offset-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
