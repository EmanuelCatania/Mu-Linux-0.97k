#!/usr/bin/env python3
"""Indicador real de durabilidad: entrada nativa y contrato de dibujo controlado."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <string>
#ifndef _WIN32
using WORD=std::uint16_t;
#endif
using std::uintptr_t;
ATTRIBUTE
CONSTANTS
void *CharacterMachine,*DAT_055c9e44,*DAT_07d78068;
char TradeOpened,PartyOpened;int PartyNumber;BYTE byte_7E11D6E;
DWORD m_dwTextColor,m_dwBackColor;
unsigned profile;bool readFails;int panel;std::vector<std::string> events;
namespace Ui{constexpr int WindowRight=1;}
namespace Gfx{void Color3f(float a,float b,float c){Require(a==1 && b==1 && c==1);events.push_back("color");}}
void EnableAlphaTest(bool on){Require(on);events.push_back("alpha");}
struct Window{int PanelOrigin(int id){Require(id==Ui::WindowRight);events.push_back("panel");return panel;}}gWindow;
int Kind(int type){return type==1000?135:type==1001?1:type;}
int ItemBehaviorType(short type){events.push_back("kind:"+std::to_string(type));return Kind(type);}
unsigned MaxFor(unsigned slot){const unsigned values[]={0,1,2,10,100,101,200,255};return values[(profile/4+slot)%8];}
unsigned Item_CalculateMaxDurability(void* item,ITEM_ATTRIBUTE* attr,int level){
 const auto address=reinterpret_cast<std::uintptr_t>(item),base=reinterpret_cast<std::uintptr_t>(CharacterMachine)+536;
 Require(address>=base && (address-base)%68==0 && (address-base)/68<12);const unsigned slot=unsigned((address-base)/68);
 WORD type;DWORD packed;std::memcpy(&type,item,2);std::memcpy(&packed,static_cast<BYTE*>(item)+4,4);
 Require(reinterpret_cast<BYTE*>(attr)==static_cast<BYTE*>(DAT_07d78068)+unsigned(type)*64 && level==int((packed>>3)&15));
 events.push_back("max:"+std::to_string(slot)+":"+std::to_string(level));return MaxFor(slot);
}
namespace Platform{
template<class F>bool RunValidatedOperation(F callback){events.push_back("read");if(readFails)return false;callback();return true;}
template<std::size_t N>void TextFormat(char(&out)[N],const char* format,const char* name,int current,int maximum){Require(std::strcmp(format,"%s (%d/%d)")==0);std::snprintf(out,N,format,name,current,maximum);}
}
std::string Draw(int x,int y,const std::string& text,DWORD color,DWORD back){return "draw:"+std::to_string(x)+":"+std::to_string(y)+":"+text+":"+std::to_string(color)+":"+std::to_string(back);}
void Text_MeasureBox(int x,int y,char* text,int a,int b,int c){Require(!a && !b && !c);events.push_back(Draw(x,y,text,m_dwTextColor,m_dwBackColor));}
'''
MAIN=r'''
constexpr unsigned tableBytes=0xa0000;
std::uint64_t Run(BYTE* memory,BYTE* attributes){
 std::memset(attributes,0x5c,tableBytes);BYTE* allocation=attributes+64;
 for(unsigned row=0;row<9216;++row){char* name=reinterpret_cast<char*>(allocation+row*64);for(unsigned i=0;i<30;++i)name[i]=char('a'+(row+i)%26);if(row%4!=3)name[row%4==0?0:row%4==1?5:29]=0;}
 const std::vector<BYTE> beforeTable(attributes,attributes+tableBytes);
 const WORD types[]={0,1,135,143,416,417,418,419,426,8191,8192,0xffff,1000,1001,32767};
 const BYTE durability[]={0,1,2,19,20,21,29,30,31,49,50,51,99,100,101,199,200,201,254,255};
 const DWORD levels[]={0,8,0x78,0xffffffffu,0x80000000u,0x7ffffff8u};
 const int origins[]={200,640,1920,-32},baselines[]={-32768,-100,0,100,32767};
 std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(profile=0;profile<64;++profile)for(unsigned guard=0;guard<9;++guard)for(unsigned party=0;party<4;++party)for(unsigned gate=0;gate<4;++gate){
  std::memset(memory,0xa5,1444);CharacterMachine=gate&2?nullptr:memory+16;TradeOpened=char(gate&1);
  PartyNumber=int(party)-1;PartyOpened=char(profile&1);panel=origins[profile%4];const int baseline=baselines[profile%5];readFails=(profile&2)!=0;
  const unsigned rowOffset=guard==1?1:guard==2?1024:0;
  DAT_055c9e44=guard==3?nullptr:allocation;DAT_07d78068=allocation+rowOffset*64;
  if(guard==4)DAT_07d78068=attributes;
  if(guard==5)DAT_07d78068=allocation+1024*64+1;
  if(guard==6)DAT_07d78068=allocation+1;
  if(guard==7)DAT_07d78068=allocation+1025*64;
  if(guard==8)DAT_07d78068=nullptr;
  for(unsigned slot=0;slot<12;++slot){
   BYTE* item=memory+16+536+slot*68;const WORD type=types[(profile+slot)%15];const DWORD packed=levels[(profile/2+slot)%6];const BYTE current=durability[(profile+slot*3)%20];
   std::memcpy(item,&type,2);std::memcpy(item+4,&packed,4);item[26]=current;
  }
  BYTE expected[1444];std::memcpy(expected,memory,1444);std::vector<std::string> wanted;
  m_dwTextColor=0x2468ace0u;m_dwBackColor=0x13572468u;byte_7E11D6E=0x80;
  DWORD textColor=m_dwTextColor,backColor=m_dwBackColor;BYTE enabled=byte_7E11D6E;unsigned drawn=0;
  if(!TradeOpened && CharacterMachine){
   wanted={"color","alpha","panel"};backColor=0x80000000u;
   for(unsigned slot=0;slot<12;++slot){
    const WORD type=types[(profile+slot)%15];if(type==0xffff || type>=8192 || guard>=3)continue;
    wanted.push_back("kind:"+std::to_string(type));const int kind=Kind(type);
    if(kind==135 || kind==143 || (kind>=416 && kind<=419))continue;
    const DWORD packed=levels[(profile/2+slot)%6];wanted.push_back("max:"+std::to_string(slot)+":"+std::to_string((packed>>3)&15));
    const unsigned maximum=type==426?200:MaxFor(slot),current=durability[(profile+slot*3)%20];unsigned state=0;
    if(current==0){state=8;textColor=0xffffffffu;}
    else if(current*5<=maximum){state=7;textColor=0xff0a0affu;}
    else if(current*10<=maximum*3){state=6;textColor=0xff009cffu;}
    else if(current*2<=maximum){state=5;textColor=0xff00e4ffu;}
    if(!state)continue;
    expected[16+536+slot*68+64]=BYTE(state);enabled=1;wanted.push_back("read");
    const char* name=reinterpret_cast<const char*>(beforeTable.data()+64+(rowOffset+type)*64);std::string display;
    if(!readFails)for(unsigned i=0;i<30 && name[i];++i)display+=name[i];
    display+=" ("+std::to_string(current)+"/"+std::to_string(maximum)+")";
    const int x=panel-(PartyNumber>0 && !PartyOpened?50:0)-10;
    wanted.push_back(Draw(x,baseline+12*int(drawn++),display,textColor,current==0?0x800a0affu:0x80000000u));
   }
  }
  events.clear();RenderBrokenItem_(baseline);
  Require(events==wanted && m_dwTextColor==textColor && m_dwBackColor==backColor && byte_7E11D6E==enabled && std::memcmp(memory,expected,1444)==0);
  for(const auto& e:events)for(unsigned char c:e){hash^=c;hash*=1099511628211ull;}
  for(unsigned i=0;i<1444;++i){hash^=memory[i];hash*=1099511628211ull;}++queries;
 }
 Require(queries==9216 && std::memcmp(attributes,beforeTable.data(),tableBytes)==0);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+0x10000);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<4;++mask){
   std::memset(low,0x7b,0xb0000);Require(Run(mask&1?high:low,(mask&2?high:low)+0x10000)==expected);
   if(mask&1)for(unsigned i=0;i<1444;++i)Require(low[i]==0x7b);
   if(mask&2)for(unsigned i=0;i<tableBytes;++i)Require(low[0x10000+i]==0x7b);
  }
  std::puts("equipo/atributos altos; alertas, estados y aliases intactos");
 }
 std::printf("Broken item: 9216 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    structs=read_source('src/structs.h',revision)
    attribute='\n'.join(re.search(r'^#define\s+'+name+r'\s+[^\n]+',structs,re.M).group(0) for name in ('MAX_CLASS','MAX_RESISTANCE'))
    attribute+='\n'+re.search(r'struct ITEM_ATTRIBUTE\b.*?\n\};',structs,re.S).group(0)
    attribute+='\nstatic_assert(sizeof(ITEM_ATTRIBUTE)==64 && offsetof(ITEM_ATTRIBUTE,Name)==0);\n'
    constants='\n'.join(re.findall(r'^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;',read_source('src/Item/ItemDefines.h',revision),re.M))
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')+SERVICES.replace('\nATTRIBUTE\n','\n'+attribute+'\n').replace('CONSTANTS',constants)+function(read_source('src/Render/HUD_Pass2.cpp',revision),'RenderBrokenItem_')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-broken-item-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
