#!/usr/bin/env python3
"""Recorrido real de reparación: slots, argumentos y orden contra x86.

Precios, durabilidad máxima y catálogo controlados; no certifica sus fórmulas
ni una interacción real con el NPC. No modifica los layouts ITEM/atributos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness, SERVICES
from SceneObjectPointer import build_and_run, function, read_source

BACKEND = r'''
int catalogMode;
int Behavior(int type) { return catalogMode && type>=512 ? type%512 : type; }
struct Catalog { int GetItemBehavior(int type) { return Behavior(type); } } gContentCatalog;
'''

SERVICES_SCAN = r'''
BYTE* inventory;
#define OffsetInventoryItems (*reinterpret_cast<BYTE(*)[4352]>(inventory))
struct Event { int operation, slot, type, level, dur, max; };
std::vector<Event> events;
int Slot(void* item) {
    const auto value=reinterpret_cast<std::uintptr_t>(item);
    const auto equipment=reinterpret_cast<std::uintptr_t>(DAT_07cf1ffc)+0x218;
    const auto bag=reinterpret_cast<std::uintptr_t>(inventory);
    if(value>=equipment && value<equipment+12*68 && (value-equipment)%68==0) return static_cast<int>((value-equipment)/68);
    Require(value>=bag && value<bag+64*68 && (value-bag)%68==0);
    return 12+static_cast<int>((value-bag)/68);
}
unsigned int Item_CalculateMaxDurability(void* ptr,ITEM_ATTRIBUTE* row,int level) {
    ITEM* item=static_cast<ITEM*>(ptr); const int slot=Slot(ptr);
    Require(reinterpret_cast<std::uintptr_t>(row)==DAT_07d78068+item->Type*64 && level==((item->Level>>3)&15));
    const unsigned value=static_cast<unsigned>(row->Durability)+level+1;
    events.push_back({0,slot,item->Type,level,item->Durability,static_cast<int>(value)});
    return value|0xa57e0000u;
}
int Item_CalculateValue(void* ptr,int mode) {
    ITEM* item=static_cast<ITEM*>(ptr); const int slot=Slot(ptr); Require(mode==2);
    events.push_back({1,slot,item->Type,0,0,0}); return 1000+slot*17;
}
unsigned int Item_CalculateRepairCost(int gold,int dur,int max,short type,char* text) {
    Require(!events.empty() && events.back().operation==1);
    const int slot=events.back().slot; Require(gold==1000+slot*17 && text!=nullptr);
    events.push_back({2,slot,type,0,dur,max}); text[0]=0;
    return static_cast<unsigned>(gold+dur+max+type);
}
'''

MAIN = r'''
std::uint64_t RunScan(BYTE* machineMemory,BYTE* bagMemory,BYTE* tableMemory) {
    DAT_07cf1ffc=machineMemory+16; inventory=bagMemory+16;
    auto* table=reinterpret_cast<ITEM_ATTRIBUTE*>(tableMemory+64); DAT_07d78068=reinterpret_cast<std::uintptr_t>(table);
    constexpr std::size_t tableSize=8192*64;
    std::memset(tableMemory,0xa5,tableSize+80);
    for(int i=0;i<8192;++i) table[i].Durability=static_cast<BYTE>(i*29);
    std::vector<BYTE> beforeTable(tableMemory,tableMemory+tableSize+80);
    const short edges[]={0,134,135,136,142,143,144,390,391,403,404,415,416,417,419,420,425,426,427,429,430,435,436,447,448,449,511,512,8191};
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    for(unsigned trial=0;trial<8192;++trial) {
        catalogMode=trial%2; std::memset(machineMemory,0x5a,1444); std::memset(bagMemory,0x6b,4384);
        std::vector<Event> expected; int total=0;
        for(int slot=0;slot<76;++slot) {
            ITEM* item=reinterpret_cast<ITEM*>(slot<12?static_cast<BYTE*>(DAT_07cf1ffc)+0x218+slot*68:inventory+(slot-12)*68);
            const short type=trial%3==0?edges[(trial/3+slot)%29]:static_cast<short>((trial+slot*127)%8192);
            const bool empty=slot<12 && (trial+slot)%7==0;
            item->Type=empty?-1:type; item->Key=(trial+slot)%5==0?0:static_cast<int>(0x80000000u|trial|1u);
            item->Level=static_cast<int>((trial*7919u+slot*31u)|0x80000000u);
            item->Durability=static_cast<BYTE>(trial*13+slot*7);
            if(empty) coverage|=1;
            if(item->Key==0) coverage|=2;
            if(empty || item->Key==0) continue;
            const int level=(item->Level>>3)&15;
            const int max=static_cast<int>(table[type].Durability)+level+1;
            expected.push_back({0,slot,type,level,item->Durability,max});
            const int kind=catalogMode && type>=512?type%512:type;
            const bool eligible=(kind<416 || kind>419) && kind!=426 && kind!=135 && kind!=143 && kind<448 && (kind<391 || kind>403) && (kind<430 || kind>435);
            if(eligible && item->Durability<max) {
                coverage|=slot<12?4:8;
                expected.push_back({1,slot,type,0,0,0}); expected.push_back({2,slot,type,0,item->Durability,max});
                total+=1000+slot*17+item->Durability+max+type;
            } else coverage|=eligible?16:32;
        }
        BYTE beforeMachine[1444],beforeBag[4384]; std::memcpy(beforeMachine,machineMemory,1444); std::memcpy(beforeBag,bagMemory,4384);
        events.clear(); DAT_07eaa0f8=-12345; Item_RecalculateRepairCost();
        Require(DAT_07eaa0f8==total && events.size()==expected.size());
        for(std::size_t i=0;i<events.size();++i) {
            const Event &a=events[i],&b=expected[i];
            Require(a.operation==b.operation && a.slot==b.slot && a.type==b.type && a.level==b.level && a.dur==b.dur && a.max==b.max);
            for(int value:{a.operation,a.slot,a.type,a.level,a.dur,a.max}) { digest^=static_cast<unsigned>(value); digest*=1099511628211ull; }
        }
        digest^=static_cast<unsigned>(total); digest*=1099511628211ull;
        Require(std::memcmp(beforeMachine,machineMemory,1444)==0 && std::memcmp(beforeBag,bagMemory,4384)==0);
    }
    Require(coverage==63 && std::memcmp(beforeTable.data(),tableMemory,tableSize+80)==0); return digest;
}
int main() {
    auto* machine=static_cast<BYTE*>(Reserve(0x76000000u)); auto* bag=static_cast<BYTE*>(Reserve(0x75000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); Require(machine && bag && table);
    const auto expected=RunScan(machine,bag,table);
    if(sizeof(void*)>4) {
        auto* highMachine=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highBag=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highMachine && highBag && highTable);
        for(int mask=1;mask<8;++mask) {
            if(mask&1) std::memset(machine,0,1444); if(mask&2) std::memset(bag,0,4384); if(mask&4) std::memset(table,0,8192*64+80);
            Require(RunScan(mask&1?highMachine:machine,mask&2?highBag:bag,mask&4?highTable:table)==expected);
            if(mask&1) for(int i=0;i<1444;++i) Require(machine[i]==0);
            if(mask&2) for(int i=0;i<4384;++i) Require(bag[i]==0);
            if(mask&4) for(int i=0;i<8192*64+80;++i) Require(table[i]==0);
        }
        std::puts("ocho combinaciones maquina/inventario/atributos altas y bajas, aliases intactos");
    }
    std::printf("Reparacion: 8192 estados de 12+64 slots, orden/argumentos/suma/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0].replace(SERVICES,BACKEND)
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_07cf1ffc","DAT_07eaa0f8"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    return prefix+SERVICES_SCAN+function(read_source("src/Item/Item_Durability.cpp",revision),"Item_RecalculateRepairCost")+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-repair-scan-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
