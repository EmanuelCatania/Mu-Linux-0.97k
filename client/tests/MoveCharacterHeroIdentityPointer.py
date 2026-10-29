#!/usr/bin/env python3
"""Cuatro usos reales de identidad de Hero; no ejecuta MoveCharacter completo."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using std::uintptr_t;
struct CHARACTER{};
ACCESSORS
void* Hero;void* CharacterAttribute;BYTE machine[128];void* CharacterMachine=machine;
int SelectedCharacter;std::vector<unsigned> calls;
void CalculateAttackSpeed(BYTE* p){Require(p==machine);calls.push_back(1);}
void Stats_CalcBase(BYTE* p){Require(p==machine);calls.push_back(2);}
void Stats_CalcMagicDmgRange(BYTE* p){Require(p==machine);calls.push_back(3);}
void Buff(DWORD c){BUFF}
void Dissolve(DWORD c,DWORD o){KILL}
bool TargetGuard(DWORD c){return (TARGET);}
float Green(DWORD v390){float Light[3]={};GREEN return Light[1];}
'''
MAIN=r'''
std::uint64_t Run(BYTE* actor,void* hero,bool local,unsigned targetMatch){
 Hero=hero;DWORD c=static_cast<DWORD>(reinterpret_cast<uintptr_t>(actor));
 const unsigned short timers[]={0,1,2,255,256,32767,32768,65535};
 const int selections[]={-1,0,-2,399,int(0x80000000u),0x7fffffff};
 std::uint64_t hash=14695981039346656037ull;unsigned count=0,coverage=0;
 for(unsigned flags=0;flags<256;++flags)for(unsigned t=0;t<8;++t)for(int selected:selections)for(int present:{0,1})for(unsigned target=0;target<4;++target){
  for(unsigned i=0;i<256;++i)actor[i]=BYTE(flags+i*13);BYTE actorBefore[256];std::memcpy(actorBefore,actor,256);
  BYTE attr[64],wanted[64];for(unsigned i=0;i<64;++i)attr[i]=BYTE(flags+i*37);
  std::memcpy(attr+42,&timers[t],2);std::memcpy(attr+44,&timers[(t+3)%8],2);attr[40]=BYTE(flags);std::memcpy(wanted,attr,64);
  CharacterAttribute=present?attr:nullptr;SelectedCharacter=selected;calls.clear();std::vector<unsigned> expected;
  if(local&&present){
   unsigned short a=timers[t]?timers[t]-1:0,b=timers[(t+3)%8]?timers[(t+3)%8]-1:0;
   std::memcpy(wanted+42,&a,2);std::memcpy(wanted+44,&b,2);
   if(!a){wanted[40]&=BYTE(~1u);expected.push_back(1);}if(!b){wanted[40]&=BYTE(~2u);expected.push_back(2);expected.push_back(3);}
  }
  Buff(c);Require(calls==expected&&std::memcmp(attr,wanted,64)==0&&std::memcmp(actor,actorBefore,256)==0);
  Dissolve(c,c);if(!local)actorBefore[0]=0;Require(std::memcmp(actor,actorBefore,256)==0);
  Require(TargetGuard(c)==(local&&selected!=-1));
  DWORD candidate=target==0?c:target==1?c+64:target==2?0:c+128;
  float color=Green(candidate),wantedColor=target==targetMatch?0.0f:0.60000002f;
  Require(std::memcmp(&color,&wantedColor,4)==0&&Hero==hero&&CharacterAttribute==(present?attr:nullptr)&&SelectedCharacter==selected);
  for(BYTE b:attr){hash^=b;hash*=1099511628211ull;}hash^=actor[0]+256u*unsigned(TargetGuard(c));hash*=1099511628211ull;
  for(unsigned v:calls){hash^=v;hash*=1099511628211ull;}DWORD bits;std::memcpy(&bits,&color,4);hash^=bits;hash*=1099511628211ull;
  coverage|=local?1:2;coverage|=present?4:8;coverage|=color==0?16:32;++count;
 }
 Require(count==98304&&(coverage&47)==(local?45u:46u));return hash;
}
int main(){
 BYTE* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);low+=16;
 for(unsigned i=0;i<128;++i)machine[i]=BYTE(i*17+5);BYTE beforeMachine[128];std::memcpy(beforeMachine,machine,128);
 void* heroes[]={low,low+64,nullptr,low+128};std::uint64_t expected[4];
 for(unsigned route=0;route<4;++route)expected[route]=Run(low,heroes[route],route==0,route);
 if(sizeof(void*)>4){
  BYTE* sign=static_cast<BYTE*>(Reserve(0x8e000000u));Require(sign);sign+=16;
  void* signedHeroes[]={sign,sign+64,nullptr,sign+128};for(unsigned route=0;route<4;++route)Require(Run(sign,signedHeroes[route],route==0,route)==expected[route]);
  BYTE* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));Require(high);std::memset(high,0x6d,256);
  // Ningún candidato bajo coincide con estos Hero altos, aun compartiendo32bits.
  auto unmatched=Run(low,low+192,false,4);
  Require(Run(low,high+16,false,4)==unmatched&&Run(low,high+80,false,4)==unmatched);
  for(unsigned i=0;i<256;++i)Require(high[i]==0x6d);
  std::puts("identidad completa:bit31,Hero alto y alias32 distintos;Hero alto readonly");
 }
 Require(std::memcmp(machine,beforeMachine,128)==0);
 std::printf("MoveCharacter Hero: 98304 casos/ruta %016llx %016llx %016llx %016llx\n",(unsigned long long)expected[0],(unsigned long long)expected[1],(unsigned long long)expected[2],(unsigned long long)expected[3]);
}
'''
def harness(revision=None):
    source=read_source('src/Net/SecondPassword.cpp',revision)
    source=source[source.index('void __cdecl MoveCharacter('):]
    start=source.index('    if (',source.index('// ─── IDA L437-477:'))
    end=source.index('    // ─── IDA L478-498:',start)
    buff=source[start:end]
    kill=re.search(r'^\s*if \(.*Hero.*\) \*\(BYTE\*\)o = 0;',source,re.M).group()
    target=re.search(r'if \(([^\n]+Hero[^\n]+SelectedCharacter[^\n]+)\) \{',source).group(1)
    green=re.search(r'^\s*Light\[1\] = [^\n]+Hero[^\n]+;',source,re.M).group()
    header=read_source('src/Entity/EntityTypes.h',revision)
    accessors='\n'.join(re.findall(r'^inline (?:const )?CHARACTER\* AsCharacter\([^\n]+',header,re.M))
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('ACCESSORS',accessors).replace('BUFF',buff).replace('KILL',kill).replace('TARGET',target).replace('GREEN',green)+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-move-hero-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
