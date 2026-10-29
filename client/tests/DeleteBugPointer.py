#!/usr/bin/env python3
"""DeleteBug real: diez slots con owner32 heredado; recorrido por cantidad."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
char* DAT_083a1218;
FUNCTION
std::uint64_t Run(BYTE* memory){
    DAT_083a1218=reinterpret_cast<char*>(memory+16);std::uint64_t digest=14695981039346656037ull;
    const DWORD owners[]={0,1,0x7fffffffu,0x80000000u,0xffffffffu,0x12345678u};
    for(unsigned trial=0;trial<32768;++trial){
        BYTE expected[10*444+32];for(unsigned i=0;i<sizeof(expected);++i)memory[i]=expected[i]=static_cast<BYTE>((trial*37+i*19)^(trial>>(i%15)));
        const DWORD wanted=owners[trial%6];
        for(unsigned slot=0;slot<10;++slot){const DWORD owner=owners[(trial/6+slot)%6];const BYTE active=(trial/36+slot)%4==0?0:(trial/36+slot)%4==1?1:(trial/36+slot)%4==2?128:255;
            BYTE* object=memory+16+slot*444;object[0]=expected[16+slot*444]=active;
            std::memcpy(object+252,&owner,4);std::memcpy(expected+16+slot*444+252,&owner,4);
            if(active && owner==wanted)expected[16+slot*444]=0;
        }
        DeleteBug(wanted);Require(std::memcmp(memory,expected,sizeof(expected))==0);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* crossing=static_cast<BYTE*>(Reserve(std::uint64_t(0x17fff0000ull)));Require(high && crossing && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(BYTE* memory:{high,crossing+0xfdf0}){std::memset(low,0x7b,10*444+32);Require(Run(memory)==expected);for(unsigned i=0;i<10*444+32;++i)Require(low[i]==0x7b);}
        std::puts("pool alto y cruce del bit31: diez slots/bytes iguales; alias bajo intacto");
    }std::printf("DeleteBug: 32768 casos, owner32/activo/canarios %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x20000')+'\n#include <initializer_list>\n'
    return prefix+MAIN.replace('FUNCTION',function(read_source('src/Scene/Scene_EntityLifecycle.cpp',revision),'DeleteBug'))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-delete-bug-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
