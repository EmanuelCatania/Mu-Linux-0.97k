#!/usr/bin/env python3
"""Ventanas libres de inventario: cuerpo real, oráculo rectangular y bytes intactos."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
using uint=unsigned;
#ifndef _WIN32
using WORD=std::uint16_t;
#endif
FUNCTION
unsigned Check(BYTE* grid,int width,int height,int windowWidth,int windowHeight){return SecondPassword_GridSlotAvail(ARG,width,height,windowWidth,windowHeight);}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;unsigned cases=0;
    for(int width=1;width<=8;++width)for(int height=1;height<=8;++height)for(int ww=0;ww<=9;++ww)for(int wh=0;wh<=9;++wh)for(unsigned profile=0;profile<8;++profile){
        std::memset(memory,0xa5,64*68+32);BYTE* grid=memory+16;
        for(int row=0;row<height;++row)for(int col=0;col<width;++col){
            const bool empty=profile==0 || (profile==2 && (row+col)%3!=0) || (profile==3 && row%2==0) || (profile==4 && col%2==0) || (profile==5 && row<3 && col<3) || (profile==6 && row>=height/2 && col>=width/2) || (profile==7 && (row*37+col*19+width*3+height)%7<5);
            const WORD key=empty?0xffffu:profile%2?0u:static_cast<WORD>(0x8000u+row*width+col);std::memcpy(grid+(row*width+col)*68,&key,2);
        }
        bool expected=false;
        for(int top=0;top+wh<=height;++top)for(int left=0;left+ww<=width;++left){
            bool free=true;for(int row=top;row<top+wh;++row)for(int col=left;col<left+ww;++col){WORD key;std::memcpy(&key,grid+(row*width+col)*68,2);if(key!=0xffffu)free=false;}
            if(free)expected=true;
        }
        BYTE before[64*68+32];std::memcpy(before,memory,sizeof(before));const unsigned result=Check(grid,width,height,ww,wh);
        Require(result==static_cast<unsigned>(expected) && std::memcmp(before,memory,sizeof(before))==0);digest^=result;digest*=1099511628211ull;++cases;
    }Require(cases==51200);return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,64*68+32);Require(Run(high)==expected);for(unsigned i=0;i<64*68+32;++i)Require(low[i]==0x7b);std::puts("grilla sobre 4 GiB: resultados/readonly iguales; alias bajo intacto");}
    std::printf("Grid available: 51200 casos, ventanas/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    code=function(read_source('src/Net/SecondPassword.cpp',revision),'SecondPassword_GridSlotAvail')
    arg='reinterpret_cast<uintptr_t>(grid)' if 'uintptr_t a1' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(grid))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+MAIN.replace('FUNCTION',code).replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-grid-avail-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
