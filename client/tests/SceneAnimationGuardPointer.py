#!/usr/bin/env python3
"""Guarda real de animación de tipo103; base nativa y tabla legacy32 controlada."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using uint=unsigned int;using byte=unsigned char;
struct Result{bool match;DWORD token;};
Result Guard(int param_1,void* pModel){float* pfVar3;
TYPE
 bool match=(GUARD);
 return {match,static_cast<DWORD>(reinterpret_cast<uintptr_t>(pfVar3))};
}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,int offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
std::uint64_t Run(BYTE* actorMemory,BYTE* modelMemory,BYTE* tableMemory,BYTE* otherMemory,BYTE* aliasMemory=nullptr){
 BYTE* actor=actorMemory+16;BYTE* model=modelMemory+16;BYTE* table=tableMemory+16;BYTE* other=otherMemory+16;
 for(unsigned i=0;i<1100;++i)actorMemory[i]=BYTE(i*13+7);for(unsigned i=0;i<220;++i)modelMemory[i]=BYTE(i*37+3);
 for(unsigned i=0;i<4128;++i){tableMemory[i]=BYTE(i*17);otherMemory[i]=BYTE(i*19);}
 std::vector<BYTE> wantedTable(tableMemory,tableMemory+4128),wantedOther(otherMemory,otherMemory+4128);
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned bits=0;bits<65536;++bits)for(int mode:{0,1})for(short type:{short(102),short(103)}){
  const short count=bits<32768?short(bits):short(int(bits)-65536);const int last=int(count)-1;
  const unsigned frameBits=unsigned(mode?int(count):last)&65535u;const short frame=frameBits<32768?short(frameBits):short(int(frameBits)-65536);
  const byte action=byte(bits);Put(actor,2,type);actor[261]=action;Put(model,48,DWORD(reinterpret_cast<uintptr_t>(table)));Put(model,168,frame);
  Put(table,action*16+8,count);Put(wantedTable.data()+16,action*16+8,count);
  const unsigned alternateBits=(bits+2)&65535u;const short alternate=alternateBits<32768?short(alternateBits):short(int(alternateBits)-65536);
  Put(other,action*16+8,alternate);Put(wantedOther.data()+16,action*16+8,alternate);
  BYTE actorBefore[1100],modelBefore[220],aliasBefore[220];std::memcpy(actorBefore,actorMemory,1100);std::memcpy(modelBefore,modelMemory,220);
  if(aliasMemory){std::memset(aliasMemory,0x6d,220);Put(aliasMemory+16,48,DWORD(reinterpret_cast<uintptr_t>(other)));Put(aliasMemory+16,168,short(frame==0?1:0));std::memcpy(aliasBefore,aliasMemory,220);}
  const Result actual=Guard(static_cast<int>(reinterpret_cast<uintptr_t>(actor)),model);
  const bool expected=type==103&&int(frame)==last;const DWORD token=static_cast<DWORD>(type==103?last:int(type));
  Require(actual.match==expected&&actual.token==token&&std::memcmp(actorMemory,actorBefore,1100)==0&&std::memcmp(modelMemory,modelBefore,220)==0);
  if(aliasMemory)Require(std::memcmp(aliasMemory,aliasBefore,220)==0);
  coverage|=expected?1:2;coverage|=type==103?4:8;hash^=unsigned(actual.match);hash*=1099511628211ull;hash^=actual.token;hash*=1099511628211ull;++cases;
 }
 Require(cases==262144&&coverage==15&&std::memcmp(tableMemory,wantedTable.data(),4128)==0&&std::memcmp(otherMemory,wantedOther.data(),4128)==0);return hash;
}
int main(){
 auto* actor=static_cast<BYTE*>(Reserve(0x26000000u));auto* low=static_cast<BYTE*>(Reserve(0x28000000u));auto* table=static_cast<BYTE*>(Reserve(0x2a000000u));auto* other=static_cast<BYTE*>(Reserve(0x2c000000u));Require(actor&&low&&table&&other);const auto expected=Run(actor,low,table,other);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x128000000ull)));auto* sign=static_cast<BYTE*>(Reserve(0x8e000000u));auto* signTable=static_cast<BYTE*>(Reserve(0x90000000u));Require(high&&sign&&signTable);
  Require(Run(actor,high,table,other,low)==expected);Require(Run(actor,sign,signTable,other)==expected);
  std::puts("modelo alto/bit31,tabla32 yalias con valores opuestos;readonly verificado");
 }
 std::printf("Scene animation guard:262144 casos/ruta %016llx\n",(unsigned long long)expected);
}
'''
def harness(revision=None):
    source=read_source('src/Render/Entity_Render_3D.cpp',revision)
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    start=source.index('if ((pfVar3 == (float *)0x67)');first=source.index('(',start)+1;end=first;depth=1
    while depth:depth+=(source[end]=='(')-(source[end]==')');end+=1
    guard=source[first:end-1];assignment='pfVar3 = (float *)(int)*(short *)(param_1 + 2);';assert assignment in source
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('TYPE',assignment).replace('GUARD',guard)+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-scene-animation-guard-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
