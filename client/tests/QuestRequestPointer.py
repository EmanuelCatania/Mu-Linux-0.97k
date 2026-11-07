#!/usr/bin/env python3
"""Condiciones de pedido reales: nivel/Zen, cuatro bases nativas y bytes x86.

Registros válidos con clases 0..3, hasta 10 contextos/12 requisitos. No
certifica entradas malformadas, UI/red ni layout nativo completo de CSQuest.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
template<class T> T Read(const BYTE* p) { T value; std::memcpy(&value,p,sizeof(value)); return value; }
template<class T> void Put(BYTE* p,T value) { std::memcpy(p,&value,sizeof(value)); }
std::uint64_t RunRequest(BYTE* memory,BYTE* recordMemory,BYTE* attributes,BYTE* machine,bool inside) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; BYTE* record=inside?quest+8:recordMemory+16;
    DAT_07cf1ff4=attributes; DAT_07cf1ffc=machine;
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    const short levels[]={-32768,-1,0,1,99,100,32767};
    const ushort actualLevels[]={0,1,99,100,32767,32768,65535};
    const int amounts[]={0,1,100,-1,2147483647,-2147483647-1};
    const BYTE tags[]={0,1,2,127,128,255};
    for(unsigned trial=0;trial<8192;++trial) {
        std::uint32_t random=trial^0x97040123u;
        auto next=[&]() { random=random*1664525u+1013904223u; return random>>16; };
        std::memset(memory,0x5a,size); std::memset(recordMemory,0xa5,616); std::memset(attributes,0x33,64); std::memset(machine,0xcc,1412); std::memset(record,0,584);
        quest[4]=static_cast<BYTE>(trial%4); quest[5]=trial%7==0?255:static_cast<BYTE>(next()%4);
        if(quest[5]==255) coverage|=64;
        const short contexts=trial%13==0?-1:static_cast<short>(next()%11), requirements=trial%17==0?-1:static_cast<short>(next()%13);
        Put(record,contexts); Put(record+2,requirements);
        for(int context=0;context<10;++context) {
            record[43+context*18]=tags[next()%6];
            record[44+context*18+quest[4]]=static_cast<BYTE>(next()%3==0?quest[5]+1:next()%5);
        }
        for(int requirement=0;requirement<12;++requirement) {
            BYTE* row=record+332+requirement*16;
            row[-3]=tags[next()%6]; Put(row,levels[next()%7]); Put(row+2,levels[next()%7]);
            Put(row+4,amounts[next()%6]); Put(row+8,static_cast<short>(next()));
        }
        Put(attributes+14,actualLevels[next()%7]); Put(machine+0x548,amounts[next()%6]);
        const char last=trial%3==0?0:trial%3==1?1:-1;
        BYTE expected[size], expectedRecord[616], beforeAttributes[64], beforeMachine[1412];
        std::memcpy(expected,memory,size); std::memcpy(expectedRecord,recordMemory,616); std::memcpy(beforeAttributes,attributes,64); std::memcpy(beforeMachine,machine,1412);
        bool fail=false; int finalCount=0;
        for(int context=0;context<contexts && !fail;++context) {
            if(record[44+context*18+quest[4]]!=static_cast<unsigned>(quest[5])+1) continue;
            finalCount=0;
            for(int requirement=0;requirement<requirements;++requirement) {
                const BYTE* row=record+332+requirement*16;
                const auto tag=static_cast<std::int8_t>(row[-3]), selected=static_cast<std::int8_t>(record[43+context*18]);
                if(tag==selected || tag==-1) {
                    if(tag==-1) coverage|=128;
                    const int level=Read<ushort>(attributes+14), minimum=Read<short>(row), maximum=Read<short>(row+2);
                    const int money=Read<int>(row+4);
                    unsigned reason=0;
                    if(minimum!=0 && level<minimum) reason=1;
                    else if(maximum!=0 && maximum<level) reason=2;
                    else if(money!=0) {
                        Put(expected+16+0x1c884,money);
                        if(!last) coverage|=16;
                        else if(Read<int>(machine+0x548)<money) reason=4;
                        else coverage|=32;
                    }
                    if(reason) {
                        coverage|=reason; Put(expected+16+0x1c880,Read<short>(row+8)); expected[16+0x1c882]=5; fail=true; break;
                    }
                }
                ++finalCount;
            }
        }
        if(!fail) coverage|=8;
        const uint expectedResult=fail?(static_cast<uint>(reinterpret_cast<std::uintptr_t>(quest))&0xffffff00u):(static_cast<uint>(finalCount)&0xffffff00u)|1u;
        const uint result=CSQuest_CheckRequestCondition(quest,reinterpret_cast<short*>(record),last);
        Require(result==expectedResult && std::memcmp(expected,memory,size)==0 && std::memcmp(expectedRecord,recordMemory,616)==0 && std::memcmp(beforeAttributes,attributes,64)==0 && std::memcmp(beforeMachine,machine,1412)==0);
        digest^=result; digest*=1099511628211ull;
        for(int i=0;i<14;++i) { digest^=quest[0x1c87a+i]; digest*=1099511628211ull; }
    }
    Require(coverage==255); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* record=static_cast<BYTE*>(Reserve(0x75000000u));
    auto* attributes=static_cast<BYTE*>(Reserve(0x74000000u)); auto* machine=static_cast<BYTE*>(Reserve(0x73000000u)); Require(low && record && attributes && machine);
    const auto expected=RunRequest(low,record,attributes,machine,false); Require(RunRequest(low,record,attributes,machine,true)==expected);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highRecord=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highAttributes=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); auto* highMachine=static_cast<BYTE*>(Reserve(std::uint64_t(0x173000000ull))); Require(high && highRecord && highAttributes && highMachine);
        for(int mask=1;mask<16;++mask) {
            if(mask&1) std::memset(low,0,0x1c890+32); if(mask&2) std::memset(record,0,616);
            if(mask&4) std::memset(attributes,0,64); if(mask&8) std::memset(machine,0,1412);
            Require(RunRequest(mask&1?high:low,mask&2?highRecord:record,mask&4?highAttributes:attributes,mask&8?highMachine:machine,false)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<616;++i) Require(record[i]==0);
            if(mask&4) for(int i=0;i<64;++i) Require(attributes[i]==0);
            if(mask&8) for(int i=0;i<1412;++i) Require(machine[i]==0);
        }
        Require(RunRequest(high,highRecord,highAttributes,highMachine,true)==expected);
        std::puts("16 combinaciones gestor/registro/atributos/maquina altas y bajas y registro interior alto");
    }
    std::printf("CheckRequestCondition: 8192 estados por modo, nivel/Zen/retorno/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    prefix += "\n#ifndef _WIN32\n#define __fastcall\nusing ULONG_PTR=std::uintptr_t;\n#endif\nusing uint=unsigned int; using ushort=unsigned short; using std::uintptr_t;\n"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("DAT_07cf1ff4", "DAT_07cf1ffc"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    return prefix + function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "CSQuest_CheckRequestCondition") + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-request-") as temp:
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
