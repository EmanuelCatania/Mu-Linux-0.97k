#!/usr/bin/env python3
"""Entrada de quest real: geometría/respuestas/botones/requisitos/cierre/envío.

Dispositivo de ventana, red, audio y presentación de texto controlados.
No certifica dibujo/socket/audio real, formato inválido ni layout CSQuest x64.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#ifndef _WIN32
#define __fastcall
#endif
using uint=unsigned int; using ushort=unsigned short; using std::uintptr_t;
BYTE* activeQuest;
int margin; unsigned coverage;
std::vector<int> events;
struct Window { int GetRightShift() { return margin; } } gWindow;
void CloseInventoryRelatedWindows() { Require(activeQuest[0x1c87f]==0); events.push_back(-1); }
struct Network {
    void Send(const BYTE* packet,int size) {
        Require(size==5 && packet[0]==0xc1 && packet[1]==5 && packet[2]==0xa2 && packet[3]==activeQuest[0x1c87a] && packet[4]==1);
        events.push_back(1000+packet[3]); coverage|=1;
    }
    void SendC1(const BYTE* packet,int size) {
        Require(size==3 && packet[0]==0xc1 && packet[1]==3 && packet[2]==0x31 && !events.empty() && events.back()==-1);
        events.push_back(2000); coverage|=2;
    }
} gNetwork;
int PlayBuffer(int id,const void* owner,int flags) { Require(id==28 && owner==nullptr && flags==0); events.push_back(3028); coverage|=8; return 0; }
void __fastcall CSQuest_ShowDialogText(int id) {
    events.push_back(4000+id); coverage|=4;
    if(id>=0 && id<DIALOG_SCRIPT_COUNT) g_iCurrentDialogScript=id;
}
'''

MAIN = r'''
template<class T> void Put(BYTE* p,T value) { std::memcpy(p,&value,sizeof(value)); }
std::uint64_t RunInput(BYTE* memory,BYTE* attributes,BYTE* machine) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; activeQuest=quest; DAT_07cf1ff4=attributes; DAT_07cf1ffc=machine;
    std::uint64_t digest=14695981039346656037ull; coverage=0;
    const int margins[]={-120,0,107,320}, scripts[]={-1,0,199,200}, slotTypes[]={-1,0,1,2,3,2147483647}, links[]={-1,0,1,199,200,2147483647};
    const int xEdges[]={-1,0,24,25,34,35,106,144,155,189,190,212,300}, yEdges[]={-1,0,249,250,354,355,378,379,394,395,418,419,432,433}, rowEdges[]={-1,0,17,18};
    const BYTE states[]={0,1,2,3,5,255}; const short limits[]={-1,0,1,100,32767}; const ushort levels[]={0,100,65535};
    const int amounts[]={0,1,100,-1,2147483647,-2147483647-1};
    std::memset(DAT_07cf5608,0,DIALOG_SCRIPT_COUNT*0x400);
    for(unsigned trial=0;trial<32768;++trial) {
        std::uint32_t seed=trial^0x970401afu;
        auto next=[&]() { seed=seed*1664525u+1013904223u; return seed>>16; };
        std::memset(memory,0x5a,size); std::memset(attributes,0x33,64); std::memset(machine,0xcc,1412);
        const unsigned index=next()%200; quest[4]=static_cast<BYTE>(next()%4); quest[5]=0;
        quest[0x1c87a]=static_cast<BYTE>(index); quest[0x1c87f]=static_cast<BYTE>(next()%2); quest[0x1c882]=states[next()%6];
        BYTE* record=quest+8+index*584; std::memset(record,0,584); Put(record,static_cast<short>(1)); Put(record+2,static_cast<short>(1));
        record[43]=7; record[44+quest[4]]=1; record[329]=7;
        Put(record+332,limits[next()%5]); Put(record+334,limits[next()%5]); Put(record+336,amounts[next()%6]);
        const short textIds[]={-32768,-1,0,1,199,200,32767}; Put(record+340,textIds[next()%7]);
        Put(attributes+14,levels[next()%3]); Put(machine+0x548,amounts[next()%6]);
        margin=margins[trial%4]; g_iCurrentDialogScript=scripts[next()%4];
        DAT_083a7c0c=next()%11; DAT_083a4324=next()%8;
        const int origin=450+margin;
        const int yBase=quest[0x1c882]!=1 && quest[0x1c87f]==1?250:((7-(static_cast<int>(DAT_083a7c0c)+static_cast<int>(DAT_083a4324)))*18)/2+66+static_cast<int>(DAT_083a4324)*18;
        const int x=origin+xEdges[next()%13];
        const int y=trial%2?yBase+(static_cast<int>(next()%13)-2)*18+rowEdges[next()%4]:yEdges[next()%14];
        MouseX=static_cast<DWORD>(x); MouseY=static_cast<DWORD>(y);
        DAT_083a4124=trial%5==0?2:trial%3==0?0:1; DAT_083a42c4=next()%4;
        DAT_07e11d28=next(); DAT_00559bec=next(); DAT_07d78094=next()%2; DAT_083a413c=next()%4;
        for(int script : {0,199}) {
            std::memset(&g_DialogScript[script],0,sizeof(DIALOG_SCRIPT));
            for(int slot=0;slot<10;++slot) {
                g_DialogScript[script].m_iReturnForAnswer[slot]=slotTypes[next()%6];
                g_DialogScript[script].m_iLinkForAnswer[slot]=links[next()%6];
            }
        }
        BYTE before[size], allowed[size], beforeAttributes[64], beforeMachine[1412];
        DIALOG_SCRIPT beforeFirst=g_DialogScript[0], beforeLast=g_DialogScript[199];
        std::memcpy(before,memory,size); std::memcpy(allowed,memory,size); std::memcpy(beforeAttributes,attributes,64); std::memcpy(beforeMachine,machine,1412);
        const BYTE oldState=quest[0x1c882]; const int oldScript=g_iCurrentDialogScript;
        events.clear(); const bool proceed=(trial/4)%2!=0;
        if(proceed) Require(CSQuest_ProceedButton(quest)==1); else FUN_00401af0(quest);
        for(int offset : {0x1c87f,0x1c880,0x1c881,0x1c882,0x1c884,0x1c885,0x1c886,0x1c887}) allowed[16+offset]=quest[offset];
        Require(std::memcmp(allowed,memory,size)==0 && std::memcmp(beforeAttributes,attributes,64)==0 && std::memcmp(beforeMachine,machine,1412)==0);
        Require(std::memcmp(&beforeFirst,&g_DialogScript[0],sizeof(DIALOG_SCRIPT))==0 && std::memcmp(&beforeLast,&g_DialogScript[199],sizeof(DIALOG_SCRIPT))==0);
        if(y<0 || x<origin || x>origin+212) Require(events.empty() && std::memcmp(before,memory,size)==0);
        if(events.empty()) coverage|=16; if(oldState!=5 && quest[0x1c882]==5) coverage|=32; if(oldScript!=g_iCurrentDialogScript) coverage|=64;
        for(int event : events) { digest^=static_cast<unsigned>(event); digest*=1099511628211ull; }
        for(int offset=0x1c87a;offset<=0x1c887;++offset) { digest^=quest[offset]; digest*=1099511628211ull; }
        const DWORD values[]={DAT_083a4124,DAT_083a42c4,DAT_07e11d28,DAT_00559bec,DAT_07d78094,DAT_083a413c,static_cast<DWORD>(g_iCurrentDialogScript),DAT_083a7c0c,DAT_083a4324};
        for(DWORD value : values) { digest^=value; digest*=1099511628211ull; }
    }
    if(coverage!=127) std::fprintf(stderr,"cobertura de entrada: %u\n",coverage);
    Require(coverage==127); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* attributes=static_cast<BYTE*>(Reserve(0x74000000u)); auto* machine=static_cast<BYTE*>(Reserve(0x73000000u)); Require(low && attributes && machine);
    const auto expected=RunInput(low,attributes,machine);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highAttributes=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); auto* highMachine=static_cast<BYTE*>(Reserve(std::uint64_t(0x173000000ull))); Require(high && highAttributes && highMachine);
        for(int mask=1;mask<8;++mask) {
            if(mask&1) std::memset(low,0,0x1c890+32); if(mask&2) std::memset(attributes,0,64); if(mask&4) std::memset(machine,0,1412);
            Require(RunInput(mask&1?high:low,mask&2?highAttributes:attributes,mask&4?highMachine:machine)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<64;++i) Require(attributes[i]==0);
            if(mask&4) for(int i=0;i<1412;++i) Require(machine[i]==0);
        }
        std::puts("ocho combinaciones quest/atributos/maquina altas y bajas y direcciones de callbacks completas");
    }
    std::printf("Entrada de quest: 32768 estados, geometria/paquetes/flags/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("char DAT_00559bb4", 1)[0].replace("0x10000", "0x100000")
    prefix += read_source("src/UI/PanelAnchor.h", revision).replace("#pragma once", "")
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    prefix += "\n" + re.search(r"^#define DIALOG_SCRIPT_COUNT[^\n]+", header, re.M).group() + "\n"
    prefix += re.search(r"struct DIALOG_SCRIPT\s*\{.*?\n\};", header, re.S).group() + "\nstatic_assert(sizeof(DIALOG_SCRIPT)==0x400);\n"
    names=("DAT_07cf1ff4","DAT_07cf1ffc","DAT_005615dc","DAT_083a7c0c","DAT_083a4324","DAT_083a4278","DAT_083a427c","DAT_083a4124","DAT_083a42c4","DAT_07e11d28","DAT_00559bec","DAT_07d78094","DAT_083a413c")
    for name in names:
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    prefix += re.search(r"^DWORD\s+DAT_07cf5608\[[^;]+;", definitions, re.M).group()
    aliases=header+read_source("src/structs.h",revision)
    for name in ("g_DialogScript","g_iCurrentDialogScript","MouseX","MouseY"):
        prefix += "\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",aliases,re.M).group()+"\n"
    prefix += SERVICES + function(header,"IsClickPushed")
    scene=read_source("src/Scene/Scene_CharSelect_Nav.cpp",revision)
    prefix += function(scene,"CSQuest_CheckRequestCondition")
    prefix += function(read_source("src/Item/Quest_UIState.cpp",revision),"CSQuest_clearQuest")
    for name in ("Quest_SendState","FUN_00401af0"):
        prefix += function(scene,name)
    prefix += function(read_source("src/Net/Crypto.cpp",revision),"CSQuest_ProceedButton")
    return prefix+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-input-") as temp:
        directory=Path(temp)
        current=build_and_run(directory/"current",harness(),args)
        print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
