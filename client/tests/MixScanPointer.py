#!/usr/bin/env python3
"""CheckMixRecipe real con cursores nativos y recetas contra x86.

Catálogo/precios controlados. Conserva contadores contiguos y offsets raw
de especiales; no certifica envío de mezcla, NPC ni fórmula de precios.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness, SERVICES
from RepairScanPointer import BACKEND
from SceneObjectPointer import build_and_run, function, read_source

SERVICES_MIX = r'''
BYTE* activeGrid; int priceMode;
std::vector<int> prices;
int ItemValue(ITEM* item,int mode) {
    const auto delta=reinterpret_cast<BYTE*>(item)-activeGrid;
    Require(delta>=0 && delta<64*68 && delta%68==0 && mode==0);
    const int slot=static_cast<int>(delta/68); prices.push_back(slot);
    return priceMode==0?0:priceMode==1?4000000:400000000;
}
'''

MAIN = r'''
std::uint64_t RunMix(BYTE* memory,BYTE* tableMemory) {
    activeGrid=memory+16; auto* items=reinterpret_cast<ITEM*>(activeGrid);
    auto* table=reinterpret_cast<ITEM_ATTRIBUTE*>(tableMemory+64); DAT_07d78068=reinterpret_cast<std::uintptr_t>(table);
    std::memset(tableMemory,0xa5,8192*64+80);
    for(int i=0;i<8192;++i) { table[i].Width=static_cast<BYTE>(i*31); table[i].Height=static_cast<BYTE>(i*37); }
    std::vector<BYTE> beforeTable(tableMemory,tableMemory+8192*64+80);
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,0x5a,4384); for(int slot=0;slot<64;++slot) items[slot].Type=-1;
        catalogMode=(trial/16)%2; priceMode=(trial/32)%3;
        int count=0; std::vector<int> expectedPrices;
        auto add=[&](short type,int level=0,int special=-1,int value=0,int option=0) {
            ITEM& item=items[count++]; std::memset(&item,0,68);
            item.Type=static_cast<short>(type+(catalogMode?512:0)); item.Key=1; item.Level=static_cast<int>(0x80000000u|static_cast<unsigned>(level*8)); item.Durability=255; item.Option1=static_cast<BYTE>(option);
            if(special>=0) { item.SpecialNum=1; *(reinterpret_cast<BYTE*>(&item)+37)=static_cast<BYTE>(special); *(reinterpret_cast<BYTE*>(&item)+45)=static_cast<BYTE>(value); }
        };
        const int profile=trial%16, level=(trial/512)%16;
        const DWORD initial168=0xdeadbeefu,initial170=0xa5b6c7d8u,initial178=0xc5d6e7f8u;
        DWORD expected168=initial168,expected170=initial170,expected178=initial178;
        int expected174=-97531,expectedResult=0;
        if(profile!=0 && profile!=13) add(ITEM_JEWEL_CHAOS);
        switch(profile) {
        case 2: case 3:
            add(465,level); add(466,profile==2?level:(level+1)%16); expectedResult=profile==2?2:-2;
            if(profile==2) expected168=level+1; break;
        case 4: case 5:
            add(432,level); add(433,profile==4?level:(level+1)%16); expectedResult=profile==4?8:-8;
            if(profile==4) expected168=level+1; break;
        case 6: add(0,9,0x40); add(ITEM_JEWEL_BLESS); add(ITEM_JEWEL_SOUL); expectedResult=3; expected178=1; expected170=1; break;
        case 7: add(0,10,0x40); add(ITEM_JEWEL_BLESS); add(ITEM_JEWEL_BLESS); add(ITEM_JEWEL_SOUL); add(ITEM_JEWEL_SOUL); expectedResult=4; expected178=1; expected170=1; break;
        case 8: for(int i=0;i<10;++i) add(ITEM_HELPER_UNIRIA); expectedResult=5; break;
        case 9: add(ITEM_JEWEL_CREATION); expectedResult=6; break;
        case 10:
            add(430); add(ITEM_WING_ELF,0,0x41,static_cast<int>(trial>>8)); expectedResult=7;
            expected170=4*static_cast<BYTE>(trial>>8); expectedPrices.push_back(2);
            expected174=priceMode==0?0:priceMode==1?1:90; break;
        case 11: case 12:
            add(profile==11?50:70,4,0x3c); expectedResult=profile==11?1:11; expected178=0; break;
        case 13: add(50,4,0x3c); expected178=0; break;
        case 14: add(ITEM_WING_ELF,0,0x3d,7); expected170=7; expectedPrices.push_back(1); break;
        case 15: add(50,4,0x3c,0,1); expectedResult=1; expected178=0; expectedPrices.push_back(1); break;
        default: break;
        }
        const int width=1+static_cast<int>((trial/64)%8), height=(count+width-1)/width+1;
        Require(width*height<=64);
        int maxWidth=1,maxHeight=1;
        for(int i=0;i<count;++i) { maxWidth=(std::max)(maxWidth,static_cast<int>(table[items[i].Type].Width)); maxHeight=(std::max)(maxHeight,static_cast<int>(table[items[i].Type].Height)); }
        for(int i=count;i<width*height;++i) { items[i].Type=static_cast<short>(trial%8192); items[i].Key=i%2?0:-1; }
        DAT_0055a3f8=-1; DAT_0055a3fc=-2; DAT_07eaa168=initial168; DAT_07eaa170=initial170; DAT_07eaa178=initial178; DAT_07eaa174=-97531;
        BYTE before[4384]; std::memcpy(before,memory,4384); prices.clear();
        const int result=CheckMixRecipe(reinterpret_cast<short*>(activeGrid),width,height);
        Require(result==expectedResult && DAT_0055a3f8==maxWidth && DAT_0055a3fc==maxHeight && DAT_07eaa168==expected168 && DAT_07eaa170==expected170 && DAT_07eaa178==expected178 && DAT_07eaa174==expected174);
        Require(prices==expectedPrices && std::memcmp(before,memory,4384)==0);
        coverage|=1u<<profile;
        const std::uint32_t values[]={static_cast<unsigned>(result),static_cast<unsigned>(maxWidth),static_cast<unsigned>(maxHeight),DAT_07eaa168,DAT_07eaa170,DAT_07eaa178,static_cast<unsigned>(DAT_07eaa174)};
        for(auto value:values) { digest^=value; digest*=1099511628211ull; }
    }
    for(int cols:{-3,0,1}) for(int rows:{-3,0,1}) if(cols<=0 || rows<=0) {
        Require(CheckMixRecipe(reinterpret_cast<short*>(activeGrid),cols,rows)==0 && DAT_0055a3f8==1 && DAT_0055a3fc==1);
    }
    Require(coverage==65535 && std::memcmp(beforeTable.data(),tableMemory,8192*64+80)==0); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && table);
    const auto expected=RunMix(low,table);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,4384); if(mask&2) std::memset(table,0,8192*64+80);
            Require(RunMix(mask&1?high:low,mask&2?highTable:table)==expected);
            if(mask&1) for(int i=0;i<4384;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<8192*64+80;++i) Require(table[i]==0);
        }
        std::puts("cuatro combinaciones grid/atributos altas y bajas, aliases intactos");
    }
    std::printf("Mix: 32768 estados, recetas/dimensiones/flags/precios/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0].replace(SERVICES,BACKEND)
    structs=read_source("src/structs.h",revision)
    prefix+="\n".join(re.findall(r"^inline (?:const )?ITEM\* AsItem[^\n]+",structs,re.M))
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_0055a3f8","DAT_0055a3fc","DAT_07eaa168","DAT_07eaa170","DAT_07eaa174","DAT_07eaa178"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    active=read_source("src/Item/Item_ChaosMix.cpp",revision).split("#endif",1)[1]
    return prefix+SERVICES_MIX+function(active,"CheckMixRecipe")+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-mix-scan-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
