#!/usr/bin/env python3
"""Liberación real BMD: grafo de asignaciones, orden, canarios y punteros altos."""
import argparse,os,tempfile
from pathlib import Path
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import read_source,function,build_and_run
PREFIX=r'''
#include <cstdint>
namespace Smd { inline void Forget(const void*) {} inline bool SelectModel(const void*) { return true; } }
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#ifndef _WIN32
#define __cdecl
#endif
using BYTE=std::uint8_t;using DWORD=std::uint32_t;
void Require(bool ok){if(!ok){std::fputs("Grafo/orden/canario BMD incorrecto\n",stderr);std::abort();}}
'''
SERVICES=r'''
struct Block{BYTE* raw;std::size_t size;bool live;std::vector<BYTE> expected;};
std::vector<Block> blocks;std::unordered_map<void*,unsigned> ids;std::vector<std::pair<int,int>> events;
unsigned Id(void* p){if(!p)return 0;Require(ids.count(p)==1);return ids[p];}
void* New(std::size_t n){n=(std::max)(n,std::size_t(1));auto* raw=static_cast<BYTE*>(std::malloc(n+32));Require(raw);std::memset(raw,0xd7,n+32);std::memset(raw+16,0x6b,n);blocks.push_back({raw,n,true,{}});ids[raw+16]=unsigned(blocks.size());if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(raw+16)>0xffffffffull);return raw+16;}
void Check(const Block& b){for(unsigned i=0;i<16;++i)Require(b.raw[i]==0xd7&&b.raw[16+b.size+i]==0xd7);Require(b.expected.empty()||std::memcmp(b.raw+16,b.expected.data(),b.size)==0);}
void operator_delete(void* p){unsigned id=Id(p);events.push_back({1,int(id)});if(!p)return;auto& b=blocks[id-1];Require(b.live);Check(b);b.live=false;std::free(b.raw);}
void UnloadImage(int id){events.push_back({2,id});}
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
'''
MAIN=r'''
struct BoneSpec{bool dummy;unsigned keys;std::vector<std::array<unsigned,3>> streams;};
struct ActionSpec{bool locked;unsigned positions;};struct MeshSpec{std::array<unsigned,4> streams;unsigned flags;int texture;};
struct Guarded{BYTE before[16];BMD value;BYTE after[16];};static Guarded storage;
int main(){
 const short counts[]={-1,0,1,2,3};std::uint64_t digest=14695981039346656037ull;unsigned calls=0,retained=0;
 for(unsigned trial=0;trial<625;++trial){
  blocks.clear();ids.clear();events.clear();std::memset(&storage,0xa5,sizeof(storage));auto* m=&storage.value;const short nb=counts[trial%5],na=counts[(trial/5)%5],nm=counts[(trial/25)%5];m->NumBones=nb;m->NumActions=na;m->NumMeshs=nm;
  const bool nullable=trial%2;
  m->Meshs=nm<=0&&nullable?nullptr:static_cast<BYTE*>(New((std::max)(int(nm),1)*sizeof(ClothModelMesh)));
  m->Bones=nb<=0&&nullable?nullptr:static_cast<BYTE*>(New((std::max)(int(nb),1)*sizeof(ModelAnimationBone)));
  m->Actions=na<=0&&nullable?nullptr:static_cast<BYTE*>(New((std::max)(int(na),1)*sizeof(ModelAnimationAction)));
  m->Textures=nm<=0&&nullable?nullptr:static_cast<BYTE*>(New((std::max)(int(nm),1)*32));
  m->IndexTexture=nm<=0&&nullable?nullptr:static_cast<short*>(New((std::max)(int(nm),1)*2));
  m->LightMaps=reinterpret_cast<BYTE*>(uintptr_t(sizeof(void*)==8?0x1e12345678ull:0x12345678u));
  auto* bones=reinterpret_cast<ModelAnimationBone*>(m->Bones);auto* actions=reinterpret_cast<ModelAnimationAction*>(m->Actions);auto* meshes=reinterpret_cast<ClothModelMesh*>(m->Meshs);
  auto Stream=[&](unsigned key){return key%4?New(32+key%5*4):nullptr;};
  std::vector<BoneSpec> bs;std::vector<ActionSpec> as;std::vector<MeshSpec> ms;
  for(int b=0;b<nb;++b){bones[b].Dummy=BYTE((trial+b)%3?0:255);BoneSpec spec{bones[b].Dummy!=0,0,{}};
   if(!spec.dummy){bones[b].Keys=static_cast<ModelAnimationKeys*>(New((std::max)(int(na),1)*sizeof(ModelAnimationKeys)));spec.keys=Id(bones[b].Keys);for(int a=0;a<na;++a){auto& k=bones[b].Keys[a];k.Positions=static_cast<float*>(Stream(trial+b*7+a*3));k.EulerAngles=static_cast<float*>(Stream(trial+b*7+a*3+1));k.Quaternions=static_cast<float*>(Stream(trial+b*7+a*3+2));spec.streams.push_back({Id(k.Positions),Id(k.EulerAngles),Id(k.Quaternions)});}}
   bs.push_back(spec);
  }
  for(int a=0;a<na;++a){actions[a].LockPositions=BYTE((trial+a)%3?128:0);actions[a].Positions=actions[a].LockPositions?static_cast<float*>(Stream(trial+a+1)):nullptr;as.push_back({actions[a].LockPositions!=0,Id(actions[a].Positions)});}
  for(int i=0;i<nm;++i){auto& mesh=meshes[i];mesh.TextureIndex=short(i);m->IndexTexture[i]=short((trial+i)%3==0?301:(trial+i)%3==1?-5:500+i);mesh.Vertices=static_cast<BYTE*>(Stream(trial+i));mesh.Normals=Stream(trial+i+1);mesh.TexCoords=Stream(trial+i+2);mesh.Triangles=static_cast<BYTE*>(Stream(trial+i+3));mesh.TextureFlags=(trial+i)%2?New(4):nullptr;ms.push_back({{Id(mesh.Vertices),Id(mesh.Normals),Id(mesh.TexCoords),Id(mesh.Triangles)},Id(mesh.TextureFlags),m->IndexTexture[i]});}
  std::array<unsigned,5> roots={Id(m->Meshs),Id(m->Bones),Id(m->Actions),Id(m->Textures),Id(m->IndexTexture)};
  for(auto& b:blocks)b.expected.assign(b.raw+16,b.raw+16+b.size);
  if(nb!=0&&m->Meshs){auto& bytes=blocks[Id(m->Meshs)-1].expected;for(int i=0;i<nm;++i)std::memset(bytes.data()+i*sizeof(ClothModelMesh)+offsetof(ClothModelMesh,TextureFlags),0,sizeof(void*));}
  std::vector<std::pair<int,int>> expected;
  if(nb!=0){for(const auto& b:bs)if(!b.dummy){for(const auto& k:b.streams)for(auto id:k)expected.push_back({1,int(id)});expected.push_back({1,int(b.keys)});}for(const auto& a:as)if(a.locked)expected.push_back({1,int(a.positions)});for(const auto& mesh:ms){for(auto id:mesh.streams)expected.push_back({1,int(id)});if(mesh.flags)expected.push_back({1,int(mesh.flags)});if(mesh.texture!=301)expected.push_back({2,mesh.texture});}for(auto id:roots)if(id)expected.push_back({1,int(id)});}
  auto modelExpected=storage;if(nb!=0){modelExpected.value.Meshs=nullptr;modelExpected.value.Bones=nullptr;modelExpected.value.Actions=nullptr;modelExpected.value.Textures=nullptr;modelExpected.value.IndexTexture=nullptr;modelExpected.value.NumBones=modelExpected.value.NumActions=modelExpected.value.NumMeshs=0;}
  CALL_RELEASE;
  Require(events==expected&&std::memcmp(&storage,&modelExpected,sizeof(storage))==0);
  for(const auto& e:events)for(std::uint32_t word:{std::uint32_t(e.first),std::uint32_t(e.second)})for(unsigned j=0;j<4;++j){digest^=BYTE(word>>(j*8));digest*=1099511628211ull;}
  for(auto& b:blocks){if(nb!=0)Require(!b.live);if(b.live){Check(b);std::free(b.raw);++retained;}}
  events.clear();BMD__Release(CALL_MODEL);Require(events.empty()&&std::memcmp(&storage,&modelExpected,sizeof(storage))==0);++calls;
 }
 std::printf("ModelRelease: %u ciclos/doble baja; grafo/orden/canarios %016llx retenciones originales=%u\n",calls,(unsigned long long)digest,retained);
}
'''
def prefix():
 return PREFIX+layouts()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\nstatic_assert(sizeof(ClothModelMesh)==(sizeof(void*)==4?40:60));static_assert(offsetof(ClothModelMesh,TextureIndex)==2);\n'
def harness(revision=None):
 body=function(read_source('src/Render/SMD_Parser.cpp',revision),'BMD__Release');arg='m' if 'BMD* model' in body else 'static_cast<int>(reinterpret_cast<uintptr_t>(m))'
 return prefix()+SERVICES+body+MAIN.replace('CALL_RELEASE','BMD__Release('+arg+')').replace('CALL_MODEL',arg)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
 if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-model-release-')as t:
  current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
  if a.reference_ref:
   other=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
   if current!=other:raise RuntimeError('Diferencia de liberación frente a x86')
   print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
