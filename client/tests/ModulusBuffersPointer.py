#!/usr/bin/env python3
"""Encode/Decode reales: consultas, bloques, padding y errores con servicios controlados."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using WORD=std::uint16_t;
DWORD g_SimpleModulusCS[17],g_SimpleModulusSC[17];
BYTE* input;BYTE* output;int length,failAt,failCode,pattern;unsigned calls;
void CSimpleModulus_EncryptBlock(DWORD* context,BYTE* dst,WORD* src,BYTE take){Require(context==g_SimpleModulusCS && dst==output+calls*11);const unsigned remaining=length-calls*8;Require(take==(remaining<8?remaining:8));for(unsigned i=0;i<8;++i)Require(reinterpret_cast<BYTE*>(src)[i]==(i<take?input[calls*8+i]:0));for(unsigned i=0;i<11;++i)dst[i]=static_cast<BYTE>((i<8?reinterpret_cast<BYTE*>(src)[i]:take)^(calls*13+i*7));++calls;}
int CSimpleModulus_DecryptBlock(DWORD* context,BYTE* dst,BYTE* src){Require(context==g_SimpleModulusSC && dst==output+calls*8 && src==input+calls*11);const int index=calls++;for(unsigned i=0;i<8;++i)dst[i]=static_cast<BYTE>(src[i]^src[10]^i);return index==failAt?failCode:(index+pattern)%9;}
'''
MAIN=r'''
void Hash(std::uint64_t& h,unsigned value){for(unsigned i=0;i<4;++i){h^=(value>>(8*i))&255;h*=1099511628211ull;}}
std::uint64_t Run(BYTE* dst,BYTE* src){input=src+16;output=dst+16;std::uint64_t hash=14695981039346656037ull;unsigned queries=0;const int failures[]={-1,0,1,2,15,31};
 for(pattern=0;pattern<4;++pattern)for(length=-32;length<=257;++length){
  for(unsigned i=0;i<640;++i)src[i]=static_cast<BYTE>(i*37+pattern*71);std::vector<BYTE> source(src,src+640);std::memset(dst,0xa5,640);BYTE expected[640];std::memcpy(expected,dst,640);calls=0;Require(Encode(nullptr,nullptr,length)==11*((length+7)/8) && calls==0);++queries;
  const int blocks=length>0?(length+7)/8:0;for(int block=0;block<blocks;++block){int take=length-block*8;if(take>8)take=8;for(unsigned i=0;i<11;++i)expected[16+block*11+i]=static_cast<BYTE>((i<8?(i<static_cast<unsigned>(take)?input[block*8+i]:0):take)^(block*13+i*7));}
  Require(Encode(output,input,length)==11*((length+7)/8) && calls==static_cast<unsigned>(blocks) && std::memcmp(dst,expected,640)==0 && std::memcmp(src,source.data(),640)==0);Hash(hash,calls);for(auto byte:expected)Hash(hash,byte);++queries;
  calls=0;Require(Decode(nullptr,nullptr,length,pattern)==8*((length+10)/11) && calls==0);++queries;
  for(int failure:failures)for(int code:{-1,-7}){failAt=failure;failCode=code;std::memset(dst,0xa5,640);std::memcpy(expected,dst,640);calls=0;int result=0,executed=0;const int count=length>0?(length+10)/11:0;for(int block=0;block<count;++block){for(unsigned i=0;i<8;++i)expected[16+block*8+i]=static_cast<BYTE>(input[block*11+i]^input[block*11+10]^i);++executed;if(block==failure){result=code;break;}result+=(block+pattern)%9;}
   const int unused=pattern==0?0:pattern==1?-1:pattern==2?0x7fffffff:static_cast<int>(0x80000000u);Require(Decode(output,input,length,unused)==result && calls==static_cast<unsigned>(executed) && std::memcmp(dst,expected,640)==0 && std::memcmp(src,source.data(),640)==0);Hash(hash,result);Hash(hash,calls);for(auto byte:expected)Hash(hash,byte);++queries;
  }
 }Require(queries==17400);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+2048);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){BYTE* dst=(mask&1)?high:low;BYTE* src=(mask&2)?high+2048:low+2048;if(mask&1)std::memset(low,0x7b,640);if(mask&2)std::memset(low+2048,0x7b,640);Require(Run(dst,src)==expected);if(mask&1)for(unsigned i=0;i<640;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<640;++i)Require(low[2048+i]==0x7b);}std::puts("origen/destino altos; consultas, bloques, padding y errores iguales");}std::printf("Modulus buffers: 17400 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    encode=function(source,'CSimpleModulus_Encode');decode=function(source,'CSimpleModulus_Decode')
    encargs='dst,src,size' if 'BYTE* a1' in encode else '(int)dst,src,size'
    decargs='dst,src,size,unused' if 'BYTE* a1' in decode else '(int)dst,(int)src,size,unused'
    wrapper='\nint Encode(BYTE* dst,BYTE* src,int size){return CSimpleModulus_Encode('+encargs+');}\nint Decode(BYTE* dst,BYTE* src,int size,int unused){return CSimpleModulus_Decode('+decargs+');}\n'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+encode+decode+wrapper+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-modulus-buffers-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
