#!/usr/bin/env python3
"""Nombre de ítem real con OBJECT* y atributos altos, contra bytes x86.

Backend de texto, catálogo y fuentes controlados. OBJECT opaco; no certifica
layout nativo, raster real, carga de assets ni selección integrada del mundo.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from ItemConvertPointer import harness as convert_harness, SERVICES
from SceneObjectPointer import build_and_run, function, read_source

BACKEND = r'''
#include <string>
#ifndef _WIN32
#define __forceinline inline
using HGDIOBJ=void*; using HFONT=void*; using HDC=void*;
struct SIZE { std::int32_t cx,cy; };
#endif
struct OBJECT;
enum { FONT_NORMAL=0,FONT_BOLD=1 };
unsigned trialNumber; int catalogMode,activeModel;
std::vector<int> fontEvents;
std::vector<float> colors;
std::string label;
int labelX,labelY,labelCalls,measureCalls,textCalls;
int MouseX,MouseY;
struct Font {
    HDC GetTextDC() { return reinterpret_cast<HDC>(std::uintptr_t(0x456)); }
    HFONT GetFont(int type) { return reinterpret_cast<HFONT>(std::uintptr_t(0x1230+type)); }
    void SelectFont(HDC dc,HGDIOBJ font) {
        Require(dc==GetTextDC());
        const auto value=reinterpret_cast<std::uintptr_t>(font);
        Require(value==0x1230 || value==0x1231); fontEvents.push_back(static_cast<int>(value-0x1230));
    }
    void MeasureText(HDC dc,const char* text,int length,SIZE* size) {
        Require(dc==GetTextDC() && length==static_cast<int>(std::strlen(text)) && size); ++measureCalls;
        size->cx=length*7+trialNumber%4; size->cy=13;
    }
} gFont;
struct NameWindow { unsigned GetWidth() { const unsigned sizes[]={640,800,1024,1280}; return sizes[trialNumber%4]; } } nameWindow;
#define gWindow nameWindow
struct Catalog {
    int GetItemBehavior(int type) { return type; }
    int GetModelItemType(int model) { Require(model==activeModel); return catalogMode==0?-1:catalogMode==1?42:8192; }
} gContentCatalog;
namespace Platform { int TextLength(const char* text) { ++textCalls; return static_cast<int>(std::strlen(text)); } }
namespace Gfx { void Color3f(float r,float g,float b) { colors.push_back(r); colors.push_back(g); colors.push_back(b); } }
void RenderText(int x,int y,const char* text,int a,int b,SIZE* alignment) {
    Require(a==0 && b==0 && reinterpret_cast<std::uintptr_t>(alignment)==3); ++labelCalls;
    labelX=x; labelY=y; label=text;
}
'''

MAIN = r'''
void Name(OBJECT* object,int level,int option,bool sort) { RenderItemName(7,ARG,level,option,sort); }
std::uint64_t RunName(BYTE* memory,BYTE* tableMemory) {
    BYTE* object=memory+16; auto* table=reinterpret_cast<ITEM_ATTRIBUTE*>(tableMemory+64);
    DAT_055c9e44=reinterpret_cast<std::uintptr_t>(table); DAT_07d78068=DAT_055c9e44;
    std::memset(tableMemory,0xa5,8192*64+80);
    for(int i=0;i<8192;++i) std::snprintf(table[i].Name,sizeof(table[i].Name),"N%04d",i);
    for(int i=0;i<4000;++i) std::snprintf(GlobalText[i],300,"G%04d",i);
    std::memset(GlobalText[176],'L',240); GlobalText[176][240]=0;
    for(int i=0;i<64;++i) std::snprintf(SkillAttribute.Raw+i*40,40,"S%02d",i);
    auto* monsters=reinterpret_cast<MONSTER_SCRIPT*>(MonsterScript); std::memset(MonsterScript,0,sizeof(MonsterScript));
    for(int i=0;i<256;++i) { monsters[i].Type=static_cast<BYTE>(i); std::snprintf(monsters[i].Name,32,"M%03d",i); }
    std::vector<BYTE> beforeTable(tableMemory,tableMemory+8192*64+80);
    BYTE beforeSkill[2560],beforeMonster[sizeof(MonsterScript)]; std::memcpy(beforeSkill,&SkillAttribute,2560); std::memcpy(beforeMonster,MonsterScript,sizeof(MonsterScript));
    const short models[]={-32768,-1,0,399,400,419,546,570,700,784,787,788,789,790,795,799,819,826,830,831,859,860,861,862,863,864,865,866,867,870,951,952,953,954,955,956,957,958,1400,8191,8591,32767};
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<65536;++trial) {
        trialNumber=trial; std::memset(memory,0x5a,128);
        const short model=models[trial%42],x=static_cast<short>(trial*139),y=static_cast<short>(trial*229);
        std::memcpy(object+2,&model,2); std::memcpy(object+92,&x,2); std::memcpy(object+94,&y,2);
        const int level=static_cast<int>(((trial/42)&255)|(trial%17==0?0x80000000u:0));
        const int option=static_cast<int>((trial*37+trial/256)&255); const bool sort=(trial/7)%2!=0;
        catalogMode=(trial/256)%3; activeModel=model; MouseX=static_cast<int>(trial%1600)-200; MouseY=static_cast<int>(trial%1200)-100;
        DAT_07d78068=trial%31==0?0:DAT_055c9e44;
        BYTE before[128]; std::memcpy(before,memory,128);
        fontEvents.clear(); colors.clear(); label.clear(); labelCalls=measureCalls=textCalls=0;
        Name(reinterpret_cast<OBJECT*>(object),level,option,sort);
        Require(labelCalls==1 && measureCalls==1 && textCalls==1 && label.size()<=255 && fontEvents.size()>=1 && fontEvents.size()<=2 && fontEvents[0]==FONT_NORMAL);
        const int measured=static_cast<int>(label.size())*7+trial%4;
        Require(labelX==(sort?x:MouseX)-640*(measured/2)/static_cast<int>(gWindow.GetWidth()) && labelY==(sort?y:MouseY)-15);
        const bool bold=model==861 || model==862 || model==864 || model==799 || model==870 || model==830 || model==865 || model==866 || model==867;
        Require(fontEvents.size()==(bold?2u:1u) && (!bold || fontEvents[1]==FONT_BOLD));
        int index=catalogMode==0?model-400:catalogMode==1?42:8192;
        const char* name=DAT_07d78068 && index>=0 && index<8192?table[index].Name:"";
        if(model==863) { char expected[80]; std::snprintf(expected,80,"%s %d",name,level); Require(label==expected); }
        if(bold && model!=865 && model!=866 && model!=867) Require(label==name);
        if(model==795) { char expected[80]; std::snprintf(expected,80,"S%02d G0102",30+((level>>3)&15)); Require(label==expected); }
        if(model==826) {
            const int summons[]={2,7,14,8,9,41},l=(level>>3)&15;
            if(l<6) { char expected[80]; std::snprintf(expected,80,"M%03d G0103",summons[l]); Require(label==expected); } else Require(label.empty());
        }
        Require(std::memcmp(before,memory,128)==0);
        for(unsigned char byte:label) { digest^=byte; digest*=1099511628211ull; }
        for(int value:{labelX,labelY,static_cast<int>(fontEvents.size())}) { digest^=static_cast<unsigned>(value); digest*=1099511628211ull; }
        for(float value:colors) { std::uint32_t bits; std::memcpy(&bits,&value,4); digest^=bits; digest*=1099511628211ull; }
    }
    Require(std::memcmp(beforeTable.data(),tableMemory,8192*64+80)==0 && std::memcmp(beforeSkill,&SkillAttribute,2560)==0 && std::memcmp(beforeMonster,MonsterScript,sizeof(MonsterScript))==0);
    return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && table);
    const auto expected=RunName(low,table);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,128); if(mask&2) std::memset(table,0,8192*64+80);
            Require(RunName(mask&1?high:low,mask&2?highTable:table)==expected);
            if(mask&1) for(int i=0;i<128;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<8192*64+80;++i) Require(table[i]==0);
        }
        std::puts("cuatro combinaciones objeto/atributos altas y bajas, aliases intactos");
    }
    std::printf("RenderItemName: 65536 estados, texto/color/coordenadas/fuentes/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=convert_harness(revision).split("void __cdecl ItemConvert(",1)[0].replace(SERVICES,BACKEND)
    structs=read_source("src/structs.h",revision)
    for name in ("MONSTER_ATTRIBUTE","MONSTER_SCRIPT"):
        prefix+=re.search(rf"struct {name}\b.*?\n\}};",structs,re.S).group()
    prefix+="\nstatic_assert(sizeof(MONSTER_SCRIPT)==54);\nconstexpr int MAX_MONSTER=256;\nchar GlobalText[4000][300];\n"
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    prefix+=re.search(r"struct _SkillAttrEntry[^\n]+",header).group()+"\n_SkillAttrEntry SkillAttribute;\nalignas(MONSTER_SCRIPT) BYTE MonsterScript[512*54];\n"
    name="DAT_055c9e44"
    prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
    prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    prefix+=re.search(r"static __forceinline [^\n]*\bItemAttribute_Base\([^\{]*\{.*?^\}",header,re.M|re.S).group()
    render=function(read_source("src/Item/Item_Display.cpp",revision),"RenderItemName")
    arg="object" if "const OBJECT*" in render.split("{",1)[0] else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(object))"
    return prefix+render+MAIN.replace("ARG",arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-item-name-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
