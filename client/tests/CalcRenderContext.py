#!/usr/bin/env python3
"""Calc_RenderObject con OBJECT/BMD reales nativos y servicios controlados."""
import argparse,re,tempfile
from pathlib import Path
from ModelContextPointer import harness as context_harness,MAIN as CONTEXT_MAIN
from SceneObjectPointer import function,read_source,build_and_run

SERVICES=r'''
#include <vector>
std::vector<int> events;
float _DAT_005597c8;
void Entity_GetLightScale(uintptr_t o,uintptr_t m){Require(o==reinterpret_cast<uintptr_t>(wantedOwner)&&m==reinterpret_cast<uintptr_t>(wantedModel));events.push_back(1);wantedModel->LightEnable=1;wantedModel->BodyLight[0]=.25f;wantedModel->BodyLight[1]=.5f;wantedModel->BodyLight[2]=.75f;}
void Skeleton_Transform(void* m,float* b,float* lo,float* hi,float* out,char t){Require(m==wantedModel&&b==wantedBones&&lo==wantedOwner->BoundingBoxMin&&hi==wantedOwner->BoundingBoxMax&&out==wantedOwner->OBB&&t==wantedTranslate);events.push_back(3);}
void BMD__RenderBody(void* m,int flags,float a,int blend,float light,float u,float v,int hidden,unsigned mask){Require(m==wantedModel&&flags==64&&a==wantedOwner->Alpha&&blend==wantedOwner->BlendMesh&&light==wantedOwner->BlendMeshLight&&u==wantedOwner->BlendMeshTexCoordU&&v==wantedOwner->BlendMeshTexCoordV&&hidden==wantedOwner->HiddenMesh&&mask==0xffffffffu);events.push_back(4);}
'''
MAIN=r'''
int main(){
 static BMD models[512];static OBJECT object;static vec34_t bones[2];
 if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(models)>0xffffffffull&&reinterpret_cast<uintptr_t>(&object)>0xffffffffull&&reinterpret_cast<uintptr_t>(bones)>0xffffffffull);
 for(unsigned trial=0;trial<8192;++trial){
  std::memset(models,0x5a,sizeof(models));std::memset(&object,0,sizeof(object));
  object.Type=short(trial%512);object.Alpha=trial%7?1.f:0.f;object.Scale=.5f+float(trial%4);object.Kind=trial%2?4:1;object.CurrentAction=BYTE(trial);object.ContrastEnable=BYTE(trial%2);object.EnableBoneMatrix=BYTE(trial%3);object.BoneMatrices=trial%5?bones:nullptr;
  for(int i=0;i<3;++i)object.Position[i]=float(trial%31+i);
  wantedOwner=&object;wantedModel=models+object.Type;wantedBones=object.EnableBoneMatrix?reinterpret_cast<float*>(object.BoneMatrices):globalBones;wantedTranslate=char(trial%2);DAT_05828d58=reinterpret_cast<uintptr_t>(models);
  BMD before=models[(object.Type+1)%512];auto expected=*wantedModel;auto savedObject=object;const bool skip=object.Alpha<_DAT_005524f8;int lod=int(trial%5);
  if(!skip){expected.BodyHeight=0;expected.ContrastEnable=object.ContrastEnable;expected.LightEnable=1;expected.BodyLight[0]=.25f;expected.BodyLight[1]=.5f;expected.BodyLight[2]=.75f;expected.BodyScale=object.Scale;std::memcpy(expected.BodyOrigin,object.Position,12);if(object.Type==331)expected.BodyOrigin[1]+=_DAT_0055290c;else if(object.Type==330)expected.BodyOrigin[1]+=_DAT_00552908;expected.CurrentAction=object.CurrentAction;}
  events.clear();bonesCalls=0;_DAT_005597c8=99;int result=Calc_RenderObject(&object,wantedTranslate,lod);
  Require(result==int(!skip)&&bonesCalls==unsigned(!skip));
  const std::vector<int> normal={1,2,3},outline={1,2,3,4,3,4,1,3},empty={};Require(events==(skip?empty:lod==1?outline:normal));
  Require(std::memcmp(wantedModel,&expected,sizeof(BMD))==0&&std::memcmp(&before,models+(object.Type+1)%512,sizeof(BMD))==0&&std::memcmp(&savedObject,&object,sizeof(object))==0);
  Require(_DAT_005597c8==(skip?99.f:lod==3?1.4f:lod==2?1.2f:1.f));
 }
 std::printf("CalcRenderContext: 8192 casos; modelos/objetos nativos, orden y canarios OK ptr=%zu\n",sizeof(void*));
}
'''
def harness():
    source=context_harness().split(CONTEXT_MAIN)[0]
    source=re.sub(r'void Calc_RenderObject\([^\n]+\n','int Calc_RenderObject(OBJECT*,char,int);\n',source)
    needle='++bonesCalls;PrepareContext(o,m);';assert needle in source
    source=source.replace(needle,'++bonesCalls;RecordBones();PrepareContext(o,m);')
    source=source.replace('void ObjectPose_PrepareBones(OBJECT* o,','void RecordBones();\nvoid ObjectPose_PrepareBones(OBJECT* o,',1)
    return source+SERVICES+'\nvoid RecordBones(){events.push_back(2);}\n'+function(read_source('src/Render/Entity_AnimationLegacy.cpp'),'Calc_RenderObject')+MAIN

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-calc-context-')as t:print(build_and_run(Path(t)/'current',harness(),a),end='')
if __name__=='__main__':main()
