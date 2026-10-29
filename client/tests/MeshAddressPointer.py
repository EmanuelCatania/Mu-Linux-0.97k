#!/usr/bin/env python3
"""MeshPtr real: base nativa y suma previa a cruzar los límites de32bits."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
std::uint64_t Run(BYTE* memory,BYTE* storage,std::uintptr_t displacement){
 BYTE* model=memory+16;const std::uintptr_t base=reinterpret_cast<std::uintptr_t>(storage)+displacement;
 Require(base<=0xffffffffull);
 const int indices[]={0,1,2,3,7,31,204,205,1023,32767};
 const unsigned offsets[]={0,40,80,120,280,1240,8160,8200,40920,1310680};
 std::uint64_t hash=14695981039346656037ull;
 for(unsigned pattern=0;pattern<256;++pattern){
  for(unsigned i=0;i<220;++i)memory[i]=static_cast<BYTE>(pattern+i*37);
  const DWORD pointer=static_cast<DWORD>(base);std::memcpy(model+0x28,&pointer,4);
  const DWORD neighbor=0xa5ff8000u|pattern;std::memcpy(model+0x2c,&neighbor,4);
  BYTE before[220];std::memcpy(before,memory,220);
  for(unsigned entry=0;entry<10;++entry){
   const int index=indices[entry];const auto expected=base+offsets[entry];
   Require(expected>=reinterpret_cast<std::uintptr_t>(storage) && expected+40<=reinterpret_cast<std::uintptr_t>(storage)+0x400000);
   BYTE payload[40];for(unsigned b=0;b<40;++b)payload[b]=static_cast<BYTE>(pattern+unsigned(index)*13+b*7);
   std::memcpy(reinterpret_cast<void*>(expected),payload,40);
   char* result=MeshPtr(model,index);
   Require(reinterpret_cast<std::uintptr_t>(result)==expected);
   Require(std::memcmp(result,payload,40)==0 && std::memcmp(memory,before,220)==0);
   hash^=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(result)-base);hash*=1099511628211ull;
   for(BYTE value:payload){hash^=value;hash*=1099511628211ull;}
  }
 }
 return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 auto* first=static_cast<BYTE*>(Reserve(0x28000000u));Require(first);
 const auto expected=Run(low,first,0x200000);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  std::memset(low,0x7b,220);Require(Run(high,first,0x200000)==expected);for(unsigned i=0;i<220;++i)Require(low[i]==0x7b);
  const std::uintptr_t addresses[]={0x8fe00000ull,0x7fff0000ull,0xffff0000ull};
  for(unsigned variant=0;variant<3;++variant){
   auto* storage=static_cast<BYTE*>(Reserve(addresses[variant]));Require(storage);
   const std::uintptr_t offset=variant==0?0x200000:0xe000;
   Require(Run(low,storage,offset)==expected);
   std::memset(low,0x7b,220);Require(Run(high,storage,offset)==expected);
   for(unsigned i=0;i<220;++i)Require(low[i]==0x7b);
  }
  std::puts("modelo alto; tabla32 con bit31/cruces2y4GiB, malla y alias intactos");
 }
 std::printf("Mesh pointer: 2560 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Physics/Cloth_Simulation.cpp',revision)
    # Normaliza sólo la extracción; el cuerpo compilado conserva char*.
    mesh=function(source.replace('char* MeshPtr','BYTE* MeshPtr'),'MeshPtr').replace('BYTE* MeshPtr','char* MeshPtr')
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x400000')+mesh+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-mesh-pointer-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
