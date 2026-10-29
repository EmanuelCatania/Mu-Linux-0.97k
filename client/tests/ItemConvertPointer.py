#!/usr/bin/env python3
"""ItemConvert real con ITEM/atributos/CharacterAttribute nativos y bytes x86.

Catálogo controlado; conserva fórmulas/extensiones del port. No certifica
recepción/UI integradas, catálogo real ni tipos fuera de -32768..8191 válidos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int activeType, catalogMode;
std::vector<int> catalogEvents;
CatalogWing wing{};
int Behavior(int type) {
    const int types[]={70,134,167,395,426,135,143,387,388,389,390,411,428,429};
    return type>=512 && (catalogMode==1 || catalogMode==3)?types[(type+catalogMode)%14]:type;
}
struct Catalog {
    int GetItemBehavior(int type) { Require(type==activeType); catalogEvents.push_back(1); return Behavior(type); }
    const CatalogWing* GetItemWing(int type) { Require(type==activeType); catalogEvents.push_back(2); return catalogMode==3 && type>=512?&wing:nullptr; }
    int GetItemSkill(int type) { Require(type==activeType); catalogEvents.push_back(3); return catalogMode==0?-1:catalogMode==1?0:catalogMode==2?24:255; }
    int GetItemSlot(int type) { Require(type==activeType); catalogEvents.push_back(4); return type%4==0?-1:type%11; }
} gContentCatalog;
'''

MAIN = r'''
void Convert(ITEM* item,int option,int excellent) { ItemConvert(ARG,option,excellent); }
void Hash(std::uint64_t& digest,const void* p,std::size_t size) {
    const auto* bytes=static_cast<const BYTE*>(p); for(std::size_t i=0;i<size;++i) { digest^=bytes[i]; digest*=1099511628211ull; }
}
std::uint64_t RunConvert(BYTE* memory,BYTE* tableMemory,BYTE* attributeMemory) {
    ITEM* item=reinterpret_cast<ITEM*>(memory+16); auto* table=reinterpret_cast<ITEM_ATTRIBUTE*>(tableMemory+64); BYTE* attribute=attributeMemory+16;
    constexpr std::size_t tableBytes=8192*sizeof(ITEM_ATTRIBUTE);
    std::memset(tableMemory,0xa5,tableBytes+80); std::memset(table,0,tableBytes);
    std::vector<BYTE> expectedTable(tableBytes,0);
    DAT_07d78068=0; DAT_07cf1ff4=nullptr; Convert(nullptr,0,0);
    std::uint64_t digest=14695981039346656037ull;
    const short types[]={-32768,-1,0,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,31,34,65,67,68,69,70,96,97,100,103,105,106,127,128,134,135,136,143,145,146,159,160,167,170,195,196,223,224,255,256,384,385,386,387,388,389,390,391,395,399,411,418,419,423,424,425,426,427,428,429,430,511,512,513,1000,8191};
    const WORD requirements[]={0,1,255,32768,65535};
    for(unsigned trial=0;trial<65536;++trial) {
        std::uint32_t seed=trial^0x9747b910u;
        auto next=[&]() { seed=seed*1664525u+1013904223u; return seed>>16; };
        std::memset(memory,0x5a,100); std::memset(attributeMemory,0xcc,96);
        const short type=trial%4==0?static_cast<short>((trial/4)%8192):types[(trial/4)%(sizeof(types)/sizeof(types[0]))];
        item->Type=type; item->Durability=static_cast<BYTE>(next()); item->byColorState=static_cast<BYTE>(trial);
        item->x=static_cast<BYTE>(trial); item->y=static_cast<BYTE>(trial>>8);
        const WORD level=static_cast<WORD>(next()); std::memcpy(attribute+14,&level,2);
        DAT_07cf1ff4=attribute; DAT_07d78068=trial%17==0?0:reinterpret_cast<std::uintptr_t>(table);
        const int a1=static_cast<int>((trial&255)|(trial%13==0?0x80000000u:trial%13==1?0x7fffff00u:0));
        const int a2=static_cast<int>((trial>>8)|(trial%11==0?0x80000000u:trial%11==1?0x7fffff00u:0));
        ITEM_ATTRIBUTE beforeRow{};
        if(type>=0) {
            ITEM_ATTRIBUTE& row=table[type]; std::memset(&row,0,sizeof(row));
            row.TwoHand=(trial&1)!=0; row.Level=static_cast<BYTE>(next()); row.AttackSpeed=static_cast<BYTE>(next());
            row.DamageMin=static_cast<BYTE>(next()); row.DamageMax=static_cast<BYTE>(next()); row.DefenseRate=static_cast<BYTE>(next());
            row.Defense=static_cast<BYTE>(next()); row.MagicDefense=static_cast<BYTE>(next()); row.WalkSpeed=static_cast<BYTE>(next());
            row.RequireStrength=requirements[next()%5]; row.RequireAgility=requirements[next()%5]; row.RequireEnergy=requirements[next()%5]; row.RequireLevel=static_cast<BYTE>(next());
            for(int klass=0;klass<4;++klass) row.RequireClass[klass]=static_cast<BYTE>(next()%3);
            std::memcpy(&beforeRow,&row,sizeof(row)); std::memcpy(expectedTable.data()+type*64,&row,64);
        }
        catalogMode=trial%4; activeType=type; wing.DefenseConstA=static_cast<WORD>(trial%32+1); catalogEvents.clear();
        BYTE before[100], beforeAttribute[96]; std::memcpy(before,memory,100); std::memcpy(beforeAttribute,attributeMemory,96);
        Convert(item,a1,a2);
        if(type<0) {
            ITEM* expected=reinterpret_cast<ITEM*>(before+16); expected->Part=255; expected->SpecialNum=0;
            std::memset(expected->Special,0,sizeof(expected->Special)); std::memset(expected->SpecialValue,0,sizeof(expected->SpecialValue));
            Require(catalogEvents.empty() && std::memcmp(before,memory,100)==0);
        } else if(DAT_07d78068==0) Require(catalogEvents.empty() && std::memcmp(before,memory,100)==0);
        else {
            const ITEM* previous=reinterpret_cast<const ITEM*>(before+16);
            Require(item->Type==type && item->Durability==previous->Durability && item->Key==previous->Key && item->byColorState==previous->byColorState && item->x==previous->x && item->y==previous->y);
            Require(item->Level==static_cast<BYTE>(a1) && item->SpecialNum<=MAX_SPECIAL_OPTION && item->TwoHand==beforeRow.TwoHand && item->WeaponSpeed==beforeRow.AttackSpeed && item->WalkSpeed==beforeRow.WalkSpeed);
            const int behavior=Behavior(type), itemLevel=(static_cast<BYTE>(a1)>>3)&15;
            int excellent=static_cast<BYTE>(a2)&63;
            if((behavior>=387 && behavior<=390) || behavior==19 || behavior==146 || behavior==170) excellent=0;
            const int bonus=3*(itemLevel<9?itemLevel:9)+(itemLevel>9?4+5*(itemLevel-10):0);
            int add=behavior==70?15:behavior==134?30:behavior==167?25:0;
            if(!add && beforeRow.Level) add=25*beforeRow.DamageMin/beforeRow.Level+5;
            const int excelBonus=excellent && beforeRow.Level?add:0;
            Require(item->DamageMin==(beforeRow.DamageMin?beforeRow.DamageMin+excelBonus+bonus:0));
            Require(item->DamageMax==(beforeRow.DamageMax?beforeRow.DamageMax+excelBonus+bonus:0));
            Require(item->MagicDefense==(beforeRow.MagicDefense?beforeRow.MagicDefense+bonus:0));
            Require(!catalogEvents.empty() && catalogEvents[0]==1);
        }
        if(type>=0) Require(std::memcmp(&beforeRow,&table[type],64)==0);
        Require(std::memcmp(beforeAttribute,attributeMemory,96)==0);
        for(int i=0;i<16;++i) Require(memory[i]==0x5a && memory[84+i]==0x5a);
        Hash(digest,memory,100); for(int event : catalogEvents) Hash(digest,&event,sizeof(event));
    }
    Require(std::memcmp(table,expectedTable.data(),tableBytes)==0);
    for(int i=0;i<64;++i) Require(tableMemory[i]==0xa5); for(int i=0;i<16;++i) Require(tableMemory[64+tableBytes+i]==0xa5);
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); auto* attribute=static_cast<BYTE*>(Reserve(0x73000000u)); Require(low && table && attribute);
    const auto expected=RunConvert(low,table,attribute);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); auto* highAttribute=static_cast<BYTE*>(Reserve(std::uint64_t(0x173000000ull))); Require(high && highTable && highAttribute);
        for(int mask=1;mask<8;++mask) {
            if(mask&1) std::memset(low,0,100); if(mask&2) std::memset(table,0,8192*64+80); if(mask&4) std::memset(attribute,0,96);
            Require(RunConvert(mask&1?high:low,mask&2?highTable:table,mask&4?highAttribute:attribute)==expected);
            if(mask&1) for(int i=0;i<100;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<8192*64+80;++i) Require(table[i]==0);
            if(mask&4) for(int i=0;i<96;++i) Require(attribute[i]==0);
        }
        std::puts("ocho combinaciones ITEM/tabla/atributos altas y bajas, aliases intactos");
    }
    std::printf("ItemConvert: 65536 estados, bytes/opciones/stats/catalogo %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("BYTE* reservation;",1)[0].replace("0x10000","0x100000")
    prefix+="\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\nusing std::uintptr_t;\n"
    structs=read_source("src/structs.h",revision)
    for name in ("MAX_SPECIAL_OPTION","MAX_CLASS","MAX_RESISTANCE"):
        prefix+="\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",structs,re.M).group()+"\n"
    item_header = '#include "Item/ItemTypes.h"' in structs
    if item_header:
        prefix += read_source("src/Item/ItemTypes.h", revision)
    for name in (("ITEM_ATTRIBUTE",) if item_header else ("ITEM","ITEM_ATTRIBUTE")):
        prefix+=re.search(rf"struct {name}\b.*?\n\}};",structs,re.S).group()
    prefix+="\nstatic_assert(sizeof(ITEM)==68 && sizeof(ITEM_ATTRIBUTE)==64);\n"
    prefix+="\n".join(re.findall(r"^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;",read_source("src/Item/ItemDefines.h",revision),re.M))
    catalog=read_source("src/Item/ContentCatalog.h",revision)
    prefix+=re.search(r"struct CatalogWing\s*\{.*?\n\};",catalog,re.S).group()
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_07d78068","DAT_07cf1ff4"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    prefix+="\n"+re.search(r"^#define\s+CharacterAttribute\s+[^\n]+",header,re.M).group()+"\n"
    prefix+=SERVICES+function(catalog,"ItemBehaviorType")
    convert=function(read_source("src/Item/Item_LegacyLinker.cpp",revision),"ItemConvert")
    arg="item" if "ITEM* ip" in convert.split("{",1)[0] else "static_cast<int>(reinterpret_cast<std::uintptr_t>(item))"
    return prefix+convert+MAIN.replace("ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-convert-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
