#!/usr/bin/env python3
"""Cadena real A0/A1/A2 a diálogo: listas/estado/condiciones/contexto nativos.

Inventario y presentación/cierre de UI controlados. No prueba socket, dibujo,
constructor/vtable, recursión de fallback ni entradas fuera de dominio.
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
using ULONG_PTR=std::uintptr_t;
#endif
using uint=unsigned int; using ushort=unsigned short; using std::uintptr_t;
BYTE* activeQuest;
int stage, inventoryCalls, missing;
short expectedDialog;
template<class... Args> void NetLog(Args...) {}
void CloseInventoryRelatedWindows() { Require(stage==0); stage=1; }
int CSQuest_FindQuestItemsInInven(int type,int count,uint level) {
    Require(stage==1 && inventoryCalls++==0 && type==activeQuest[48]*32+activeQuest[49] && count==activeQuest[50] && level==0xffffffffu);
    return missing;
}
void CSQuest__ShowDialogText(int index,int param) {
    Require(stage==1 && index==expectedDialog && param==0 && activeQuest[0x1c87f]==1);
    Require(GoldenArcherOpenType==0 && InventoryOpened==0 && CharacterOpened==0); stage=2;
}
void __fastcall CSQuest_CheckQuestState(void*,char);
'''

MAIN = r'''
template<class T> T Read(const BYTE* p) { T value; std::memcpy(&value,p,sizeof(value)); return value; }
template<class T> void Put(BYTE* p,T value) { std::memcpy(p,&value,sizeof(value)); }
std::uint64_t RunFlow(BYTE* memory,BYTE* packetMemory,BYTE* heroMemory,BYTE* attributesMemory,BYTE* machineMemory) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; BYTE* record=quest+8; BYTE* packet=packetMemory+16;
    BYTE* hero=heroMemory+16; BYTE* attributes=attributesMemory+16; BYTE* machine=machineMemory+16;
    g_csQuest=reinterpret_cast<std::uintptr_t>(quest); activeQuest=quest;
    std::uint64_t digest=14695981039346656037ull;
    const short limits[]={-1,0,1,100,32767}; const ushort levels[]={0,100,65535};
    const int moneyValues[]={0,1,100,-1,2147483647,-2147483647-1};
    for(unsigned trial=0;trial<2048;++trial) {
        std::memset(memory,0x5a,size); std::memset(packetMemory,0xa5,96); std::memset(heroMemory,0x33,544); std::memset(attributesMemory,0xcc,96); std::memset(machineMemory,0xee,1444);
        std::memset(record,0,584); Put(record,static_cast<short>(1)); Put(record+2,static_cast<short>(1));
        record[39]=1; record[40]=static_cast<BYTE>(trial%32); record[41]=static_cast<BYTE>(trial); record[42]=static_cast<BYTE>(trial%5+1); record[43]=7;
        for(int klass=0;klass<4;++klass) record[44+klass]=1;
        for(int column=0;column<4;++column) Put(record+48+column*2,static_cast<short>(trial*73+column));
        record[329]=7; Put(record+332,limits[trial%5]); Put(record+334,limits[(trial/5)%5]);
        Put(record+336,moneyValues[(trial/25)%6]); Put(record+340,static_cast<short>(trial*40503u));
        Put(attributes+14,levels[(trial/3)%3]); Put(machine+0x548,moneyValues[trial%6]);
        hero[444]=static_cast<BYTE>(trial%4); attributes[11]=static_cast<BYTE>((trial*73)%4);
        Hero=trial%2?reinterpret_cast<char*>(hero):nullptr; DAT_07cf1ff4=attributes; DAT_07cf1ffc=machine;
        GoldenArcherOpenType=trial*40503u; InventoryOpened=static_cast<BYTE>(trial); CharacterOpened=static_cast<BYTE>(trial*73);
        const DWORD oldEvent=GoldenArcherOpenType; const auto oldInventory=InventoryOpened,oldCharacter=CharacterOpened;
        stage=0; inventoryCalls=0; missing=(trial/4)%2;
        BYTE expected[size], beforeHero[544], beforeAttributes[96], beforeMachine[1444], beforePacket[96];
        std::memcpy(expected,memory,size); std::memcpy(beforeHero,heroMemory,544); std::memcpy(beforeAttributes,attributesMemory,96); std::memcpy(beforeMachine,machineMemory,1444);
        for(int i=0;i<54;++i) packet[i]=static_cast<BYTE>(trial+i*73); packet[3]=50;
        expected[16+4]=Hero?hero[444]:attributes[11]; expected[16+5]=0;
        std::memcpy(expected+16+0x1c848,packet+4,50); std::memcpy(beforePacket,packetMemory,96);
        NetRecv_A0(packet,54,0xc1,0,trial&1);
        Require(stage==0 && inventoryCalls==0 && std::memcmp(expected,memory,size)==0 && std::memcmp(beforePacket,packetMemory,96)==0);
        const bool a2=(trial/4)%2!=0, error=a2 && trial%5==0;
        const BYTE state=static_cast<BYTE>(trial%4);
        packet[0]=0xc1; packet[1]=6; packet[2]=a2?0xa2:0xa1; packet[3]=0; packet[4]=a2?static_cast<BYTE>(error):state; packet[5]=state;
        std::memcpy(beforePacket,packetMemory,96);
        if(!error) {
            expected[16+0x1c87a]=0; expected[16+0x1c848]=state; expected[16+0x1c87f]=1; expected[16+0x1c882]=state;
            if(state==1) Put(expected+16+0x1c880,Read<short>(record+48+(missing?1:2)*2));
            if(state==2) Put(expected+16+0x1c880,Read<short>(record+54));
            if(state==3) {
                const int minimum=Read<short>(record+332),maximum=Read<short>(record+334),level=Read<ushort>(attributes+14);
                if((minimum!=0 && level<minimum) || (maximum!=0 && maximum<level)) {
                    Put(expected+16+0x1c880,Read<short>(record+340)); expected[16+0x1c882]=5;
                } else {
                    const int amount=Read<int>(record+336); if(amount) Put(expected+16+0x1c884,amount);
                    Put(expected+16+0x1c880,Read<short>(record+48));
                }
            }
        }
        expectedDialog=Read<short>(expected+16+0x1c880);
        if(a2) NetRecv_A2(packet,6,0xc1,0,trial&1); else NetRecv_A1(packet,6,0xc1,0,trial&1);
        Require(stage==(error?0:2) && inventoryCalls==(!error && state==1?1:0));
        Require(std::memcmp(expected,memory,size)==0 && std::memcmp(beforePacket,packetMemory,96)==0 && std::memcmp(beforeHero,heroMemory,544)==0 && std::memcmp(beforeAttributes,attributesMemory,96)==0 && std::memcmp(beforeMachine,machineMemory,1444)==0);
        Require(GoldenArcherOpenType==(error?oldEvent:0) && InventoryOpened==(error?oldInventory:0) && CharacterOpened==(error?oldCharacter:0));
        for(int i=0;i<72;++i) { digest^=quest[0x1c848+i]; digest*=1099511628211ull; }
        digest^=static_cast<unsigned>(stage+inventoryCalls*4); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    BYTE* low[5]; BYTE* high[5]; const std::size_t sizes[]={0x1c890+32,96,544,96,1444};
    for(int i=0;i<5;++i) { low[i]=static_cast<BYTE*>(Reserve(0x76000000u-i*0x1000000u)); Require(low[i]); }
    const auto expected=RunFlow(low[0],low[1],low[2],low[3],low[4]);
    if(sizeof(void*)>4) {
        for(int i=0;i<5;++i) { high[i]=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)-i*0x1000000u)); Require(high[i]); }
        for(int mask=1;mask<32;++mask) {
            BYTE* selected[5];
            for(int i=0;i<5;++i) { selected[i]=(mask&(1<<i))?high[i]:low[i]; if(mask&(1<<i)) std::memset(low[i],0,sizes[i]); }
            Require(RunFlow(selected[0],selected[1],selected[2],selected[3],selected[4])==expected);
            for(int i=0;i<5;++i) if(mask&(1<<i)) for(std::size_t j=0;j<sizes[i];++j) Require(low[i][j]==0);
        }
        std::puts("32 combinaciones quest/paquete/Hero/atributos/maquina altas y bajas, aliases intactos");
    }
    std::printf("Flujo A0/A1/A2-dialogo: 2048 secuencias, helpers reales y bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    prefix += "\nusing std::uintptr_t;\n"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("g_csQuest", "Hero", "DAT_07cf1ff4", "DAT_07cf1ffc", "GoldenArcherOpenType", "DAT_07eaa117", "DAT_07eaa116"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    aliases = header + read_source("src/structs.h", revision)
    for name in ("DAT_07abf5d8", "CharacterAttribute", "_g_bEventChipDialogEnable", "InventoryOpened", "CharacterOpened"):
        prefix += "\n" + re.search(rf"^#define\s+{name}\s+[^\n]+", aliases, re.M).group() + "\n"
    prefix += SERVICES
    scene = read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision)
    for name in ("CSQuest_GetQuestState", "CSQuest_FindQuestContext", "CSQuest_CheckActCondition", "CSQuest_CheckRequestCondition", "CSQuest_CheckQuestState"):
        prefix += function(scene, name)
    prefix += function(read_source("src/Scene/Scene_LegacyCleanup.cpp", revision), "CSQuest__CheckQuestState")
    medium = read_source("src/Core/Runtime_Medium.cpp", revision)
    prefix += function(medium, "CSQuest__ShowQuestNpcWindow") + function(medium, "CSQuest__setQuestLists")
    prefix += function(read_source("src/Core/Runtime_Small.cpp", revision), "CSQuest__setQuestList")
    recv = read_source("src/Net/Recv/Recv_Quest.cpp", revision)
    for name in ("NetRecv_A0", "NetRecv_A1", "NetRecv_A2"):
        prefix += function(recv, name)
    return prefix + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-flow-") as temp:
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
