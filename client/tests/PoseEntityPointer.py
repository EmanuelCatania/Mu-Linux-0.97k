#!/usr/bin/env python3
"""Entidad nativa en pose/render: contexto, replay y despacho controlado.

Vista opaca x86: Owner32 y unión BoneMatrices/Min conservan offsets.
Con EnableBoneMatrix activo, la paleta propia es baja y Min[0] es cero;
no certifica OBJECT nativo ni los layouts pendientes de B2.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT, build_and_run, function, read_source
SERVICES=r'''
#include <array>
#include <algorithm>
VIEW
inline OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}inline const OBJECT* AsObject(const void* p){return static_cast<const OBJECT*>(p);}inline OBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}
TRANSFORMS
STRUCTURES
std::vector<PreparedBones> prepared;PreparedBones* geometryPose;bool batchActive;
uintptr_t DAT_05828d58,DAT_07abf5d0;float* globalBones;
#define DAT_06970a9c (*reinterpret_cast<DWORD*>(globalBones))
char DAT_05826e18[200*16];float parentStore[12];
#define DAT_06989c9c parentStore[0]
int DAT_07e11d30;DWORD MoveSceneFrame=100;
OBJECT* subject;BYTE* activeModel;float* selectedBones;unsigned operation;char translation;int selection;
std::vector<int> events;unsigned skeletonCalls;
void Context(BYTE* output){std::memcpy(output+104,&subject->Scale,4);output[160]=subject->CurrentAction;std::memcpy(output+108,subject->Position,12);if(subject->Type==331){const float y=subject->Position[1]+_DAT_0055290c;std::memcpy(output+112,&y,4);}else if(subject->Type==330){const float y=subject->Position[1]+_DAT_00552908;std::memcpy(output+112,&y,4);}}
void BMD_Animation(void* model,float* bones,float frame,unsigned prior,unsigned char action,unsigned* angles,float* head,char special,char translate){
 Require(model==activeModel && bones==selectedBones && frame==subject->AnimationFrame && action==subject->PriorAction && angles==reinterpret_cast<unsigned*>(subject->Angle) && head==subject->HeadAngle && !special && translate==(translation==0));unsigned expected;std::memcpy(&expected,&subject->PriorAnimationFrame,4);Require(prior==expected);BYTE context[188];std::memcpy(context,activeModel,188);Context(context);Require(std::memcmp(context+104,activeModel+104,16)==0 && activeModel[160]==subject->CurrentAction);std::memcpy(activeModel+120,angles,12);std::memcpy(activeModel+164,&frame,4);const short index=123;std::memcpy(activeModel+168,&index,2);bones[0]=77;bones[1]=-33;events.push_back(2);
}
void Entity_GetLightScale(uintptr_t entity,uintptr_t model){Require(entity==reinterpret_cast<uintptr_t>(subject) && model==reinterpret_cast<uintptr_t>(activeModel));const float rgb[]={.25f,.5f,.75f};std::memcpy(activeModel+72,rgb,12);events.push_back(1);}
void VerifyOutline(){const unsigned pass=skeletonCalls;Require(pass<2);float x,y,scale;const bool npc=subject->Kind==4;if(npc){x=pass?.16f:.02f;y=pass?.7f:.1f;scale=pass?1.08f:1.2f;}else{x=pass?.7f:.1f;y=pass?.07f:.01f;scale=subject->Type==302?(pass?1.08f:1.2f):(pass?.039999999f:.1f)/subject->Scale+1;}float rgb[3];std::memcpy(rgb,activeModel+72,12);Require(rgb[0]==x && rgb[1]==y && rgb[2]==0 && _DAT_005597c8==scale && activeModel[68]==0);}
void Skeleton_Transform(void* model,float* bones,float* minimum,float* maximum,float* bounds,char translate){Require(model==activeModel && bones==selectedBones && minimum==subject->BoundingBoxMin && maximum==subject->BoundingBoxMax && translate==translation);if(operation!=4 && selection==1 && skeletonCalls<2)VerifyOutline();if(operation!=4)Require(bounds==subject->OBB);for(unsigned i=0;i<12;++i)bounds[i]=float(100+i);events.push_back(3);++skeletonCalls;}
void BMD__RenderBody(void* model,int flags,float alpha,int blend,float light,float u,float v,int hidden,unsigned extra){Require(model==activeModel && flags==64 && alpha==subject->Alpha && blend==subject->BlendMesh && light==subject->BlendMeshLight && u==subject->BlendMeshTexCoordU && v==subject->BlendMeshTexCoordV && hidden==subject->HiddenMesh && extra==0xffffffffu);--skeletonCalls;VerifyOutline();++skeletonCalls;events.push_back(4);}
int PlayBuffer(int id,const OBJECT* owner,bool loop){Require(id==106 && !owner && !loop);events.push_back(5);return 0;}
void BMD__RenderMeshEffect(void* model,int zero,int type){Require(model==activeModel && !zero && type==(subject->Type==330?260:262));events.push_back(6);}
void Draw_RenderObject(void* object,int slot,int flag,char mode){Require(object==subject && slot==translation && flag==selection && mode==7);events.push_back(7);}
FORWARDS
'''
MAIN=r'''
void Invoke(unsigned op){switch(op){case 0:ObjectPose_PrepareBones(ENTITY,activeModel,globalBones,translation);break;case 1:Require(Calc_RenderObject(ENTITY,translation,selection)==(subject->Alpha<_DAT_005524f8?0:1));break;case 2:ObjectPose_PrepareBounds(ENTITY,translation);break;case 3:ObjectPose_PrepareSimulation(ENTITY,translation);break;case 4:ObjectPose_TickMeshEvents(ENTITY,activeModel,globalBones,translation);break;case 5:Entity_PrepareRender(subject,translation,selection,7);break;}}
template<class T>void Store(BYTE* p,unsigned offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
void Hash(std::uint64_t& h,const void* p,unsigned n){for(unsigned i=0;i<n;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* data,BYTE* pool,BYTE* models,BYTE* boneMem){
 std::uint64_t hash=14695981039346656037ull;FrozenModel frozen;prepared.resize(4);auto& cached=prepared[3];cached.frozen=&frozen;globalBones=reinterpret_cast<float*>(boneMem+16);subject=reinterpret_cast<OBJECT*>(pool+16+3*916);const short types[]={1,302,330,331,334,335,390};const float alphas[]={0,.009f,.01f,.5f,1},scales[]={-2,.5f,1,2};const int selections[]={-1,0,1,2,3,4};unsigned cases=0,hits=0;
 for(unsigned trial=0;trial<2048;++trial)for(operation=0;operation<6;++operation){
  std::memset(data,0xc3,8192);std::memset(subject,0xa5,444);std::memset(boneMem,0x3c,192);std::memset(DAT_05826e18,0x5a,sizeof(DAT_05826e18));std::memset(parentStore,0x6b,sizeof(parentStore));
  subject->Type=types[trial%7];subject->Live=(trial/7)%2;subject->Alpha=alphas[(trial/14)%5];subject->Scale=scales[(trial/70)%4];subject->Kind=(trial/3)%3==0?4:1;subject->CurrentAction=(trial/5)%3==0?6:2;subject->PriorAction=(trial/11)%13;subject->EnableBoneMatrix=(trial/13)%2;subject->ContrastEnable=(trial/17)%256;subject->InitialSceneFrame=70+(trial/19)%40;subject->HiddenMesh=-1;subject->BlendMesh=3;subject->BlendMeshLight=.5f;subject->BlendMeshTexCoordU=.25f;subject->BlendMeshTexCoordV=-.5f;subject->AnimationFrame=float(trial%9)*.25f;
  const std::uint32_t previous[]={0u,0x80000000u,0x3f400000u,0x7fc12345u};std::memcpy(&subject->PriorAnimationFrame,&previous[(trial/23)%4],4);
  for(unsigned i=0;i<3;++i){subject->Position[i]=float(int(trial%17)-8+int(i));subject->Angle[i]=float(i*30);subject->HeadAngle[i]=float(i*5);subject->BoundingBoxMax[i]=float(i+4);}
  auto* alternate=reinterpret_cast<float*>(data+4096);subject->BoneMatrices=reinterpret_cast<vec34_t*>(alternate);subject->BoundingBoxMin[0]=0;subject->BoundingBoxMin[1]=-2;subject->BoundingBoxMin[2]=-3;
  activeModel=models+16+subject->Type*188;std::memset(activeModel-16,0xa5,220);DAT_05828d58=reinterpret_cast<uintptr_t>(models+16);DAT_07abf5d0=reinterpret_cast<uintptr_t>(pool+16);const auto bonePtr=static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(data+16)),actionPtr=static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(data+512));Store(activeModel,44,bonePtr);Store(activeModel,48,actionPtr);Store(activeModel,34,short(2));Store(activeModel,38,short(2));Store(activeModel,84,int(-1));Store(activeModel,132,2.0f);data[16+34]=0;data[16+140+34]=trial&1;
  frozen.boneHeaders.assign(data+16,data+16+280);frozen.actionHeaders.assign(data+512,data+512+32);std::memcpy(cached.inputModel.data(),activeModel,188);Context(cached.inputModel.data());std::memcpy(cached.model.data(),cached.inputModel.data(),188);for(unsigned i=0;i<3;++i)Store(cached.model.data(),120+i*4,float(9-i));Store(cached.model.data(),164,3.75f);Store(cached.model.data(),168,short(8));cached.model[160]=171;
  std::memcpy(cached.object.data(),subject,444);cached.boneCount=2;cached.valid=true;cached.writesParent=(trial>>1)&1;cached.originalModel=activeModel;for(unsigned i=0;i<24;++i)cached.bones[i]=float(10+i);for(unsigned i=0;i<32;++i)cached.quaternions[i]=char(32+i);for(unsigned i=0;i<12;++i)cached.parent[i]=float(200+i);
  translation=(trial/16)%3==0?0:(trial/16)%3==1?1:-1;selection=selections[(trial/48)%6];DAT_07e11d30=(trial/29)%2?2:0;batchActive=operation==0;const unsigned mode=trial%16;
  if(operation==0){if(mode==0)batchActive=false;if(mode==1)translation=1;if(mode==2)translation=0;if(mode==3)cached.valid=false;if(mode==4)cached.originalModel=activeModel+4;if(mode==5)cached.inputModel[84]^=1;if(mode==6)frozen.boneHeaders[0]^=1;if(mode==7)frozen.actionHeaders[0]^=1;if(mode==8)cached.object[264]^=1;if(mode==9)cached.object[262]^=1;if(mode==10)cached.object[28]^=1;if(mode==11)cached.object[40]^=1;if(mode==12)++DAT_07abf5d0;if(mode==13)DAT_07abf5d0=reinterpret_cast<uintptr_t>(subject)-400*916;}
  const bool hit=operation==0 && batchActive && translation==1 && cached.valid && mode!=4 && mode!=5 && mode!=6 && mode!=7 && mode!=8 && mode!=9 && mode!=10 && mode!=11 && mode!=12 && mode!=13;
  geometryPose=&prepared[1];_DAT_005597c8=9.75f;events.clear();skeletonCalls=0;
  BYTE objectExpected[444],modelExpected[220],bonesExpected[192],dataExpected[8192],qExpected[sizeof(DAT_05826e18)],parentExpected[sizeof(parentStore)];std::memcpy(objectExpected,subject,444);std::memcpy(modelExpected,activeModel-16,220);std::memcpy(bonesExpected,boneMem,192);std::memcpy(dataExpected,data,8192);std::memcpy(qExpected,DAT_05826e18,sizeof(qExpected));std::memcpy(parentExpected,parentStore,sizeof(parentExpected));auto* modelOut=modelExpected+16;
  const bool visible=subject->Alpha>=_DAT_005524f8,calc=visible && (operation==1 || operation==5 || ((operation==2 || operation==3) && DAT_07e11d30==2));const bool pose=operation==0 || calc || (operation==3 && visible);const bool meshEvent=operation==4 && visible && subject->Live && ((subject->Type==331 && subject->CurrentAction==6) || (subject->Type==330 && (subject->CurrentAction==6 || int(MoveSceneFrame)-subject->InitialSceneFrame<25)));
  if(operation==2 || operation==3)selection=0;selectedBones=operation==0 || operation==4?globalBones:subject->EnableBoneMatrix?alternate:globalBones;
  std::vector<int> expectedEvents;
  if(calc){Store(modelOut,132,0u);modelOut[69]=subject->ContrastEnable;const float rgb[]={.25f,.5f,.75f};std::memcpy(modelOut+72,rgb,12);expectedEvents.push_back(1);}
  if(operation==3 && visible && !calc)Store(modelOut,132,0u);
  if(pose){Context(modelOut);if(hit){std::memcpy(modelOut+120,cached.model.data()+120,12);std::memcpy(modelOut+164,cached.model.data()+164,6);modelOut[160]=cached.model[160];for(unsigned i=0;i<2;++i)if(!data[16+i*140+34]){std::memcpy(bonesExpected+16+i*48,cached.bones.data()+i*12,48);std::memcpy(qExpected+i*16,cached.quaternions.data()+i*16,16);}if(cached.writesParent)std::memcpy(parentExpected,cached.parent.data(),48);++hits;}else{std::memcpy(modelOut+120,subject->Angle,12);Store(modelOut,164,subject->AnimationFrame);Store(modelOut,168,short(123));const float values[]={77,-33};if(selectedBones==globalBones)std::memcpy(bonesExpected+16,values,8);else std::memcpy(dataExpected+4096,values,8);expectedEvents.push_back(2);}}
  float expectedScale=9.75f;
  if(calc){if(selection==1){modelOut[68]=0;for(unsigned i=0;i<2;++i){expectedEvents.push_back(3);expectedEvents.push_back(4);}expectedEvents.push_back(1);}expectedEvents.push_back(3);expectedScale=selection==3?1.4f:selection==2?1.2f:1.0f;for(unsigned i=0;i<12;++i)Store(objectExpected,304+i*4,float(100+i));if(operation==5)expectedEvents.push_back(7);}
  else if(visible && (operation==2 || operation==3)){float origin[3];std::memcpy(origin,subject->Position,12);if(subject->Type==331)origin[1]+=_DAT_0055290c;else if(subject->Type==330)origin[1]+=_DAT_00552908;float result[12]={};for(unsigned i=0;i<3;++i){result[i]=subject->BoundingBoxMin[i]+origin[i];result[3+4*i]=subject->BoundingBoxMax[i]-subject->BoundingBoxMin[i];}std::memcpy(objectExpected+304,result,48);}
  if(meshEvent){expectedEvents={3,5,6};if(subject->CurrentAction==6)objectExpected[0]=0;}
  Invoke(operation);Require(events==expectedEvents && _DAT_005597c8==expectedScale && geometryPose==(hit?&cached:pose?nullptr:&prepared[1]) && std::memcmp(subject,objectExpected,444)==0 && std::memcmp(activeModel-16,modelExpected,220)==0 && std::memcmp(boneMem,bonesExpected,192)==0 && std::memcmp(data,dataExpected,8192)==0 && std::memcmp(DAT_05826e18,qExpected,sizeof(qExpected))==0 && std::memcmp(parentStore,parentExpected,sizeof(parentExpected))==0);
  Hash(hash,objectExpected,444);Hash(hash,modelExpected,220);Hash(hash,bonesExpected,192);Hash(hash,qExpected,64);Hash(hash,parentExpected,48);for(int event:events)Hash(hash,&event,4);++cases;
 }
 Require(cases==12288 && hits>0);BYTE expectedBatch[188],beforeBatch[444];std::memcpy(expectedBatch,cached.model.data(),188);std::memcpy(beforeBatch,cached.object.data(),444);OBJECT* saved=subject;subject=AsObject(cached.object.data());Context(expectedBatch);BatchContext(cached);Require(std::memcmp(expectedBatch,cached.model.data(),188)==0 && std::memcmp(beforeBatch,cached.object.data(),444)==0);subject=saved;Hash(hash,cached.model.data(),188);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* pool=static_cast<BYTE*>(Reserve(0x27000000u));Require(low && pool);const auto expected=Run(low,pool,low+8192,low+131072);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* highPool=static_cast<BYTE*>(Reserve(std::uint64_t(0x127000000ull)));Require(high && highPool);for(unsigned mask=1;mask<8;++mask){std::memset(pool,0x7b,8192);std::memset(low+8192,0x7b,100000);std::memset(low+131072,0x7b,192);Require(Run(low,mask&1?highPool:pool,(mask&2?high:low)+8192,(mask&4?high:low)+131072)==expected);if(mask&1)for(unsigned i=0;i<8192;++i)Require(pool[i]==0x7b);if(mask&2)for(unsigned i=0;i<100000;++i)Require(low[8192+i]==0x7b);if(mask&4)for(unsigned i=0;i<192;++i)Require(low[131072+i]==0x7b);}std::puts("ocho combinaciones entidad/modelos/paleta altos; cache y aliases intactos");}
 std::printf("Pose entity: 12288 casos, contexto/cache/SIM/render %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 pose=read_source('src/Entity/ObjectPose.cpp',revision);render=read_source('src/Render/Entity_AnimationLegacy.cpp',revision)
 names=('PrepareContext','ReuseBones','ObjectPose_PrepareBones','ObjectPose_PrepareBounds','ObjectPose_PrepareSimulation','ObjectPose_TickMeshEvents')
 bodies=[function(pose,n)for n in names]+[function(render,'Calc_RenderObject'),function(read_source('src/Render/Entity_PrepareRender.cpp',revision),'Entity_PrepareRender')]
 prototypes='\n'.join(b.split('{',1)[0]+';'for b in bodies)
 fields=re.search(r'struct OBJECT\s*\{[\s\S]*?\n\};',read_source('src/Entity/EntityTypes.h',revision)).group();fields=re.sub(r'OBJECT\*\s+Owner;',r'std::uint32_t Owner;',fields)
 fields=re.sub(r'vec34_t\*\s+BoneMatrices;[^\n]*\n\s*vec3_t\s+BoundingBoxMin;[^\n]*',r'union { vec34_t* BoneMatrices; struct { std::uint32_t BoneLow; vec3_t BoundingBoxMin; }; };',fields)
 view='using vec3_t=float[3];using vec34_t=float[3][4];\n#pragma pack(push,1)\n'+fields+'\n#pragma pack(pop)\nstatic_assert(sizeof(OBJECT)==444 && offsetof(OBJECT,BoneMatrices)==276 && offsetof(OBJECT,BoundingBoxMin)==280 && offsetof(OBJECT,OBB)==304);\n'
 structures=pose[pose.index('struct FrozenModel {'):pose.index('// Cada slot conserva capacidad',pose.index('struct FrozenModel {'))]
 code=''.join(bodies)+function(read_source('src/Entity/ObjectBounds.h',revision),'ObjectBounds_Build')
 constants=''.join(re.search(r'^float\s+'+n+r'\s*=\s*[^;]+;',read_source('src/globals.cpp',revision),re.M).group()for n in sorted(set(re.findall(r'\b_DAT_[0-9a-f]+\b',code))))
 services=SERVICES.replace('VIEW',view).replace('TRANSFORMS','#include "'+(ROOT/'src/Render/TransformMath.h').as_posix()+'"').replace('STRUCTURES',structures).replace('FORWARDS',prototypes)
 bounds=function(read_source('src/Entity/ObjectBounds.h',revision),'ObjectBounds_Build')
 body=PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')+constants+services+bounds+''.join(bodies)
 batch=re.search(r'^\s*PrepareContext\([^;\n]*p\.object\.data\(\)[^;\n]*;',pose,re.M).group()
 batch_fn='\nvoid BatchContext(PreparedBones& p){'+batch+'}\n'
 return body+batch_fn+MAIN.replace('ENTITY','subject' if 'OBJECT* entity' in bodies[2] else 'static_cast<int>(reinterpret_cast<uintptr_t>(subject))')
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-pose-entity-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
