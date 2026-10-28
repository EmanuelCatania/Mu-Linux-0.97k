#!/usr/bin/env python3
"""Inicializador real de doce DWORDs: contexto nativo y constructor controlado."""
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
static_assert(sizeof(int)==4,"Palabras del port");
#define PTR_FUN_0055389c (*reinterpret_cast<int*>(std::uintptr_t(0x26008000u)))
'''
MAIN=r'''
void Reset(unsigned path,BYTE* data){
 if(path==0)FUN_0053cc00_impl(ARG);
 else FUN_0053cbb0(reinterpret_cast<int*>(data));
}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;unsigned calls=0;
 for(unsigned pattern=0;pattern<512;++pattern)for(unsigned alignment=0;alignment<4;++alignment)for(unsigned path=0;path<2;++path){
  unsigned state=pattern*1664525u+1013904223u;
  for(unsigned i=0;i<128;++i){state=state*1664525u+1013904223u;memory[i]=static_cast<BYTE>(state>>24);}
  BYTE* data=memory+16+alignment*4;BYTE expected[128];std::memcpy(expected,memory,128);const auto offset=unsigned(data-memory);
  std::memset(expected+offset+4,0,48);
  if(path==1){const int vtable=0x26008000;std::memcpy(expected+offset,&vtable,4);}
  for(unsigned repeat=0;repeat<2;++repeat){
   Reset(path,data);Require(std::memcmp(memory,expected,128)==0);++calls;
   for(unsigned i=0;i<128;++i){hash^=memory[i];hash*=1099511628211ull;}
  }
 }
 Require(calls==8192);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("contexto alto; doce palabras y canarios, constructor con descriptor32 controlado");}
 std::printf("Twelve word reset: 8192 llamadas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    helper=function(source,'FUN_0053cc00_impl')
    constructor=function(source,'FUN_0053cbb0')
    legacy='(int)&PTR_FUN_0055389c'
    assert constructor.count(legacy)==1
    # Adaptación sólo del store32 pendiente de B2; no se elimina en producción.
    constructor=constructor.replace(legacy,'static_cast<int>(reinterpret_cast<std::uintptr_t>(&PTR_FUN_0055389c))')
    arg='data' if 'void* param_1' in helper else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(data))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+helper+constructor+MAIN.replace('ARG',arg)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-twelve-word-reset-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
