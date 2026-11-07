#!/usr/bin/env python3
"""Tick real de estelas: pool nativo, contadores, límites y bytes intactos."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
BYTE* pool;
#define g_RenderPool_07c608a8 (*reinterpret_cast<BYTE(*)[75200]>(pool))
ANCHOR
'''
MAIN=r'''
int Get(const BYTE* p){int value;std::memcpy(&value,p,4);return value;}
void Put(BYTE* p,int value){std::memcpy(p,&value,4);}
std::uint64_t Run(BYTE* memory){
 pool=memory+16;std::uint64_t hash=14695981039346656037ull;
 const int counts[]={-17,-1,0,1,2,33,0x7fffffff,static_cast<int>(0x80000400u)};
 const BYTE flags[]={0,1,0x80,0xff};
 for(unsigned pattern=0;pattern<64;++pattern){
  unsigned state=pattern*1664525u+1013904223u;
  for(unsigned i=0;i<75232;++i){state=state*1664525u+1013904223u;memory[i]=static_cast<BYTE>(state>>24);}
  for(unsigned slot=0;slot<100;++slot){
   BYTE* record=pool+slot*752;record[0]=flags[(pattern+slot)%4];
   Put(record+8,counts[(pattern/4+slot)%8]);Put(record+16,counts[(pattern/8+slot/8)%8]);
  }
  std::vector<BYTE> expected(memory,memory+75232);
  for(unsigned tick=0;tick<32;++tick){
   for(unsigned slot=0;slot<100;++slot){
    BYTE* record=expected.data()+16+slot*752;
    if(!record[0])continue;
    const auto life=std::int64_t(Get(record+8))-1;Require(life>=-2147483648ll && life<=2147483647ll);
    Put(record+8,static_cast<int>(life));
    if(life<=0){
     const auto segments=std::int64_t(Get(record+16))-1;Require(segments>=-2147483648ll && segments<=2147483647ll);
     Put(record+16,static_cast<int>(segments));if(segments<=0)record[0]=0;
    }
   }
   MoveBlurs();Require(std::memcmp(memory,expected.data(),75232)==0);
   for(unsigned slot=0;slot<100;++slot)for(unsigned offset: {0u,8u,9u,10u,11u,16u,17u,18u,19u}){
    hash^=pool[slot*752+offset];hash*=1099511628211ull;
   }
  }
 }
 return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 const auto expected=Run(low);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  std::memset(low,0x7b,75232);Require(Run(high)==expected);
  for(unsigned i=0;i<75232;++i)Require(low[i]==0x7b);
  auto* crossing=static_cast<BYTE*>(Reserve(std::uint64_t(0xffff0000ull)));Require(crossing);
  Require(Run(crossing+0xe000)==expected);
  std::puts("pool alto y cruzando 4 GiB; contador, bajas y alias bajo intactos");
 }
 std::printf("Blur tick: 2048 ticks/204800 slots por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    header=read_source('src/globals.h',revision)
    anchor=re.search(r'^#define DAT_07c608b8\s+[^\n]+',header,re.M).group(0)
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x40000')+SERVICES.replace('ANCHOR',anchor)+function(read_source('src/Core/Runtime_Small.cpp',revision),'MoveBlurs')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-blur-tick-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
