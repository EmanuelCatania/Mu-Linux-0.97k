#!/usr/bin/env python3
"""Helpers escalares reales de tela; no construye widget, nodos de lista ni vtables."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <cmath>
#include <initializer_list>
#ifndef _WIN32
#define __fastcall
#endif
unsigned currentTrial;
#define Require(...) do {if(!(__VA_ARGS__)){std::fprintf(stderr,"cloth assertion line %d trial %u\n",__LINE__,currentTrial);std::exit(1);}}while(false)
'''
MAIN = r'''
template<class T> void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
template<class T> T Get(const BYTE* p){T value;std::memcpy(&value,p,sizeof(value));return value;}
void Zero(BYTE* node){SpringNode_ZeroFields(ZERO_ARG);}
float Delta(BYTE* a,BYTE* b,float* out){return SpringNode_Delta(a,B_ARG,out);}
int Range(BYTE* a,BYTE* b,const float* range){return Cloth_SpringRange(A_ARG,B_ARG,range);}
void Equal(BYTE* a,BYTE* b,float rest){Cloth_SpringEqual(A_ARG,B_ARG,rest);}
void Forces(BYTE* a,int distance,short flags){ClothNode_ApplyForces(A_ARG,distance,flags);}
unsigned char Flush(BYTE* a){return static_cast<unsigned char>(VerletSystem_Flush(A_ARG));}
void Correct(BYTE* a,const float* correction){ClothNode_AddCorrection(A_ARG,correction);}
void Hash(std::uint64_t& digest,const void* bytes,std::size_t size){const BYTE* p=static_cast<const BYTE*>(bytes);while(size--){digest^=*p++;digest*=1099511628211ull;}}
void Fill(BYTE* memory,unsigned trial){
    for(unsigned i=0;i<96;++i)memory[i]=static_cast<BYTE>((trial*37+i*19)^(trial>>(i%15)));
    BYTE* node=memory+16;
    for(int field:{4,16,28,48})for(int axis=0;axis<3;++axis)Put(node+field+axis*4,static_cast<float>(static_cast<int>((trial*7+field+axis*13)%65)-32)*0.125f);
    node[40]=(trial/8)%4==0?0:(trial/8)%4==1?1:(trial/8)%4==2?128:255;
    Put(node+44,trial%5==0?-3:static_cast<int>(trial%5));
}
std::uint64_t Run(BYTE* first,BYTE* second,BYTE* vectorMemory){
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<32768;++trial){
        currentTrial=trial;
        Fill(first,trial);Fill(second,trial*3+11);std::memset(vectorMemory,0xa5,96);
        BYTE *a=first+16,*b=second+16;float* vector=reinterpret_cast<float*>(vectorMemory+16);
        BYTE beforeA[96],beforeB[96];std::memcpy(beforeA,first,96);std::memcpy(beforeB,second,96);
        const float rest=static_cast<float>(trial%17)*0.25f;
        const unsigned path=trial%8;
        const bool pinned=(a[40]&1)!=0;
        if(path==0){
            Zero(a);
            for(int field:{4,16,28,48})std::memset(beforeA+16+field,0,12);
            std::memset(beforeA+16+44,0,4);beforeA[16+40]=0;
            Require(std::memcmp(first,beforeA,96)==0 && std::memcmp(second,beforeB,96)==0);
        }else if(path==1){
            const float d=Delta(a,b,vector);Require(d>=0);
            for(int axis=0;axis<3;++axis)Require(vector[axis]==Get<float>(a+28+axis*4)-Get<float>(b+28+axis*4));
            Require(std::memcmp(first,beforeA,96)==0 && std::memcmp(second,beforeB,96)==0);
            for(unsigned i=0;i<96;++i)if(i<16 || i>=28)Require(vectorMemory[i]==0xa5);
            Hash(digest,&d,4);
        }else if(path==2){
            const float bounds[]={rest,rest+static_cast<float>(trial%3)*0.25f};
            float difference[3];const double distance=Delta(a,b,difference);const double limited=distance<0.001?0.001:distance;
            const int expected=pinned || limited<=bounds[1]*20.0;
            const int result=Range(a,b,bounds);Require(result==expected);Hash(digest,&result,4);
            Require(std::memcmp(first,beforeA,44)==0 && std::memcmp(first+56,beforeA+56,40)==0 && std::memcmp(second,beforeB,96)==0);
            if(pinned || !result || (limited>=bounds[0] && limited<=bounds[1]))Require(std::memcmp(first,beforeA,96)==0);
        }else if(path==3){
            const int ca=Get<int>(a+44),cb=Get<int>(b+44);Equal(a,b,rest);
            Require(Get<int>(a+44)==ca+1 && Get<int>(b+44)==cb+1);
            Require(std::memcmp(first,beforeA,60)==0 && std::memcmp(second,beforeB,60)==0);
        }else if(path==4){
            float correction[3]={rest,-rest,rest*0.5f};Correct(a,correction);
            Require(Get<int>(a+44)==Get<int>(beforeA+60)+1);
            for(int axis=0;axis<3;++axis)Require(Get<float>(a+48+axis*4)==correction[axis]+Get<float>(beforeA+64+axis*4));
            const int count=Get<int>(a+44);const auto result=Flush(a);
            const unsigned char expected=pinned?a[40]:count>0?static_cast<unsigned char>(reinterpret_cast<std::uintptr_t>(a)+60):static_cast<unsigned char>(count);
            Require(result==expected);Hash(digest,&result,1);
            if(pinned || count>0)for(int axis=0;axis<3;++axis)Require(Get<std::uint32_t>(a+48+axis*4)==0);
            Require(Get<int>(a+44)==(!pinned && count>0?0:count));
            Require(std::memcmp(second,beforeB,96)==0);
        }else if(path==5){
            DAT_00590af4=rest;DAT_00590af8=-rest;DAT_00590afc=rest*0.5f;_DAT_00590af0=rest*0.25f;
            Forces(a,static_cast<int>((trial/512)%12)-2,static_cast<short>(((trial/32)%16)<<8));
            Require(std::memcmp(first,beforeA,20)==0 && std::memcmp(first+32,beforeA+32,64)==0 && std::memcmp(second,beforeB,96)==0);
            if(pinned)for(int axis=0;axis<3;++axis)Require(Get<std::uint32_t>(a+4+axis*4)==0);
            ClothNode_Integrate(a,static_cast<float>(trial%4)*0.125f);
        }else if(path==6){
            Equal(a,b,rest);Flush(a);Flush(b);
        }else{
            Zero(a);SpringNode_SetPos(a,rest,-rest,rest*0.5f,trial%3);
            SpringNode_GetPos(a,vector);Require(vector[0]==rest && vector[1]==-rest && vector[2]==rest*0.5f);
            VerletNode_AddAccel(a,rest,rest,rest);ClothNode_Integrate(a,0.25f);
        }
        for(unsigned i=0;i<96;++i)if(i<16 || i>=76){Require(first[i]==beforeA[i]);Require(second[i]==beforeB[i]);}
        Hash(digest,first,96);Hash(digest,second,96);Hash(digest,vectorMemory,96);
    }return digest;
}
int main(){
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024,low+2048);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,3072);Require(Run(mask&1?high:low,(mask&2?high:low)+1024,(mask&4?high:low)+2048)==expected);
            for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<96;++i)Require(low[slot*1024+i]==0x7b);
        }std::puts("ocho combinaciones nodos/vector altos y bajos; aliases intactos");
    }std::printf("Cloth helpers: 32768 casos, ocho rutas %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Physics/Cloth_Simulation.cpp',revision)
    names=['Vec3_Length','SpringNode_Delta','Cloth_SpringRange','Cloth_SpringEqual','ClothNode_ApplyForces','VerletSystem_Flush','SpringNode_ZeroFields','SpringNode_SetPos','SpringNode_GetPos','VerletNode_AddAccel','ClothNode_Integrate','ClothNode_AddCorrection']
    code='\n'.join(function(source,name) for name in names)
    header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES
    for name in sorted(set(re.findall(r'\b_?DAT_[0-9a-f]+',code))):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',definitions,re.M).group()
    current='uintptr_t b' in function(source,'SpringNode_Delta')
    arg='reinterpret_cast<uintptr_t>(PTR)' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(PTR))'
    main=MAIN.replace('A_ARG',arg.replace('PTR','a')).replace('B_ARG',arg.replace('PTR','b'))
    main=main.replace('ZERO_ARG','node' if current else arg.replace('PTR','node'))
    return prefix+code+main


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-cloth-helpers-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
