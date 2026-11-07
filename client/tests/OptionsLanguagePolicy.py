#!/usr/bin/env python3
"""Selector local de idioma disponible en login, sin enviar al ConnectServer."""
import argparse
from pathlib import Path
import tempfile
from SceneObjectPointer import build_and_run,function,read_source

def harness(revision=None):
    source=read_source('src/UI/OptionsMenu.cpp',revision)
    toggle=''
    if 'const char* ToggleValue(' in source:
        start=source.index('const char* ToggleValue(');end=source.index('\n}',start)+2
        toggle=source[start:end]
    else:
        toggle='const char* ToggleValue(bool value){return value?"On":"Off";}'
    return r'''
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <string>
int SceneFlag,CurrentProtocolState;
constexpr int USER_LANG_DEFAULT=-1,USER_LANG_SPANISH=1,USER_LANG_PORTUGUESE=2;
int settingsCalls,textCalls,dialogCalls,sendCalls,sent;
struct Settings{int value;int GetLanguage()const{return value;}void SetLanguage(int language){value=language;++settingsCalls;}}gUserSettings;
struct Network{bool cs;bool IsConnectServerMode()const{return cs;}}gNetwork;
struct COptionsMenu{void ChangeLanguage(int);};
void OpenTextData(){++textCalls;}void OpenDialogData(){++dialogCalls;}void Lang_Send(int language){++sendCalls;sent=language;}
void Require(bool ok){if(!ok){std::fputs("Politica de idioma incorrecta\n",stderr);std::exit(1);}}
'''+toggle+function(source,'LanguageRowVisible')+function(source,'COptionsMenu::ChangeLanguage')+r'''
int main(){unsigned cases=0;COptionsMenu menu;
 const char* states[]={"Off","On","Off","On","No","Sí","Desligado","Ligado"};int index=0;
 for(int language:{-1,0,1,2})for(bool value:{false,true}){gUserSettings.value=language;Require(std::string(ToggleValue(value))==states[index++]);++cases;}
 for(int scene=0;scene<=6;++scene){SceneFlag=scene;Require(LanguageRowVisible()==(scene==2||scene==4||scene==5));++cases;}
 for(bool cs:{false,true})for(int state:{0,2,20,51,60})for(int language:{-1,0,1,2}){
  gNetwork.cs=cs;CurrentProtocolState=state;settingsCalls=textCalls=dialogCalls=sendCalls=0;sent=-9;
  menu.ChangeLanguage(language);
  Require(settingsCalls==1&&textCalls==1&&dialogCalls==1&&gUserSettings.value==language);
  const bool notify=language!=-1&&!cs&&state!=0;
  Require(sendCalls==int(notify)&&(!notify||sent==language));++cases;
 }
 std::printf("OptionsLanguagePolicy: %u casos login/seleccion/servidor\n",cases);
}
'''

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-options-language-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args.reference_ref),args),end='')
if __name__=='__main__':main()
