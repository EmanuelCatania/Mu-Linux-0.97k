#!/usr/bin/env python3
"""Destructor real de ancla: adaptador de vtable y contexto de ancho nativo."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __fastcall
#endif
#define DAT_00552508 (*reinterpret_cast<DWORD*>(std::uintptr_t(0x26009000u)))
static void __cdecl ClothAnchor_VSlot0(void*,char);
static void __cdecl ClothAnchor_VSlot1(void*);
void __cdecl ClothAnchor_Collide(float*,int*){Require(false);}
extern void* g_ClothAnchorVTable[3];
BYTE *memory,*object;BYTE before[128],finalState[128];unsigned stage;int wantedEdx;BYTE wantedFlags;bool baseRoute;
BASE
void __cdecl FUN_00407de0(DWORD* context){
 Require(stage++==(baseRoute?1u:2u) && reinterpret_cast<BYTE*>(context)==object);
 BYTE intermediate[128];std::memcpy(intermediate,before,128);if(!baseRoute){const DWORD descriptor=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_ClothAnchorVTable));std::memcpy(intermediate+(object-memory),&descriptor,4);}
 Require(std::memcmp(memory,intermediate,128)==0);ActualBase(context);
}
DERIVED
void __cdecl FUN_00407ec0(DWORD* context){Require(!baseRoute && stage++==1 && reinterpret_cast<BYTE*>(context)==object && std::memcmp(memory,before,128)==0);ActualDerived(context);}
void operator_delete(void* p){Require(stage++==(baseRoute?2u:3u) && p==object && (wantedFlags&1) && std::memcmp(memory,finalState,128)==0);}
DESTROY
API_HEADER{
 Require(stage++==0 && ADDRESS==reinterpret_cast<std::uintptr_t>(object) && edx==wantedEdx && param_1==wantedFlags && std::memcmp(memory,before,128)==0);
 ActualDestroy(ecx,edx,param_1);
}
BASE_DESTROY
BASE_API_HEADER{
 Require(stage++==0 && BASE_ADDRESS==reinterpret_cast<std::uintptr_t>(object) && edx==wantedEdx && param_1==wantedFlags && std::memcmp(memory,before,128)==0);
 ActualBaseDestroy(ecx,edx,param_1);
}
ADAPTER
SLOT1
TABLE
'''
MAIN=r'''
void Release(unsigned path,int edx,BYTE flags){
 baseRoute=path==2;wantedEdx=path==1?0:edx;wantedFlags=flags;
 if(!path)FUN_00407ea0(ARG,edx,flags);
 else if(path==1){using ReleaseFn=void(__cdecl*)(void*,char);reinterpret_cast<ReleaseFn>(g_ClothAnchorVTable[0])(object,static_cast<char>(flags));}
 else FUN_00407dc0(BASE_ARG,edx,flags);
}
std::uint64_t Run(BYTE* data){
 memory=data;std::uint64_t hash=14695981039346656037ull;unsigned calls=0;
 const int edxValues[]={0,-1,0x7fffffff,static_cast<int>(0x80000000u)};
 for(unsigned pattern=0;pattern<64;++pattern)for(unsigned path=0;path<3;++path)for(unsigned flags=0;flags<256;++flags)for(int edx:edxValues){
  unsigned state=pattern*1664525u+1013904223u;
  for(unsigned i=0;i<128;++i){state=state*1664525u+1013904223u;memory[i]=static_cast<BYTE>(state>>24);}
  object=memory+16+(pattern%4)*4;std::memcpy(before,memory,128);std::memcpy(finalState,memory,128);const DWORD descriptor=0x26009000;std::memcpy(finalState+(object-memory),&descriptor,4);
  stage=0;Release(path,edx,static_cast<BYTE>(flags));
  Require(stage==(baseRoute?2u:3u)+(flags&1) && std::memcmp(memory,finalState,128)==0);
  for(unsigned i=0;i<128;++i){hash^=memory[i];hash*=1099511628211ull;}hash^=stage;hash*=1099511628211ull;++calls;
 }
 Require(calls==196608);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("ancla alta; slot real, orden de setters/delete y alias intactos");}
 std::printf("Anchor destructor: 196608 llamadas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    runtime=read_source('src/Core/Runtime_Small.cpp',revision).replace('int /*edx*/','int edx')
    cloth=read_source('src/Physics/Cloth_Simulation.cpp',revision)
    base=function(runtime,'FUN_00407de0').replace('FUN_00407de0','ActualBase')
    derived=function(runtime,'FUN_00407ec0').replace('FUN_00407ec0','ActualDerived')
    # Sólo en el banco: conserva stores32 de descriptor pendientes de B2.
    base=base.replace('(DWORD)&DAT_00552508','static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&DAT_00552508))')
    derived=derived.replace('(DWORD)g_ClothAnchorVTable','static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_ClothAnchorVTable))')
    destroy=function(runtime,'FUN_00407ea0')
    base_destroy=function(runtime,'FUN_00407dc0')
    native='void* ecx' in destroy
    base_native='void* ecx' in base_destroy
    api=destroy[:destroy.index('{')]
    address='reinterpret_cast<std::uintptr_t>(ecx)' if native else 'static_cast<std::uintptr_t>(static_cast<DWORD>(ecx))'
    adapter=function(cloth,'ClothAnchor_VSlot0')
    slot1=function(cloth,'ClothAnchor_VSlot1')
    table=re.search(r'void \*g_ClothAnchorVTable\[3\]\s*=\s*\{.*?\};',cloth,re.S).group(0)
    replacements={'BASE':base,'DERIVED':derived,'DESTROY':destroy.replace('FUN_00407ea0','ActualDestroy'),'BASE_DESTROY':base_destroy.replace('FUN_00407dc0','ActualBaseDestroy'),'BASE_API_HEADER':base_destroy[:base_destroy.index('{')],'BASE_ADDRESS':'reinterpret_cast<std::uintptr_t>(ecx)' if base_native else 'static_cast<std::uintptr_t>(static_cast<DWORD>(ecx))','API_HEADER':api,'ADDRESS':address,'ADAPTER':adapter,'SLOT1':slot1,'TABLE':table}
    services=SERVICES
    for key,value in replacements.items():services=services.replace('\n'+key+'\n','\n'+value+'\n') if key in ('BASE','DERIVED','DESTROY','BASE_DESTROY','ADAPTER','SLOT1','TABLE') else services.replace(key,value)
    arg='object' if native else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(object))'
    base_arg='object' if base_native else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(object))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+MAIN.replace('BASE_ARG',base_arg).replace('ARG',arg)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-anchor-dtor-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
