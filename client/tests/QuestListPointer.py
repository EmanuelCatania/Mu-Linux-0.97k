#!/usr/bin/env python3
"""Listas de quest y receptores A0/A1/A2 reales con cuatro bases nativas.

Red ya recibida en memoria; NetLog/ShowQuestNpcWindow controlados. Conserva
los límites y offsets actuales, sin certificar socket, UI o layout CSQuest.
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
int windows=0;
template<class... Args> void NetLog(Args...) {}
void CSQuest__ShowQuestNpcWindow(void* self,int edx,int index) {
    Require(reinterpret_cast<std::uintptr_t>(self)==static_cast<std::uintptr_t>(g_csQuest) && edx==0 && index==-1); ++windows;
}
'''

MAIN = r'''
void List(BYTE* quest,BYTE* source,int count,int klass) { CSQuest__setQuestLists(LISTARG,0,source,count,klass); }
void Entry(BYTE* quest,int index,int result) { CSQuest__setQuestList(ENTRYARG,0,index,result); }
void ExpectedList(BYTE* quest,const BYTE* source,int count,int klass) {
    if(klass!=-1) { quest[4]=static_cast<BYTE>(klass)&7; quest[5]=static_cast<BYTE>(klass>>3); }
    std::memset(quest+0x1c848,0,50); std::memcpy(quest+0x1c848,source,count);
}
void ExpectedEntry(BYTE* quest,int index,int result) {
    quest[0x1c87a]=static_cast<BYTE>(index); quest[0x1c848+((index&255)>>2)]=static_cast<BYTE>(result);
}
void Hash(std::uint64_t& digest,const BYTE* bytes,std::size_t size) {
    for(std::size_t i=0;i<size;++i) { digest^=bytes[i]; digest*=1099511628211ull; }
}
std::uint64_t Run(BYTE* questMemory,BYTE* packetMemory,BYTE* heroMemory,BYTE* statsMemory) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=questMemory+16; BYTE* packet=packetMemory+16;
    std::uint64_t digest=14695981039346656037ull;
    const int classes[]={-1,0,1,7,8,15,255,-9,0x7fffffff};
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(questMemory,0x5a,size); std::memset(packetMemory,0xa5,96);
        for(int i=0;i<64;++i) packet[i]=static_cast<BYTE>(trial+i*73);
        BYTE expected[size], beforePacket[96]; std::memcpy(expected,questMemory,size); std::memcpy(beforePacket,packetMemory,96);
        const int count=trial%51, klass=classes[trial%9];
        ExpectedList(expected+16,packet,count,klass); List(quest,packet,count,klass);
        const int index=static_cast<int>(trial%3==0?0x80000000u|trial:trial%3==1?0x7fffff00u|trial:trial);
        ExpectedEntry(expected+16,index,static_cast<int>(trial*40503)); Entry(quest,index,static_cast<int>(trial*40503));
        Require(std::memcmp(expected,questMemory,size)==0 && std::memcmp(beforePacket,packetMemory,96)==0);
        Hash(digest,quest+4,2); Hash(digest,quest+0x1c848,72);
    }
    for(unsigned trial=0;trial<4096;++trial) {
        std::memset(questMemory,0x5a,size); std::memset(packetMemory,0xa5,96); std::memset(heroMemory,0x33,512); std::memset(statsMemory,0xcc,64);
        for(int i=0;i<64;++i) packet[i]=static_cast<BYTE>(trial*73+i*40503);
        heroMemory[444]=static_cast<BYTE>(trial); statsMemory[11]=static_cast<BYTE>(trial*73);
        Hero=trial%3==0?reinterpret_cast<char*>(heroMemory):nullptr;
        DAT_07cf1ff4=trial%3==1?statsMemory:nullptr;
        g_csQuest=trial%7==0?0:reinterpret_cast<std::uintptr_t>(quest);
        const int route=trial%3, length=static_cast<int>((trial/3)%67)-1;
        if(route==0) packet[3]=static_cast<BYTE>(trial); if(route==2) packet[4]=trial%2==0?0:1;
        BYTE expected[size], beforePacket[96], beforeHero[512], beforeStats[64];
        std::memcpy(expected,questMemory,size); std::memcpy(beforePacket,packetMemory,96); std::memcpy(beforeHero,heroMemory,512); std::memcpy(beforeStats,statsMemory,64);
        int expectedWindows=0;
        if(g_csQuest) {
            if(route==0 && length>=4) {
                int n=packet[3]; if(n>50) n=50; if(length<4+n) n=length-4;
                if(n>0) ExpectedList(expected+16,packet+4,n,Hero?heroMemory[444]:DAT_07cf1ff4?statsMemory[11]:-1);
            } else if(route==1 && length>=5) { ExpectedEntry(expected+16,packet[3],packet[4]); expectedWindows=1; }
            else if(route==2 && length>=6 && packet[4]==0) { ExpectedEntry(expected+16,packet[3],packet[5]); expectedWindows=1; }
        }
        windows=0;
        if(route==0) NetRecv_A0(packet,length,0xc1,0,trial&1);
        if(route==1) NetRecv_A1(packet,length,0xc1,0,trial&1);
        if(route==2) NetRecv_A2(packet,length,0xc1,0,trial&1);
        Require(windows==expectedWindows && std::memcmp(expected,questMemory,size)==0 && std::memcmp(beforePacket,packetMemory,96)==0 && std::memcmp(beforeHero,heroMemory,512)==0 && std::memcmp(beforeStats,statsMemory,64)==0);
        Hash(digest,quest+4,2); Hash(digest,quest+0x1c848,72); digest^=static_cast<unsigned>(windows); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* quest=static_cast<BYTE*>(Reserve(0x76000000u)); auto* packet=static_cast<BYTE*>(Reserve(0x75000000u));
    auto* hero=static_cast<BYTE*>(Reserve(0x74000000u)); auto* stats=static_cast<BYTE*>(Reserve(0x73000000u)); Require(quest && packet && hero && stats);
    const auto expected=Run(quest,packet,hero,stats);
    if(sizeof(void*)>4) {
        auto* highQuest=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highPacket=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull)));
        auto* highHero=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); auto* highStats=static_cast<BYTE*>(Reserve(std::uint64_t(0x173000000ull))); Require(highQuest && highPacket && highHero && highStats);
        for(int mask=1;mask<16;++mask) {
            if(mask&1) std::memset(quest,0,0x1c890+32); if(mask&2) std::memset(packet,0,96);
            if(mask&4) std::memset(hero,0,512); if(mask&8) std::memset(stats,0,64);
            Require(Run(mask&1?highQuest:quest,mask&2?highPacket:packet,mask&4?highHero:hero,mask&8?highStats:stats)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(quest[i]==0);
            if(mask&2) for(int i=0;i<96;++i) Require(packet[i]==0);
            if(mask&4) for(int i=0;i<512;++i) Require(hero[i]==0);
            if(mask&8) for(int i=0;i<64;++i) Require(stats[i]==0);
        }
        std::puts("16 combinaciones quest/paquete/Hero/atributos altos y bajos, aliases intactos");
    }
    std::printf("Listas y A0/A1/A2: 8192 estados, bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    prefix += "\nusing std::uintptr_t;\n"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("g_csQuest", "Hero", "DAT_07cf1ff4"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    for name in ("DAT_07abf5d8", "CharacterAttribute"):
        prefix += "\n" + re.search(rf"^#define\s+{name}\s+[^\n]+", header + read_source("src/structs.h", revision), re.M).group() + "\n"
    prefix += SERVICES
    lists = function(read_source("src/Core/Runtime_Medium.cpp", revision), "CSQuest__setQuestLists")
    entry = function(read_source("src/Core/Runtime_Small.cpp", revision), "CSQuest__setQuestList")
    prefix += lists + entry
    source = read_source("src/Net/Recv/Recv_Quest.cpp", revision)
    for name in ("NetRecv_A0", "NetRecv_A1", "NetRecv_A2"):
        prefix += function(source, name)
    legacy = "static_cast<int>(reinterpret_cast<std::uintptr_t>(quest))"
    return prefix + MAIN.replace("LISTARG", "quest" if "BYTE* ecx" in lists else legacy).replace("ENTRYARG", "quest" if "BYTE* ecx" in entry else legacy)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-list-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
