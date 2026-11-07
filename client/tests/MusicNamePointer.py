#!/usr/bin/env python3
"""Puentes/cuerpos reales de selección musical: cadena nativa y estado x86.

Usa CSound y copia segura reales, con filesystem y reproducción controlados.
No certifica archivos/decodificación/audio reales ni selección de mapa integrada.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES = r'''
#include <string>
#include <cerrno>
#include <type_traits>
using std::uintptr_t;
CSound gSound;
struct Event { int kind; std::string name; };
std::vector<Event> events;
const char* activeName;
bool exists,plays;
FILE* crt_fopen(const char* name,const char* mode) {
    Require(name==activeName && std::strcmp(mode,"rb")==0); events.push_back({2,name});
    return exists && name[0]?reinterpret_cast<FILE*>(std::uintptr_t(0x1234)):nullptr;
}
int crt_fclose(FILE* file) { Require(file==reinterpret_cast<FILE*>(std::uintptr_t(0x1234))); events.push_back({3,""}); return 0; }
void CSound::StopMusic() { events.push_back({1,GetCurrentTrack()}); }
bool CSound::PlayMusic(const char* name) { Require(name==activeName); events.push_back({4,name}); return plays; }
'''

MAIN = r'''
void Play(const char* name,int enforce) { Music_PlayTrack(NAME_ARG,enforce); }
void Stop(const char* name,int enforce,bool wrapper) { if(wrapper) StopMp3(const_cast<char*>(name),enforce); else Music_StopTrack(NAME_ARG,enforce); }
std::uint64_t RunMusic(BYTE* firstMemory,BYTE* secondMemory) {
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    for(unsigned trial=0;trial<32768;++trial) {
        std::memset(firstMemory,0x5a,600); std::memset(secondMemory,0xa5,600);
        char* first=reinterpret_cast<char*>(firstMemory+16); char* second=reinterpret_cast<char*>(secondMemory+16);
        const int length=1+static_cast<int>((trial/21)%255);
        for(int i=0;i<length;++i) first[i]=static_cast<char>('a'+(trial+i)%26);
        first[length]=0; std::memcpy(second,first,length+1);
        const int scenario=trial%21;
        bool play=true,userStopped=false; DWORD enabled=1,destroy=0;
        int enforce=0,action=0; const char* initial=""; activeName=first; exists=true; plays=true;
        switch(scenario) {
        case 0: activeName=nullptr; initial="old"; break;
        case 1: enabled=0; initial="old"; break;
        case 2: destroy=1; enforce=1; initial="old"; break;
        case 3: userStopped=true; initial="old"; action=1; break;
        case 4: initial=second; break;
        case 5: initial="different"; action=2; break;
        case 6: exists=false; action=3; break;
        case 7: plays=false; action=4; break;
        case 8: action=5; break;
        case 9: play=false; initial=second; action=2; break;
        case 10: play=false; initial="different"; break;
        case 11: play=false; enabled=0; initial=second; break;
        case 12: play=false; enabled=0; enforce=-1; initial=second; action=2; break;
        case 13: play=false; activeName=nullptr; initial="old"; break;
        case 14: play=false; break;
        case 15: initial="different"; action=6; break;
        case 16: initial=first; break;
        case 17: enabled=0; enforce=2; action=5; break;
        case 18: play=false; destroy=0xffffffffu; initial=first; action=2; break;
        case 19: enabled=2; action=5; break;
        case 20: first[0]=0; exists=false; action=3; break;
        }
        gSound=CSound{}; gSound.SetMusicEnabled(enabled); gSound.SetMusicStoppedByUser(userStopped);
        char* current=const_cast<char*>(gSound.GetCurrentTrack()); std::memset(current,0x6b,256); std::strcpy(current,initial);
        if(scenario==16 || scenario==18) activeName=current;
        DAT_055ca018=destroy;
        BYTE beforeFirst[600],beforeSecond[600]; std::memcpy(beforeFirst,firstMemory,600); std::memcpy(beforeSecond,secondMemory,600);
        std::vector<BYTE> expectedObject(sizeof(gSound)); std::memcpy(expectedObject.data(),&gSound,sizeof(gSound));
        const auto trackOffset=reinterpret_cast<BYTE*>(current)-reinterpret_cast<BYTE*>(&gSound);
        char* expectedTrack=reinterpret_cast<char*>(expectedObject.data()+trackOffset);
        std::vector<Event> expected;
        const std::string incoming=activeName?activeName:"",oldTrack=initial;
        switch(action) {
        case 1: expectedTrack[0]=0; break;
        case 2: expected.push_back({1,oldTrack}); expectedTrack[0]=0; break;
        case 3: expected.push_back({2,incoming}); break;
        case 4: expected={{2,incoming},{3,""},{4,incoming}}; break;
        case 5: expected={{2,incoming},{3,""},{4,incoming}}; std::memcpy(expectedTrack,incoming.c_str(),incoming.size()+1); break;
        case 6: expected={{1,oldTrack},{2,incoming},{3,""},{4,incoming},{1,incoming}}; expectedTrack[0]=0; std::memcpy(expectedTrack+1,incoming.c_str()+1,incoming.size()); break;
        }
        events.clear();
        if(play) Play(activeName,enforce); else Stop(activeName,enforce,trial%2!=0);
        if(action==6) {
            Play(activeName,enforce); Play(activeName,enforce);
            activeName=gSound.GetCurrentTrack(); Stop(activeName,0,trial%2!=0);
        }
        Require(events.size()==expected.size());
        for(std::size_t i=0;i<events.size();++i) {
            Require(events[i].kind==expected[i].kind && events[i].name==expected[i].name);
            digest^=static_cast<unsigned>(events[i].kind); digest*=1099511628211ull;
            for(unsigned char value:events[i].name) { digest^=value; digest*=1099511628211ull; }
        }
        Require(DAT_055ca018==destroy && std::memcmp(expectedObject.data(),&gSound,sizeof(gSound))==0);
        Require(std::memcmp(beforeFirst,firstMemory,600)==0 && std::memcmp(beforeSecond,secondMemory,600)==0);
        for(int i=0;i<256;++i) { digest^=static_cast<BYTE>(current[i]); digest*=1099511628211ull; }
        coverage|=1u<<scenario;
    }
    Require(coverage==0x1fffffu); return digest;
}
int main() {
    if(sizeof(void*)>4) Require(reinterpret_cast<std::uintptr_t>(gSound.GetCurrentTrack())>0xffffffffu);
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* other=static_cast<BYTE*>(Reserve(0x75000000u)); Require(low && other);
    const auto expected=RunMusic(low,other);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highOther=static_cast<BYTE*>(Reserve(std::uint64_t(0x175000000ull))); Require(high && highOther);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,600); if(mask&2) std::memset(other,0,600);
            Require(RunMusic(mask&1?high:low,mask&2?highOther:other)==expected);
            if(mask&1) for(int i=0;i<600;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<600;++i) Require(other[i]==0);
        }
        std::puts("cuatro combinaciones de cadenas altas/bajas, alias interno y canarios intactos");
    }
    std::printf("Music names: 32768 casos, transiciones/IO/flags/estado %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0]
    prefix+=read_source("src/Sound/SoundManager.h",revision).replace("#pragma once","").replace('#include "Platform/PlatformHeaders.h"',"")
    prefix+=SERVICES
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    for name in ("DAT_055ca018",):
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    prefix+=re.search(r"^char\s+DAT_005580ac\[\][^;]+;",definitions,re.M).group()
    text=read_source("src/Platform/Text.h",revision)
    prefix+="\nnamespace Platform {"+text[text.index("constexpr std::size_t TruncateText"):text.index("inline int TextConcatSecure")]+"}\n"
    prefix+=function(read_source("src/Sound/SoundManager.cpp",revision),"CSound::SetMusicStoppedByUser")
    source=read_source("src/Sound/Music.cpp",revision)
    for name in ("CSound::StopTrack","CSound::PlayTrack","Music_PlayTrack","Music_StopTrack"):
        prefix+=function(source,name)
    prefix+=function(read_source("src/Core/Runtime_Linker.cpp",revision),"StopMp3")
    arg="name" if "const char* name" in function(source,"Music_PlayTrack").split("{",1)[0] else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(name))"
    return prefix+MAIN.replace("NAME_ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-music-name-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
