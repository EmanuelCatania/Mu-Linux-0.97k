#!/usr/bin/env python3
"""Publicación de dos objetos chat y operaciones escalares reales contra x86.

Constructores y callback virtual controlados. No certifica nodos, vtables,
constructores reales ni layout chat x64. Conserva las reglas actuales del port.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <type_traits>
using undefined4=std::uint32_t; using std::uintptr_t;
BYTE* constructed[2]; int constructorCalls,scrollCalls; BYTE* activeWidget;
void* ChatListBox_Construct() { Require(constructorCalls++==0); return constructed[0]; }
void* ChatListBox_ConstructWhisper() { Require(constructorCalls++==1); return constructed[1]; }
template<class T> std::uintptr_t Address(T value) {
    if constexpr(std::is_pointer<T>::value) return reinterpret_cast<std::uintptr_t>(value);
    else return static_cast<std::uintptr_t>(value);
}
static void ChatLB_ScrollBy0(int* ptr) { Require(reinterpret_cast<BYTE*>(ptr)==activeWidget); ++scrollCalls; }
'''

MAIN = r'''
template<class T> void Put(BYTE* ptr,T value) { std::memcpy(ptr,&value,sizeof(value)); }
void Clear(BYTE* object) { Object_ClearMembers(CLEAR_ARG); }
void Resize(BYTE* object) { ChatListBox_ScrollByN(SIZE_ARG); }
std::uint64_t RunWidgets(BYTE* memory,BYTE* whisperMemory) {
    constexpr std::size_t mainSize=1480+32,whisperSize=188+32;
    BYTE* widget=memory+16; BYTE* whisper=whisperMemory+16; activeWidget=widget;
    const int rows[]={-2147483647-1,-1,0,1,2,3,4,5,6,9,12,15,18,21,24,27,28,29,30,31,32,2147483647};
    const DWORD flags[]={0,1,2,255,0xffffffffu};
    const std::uint32_t bits[]={0,1,0x80000000u,0x80000001u,0x7fc01234u,0x7f800000u,0xff800000u,0xffffffffu,0x3f800000u};
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(memory,static_cast<int>(trial&255),mainSize); std::memset(whisperMemory,static_cast<int>((trial>>8)&255),whisperSize);
        constructed[0]=trial%17==0?nullptr:widget; constructed[1]=trial%19==0?nullptr:whisper;
        constructorCalls=0; Publish(); Require(constructorCalls==2);
        Require(Address(DAT_055c9ff0)==reinterpret_cast<std::uintptr_t>(constructed[0]) && Address(DAT_055c9ff4)==reinterpret_cast<std::uintptr_t>(constructed[1]));
        BYTE expected[mainSize],expectedWhisper[whisperSize]; std::memcpy(expected,memory,mainSize); std::memcpy(expectedWhisper,whisperMemory,whisperSize);
        const std::uint32_t a=bits[trial%9],b=trial%3==0?bits[(trial/9)%9]:trial*0x97000001u;
        Object_SetRectFields(widget,a,b); Put(expected+16+44,a); Put(expected+16+48,b);
        Require(std::memcmp(expected,memory,mainSize)==0);
        Object_SetRectFields(whisper,b,a); Put(expectedWhisper+16+44,b); Put(expectedWhisper+16+48,a);
        Require(std::memcmp(expectedWhisper,whisperMemory,whisperSize)==0);
        Clear(widget); std::memset(expected+16+200,0,1280);
        Require(std::memcmp(expected,memory,mainSize)==0 && std::memcmp(expectedWhisper,whisperMemory,whisperSize)==0);
        const int initialRows=rows[(trial/5)%22]; const DWORD initialFlag=flags[trial%5]; const int marker=trial%7==0?0:1;
        Put(widget,marker); Put(widget+140,initialRows); Put(expected+16,marker); Put(expected+16+140,initialRows);
        g_bUseChatListBox=initialFlag; scrollCalls=0;
        int wantedRows=initialRows; DWORD wantedFlag=initialFlag;
        if(marker) {
            if(initialRows==3) { wantedRows=6; coverage|=1; }
            else if(initialRows==30) { wantedRows=6; wantedFlag=0; coverage|=2; }
            else if(initialRows>=6 && initialRows<=27 && initialRows%3==0) {
                wantedRows=initialFlag==1?30:3; wantedFlag=1; coverage|=initialFlag==1?4:8;
            } else if(initialRows>=31) { wantedRows=6; wantedFlag=1; coverage|=16; }
            else coverage|=32;
        } else coverage|=64;
        Resize(widget); Put(expected+16+140,wantedRows);
        Require(scrollCalls==(marker?1:0) && g_bUseChatListBox==wantedFlag && std::memcmp(expected,memory,mainSize)==0);
        const DWORD flagBefore=g_bUseChatListBox; const int callsBefore=scrollCalls; Resize(nullptr);
        Require(g_bUseChatListBox==flagBefore && scrollCalls==callsBefore);
        if(trial%2) { Object_SetRectFields(widget,b,a); Put(expected+16+44,b); Put(expected+16+48,a); }
        Require(std::memcmp(expected,memory,mainSize)==0 && std::memcmp(expectedWhisper,whisperMemory,whisperSize)==0);
        for(std::size_t i=0;i<mainSize;++i) { digest^=memory[i]; digest*=1099511628211ull; }
        for(std::size_t i=0;i<whisperSize;++i) { digest^=whisperMemory[i]; digest*=1099511628211ull; }
        digest^=static_cast<unsigned>(g_bUseChatListBox); digest*=1099511628211ull;
    }
    Require(coverage==127); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* whisper=static_cast<BYTE*>(Reserve(0x75000000u)); Require(low && whisper);
    const auto expected=RunWidgets(low,whisper);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highWhisper=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(high && highWhisper);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,1512); if(mask&2) std::memset(whisper,0,220);
            Require(RunWidgets(mask&1?high:low,mask&2?highWhisper:whisper)==expected);
            if(mask&1) for(int i=0;i<1512;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<220;++i) Require(whisper[i]==0);
        }
        std::puts("cuatro combinaciones historial/susurro altas y bajas, aliases intactos");
    }
    std::printf("Chat widgets: 32768 casos, publicacion/rect/limpieza/tamano/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None, legacy_function=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0]+SERVICES
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_055c9ff0","DAT_055c9ff4","g_bUseChatListBox"):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    source=read_source("src/Platform/ClientEntry.cpp",revision)
    assignments=[re.search(rf"\b{name}\s*=\s*[^;]*\b{ctor}\([^;]*;",source).group() for name,ctor in (("DAT_055c9ff0","ChatListBox_Construct"),("DAT_055c9ff4","ChatListBox_ConstructWhisper"))]
    prefix+="\nvoid Publish() {"+"".join(assignments)+"}\n"
    def body(path,name):
        selected=legacy_function[1] if legacy_function and legacy_function[0]==name else revision
        return function(read_source(path,selected),name)
    rect=body("src/Util/Misc.cpp","Object_SetRectFields")
    clear=body("src/Util/Misc.cpp","Object_ClearMembers")
    resize=body("src/Core/Runtime_Externs.cpp","ChatListBox_ScrollByN")
    clear_arg="object" if re.search(r"Object_ClearMembers\(void\s*\*",clear) else "static_cast<int>(reinterpret_cast<std::uintptr_t>(object))"
    size_arg="object" if "void* val" in resize else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix+rect+clear+resize+MAIN.replace("CLEAR_ARG",clear_arg).replace("SIZE_ARG",size_arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia DWORD/int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-chat-widget-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
