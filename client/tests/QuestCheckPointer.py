#!/usr/bin/env python3
"""Coordinador de quest con GetQuestState/FindQuestContext reales y base nativa.

Condiciones de acción/pedido controladas; no certifica sus cuerpos, recursión
de fallback, registros fuera de 0..199, UI/socket ni layout completo CSQuest.
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
using uint=unsigned int;
BYTE* activeQuest;
short* activeRecord;
bool actionAllowed, requestAllowed;
int event;
uint CSQuest_CheckActCondition(void* self,short* record) {
    Require(self==activeQuest && record==activeRecord && event==0); event=1;
    return 0xa5a50000u|static_cast<unsigned>(actionAllowed);
}
uint CSQuest_CheckRequestCondition(void* self,short* record,char flag) {
    Require(self==activeQuest && record==activeRecord && flag==0 && event==0); event=2;
    return 0x5a5a0000u|static_cast<unsigned>(requestAllowed);
}
void __fastcall CSQuest_CheckQuestState(void*,char);
'''

MAIN = r'''
std::uint64_t RunCheck(BYTE* memory) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; activeQuest=quest;
    std::uint64_t digest=14695981039346656037ull;
    const char states[]={-1,0,1,2,3,4,5,-128,127};
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,0x5a,size); quest[4]=static_cast<BYTE>(trial%4);
        const unsigned index=(trial*73)%200; quest[0x1c87a]=static_cast<BYTE>(index);
        BYTE* record=quest+8+index*584; activeRecord=reinterpret_cast<short*>(record);
        std::memset(record,0,584); const short count=4; std::memcpy(record,&count,2);
        const unsigned row=(trial/4)%4;
        for(int r=0;r<4;++r) for(int column=0;column<4;++column) {
            const short text=static_cast<short>(trial*40503u+r*73+column);
            std::memcpy(record+48+r*18+column*2,&text,2);
        }
        record[44+row*18+quest[4]]=0x80;
        quest[0x1c848]=static_cast<BYTE>(trial*73); quest[0x1c882]=static_cast<BYTE>(trial);
        actionAllowed=(trial&2)!=0; requestAllowed=(trial&4)!=0; event=0;
        const char state=states[trial%9];
        BYTE expected[size]; std::memcpy(expected,memory,size);
        const BYTE effective=state==-1?static_cast<BYTE>((quest[0x1c848]>>((index*2)%32))&3):static_cast<BYTE>(state);
        expected[16+0x1c882]=effective;
        const int expectedEvent=effective==1?1:effective==3?2:0;
        int column=-1;
        if(effective==1 && actionAllowed) column=2;
        if(effective==2) column=3;
        if(effective==3 && requestAllowed) column=0;
        if(column>=0) std::memcpy(expected+16+0x1c880,record+48+row*18+column*2,2);
        CSQuest_CheckQuestState(quest,state);
        Require(event==expectedEvent && std::memcmp(expected,memory,size)==0);
        for(int i=0;i<72;++i) { digest^=quest[0x1c848+i]; digest*=1099511628211ull; }
        digest^=static_cast<unsigned>(event); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunCheck(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x1c890+32); Require(RunCheck(high)==expected);
        for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
        std::puts("coordinador/registro interior sobre 4 GiB, alias bajo intacto");
    }
    std::printf("CheckQuestState: 32768 estados, helpers reales y bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000") + SERVICES
    source = read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    for name in ("CSQuest_GetQuestState", "CSQuest_FindQuestContext", "CSQuest_CheckQuestState"):
        prefix += function(source, name)
    return prefix + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-check-") as temp:
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
