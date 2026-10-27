#!/usr/bin/env python3
"""Direcciones nativas del helper legado; conserva su mapeo y saver inactivo."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
using longlong=long long;
ENTRY
char* table;
#define SkillAttribute (*reinterpret_cast<_SkillAttrEntry*>(table))
FILE* DAT_07d7806c;
const char DAT_005580ac[]="rb", DAT_005597d4[]="wb";
float ParserTokenNumber;
char TokenString[256];
FILE* opened;bool failOpen;unsigned openCalls,tokenCalls,step;int row;unsigned length;float number;
struct FileSystem{FILE* Open(const char* path,const char* mode){Require(std::strcmp(path,"fixture.txt")==0 && std::strcmp(mode,"rb")==0);++openCalls;return failOpen?nullptr:opened;}}fs;
namespace Platform{FileSystem& GetFileSystem(){return fs;}}
int GetToken(){++tokenCalls;switch(step++){case 0:return 0;case 1:ParserTokenNumber=number;return 1;case 2:for(unsigned i=0;i<length;++i)TokenString[i]=static_cast<char>('a'+(i+row)%26);TokenString[length]=0;return 0;case 3:return 3;case 4:return 2;default:Require(false);return 2;}}
void* operator_new(int){std::abort();}void operator_delete(void*){std::abort();}
void BuxConvert_0(BYTE*,int){std::abort();}int crt_fwrite(char*,int,int,int*){std::abort();}
'''
MAIN = r'''
void Hash(std::uint64_t& h,const BYTE* p,unsigned n){for(unsigned i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory){table=reinterpret_cast<char*>(memory+16);std::uint64_t hash=14695981039346656037ull;unsigned queries=0;opened=std::tmpfile();Require(opened!=nullptr);
 for(row=0;row<9;++row)for(length=0;length<256;++length){if(row==8 && length>155)continue;for(unsigned fraction=0;fraction<4;++fraction){
  number=static_cast<float>(row)+fraction*.25f;std::memset(memory,0xa5,2592);BYTE expected[2592];std::memcpy(expected,memory,2592);for(unsigned i=0;i<length;++i)expected[16+row*300+4+i]=static_cast<BYTE>('a'+(i+row)%26);expected[16+row*300+4+length]=0;
  failOpen=false;openCalls=tokenCalls=step=0;SkillAttribute_LoadNames("fixture.txt");Require(openCalls==1 && tokenCalls==5 && DAT_07d7806c==opened && std::memcmp(memory,expected,2592)==0);for(unsigned i=0;i<length;++i)Require(TokenString[i]==static_cast<char>('a'+(i+row)%26));Require(!TokenString[length]);
  Hash(hash,memory,2592);++queries;
 }}
 Require(queries==8816);BYTE before[2592];std::memcpy(before,memory,2592);failOpen=true;openCalls=tokenCalls=step=0;SkillAttribute_LoadNames("fixture.txt");Require(openCalls==1 && tokenCalls==0 && !DAT_07d7806c && std::memcmp(memory,before,2592)==0);
 openCalls=tokenCalls=0;SkillAttribute_SaveBin("fixture.txt");Require(openCalls==0 && tokenCalls==0 && std::memcmp(memory,before,2592)==0);Require(std::fseek(opened,0,SEEK_SET)==0);std::fclose(opened);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,2592);Require(Run(high)==expected);for(unsigned i=0;i<2592;++i)Require(low[i]==0x7b);std::puts("destino legado alto; alias bajo intacto y saver sin efectos");}std::printf("Legacy text: 8816 consultas %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision)
    header=read_source('src/globals.h',revision)
    entry=re.search(r'struct _SkillAttrEntry\s*\{[^}]+\};',header).group(0)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('ENTRY',entry)+'\n'+function(source,'SkillAttribute_LoadNames')+'\n'+function(source,'SkillAttribute_SaveBin')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):
        parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-legacy-text-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
