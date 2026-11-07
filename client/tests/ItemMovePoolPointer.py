#!/usr/bin/env python3
"""Identidad de pools: setters/contexto/getter y NetRecv_24 reales contra x86.

Inserción/restauración, heurística de stacks, limpieza y callbacks controlados.
No certifica UI de pickup, mutación real de inventario, cifrado ni socket.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <type_traits>
BYTE* pools[5];
#define OffsetInventoryItems (*reinterpret_cast<BYTE(*)[4352]>(pools[0]))
#define OffsetTradeItems (*reinterpret_cast<BYTE(*)[2176]>(pools[1]))
#define OffsetMixItems (*reinterpret_cast<BYTE(*)[2176]>(pools[2]))
#define OffsetWarehouseItems (*reinterpret_cast<BYTE(*)[8160]>(pools[3]))
#define Inventory (*reinterpret_cast<BYTE(*)[4352]>(pools[4]))
#define dword_7EA9800 DAT_07ea9800
alignas(ITEM) char DAT_07e91350[68];
bool s_bPacketAfterEquipmentItem;
BYTE s_byPacketAfterEquipmentItem[4];
bool mergeAck;
struct Event { int kind; std::vector<int> args; std::vector<BYTE> bytes; };
std::vector<Event> events;
void Add(int kind,std::initializer_list<int> args,const void* bytes=nullptr,std::size_t count=0) {
    const auto* p=static_cast<const BYTE*>(bytes); events.push_back({kind,args,count?std::vector<BYTE>(p,p+count):std::vector<BYTE>()});
}
int PoolId(const BYTE* pool) {
    for(int i=0;i<5;++i) if(pool==pools[i]) return i;
    if(!pool) return 5;
    Require(reinterpret_cast<std::uintptr_t>(pool)==0x123); return 6;
}
template<class T> std::uintptr_t Address(T value) {
    if constexpr(std::is_pointer<T>::value) return reinterpret_cast<std::uintptr_t>(value);
    else return static_cast<std::uintptr_t>(value);
}
template<class... Args> void NetLog(const char*,Args...) {}
int ConvertItemType(BYTE* wire) { return wire[0]; }
bool ItemMove_LooksLikeStackMerge(BYTE* pool,int slot,BYTE* item) {
    Require(item==reinterpret_cast<BYTE*>(DAT_07e91350)); Add(1,{PoolId(pool),slot,mergeAck}); return mergeAck;
}
void ItemMove_RestoreSlot(BYTE* pool,int slot,const BYTE* item) { Add(2,{PoolId(pool),slot},item,68); }
void ItemMove_ClearPickedState();
void PlayBuffer(int id,const void* owner,int flags) { Require(id==29 && owner==nullptr && flags==0); Add(4,{}); }
void ReceiveTradeExit97k(BYTE* packet,int size) { Require(size==4); Add(5,{},packet,4); }
void SeedQuickPotionTypesFromInventory() { Add(6,{}); }
void InsertInventoryItem(BYTE* pool,int w,int h,int slot,BYTE* wire,int first) { Add(7,{PoolId(pool),w,h,slot,first},wire,ITEM_INFO_SIZE); }
'''

MAIN = r'''
std::uint64_t RunMoves(BYTE* low,BYTE* high,unsigned mask) {
    constexpr std::size_t bytes=0x10000;
    std::memset(low,0xa5,bytes); if(high) std::memset(high,0xa5,bytes);
    std::vector<BYTE> expectedLow(bytes,0xa5),expectedHigh(bytes,0xa5);
    for(int i=0;i<5;++i) pools[i]=(mask&(1u<<i)?high:low)+16+i*0x3000;
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    const int slots[]={0,1,11,12,31,32,63,64,75,76,119,120,255};
    const int sizes[]={0,4,5,11,12,16};
    for(unsigned trial=0;trial<4096;++trial) {
        std::uint32_t seed=trial^0x970024u;
        auto next=[&]() { seed=seed*1664525u+1013904223u; return seed>>16; };
        const int sourceId=next()%7,targetId=next()%7;
        auto pool=[&](int id)->BYTE* { return id<5?pools[id]:id==5?nullptr:reinterpret_cast<BYTE*>(std::uintptr_t(0x123)); };
        BYTE* source=pool(sourceId); BYTE* target=pool(targetId);
        SetContext(source); Require(Address(DAT_07ea9800)==reinterpret_cast<std::uintptr_t>(source) && ReadContext()==source);
        if(trial%2) FromDisplay::InventoryMove_SetPendingPools(source,target);
        else FromMove::InventoryMove_SetPendingPools(source,target);
        Require(Address(g_ItemMoveSourcePool)==reinterpret_cast<std::uintptr_t>(source) && Address(g_ItemMoveTargetPool)==reinterpret_cast<std::uintptr_t>(target));
        BYTE message[16]; for(int i=0;i<16;++i) message[i]=static_cast<BYTE>(next());
        message[0]=0xc1; message[2]=0x24; message[3]=trial%2?0xff:static_cast<BYTE>(next()%255);
        message[4]=static_cast<BYTE>(slots[next()%13]);
        const int size=sizes[next()%6]; message[1]=static_cast<BYTE>(size);
        for(int i=0;i<68;++i) DAT_07e91350[i]=static_cast<char>(next());
        ITEM* picked=reinterpret_cast<ITEM*>(DAT_07e91350); picked->Durability=static_cast<BYTE>(next());
        DAT_07ea5b18=trial%5==0?0xffffffffu:static_cast<DWORD>(slots[next()%13]);
        g_ItemMoveTargetDurBefore=trial%5==0?-1:static_cast<int>(next()%256);
        mergeAck=message[4]>=12 && message[4]<76 && trial%3!=0;
        if(message[4]>=12 && message[4]<76) {
            const std::size_t offset=static_cast<std::size_t>(message[4]-12)*68+26;
            pools[0][offset]=static_cast<BYTE>(next());
            (mask&1?expectedHigh:expectedLow)[16+offset]=pools[0][offset];
        }
        s_bPacketAfterEquipmentItem=(trial/2)%2!=0;
        for(int i=0;i<4;++i) s_byPacketAfterEquipmentItem[i]=static_cast<BYTE>(next());
        BYTE beforeMessage[16],beforePicked[68]; std::memcpy(beforeMessage,message,16); std::memcpy(beforePicked,DAT_07e91350,68);
        std::vector<Event> expected;
        auto add=[&](int kind,std::initializer_list<int> args,const void* data=nullptr,std::size_t count=0) {
            const auto* p=static_cast<const BYTE*>(data); expected.push_back({kind,args,count?std::vector<BYTE>(p,p+count):std::vector<BYTE>()});
        };
        const int src=sourceId==5?0:sourceId,dst=targetId==5?0:targetId;
        if(message[3]==255) {
            add(1,{dst,message[4],mergeAck});
            if(!mergeAck) { add(2,{src,static_cast<int>(DAT_07ea5b18)},beforePicked,68); coverage|=1; }
            else if(g_ItemMoveTargetDurBefore>=0) {
                const int actual=pools[0][static_cast<std::size_t>(message[4]-12)*68+26];
                const int added=actual>g_ItemMoveTargetDurBefore?actual-g_ItemMoveTargetDurBefore:0;
                const int left=picked->Durability-added;
                if(left>0) { BYTE remaining[68]; std::memcpy(remaining,beforePicked,68); remaining[26]=static_cast<BYTE>(left); add(2,{src,static_cast<int>(DAT_07ea5b18)},remaining,68); coverage|=2; }
                else coverage|=4;
            } else coverage|=8;
        } else if(size>=12) {
            const int max=dst==0?76:dst==3?120:dst==1 || dst==2?32:64;
            if(message[4]<max) { add(7,{dst,8,dst==3?15:dst==1 || dst==2?4:8,message[4],dst==0 && message[4]>=12?0:1},message+5,7); coverage|=16; }
            else coverage|=32;
        } else coverage|=64;
        add(3,{}); add(4,{});
        if(s_bPacketAfterEquipmentItem) { add(5,{},s_byPacketAfterEquipmentItem,4); coverage|=128; }
        add(6,{}); events.clear();
        NetRecv_24(message,size,0xc1,0x24,trial%2!=0);
        Require(events.size()==expected.size());
        for(std::size_t i=0;i<events.size();++i) {
            const auto &a=events[i],&b=expected[i]; Require(a.kind==b.kind && a.args==b.args && a.bytes==b.bytes);
            digest^=static_cast<unsigned>(a.kind); digest*=1099511628211ull;
            for(int value:a.args) { digest^=static_cast<unsigned>(value); digest*=1099511628211ull; }
            for(BYTE value:a.bytes) { digest^=value; digest*=1099511628211ull; }
        }
        Require(Address(DAT_07ea9800)==0 && Address(g_ItemMoveSourcePool)==0 && Address(g_ItemMoveTargetPool)==0 && !s_bPacketAfterEquipmentItem);
        Require(std::memcmp(beforeMessage,message,16)==0 && std::memcmp(beforePicked,DAT_07e91350,68)==0);
    }
    if(coverage!=255) std::fprintf(stderr,"cobertura pools: %u\n",coverage);
    Require(coverage==255 && std::memcmp(expectedLow.data(),low,bytes)==0 && (!high || std::memcmp(expectedHigh.data(),high,bytes)==0));
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low); const auto expected=RunMoves(low,nullptr,0);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high);
        for(unsigned mask=1;mask<32;++mask) Require(RunMoves(low,high,mask)==expected);
        std::puts("32 combinaciones independientes de cinco pools altos/bajos y canarios intactos");
    }
    std::printf("ItemMove pools: 4096 casos, contexto/setters/ack/rechazo/merge/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0]
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_07ea9800","g_ItemMoveSourcePool","g_ItemMoveTargetPool","DAT_07ea5b18"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    move=read_source("src/Item/Item_Move.cpp",revision)
    prefix+=re.search(r"^int g_ItemMoveTargetDurBefore[^;]+;",move,re.M).group()+SERVICES
    for namespace,path in (("FromMove","src/Item/Item_Move.cpp"),("FromDisplay","src/Item/Item_Display.cpp")):
        prefix+="\nnamespace "+namespace+" {"+function(read_source(path,revision),"InventoryMove_SetPendingPools")+"}\n"
    click=read_source("src/Item/Item_ClickHandler.cpp",revision)
    setter=re.search(r"\bdword_7EA9800\s*=\s*[^;]*\binv_base[^;]*;",click).group()
    getter=re.search(r"BYTE\* srcPool\s*=\s*[^;]+;",function(click,"RestorePickedItemToSource")).group()
    prefix+="\nvoid SetContext(BYTE* inv_base) {"+setter+"}\nBYTE* ReadContext() {"+getter+" return srcPool; }\n"
    recv=read_source("src/Net/Recv/Recv_Item.cpp",revision)
    clear=function(recv,"ItemMove_ClearPickedState")
    assignments=[re.search(rf"\b{name}\s*=\s*0\s*;",clear).group() for name in ("DAT_07ea9800","g_ItemMoveSourcePool","g_ItemMoveTargetPool")]
    prefix+="\nvoid ItemMove_ClearPickedState() { Add(3,{});"+"".join(assignments)+"}\n"
    for name in ("ItemMove_GetPool","ItemMove_GetGridH","ItemMove_ToGridSlot","ItemMove_IsInventoryEquipSlot","NetRecv_24"):
        prefix+=function(recv,name)
    return prefix+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-move-pool-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
