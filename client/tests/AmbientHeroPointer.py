#!/usr/bin/env python3
"""Emisión ambiental real de MoveObjects con Hero nativo y buckets vacíos."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __stdcall
#endif
struct OBJECT;
void* Hero;int DAT_083a3fec;
char g_ObjectBucketGrid[0x1000];
GRID_ALIAS
HEIGHT
unsigned trial,randomCalls,validatedCalls;std::vector<std::uint32_t> events;
std::uint32_t Bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
int RandomAt(unsigned i){const int values[]={0,1,49,50,299,300,899,900,0x7fffffff};return values[(trial+i*3)%9];}
int ControlledRand(){Require(randomCalls<4 && !validatedCalls && DAT_083a3fec==0x12345678);const int result=RandomAt(randomCalls++);events.insert(events.end(),{3,static_cast<std::uint32_t>(result)});return result;}
#define rand ControlledRand
struct Map{int world;int GetCurrentMap(){events.push_back(1);return world;}}gMapManager;
float MoveHeavenThunder(){events.push_back(2);return float(trial%5);}
int Particle_Spawn(int type,float* position,float* angle,float* light,int flag,float scale,int owner){
 Require(type==0x4e1 && flag==3 && scale==.19f && !owner && !validatedCalls && DAT_083a3fec==0x12345678);
 for(unsigned i=0;i<3;++i)Require(Bits(angle[i])==0 && light[i]==1);
 events.insert(events.end(),{4,Bits(position[0]),Bits(position[1]),Bits(position[2])});return -1;
}
namespace Platform{template<class F>bool RunValidatedOperation(F callback){Require(DAT_083a3fec==0);++validatedCalls;events.push_back(5);callback();return true;}}
namespace SceneObjects{bool Contains(void*){Require(false);return false;}}
OBJECT* AsObject(void* p){return reinterpret_cast<OBJECT*>(p);}
void MoveObject_Special(OBJECT*){Require(false);}
void FUN_004fdc00(OBJECT*){Require(false);}
template<class... A>void CreateSprite(A...){Require(false);}
template<class... A>void Joint_Create(A...){Require(false);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory){
 Hero=memory+16;std::uint64_t hash=14695981039346656037ull;unsigned queries=0,emitted=0;
 for(int world=-1;world<=20;++world)for(trial=0;trial<256;++trial){
  for(unsigned i=0;i<128;++i)memory[i]=static_cast<BYTE>(i*37+trial);
  const float original[]={float(int(trial)-128)*.125f,float(int(trial)*3-512)*.25f,float(int(trial%32)-16)*.5f};
  std::memcpy(static_cast<BYTE*>(Hero)+16,original,12);
  BYTE before[128];std::memcpy(before,memory,128);std::memset(g_ObjectBucketGrid,0,sizeof(g_ObjectBucketGrid));
  gMapManager.world=world;DAT_083a3fec=0x12345678;std::vector<std::uint32_t> wanted={1};unsigned expectedRandom=0;
  if(world==10)wanted.push_back(2);
  else{wanted.push_back(1);if(world>10)wanted.push_back(1);}
  if(world>=11 && world<=16){
   for(unsigned i=0;i<4;++i)wanted.insert(wanted.end(),{3,static_cast<std::uint32_t>(RandomAt(i))});expectedRandom=4;
   if(RandomAt(3)%4==0){
    const float x=static_cast<float>(double(original[0])+(RandomAt(0)%900)-300);
    const float y=static_cast<float>(double(original[1])+(RandomAt(1)%900)-300);
    const float z=static_cast<float>(double(original[2])+(RandomAt(2)%50)+double(_DAT_00552994));
    wanted.insert(wanted.end(),{4,Bits(x),Bits(y),Bits(z)});++emitted;
   }
  }
  wanted.insert(wanted.end(),256,5);events.clear();randomCalls=validatedCalls=0;MoveObjects();
  Require(events==wanted && randomCalls==expectedRandom && validatedCalls==256 && DAT_083a3fec==0 && std::memcmp(memory,before,128)==0);
  for(char c:g_ObjectBucketGrid)Require(c==0);
  for(auto value:events){hash^=value;hash*=1099511628211ull;}
  for(unsigned i=0;i<128;++i){hash^=memory[i];hash*=1099511628211ull;}++queries;
 }
 Require(queries==5632 && emitted==510);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("Hero alto; posición de partículas, buckets vacíos y alias intactos");}
 std::printf("Ambient hero: 5632 casos/510 emisiones por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Scene/Scene_ObjectUpdate.cpp',revision)
    # Adapta la extracción sin cambiar el __stdcall del cuerpo compilado.
    move=function(source.replace('void __stdcall MoveObjects','void __cdecl MoveObjects'),'MoveObjects').replace('void __cdecl MoveObjects','void __stdcall MoveObjects')
    # Tres casts de owner ajenos a esta causa siguen en producción; sus ramas
    # no se ejecutan con buckets vacíos. Se conserva su truncamiento para GCC.
    for expression,count in (('(int)Hero',1),('(int)pcVar6',2)):
        assert move.count(expression)==count
        pointer=expression[len('(int)'):]
        move=move.replace(expression,'static_cast<int>(reinterpret_cast<std::uintptr_t>('+pointer+'))')
    header=read_source('src/globals.h',revision)
    alias=re.search(r'^#define DAT_083a021c\s+[^\n]+',header,re.M).group(0)
    height=re.search(r'^float\s+_DAT_00552994\s*=\s*[^;]+;',read_source('src/globals.cpp',revision),re.M).group(0)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('GRID_ALIAS',alias).replace('HEIGHT',height)+move+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-hero-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
