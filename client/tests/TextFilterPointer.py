#!/usr/bin/env python3
"""Filtro real de cinco cadenas: base nativa, corte y contrato de búsqueda."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __fastcall
#endif
char *context,*text;unsigned matchMask,calls;
bool __cdecl FindTextA(char* input,char* token,bool sensitive){
 Require(input==text && !sensitive && calls<5 && token==context+200+calls*256);
 return (matchMask&(1u<<calls++))!=0;
}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory,BYTE* input){
 context=reinterpret_cast<char*>(memory+16);text=reinterpret_cast<char*>(input+16);
 std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(unsigned pattern=0;pattern<256;++pattern)for(unsigned empty=0;empty<6;++empty)for(matchMask=0;matchMask<32;++matchMask){
  std::memset(memory,0xa5,1536);std::memset(input,0x6b,320);
  for(unsigned i=0;i<5;++i){
   char* row=context+200+i*256;const unsigned length=(pattern+i)%255+1;
   for(unsigned j=0;j<length;++j)row[j]=static_cast<char>(1+(pattern+i+j)%255);
   row[length]=0;if(i==empty)row[0]=0;
  }
  for(unsigned i=0;i<255;++i)text[i]=static_cast<char>(1+(pattern+i)%255);text[255]=0;
  BYTE before[1536],beforeText[320];std::memcpy(before,memory,1536);std::memcpy(beforeText,input,320);
  unsigned expectedCalls=0;int expected=empty==0?1:0;
  if(empty!=0)for(unsigned i=0;i<5 && i<empty;++i){++expectedCalls;if(matchMask&(1u<<i)){expected=1;break;}}
  calls=0;const int result=FUN_0040e730(context,static_cast<int>(pattern),text);
  Require(result==expected && calls==expectedCalls && std::memcmp(memory,before,1536)==0 && std::memcmp(input,beforeText,320)==0);
  hash^=static_cast<unsigned>(result);hash*=1099511628211ull;hash^=calls;hash*=1099511628211ull;++queries;
 }
 Require(queries==49152);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 const auto expected=Run(low,low+4096);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<4;++mask){
   std::memset(low,0x7b,8192);Require(Run(mask&1?high:low,(mask&2?high:low)+4096)==expected);
   if(mask&1)for(unsigned i=0;i<1536;++i)Require(low[i]==0x7b);
   if(mask&2)for(unsigned i=0;i<320;++i)Require(low[4096+i]==0x7b);
  }
  std::puts("contexto/texto altos independientes; orden, corte y aliases intactos");
 }
 std::printf("Text filter: 49152 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Core/Runtime_Medium.cpp',revision),'FUN_0040e730')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-filter-context-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
