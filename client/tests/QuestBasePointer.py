#!/usr/bin/env python3
"""Publicación de g_csQuest y lectura/cierre reales con base sobre 4 GiB.

Extrae sólo publicación/reset del global, no ejecuta ctor/dtor con vtable de
32 bits. No certifica esos layouts ni los métodos CSQuest que aún reciben int.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from QuestClearPointer import harness as clear_harness
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint64_t RunBase(BYTE* memory) {
    constexpr std::size_t size=0x40000;
    auto* quest=memory+16; auto* other=memory+0x20000;
    std::uint64_t digest=14695981039346656037ull;
    ResetBase(); Require(g_csQuest==0 && !IsQuestPanelOpen());
    for(unsigned flag=0;flag<256;++flag) {
        std::memset(memory,static_cast<BYTE>(flag^0x5a),size); quest[0x1c87f]=static_cast<BYTE>(flag);
        ResetBase(); PublishBase(quest); PublishBase(other);
        Require(static_cast<std::uintptr_t>(g_csQuest)==reinterpret_cast<std::uintptr_t>(quest) && DAT_00583d8c==g_csQuest);
        Require(IsQuestPanelOpen()==(flag!=0));
        activeQuest=quest; event=0; sends=0;
        BYTE expected[size]; std::memcpy(expected,memory,size); expected[16+0x1c87f]=0;
        CSQuest_clearQuest(reinterpret_cast<void*>(static_cast<std::uintptr_t>(g_csQuest)));
        Require(event==2 && sends==1 && !IsQuestPanelOpen() && std::memcmp(expected,memory,size)==0);
        ResetBase(); Require(!g_csQuest && !IsQuestPanelOpen());
        for(std::size_t i=0;i<size;++i) { digest^=memory[i]; digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=RunBase(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x40000); Require(RunBase(high)==expected);
        for(std::size_t i=0;i<0x40000;++i) Require(low[i]==0);
        std::puts("global de quest sobre 4 GiB, alias y lector/cierre completos");
    }
    std::printf("g_csQuest: 256 flags, publicar/no reemplazar/reset/leer/cerrar; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = clear_harness(revision).split("void Call(void* quest)", 1)[0]
    prefix += "\nusing std::uintptr_t; using std::size_t;\n"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    prefix += re.search(r"extern [^;\n]*\bg_csQuest\s*;", header).group()
    prefix += re.search(r"^[^/\n]*\bg_csQuest\s*=\s*[^;]+;", definitions, re.M).group()
    prefix += "\n" + re.search(r"^#define DAT_00583d8c[^\n]+", header, re.M).group() + "\n"
    source = read_source("src/Net/Crypto.cpp", revision)
    init = function(source, "Quest_FullInit")
    publish = re.search(r"if \(g_csQuest == 0\) g_csQuest = [^;]+;", init).group()
    reset = re.search(r"g_csQuest = 0;", function(source, "FUN_00403ef0")).group()
    prefix += "void PublishBase(void* param_1) { " + publish + " }\nvoid ResetBase() { " + reset + " }\n"
    prefix += function(read_source("src/UI/UIState.cpp", revision), "IsQuestPanelOpen")
    return prefix + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-base-") as temp:
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
