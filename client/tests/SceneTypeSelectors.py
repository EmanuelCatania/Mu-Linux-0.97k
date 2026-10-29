#!/usr/bin/env python3
"""Selectores escalares reales de efectos: rango y dos switches, todos los short."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,read_source

def harness(revision=None):
 source=read_source('src/Render/Entity_Render_3D.cpp',revision)
 source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
 wrappers=[]
 for name,marker,kind in [('Range','case 4:', 'if'),('Map8','case 8:', 'switch'),('Map10','case 0xa:', 'switch')]:
  section=source[source.index(marker):];start=section.index('pfVar3 = (float *)(int)*(short *)(param_1 + 2);');section=section[start:]
  match=re.search(r'\b'+kind+r'\s*\(',section);begin=match.end();end=begin;depth=1
  while depth:depth+=(section[end]=='(')-(section[end]==')');end+=1
  setup=section[:match.start()];expression=section[begin:end-1]
  wrappers.append(f'int {name}(int param_1){{float* pfVar3;int entityType;{setup} return {expression};}}')
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n'.join(wrappers)+r'''
int main(){
 auto* memory=static_cast<BYTE*>(Reserve(0x26000000u));Require(memory);std::memset(memory,0x6d,64);BYTE wanted[64];std::uint64_t hash=14695981039346656037ull;
 for(unsigned bits=0;bits<65536;++bits){
  short type=bits<32768?short(bits):short(int(bits)-65536);std::memcpy(memory+18,&type,2);std::memcpy(wanted,memory,64);int actor=static_cast<int>(reinterpret_cast<std::uintptr_t>(memory+16));
  int range=Range(actor),eight=Map8(actor),ten=Map10(actor);Require(range==(type>18&&type<21)&&eight==int(type)&&ten==int(type)&&std::memcmp(wanted,memory,64)==0);
  for(int value:{range,eight,ten}){hash^=std::uint32_t(value);hash*=1099511628211ull;}
 }
 std::printf("Scene selectors:65536 tipos,3 selectores %016llx\n",(unsigned long long)hash);
}
'''

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-scene-selectors-') as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
