#!/usr/bin/env python3
"""Medición real de burbujas; fuente controlada y owner opaco de4 bytes (B2)."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
using LONG=std::int32_t;struct SIZE{LONG cx,cy;};
#endif
CHAT* active;
const char *nameText,*guildText,*wanted[4];
unsigned nameCalls,guildCalls,lengthCalls,measureCalls,heightCalls,dcCalls,factor;
int height;
void* dc=reinterpret_cast<void*>(sizeof(void*)>4?std::uintptr_t(0x123456780ull):std::uintptr_t(0x23456780u));
const char* ChatDisplayName(const CHAT* c){Require(c==active);++nameCalls;return nameText;}
const char* ChatDisplayGuild(const CHAT* c){Require(c==active);++guildCalls;return guildText;}
namespace Platform{int TextLength(const char* text){++lengthCalls;return static_cast<int>(std::strlen(text));}}
struct Font{
    void* GetTextDC(){++dcCalls;return dc;}
    bool MeasureText32(void* context,const char* text,int length,SIZE* out){Require(context==dc && measureCalls<4 && wanted[measureCalls]==text && length==static_cast<int>(std::strlen(text)));++measureCalls;out->cx=length*factor;out->cy=height;return true;}
    int GetHeight(){++heightCalls;return height;}
}gFont;
'''
MAIN=r'''
void Measure(CHAT* chat,int p2,int p3){FloatingLabel_MeasureText(ARG,p2,p3);}
void Fill(char* out,unsigned length,unsigned seed){for(unsigned i=0;i<length;++i)out[i]='a'+(seed+i*7)%26;out[length]=0;}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;const int times[]={-1,0,1,(-2147483647-1),2147483647};const int heights[]={0,1,15,31};
    for(unsigned trial=0;trial<32768;++trial){
        std::memset(memory,0xa5,628);active=reinterpret_cast<CHAT*>(memory+16);
        char name[65],guild[33];Fill(name,trial%65,trial);Fill(guild,(trial/3)%33,trial+1);Fill(active->ID,trial%24,trial+2);Fill(active->Guild,trial%12,trial+3);Fill(active->Text[0],trial%256,trial+4);Fill(active->Text[1],(trial*7)%256,trial+5);
        active->LifeTime[0]=times[trial%5];active->LifeTime[1]=times[(trial/5)%5];
        nameText=trial%2?name:active->ID;guildText=(trial/2)%2?guild:active->Guild;
        wanted[0]=nameText;wanted[1]=active->Text[0];wanted[2]=active->Text[1];wanted[3]=guildText;
        factor=trial%4+1;height=heights[(trial/4)%4];nameCalls=guildCalls=lengthCalls=measureCalls=heightCalls=dcCalls=0;
        const int combined=static_cast<int>(std::strlen(nameText)+std::strlen(guildText))*factor+3;
        const int first=static_cast<int>(std::strlen(active->Text[0]))*factor,second=static_cast<int>(std::strlen(active->Text[1]))*factor;
        const int lines=active->LifeTime[1]>0?3:active->LifeTime[0]>0?2:1;
        const int width=lines==1?combined:lines==2?(std::max)(combined,first):(std::max)(combined,(std::max)(first,second));
        BYTE expected[628];std::memcpy(expected,memory,628);std::memcpy(expected+16+576,&width,4);const int pixels=lines*height;std::memcpy(expected+16+580,&pixels,4);
        Measure(active,trial%2?2147483647:-1,trial%2?(-2147483647-1):0);
        Require(std::memcmp(memory,expected,628)==0 && nameCalls==2 && guildCalls==2 && lengthCalls==4 && measureCalls==4 && dcCalls==4 && heightCalls==(lines==3?2u:1u));
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,628);Require(Run(high)==expected);for(unsigned i=0;i<628;++i)Require(low[i]==0x7b);std::puts("burbuja alta: dimensiones/bytes/callbacks iguales; alias bajo intacto");}
    std::printf("Floating label: 32768 casos, lineas/dimensiones/canarios %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    header=read_source('src/UI/ChatTypes.h',revision)
    record=re.search(r'struct CHAT\s*\{.*?\};',header,re.S).group()
    # Vista x86 de prueba: Owner no se lee en esta función y permanece opaco.
    record=re.sub(r'CHARACTER\*\s+Owner;', 'BYTE opaqueOwner[4];',record)
    prefix='#define NOMINMAX\n'+PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#include <algorithm>\nusing vec3_t=float[3];\n#pragma pack(push,1)\n'+record+'\n#pragma pack(pop)\nstatic_assert(sizeof(CHAT)==596);static_assert(offsetof(CHAT,Width)==576);static_assert(offsetof(CHAT,Height)==580);\n'
    prefix+='\n'.join(re.findall(r'^inline (?:const )?CHAT\* AsChat[^\n]+',header,re.M))+SERVICES
    code=function(read_source('src/Render/HUD_Pass1.cpp',revision),'FloatingLabel_MeasureText')
    arg='chat' if 'CHAT* c' in code.split('{',1)[0] else 'static_cast<int>(reinterpret_cast<uintptr_t>(chat))'
    return prefix+code+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-floating-label-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
