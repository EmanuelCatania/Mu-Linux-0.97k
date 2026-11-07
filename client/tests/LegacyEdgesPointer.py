#!/usr/bin/env python3
"""Recorrido de aristas legado con base de vértices nativa; tabla de aristas32."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
BYTE* object;short* edges;float* vertices;unsigned calls,randomCalls,frame;int count,mesh;std::uint64_t hash;
int ControlledRandom(){const int numbers[]={0,1,99,100,101,32767,1234567,2147483647};Require(randomCalls==calls);return numbers[(frame+randomCalls++)%8];}
#define rand ControlledRandom
void Hash(unsigned value){for(unsigned i=0;i<4;++i){hash^=(value>>(i*8))&255;hash*=1099511628211ull;}}
void MoltSilhouette_DrawEdge(void* obj,float* first,float* second,float jitter){Require(obj==object && calls<static_cast<unsigned>(count) && randomCalls==calls+1);const short* edge=edges+calls*5;const int firstIndex=mesh*15000+edge[0],secondIndex=mesh*15000+edge[1];Require(first==vertices+firstIndex*3 && second==vertices+secondIndex*3);for(unsigned axis=0;axis<3;++axis){Require(first[axis]==static_cast<float>(firstIndex*3+int(axis))*.125f && second[axis]==static_cast<float>(secondIndex*3+int(axis))*.125f);std::uint32_t bits;std::memcpy(&bits,first+axis,4);Hash(bits);std::memcpy(&bits,second+axis,4);Hash(bits);}std::uint32_t bits;std::memcpy(&bits,&jitter,4);Hash(bits);++calls;}
'''
MAIN=r'''
void Put(BYTE* p,int value){std::memcpy(p,&value,4);}
std::uint64_t Run(BYTE* obj,BYTE* table,BYTE* palette){object=obj;edges=reinterpret_cast<short*>(table);vertices=reinterpret_cast<float*>(palette+1048576);hash=14695981039346656037ull;const int counts[]={-1,0,1,2,7,16,32},meshes[]={-1,0,1,2,3};const short indices[]={-32768,-16,-1,0,1,31,32767};unsigned queries=0;
 for(int i=-250000;i<250000;++i)vertices[i]=static_cast<float>(i)*.125f;
 std::vector<BYTE> paletteBefore(palette, palette+2097152);
 for(frame=0;frame<128;++frame)for(int value:counts)for(int part:meshes){count=value;mesh=part;std::memset(obj,0xa5,64);std::memset(table,0x6d,352);Put(obj+24,count);Put(obj+28,static_cast<int>(reinterpret_cast<std::uintptr_t>(table)));for(int i=0;i<(count>0?count:0);++i){edges[i*5]=indices[(frame+i)%7];edges[i*5+1]=indices[(frame*3+i+1)%7];edges[i*5+2]=static_cast<short>(mesh);edges[i*5+3]=static_cast<short>(0x5a5a);edges[i*5+4]=static_cast<short>(0xa5a5);}
  BYTE beforeObject[64],beforeEdges[352];std::memcpy(beforeObject,obj,64);std::memcpy(beforeEdges,table,352);calls=randomCalls=0;Draw(obj,vertices);Require(calls==static_cast<unsigned>(count>0?count:0) && randomCalls==calls && std::memcmp(beforeObject,obj,64)==0 && std::memcmp(beforeEdges,table,352)==0);Hash(calls);++queries;
 }Require(queries==4480 && std::memcmp(palette,paletteBefore.data(),2097152)==0);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low+1024,low+8192,low+1048576);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){std::memset(low+1024,0x7b,64);if(mask&2)std::memset(low+1048576,0x7b,2097152);Require(Run((mask&1)?high+1024:low+1024,low+8192,(mask&2)?high+1048576:low+1048576)==expected);if(mask&1)for(unsigned i=0;i<64;++i)Require(low[1024+i]==0x7b);if(mask&2)for(unsigned i=0;i<2097152;++i)Require(low[1048576+i]==0x7b);}std::puts("objeto/paleta altos; tabla32 baja, indices signed y aliases intactos");}std::printf("Legacy edges: 4480 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Scene/Scene_LegacyHelpers.cpp',revision)
    body=function(source,'VTable_Release')
    args='obj,vertices' if 'float* vertices' in body else 'obj,(int)vertices'
    wrapper='\nvoid Draw(void* obj,float* vertices){VTable_Release('+args+');}\n'
    edge=function(read_source('src/Terrain/Terrain.cpp',revision),'FUN_0040a8f0')
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x400000')+SERVICES+edge+body+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-legacy-edges-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
