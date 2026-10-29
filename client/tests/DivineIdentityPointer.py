#!/usr/bin/env python3
"""Caché y solver reales de falda Divine; clave de pieza y entidad nativas, modelo legacy separado."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,read_source
SERVICES=r'''
#include <cmath>
using std::uintptr_t;typedef float vec3_t[3];typedef float vec34_t[3][4];
DWORD MoveSceneFrame;float _DAT_00590af0=0;bool continuous;const void* interpolationOwner;
OBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}
char g_BoneVertexBuf[4*15000*12];
bool RenderInterpolation_OwnerContinuous(const void* p){interpolationOwner=p;return continuous;}float RenderInterpolation_Alpha(){return .25f;}float RenderBlend(float a,float b,float t){return a+(b-a)*t;}
void Vector_Transform(float* p,float* m,float* out){for(int i=0;i<3;++i)out[i]=p[0]*m[i*4]+p[1]*m[i*4+1]+p[2]*m[i*4+2]+m[i*4+3];}
void BMD_TransformPosition(void* model,float* matrix,float* p,float* out,char translate){Vector_Transform(p,matrix,out);if(translate){for(int i=0;i<3;++i)out[i]+=reinterpret_cast<float*>(static_cast<BYTE*>(model)+0x6c)[i];}}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,int offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
std::uint64_t Run(BYTE* memory,BYTE* model,BYTE* meshes,BYTE* verts,BYTE* triangles){
 auto* actor=reinterpret_cast<OBJECT*>(memory+16);auto* parts=reinterpret_cast<PART_t*>(memory+4096);std::memset(memory,0x6d,8192);std::memset(model,0,188);std::memset(meshes,0,160);std::memset(verts,0,64);std::memset(triangles,0,72);
 float bones[4*12]{};for(int i=0;i<4;++i){bones[i*12]=bones[i*12+5]=bones[i*12+10]=1;}
 Put(model,0x28,DWORD(reinterpret_cast<uintptr_t>(meshes)));Put(model,0x24,short(4));Put(model,0x68,1.f);for(int i=0;i<3;++i)Put(model,0x6c+i*4,float(100*(i+1)));
 BYTE* mesh=meshes+120;Put(mesh,4,short(4));Put(mesh,10,short(2));Put(mesh,0x10,DWORD(reinterpret_cast<uintptr_t>(verts)));Put(mesh,0x1c,DWORD(reinterpret_cast<uintptr_t>(triangles)));
 const float positions[4][3]={{0,0,0},{20,0,0},{0,50,-20},{20,50,-20}};for(int i=0;i<4;++i){Put(verts,i*16,short(i<2?2:3));std::memcpy(verts+i*16+4,positions[i],12);}const short indices[2][3]={{0,1,2},{2,1,3}};for(int t=0;t<2;++t)for(int i=0;i<3;++i)Put(triangles,t*36+2+i*2,indices[t][i]);
 for(auto& c:g_Slots)Destroy(c);std::uint64_t hash=14695981039346656037ull;auto Mix=[&](unsigned n){hash^=n;hash*=1099511628211ull;};auto MixFloat=[&](float n){Require(std::isfinite(n));std::uint32_t bits;std::memcpy(&bits,&n,4);Mix(bits);};
 for(int trial=0;trial<256;++trial){int mode=trial%13;PART_t* part=parts+(trial/2)%20;actor->BoneMatrices=reinterpret_cast<vec34_t*>(bones);actor->InitialSceneFrame=trial/40;actor->Angle[2]=mode==11?45.f:0.f;MoveSceneFrame=mode==3?trial+10:mode==4?1:trial;Put(model,0x24,short(mode==5?3:4));Put(mesh,4,short(mode==7?0:4));
  if(mode==7)for(auto& c:g_Slots)Destroy(c);if(mode==6)actor->BoneMatrices=nullptr;
  OBJECT* object=mode==1?nullptr:actor;PART_t* key=mode==2?nullptr:part;void* modelArg=mode==0?model:mode==12?nullptr:model;int modelType=mode==10?706:707;
  if(mode==8){auto* c=Find(part);if(!c->vertices)Require(Create(*c,model,bones));c->spawnFrame=actor->InitialSceneFrame;c->lastFrame=int(MoveSceneFrame)-1;for(int i=0;i<c->numLinks;++i)c->links[i].dist[1]=0;c->vertices[2].pos.x=100000.f;}
  BYTE actorBefore[512],partsBefore[480],modelBefore[188],bonesBefore[sizeof(bones)];std::memcpy(actorBefore,memory,512);std::memcpy(partsBefore,parts,480);std::memcpy(modelBefore,model,188);std::memcpy(bonesBefore,bones,sizeof(bones));Tick(object,modelType,key,modelArg);if(mode==8)for(auto& c:g_Slots)Require(Key(c)!=reinterpret_cast<uintptr_t>(part)||!c.vertices);
  if(mode==7)for(auto& c:g_Slots)Require(!c.part&&!c.vertices);
  for(auto& c:g_Slots){uintptr_t address=Key(c);if(address){Require(address>=reinterpret_cast<uintptr_t>(parts)&&address<reinterpret_cast<uintptr_t>(parts+20)&&(address-reinterpret_cast<uintptr_t>(parts))%sizeof(PART_t)==0);Mix(unsigned((address-reinterpret_cast<uintptr_t>(parts))/sizeof(PART_t))+1);}else Mix(0);Mix(c.numVertices);Mix(c.numLinks);Mix(c.lastFrame);Mix(c.spawnFrame);Mix(c.presentationContinuous);
   if(c.vertices){Require(c.numVertices==4);for(int i=0;i<4;++i){MixFloat(c.vertices[i].pos.x);MixFloat(c.vertices[i].pos.y);MixFloat(c.vertices[i].pos.z);}}
  }
  float* output=VT(kSkirtMesh,0);for(int i=0;i<12;++i)output[i]=-777.f;float expected[12];for(float& f:expected)f=-777.f;continuous=trial%2==0;interpolationOwner=nullptr;
  if(modelType==707&&object&&key&&modelArg)for(auto& c:g_Slots)if(Key(c)==reinterpret_cast<uintptr_t>(key)&&c.vertices&&c.spawnFrame==object->InitialSceneFrame){float alpha=c.presentationContinuous&&continuous?.25f:1.f;for(int i=0;i<4;++i){expected[i*3]=RenderBlend(c.vertices[i].presentationPrevious.x,c.vertices[i].pos.x,alpha);expected[i*3+1]=RenderBlend(c.vertices[i].presentationPrevious.y,c.vertices[i].pos.y,alpha);expected[i*3+2]=RenderBlend(c.vertices[i].presentationPrevious.z,c.vertices[i].pos.z,alpha);}break;}
  Apply(object,modelType,key,modelArg);Require(std::memcmp(expected,output,sizeof(expected))==0);if(interpolationOwner)Require(interpolationOwner==actor);for(float f:expected)MixFloat(f);
  Require(std::memcmp(actorBefore,memory,512)==0&&std::memcmp(partsBefore,parts,480)==0&&std::memcmp(modelBefore,model,188)==0&&std::memcmp(bonesBefore,bones,sizeof(bones))==0);
 }
 for(auto& c:g_Slots)Destroy(c);for(int i=0;i<16;++i){auto* c=Find(parts+i);Require(c==&g_Slots[i]);c->lastFrame=i;}Require(Find(parts+16)==&g_Slots[0]&&Key(g_Slots[0])==reinterpret_cast<uintptr_t>(parts+16));for(int i=1;i<16;++i)Require(Key(g_Slots[i])==reinterpret_cast<uintptr_t>(parts+i));for(auto& c:g_Slots)Destroy(c);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* model=static_cast<BYTE*>(Reserve(0x28000000u));auto* mesh=static_cast<BYTE*>(Reserve(0x2a000000u));auto* verts=static_cast<BYTE*>(Reserve(0x2c000000u));auto* tris=static_cast<BYTE*>(Reserve(0x2e000000u));Require(low&&model&&mesh&&verts&&tris);auto expected=Run(low,model,mesh,verts,tris);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));auto* highModel=static_cast<BYTE*>(Reserve(uintptr_t(0x128000000ull)));auto* sign=static_cast<BYTE*>(Reserve(0x8a000000u));Require(high&&highModel&&sign);BYTE alias[8192];std::memcpy(alias,low,8192);Require(Run(high,highModel,mesh,verts,tris)==expected&&std::memcmp(alias,low,8192)==0);auto* lowPart=reinterpret_cast<PART_t*>(low+4096);auto* highPart=reinterpret_cast<PART_t*>(high+4096);auto* first=Find(lowPart);auto* second=Find(highPart);Require(first!=second&&Key(*first)==reinterpret_cast<uintptr_t>(lowPart)&&Key(*second)==reinterpret_cast<uintptr_t>(highPart));Destroy(*second);first->vertices=new Vertex[1]{};first->numVertices=1;first->spawnFrame=reinterpret_cast<OBJECT*>(high+16)->InitialSceneFrame;float* output=VT(kSkirtMesh,0);for(int i=0;i<3;++i)output[i]=-777.f;Apply(reinterpret_cast<OBJECT*>(high+16),707,highPart,highModel);for(int i=0;i<3;++i)Require(output[i]==-777.f);for(auto& c:g_Slots)Destroy(c);Require(Run(sign,highModel,mesh,verts,tris)==expected);
 }std::printf("Divine identity:256 ticks y draws/ruta %016llx\n",(unsigned long long)expected);}
'''
def struct(source,name):
 a=source.index('struct '+name+' {');b=source.index('};',a)+2;return source[a:b]
def harness(revision=None):
 entity=read_source('src/Entity/EntityTypes.h',revision);types='#pragma pack(push,1)\n'+struct(entity,'OBJECT')+struct(entity,'PART_t')+'\n#pragma pack(pop)\n'
 source=read_source('src/Physics/Cloth_Simulation.cpp',revision);start=source.index('namespace {',source.index('TELA POR MALLA'));source=source[start:]
 if revision:
  wrappers=r'''uintptr_t Key(const MeshCloth& c){return static_cast<DWORD>(c.part);}MeshCloth* Find(PART_t* p){return FindSlot(static_cast<int>(reinterpret_cast<uintptr_t>(p)));}void Tick(OBJECT* o,int type,PART_t* p,void* model){DivineSkirt_Tick(static_cast<int>(reinterpret_cast<uintptr_t>(o)),type,static_cast<int>(reinterpret_cast<uintptr_t>(p)),model);}void Apply(OBJECT* o,int type,PART_t* p,void* model){DivineSkirt_Apply(static_cast<int>(reinterpret_cast<uintptr_t>(o)),type,static_cast<int>(reinterpret_cast<uintptr_t>(p)),model);}'''
 else:
  wrappers=r'''uintptr_t Key(const MeshCloth& c){return reinterpret_cast<uintptr_t>(c.part);}MeshCloth* Find(PART_t* p){return FindSlot(p);}void Tick(OBJECT* o,int type,PART_t* p,void* model){DivineSkirt_Tick(o,type,p,model);}void Apply(OBJECT* o,int type,PART_t* p,void* model){DivineSkirt_Apply(o,type,p,model);}'''
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.split('DWORD MoveSceneFrame',1)[0]+types+'DWORD MoveSceneFrame'+SERVICES.split('DWORD MoveSceneFrame',1)[1]+source+wrappers+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-divine-identity-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
