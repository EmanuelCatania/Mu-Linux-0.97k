#!/usr/bin/env python3
"""Las opciones respetan el idioma local antes de recibir textos del server."""
import argparse
from pathlib import Path
import tempfile
from SceneObjectPointer import build_and_run,function,read_source

def harness(revision=None):
    source=read_source('src/Local/ClientText.cpp',revision)
    namespace=source[source.index('namespace {'):source.index('const char* CClientText::Get')]
    getter=source[source.index('const char* CClientText::Get'):source.index('bool CClientText::SetServerText')]
    settings=read_source('src/Config/UserSettings.h')
    enum=settings[settings.index('enum eUserLanguage {'):settings.index('};',settings.index('enum eUserLanguage {'))+2]
    prefix='#include <cstddef>\n#include <cstdint>\n#include <cstdio>\n#include <cstdlib>\n#include <cstring>\n#include <string>\n'+read_source('src/Local/ClientText.h')+enum+r'''
struct Settings { int language=-1;int GetLanguage()const{return language;} }gUserSettings;
void Require(bool ok){if(!ok){std::fputs("Texto de opciones incorrecto\n",stderr);std::exit(1);}}
'''
    main=r'''
int main(){
    const ClientTextId ids[]={ClientTextId::LanguageDefault,ClientTextId::ScreenWindowed,ClientTextId::ScreenFullscreen,ClientTextId::ScreenBorderless,ClientTextId::RenderDecoupled,ClientTextId::RenderVSync,ClientTextId::RenderFps,ClientTextId::RenderUnlimited,ClientTextId::RenderVSyncUnavailable};
    const char* spanish[]={"Predeterminado","Ventana","Pantalla completa","Sin bordes","Render libre","VSync","FPS: %d","FPS: sin límite","VSync no disponible"};
    const char* portuguese[]={"Padrão","Janela","Tela cheia","Sem bordas","Render livre","VSync","FPS: %d","FPS: sem limite","VSync indisponível"};
    CClientText text;unsigned cases=0;
    for(int language: {-1,0,1,2}){
        gUserSettings.language=language;text.ClearServerTexts();
        for(unsigned i=0;i<9;++i){
            const char* expected=language==1?spanish[i]:language==2?portuguese[i]:English[static_cast<size_t>(ids[i])];
            Require(std::strcmp(text.Get(ids[i]),expected)==0);++cases;
            s_ServerText[static_cast<size_t>(ids[i])]="Texto del servidor";
            Require(std::strcmp(text.Get(ids[i]),"Texto del servidor")==0);++cases;
        }
        text.ClearServerTexts();
        for(unsigned i=0;i<9;++i){
            const char* expected=language==1?spanish[i]:language==2?portuguese[i]:English[static_cast<size_t>(ids[i])];
            Require(std::strcmp(text.Get(ids[i]),expected)==0);++cases;
        }
        char fps[32];std::snprintf(fps,sizeof(fps),text.Get(ClientTextId::RenderFps),144);Require(std::strcmp(fps,"FPS: 144")==0);
        Require(std::strcmp(text.Get(ClientTextId::ChaosRatePending),English[static_cast<size_t>(ClientTextId::ChaosRatePending)])==0);
    }
    Require(std::strcmp(text.Get(ClientTextId::Count),"")==0);
    std::printf("OptionsLocalText: %u casos de idioma/fallback/precedencia\n",cases);
}
'''
    return prefix+namespace+getter+function(source,'CClientText::ClearServerTexts')+main

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-options-text-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args.reference_ref),args),end='')
if __name__=='__main__':main()
