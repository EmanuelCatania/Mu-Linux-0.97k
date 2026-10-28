#!/usr/bin/env python3
"""Panel real de tienda: nodo nativo y flujo de botones con servicios controlados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using uint=unsigned;
int MAIN_HASH_CLASS;BYTE *node;bool missing,pushed;unsigned factoryCalls;std::vector<unsigned> events;
char DAT_07eaa118,DAT_07eaa132,DAT_07eaa134,DAT_07eaa117;
DWORD DAT_07eaa150,DAT_07eaa0c8,DAT_07eaa0cc,DAT_083a427c,DAT_083a4278,DAT_083a4124,DAT_07ea5288,DAT_07ea5284,DAT_07e11d28,DAT_00559bec;
uint HashTable_GetIndex(void* context,void* key){Require(context==&MAIN_HASH_CLASS && key==&DAT_07eaa118);events.push_back(1);return missing?0xffffffffu:3;}
void* AntiTamper_HashNode(){++factoryCalls;events.push_back(2);return node;}
void HashTable_Insert(void* context,void* value,void* key){Require(context==&MAIN_HASH_CLASS && value==node && key==&DAT_07eaa118 && node[1]==1);events.push_back(3);}
bool IsClickPushed(){events.push_back(4);return pushed;}
void Item_RecalculateRepairCost(){events.push_back(5);}
struct Network{void Send(BYTE* packet,unsigned n){const BYTE expected[]={0xc1,5,0x34,0xff,0};Require(n==5 && std::memcmp(packet,expected,5)==0);events.push_back(6);}}gNetwork;
void CloseInventoryRelatedWindows(){Require(DAT_083a4124==0 && DAT_07eaa117==0 && DAT_07e11d28==0x12345678 && DAT_00559bec==0x76543210);events.push_back(7);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory){
 node=memory+16;std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 const BYTE flags[]={0,1,128,255};
 const unsigned xs[]={124,125,148,149,184,185,208,209,214,215,238,239,44,45,68,69};
 const unsigned ys[]={404,405,428,429,414,415,438,439};
 for(unsigned pattern=0;pattern<64;++pattern)for(unsigned absent=0;absent<2;++absent)for(unsigned click=0;click<2;++click)for(auto x:xs)for(auto y:ys){
  std::memset(memory,0xa5,1445);BYTE expectedMemory[1445];std::memcpy(expectedMemory,memory,1445);missing=absent!=0;pushed=click!=0;factoryCalls=0;
  DAT_07eaa118=static_cast<char>(flags[pattern%4]);DAT_07eaa132=static_cast<char>(flags[(pattern/4)%4]);DAT_07eaa134=static_cast<char>(flags[(pattern/8)%4]);
  DAT_07eaa150=0x7e003456u|(DWORD(flags[pattern/16])<<16);DAT_083a4124=0xabcdef98;DAT_07eaa117=static_cast<char>(0x80);DAT_07e11d28=0x12345678;DAT_00559bec=0x76543210;
  DAT_07eaa0c8=100;DAT_07eaa0cc=40;DAT_07ea5288=20;DAT_07ea5284=20;DAT_083a427c=x;DAT_083a4278=y;
  DWORD expectedMode=DAT_07eaa150,expectedClick=DAT_083a4124,expectedWindow=DAT_07e11d28,expectedState=DAT_00559bec;char expectedRepair=DAT_07eaa134,expectedOpen=DAT_07eaa117;
  std::vector<unsigned> expected={1};if(missing){expected.insert(expected.end(),{2,3});expectedMemory[17]=1;}
  if(DAT_07eaa118){
   if(DAT_07eaa132){
    const bool row=y==405 || y==414 || y==415 || y==428;
    if(row && (x==125 || x==148)){
     expected.push_back(4);if(pushed){expectedClick=0;const bool enable=((expectedMode>>16)&255)==0;expectedMode=(expectedMode&0xff00ffffu)|(enable?0x20000u:0);expectedRepair=enable?1:0;}
    }
    if(row && (x==185 || x==208)){
     expected.push_back(4);if(pushed){expectedClick=0;expectedRepair=static_cast<char>(static_cast<BYTE>(expectedRepair)^1);expectedMode=(expectedMode&0xff00ffffu)|(expectedRepair?0x20000u:0);}
    }
    bool sent=false;
    if(row && (x==215 || x==238)){expected.push_back(4);if(pushed){expected.push_back(6);expectedClick=0;sent=true;}}
    if(!sent)expected.push_back(5);expected.push_back(5);
   }
   const bool cancel=(x==45 || x==68) && (y==415 || y==428 || y==429 || y==438);
   if(cancel){expected.push_back(4);if(pushed){expectedClick=0;expectedOpen=0;expected.push_back(7);expectedWindow=0;expectedState=6;}}
  }
  events.clear();FUN_004ec330();
  Require(events==expected && factoryCalls==absent && DAT_07eaa150==expectedMode && DAT_07eaa134==expectedRepair && DAT_083a4124==expectedClick && DAT_07eaa117==expectedOpen && DAT_07e11d28==expectedWindow && DAT_00559bec==expectedState && std::memcmp(memory,expectedMemory,1445)==0);
  Require(DAT_07eaa118==static_cast<char>(flags[pattern%4]) && DAT_07eaa132==static_cast<char>(flags[(pattern/4)%4]) && DAT_07eaa0c8==100 && DAT_07eaa0cc==40 && DAT_07ea5288==20 && DAT_07ea5284==20 && DAT_083a427c==x && DAT_083a4278==y);
  for(unsigned value:events){hash^=value;hash*=1099511628211ull;}
  const DWORD states[]={DAT_07eaa150,DAT_083a4124,DAT_07e11d28,DAT_00559bec,DWORD(static_cast<BYTE>(DAT_07eaa134)),DWORD(static_cast<BYTE>(DAT_07eaa117))};
  for(auto value:states){hash^=value;hash*=1099511628211ull;}
  ++queries;
 }
 Require(queries==32768);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,1445);Require(Run(high)==expected);for(unsigned i=0;i<1445;++i)Require(low[i]==0x7b);std::puts("nodo alto; botones, secuencia, paquete y alias bajo intactos");}
 std::printf("Shop hash: 32768 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Net/SecondPassword.cpp',revision),'FUN_004ec330')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-shop-hash-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
