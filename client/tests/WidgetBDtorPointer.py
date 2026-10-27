#!/usr/bin/env python3
"""Destructor WidgetB real: puntero completo y flags byte sin cambiar el port."""
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
BYTE* target;BYTE* memory;BYTE expected[128];unsigned stages,flags;
VTABLE
void WidgetB_SetVtable(void* widget){Require(widget==target && stages++==0 && std::memcmp(memory,expected,128)==0);ActualSetVtable(widget);}
void operator_delete(void* widget){Require(widget==target && (flags&1) && stages++==1 && std::memcmp(memory,expected,128)==0);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* data){memory=data;std::uint64_t hash=14695981039346656037ull;const int extras[]={0,-1,0x7fffffff,static_cast<int>(0x80000000u)};unsigned calls=0;
 for(unsigned trial=0;trial<128;++trial)for(flags=0;flags<256;++flags)for(int unused:extras)for(unsigned construct=0;construct<2;++construct){unsigned state=trial*1664525u+1013904223u;for(unsigned i=0;i<128;++i){state=state*1664525u+1013904223u;data[i]=static_cast<BYTE>(state>>24);}const unsigned start=16+(trial%4)*4;target=data+start;std::memcpy(expected,data,128);if(construct){std::memset(expected+start+4,0,2);std::memset(expected+start+8,0,4);std::memset(expected+start+24,0,8);Require(WidgetB_Ctor(target)==target && std::memcmp(data,expected,128)==0);}
  stages=0;Destroy(target,unused,static_cast<BYTE>(flags));Require(stages==1+(flags&1) && std::memcmp(data,expected,128)==0);for(unsigned i=0;i<128;++i){hash^=data[i];hash*=1099511628211ull;}hash^=stages;hash*=1099511628211ull;++calls;
 }Require(calls==262144);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expectedHash=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expectedHash);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("widget alto; setter/free con identidad nativa y alias intacto");}std::printf("WidgetB dtor: 262144 llamadas/flags0..255 %016llx\n",static_cast<unsigned long long>(expectedHash));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    setter=function(source,'WidgetB_SetVtable').replace('WidgetB_SetVtable','ActualSetVtable')
    destructor=function(source,'FUN_00409ef0')
    args='widget,unused,flags' if 'void* widget' in destructor else '(int)widget,unused,flags'
    wrapper='\nvoid Destroy(void* widget,int unused,BYTE flags){FUN_00409ef0('+args+');}\n'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('VTABLE',setter)+function(source,'WidgetB_ZeroFields')+function(source,'WidgetB_Ctor')+destructor+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-widgetb-dtor-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
