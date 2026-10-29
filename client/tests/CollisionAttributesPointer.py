#!/usr/bin/env python3
"""Colisión real: tabla de skills alta, objetivo y entidades legado bajos.

RNG, paquetes, audio y emisiones controlados. No valida owners/layouts
nativos, red real ni ejecución completa de efectos/joints.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
using std::uintptr_t;
void *DAT_07cf1ff4,*DAT_07abf5d8,*DAT_07abf5d0;
BYTE *memory,*target;BYTE before[512],expected[512];unsigned rngCalls,skillIndex;int firstRand,soundRand;bool packetHit;
std::vector<int> events;
void* AsObject(int value){return reinterpret_cast<void*>(static_cast<uintptr_t>(value));}
void Final(){Require(std::memcmp(memory,expected,512)==0);}
int ControlledRand(){Require(rngCalls<2);events.push_back(1);if(rngCalls++==0){Require(std::memcmp(memory,before,512)==0);return firstRand;}Final();return soundRand;}
bool Entity_FindNearby_SendPacket(unsigned skill,float* p,float radius,BYTE mode,short id){
 Require(skill==skillIndex && reinterpret_cast<BYTE*>(p)==target+16 && radius==100.0f && mode==target[136] && id==*reinterpret_cast<short*>(target+134) && std::memcmp(memory,before,512)==0);events.push_back(2);return packetHit;
}
void Joint_Create(int type,float* a,float* b,float* angle,int kind,int owner,float scale,int key,int flags){
 Require(type==1249 && reinterpret_cast<BYTE*>(a)==target+16 && a==b && reinterpret_cast<BYTE*>(angle)==target+28 && kind==6 && owner==int(reinterpret_cast<uintptr_t>(target)) && scale==30.0f && key==-1 && flags==0);Final();events.push_back(3);
}
void Effect_SpawnSmokeBurst(float* p,char flag){Require(reinterpret_cast<BYTE*>(p)==target+16 && flag==1);Final();events.push_back(4);}
void PlayBuffer(int sound,void* object,int mode){Require(sound==soundRand%7+50 && object==target && mode==0);Final();events.push_back(100+sound);}
'''
MAIN=r'''
template<class T>void Put(BYTE* base,unsigned offset,T value){std::memcpy(base+offset,&value,sizeof(value));}
constexpr unsigned poolBytes=400*916+32;
std::uint64_t Run(BYTE* allocation,BYTE* attributes,BYTE* poolStorage){
 memory=allocation;target=allocation+16;BYTE* pool=poolStorage+16;DAT_07abf5d0=pool;DAT_07abf5d8=pool+916;
 std::memset(poolStorage,0,poolBytes);
 for(unsigned i=0;i<6;++i){BYTE* entity=pool+i*916;entity[0]=i==2?0:1;entity[352]=i==3?0:1;entity[765]=i==4?1:0;Put(entity,16,123.5f+(i==5?101.0f:0.0f));Put(entity,20,-42.5f);}
 const BYTE skills[]={0,50,51,52,53,127,128,255};const short types[]={223,243,17,-1,32767};const int randoms[]={0,1,static_cast<int>(0x80000000u),-1};
 const int sounds[]={0,6,7,14,0x7fffffff,-1,static_cast<int>(0x80000000u),99};
 const int cooldowns[]={-1,0,1,0x7fffffff};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned profile=0;profile<8;++profile)for(skillIndex=0;skillIndex<256;++skillIndex)for(unsigned scenario=0;scenario<16;++scenario){
  for(unsigned i=0;i<512;++i){memory[i]=static_cast<BYTE>(profile+i*37);attributes[i]=static_cast<BYTE>(profile+i*13);}
  attributes[87+skillIndex]=skills[profile];BYTE tableBefore[512];std::memcpy(tableBefore,attributes,512);
  DAT_07cf1ff4=scenario==1?nullptr:attributes;
  const bool directed=scenario<10;const short type=scenario==6||scenario==7?243:scenario==8?223:types[(profile+scenario)%5];const int subtype=scenario==6||scenario==7||scenario==8||scenario==11||scenario==12?2:0;
  const int cooldown=scenario==4?cooldowns[2+profile%2]:cooldowns[profile%2];
  firstRand=scenario==2?randoms[1+2*(profile%2)]:scenario==9?randoms[2]:0;soundRand=sounds[profile];packetHit=scenario!=5;
  const int owner=static_cast<int>(reinterpret_cast<uintptr_t>(scenario==3||!directed?pool:static_cast<BYTE*>(DAT_07abf5d8)));
  memory[16]=1;Put(target,2,type);Put(target,4,subtype);Put(target,16,123.5f);Put(target,20,-42.5f);Put(target,24,19.25f);
  target[132]=directed?1:0;target[133]=static_cast<BYTE>(skillIndex);Put(target,134,short(0x7f00|skillIndex));target[136]=static_cast<BYTE>(profile*31);Put(target,96,scenario==7?15:14);Put(target,244,cooldown);Put(target,252,owner);
  // La rama AOE salta dueño,héroe,inactivo,invisible,muerto y distancia101.
  // Los candidatos pueden aparecer al principio o al final del pool.
  for(unsigned index: {6u,399u}){BYTE* entity=pool+index*916;entity[0]=0;entity[352]=1;entity[765]=0;Put(entity,16,223.5f);Put(entity,20,-42.5f);}
  const bool found=scenario>=11 && scenario!=14;
  if(scenario>=11){BYTE* entity=pool+(scenario==15?399:6)*916;entity[0]=1;Put(entity,16,scenario==14?224.5f:223.5f);}
  std::vector<BYTE> poolBefore(poolStorage,poolStorage+poolBytes);
  std::memcpy(before,memory,512);std::memcpy(expected,memory,512);std::vector<int> expectedEvents;unsigned expectedRng=0;
  // Oráculo de acciones: guarda, decremento, joint, baja, humo y sonido.
  unsigned action=0;int nextCooldown=cooldown;
  if(scenario>1){
   if(directed){
    expectedEvents.push_back(1);++expectedRng;
    if((static_cast<unsigned>(firstRand)&1u)==0 && scenario!=3){
     if(skills[profile]==52 && cooldown>0){action=1;nextCooldown=cooldown-1;}
     else {
      expectedEvents.push_back(2);
      if(packetHit){
       if(skills[profile]==52){
        if(subtype==2){action=2;nextCooldown=(type==223||(type==243&&scenario==7))?5:2;}
        else action=4;
       }else if(skills[profile]==51)action=type==223||type==243?4:3;
       else {expectedEvents.push_back(1);++expectedRng;action=type==223||type==243?6:5;}
      }
     }
    }
   }else if(found){
    if(skills[profile]==52)action=subtype==2?2:4;
    else action=type==223||type==243?4:3;
   }
  }
  if(action==1||action==2)Put(expected+16,244,nextCooldown);
  if(action>=3)expected[16]=0;
  if(action==2)expectedEvents.push_back(3);
  if(action==5||action==6)expectedEvents.push_back(100+soundRand%7+50);
  if(action==4||action==6)expectedEvents.push_back(4);
  events.clear();rngCalls=0;Effect_CollisionCheck(scenario==0?0:static_cast<int>(reinterpret_cast<uintptr_t>(target)));
  Final();Require(events==expectedEvents && rngCalls==expectedRng && std::memcmp(attributes,tableBefore,512)==0 && std::memcmp(poolStorage,poolBefore.data(),poolBytes)==0);
  coverage|=1u<<action;
  if(action==2 && directed && nextCooldown==5)coverage|=128;if(action==2 && directed && nextCooldown==2)coverage|=256;
  if(!directed && found)coverage|=512;if(!directed && !found)coverage|=1024;
  for(BYTE value:expected){hash^=value;hash*=1099511628211ull;}for(int event:events){hash^=unsigned(event);hash*=1099511628211ull;}++cases;
 }
 Require(cases==32768 && coverage==2047);return hash;
}
int main(){
 auto* targetStorage=static_cast<BYTE*>(Reserve(0x26000000u));auto* low=static_cast<BYTE*>(Reserve(0x28000000u));auto* pool=static_cast<BYTE*>(Reserve(0x2a000000u));Require(targetStorage&&low&&pool);
 const auto expected=Run(targetStorage,low,pool);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x128000000ull)));Require(high);std::memset(low,0x7b,512);Require(Run(targetStorage,high,pool)==expected);for(unsigned i=0;i<512;++i)Require(low[i]==0x7b);std::puts("atributos altos; ramas dirigida/AOE, bytes y alias intactos");}
 std::printf("Collision attributes: 32768 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Render/SMD_Parser.cpp',revision)
    preamble=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x80000')
    return preamble+SERVICES+'\n#define rand ControlledRand\n'+function(source,'Effect_CollisionCheck')+'\n#undef rand\n'+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-collision-attributes-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
