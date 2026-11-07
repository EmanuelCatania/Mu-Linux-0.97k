#!/usr/bin/env python3
"""Residuos numéricos de efectos; ejecuta los cuatro fragmentos reales sin emitir partículas."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,read_source

def harness(revision=None):
 source=read_source('src/Render/Entity_Render_3D.cpp',revision);source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S);wrappers=[]
 for name,needle,last in [('Smoke','uVar4 = _rand();',False),('Burst','uVar4 = _rand() & 0x80000007;',False),('Light','uVar4 = _rand() & 0x80000001;',True)]:
  start=source.rindex(needle) if last else source.index(needle);part=source[start:];match=re.search(r'if \((?:pfVar3 == \(float \*\)0x0|randomRemainder == 0)\) \{',part);assert match
  setup=part[:match.start()];condition=match.group()[4:-3]
  wrappers.append(f'Result {name}(std::uint64_t input){{randomInput=std::uint32_t(input);uint uVar4,randomRemainder;float* pfVar3;{setup}return {{static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(pfVar3)),{condition}}};}}')
 end=source.index('bVar8 = ((uVar11 & 0x80000003) == 0);',source.rindex('uVar4 = _rand() & 0x80000001;'))
 # La asignación inmediatamente anterior al flag inicia el fragmento módulo4.
 start=source[:end].rstrip().rfind('\n')+1
 part=source[start:];stop=part.index('*(float *)(param_1 + 0x80) =');setup=part[:stop]
 wrappers.append(f'Result Timer(std::uint64_t input){{std::uint64_t uVar11=input;uint randomRemainder;float* pfVar3;bool bVar8;{setup}return {{static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(pfVar3)),bVar8}};}}')
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+r'''
using uint=unsigned int;struct Result{DWORD token;bool zero;};std::uint32_t randomInput;int _rand(){return randomInput<0x80000000u?int(randomInput):int(std::int64_t(randomInput)-0x100000000ll);}
'''+ '\n'.join(wrappers)+r'''
int main(){std::uint64_t hash=14695981039346656037ull;std::uint64_t state=0x87436ab45213789full;unsigned count=0;
 for(unsigned i=0;i<100032;++i){state^=state<<13;state^=state>>7;state^=state<<17;std::uint64_t input=i<32?std::uint64_t(i&15)|((i&16)?0x80000000ull:0):state;std::uint32_t bits=std::uint32_t(input);std::int64_t signedValue=bits<0x80000000u?bits:std::int64_t(bits)-0x100000000ll;
  Result results[]={Smoke(input),Burst(input),Light(input),Timer(input)};int divisors[]={2,8,2,4};
  for(int k=0;k<4;++k){std::int64_t remainder=signedValue%divisors[k];Require(results[k].token==std::uint32_t(remainder)&&results[k].zero==(remainder==0));hash^=results[k].token;hash*=1099511628211ull;hash^=results[k].zero;hash*=1099511628211ull;++count;}
 }Require(count==400128);std::printf("Scene remainders:400128 casos %016llx\n",(unsigned long long)hash);}
'''

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-scene-remainders-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
