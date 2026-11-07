#!/usr/bin/env python3
"""Emisor ambiental real: dirección completa, bytes/eventos y RNG controlado.

Conserva las diferencias del port respecto de IDA 0x00452030. El fixture
OBJECT es escalar con offsets x86; partículas registradas sin pool ni render.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemResetPointer import harness as reset_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int randomValues[2], randomIndex=0, calls=0, actualFlag, actualMode;
float actualVectors[9], actualAlpha;
int TestRand() { Require(randomIndex<2); return randomValues[randomIndex++]; }
#define rand TestRand
int Particle_Spawn(int type,float* position,float* angle,float* light,int flag,float alpha,int mode) {
    Require(type==1221 && calls++==0);
    std::memcpy(actualVectors,position,12); std::memcpy(actualVectors+3,angle,12); std::memcpy(actualVectors+6,light,12);
    actualFlag=flag; actualAlpha=alpha; actualMode=mode; return -1;
}
'''

MAIN = r'''
void Call(OBJECT* object) { Combat_SpawnIdleAmbientParticle(ARG); }
std::uint64_t Run(BYTE* memory) {
    auto* object=reinterpret_cast<OBJECT*>(memory+16);
    std::uint64_t digest=14695981039346656037ull;
    const DWORD zBits[]={0u,0x80000000u,0x3f800000u,0xc2c80000u,0x7fc00001u,0x7f800000u};
    for(unsigned trial=0;trial<65536;++trial) {
        std::memset(memory,0x5a,sizeof(OBJECT)+32);
        object->CurrentAction=static_cast<BYTE>(trial%3==0?2:trial&255);
        object->Position[0]=static_cast<int>(trial%2048)*0.25f-200.0f;
        object->Position[1]=static_cast<int>((trial*73)%2048)*0.125f-100.0f;
        const DWORD z=zBits[trial%6]; std::memcpy(&object->Position[2],&z,4);
        randomValues[0]=static_cast<int>((trial*40503u)&32767); randomValues[1]=static_cast<int>((trial*73u+32767)&32767);
        randomIndex=0; calls=0; actualFlag=0; actualMode=0; actualAlpha=0; std::memset(actualVectors,0,sizeof(actualVectors));
        BYTE before[sizeof(OBJECT)+32]; std::memcpy(before,memory,sizeof(before));
        Call(object); const bool emit=object->CurrentAction==2;
        Require(calls==(emit?1:0) && randomIndex==(emit?2:0) && std::memcmp(before,memory,sizeof(before))==0);
        if(emit) {
            const float x=(static_cast<float>(randomValues[0]%200)+object->Position[0])-_DAT_005524f0;
            const float y=(static_cast<float>(randomValues[1]%200)+object->Position[1])-_DAT_005524f0;
            const float expected[]={x,object->Position[1],object->Position[2],1.0f,1.0f,0.0f,x,y,0.0f};
            Require(std::memcmp(actualVectors,expected,sizeof(expected))==0 && static_cast<DWORD>(actualFlag)==z && actualAlpha==0.0f && actualMode==randomValues[1]%200);
        }
        for(BYTE value : before) { digest^=value; digest*=1099511628211ull; }
        for(BYTE value : reinterpret_cast<const BYTE(&)[sizeof(actualVectors)]>(actualVectors)) { digest^=value; digest*=1099511628211ull; }
        for(int value : {calls,randomIndex,actualFlag,actualMode}) for(int shift=0;shift<32;shift+=8) { digest^=static_cast<BYTE>(static_cast<unsigned>(value)>>shift); digest*=1099511628211ull; }
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,sizeof(OBJECT)+32); Require(Run(high)==expected);
        for(std::size_t i=0;i<sizeof(OBJECT)+32;++i) Require(low[i]==0);
        std::puts("emisor sobre 4 GiB y alias bajo intacto");
    }
    std::printf("Combat_SpawnIdleAmbientParticle: 65536 estados; bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = reset_harness(revision).split("int ItemObjectAttributeImpl(", 1)[0]
    name = "_DAT_005524f0"
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
    prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    body = function(read_source("src/Combat/Combat_AttackStage.cpp", revision), "Combat_SpawnIdleAmbientParticle")
    arg = "object" if "OBJECT* object" in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix + SERVICES + body + MAIN.replace("ARG", arg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-combat-ambient-") as temp:
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
