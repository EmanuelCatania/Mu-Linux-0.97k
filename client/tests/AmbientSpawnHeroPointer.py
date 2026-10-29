#!/usr/bin/env python3
"""Posición inicial ambiental real: Hero y pool nativos, Z conservada por bits."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using uint=unsigned int;
char* DAT_07abf5d8;unsigned trial,randomAt;
int Random(unsigned n){return trial==1024?0x7fffffff:int((trial+n*617)%1024);}
int ControlledRand(){Require(randomAt<2);return Random(randomAt++);}
#define _rand ControlledRand
void Position(unsigned int* puVar9){float* pfVar1;uint uVar6;int iVar8;
BLOCK
}
'''
MAIN=r'''
constexpr unsigned size=4*444+32;
template<class T>void Put(BYTE* p,int off,T v){std::memcpy(p+off,&v,sizeof(v));}
std::uint64_t Run(BYTE* pool,BYTE* heroMemory){
 DAT_07abf5d8=reinterpret_cast<char*>(heroMemory+16);std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 const DWORD z[]={0,0x80000000u,1,0x7f800000u,0xff800000u,0x7fc01234u,0x3f800000u,0xffffffffu};
 for(unsigned profile=0;profile<16;++profile)for(trial=0;trial<=1024;++trial)for(unsigned slot=0;slot<4;++slot){
  for(unsigned i=0;i<size;++i)pool[i]=BYTE(trial+i*13);for(unsigned i=0;i<128;++i)heroMemory[i]=BYTE(profile+i*37);
  const float x=float(int(profile)-8)*.125f,y=float(int(profile)*3-32)*.25f;Put(heroMemory,32,x);Put(heroMemory,36,y);Put(heroMemory,40,z[profile%8]);
  BYTE beforeHero[128];std::memcpy(beforeHero,heroMemory,128);BYTE wanted[size];std::memcpy(wanted,pool,size);BYTE* record=pool+16+slot*444;BYTE* out=wanted+16+slot*444;
  Put(out,16,float(int(unsigned(Random(0))%1024)-512)+x);Put(out,20,float(int(unsigned(Random(1))%1024)-512)+y);Put(out,24,z[profile%8]);
  randomAt=0;Position(reinterpret_cast<unsigned int*>(record+232));
  Require(randomAt==2&&DAT_07abf5d8==reinterpret_cast<char*>(heroMemory+16)&&std::memcmp(pool,wanted,size)==0&&std::memcmp(heroMemory,beforeHero,128)==0);
  for(unsigned i=0;i<size;++i){hash^=pool[i];hash*=1099511628211ull;}++cases;
 }
 Require(cases==65600);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* hero=static_cast<BYTE*>(Reserve(0x28000000u));Require(low&&hero);const auto expected=Run(low,hero);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));auto* highHero=static_cast<BYTE*>(Reserve(uintptr_t(0x128000000ull)));Require(high&&highHero);
  std::memset(low,0x7b,size);Require(Run(high,hero)==expected);for(unsigned i=0;i<size;++i)Require(low[i]==0x7b);
  std::memset(hero,0x6d,128);Require(Run(low,highHero)==expected);for(unsigned i=0;i<128;++i)Require(hero[i]==0x6d);
  std::memset(low,0x7b,size);std::memset(hero,0x6d,128);Require(Run(high,highHero)==expected);for(unsigned i=0;i<size;++i)Require(low[i]==0x7b);for(unsigned i=0;i<128;++i)Require(hero[i]==0x6d);
  auto* sign=static_cast<BYTE*>(Reserve(0x8e000000u));auto* signHero=static_cast<BYTE*>(Reserve(0x90000000u));Require(sign&&signHero&&Run(sign,signHero)==expected);
  std::puts("pool/Hero bajos,altos ybit31;Z por bits,readonly yaliases intactos");
 }
 std::printf("Ambient spawn Hero:65600 casos/ruta %016llx\n",(unsigned long long)expected);
}
'''
def harness(revision=None):
    source=read_source('src/Render/Ambient_Particles.cpp',revision)
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    start=source.index('pfVar1 = (float *)(puVar9 + -0x36);');end=source.index('lVar12 =',start)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('BLOCK',source[start:end])+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-spawn-hero-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
