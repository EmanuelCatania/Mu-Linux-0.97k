#!/usr/bin/env python3
"""Fila superior real de tela con widget/matriz/modelos nativos; owner/nodos32."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
#ifndef _WIN32
#define __fastcall
#endif
BYTE* DAT_05828d58;unsigned transforms;
namespace Platform{struct Timer{DWORD Ticks(){++transforms;return 1;}}timer;Timer& GetTimer(){return timer;}}
'''
MAIN=r'''
void PutInt(BYTE* p,int value){std::memcpy(p,&value,4);}void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}int Address(BYTE* p){return static_cast<int>(reinterpret_cast<std::uintptr_t>(p));}
void Hash(std::uint64_t& h,const BYTE* p,unsigned size){for(unsigned i=0;i<size;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* widget,BYTE* matrix,BYTE* models,BYTE* low){BYTE* nodes=low+8192;BYTE* entity=low+4096;DAT_05828d58=models+188;std::uint64_t hash=14695981039346656037ull;const int widths[]={-1,0,1,2,3,8},flagsList[]={0,1,4,5,8,9,0x104,0x205};unsigned queries=0;
 for(unsigned frame=0;frame<128;++frame)for(int width:widths)for(int flags:flagsList){if(width==1 && ((flags&3)==1 || (flags&12)==4))continue;std::memset(widget,0xa5,84);std::memset(matrix,0x6d,48);std::memset(models,0x7e,564);std::memset(nodes,0x6c,1024);std::memset(entity,0x7f,444);const short modelType=static_cast<short>(int(frame%3)-1);std::memcpy(entity+2,&modelType,2);PutInt(widget+4,Address(entity));PutInt(widget+20,flags);PutFloat(widget+24,float(int(frame%9)-4)*.25f);PutFloat(widget+28,float(int(frame%11)-5)*.5f);PutFloat(widget+32,float(int(frame%17)-8)*.5f);PutInt(widget+40,width);PutInt(widget+52,Address(nodes));PutFloat(widget+64,float(int(frame%7)-3)*.125f);
  for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column)PutFloat(matrix+(row*4+column)*4,column==row?1.0f:column==3?float(int(frame%5)-2)*float(row+1):0.0f);
  BYTE* model=DAT_05828d58+modelType*188;PutFloat(model+104,float(frame%4)*.5f);for(unsigned axis=0;axis<3;++axis)PutFloat(model+108+axis*4,float(int(frame%13)-6)*float(axis+1)*.125f);
  BYTE beforeWidget[84],beforeMatrix[48],beforeModels[564],beforeNodes[1024],beforeEntity[444];std::memcpy(beforeWidget,widget,84);std::memcpy(beforeMatrix,matrix,48);std::memcpy(beforeModels,models,564);std::memcpy(beforeNodes,nodes,1024);std::memcpy(beforeEntity,entity,444);transforms=0;Pin(widget,width>0?reinterpret_cast<float(*)[4]>(matrix):nullptr);Require(transforms==unsigned(width>0?width:0) && std::memcmp(widget,beforeWidget,84)==0 && std::memcmp(matrix,beforeMatrix,48)==0 && std::memcmp(models,beforeModels,564)==0 && std::memcmp(entity,beforeEntity,444)==0);
  for(unsigned i=0;i<1024;++i){const unsigned node=i/60,offset=i%60;if(width>0 && node<static_cast<unsigned>(width) && offset>=28 && offset<40)continue;const BYTE expected=width>0 && node<static_cast<unsigned>(width) && offset==40?static_cast<BYTE>(beforeNodes[i]|1):beforeNodes[i];Require(nodes[i]==expected);}for(int node=0;node<width;++node)for(unsigned axis=0;axis<3;++axis){float value;std::memcpy(&value,nodes+node*60+28+axis*4,4);Require(std::isfinite(value));}Hash(hash,nodes,1024);++queries;
 }Require(queries==5376);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low+1024,low+2048,low+16384,low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<8;++mask){if(mask&1)std::memset(low+1024,0x7b,84);if(mask&2)std::memset(low+2048,0x7b,48);if(mask&4)std::memset(low+16384,0x7b,564);Require(Run((mask&1)?high+1024:low+1024,(mask&2)?high+2048:low+2048,(mask&4)?high+16384:low+16384,low)==expected);if(mask&1)for(unsigned i=0;i<84;++i)Require(low[1024+i]==0x7b);if(mask&2)for(unsigned i=0;i<48;++i)Require(low[2048+i]==0x7b);if(mask&4)for(unsigned i=0;i<564;++i)Require(low[16384+i]==0x7b);}std::puts("widget/matriz/modelos altos; owner/nodos32 bajos y aliases intactos");}std::printf("Cloth pin: 5376 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Physics/Cloth_Simulation.cpp',revision)
    body=function(source,'Cloth_PinTopRow');args='widget,matrix' if 'void* widget' in body else '(int)widget,matrix'
    wrapper='\nvoid Pin(void* widget,float(*matrix)[4]){Cloth_PinTopRow('+args+');}\n'
    math=read_source('src/Math/Math_3D.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(math,'Vector_Transform')+function(math,'BMD_TransformPosition')+function(source,'SpringNode_SetPos')+body+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-cloth-pin-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
