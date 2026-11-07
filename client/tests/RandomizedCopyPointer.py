#!/usr/bin/env python3
"""Copias XOR reales de4/1412bytes: buffers nativos, azar controlado y solapamiento."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using byte=BYTE;using undefined4=std::uint32_t;using uint=unsigned;
BYTE PacketXorKey16[16];BYTE *bankLow,*bankHigh,*activeScratch;unsigned size,trial,randomCalls,allocations,deletions;
BYTE RandomByte(unsigned index){const int values[]={0,1,127,128,255,256,32767,1024};return static_cast<BYTE>(values[(trial+index)%8]);}
int _rand(){Require(allocations==1 && !deletions && randomCalls<size);return RandomByte(randomCalls++);}
void* operator_new(unsigned n){Require(!allocations++ && n==size);return activeScratch;}void operator_delete(void* p){Require(p==activeScratch && allocations==1 && !deletions++ && randomCalls==size);}
'''
MAIN=r'''
void Write(BYTE* low,BYTE* high,const BYTE* destination,const BYTE* values,unsigned n){const auto address=reinterpret_cast<std::uintptr_t>(destination),base=reinterpret_cast<std::uintptr_t>(bankLow);if(address>=base && address+n<=base+16384)std::memcpy(low+(address-base),values,n);else{Require(bankHigh);const auto other=reinterpret_cast<std::uintptr_t>(bankHigh);Require(address>=other && address+n<=other+16384);std::memcpy(high+(address-other),values,n);}}
void Hash(std::uint64_t& h,const BYTE* p,unsigned n){for(unsigned i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(unsigned mask){std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(trial=0;trial<256;++trial)for(unsigned large=0;large<2;++large)for(unsigned overlap=0;overlap<4;++overlap){size=large?1412:4;std::memset(bankLow,0xa5,16384);if(bankHigh)std::memset(bankHigh,0xa5,16384);BYTE* src=((mask&1)?bankHigh:bankLow)+16;BYTE* dst=((mask&2)?bankHigh:bankLow)+4112;activeScratch=((mask&4)?bankHigh:bankLow)+8208;if(overlap)dst=src+(overlap==1?0:overlap==2?4:-4);
  for(unsigned i=0;i<16;++i)PacketXorKey16[i]=static_cast<BYTE>(trial*13+i*17);for(unsigned i=0;i<size;++i)src[i]=static_cast<BYTE>(trial*37+i*19);std::vector<BYTE> input(src,src+size),encoded(size),randomized(size);for(unsigned i=0;i<size;++i){const unsigned transformed=((static_cast<unsigned>(input[i])+71)&255)^PacketXorKey16[i%16];encoded[i]=static_cast<BYTE>((transformed+35)&255);if(i+1<size)encoded[i]^=input[i+1];randomized[i]=RandomByte(i);}
  BYTE expectedLow[16384],expectedHigh[16384];std::memcpy(expectedLow,bankLow,16384);if(bankHigh)std::memcpy(expectedHigh,bankHigh,16384);else std::memset(expectedHigh,0xa5,16384);Write(expectedLow,expectedHigh,activeScratch,encoded.data(),size);Write(expectedLow,expectedHigh,src,randomized.data(),size);Write(expectedLow,expectedHigh,dst,encoded.data(),size);BYTE beforeKeys[16];std::memcpy(beforeKeys,PacketXorKey16,16);randomCalls=allocations=deletions=0;if(large)Packet_EncryptBuffer(dst,src);else Packet_EncryptDword(dst,src);
  Require(allocations==1 && deletions==1 && randomCalls==size && std::memcmp(bankLow,expectedLow,16384)==0 && (!bankHigh || std::memcmp(bankHigh,expectedHigh,16384)==0) && std::memcmp(PacketXorKey16,beforeKeys,16)==0);Hash(hash,dst,size);Hash(hash,src,size);Hash(hash,activeScratch,size);++queries;
 }Require(queries==2048);return hash;
}
int main(){bankLow=static_cast<BYTE*>(Reserve(0x26000000u));Require(bankLow);if(sizeof(void*)>4){bankHigh=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(bankHigh);}const auto expected=Run(0);if(bankHigh){for(unsigned mask=1;mask<8;++mask)Require(Run(mask)==expected);std::puts("origen/destino/scratch altos; solapamientos, bytes y canarios iguales");}std::printf("Randomized copy: 2048 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(source,'Packet_EncryptDword')+function(source,'Packet_EncryptBuffer')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-randomized-copy-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
