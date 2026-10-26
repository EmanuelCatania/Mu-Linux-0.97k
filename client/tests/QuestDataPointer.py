#!/usr/bin/env python3
"""Lector Quest.bmd/XOR reales: 200 bloques fijos, destino y scratch nativos.

La conversión de nombres sólo valida sus 200 rangos/orden; no certifica
UTF-8, archivos truncados ni layout completo/constructor del gestor CSQuest.
"""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT, build_and_run, function, read_source

SERVICES = r'''
using uint=unsigned int;
using byte=unsigned char;
char DAT_005580ac[]="rb";
const char s__s___File_not_exist__00558094[]="archivo inexistente";
BYTE* scratch;
BYTE* quest;
bool failOpen=false, closed=false;
int allocations=0, frees=0, reads=0, names=0, errors=0;
FILE* crt_fopen(const char* path,const char* mode) { Require(std::strcmp(mode,"rb")==0); return failOpen?nullptr:std::fopen(path,mode); }
std::size_t crt_fread(void* buffer,int size,int count,int* file) {
    Require(buffer==scratch+16 && size==584 && count==1 && reads<200 && !closed); ++reads;
    const auto result=std::fread(buffer,size,count,reinterpret_cast<FILE*>(file)); Require(result==1); return result;
}
void* operator_new(int size) { Require(size==584 && allocations++==0); return scratch+16; }
void operator_delete(void* buffer) { Require(buffer==scratch+16 && frees++==0 && reads==200); }
int crt_fclose(FILE* file) { Require(frees==1 && !closed); closed=true; return std::fclose(file); }
void crt_sprintf(char*,const char* format) { Require(failOpen && std::strcmp(format,s__s___File_not_exist__00558094)==0); ++errors; }
namespace TextEngine {
void ConvertWindows1252InPlace(char* address,int size) {
    Require(closed && names<200 && size==32 && reinterpret_cast<BYTE*>(address)==quest+8+names*584+5); ++names;
}
}
'''

MAIN = r'''
uint Call(BYTE* destination,const char* path) { return CSQuest_OpenQuestScript(ARG,path); }
std::vector<BYTE> Read(const char* path) {
    std::ifstream file(path,std::ios::binary); Require(file.good());
    return {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
}
std::uint64_t Run(BYTE* memory,BYTE* buffer,const std::vector<BYTE>& file) {
    quest=memory+16; scratch=buffer; std::memset(memory,0x5a,116832+32); std::memset(scratch,0xa5,616);
    allocations=frees=reads=names=errors=0; closed=false; failOpen=false;
    Require(Call(quest,SAMPLE_PATH)==1 && allocations==1 && frees==1 && reads==200 && names==200 && errors==0 && closed);
    std::vector<BYTE> expected(116832+32,0x5a);
    for(int record=0;record<200;++record) for(int offset=0;offset<584;++offset)
        expected[16+8+record*584+offset]=static_cast<BYTE>(file[record*584+offset]^static_cast<BYTE>(bBuxCode[offset%3]));
    Require(std::memcmp(memory,expected.data(),expected.size())==0);
    for(int i=0;i<16;++i) Require(scratch[i]==0xa5 && scratch[600+i]==0xa5);
    for(int i=0;i<584;++i) Require(scratch[16+i]==expected[16+8+199*584+i]);
    const auto before=expected;
    failOpen=true; closed=false; allocations=frees=reads=names=errors=0;
    Require(Call(quest,"missing-quest.bmd")==0 && allocations==0 && frees==0 && reads==0 && names==0 && errors==1 && !closed);
    Require(std::memcmp(memory,before.data(),before.size())==0);
    std::uint64_t digest=14695981039346656037ull;
    for(BYTE value : expected) { digest^=value; digest*=1099511628211ull; }
    return digest;
}
int main() {
    const auto file=Read(SAMPLE_PATH); Require(file.size()==116800);
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* lowScratch=static_cast<BYTE*>(Reserve(0x75000000u)); Require(low && lowScratch);
    const auto expected=Run(low,lowScratch,file);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull)));
        auto* highScratch=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(high && highScratch);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,116832+32); if(mask&2) std::memset(lowScratch,0,616);
            Require(Run(mask&1?high:low,mask&2?highScratch:lowScratch,file)==expected);
            if(mask&1) for(int i=0;i<116832+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<616;++i) Require(lowScratch[i]==0);
        }
        std::puts("cuatro combinaciones gestor/scratch altos y bajos, aliases intactos");
    }
    std::printf("Quest.bmd: 116800 bytes, 200 registros/lecturas/nombres y error de apertura; %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix = PREAMBLE.split("char DAT_00559bb4", 1)[0].replace("0x10000", "0x100000")
    definitions = read_source("src/globals.cpp", revision)
    prefix += re.search(r"^char\s+bBuxCode\[3\]\s*=\s*[^;]+;", definitions, re.M).group()
    prefix += SERVICES
    prefix += function(read_source("src/Scene/Scene_CharSelect_Nav.cpp", revision), "BuxConvert_1")
    body = function(read_source("src/Item/Quest_Data.cpp", revision), "CSQuest_OpenQuestScript")
    sample = ROOT / "bin/Client/Data/Local/Quest.bmd"
    if not sample.is_file():
        raise FileNotFoundError(sample)
    arg = "destination" if "BYTE* handle" in body else "static_cast<int>(reinterpret_cast<std::uintptr_t>(destination))"
    return prefix + body + MAIN.replace("ARG", arg).replace("SAMPLE_PATH", json.dumps(str(sample)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-data-") as temp:
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
