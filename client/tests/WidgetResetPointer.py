#!/usr/bin/env python3
"""Reset escalar real de tela y borrado real de muestra con clave nativa."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <unordered_map>
TYPES
std::unordered_map<const void*,Sample> samples;
BYTE* target;BYTE before[128];BYTE* memory;unsigned forgets;
FORGET
void ClothPresentation_Forget(const void* widget){Require(widget==target && std::memcmp(memory,before,128)==0);++forgets;ForgetActual(widget);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* data){memory=data;std::uint64_t hash=14695981039346656037ull;int other;const void* otherKey=&other;
 for(unsigned trial=0;trial<4096;++trial){unsigned state=trial*1664525u+1013904223u;for(unsigned i=0;i<128;++i){state=state*1664525u+1013904223u;data[i]=static_cast<BYTE>(state>>24);}const unsigned start=16+(trial%4)*4;target=data+start;std::memcpy(before,data,128);BYTE expected[128];std::memcpy(expected,data,128);
  for(unsigned i=4;i<72;++i)expected[start+i]=0;const std::uint32_t one=0x3f800000u;for(unsigned offset:{32,36,64,68})std::memcpy(expected+start+offset,&one,4);
  samples.clear();samples[target].step=trial;samples[otherKey].step=123456;const void* alias=reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(target)&0xffffffffull);if(alias!=target)samples[alias].step=789012;const unsigned remaining=alias==target?1:2;forgets=0;Reset(target);Require(forgets==1 && std::memcmp(data,expected,128)==0 && samples.size()==remaining && samples.find(target)==samples.end() && samples.at(otherKey).step==123456);if(alias!=target)Require(samples.at(alias).step==789012);
  std::memcpy(before,data,128);Reset(target);Require(forgets==2 && std::memcmp(data,expected,128)==0 && samples.size()==remaining);for(unsigned i=0;i<128;++i){hash^=data[i];hash*=1099511628211ull;}
 }samples.clear();return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,128);Require(Run(high)==expected);for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);std::puts("widget alto; reset/clave de muestra nativos y alias bajo intacto");}std::printf("Widget reset: 4096 patrones/dos resets %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    body=function(read_source('src/UI/UI_LegacyWidgetSystem.cpp',revision),'Widget_NodeInit')
    presentation=read_source('src/Entity/ClothPresentation.cpp',revision)
    types='\n'.join(re.search(r'struct '+name+r'\s*\{.*?\};',presentation,re.S).group(0) for name in ('Point','Sample'))
    forget=function(presentation,'ClothPresentation_Forget').replace('ClothPresentation_Forget','ForgetActual')
    args='widget' if 'void* widget' in body else '(int)widget'
    wrapper='\nvoid Reset(void* widget){Widget_NodeInit('+args+');}\n'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('TYPES',types).replace('FORGET',forget)+body+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-widget-reset-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
