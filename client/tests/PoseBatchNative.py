#!/usr/bin/env python3
"""Ciclo real de caché de poses y geometría con layouts nativos.

El equipo de trabajo divide rangos en orden controlado; animación, caché y
TransformMath son reales. No prueba el loader ni el backend gráfico.
"""
import argparse,os,re,tempfile
from pathlib import Path
from AnimationBonesPointer import harness as animation_harness
from ObjectLayoutContract import object_types,record
from SceneObjectPointer import ROOT,read_source,function,build_and_run

SERVICES=r'''
#include <array>
#include <unordered_map>
#include <algorithm>
using vec34_t=float[3][4];
uintptr_t DAT_05828d58,DAT_07abf5d0,DAT_07abf5d8;int SceneFlag,DAT_07e11d30,DAT_005597c4;DWORD MoveSceneFrame;
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}OBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
const BMD* AsModel(const void* p){return static_cast<const BMD*>(p);}
namespace ResourceTables{std::size_t ModelCount(){return 2;}}
namespace TransformWorkers{
unsigned available=2,runs=0;bool reverse=false;
struct Team{unsigned Workers(){return available;}void Run(std::size_t n,void* c,void(*f)(void*,std::size_t,std::size_t)){++runs;if(reverse){f(c,n/2,n);f(c,0,n/2);}else{f(c,0,n/2);f(c,n/2,n);}};}team;
Team& SharedTeam(){return team;}
}
float globalBoneFallback[200*12];
#define DAT_06970a9c globalBoneFallback[0]
void BMD_Animation(void*,float*,float,unsigned,unsigned char,unsigned*,float*,char,char);
int Calc_RenderObject(OBJECT*,char,int){return 0;}
void Skeleton_Transform(void*,float*,float*,float*,float*,char){Require(false);}
int PlayBuffer(int,const OBJECT*,bool){Require(false);return 0;}
void BMD__RenderMeshEffect(void*,int,int){Require(false);}
'''
MAIN=r'''
static CHARACTER characters[400];static BMD models[2];static ModelAnimationBone descriptors[2][3];static ModelAnimationAction actions[2][2];static ModelAnimationKeys keys[2][3][2];
static float translations[2][3][2][3][3],eulers[2][3][2][3][3],quats[2][3][2][3][4];
struct Vertex{short bone,padding;float p[3];};struct Normal{short bone,padding;float p[3];int extra;};static_assert(sizeof(Vertex)==16&&sizeof(Normal)==20);
static Vertex vertices[2][4096];static Normal normals[2][4];static ClothModelMesh meshes[2];
static char globalQuats[200*16];static float globalParent[12];unsigned poseChecks,geometryChecks,missChecks;std::uint64_t digest=14695981039346656037ull;
void Hash(const void* p,std::size_t n){const auto* b=static_cast<const BYTE*>(p);while(n--){digest^=*b++;digest*=1099511628211ull;}}
std::array<float,200*12> Consume(unsigned slot,bool hit){
 auto* o=&characters[slot].Object;BMD* m=models+o->Type;const BMD before=*m;const OBJECT ownerBefore=*o;
 std::array<float,200*12> direct,cached;direct.fill(77.25f);cached=direct;
 std::memset(globalQuats,0x5a,sizeof(globalQuats));for(unsigned i=0;i<12;++i)globalParent[i]=float(i)*.25f;
 char seedQ[sizeof(globalQuats)];float seedP[12];std::memcpy(seedQ,globalQuats,sizeof(seedQ));std::memcpy(seedP,globalParent,sizeof(seedP));
 batchActive=false;ObjectPose_PrepareBones(o,m,direct.data(),1);BMD expected=*m;char expectedQ[sizeof(globalQuats)];float expectedP[12];std::memcpy(expectedQ,globalQuats,sizeof(expectedQ));std::memcpy(expectedP,globalParent,sizeof(expectedP));
 *m=before;std::memcpy(globalQuats,seedQ,sizeof(seedQ));std::memcpy(globalParent,seedP,sizeof(seedP));batchActive=true;ObjectPose_PrepareBones(o,m,cached.data(),1);
 Require((geometryPose==&prepared[slot])==hit);Require(hit||geometryPose==nullptr);
 Require(std::memcmp(&expected,m,sizeof(BMD))==0&&std::memcmp(direct.data(),cached.data(),sizeof(direct))==0&&std::memcmp(expectedQ,globalQuats,sizeof(expectedQ))==0&&std::memcmp(expectedP,globalParent,sizeof(expectedP))==0);Hash(cached.data(),sizeof(cached));Hash(globalQuats,sizeof(globalQuats));Hash(globalParent,sizeof(globalParent));Hash(m->BodyAngle,12);Hash(&m->CurrentAnimation,6);Hash(&m->CurrentAction,1);Require(std::memcmp(o,&ownerBefore,sizeof(OBJECT))==0);++poseChecks;return cached;
}
int main(){
 DAT_07abf5d0=reinterpret_cast<uintptr_t>(characters);DAT_05828d58=reinterpret_cast<uintptr_t>(models);DAT_07abf5d8=reinterpret_cast<uintptr_t>(characters);quaternionGlobal=globalQuats;parentGlobal=globalParent;
 if(sizeof(void*)==8)Require(DAT_07abf5d0>0xffffffffull&&DAT_05828d58>0xffffffffull&&reinterpret_cast<uintptr_t>(descriptors)>0xffffffffull&&reinterpret_cast<uintptr_t>(actions)>0xffffffffull&&reinterpret_cast<uintptr_t>(keys)>0xffffffffull&&reinterpret_cast<uintptr_t>(vertices)>0xffffffffull&&reinterpret_cast<uintptr_t>(normals)>0xffffffffull);
 for(unsigned trial=0;trial<32;++trial){
  ObjectPose_EndBatch();std::memset(characters,0,sizeof(characters));std::memset(models,0,sizeof(models));std::memset(descriptors,0,sizeof(descriptors));std::memset(actions,0,sizeof(actions));std::memset(meshes,0,sizeof(meshes));
  SceneFlag=5;DAT_07e11d30=0;DAT_005597c4=int(trial%2);TransformWorkers::available=2;TransformWorkers::reverse=trial%2;
  for(unsigned m=0;m<2;++m){
   models[m].NumBones=3;models[m].NumActions=2;models[m].NumMeshs=1;models[m].Bones=reinterpret_cast<BYTE*>(descriptors[m]);models[m].Actions=reinterpret_cast<BYTE*>(actions[m]);models[m].Meshs=reinterpret_cast<BYTE*>(meshes+m);models[m].BoneHead=-1;models[m].ShadowAngle[2]=float(trial%3)*15;
   meshes[m].VertexCount=4096;meshes[m].NormalCount=4;meshes[m].Vertices=reinterpret_cast<BYTE*>(vertices[m]);meshes[m].Normals=normals[m];
   for(unsigned a=0;a<2;++a)actions[m][a].FrameCount=3;
   for(unsigned b=0;b<3;++b){descriptors[m][b].Parent=short(b?b-1:-1);descriptors[m][b].Dummy=BYTE(b==2&&trial%2);descriptors[m][b].Keys=keys[m][b];for(unsigned a=0;a<2;++a){keys[m][b][a]={&translations[m][b][a][0][0],&eulers[m][b][a][0][0],&quats[m][b][a][0][0]};for(unsigned f=0;f<3;++f){for(unsigned axis=0;axis<3;++axis){translations[m][b][a][f][axis]=float(m+b+a+f+axis)*.25f;eulers[m][b][a][f][axis]=float(int(axis)-1)*.125f*float(trial%3);}EulerToQuat(eulers[m][b][a][f],quats[m][b][a][f],0,0);}}}
   for(unsigned v=0;v<4096;++v){vertices[m][v].bone=short(v%2);for(unsigned a=0;a<3;++a)vertices[m][v].p[a]=float((v+a)%17)*.125f;}
   for(unsigned n=0;n<4;++n){normals[m][n].bone=short(n%2);for(unsigned a=0;a<3;++a)normals[m][n].p[a]=a==n%3?1.f:0.f;}
  }
  for(unsigned slot=0;slot<4;++slot){auto& o=characters[slot].Object;o.Live=1;o.Visible=1;o.Alpha=1;o.Type=short(slot%2);o.LightEnable=BYTE((slot+trial)%2);o.Scale=.5f+float(slot)*.25f;o.CurrentAction=BYTE(slot%2);o.PriorAction=BYTE((slot+1)%2);o.AnimationFrame=float(trial%9)*.25f;o.PriorAnimationFrame=.75f;for(unsigned a=0;a<3;++a){o.Position[a]=float(slot*3+a);o.Angle[a]=float(trial%3)*5;}}
  const unsigned mode=trial%16;bool active=true;
  if(mode==2){descriptors[0][2].Parent=2;descriptors[1][2].Dummy=0;descriptors[1][2].Parent=2;active=false;}
  if(mode==3){TransformWorkers::available=0;active=false;}
  if(mode==4){characters[3].Object.Live=0;active=false;}
  if(mode==5){meshes[0].VertexCount=1;meshes[1].VertexCount=1;active=false;}
  if(mode==6){characters[3].Object.Alpha=0;active=false;}
  if(mode==7){DAT_07e11d30=2;active=false;}
  if(mode==8){SceneFlag=2;active=false;}
  if(mode==9){models[0].NumBones=0;models[1].NumBones=0;active=false;}
  if(mode==10){characters[3].Object.Visible=0;active=false;}
  if(mode==11){characters[3].Object.Type=2;active=false;}
  if(mode==12)characters[0].Object.Visible=0;
  if(mode==13){descriptors[0][2].Dummy=descriptors[1][2].Dummy=1;vertices[0][0].bone=vertices[1][0].bone=2;}
  if(mode==13||mode==14)for(unsigned i=4;i<400;++i)characters[i].Object=characters[i%4].Object;
  if(mode==14)models[0].NumMeshs=models[1].NumMeshs=0;
  if(mode==15){models[0].NumActions=models[1].NumActions=0;active=false;}
  BMD modelBefore[2];std::memcpy(modelBefore,models,sizeof(models));std::vector<CHARACTER> poolBefore(characters,characters+400);const unsigned runs=TransformWorkers::runs;
  ObjectPose_BeginBatch();Require(batchActive==active);Require(TransformWorkers::runs-runs==unsigned(active));Require(std::memcmp(models,modelBefore,sizeof(models))==0&&std::memcmp(characters,poolBefore.data(),sizeof(characters))==0);
  if(!active)continue;
  for(unsigned slot=0;slot<4;++slot){auto output=Consume(slot,true);auto& o=characters[slot].Object;auto& m=models[o.Type];auto& mesh=meshes[o.Type];m.LightEnable=o.LightEnable;
   if(!prepared[slot].geometry){float dummy[3]{};Require(!ObjectPose_RestoreGeometry(&m,output.data(),1,1,dummy,dummy,dummy,dummy,dummy));++missChecks;continue;}
   float direction[3]={0,0,0},coefficients[3]={_DAT_00552530,_DAT_005528b4,_DAT_005526e4};if(m.LightEnable){float in[3]={0,-1.5f,0},rot[12];if(DAT_005597c4){in[0]=1.3f;in[1]=0;in[2]=2;}Matrix_BuildFromEuler(m.ShadowAngle,rot);Vector_InverseRotate(in,rot,direction);}
   std::vector<float> pos(45000,91),norm(45000,92),light(15000,93),expectedPos=pos,expectedNorm=norm,expectedLight=light;
   TransformMath::Mesh source{reinterpret_cast<const BYTE*>(vertices[o.Type]),reinterpret_cast<const BYTE*>(normals[o.Type]),expectedPos.data(),expectedNorm.data(),expectedLight.data(),4096,4};TransformMath::Batch reference{&source,1,output.data(),1,m.BodyScale,{m.BodyOrigin[0],m.BodyOrigin[1],m.BodyOrigin[2]},{direction[0],direction[1],direction[2]},{coefficients[0],coefficients[1],coefficients[2]},1.f==_DAT_0055256c,true,m.LightEnable!=0};TransformMath::Run(&reference,0,4100);
   Require(ObjectPose_RestoreGeometry(&m,output.data(),1,1,direction,coefficients,pos.data(),norm.data(),light.data()));Require(pos==expectedPos&&norm==expectedNorm&&light==expectedLight);Hash(pos.data(),pos.size()*sizeof(float));Hash(norm.data(),norm.size()*sizeof(float));Hash(light.data(),light.size()*sizeof(float));++geometryChecks;
   for(unsigned invalid=0;invalid<(sizeof(void*)==8?13u:12u);++invalid){auto oldM=m;auto oldMesh=mesh;auto oldVertex=vertices[o.Type][0];auto oldNormal=normals[o.Type][0];auto oldBone=output[0];float d[3];std::memcpy(d,direction,12);float c[3];std::memcpy(c,coefficients,12);char translate=1;float scale=1;
    if(invalid==0)translate=0;if(invalid==1)scale=2;if(invalid==2)d[0]+=1;if(invalid==3)c[1]+=.5f;if(invalid==4)output[0]+=1;if(invalid==5)vertices[o.Type][0].p[0]+=.5f;if(invalid==6)normals[o.Type][0].p[0]+=.5f;if(invalid==7)mesh.Vertices+=16;if(invalid==8)--mesh.VertexCount;if(invalid==9)m.Meshs=reinterpret_cast<BYTE*>(meshes+(1-o.Type));if(invalid==10)m.BodyScale+=1;if(invalid==11)m.LightEnable^=1;if(invalid==12)m.Meshs=reinterpret_cast<BYTE*>(reinterpret_cast<uintptr_t>(m.Meshs)^std::uint64_t(0x100000000ull));
    std::fill(pos.begin(),pos.end(),91);std::fill(norm.begin(),norm.end(),92);std::fill(light.begin(),light.end(),93);Require(!ObjectPose_RestoreGeometry(&m,output.data(),scale,translate,d,c,pos.data(),norm.data(),light.data()));Require(std::all_of(pos.begin(),pos.end(),[](float f){return f==91;})&&std::all_of(norm.begin(),norm.end(),[](float f){return f==92;})&&std::all_of(light.begin(),light.end(),[](float f){return f==93;}));++missChecks;
    m=oldM;mesh=oldMesh;vertices[o.Type][0]=oldVertex;normals[o.Type][0]=oldNormal;output[0]=oldBone;
   }
   auto oldObject=o;auto oldModel=m;auto oldDescriptor=descriptors[o.Type][0];auto oldAction=actions[o.Type][0];auto oldLastDescriptor=descriptors[o.Type][2];auto oldLastAction=actions[o.Type][1];
   for(unsigned invalid=0;invalid<11;++invalid){if(invalid==0)o.AnimationFrame+=.25f;if(invalid==1)o.PriorAnimationFrame+=.25f;if(invalid==2)o.PriorAction=BYTE(1-o.PriorAction);if(invalid==3)o.Angle[0]+=5;if(invalid==4)o.HeadAngle[0]+=.5f;if(invalid==5)m.BoneHead=1;if(invalid==6)descriptors[o.Type][0].Name[0]^=1;if(invalid==7)actions[o.Type][0].Unknown0[0]^=1;if(invalid==8)m.BodyHeight+=1;if(invalid==9)reinterpret_cast<BYTE*>(&descriptors[o.Type][2].BoundingBox[7][2])[3]^=1;if(invalid==10)actions[o.Type][1].Positions=reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(actions[o.Type][1].Positions)^uintptr_t(1));Consume(slot,false);++missChecks;o=oldObject;m=oldModel;descriptors[o.Type][0]=oldDescriptor;actions[o.Type][0]=oldAction;descriptors[o.Type][2]=oldLastDescriptor;actions[o.Type][1]=oldLastAction;}
  }
 }
 ObjectPose_EndBatch();Require(!batchActive&&geometryPose==nullptr);std::printf("PoseBatchNative: %u poses, %u geometrías, %u invalidaciones; ptr=%zu OBJECT=%zu BMD=%zu mesh=%zu hash=%016llx\n",poseChecks,geometryChecks,missChecks,sizeof(void*),sizeof(OBJECT),sizeof(BMD),sizeof(ClothModelMesh),(unsigned long long)digest);
}
'''

