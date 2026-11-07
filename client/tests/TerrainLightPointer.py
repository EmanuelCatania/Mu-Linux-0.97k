#!/usr/bin/env python3
"""Grilla real de iluminación: cálculo, límites y tres bases nativas."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
float *normals,*ambient,*output;
#define DAT_07feb288 (*reinterpret_cast<float(*)[196608]>(normals))
#define DAT_07eeb238 (*reinterpret_cast<float(*)[196608]>(ambient))
#define DAT_0828b608 (*reinterpret_cast<float(*)[196608]>(output))
CONSTANTS
'''
MAIN=r'''
constexpr unsigned bytes=786432,region=bytes+32;
std::uint64_t Run(BYTE* n,BYTE* a,BYTE* o){
 normals=reinterpret_cast<float*>(n+16);ambient=reinterpret_cast<float*>(a+16);output=reinterpret_cast<float*>(o+16);
 std::uint64_t hash=14695981039346656037ull;
 const float values[]={-4,-2,-1,-.5f,-.25f,-0.0f,0,.25f,.5f,1,2,4};
 for(unsigned pattern=0;pattern<16;++pattern){
  std::memset(n,0xa5,region);std::memset(a,0x6b,region);std::memset(o,0xcd,region);
  std::vector<BYTE> expected(region,0xcd);
  for(unsigned vertex=0;vertex<65536;++vertex){
   float xyz[3];
   for(unsigned component=0;component<3;++component){
    xyz[component]=values[(vertex/(component+1)+pattern*3+component*5)%12];
    normals[vertex*3+component]=xyz[component];
    ambient[vertex*3+component]=values[(vertex+pattern+component*7)%12];
   }
   double factor=(double(xyz[0])+xyz[2])*.5-double(xyz[1])*.5+.5;
   if(factor<0)factor=0;if(factor>1)factor=1;
   for(unsigned component=0;component<3;++component){
    const float result=static_cast<float>(factor*ambient[vertex*3+component]);
    std::memcpy(expected.data()+16+(vertex*3+component)*4,&result,4);
   }
  }
  const std::vector<BYTE> beforeN(n,n+region),beforeA(a,a+region);
  CreateTerrainLight();
  Require(std::memcmp(o,expected.data(),region)==0 && std::memcmp(n,beforeN.data(),region)==0 && std::memcmp(a,beforeA.data(),region)==0);
  for(unsigned i=0;i<region;++i){hash^=o[i];hash*=1099511628211ull;}
 }
 return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 const auto expected=Run(low,low+0x100000,low+0x200000);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<8;++mask){
   std::memset(low,0x7b,0x300000);
   Require(Run((mask&1?high:low),(mask&2?high:low)+0x100000,(mask&4?high:low)+0x200000)==expected);
   for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<region;++i)Require(low[slot*0x100000+i]==0x7b);
  }
  std::puts("tres bases altas independientes; entradas y aliases intactos");
 }
 std::printf("Terrain light: 16 grillas completas %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Terrain/Terrain.cpp',revision)
    globals_source=read_source('src/globals.cpp',revision)
    constants='\n'.join(re.search(r'float\s+'+name+r'\s*=\s*[^;]+;',globals_source).group(0) for name in ('_DAT_00552504','_DAT_00552580','_DAT_0055256c'))
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x300000')+SERVICES.replace('CONSTANTS',constants)+function(source,'CreateTerrainLight')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-terrain-light-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
