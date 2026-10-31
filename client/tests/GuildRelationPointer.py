#!/usr/bin/env python3
"""Relación de guild y recorrido de400 personajes reales; prefijo x86 opaco."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
from LegacyEntityLayout import opaque_cloth_fields

SERVICES=r'''
std::uintptr_t Hero,CharactersClient;
BYTE* markBase;
#define DAT_07e919bc (*reinterpret_cast<char(*)[0x13c30]>(markBase))
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
template<class T>T Get(const BYTE* p){T value;std::memcpy(&value,p,sizeof(value));return value;}
void Update(BYTE* entity){GuildWar_UpdateEntityRelation(ARG,0,0,0);}
int FindExpected(){for(int i=0;i<kGuildMarkRecordCount;++i)if(markBase[i*80] && std::strcmp(s_GuildMarkName[i],GuildWarName)==0)return i;return -1;}
BYTE Expected(const BYTE* entity,const BYTE* hero,int& index){
    if(!entity || !entity[0] || !hero)return entity?entity[745]:0;
    BYTE team=0;const short own=Get<short>(hero+474),guild=Get<short>(entity+474);
    if(own!=-1 && own==guild)team=1;
    if(EnableGuildWar){if(index==-1 && GuildWarName[0])index=FindExpected();if(index>=0 && guild==index)team=2;}
    return team;
}
void SetupWar(unsigned value){
    const int indices[]={-1,0,31,999,-2,556};GuildWarIndex=indices[value%6];EnableGuildWar=(value/6)%3;
    const char* names[]={"G0001","G0000","G0999","DUP","missing","","ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"};std::strcpy(GuildWarName,names[(value/18)%7]);
}
std::uint64_t Run(BYTE* poolMemory,BYTE* heroMemory,BYTE* tableMemory){
    std::uint64_t digest=14695981039346656037ull;markBase=tableMemory+16;
    std::memset(tableMemory,0xa5,0x13c30+32);
    for(int i=0;i<kGuildMarkRecordCount;++i){markBase[i*80]=i%5?1:0;std::snprintf(s_GuildMarkName[i],sizeof(s_GuildMarkName[i]),"G%04d",i);}
    std::strcpy(s_GuildMarkName[31],"DUP");std::strcpy(s_GuildMarkName[33],"DUP");std::strcpy(s_GuildMarkName[556],"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456");
    std::vector<BYTE> beforeTable(tableMemory,tableMemory+0x13c30+32);const auto beforeNames=std::vector<char>(&s_GuildMarkName[0][0],&s_GuildMarkName[0][0]+sizeof(s_GuildMarkName));
    const short guilds[]={-1,0,1,31,999,556,-2};
    for(unsigned trial=0;trial<32768;++trial){
        std::memset(poolMemory,0xa5,948);std::memset(heroMemory,0x5a,948);BYTE* entity=poolMemory+16;BYTE* hero=heroMemory+16;
        entity[0]=(trial%4==0?0:trial%4==1?1:trial%4==2?128:255);Put(entity+474,guilds[(trial/4)%7]);Put(hero+474,guilds[(trial/28)%7]);
        const bool nullEntity=trial%19==0;Hero=trial%23==0?0:reinterpret_cast<std::uintptr_t>(trial%17==0?entity:hero);
        SetupWar(trial/7);int index=GuildWarIndex;const BYTE team=Expected(nullEntity?nullptr:entity,reinterpret_cast<BYTE*>(Hero),index);
        BYTE before[948],beforeHero[948];std::memcpy(before,poolMemory,948);std::memcpy(beforeHero,heroMemory,948);if(!nullEntity)before[16+745]=team;
        Update(nullEntity?nullptr:entity);Require(GuildWarIndex==index && std::memcmp(poolMemory,before,948)==0 && std::memcmp(heroMemory,beforeHero,948)==0);
        digest^=team;digest*=1099511628211ull;digest^=static_cast<unsigned>(index);digest*=1099511628211ull;
    }
    constexpr unsigned poolBytes=400*916+32;
    for(unsigned frame=0;frame<64;++frame){
        std::memset(poolMemory,0xa5,poolBytes);std::memset(heroMemory,0x5a,948);Put(heroMemory+16+474,guilds[frame%7]);Hero=frame%9==0?0:reinterpret_cast<std::uintptr_t>(heroMemory+16);
        for(unsigned slot=0;slot<400;++slot){BYTE* entity=poolMemory+16+slot*916;entity[0]=(slot+frame)%4==0?0:(slot+frame)%4==1?1:(slot+frame)%4==2?128:255;Put(entity+474,guilds[(slot+frame)%7]);}
        SetupWar(frame);int index=GuildWarIndex;std::vector<BYTE> expected(poolMemory,poolMemory+poolBytes);
        for(unsigned slot=0;slot<400;++slot)expected[16+slot*916+745]=Expected(poolMemory+16+slot*916,reinterpret_cast<BYTE*>(Hero),index);
        CharactersClient=reinterpret_cast<std::uintptr_t>(poolMemory+16);GuildWar_RefreshEntityRelations();Require(GuildWarIndex==index && std::memcmp(poolMemory,expected.data(),poolBytes)==0);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}
    }
    CharactersClient=0;GuildWar_RefreshEntityRelations();
    Require(std::memcmp(tableMemory,beforeTable.data(),beforeTable.size())==0 && std::memcmp(&s_GuildMarkName[0][0],beforeNames.data(),beforeNames.size())==0);return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+0x70000,low+0x80000);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,0xa0000);Require(Run(mask&1?high:low,(mask&2?high:low)+0x70000,(mask&4?high:low)+0x80000)==expected);
            if(mask&1)for(unsigned i=0;i<400*916+32;++i)Require(low[i]==0x7b);
            if(mask&2)for(unsigned i=0;i<948;++i)Require(low[0x70000+i]==0x7b);
            if(mask&4)for(unsigned i=0;i<0x13c30+32;++i)Require(low[0x80000+i]==0x7b);
        }std::puts("ocho combinaciones personajes/heroe/marcas altos y bajos; aliases intactos");
    }std::printf("Guild relation: 32768 casos y64 recorridos de400 slots %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    header=read_source('src/Entity/EntityTypes.h',revision);source=read_source('src/Net/Recv/Recv_Guild.cpp',revision)
    part=re.search(r'struct PART_t\s*\{.*?\};',header,re.S).group();part=opaque_cloth_fields(part);character=header[header.index('struct CHARACTER {'):header.index('    BYTE     PK;')]
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')+'\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\nstruct OBJECT{BYTE Live;BYTE opaque[443];};\n'
    prefix+=part+character+'BYTE remainder[170];};\nstatic_assert(sizeof(CHARACTER)==916);static_assert(offsetof(CHARACTER,GuildMarkIndex)==474);static_assert(offsetof(CHARACTER,GuildTeam)==745);\n'
    prefix+='\n'.join(re.findall(r'^inline (?:const )?CHARACTER\* AsCharacter[^\n]+',header,re.M))
    prefix+=re.search(r'constexpr int kGuildMarkRecordCount\s*=\s*[^;]+;',read_source('src/Net/Recv/NetRecv.h',revision)).group()
    size=re.search(r'constexpr std::size_t kGuildNameSize\s*=\s*(\d+)',read_source('src/Net/Protocol/ProtocolBase.h',revision)).group(1)
    prefix+=f'namespace NetText{{constexpr std::size_t GuildNameSize={size};}}\n'
    prefix+=re.search(r'static char s_GuildMarkName[^;]+;',source).group()+SERVICES
    gh,gd=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in ['EnableGuildWar','GuildWarIndex','GuildWarName']:
        prefix+=re.search(rf'extern [^;\n]*\b{name}(?:\[[^\]]*\])?\s*;',gh).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}(?:\[[^\]]*\])?\s*=\s*[^;]+;',gd,re.M).group()
    update=function(source,'GuildWar_UpdateEntityRelation');code=function(source,'GuildMark_Record')+function(source,'GuildMark_FindRecordByName')+update+function(source,'GuildWar_RefreshEntityRelations')
    arg='reinterpret_cast<uintptr_t>(entity)' if 'uintptr_t entityAddress' in update else 'static_cast<int>(reinterpret_cast<uintptr_t>(entity))'
    return prefix+code+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-guild-relation-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
