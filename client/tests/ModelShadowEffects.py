#!/usr/bin/env python3
"""Sombras y efectos BMD reales, adaptador residente y oráculos de orden/posiciones/RNG."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import read_source,function,build_and_run

SERVICES=r'''
#include <array>
#include <cmath>
#include <unordered_map>
#include <algorithm>
using UINT_PTR=uintptr_t;using byte=BYTE;
BYTE* worldData;BMD* subject;ClothModelMesh* meshData;
#define DAT_0584621c (*reinterpret_cast<DWORD*>(worldData))
#define g_BoneVertexBuf worldData
constexpr int ANTILAG_SHADOWS=3;
std::vector<unsigned> events;bool antialag;
unsigned Bits(float f){unsigned u;std::memcpy(&u,&f,4);return std::isnan(f)?0x7fc00000u:u;}
void Event(unsigned id,std::initializer_list<unsigned> args={}){events.push_back(id);events.insert(events.end(),args.begin(),args.end());}
void Vec(const float* p,unsigned n){for(unsigned i=0;i<n;++i)events.push_back(Bits(p[i]));}
struct Settings{bool GetAntilag(int id){Require(id==ANTILAG_SHADOWS);Event(1,{unsigned(antialag)});return antialag;}}gUserSettings;
unsigned randomCalls;const int randomValues[]={0,1,2,10,12,24,50,100,32767,13,37,60};
int Random(unsigned n){return randomValues[n%12];}
int _rand(){int v=Random(randomCalls++);Event(2,{unsigned(v)});return v;}
float* CreateEffect(int type,float* pos,float* angle,float* light,float* p4,float* p5,float* p6,float* p7,byte flag){Require(!p4&&!p5&&uintptr_t(p6)==0xffffffffu&&!p7&&!flag);Require(reinterpret_cast<BYTE*>(pos)>=worldData&&reinterpret_cast<BYTE*>(pos)+12<=worldData+540128);auto offset=unsigned((reinterpret_cast<BYTE*>(pos)-worldData)/12);Event(3,{unsigned(type),offset});Vec(pos,3);Vec(angle,3);Vec(light,3);return nullptr;}
namespace BMDDevice{
void Select(const PassState& s){Event(4,{unsigned(s.pass),s.flags,unsigned(s.textureSlot),unsigned(s.alpha)});}
void BeginModel(){Event(5);}void EndModel(){Event(6);}
}
namespace BufferedDraw{
void Begin(Primitive p){Require(p==Primitive::Triangles);Event(7);}void End(){Event(8);}
Stream stream;
void NativeVertex(float x,float y,float z){const float p[]={x,y,z};Event(9);Vec(p,3);}
Vertex attributes;
Vertex CaptureAttributes(bool color,bool uv){Require(color&&uv);Event(14);return attributes;}
}
void glColor4fv(const float* p){Event(16);Vec(p,4);}void glTexCoord4fv(const float* p){Event(17);Vec(p,4);}
bool topologyAllowed,residentAllowed,drawAllowed;int minimumIndex,maximumIndex;
namespace MeshDraw{
struct Mesh{struct Limit{int position;}minimum,maximum;unsigned residentArray=77;std::vector<unsigned> indices{0,1,2,2,1,0};int maxBone=5;};
std::unordered_map<const void*,Mesh> meshes;bool residentEnabled;
unsigned Index(const void* p){auto diff=static_cast<const BYTE*>(p)-reinterpret_cast<BYTE*>(meshData);Require(diff>=0&&diff%sizeof(ClothModelMesh)==0&&diff/sizeof(ClothModelMesh)<3);return unsigned(diff/sizeof(ClothModelMesh));}
bool Topology(Mesh& mesh,const Input& in){const unsigned i=Index(in.key);const auto& src=meshData[i];Require(in.faces==src.Triangles&&in.faceCount==src.TriangleCount&&in.positionCount==src.VertexCount&&in.normalCount==src.NormalCount&&in.uvCount==src.TexCoordCount&&in.uv==src.TexCoords&&in.rawVertices==src.Vertices&&in.rawNormals==src.Normals&&in.model==subject&&in.positions==reinterpret_cast<float*>(worldData)+i*45000);Require(!in.colors&&!in.chromeUV&&!in.bodyLight&&!in.attributes&&!in.mode&&!in.light&&!in.offset&&!in.alpha&&!in.u&&!in.v);mesh.minimum.position=minimumIndex;mesh.maximum.position=maximumIndex;Event(12,{i});return topologyAllowed;}
bool Resident(Mesh&,const Input& in){Event(13,{Index(in.key)});return residentAllowed;}
}
namespace Skinning{
bool DrawResident(const ResidentDraw& d){const auto i=MeshDraw::Index(d.key);Require(d.model==subject&&d.array==77&&d.indices==6&&d.maxBone==5&&d.world==reinterpret_cast<float*>(worldData)+i*45000&&d.positionCount==meshData[i].VertexCount&&d.normalCount==meshData[i].NormalCount&&!d.colors&&!d.lit&&!d.textured&&d.bodyLight&&d.bodyLight[0]==0&&d.bodyLight[1]==0&&d.bodyLight[2]==0&&!d.uOffset&&!d.vOffset&&d.shadow&&d.shadowOrigin==subject->BodyOrigin);Require(std::memcmp(d.color,BufferedDraw::attributes.color,16)==0&&std::memcmp(d.uv,BufferedDraw::attributes.uv,16)==0);Event(15,{i});Vec(d.shadowOrigin,3);return drawAllowed;}
}
'''

MAIN=r'''
void PutShort(BYTE* p,short v){std::memcpy(p,&v,2);}
std::array<float,3> Position(unsigned mesh,unsigned vertex,unsigned trial){std::array<float,3> p{float(int(vertex)-2)*.25f+float(mesh)*4,float(vertex+mesh)*.5f,float(vertex)*.75f};if(trial%17==0)p[2]=4000.f;if(trial%19==0)p[2]=3999.5f;if(trial%23==0)p[2]=4000.5f;return p;}
void GoldVec(std::vector<unsigned>& out,const float* p,unsigned n){for(unsigned i=0;i<n;++i)out.push_back(Bits(p[i]));}
void GoldEvent(std::vector<unsigned>& out,unsigned id,std::initializer_list<unsigned> args={}){out.push_back(id);out.insert(out.end(),args.begin(),args.end());}
void Hash(std::uint64_t& h,unsigned v){for(unsigned b=0;b<4;++b){h^=BYTE(v>>(b*8));h*=1099511628211ull;}}
std::uint64_t Run(BYTE* modelMem,BYTE* tables,BYTE* data){std::uint64_t digest=14695981039346656037ull;subject=reinterpret_cast<BMD*>(modelMem+16);meshData=reinterpret_cast<ClothModelMesh*>(tables+16);worldData=data+64;
 unsigned activeEffects=0,activeFallbacks=0,activeResidents=0;
 for(unsigned trial=0;trial<2048;++trial){
  std::memset(modelMem,0xa5,sizeof(BMD)+32);std::memset(tables,0x6d,2048);std::memset(data,0x37,540256);MeshDraw::meshes.clear();subject->Meshs=reinterpret_cast<BYTE*>(meshData);const short meshCounts[]={-1,0,1,2,3};subject->NumMeshs=meshCounts[trial%5];subject->BodyOrigin[0]=float(int(trial%7)-3)*.25f;subject->BodyOrigin[1]=-.5f;subject->BodyOrigin[2]=trial%11?0.f:.75f;
  int polyCounts[3],vertexCounts[3];short textureFields[3];std::array<std::vector<std::vector<unsigned>>,3> faces;
  for(unsigned m=0;m<3;++m){auto& mesh=meshData[m];textureFields[m]=short(m==1?0:m+2);mesh.TextureIndex=textureFields[m];polyCounts[m]=int((trial/5+m)%4)-1;mesh.TriangleCount=short(polyCounts[m]);mesh.VertexCount=short(trial%31?4:0);mesh.NormalCount=4;mesh.TexCoordCount=4;mesh.Triangles=tables+256+m*144;mesh.TexCoords=tables+800+m*32;mesh.Vertices=tables+1024+m*64;mesh.Normals=tables+1280+m*80;
   vertexCounts[m]=int((trial/7+m)%6)-1;for(int f=0;f<polyCounts[m];++f){auto* raw=mesh.Triangles+f*36;raw[0]=BYTE(vertexCounts[m]);std::vector<unsigned> indices;for(int k=0;k<4;++k){const unsigned v=(unsigned(k)+m+unsigned(f))%4;PutShort(raw+2+k*2,short(v));if(k<vertexCounts[m])indices.push_back(v);}faces[m].push_back(indices);}
   for(unsigned v=0;v<4;++v){const auto p=Position(m,v,trial);std::memcpy(worldData+(m*15000+v)*12,p.data(),12);}
  }
  const std::array<float,3> origin{subject->BodyOrigin[0],subject->BodyOrigin[1],subject->BodyOrigin[2]};
  for(unsigned c=0;c<4;++c){BufferedDraw::attributes.color[c]=float(c+1)*.125f;BufferedDraw::attributes.uv[c]=float(c)*.25f;}
  const std::vector<BYTE> beforeModel(modelMem,modelMem+sizeof(BMD)+32),beforeTables(tables,tables+2048),beforeData(data,data+540256);
  events.clear();randomCalls=trial%12;unsigned expectedRandom=randomCalls;std::vector<unsigned> expected;
  const int requestedMesh=int((trial/3)%5)-1;const int effects[]={260,262,261,0};const int effect=effects[(trial/13)%4];
  if(requestedMesh>=0&&requestedMesh<subject->NumMeshs&&polyCounts[requestedMesh]>0)for(const auto& face:faces[requestedMesh])for(unsigned v:face)if(effect==260||effect==262){
   const auto pos=Position(unsigned(requestedMesh),v,trial);const float angle[]={0,0,0};const float l=effect==260?1.f:.2f,light[]={l,l,l};const unsigned divisors[2]={effect==260?2u:12u,effect==260?10u:50u};const unsigned types[2]={unsigned(effect+1),unsigned(effect)};
   for(unsigned call=0;call<2;++call){const unsigned value=unsigned(Random(expectedRandom++));GoldEvent(expected,2,{value});if(value%divisors[call]==0){GoldEvent(expected,3,{types[call],unsigned(requestedMesh)*15000+v});GoldVec(expected,pos.data(),3);GoldVec(expected,angle,3);GoldVec(expected,light,3);++activeEffects;}}
  }
  BMD__RenderMeshEffect(subject,requestedMesh,effect);Require(events==expected&&randomCalls==expectedRandom);
  for(unsigned v:events)Hash(digest,v);events.clear();expected.clear();
  antialag=trial%29==0;MeshDraw::residentEnabled=trial%3!=0;topologyAllowed=trial%7!=0;residentAllowed=trial%11!=0;drawAllowed=trial%13!=0;minimumIndex=trial%17==0?-1:0;maximumIndex=trial%19==0?4:3;
  const int excludedTextures[]={-1,0,2,4};const int exclude=excludedTextures[(trial/17)%4],hidden=int((trial/23)%5)-1;
  GoldEvent(expected,1,{unsigned(antialag)});
  if(!antialag&&subject->NumMeshs!=0){GoldEvent(expected,4,{unsigned(BMDDevice::Pass::Shadow),0,0,0});GoldEvent(expected,5);
   for(int m=0;m<subject->NumMeshs;++m){if(m==hidden||polyCounts[m]<=0||textureFields[m]==exclude)continue;bool accepted=false;
    if(MeshDraw::residentEnabled&&meshData[m].VertexCount>0){GoldEvent(expected,12,{unsigned(m)});if(topologyAllowed&&minimumIndex>=0&&maximumIndex<meshData[m].VertexCount){GoldEvent(expected,13,{unsigned(m)});if(residentAllowed){GoldEvent(expected,14);GoldEvent(expected,15,{unsigned(m)});GoldVec(expected,origin.data(),3);if(drawAllowed){GoldEvent(expected,16);GoldVec(expected,BufferedDraw::attributes.color,4);GoldEvent(expected,17);GoldVec(expected,BufferedDraw::attributes.uv,4);accepted=true;++activeResidents;}}}}
    if(accepted)continue;++activeFallbacks;GoldEvent(expected,7);for(const auto& face:faces[m])for(unsigned vertex:face){auto p=Position(unsigned(m),vertex,trial);const float dx=p[0]-origin[0],dy=p[1]-origin[1],dz=p[2]-origin[2];float shadow[]={dx+(dx+2000.f)*dz/(dz-4000.f)+origin[0],dy+origin[1],origin[2]+5.f};GoldEvent(expected,9);GoldVec(expected,shadow,3);}GoldEvent(expected,8);
   }GoldEvent(expected,6);GoldEvent(expected,4,{unsigned(BMDDevice::Pass::ShadowExit),0,0,0});
  }
  BMD__RenderBodyShadow(subject,exclude,hidden);Require(events==expected);
  Require(std::memcmp(modelMem,beforeModel.data(),beforeModel.size())==0&&std::memcmp(tables,beforeTables.data(),beforeTables.size())==0&&std::memcmp(data,beforeData.data(),beforeData.size())==0);for(unsigned v:events)Hash(digest,v);
 }
 Require(activeEffects&&activeFallbacks&&activeResidents);
 // Guardas del adaptador que no pasan por la función de sombras del modelo.
 MeshDraw::residentEnabled=true;events.clear();const auto* mesh=reinterpret_cast<const char*>(meshData);const auto* world=reinterpret_cast<const float*>(worldData);
 Require(!MeshDraw::DrawShadow(nullptr,mesh,world,subject->BodyOrigin)&&!MeshDraw::DrawShadow(subject,nullptr,world,subject->BodyOrigin));Require(events.empty());
 meshData[0].TriangleCount=1;meshData[0].VertexCount=4;Require(!MeshDraw::DrawShadow(subject,mesh,nullptr,subject->BodyOrigin));auto* triangles=meshData[0].Triangles;meshData[0].Triangles=nullptr;Require(!MeshDraw::DrawShadow(subject,mesh,world,subject->BodyOrigin));meshData[0].Triangles=triangles;Require(events.empty());
 return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);auto expected=Run(low+256,low+4096,low+65536);
 if(sizeof(void*)==8){auto* high=static_cast<BYTE*>(Reserve(0x126000000ull));Require(high);for(unsigned mask=1;mask<8;++mask){std::memset(low+256,0x7b,sizeof(BMD)+32);std::memset(low+4096,0x7b,2048);std::memset(low+65536,0x7b,540256);Require(Run((mask&1?high:low)+256,(mask&2?high:low)+4096,(mask&4?high:low)+65536)==expected);if(mask&1)for(unsigned i=0;i<sizeof(BMD)+32;++i)Require(low[256+i]==0x7b);if(mask&2)for(unsigned i=0;i<2048;++i)Require(low[4096+i]==0x7b);if(mask&4)for(unsigned i=0;i<540256;++i)Require(low[65536+i]==0x7b);}}
 std::printf("ModelShadowEffects: 2048 ciclos efectos/sombra/adaptador por ruta %016llx\n",(unsigned long long)expected);
}
'''

def harness(revision=None):
    headers='\n'.join(re.sub(r'#include "[^\"]+"|#pragma once','',read_source(p)) for p in ['src/Render/Materials.h','src/Render/BufferedDraw.h','src/Render/BMDDevice.h','src/Render/MeshDraw.h','src/Render/Skinning.h'])
    effects=read_source('src/Render/BMD_LegacyEffects.cpp',revision)
    bodies='namespace MeshDraw{'+function(read_source('src/Render/Backend/GL/MeshDraw.cpp',revision),'DrawShadow')+'}\n'+''.join(function(effects,n)for n in ['BMD__BeginRender','BMD__RenderMeshEffect','BMD__RenderBodyShadow'])
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x200000')+layouts()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\n'+headers+SERVICES+bodies+MAIN

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
    if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-shadow-effects-')as t:
        current=build_and_run(Path(t)/'current',harness(),a);print(current,end='')
        if a.reference_ref:
            old=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
            if current!=old:raise RuntimeError('Diferencia frente a x86')
            print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
