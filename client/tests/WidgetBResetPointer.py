#!/usr/bin/env python3
"""Reset WidgetB y ambos constructores reales, con bytes x86 y dirección nativa."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
std::uint64_t Run(BYTE* data){std::uint64_t hash=14695981039346656037ull;unsigned calls=0;
 for(unsigned trial=0;trial<8192;++trial)for(unsigned route=0;route<3;++route){unsigned state=trial*1664525u+1013904223u;for(unsigned i=0;i<128;++i){state=state*1664525u+1013904223u;data[i]=static_cast<BYTE>(state>>24);}const unsigned start=16+(trial%4)*4;BYTE* widget=data+start;BYTE expected[128];std::memcpy(expected,data,128);std::memset(expected+start+4,0,2);std::memset(expected+start+8,0,4);std::memset(expected+start+24,0,8);
  for(unsigned repeat=0;repeat<2;++repeat){Require(Call(route,widget)==widget && std::memcmp(data,expected,128)==0);++calls;}for(unsigned i=0;i<128;++i){hash^=data[i];hash*=1099511628211ull;}
 }Require(calls==49152);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("widget alto; reset/retornos de constructores nativos y alias intacto");}std::printf("WidgetB reset: 8192 patrones/49152 llamadas %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    reset=function(source,'WidgetB_ZeroFields')
    args='widget' if 'void* widget' in reset else '(int)widget'
    wrapper='\nvoid* Call(unsigned route,void* widget){if(route==0){WidgetB_ZeroFields('+args+');return widget;}return route==1?WidgetB_Ctor(widget):WidgetB_CtorFull(widget);}\n'
    fastcall='\n#ifndef _WIN32\n#define __fastcall\n#endif\n'
    body=reset+function(source,'WidgetB_SetVtable')+function(source,'WidgetB_Ctor')+function(read_source('src/Scene/Scene_LegacyHelpers.cpp',revision),'WidgetB_CtorFull')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+fastcall+body+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-widgetb-reset-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
