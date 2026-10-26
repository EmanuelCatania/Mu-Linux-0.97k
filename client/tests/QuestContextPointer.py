#!/usr/bin/env python3
"""Contexto real: gestor/registro nativos, selección y fallback contra x86.

CheckQuestState controlado; no prueba recursión real, clase fuera de 0..3,
registros inválidos ni el layout nativo completo de CSQuest.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#ifndef _WIN32
#define __fastcall
#endif
BYTE* activeQuest;
BYTE expectedIndex;
short fallback;
int calls;
void CSQuest_CheckQuestState(void* self,char state) {
    Require(self==activeQuest && state==-1 && calls++==0 && activeQuest[0x1c87a]==expectedIndex);
    std::memcpy(activeQuest+0x1c880,&fallback,2);
}
'''

MAIN = r'''
std::uint64_t Run(BYTE* memory,BYTE* recordMemory,bool inside) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; BYTE* record=inside?quest+8:recordMemory+16; activeQuest=quest;
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<16384;++trial) {
        std::memset(memory,0x5a,size); std::memset(recordMemory,0xa5,616); std::memset(record,0,584);
        quest[4]=static_cast<BYTE>(trial%4); quest[0x1c87a]=static_cast<BYTE>(trial*73);
        const short count=trial%7==0?static_cast<short>(-static_cast<int>(trial%32768)):static_cast<short>(trial%21);
        std::memcpy(record,&count,2);
        for(int row=0;row<20;++row) {
            for(int column=0;column<4;++column) {
                const short text=static_cast<short>(trial*40503u+row*73+column);
                std::memcpy(record+48+row*18+column*2,&text,2);
            }
            record[44+row*18+quest[4]]=trial%3==0?0:static_cast<BYTE>((trial+row)%7==0?0x80:0);
        }
        const int column=(trial/4)%4;
        int match=-1;
        for(int row=0;row<count;++row) if(record[44+row*18+quest[4]]!=0) { match=row; break; }
        BYTE expected[size], expectedRecord[616]; std::memcpy(expected,memory,size); std::memcpy(expectedRecord,recordMemory,616);
        fallback=static_cast<short>(trial*65521u); calls=0;
        short resultExpected;
        if(match>=0) std::memcpy(&resultExpected,record+48+match*18+column*2,2);
        else {
            expectedIndex=static_cast<BYTE>(quest[0x1c87a]-1); expected[16+0x1c87a]=expectedIndex;
            std::memcpy(expected+16+0x1c880,&fallback,2); resultExpected=fallback;
        }
        const short result=CSQuest_FindQuestContext(quest,reinterpret_cast<short*>(record),column);
        Require(result==resultExpected && calls==(match<0?1:0) && std::memcmp(expected,memory,size)==0 && std::memcmp(expectedRecord,recordMemory,616)==0);
        digest^=static_cast<unsigned short>(result); digest*=1099511628211ull; digest^=static_cast<unsigned>(calls); digest*=1099511628211ull;
        digest^=quest[0x1c87a]; digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* lowRecord=static_cast<BYTE*>(Reserve(0x75000000u)); Require(low && lowRecord);
    const auto expected=Run(low,lowRecord,false), inside=Run(low,lowRecord,true); Require(expected==inside);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highRecord=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(high && highRecord);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,0x1c890+32); if(mask&2) std::memset(lowRecord,0,616);
            Require(Run(mask&1?high:low,mask&2?highRecord:lowRecord,false)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<616;++i) Require(lowRecord[i]==0);
        }
        Require(Run(high,highRecord,true)==inside);
        std::puts("cuatro combinaciones gestor/registro altas y bajas y registro interior alto");
    }
    std::printf("FindQuestContext: 16384 estados por modo, seleccion/fallback/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    return prefix + SERVICES + function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "CSQuest_FindQuestContext") + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-context-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
