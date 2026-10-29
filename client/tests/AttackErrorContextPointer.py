#!/usr/bin/env python3
"""Argumentos de las19llamadas reales al shim local de errores de combate.

Ejecuta el no-op real con contexto bajo/alto. No ejecuta AttackEffect ni
certifica sus ramas, hash, emisiones o logger real.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
BYTE* report;unsigned calls,wantedSite;BYTE before[128];BYTE* memory;
#define g_ErrorReport (*reinterpret_cast<DWORD*>(report))
TEXT
namespace CErrorReport{
ACTUAL
inline void Write(TYPE context,const char* text){
 Require(ADDRESS==reinterpret_cast<std::uintptr_t>(report) && text==aHashTableFullG && std::memcmp(memory,before,128)==0);
 ActualWrite(context,text);Require(std::memcmp(memory,before,128)==0);++calls;
}
}
CALLS
'''
MAIN=r'''
std::uint64_t Run(BYTE* allocation){
 memory=allocation;std::uint64_t hash=14695981039346656037ull;unsigned total=0;
 using Call=void(*)();const Call sites[]={SITES};
 static_assert(sizeof(sites)/sizeof(sites[0])==19);
 const char textBefore[]="hash table full";
 for(unsigned pattern=0;pattern<256;++pattern){
  for(unsigned i=0;i<128;++i)memory[i]=static_cast<BYTE>(pattern+i*37);
  report=memory+16+(pattern%8)*4;std::memcpy(before,memory,128);
  for(wantedSite=0;wantedSite<19;++wantedSite){
   calls=0;sites[wantedSite]();Require(calls==1 && std::memcmp(memory,before,128)==0 && std::memcmp(aHashTableFullG,textBefore,sizeof(textBefore))==0);
   for(BYTE byte:before){hash^=byte;hash*=1099511628211ull;}hash^=wantedSite;hash*=1099511628211ull;++total;
  }
 }
 Require(total==4864);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("contexto alto;19llamadas,noop,canarios y alias intactos");}
 std::printf("Attack error context: 4864 llamadas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Combat/Combat_AttackEffect.cpp',revision)
    shim=re.search(r'namespace CErrorReport\s*\{\s*(inline void Write\((DWORD|void\*)\s*,\s*const char\*\)\s*\{[^{}]*\})\s*\}',source)
    if not shim:raise ValueError('Shim local inesperado')
    calls=re.findall(r'CErrorReport::Write\([^;]*\);',source)
    if len(calls)!=19:raise ValueError(f'Se esperaban19llamadas,hay{len(calls)}')
    text=re.search(r'static const char aHashTableFullG\[\]\s*=\s*"[^"\n]*";',source).group(0)
    actual=shim[1].replace('Write(','ActualWrite(',1)
    address='reinterpret_cast<std::uintptr_t>(context)' if shim[2]=='void*' else 'static_cast<std::uintptr_t>(context)'
    services=SERVICES.replace('TEXT',text).replace('ACTUAL',actual).replace('TYPE',shim[2]).replace('ADDRESS',address).replace('CALLS','\n'.join(f'void Call{i}(){{{call}}}' for i,call in enumerate(calls)))
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+MAIN.replace('SITES',','.join(f'Call{i}' for i in range(19)))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-attack-error-context-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
