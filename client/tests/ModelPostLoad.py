#!/usr/bin/env python3
"""Postcarga BMD real: adyacencias, cajas por hueso y reset de animación."""
import argparse,os,tempfile
from pathlib import Path
from ModelReleaseLifecycle import prefix
from SceneObjectPointer import read_source,function,build_and_run
SERVICES=r'''
#include <cmath>
struct Counts{std::uint32_t before;short rows[200];std::uint32_t after;};
struct Rows{std::uint32_t before;float rows[600];std::uint32_t after;};
Counts countData;Rows maximumData,minimumData;
#define DAT_077d87fc countData.rows
#define DAT_05827a98 maximumData.rows
#define DAT_06f42a5c minimumData.rows
#define DAT_05827a9c (*(DWORD*)((BYTE*)maximumData.rows+4))
#define DAT_05827aa0 (*(DWORD*)((BYTE*)maximumData.rows+8))
#define DAT_06f42a60 (*(DWORD*)((BYTE*)minimumData.rows+4))
#define DAT_06f42a64 (*(DWORD*)((BYTE*)minimumData.rows+8))
static_assert(sizeof(Counts)==408&&sizeof(Rows)==2408);
'''
MAIN=r'''
struct GuardModel{BYTE before[16];BMD model;BYTE after[16];};static GuardModel storage;
std::uint64_t hash=14695981039346656037ull;
void Hash(const void* p,std::size_t n){const auto* b=static_cast<const BYTE*>(p);while(n--){hash^=*b++;hash*=1099511628211ull;}}
float FromBits(std::uint32_t b){float f;std::memcpy(&f,&b,4);return f;}
int main(){
 const short boneCounts[]={-1,0,1,3,17,200},meshCounts[]={-1,0,1,2,3};
 const std::uint32_t patterns[]={0,0x80000000u,0x3f800000u,0xbf800000u,1,0x7fc12345u,0x7f800000u,0xff800000u,0x47435000u,0xc7435000u};
 for(unsigned trial=0;trial<513;++trial){
  std::memset(&storage,0x6b,sizeof(storage));auto* model=&storage.model;model->NumBones=trial==512?1:boneCounts[trial%6];model->NumMeshs=trial==512?3:meshCounts[(trial/6)%5];model->NumActions=2;
  std::vector<ModelAnimationBone> bones(201);std::memset(bones.data(),0xa5,bones.size()*sizeof(bones[0]));std::array<ClothModelMesh,3> meshes;std::memset(meshes.data(),0xa5,sizeof(meshes));
  std::array<std::vector<ModelRuntimeVertex>,3> vertices;std::array<std::vector<ModelRuntimeTriangle>,3> triangles;
  for(unsigned b=0;b<201;++b){bones[b].Keys=nullptr;bones[b].Dummy=17;const char* names[]={"Du","DU","dummy","Other"};std::memcpy(bones[b].Name,names[(b+trial)%4],2);}
  model->Bones=model->NumBones<=0&&trial%2?nullptr:reinterpret_cast<BYTE*>(bones.data());model->Meshs=model->NumMeshs<=0&&trial%2?nullptr:reinterpret_cast<BYTE*>(meshes.data());
  for(unsigned m=0;m<3;++m){
   const short sizes[]={-1,0,1,9,17};meshes[m].VertexCount=trial==512?short(m==2?2:32767):sizes[(trial+m)%5];vertices[m].resize((std::max)(int(meshes[m].VertexCount),1));std::memset(vertices[m].data(),0xa5,vertices[m].size()*sizeof(ModelRuntimeVertex));
   for(int v=0;v<meshes[m].VertexCount;++v){auto& vertex=vertices[m][v];vertex.Bone=trial==512?0:short((trial+v*7+m)%unsigned(model->NumBones>0?model->NumBones:200));for(unsigned axis=0;axis<3;++axis){auto bits=patterns[(trial+v+axis+m)%10];std::memcpy(vertex.Position+axis,&bits,4);}}
   meshes[m].Vertices=meshes[m].VertexCount<=0&&trial%2?nullptr:reinterpret_cast<BYTE*>(vertices[m].data());
   const unsigned topology=(trial+m)%9;std::vector<std::array<short,3>> faces;std::vector<std::array<short,3>> links;
   if(topology==2){faces={{{0,1,2}}};links={{{-1,-1,-1}}};}
   if(topology==3){faces={{{0,1,2}},{{1,0,3}}};links={{{1,-1,-1}},{{0,-1,-1}}};}
   if(topology==4){faces={{{0,1,2}},{{0,1,3}}};links={{{-1,-1,-1}},{{-1,-1,-1}}};}
   if(topology==5){faces={{{0,1,2}},{{1,0,3}},{{1,0,4}}};links={{{1,-1,-1}},{{0,-1,-1}},{{-1,-1,-1}}};}
   if(topology==6){faces={{{0,0,1}},{{0,0,2}}};links={{{1,-1,-1}},{{0,-1,-1}}};}
   if(topology==7)for(short pair=0;pair<3;++pair){short b=short(pair*10);faces.push_back({b,short(b+1),short(b+2)});faces.push_back({short(b+1),b,short(b+3)});links.push_back({short(pair*2+1),-1,-1});links.push_back({short(pair*2),-1,-1});}
   if(topology==8){faces={{{-1,0,1}},{{0,-1,2}}};links={{{1,-1,-1}},{{0,-1,-1}}};}
   meshes[m].TriangleCount=topology==0?-1:short(faces.size());triangles[m].resize((std::max)(faces.size(),std::size_t(1)));std::memset(triangles[m].data(),0xa5,triangles[m].size()*sizeof(ModelRuntimeTriangle));for(unsigned f=0;f<faces.size();++f)std::memcpy(triangles[m][f].VertexIndex,faces[f].data(),6);
   meshes[m].Triangles=meshes[m].TriangleCount<=0&&trial%2?nullptr:reinterpret_cast<BYTE*>(triangles[m].data());
   // El oráculo topológico fija explícitamente el resultado de cada caso.
   if(int(m)<model->NumMeshs)for(unsigned f=0;f<links.size();++f)std::memcpy(triangles[m][f].Adjacent,links[f].data(),6);
  }
  auto expectedTriangles=triangles;
  // Restaurar adyacencias arbitrarias: la función debe inicializar sólo las activas.
  for(int m=0;m<model->NumMeshs;++m)for(int f=0;f<meshes[m].TriangleCount;++f)for(short& edge:triangles[m][f].Adjacent)edge=77;
  const auto modelBefore=storage;auto meshBefore=meshes;auto vertexBefore=vertices;
  BMD__FindNearTriangle(model);
  Require(std::memcmp(&storage,&modelBefore,sizeof(storage))==0&&std::memcmp(meshes.data(),meshBefore.data(),sizeof(meshes))==0);
  for(unsigned m=0;m<3;++m){Require(std::memcmp(triangles[m].data(),expectedTriangles[m].data(),triangles[m].size()*sizeof(ModelRuntimeTriangle))==0);Hash(triangles[m].data(),triangles[m].size()*sizeof(ModelRuntimeTriangle));}
  // Una arista ya resuelta no debe alterarse al consultar otra vez.
  for(int m=0;m<model->NumMeshs;++m)for(int f=0;f<meshes[m].TriangleCount;++f)if(triangles[m][f].Adjacent[0]!=-1)BMD__FindTriangleForEdge(model,m,f,0);
  for(unsigned m=0;m<3;++m)Require(std::memcmp(triangles[m].data(),expectedTriangles[m].data(),triangles[m].size()*sizeof(ModelRuntimeTriangle))==0);
  countData.before=maximumData.before=minimumData.before=0x13572468;countData.after=maximumData.after=minimumData.after=0x24681357;std::memset(countData.rows,0xa5,sizeof(countData.rows));for(unsigned i=0;i<600;++i){maximumData.rows[i]=FromBits(0x3cf0abcd);minimumData.rows[i]=FromBits(0xbcf0abcd);}
  auto expectedCounts=countData;auto expectedMaximum=maximumData;auto expectedMinimum=minimumData;auto expectedBones=bones;auto expectedModel=storage;
  for(int b=0;b<model->NumBones;++b){expectedCounts.rows[b]=0;for(unsigned a=0;a<3;++a){expectedMaximum.rows[b*3+a]=FromBits(0xc61c3c00);expectedMinimum.rows[b*3+a]=FromBits(0x461c3c00);}}
  std::array<std::vector<float>,600> samples;
  for(int m=0;m<model->NumMeshs;++m)for(int v=0;v<meshes[m].VertexCount;++v){const auto& vertex=vertices[m][v];auto* counts=reinterpret_cast<std::uint16_t*>(expectedCounts.rows);++counts[vertex.Bone];for(unsigned a=0;a<3;++a)if(!std::isnan(vertex.Position[a]))samples[vertex.Bone*3+a].push_back(vertex.Position[a]);}
  for(unsigned i=0;i<600;++i)if(!samples[i].empty()){auto lo=std::min_element(samples[i].begin(),samples[i].end());auto hi=std::max_element(samples[i].begin(),samples[i].end());if(*lo<expectedMinimum.rows[i])expectedMinimum.rows[i]=*lo;if(expectedMaximum.rows[i]<*hi)expectedMaximum.rows[i]=*hi;}
  const unsigned route=trial%3;const char flag=route==2?char(trial%2?-1:1):0;
  for(int b=0;b<model->NumBones;++b){auto& expected=expectedBones[b];if(flag)expected.Dummy=expected.Name[0]=='D'&&expected.Name[1]=='u';expected.BoundingBoxEnabled=reinterpret_cast<std::uint16_t*>(expectedCounts.rows)[b]!=0;const float* hi=expectedMaximum.rows+b*3;const float* lo=expectedMinimum.rows+b*3;float corners[8][3]={{hi[0],hi[1],hi[2]},{hi[0],hi[1],lo[2]},{hi[0],lo[1],hi[2]},{hi[0],lo[1],lo[2]},{lo[0],hi[1],hi[2]},{lo[0],hi[1],lo[2]},{lo[0],lo[1],hi[2]},{lo[0],lo[1],lo[2]}};std::memcpy(expected.BoundingBox,corners,sizeof(corners));}
  if(route){expectedModel.model.BoneHead=-1;const BYTE unset=255;std::memcpy(&expectedModel.model.StreamMesh,&unset,1);BMD__Init(model,flag);}else BMD_CreateBoundingBox(BOUND_MODEL);
  Require(std::memcmp(&storage,&expectedModel,sizeof(storage))==0&&std::memcmp(bones.data(),expectedBones.data(),bones.size()*sizeof(ModelAnimationBone))==0&&std::memcmp(meshes.data(),meshBefore.data(),sizeof(meshes))==0);
  Require(std::memcmp(&countData,&expectedCounts,sizeof(countData))==0&&std::memcmp(&maximumData,&expectedMaximum,sizeof(maximumData))==0&&std::memcmp(&minimumData,&expectedMinimum,sizeof(minimumData))==0);
  for(unsigned m=0;m<3;++m)Require(std::memcmp(vertices[m].data(),vertexBefore[m].data(),vertices[m].size()*sizeof(ModelRuntimeVertex))==0);
  if(trial==512)Require(reinterpret_cast<std::uint16_t*>(countData.rows)[0]==0&&bones[0].BoundingBoxEnabled==0);
  for(unsigned b=0;b<201;++b){Hash(&bones[b].Dummy,1);Hash(&bones[b].BoundingBoxEnabled,1);Hash(bones[b].BoundingBox,sizeof(bones[b].BoundingBox));}Hash(&countData,sizeof(countData));Hash(&maximumData,sizeof(maximumData));Hash(&minimumData,sizeof(minimumData));Hash(&model->BoneHead,4);Hash(&model->StreamMesh,1);
  if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(model)>0xffffffffull&&reinterpret_cast<uintptr_t>(bones.data())>0xffffffffull&&reinterpret_cast<uintptr_t>(meshes.data())>0xffffffffull);
 }
 std::printf("ModelPostLoad: 513 casos topología/cajas/init, NaN/ceros/wrap16/canarios %016llx\n",(unsigned long long)hash);
}
'''
def harness(revision=None):
 source=read_source('src/Render/BMD_Load.cpp',revision);names=('BMD__FindTriangleForEdge','BMD__FindNearTriangle','BMD_CreateBoundingBox','BMD__Init');bodies='\n'.join(function(source,n)for n in names)
 arg='model' if 'BMD* model' in function(source,'BMD_CreateBoundingBox') else 'static_cast<int>(reinterpret_cast<uintptr_t>(model))'
 return prefix()+SERVICES+bodies+MAIN.replace('BOUND_MODEL',arg)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
 if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-model-postload-')as t:
  current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
  if a.reference_ref:
   other=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
   if current!=other:raise RuntimeError('Diferencia de postcarga frente a x86')
   print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
