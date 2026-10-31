"""Reproduce el cambio de ocupante sin un tick intermedio de MoveJoint."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile
import os

ROOT=Path(__file__).resolve().parents[1]
def function(source,name):
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    match=re.search(r'(?:extern\s+"C"\s+)?void\s+(?:__cdecl\s+)?'+name+r'\([^;{]*\)\s*\{',source)
    end=match.end();depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[match.start():end]
code=r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using DWORD=std::uint32_t;
#ifndef _MSC_VER
#define __cdecl
#endif
struct OBJECT{char bytes[444];};
struct CHARACTER;struct PART_t;
CHARACTER* AsCharacter(void* p){return reinterpret_cast<CHARACTER*>(p);}
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}
struct JOINT{unsigned char Live;int Type,SubType;OBJECT* Target;};
JOINT DAT_07b27150[500];
JOINT* AsJoint(JOINT* p){return p;}
char entities[0x59740],butterflies[0x1158];
std::uintptr_t DAT_07abf5d0=reinterpret_cast<std::uintptr_t>(entities);
char* DAT_083a1218=butterflies;
struct Service{void Clear(){}void Close(){}void Reset(){}}gHealthBar,gMoveList,gEventTimer,gChaosMixRates;
void GoldenArcher_ResetCharacter(){}void Party_RefreshViewportLinks(){}
void DeleteCloth(int,int,int){}
void DeleteCloth(CHARACTER*,OBJECT*,PART_t*){}
DELETE
CLEAR
void Require(bool ok){if(!ok)std::exit(1);}
void PutKey(char* slot,short key){std::memcpy(slot+476,&key,2);}
int main(){
 for(int dead=0;dead<2;++dead)for(int preserve=0;preserve<2;++preserve){
  std::memset(entities,0,sizeof(entities));std::memset(butterflies,0,sizeof(butterflies));
  char* previous=entities;char* kept=entities+916;
  previous[0]=!dead;kept[0]=1;PutKey(previous,9000);PutKey(kept,9001);
  for(int i=0;i<5;++i)DAT_07b27150[i]={1,266,0,reinterpret_cast<OBJECT*>(previous)};
  DAT_07b27150[5]={1,266,0,reinterpret_cast<OBJECT*>(kept)};
  DAT_07b27150[6]={1,266,4,reinterpret_cast<OBJECT*>(previous)};
  DAT_07b27150[7]={1,1254,0,reinterpret_cast<OBJECT*>(previous)};
  DAT_07b27150[8]={0,266,0,reinterpret_cast<OBJECT*>(previous)};
  ClearCharacters(preserve?9001:-1);
  // La lista de selección reutiliza inmediatamente la dirección del personaje.
  previous[0]=1;PutKey(previous,0);
  for(int i=0;i<5;++i)Require(!DAT_07b27150[i].Live);
  Require(DAT_07b27150[5].Live==preserve);
  Require(DAT_07b27150[6].Live&&DAT_07b27150[7].Live&&!DAT_07b27150[8].Live);
  Require(kept[0]==preserve);
 }
 std::puts("PASS: slot vivo/inactivo, selección inmediata y dueño preservado; sin transferencia de Mana Shield");
}
'''
parser=argparse.ArgumentParser();parser.add_argument('--architecture',choices=['Win32','x64'],default='Win32');parser.add_argument('--revision');args=parser.parse_args()
def read(path):
    return subprocess.check_output(['git','show',args.revision+':'+path],cwd=ROOT).decode('utf-8') if args.revision else (ROOT/path).read_text(encoding='utf-8')
code=code.replace('DELETE',function(read('src/Entity/Entity_LegacyTeleport.cpp'),'DeleteJoint')).replace('CLEAR',function(read('src/Render/SMD_Parser.cpp'),'ClearCharacters'))
with tempfile.TemporaryDirectory(prefix='mu-shield-slot-') as temp:
    path=Path(temp);(path/'main.cpp').write_text(code,encoding='utf-8')
    if os.name=='nt':
        (path/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21)\nproject(ShieldSlot LANGUAGES CXX)\nadd_executable(shield_test main.cpp)\ntarget_compile_features(shield_test PRIVATE cxx_std_17)\n')
        subprocess.run(['cmake','-S',temp,'-B',str(path/'build'),'-G','Visual Studio 17 2022','-A',args.architecture],check=True)
        subprocess.run(['cmake','--build',str(path/'build'),'--config','Release'],check=True)
        binary=path/'build/Release/shield_test.exe'
    else:
        binary=path/'shield_test';subprocess.run(['c++','-std=c++17','-fpermissive',str(path/'main.cpp'),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
