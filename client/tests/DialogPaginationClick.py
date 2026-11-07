#!/usr/bin/env python3
"""Clics de paginación: render real de flechas y rama modal real, con renders entre ticks."""
import argparse,tempfile
from pathlib import Path
from SceneObjectPointer import read_source,build_and_run

def block(source,needle):
    start=source.index(needle);brace=source.index('{',start);end=brace+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

PRE=r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifndef _WIN32
#define __cdecl
#endif
using DWORD=std::uint32_t;using BYTE=unsigned char;
void Require(bool ok){if(!ok){std::fputs("Clic perdido o repetido entre render y lógica\n",stderr);std::exit(1);}}
int DAT_083a7c24,DAT_083a7c28,DAT_083a427c,DAT_083a4278,DAT_083a42f8[10];
BYTE DAT_083a4124,DAT_083a42c4,DAT_083a413c,DAT_055ca038,DAT_083a7c08,DAT_083a7c09;
DWORD DAT_083a7c04;char GlobalText[800][300],DAT_083a44c4[300];int g_iNumLineMessageBoxCustom,DAT_00559bec,DAT_07e11d28;
unsigned sounds,updates,arrows;
void GL_SetBlendSrcOver(char){}namespace Gfx{void Color3f(float,float,float){}}
void GL_DrawRotatedRect(int id,float,float,float,float,int){Require(id==9);++arrows;}
void PlayBuffer(int id,int a,int b){Require(id==25&&!a&&!b);++sounds;}
int SeparateTextIntoLines(const char* src,char* dst,int lines,int width){Require(src==GlobalText[713+DAT_083a7c08]&&dst==DAT_083a44c4&&lines==7&&width==38);++updates;return 7;}
'''
MAIN=r'''
void Reset(unsigned page){DAT_083a7c24=141;DAT_083a7c28=0;DAT_083a7c04=713;DAT_083a7c08=BYTE(page);DAT_083a7c09=5;DAT_055ca038=0;DAT_083a4124=1;DAT_083a42c4=1;DAT_083a413c=0;sounds=updates=arrows=0;std::memset(DAT_083a42f8,0,sizeof(DAT_083a42f8));int button[]={1,71,140,70,21};std::memcpy(DAT_083a42f8,button,sizeof(button));}
int main(){unsigned cases=0;
 for(unsigned page=0;page<=5;++page)for(unsigned direction=0;direction<2;++direction)for(unsigned renders=0;renders<=12;++renders)for(unsigned released=0;released<2;++released){Reset(page);DAT_083a42c4=BYTE(!released);DAT_083a413c=BYTE(released);DAT_083a427c=direction?383:253;DAT_083a4278=210;
  for(unsigned r=0;r<renders;++r){Draw();Require(DAT_083a4124==1&&DAT_083a42c4==!released&&DAT_083a413c==released&&DAT_083a7c08==page);}
  Tick();const unsigned wanted=direction?(page==5?5:page+1):(page? page-1:0);Require(DAT_083a7c08==wanted&&DAT_083a7c24==141&&updates==1&&!sounds&&!DAT_083a4124&&!DAT_083a42c4);
  for(unsigned r=0;r<13;++r){Draw();Tick();Require(DAT_083a7c08==wanted&&updates==1&&DAT_083a7c24==141);}
  ++cases;
 }
 Reset(2);DAT_083a427c=300;DAT_083a4278=210;Tick();Require(DAT_083a7c24==0&&sounds==1&&!updates&&!DAT_083a4124);
 Reset(2);DAT_083a427c=0;DAT_083a4278=0;Tick();Require(DAT_083a7c24==141&&DAT_083a4124==1&&!updates&&!sounds);
 Reset(2);DAT_083a4124=0;DAT_055ca038=1;DAT_083a427c=0;DAT_083a4278=0;Tick();Require(DAT_083a7c24==0&&sounds==1&&!updates);
 std::printf("DialogPagination: %u secuencias render/tick y OK/fuera/Enter OK\n",cases);
}
'''
def harness():
    render=block(read_source('src/UI/UI_StatsPanel.cpp'),'if (DAT_083a7c24 == 0x8d)')
    source=read_source('src/UI/UI_InGameMenu.cpp');start=source.index('    case 0x8b:');end=source.index('    // ── State 142',start)
    tick='void Tick(){int state=DAT_083a7c24,mouseX=DAT_083a427c,mouseY=DAT_083a4278;switch(state){'+source[start:end]+'}}'
    return PRE+'void Draw(){unsigned uVar10,uVar15,uVar16;'+render+'}'+tick+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-dialog-click-')as t:print(build_and_run(Path(t)/'current',harness(),a),end='')
if __name__=='__main__':main()
