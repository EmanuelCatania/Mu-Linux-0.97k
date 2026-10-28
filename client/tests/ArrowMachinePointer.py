#!/usr/bin/env python3
"""CreateArrow/CreateArrows reales: base CharacterMachine nativa; efectos controlados."""
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
using byte=BYTE;
void* DAT_07cf1ffc;BYTE* DAT_07abf5d8;DWORD CurrentSkill;
CONSTANTS
struct OBJECT{};
struct CharacterView{struct Part{short Type;}Weapon[2];}character;
DWORD characterAddress,objectAddress;BYTE serialBefore;WORD wantedIndex,wantedKey,wantedSkill,wantedPK;bool aliases;
std::vector<std::uint32_t> events;
std::uint32_t Bits(float v){std::uint32_t x;std::memcpy(&x,&v,4);return x;}
CharacterView* AsCharacter(DWORD address){Require(address==characterAddress);return &character;}
OBJECT* AsObject(DWORD address){Require(address==objectAddress);return reinterpret_cast<OBJECT*>(static_cast<std::uintptr_t>(address));}
int Behavior(int type){return aliases && type==999?136:aliases && type==888?128:type;}
void CheckSerial(){Require(static_cast<BYTE*>(DAT_07cf1ffc)[1408]==serialBefore);}
struct Catalog{int GetItemBehavior(int type){CheckSerial();events.insert(events.end(),{0,static_cast<std::uint32_t>(type)});return Behavior(type);}}gContentCatalog;
void PlayBuffer(int sound,OBJECT* object,int mode){CheckSerial();Require(object==AsObject(objectAddress) && !mode);events.insert(events.end(),{1,static_cast<std::uint32_t>(sound)});}
float* CreateEffect(int type,float* pos,float* angle,float* light,float* flags,float* owner,float* pk,float* index,BYTE skill){
 CheckSerial();const auto base=static_cast<std::uintptr_t>(objectAddress);
 Require(reinterpret_cast<std::uintptr_t>(pos)==base+16 && reinterpret_cast<std::uintptr_t>(angle)==base+28 && reinterpret_cast<std::uintptr_t>(light)==base+232 && reinterpret_cast<std::uintptr_t>(owner)==base && reinterpret_cast<std::uintptr_t>(flags)==unsigned((wantedKey&255)==52?2:0) && reinterpret_cast<std::uintptr_t>(pk)==wantedPK && reinterpret_cast<std::uintptr_t>(index)==wantedIndex && skill==BYTE(wantedSkill) && CurrentSkill==(wantedKey&255));
 events.insert(events.end(),{2,static_cast<std::uint32_t>(type),Bits(angle[2]),skill});return nullptr;
}
'''
MAIN=r'''
int Primary(int type){
 switch(type){case 136:case 137:return 0xd9;case 138:return 0xdd;case 139:return 0xdb;case 140:return 0xda;case 141:return 0xe0;case 142:return 0xdf;case 144:case 146:return 0xf2;default:return 0;}
}
int Secondary(int type){
 switch(type){case 128:case 129:case 131:case 132:case 133:return 0xd8;case 130:return 0xdc;case 134:return 0xde;case 145:return 0xf3;default:return 0;}
}
std::uint64_t Run(BYTE* memory,BYTE* object,BYTE* identity){
 DAT_07cf1ffc=memory+16;objectAddress=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(object+16));characterAddress=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(identity+16));
 const short types[]={-1,128,129,130,131,132,133,134,136,137,138,139,140,141,142,144,145,146,888,999};
 const BYTE serials[]={0,1,126,127,128,253,254,255};
 const WORD skills[]={0,1,2,3,0x101,0x102,0xffff};
 const WORD keys[]={0,0x33,0x34,0x133,0x134,0xff33,0x80ff};
 std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(unsigned t=0;t<20;++t)for(unsigned p=0;p<8;++p)for(WORD skill:skills)for(WORD key:keys)for(unsigned hero=0;hero<2;++hero){
  std::memset(memory,0xa5,1444);std::memset(object,0x6b,512);std::memset(identity,0x7c,128);
  BYTE* cm=static_cast<BYTE*>(DAT_07cf1ffc);const short a=types[t],b=types[(t*3+p)%20];std::memcpy(cm+536,&a,2);std::memcpy(cm+604,&b,2);serialBefore=serials[p];cm[1408]=serialBefore;
  character.Weapon[0].Type=types[(t+3)%20];character.Weapon[1].Type=types[(t*7+2)%20];const auto beforeCharacter=character;
  DAT_07abf5d8=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(characterAddress+(hero?0:1)));
  aliases=(p&1)!=0;wantedIndex=WORD((t*32767+p*8191)&65535);wantedKey=key;wantedSkill=skill;wantedPK=WORD(0x8000+t*31+p);
  std::memcpy(object+16+134,&wantedPK,2);
  const float original=float(int(t)-int(p))* .125f;std::memcpy(object+16+36,&original,4);
  BYTE beforeMemory[1444],beforeObject[512],beforeIdentity[128],expectedMemory[1444];std::memcpy(beforeMemory,memory,1444);std::memcpy(expectedMemory,memory,1444);std::memcpy(beforeObject,object,512);std::memcpy(beforeIdentity,identity,128);
  const unsigned keyLow=key&255,skillLow=skill&255;
  std::vector<float> directions={original};
  const bool single=keyLow==51 || keyLow==52;
  if(!single && skillLow==1)directions={original,original+15,original-15};
  if(!single && skillLow==2)directions={original,original+7.5f,original+15,original-7.5f,original-15};
  if(single || skillLow==1 || skillLow==2)expectedMemory[16+1408]=BYTE(serialBefore+1);
  const int first=hero?a:character.Weapon[0].Type,second=hero?b:character.Weapon[1].Type;
  std::vector<std::uint32_t> expected;
  for(float angle:directions){
   if(first!=-1)expected.insert(expected.end(),{0,static_cast<std::uint32_t>(first)});
   if(second!=-1)expected.insert(expected.end(),{0,static_cast<std::uint32_t>(second)});
   if(keyLow==52)expected.insert(expected.end(),{1,0x41});
   if(keyLow==51)expected.insert(expected.end(),{1,0x65});
   const int primary=first==-1?0:Primary(Behavior(first)),secondary=second==-1?0:Secondary(Behavior(second));
   if(primary)expected.insert(expected.end(),{2,unsigned(primary),Bits(angle),skillLow});
   if(secondary)expected.insert(expected.end(),{2,unsigned(secondary),Bits(angle),skillLow});
  }
  CurrentSkill=0xdeadbeef;events.clear();CreateArrows(characterAddress,objectAddress,characterAddress+32,wantedIndex,skill,key);
  Require(events==expected && CurrentSkill==keyLow && std::memcmp(memory,expectedMemory,1444)==0 && std::memcmp(object,beforeObject,512)==0 && std::memcmp(identity,beforeIdentity,128)==0 && std::memcmp(&character,&beforeCharacter,sizeof(character))==0);
  for(auto value:events){hash^=value;hash*=1099511628211ull;}hash^=cm[1408];hash*=1099511628211ull;++queries;
 }
 Require(queries==15680);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096,low+8192);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,1444);
  Require(Run(high,low+4096,low+8192)==expected);for(unsigned i=0;i<1444;++i)Require(low[i]==0x7b);
  std::puts("CharacterMachine alto; entidades bajas, armas, abanicos, serial y alias intactos");
 }
 std::printf("Arrow machine: 15680 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Combat/Combat_Projectiles.cpp',revision)
    # El cast heredado de Hero sigue diagnosticado en producción; aquí conserva
    # los mismos bits con sintaxis aceptada por GCC para aislar CharacterMachine.
    legacy='(DWORD)DAT_07abf5d8'
    assert source.count(legacy)==1
    source=source.replace(legacy,'static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(DAT_07abf5d8))')
    globals_source=read_source('src/globals.cpp',revision)
    constants='\n'.join(re.search(r'^float\s+'+name+r'\s*=\s*[^;]+;',globals_source,re.M).group(0) for name in ('_DAT_00552834','_DAT_0055284c','_DAT_00552ab0','_DAT_00552584'))
    constants+='\n'+re.search(r'^constexpr int ITEM_MODEL_BASE[^;]+;',read_source('src/Item/ItemDefines.h',revision),re.M).group(0)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('CONSTANTS',constants)+function(source,'CreateArrow')+function(source,'CreateArrows')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-arrow-machine-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
