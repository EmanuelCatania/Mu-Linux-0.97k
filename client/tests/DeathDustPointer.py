#!/usr/bin/env python3
"""Ráfaga real de muerte: bytes, 20 partículas, orden y puntero sobre 4 GiB.

OBJECT usa offsets x86 con punteros opacos. RNG/partículas controlados;
no certifica pool, render, layout nativo ni muerte integrada del personaje.
"""
import argparse
import os
from pathlib import Path
import tempfile

from ItemResetPointer import harness as reset_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
int randomValues[60], randomIndex=0, calls=0;
float actualVectors[20][9];
OBJECT* activeObject;
int TestRand() { Require(randomIndex<60); return randomValues[randomIndex++]; }
#define rand TestRand
int Particle_Spawn(int type,float* position,float* angle,float* light,int flag,float alpha,int mode) {
    Require(type==1221 && calls<20 && flag==1 && alpha==1.0f && mode==0);
    Require(angle==activeObject->Angle && light==activeObject->Light);
    std::memcpy(actualVectors[calls],position,12); std::memcpy(actualVectors[calls]+3,angle,12); std::memcpy(actualVectors[calls]+6,light,12);
    ++calls; return -1;
}
'''

MAIN = r'''
void Call(OBJECT* object) { Combat_SpawnDeathDustParticles(ARG); }
std::uint64_t Run(BYTE* memory) {
    auto* object=reinterpret_cast<OBJECT*>(memory+16); activeObject=object;
    std::uint64_t digest=14695981039346656037ull;
    const DWORD frames[]={0u,0x80000000u,0x40ffffffu,0x41000000u,0x41000001u,0x41080000u,0x410fffffu,0x41100000u,0x7fc00001u,0x7f800000u,0xff800000u};
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,0x5a,sizeof(OBJECT)+32);
        object->CurrentAction=static_cast<BYTE>(trial%3==0?6:trial&255);
        const DWORD frame=frames[trial%11]; std::memcpy(&object->AnimationFrame,&frame,4);
        for(int axis=0;axis<3;++axis) {
            object->Position[axis]=static_cast<int>((trial*73+axis*65521)%2048)*0.25f-200.0f;
            object->Angle[axis]=axis*45.0f; object->Light[axis]=axis*0.25f;
        }
        for(int i=0;i<60;++i) randomValues[i]=static_cast<int>((trial*40503u+i*73u)&32767);
        randomIndex=0; calls=0; std::memset(actualVectors,0,sizeof(actualVectors));
        BYTE expected[sizeof(OBJECT)+32]; std::memcpy(expected,memory,sizeof(expected));
        const bool emit=object->CurrentAction==6 && object->AnimationFrame>=8.0f && object->AnimationFrame<9.0f;
        if(emit) {
            const float light[]={1.0f,1.0f,1.0f}; std::memcpy(expected+16+offsetof(OBJECT,Light),light,sizeof(light));
        }
        Call(object);
        Require(calls==(emit?20:0) && randomIndex==(emit?60:0) && std::memcmp(expected,memory,sizeof(expected))==0);
        for(int particle=0;particle<calls;++particle) {
            float position[3];
            for(int axis=0;axis<3;++axis) position[axis]=static_cast<float>(static_cast<double>(randomValues[particle*3+axis]%(axis==2?32:64)-(axis==2?16:32))+object->Position[axis]);
            const float light[]={1.0f,1.0f,1.0f};
            Require(std::memcmp(actualVectors[particle],position,12)==0 && std::memcmp(actualVectors[particle]+3,object->Angle,12)==0 && std::memcmp(actualVectors[particle]+6,light,12)==0);
        }
        for(BYTE value : expected) { digest^=value; digest*=1099511628211ull; }
        const BYTE* bytes=reinterpret_cast<const BYTE*>(actualVectors);
        for(std::size_t i=0;i<sizeof(actualVectors);++i) { digest^=bytes[i]; digest*=1099511628211ull; }
        digest^=static_cast<unsigned>(calls); digest*=1099511628211ull;
    }
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); Require(low); const auto expected=Run(low);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); Require(high);
        std::memset(low,0,sizeof(OBJECT)+32); Require(Run(high)==expected);
        for(std::size_t i=0;i<sizeof(OBJECT)+32;++i) Require(low[i]==0);
        std::puts("emisor de muerte sobre 4 GiB y alias bajo intacto");
    }
    std::printf("Combat_SpawnDeathDustParticles: 32768 estados; bytes/eventos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = reset_harness(revision).split("int ItemObjectAttributeImpl(", 1)[0]
    body = function(read_source("src/Combat/Combat_AttackStage.cpp", revision), "Combat_SpawnDeathDustParticles")
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
    with tempfile.TemporaryDirectory(prefix="mu-death-dust-") as temp:
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
