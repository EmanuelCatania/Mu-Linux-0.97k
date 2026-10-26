#!/usr/bin/env python3
"""Marca footprints con SetItemGridFlag real, ITEM/atributos y bases nativas.

Verifica sólo esta operación, no SortInventory ni trade/UI integrados.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
void Call(int slot,BYTE* pool,BYTE flag) { SetItemGridFlag(slot,ARG,flag); }
std::uint64_t Run(BYTE* memory,ITEM_ATTRIBUTE* attributes) {
    DAT_07d78068=reinterpret_cast<std::uintptr_t>(attributes);
    BYTE* pool=memory+16; std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<65536;++trial) for(int mode=0;mode<4;++mode) {
        std::memset(memory,0x5a,10032); std::memset(attributes,0xa5,512*sizeof(*attributes));
        const int slot=(trial/7)%64, type=trial%512;
        auto* item=reinterpret_cast<ITEM*>(pool+slot*68);
        item->Type=static_cast<short>(type);
        item->x=static_cast<BYTE>((trial/81)%8); item->y=static_cast<BYTE>((trial/648)%8);
        attributes[type].Width=static_cast<BYTE>(trial%9); attributes[type].Height=static_cast<BYTE>((trial/9)%9);
        if(mode==1) { item->x=255; attributes[type].Width=0; }
        if(mode==2) { item->y=255; attributes[type].Height=0; }
        const BYTE flag=static_cast<BYTE>(mode==3?trial*73:mode==2?99:mode);
        BYTE expected[10032], beforeAttributes[512*sizeof(*attributes)];
        std::memcpy(expected,memory,sizeof(expected)); std::memcpy(beforeAttributes,attributes,sizeof(beforeAttributes));
        for(unsigned dy=0;dy<attributes[type].Height;++dy)
        for(unsigned dx=0;dx<attributes[type].Width;++dx)
            expected[16+64+((item->y+dy)*8+item->x+dx)*68]=flag;
        Call(slot,pool,flag);
        Require(std::memcmp(expected,memory,sizeof(expected))==0 && std::memcmp(beforeAttributes,attributes,sizeof(beforeAttributes))==0);
        for(int cell=0;cell<144;++cell) { digest^=pool[cell*68+64]; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* pool=static_cast<BYTE*>(Reserve(0x76000000u)); auto* attributes=static_cast<ITEM_ATTRIBUTE*>(Reserve(0x75000000u)); Require(pool && attributes);
    const auto expected=Run(pool,attributes);
    if(sizeof(void*)>4) {
        auto* highPool=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttributes=static_cast<ITEM_ATTRIBUTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highPool && highAttributes);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(pool,0,10032); if(mask&2) std::memset(attributes,0,512*sizeof(*attributes));
            Require(Run(mask&1?highPool:pool,mask&2?highAttributes:attributes)==expected);
            if(mask&1) for(int i=0;i<10032;++i) Require(pool[i]==0);
            if(mask&2) for(std::size_t i=0;i<512*sizeof(*attributes);++i) Require(reinterpret_cast<BYTE*>(attributes)[i]==0);
        }
        std::puts("cuatro combinaciones de grilla/atributos altos y bajos, aliases intactos");
    }
    std::printf("SetItemGridFlag: 262144 footprints, bytes/canarios/oraculo; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    prefix += "\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\nusing std::uintptr_t;\n"
    structs = read_source("src/structs.h", revision)
    prefix += re.search(r"^#define MAX_SPECIAL_OPTION[^\n]+", structs, re.M).group() + "\n"
    prefix += "\n".join(re.findall(r"^#define\s+(?:MAX_CLASS|MAX_RESISTANCE)\s+[^\n]+", structs, re.M)) + "\n"
    for name in ("ITEM", "ITEM_ATTRIBUTE"):
        prefix += re.search(rf"struct {name}\b.*?\n\}};", structs, re.S).group()
    prefix += "\nstatic_assert(sizeof(ITEM)==68 && offsetof(ITEM,x)==62 && offsetof(ITEM,y)==63 && sizeof(ITEM_ATTRIBUTE)==64);\n"
    prefix += "\n".join(re.findall(r"^inline (?:const )?ITEM\* AsItem[^\n]+", structs, re.M))
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    name = "DAT_07d78068"
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
    prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    body = function(read_source("src/Item/Item_InventoryGrid.cpp", revision), "SetItemGridFlag")
    arg = "pool" if "BYTE* param_2" in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(pool))"
    return prefix + body + MAIN.replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-grid-flag-") as temp:
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
