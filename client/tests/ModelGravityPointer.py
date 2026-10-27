#!/usr/bin/env python3
"""Entity_SetGravity real: tabla de rangos, bits exactos y base nativa."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
using undefined4=std::uint32_t;
FUNCTION
void Set(int type,BYTE* model){Entity_SetGravity(type,ARG);}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<32768;++trial){
        const int type=trial==0?(-2147483647-1):trial==1?2147483647:static_cast<int>(trial%8192)-4096;
        BYTE before[224];for(unsigned i=0;i<224;++i)memory[i]=before[i]=static_cast<BYTE>((trial*37+i*19)^(trial>>(i%15)));
        const std::uint32_t expected=type>=624 && type<=655?0xc3200000u:type>=656 && type<=687?0xc2c80000u:type>=688 && type<=719?0xc2480000u:type>=720 && type<=751?0xc28c0000u:0;
        std::memcpy(before+16+132,&expected,4);Set(type,memory+16);Require(std::memcmp(before,memory,224)==0);
        for(BYTE value:before){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        std::memset(low,0x7b,224);Require(Run(high)==expected);for(unsigned i=0;i<224;++i)Require(low[i]==0x7b);
        std::puts("modelo sobre 4 GiB: bits y canarios iguales; alias bajo intacto");
    }std::printf("Model gravity: 32768 casos, rangos/bits/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    code=function(read_source('src/Render/Entity_Render.cpp',revision),'Entity_SetGravity')
    arg='model' if 'void* model' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(model))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+MAIN.replace('FUNCTION',code).replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-model-gravity-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
