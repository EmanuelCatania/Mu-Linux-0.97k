#!/usr/bin/env python3
"""Identidad y escritura nativa de CHAT::Owner; conserva el pool legacy sin migrar su layout."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,function,read_source
SERVICES=r'''
#include <string>
#include <string_view>
#include <algorithm>
using std::uintptr_t;typedef float vec3_t[3];
#pragma pack(push,1)
struct OBJECT{BYTE Live;BYTE pad1[131];BYTE Kind;BYTE pad2[219];BYTE Visible;BYTE tail[91];};
struct CHARACTER{OBJECT Object;BYTE pad1[30];short GuildMarkIndex;BYTE pad2[269];BYTE GuildTeam,PK;BYTE tail[169];};
#pragma pack(pop)
static_assert(sizeof(OBJECT)==444&&sizeof(CHARACTER)==916&&offsetof(OBJECT,Visible)==352&&offsetof(OBJECT,Kind)==132&&offsetof(CHARACTER,GuildMarkIndex)==474&&offsetof(CHARACTER,GuildTeam)==745&&offsetof(CHARACTER,PK)==746);
CHARACTER* AsCharacter(uintptr_t p){return reinterpret_cast<CHARACTER*>(p);}CHARACTER* AsCharacter(void* p){return static_cast<CHARACTER*>(p);}
namespace TextEngine{std::size_t FitBytes(std::string_view s,std::size_t n){return (std::min)(s.size(),n);}void CopyTruncated(char* out,std::size_t capacity,std::string_view s){Require(capacity);auto n=(std::min)(s.size(),capacity-1);std::memcpy(out,s.data(),n);out[n]=0;}}
const char* Guild_GetMarkName(int row){Require(row==0||row==2);return row==0?"G0":"G2";}
'''
AFTER=r'''
char DAT_07e016f8[100*0x254];CHAT* AsChat(void* p){return static_cast<CHAT*>(p);}unsigned assigned;int assignedFlag;
void FUN_00481a40(CHAT* slot,char* text,int flag){++assigned;assignedFlag=flag;std::strcpy(slot->Text[0],text);slot->LifeTime[0]=1000+flag;}
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory,uintptr_t alias){auto* actor=reinterpret_cast<CHARACTER*>(memory+16);std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 const int count=sizeof(DAT_07e016f8)/sizeof(CHAT);Require(count>=99);std::vector<BYTE> wanted(sizeof(DAT_07e016f8));
 for(int selected:{0,50,98})for(int profile=0;profile<7;++profile)for(short guild:{short(-1),short(0),short(2)})for(BYTE pk:{BYTE(0),BYTE(3),BYTE(7)})for(BYTE kind:{BYTE(0),BYTE(4)})for(int color:{-1,2})for(bool text:{false,true})for(int flag:{0,1}){
  std::memset(memory,0x6d,960);actor->Object.Live=profile==4?0:1;actor->Object.Visible=profile==5?0:1;actor->Object.Kind=kind;actor->PK=pk;actor->GuildMarkIndex=guild;actor->GuildTeam=3;BYTE actorBefore[960];std::memcpy(actorBefore,memory,960);
  std::memset(DAT_07e016f8,0,sizeof(DAT_07e016f8));auto* pool=AsChat(DAT_07e016f8);for(int i=0;i<count;++i){pool[i].IDLifeTime=7;pool[i].LifeTime[0]=5;std::strcpy(pool[i].Text[0],"old");}
  int target=selected;if(profile==0){pool[selected].IDLifeTime=pool[selected].LifeTime[0]=0;pool[selected].Owner=reinterpret_cast<CHARACTER*>(alias);}if(profile==1||profile==2)pool[selected].Owner=actor;
  if(profile==6){pool[selected].Owner=reinterpret_cast<CHARACTER*>(alias);target=(selected+1)%99;pool[target].IDLifeTime=pool[target].LifeTime[0]=0;}
  std::memcpy(wanted.data(),DAT_07e016f8,sizeof(DAT_07e016f8));auto* expected=AsChat(wanted.data());bool change=profile!=3&&profile!=4&&profile!=5;bool existing=profile==1||profile==2;
  if(change){CHAT& value=expected[target];std::strcpy(value.ID,"UnitName");value.Color=char(color==-1?(kind==4?0:pk):color);value.GuildColor=0;if(guild<0)value.Guild[0]=0;else{value.GuildColor=3;std::strcpy(value.Guild,guild==0?"[G0] ":"[G2] ");}
   if(!existing)value.Owner=actor;
   if(!text)value.IDLifeTime=existing?10:100;else{if(existing&&value.LifeTime[0]>0){std::strcpy(value.Text[1],value.Text[0]);value.LifeTime[1]=value.LifeTime[0];}value.Owner=actor;std::strcpy(value.Text[0],"Hi");value.LifeTime[0]=1000+flag;}
  }
  assigned=0;assignedFlag=-1;char id[]="UnitName",message[3]="";if(text)std::strcpy(message,"Hi");Invoke(id,message,actor,flag,color);
  Require(std::memcmp(wanted.data(),DAT_07e016f8,sizeof(DAT_07e016f8))==0&&std::memcmp(actorBefore,memory,960)==0&&assigned==unsigned(change&&text));if(assigned)Require(assignedFlag==flag);
  const CHAT& result=pool[target];for(const char* s:{result.ID,result.Guild,result.Text[0],result.Text[1]})for(;*s;++s){hash^=BYTE(*s);hash*=1099511628211ull;}for(int n:{result.IDLifeTime,result.LifeTime[0],result.LifeTime[1],int(result.Color),int(result.GuildColor),int(assigned)}){hash^=unsigned(n);hash*=1099511628211ull;}++cases;
 }Require(cases==3024);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);auto expected=Run(low,0x28000000u);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));auto* sign=static_cast<BYTE*>(Reserve(0x8a000000u));Require(high&&sign);BYTE alias[960];std::memcpy(alias,low,960);Require(Run(high,0x26000010u)==expected&&std::memcmp(alias,low,960)==0);Require(Run(sign,0x28000000u)==expected);}std::printf("Chat owner:3024 casos/ruta %016llx\n",(unsigned long long)expected);}
'''
def harness(revision=None):
 source=read_source('src/UI/ChatTypes.h',revision);a=source.index('struct CHAT {');b=source.index('};',a)+2;chat='#pragma pack(push,1)\n'+source[a:b]+'\n#pragma pack(pop)\n'
 source=read_source('src/UI/Chat_Bubbles.cpp',revision);body=function(source,'CreateChat');format_guild=function(source,'ChatFormatGuild');call='CreateChat(id,text,actor,flag,color)' if not revision else 'CreateChat(id,text,static_cast<DWORD>(reinterpret_cast<uintptr_t>(actor)),flag,color)'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+chat+AFTER+format_guild+body+'\nvoid Invoke(char* id,char* text,CHARACTER* actor,int flag,int color){'+call+';}\n'+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-chat-owner-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
