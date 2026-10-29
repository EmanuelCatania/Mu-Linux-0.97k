#!/usr/bin/env python3
"""Bloque real de hash del HUD con las dos variantes reales del stub.

Ejecuta factory real compartida; Insert controlado. No ejecuta el HUD
completo ni certifica hash anti-tamper activo o layouts nativos.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __fastcall
using UINT=unsigned int;
#endif
using uint=unsigned int;
void* CharacterMachine;DWORD DAT_055c9bd4;BYTE* context;BYTE* keyMemory;BYTE* realNode;
#define MAIN_HASH_CLASS (*reinterpret_cast<DWORD*>(context+16))
unsigned stage;bool native;
LEGACY
NATIVE
FACTORY
uint __fastcall HashTable_GetIndex(void* ctx,void* edx,DWORD key){Require(!native && stage++==0 && ctx==&MAIN_HASH_CLASS && edx==nullptr && static_cast<std::uintptr_t>(key)==reinterpret_cast<std::uintptr_t>(CharacterMachine));return ActualLegacy(ctx,edx,key);}
uint __cdecl HashTable_GetIndex(void* ctx,void* key){Require(native && stage++==0 && ctx==&MAIN_HASH_CLASS && key==CharacterMachine);return ActualNative(ctx,key);}
void* AntiTamper_HashNode(){Require(stage++==1);void* node=ActualFactory();Require(node==realNode);return node;}
void HashTable_Insert(void* ctx,void* value,void* key){Require(stage++==2 && ctx==&MAIN_HASH_CLASS && value==realNode && key==CharacterMachine && realNode[1412]==1);}
void Block(){
BLOCK
}
'''
MAIN=r'''
std::uint64_t Run(BYTE* storage){
 keyMemory=storage;const DWORD capacities[]={0,1,2,0x7fffffffu,0x80000000u,0xffffffffu};
 BYTE ctx[64];context=ctx;realNode=static_cast<BYTE*>(ActualFactory());Require(realNode);
 std::uint64_t hash=14695981039346656037ull;unsigned calls=0;
 for(unsigned pattern=0;pattern<256;++pattern)for(DWORD capacity:capacities)for(unsigned guard=0;guard<2;++guard){
  for(unsigned i=0;i<128;++i)keyMemory[i]=static_cast<BYTE>(pattern+i*37);
  for(unsigned i=0;i<64;++i)context[i]=static_cast<BYTE>(pattern+i*13);
  for(unsigned i=0;i<0x585;++i)realNode[i]=static_cast<BYTE>(pattern+i*7);
  BYTE beforeKey[128],beforeCtx[64],expectedNode[0x585];std::memcpy(beforeKey,keyMemory,128);std::memcpy(beforeCtx,context,64);std::memcpy(expectedNode,realNode,0x585);
  CharacterMachine=guard?nullptr:keyMemory+16+(pattern%8)*4;DAT_055c9bd4=capacity;if(!guard)expectedNode[1412]=1;
  native=NATIVE_FLAG;stage=0;Block();
  Require(stage==(guard?0u:3u) && DAT_055c9bd4==capacity && CharacterMachine==(guard?nullptr:keyMemory+16+(pattern%8)*4));
  Require(std::memcmp(keyMemory,beforeKey,128)==0 && std::memcmp(context,beforeCtx,64)==0 && std::memcmp(realNode,expectedNode,0x585)==0);
  for(BYTE value:expectedNode){hash^=value;hash*=1099511628211ull;}hash^=stage;hash*=1099511628211ull;++calls;
 }
 Require(calls==3072);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("clave alta; stubs/factory reales,marca,orden y alias intactos");}
 std::printf("HUD hash context: 3072 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Render/HUD_Pass1.cpp',revision)
    body=function(source,'RenderMainFrameWindow_');start=body.index('if (CharacterMachine)');brace=body.index('{',start);end=brace+1;depth=1
    while depth:
        depth+=(body[end]=='{')-(body[end]=='}');end+=1
    block=body[start:end]
    system=read_source('src/Core/System_Legacy.cpp',revision)
    header=read_source('src/functions.h',revision)
    # uint conserva exactamente el tipo unsigned int de ambos contratos.
    legacy=function(system.replace('UINT __fastcall HashTable_GetIndex','uint __fastcall HashTable_GetIndex'),'HashTable_GetIndex').replace('HashTable_GetIndex','ActualLegacy')
    native=function(header.replace('unsigned int HashTable_GetIndex','uint HashTable_GetIndex'),'HashTable_GetIndex').replace('HashTable_GetIndex','ActualNative')
    factory=function(system,'AntiTamper_HashNode').replace('AntiTamper_HashNode','ActualFactory')
    services=SERVICES.replace('LEGACY',legacy).replace('NATIVE',native).replace('FACTORY',factory).replace('BLOCK',block)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+MAIN.replace('NATIVE_FLAG','true' if '(DWORD)v0' not in block else 'false')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-hud-hash-context-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
