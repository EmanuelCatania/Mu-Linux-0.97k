#!/usr/bin/env python3
"""Dispatcher real de dibujo quest: identidad nativa, byte y guarda nula.

Renderizadores controlados: no certifica sus cuerpos, que aún tienen casts
de puntero a int, ni dibujo/backend gráfico o layout quest nativo.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from QuestDispatchPointer import MAIN as DISPATCH_MAIN
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#ifndef _WIN32
#define __fastcall
#endif
BYTE* activeQuest; int calls,lastHandler;
void __fastcall FUN_00403320(void* ptr) { Require(ptr==activeQuest); ++calls; lastHandler=1; }
void __fastcall FUN_00403a40(void* ptr) { Require(ptr==activeQuest); ++calls; lastHandler=2; }
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0].replace("0x10000","0x100000")+SERVICES
    render=function(read_source("src/UI/Quest_Legacy.cpp",revision),"FUN_00403f30")
    main=DISPATCH_MAIN.replace("Packet_ParseReceived","FUN_00403f30").replace("RunDispatch","RunRenderDispatch")
    main=main.replace(" handlerResult=static_cast<int>(trial*0x78563412u);","")
    main=main.replace("int main() {","int main() {\n    calls=0; Require(FUN_00403f30(nullptr)==1 && calls==0);")
    main=main.replace("Quest dispatch: 4096 estados, 256 flags/retornos/bytes","Quest render dispatch: 4096 estados, 256 flags/guarda nula/bytes")
    return prefix+render+main


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-render-dispatch-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
