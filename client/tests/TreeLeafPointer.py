#!/usr/bin/env python3
"""Borrado real de hoja: contexto/handle nativos y enlaces de nodo32."""
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
BYTE *contextMemory,*nodeMemory,*handleMemory,*leaf;BYTE expectedContext[64],expectedNodes[1024],beforeHandle[32];unsigned deletes;
void operator_delete(void* value){Require(value==leaf && !deletes++ && std::memcmp(contextMemory,expectedContext,64)==0 && std::memcmp(nodeMemory,expectedNodes,1024)==0 && std::memcmp(handleMemory,beforeHandle,32)==0);}
'''
MAIN=r'''
void Put(BYTE* p,int value){std::memcpy(p,&value,4);}int Address(BYTE* p){return static_cast<int>(reinterpret_cast<std::uintptr_t>(p));}
void Hash(std::uint64_t& h,const BYTE* p,unsigned n){for(unsigned i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* owner,BYTE* handle,BYTE* nodes){contextMemory=owner;handleMemory=handle;nodeMemory=nodes;leaf=nodes+16;BYTE* parent=nodes+128;BYTE* sibling=nodes+256;std::uint64_t hash=14695981039346656037ull;const int counts[]={0,1,2,17,32767,0x7fffffff,-1,-17},extras[]={0,-1,0x7fffffff,static_cast<int>(0x80000000u)};unsigned queries=0;
 for(unsigned pattern=0;pattern<256;++pattern)for(unsigned branch=0;branch<3;++branch)for(int count:counts){std::memset(owner,static_cast<int>(pattern),64);std::memset(nodes,0x6c,1024);std::memset(handle,0xa5,32);Put(owner+20,count);Put(owner+24,Address(branch?parent:leaf));Put(leaf+16,branch?Address(parent):0);Put(parent+8,Address(branch==1?leaf:sibling));Put(parent+12,Address(branch==2?leaf:sibling));Put(handle+16,Address(leaf));std::memcpy(expectedContext,owner,64);std::memcpy(expectedNodes,nodes,1024);std::memcpy(beforeHandle,handle,32);Put(expectedContext+20,count-1);if(branch==0)Put(expectedContext+24,0);else Put(expectedNodes+128+(branch==1?8:12),0);deletes=0;FUN_004236c0(owner+16,extras[pattern%4],reinterpret_cast<int*>(handle+16));Require(deletes==1 && std::memcmp(owner,expectedContext,64)==0 && std::memcmp(nodes,expectedNodes,1024)==0);Put(beforeHandle+16,0);Require(std::memcmp(handle,beforeHandle,32)==0);Hash(hash,owner,64);Hash(hash,nodes,1024);Hash(hash,handle,32);++queries;
 }Require(queries==6144);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+256,low+1024);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){if(mask&1)std::memset(low,0x7b,64);if(mask&2)std::memset(low+256,0x7b,32);Require(Run((mask&1)?high:low,(mask&2)?high+256:low+256,low+1024)==expected);if(mask&1)for(unsigned i=0;i<64;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<32;++i)Require(low[256+i]==0x7b);}std::puts("contexto/handle altos; enlaces32 bajos y orden delete/clear intactos");}std::printf("Tree leaf: 6144 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Net/Crypto.cpp',revision),'FUN_004236c0')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-tree-leaf-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
