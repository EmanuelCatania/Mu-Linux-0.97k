#!/usr/bin/env python3
"""Recorridos reales de descarga de modelos y cambio de fondo de selección."""
import argparse,os,tempfile
from pathlib import Path
from ModelReleaseLifecycle import prefix
from SceneObjectPointer import function,read_source,build_and_run
SERVICES=r'''
std::vector<std::pair<int,int>> events;static BMD table[380];uintptr_t DAT_05828d58=reinterpret_cast<uintptr_t>(table);
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
void BMD__Release(BMD* p){auto n=reinterpret_cast<uintptr_t>(p)-DAT_05828d58;Require(n%sizeof(BMD)==0&&n/sizeof(BMD)<380);events.push_back({1,int(n/sizeof(BMD))});}
void BMD__Release(uintptr_t p){BMD__Release(reinterpret_cast<BMD*>(p));}
void UnloadImage(int n){events.push_back({2,n});}
void DeleteObjects(){events.push_back({3,0});}
void ClearCharacters(int n){events.push_back({4,n});}
void Sound_ReleaseBuffer(int n){events.push_back({5,n});}
std::vector<int> selectBackdropTextures;bool selectBackdropWide;
struct Window{bool wide=false;bool IsWide(){return wide;}}gWindow;
BYTE g_BitmapsRaw[2048*56];char DAT_005606ac[]="Backdrop",s_Logo__00560698[]="Logo/";
const char* SelectBackdropPath(){return gWindow.wide?"Wide/":"Normal/";}
namespace MeshDraw{void Release(){events.push_back({6,0});}}
void AccessModel(int slot,const char* path,const char* name,int index){Require(slot==164&&std::strcmp(path,SelectBackdropPath())==0&&std::strcmp(name,"Backdrop")==0&&index==5);events.push_back({7,slot});}
void OpenTexture(int slot,const char* path,int filter,char flag){Require(slot==164&&std::strcmp(path,"Logo/")==0&&filter==0x2601&&flag==1);events.push_back({8,slot});}
'''
MAIN=r'''
int main(){
 if(sizeof(void*)==8)Require(DAT_05828d58>0xffffffffull);
 std::uint64_t hash=14695981039346656037ull;
 auto Check=[&](const std::vector<std::pair<int,int>>& expected){Require(events==expected);for(const auto& e:events)for(std::uint32_t v:{std::uint32_t(e.first),std::uint32_t(e.second)})for(unsigned j=0;j<4;++j){hash^=BYTE(v>>(8*j));hash*=1099511628211ull;}events.clear();};
 std::vector<std::pair<int,int>> expected;
 DeleteNpcs();for(int i=335;i<380;++i)expected.push_back({1,i});for(int i=120;i<170;++i)expected.push_back({5,i});Check(expected);expected.clear();
 DeleteMonsters();for(int i=270;i<335;++i)expected.push_back({1,i});for(int i=170;i<420;++i)expected.push_back({5,i});Check(expected);expected.clear();
 ReleaseWorldModels();for(int i=0;i<160;++i)expected.push_back({1,i});Check(expected);expected.clear();
 selectBackdropTextures={600,601};ReleaseCharacterSceneData();Require(selectBackdropTextures.empty());expected={{2,600},{2,601}};for(int i=12;i<30;++i)expected.push_back({2,i});for(int i=1386;i<1450;++i)expected.push_back({2,i});expected.push_back({3,0});for(int i=170;i<174;++i)expected.push_back({1,i});expected.push_back({4,-1});Check(expected);expected.clear();
 static short textures[]={301,302,301,303};table[164].NumMeshs=4;table[164].IndexTexture=textures;
 selectBackdropWide=false;gWindow.wide=false;std::memset(g_BitmapsRaw,0,sizeof(g_BitmapsRaw));
 for(unsigned trial=0;trial<16;++trial){gWindow.wide=trial%2;Scene_RefreshCharSelectBackdrop();if(trial)expected={{6,0},{1,164},{7,164},{8,164}};Check(expected);expected.clear();for(int id:{301,302,303})Require(g_BitmapsRaw[id*56+48]==BYTE(trial?1:0));Require(selectBackdropTextures.size()==(trial?3u:0u));}
 std::printf("ModelReleaseSlots: 274 slots, sonidos/texturas y 16 alternancias, orden %016llx\n",(unsigned long long)hash);
}
'''
def harness(revision=None):
 terrain=read_source('src/Terrain/Terrain_LegacyLoad.cpp',revision);scene=read_source('src/Scene/Scene_Resources.cpp',revision)
 world=function(read_source('src/Render/SMD_Parser.cpp',revision),'DeleteObjects');world=world[:world.index('SceneObjectCell* puVar5')]+"}\n";world=world.replace('DeleteObjects(','ReleaseWorldModels(',1)
 return prefix()+SERVICES+function(terrain,'DeleteNpcs')+function(terrain,'DeleteMonsters')+world+function(scene,'ReleaseCharacterSceneData')+function(scene,'Scene_RefreshCharSelectBackdrop')+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
 if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-model-release-slots-')as t:
  current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
  if a.reference_ref:
   other=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
   if current!=other:raise RuntimeError('Diferencia de slots frente a x86')
   print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
