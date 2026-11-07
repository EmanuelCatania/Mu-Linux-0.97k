#!/usr/bin/env python3
"""Validador real del chat: texto nativo; condición de dirección heredada conservada."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <string>
using uint=unsigned;using byte=BYTE;
BYTE *inputMemory,*namesMemory,*wordsMemory;char *input,*names,*words;
#define DAT_07db8710 (*reinterpret_cast<char(*)[10][256]>(input))
#define DAT_07d27610 (*reinterpret_cast<char(*)[20000]>(names))
#define DAT_07d73104 (*reinterpret_cast<char(*)[20000]>(words))
char GlobalText[459][300];
ALIASES
CONSTANTS
CONCAT
int DAT_07d78074,DAT_07d78070;std::string expectedRaw,expectedStripped;std::vector<unsigned> calls;
int mbclen(byte* p){Require(*p>0 && *p<128);return 1;}
FIND
char* Block(unsigned n){
 char* values[]={(char*)&DAT_00561740,(char*)&DAT_00561744,(char*)&DAT_00561748,(char*)&DAT_0056174c,(char*)&DAT_00561750,s_Webzen_00561754,s_WebZen_0056175c,s_Webzen_00561764,s_WEBZEN_0056176c,&DAT_07d4b4b0,&DAT_07d4b5dc};
 Require(n<11);return values[n];
}
bool FindTextA(char* text,char* pattern,bool sensitive){
 const auto address=reinterpret_cast<std::uintptr_t>(pattern),a=reinterpret_cast<std::uintptr_t>(names),b=reinterpret_cast<std::uintptr_t>(words);unsigned id=0;
 if(address>=a && address<a+20000){Require((address-a)%20==0);id=1000+unsigned((address-a)/20);}
 else if(address>=b && address<b+20000){Require((address-b)%20==0);id=2000+unsigned((address-b)/20);}
 else{for(unsigned i=0;i<11;++i)if(pattern==Block(i)){id=4000+i;break;}}
 Require(id && !sensitive && std::strcmp(text,(id<4000?expectedStripped:expectedRaw).c_str())==0);
 calls.push_back(id);return ActualFind(text,pattern,sensitive);
}
'''
MAIN=r'''
bool Contains(const std::string& text,const char* pattern){return *pattern && text.find(pattern)!=std::string::npos;}
std::uint64_t Run(BYTE* n,BYTE* a,BYTE* b){
 inputMemory=n;namesMemory=a;wordsMemory=b;names=reinterpret_cast<char*>(a+16);words=reinterpret_cast<char*>(b+16);
 const std::string texts[]={"","alpha","/alpha","a l p h a"," beta ","omega","Webzen","WEBZEN",".","/cmd","none","  ","ab cd","slash/",std::string(255,'x'),"/"+std::string(254,'x')};
 const int counts[]={-1,0,1,3,1000};std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(unsigned offset: {0u,31u})for(unsigned pattern=0;pattern<8;++pattern)for(auto& text:texts)for(int countA:counts)for(int countB:counts){
  std::memset(n,0xa5,2624);std::memset(a,0x6b,20032);std::memset(b,0x7c,20032);input=reinterpret_cast<char*>(n+16+offset);std::memcpy(input,text.c_str(),text.size()+1);
  for(unsigned i=0;i<1000;++i){std::memset(names+i*20,0,20);std::memset(words+i*20,0,20);std::strcpy(names+i*20,pattern%4==0?"":pattern%4==1?"alpha":"unmatched");std::strcpy(words+i*20,pattern%4==0?"":pattern%4==2?"beta":"other");}
  if(countA>0 && pattern%4==3)std::strcpy(names+(countA-1)*20,"omega");
  if(countB>0 && pattern%4==3)std::strcpy(words+(countB-1)*20,"alpha");
  std::memset(GlobalText[457],0,600);if(pattern&4){std::strcpy(GlobalText[457],"omega");std::strcpy(GlobalText[458],"cmd");}
  DAT_07d78074=countA;DAT_07d78070=countB;expectedRaw=text;expectedStripped.clear();for(char c:text)if(c!=' ')expectedStripped+=c;
  const std::vector<BYTE> beforeInput(n,n+2624),beforeNames(a,a+20032),beforeWords(b,b+20032);
  char beforeGlobal[600];std::memcpy(beforeGlobal,GlobalText[457],600);
  std::vector<unsigned> expected;bool blocked=false;
  for(int i=0;i<countA;++i){expected.push_back(1000+i);if(Contains(expectedStripped,names+i*20)){blocked=true;break;}}
  // La condición heredada depende del byte bajo de la dirección, no del texto.
  if(!blocked && offset==0)for(int i=0;i<countB;++i){expected.push_back(2000+i);if(Contains(expectedStripped,words+i*20)){blocked=true;break;}}
  if(!blocked)for(unsigned i=0;i<11;++i){expected.push_back(4000+i);if(Contains(expectedRaw,Block(i))){blocked=true;break;}}
  calls.clear();const char result=Chat_ValidateInputCommand();
  Require(result==char(blocked) && calls==expected && DAT_07d78074==countA && DAT_07d78070==countB && std::memcmp(n,beforeInput.data(),2624)==0 && std::memcmp(a,beforeNames.data(),20032)==0 && std::memcmp(b,beforeWords.data(),20032)==0 && std::memcmp(GlobalText[457],beforeGlobal,600)==0);
  hash^=static_cast<BYTE>(result);hash*=1099511628211ull;for(auto id:calls){hash^=id;hash*=1099511628211ull;}++queries;
 }
 Require(queries==6400);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+0x10000,low+0x20000);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<8;++mask){
   std::memset(low,0x7b,0x30000);
   Require(Run(mask&1?high:low,(mask&2?high:low)+0x10000,(mask&4?high:low)+0x20000)==expected);
   const unsigned sizes[]={2624,20032,20032};
   for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<sizes[slot];++i)Require(low[slot*0x10000+i]==0x7b);
  }
  std::puts("texto/tablas altos; dos ramas de dirección heredada y aliases intactos");
 }
 std::printf("Chat validation: 6400 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    globals_source=read_source('src/globals.cpp',revision)
    header=read_source('src/globals.h',revision)
    names=('DAT_00561740','DAT_00561744','DAT_00561748','DAT_0056174c','DAT_00561750','s_Webzen_00561754','s_WebZen_0056175c','s_Webzen_00561764','s_WEBZEN_0056176c')
    constants='\n'.join(re.search(r'^(?:DWORD|char)\s+'+name+r'\s*(?:\[\])?\s*=\s*[^;]+;',globals_source,re.M).group(0) for name in names)
    aliases='\n'.join(re.search(r'^#define '+name+r'\s+[^\n]+',header,re.M).group(0) for name in ('DAT_07d4b4b0','DAT_07d4b5dc'))
    concat=re.search(r'^#define CONCAT31[^\n]+',read_source('src/types.h',revision),re.M).group(0)
    find=function(read_source('src/UI/UI_LegacyExterns.cpp',revision),'FindTextA').replace('FindTextA','ActualFind')
    services=SERVICES.replace('ALIASES',aliases).replace('CONSTANTS',constants).replace('CONCAT',concat).replace('FIND',find)
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x40000')+services+function(read_source('src/UI/Chat.cpp',revision),'Chat_ValidateInputCommand')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-chat-validation-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
