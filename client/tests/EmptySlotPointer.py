#!/usr/bin/env python3
"""Búsqueda real de espacio con grilla y atributos sobre 4 GiB.

Conserva stride ITEM 68 y Width/Height en 32/33 del atributo de 64 bytes.
No certifica UI, recogida integrada ni memoria fuera de los índices válidos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
int Call(int type,const BYTE* pool,int width,int height) { return FindEmptySlot(type,ARG,width,height); }
bool Expected(const BYTE* pool,int gridW,int gridH,int w,int h) {
    if(w<1 || w>8 || h<1 || h>8) return false;
    for(int y=0;y+h<=gridH;++y) for(int x=0;x+w<=gridW;++x) {
        bool empty=true;
        for(int dy=0;dy<h;++dy) for(int dx=0;dx<w;++dx) {
            short value; std::memcpy(&value,pool+((y+dy)*gridW+x+dx)*68,2); if(value!=-1) empty=false;
        }
        if(empty) return true;
    }
    return false;
}
std::uint64_t Run(BYTE* memory,BYTE* attributes) {
    DAT_07d78068=reinterpret_cast<std::uintptr_t>(attributes);
    BYTE* pool=memory+16; std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<65536;++trial) for(int mode=0;mode<4;++mode) {
        std::memset(memory,0x5a,10032); std::memset(attributes,0xa5,512*64);
        const int type=trial%512;
        const int gridW=mode<2?4:static_cast<int>(trial%14)-1;
        const int gridH=mode<2?4:static_cast<int>((trial/14)%14)-1;
        const int w=mode<2?mode+1:mode==2?static_cast<int>(trial&255):static_cast<int>(trial%8)+1;
        const int h=mode<2?mode+1:mode==2?static_cast<int>((trial>>8)&255):static_cast<int>((trial/8)%8)+1;
        attributes[type*64+32]=static_cast<BYTE>(w); attributes[type*64+33]=static_cast<BYTE>(h);
        for(int cell=0;cell<144;++cell) {
            const bool empty=mode<2?((trial>>(cell%16))&1)==0:mode==3 || ((trial+cell*40503u)&3)==0;
            const short value=empty?-1:static_cast<short>(cell%2?500:-2);
            std::memcpy(pool+cell*68,&value,2);
            // El marcador de origen +56 no interviene: sólo importa el sentinel.
            const DWORD marker=cell%2?0:0xffffffffu; std::memcpy(pool+cell*68+56,&marker,4);
        }
        BYTE before[10032], attrBefore[512*64]; std::memcpy(before,memory,sizeof(before)); std::memcpy(attrBefore,attributes,sizeof(attrBefore));
        const int expected=Expected(pool,gridW,gridH,w,h), result=Call(type,pool,gridW,gridH);
        Require(result==expected && std::memcmp(before,memory,sizeof(before))==0 && std::memcmp(attrBefore,attributes,sizeof(attrBefore))==0);
        digest^=static_cast<unsigned>(result); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* pool=static_cast<BYTE*>(Reserve(0x76000000u)); auto* attributes=static_cast<BYTE*>(Reserve(0x75000000u)); Require(pool && attributes);
    const auto expected=Run(pool,attributes);
    if(sizeof(void*)>4) {
        auto* highPool=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highAttributes=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highPool && highAttributes);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(pool,0,10032); if(mask&2) std::memset(attributes,0,512*64);
            Require(Run(mask&1?highPool:pool,mask&2?highAttributes:attributes)==expected);
            if(mask&1) for(int i=0;i<10032;++i) Require(pool[i]==0);
            if(mask&2) for(int i=0;i<512*64;++i) Require(attributes[i]==0);
        }
        std::puts("cuatro combinaciones de grilla/atributos altos y bajos, aliases intactos");
    }
    std::printf("FindEmptySlot: 262144 grillas, oraculo y memoria intacta; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0]
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    name = "DAT_07d78068"
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
    prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    body = function(read_source("src/Item/Item_InventoryGrid.cpp", revision), "FindEmptySlot")
    arg = "pool" if "const BYTE* param_2" in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(pool))"
    return prefix + body + MAIN.replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-empty-slot-") as temp:
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
