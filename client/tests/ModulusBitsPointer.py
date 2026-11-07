#!/usr/bin/env python3
"""Copia OR de bits MSB con buffers nativos y Shift/GetByte reales."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __stdcall
#endif
unsigned allocations,deletions;void* scratch;
void* operator_new(unsigned n){++allocations;scratch=std::malloc(n);Require(scratch!=nullptr);return scratch;}
void operator_delete(void* p){Require(p==scratch);++deletions;std::free(p);}
#define delete__(p) operator_delete(p)
'''
MAIN=r'''
std::uint64_t Run(BYTE* dst,BYTE* src,bool overlap){std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(unsigned pattern=0;pattern<4;++pattern)for(unsigned from=0;from<32;++from)for(unsigned to=0;to<32;++to)for(int count=1;count<=32;++count){
  for(unsigned i=0;i<64;++i){src[i]=static_cast<BYTE>(pattern==0?0:pattern==1?255:(i*37+pattern*71));dst[i]=static_cast<BYTE>(pattern==0?0:pattern==1?0x55:(i*19+pattern*11));}
  BYTE source[64],expected[64];std::memcpy(source,src,64);std::memcpy(expected,dst,64);
  for(int i=0;i<count;++i)if(source[16+(from+i)/8] & (0x80u>>((from+i)%8)))expected[16+(to+i)/8]|=static_cast<BYTE>(0x80u>>((to+i)%8));
  allocations=deletions=0;const int result=Call(dst+16,to,src+16,from,count);Require(result==static_cast<int>(to)+count && allocations==1 && deletions==1 && std::memcmp(dst,expected,64)==0);if(!overlap)Require(std::memcmp(src,source,64)==0);
  for(auto byte:expected){hash^=byte;hash*=1099511628211ull;}++queries;
 }Require(queries==131072);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+256,false);const auto alias=Run(low,low,true);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){BYTE* dst=(mask&1)?high:low;BYTE* src=(mask&2)?high+256:low+256;if(mask&1)std::memset(low,0x7b,64);if(mask&2)std::memset(low+256,0x7b,64);Require(Run(dst,src,false)==expected);if(mask&1)for(unsigned i=0;i<64;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<64;++i)Require(low[256+i]==0x7b);}Require(Run(high,high,true)==alias);std::puts("origen/destino altos y alias in-place; bits y canarios iguales");}std::printf("Modulus bits: 131072 consultas por ruta %016llx/%016llx\n",static_cast<unsigned long long>(expected),static_cast<unsigned long long>(alias));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision).replace('__stdcall','__cdecl')
    add=function(source,'CSimpleModulus_AddBits').replace('__cdecl','__stdcall')
    args='dst,to,src,from,count' if 'void* a1' in add else '(int)dst,to,(int)src,from,count'
    wrapper='\nint Call(BYTE* dst,unsigned to,const BYTE* src,unsigned from,int count){return CSimpleModulus_AddBits('+args+');}\n'
    shift=function(source,'CSimpleModulus_Shift').replace('__cdecl','__stdcall')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(source,'CSimpleModulus_GetByteOfBit')+shift+add+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-modulus-bits-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
