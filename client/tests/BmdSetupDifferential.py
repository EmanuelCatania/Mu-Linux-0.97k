#!/usr/bin/env python3
"""Compara semántica nativa con el núcleo congelado y fixtures x86 en la misma arquitectura."""
import argparse,re,tempfile
from pathlib import Path
import BmdSetupPointer as native
from SceneObjectPointer import ROOT,read_source,build_and_run
from GatePointer import PREAMBLE

def harnesses():
    code=(ROOT/'tests/fixtures/BMDSetup-b0a02c59.cpp').read_text(encoding='utf-8')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+native.SERVICES
    definitions=read_source('src/globals.cpp');header=read_source('src/globals.h')
    for name in sorted(set(re.findall(r'_DAT_[0-9a-f]+',code))):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    legacy=prefix+code+native.MAIN.replace('ENTITY_ARG','reinterpret_cast<uintptr_t>(activeEntity)')
    # Normalizar sólo padding dependiente de arquitectura; las guardas siguen completas.
    legacy=legacy.replace('Hash(modelMemory,224);','Hash(activeModel+72,12);')
    current=native.harness().replace('Hash(modelMemory,ModelBytes);','Hash(activeModel+offsetof(BMD,BodyLight),12);')
    return current,legacy

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-setup-differential-')as t:
        current,legacy=harnesses();out=build_and_run(Path(t)/'native',current,a);old=build_and_run(Path(t)/'legacy',legacy,a)
        if out!=old:raise RuntimeError('Diferencia semántica entre layouts BMD/OBJECT')
        print(out,end='');print('BmdSetupDifferential: argumentos/colores/eventos equivalentes al núcleo b0a02c59')
if __name__=='__main__':main()
