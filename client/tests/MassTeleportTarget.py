#!/usr/bin/env python3
"""Recall de party real: descarta terreno bloqueado antes de enviar o animar.

Extrae el dispatch DW y los constructores reales de estado/paquete. Entidades,
party,RNG,ángulo,transporte y animación controlados; no es una sesión de juego.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def function(source,name):
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    match=re.search(r'(?:static\s+)?void\s+(?:__cdecl\s+)?'+name+r'\([^;{]*\)\s*\{',source)
    if not match:raise ValueError('Función no encontrada:'+name)
    end=match.end();depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[match.start():end]

SERVICES=r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;
#endif
#ifndef _MSC_VER
#define __cdecl
#endif
using uintptr_t=std::uintptr_t;
void Require(bool ok){if(!ok){std::fputs("Recall: destino,paquete,estado u orden incorrecto\n",stderr);std::exit(1);}}
struct OBJECT{WORD Type;BYTE CurrentAction,Teleport;float Alpha,Position[3],Angle[3];};
struct CHARACTER{BYTE Class;OBJECT Object;float TargetPosition[2];}heroView;
char *hero,*characters,*Hero,*CharactersClient,*target;
int DAT_07e91388,SelectedCharacter,PartyNumber,Teleport;DWORD TargetX,TargetY,DAT_07e11dc4,DAT_07e11dc8;
DWORD DAT_07d78098,DAT_07d7809c,MovementSkillTarget,CurrentSkill;
BYTE terrain[65536];unsigned char* TerrainWall=terrain;
bool member;int castingSkill=15;std::vector<int> randoms,events;unsigned randomAt;std::vector<BYTE> packet;
CHARACTER* AsCharacter(char* p){Require(p==hero);return &heroView;}
OBJECT* AsObject(char* p){Require(p==hero||p==target);return p==hero?&heroView.Object:reinterpret_cast<OBJECT*>(target);}
namespace EntityName{const char* Get(const void* p){Require(p==target);return "Ally";}}
const char* Party_GetName(int row){Require(row>=0&&row<PartyNumber);return row==1&&member?"Ally":"Someone";}
int ControlledRand(){Require(randomAt<randoms.size());return randoms[randomAt++];}
namespace Platform{struct Timer{DWORD Ticks(){return 10000;}};Timer& GetTimer(){static Timer timer;return timer;}}
float CreateAngle(float x,float y,float gx,float gy){Require(x==10&&y==20&&gx==30&&gy==40);events.push_back(1);return 42;}
struct Network{void Send(const BYTE* data,int size){Require(size==(castingSkill==6?6:7)&&packet.empty());packet.assign(data,data+size);events.push_back(3);}}gNetwork;
void SetPlayerTeleport(OBJECT* p){const bool role=castingSkill==6?(p==&heroView.Object||p==reinterpret_cast<OBJECT*>(hero)):p==reinterpret_cast<OBJECT*>(target);Require(role&&Teleport==1&&packet.size()==unsigned(castingSkill==6?6:7));events.push_back(4);}
void SetPlayerTeleport(int p){SetPlayerTeleport(reinterpret_cast<OBJECT*>(static_cast<uintptr_t>(static_cast<std::uint32_t>(p))));}
void* Reserve(uintptr_t address){
#ifdef _WIN32
 return VirtualAlloc(reinterpret_cast<void*>(address),65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
#else
 void* p=mmap(reinterpret_cast<void*>(address),65536,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);return p==MAP_FAILED?nullptr:p;
#endif
}
SEED
SEND
void Combat_SeedRuntimeState97k(int skill,int selected){Require(skill==15&&selected==1);events.push_back(2);ActualSeed(skill,selected);}
void Combat_SendPartyRecall97k(char* entity,int selected,BYTE x,BYTE y){Require(entity==hero&&selected==1);ActualSend(entity,selected,x,y);}
#define rand ControlledRand
ATTACK
#undef rand
'''
MAIN=r'''
template<class T>void Put(char* p,int offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
unsigned Tile(int x,int y){return ((unsigned(y)&255)<<8)|(unsigned(x)&255);}
void Case(int attr,int grid,int pending,int mode,int gate){
 castingSkill=15;
 std::memset(hero,0,916);std::memset(characters,0,3*916);std::memset(&heroView,0,sizeof(heroView));std::memset(terrain,4,sizeof(terrain));
 heroView.Class=0;heroView.Object.Type=390;heroView.Object.Alpha=1;heroView.Object.Position[0]=heroView.Object.Position[1]=float(grid)*100.0f;
 hero[913]=3;Put(target,2,WORD(390));Put(target,16,10.0f);Put(target,20,20.0f);Put(target,36,-5.0f);Put(target,360,1.0f);Put(target,476,WORD(0x1234));Put(target,788,30.0f);Put(target,792,40.0f);
 PartyNumber=2;SelectedCharacter=1;member=true;DAT_07e91388=0;Teleport=pending;TargetX=77;TargetY=88;DAT_07e11dc4=DAT_07e11dc8=0;
 DAT_07d78098=9;DAT_07d7809c=10;MovementSkillTarget=11;CurrentSkill=12;
 bool hasTarget=true;
 switch(gate){case 1:heroView.Class=1;break;case 2:hasTarget=false;break;case 3:SelectedCharacter=-1;break;case 4:PartyNumber=0;break;case 5:DAT_07e91388=1;break;case 6:member=false;break;case 7:target[261]=87;break;case 8:Put(target,2,WORD(1));target[261]=5;break;case 9:target[124]=1;break;case 10:target[124]=2;break;case 11:Put(target,360,0.5f);break;}
 randoms.clear();for(int i=0;i<11;++i){randoms.push_back(mode==2?1:i==1?0:2);randoms.push_back(1);}
 terrain[Tile(grid+1,grid)]=BYTE(mode==1?4:attr);terrain[Tile(grid-1,grid)]=mode==1?4:BYTE(grid==100?32:0);
 events.clear();packet.clear();randomAt=0;
 char beforeHero[916],expectedTarget[916];std::memcpy(beforeHero,hero,916);std::memcpy(expectedTarget,target,916);
 std::vector<BYTE> beforeTerrain(terrain,terrain+65536);
 const bool eligible=gate==0;const bool destination=eligible&&mode==0;const bool firstAllowed=attr==0||attr==32;const bool sends=destination&&!pending;
 const int wantedX=grid+(firstAllowed?1:-1),wantedY=grid;
 if(destination)Put(expectedTarget,36,42.0f);
 Attack_Label1585_97k(hero,15,hasTarget);
 const unsigned rng=eligible?(mode==0?(firstAllowed?2u:4u):22u):0u;
 Require(randomAt==rng&&std::memcmp(target,expectedTarget,916)==0&&std::memcmp(hero,beforeHero,916)==0&&std::memcmp(terrain,beforeTerrain.data(),65536)==0);
 const std::vector<int> wantedEvents=sends?std::vector<int>{1,2,3,4}:destination?std::vector<int>{1}:std::vector<int>{};Require(events==wantedEvents);
 if(sends){const std::vector<BYTE> wanted={0xc1,7,0xb0,0x34,0x12,BYTE(wantedX),BYTE(wantedY)};Require(packet==wanted&&DAT_07d78098==1&&DAT_07d7809c==3&&MovementSkillTarget==1&&CurrentSkill==15);}
 else Require(packet.empty()&&DAT_07d78098==9&&DAT_07d7809c==10&&MovementSkillTarget==11&&CurrentSkill==12);
 Require(Teleport==(destination?(pending?0:1):pending));
 if(destination)Require(TargetX==DWORD(wantedX)&&TargetY==DWORD(wantedY));else if(eligible&&mode==1)Require(TargetX==DWORD(grid+1)&&TargetY==DWORD(grid));else Require(TargetX==77&&TargetY==88);
}
int main(){
 hero=static_cast<char*>(Reserve(0x26000000u));characters=static_cast<char*>(Reserve(0x28000000u));Require(hero&&characters);Hero=hero;CharactersClient=characters;target=characters+916;
 unsigned cases=0;
 for(int attr=0;attr<256;++attr)for(int grid:{0,100,255})for(int pending:{0,1}){Case(attr,grid,pending,0,0);++cases;}
 for(int pending:{0,1}){Case(4,100,pending,1,0);Case(0,100,pending,2,0);cases+=2;for(int gate=1;gate<=11;++gate){Case(0,100,pending,0,gate);++cases;}}
 // Control del teleport propio: conserva su paquete y la animación del héroe.
 castingSkill=6;Teleport=0;packet.clear();events.clear();randomAt=0;heroView.Object.Position[0]=10;heroView.Object.Position[1]=20;heroView.TargetPosition[0]=30;heroView.TargetPosition[1]=40;TargetX=10;TargetY=11;terrain[Tile(10,11)]=0;
 Attack_Label1585_97k(hero,6,true);Require(packet==std::vector<BYTE>({0xc1,6,0x1c,0,10,11})&&events==std::vector<int>({1,3,4})&&Teleport==1&&randomAt==0&&heroView.Object.Angle[2]==42);++cases;
 Require(cases==1563);std::printf("Mass teleport: %u casos correctos;terreno,reintentos,paquete y guardas\n",cases);
}
'''

def harness(root=ROOT,revision=None):
    path='src/Combat/Combat.cpp'
    source=subprocess.check_output(['git','show',revision+':'+path],cwd=root).decode('utf-8') if revision else (root/path).read_text(encoding='utf-8')
    seed=function(source,'Combat_SeedRuntimeState97k').replace('Combat_SeedRuntimeState97k','ActualSeed')
    send=function(source,'Combat_SendPartyRecall97k').replace('Combat_SendPartyRecall97k','ActualSend')
    return SERVICES.replace('SEED',seed).replace('SEND',send).replace('ATTACK',function(source,'Attack_Label1585_97k'))+MAIN

def build_and_run(directory,source,args):
    directory.mkdir();(directory/'main.cpp').write_text(source,encoding='utf-8')
    (directory/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.25)\nproject(MassTeleport LANGUAGES CXX)\nadd_executable(mass_teleport_test main.cpp)\ntarget_compile_features(mass_teleport_test PRIVATE cxx_std_17)\nif(MSVC)\n target_compile_options(mass_teleport_test PRIVATE /utf-8 /fp:precise)\nendif()\n',encoding='utf-8')
    command=['cmake','-S',str(directory),'-B',str(directory/'build')]
    if args.generator:command+=['-G',args.generator]
    if os.name=='nt':command+=['-A',args.architecture,'-T','v143']
    subprocess.run(command,check=True);subprocess.run(['cmake','--build',str(directory/'build'),'--config','Release'],check=True)
    binary=directory/'build'/'Release'/'mass_teleport_test.exe' if os.name=='nt' else directory/'build'/'mass_teleport_test'
    print(subprocess.check_output([str(binary)],text=True),end='')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='Win32' if os.name=='nt' else 'x64');p.add_argument('--generator');p.add_argument('--source-root',type=Path,default=ROOT)
    args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-mass-teleport-') as temp:build_and_run(Path(temp)/'case',harness(args.source_root),args)
if __name__=='__main__':main()
