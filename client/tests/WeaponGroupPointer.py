#!/usr/bin/env python3
"""Conteo real de grupos con ocho columnas, sin comparar direcciones truncadas.

Preserva el árbol actual del port, incluida su omisión de 458 en QKey.
No certifica render de HUD, asignación de teclas ni comportamiento del servidor.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint64_t RunGroup(BYTE* memory) {
    inventory=memory+16; InitializeAliases();
    auto* items=reinterpret_cast<ITEM*>(inventory);
    const int types[]={-32768,-1,0,143,447,448,449,450,451,452,453,454,455,456,457,458,459,467,468,469,511,512,8191,32767};
    const int modes[]={0,1,2,3,448,458,1000,-1,-32768,2147483647};
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    for(unsigned trial=0;trial<8192;++trial) {
        std::memset(memory,0x5a,4384);
        for(int slot=0;slot<64;++slot) {
            ITEM& item=items[slot]; item.Type=static_cast<short>(types[(trial+slot*7)%24]);
            item.Key=(trial+slot)%5==0?0:(trial+slot)%5==1?-1:static_cast<int>(trial+1);
            item.Durability=static_cast<BYTE>(trial*13+slot*71);
        }
        DAT_00559c60=types[(trial/10)%24]; DAT_00559c64=types[(trial/240)%24]; DAT_00559c68=types[(trial/2)%24];
        const int mode=modes[trial%10]; int first=mode,last=mode; bool one=false;
        if(mode>=0 && mode<=2) {
            const int key=mode==0?DAT_00559c60:mode==1?DAT_00559c64:DAT_00559c68;
            if(key==456 || key==457 || key==468 || (mode!=0 && key==458)) { first=last=key; one=key!=456; }
            else if(mode==1) { first=key>=448 && key<=451?448:452; last=first+3-(first==452?1:0); }
            else if(mode==2 && (key<448 || key>451) && (key<452 || key>454)) first=last=456;
            else if(key>=452 && key<=454) { first=452; last=454; }
            else { first=448; last=451; }
        }
        int expected=0;
        for(int slot=0;slot<64;++slot) if(static_cast<std::int32_t>(items[slot].Key)>0 && items[slot].Type>=first && items[slot].Type<=last) expected+=one?1:items[slot].Durability;
        BYTE before[4384]; std::memcpy(before,memory,4384);
        const int result=Item_CountWeaponGroupItems(mode);
        if(result!=expected) std::fprintf(stderr,"trial=%u mode=%d keys=%d/%d/%d range=%d..%d one=%d expected=%d result=%d\n",trial,mode,DAT_00559c60,DAT_00559c64,DAT_00559c68,first,last,one,expected,result);
        Require(result==expected && std::memcmp(before,memory,4384)==0);
        Require(DAT_00559c60==types[(trial/10)%24] && DAT_00559c64==types[(trial/240)%24] && DAT_00559c68==types[(trial/2)%24]);
        coverage|=1u<<(trial%10); digest^=static_cast<unsigned>(result); digest*=1099511628211ull;
    }
    Require(coverage==1023); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunGroup(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* boundary=static_cast<BYTE*>(Reserve(std::uint64_t(0x17fff0000ull))); Require(high && boundary);
        std::memset(low,0,4384); Require(RunGroup(high)==expected && RunGroup(boundary+0xf000)==expected);
        for(int i=0;i<4384;++i) Require(low[i]==0);
        std::puts("inventario alto y limite signed de 32 bits dentro del array, alias bajo intacto");
    }
    std::printf("Grupos: 8192 estados, teclas/tipos/claves/durabilidad/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0]
    prefix+="\nBYTE* inventory;\n#define OffsetInventoryItems (*reinterpret_cast<BYTE(*)[4352]>(inventory))\n"
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_00559c60","DAT_00559c64","DAT_00559c68"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    initializers=[]
    for name in ("p_DAT_07ea9504_","p_DAT_07ea9328_"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=f"\nint* {name}=nullptr;\n"
        initializers.append(re.search(rf"\b{name}\s*=\s*[^;]+;",definitions).group())
    for name in ("DAT_07ea9504","DAT_07ea9328"):
        prefix+="\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",header,re.M).group()+"\n"
    prefix+="\nvoid InitializeAliases() {"+"".join(initializers)+"}\n"
    return prefix+function(read_source("src/Item/Item_EquipmentQueries.cpp",revision),"Item_CountWeaponGroupItems")+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-weapon-group-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
