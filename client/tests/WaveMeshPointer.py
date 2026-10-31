#!/usr/bin/env python3
"""Malla deformada real: equivalencia de eventos y buffers con base BMD nativa."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
BYTE* buffers;
#define DAT_0584621c (*reinterpret_cast<DWORD*>(buffers))
#define DAT_077e298c (*reinterpret_cast<DWORD*>(buffers+400000))
#define DAT_060db65c (*reinterpret_cast<char*>(buffers+550000))
#define DAT_06f433bc (*reinterpret_cast<char*>(buffers+1000000))
#define DAT_083a7cc8 (*reinterpret_cast<char*>(buffers+1450000))
float _DAT_00552544=.99f,_DAT_005528c4=.007f,_DAT_00552644=28.0f,WorldTime;
std::vector<std::uint32_t> events;unsigned vertices,begins,ends;
void Value(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);events.push_back(bits);}
namespace BMDDevice{enum class Pass{ColorAnimated,Blend,Surface,Metal};struct PassState{Pass pass;unsigned flags=0;int textureSlot=0;bool alpha=false;};void Select(const PassState& state){events.push_back(1);events.push_back(static_cast<unsigned>(state.pass));events.push_back(state.flags);events.push_back(state.textureSlot);events.push_back(state.alpha);}}
namespace BufferedDraw{
enum class Primitive{Triangles};void Begin(Primitive p){Require(p==Primitive::Triangles && !begins++);events.push_back(2);}void End(){Require(begins && !ends++);events.push_back(3);}
void Color3f(float r,float g,float b){events.push_back(4);Value(r);Value(g);Value(b);}void Color3fv(const float* p){Color3f(p[0],p[1],p[2]);}void Color4f(float r,float g,float b,float a){events.push_back(5);Value(r);Value(g);Value(b);Value(a);}void TexCoord2f(float u,float v){events.push_back(6);Value(u);Value(v);}void Vertex3fv(const float* p){Require(begins && !ends);events.push_back(7);for(unsigned i=0;i<3;++i){Require(std::isfinite(p[i]));Value(p[i]);}++vertices;}
}
'''
MAIN=r'''
void PutInt(BYTE* p,int value){std::memcpy(p,&value,4);}void PutShort(BYTE* p,short value){std::memcpy(p,&value,2);}void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}int Address(BYTE* p){return static_cast<int>(reinterpret_cast<std::uintptr_t>(p));}
void Hash(std::uint64_t& h,unsigned value){for(unsigned i=0;i<4;++i){h^=(value>>(i*8))&255;h*=1099511628211ull;}}
std::uint64_t Run(BYTE* model,BYTE* tables,BYTE* data){buffers=data;std::uint64_t hash=14695981039346656037ull;const int flagsList[]={0,1,2,0x40,3,0x42,0x401,0x402},blends[]={-2,-1,0,1,3};const float alphas[]={0,.5f,.99f,1};
 std::memset(data,0x5a,1500000);for(unsigned mesh=0;mesh<2;++mesh){for(unsigned vertex=0;vertex<4;++vertex)for(unsigned axis=0;axis<3;++axis){PutFloat(data+mesh*180000+vertex*12+axis*4,float(int(mesh*10+vertex*3+axis)-7)*.125f);PutFloat(data+1000000+mesh*180000+vertex*12+axis*4,float(int(vertex+axis)-3)*.0625f);}for(unsigned i=0;i<4;++i)PutFloat(data+400000+mesh*60000+i*4,float(i)*.25f);}
 std::vector<BYTE> original(data,data+1500000);unsigned queries=0;
 for(unsigned frame=0;frame<2048;++frame){const unsigned mesh=frame%2;const int polygons=(frame/2)%3,nv=2+(frame/6)%3;const short texture=frame%17==0?300:static_cast<short>(5+frame%2);const unsigned overrideTexture=frame%11==0?3:0xffffffffu;const int usedTexture=overrideTexture==0xffffffffu?texture:static_cast<int>(overrideTexture);const int flags=flagsList[(frame/3)%8],blend=blends[(frame/17)%5];const float alpha=alphas[(frame/31)%4],u=frame%3?.125f:0,v=frame%5?-.25f:0;WorldTime=float(int(frame)-1000)+.25f;
  std::memcpy(data,original.data(),1500000);std::memset(model,0xa5,188);std::memset(tables,0x6d,1024);BYTE* entry=tables+mesh*40;PutInt(model+40,Address(tables));PutInt(model+56,Address(tables+128));model[68]=static_cast<BYTE>((frame/7)%2);model[136]=static_cast<BYTE>(int((frame/13)%3)-1);for(unsigned i=0;i<3;++i)PutFloat(model+72+i*4,float(i+1)*.25f);
  PutShort(entry+2,static_cast<short>(mesh));PutShort(entry+6,4);PutShort(entry+10,static_cast<short>(polygons));PutInt(entry+24,Address(tables+256));PutInt(entry+28,Address(tables+384));PutInt(entry+36,frame%7?Address(tables+512):0);tables[514]=static_cast<BYTE>((frame/19)%2);PutShort(tables+128+mesh*2,texture);for(unsigned i=0;i<4;++i){PutFloat(tables+256+i*8,float(i)*.25f);PutFloat(tables+260+i*8,float(i)*.125f);}
  for(int poly=0;poly<polygons;++poly){BYTE* tri=tables+384+poly*36;tri[0]=static_cast<BYTE>(nv);for(int i=0;i<nv;++i){PutShort(tri+2+i*2,static_cast<short>((i+poly)%4));PutShort(tri+10+i*2,static_cast<short>((i+1)%4));PutShort(tri+18+i*2,static_cast<short>((i+2)%4));}}
  data[1450000+usedTexture*56]=frame%2?4:3;BYTE beforeModel[188],beforeTables[1024];std::memcpy(beforeModel,model,188);std::memcpy(beforeTables,tables,1024);std::vector<BYTE> before(data,data+1500000);events.clear();vertices=begins=ends=0;BMD__RenderMeshTranslate(model,static_cast<char>((frame/23)%4),int(frame),float(mesh)+.25f,flags,alpha,blend,u,v,.75f,overrideTexture);
  const bool hidden=polygons==0 || texture==300;const bool metalReject=!hidden && !(flags&1) && !(blend<=-2 || int(mesh)==blend) && !(flags&2) && (flags&0x40) && frame%2;const bool drawing=!hidden && !metalReject;Require(vertices==(drawing?unsigned(polygons*nv):0) && begins==unsigned(drawing) && ends==unsigned(drawing));Require(std::memcmp(model,beforeModel,188)==0 && std::memcmp(tables,beforeTables,1024)==0);
  const bool lightWrite=!hidden && static_cast<signed char>(model[136])!=int(mesh) && model[68];if(lightWrite){const unsigned first=550000+mesh*180000,last=first+48;Require(std::memcmp(data,before.data(),first)==0 && std::memcmp(data+last,before.data()+last,1500000-last)==0);}else Require(std::memcmp(data,before.data(),1500000)==0);for(auto value:events)Hash(hash,value);for(unsigned i=0;i<48;++i)Hash(hash,data[550000+mesh*180000+i]);++queries;
 }Require(queries==2048);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);BMD__RenderMeshTranslate(nullptr,0,0,0,0,1,0,0,0,1,0xffffffffu);Require(events.empty());const auto expected=Run(low+256,low+4096,low+65536);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){if(mask&1)std::memset(low+256,0x7b,188);if(mask&2)std::memset(low+65536,0x7b,1500000);Require(Run((mask&1)?high+256:low+256,low+4096,(mask&2)?high+65536:low+65536)==expected);if(mask&1)for(unsigned i=0;i<188;++i)Require(low[256+i]==0x7b);if(mask&2)for(unsigned i=0;i<1500000;++i)Require(low[65536+i]==0x7b);}std::puts("modelo/buffers altos; tablas32 bajas y aliases intactos");}std::printf("Wave mesh: 2048 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    body=function(read_source('src/Render/BMD_LegacyDraw.cpp',revision),'BMD__RenderMeshTranslate')
    # Mismos datos y oráculo; sólo la representación de tablas pasa a tipos nativos.
    main=MAIN.replace('188','sizeof(BMD)')
    start=main.index('BYTE* entry=tables+mesh*40;')
    end=main.index('for(unsigned i=0;i<4;++i){PutFloat(tables+256',start)
    main=main[:start]+r'''auto* bmd=reinterpret_cast<BMD*>(model);auto* entry=reinterpret_cast<ClothModelMesh*>(tables)+mesh;
  bmd->Meshs=tables;bmd->IndexTexture=reinterpret_cast<short*>(tables+128);bmd->LightEnable=static_cast<BYTE>((frame/7)%2);bmd->StreamMesh=char(int((frame/13)%3)-1);for(unsigned i=0;i<3;++i)bmd->BodyLight[i]=float(i+1)*.25f;
  entry->TextureIndex=short(mesh);entry->NormalCount=4;entry->TriangleCount=short(polygons);entry->TexCoords=tables+256;entry->Triangles=tables+384;entry->TextureFlags=frame%7?tables+512:nullptr;tables[514]=static_cast<BYTE>((frame/19)%2);bmd->IndexTexture[mesh]=texture;'''+main[end:]
    main=main.replace('static_cast<signed char>(model[136])','static_cast<signed char>(bmd->StreamMesh)').replace('model[68]','bmd->LightEnable')
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x200000')+layouts()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\n'+SERVICES+body+main

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-wave-mesh-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
