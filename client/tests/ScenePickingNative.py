#!/usr/bin/env python3
"""Picking real de escena: listas, filtros, argumentos y memoria con layouts nativos."""
import argparse,os,tempfile,re
from pathlib import Path
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import ROOT,read_source,function,build_and_run
SERVICES=r'''
#include <cmath>
#include <array>
#include <limits>
#ifndef _WIN32
#define __stdcall
#endif
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}
uintptr_t DAT_05828d58;SceneObjectCell* g_ObjectBucketGrid;OBJECT* nodes;BMD* models;
float _DAT_005524f8=.01f,startRay[3],endRay[3],closest;
#define CameraRayOriginX startRay[0]
#define DAT_083a4110 endRay[0]
unsigned active,cursor;std::vector<unsigned> plan,events;bool boxHit[8],meshHit[8];float distances[8];
std::size_t count;namespace ResourceTables{std::size_t ModelCount(){return count;}}
void Event(unsigned e,unsigned id=0){events.push_back(e);events.push_back(id);}
void Map_InitRayCast(){closest=1000;Event(1);}
void BMD_Animation(void* model,int zero,float frame,unsigned prior,unsigned char byte,unsigned* angles,float* head,char a,char b){Require(cursor<plan.size());active=plan[cursor++];auto& o=nodes[active];Require(model==models+o.Type&&!zero&&frame==o.PriorAnimationFrame&&prior==o.PriorAction&&!a&&!b);BYTE wanted;std::memcpy(&wanted,o.Angle,1);Require(byte==wanted);const float* f=reinterpret_cast<float*>(angles);Require(f[0]==0&&f[1]==0&&f[2]==o.HeadAngle[0]&&head[0]==0&&head[1]==0&&head[2]==0);Require(std::memcmp(&models[o.Type].BodyScale,&o.Scale,4)==0&&models[o.Type].CurrentAction==o.CurrentAction&&std::memcmp(models[o.Type].BodyOrigin,o.Position,12)==0);Event(2,active);}
void BMD_TransformPosition(void* model,float* bounds,float* obb,float* out,char flag){auto& o=nodes[active];Require(model==models+o.Type&&bounds==o.BoundingBoxMin&&obb==o.OBB&&out&&!flag);for(unsigned a=0;a<3;++a){obb[a]=o.BoundingBoxMin[a]+float(a+1);out[a]=float(active);}Event(3,active);}
bool Collision_SegmentToOBB(float* a,float* b,float* obb){Require(a==startRay&&b==endRay&&std::memcmp(obb,nodes[active].OBB,48)==0);Event(4,active);return boxHit[active];}
bool BMD__CollisionDetectLineToMesh(BMD* model,float* a,float* b,bool update,int mesh,int face){Require(model==models+nodes[active].Type&&a==startRay&&b==endRay&&update&&mesh==-1&&face==-1);Event(5,active);if(meshHit[active]&&distances[active]<=closest){closest=distances[active];return true;}return false;}
'''
MAIN=r'''
void Gold(std::vector<unsigned>& out,unsigned e,unsigned id=0){out.push_back(e);out.push_back(id);}
std::uint64_t Run(BYTE* objectMem,BYTE* modelMem,BYTE* gridMem){nodes=reinterpret_cast<OBJECT*>(objectMem+16);models=reinterpret_cast<BMD*>(modelMem+16);g_ObjectBucketGrid=reinterpret_cast<SceneObjectCell*>(gridMem+16);std::uint64_t digest=14695981039346656037ull;
 for(unsigned trial=0;trial<1024;++trial){std::memset(objectMem,0x5a,8*sizeof(OBJECT)+32);std::memset(modelMem,0xa5,4*sizeof(BMD)+32);std::memset(gridMem,0,256*sizeof(SceneObjectCell)+32);SceneObjects::LiveNodes().clear();SceneObjects::Generations().clear();DAT_05828d58=trial%31?uintptr_t(models):0;count=trial%29?4:0;std::array<std::vector<unsigned>,256> lists;
  for(unsigned id=0;id<8;++id){auto& o=nodes[id];o.Live=BYTE((trial+id)%5!=0);o.Visible=BYTE((trial+id)%7!=0);const float alphas[]={0,.005f,.01f,1,(std::numeric_limits<float>::quiet_NaN)()};o.Alpha=alphas[(trial+id)%5];o.Type=short(int((trial/3+id)%6)-1);o.Scale=float(id)*.25f;for(unsigned a=0;a<3;++a){o.Position[a]=float(id*4+a);o.Angle[a]=float(id+a)*.125f;o.HeadAngle[a]=float(a+id)*.25f;o.BoundingBoxMin[a]=float(int(id)-int(a));}for(unsigned a=0;a<12;++a)o.OBB[a]=float(id*16+a);const unsigned angleBits=0x3f800011u+id+trial%4;std::memcpy(o.Angle,&angleBits,4);o.CurrentAction=BYTE(128+id);o.PriorAction=BYTE(id+2);o.PriorAnimationFrame=float(id)+.25f;o.SceneNext=nullptr;boxHit[id]=(trial+id)%3!=0;meshHit[id]=(trial+id)%4!=0;distances[id]=float(8-id)*10;
   if(id==7&&trial%11==0)continue;SceneObjects::Register(&o);lists[(id/3)*80+(trial%16)].push_back(id);
  }
  for(unsigned cell=0;cell<256;++cell){auto& ids=lists[cell];if(ids.empty())continue;g_ObjectBucketGrid[cell].Head=nodes+ids.front();g_ObjectBucketGrid[cell].Tail=nodes+ids.back();for(unsigned i=1;i<ids.size();++i)nodes[ids[i-1]].SceneNext=nodes+ids[i];}
  g_ObjectBucketGrid[254].Head=reinterpret_cast<void*>(uintptr_t(0x12345678));
  if(trial==1023){DAT_05828d58=uintptr_t(models);count=4;std::memset(g_ObjectBucketGrid,0,256*sizeof(SceneObjectCell));for(auto& ids:lists)ids.clear();lists[255]={0,1};for(unsigned id:{0u,1u}){nodes[id].Live=nodes[id].Visible=1;nodes[id].Alpha=1;nodes[id].Type=1;}nodes[0].SceneNext=nodes+1;nodes[1].SceneNext=nodes;g_ObjectBucketGrid[255].Head=nodes;}
  auto expectedObjects=std::vector<BYTE>(objectMem,objectMem+8*sizeof(OBJECT)+32),expectedModels=std::vector<BYTE>(modelMem,modelMem+4*sizeof(BMD)+32);const auto beforeGrid=std::vector<BYTE>(gridMem,gridMem+256*sizeof(SceneObjectCell)+32);std::vector<unsigned> wanted;plan.clear();cursor=0;events.clear();Gold(wanted,1);int result=-1;float expectedClosest=1000;
  for(unsigned cell=0;cell<256;++cell){auto ids=lists[cell];if(ids.empty())continue;unsigned limit=trial==1023?4096:unsigned(ids.size());if(trial==1023&&cell!=255)limit=4096;for(unsigned step=0;step<limit;++step){unsigned id=ids[step%ids.size()];const auto& o=nodes[id];if(!(o.Live&&o.Visible&&o.Alpha>=.01f&&o.Type>=0&&std::size_t(o.Type)<count&&DAT_05828d58))continue;plan.push_back(id);Gold(wanted,2,id);Gold(wanted,3,id);Gold(wanted,4,id);auto* eo=reinterpret_cast<OBJECT*>(expectedObjects.data()+16)+id;for(unsigned a=0;a<3;++a)eo->OBB[a]=o.BoundingBoxMin[a]+float(a+1);auto* em=reinterpret_cast<BMD*>(expectedModels.data()+16)+o.Type;std::memcpy(&em->BodyScale,&o.Scale,4);em->CurrentAction=o.CurrentAction;std::memcpy(em->BodyOrigin,o.Position,12);if(boxHit[id]){Gold(wanted,5,id);if(meshHit[id]&&distances[id]<=expectedClosest){expectedClosest=distances[id];result=int(id);}}}}
  auto* picked=PickObject_Mouse();Require(picked==(result<0?nullptr:reinterpret_cast<BYTE*>(nodes+result))&&cursor==plan.size()&&events==wanted&&closest==expectedClosest);Require(std::memcmp(objectMem,expectedObjects.data(),expectedObjects.size())==0&&std::memcmp(modelMem,expectedModels.data(),expectedModels.size())==0&&std::memcmp(gridMem,beforeGrid.data(),beforeGrid.size())==0);for(unsigned e:events){digest^=e;digest*=1099511628211ull;}
 }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000));Require(low);auto expected=Run(low+256,low+4096,low+8192);if(sizeof(void*)==8){auto* high=static_cast<BYTE*>(Reserve(0x126000000ull));Require(high);for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,65536);Require(Run((mask&1?high:low)+256,(mask&2?high:low)+4096,(mask&4?high:low)+8192)==expected);}}std::printf("ScenePicking:1024 escenas, filtros/orden/memoria %016llx\n",(unsigned long long)expected);}
'''
def harness(revision=None):
    s=read_source('src/Scene/Scene_ObjectInteraction.cpp',revision).replace('char* __stdcall PickObject_Mouse','BYTE* __cdecl PickObject_Mouse').replace('char*','BYTE*').replace('char *','BYTE *')
    body=function(s,'PickObject_Mouse')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+layouts()+'\nusing vec34_t=float[3][4];\n#pragma pack(push,1)\n'+record(read_source('src/Entity/EntityTypes.h'),'OBJECT')+'\n#pragma pack(pop)\n'
    prefix+=read_source('src/Scene/SceneObjectCell.h').replace('#pragma once','')+read_source('src/Scene/ObjectRegistry.h').replace('#pragma once','')
    prefix+=read_source('src/Platform/GuardedOperation.h').replace('#pragma once','').replace('#include "Platform/PlatformHeaders.h"','')
    prefix+='static_assert(sizeof(OBJECT)==(sizeof(void*)==4?444:464));static_assert(offsetof(OBJECT,Angle)==28);static_assert(offsetof(OBJECT,OBB)==(sizeof(void*)==4?304:312));\n'
    return prefix+SERVICES+body+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-picking-native-')as t:
        result=build_and_run(Path(t)/'current',harness(),a);print(result,end='')
        if a.reference_ref:
            if os.name!='nt'or a.architecture!='Win32':p.error('Referencia sólo Windows x86')
            old=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
            if old!=result:raise RuntimeError('Diferencia frente a x86')
            print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
