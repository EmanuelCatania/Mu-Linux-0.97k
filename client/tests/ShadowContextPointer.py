#!/usr/bin/env python3
"""Contexto real de sombra: normalización y rama de liberación controlada."""
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
BYTE *target,*allocation;bool forced;unsigned stage;int first,second,third;char flag;
ACTUAL
void Vec3_Normalize(float* vector){Require(stage++==0 && reinterpret_cast<BYTE*>(vector)==target+12);const DWORD expected[]={0xbf800000u,0x3cf5c28f,0xbf800000u};Require(std::memcmp(vector,expected,12)==0);ActualNormalize(vector);}
int __fastcall FUN_00409f30(void* context,void* edx,int a,int b,int c,char d){Require(stage++==1 && context==target && !edx && a==first && b==second && c==third && d==flag);const int result=ActualFind(context,edx,a,b,c,d);Require(result==0);return forced?1:result;}
void __fastcall FUN_0040a300(void* context,void* edx,int a){Require(stage++==2 && forced && context==target && !edx && a==first);ActualBuild(context,edx,a);}
void operator_delete(void* p){Require(stage++==3 && forced && p==allocation);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory,BYTE* data){target=memory+16;allocation=data+16;std::uint64_t hash=14695981039346656037ull;const int values[]={0,-1,0x7fffffff,static_cast<int>(0x80000000u)};unsigned calls=0;
 for(unsigned pattern=0;pattern<4096;++pattern)for(unsigned force=0;force<2;++force){unsigned state=pattern*1664525u+1013904223u;for(unsigned i=0;i<96;++i){state=state*1664525u+1013904223u;memory[i]=static_cast<BYTE>(state>>24);}std::memset(data,0x6c,64);const std::uintptr_t pointer=reinterpret_cast<std::uintptr_t>(allocation);const std::uint64_t legacyPointer=pointer;std::memcpy(target+28,&legacyPointer,8);BYTE before[96],beforeAllocation[64];std::memcpy(before,memory,96);std::memcpy(beforeAllocation,data,64);forced=force!=0;first=values[pattern%4];second=values[(pattern/4)%4];third=values[(pattern/16)%4];flag=static_cast<char>(pattern);stage=0;FUN_0040a0a0(target,values[(pattern/64)%4],first,second,third,flag);Require(stage==(forced?4u:2u) && std::memcmp(data,beforeAllocation,64)==0);for(unsigned i=0;i<96;++i)if(i<28 || i>=40)Require(memory[i]==before[i]);float vector[3];std::memcpy(vector,target+12,12);const double length=std::sqrt(double(vector[0])*vector[0]+double(vector[1])*vector[1]+double(vector[2])*vector[2]);Require(std::fabs(length-1)<.000001 && vector[0]<0 && vector[1]>0 && vector[2]<0 && vector[0]==vector[2] && std::fabs(double(vector[1])/(-vector[0])-.03)<.000001);for(unsigned i=0;i<96;++i){hash^=memory[i];hash*=1099511628211ull;}hash^=stage;hash*=1099511628211ull;++calls;
 }Require(calls==8192);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,96);Require(Run(high,low+4096)==expected);for(unsigned i=0;i<96;++i)Require(low[i]==0x7b);std::puts("contexto alto; vector real y buffer interno bajo/zero-extend compatible");}std::printf("Shadow context: 8192 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    actual=function(read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision),'Vec3_Normalize').replace('Vec3_Normalize','ActualNormalize')+function(source,'FUN_00409f30').replace('FUN_00409f30','ActualFind')+function(source,'FUN_0040a300').replace('FUN_0040a300','ActualBuild')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('ACTUAL',actual)+function(source,'FUN_0040a0a0')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-shadow-context-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
