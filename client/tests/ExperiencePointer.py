#!/usr/bin/env python3
"""Experiencia real: todos los niveles WORD, aritmética32 y base nativa."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
#ifndef _WIN32
#define __fastcall
#endif
using uint=unsigned;
using ushort=std::uint16_t;
FUNCTION
void Calculate(BYTE* machine){CalculateNextExperince(ARG);}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned level=0;level<65536;++level){
        BYTE before[96];for(unsigned i=0;i<96;++i)memory[i]=before[i]=static_cast<BYTE>((level*37+i*19)^(level>>(i%15)));
        const ushort word=static_cast<ushort>(level);std::memcpy(memory+16+14,&word,2);std::memcpy(before+16+14,&word,2);
        const std::uint64_t base=10ull*level*level*(level+9ull);
        const std::uint64_t extra=level>255?1000ull*(level-255ull)*(level-255ull)*(level-246ull):0;
        const auto expected=static_cast<std::uint32_t>(base+extra);std::memcpy(before+16+52,&expected,4);
        Calculate(memory+16);Require(std::memcmp(memory,before,96)==0);
        for(BYTE value:before){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,96);Require(Run(high)==expected);for(unsigned i=0;i<96;++i)Require(low[i]==0x7b);std::puts("ficha sobre 4 GiB: experiencia/bytes iguales; alias bajo intacto");}
    std::printf("Experience: 65536 niveles, modulo32/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    code=function(read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision),'CalculateNextExperince')
    arg='machine' if 'void* machine' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(machine))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+MAIN.replace('FUNCTION',code).replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-experience-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
