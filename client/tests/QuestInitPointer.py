#!/usr/bin/env python3
"""Constructor de quest real: base nativa y campos/guardas idénticos a x86.

El slot de vtable es opaco de 32 bits: verifica su representación legacy,
no certifica despacho virtual ni layout nativo completo de CSQuest.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint64_t RunInit(BYTE* memory,BYTE* tableMemory) {
    constexpr std::size_t size=0x40000;
    BYTE* quest=memory+16; BYTE* other=memory+0x20000; table=tableMemory;
    const std::uint32_t legacyTable=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table));
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(memory,static_cast<BYTE>(trial*73),size); std::memset(table,0xa5,32);
        for(int offset=0;offset<10;++offset) quest[0x1c87a+offset]=static_cast<BYTE>(trial+offset);
        g_csQuest=trial%2?reinterpret_cast<std::uintptr_t>(other):0;
        BYTE expected[size]; std::memcpy(expected,memory,size);
        std::memcpy(expected+16,&legacyTable,4); expected[16+4]=255;
        for(int offset : {0x1c87a,0x1c87b,0x1c87d,0x1c87e,0x1c87f,0x1c880,0x1c881}) expected[16+offset]=0;
        Quest_FullInit(quest);
        Require(static_cast<std::uintptr_t>(g_csQuest)==reinterpret_cast<std::uintptr_t>(trial%2?other:quest));
        Require(std::memcmp(expected,memory,size)==0); for(int i=0;i<32;++i) Require(table[i]==0xa5);
        for(int i=0;i<4;++i) { digest^=quest[i]; digest*=1099511628211ull; }
        for(int i=0;i<10;++i) { digest^=quest[0x1c87a+i]; digest*=1099511628211ull; }
        digest^=quest[4]; digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* lowTable=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && lowTable);
    const auto expected=RunInit(low,lowTable);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,0x40000); if(mask&2) std::memset(lowTable,0,32);
            Require(RunInit(mask&1?high:low,mask&2?highTable:lowTable)==expected);
            if(mask&1) for(std::size_t i=0;i<0x40000;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<32;++i) Require(lowTable[i]==0);
        }
        std::puts("constructor/base global altos; slot de vtable legacy de 32 bits sin despacho");
    }
    std::printf("Quest_FullInit: 4096 estados, bytes y no reemplazar global; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    prefix += "\n#ifndef _WIN32\n#define __fastcall\n#endif\nusing std::uintptr_t;\nBYTE* table;\n#define PTR_LAB_005524b8 (*reinterpret_cast<int*>(table))\n"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    prefix += re.search(r"extern [^;\n]*\bg_csQuest\s*;", header).group()
    prefix += re.search(r"^[^/\n]*\bg_csQuest\s*=\s*[^;]+;", definitions, re.M).group()
    return prefix + function(read_source("src/Net/Crypto.cpp", revision), "Quest_FullInit") + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-init-") as temp:
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
