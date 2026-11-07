#!/usr/bin/env python3
"""Propiedad nativa de tres reservas hash; fragmentos reales sin recorrer la tabla legacy."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,read_source
SERVICES=r'''
#include <utility>
using std::uintptr_t;
std::vector<std::pair<uintptr_t,unsigned>> allocations;std::vector<uintptr_t> releases;uintptr_t nextAddress;unsigned randomValue;
void* operator_new(unsigned size){uintptr_t p=nextAddress;nextAddress+=0x100000;allocations.push_back({p,size});return reinterpret_cast<void*>(p);}
void operator_delete(void* p){releases.push_back(reinterpret_cast<uintptr_t>(p));}
int _rand(){return int(randomValue);}
DWORD DAT_055c9bcc=0x12000000,DAT_055c9bd0=0x14000000,DAT_055c9bd4;
'''
MAIN=r'''
std::uint64_t Run(uintptr_t base){std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 for(unsigned value=0;value<32768;++value)for(unsigned capacity:{0u,1u,64u,65535u})for(int existing:{0,1}){
  SetOwners(existing?base:0,existing?base+0x100000:0,existing?base+0x200000:0);uintptr_t old0=Owner0(),old8=Owner8(),oldC=OwnerC();nextAddress=base+0x300000;randomValue=value;DAT_055c9bd4=capacity;allocations.clear();releases.clear();Replace();
  Require(allocations.size()==3&&allocations[0].first==base+0x300000&&allocations[1].first==base+0x400000&&allocations[2].first==base+0x500000);
  Require(allocations[0].second==value%0xcc7+0x159&&allocations[1].second==capacity*4+0x100&&allocations[2].second==capacity*4+0x100);
  Require(Owner0()==allocations[0].first&&Owner8()==allocations[1].first&&OwnerC()==allocations[2].first);
  Require(releases.size()==unsigned(existing?3:2));unsigned index=0;if(existing)Require(releases[index++]==old0);Require(releases[index++]==old8&&releases[index]==oldC);
  for(auto p:allocations){hash^=p.second;hash*=1099511628211ull;}hash^=releases.size();hash*=1099511628211ull;++cases;
 }Require(cases==262144);return hash;
}
int main(){auto expected=Run(0x26000000u);if(sizeof(void*)>4){Require(Run(uintptr_t(0x126000000ull))==expected);Require(Run(0x8a000000u)==expected);}std::printf("Hash backing:262144 reemplazos/ruta %016llx\n",(unsigned long long)expected);}
'''
def harness(revision=None):
 source=read_source('src/Game/Game_MainLoop.cpp',revision);start=source.index('if (DAT_055c9be0 != NULL)');end=source.index('unsigned rnd1',start);prefix=source[start:end];tail='operator_delete(old_be8);\n            operator_delete(old_bec);';assert tail in source
 declarations=[];getters=[];setters=[]
 for name,suffix,arg in [('DAT_055c9be0','0','a'),('DAT_055c9be8','8','b'),('DAT_055c9bec','C','c')]:
  match=re.search(r'(DWORD|BYTE\*)\s+'+name+r'\s*=\s*(?:0|nullptr);',read_source('src/globals.cpp',revision));assert match;declarations.append(match.group());kind=match[1]
  getters.append('uintptr_t Owner'+suffix+'(){return '+('static_cast<uintptr_t>('+name+')' if kind=='DWORD' else 'reinterpret_cast<uintptr_t>('+name+')')+';}')
  setters.append(name+' = '+('static_cast<DWORD>('+arg+')' if kind=='DWORD' else 'reinterpret_cast<BYTE*>('+arg+')')+';')
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+'\n'.join(declarations+getters)+'\nvoid SetOwners(uintptr_t a,uintptr_t b,uintptr_t c){'+''.join(setters)+'}\nvoid Replace(){'+prefix+tail+'}\n'+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-hash-backing-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
