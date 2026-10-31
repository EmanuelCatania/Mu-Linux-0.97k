#!/usr/bin/env python3
"""Productores de mallas reales: traza de materiales/atributos, fallback y punteros nativos."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import read_source,function,build_and_run

SERVICES=r'''
#include <cmath>
#include <algorithm>
#include <array>
BYTE* buffers;uintptr_t DAT_05828d58;unsigned DAT_0839bc8c;
#define DAT_0584621c (*reinterpret_cast<DWORD*>(buffers))
#define DAT_077e298c (*reinterpret_cast<DWORD*>(buffers+400000))
#define DAT_060db65c (*reinterpret_cast<char*>(buffers+550000))
#define DAT_06f433bc (*reinterpret_cast<char*>(buffers+1000000))
#define DAT_06f433c0 (*reinterpret_cast<float*>(buffers+1000004))
#define DAT_083a7cc8 (*reinterpret_cast<char*>(buffers+1450000))
#define DAT_05828d5c (*reinterpret_cast<float*>(buffers+1550000))
float WorldTime;
constexpr int MODEL_MAX_VANILLA=963;
std::vector<std::uint32_t> events;unsigned vertices,begins,ends,draws;bool handled;
std::vector<std::array<float,3>> expectedPositions;unsigned passMask;
std::vector<std::array<float,2>> expectedUV;bool checkUV;
BMD* subject;ClothModelMesh* wantedMesh;int wantedModel,wantedIndex;
void Value(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);events.push_back(bits);}
struct Catalog{bool IsTwoSidedMesh(int model,int mesh){Require(model==wantedModel&&mesh==wantedIndex);return (model+mesh)%2!=0;}}gContentCatalog;
namespace Materials{TwoSidedScope::TwoSidedScope(bool){}TwoSidedScope::~TwoSidedScope(){}}
namespace BMDDevice{
MeshScope::MeshScope(bool two):cull(two){events.push_back(10);events.push_back(two);}
MeshScope::~MeshScope(){events.push_back(11);}
void Select(const PassState& s){passMask|=1u<<unsigned(s.pass);events.push_back(1);events.push_back(unsigned(s.pass));events.push_back(s.flags);events.push_back(s.textureSlot);events.push_back(s.alpha);}
}
namespace BufferedDraw{
Vertex current;Stream stream;
void Begin(Primitive p){Require(p==Primitive::Triangles&&!begins++);events.push_back(2);}
void End(){Require(begins&&!ends++);events.push_back(3);}
void NativeColor(float r,float g,float b,float a){events.push_back(4);float c[]={r,g,b,a};std::memcpy(current.color,c,16);for(float f:c)Value(f);}
void NativeUV(float u,float v){events.push_back(6);Value(u);Value(v);current.uv[0]=u;current.uv[1]=v;current.uv[2]=0;current.uv[3]=1;}
void NativeVertex(float x,float y,float z){Require(begins&&!ends&&vertices<expectedPositions.size());const float actual[]={x,y,z};Require(std::memcmp(actual,expectedPositions[vertices].data(),12)==0);if(checkUV)Require(std::memcmp(current.uv,expectedUV[vertices].data(),8)==0);events.push_back(7);Value(x);Value(y);Value(z);++vertices;}
Vertex CaptureAttributes(bool color,bool uv){events.push_back(8);events.push_back(color);events.push_back(uv);return current;}
}
void Bytes(const void* p,std::size_t n){auto* b=static_cast<const BYTE*>(p);for(std::size_t i=0;i<n;++i)events.push_back(b[i]);}
namespace MeshDraw{
bool Draw(const Input& in){++draws;Require(in.key==wantedMesh&&in.model==subject&&in.faces==wantedMesh->Triangles&&in.uv==wantedMesh->TexCoords&&in.rawVertices==wantedMesh->Vertices&&in.rawNormals==wantedMesh->Normals&&in.bodyLight==subject->BodyLight);Require(in.faceCount==wantedMesh->TriangleCount&&in.positionCount==wantedMesh->VertexCount&&in.normalCount==wantedMesh->NormalCount&&in.uvCount==wantedMesh->TexCoordCount);Require(in.positions==reinterpret_cast<float*>(buffers)+wantedIndex*45000&&in.colors==reinterpret_cast<float*>(buffers+550000)+wantedIndex*45000&&in.chromeUV==reinterpret_cast<float*>(buffers+1550000));Require(in.attributes);events.push_back(9);events.push_back(in.mode);events.push_back(in.light);events.push_back(in.offset);Value(in.alpha);Value(in.u);Value(in.v);Bytes(in.attributes,sizeof(*in.attributes));Bytes(in.faces,in.faceCount*36);Bytes(in.uv,in.uvCount*8);Bytes(in.rawVertices,in.positionCount*16);Bytes(in.rawNormals,in.normalCount*20);Bytes(in.bodyLight,12);events.push_back(handled);return handled;}
}
'''
MAIN=r'''
void PutShort(BYTE* p,short v){std::memcpy(p,&v,2);}void PutFloat(BYTE* p,float v){std::memcpy(p,&v,4);}
void Hash(std::uint64_t& h,unsigned v){for(unsigned i=0;i<4;++i){h^=BYTE(v>>(8*i));h*=1099511628211ull;}}
std::uint64_t Run(BYTE* modelMem,BYTE* tables,BYTE* data){buffers=data;std::uint64_t hash=14695981039346656037ull;passMask=0;unsigned handledCount=0,fallbackCount=0;
 const unsigned flagsList[]={0,1,2,4,8,0x200,0x40,0x80,0x42,0x44,0x84,0x204,0x10,0x400,0x401,0x402,0x440};const int blends[]={-2,-1,0,1,3};const float alphas[]={0,.5f,.99f,1};const int slots[]={1,816,1000};
 std::memset(data,0x5a,1600000);
 for(unsigned mesh=0;mesh<2;++mesh){for(unsigned v=0;v<4;++v)for(unsigned a=0;a<3;++a){PutFloat(data+mesh*180000+v*12+a*4,float(int(mesh*10+v*3+a)-7)*.125f);PutFloat(data+1000000+mesh*180000+v*12+a*4,float(int(v+a)-3)*.0625f);}for(unsigned i=0;i<4;++i)PutFloat(data+400000+mesh*60000+i*4,float(i)*.25f);}
 std::vector<BYTE> original(data,data+1600000);
 for(unsigned trial=0;trial<4096;++trial){
  std::memcpy(data,original.data(),1600000);std::memset(modelMem,0xa5,sizeof(BMD)+32);std::memset(tables,0x6d,2048);subject=reinterpret_cast<BMD*>(modelMem+16);const unsigned mesh=trial%2;wantedIndex=mesh;wantedModel=slots[(trial/2)%3];DAT_05828d58=reinterpret_cast<uintptr_t>(subject)-wantedModel*sizeof(BMD);DAT_0839bc8c=trial%4;WorldTime=float(int(trial)-2000)+.25f;
  subject->Meshs=tables;subject->IndexTexture=reinterpret_cast<short*>(tables+192);subject->LightEnable=(trial/7)%2;subject->StreamMesh=char(int((trial/13)%3)-1);subject->Skin=char(int(trial%7)-3);subject->HideSkin=(trial/19)%2;for(unsigned a=0;a<3;++a)subject->BodyLight[a]=float(a+1)*.25f;
  wantedMesh=reinterpret_cast<ClothModelMesh*>(tables)+mesh;wantedMesh->TextureIndex=short(mesh);wantedMesh->VertexCount=4;wantedMesh->NormalCount=4;wantedMesh->TexCoordCount=4;wantedMesh->TriangleCount=short((trial/2)%3);wantedMesh->Unknown0[0]=(trial/23)%5==0;wantedMesh->TexCoords=tables+256;wantedMesh->Triangles=tables+384;wantedMesh->TextureFlags=trial%7?tables+512:nullptr;wantedMesh->Vertices=tables+640;wantedMesh->Normals=tables+768;
  tables[512]=(trial/29)%2;tables[514]=(trial/19)%2;tables[515]=(trial/31)%2;
  const short textures[]={5,6,300,301,65};const short texture=textures[(trial/5)%5];subject->IndexTexture[mesh]=texture;
  for(unsigned i=0;i<4;++i){PutFloat(tables+256+i*8,float(i)*.25f);PutFloat(tables+260+i*8,float(i)*.125f);}
  const int nv=(trial/6)%6;for(int poly=0;poly<wantedMesh->TriangleCount;++poly){BYTE* tri=tables+384+poly*36;tri[0]=BYTE(nv==5?255:nv);for(int i=0;i<4;++i){PutShort(tri+2+i*2,short((i+poly)%4));PutShort(tri+10+i*2,short((i+1)%4));PutShort(tri+18+i*2,short((i+2)%4));}}
  const unsigned flags=flagsList[(trial/3)%17];const float alpha=alphas[(trial/31)%4];const int blend=blends[(trial/17)%5];const float u=trial%3?.125f:0,v=trial%5?-.25f:0;const unsigned overrideTexture=trial%11==0?3:0xffffffffu;
  for(unsigned tex=0;tex<1300;++tex)data[1450000+tex*56]=trial%2?4:3;
  std::vector<BYTE> beforeModel(modelMem,modelMem+sizeof(BMD)+32),beforeTables(tables,tables+2048),before(data,data+1600000);
  events.clear();vertices=begins=ends=draws=0;handled=(trial/37)%2;BufferedDraw::current={};for(float& c:BufferedDraw::current.color)c=.75f;BufferedDraw::current.uv[3]=1;
  // Oráculo de posiciones desde los datos de la muestra, sin leer los índices productivos.
  expectedPositions.clear();const bool direct=trial%41==0,animated=direct||(flags&0x400);const bool deform=direct?((trial/23)%4)&1:bool(flags&0x400);
  // Oráculo UV: prioridad de modos y fórmulas desde las muestras, sin consumir scratch.
  const bool bright=wantedModel>=963&&wantedMesh->TextureFlags&&tables[512];const bool blended=blend<=-2||int(mesh)==blend||(!animated&&bright);const bool chrome=!animated&&!(flags&1)&&(flags&(4|8|0x200));const bool textured=!(flags&1)&&!chrome&&(blended||(flags&2)||!(flags&0x40));checkUV=chrome||textured;expectedUV.clear();
  const bool uvOffset=(int(mesh)==blend||int(mesh)==int(subject->StreamMesh)||(wantedMesh->TextureFlags&&tables[514]))&&(u!=0||v!=0);
  for(int poly=0;poly<wantedMesh->TriangleCount;++poly)for(int corner=0;corner<(nv==5?0:nv);++corner){const unsigned vertex=(corner+poly)%4,normal=(corner+1)%4;std::array<float,3> pos{};for(unsigned a=0;a<3;++a){pos[a]=float(int(mesh*10+vertex*3+a)-7)*.125f;if(animated&&deform){const int phase=int(WorldTime)+931*int(vertex);const float sn=float(std::sin(double(phase)*double(_DAT_005528c4)));pos[a]=sn*(float(int(normal+a)-3)*.0625f)*_DAT_00552644+pos[a];}}expectedPositions.push_back(pos);}
  for(int poly=0;poly<wantedMesh->TriangleCount;++poly)for(int corner=0;corner<(nv==5?0:nv);++corner){std::array<float,2> uv{};const unsigned normal=(corner+1)%4,tex=(corner+2)%4;if(textured){uv={float(tex)*.25f,float(tex)*.125f};if(uvOffset){uv[0]+=u;uv[1]+=v;}}else if(chrome){const float x=float(int(normal)-3)*.0625f,y=float(int(normal)-2)*.0625f,z=float(int(normal)-1)*.0625f;const auto time=static_cast<long long>(WorldTime);const float phase=float(int(time%10000))*_DAT_00552868,oil=float(int(time%5000))*_DAT_005528c0-_DAT_005528b4;if(flags&0x200)uv={oil+oil+(z+x)*_DAT_00552530,oil*_DAT_00552540+y+x};else if(flags&4)uv={z*_DAT_00552504+phase,y*_DAT_00552504+phase+phase};else uv={z*_DAT_00552504+_DAT_005526e4,(y+_DAT_0055256c)*_DAT_00552504};}expectedUV.push_back(uv);}
  float index=float(mesh);if(trial%2){unsigned bits=mesh;std::memcpy(&index,&bits,4);}
  if(trial%41==0)BMD__RenderMeshTranslate(subject,char((trial/23)%4),0,float(mesh)+.25f,int(flags),alpha,blend,u,v,.75f,overrideTexture);
  else BMD__RenderMesh(subject,index,int(flags),alpha,blend,.75f,u,v,overrideTexture);
  Require(begins==ends&&vertices<=8&&draws<=1);if(draws&&handled){Require(!begins);++handledCount;}if(begins){Require(vertices==expectedPositions.size());++fallbackCount;}
  // Sólo BodyLight puede cambiar en el modelo (chrome aditivo con alpha).
  auto light=offsetof(BMD,BodyLight)+16;Require(std::memcmp(modelMem,beforeModel.data(),light)==0&&std::memcmp(modelMem+light+12,beforeModel.data()+light+12,sizeof(BMD)+32-light-12)==0);Require(std::memcmp(tables,beforeTables.data(),2048)==0);
  // Sólo luz por normal y UV chrome son salidas; todos los demás bytes se conservan.
  const unsigned first=550000+mesh*180000,last=first+48;Require(std::memcmp(data,before.data(),first)==0&&std::memcmp(data+last,before.data()+last,1550000-last)==0&&std::memcmp(data+1550032,before.data()+1550032,1600000-1550032)==0);
  for(unsigned e:events)Hash(hash,e);for(unsigned i=0;i<48;++i)Hash(hash,data[first+i]);for(unsigned i=0;i<32;++i)Hash(hash,data[1550000+i]);for(float f:subject->BodyLight){unsigned bits;std::memcpy(&bits,&f,4);Hash(hash,bits);}
 }Require((passMask&255)==255&&handledCount>0&&fallbackCount>0);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);auto expected=Run(low+256,low+4096,low+65536);unsigned routes=1;
 if(sizeof(void*)==8){auto* high=static_cast<BYTE*>(Reserve(0x126000000ull));Require(high);for(unsigned mask=1;mask<8;++mask){std::memset(low+256,0x7b,sizeof(BMD)+32);std::memset(low+4096,0x7b,2048);std::memset(low+65536,0x7b,1600000);Require(Run((mask&1?high:low)+256,(mask&2?high:low)+4096,(mask&4?high:low)+65536)==expected);if(mask&1)for(unsigned i=0;i<sizeof(BMD)+32;++i)Require(low[256+i]==0x7b);if(mask&2)for(unsigned i=0;i<2048;++i)Require(low[4096+i]==0x7b);if(mask&4)for(unsigned i=0;i<1600000;++i)Require(low[65536+i]==0x7b);++routes;}}
 std::printf("MeshRender: 4096 llamadas por ruta, materiales/atributos/bytes %016llx\n",(unsigned long long)expected);
}
'''

def harness(revision=None):
    headers='\n'.join(re.sub(r'#include "[^\"]+"|#pragma once','',read_source(p)) for p in ['src/Render/Materials.h','src/Render/BufferedDraw.h','src/Render/BMDDevice.h','src/Render/MeshDraw.h'])
    constants='\n'.join(re.findall(r'^float\s+_DAT_(?:00552868|00552544|00552580|005528c0|005528b4|00552504|0055256c|005526e4|00552540|00552530|005528c4|00552644)\s*=\s*[^;]+;',read_source('src/globals.cpp'),re.M))
    source=read_source('src/Render/BMD_DrawMesh.cpp',revision)
    funcs=function(read_source('src/Render/BMD_LegacyDraw.cpp',revision),'BMD__RenderMeshTranslate')+function(source,'DecodeMeshIndex')+function(source,'BMD__RenderMesh')
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x200000')+layouts()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\n'+headers+SERVICES+constants+funcs+MAIN

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
    if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-mesh-native-')as t:
        current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
        if a.reference_ref:
            old=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
            if current!=old:raise RuntimeError('Diferencia frente a x86')
            print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
