#!/usr/bin/env python3
"""Lector BMD completo: archivos v10/v12, construcción nativa, postcarga y baja.

Oráculo de archivo independiente en Python. Usa CRT y asignaciones reales;
texturas/backend son servicios controlados. No prueba el writer ni SMD.
"""
import argparse,os,re,tempfile
from pathlib import Path
from ModelReleaseLifecycle import prefix
from ModelPostLoad import SERVICES as POST_SERVICES
from BMDFileOracle import prepare
from SceneObjectPointer import ROOT,read_source,function,build_and_run
SERVICES=r'''
#include <fstream>
#include <iterator>
#include <string>
#include <cmath>
#define crt_sprintf std::sprintf
char DAT_005580ac[]="rb";unsigned adjCalls,initCalls,finishCalls;int endCursor,endSize;
FILE* crt_fopen(const char* p,const char* m){return std::fopen(p,m);}
void DbgLogPublic(const char*){}
namespace Platform{constexpr int TruncateText=0;template<std::size_t N,class...A>int TextFormatTruncated(char(&s)[N],std::size_t cap,int,const char* f,A...a){return std::snprintf(s,cap,f,a...);}}
struct Block{BYTE* raw;std::size_t n;bool live,checked;};std::vector<Block> blocks;std::unordered_map<void*,unsigned> ids;
void* operator_new(std::size_t n){Require(n<16*1024*1024);auto* p=static_cast<BYTE*>(std::malloc(n+32));Require(p);std::memset(p,0xd7,n+32);std::memset(p+16,0xa5,n);blocks.push_back({p,n,true,false});ids[p+16]=unsigned(blocks.size()-1);if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(p+16)>0xffffffffull);return p+16;}
void Guard(const Block& b){for(unsigned i=0;i<16;++i)Require(b.raw[i]==0xd7&&b.raw[16+b.n+i]==0xd7);}
void operator_delete(void* p){if(!p)return;Require(ids.count(p));auto& b=blocks[ids[p]];Require(b.live);Guard(b);b.live=false;std::free(b.raw);}
void UnloadImage(int){}
void Size(void* p,std::size_t n){Require(ids.count(p));auto& b=blocks[ids[p]];Require(b.live&&b.n==n);Guard(b);b.checked=true;}
void Bytes(void* p,const std::vector<BYTE>& bytes){Size(p,bytes.size());if(!bytes.empty())Require(std::memcmp(p,bytes.data(),bytes.size())==0);}
void Finish(int c,int s){++finishCalls;endCursor=c;endSize=s;}
struct Reader{std::vector<BYTE> data;std::size_t at=0;Reader(){std::ifstream f("../oracle.bin",std::ios::binary);Require(bool(f));data.assign(std::istreambuf_iterator<char>(f),{});}unsigned U(){Require(at+4<=data.size());unsigned v=0;for(unsigned i=0;i<4;++i)v|=unsigned(data[at++])<<(i*8);return v;}std::vector<BYTE> Blob(){auto n=U();Require(at+n<=data.size());std::vector<BYTE> result(data.begin()+at,data.begin()+at+n);at+=n;return result;}};
std::uint64_t hash=14695981039346656037ull;
void Hash(const void* p,std::size_t n){auto* b=static_cast<const BYTE*>(p);while(n--){hash^=*b++;hash*=1099511628211ull;}}
void HashBlob(const std::vector<BYTE>& b){Hash(b.data(),b.size());}
struct MeshGold{BmdFile::MeshHeader h;std::array<std::vector<BYTE>,6> b;};
struct ActionGold{BmdFile::ActionHeader h;std::vector<BYTE> b;};
struct BoneGold{BYTE dummy;std::vector<BYTE> h;std::vector<std::array<std::vector<BYTE>,2>> keys;};
struct GuardModel{BYTE before[16];BMD model;BYTE after[16];};static GuardModel storage;
'''
MAIN=r'''
int main(){Reader reader;auto count=reader.U();unsigned successful=0;
 for(unsigned trial=0;trial<count;++trial){
  blocks.clear();ids.clear();adjCalls=initCalls=finishCalls=0;auto leaf=reader.Blob();std::string name(leaf.begin(),leaf.end());auto kind=reader.U();BmdFile::Header header{};unsigned size=0,cursor=0;
  std::vector<MeshGold> meshes;std::vector<ActionGold> actions;std::vector<BoneGold> bones;
  if(kind!=1){auto h=reader.Blob();Require(h.size()==42);std::memcpy(&header,h.data(),42);}
  if(kind==0){size=reader.U();cursor=reader.U();meshes.resize(header.MeshCount);actions.resize(header.ActionCount);bones.resize(header.BoneCount);
   for(auto& g:meshes){auto h=reader.Blob();Require(h.size()==10);std::memcpy(&g.h,h.data(),10);for(auto& b:g.b)b=reader.Blob();}
   for(auto& g:actions){auto h=reader.Blob();Require(h.size()==3);std::memcpy(&g.h,h.data(),3);g.b=reader.Blob();}
   for(auto& g:bones){auto flag=reader.Blob();Require(flag.size()==1);g.dummy=flag[0];g.h=reader.Blob();if(!g.dummy){g.keys.resize(header.ActionCount);for(auto& k:g.keys)for(auto& b:k)b=reader.Blob();}}
  }
  std::memset(&storage,0xa5,sizeof(storage));auto* m=&storage.model;const uintptr_t sentinel=sizeof(void*)==8?uintptr_t(0x123456780ull):uintptr_t(0x23456780u);m->Meshs=m->Bones=m->Actions=m->Textures=m->LightMaps=reinterpret_cast<BYTE*>(sentinel);m->IndexTexture=reinterpret_cast<short*>(sentinel);auto expected=storage;
  CALL_OPEN;
  for(float& value:expected.model.BodyLight)value=1;
  if(kind!=1){expected.model.Version=char(header.Version);std::memcpy(expected.model.Name,header.Name,32);}
  if(kind==2){expected.model.NumBones=expected.model.NumActions=expected.model.NumMeshs=0;expected.model.Meshs=expected.model.Bones=expected.model.Actions=expected.model.Textures=nullptr;expected.model.IndexTexture=nullptr;}
  if(kind==0){++successful;Require(adjCalls==1&&initCalls==1&&finishCalls==1&&endCursor==int(cursor)&&endSize==int(size));Require(m->NumBones==header.BoneCount&&m->NumActions==header.ActionCount&&m->NumMeshs==header.MeshCount);
   Size(m->Meshs,(std::max)(int(header.MeshCount),1)*sizeof(ClothModelMesh));Size(m->Bones,(std::max)(int(header.BoneCount),1)*sizeof(ModelAnimationBone));Size(m->Actions,(std::max)(int(header.ActionCount),1)*sizeof(ModelAnimationAction));Size(m->Textures,(std::max)(int(header.MeshCount),1)*32);Size(m->IndexTexture,(std::max)(int(header.MeshCount),1)*2);
   auto* mm=reinterpret_cast<ClothModelMesh*>(m->Meshs);auto* aa=reinterpret_cast<ModelAnimationAction*>(m->Actions);auto* bb=reinterpret_cast<ModelAnimationBone*>(m->Bones);
   std::vector<std::uint16_t> verticesPerBone(header.BoneCount,0);std::vector<std::array<float,3>> minimum(header.BoneCount),maximum(header.BoneCount);
   for(int b=0;b<header.BoneCount;++b)for(unsigned a=0;a<3;++a){minimum[b][a]=9999.f;maximum[b][a]=-9999.f;}
   for(unsigned i=0;i<meshes.size();++i){const auto& g=meshes[i];auto& mesh=mm[i];Require(mesh.VertexCount==g.h.VertexCount&&mesh.NormalCount==g.h.NormalCount&&mesh.TexCoordCount==g.h.TexCoordCount&&mesh.TriangleCount==g.h.TriangleCount&&mesh.TextureIndex==g.h.TextureIndex);
    Bytes(mesh.Vertices,g.b[0]);Bytes(mesh.Normals,g.b[1]);Bytes(mesh.TexCoords,g.b[2]);Size(mesh.Triangles,g.b[3].size());Require(std::memcmp(m->Textures+i*32,g.b[4].data(),32)==0);
    for(int f=0;f<g.h.TriangleCount;++f){auto* actual=mesh.Triangles+f*36;auto* wanted=g.b[3].data()+f*36;Require(std::memcmp(actual,wanted,26)==0&&std::memcmp(actual+32,wanted+32,4)==0);Hash(actual,36);}
    if(g.b[5].empty())Require(mesh.TextureFlags==nullptr);else Bytes(mesh.TextureFlags,g.b[5]);
    ClothModelMesh shape;std::memset(&shape,0xa5,sizeof(shape));shape.Unknown0[0]=0;shape.TextureIndex=g.h.TextureIndex;shape.VertexCount=g.h.VertexCount;shape.NormalCount=g.h.NormalCount;shape.TexCoordCount=g.h.TexCoordCount;shape.TriangleCount=g.h.TriangleCount;shape.Vertices=mesh.Vertices;shape.Normals=mesh.Normals;shape.TexCoords=mesh.TexCoords;shape.Triangles=mesh.Triangles;shape.TextureFlags=mesh.TextureFlags;Require(std::memcmp(&shape,&mesh,sizeof(shape))==0);
    for(int v=0;v<g.h.VertexCount;++v){ModelRuntimeVertex vertex;std::memcpy(&vertex,g.b[0].data()+v*16,16);Require(vertex.Bone>=0&&vertex.Bone<header.BoneCount);++verticesPerBone[vertex.Bone];for(unsigned a=0;a<3;++a){if(vertex.Position[a]<minimum[vertex.Bone][a])minimum[vertex.Bone][a]=vertex.Position[a];if(maximum[vertex.Bone][a]<vertex.Position[a])maximum[vertex.Bone][a]=vertex.Position[a];}}
    for(unsigned part:{0u,1u,2u,4u,5u})HashBlob(g.b[part]);Hash(&g.h,sizeof(g.h));
   }
   for(unsigned i=0;i<actions.size();++i){const auto& g=actions[i];auto& action=aa[i];Require(action.FrameCount==g.h.FrameCount&&action.LockPositions==g.h.LockPositions);ModelAnimationAction shape;std::memset(&shape,0xa5,sizeof(shape));shape.Unknown0[0]=0;shape.FrameCount=g.h.FrameCount;shape.LockPositions=g.h.LockPositions;if(g.h.LockPositions){Bytes(action.Positions,g.b);shape.Positions=action.Positions;}Require(std::memcmp(&shape,&action,sizeof(shape))==0);Hash(&g.h,sizeof(g.h));HashBlob(g.b);}
   for(unsigned b=0;b<bones.size();++b){const auto& g=bones[b];auto& bone=bb[b];Require(bone.Dummy==g.dummy);ModelAnimationBone shape;std::memset(&shape,0xa5,sizeof(shape));shape.Dummy=g.dummy;
    if(!g.dummy){Require(g.h.size()==34);std::memcpy(shape.Name,g.h.data(),32);std::memcpy(&shape.Parent,g.h.data()+32,2);Size(bone.Keys,actions.size()*sizeof(ModelAnimationKeys));shape.Keys=bone.Keys;
     for(unsigned a=0;a<actions.size();++a){auto& k=bone.Keys[a];Bytes(k.Positions,g.keys[a][0]);Bytes(k.EulerAngles,g.keys[a][1]);auto frames=actions[a].h.FrameCount;Size(k.Quaternions,std::size_t(frames)*16);std::vector<float> euler(std::size_t(frames)*3),expectedQ(std::size_t(frames)*4);if(frames)std::memcpy(euler.data(),g.keys[a][1].data(),g.keys[a][1].size());for(int f=0;f<frames;++f)EulerToQuat(euler.data()+f*3,expectedQ.data()+f*4,0,0);if(frames)Require(std::memcmp(k.Quaternions,expectedQ.data(),expectedQ.size()*4)==0);HashBlob(g.keys[a][0]);HashBlob(g.keys[a][1]);}
    }
    shape.BoundingBoxEnabled=verticesPerBone[b]!=0;const auto& hi=maximum[b];const auto& lo=minimum[b];float corners[8][3]={{hi[0],hi[1],hi[2]},{hi[0],hi[1],lo[2]},{hi[0],lo[1],hi[2]},{hi[0],lo[1],lo[2]},{lo[0],hi[1],hi[2]},{lo[0],hi[1],lo[2]},{lo[0],lo[1],hi[2]},{lo[0],lo[1],lo[2]}};std::memcpy(shape.BoundingBox,corners,sizeof(corners));Require(std::memcmp(&shape,&bone,sizeof(shape))==0);Hash(&bone.Dummy,1);Hash(bone.Name,32);Hash(&bone.Parent,2);Hash(bone.BoundingBox,sizeof(bone.BoundingBox));
   }
   for(const auto& block:blocks)if(block.live)Require(block.checked);
   expected.model.NumBones=header.BoneCount;expected.model.NumActions=header.ActionCount;expected.model.NumMeshs=header.MeshCount;expected.model.Meshs=m->Meshs;expected.model.Bones=m->Bones;expected.model.Actions=m->Actions;expected.model.Textures=m->Textures;expected.model.IndexTexture=m->IndexTexture;expected.model.BoneHead=-1;const BYTE ff=255;std::memcpy(&expected.model.StreamMesh,&ff,1);
  }else Require(adjCalls==0&&initCalls==0&&finishCalls==0);
  Require(std::memcmp(&storage,&expected,sizeof(storage))==0);Hash(m->Name,32);Hash(&m->Version,1);Hash(m->BodyLight,12);
  if(kind==0){for(unsigned i=0;i<meshes.size();++i)m->IndexTexture[i]=short(1000+i);BMD__Release(m);if(header.BoneCount!=0)for(const auto& block:blocks)Require(!block.live);}
  for(auto& block:blocks)if(block.live){Require(kind==0&&header.BoneCount==0);Guard(block);std::free(block.raw);block.live=false;}
 }
 Require(reader.at==reader.data.size());std::printf("BMDLoader: %u archivos (%u válidos), v10/v12/grafos/postcarga/baja %016llx ptr=%zu\n",count,successful,(unsigned long long)hash,sizeof(void*));
}
'''
def harness(revision=None):
 source=read_source('src/Render/BMD_Load.cpp',revision);body=function(source,'BMD__Open');needle="operator_delete(Buffer);\n    BMD__Init(thisPtr, '\\0');";assert needle in body;body=body.replace(needle,"Finish(cursor,fileSize);\n    "+needle)
 post=read_source('src/Render/BMD_Load.cpp');funcs='\n'.join(function(post,n)for n in ['BMD__FindTriangleForEdge','BMD__FindNearTriangle','BMD_CreateBoundingBox','BMD__Init']);funcs=funcs.replace('void __cdecl BMD__FindNearTriangle(', 'void __cdecl Post_FindNearTriangle(').replace('void __cdecl BMD__Init(','void __cdecl Post_Init(')
 # El extractor puede omitir __cdecl: renombrar también esa variante.
 funcs=funcs.replace('void BMD__FindNearTriangle(', 'void Post_FindNearTriangle(').replace('void BMD__Init(','void Post_Init(')
 wrappers='\nvoid BMD__FindNearTriangle(void* p){++adjCalls;Post_FindNearTriangle(p);}\nvoid BMD__Init(void* p,char f){++initCalls;Post_Init(p,f);}\n'
 euler=function(read_source('src/Render/Render_LegacyTransforms.cpp'),'EulerToQuat');const=re.search(r'^float\s+_DAT_00552ce0\s*=\s*[^;]+;',read_source('src/globals.cpp'),re.M).group()
 code=prefix()+'\n#include "'+(ROOT/'src/Model/BMDFileTypes.h').as_posix()+'"\n'+POST_SERVICES+SERVICES+const+euler+funcs+wrappers+function(read_source('src/Render/SMD_Parser.cpp'),'BMD__Release')+body
 arg='m' if 'BMD* param_1' in body else 'static_cast<int>(reinterpret_cast<uintptr_t>(m))'
 return code+MAIN.replace('CALL_OPEN','BMD__Open('+arg+',"../samples/",name.c_str(),0)')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
 if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-bmd-loader-')as t:
  directory=Path(t);prepare(ROOT,directory);current=build_and_run(directory/'current',harness(),a);print(current,end='')
  if a.reference_ref:
   other=build_and_run(directory/'reference',harness(a.reference_ref),a)
   if current!=other:raise RuntimeError('Diferencia de lectura BMD frente a x86')
   print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
