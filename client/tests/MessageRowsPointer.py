#!/usr/bin/env python3
"""Filas reales del diálogo: base de texto nativa, coordenadas32 y fuente controlada."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source
SERVICES=r'''
#ifndef _WIN32
using LPCSTR=const char*;using HGDIOBJ=void*;
struct tagSIZE{std::int32_t cx,cy;};using LPSIZE=tagSIZE*;
#endif
using uint=unsigned;using undefined4=std::uint32_t;
char* table;
#define DAT_083a44c4 (*reinterpret_cast<char(*)[7*38]>(table))
DWORD DAT_083a4324;int DAT_07eaa13c;constexpr int FONT_BOLD=1;
unsigned drawCalls,dcCalls,fontCalls,selectCalls,lengthCalls,measureCalls,widthCalls,startX,startY,glyphWidth,screenWidth;float currentColor[3];int dcToken,fontToken;std::vector<int> events;
namespace Gfx{void Color3f(float r,float g,float b){currentColor[0]=r;currentColor[1]=g;currentColor[2]=b;events.push_back(1);}}
namespace Platform{int TextLength(const char* p){Require(p==table+drawCalls*38);++lengthCalls;events.push_back(3);return static_cast<int>(std::strlen(p));}}
struct Font{void* GetTextDC(){++dcCalls;return &dcToken;}void* GetFont(int id){Require(id==FONT_BOLD);++fontCalls;return &fontToken;}void SelectFont(void* dc,HGDIOBJ font){Require(dc==&dcToken && font==&fontToken);++selectCalls;events.push_back(2);}void MeasureText(void* dc,const char* text,int length,tagSIZE* size){Require(dc==&dcToken && text==table+drawCalls*38 && length==static_cast<int>(std::strlen(text)));size->cx=length*glyphWidth;size->cy=12;++measureCalls;events.push_back(4);}}gFont;
struct Window{unsigned GetWidth(){++widthCalls;return screenWidth;}}gWindow;
std::uint64_t UI_RenderText(std::uint32_t x,std::uint32_t y,const char* text,LPSIZE maximum,char bold,std::uint32_t extra){Require(drawCalls<DAT_083a4324 && text==table+drawCalls*38 && !maximum && !bold && !extra);const unsigned length=static_cast<unsigned>(std::strlen(text));const unsigned half=(length*glyphWidth*640/screenWidth)/2;Require(x==startX-half && y==startY+18*drawCalls);float color[3]={1,.7f,0};if(DAT_07eaa13c==1 && !drawCalls)color[1]=color[2]=0;else if(DAT_07eaa13c==3 && drawCalls==3)color[1]=color[2]=.1f;if(DAT_07eaa13c==2){color[0]=color[1]=.9f;color[2]=1;}Require(std::memcmp(color,currentColor,12)==0);++drawCalls;events.push_back(5);return 0;}
int Rows(unsigned x,unsigned y){LPCSTR pCVar2,pCVar6;int iVar3,iVar4;undefined4 uVar10;tagSIZE local_d0;tagSIZE* ptVar17;unsigned local_d8=x,local_dc=y;
BLOCK
}
'''
MAIN=r'''
void Hash(std::uint64_t& hash,unsigned value){for(unsigned i=0;i<4;++i){hash^=(value>>(8*i))&255;hash*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory){table=reinterpret_cast<char*>(memory+16);std::uint64_t hash=14695981039346656037ull;const unsigned widths[]={640,800,1024,1280,1920},glyphs[]={5,7,9,13},xs[]={320,350,0,0xffffffffu},ys[]={0,90,330,0xfffffff0u};unsigned queries=0;
 for(unsigned frame=0;frame<128;++frame){std::memset(memory,0xa5,298);for(unsigned row=0;row<7;++row){const unsigned length=(frame*7+row*11)%38;for(unsigned i=0;i<length;++i)table[row*38+i]=static_cast<char>('a'+(i+frame+row)%26);table[row*38+length]=0;}BYTE before[298];std::memcpy(before,memory,298);glyphWidth=glyphs[frame%4];startX=xs[(frame/4)%4];startY=ys[(frame/16)%4];
  for(DAT_083a4324=0;DAT_083a4324<=7;++DAT_083a4324)for(DAT_07eaa13c=0;DAT_07eaa13c<6;++DAT_07eaa13c)for(unsigned width:widths){screenWidth=width;drawCalls=dcCalls=fontCalls=selectCalls=lengthCalls=measureCalls=widthCalls=0;std::memset(currentColor,0,12);events.clear();const int result=Rows(startX,startY);Require(result==static_cast<int>(DAT_083a4324) && drawCalls==DAT_083a4324 && dcCalls==2*DAT_083a4324 && fontCalls==DAT_083a4324 && selectCalls==DAT_083a4324 && lengthCalls==DAT_083a4324 && measureCalls==DAT_083a4324 && widthCalls==DAT_083a4324 && std::memcmp(memory,before,298)==0);
   std::vector<int> expected;for(unsigned row=0;row<DAT_083a4324;++row){expected.push_back(1);expected.push_back(2);if(DAT_07eaa13c==2 || (DAT_07eaa13c==1 && row==0) || (DAT_07eaa13c==3 && row==3))expected.push_back(1);expected.push_back(3);expected.push_back(4);expected.push_back(5);}Require(events==expected);Hash(hash,result);Hash(hash,drawCalls);for(int event:events)Hash(hash,event);++queries;
  }
 }
 Require(queries==30720);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,298);Require(Run(high)==expected);for(unsigned i=0;i<298;++i)Require(low[i]==0x7b);std::puts("tabla de texto alta; filas/colores/posiciones32 iguales y alias bajo intacto");}std::printf("Message rows: 30720 consultas, punteros/orden/coords %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 source=read_source('src/UI/UI_StatsPanel.cpp',revision);start=source.index('LAB_0051d371:');end=source.index('  case 0x98:',start);block=source[start:end]
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('BLOCK',block)+MAIN
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-message-rows-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
