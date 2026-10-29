#!/usr/bin/env python3
"""Referencia render de partícula: direcciones nativas y offset con signo."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
using uint=unsigned;
using byte=unsigned char;
uintptr_t DAT_05828d58,DAT_07abf5d8;
BYTE* expectedModel;BYTE* expectedOutput;float* expectedMatrix;float expectedY;unsigned calls;
void BMD_TransformPosition(void* model,float* matrix,float* local,float* output,char flag){Require(!calls && model==expectedModel && matrix==expectedMatrix && reinterpret_cast<BYTE*>(output)==expectedOutput && local[0]==0 && local[1]==expectedY && local[2]==0 && flag==1);for(unsigned i=0;i<3;++i)output[i]=float(31+7*i);++calls;}
'''
MAIN=r'''
void Invoke(BYTE* slot){ItemDrop_SetupRenderRef(ARG);}
void Hash(std::uint64_t& h,const void* p,unsigned size){for(unsigned i=0;i<size;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* ownerMem,BYTE* particleMem,BYTE* heroMem,BYTE* modelMem){
 std::uint64_t hash=14695981039346656037ull;auto* owner=ownerMem+16;auto* slot=particleMem+16;auto* hero=heroMem+16;auto* models=modelMem+512;auto* matrix=ownerMem+512;
 const int subtypes[]={0,1,2,3,-1,-2,-3,(-2147483647-1),2147483647};const short types[]={-2,-1,0,1,2,7,20,31};
 for(unsigned trial=0;trial<32768;++trial){
  std::memset(ownerMem,0xc3,768);std::memset(particleMem,0xa5,160);std::memset(heroMem,0x5a,768);std::memset(modelMem,0x3c,6656);
  const int subtype=subtypes[trial%9],kind=trial%3==0?0x4d0:trial%3==1?0x4cf:0;const short type=types[(trial/9)%8];
  const auto owner32=static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(owner)),matrix32=static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(matrix));
  std::memcpy(slot+60,&owner32,4);std::memcpy(owner+276,&matrix32,4);std::memcpy(owner+2,&type,2);std::memcpy(slot+4,&kind,4);std::memcpy(slot+8,&subtype,4);
  for(unsigned i=0;i<3;++i){std::uint32_t bits=(trial*0x10101u+i*0x220033u)^0x7f800000u;std::memcpy(owner+16+4*i,&bits,4);}
  const int index=subtype%2;hero[628-24]=static_cast<BYTE>((trial*7)%5);hero[628]=static_cast<BYTE>((trial*11)%5);hero[628+24]=static_cast<BYTE>((trial*13)%5);const unsigned bone=hero[628+index*24];
  DAT_05828d58=reinterpret_cast<uintptr_t>(models);DAT_07abf5d8=reinterpret_cast<uintptr_t>(hero);expectedModel=models+type*188;expectedOutput=slot+16;expectedMatrix=reinterpret_cast<float*>(matrix+bone*48);expectedY=kind==0x4d0?-120.0f:0.0f;calls=0;
  BYTE ownerBefore[768],heroBefore[768],particleExpected[160],modelExpected[6656];std::memcpy(ownerBefore,ownerMem,768);std::memcpy(heroBefore,heroMem,768);std::memcpy(particleExpected,particleMem,160);std::memcpy(modelExpected,modelMem,6656);
  std::memcpy(modelExpected+(expectedModel-modelMem)+108,owner+16,12);for(unsigned i=0;i<3;++i){const float value=float(31+7*i);std::memcpy(particleExpected+16+16+4*i,&value,4);}
  Invoke(slot);Require(calls==1 && std::memcmp(ownerBefore,ownerMem,768)==0 && std::memcmp(heroBefore,heroMem,768)==0 && std::memcmp(particleExpected,particleMem,160)==0 && std::memcmp(modelExpected,modelMem,6656)==0);
  Hash(hash,particleMem,160);Hash(hash,expectedModel,188);
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024,low+2048,low+8192);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,16384);Require(Run(low,(mask&1?high:low)+1024,(mask&2?high:low)+2048,(mask&4?high:low)+8192)==expected);if(mask&1)for(unsigned i=0;i<160;++i)Require(low[1024+i]==0x7b);if(mask&2)for(unsigned i=0;i<768;++i)Require(low[2048+i]==0x7b);if(mask&4)for(unsigned i=0;i<6656;++i)Require(low[8192+i]==0x7b);}std::puts("ocho combinaciones particula/heroe/modelos altos y bajos; owner/matrices32 bajos");}
 std::printf("Particle ref: 32768 casos, modelo/hueso/bytes %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/Render/Render_LegacyBillboards.cpp',revision),'ItemDrop_SetupRenderRef')
 arg='slot' if 'BYTE* param_1' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(slot))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+code+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-particle-ref-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
