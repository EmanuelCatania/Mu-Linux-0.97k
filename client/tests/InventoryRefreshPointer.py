#!/usr/bin/env python3
"""Refresco real de inventario con equipo, atributos y nodo de ancho nativo.

UI/hash controlados; pools opacos con stride68 legado. No certifica layouts
nativos, hash real, clics del sistema ni render completo.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using uint=unsigned int;using ushort=unsigned short;using std::uintptr_t;
DWORD DAT_00559bec,DAT_07d78094,DAT_07e11d28,DAT_07ea5284,DAT_07ea5288,DAT_07ea528c,DAT_07ea5290,DAT_07eaa0c8,DAT_07eaa0cc,DAT_07eaa138,DAT_07eaa140,DAT_07eaa164,DAT_083a4124,DAT_083a4278,DAT_083a427c,GoldenArcherOpenType;
char DAT_07eaa117,DAT_07eaa118,DAT_07eaa119,DAT_07eaa11a,DAT_07eaa11b,DAT_07eaa11c,DAT_07eaa134,CharacterOpened,PartyOpened;
#define InventoryStartX DAT_07ea5288
#define InventoryStartY DAT_07ea5284
void *DAT_07cf1ffc,*DAT_07cf1ff4;int MAIN_HASH_CLASS;
BYTE *OffsetInventoryItems,*OffsetTradeItems,*OffsetWarehouseItems,*OffsetMixItems,*ShopItems,*Inventory,*node;
BYTE *machine,*attributes,*grids;BYTE expectedMachine[1536],expectedNode[1536],expectedAttributes[64],expectedGrids[28672];
unsigned profile,hashSlot,panelCalls;bool click;std::vector<int> events;
const unsigned offsets[]={16,4400,6656,14848,17100,25300};
const unsigned cells[]={64,32,120,32,120,32};
void Check(){Require(std::memcmp(machine,expectedMachine,1536)==0 && std::memcmp(node,expectedNode,1536)==0 && std::memcmp(attributes,expectedAttributes,64)==0 && std::memcmp(grids,expectedGrids,28672)==0);}
namespace Ui{constexpr int InventoryPair=1,RightPanel=2;}
int Origin(int id){return (id==Ui::InventoryPair?450:640)+int(profile)*20;}
struct Window{int PanelOrigin(int id){Require(id==(panelCalls++?Ui::InventoryPair:(CharacterOpened||PartyOpened?Ui::InventoryPair:Ui::RightPanel)));events.push_back(100+id);return Origin(id);}}gWindow;
bool Missing(unsigned slot){return profile%4==0?false:profile%4==1?true:profile%4==2?(slot%2==0):(profile+slot)%3!=0;}
uint HashTable_GetIndex(int* table,void* key){Require(table==&MAIN_HASH_CLASS && key==machine && hashSlot<12);events.push_back(200+int(hashSlot));return Missing(hashSlot++)?0xffffffffu:17;}
void* AntiTamper_HashNode(){Require(hashSlot && Missing(hashSlot-1));events.push_back(300);return node;}
void HashTable_Insert(int* table,void* value,void* key){Require(table==&MAIN_HASH_CLASS && value==node && key==machine && node[0x584]==1);events.push_back(301);}
void FUN_004d1fc0(){Check();events.push_back(400);}
void FUN_004d23b0(char* x,int y,short* pool,int width,int height,char mode){
 Check();Require(width==8);unsigned id=0;while(id<6 && reinterpret_cast<BYTE*>(pool)!=grids+offsets[id])++id;Require(id<6);
 const int expectedX=id==0?int(DAT_07ea5288)+15:id==1||id==5?int(DAT_07ea5290)+15:Origin(Ui::InventoryPair)+15;
 const int expectedY=id==0?200:id==1?int(DAT_07ea528c)+270:id==5?int(DAT_07ea528c)+70:id==3?110:50;
 Require(reinterpret_cast<uintptr_t>(x)==static_cast<uintptr_t>(expectedX) && y==expectedY && height==int(cells[id]/8) && mode==(id==4?1:0));events.push_back(410+int(id));
}
bool IsClickPushed(){events.push_back(500);return click;}
void CloseInventoryRelatedWindows(){events.push_back(600);}
'''
MAIN=r'''
bool Inside(int x,int y,int left,int top,int width,int height){return x>=left && x<left+width && y>=top && y<top+height;}
std::uint64_t Run(BYTE* m,BYTE* a,BYTE* n,BYTE* g){
 machine=m;attributes=a;node=n;grids=g;DAT_07cf1ffc=m;DAT_07cf1ff4=a;
 OffsetInventoryItems=g+offsets[0];OffsetTradeItems=g+offsets[1];OffsetWarehouseItems=g+offsets[2];OffsetMixItems=g+offsets[3];ShopItems=g+offsets[4];Inventory=g+offsets[5];
 const ushort levels[]={0,49,50,51,255,32768,65535,100};const short types[]={-1,0,32767,short(-32768)};const BYTE colors[]={0,1,2,99,255};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(profile=0;profile<8;++profile)for(unsigned flags=0;flags<32;++flags)for(unsigned mouse=0;mouse<8;++mouse){
  for(unsigned i=0;i<1536;++i){m[i]=static_cast<BYTE>(profile+i*13);n[i]=static_cast<BYTE>(profile+i*7);}
  for(unsigned i=0;i<64;++i)a[i]=static_cast<BYTE>(profile+i*31);
  for(unsigned i=0;i<28672;++i)g[i]=static_cast<BYTE>(profile+i*37);
  std::memcpy(a+14,&levels[profile],2);
  for(unsigned i=0;i<12;++i)std::memcpy(m+536+i*68,&types[(profile+i)%4],2);
  for(unsigned pool=0;pool<6;++pool)for(unsigned i=0;i<cells[pool];++i){BYTE* cell=g+offsets[pool]+i*68;std::memcpy(cell,&types[(profile+i+pool)%4],2);cell[64]=colors[(profile+i+pool)%5];}
  std::memcpy(expectedMachine,m,1536);std::memcpy(expectedNode,n,1536);std::memcpy(expectedAttributes,a,64);std::memcpy(expectedGrids,g,28672);
  for(unsigned i=0;i<12;++i)expectedMachine[600+i*68]=types[(profile+i)%4]!=-1;
  if(profile%4!=0)expectedNode[0x584]=1;
  for(unsigned pool=0;pool<6;++pool)for(unsigned i=0;i<cells[pool];++i){const unsigned at=offsets[pool]+i*68+64;const bool occupied=types[(profile+i+pool)%4]!=-1;if(!occupied)expectedGrids[at]=0;else if(pool!=5 || expectedGrids[at]!=99)expectedGrids[at]=1;}
  CharacterOpened=char(profile&1);PartyOpened=char(profile&2);click=(profile&4)!=0;
  DAT_07eaa118=char(flags&1);DAT_07eaa119=char(flags&2);DAT_07eaa11a=char(flags&4);DAT_07eaa11b=char(flags&8);DAT_07eaa11c=char(flags&16);DAT_07eaa140=profile%3;GoldenArcherOpenType=flags&16?7:0;
  DAT_07ea5290=240;DAT_07ea528c=40;DAT_07eaa0c8=240;DAT_07eaa0cc=40;
  DAT_07d78094=0x55;DAT_07eaa138=0x77;DAT_07eaa164=0x99;DAT_07eaa134=char(0x52);DAT_07eaa117=7;DAT_07e11d28=9;DAT_00559bec=10;DAT_083a4124=1;
  const int origin=Origin(CharacterOpened||PartyOpened?Ui::InventoryPair:Ui::RightPanel);
  const int xs[]={-1,20,20,190,0,60,25,189},ys[]={-1,20,250,431,0,395,395,433};
  const int x=origin+xs[mouse],y=ys[mouse];DAT_083a427c=static_cast<DWORD>(x);DAT_083a4278=static_cast<DWORD>(y);
  std::vector<int> expectedEvents={100+(CharacterOpened||PartyOpened?Ui::InventoryPair:Ui::RightPanel)};
  for(unsigned i=0;i<12;++i){expectedEvents.push_back(200+int(i));if(Missing(i)){expectedEvents.push_back(300);expectedEvents.push_back(301);}}
  expectedEvents.push_back(y<200?400:410);
  const bool buttons=!(flags&15) && GoldenArcherOpenType==0;
  const bool repair=buttons && levels[profile]>=50;
  const bool repairHit=Inside(x,y,origin+60,395,24,24),backHit=Inside(x,y,origin+25,395,24,24);
  if(repair && repairHit)expectedEvents.push_back(500);
  if(buttons && backHit){expectedEvents.push_back(500);if(click)expectedEvents.push_back(600);}
  if(flags&8){expectedEvents.push_back(415);expectedEvents.push_back(411);}
  expectedEvents.push_back(101);if(flags&1)expectedEvents.push_back(414);if(flags&2)expectedEvents.push_back(412);if((flags&4)&&profile%3!=1)expectedEvents.push_back(413);
  bool onWindow=Inside(x,y,origin,0,190,433);
  if(flags&(1|2|4|16))onWindow=onWindow||Inside(x,y,240,40,190,433);
  if(flags&8)onWindow=onWindow||Inside(x,y,240,40,190,433);
  events.clear();hashSlot=panelCalls=0;SecondPassword_Screen4();Check();
  Require(events==expectedEvents && hashSlot==12 && panelCalls==2);
  Require(DAT_07d78094==(onWindow?1u:0x55u) && DAT_07eaa138==unsigned(repair) && DAT_07eaa164==0);
  const bool closed=buttons&&backHit&&click;
  Require(DAT_07eaa134==char(0x52^(repair&&repairHit&&click)) && DAT_07eaa117==(closed?0:7));
  Require(DAT_07e11d28==(closed?0u:9u) && DAT_00559bec==(closed?6u:10u) && DAT_083a4124==((closed||(repair&&repairHit&&click))?0u:1u));
  Require(DAT_07ea5288==unsigned(origin) && DAT_07ea5284==0 && DAT_07eaa0c8==unsigned(Origin(Ui::InventoryPair)) && DAT_07eaa0cc==0);
  coverage|=profile%4==0?1u:2u;coverage|=buttons?(repair?4u:8u):0;
  if(repair&&repairHit&&click)coverage|=16;if(closed)coverage|=32;
  coverage|=y<200?64u:128u;coverage|=onWindow?256u:512u;
  if((flags&4)&&profile%3!=1)coverage|=1024;if((flags&4)&&profile%3==1)coverage|=2048;
  for(int event:events){hash^=unsigned(event);hash*=1099511628211ull;}
  for(unsigned i=0;i<12;++i){hash^=m[600+i*68];hash*=1099511628211ull;}
  for(unsigned pool=0;pool<6;++pool)for(unsigned i=0;i<cells[pool];++i){hash^=g[offsets[pool]+i*68+64];hash*=1099511628211ull;}++cases;
 }
 Require(cases==2048 && coverage==4095);return hash;
}
int main(){
 BYTE* low[4];BYTE* high[4];
 for(unsigned i=0;i<4;++i){low[i]=static_cast<BYTE*>(Reserve(0x26000000u+i*0x2000000u));Require(low[i]);}
 const auto expected=Run(low[0]+16,low[1]+16,low[2]+16,low[3]);
 if(sizeof(void*)>4){
  for(unsigned i=0;i<4;++i){high[i]=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)+i*0x2000000u));Require(high[i]);}
  for(unsigned mask=1;mask<16;++mask){
   for(unsigned i=0;i<4;++i)if(mask&(1u<<i))std::memset(low[i],0x7b,0x10000);
   Require(Run((mask&1?high[0]:low[0])+16,(mask&2?high[1]:low[1])+16,(mask&4?high[2]:low[2])+16,mask&8?high[3]:low[3])==expected);
   for(unsigned i=0;i<4;++i)if(mask&(1u<<i))for(unsigned b=0;b<0x10000;++b)Require(low[i][b]==0x7b);
  }
  std::puts("equipo/atributos/nodo/grillas altos; bytes, UI y aliases intactos");
 }
 std::printf("Inventory refresh: 2048 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Net/SecondPassword.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(source,'SecondPassword_Screen4')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-inventory-refresh-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
