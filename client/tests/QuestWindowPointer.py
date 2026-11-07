#!/usr/bin/env python3
"""Apertura real de quest y A1/A2 con base nativa; bytes/eventos contra x86.

CheckQuestState/ShowDialogText/cierre controlados. No prueba la lógica interna
de esos servicios, dibujo de la ventana, socket o layout nativo CSQuest.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from QuestListPointer import harness as list_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
BYTE* activeQuest;
BYTE expectedIndex;
short dialog;
int event=0;
void CloseInventoryRelatedWindows() { Require(event==0 && activeQuest[0x1c87a]==expectedIndex); event=1; }
void CSQuest__CheckQuestState(void* self,int value) {
    Require(event==1 && self==activeQuest && value==255 && activeQuest[0x1c87f]==1);
    Require(GoldenArcherOpenType==0 && InventoryOpened==0 && CharacterOpened==0);
    std::memcpy(activeQuest+0x1c880,&dialog,2); event=2;
}
void CSQuest__ShowDialogText(int index,int param) { Require(event==2 && index==dialog && param==0); event=3; ++windows; }
'''

MAIN = r'''
std::uint64_t RunWindow(BYTE* memory) {
    constexpr std::size_t size=0x1c890+32;
    BYTE* quest=memory+16; activeQuest=quest; g_csQuest=reinterpret_cast<std::uintptr_t>(quest);
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<65536;++trial) {
        std::memset(memory,0x5a,size); quest[0x1c87a]=static_cast<BYTE>(trial*73);
        GoldenArcherOpenType=trial*40503u; InventoryOpened=static_cast<BYTE>(trial); CharacterOpened=static_cast<BYTE>(trial*73);
        const DWORD oldEvent=GoldenArcherOpenType; const auto oldInventory=InventoryOpened, oldCharacter=CharacterOpened;
        const int route=trial%3;
        const int index=trial%4==0?-1:static_cast<int>(trial%4==1?0x80000000u|trial:trial*40503u);
        dialog=static_cast<short>(trial);
        BYTE packet[]={0xc1,6,0xa1,static_cast<BYTE>(trial),static_cast<BYTE>(route==2?trial&1:trial*73),static_cast<BYTE>(trial*40503)};
        BYTE beforePacket[6]; std::memcpy(beforePacket,packet,6);
        BYTE expected[size]; std::memcpy(expected,memory,size);
        const bool show=route!=2 || packet[4]==0;
        if(route==0 && index!=-1) expected[16+0x1c87a]=static_cast<BYTE>(index);
        if(route!=0 && show) {
            expected[16+0x1c87a]=packet[3]; expected[16+0x1c848+(packet[3]>>2)]=route==1?packet[4]:packet[5];
        }
        expectedIndex=expected[16+0x1c87a];
        if(show) { expected[16+0x1c87f]=1; std::memcpy(expected+16+0x1c880,&dialog,2); }
        event=0; windows=0;
        if(route==0) CSQuest__ShowQuestNpcWindow(quest,0,index);
        if(route==1) NetRecv_A1(packet,6,0xc1,0,trial&1);
        if(route==2) NetRecv_A2(packet,6,0xc1,0,trial&1);
        Require(event==(show?3:0) && windows==(show?1:0) && std::memcmp(expected,memory,size)==0 && std::memcmp(beforePacket,packet,6)==0);
        Require(GoldenArcherOpenType==(show?0:oldEvent) && InventoryOpened==(show?0:oldInventory) && CharacterOpened==(show?0:oldCharacter));
        for(int i=0;i<72;++i) { digest^=quest[0x1c848+i]; digest*=1099511628211ull; }
        digest^=static_cast<unsigned>(event); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunWindow(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x1c890+32); Require(RunWindow(high)==expected);
        for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
        std::puts("ventana de quest y A1/A2 sobre 4 GiB, alias bajo intacto");
    }
    std::printf("ShowQuestNpcWindow: 65536 estados, bytes/flags/orden/dialogo %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = list_harness(revision).split("void List(BYTE* quest", 1)[0]
    fake = function(prefix, "CSQuest__ShowQuestNpcWindow")
    prefix = prefix.replace(fake, "void __fastcall CSQuest__ShowQuestNpcWindow(void*,int,int);")
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("GoldenArcherOpenType", "DAT_07eaa117", "DAT_07eaa116"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    for name in ("_g_bEventChipDialogEnable", "InventoryOpened", "CharacterOpened"):
        prefix += "\n" + re.search(rf"^#define\s+{name}\s+[^\n]+", header, re.M).group() + "\n"
    prefix += SERVICES
    prefix += function(read_source("src/Core/Runtime_Medium.cpp", revision), "CSQuest__ShowQuestNpcWindow")
    return prefix + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-window-") as temp:
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
