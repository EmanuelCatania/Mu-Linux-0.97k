#!/usr/bin/env python3
"""Asignación real de texto a burbuja: entrada nativa y campos previos al Owner."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <string_view>
#ifndef _WIN32
#define __fastcall
#endif
using byte=BYTE;using vec3_t=float[3];struct CHARACTER;
CHAT_TYPE
CHAT_ACCESSORS
static_assert(offsetof(CHAT,Text)==0x2c && offsetof(CHAT,LifeTime)==0x22c,"Campos de texto del port");
BYTE *entry,*input,*history,*table;BYTE* DAT_055c9ff0;
unsigned profile,splitCalls,dispatches;
BYTE beforeEntry[768];
namespace TextEngine{
std::vector<unsigned char> LineBreaks(std::string_view text){
 ++splitCalls;std::vector<unsigned char> out(text.size()+1);
 for(unsigned i=0;i<text.size();++i)out[i]=profile==0?0:profile==1?1:(i+profile)%7==0?2:0;
 return out;
}
}
int mbclen(byte* p){Require(*p>0 && *p<128);return 1;}
void __fastcall Dispatch(DWORD* object,int edx,const char* id,const char* text,int color,int flag){
 Require(reinterpret_cast<BYTE*>(object)==history+16 && edx==0 && id==reinterpret_cast<char*>(entry+16) && text==reinterpret_cast<char*>(input+16) && color==3 && flag==0 && !dispatches++ && std::memcmp(entry,beforeEntry,768)==0);
}
'''
MAIN=r'''
void Assign(char* text,int flag){FUN_00481a40(ARG,text,flag);}
std::uint64_t Run(BYTE* e,BYTE* s,BYTE* h){
 entry=e;input=s;history=h;std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 const int flags[]={0,1,2,-1};
 for(profile=0;profile<4;++profile)for(unsigned length=0;length<256;++length)for(unsigned mode=0;mode<3;++mode)for(int flag:flags){
  for(unsigned i=0;i<768;++i)entry[i]=static_cast<BYTE>(i*37+profile*19);
  std::memset(input,0x6b,320);std::memset(history,0x5c,64);std::memset(table,0,256);
  char* text=reinterpret_cast<char*>(input+16);
  for(unsigned i=0;i<length;++i)text[i]=(i+profile)%11==0?' ':static_cast<char>('a'+(i*7+profile)%26);text[length]=0;
  DAT_055c9ff0=mode?history+16:nullptr;const DWORD vtable=mode==2?static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(table)):0;
  std::memcpy(history+16,&vtable,4);void* callback=reinterpret_cast<void*>(&Dispatch);std::memcpy(table+28*sizeof(void*),&callback,sizeof(callback));
  std::memcpy(beforeEntry,entry,768);BYTE beforeInput[320],beforeHistory[64],beforeTable[256],expected[768];std::memcpy(beforeInput,input,320);std::memcpy(beforeHistory,history,64);std::memcpy(beforeTable,table,256);std::memcpy(expected,entry,768);
  const int timer=flag==0?int(2*length+160):flag==1?1000:static_cast<int>(reinterpret_cast<std::uintptr_t>(text));
  if(length<=19)std::memcpy(expected+16+0x2c,text,length+1);
  else{
   unsigned cut=0;
   for(unsigned i=0;i<length;++i){
    const bool allowed=profile==1 || (profile>=2 && (i+profile)%7==0);
    if((int(i)>=int(length)/2-2 && (text[i]==' ' || allowed)) || int(i)>=int(length)/2+2){cut=i;break;}
   }
   std::memcpy(expected+16+0x2c,text+cut,length-cut);expected[16+0x2c+length-cut]=0;
   std::memcpy(expected+16+0x12c,text,cut);expected[16+0x12c+cut]=0;
   std::memcpy(expected+16+0x230,&timer,4);
  }
  std::memcpy(expected+16+0x22c,&timer,4);
  splitCalls=dispatches=0;Assign(text,flag);
  Require(splitCalls==unsigned(length>19) && dispatches==unsigned(flag==1 && mode==2) && std::memcmp(entry,expected,768)==0 && std::memcmp(input,beforeInput,320)==0 && std::memcmp(history,beforeHistory,64)==0 && std::memcmp(table,beforeTable,256)==0);
  for(unsigned i=0;i<768;++i){hash^=entry[i];hash*=1099511628211ull;}hash^=dispatches;hash*=1099511628211ull;++queries;
 }
 Require(queries==12288);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);table=low+0x8000;
 const auto expected=Run(low,low+4096,low+8192);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<8;++mask){
   std::memset(low,0x7b,0x3000);Require(Run(mask&1?high:low,(mask&2?high:low)+4096,(mask&4?high:low)+8192)==expected);
   const unsigned sizes[]={768,320,64};
   for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<sizes[slot];++i)Require(low[slot*4096+i]==0x7b);
  }
  std::puts("entrada/texto/historial altos; vtable controlada baja, campos y aliases intactos");
 }
 std::printf("Chat assignment: 12288 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    header=read_source('src/UI/ChatTypes.h',revision)
    chat=header[header.index('#pragma pack(push, 1)'):header.index('#pragma pack(pop)')+len('#pragma pack(pop)')]
    accessors='\n'.join(re.findall(r'^inline (?:const )?CHAT\* AsChat[^\n]+',header,re.M))
    assign=function(read_source('src/UI/Chat_Bubbles.cpp',revision),'FUN_00481a40')
    argument='reinterpret_cast<CHAT*>(entry+16)' if 'CHAT* param_1' in assign else 'static_cast<int>(reinterpret_cast<std::uintptr_t>(entry+16))'
    cut=function(read_source('src/UI/UI_LegacyExterns.cpp',revision),'CutText')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('CHAT_TYPE',chat).replace('CHAT_ACCESSORS',accessors)+cut+assign+MAIN.replace('ARG',argument)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-chat-assignment-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
