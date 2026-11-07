#!/usr/bin/env python3
"""Requisitos de clase reales con ficha nativa; formato y buffers verificados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
#define CHARMENU_ROW_MAX 28
#define crt_sprintf std::sprintf
uintptr_t DAT_07abf5d8;
char GlobalText[1040][32];
char lpString_07e90798[3200];
int DAT_07e91708[32],DAT_07ea7b10[32],DAT_07eaa154,DAT_07eaa158;
'''
MAIN=r'''
void Invoke(const BYTE* item){ItemHelp_RequireClass(ARG);}
void Hash(std::uint64_t& h,const void* p,unsigned size){for(unsigned i=0;i<size;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* itemMem,BYTE* heroMem){
 std::uint64_t digest=14695981039346656037ull;auto* item=itemMem+16;auto* hero=heroMem+16;unsigned cases=0;
 for(unsigned skin=0;skin<256;++skin)for(unsigned pattern=0;pattern<256;++pattern){
  std::memset(itemMem,0xa5,96);std::memset(heroMem,0xc3,512);for(unsigned c=0;c<4;++c)item[56+c]=(pattern>>(2*c))&3;hero[444]=static_cast<BYTE>(skin);
  BYTE itemBefore[96],heroBefore[512];std::memcpy(itemBefore,itemMem,96);std::memcpy(heroBefore,heroMem,512);
  std::memset(lpString_07e90798,0x5a,sizeof(lpString_07e90798));for(unsigned r=0;r<32;++r){DAT_07e91708[r]=37+r;DAT_07ea7b10[r]=73+r;}
  const int rows[]={0,1,24,25};const int start=rows[(pattern+skin)%4];DAT_07eaa154=start;DAT_07eaa158=17;DAT_07abf5d8=reinterpret_cast<uintptr_t>(hero);
  char expected[3200];int color[32],bold[32],row=start,blanks=17;std::memcpy(expected,lpString_07e90798,sizeof(expected));std::memcpy(color,DAT_07e91708,sizeof(color));std::memcpy(bold,DAT_07ea7b10,sizeof(bold));
  if(start<=24){
   color[start+2]=color[start+3]=0;unsigned eligible[4],n=0;for(unsigned c=0;c<4;++c)if(item[56+c])eligible[n++]=c;
   const unsigned cls=skin%8,level=skin/8;const int unmet=item[56+cls]?2:5;
   if(n && (cls==3 || n<4)){
    std::strcpy(expected+100*row,"\n");++row;++blanks;
    if(cls==3){const bool ok=item[59] && level>=static_cast<unsigned>(item[59]-1);std::sprintf(expected+100*row,"%s:%s",ok?"ok":"no",GlobalText[23]);color[row]=ok?0:unmet;bold[row++]=0;}
    else if(n==3){unsigned missing=0;while(item[56+missing])++missing;std::sprintf(expected+100*row,"no:%s",GlobalText[20+missing]);color[row]=unmet;bold[row++]=0;}
    else{bool ok=false;for(unsigned i=0;i<n;++i){const unsigned c=eligible[i];ok|=cls==c && level>=static_cast<unsigned>(item[56+c]-1);}for(unsigned i=0;i<n;++i){const unsigned c=eligible[i];std::sprintf(expected+100*row,"ok:%s",GlobalText[4*item[56+c]+16+c]);color[row]=ok?0:unmet;bold[row++]=0;}}
   }
  }
  Invoke(item);Require(DAT_07eaa154==row && DAT_07eaa158==blanks && std::memcmp(expected,lpString_07e90798,sizeof(expected))==0 && std::memcmp(color,DAT_07e91708,sizeof(color))==0 && std::memcmp(bold,DAT_07ea7b10,sizeof(bold))==0 && std::memcmp(itemMem,itemBefore,96)==0 && std::memcmp(heroMem,heroBefore,512)==0);
  Hash(digest,expected,sizeof(expected));Hash(digest,color,sizeof(color));Hash(digest,bold,sizeof(bold));Hash(digest,&row,4);Hash(digest,&blanks,4);++cases;
 }
 char before[3200];std::memcpy(before,lpString_07e90798,sizeof(before));const int row=DAT_07eaa154,blank=DAT_07eaa158;Invoke(nullptr);DAT_07abf5d8=0;Invoke(item);Require(DAT_07eaa154==row && DAT_07eaa158==blank && std::memcmp(before,lpString_07e90798,sizeof(before))==0);
 Require(cases==65536);return digest;
}
int main(){for(unsigned i=0;i<1040;++i)std::sprintf(GlobalText[i],"g%04u",i);std::strcpy(GlobalText[60],"no:%s");std::strcpy(GlobalText[61],"ok:%s");
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,1600);Require(Run(mask&1?high:low,(mask&2?high:low)+1024)==expected);if(mask&1)for(unsigned i=0;i<96;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<512;++i)Require(low[1024+i]==0x7b);}std::puts("cuatro combinaciones ficha/heroe altos y bajos; aliases intactos");}
 std::printf("Item class: 65536 casos, texto/colores/filas %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/UI/CharMenu_Build.cpp',revision),'ItemHelp_RequireClass')
 arg='item' if 'const BYTE* pItem)' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(item))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+code+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-item-class-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
