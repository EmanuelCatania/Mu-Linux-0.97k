#!/usr/bin/env python3
"""Divisor de chat real: buffer nativo; medición y cortes controlados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <string_view>
using LPCSTR=const char*;
using UINT=unsigned;
#ifndef _WIN32
struct SIZE{int cx,cy;};
#endif
unsigned unit,scale,step,profile,measures,fits;
struct Font {void* GetTextDC(){return nullptr;} void MeasureText(void*,const char* text,int count,SIZE* size){Require(text && count>=0 && count<=static_cast<int>(std::strlen(text)));size->cx=count*unit;size->cy=19;++measures;}}gFont;
struct Window{double GetScreenRateX(){return scale;}}gWindow;
namespace Platform{int TextLength(const char* p){return static_cast<int>(std::strlen(p));} unsigned TextCharacterBytes(const unsigned char*){return step;}}
namespace TextEngine{std::size_t FitLineBreak(std::string_view text,std::size_t n){Require(n<=text.size());++fits;return profile==0?0:profile==1?n:profile==2?n/2:n/3;}}
'''
MAIN=r'''
int Split(const char* text,char* destination,int width,int count,unsigned indent,int reverse){return FUN_0040c2a0(text,ARG,width,count,128,indent,reverse);}
std::uint64_t Run(BYTE* input,BYTE* output){
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 for(unsigned length=0;length<=64;++length)for(unsigned w=0;w<4;++w)for(int count=0;count<=4;++count)for(int reverse=0;reverse<=2;++reverse)for(profile=0;profile<4;++profile)for(scale=1;scale<=2;++scale)for(step=1;step<=2;++step){
  unit=8;const int width=16<<w;const unsigned indent=w%2?8:0;std::memset(input,0xc3,256);std::memset(output,0xa5,544);
  auto* source=reinterpret_cast<char*>(input+16);for(unsigned i=0;i<length;++i)source[i]=static_cast<char>('a'+(i+length)%26);source[length]=0;
  BYTE before[256],expected[544];std::memcpy(before,input,256);std::memcpy(expected,output,544);
  unsigned cursor=0,lines=0,expectedMeasures=0,expectedFits=0;int result=1;
  for(;lines<static_cast<unsigned>(count);++lines){
   const unsigned remaining=length-cursor;++expectedMeasures;if(!remaining){result=lines;break;}
   const unsigned slot=reverse==1?count-1-lines:lines;BYTE* dest=expected+16+slot*128;
   const unsigned avail=width-(lines==0?indent:0),per=unit/scale;
   if(remaining*per<=avail){std::memcpy(dest,source+cursor,remaining);std::memset(dest+remaining,0,128-remaining);result=lines+1;break;}
   ++expectedMeasures;++expectedFits;unsigned cut=avail/per;cut=((cut+step-1)/step)*step;
   const unsigned fit=profile==0?0:profile==1?cut:profile==2?cut/2:cut/3;if(fit && fit*2>=cut)cut=fit;
   Require(cut>0 && cut<=remaining);std::memcpy(dest,source+cursor,cut);dest[cut]=0;cursor+=cut;result=lines+2;
  }
  measures=fits=0;const int actual=Split(source,reinterpret_cast<char*>(output+16),width,count,indent,reverse);
  if(!(actual==result && measures==expectedMeasures && fits==expectedFits && std::memcmp(input,before,256)==0 && std::memcmp(output,expected,544)==0)){std::fprintf(stderr,"caso %u %u %d %d %u %u %u: resultado %d/%d medidas %u/%u cortes %u/%u bytes %d\n",length,w,count,reverse,profile,scale,step,actual,result,measures,expectedMeasures,fits,expectedFits,std::memcmp(output,expected,544));Require(false);}
  for(BYTE value:expected){hash^=value;hash*=1099511628211ull;}hash^=actual;hash*=1099511628211ull;++cases;
 }
 measures=fits=0;Require(Split(nullptr,nullptr,16,4,0,1)==0 && measures==0 && fits==0);
 Require(cases==62400);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,1600);Require(Run(mask&1?high:low,(mask&2?high:low)+1024)==expected);if(mask&1)for(unsigned i=0;i<256;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<544;++i)Require(low[1024+i]==0x7b);}std::puts("cuatro combinaciones texto/destino altos y bajos; aliases intactos");}
 std::printf("Chat lines: 62400 casos, bytes/orden/canarios %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/UI/ChatListBox.cpp',revision),'FUN_0040c2a0')
 arg='destination' if 'char* dst' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(destination))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+code+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-chat-lines-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
