#!/usr/bin/env python3
"""Actualización especial real: objeto nativo, estados y partículas controladas."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
using WORD=std::uint16_t;
#endif
struct OBJECT;
GLOBALS
ALIAS
BYTE* object;unsigned trial,randomCalls;std::vector<std::uint32_t> events;
std::uint32_t Bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
int RandomAt(unsigned index){const int values[]={0,1,19,20,149,150,299,300,32767};return values[(trial+index)%9];}
int ControlledRand(){Require(randomCalls<20);const int result=RandomAt(randomCalls++);events.insert(events.end(),{3,static_cast<std::uint32_t>(result)});return result;}
#define rand ControlledRand
struct Map{int world;int GetCurrentMap(){events.insert(events.end(),{1,static_cast<std::uint32_t>(world)});return world;}}gMapManager;
void PlayBuffer(int sound,int owner,int mode){Require(sound==108 && !owner && !mode);events.push_back(2);}
int Particle_Spawn(int type,float* position,float* angle,float* light,int flag,float alpha,int mode){
 Require(type==1221 && reinterpret_cast<BYTE*>(angle)==object+28 && reinterpret_cast<BYTE*>(light)==object+232 && !flag && alpha==1 && !mode);
 events.insert(events.end(),{4,Bits(position[0]),Bits(position[1]),Bits(position[2]),Bits(angle[0]),Bits(light[0]),Bits(light[1]),Bits(light[2])});return -1;
}
CLEAR
void ClearActionObject(){events.push_back(5);ActualClear();}
void AddTerrainAttributeRange(int x,int y,int width,int height,unsigned attr,int mode){
 Require(x==13 && y==70 && width==3 && height==6 && attr==8 && !mode && DAT_0055a7b0==-1 && DAT_0055a7b4==-1 && DAT_0055a7b8==-1 && DAT_0055a7bc==-1);
 events.push_back(6);
}
'''
MAIN=r'''
void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}
void PutInt(BYTE* p,int value){std::memcpy(p,&value,4);}
void Update(BYTE* p){MoveObject_Special(ARG);}
std::uint64_t Run(BYTE* memory){
 object=memory+16;std::uint64_t hash=14695981039346656037ull;unsigned queries=0,coverage=0;
 const int worlds[]={-1,0,3},targets[]={-1,0,9,10,160,161,32767},timers[]={-1,0,1,10,20,21,100};
 const short types[]={-32768,0,9,10,160,161,32767};
 const float angles[]={0,35,75,79,80,89,90,95,100},speeds[]={-1,0,1.5f,2,10,15};
 for(int world:worlds)for(unsigned match=0;match<2;++match)for(int target:targets)for(int timer:timers)for(short type:types)for(float angle:angles)for(float speed:speeds){
  trial=queries;for(unsigned i=0;i<320;++i)memory[i]=static_cast<BYTE>(i*37+trial);
  std::memcpy(object+2,&type,2);PutFloat(object+28,angle);const float position[]={float(target)*.125f,float(type)*.25f,-12.5f},light[]={.25f,.5f,1};
  std::memcpy(object+16,position,12);std::memcpy(object+232,light,12);
  DAT_0055a7b0=target;DAT_0055a7b4=world;DAT_0055a7b8=timer;DAT_0055a7bc=speed;gMapManager.world=match?world:world+1;
  BYTE expected[320];std::memcpy(expected,memory,320);std::vector<std::uint32_t> wanted;
  int nextTarget=target,nextWorld=world,nextTimer=timer;float nextSpeed=speed;unsigned wantedRandom=0;
  const bool guards=world>=0 && target>=0 && timer>=0;
  if(!guards)coverage|=1;
  if(guards){
   wanted.insert(wanted.end(),{1,static_cast<std::uint32_t>(gMapManager.world)});
   if(!match)coverage|=2;
   if(match){
    if(type==9 || type==10){
     if(timer==0){PutInt(expected+16+88,-1);const WORD key=4;std::memcpy(expected+16+134,&key,2);coverage|=4;}
    }else if(type==target){
     float nextAngle=angle;
     if(timer==20){nextAngle=35;PutInt(expected+16+88,-1);wanted.push_back(2);coverage|=8;}
     nextAngle=static_cast<float>(double(nextAngle)+speed);nextSpeed=static_cast<float>(double(speed)+1.5);
     if(nextAngle>=90){
      nextAngle=static_cast<float>(double(nextAngle)-timer);nextSpeed=2;coverage|=16;
      if(nextAngle==80){
       coverage|=32;
       for(unsigned i=0;i<10;++i){
        const int rx=RandomAt(wantedRandom++),ry=RandomAt(wantedRandom++);
        wanted.insert(wanted.end(),{3,static_cast<std::uint32_t>(rx),3,static_cast<std::uint32_t>(ry)});
        const float x=static_cast<float>(double(position[0])+(rx%300)-150),y=static_cast<float>(double(position[1])-(ry%20)-600);
        wanted.insert(wanted.end(),{4,Bits(x),Bits(y),Bits(position[2]),Bits(nextAngle),Bits(light[0]),Bits(light[1]),Bits(light[2])});
       }
      }
     }
     if(timer==0){
      PutInt(expected+16+88,-2);nextAngle=90;wanted.insert(wanted.end(),{5,6});
      nextTarget=nextWorld=-1;nextTimer=-1;nextSpeed=-1;coverage|=64;
     }
     --nextTimer;PutFloat(expected+16+28,nextAngle);
    }
   }
  }
  events.clear();randomCalls=0;Update(object);
  Require(events==wanted && randomCalls==wantedRandom && DAT_0055a7b0==nextTarget && DAT_0055a7b4==nextWorld && DAT_0055a7b8==nextTimer && Bits(DAT_0055a7bc)==Bits(nextSpeed) && std::memcmp(memory,expected,320)==0);
  for(unsigned i=0;i<320;++i){hash^=memory[i];hash*=1099511628211ull;}
  for(auto value:events){hash^=value;hash*=1099511628211ull;}
  for(auto value: {std::uint32_t(nextTarget),std::uint32_t(nextWorld),std::uint32_t(nextTimer),Bits(nextSpeed)}){hash^=value;hash*=1099511628211ull;}
  ++queries;
 }
 Require(queries==111132 && coverage==127);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,320);Require(Run(high)==expected);for(unsigned i=0;i<320;++i)Require(low[i]==0x7b);std::puts("objeto alto; estados, partículas, limpieza real y alias intactos");}
 std::printf("Special object: 111132 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    globals_source=read_source('src/globals.cpp',revision)
    definitions='\n'.join(re.search(r'^(?:int|float)\s+'+name+r'\s*=\s*[^;]+;',globals_source,re.M).group(0) for name in ('DAT_0055a7b0','DAT_0055a7b4','DAT_0055a7b8','_DAT_0055a7bc'))
    alias=re.search(r'^#define DAT_0055a7bc\s+[^\n]+',read_source('src/globals.h',revision),re.M).group(0)
    clear=function(read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision),'ClearActionObject').replace('ClearActionObject','ActualClear')
    special=function(read_source('src/Render/Render_LegacyLinker.cpp',revision),'MoveObject_Special')
    arg='reinterpret_cast<OBJECT*>(p)' if 'OBJECT* object' in special else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(p))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('GLOBALS',definitions).replace('ALIAS',alias).replace('CLEAR',clear)+special+MAIN.replace('ARG',arg)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-special-object-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
