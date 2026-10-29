#!/usr/bin/env python3
"""Inicialización ambiental real; compara el pool completo con direcciones altas."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using uint=unsigned int;
const float _DAT_005524f4=.1f,_DAT_00552534=.6f,_DAT_00552950=2.5f;
void* DAT_07abf5d8=reinterpret_cast<void*>(uintptr_t(0x28000000u));
BYTE* record;unsigned trial,randomAt;int world;std::vector<DWORD> events;
DWORD Bits(float f){DWORD b;std::memcpy(&b,&f,4);return b;}
int Random(unsigned n){const int values[]={0,1,2,3,7,127,128,255,511,512,599,1023,4095,32767,65535,0x7fffffff};return values[(trial+n*5)%16];}
int ControlledRand(){Require(randomAt<3);int r=Random(randomAt++);events.insert(events.end(),{2,DWORD(r)});return r;}
struct Map{int GetCurrentMap(){events.insert(events.end(),{3,DWORD(world)});return world;}}gMapManager;
void RenderFxSnapshots_New(void* p){Require(p==record);events.push_back(1);}
void Joint_Create(int type,float* p,float* q,float* a,int sub,int owner,float scale,int key,int flag){
 Require(type==1258&&reinterpret_cast<BYTE*>(p)==record+16&&p==q&&reinterpret_cast<BYTE*>(a)==record+28&&sub==4&&DWORD(owner)==DWORD(reinterpret_cast<uintptr_t>(record))&&scale==30&&key==-1&&flag==0);
 events.push_back(4);
}
#define _rand ControlledRand
void Init(unsigned int* puVar9){float* pfVar1=reinterpret_cast<float*>(puVar9-0x36);uint uVar6;int iVar8;float fVar3;
BLOCK
}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,int off,T value){std::memcpy(p+off,&value,sizeof(value));}
DWORD Get(BYTE* p,int off){DWORD r;std::memcpy(&r,p+off,4);return r;}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(int mode:{-1,0,1,6,7,8,9})for(trial=0;trial<256;++trial)for(unsigned slot=0;slot<4;++slot){
  constexpr unsigned size=4*444+32;for(unsigned i=0;i<size;++i)memory[i]=BYTE(trial+i*13);
  record=memory+16+slot*444;BYTE wanted[size];std::memcpy(wanted,memory,size);BYTE* out=wanted+16+slot*444;
  world=mode;randomAt=0;events.clear();std::vector<DWORD> expected={1};unsigned used=0;
  auto random=[&](){int r=Random(used++);expected.insert(expected.end(),{2,DWORD(r)});return unsigned(r);};
  for(int off:{232,236,240})Put(out,off,DWORD(0x3f000000));
  out[0]=1;Put(out,360,DWORD(0));Put(out,356,DWORD(0x3f800000));Put(out,100,DWORD(0xffffffff));
  for(int off:{28,32,36})Put(out,off,DWORD(0));out[220]=1;out[353]=1;Put(out,4,DWORD(0));Put(out,88,DWORD(0xffffffff));
  Put(out,96,DWORD(random()&127));unsigned speed=random()&3;expected.insert(expected.end(),{3,DWORD(mode)});
  Put(out,216,DWORD(0x41500000));out[138]=1;float f=float(speed+4)*.1f;Put(out,12,Bits(f));
  switch(mode){
   case 0:Put(out,2,short(181));Put(out,356,DWORD(float((random()&1)+2)*.1f));Put(out,204,Bits(.6f/f));expected.insert(expected.end(),{3,DWORD(mode)});coverage|=1;break;
   case 1:case 6:Put(out,2,short(mode==1?177:178));Put(out,204,Bits(.6f/f));coverage|=mode==1?2:4;break;
   case 7:Put(out,12,DWORD(float((random()&1)+8)*.1f));expected.insert(expected.end(),{3,DWORD(mode)});coverage|=8;break;
   case 8:Put(out,2,short(179));speed=random()&3;Put(out,64,Get(out,16));f=float(speed+8)*.1f;Put(out,12,Bits(f));
    Put(out,216,DWORD(0x41100000));Put(out,96,DWORD(100));Put(out,68,Get(out,20));Put(out,72,Get(out,24));Put(out,204,Bits(2.5f/f));expected.insert(expected.end(),{4,3,DWORD(mode)});coverage|=16;break;
   default:coverage|=32;break;
  }
  Init(reinterpret_cast<unsigned int*>(record+232));
  Require(events==expected&&randomAt==used&&world==mode&&std::memcmp(memory,wanted,size)==0);
  for(unsigned i=0;i<size;++i){hash^=memory[i];hash*=1099511628211ull;}for(DWORD e:events){hash^=e;hash*=1099511628211ull;}++cases;
 }
 Require(cases==7168&&coverage==63);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){
  auto* sign=static_cast<BYTE*>(Reserve(0x8e000000u));auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));Require(sign&&high);
  Require(Run(sign)==expected);std::memset(low,0x7b,4*444+32);Require(Run(high)==expected);
  for(unsigned i=0;i<4*444+32;++i)Require(low[i]==0x7b);
  std::puts("pool bajo,bit31 y alto;alias inferior yslots vecinos intactos");
 }
 std::printf("Ambient pool init:7168 casos/ruta;pool,RNG,subtipos ycallbacks %016llx\n",(unsigned long long)expected);
}
'''
def harness(revision=None):
    source=read_source('src/Render/Ambient_Particles.cpp',revision)
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    start=source.index('puVar9[0]    =');switch=source.index('switch (iVar8)',start);end=source.index('{',switch)+1;depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    block=source[start:end]
    # Dos fronteras32 ajenas al cursor: se conserva su truncamiento en el banco.
    for expression,pointer in [('(int)DAT_07abf5d8','DAT_07abf5d8'),('(int)(puVar9 + -0x3a)','(puVar9 + -0x3a)')]:
        assert block.count(expression)==1
        block=block.replace(expression,'static_cast<int>(reinterpret_cast<uintptr_t>('+pointer+'))')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('BLOCK',block)+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-init-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
