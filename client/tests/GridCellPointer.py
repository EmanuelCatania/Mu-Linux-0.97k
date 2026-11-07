#!/usr/bin/env python3
"""Unlink real: celda nativa alta, enlaces32 y callbacks de registro/liberación controlados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
BYTE *nodes,*cellMemory,*selected;BYTE beforeNodes[8192],expectedNodes[8192],beforeCell[48],expectedCell[48];unsigned callbacks;
namespace SceneObjects{void Unregister(void* node){Require(node==selected && callbacks++==0 && std::memcmp(nodes,beforeNodes,8192)==0 && std::memcmp(cellMemory,beforeCell,48)==0);}}
void operator_delete(void* node){Require(node==selected && callbacks++==1 && std::memcmp(nodes,expectedNodes,8192)==0 && std::memcmp(cellMemory,expectedCell,48)==0);}
'''
MAIN=r'''
void Put(BYTE* p,int value){std::memcpy(p,&value,4);}int Address(BYTE* p){return static_cast<int>(reinterpret_cast<std::uintptr_t>(p));}
void Hash(std::uint64_t& h,const BYTE* p,unsigned size){for(unsigned i=0;i<size;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* cell,BYTE* storage){cellMemory=cell;nodes=storage;std::uint64_t hash=14695981039346656037ull;unsigned removals=0;
 for(unsigned trial=0;trial<256;++trial){std::memset(cell,0xa5,48);std::memset(nodes,0x6d,8192);std::vector<unsigned> chain;const unsigned count=trial%17;for(unsigned i=0;i<count;++i)chain.push_back(i);Put(cell+20,count?Address(nodes):0);Put(cell+24,count?Address(nodes+(count-1)*444):0);
  for(unsigned i=0;i<count;++i){Put(nodes+i*444+436,i?Address(nodes+(i-1)*444):0);Put(nodes+i*444+440,i+1<count?Address(nodes+(i+1)*444):0);}
  std::memcpy(beforeNodes,nodes,8192);std::memcpy(beforeCell,cell,48);callbacks=0;Unlink(nullptr,nullptr);Require(callbacks==0 && std::memcmp(nodes,beforeNodes,8192)==0 && std::memcmp(cell,beforeCell,48)==0);
  unsigned seed=trial*1664525u+1013904223u;while(!chain.empty()){seed=seed*1664525u+1013904223u;const unsigned index=seed%chain.size(),chosen=chain[index];selected=nodes+chosen*444;std::memcpy(beforeNodes,nodes,8192);std::memcpy(expectedNodes,nodes,8192);std::memcpy(beforeCell,cell,48);std::memcpy(expectedCell,cell,48);
   const int previous=index?static_cast<int>(chain[index-1]):-1,next=index+1<chain.size()?static_cast<int>(chain[index+1]):-1;if(previous>=0)Put(expectedNodes+previous*444+440,next>=0?Address(nodes+next*444):0);else Put(expectedCell+20,next>=0?Address(nodes+next*444):0);if(next>=0)Put(expectedNodes+next*444+436,previous>=0?Address(nodes+previous*444):0);else Put(expectedCell+24,previous>=0?Address(nodes+previous*444):0);
   callbacks=0;Unlink(selected,cell+16);Require(callbacks==2 && std::memcmp(nodes,expectedNodes,8192)==0 && std::memcmp(cell,expectedCell,48)==0);chain.erase(chain.begin()+index);Hash(hash,nodes,8192);Hash(hash,cell,48);++removals;
  }
 }Require(removals==2040);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,48);Require(Run(high,low+4096)==expected);for(unsigned i=0;i<48;++i)Require(low[i]==0x7b);std::puts("celda alta; enlaces32 bajos, orden de callbacks y alias intactos");}std::printf("Grid cell: 256 listas/2040 bajas %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Scene/Scene_LegacyHelpers.cpp',revision)
    body=function(source,'Entity_GridUnlink')
    args='node,cell' if 'void* cell' in body else 'node,(int)cell'
    wrapper='\nvoid Unlink(void* node,void* cell){Entity_GridUnlink('+args+');}\n'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+body+wrapper+MAIN

def main():
    import sys
    if "Win32" not in sys.argv:
        print("Omitido en x64: banco atado al layout x86; lo cubren los bancos nativos de B2")
        return
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-grid-cell-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
