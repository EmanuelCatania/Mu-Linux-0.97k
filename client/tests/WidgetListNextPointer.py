#!/usr/bin/env python3
"""Destructor real 408070: contexto y nodo siguiente sin truncar.

Reutiliza el fixture de listas con los campos del widget en +72/+76/+80.
Centinelas y último nodo bajos: no certifica layout nativo ni allocator real.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from ListNextPointer import SERVICES, MAIN
from SceneObjectPointer import build_and_run, function, read_source

def replace_once(text,old,new):
    if text.count(old)!=1:raise ValueError(f'Fixture inesperado: {old}')
    return text.replace(old,new)

def harness(revision=None):
    services=replace_once(SERVICES,'std::memset(expectedHeader,0,4);','std::memset(expectedHeader+72,0,4);')
    services=replace_once(services,'void operator_delete(void* p){','unsigned forgetCalls;BYTE beforeLow[4096],beforeHigh[4096];\nvoid ClothPresentation_Forget(void* p){Require(p==header && forgetCalls++==0 && events==0 && std::memcmp(low,beforeLow,4096)==0 && std::memcmp(high,beforeHigh,4096)==0);}\nvoid operator_delete(void* p){\n Require(forgetCalls==1);')
    main=replace_once(MAIN,'Store(reinterpret_cast<BYTE*>(header),&initialCount,4);','Store(reinterpret_cast<BYTE*>(header)+72,&initialCount,4);')
    main=replace_once(main,'Store(reinterpret_cast<BYTE*>(header)+4,&head32,4);Store(reinterpret_cast<BYTE*>(header)+8,&tail32,4);','Store(reinterpret_cast<BYTE*>(header)+76,&head32,4);Store(reinterpret_cast<BYTE*>(header)+80,&tail32,4);')
    main=replace_once(main,'// El cuerpo corta el enlace del último nodo antes de empezar a liberar.',r'''// La presentación se olvida antes de tocar descriptor o enlaces.
  std::memcpy(beforeLow,low,4096);std::memcpy(beforeHigh,high,4096);forgetCalls=0;
  BYTE* expectedHeader=(highHeader?expectedHigh:expectedLow)+2048+alignment*8;
  const DWORD descriptor=0x00552520;std::memcpy(expectedHeader,&descriptor,4);
  // El cuerpo corta el enlace del último nodo antes de empezar a liberar.''')
    main=replace_once(main,'LinkedList_DestroyAll(header);','FUN_00408070(header);Require(forgetCalls==1);')
    main=replace_once(main,'List next:','Widget list next:')
    source=read_source('src/Core/Runtime_Medium.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+function(source,'FUN_00408070')+main

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-widget-list-next-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
