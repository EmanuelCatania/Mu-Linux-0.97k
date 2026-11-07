#!/usr/bin/env python3
"""getQuestState real: máscaras/estado/bytes y dirección sobre 4 GiB.

Conserva el cálculo y retorno void del port. No certifica la máquina de
estados completa, UI, protocolo ni layout nativo CSQuest.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint64_t Run(BYTE* memory) {
    constexpr std::size_t size=0x1c890+32; BYTE* quest=memory+16;
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned index=0;index<256;++index) for(unsigned packed=0;packed<256;++packed) {
        std::memset(memory,0x5a,size); quest[0x1c87a]=static_cast<BYTE>(index); quest[0x1c848]=static_cast<BYTE>(packed);
        BYTE expected[size]; std::memcpy(expected,memory,size);
        expected[16+0x1c882]=static_cast<BYTE>((packed >> ((index*2)%32))&3);
        CSQuest_GetQuestState(quest,-1); Require(std::memcmp(expected,memory,size)==0);
        digest^=quest[0x1c882]; digest*=1099511628211ull;
    }
    for(unsigned index=0;index<256;++index) for(int param : {0,1,-2147483647-1,2147483647}) {
        std::memset(memory,0xa5,size); quest[0x1c87a]=static_cast<BYTE>(index);
        BYTE expected[size]; std::memcpy(expected,memory,size);
        CSQuest_GetQuestState(quest,param); Require(std::memcmp(expected,memory,size)==0);
        digest^=quest[0x1c882]; digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,0x1c890+32); Require(Run(high)==expected);
        for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
        std::puts("lectura de estado sobre 4 GiB y alias bajo intacto");
    }
    std::printf("CSQuest_GetQuestState: 66560 estados, mascaras/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    prefix += "\n#ifndef _WIN32\n#define __fastcall\n#endif\nusing uint=unsigned int;\n"
    return prefix + function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "CSQuest_GetQuestState") + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-state-") as temp:
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
