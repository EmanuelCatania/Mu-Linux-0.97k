#!/usr/bin/env python3
"""Render de quest real: respuestas, requisitos, NPC, panel y dispatcher.

Captura texto/estado/llamadas 3D con backend controlado, contra x86. Conserva
omisiones del panel y nivel0 del preview. No certifica raster, modelos reales,
inventario real, lista omitida ni layout completo del gestor x64.
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
#include <array>
#include <cstdarg>
#ifndef _WIN32
#define __fastcall
#define __forceinline inline
using WORD=std::uint16_t; using UINT=unsigned int;
using HFONT=void*; using HGDIOBJ=void*; using HDC=void*;
#endif
unsigned trialNumber; int margin,windowWidth; BYTE* activeQuest;
struct Event { int kind; std::vector<std::int64_t> args; std::string text; };
std::vector<Event> events;
void Record(int kind,std::initializer_list<std::int64_t> args,const char* text="") { events.push_back({kind,args,text}); }
std::uint32_t Bits(float value) { std::uint32_t bits; std::memcpy(&bits,&value,4); return bits; }
enum { FONT_NORMAL=0,FONT_BOLD=1 };
int selectedFont;
struct Font {
    HDC GetTextDC() { return reinterpret_cast<HDC>(std::uintptr_t(0x456)); }
    HFONT GetFont(int type) { return reinterpret_cast<HFONT>(std::uintptr_t(sizeof(void*)>4?0x100001230ull:0x1230ull)+type); }
    void SelectFont(HDC dc,HGDIOBJ font) {
        Require(dc==GetTextDC());
        Require(font==reinterpret_cast<HGDIOBJ>(GetFont(0)) || font==reinterpret_cast<HGDIOBJ>(GetFont(1)));
        selectedFont=font==reinterpret_cast<HGDIOBJ>(GetFont(0))?0:1; Record(1,{selectedFont});
    }
} gFont;
struct Window {
    int GetRightShift() { return margin; }
    int PanelOrigin(Ui::Panel panel) { return panel.Origin(margin); }
    unsigned GetWidth() { return static_cast<unsigned>(windowWidth); }
} gWindow;
int QuestX(int offset);
namespace Gfx { void Color3f(float r,float g,float b) { Record(2,{Bits(r),Bits(g),Bits(b)}); } }
void EnableAlphaTest(bool enabled) { Record(3,{enabled}); }
void RenderInventoryInterface(int x,int y,int mode) { Record(4,{x,y,mode}); }
void RenderCenterText(int x,int y,const char* text) { Record(5,{x,y,m_dwTextColor,m_dwBackColor,selectedFont},text); }
void RenderText(int x,int y,const char* text,int width,int mode,const void* alignment) {
    const auto address=reinterpret_cast<std::uintptr_t>(alignment); Require(address==0 || address==3);
    const bool title=text==reinterpret_cast<const char*>(activeQuest+584*activeQuest[0x1c87a]+13);
    Record(6,{x,y,width,mode,static_cast<std::int64_t>(address),m_dwTextColor,m_dwBackColor,selectedFont,title},text);
}
void RenderTipText(int x,int y,const char* text) { Record(7,{x,y,m_dwTextColor,m_dwBackColor,selectedFont},text); }
int Missing(int type,int count) { return (static_cast<unsigned>(type+count)+trialNumber)%4==0?0:1+count%3; }
int CSQuest_FindQuestItemsInInven(int type,int count,unsigned mask) {
    Require(mask==0xffffffffu); const int result=Missing(type,count); Record(8,{type,count,result}); return result;
}
void RenderItem3D(float x,float y,float w,float h,int type,int level,int option,int excellent,bool sort) {
    Record(9,{Bits(x),Bits(y),Bits(w),Bits(h),type,level,option,excellent,sort});
}
void GL_DrawTexture(int id,float x,float y,float w,float h,float u,float v,float uw,float vh,int a,int b) {
    Record(10,{id,Bits(x),Bits(y),Bits(w),Bits(h),Bits(u),Bits(v),Bits(uw),Bits(vh),a,b});
}
int FUN_004c3dd0(int zen) { Record(11,{zen}); return zen<0?static_cast<int>(0xff010203u):static_cast<int>(0xffa0b0c0u); }
char npcName[40];
char* getMonsterName(int type) { Record(12,{type}); std::snprintf(npcName,40,"NPC%03d",type); return trialNumber%3?npcName:nullptr; }
UINT __fastcall FUN_00403700(void* ptr,void* edx,UINT state) {
    Require(ptr==activeQuest && edx==nullptr && (state==1 || state==2)); Record(13,{state}); return 0;
}
void CWsctlc__LogPrintOn() { Record(14,{}); }
int crt_sprintf(char* buffer,const char* format,...) { va_list args; va_start(args,format); const int count=std::vsprintf(buffer,format,args); va_end(args); Require(count>=0 && count<100); return count; }
'''

MAIN = r'''
template<class T> void Put(BYTE* ptr,T value) { std::memcpy(ptr,&value,sizeof(value)); }
std::uint64_t RunRender(BYTE* memory,BYTE* tableMemory) {
    constexpr std::size_t size=0x1c890+32,tableSize=0xa0000;
    activeQuest=memory+16; auto* allocation=tableMemory+64;
    std::memset(tableMemory,0xa5,tableSize);
    auto* rows=reinterpret_cast<ITEM_ATTRIBUTE*>(allocation);
    for(int i=0;i<9216;++i) std::snprintf(rows[i].Name,sizeof(rows[i].Name),"I%05d",i);
    std::vector<BYTE> beforeTable(tableMemory,tableMemory+tableSize);
    for(int i=0;i<4000;++i) std::snprintf(GlobalText[i],300,"G%04d",i);
    std::uint64_t digest=14695981039346656037ull; unsigned coverage=0;
    const BYTE states[]={0,1,3,5,127,128,255},modes[]={0,1,2,255};
    const int margins[]={-120,0,107,320},widths[]={640,800,1024,1280};
    const int zs[]={0,1,999,1000,1000000,2147483647,-1,-2147483647-1};
    for(unsigned trial=0;trial<16384;++trial) {
        trialNumber=trial; std::uint32_t seed=trial^0x97040332u;
        auto next=[&]() { seed=seed*1664525u+1013904223u; return seed>>16; };
        std::memset(memory,0x5a,size); BYTE* quest=activeQuest;
        const int path=trial%6,index=next()%200,klass=next()%4;
        quest[4]=static_cast<BYTE>(klass); quest[0x1c87a]=static_cast<BYTE>(index);
        quest[0x1c87d]=static_cast<BYTE>(next()%5); quest[0x1c87f]=modes[next()%4]; quest[0x1c882]=states[next()%7];
        const int zen=zs[next()%8]; Put(quest+0x1c884,zen);
        BYTE* record=quest+8+index*584; std::memset(record,0,584);
        const short count=static_cast<short>(next()%22-1); Put(record,count); record[4]=static_cast<BYTE>(next());
        std::snprintf(reinterpret_cast<char*>(record+5),30,"Quest%03d-%u",index,trial);
        for(int i=0;i<count;++i) {
            BYTE* entry=record+40+i*18; entry[-1]=static_cast<BYTE>(next()%3);
            entry[0]=static_cast<BYTE>(next()); entry[1]=static_cast<BYTE>(next()); entry[2]=static_cast<BYTE>(next());
            for(int c=0;c<4;++c) entry[4+c]=static_cast<BYTE>(next()%3);
        }
        DAT_055c9e44=reinterpret_cast<std::uintptr_t>(allocation);
        DAT_07d78068=trial%13==0?0:DAT_055c9e44+(trial%2?1023u*64u:0u);
        g_iNumLineMessageBoxCustom=static_cast<int>(next()%9)-1; g_iNumAnswer=static_cast<int>(next()%12)-1;
        for(int i=0;i<7;++i) std::snprintf(g_lpszMessageBoxCustom[i],38,"Message%d-%u",i,trial);
        for(int i=0;i<10;++i) { std::snprintf(&g_lpszDialogAnswer[i][0][0],38,"Answer%d-%u",i,trial); if((trial+i)%5==0) g_lpszDialogAnswer[i][0][0]=0; }
        margin=margins[next()%4]; windowWidth=widths[next()%4]; const int origin=450+margin;
        const int xs[]={origin-1,origin,origin+24,origin+25,origin+35,origin+106,origin+155,origin+212,origin+213};
        const int ys[]={-1,0,249,250,354,355,378,379,394,395,418,419,1000};
        MouseX=static_cast<DWORD>(xs[next()%9]); MouseY=static_cast<DWORD>(ys[next()%13]);
        MouseLButtonPush=next()%3; MouseLButtonPop=next()%3; MouseLButton=next()%3;
        const DWORD oldPush=MouseLButtonPush,oldPop=MouseLButtonPop,oldButton=MouseLButton;
        const char wanted=next()%2?static_cast<char>(quest[0x1c882]):static_cast<char>(quest[0x1c882]+1);
        m_dwTextColor=0xabcdef01u; m_dwBackColor=0x12345678u; selectedFont=0;
        BYTE before[size]; std::memcpy(before,memory,size);
        char beforeMessages[7*38],beforeAnswers[10*38]; std::memcpy(beforeMessages,DAT_083a44c4,sizeof(beforeMessages)); std::memcpy(beforeAnswers,DAT_083a4348,sizeof(beforeAnswers));
        events.clear(); int result=-1;
        if(path==0) CallAnswers(quest);
        else if(path==1 || path==2) result=FUN_00403150(quest,0,wanted,path==1?1:0);
        else if(path==3) FUN_00403320(quest);
        else if(path==4) FUN_00403a40(quest);
        else Require(FUN_00403f30(quest)==1);
        coverage|=1u<<path;
        Require(std::memcmp(before,memory,size)==0 && std::memcmp(beforeMessages,DAT_083a44c4,sizeof(beforeMessages))==0 && std::memcmp(beforeAnswers,DAT_083a4348,sizeof(beforeAnswers))==0);
        const bool npc=path==3 || (path==5 && quest[0x1c87f]==1);
        const bool consume=npc && quest[0x1c882]==1 && static_cast<int>(MouseX)>=origin+35 && static_cast<int>(MouseX)<origin+155 && static_cast<int>(MouseY)>=355 && static_cast<int>(MouseY)<379 && oldPush && oldPop;
        if(consume) coverage|=1u<<6;
        Require(MouseLButtonPush==(consume?0:oldPush) && MouseLButton==(consume?0:oldButton) && MouseLButtonPop==oldPop);
        if(path==0) {
            std::vector<Event> lines; for(const auto& event:events) if(event.kind==5) lines.push_back(event);
            const int messages=g_iNumLineMessageBoxCustom>0?g_iNumLineMessageBoxCustom:0;
            int expectedCount=messages; for(int i=0;i<g_iNumAnswer;++i) if(g_lpszDialogAnswer[i][0][0]) ++expectedCount;
            Require(lines.size()==static_cast<std::size_t>(expectedCount));
            const int top=((7-g_iNumAnswer-g_iNumLineMessageBoxCustom)*18)/2+66;
            for(int i=0;i<messages;++i) Require(lines[i].args[0]==origin+100 && lines[i].args[1]==top+18*i && lines[i].text==g_lpszMessageBoxCustom[i]);
            const int answerTop=quest[0x1c882]!=1 && quest[0x1c87f]==1?250:top+messages*18;
            const int hovered=(static_cast<int>(MouseY)-answerTop)/18;
            int row=messages;
            for(int i=0;i<g_iNumAnswer;++i) if(g_lpszDialogAnswer[i][0][0]) {
                const auto& event=lines[row++]; const int dx=origin+106-static_cast<int>(MouseX);
                const DWORD color=hovered==i && dx>=-106 && dx<=106?0xff0000ffu:0xff67bfdfu;
                Require(event.args[0]==origin+100 && event.args[1]==answerTop+18*i && event.args[2]==color && event.args[3]==0 && event.args[4]==1 && event.text==&g_lpszDialogAnswer[i][0][0]);
            }
        }
        if(path==1 || path==2) {
            const bool visible=quest[0x1c87f]!=0 && static_cast<char>(quest[0x1c882])==wanted;
            int expectedResult=visible?1:0,rendered=0; std::vector<const Event*> output,queries;
            for(const auto& event:events) { if(event.kind==(path==1?6:9)) output.push_back(&event); if(event.kind==8) queries.push_back(&event); }
            if(visible) for(int i=0;i<count;++i) {
                const BYTE* entry=record+40+18*i;
                if(entry[-1]!=1 || entry[4+klass]!=1) continue;
                const int type=entry[0]*32+entry[1],amount=entry[2]; Require(rendered<static_cast<int>(output.size()));
                const Event& event=*output[rendered];
                if(path==1) {
                    const int missing=Missing(type,amount); if(missing) expectedResult=0;
                    Require(rendered<static_cast<int>(queries.size()) && queries[rendered]->args==std::vector<std::int64_t>({type,amount,missing}));
                    const char* name=DAT_07d78068 && type<8192?reinterpret_cast<const ITEM_ATTRIBUTE*>(DAT_07d78068)[type].Name:"";
                    char text[100]; std::snprintf(text,100,"%s x %d",name,amount);
                    Require(event.text==text && event.args[0]==origin+60 && event.args[1]==240+25*rendered && event.args[5]==(missing?0xff1e1effu:0xff67bfdfu));
                } else Require(event.args==std::vector<std::int64_t>({Bits(static_cast<float>(origin+30)),Bits(235.0f+25.0f*rendered),Bits(20.0f),Bits(20.0f),type,0,0,0,0}));
                ++rendered;
            }
            Require(result==expectedResult && output.size()==static_cast<std::size_t>(rendered) && queries.size()==(path==1?output.size():0));
            if(!visible) coverage|=1u<<7;
            else if(path==2) coverage|=1u<<8;
            else coverage|=1u<<(expectedResult?9:10);
        }
        if(npc) {
            int titles=0; for(const auto& event:events) if(event.kind==6 && event.args[8]) {
                ++titles; Require(event.text==reinterpret_cast<char*>(record+5) && event.args[0]==origin+22 && event.args[1]==22 && event.args[2]==150*windowWidth/640);
            }
            Require(titles==1);
            int names=0; for(const auto& event:events) if(event.kind==12) { ++names; Require(event.args==std::vector<std::int64_t>({record[4]})); }
            Require(names==1);
            if(quest[0x1c882]==3) {
                coverage|=1u<<11;
                std::string formatted=std::to_string(zen);
                if(zen>999) for(int offset=static_cast<int>(formatted.size())-3;offset>0;offset-=3) formatted.insert(static_cast<std::size_t>(offset),",");
                int prices=0,amounts=0;
                for(const auto& event:events) {
                    if(event.kind==11) { ++prices; Require(event.args==std::vector<std::int64_t>({zen})); }
                    if(event.kind==6 && event.args[0]==origin+60 && event.args[1]==370) { ++amounts; Require(event.text==formatted); }
                }
                Require(prices==1 && amounts==1);
            }
        }
        if(path==4 || (path==5 && quest[0x1c87f]==2)) {
            const int tab=quest[0x1c87d]; int lists=0,logs=0;
            for(const auto& event:events) {
                if(event.kind==13) { ++lists; Require(event.args==std::vector<std::int64_t>({tab+1})); }
                if(event.kind==14) ++logs;
            }
            Require(lists==(tab<2?1:0) && logs==(tab==2?1:0));
            coverage|=1u<<(12+(tab<3?tab:3));
        }
        for(const auto& event:events) {
            digest^=static_cast<unsigned>(event.kind); digest*=1099511628211ull;
            for(auto value:event.args) { digest^=static_cast<std::uint64_t>(value); digest*=1099511628211ull; }
            for(unsigned char value:event.text) { digest^=value; digest*=1099511628211ull; }
        }
        for(std::uint32_t value:{static_cast<std::uint32_t>(result),static_cast<std::uint32_t>(m_dwTextColor),static_cast<std::uint32_t>(m_dwBackColor),static_cast<std::uint32_t>(MouseLButtonPush),static_cast<std::uint32_t>(MouseLButton)}) { digest^=value; digest*=1099511628211ull; }
    }
    if(coverage!=65535) std::fprintf(stderr,"cobertura render quest: %u\n",coverage);
    Require(coverage==65535 && std::memcmp(beforeTable.data(),tableMemory,tableSize)==0); return digest;
}
int main() {
    auto* low=static_cast<BYTE*>(Reserve(0x76000000u)); auto* table=static_cast<BYTE*>(Reserve(0x74000000u)); Require(low && table);
    const auto expected=RunRender(low,table);
    if(sizeof(void*)>4) {
        auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x176000000ull))); auto* highTable=static_cast<BYTE*>(Reserve(std::uint64_t(0x174000000ull))); Require(high && highTable);
        for(int mask=1;mask<4;++mask) {
            if(mask&1) std::memset(low,0,0x1c890+32); if(mask&2) std::memset(table,0,0xa0000);
            Require(RunRender(mask&1?high:low,mask&2?highTable:table)==expected);
            if(mask&1) for(std::size_t i=0;i<0x1c890+32;++i) Require(low[i]==0);
            if(mask&2) for(int i=0;i<0xa0000;++i) Require(table[i]==0);
        }
        std::puts("cuatro combinaciones quest/atributos altas y bajas, fuentes altas y aliases intactos");
    }
    std::printf("Quest render: 16384 escenarios, respuestas/items/NPC/panel/despacho %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0].replace("0x10000","0x100000")
    prefix+=read_source("src/UI/PanelAnchor.h",revision).replace("#pragma once","")
    structs=read_source("src/structs.h",revision)
    for name in ("MAX_CLASS","MAX_RESISTANCE"):
        prefix+="\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",structs,re.M).group()+"\n"
    prefix+="\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\n"
    prefix+=re.search(r"struct ITEM_ATTRIBUTE\b.*?\n\};",structs,re.S).group()+"\nstatic_assert(sizeof(ITEM_ATTRIBUTE)==64);\n"
    prefix+="\n".join(re.findall(r"^constexpr\s+(?:int|short)\s+\w+\s*=[^;]+;",read_source("src/Item/ItemDefines.h",revision),re.M))
    header,definitions=read_source("src/globals.h",revision),read_source("src/globals.cpp",revision)
    names=("DAT_00559c78","SetBackgroundTextColor","DAT_083a7c0c","DAT_083a4324","DAT_083a4124","DAT_083a413c","DAT_083a42c4","DAT_083a4278","DAT_083a427c","_DAT_00552464","DAT_055c9e44","DAT_07d78068")
    for name in names:
        prefix+=re.search(rf"extern [^;\n]*\b{name}\s*;",header).group()
        prefix+=re.search(rf"^[^/\n]*\b{name}\s*=\s*[^;]+;",definitions,re.M).group()
    for name in ("DAT_083a44c4","DAT_083a4348"):
        declaration=re.search(rf"extern [^;\n]*\b{name}\[[^;]+;",header).group()
        prefix+=declaration+declaration.replace("extern ","",1).replace(";"," = {};",1)
    for name in ("m_dwTextColor","m_dwBackColor","g_iNumAnswer","g_iNumLineMessageBoxCustom","g_lpszMessageBoxCustom","g_lpszDialogAnswer","MouseX","MouseY","MouseLButtonPush","MouseLButtonPop","MouseLButton"):
        prefix+="\n"+re.search(rf"^#define\s+{name}\s+[^\n]+",header+structs,re.M).group()+"\n"
    prefix+="\nchar GlobalText[4000][300];\n"+SERVICES
    prefix+=re.search(r"static __forceinline [^\n]*\bItemAttribute_Base\([^\{]*\{.*?^\}",header,re.M|re.S).group()
    prefix+=function(read_source("src/Item/Item_Display.cpp",revision),"ConvertGold64")
    source=read_source("src/UI/Quest_Legacy.cpp",revision)
    prefix+=function(source,"QuestX")
    answers=function(source,"FUN_00402ff0"); prefix+=answers
    argument="quest" if "const BYTE*" in answers.split("{",1)[0] else "static_cast<int>(reinterpret_cast<std::uintptr_t>(quest))"
    prefix+="\nvoid CallAnswers(BYTE* quest) { FUN_00402ff0("+argument+"); }\n"
    for name in ("FUN_00403150","FUN_00403320","FUN_00403a40","FUN_00403f30"):
        prefix+=function(source,name)
    return prefix+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia int sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-quest-render-") as temp:
        directory=Path(temp); current=build_and_run(directory/"current",harness(),args); print(current,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            if current!=reference: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
