#!/usr/bin/env python3
"""Preparación real de animación y lecturas de tipo del pool ambiental."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
using std::uintptr_t;
uintptr_t DAT_05828d58;const float _DAT_00552504=.5f;
BYTE* record;int expectedType;unsigned animations;
DWORD Bits(float f){DWORD b;std::memcpy(&b,&f,4);return b;}
void BMD__PlayAnimation(void* model,float* frame,float* previous,unsigned char* action,float speed){
 Require(reinterpret_cast<uintptr_t>(model)==DAT_05828d58+expectedType*188&&reinterpret_cast<BYTE*>(frame)==record+264&&reinterpret_cast<BYTE*>(previous)==record+268&&action==record+262);
 float wanted;std::memcpy(&wanted,record+204,4);Require(Bits(speed)==Bits(wanted*.5f)&&static_cast<BYTE*>(model)[160]==record[261]);
 float old=*frame;*frame+=speed;*previous=old;*action=BYTE(*action+1);++animations;
}
void Animate(unsigned int* puVar9){void* this_model;
ANIMATE
LAB_00502b38:return;
}
short Read0(unsigned int* puVar9){short sVar5;READ0 return sVar5;}
short Read1(unsigned int* puVar9){short sVar5;READ1 return sVar5;}
bool Expired(unsigned int* puVar9,int iVar8){return (GUARD);}
'''
MAIN=r'''
constexpr unsigned poolSize=4*444+32,modelSize=1193*188+32;
template<class T>void Put(BYTE* p,int off,T v){std::memcpy(p+off,&v,sizeof(v));}
void Hash(std::uint64_t& h,unsigned v){h^=v;h*=1099511628211ull;}
std::uint64_t Run(BYTE* pool,BYTE* models){
 DAT_05828d58=reinterpret_cast<uintptr_t>(models+16);
 for(unsigned i=0;i<modelSize;++i)models[i]=BYTE(i*37+5);std::vector<BYTE> wantedModels(models,models+modelSize);
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0,totalAnimations=0;
 const short types[]={-32768,-1,0,1,177,178,179,181,191,1192,1193,32767};
 const float speeds[]={0,.125f,.5f,1,2,-1};
 for(unsigned profile=0;profile<32;++profile)for(short type:types)for(unsigned slot=0;slot<4;++slot){
  for(unsigned i=0;i<poolSize;++i)pool[i]=BYTE(profile+i*13);record=pool+16+slot*444;
  Put(record,2,type);record[261]=BYTE(profile*17);record[262]=BYTE(profile^0x5a);
  const float frame=float(profile)*.25f;Put(record,264,frame);Put(record,268,float(profile)*.5f);Put(record,204,speeds[profile%6]);
  BYTE wanted[poolSize];std::memcpy(wanted,pool,poolSize);BYTE* out=wanted+16+slot*444;
  const bool valid=type>=0&&type<=1192;
  if(valid){wantedModels[16+unsigned(type)*188+160]=record[261];Put(out,264,frame+speeds[profile%6]*.5f);Put(out,268,frame);out[262]=BYTE(out[262]+1);coverage|=1;}
  else coverage|=2;
  animations=0;expectedType=type;Animate(reinterpret_cast<unsigned int*>(record+232));
  Require(animations==unsigned(valid)&&std::memcmp(pool,wanted,poolSize)==0);totalAnimations+=animations;
  for(unsigned i=0;i<poolSize;++i)Hash(hash,pool[i]);Hash(hash,animations);++cases;
 }
 Require(cases==1536&&totalAnimations==1024&&coverage==3&&std::memcmp(models,wantedModels.data(),modelSize)==0);
 const int lives[]={-2147483647,-1,0,1,2,2147483647};unsigned readCases=0,guardCases=0;
 for(unsigned slot=0;slot<4;++slot){
  for(unsigned i=0;i<poolSize;++i)pool[i]=BYTE(slot+i*7);record=pool+16+slot*444;BYTE wanted[poolSize];std::memcpy(wanted,pool,poolSize);
  for(unsigned bits=0;bits<65536;++bits){
   const short type=bits<32768?short(bits):short(int(bits)-65536);Put(record,2,type);Put(wanted+16+slot*444,2,type);
   auto* cursor=reinterpret_cast<unsigned int*>(record+232);Require(Read0(cursor)==type&&Read1(cursor)==type);
   for(int life:lives){bool actual=Expired(cursor,life),expected=life<=1&&type!=178;Require(actual==expected);Hash(hash,unsigned(actual));++guardCases;}
   Hash(hash,bits);++readCases;
  }
  Require(std::memcmp(pool,wanted,poolSize)==0);
 }
 Require(readCases==262144&&guardCases==1572864&&std::memcmp(models,wantedModels.data(),modelSize)==0);return hash;
}
BYTE* ModelPool(uintptr_t base){for(unsigned i=0;i<4;++i)Require(Reserve(base+i*0x10000)==reinterpret_cast<void*>(base+i*0x10000));return reinterpret_cast<BYTE*>(base);}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* models=ModelPool(0x28000000u);Require(low);const auto expected=Run(low,models);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));auto* highModels=ModelPool(uintptr_t(0x128000000ull));Require(high);
  std::memset(low,0x7b,poolSize);Require(Run(high,models)==expected);for(unsigned i=0;i<poolSize;++i)Require(low[i]==0x7b);
  std::memset(models,0x6d,modelSize);Require(Run(low,highModels)==expected);for(unsigned i=0;i<modelSize;++i)Require(models[i]==0x6d);
  std::memset(low,0x7b,poolSize);std::memset(models,0x6d,modelSize);Require(Run(high,highModels)==expected);
  for(unsigned i=0;i<poolSize;++i)Require(low[i]==0x7b);for(unsigned i=0;i<modelSize;++i)Require(models[i]==0x6d);
  auto* sign=static_cast<BYTE*>(Reserve(0x8e000000u));auto* signModels=ModelPool(0x90000000u);Require(sign&&Run(sign,signModels)==expected);
  std::puts("pool/modelos bajos,altos ybit31;aliases,slots ytabla completos verificados");
 }
 std::printf("Ambient animation:1536 casos,1024 callbacks,262144 tipos y1572864 guardas/ruta %016llx\n",(unsigned long long)expected);
}
'''
def harness(revision=None):
    source=read_source('src/Render/Ambient_Particles.cpp',revision)
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    start=source.index('short modelIdx =');end=source.index(';',source.index('BMD__PlayAnimation(',start))+1
    reads=re.findall(r'^\s*sVar5\s*=[^\n]+;',source,re.M)
    if len(reads)!=2:raise ValueError('Lecturas de tipo inesperadas')
    guard=re.search(r'if \((\(iVar8 - 1 < 1\)[^\n]+)\) \{',source).group(1)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('ANIMATE',source[start:end]).replace('READ0',reads[0]).replace('READ1',reads[1]).replace('GUARD',guard)+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-animation-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
