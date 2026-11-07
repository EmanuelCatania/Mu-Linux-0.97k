#!/usr/bin/env python3
"""Viento real: promoción nativa de base32, bit31 y pool que cruza4GiB."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
DWORD MoveSceneFrame;float DAT_00590af4,DAT_00590af8,DAT_00590afc,_DAT_00590af0,_DAT_00559068=9.8f,_DAT_0055906c=.0025f;
BYTE* nodes;unsigned calls;int count,flags;
'''
OBSERVER=r'''
void ObserveForces(uintptr_t node,int distance,short actualFlags){Require(calls<static_cast<unsigned>(count) && node==reinterpret_cast<uintptr_t>(nodes)+calls*60 && distance>=0 && actualFlags==static_cast<short>(flags));++calls;ClothNode_ApplyForces(node,distance,actualFlags);}
'''
MAIN=r'''
void PutInt(BYTE* p,int value){std::memcpy(p,&value,4);}void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}
std::uint64_t Run(BYTE* widget,BYTE* memory){nodes=memory+16;std::uint64_t hash=14695981039346656037ull;const int counts[]={-1,0,1,2,3,8,16,32,96},widths[]={-1,0,1,2,3,8,17},flagList[]={0,1,0x100,0x200,0x400,0x800,0x500,0xc00,0xffff},frames[]={0,1,9,10,11,19,20,99,100,101,32767,-1,-10,-1000};
 for(unsigned trial=0;trial<4096;++trial){count=counts[trial%9];const int width=widths[(trial/9)%7];flags=flagList[(trial/13)%9];MoveSceneFrame=static_cast<DWORD>(frames[(trial/7)%14]);DAT_00590af4=float(int(trial%9)-4)*.125f;DAT_00590af8=float(int(trial%7)-3)*.25f;DAT_00590afc=float(int(trial%5)-2)*.0625f;_DAT_00590af0=float(trial%4)*.125f;std::memset(widget,0xa5,84);std::memset(memory,0x6c,5792);PutInt(widget+20,flags);PutInt(widget+40,width);PutInt(widget+48,count);PutInt(widget+52,static_cast<int>(reinterpret_cast<uintptr_t>(nodes)));
  for(int node=0;node<count;++node){for(unsigned axis=0;axis<3;++axis)PutFloat(nodes+node*60+16+axis*4,float(int((trial+node+axis)%19)-9)*.25f);nodes[node*60+40]=static_cast<BYTE>((trial+node)%4==0?0:(trial+node)%4==1?1:(trial+node)%4==2?0x80:0x81);}
  BYTE beforeWidget[84],before[5792];std::memcpy(beforeWidget,widget,84);std::memcpy(before,memory,5792);calls=0;Cloth_Wind(reinterpret_cast<DWORD*>(widget));const int active=count>0 && width>0?count:0;Require(calls==static_cast<unsigned>(active) && std::memcmp(widget,beforeWidget,84)==0);
  for(unsigned i=0;i<5792;++i){if(i>=16){const unsigned node=(i-16)/60,offset=(i-16)%60;if(node<static_cast<unsigned>(active) && offset>=4 && offset<16)continue;}Require(memory[i]==before[i]);}
  for(int node=0;node<active;++node)for(unsigned axis=0;axis<3;++axis){float acceleration;std::memcpy(&acceleration,nodes+node*60+4+axis*4,4);Require(std::isfinite(acceleration));if(nodes[node*60+40]&1)Require(acceleration==0);else if(axis<2 && (axis==0?DAT_00590af4:DAT_00590af8)==0){float velocity;std::memcpy(&velocity,nodes+node*60+16+axis*4,4);Require(std::fabs(acceleration+velocity*.01f)<.00001f);}}
  for(unsigned i=0;i<5792;++i){hash^=memory[i];hash*=1099511628211ull;}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low+8192,low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* bit31=static_cast<BYTE*>(Reserve(0x90000000u));auto* crossing=static_cast<BYTE*>(Reserve(std::uint64_t(0xffff0000ull)));Require(high && bit31 && crossing);const std::uintptr_t starts[]={reinterpret_cast<std::uintptr_t>(low),reinterpret_cast<std::uintptr_t>(bit31),reinterpret_cast<std::uintptr_t>(crossing+0xff00)};for(unsigned address=0;address<3;++address)for(unsigned widgetHigh=0;widgetHigh<2;++widgetHigh){if(address==0 && !widgetHigh)continue;BYTE* memory=reinterpret_cast<BYTE*>(starts[address]);if(address)std::memset(low,0x7b,5792);if(widgetHigh)std::memset(low+8192,0x7b,84);Require(Run(widgetHigh?high+8192:low+8192,memory)==expected);if(address)for(unsigned i=0;i<5792;++i)Require(low[i]==0x7b);if(widgetHigh)for(unsigned i=0;i<84;++i)Require(low[8192+i]==0x7b);}std::puts("widget alto, nodos con bit31 y cruce4GiB; base32 y aliases intactos");}std::printf("Cloth wind: 4096 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Physics/Cloth_Simulation.cpp',revision)
    body=function(source,'Cloth_Wind').replace('ClothNode_ApplyForces(','ObserveForces(')
    extra='';observer,main=OBSERVER,MAIN
    if 'ClothSystem(' in body:
        import ClothFixture
        extra=ClothFixture.prefix();observer,main=ClothFixture.wind(observer,main)
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x20000')+extra+SERVICES+function(source,'ClothNode_ApplyForces')+observer+body+main

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-cloth-wind-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
