#!/usr/bin/env python3
"""CutText real: destinos nativos, bytes x86 y ocho combinaciones de direcciones.

Los servicios de corte y longitud multibyte son controlados; no certifica UAX14.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <string_view>
using byte=unsigned char;
unsigned profile,step,calls;
namespace TextEngine {
std::vector<unsigned char> LineBreaks(std::string_view text) {
    ++calls;
    std::vector<unsigned char> out(text.size()+1);
    for(unsigned i=0;i<text.size();++i) out[i]=(profile%4==0?0:profile%4==1?1:(i+profile)%7==0?2:0);
    return out;
}
}
int mbclen(const byte*) { return step; }
'''
MAIN = r'''
void Split(char* source,char* second,char* first,int size) { CutText(source,ARG,first,size); }
std::uint64_t Run(BYTE* input,BYTE* output1,BYTE* output2) {
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned size=0;size<256;++size) for(profile=0;profile<64;++profile) {
        step=profile%4+1; calls=0;
        std::memset(input,0xc3,320); std::memset(output1,0xa5,320); std::memset(output2,0x5a,320);
        char* source=reinterpret_cast<char*>(input+16);
        for(unsigned i=0;i<size;++i) source[i]=(i+profile)%11==0?' ':static_cast<char>('a'+(i*7+profile)%26);
        source[size]=0;
        BYTE before[320],first[320],second[320];
        std::memcpy(before,input,320); std::memcpy(first,output1,320); std::memcpy(second,output2,320);
        unsigned split=0;
        for(unsigned offset=0;offset<size;offset+=step) {
            const bool allowed=profile%4==1 || (profile%4>=2 && (offset+profile)%7==0);
            const int position=static_cast<int>(offset),half=static_cast<int>(size)/2;
            if((position>=half-2 && (source[offset]==' ' || allowed)) || position>=half+2) { split=offset; break; }
        }
        std::memcpy(first+16,source,split); first[16+split]=0;
        std::memcpy(second+16,source+split,size-split); second[16+size-split]=0;
        Split(source,reinterpret_cast<char*>(output2+16),reinterpret_cast<char*>(output1+16),size);
        Require(calls==1 && std::memcmp(input,before,320)==0 && std::memcmp(output1,first,320)==0 && std::memcmp(output2,second,320)==0);
        for(unsigned i=0;i<320;++i) { digest^=output1[i];digest*=1099511628211ull;digest^=output2[i];digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u)); Require(low);
    const auto expected=Run(low,low+1024,low+2048);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull))); Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<8;++mask) {
            std::memset(low,0x7b,3072);
            Require(Run(mask&1?high:low,(mask&2?high:low)+1024,(mask&4?high:low)+2048)==expected);
            for(unsigned slot=0;slot<3;++slot) if(mask&(1u<<slot)) for(unsigned i=0;i<320;++i) Require(low[slot*1024+i]==0x7b);
        }
        std::puts("ocho combinaciones origen/destinos altos y bajos; aliases intactos");
    }
    std::printf("CutText: 16384 casos, bytes/canarios %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    code=function(read_source('src/UI/UI_LegacyExterns.cpp',revision),'CutText')
    arg='second' if 'char* param_2' in code else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(second))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+code+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator'); parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):
        parser.error('La referencia int sólo se compara en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-cut-text-') as temp:
        directory=Path(temp); current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':
    main()
