#!/usr/bin/env python3
"""Modo numérico TextOut: OpenFont real, sin GDI ni reconstruir el renderer legacy."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,function,read_source
SERVICES=r'''
#include <type_traits>
using std::uintptr_t;
#ifndef _WIN32
using LPBYTE=BYTE*;
#endif
DWORD DAT_055c9ff8=0x26000000u;unsigned calls;int rendererType;unsigned selectedSize;DWORD textDc,rendererDc;
struct Window{void* GetHdc(){return reinterpret_cast<void*>(uintptr_t(0x12345678u));}}gWindow;
void PathFinder_ResetContext(){Require(calls++==0);}
void OpenTGA(const char* name,int slot,unsigned filter,unsigned wrap,int arg,char flag){Require(calls==unsigned(slot+1)&&std::strcmp(name,slot==0?"Interface/FontInput.tga":"Interface/FontTest.tga")==0&&filter==0x2600&&wrap==0x2900&&arg==0&&flag==1);++calls;}
void Font_CreateTextDib(DWORD dc){Require(calls++==3);textDc=dc;}
void Font_CreateRenderer(int self,int param_1,int dc){Require(calls++==4&&DWORD(self)==DAT_055c9ff8);rendererType=param_1;rendererDc=DWORD(dc);selectedSize=(MODE_CONDITION)?0x2c4:4;}
'''
MAIN=r'''
int main(){std::uint64_t hash=14695981039346656037ull;unsigned cases=0;const DWORD boundary[]={0x10000u,0x7fffffffu,0x80000000u,0xfffffffeu,0xffffffffu};
 for(unsigned i=0;i<65541;++i){DWORD expected=i<65536?DWORD(i):boundary[i-65536];SetMode(expected);calls=0;OpenFont();Require(calls==5&&DWORD(rendererType)==expected&&selectedSize==(expected==1?0x2c4u:4u)&&textDc==0x12345678u&&rendererDc==0x12345678u&&GetMode()==expected);hash^=DWORD(rendererType);hash*=1099511628211ull;hash^=selectedSize;hash*=1099511628211ull;++cases;}
 Require(cases==65541);std::printf("Font mode:65541 valores32 %016llx\n",(unsigned long long)hash);}
'''
def harness(revision=None):
 source=read_source('src/globals.cpp',revision);match=re.search(r'(DWORD|LPBYTE)\s+lpData_055ca044\s*=\s*(?:0|NULL);',source);assert match;declaration=match.group();assertion='' if revision else 'static_assert(std::is_same<decltype(lpData_055ca044),DWORD>::value,"TextOut es valor32");'
 set_mode='lpData_055ca044=reinterpret_cast<LPBYTE>(uintptr_t(mode));' if revision else 'lpData_055ca044=mode;';get_mode='static_cast<DWORD>(reinterpret_cast<uintptr_t>(lpData_055ca044))' if revision else 'lpData_055ca044'
 factory=function(read_source('src/Render/Font_LegacyFactory.cpp',revision),'Font_CreateRenderer');condition=re.search(r'if \((param_1 == 1)\)',factory);assert condition
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('MODE_CONDITION',condition[1])+declaration+assertion+'void SetMode(DWORD mode){'+set_mode+'}DWORD GetMode(){return '+get_mode+';}'+function(read_source('src/Scene/Scene_OpenWorld.cpp',revision),'OpenFont')+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-font-mode-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
