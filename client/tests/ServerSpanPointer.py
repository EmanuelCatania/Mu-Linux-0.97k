#!/usr/bin/env python3
"""Clasificación real de25 grupos de servidor; no ejecuta los otros pases de UI."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

MAIN=r'''
#include <initializer_list>
char* serverData;
#define DAT_083a45d8 (*reinterpret_cast<char(*)[0x3600]>(serverData))
ALIAS
std::uint64_t Classify(){
    int iVar8,iVar10;char *pcVar7,*pcVar13,cVar1;unsigned uVar6;
    PASS
    return static_cast<unsigned>(iVar8)|(static_cast<std::uint64_t>(static_cast<unsigned>(iVar10))<<32);
}
std::uint64_t Run(BYTE* memory){
    serverData=reinterpret_cast<char*>(memory+16);std::uint64_t digest=14695981039346656037ull;
    for(unsigned frame=0;frame<512;++frame){
        BYTE expected[0x3600+32];std::memset(memory,0xa5,sizeof(expected));unsigned pvp=0,normal=0;
        for(unsigned slot=0;slot<25;++slot){char* row=serverData+slot*542;
            const unsigned length=(frame+slot*7)%19+1;for(unsigned i=0;i<length;++i)row[i]='A'+i%26;
            const bool isPvp=(frame+slot)%3==0;row[length-1]=isPvp?'2':'1';row[length]=0;
            const BYTE channels=(frame/3+slot)%4==0?0:(frame/3+slot)%4==1?1:(frame/3+slot)%4==2?20:255;row[20]=static_cast<char>(channels);
            if(channels){if(isPvp)++pvp;else ++normal;}
        }
        std::memcpy(expected,memory,sizeof(expected));
        for(unsigned slot=0;slot<25;++slot)if(serverData[slot*542+20])expected[16+slot*542+21]=(frame+slot)%3==0?1:0;
        Require(Classify()==(pvp|(static_cast<std::uint64_t>(normal)<<32)) && std::memcmp(memory,expected,sizeof(expected))==0);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* crossing=static_cast<BYTE*>(Reserve(std::uint64_t(0x17fff0000ull)));Require(high && crossing && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(BYTE* memory:{high,crossing+0xfbf0}){std::memset(low,0x7b,0x3600+32);Require(Run(memory)==expected);for(unsigned i=0;i<0x3600+32;++i)Require(low[i]==0x7b);}std::puts("tabla alta y cruce bit31:25 grupos/bytes iguales; alias bajo intacto");
    }std::printf("Server span: 512 tablas, clasificacion/canarios %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Scene/Scene_Login_ServerSelect.cpp',revision)
    start=source.index('    // ── PASS 2:');end=source.index('    // ── PASS 3:',start)
    alias=re.search(r'^#define DAT_083a45ec[^\n]+',read_source('src/globals.h',revision),re.M).group()
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x20000')+MAIN.replace('ALIAS',alias).replace('PASS',source[start:end])


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-server-span-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
