#!/usr/bin/env python3
"""Identidad nativa en la caché propia de equipo; sin renderer ni mallas."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <new>
#include <utility>
using std::uintptr_t;
#pragma pack(push,1)
struct OBJECT{BYTE Live,pad;short Type;BYTE beforeSpawn[SPAWN_OFFSET-4];int InitialSceneFrame;BYTE tail[444-SPAWN_OFFSET-4];};
struct CHARACTER{OBJECT Object;BYTE beforeKey[32];unsigned short Key;BYTE tail[438];};
#pragma pack(pop)
static_assert(sizeof(OBJECT)==444&&sizeof(CHARACTER)==916);
ACCESSORS
BYTE* DAT_07abf5d0;BYTE* DAT_07abf050;int SceneFlag;
struct Map{int current;int GetCurrentMap(){return current;}}gMapManager;
struct Mesh{}; // Sólo se prueba la cola de sprites; no se ejecuta AddMesh/draw.
SPRITE
COMMAND
FRAME
GLOBALS
SLOT
FUNCTIONS
DRAW_GUARD
void BeginFor(const CHARACTER* owner){BEGIN_CALL;}
bool DrawFor(const CHARACTER* owner){return DRAW_CALL;}
uintptr_t OwnerBits(const CHARACTER* p){return reinterpret_cast<uintptr_t>(p);}
uintptr_t OwnerBits(int p){return static_cast<DWORD>(p);}
'''
MAIN=r'''
constexpr unsigned bytes=400*916+32;
void Hash(std::uint64_t& h,unsigned value){h^=value;h*=1099511628211ull;}
std::uint64_t Run(BYTE* pool,BYTE* preview){
 DAT_07abf5d0=pool+16;DAT_07abf050=preview+16;std::uint64_t hash=14695981039346656037ull;unsigned count=0;
 scene=-1;world=-1;for(auto& f:frames){f.valid=false;f.commands.clear();f.sprites.clear();}for(auto& f:previousFrames){f.valid=false;f.commands.clear();f.sprites.clear();}
 for(unsigned profile=0;profile<8;++profile){
  SceneFlag=profile<4?5:4;gMapManager.current=profile<6?2:10;Equipment_BeginStep();
  for(unsigned slot=0;slot<401;++slot){
   auto* actor=new(slot==400?DAT_07abf050:DAT_07abf5d0+slot*916) CHARACTER{};
   actor->Object.Live=BYTE((profile+slot)%4);actor->Object.Type=short(int(slot%5)*177-1);actor->Object.InitialSceneFrame=int(profile*10000+slot);actor->Key=static_cast<unsigned short>(slot*257+profile);
   BYTE before[916];std::memcpy(before,actor,916);const unsigned stages[]={0,7,23};
   Require(Slot(actor)==int(slot));
   for(unsigned stage:stages){
    BeginFor(actor);Require(recording==&frames[slot]);const auto& frame=frames[slot];
    Require(frame.valid&&OwnerBits(frame.entity)==reinterpret_cast<uintptr_t>(actor)&&frame.spawn==actor->Object.InitialSceneFrame&&frame.type==actor->Object.Type&&frame.key==actor->Key);
    Equipment_SetStage(int(stage));float pos[]={float(slot),float(profile)*.25f,float(stage)},light[]={1,.5f,.25f};
    Equipment_AddSprite(1191,pos,1.5f,light,0x1234567,.25f,3);Equipment_EndCharacter();Require(!recording);
   }
   Require(frames[slot].commands.size()==3&&frames[slot].sprites.size()==3&&frames[slot].meshes.empty());
   for(unsigned i=0;i<3;++i){const auto& cmd=frames[slot].commands[i];const auto& sprite=frames[slot].sprites[i];Require(!cmd.mesh&&cmd.index==i&&cmd.stage==int(stages[i])&&sprite.stage==int(stages[i])&&sprite.type==1191&&sprite.scale==1.5f&&sprite.owner==0x1234567&&sprite.mode==3&&sprite.rotation==.25f);Require(sprite.position[0]==float(slot)&&sprite.position[1]==float(profile)*.25f&&sprite.position[2]==float(stages[i])&&sprite.light[0]==1&&sprite.light[1]==.5f&&sprite.light[2]==.25f);}
   Require(DrawFor(actor)==(actor->Object.Live!=0));actor->Object.Live=1;Require(DrawFor(actor));
   ++actor->Object.InitialSceneFrame;Require(!DrawFor(actor));--actor->Object.InitialSceneFrame;
   ++actor->Object.Type;Require(!DrawFor(actor));--actor->Object.Type;
   ++actor->Key;Require(!DrawFor(actor));--actor->Key;
   ++SceneFlag;Require(!DrawFor(actor));--SceneFlag;++gMapManager.current;Require(!DrawFor(actor));--gMapManager.current;
   actor->Object.Live=before[0];Require(std::memcmp(actor,before,916)==0);
   Hash(hash,unsigned(frames[slot].spawn));Hash(hash,unsigned(frames[slot].type));Hash(hash,unsigned(frames[slot].key));Hash(hash,unsigned(DrawFor(actor)));++count;
  }
  Equipment_BeginStep();
  for(unsigned slot=0;slot<401;++slot){Require(!frames[slot].valid&&frames[slot].commands.empty()&&frames[slot].sprites.empty()&&previousFrames[slot].valid&&previousFrames[slot].commands.size()==3);Equipment_Invalidate(slot==400?DAT_07abf050:DAT_07abf5d0+slot*916);Require(!previousFrames[slot].valid);}
  const CHARACTER* invalid[]={nullptr,reinterpret_cast<const CHARACTER*>(DAT_07abf5d0-1),reinterpret_cast<const CHARACTER*>(DAT_07abf5d0+1),reinterpret_cast<const CHARACTER*>(DAT_07abf5d0+400*916)};
  for(auto* actor:invalid){BeginFor(actor);Require(!recording&&!DrawFor(actor));Equipment_Invalidate(actor);}
 }
 Require(count==3208);return hash;
}
BYTE* Pool(uintptr_t base){for(unsigned i=0;i<6;++i)Require(Reserve(base+i*0x10000)==reinterpret_cast<void*>(base+i*0x10000));return reinterpret_cast<BYTE*>(base);}
int main(){
 auto* low=Pool(0x26000000u);auto* preview=static_cast<BYTE*>(Reserve(0x2a000000u));Require(preview);const auto expected=Run(low,preview);
 if(sizeof(void*)>4){
  auto* high=Pool(uintptr_t(0x126000000ull));auto* highPreview=static_cast<BYTE*>(Reserve(uintptr_t(0x12a000000ull)));Require(highPreview);
  // Una tabla reubicada no puede heredar la identidad de su alias32 anterior.
  SceneFlag=5;gMapManager.current=2;Equipment_BeginStep();
  auto* oldActor=reinterpret_cast<CHARACTER*>(low+16);oldActor->Object.Live=1;oldActor->Object.Type=390;oldActor->Object.InitialSceneFrame=77;oldActor->Key=65535;
  BeginFor(oldActor);Equipment_EndCharacter();Require(DrawFor(oldActor));
  auto* newActor=new(high+16) CHARACTER(*oldActor);DAT_07abf5d0=high+16;Require(!DrawFor(newActor));
  auto* oldPreview=reinterpret_cast<CHARACTER*>(preview+16);oldPreview->Object.Live=1;
  BeginFor(oldPreview);Equipment_EndCharacter();Require(DrawFor(oldPreview));
  auto* newPreview=new(highPreview+16) CHARACTER(*oldPreview);DAT_07abf050=highPreview+16;Require(!DrawFor(newPreview));
  std::memset(low,0x7b,bytes);std::memset(preview,0x6d,948);Require(Run(high,highPreview)==expected);
  for(unsigned i=0;i<bytes;++i)Require(low[i]==0x7b);for(unsigned i=0;i<948;++i)Require(preview[i]==0x6d);
  auto* sign=Pool(0x8e000000u);auto* signPreview=static_cast<BYTE*>(Reserve(0x92000000u));Require(signPreview&&Run(sign,signPreview)==expected);
  std::puts("caché nativa:pool ypreview altos,bit31,aliases intactos");
 }
 std::printf("Equipment owner:3208 registros/ruta,9624 etapas yguardas %016llx\n",(unsigned long long)expected);
}
'''
def struct(source,name):
    return re.search(r'^struct '+name+r'\s*\{[\s\S]*?^\};',source,re.M).group()
def harness(revision=None):
    source=read_source('src/Render/EquipmentVisuals.cpp',revision);header=read_source('src/Entity/EntityTypes.h',revision)
    draw=function(source,'Equipment_Draw');brace=draw.index('{');end=draw.index('char savedBones')
    draw=draw[:brace].replace('void Equipment_Draw','bool CheckDraw')+'{'+draw[brace+1:end].replace('return;','return false;')+'return true;}'
    native='Equipment_BeginCharacter(const CHARACTER*' in source
    actor='owner' if native else 'static_cast<int>(reinterpret_cast<uintptr_t>(owner))'
    funcs='\n'.join(function(source,n) for n in ['Equipment_BeginStep','Equipment_BeginCharacter','Equipment_EndCharacter','Equipment_SetStage','Equipment_Invalidate','Equipment_AddSprite'])
    globals=source[source.index('EquipmentFrame frames['):source.index('int Slot(')]
    spawn=re.search(r'MU_ASSERT_FIELD\(OBJECT, InitialSceneFrame, ([^)]+)\)',header).group(1)
    accessors='\n'.join(re.findall(r'^inline (?:const )?(?:CHARACTER|OBJECT)\* As(?:Character|Object)\([^\n]+',header,re.M))
    services=SERVICES.replace('SPAWN_OFFSET',spawn).replace('ACCESSORS',accessors).replace('SPRITE',struct(source,'Sprite')).replace('COMMAND',re.search(r'^struct Command[^\n]+',source,re.M).group()).replace('FRAME',struct(source,'EquipmentFrame')).replace('GLOBALS',globals).replace('SLOT',function(source,'Slot')).replace('FUNCTIONS',funcs).replace('DRAW_GUARD',draw).replace('BEGIN_CALL','Equipment_BeginCharacter('+actor+')').replace('DRAW_CALL','CheckDraw('+actor+',0)')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-equipment-owner-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
