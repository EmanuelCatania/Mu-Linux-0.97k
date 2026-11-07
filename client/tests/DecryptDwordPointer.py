#!/usr/bin/env python3
"""Descifrado DWORD real: scratch nativo, bytes, solapamiento y canarios."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using byte=BYTE;using undefined4=std::uint32_t;using undefined=BYTE;using uint=unsigned;
BYTE PacketXorKey16[16],*scratch;unsigned allocations,deletions;
void* operator_new(unsigned n){Require(n==4 && allocations++==0 && !deletions);return scratch;}
void operator_delete(void* p){Require(p==scratch && allocations==1 && deletions++==0);}
'''
MAIN=r'''
std::uint64_t Run(BYTE* n,BYTE* a,BYTE* o){
 std::uint64_t hash=14695981039346656037ull;
 scratch=o+16;
 for(unsigned pattern=0;pattern<65536;++pattern)for(unsigned overlap=0;overlap<4;++overlap){
  std::memset(n,0x7b,128);if(a!=n)std::memset(a,0x7b,128);if(o!=n && o!=a)std::memset(o,0x7b,128);
  BYTE* src=n+32;BYTE* dst=a+64;
  if(overlap)dst=src+(overlap==1?0:overlap==2?4:-4);
  for(unsigned i=0;i<16;++i)PacketXorKey16[i]=static_cast<BYTE>((pattern>>8)+i*31);
  for(unsigned i=0;i<4;++i)src[i]=static_cast<BYTE>(pattern+i*73);
  BYTE decoded[4];std::memcpy(decoded,src,4);
  for(int i=3;i>=0;--i){
   unsigned value=decoded[i];if(i<3)value^=decoded[i+1];
   decoded[i]=static_cast<BYTE>((((value+256-35)&255)^PacketXorKey16[i])+185);
  }
  std::vector<BYTE> expectedN(n,n+128),expectedA(a,a+128),expectedO(o,o+128);
  auto write=[&](BYTE* p){
   for(unsigned i=0;i<4;++i){
    const auto address=reinterpret_cast<std::uintptr_t>(p+i);
    const auto bn=reinterpret_cast<std::uintptr_t>(n),ba=reinterpret_cast<std::uintptr_t>(a),bo=reinterpret_cast<std::uintptr_t>(o);
    if(address>=bn && address<bn+128)expectedN[address-bn]=decoded[i];
    if(address>=ba && address<ba+128)expectedA[address-ba]=decoded[i];
    if(address>=bo && address<bo+128)expectedO[address-bo]=decoded[i];
   }
  };
  write(scratch);write(dst);
  BYTE keys[16];std::memcpy(keys,PacketXorKey16,16);
  allocations=deletions=0;Packet_DecryptDword(dst,src);
  Require(allocations==1 && deletions==1 && std::memcmp(n,expectedN.data(),128)==0 && std::memcmp(a,expectedA.data(),128)==0 && std::memcmp(o,expectedO.data(),128)==0 && std::memcmp(keys,PacketXorKey16,16)==0);
  for(unsigned i=0;i<4;++i){hash^=dst[i];hash*=1099511628211ull;}
 }
 return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);
 const auto expected=Run(low,low+4096,low+8192);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<8;++mask){
   std::memset(low,0x7b,16384);
   Require(Run((mask&1?high:low),(mask&2?high:low)+4096,(mask&4?high:low)+8192)==expected);
   for(unsigned slot=0;slot<3;++slot)if(mask&(1u<<slot))for(unsigned i=0;i<128;++i)Require(low[slot*4096+i]==0x7b);
  }
  std::puts("origen/destino/scratch altos; solapamientos y aliases intactos");
 }
 std::printf("Decrypt DWORD: 262144 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Net/Crypto.cpp',revision),'Packet_DecryptDword')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-decrypt-dword-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
