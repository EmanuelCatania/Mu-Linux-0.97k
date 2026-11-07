#!/usr/bin/env python3
"""Cierre real de quest con dirección nativa, bytes y paquete C1 intactos.

Red/cierre de ventanas controlados; no prueba socket, UI o layout CSQuest
nativo. g_csQuest y los demás métodos de la clase se validan por separado.
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
int event=0, sends=0;
void CloseInventoryRelatedWindows() { Require(event==0 && activeQuest[0x1c87f]==0); event=1; }
struct Network {
    void SendC1(const BYTE* packet,int size) {
        const BYTE expected[]={0xc1,3,0x31};
        Require(event==1 && size==3 && std::memcmp(packet,expected,3)==0 && activeQuest[0x1c87f]==0);
        event=2; ++sends;
    }
} gNetwork;
'''

MAIN = r'''
void Call(void* quest) { CSQuest_clearQuest(ARG); }
std::uint64_t Run(BYTE* memory) {
    constexpr std::size_t size=0x1c884+32;
    std::uint64_t digest=14695981039346656037ull;
    activeQuest=memory+16; event=0; sends=0;
    Call(nullptr); Require(event==0 && sends==0);
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(memory,static_cast<BYTE>(trial/256*17),size);
        activeQuest[0x1c87f]=static_cast<BYTE>(trial); event=0;
        BYTE expected[size]; std::memcpy(expected,memory,size); expected[16+0x1c87f]=0;
        Call(activeQuest);
        Require(event==2 && sends==static_cast<int>(trial+1) && std::memcmp(expected,memory,size)==0);
        for(std::size_t i=0;i<size;++i) { digest^=memory[i]; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x1c884+32); Require(Run(high)==expected);
        for(std::size_t i=0;i<0x1c884+32;++i) Require(low[i]==0);
        std::puts("quest sobre 4 GiB y alias bajo intacto");
    }
    std::printf("CSQuest_clearQuest: 4096 estados, orden y C1 03 31; bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    body = function(read_source("src/Item/Quest_UIState.cpp", revision), "CSQuest_clearQuest")
    arg = "quest" if "void* param_1" in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(quest))"
    return prefix + SERVICES + body + MAIN.replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-clear-") as temp:
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
