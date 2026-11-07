#!/usr/bin/env python3
"""Cola real de 300 paquetes: bytes, selección de slot y punteros nativos."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

MAIN=r'''
#ifndef _WIN32
#define __fastcall
#endif
FUNCTIONS
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
template<class T>T Get(const BYTE* p){T value;std::memcpy(&value,p,sizeof(value));return value;}
BYTE* Pop(BYTE* context){return POP_CALL;}
void Push(BYTE* context,const BYTE* packet,int size){CPacketQueue_PushPacket(CTX,PACKET,size);}
void Clear(BYTE* context){Require(static_cast<std::uintptr_t>(NetContext_ClearSendBuffer(CTX))==reinterpret_cast<std::uintptr_t>(context));}
constexpr unsigned contextBytes=0x260000,slotStride=0x2008,slotStart=0x401c;
std::uint64_t Run(BYTE* memory,BYTE* packetMemory){
    BYTE* context=memory+16;const BYTE* packet=packetMemory+16;
    std::vector<BYTE> expected(contextBytes+32),source(0x2040);
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned i=0;i<source.size();++i)packetMemory[i]=source[i]=static_cast<BYTE>(i*37u+19u);
    std::memset(memory,0xa5,contextBytes+32);std::memcpy(expected.data(),memory,expected.size());
    Clear(context);std::memset(expected.data()+16+slotStart,0,300*slotStride);Put(expected.data()+16+0x4014,0);Put(expected.data()+16+0x4018,0);
    Require(std::memcmp(memory,expected.data(),expected.size())==0);
    const int lengths[]={0,1,2,3,4,5,31,32,33,8191,8192,8193};
    for(unsigned trial=0;trial<1024;++trial){
        std::memset(memory,0xa5,contextBytes+32);
        const int freeSlot=trial%5==0?-1:static_cast<int>((trial*137)%300);
        const int usedSlot=trial%7==0?-1:static_cast<int>((trial*193)%300);
        for(int i=0;i<300;++i)Put(context+slotStart+i*slotStride,freeSlot<0?2:(i==freeSlot?0:2));
        if(usedSlot>=0 && usedSlot!=freeSlot)Put(context+slotStart+usedSlot*slotStride,1);
        std::memcpy(expected.data(),memory,expected.size());const int size=lengths[trial%12];
        Push(context,packet,size);
        if(freeSlot>=0 && size<=8192){BYTE* slot=expected.data()+16+slotStart+freeSlot*slotStride;Put(slot,1);Put(slot+4,size);std::memcpy(slot+8,packet,size);}
        Require(std::memcmp(memory,expected.data(),expected.size())==0);
        int first=-1;for(int i=0;i<300;++i)if(Get<int>(expected.data()+16+slotStart+i*slotStride)==1){first=i;break;}
        BYTE* result=Pop(context);Require(result==(first<0?nullptr:context+slotStart+first*slotStride+8));
        if(first>=0)Put(expected.data()+16+slotStart+first*slotStride,0);
        Require(std::memcmp(memory,expected.data(),expected.size())==0 && std::memcmp(packetMemory,source.data(),source.size())==0);
        digest^=static_cast<unsigned>(first+1);digest*=1099511628211ull;
        if(freeSlot>=0)for(unsigned i=0;i<slotStride;++i){digest^=context[slotStart+freeSlot*slotStride+i];digest*=1099511628211ull;}
    }
    // Llenar la cola y vaciarla recorre los 300 slots, incluido el último.
    Clear(context);
    for(int i=0;i<300;++i)Push(context,packet,3+i%29);
    Push(context,nullptr,8193);
    for(int i=0;i<300;++i){BYTE* out=Pop(context);Require(out==context+slotStart+i*slotStride+8 && Get<int>(out-4)==3+i%29 && std::memcmp(out,packet,3+i%29)==0);}
    Require(Pop(context)==nullptr && std::memcmp(packetMemory,source.data(),source.size())==0);
    return digest;
}
int main(){
    auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+0x280000);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,0x290000);Require(Run(mask&1?high:low,(mask&2?high:low)+0x280000)==expected);
            if(mask&1)for(unsigned i=0;i<contextBytes+32;++i)Require(low[i]==0x7b);
            if(mask&2)for(unsigned i=0;i<0x2040;++i)Require(low[0x280000+i]==0x7b);
        }std::puts("cuatro combinaciones contexto/paquete altos y bajos; aliases intactos");
    }std::printf("Packet queue: 1024 casos y 300 slots, bytes/retornos %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    source=read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision)
    clear=function(source,'NetContext_ClearSendBuffer');push=function(source,'CPacketQueue_PushPacket');pop=function(source,'CWsctlc_GetReadMsg')
    current='uintptr_t param_1' in clear
    arg='reinterpret_cast<uintptr_t>(context)' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(context))'
    packet='packet' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(packet))'
    pop_call=f'CWsctlc_GetReadMsg({arg})' if current else f'reinterpret_cast<BYTE*>(static_cast<uintptr_t>(static_cast<unsigned>(CWsctlc_GetReadMsg({arg}))))'
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x300000')
    return prefix+MAIN.replace('FUNCTIONS',clear+push+pop).replace('POP_CALL',pop_call).replace('CTX',arg).replace('PACKET',packet)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-packet-queue-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
