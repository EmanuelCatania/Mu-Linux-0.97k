#!/usr/bin/env python3
"""Rutas nativas de AccessModel y prefijo textual de BMD__Open; no analiza geometría."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,function,read_source
SERVICES=r'''
#include <string>
using std::uintptr_t;
#define crt_sprintf std::sprintf
uintptr_t DAT_05828d58;char DAT_0055a7c4;unsigned opened,saved,pumped;std::string actual;
namespace ResourceTables{void EnsureModels(std::size_t n){Require(n<=2);}}
namespace Platform{struct Window{void ServiceLoadingEvents(int n){Require(n==16);++pumped;}};Window& GetWindow(){static Window window;return window;}}
FILE* crt_fopen(const char* path,const char* mode){Require(std::strcmp(mode,"rb")==0);++opened;actual=path;return nullptr;}
int BMD__Save(int,char* path,char* name){++saved;actual=std::string(path)+name;return 1;}
'''
MAIN=r'''
std::uint64_t Run(char* path,char* base,char* leaf){std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 for(const char* directory:{"Data/Logo/","Data\\Player\\","Data/Long Directory/"}){
  std::strcpy(path,directory);std::strcpy(base,"Ship");BYTE pathBefore[128],baseBefore[128];std::memcpy(pathBefore,path,128);std::memcpy(baseBefore,base,128);
  for(int index:{-1,0,1,9,10,99,100,999})for(int mode:{0,1})for(short bones:{short(0),short(2)})for(int slot:{0,1}){
   auto* model=reinterpret_cast<BYTE*>(DAT_05828d58)+slot*188;std::memset(model,0x6d,188);std::memcpy(model+0x22,&bones,2);BYTE before[188];std::memcpy(before,model,188);float one=1;for(int offset:{0x48,0x4c,0x50})std::memcpy(before+offset,&one,4);
   DAT_0055a7c4=char(mode);opened=saved=pumped=0;actual.clear();AccessModel(slot,path,base,index);
   std::string expected=directory;expected+="Ship";if(index!=-1){if(index<10)expected+='0';expected+=std::to_string(index);}expected+=".bmd";
   Require(actual==expected&&saved==unsigned(mode==0&&bones>0)&&opened==unsigned(!(mode==0&&bones>0))&&pumped<=1&&std::memcmp(before,model,188)==0);
   Require(std::memcmp(pathBefore,path,128)==0&&std::memcmp(baseBefore,base,128)==0);for(unsigned char c:actual){hash^=c;hash*=1099511628211ull;}hash^=saved;hash*=1099511628211ull;++cases;
  }
  for(const char* name:{"Logo.bmd","Ship01.bmd","Player.bmd","Long Model Name.bmd"}){std::strcpy(leaf,name);BYTE before[128];std::memcpy(before,leaf,128);opened=saved=0;actual.clear();Receive(path,leaf);Require(actual==std::string(directory)+name&&opened==1&&saved==0&&std::memcmp(before,leaf,128)==0&&std::memcmp(pathBefore,path,128)==0);for(unsigned char c:actual){hash^=c;hash*=1099511628211ull;}++cases;}
 }Require(cases==204);return hash;
}
int main(){auto* models=static_cast<BYTE*>(Reserve(0x30000000u));auto* low=static_cast<char*>(Reserve(0x26000000u));Require(models&&low);DAT_05828d58=reinterpret_cast<uintptr_t>(models);std::memset(low,0x7b,512);auto expected=Run(low,low+128,low+256);
 if(sizeof(void*)>4){auto* high=static_cast<char*>(Reserve(uintptr_t(0x126000000ull)));auto* sign=static_cast<char*>(Reserve(0x8c000000u));Require(high&&sign);std::memset(low,0,512);std::strcpy(low,"Wrong/");std::strcpy(low+128,"Wrong");std::strcpy(low+256,"Wrong.bmd");BYTE alias[512];std::memcpy(alias,low,512);std::memset(high,0x7b,512);std::memset(sign,0x7b,512);Require(Run(high,high+128,high+256)==expected&&std::memcmp(alias,low,512)==0);Require(Run(sign,sign+128,sign+256)==expected);
 }std::printf("BMD paths:204 casos/ruta %016llx\n",(unsigned long long)expected);}
'''
def harness(revision=None):
 source=read_source('src/Render/BMD_Load.cpp',revision);body=function(source,'BMD__Open');signature=body[:body.index('{')+1];a=body.index('const char *path');b=body.index('char local_4c');decl='\n'.join(line for line in body[a:b].splitlines() if line.strip().startswith('const char *'))
 fragment=body[b:body.index('fseek(fp,')];receiver=signature+decl+'\n'+fragment+'}\n'
 globals_source=read_source('src/globals.cpp',revision);constants=[]
 for name in ['DAT_005580ac','s__s_bmd_0055a7f8','s__s0_d_bmd_0055a7ec','s__s_d_bmd_0055a7e0']:
  match=re.search(r'char\s+'+name+r'\[\]\s*=\s*"[^"\n]*";',globals_source);assert match;constants.append(match.group())
 call='BMD__Open(0,path,name,0)' if not revision else 'BMD__Open(0,static_cast<int>(reinterpret_cast<uintptr_t>(path)),static_cast<int>(reinterpret_cast<uintptr_t>(name)),0)'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+'\n'.join(constants)+receiver+'void Receive(char* path,char* name){'+call+';}\n'+function(read_source('src/Render/BMD_LegacyLoading.cpp',revision),'AccessModel')+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-bmd-paths-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
