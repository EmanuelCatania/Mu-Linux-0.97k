#!/usr/bin/env python3
"""Regresión x86: identidad del héroe tras el viewport de vuelta del anillo.

Ejecuta la rama local real y EntityName real. Recreación, equipo y buffs son
servicios controlados; no reemplaza una sesión con servidor.
"""
import argparse,re,tempfile
from pathlib import Path
from SceneObjectPointer import read_source,build_and_run

def harness(negative=False):
    header=read_source('src/Entity/EntityTypes.h')
    records='\n'.join(re.search(r'struct '+n+r'\s*\{.*?\n\};',header,re.S).group() for n in ('OBJECT','PART_t','CHARACTER'))
    source=read_source('src/Net/Recv/Recv_Viewport.cpp')
    start=source.index('            if (hero) {',source.index('if (entityId == g_HeroKey'))
    end=source.index('            NetLog("NET:    0x12 own HeroKey=',start)
    branch=source[start:end]
    if negative:
        assert branch.count('EntityName::Set(hero, name);')==1
        branch=branch.replace('EntityName::Set(hero, name);','')
    names=read_source('src/Entity/EntityName.cpp');names=names[names.index('namespace EntityName {'):]
    assert 'EntityName::Clear(param_1);' in read_source('src/Entity/Entity_Spawn.cpp')
    prefix=r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;
using vec3_t=float[3];using vec34_t=float[3][4];
void Require(bool v){if(!v){std::fputs("Identidad perdida al volver del anillo\n",stderr);std::abort();}}
#pragma pack(push,1)
'''+records+r'''
#pragma pack(pop)
static_assert(sizeof(CHARACTER)==916);
CHARACTER pool[400];uintptr_t CharactersClient=reinterpret_cast<uintptr_t>(pool);
CHARACTER* AsCharacter(void* p){return static_cast<CHARACTER*>(p);}
namespace TextEngine{void CopyTruncated(char* out,std::size_t cap,std::string_view value){auto n=value.size()<cap-1?value.size():cap-1;std::memcpy(out,value.data(),n);out[n]=0;}}
namespace EntityName{constexpr std::size_t Capacity=96;}
'''+names+r'''
unsigned recreated,equipped,buffed;WORD g_HeroKey=91;BYTE attributes[32];uintptr_t CharacterAttribute=reinterpret_cast<uintptr_t>(attributes);int HeroIndex;
void CreateCharacterPointer(BYTE* p,int type,int,int,float){++recreated;EntityName::Clear(p);AsCharacter(p)->Object.Type=short(type);}
void SetCharacterClass(CHARACTER*){}
void NetLog(const char*,...){}
void ChangeCharacterExt(int index,BYTE*){Require(index==HeroIndex);++equipped;}
void Hero_ApplyCatalogEquipment(CHARACTER*){}
void ApplyPersistentSkillEffect97k(BYTE*,WORD,int){++buffed;}
void Apply(BYTE* hero,const char* name){BYTE x=11,y=12,tx=13,ty=14,dir=3,dirpk=0x32;WORD viewSkillState=0;BYTE payload[62]{};const BYTE* e=payload;
'''+branch+r'''
}
int main(){
 const char* names[]={"NewEvoTest","MagoTest","NombreDePersonajeConCuarentaBytes123456789"};
 for(unsigned trial=0;trial<2400;++trial){
  HeroIndex=int(trial%400);auto* c=pool+HeroIndex;auto* other=pool+(HeroIndex+1)%400;
  EntityName::Set(c,"NombreViejo");EntityName::Set(other,"OtroPersonaje");
  c->Object.Type=trial%3?2:390;unsigned old=recreated;const char* name=names[trial%3];
  Apply(reinterpret_cast<BYTE*>(c),name);
  Require(recreated==old+unsigned(trial%3!=0));
  Require(std::strcmp(EntityName::Get(c),name)==0);
  // Contrato CGChatRecv: el nombre que autentica /move coincide con el conectado.
  Require(std::strcmp(EntityName::Get(other),"OtroPersonaje")==0);
  Require(c->Object.Type==390&&c->PositionX==11&&c->TargetY==14);
 }
 Require(equipped==2400&&buffed==2400);Apply(nullptr,"SinHeroe");
 std::puts("RingHeroIdentity: 2400 refrescos/reversiones; nombre completo y otros slots intactos OK");
}
'''
    return prefix

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=['Win32'],default='Win32');p.add_argument('--generator');p.add_argument('--negative',action='store_true');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-ring-identity-') as t:print(build_and_run(Path(t)/'current',harness(a.negative),a),end='')
if __name__=='__main__':main()