def harness(revision=None):
 prefix=animation_harness();prefix=prefix[:re.search(r'(?:static )?void AnimateWithScratch\(',prefix).start()]
 prefix+='\nusing vec34_t=float[3][4];\n'+object_types()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\nstatic_assert(sizeof(ClothModelMesh)==(sizeof(void*)==4?40:60));\n'
 prefix+='\n#include "'+(ROOT/'src/Render/TransformMath.h').as_posix()+'"\n'
 source=read_source('src/Entity/ObjectPose.cpp',revision);source=re.sub(r'^#include[^\n]*\n','',source,flags=re.M)
 existing=set(re.findall(r'\bfloat\s+(\w+)\s*=',prefix,re.M));names=set(re.findall(r'\b_DAT_[0-9a-f]+\b',source));globals=read_source('src/globals.cpp')
 constants='\n'.join(re.search(r'^float\s+'+n+r'\s*=\s*[^;]+;',globals,re.M).group()for n in sorted(names-existing))
 return prefix+SERVICES+constants+function(read_source('src/Math/Math_3D.cpp'),'Vector_InverseRotate')+function(read_source('src/Entity/ObjectBounds.h'),'ObjectBounds_Build')+source+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
 if a.reference_ref and (os.name!='nt' or a.architecture!='Win32'):p.error('La referencia usa layouts x86')
 with tempfile.TemporaryDirectory(prefix='mu-pose-batch-')as t:
  current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
  if a.reference_ref:
   reference=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
   if current!=reference:raise RuntimeError('Diferencia de caché frente a referencia x86')
   print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
