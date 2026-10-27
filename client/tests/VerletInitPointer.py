#!/usr/bin/env python3
"""Inicializadores reales de Verlet: bytes y retorno del port, sin vtables nativas."""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void Zero(BYTE* p) { VerletNode_ZeroFields(ZERO_ARG); }
std::uint64_t Run(BYTE* memory) {
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<32768;++trial) {
        BYTE expected[96];
        for(unsigned i=0;i<96;++i) memory[i]=expected[i]=static_cast<BYTE>((trial*37u+i*19u)^(trial>>(i%15)));
        BYTE* node=memory+16;
        const unsigned path=trial%5;
        std::memset(expected+20,0,path==2 || path==4?32:28);
        if(path==0) Zero(node);
        if(path==1) Require(VerletNode_CtorBase(node)==node);
        if(path==2) Require(static_cast<std::uintptr_t>(VerletNode_CtorExt(reinterpret_cast<DWORD*>(node)))==reinterpret_cast<std::uintptr_t>(node));
        if(path==3) { Zero(node); Require(VerletNode_CtorBase(node)==node); }
        if(path==4) { Require(VerletNode_CtorBase(node)==node); Require(static_cast<std::uintptr_t>(VerletNode_CtorExt(reinterpret_cast<DWORD*>(node)))==reinterpret_cast<std::uintptr_t>(node)); Zero(node); }
        Require(std::memcmp(memory,expected,96)==0);
        for(BYTE value:expected) { digest^=value; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u)); Require(low);
    auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull))); Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        std::memset(low,0xa5,96); Require(Run(high)==expected);
        for(unsigned i=0;i<96;++i) Require(low[i]==0xa5);
        std::puts("nodo sobre 4 GiB: bytes y retornos completos; alias bajo intacto");
    }
    std::printf("Verlet init: 32768 casos, cinco secuencias %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0]+"\n#ifndef _WIN32\n#define __fastcall\n#endif\n"
    source=read_source("src/Physics/Cloth_Simulation.cpp",revision)
    zero=function(source,"VerletNode_ZeroFields")
    code=zero+function(source,"VerletNode_CtorBase")+function(source,"VerletNode_CtorExt")
    arg="p" if "void*" in zero.split("{",1)[0] else "static_cast<int>(reinterpret_cast<std::uintptr_t>(p))"
    return prefix+code+MAIN.replace("ZERO_ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-verlet-init-") as temp:
        directory=Path(temp)
        current=build_and_run(directory/"current",harness(),args)
        print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
