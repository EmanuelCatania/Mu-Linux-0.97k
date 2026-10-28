#!/usr/bin/env python3
"""Condición real de selección por guild con Hero nativo y lector real.

AsCharacter es una proyección controlada. No ejecuta MoveCharacter completo
ni certifica layout CHARACTER,PKKey,paquetes,combate o carga de nombres.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
char* Hero;int EnableGuildWar;DWORD selSlot=7;
struct Selected{BYTE PK;short GuildMarkIndex;}selected;
unsigned queries,nameCalls;int names[2];
Selected* AsCharacter(DWORD key){Require(key==selSlot);++queries;return &selected;}
CONSTANTS
char DAT_07e919bc[0x13C30];char s_GuildMarkName[kGuildMarkRecordCount][guildNameSize+1];
RECORD
GET_NAME
const char* Guild_GetMarkName(int row){Require(nameCalls<2);names[nameCalls++]=row;return ActualName(row);}
bool Guard(){return (CONDITION);}
'''
MAIN=r'''
int NameCode(int row){return row>=0 && row<kGuildMarkRecordCount && row%3!=0?1+row%7:0;}
std::uint64_t Run(BYTE* memory){
 for(unsigned i=0;i<sizeof(DAT_07e919bc);++i)DAT_07e919bc[i]=static_cast<char>(i*37+5);
 for(int row=0;row<kGuildMarkRecordCount;++row){DAT_07e919bc[row*80]=row%3==0?0:1;std::snprintf(s_GuildMarkName[row],guildNameSize+1,"g%03d",row%7);}
 const std::vector<char> recordsBefore(DAT_07e919bc,DAT_07e919bc+sizeof(DAT_07e919bc));std::vector<BYTE> namesBefore(sizeof(s_GuildMarkName));std::memcpy(namesBefore.data(),s_GuildMarkName,sizeof(s_GuildMarkName));
 const int wars[]={0,1,-1,static_cast<int>(0x80000000u),0x7fffffff};const BYTE pks[]={0,5,6,7,127,128,255};const short guilds[]={-32768,-1,0,1,2,3,6,7,999,1000,32767};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned profile=0;profile<32;++profile){
  for(unsigned i=0;i<512;++i)memory[i]=static_cast<BYTE>(profile+i*13);
  char* hero=reinterpret_cast<char*>(memory+16+(profile%4)*2);Hero=profile&16?nullptr:hero;EnableGuildWar=wars[profile%5];selected.PK=pks[profile%7];selected.GuildMarkIndex=guilds[profile%11];
  BYTE expected[512];std::memcpy(expected,memory,512);
  for(unsigned bits=0;bits<65536;++bits){
   const short guild=bits<32768?static_cast<short>(bits):static_cast<short>(int(bits)-65536);
   std::memcpy(hero+474,&guild,2);std::memcpy(expected+(reinterpret_cast<BYTE*>(hero)-memory)+474,&guild,2);queries=nameCalls=0;
   const bool reachesNames=EnableGuildWar!=0 && selected.PK>=6 && selected.GuildMarkIndex!=-1 && Hero!=nullptr;
   const bool wanted=reachesNames && NameCode(guild)==NameCode(selected.GuildMarkIndex);
   const unsigned wantedQueries=EnableGuildWar==0?0:selected.PK<6?1:selected.GuildMarkIndex==-1||!Hero?2:3;
   Require(Guard()==wanted && queries==wantedQueries && nameCalls==(reachesNames?2u:0u));
   // El orden de evaluación de los dos argumentos de strcmp no se presupone.
   if(reachesNames)Require((names[0]==guild && names[1]==selected.GuildMarkIndex)||(names[1]==guild && names[0]==selected.GuildMarkIndex));
   Require(Hero==(profile&16?nullptr:hero) && EnableGuildWar==wars[profile%5] && selected.PK==pks[profile%7] && selected.GuildMarkIndex==guilds[profile%11] && selSlot==7 && std::memcmp(memory,expected,512)==0);
   if(!EnableGuildWar)coverage|=1;else if(selected.PK<6)coverage|=2;else if(selected.GuildMarkIndex==-1)coverage|=4;else if(!Hero)coverage|=8;else{coverage|=wanted?16u:32u;coverage|=guild<0?64u:128u;}
   hash^=unsigned(wanted)+4*queries+32*nameCalls;hash*=1099511628211ull;if(reachesNames){hash^=bits;hash*=1099511628211ull;}++cases;
  }
 }
 Require(cases==2097152 && coverage==255 && std::memcmp(recordsBefore.data(),DAT_07e919bc,sizeof(DAT_07e919bc))==0 && std::memcmp(namesBefore.data(),s_GuildMarkName,sizeof(s_GuildMarkName))==0);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,512);Require(Run(high)==expected);for(unsigned i=0;i<512;++i)Require(low[i]==0x7b);std::puts("Hero alto;indices signed16,nombres reales,guardas y alias intactos");}
 std::printf("Guild target hero: 2097152 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def condition(source):
    body=function(source,'MoveCharacter');found=[]
    for start in re.finditer(r'\bif\s*\(',body):
        first=start.end();end=first;depth=1
        while depth:depth+=(body[end]=='(')-(body[end]==')');end+=1
        expression=body[first:end-1]
        if 'Guild_GetMarkName' in expression and '474' in expression:found.append(expression)
    if len(found)!=1:raise ValueError('Condición de guild inesperada')
    return found[0]

def harness(revision=None):
    source=read_source('src/Net/SecondPassword.cpp',revision);guild=read_source('src/Net/Recv/Recv_Guild.cpp',revision)
    header=read_source('src/Net/Recv/NetRecv.h',revision);protocol=read_source('src/Net/Protocol/ProtocolBase.h',revision)
    count=re.search(r'constexpr int kGuildMarkRecordCount\s*=\s*\d+;',header).group(0)
    width=re.search(r'constexpr std::size_t kGuildNameSize\s*=\s*(\d+);',protocol)[1]
    # Sólo la extracción normaliza el tipo del retorno antes de restaurarlo.
    name=function(guild.replace('const char* Guild_GetMarkName','BYTE* Guild_GetMarkName'),'Guild_GetMarkName').replace('BYTE* Guild_GetMarkName','const char* ActualName')
    services=SERVICES.replace('CONSTANTS',count+f'\nconstexpr unsigned guildNameSize={width};').replace('RECORD',function(guild,'GuildMark_Record')).replace('GET_NAME',name).replace('CONDITION',condition(source))
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+services+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-guild-target-hero-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
