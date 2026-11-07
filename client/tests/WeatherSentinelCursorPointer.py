#!/usr/bin/env python3
"""Dos comparaciones reales de cuatro bytes de uStack_48, sin ejecutar el tick.

Controla el valor de la clave: no certifica su inicialización en Weather_Update.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source
from WeatherCompareCursorPointer import SERVICES, MAIN

def harness(revision=None):
    source=read_source('src/Render/Weather.cpp',revision);comparisons=[]
    starts=list(re.finditer(r'^\s*puVar16\s*=[^\n]*&uStack_48[^\n]*$',source,re.M))
    if len(starts)!=2:raise ValueError('Recorridos de centinela inesperados')
    for index,start in enumerate(starts):
        end=source.index('} while (bVar21);',start.end())+len('} while (bVar21);')
        end=source.index(';',source.index('if (!bVar21)',end))+1
        code=source[start.start():end]
        declaration=re.search(r'^\s*(?:DWORD\s*\*|const unsigned char\*)\s*puVar16;',source,re.M).group()
        comparisons.append(f'''Result Compare{index}(DWORD* slot,unsigned char* key){{
 DWORD& uStack_48=*slot;{declaration}
 unsigned char* pbVar18=key;int iVar12=4,iVar14=0;bool bVar20=false,bVar21=true;
 {code}
 return {{iVar12,iVar14,bVar20,bVar21,reinterpret_cast<std::uintptr_t>(puVar16),reinterpret_cast<std::uintptr_t>(pbVar18)}};
}}''')
    main=MAIN.replace('Result(*)(DWORD**,unsigned char*)','Result(*)(DWORD*,unsigned char*)')
    main=re.sub(r'  const auto tag=[^\n]*\n','',main)
    main=main.replace('  // El valor opaco del puntero nunca se desreferencia: sólo su representación.\n','')
    main=main.replace('DWORD*(reinterpret_cast<DWORD*>(tag))','DWORD(word)')
    main=main.replace('Weather compare cursor:','Weather sentinel cursor:')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+'\n'.join(comparisons)+main

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-weather-sentinel-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
