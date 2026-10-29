#!/usr/bin/env python3
"""Silueta SIM real: entidad nativa, caché/generación e interpolación sin azar en draw.

Builder de geometría, Skeleton y backend controlados; OBJECT es vista escalar x86.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from ItemResetPointer import harness as reset_harness
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
int SceneFlag,worldId,DAT_07e11d30;
struct Map{int GetCurrentMap(){return worldId;}}gMapManager;
float* vertexBuffer;
#define DAT_0584621c (*vertexBuffer)
OBJECT* activeObject;void* activeModel;float* activeBones;char activeTranslate;float savedLod;unsigned frameNumber,randomCalls,skeletonCalls,buildCalls,ownerCalls,alphaCalls;bool continuousFlag;float alphaValue;
short inputs[7][5];unsigned edgeCount;
struct Draw{float first[3],second[3],jitter;};std::vector<Draw> observed;
int ControlledRandom(){Require(randomCalls<edgeCount);const int values[]={0,1,99,100,101,32767,1234567,2147483647};return values[(frameNumber+randomCalls++)%8];}
#define rand ControlledRandom
void Skeleton_Transform(void* model,float* bones,float* minimum,float* maximum,float* bounds,char translate){Require(!skeletonCalls && !buildCalls && model==activeModel && bones==activeBones && minimum==activeObject->BoundingBoxMin && maximum==activeObject->BoundingBoxMax && translate==activeTranslate && _DAT_005597c8==1);++skeletonCalls;for(unsigned i=0;i<15;++i)bounds[i]=float(i);}
void MoltSilhouette_Build(MoltSilhouette* s,unsigned char* vertices,unsigned char* model,unsigned char* entity){Require(skeletonCalls==1 && !buildCalls && model==activeModel && entity==reinterpret_cast<unsigned char*>(activeObject) && vertices==reinterpret_cast<unsigned char*>(vertexBuffer) && _DAT_005597c8==savedLod && !s->vtable && !s->buf8 && !s->edges && !s->count);++buildCalls;for(unsigned i=0;i<3;++i)s->dir[i]=activeObject->Position[i]*.125f+float(i);s->count=edgeCount;s->edges=static_cast<unsigned char*>(::operator new(edgeCount*10));for(unsigned i=0;i<edgeCount;++i)std::memcpy(s->edges+i*10,inputs[i],10);if(frameNumber%2)s->buf8=::operator new(32);}
bool RenderInterpolation_OwnerContinuous(const void* object){Require(object==activeObject);++ownerCalls;return continuousFlag;}
float RenderInterpolation_Alpha(){++alphaCalls;return alphaValue;}
void MoltSilhouette_DrawEdge(void* silhouette,float* first,float* second,float jitter){auto* s=static_cast<MoltSilhouette*>(silhouette);for(unsigned i=0;i<3;++i)Require(s->dir[i]==activeObject->Position[i]*.125f+float(i));Draw draw;std::memcpy(draw.first,first,12);std::memcpy(draw.second,second,12);draw.jitter=jitter;observed.push_back(draw);}
'''
MAIN=r'''
void Tick(){MoltSilhouette_Tick(ARG,activeModel,activeBones,activeTranslate);}
struct Golden{bool present=false,registered=false;std::uint64_t generation=0;std::vector<MoltEdgeSample> edges;};
std::uint64_t Key(const MoltEdgeSample& edge){return std::uint64_t(static_cast<unsigned short>(edge.first))+65536ull*static_cast<unsigned short>(edge.second)+4294967296ull*static_cast<unsigned short>(edge.mesh);}
void Hash(std::uint64_t& hash,const void* p,unsigned n){for(unsigned i=0;i<n;++i){hash^=static_cast<const BYTE*>(p)[i];hash*=1099511628211ull;}}
float Blend(float a,float b,float t){return t==0?a:t==1?b:a+(b-a)*t;}
void CheckDraw(const Golden& current,const Golden& previous,bool blocked,std::uint64_t& hash){
 const bool render=current.present && !blocked && (!current.registered || (SceneObjects::Contains(activeObject) && SceneObjects::Generation(activeObject)==current.generation));const bool continuous=continuousFlag && previous.present && previous.registered==current.registered && previous.generation==current.generation;unsigned expectedAlpha=0;
 observed.clear();ownerCalls=alphaCalls=0;const unsigned randomBefore=randomCalls;MoltSilhouette_Render(nullptr,reinterpret_cast<unsigned char*>(activeObject));Require(randomCalls==randomBefore && ownerCalls==(render?1u:0u) && observed.size()==(render?current.edges.size():0));
 if(render)for(unsigned i=0;i<current.edges.size();++i){const auto& expected=current.edges[i];const MoltEdgeSample* old=nullptr;if(continuous)for(const auto& candidate:previous.edges)if(Key(candidate)==Key(expected)){old=&candidate;break;}for(unsigned a=0;a<3;++a){const float first=old?Blend(old->p1[a],expected.p1[a],alphaValue):expected.p1[a],second=old?Blend(old->p2[a],expected.p2[a],alphaValue):expected.p2[a];Require(observed[i].first[a]==first && observed[i].second[a]==second);if(old)expectedAlpha+=2;}Require(observed[i].jitter==expected.jitter);Hash(hash,observed[i].first,12);Hash(hash,observed[i].second,12);Hash(hash,&observed[i].jitter,4);}
 Require(alphaCalls==expectedAlpha);const auto count=static_cast<unsigned>(observed.size());Hash(hash,&count,4);
}
std::uint64_t Run(BYTE* objectMemory,BYTE* modelMemory,BYTE* boneMemory,BYTE* vertexMemory){
 activeObject=reinterpret_cast<OBJECT*>(objectMemory+16);activeModel=modelMemory+16;activeBones=reinterpret_cast<float*>(boneMemory+16);vertexBuffer=reinterpret_cast<float*>(vertexMemory+16);moltSamples.clear();previousMoltSamples.clear();SceneObjects::LiveNodes().clear();SceneObjects::Generations().clear();moltScene=moltWorld=-1;
 Golden current;int oldScene=-1,oldWorld=-1;std::uint64_t hash=14695981039346656037ull;std::vector<BYTE> vertexBefore(180128);unsigned eligibleFrames=0;
 for(frameNumber=0;frameNumber<512;++frameNumber){
  std::memset(objectMemory,0xa5,476);std::memset(modelMemory,0x3c,220);std::memset(boneMemory,0x6b,192);std::memset(vertexMemory,0x5a,180128);
  activeObject->Live=frameNumber%7!=0;activeObject->Type=frameNumber%11?319:318;const float alpha[]={0,.009f,.01f,.5f,1};activeObject->Alpha=alpha[(frameNumber/3)%5];activeObject->RenderType=frameNumber%13?0:128;activeObject->HiddenMesh=frameNumber%17?-1:-2;for(unsigned a=0;a<3;++a)activeObject->Position[a]=float(int(frameNumber%19)-9+int(a));DAT_07e11d30=frameNumber%19?0:1;activeTranslate=frameNumber%3?1:0;savedLod=_DAT_005597c8=frameNumber%2?3.25f:-2;
  SceneFlag=2+(frameNumber/97)%3;worldId=(frameNumber/79)%4;Golden previous=current;if(SceneFlag!=oldScene || worldId!=oldWorld)previous.present=false;oldScene=SceneFlag;oldWorld=worldId;MoltSilhouette_BeginStep();
  if(frameNumber%9==0)SceneObjects::Register(activeObject);else if(frameNumber%9==1)SceneObjects::Unregister(activeObject);
  edgeCount=frameNumber%7;for(unsigned i=0;i<edgeCount;++i){inputs[i][0]=(i+frameNumber)%6;inputs[i][1]=(i+frameNumber+1)%6;inputs[i][2]=(i+frameNumber/5)%2;inputs[i][3]=static_cast<short>(frameNumber);inputs[i][4]=static_cast<short>(i*7);if(i>0 && frameNumber%4==0 && i==edgeCount-1)std::memcpy(inputs[i],inputs[0],10);}
  for(unsigned mesh=0;mesh<2;++mesh)for(unsigned i=0;i<6;++i)for(unsigned a=0;a<3;++a)vertexBuffer[mesh*45000+i*3+a]=float(frameNumber+mesh*7+i*3+a)*.25f;
  BYTE objectBefore[476],modelBefore[220],bonesBefore[192];std::memcpy(objectBefore,objectMemory,476);std::memcpy(modelBefore,modelMemory,220);std::memcpy(bonesBefore,boneMemory,192);std::memcpy(vertexBefore.data(),vertexMemory,180128);
  current=Golden{};current.present=activeObject->Live && activeObject->Type==319 && activeObject->Alpha>=_DAT_005524f8 && activeObject->RenderType!=128 && (DAT_07e11d30!=0 || activeObject->HiddenMesh!=-2);current.registered=SceneObjects::Contains(activeObject);current.generation=SceneObjects::Generation(activeObject);
  if(current.present){++eligibleFrames;const int randomValues[]={0,1,99,100,101,32767,1234567,2147483647};for(unsigned i=0;i<edgeCount;++i){MoltEdgeSample edge;edge.first=inputs[i][0];edge.second=inputs[i][1];edge.mesh=inputs[i][2];const unsigned base=edge.mesh*45000;std::memcpy(edge.p1,vertexBuffer+base+3*edge.first,12);std::memcpy(edge.p2,vertexBuffer+base+3*edge.second,12);edge.jitter=float(double(randomValues[(frameNumber+i)%8]%100)*0.0099999998);current.edges.push_back(edge);}}
  randomCalls=skeletonCalls=buildCalls=0;Tick();Require(skeletonCalls==(current.present?1u:0u) && buildCalls==skeletonCalls && randomCalls==(current.present?edgeCount:0u) && _DAT_005597c8==savedLod && std::memcmp(objectMemory,objectBefore,476)==0 && std::memcmp(modelMemory,modelBefore,220)==0 && std::memcmp(boneMemory,bonesBefore,192)==0 && std::memcmp(vertexMemory,vertexBefore.data(),180128)==0);
  const auto found=moltSamples.find(activeObject);Require((found!=moltSamples.end())==current.present);if(current.present){const auto& sample=found->second;Require(sample.registered==current.registered && sample.generation==current.generation && sample.edges.size()==current.edges.size());std::unordered_map<std::uint64_t,std::size_t> expectedIndex;for(unsigned i=0;i<current.edges.size();++i){const auto& e=current.edges[i];const auto& actual=sample.edges[i];Require(actual.first==e.first && actual.second==e.second && actual.mesh==e.mesh && actual.jitter==e.jitter && std::memcmp(actual.p1,e.p1,12)==0 && std::memcmp(actual.p2,e.p2,12)==0);if(expectedIndex.find(Key(e))==expectedIndex.end())expectedIndex[Key(e)]=i;}Require(sample.edgeIndex==expectedIndex);for(unsigned a=0;a<3;++a)Require(sample.direction[a]==activeObject->Position[a]*.125f+float(a));}
  if(frameNumber%23==0){SceneObjects::Unregister(activeObject);SceneObjects::Register(activeObject);}else if(frameNumber%25==0)SceneObjects::Unregister(activeObject);
  continuousFlag=frameNumber%3!=0;const float fractions[]={0,.25f,.5f,1};alphaValue=fractions[(frameNumber/3)%4];const bool blocked=frameNumber%31==0;if(blocked)++SceneFlag;CheckDraw(current,previous,blocked,hash);if(blocked)--SceneFlag;CheckDraw(current,previous,false,hash);
  if(frameNumber%29==0){MoltSilhouette_Invalidate(activeObject);current.present=false;Golden none;CheckDraw(none,none,false,hash);Require(moltSamples.find(activeObject)==moltSamples.end() && previousMoltSamples.find(activeObject)==previousMoltSamples.end());}
 }
 Require(eligibleFrames>100);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+4096,low+8192,low+32768);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<16;++mask){std::memset(low,0x7b,220000);Require(Run(mask&1?high:low,(mask&2?high:low)+4096,(mask&4?high:low)+8192,(mask&8?high:low)+32768)==expected);if(mask&1)for(unsigned i=0;i<476;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<220;++i)Require(low[4096+i]==0x7b);if(mask&4)for(unsigned i=0;i<192;++i)Require(low[8192+i]==0x7b);if(mask&8)for(unsigned i=0;i<180128;++i)Require(low[32768+i]==0x7b);}std::puts("16 combinaciones entidad/modelo/paleta/vertices altos; claves y aliases intactos");}
 std::printf("Molt tick: 512 frames, replay/generacion/azar %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 prefix=reset_harness(revision).split('int ItemObjectAttributeImpl(',1)[0].replace('0x10000','0x100000')
 source=read_source('src/Render/Entity_DrawByType.cpp',revision);prefix+=read_source('src/Scene/ObjectRegistry.h',revision).replace('#pragma once','')
 for name in ('MoltSilhouette','MoltEdgeSample','MoltSample'):prefix+=re.search(r'struct '+name+r'\s*\{.*?\};',source,re.S).group()
 prefix+=re.search(r'^std::unordered_map<const void \*, MoltSample>[^;]+;',source,re.M).group()+re.search(r'^int moltScene[^;]+;',source,re.M).group()
 prefix+=re.search(r'std::uint64_t MoltEdgeKey\([^)]*\)\s*\{.*?\n\}',source,re.S).group()
 for name in ('_DAT_005524f8','_DAT_005597c8'):
  if not re.search(r'^float\s+'+name+r'\b',prefix,re.M):prefix+=re.search(r'^float\s+'+name+r'\s*=\s*[^;]+;',read_source('src/globals.cpp',revision),re.M).group()
 prefix+=SERVICES+function(read_source('src/Game/RenderInterpolationMath.cpp',revision),'RenderBlend')
 for name in ('MoltSilhouette_BeginStep','MoltSilhouette_Invalidate','MoltSilhouette_Tick','MoltSilhouette_Render'):prefix+=function(source,name)
 tick=function(source,'MoltSilhouette_Tick');arg='activeObject' if 'OBJECT* entity' in tick else 'static_cast<int>(reinterpret_cast<uintptr_t>(activeObject))'
 return prefix+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-molt-tick-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
