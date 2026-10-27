#!/usr/bin/env python3
"""Filas de inventario como cantidad int: ABI, valor, callbacks y stride68 reales."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
#include <type_traits>
#ifndef _WIN32
#define __fastcall
#endif
unsigned frameNumber,callbackCursor;BYTE* inventory;
std::vector<unsigned> callbackSlots;
int Price(short type,std::uint32_t level){return int(type)*17+int((level>>3)&15)*13-int(frameNumber%5)*10;}
int Item_CalculateValue(void* item,int mode){Require(mode==0 && callbackCursor<callbackSlots.size() && item==inventory+callbackSlots[callbackCursor]*68);const unsigned slot=callbackSlots[callbackCursor++];short type;std::uint32_t level;std::memcpy(&type,item,2);std::memcpy(&level,static_cast<BYTE*>(item)+4,4);const int price=Price(type,level);if(frameNumber%4==3 && slot%5==0){const short chaos=399;std::memcpy(item,&chaos,2);}return price;}
'''
MAIN=r'''
using ExpectedFunction=long long (__fastcall*)(int,unsigned,short*,int,ROW_TYPE);
static_assert(std::is_same<decltype(&CalculateInventoryValue),ExpectedFunction>::value,"Filas: contrato incompatible");
long long Invoke(short* grid,int columns,int rows){return CalculateInventoryValue(-42,0xffffffffu,grid,columns,ROWS);}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;inventory=memory+16;unsigned queries=0;const short types[]={-1,-2,0,1,15,399,461,462,464,470,32767};const int keys[]={0,1,-1,(-2147483647-1),2147483647};
 for(frameNumber=0;frameNumber<256;++frameNumber){BYTE original[4384];std::memset(original,0xa5,sizeof(original));for(unsigned slot=0;slot<64;++slot){const unsigned pattern=frameNumber%8;const short type=pattern==0?-1:pattern==1?461:pattern==2?399:pattern==3?1:types[(slot*7+frameNumber)%11];const int key=pattern<4?(pattern==0?0:1):keys[(slot+frameNumber/8)%5];const std::uint32_t level=(slot*0x12345u+frameNumber*0x11021u)^0x80000000u;std::memcpy(original+16+slot*68,&type,2);std::memcpy(original+16+slot*68+4,&level,4);std::memcpy(original+16+slot*68+56,&key,4);}
  for(int columns=0;columns<=8;++columns)for(int rows=-1;rows<=8;++rows){
   std::memcpy(memory,original,sizeof(original));BYTE expected[4384];std::memcpy(expected,original,sizeof(expected));callbackSlots.clear();callbackCursor=0;long long sum=0;bool chaos=false;
   const unsigned count=rows>0?static_cast<unsigned>(rows*columns):0;
   for(unsigned slot=0;slot<count;++slot){short type;int key;std::uint32_t level;std::memcpy(&type,original+16+slot*68,2);std::memcpy(&key,original+16+slot*68+56,4);std::memcpy(&level,original+16+slot*68+4,4);if(type==-1 || key<=0)continue;
    if(type==461)sum+=100000;else if(type==462)sum+=70000;else if(type==399)sum+=40000;else if(type==464 || type==470)sum+=450000;else{sum+=Price(type,level);callbackSlots.push_back(slot);if(frameNumber%4==3 && slot%5==0){type=399;std::memcpy(expected+16+slot*68,&type,2);}}
    chaos|=type==399;
   }
   const long long result=Invoke(rows<=0?nullptr:reinterpret_cast<short*>(inventory),columns,rows);Require(result==(chaos?sum:0) && callbackCursor==callbackSlots.size() && std::memcmp(memory,expected,sizeof(expected))==0);for(unsigned i=0;i<8;++i){hash^=(static_cast<std::uint64_t>(result)>>(8*i))&255;hash*=1099511628211ull;}for(BYTE value:expected){hash^=value;hash*=1099511628211ull;}++queries;
  }
 }
 Require(queries==23040);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,4384);Require(Run(high)==expected);for(unsigned i=0;i<4384;++i)Require(low[i]==0x7b);std::puts("grilla alta, filas int y alias bajo intacto");}
 std::printf("Inventory value: 23040 consultas, valor/orden/bytes %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 source=read_source('src/Item/Item_InventoryGrid.cpp',revision);body=function(source.replace('long long __fastcall CalculateInventoryValue','intptr_t __fastcall CalculateInventoryValue'),'CalculateInventoryValue').replace('intptr_t __fastcall CalculateInventoryValue','long long __fastcall CalculateInventoryValue',1)
 definitions=read_source('src/Item/ItemDefines.h',revision);entries=re.findall(r'^(constexpr (?:int|short) (\w+)\s*=\s*([^;]+);)',definitions,re.M);wanted=set(re.findall(r'\bITEM_JEWEL_\w+\b',body));lookup={name:value for _,name,value in entries}
 while True:
  expanded=wanted|{dep for name in wanted for dep in re.findall(r'\bITEM_\w+\b',lookup[name])}
  if expanded==wanted:break
  wanted=expanded
 constants=''.join(text for text,name,_ in entries if name in wanted)
 new='int p5)' in body
 rows='rows' if new else 'reinterpret_cast<short*>(static_cast<intptr_t>(rows))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+constants+body+MAIN.replace('ROW_TYPE','int' if new else 'short*').replace('ROWS',rows)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-inventory-value-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
