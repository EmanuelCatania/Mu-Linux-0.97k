#!/usr/bin/env python3
"""Recorridos reales 8x8: conteo/búsqueda, orden y límites nativos."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
#include <climits>
BYTE* OffsetInventoryItems;
std::vector<int> observed;
int Behavior(int type){return type>=0x700 && type<=0x707?type-0x700:type;}
struct Catalog{int GetItemBehavior(int type){Require(type>=0 && observed.size()<65);observed.push_back(type);return Behavior(type);}}gContentCatalog;
DECLARATIONS
BEHAVIOR
void Initialize(){INITIALIZERS}
'''
MAIN=r'''
void Hash(std::uint64_t& h,int value){std::uint32_t bits;std::memcpy(&bits,&value,4);for(unsigned i=0;i<4;++i){h^=(bits>>(i*8))&255;h*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;unsigned queries=0;OffsetInventoryItems=memory+16;Initialize();
 const int types[]={-2,-1,0,1,2,3,4,5,6,7,0x700,0x707,99,INT_MAX};
 for(unsigned frame=0;frame<256;++frame){
  std::memset(memory,0xa5,4384);short item[64];int key[64];unsigned level[64];
  for(unsigned slot=0;slot<64;++slot){
   const unsigned mode=frame%8;short type=0;
   if(mode==0)type=-1;else if(mode==1)type=0;else if(mode==2)type=slot%8;else if(mode==3)type=0x700+slot%8;else if(mode==4)type=slot%2?-1:slot%8;else if(mode==5)type=slot%3?-2:slot%8;else if(mode==6)type=slot%8;else type=(slot*13+frame)%19==0?0x7fff:static_cast<short>((slot*7+frame)%10-2);
   const int keys[]={0,1,-1,INT_MIN,INT_MAX};key[slot]=mode==0?0:mode==1?1:keys[(slot+frame/8)%5];item[slot]=type;
   const std::uint32_t packed=(slot*0x12345u+frame*0x11021u)^0x80000000u;level[slot]=(packed>>3)&15;
   auto* record=OffsetInventoryItems+slot*68;std::memcpy(record,&type,2);std::memcpy(record+4,&packed,4);std::memcpy(record+56,key+slot,4);
  }
  BYTE before[4384];std::memcpy(before,memory,4384);
  for(int type:types)for(int required=-1;required<=16;++required){
   int count=0,first=-1;std::vector<int> allCalls,slotCalls;
   for(unsigned slot=0;slot<64;++slot)if(Behavior(item[slot])==type && (type==-1 || key[slot]>0) && (required==-1 || level[slot]==static_cast<unsigned>(required)))++count;
   for(int column=7;column>=0;--column)for(int row=7;row>=0;--row){const unsigned slot=row*8+column;if(item[slot]>=0)allCalls.push_back(item[slot]);if(first<0){if(item[slot]>=0)slotCalls.push_back(item[slot]);if(Behavior(item[slot])==type && key[slot]>0 && (required==-1 || level[slot]==static_cast<unsigned>(required)))first=slot;}}
   observed.clear();const int actualCount=GetItemCount(type,required);Require(actualCount==count && observed==allCalls);
   observed.clear();const int actualFirst=GetItemSlot(type,required);Require(actualFirst==first && observed==slotCalls && std::memcmp(memory,before,4384)==0);
   Hash(hash,actualCount);Hash(hash,actualFirst);++queries;
  }
 }
 Require(queries==64512);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* crossing=static_cast<BYTE*>(Reserve(std::uint64_t(0x17fff0000ull)));Require(high && crossing);for(BYTE* memory:{high,crossing+0xf000}){std::memset(low,0x7b,4384);Require(Run(memory)==expected);for(unsigned i=0;i<4384;++i)Require(low[i]==0x7b);}std::puts("inventario alto y cruzando bit31; conteo/primer slot/orden iguales");}
 std::printf("Inventory scan: 64512 consultas, grilla64/readonly %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 header=read_source('src/globals.h',revision);globals=read_source('src/globals.cpp',revision)
 declarations='';initializers=''
 for name in ('p_DAT_07ea9504_','p_DAT_07ea9328_'):
  declarations+=re.search(r'^extern int\s*\*'+name+r'\s*;',header,re.M).group().replace('extern ','')
  definition=re.search(r'^int\s*\*'+name+r'\s*=\s*[^;]+;',globals,re.M).group();initializers+=re.sub(r'^int\s*\*','',definition)
 for name in ('DAT_07ea9504','DAT_07ea9328'):declarations+='\n'+re.search(r'^#define '+name+r'\s+[^\n]+',header,re.M).group()+'\n'
 behavior=re.search(r'inline int ItemBehaviorType\([^\n]+',read_source('src/Item/ContentCatalog.h',revision)).group()
 source=read_source('src/Item/Item_LegacyHelpers.cpp',revision)
 return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x20000')+SERVICES.replace('DECLARATIONS',declarations).replace('BEHAVIOR',behavior).replace('INITIALIZERS',initializers)+function(source,'GetItemCount')+function(source,'GetItemSlot')+MAIN
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-inventory-scan-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
