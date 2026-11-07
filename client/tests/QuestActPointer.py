#!/usr/bin/env python3
"""Condición de acción real: bases nativas, índice escalar y retorno uint legado.

FindQuestContext real; inventario controlado. No certifica escaneo real del
inventario, recursión, registros/clases inválidos ni layout completo CSQuest.
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
using ULONG_PTR=std::uintptr_t;
#endif
using uint=unsigned int;
using std::uintptr_t;
unsigned trialSeed;
int events[40], eventCount;
int InventoryResult(int type,int count) { return ((static_cast<unsigned>(type)+count+trialSeed)%5)==0?2:0; }
int CSQuest_FindQuestItemsInInven(int type,int count,uint level) {
    Require(level==0xffffffffu && eventCount<40); events[eventCount++]=type; events[eventCount++]=count;
    return InventoryResult(type,count);
}
void CSQuest_CheckQuestState(void*,char) { Require(false); }
'''

MAIN = r'''
std::uint64_t RunAct(BYTE* memory,BYTE* recordMemory,bool inside) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; BYTE* record=inside?quest+8:recordMemory+16;
    std::uint64_t digest=14695981039346656037ull;
    const BYTE flags[]={0,1,2,0x80}, kinds[]={0,1,2,255};
    for(unsigned trial=0;trial<16384;++trial) {
        trialSeed=trial; std::memset(memory,0x5a,size); std::memset(recordMemory,0xa5,616); std::memset(record,0,584);
        quest[4]=static_cast<BYTE>(trial%4);
        const short count=trial%7==0?static_cast<short>(-static_cast<int>(trial%32768)):static_cast<short>(trial%21);
        std::memcpy(record,&count,2);
        for(int row=0;row<20;++row) {
            BYTE* action=record+40+row*18;
            action[-1]=kinds[(trial+row)%4]; action[0]=static_cast<BYTE>(trial+row*73);
            action[1]=static_cast<BYTE>(trial*73+row); action[2]=static_cast<BYTE>(trial*40503u+row);
            action[4+quest[4]]=flags[(trial/4+row)%4];
            for(int column=0;column<4;++column) {
                const short text=static_cast<short>(trial*65521u+row*73+column); std::memcpy(action+8+column*2,&text,2);
            }
        }
        uint status=0; bool fail=false; int expectedEvents[40], expectedCount=0;
        for(int row=0;row<count;++row) {
            const BYTE* action=record+40+row*18; status=row*18+quest[4];
            if(action[4+quest[4]]==1) {
                status=static_cast<uint>(static_cast<int>(action[-1])-1);
                if(status==0) {
                    const int type=action[0]*32+action[1], n=action[2];
                    expectedEvents[expectedCount++]=type; expectedEvents[expectedCount++]=n;
                    if(InventoryResult(type,n)!=0) { fail=true; break; }
                    status=0;
                }
            }
        }
        BYTE expected[size], expectedRecord[616]; std::memcpy(expected,memory,size); std::memcpy(expectedRecord,recordMemory,616);
        uint expectedResult=(status&0xffffff00u)|1u;
        if(fail) {
            int match=0; while(record[44+match*18+quest[4]]==0) ++match; Require(match<count);
            std::memcpy(expected+16+0x1c880,record+48+match*18+2,2);
            expectedResult=static_cast<uint>(reinterpret_cast<std::uintptr_t>(quest))&0xffffff00u;
        }
        eventCount=0; std::memset(events,0,sizeof(events));
        const uint result=CSQuest_CheckActCondition(quest,reinterpret_cast<short*>(record));
        Require(result==expectedResult && eventCount==expectedCount && std::memcmp(events,expectedEvents,eventCount*sizeof(int))==0);
        Require(std::memcmp(expected,memory,size)==0 && std::memcmp(expectedRecord,recordMemory,616)==0);
        digest^=result; digest*=1099511628211ull;
        for(int i=0;i<eventCount;++i) { digest^=static_cast<unsigned>(events[i]); digest*=1099511628211ull; }
        for(int i=0;i<2;++i) { digest^=quest[0x1c880+i]; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* lowRecord=static_cast<BYTE*>(Reserve(0x75000000u)); Require(low && lowRecord);
    const auto expected=RunAct(low,lowRecord,false), inside=RunAct(low,lowRecord,true); Require(expected==inside);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highRecord=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(high && highRecord);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,0x1c890+32); if(mask&2) std::memset(lowRecord,0,616);
            Require(RunAct(mask&1?high:low,mask&2?highRecord:lowRecord,false)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<616;++i) Require(lowRecord[i]==0);
        }
        Require(RunAct(high,highRecord,true)==inside);
        std::puts("cuatro combinaciones gestor/registro altas y bajas, registro interior y retorno uint completo");
    }
    std::printf("CheckActCondition: 16384 estados por modo, bytes/retorno/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000") + SERVICES
    source = read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    for name in ("CSQuest_FindQuestContext", "CSQuest_CheckActCondition"):
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
    with tempfile.TemporaryDirectory(prefix="mu-quest-act-") as temp:
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
