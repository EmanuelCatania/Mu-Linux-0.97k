#!/usr/bin/env python3
"""Dos recorridos reales de cuatro bytes del bloque hash de clima.

Sólo extrae las comparaciones de local_40/puStack_44. No ejecuta Weather_Update
ni reproduce uStack_48,dispatch,hash,pools,tiempo o emisiones del resto.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
#include <new>
using uint=unsigned int;
struct Result{int remaining,order;bool less,equal;std::uintptr_t left,right;};
'''
MAIN=r'''
std::uint64_t Run(BYTE* left,BYTE* right){
 std::uint64_t hash=14695981039346656037ull;unsigned calls=0,coverage=0;
 using Compare=Result(*)(DWORD**,unsigned char*);const Compare comparisons[]={Compare0,Compare1};
 for(unsigned pattern=0;pattern<256;++pattern){
  for(unsigned i=0;i<128;++i)left[i]=static_cast<BYTE>(pattern+i*37);
  const DWORD word=pattern|(((pattern*37)&255)<<8)|(((pattern*73)&255)<<16)|(((pattern*113)&255)<<24);
  const auto tag=static_cast<std::uintptr_t>(word)|(sizeof(void*)>4?static_cast<std::uintptr_t>(0x100000000ull):0);
  // El valor opaco del puntero nunca se desreferencia: sólo su representación.
  auto* slot=new(left+16+(pattern%4)*8) DWORD*(reinterpret_cast<DWORD*>(tag));
  BYTE beforeLeft[128];std::memcpy(beforeLeft,left,128);
  BYTE bytes[4];std::memcpy(bytes,slot,4);
  for(unsigned pivot=0;pivot<5;++pivot)for(unsigned value=0;value<256;++value){
   for(unsigned i=0;i<128;++i)right[i]=static_cast<BYTE>(pattern+i*13);
   BYTE* key=right+16+(pattern%4)*8;std::memcpy(key,bytes,4);if(pivot<4)key[pivot]=static_cast<BYTE>(value);
   BYTE beforeRight[128];std::memcpy(beforeRight,right,128);
   const int cmp=std::memcmp(bytes,key,4);const int order=(cmp>0)-(cmp<0);
   unsigned consumed=4;for(unsigned i=0;i<4;++i)if(bytes[i]!=key[i]){consumed=i+1;break;}
   for(Compare compare:comparisons){
    const Result actual=compare(slot,key);
    Require(actual.remaining==int(4-consumed) && actual.order==order && actual.less==(cmp<0) && actual.equal==(cmp==0));
    Require(actual.left==reinterpret_cast<std::uintptr_t>(slot)+consumed && actual.right==reinterpret_cast<std::uintptr_t>(key)+consumed);
    Require(std::memcmp(left,beforeLeft,128)==0 && std::memcmp(right,beforeRight,128)==0);
    coverage|=cmp==0?1u:cmp<0?2u:4u;coverage|=1u<<(consumed+2);
    hash^=unsigned(actual.remaining)+8*unsigned(actual.order+1)+32*unsigned(actual.less)+64*unsigned(actual.equal);hash*=1099511628211ull;++calls;
   }
  }
 }
 Require(calls==655360 && coverage==127);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* key=static_cast<BYTE*>(Reserve(0x28000000u));Require(low&&key);const auto expected=Run(low,key);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));auto* highKey=static_cast<BYTE*>(Reserve(std::uintptr_t(0x128000000ull)));Require(high&&highKey);
  std::memset(low,0x7b,128);Require(Run(high,key)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);
  std::memset(key,0x6d,128);Require(Run(low,highKey)==expected);for(unsigned i=0;i<128;++i)Require(key[i]==0x6d);
  std::memset(low,0x7b,128);std::memset(key,0x6d,128);Require(Run(high,highKey)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b && key[i]==0x6d);
  std::puts("cursores altos;cuatro bytes,flags,destinos,readonly y aliases intactos");
 }
 std::printf("Weather compare cursor: 655360 comparaciones por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Render/Weather.cpp',revision);comparisons=[]
    for index,name in enumerate(('ppuVar15','ppuVar15b')):
        line=re.search(r'^[^\n]*\b'+name+r'\s*=[^\n]*$',source,re.M)
        if not line:raise ValueError('Cursor inesperado')
        end=source.index('} while (bVar21);',line.end())+len('} while (bVar21);')
        end=source.index(';',source.index('if (!bVar21)',end))+1
        code=source[line.start():end]
        comparisons.append(f'''Result Compare{index}(DWORD** slot,unsigned char* key){{
 DWORD*& local_40=*slot;DWORD*& puStack_44=*slot;
 unsigned char *pbVar17,*pbVar18=key;int iVar12=4,iVar14=0;bool bVar20=false,bVar21=true;
 {code}
 return {{iVar12,iVar14,bVar20,bVar21,reinterpret_cast<std::uintptr_t>({name}),reinterpret_cast<std::uintptr_t>(pbVar17)}};
}}''')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+'\n'.join(comparisons)+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-weather-compare-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
