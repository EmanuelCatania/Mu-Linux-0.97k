#!/usr/bin/env python3
"""Reset real de CErrorReport: helper, entrada pública y llamada del constructor."""
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
#define PTR_FUN_005524c4 (*reinterpret_cast<int*>(std::uintptr_t(0x26008000u)))
'''
MAIN=r'''
void Reset(unsigned path,BYTE* data){
 if(path==0)FUN_00405290_impl(HELPER);
 else if(path==1)Require(FUN_00405290(PUBLIC)==0);
 else FUN_00405240_init(data);
}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;unsigned calls=0;
 for(unsigned pattern=0;pattern<256;++pattern)for(unsigned alignment=0;alignment<4;++alignment)for(unsigned path=0;path<3;++path){
  for(unsigned i=0;i<320;++i)memory[i]=static_cast<BYTE>(pattern+i*37);
  BYTE* data=memory+16+alignment*4;BYTE expected[320];std::memcpy(expected,memory,320);
  const auto offset=unsigned(data-memory);
  for(unsigned i=0;i<4;++i)expected[offset+4+i]=0xff;
  expected[offset+8]=0;
  for(unsigned i=0;i<4;++i)expected[offset+268+i]=0;
  if(path==2){const int vtable=0x26008000;std::memcpy(expected+offset,&vtable,4);}
  for(unsigned repeat=0;repeat<2;++repeat){
   Reset(path,data);Require(std::memcmp(memory,expected,320)==0);++calls;
   for(unsigned i=0;i<320;++i){hash^=memory[i];hash*=1099511628211ull;}
  }
 }
 Require(calls==6144);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,320);Require(Run(high)==expected);for(unsigned i=0;i<320;++i)Require(low[i]==0x7b);std::puts("contexto alto; reset público/privado y constructor con vtable32 controlada");}
 std::printf("Error report reset: 6144 llamadas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Core/Runtime_Small.cpp',revision)
    helper=function(source,'FUN_00405290_impl')
    public=function(source,'FUN_00405290')
    constructor=function(source,'FUN_00405240_init')
    # Sólo en el banco: conserva el store32 pendiente de B2 con sintaxis que GCC acepta.
    legacy='(int)&PTR_FUN_005524c4'
    assert constructor.count(legacy)==1
    constructor=constructor.replace(legacy,'static_cast<int>(reinterpret_cast<std::uintptr_t>(&PTR_FUN_005524c4))')
    integer='static_cast<int>(reinterpret_cast<std::uintptr_t>(data))'
    main=MAIN.replace('HELPER','data' if 'void* param_1' in helper else integer).replace('PUBLIC','data' if 'void* _this' in public else integer)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+helper+public+constructor+main

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-error-reset-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
