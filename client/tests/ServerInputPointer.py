#!/usr/bin/env python3
"""Selección real de servidores: tabla nativa, bordes, paquetes y estado x86."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <algorithm>
char* table;
#define DAT_083a45d8 (*reinterpret_cast<char(*)[0x3600]>(table))
#define DAT_083a45ec (*reinterpret_cast<DWORD*>(table+20))
#define DAT_083a45ed (*(table+21))
#define DAT_083a4604 (*reinterpret_cast<DWORD*>(table+44))
#define DAT_083a4606 (*reinterpret_cast<unsigned short*>(table+46))
struct State{DWORD hi,lo,local,click,x,y,cooldown,c24,c28,tick,reset,code;int login;};State state;
#define ServerSelectHi state.hi
#define ServerSelectLo state.lo
#define ServerLocalSelect state.local
#define DAT_083a4124 state.click
#define DAT_083a427c state.x
#define DAT_083a4278 state.y
#define DAT_083a7c44 state.cooldown
#define DAT_083a7c24 state.c24
#define DAT_083a7c28 state.c28
#define DAT_083a7ac8 state.tick
#define DAT_083a4320 state.reset
#define DAT_083a4328 state.code
#define DAT_083a7c14 state.login
int DAT_055c9bf0;char DAT_083a7c64[64];DWORD DAT_083a7c68;char GlobalText[1000][300];const char* PTR_s_connect_muonline_co_kr_005615b8="fixture";unsigned short g_ServerPort=55901;bool connectMode;unsigned trial;std::vector<DWORD> events;
void CErrorReport_Write(int* context,const char* message){Require(context==&DAT_055c9bf0);if(std::strcmp(message,"> Server group selected")==0)events.push_back(1);else{Require(std::strcmp(message,"> Server selected")==0);events.push_back(4);}}
struct Network{void SendToConnectServer(const BYTE* packet,int n){Require(n==4 || n==6);events.push_back(2);events.push_back(n);for(int i=0;i<n;++i)events.push_back(packet[i]);}bool IsConnectServerMode(){events.push_back(6);return connectMode;}}gNetwork;
void PlayBuffer(int id,int a,int b){Require(id==0x19 && !a && !b);events.push_back(3);}
namespace Platform{struct Timer{DWORD Ticks(){events.push_back(5);return 0x12345678u+trial;}}timer;Timer& GetTimer(){return timer;}}
void CreateSocket(const char* host,unsigned port){Require(host==PTR_s_connect_muonline_co_kr_005615b8 && port==g_ServerPort);events.push_back(7);}
void UIChatLogWindow_AddText(const char* context,const char* text,int flag){Require(flag==1);events.push_back(8);if(context==DAT_083a7c64){Require(text==GlobalText[470]);events.push_back(470);}else{Require(context==reinterpret_cast<const char*>(&DAT_083a7c68) && text==GlobalText[471]);events.push_back(471);}}
'''
MAIN=r'''
void Packet(std::vector<DWORD>& out,unsigned code){out.push_back(2);if(code==0xffffffffu){out.insert(out.end(),{4,0xc1,4,0xf4,2});}else out.insert(out.end(),{6,0xc1,6,0xf4,3,code&255,code>>8});}
unsigned short Code(unsigned group,unsigned channel){return static_cast<unsigned short>(0x3456+group*21+channel);}
std::uint64_t Run(BYTE* memory){table=reinterpret_cast<char*>(memory+16);std::uint64_t hash=14695981039346656037ull;const int offsetsX[2][4]={{-1,0,69,70},{-1,0,134,135}},offsetsY[2][4]={{-1,0,15,16},{-1,0,19,20}};const unsigned loads[]={0,99,100,128},cooldowns[]={0,1,50};
 for(trial=0;trial<8192;++trial){std::memset(memory,0xa5,13856);std::memset(table,0,0x3600);bool active[25],pvp[25];unsigned channels[25];int non=0,yes=0;const unsigned pattern=trial%4,target=(trial/4)%25,kind=(trial/100)%2;for(unsigned group=0;group<25;++group){active[group]=pattern==3?(group+trial)%3==0:true;pvp[group]=pattern==1 || (pattern>=2 && group%2);channels[group]=active[group]?1+(group+trial)%4:0;char* row=table+group*542;std::sprintf(row,"group%u%c",group,pvp[group]?'2':'1');row[20]=static_cast<char>(channels[group]);row[21]=static_cast<char>(0x7f);if(active[group]){if(pvp[group])++yes;else ++non;}for(unsigned channel=0;channel<4;++channel){const auto code=Code(group,channel);std::memcpy(row+44+channel*26,&code,2);row[46+channel*26]=static_cast<char>(loads[channel]);}}
  const int base=(std::min)(444-16*non,220),pvpBase=base+(non-yes-1)*16;int beforeTarget=0;for(unsigned group=0;group<target;++group)if(active[group] && pvp[group]==pvp[target])++beforeTarget;const int origin=kind?(std::min)((pvp[target]?pvpBase:base)-int(channels[target])*10+8,(24-int(channels[target]))*20):(pvp[target]?pvpBase:base)+beforeTarget*16;const unsigned channel=(trial/8)%4;const int x=(kind?229:pvp[target]?375:150)+offsetsX[kind][(trial/5)%4],y=origin+(kind?int(channel)*20:0)+offsetsY[kind][(trial/13)%4];state={};state.hi=kind?target:0xffffffffu;state.lo=37;state.local=41;state.click=(trial/2)%2;state.x=static_cast<DWORD>(x);state.y=static_cast<DWORD>(y);state.cooldown=cooldowns[(trial/11)%3];state.c24=trial%3?0:9;state.c28=7;state.tick=17;state.reset=23;state.code=0xa5a50000u;state.login=9;connectMode=(trial/7)%2;State expected=state;std::vector<DWORD> expectedEvents;BYTE expectedMemory[13856];std::memcpy(expectedMemory,memory,13856);int positions[2]={base,pvpBase};
  for(unsigned group=0;group<25;++group){if(!active[group])continue;expectedMemory[16+group*542+21]=pvp[group]?1:0;const int top=positions[pvp[group]],left=pvp[group]?375:150;positions[pvp[group]]+=16;if(x>=left && x<left+70 && y>=top && y<top+16 && expected.click){expected.click=0;expected.hi=group;expected.lo=0xffffffffu;expectedEvents.push_back(1);Packet(expectedEvents,0xffffffffu);expectedEvents.push_back(3);}}
  if(expected.hi!=0xffffffffu && channels[expected.hi]){const unsigned group=expected.hi,n=channels[group];const int top=(std::min)((pvp[group]?pvpBase:base)-int(n)*10+8,(24-int(n))*20);expected.lo=0xffffffffu;for(unsigned ch=0;ch<n;++ch){if(x<229 || x>=364 || y<top+int(ch)*20 || y>=top+int(ch+1)*20)continue;const auto code=Code(group,ch);expected.local=code%20+1;expected.lo=ch;const bool full=(loads[ch]&128)!=0,ping=(loads[ch]&127)<100;if(!full && expected.click){expected.click=0;expectedEvents.push_back(3);if(ping && !expected.cooldown){expectedEvents.push_back(4);expected.cooldown=50;expected.c24=expected.c28;expected.c28=0;expected.tick=0x12345678u+trial;expected.reset=0;expected.code=(expected.code&0xffff0000u)|code;expectedEvents.push_back(5);expectedEvents.push_back(6);if(connectMode)Packet(expectedEvents,code);else expectedEvents.push_back(7);expected.login=1;expectedEvents.insert(expectedEvents.end(),{8,470,8,471});}else if(ping){if(!expected.c24)expected.c24=24;else expected.c28=24;}}}}
  if(expected.cooldown)--expected.cooldown;events.clear();CServerSelWin_UpdateWhileActive();Require(std::memcmp(&state,&expected,sizeof(state))==0 && events==expectedEvents && std::memcmp(memory,expectedMemory,13856)==0);for(auto value:events)for(unsigned i=0;i<4;++i){hash^=(value>>(i*8))&255;hash*=1099511628211ull;}const BYTE* bytes=reinterpret_cast<const BYTE*>(&state);for(unsigned i=0;i<sizeof(state);++i){hash^=bytes[i];hash*=1099511628211ull;}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* bit31=static_cast<BYTE*>(Reserve(0x7fff0000u));auto* wrap=static_cast<BYTE*>(Reserve(std::uint64_t(0xffff0000ull)));Require(high && bit31 && wrap);BYTE* addresses[]={high,bit31+0xe000,wrap+0xe000};for(auto* address:addresses){std::memset(low,0x7b,13856);Require(Run(address)==expected);for(unsigned i=0;i<13856;++i)Require(low[i]==0x7b);}std::puts("tabla alta y cruces bit31/4GiB; paquetes/estado/aliases iguales");}std::printf("Server input: 8192 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x20000')+SERVICES+function(read_source('src/Scene/Scene_ServerSelect_Input.cpp',revision),'CServerSelWin_UpdateWhileActive')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-server-input-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
