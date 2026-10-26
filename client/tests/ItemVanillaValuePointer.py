#!/usr/bin/env python3
"""Precio vanilla: base ITEM nativa, guardas, Money y joyas contra x86.

Esta fórmula está inactiva con MuEmu. No certifica su fórmula general ni la
penalización por durabilidad; esos caminos fallan si llaman al doble aislado.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
int Rounded(int value) { return value>=1000?100*(value/100):value>=100?10*(value/10):value; }
std::uint64_t RunValue(BYTE* memory,BYTE* tableMemory) {
    ITEM* item=reinterpret_cast<ITEM*>(memory+16);
    constexpr std::size_t bytes=0xa0000; std::memset(tableMemory,0xa5,bytes); std::vector<BYTE> expectedTable(bytes,0xa5);
    const auto allocation=reinterpret_cast<std::uintptr_t>(tableMemory+64);
    Require(ItemValue_Vanilla(nullptr,0)==0 && ItemValue_Vanilla(reinterpret_cast<void*>(std::uintptr_t(0x123)),0)==0);
    std::uint64_t digest=14695981039346656037ull;
    const short jewels[]={ITEM_JEWEL_BLESS,ITEM_JEWEL_SOUL,ITEM_JEWEL_CHAOS,ITEM_JEWEL_LIFE,ITEM_JEWEL_CREATION};
    const int prices[]={9000000,6000000,810000,45000000,36000000}, money[]={1,99,100,999,1000,10000,2000000000};
    const short invalid[]={-32768,-1,8192,32767};
    for(unsigned trial=0;trial<65536;++trial) {
        std::memset(memory,0x5a,100); item->SpecialNum=0; item->Level=static_cast<int>(trial&255);
        const int mode=trial%4, goldType=(trial/4)%3;
        const short type=mode==1?jewels[(trial/4)%5]:mode==2?invalid[(trial/4)%4]:static_cast<short>((trial/4)%8192);
        item->Type=type;
        const auto base=allocation+((trial/4)%2?1023u*64u:0u);
        DAT_055c9e44=mode==3 && trial%8==3?0:allocation;
        DAT_07d78068=mode==3?base+1:base;
        const int price=mode==1?prices[(trial/4)%5]:money[(trial/4)%7];
        ITEM_ATTRIBUTE* row=nullptr; ITEM_ATTRIBUTE beforeRow{};
        if(mode<2) {
            row=reinterpret_cast<ITEM_ATTRIBUTE*>(base)+type; std::memset(row,0,sizeof(*row)); row->Money=mode==1?0:price; row->Durability=1;
            std::memcpy(&beforeRow,row,64); std::memcpy(expectedTable.data()+(reinterpret_cast<BYTE*>(row)-tableMemory),row,64);
        }
        BYTE before[100]; std::memcpy(before,memory,100);
        const int expected=mode<2?Rounded(goldType?price/3:price):0;
        const int result=ItemValue_Vanilla(item,goldType);
        Require(result==expected && std::memcmp(before,memory,100)==0);
        if(row) Require(std::memcmp(&beforeRow,row,64)==0);
        digest^=static_cast<unsigned>(result); digest*=1099511628211ull;
    }
    Require(std::memcmp(expectedTable.data(),tableMemory,bytes)==0); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && table);
    const auto expected=RunValue(low,table);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,100); if(mask&2) std::memset(table,0,0xa0000);
            Require(RunValue(mask&1?high:low,mask&2?highTable:table)==expected);
            if(mask&1) for(int i=0;i<100;++i) Require(low[i]==0);
            if(mask&2) for(std::size_t i=0;i<0xa0000;++i) Require(table[i]==0);
        }
        std::puts("cuatro combinaciones ITEM/tabla altas y bajas, offsets 0/1023 y aliases intactos");
    }
    std::printf("ItemValue_Vanilla: 65536 estados de Money/joyas/guardas; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0]
    prefix+="\n#ifndef _WIN32\n#define __forceinline inline\n#endif\n"
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    name="DAT_055c9e44"
    prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
    prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    prefix+=re.search(r"static __forceinline [^\n]*\bItemAttribute_Base\([^\{]*\{.*?^\}",header,re.M|re.S).group()
    prefix+="\n".join(re.findall(r"^inline (?:const )?ITEM\* AsItem[^\n]+",read_source("src/structs.h",revision),re.M))
    prefix+="\nunsigned int Item_CalculateMaxDurability(void*,ITEM_ATTRIBUTE*,int) { Require(false); return 0; }\n"
    return prefix+function(read_source("src/Item/Item_Durability.cpp",revision),"ItemValue_Vanilla")+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-vanilla-value-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
