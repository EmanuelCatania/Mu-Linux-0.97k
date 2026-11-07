#!/usr/bin/env python3
"""Destructor/reset reales con base nativa, flags y liberación en orden.

La liberación registra argumentos; slot de vtable legacy de cuatro bytes
sin despacho. No certifica heap real, vtable/layout nativo ni salida del juego.
"""
import argparse
import os
from pathlib import Path
import tempfile

from QuestInitPointer import harness as init_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
BYTE* activeQuest;
int frees;
void operator_delete(void* address) {
    Require(address==activeQuest && g_csQuest==0 && frees++==0);
    const auto expected=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table));
    Require(std::memcmp(activeQuest,&expected,4)==0);
}
'''

MAIN = r'''
void Destroy(void* quest,BYTE flags) { FUN_00403f10(ARG,0,flags); }
void Reset(void* quest) { FUN_00403ef0(RESETARG); }
std::uint64_t RunDestructor(BYTE* memory,BYTE* tableMemory) {
    constexpr std::size_t size=0x40000;
    BYTE* quest=memory+16; BYTE* other=memory+0x20000; table=tableMemory; activeQuest=quest;
    const auto legacyTable=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table));
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(memory,static_cast<BYTE>(trial*73),size); std::memset(table,0xa5,32);
        g_csQuest=trial%3==0?0:reinterpret_cast<std::uintptr_t>(trial%3==1?quest:other);
        BYTE expected[size]; std::memcpy(expected,memory,size); std::memcpy(expected+16,&legacyTable,4);
        frees=0; Destroy(quest,static_cast<BYTE>(trial));
        Require(g_csQuest==0 && frees==static_cast<int>(trial&1) && std::memcmp(expected,memory,size)==0);
        for(int i=0;i<32;++i) Require(table[i]==0xa5);
        digest^=static_cast<unsigned>(frees); digest*=1099511628211ull;
        std::memset(memory,0xa5,size); std::memcpy(expected,memory,size); std::memcpy(expected+16,&legacyTable,4);
        g_csQuest=reinterpret_cast<std::uintptr_t>(other); frees=0; Reset(quest);
        Require(g_csQuest==0 && frees==0 && std::memcmp(expected,memory,size)==0);
        for(int i=0;i<4;++i) { digest^=quest[i]; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* lowTable=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && lowTable);
    const auto expected=RunDestructor(low,lowTable);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,0x40000); if(mask&2) std::memset(lowTable,0,32);
            Require(RunDestructor(mask&1?high:low,mask&2?highTable:lowTable)==expected);
            if(mask&1) for(std::size_t i=0;i<0x40000;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<32;++i) Require(lowTable[i]==0);
        }
        std::puts("destructor/liberacion de direccion completa; slot legacy de 32 bits sin despacho");
    }
    std::printf("Quest destructor: 4096 estados, reset/flags/orden/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = init_harness(revision).split("std::uint64_t RunInit(", 1)[0]
    prefix = prefix.replace(function(prefix, "operator_delete"), "void operator_delete(void*);")
    source = read_source("src/Net/Crypto.cpp", revision)
    reset, destructor = function(source, "FUN_00403ef0"), function(source, "FUN_00403f10")
    arg = "quest" if "void* ecx" in destructor else "static_cast<int>(reinterpret_cast<std::uintptr_t>(quest))"
    resetarg = "quest" if "void* param_1" in reset else "static_cast<int*>(quest)"
    return prefix + SERVICES + reset + destructor + MAIN.replace("RESETARG", resetarg).replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-destructor-") as temp:
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
