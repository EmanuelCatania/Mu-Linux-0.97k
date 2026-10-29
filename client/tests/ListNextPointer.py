#!/usr/bin/env python3
"""Recorrido real de lista: dirección del nodo siguiente de ancho nativo.

Los centinelas y los campos de enlace32 permanecen bajos. No certifica un
layout de lista nativo: conserva incluso las lecturas void** del port.
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
#endif
BYTE *low,*high;BYTE expectedLow[4096],expectedHigh[4096];
void* wanted[10];unsigned events,total;bool poison;std::uint64_t digest;
void Store(BYTE* p,const void* value,unsigned size){
 std::memcpy(p,value,size);
 if(p>=low && p<low+4096)std::memcpy(expectedLow+(p-low),value,size);
 else {Require(p>=high && p<high+4096);std::memcpy(expectedHigh+(p-high),value,size);}
}
void Check(){Require(std::memcmp(low,expectedLow,4096)==0 && std::memcmp(high,expectedHigh,4096)==0);}
BYTE *head,*tail,*last;int* header;
void operator_delete(void* p){
 Require(events<total+2 && p==wanted[events]);
 if(events==total){
  const DWORD tail32=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(tail));
  const DWORD head32=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(head));
  std::memcpy(expectedLow+(head-low)+8,&tail32,4);
  std::memcpy(expectedLow+(tail-low)+4,&head32,4);
  BYTE* expectedHeader=reinterpret_cast<BYTE*>(header)>=high && reinterpret_cast<BYTE*>(header)<high+4096?expectedHigh+(reinterpret_cast<BYTE*>(header)-high):expectedLow+(reinterpret_cast<BYTE*>(header)-low);
  std::memset(expectedHeader,0,4);
 }
 Check();digest^=events+1;digest*=1099511628211ull;++events;
 if(poison){BYTE bytes[32];std::memset(bytes,0xd3,32);Store(static_cast<BYTE*>(p),bytes,32);}
}
'''
MAIN=r'''
std::uint64_t Run(bool highNodes,bool highHeader){
 digest=14695981039346656037ull;unsigned cases=0;
 for(unsigned pattern=0;pattern<64;++pattern)for(unsigned count=0;count<=8;++count)for(unsigned alignment=0;alignment<2;++alignment)for(unsigned overwrite=0;overwrite<2;++overwrite){
  for(unsigned i=0;i<4096;++i){low[i]=static_cast<BYTE>(pattern+i*37);high[i]=static_cast<BYTE>(pattern+i*17+3);}
  std::memcpy(expectedLow,low,4096);std::memcpy(expectedHigh,high,4096);
  head=low+16+alignment*8;tail=low+80+alignment*8;last=low+144+alignment*8;
  header=reinterpret_cast<int*>((highHeader?high:low)+2048+alignment*8);total=count;events=0;poison=overwrite!=0;
  BYTE* nodes[8];
  for(unsigned i=0;i<count;++i){nodes[i]=i==count-1?last:(highNodes?high:low)+208+i*64+alignment*8;wanted[i]=nodes[i];}
  wanted[count]=tail;wanted[count+1]=head;
  const int initialCount=static_cast<int>(pattern*0x10204081u);Store(reinterpret_cast<BYTE*>(header),&initialCount,4);
  const DWORD head32=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(head)),tail32=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(tail));
  Store(reinterpret_cast<BYTE*>(header)+4,&head32,4);Store(reinterpret_cast<BYTE*>(header)+8,&tail32,4);
  const DWORD prev=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(count?last:head));Store(tail+4,&prev,4);
  void* first=count?static_cast<void*>(nodes[0]):tail;Store(head+8,&first,sizeof(first));
  for(unsigned i=0;i<count;++i){void* next=i+1<count?static_cast<void*>(nodes[i+1]):tail;Store(nodes[i]+8,&next,sizeof(next));}
  // El cuerpo corta el enlace del último nodo antes de empezar a liberar.
  std::memset(expectedLow+((count?last:head)-low)+8,0,4);
  LinkedList_DestroyAll(header);Require(events==count+2);Check();
  // El oráculo compara todos los bytes; el digest excluye direcciones y ancho.
  digest^=pattern+count*67+alignment*1024+overwrite*2048;digest*=1099511628211ull;++cases;
 }
 Require(cases==2304);return digest;
}
int main(){
 low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 // En x86, el segundo bloque sólo sirve de canario; no contiene enlaces altos.
 high=static_cast<BYTE*>(Reserve(sizeof(void*)>4?std::uintptr_t(0x126000000ull):std::uintptr_t(0x28000000u)));Require(high);
 const auto expected=Run(false,false);
 if(sizeof(void*)>4){Require(Run(true,false)==expected);Require(Run(false,true)==expected);Require(Run(true,true)==expected);std::puts("nodos/contexto altos; bytes, aliases y orden de liberacion correctos");}
 std::printf("List next: 2304 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(source,'LinkedList_DestroyAll')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-list-next-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
