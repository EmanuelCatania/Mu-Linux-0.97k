#!/usr/bin/env python3
"""Dos transformaciones BMD reales: base nativa, oráculo exacto y bytes x86.

Datos finitos diádicos y buffers solapados. No certifica BMD188 nativo,
carga/animación/render ni consumidores que todavía guardan punteros en int.
"""
import argparse
import os
from pathlib import Path
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#ifndef _WIN32
#define __fastcall
#endif
unsigned timerCalls;
namespace Platform { struct Timer { DWORD Ticks() { ++timerCalls; return timerCalls*777u; } } timer; Timer& GetTimer() { return timer; } }
'''

MAIN = r'''
std::uint64_t RunPosition(BYTE* modelMemory,BYTE* vectorMemory,BYTE* outputMemory) {
    std::uint64_t digest=14695981039346656037ull; timerCalls=0;
    for(unsigned trial=0;trial<32768;++trial) {
        const bool translate=(trial&1)!=0; const int alias=(trial/2)%4;
        std::memset(modelMemory,0x5a,256); std::memset(vectorMemory,0xa5,160); std::memset(outputMemory,0x6b,64);
        BYTE* model=modelMemory+16;
        auto* matrix=reinterpret_cast<float*>(vectorMemory+16); auto* pos=reinterpret_cast<float*>(vectorMemory+80);
        float* output=alias==0?reinterpret_cast<float*>(outputMemory+16):alias==1?pos:alias==2?pos+1:matrix;
        for(int i=0;i<12;++i) matrix[i]=static_cast<float>(static_cast<int>((trial*7+i*29)%17)-8)*0.5f;
        for(int i=0;i<3;++i) pos[i]=static_cast<float>(static_cast<int>((trial*31+i*7)%17)-8)*0.5f;
        const float scale=static_cast<float>(static_cast<int>((trial/8)%17)-8)*0.25f;
        std::memcpy(model+104,&scale,4);
        for(int i=0;i<3;++i) { const float origin=static_cast<float>(static_cast<int>((trial+i*19)%257)-128)*0.25f; std::memcpy(model+108+i*4,&origin,4); }
        BYTE beforeModel[256],beforeVectors[160],beforeOutput[64]; std::memcpy(beforeModel,modelMemory,256); std::memcpy(beforeVectors,vectorMemory,160); std::memcpy(beforeOutput,outputMemory,64);
        for(int implementation=0;implementation<2;++implementation) {
            std::memcpy(modelMemory,beforeModel,256); std::memcpy(vectorMemory,beforeVectors,160); std::memcpy(outputMemory,beforeOutput,64);
            alignas(float) BYTE expectedVectors[160],expectedOutput[64]; std::memcpy(expectedVectors,beforeVectors,160); std::memcpy(expectedOutput,beforeOutput,64);
            auto* m=reinterpret_cast<float*>(expectedVectors+16); auto* p=reinterpret_cast<float*>(expectedVectors+80);
            float* expected=alias==0?reinterpret_cast<float*>(expectedOutput+16):reinterpret_cast<float*>(expectedVectors+(reinterpret_cast<BYTE*>(output)-vectorMemory));
            float temp[3];
            for(int row=0;row<3;++row) {
                const float value=p[0]*m[row*4]+p[1]*m[row*4+1]+p[2]*m[row*4+2]+m[row*4+3];
                if(translate) temp[row]=value; else expected[row]=value;
            }
            if(translate) for(int row=0;row<3;++row) { float origin; std::memcpy(&origin,model+108+row*4,4); expected[row]=temp[row]*scale+origin; }
            const auto previousCalls=timerCalls;
            if(implementation==0) BMD_TransformPosition(translate?model:nullptr,matrix,pos,output,translate?static_cast<char>(trial%2?255:1):0);
            else BMD__TransformPosition(translate?model:nullptr,reinterpret_cast<float(*)[4]>(matrix),pos,output,translate);
            Require(timerCalls-previousCalls==(implementation==0 && translate?1u:0u));
            Require(std::memcmp(modelMemory,beforeModel,256)==0 && std::memcmp(vectorMemory,expectedVectors,160)==0 && std::memcmp(outputMemory,expectedOutput,64)==0);
            for(const BYTE* bytes:{vectorMemory,outputMemory}) {
                const int size=bytes==vectorMemory?160:64;
                for(int i=0;i<size;++i) { digest^=bytes[i]; digest*=1099511628211ull; }
            }
        }
    }
    return digest;
}
int main() {
    auto* model=static_cast<BYTE*>(Reserve(0x76000000u)); auto* vectors=static_cast<BYTE*>(Reserve(0x75000000u)); auto* output=static_cast<BYTE*>(Reserve(0x74000000u)); Require(model && vectors && output);
    const auto expected=RunPosition(model,vectors,output);
    if(sizeof(void*)>4) {
        auto* highModel=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highVectors=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); auto* highOutput=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(highModel && highVectors && highOutput);
        for(int mask=1;mask<8;++mask) {
            if(mask&1) std::memset(model,0,256); if(mask&2) std::memset(vectors,0,160); if(mask&4) std::memset(output,0,64);
            Require(RunPosition(mask&1?highModel:model,mask&2?highVectors:vectors,mask&4?highOutput:output)==expected);
            if(mask&1) for(int i=0;i<256;++i) Require(model[i]==0);
            if(mask&2) for(int i=0;i<160;++i) Require(vectors[i]==0);
            if(mask&4) for(int i=0;i<64;++i) Require(output[i]==0);
        }
        std::puts("ocho combinaciones modelo/vectores/salida altas y bajas, tres solapamientos y aliases intactos");
    }
    std::printf("TransformPosition: 32768 estados por implementacion, oraculo/timer/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0]+SERVICES
    source=read_source("src/Math/Math_3D.cpp",revision)
    if "AsModel(" in source:
        from LegacyModelLayout import VIEW
        prefix+=VIEW
    return prefix+function(source,"Vector_Transform")+function(source,"BMD_TransformPosition")+function(read_source("src/Math/Math_LegacyTransforms.cpp",revision),"BMD__TransformPosition")+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-model-position-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
