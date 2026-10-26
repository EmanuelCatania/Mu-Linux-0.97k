#!/usr/bin/env python3
"""Bases nativas de SetAttackSpeed; conserva el campo BMD legacy de 32 bits.

Compara bytes contra x86 y un oráculo de velocidades/acciones. La tabla de
acciones permanece baja: no certifica su puntero anidado ni layout BMD x64.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN = r'''
std::uint64_t Hash(const BYTE* bytes, std::size_t size, std::uint64_t digest) {
    for(std::size_t i=0; i<size; ++i) { digest ^= bytes[i]; digest *= 1099511628211ull; }
    return digest;
}
std::uint64_t Run(BYTE* stats, BYTE* models, BYTE* actions) {
    DAT_07cf1ff4=stats; DAT_05828d58=reinterpret_cast<std::uintptr_t>(models);
    std::memset(stats,0x5a,128); std::memset(models,0xa5,80000);
    const std::uint32_t address=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actions));
    std::memcpy(models+73368,&address,4);
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned speed=0; speed<65536; ++speed) {
        const std::uint16_t attack=static_cast<std::uint16_t>(speed);
        const std::uint16_t magic=static_cast<std::uint16_t>(speed*40503u+65535u);
        std::memcpy(stats+0x38,&attack,2); std::memcpy(stats+0x44,&magic,2);
        BYTE expected[1600], beforeStats[128];
        std::memset(actions,0x5a,1600); std::memcpy(expected,actions,1600); std::memcpy(beforeStats,stats,128);
        const float a=static_cast<float>(attack)*_DAT_005524bc;
        const float m=static_cast<float>(magic)*_DAT_005524bc;
        auto set=[&](int index,float value) { std::memcpy(expected+index*16+4,&value,4); };
        set(34,a+0.6f);
        for(int index=35;index<=55;++index) set(index,a+0.25f);
        for(int index : {46,47,48,49,54,55,56,57,59,64,65,66,67}) set(index,a+0.3f);
        set(60,a+0.24f); set(61,a+0.24f);
        set(58,a+0.27f); set(81,a+0.25f);
        const DWORD fury=0x3ec28f5c, vitality=0x3eae147b;
        std::memcpy(expected+62*16+4,&fury,4); std::memcpy(expected+63*16+4,&vitality,4);
        for(int index=82;index<=85;++index) set(index,a+0.29f);
        set(86,m+0.25f); set(87,a+0.3f); set(88,a+0.4f);
        set(89,a+0.6f); set(90,a+0.5f); set(91,a+0.3f);
        SetAttackSpeed();
        if(std::memcmp(actions,expected,1600)!=0) {
            for(int offset=0;offset<1600;++offset) if(actions[offset]!=expected[offset]) {
                std::fprintf(stderr,"velocidad %u, offset %d: %02x != %02x\n",speed,offset,actions[offset],expected[offset]); break;
            }
            Require(false);
        }
        Require(std::memcmp(stats,beforeStats,128)==0);
        digest=Hash(actions,1600,digest);
    }
    for(int i=0;i<80000;++i) {
        if(i>=73368 && i<73372) Require(models[i]==reinterpret_cast<const BYTE*>(&address)[i-73368]);
        else Require(models[i]==0xa5);
    }
    // Las tres guardas deben retornar sin tocar la tabla.
    BYTE before[1600]; std::memcpy(before,actions,1600);
    DAT_07cf1ff4=nullptr; SetAttackSpeed(); DAT_07cf1ff4=stats;
    DAT_05828d58=0; SetAttackSpeed(); DAT_05828d58=reinterpret_cast<std::uintptr_t>(models);
    std::memset(models+73368,0,4); SetAttackSpeed(); Require(std::memcmp(before,actions,1600)==0);
    return digest;
}
int main() {
    auto* stats=static_cast<BYTE*>(Reserve(0x76000000u));
    auto* models=static_cast<BYTE*>(Reserve(0x75000000u));
    auto* actions=static_cast<BYTE*>(Reserve(0x74000000u)); Require(stats && models && actions);
    const auto expected=Run(stats,models,actions);
    if(sizeof(void*)>4) {
        auto* highStats=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highModels=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(highStats && highModels);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(stats,0,128); if(mask&2) std::memset(models,0,80000);
            Require(Run(mask&1?highStats:stats,mask&2?highModels:models,actions)==expected);
            if(mask&1) for(int i=0;i<128;++i) Require(stats[i]==0);
            if(mask&2) for(int i=0;i<80000;++i) Require(models[i]==0);
        }
        std::puts("cuatro combinaciones de bases altas/bajas; tabla de acciones legacy baja");
    }
    std::printf("SetAttackSpeed: 65536 pares, oraculo de acciones y guardas; bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("BYTE* reservation;", 1)[0].replace("0x10000", "0x100000")
    header, definitions = read_source("src/globals.h", revision), read_source("src/globals.cpp", revision)
    for name in ("DAT_07cf1ff4", "DAT_05828d58", "_DAT_005524bc"):
        prefix += re.search(rf"extern [^;\n]*\b{name}\s*;", header).group()
        prefix += re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;", definitions, re.M).group()
    return prefix + function(read_source("src/Combat/Combat_LegacyAnimation.cpp", revision), "SetAttackSpeed") + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-attack-animation-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia de bytes con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
