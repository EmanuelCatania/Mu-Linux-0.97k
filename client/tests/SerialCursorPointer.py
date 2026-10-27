#!/usr/bin/env python3
"""Bootstrap de seriales real: comparación byte a byte con cursor nativo."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
using byte=BYTE;using uint=unsigned;using undefined1=BYTE;using code=uint(void*);
DWORD g_HashTableCtx[4];
#define MAIN_HASH_CLASS g_HashTableCtx[0]
#define DAT_055c9bd0 g_HashTableCtx[2]
#define DAT_055c9bd4 g_HashTableCtx[3]
struct CNetwork{BYTE prefix[16];char m_SendSerial;BYTE gap[3];DWORD m_ReceiveSerial;BYTE suffix[11];void InitializePacketSerials();};
CNetwork* active;BYTE* node;unsigned calls,start;bool receiveMissing;int DAT_055c9bf0;const char s_Hash_table_full______GetIndex_00558108[]="full";std::vector<DWORD> events;
uint Dispatch(void* key){Require(key==&active->m_SendSerial && calls<2);events.push_back(1);events.push_back(static_cast<BYTE>(active->m_SendSerial));++calls;return start;}
void CErrorReport_Write(int* context,const char* message){Require(context==&DAT_055c9bf0 && message==s_Hash_table_full______GetIndex_00558108);events.push_back(2);}
void* AntiTamper_HashNode(){events.push_back(3);return node;}
void HashTable_Insert(void* context,void* value,void* key){Require(context==&MAIN_HASH_CLASS && value==node && node[1]==1 && (key==&active->m_SendSerial || key==&active->m_ReceiveSerial));events.push_back(4);events.push_back(key==&active->m_SendSerial?0:1);}
void* HashTable_GetNode(void* context,void* key){Require(context==&MAIN_HASH_CLASS && key==&active->m_SendSerial);events.push_back(5);return node;}
void Packet_DecryptByte(void* key,void* value){Require(key==&active->m_SendSerial && value==node && node[1]<2);events.push_back(6);active->m_SendSerial=0x5a;}
void Packet_EncryptByte(void*,void*){Require(false);}
uint HashTable_GetIndex(void* context,void* key){Require(context==&MAIN_HASH_CLASS && key==&active->m_ReceiveSerial && calls==2);events.push_back(7);return receiveMissing?0xffffffffu:0;}
'''
MAIN=r'''
void Hash(std::uint64_t& h,const BYTE* p,unsigned n){for(unsigned i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* object,BYTE* shared,BYTE* low){active=reinterpret_cast<CNetwork*>(object+16);node=shared+16;const DWORD words[]={0,1,0x100,0x10000,0x1000000,0xffffffffu,0x7f010000},capacities[]={0,1,2,4},counts[]={0,1,127,255};auto* keys=reinterpret_cast<DWORD*>(low+8192);BYTE* vtable=low+12288;code* callback=&Dispatch;std::memcpy(vtable+12,&callback,sizeof(callback));MAIN_HASH_CLASS=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(vtable));DAT_055c9bd0=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(keys));std::uint64_t hash=14695981039346656037ull;
 for(unsigned trial=0;trial<4096;++trial){std::memset(object,0xa5,96);std::memset(shared,0x6c,1445);active->m_SendSerial=static_cast<BYTE>(trial);active->m_ReceiveSerial=~trial;node[1]=static_cast<BYTE>(counts[(trial/3)%4]);DAT_055c9bd4=capacities[trial%4];for(unsigned i=0;i<4;++i)keys[i]=words[(trial/4+i)%7];start=DAT_055c9bd4?(trial/7)%DAT_055c9bd4:0;receiveMissing=(trial/11)%2;BYTE expectedObject[96],expectedNode[1445],beforeVtable[24];DWORD beforeContext[4],beforeKeys[4];std::memcpy(expectedObject,object,96);std::memcpy(expectedNode,shared,1445);std::memcpy(beforeContext,g_HashTableCtx,16);std::memcpy(beforeKeys,keys,16);std::memcpy(beforeVtable,vtable,24);std::vector<DWORD> expected={1,static_cast<BYTE>(active->m_SendSerial)};int found=-1;for(unsigned i=0;i<DAT_055c9bd4;++i){const DWORD word=keys[(start+i)%DAT_055c9bd4];if(word==0 || word==1){found=int(word);break;}}
  if(found<0 || found==0){if(found<0)expected.push_back(2);expected.insert(expected.end(),{3,4,0});expectedNode[17]=1;}else{expected.push_back(5);expectedNode[17]=static_cast<BYTE>(expectedNode[17]+1);if(expectedNode[17]<2)expected.push_back(6);}expected.insert(expected.end(),{1,0});bool one=false;for(unsigned i=0;i<DAT_055c9bd4;++i)if(keys[(start+i)%DAT_055c9bd4]==1){one=true;break;}if(!one)expected.push_back(2);expected.push_back(7);if(receiveMissing){expected.insert(expected.end(),{3,4,1});expectedNode[17]=1;}expectedObject[16+offsetof(CNetwork,m_SendSerial)]=0;std::memset(expectedObject+16+offsetof(CNetwork,m_ReceiveSerial),0,sizeof(active->m_ReceiveSerial));events.clear();calls=0;active->InitializePacketSerials();Require(calls==2 && events==expected && std::memcmp(object,expectedObject,96)==0 && std::memcmp(shared,expectedNode,1445)==0 && std::memcmp(keys,beforeKeys,16)==0 && std::memcmp(g_HashTableCtx,beforeContext,16)==0 && std::memcmp(vtable,beforeVtable,24)==0);Hash(hash,object,96);Hash(hash,shared,1445);for(auto value:events)Hash(hash,reinterpret_cast<BYTE*>(&value),4);
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096,low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){if(mask&1)std::memset(low,0x7b,96);if(mask&2)std::memset(low+4096,0x7b,1445);Require(Run((mask&1)?high:low,(mask&2)?high+4096:low+4096,low)==expected);if(mask&1)for(unsigned i=0;i<96;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<1445;++i)Require(low[4096+i]==0x7b);}std::puts("objeto/nodo altos y cursor de pila nativo; estado/eventos/aliases iguales");}std::printf("Serial cursor: 4096 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+function(read_source('src/Net/Net_Connect.cpp',revision),'CNetwork::InitializePacketSerials')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-serial-cursor-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
