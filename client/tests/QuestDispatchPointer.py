#!/usr/bin/env python3
"""Despacho quest real: todos los BYTE y flujo real de botones/respuestas.

Backend de ventana/red/audio/texto controlado. No certifica sesión real,
socket, dibujo ni layout completo del gestor en x64.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from QuestInputPointer import harness as input_harness, MAIN as INPUT_MAIN
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
using byte=BYTE; using undefined4=std::uint32_t;
BYTE* activeQuest; int calls,lastHandler,handlerResult;
int CSQuest_ProceedButton(void* ptr) { Require(ptr==activeQuest); ++calls; lastHandler=1; return handlerResult; }
undefined4 Packet_ParseType2(void* ptr) { Require(ptr==activeQuest); ++calls; lastHandler=2; return static_cast<undefined4>(handlerResult); }
'''

MAIN = r'''
std::uint64_t RunDispatch(BYTE* memory) {
    constexpr std::size_t size=0x1c890+32; activeQuest=memory+16;
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(memory,static_cast<int>(trial>>8),size); activeQuest[0x1c87f]=static_cast<BYTE>(trial);
        std::vector<BYTE> before(memory,memory+size);
        calls=0; lastHandler=0; handlerResult=static_cast<int>(trial*0x78563412u);
        Require(Packet_ParseReceived(activeQuest)==1);
        const unsigned state=trial&255;
        Require(calls==(state==1 || state==2?1:0) && lastHandler==(state==1?1:state==2?2:0));
        Require(std::memcmp(before.data(),memory,size)==0);
        digest^=static_cast<unsigned>(calls*3+lastHandler); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunDispatch(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x1c890+32); Require(RunDispatch(high)==expected);
        for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
        std::puts("gestor alto, callbacks con direccion completa y alias bajo intacto");
    }
    std::printf("Quest dispatch: 4096 estados, 256 flags/retornos/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def dispatch(revision=None):
    return function(read_source("src/Util/Misc.cpp",revision),"Packet_ParseReceived")


def harness(revision=None):
    return PREAMBLE.split("char DAT_00559bb4",1)[0].replace("0x10000","0x100000")+SERVICES+dispatch(revision)+MAIN


def integrated_harness(revision=None):
    prefix=input_harness(revision).removesuffix(INPUT_MAIN)+"\nusing byte=BYTE; using undefined4=std::uint32_t;\n"
    prefix+=function(read_source("src/Scene/Scene_CharSelect_Nav.cpp",revision),"Packet_ParseType2")
    prefix+=dispatch(revision)
    main=INPUT_MAIN.replace("quest[0x1c87f]=static_cast<BYTE>(next()%2)","quest[0x1c87f]=static_cast<BYTE>(next()%4)")
    invocation="if(proceed) Require(CSQuest_ProceedButton(quest)==1); else FUN_00401af0(quest);"
    if main.count(invocation)!=1: raise ValueError("Cambió el fixture de entrada de quest")
    return prefix+main.replace(invocation,"Require(Packet_ParseReceived(quest)==1);")


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-dispatch-") as temp:
        directory=Path(temp)
        current=build_and_run(directory/"current",harness(),args); print(current,end="")
        integrated=build_and_run(directory/"integrated",integrated_harness(),args); print(integrated,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            old_integrated=build_and_run(directory/"old-integrated",integrated_harness(args.reference_ref),args)
            if current!=reference or integrated!=old_integrated: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
