#!/usr/bin/env python3
"""Entradas SMD reales: slots nativos y orden de servicios; no prueba el parser."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT,build_and_run,function,read_source

SERVICES=r'''
#include <string>
uintptr_t DAT_05828d58;int selected,actions;char flip,lock;bool openResult;const char* filename;
std::vector<unsigned> events;
namespace ResourceTables{void EnsureModels(std::size_t count){Require(count==std::size_t(selected)+1);events.push_back(1);}}
bool OpenSMDFile(const char* path,int type,char value){Require(path==filename && type==(actions==-999?1:0) && value==(actions==-999?0:flip));events.push_back(2);return openResult;}
void FixupSMD(){events.push_back(3);}
void SMD2BMDModel(int id,int count){Require(id==selected && count==actions);events.push_back(4);}
void SMD2BMDAnimation(int id,char value){Require(id==selected && value==lock);events.push_back(5);}
'''
MAIN=r'''
void Hash(std::uint64_t& h,const void* data,std::size_t bytes){auto* p=static_cast<const BYTE*>(data);while(bytes--){h^=*p++;h*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory,char* name){
 constexpr unsigned slots=32,bytes=slots*sizeof(BMD)+32;const short counts[]={-32768,-1,0,1,32767};
 const unsigned lengths[]={0,1,30,31,32,63,127};std::uint64_t digest=14695981039346656037ull;
 DAT_05828d58=reinterpret_cast<uintptr_t>(memory+16);
 for(unsigned trial=0;trial<4096;++trial){
  std::memset(memory,0xa5,bytes);std::memset(name,0x7b,256);const auto length=lengths[(trial/2)%7];
  for(unsigned i=0;i<length;++i)name[i]=static_cast<char>(i%3==0?0xc3:'a'+i%26);name[length]=0;filename=name;
  selected=(trial/3)%slots;auto* slot=reinterpret_cast<BMD*>(memory+16)+selected;
  slot->NumMeshs=counts[(trial/5)%5];slot->NumBones=counts[(trial/7)%5];slot->NumActions=counts[(trial/11)%5];
  actions=trial%2?-999:static_cast<int>(trial%19)-9;flip=static_cast<char>((trial/13)%256);lock=static_cast<char>((trial/17)%256);openResult=(trial/19)%2;
  std::vector<BYTE> expected(memory,memory+bytes);BYTE nameBefore[256];std::memcpy(nameBefore,name,256);events.clear();std::vector<unsigned> wanted{1};
  if(actions!=-999){
   if(slot->NumMeshs<=0){wanted.push_back(2);if(openResult){wanted.push_back(3);wanted.push_back(4);auto* target=reinterpret_cast<BMD*>(expected.data()+16)+selected;std::memset(target->Name,0,32);std::memcpy(target->Name,name,length<31?length:31);target->Version=10;}}
   OpenSMDModel(selected,name,actions,flip);
  }else{
   if(slot->NumBones>0){wanted.push_back(2);wanted.push_back(5);}
   OpenSMDAnimation(selected,name,lock);
  }
  Require(events==wanted && std::memcmp(memory,expected.data(),bytes)==0 && std::memcmp(name,nameBefore,256)==0);
  Hash(digest,slot->Name,32);Hash(digest,&slot->Version,1);Hash(digest,&slot->NumBones,6);for(auto event:events)Hash(digest,&event,4);
 }
 return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* names=static_cast<char*>(Reserve(0x24000000u));Require(low&&names);auto expected=Run(low,names);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* highNames=static_cast<char*>(Reserve(std::uint64_t(0x124000000ull)));Require(high&&highNames);for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,65536);std::memset(names,0x6b,65536);Require(Run(mask&1?high:low,mask&2?highNames:names)==expected);if(mask&1)for(unsigned i=0;i<65536;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<65536;++i)Require(names[i]==0x6b);}std::puts("SMD: cuatro combinaciones tabla/nombre altos y bajos, aliases intactos");}
 std::printf("SMD entry:4096 casos, memoria/orden %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
    source=read_source('src/Render/BMD_LegacyLoading.cpp',revision)
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#include "'+(ROOT/'src/Model/ModelTypes.h').as_posix()+'"\n'
    return prefix+'namespace Smd { inline bool SelectModel(const void*) { return true; } }
'+SERVICES+function(source,'OpenSMDModel')+function(source,'OpenSMDAnimation')+MAIN

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
    if a.reference_ref and (os.name!='nt' or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-smd-entry-') as t:
        result=build_and_run(Path(t)/'current',harness(),a);print(result,end='')
        if a.reference_ref:
            old=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
            if old!=result:raise RuntimeError('Diferencia frente al port x86')
            print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
