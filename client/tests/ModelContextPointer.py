#!/usr/bin/env python3
"""Contexto BMD real: campos, restauración completa y transformación x86/nativa."""
import argparse
import re
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, function, read_source
from ObjectLayoutContract import object_types, record


def harness(contracts_only=False):
    header=read_source("src/Model/ModelTypes.h")
    fields=re.findall(r'MU_ASSERT_FIELD\(BMD,\s*(\w+),\s*(0x[\da-fA-F]+)\)',header)
    checks="\n".join(f'static_assert(offsetof(BMD,{name})==M({offset}));' for name,offset in fields)
    prefix=r'''
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#ifndef _WIN32
#define __fastcall
#define __cdecl
#endif
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;using UINT=std::uint32_t;
using vec3_t=float[3];using vec34_t=float[3][4];
''' + object_types() + "\n#pragma pack(push,1)\n"+record(header,"BMD")+"\n#pragma pack(pop)\n"+r'''
constexpr unsigned M(unsigned offset){unsigned n=offset;for(unsigned p:{40u,44u,48u,52u,56u,64u})if(p<offset)n+=sizeof(void*)-4;return n;}
static_assert(sizeof(BMD)==(sizeof(void*)==4?188:212));
static_assert(alignof(BMD)==1&&std::is_standard_layout_v<BMD>&&std::is_trivially_copyable_v<BMD>);
''' + checks
    if contracts_only:
        return prefix+'\nint main(){std::printf("BMD contratos ptr=%zu size=%zu Scale=%zu Origin=%zu Action=%zu OK\\n",sizeof(void*),sizeof(BMD),offsetof(BMD,BodyScale),offsetof(BMD,BodyOrigin),offsetof(BMD,CurrentAction));}\n'
    context=re.search(r'class EquipmentModelContext\s*\{.*?\n\};',read_source("src/Render/EquipmentVisuals.h"),re.S).group()
    equipment=read_source("src/Render/EquipmentVisuals.cpp")
    ctor=re.search(r'EquipmentModelContext::EquipmentModelContext\(.*?\n\}',equipment,re.S).group()
    dtor=re.search(r'EquipmentModelContext::~EquipmentModelContext\(\)[^\n]*',equipment).group()
    pose=read_source("src/Entity/ObjectPose.cpp")
    prepare=function(pose,"PrepareContext")
    simulation=function(pose,"ObjectPose_PrepareSimulation")
    math=read_source("src/Math/Math_3D.cpp")
    constants=""
    for name in ["_DAT_0055290c","_DAT_00552908","_DAT_005524f8"]:
        constants+=re.search(r'^float\s+'+name+r'\s*=\s*[^;]+;',read_source("src/globals.cpp"),re.M).group()+"\n"
    services=r'''
void Require(bool x){if(!x){std::fputs("Contexto BMD/canario/selección incorrectos\n",stderr);std::abort();}}
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}BMD* AsModel(void* p){return static_cast<BMD*>(p);}BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
namespace Platform{struct Timer{DWORD Ticks(){return 1;}}timer;Timer& GetTimer(){return timer;}}
std::uintptr_t DAT_05828d58;int DAT_07e11d30;
float globalBones[12];
float rotationParent[12];
#define DAT_06989c9c rotationParent[0]
#define DAT_06970a9c (*reinterpret_cast<DWORD*>(globalBones))
unsigned bonesCalls,boundsCalls,editorCalls;OBJECT* wantedOwner;BMD* wantedModel;float* wantedBones;char wantedTranslate;
void PrepareContext(OBJECT*,void*);
void ObjectPose_PrepareBones(OBJECT* o,void* m,float* b,char t){Require(o==wantedOwner&&m==wantedModel&&b==wantedBones&&t==wantedTranslate);++bonesCalls;PrepareContext(o,m);}
void ObjectPose_PrepareBounds(OBJECT* o,char t){Require(o==wantedOwner&&t==wantedTranslate);++boundsCalls;}
void Calc_RenderObject(OBJECT* o,char t,int flag){Require(o==wantedOwner&&t==wantedTranslate&&flag==0);++editorCalls;}
'''
    return prefix+context+services+constants+ctor+dtor+prepare+function(math,"Vector_Rotate")+function(math,"Vector_Transform")+function(math,"BMD_TransformPosition")+function(read_source("src/Math/Math_LegacyTransforms.cpp"),"BMD__TransformPosition")+function(read_source("src/Render/BMD_SetupRender.cpp"),"BMD__RotationPosition")+simulation+"\n#undef DAT_06989c9c\n#define DAT_06989c9c referenceParent[0]\nnamespace Legacy {\nfloat referenceParent[12];\n"+read_source("tests/fixtures/model-context-x86.inc")+"\n}\n"+MAIN


MAIN=r'''
struct ModelBank{BYTE before[16];BMD model;BYTE after[16];};
static ModelBank bank;static OBJECT owner;static BMD models[512];static vec34_t privateBones[1];
bool PointerByte(unsigned i){for(unsigned p:{40u,44u,48u,52u,56u,64u})if(i>=p&&i<p+4)return true;return false;}
void Translate(const BYTE* legacy,BYTE* native){for(unsigned i=0;i<188;++i)if(!PointerByte(i))native[M(i)]=legacy[i];}
int main(){
 if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(&bank)>0xffffffffull&&reinterpret_cast<uintptr_t>(&owner)>0xffffffffull);
 std::uint64_t hash=14695981039346656037ull;
 auto Hash=[&](const void* p,std::size_t n){auto* b=static_cast<const BYTE*>(p);while(n--){hash^=*b++;hash*=1099511628211ull;}};
 const std::uint32_t bits[]={0,0x80000000u,0x3f800000u,0xbf800000u,0x7f800000u,0xff800000u,0x7fc12345u,0x7f800001u,1u,0x55555555u};
 unsigned calls=0;
 for(unsigned trial=0;trial<32768;++trial){
  std::memset(&bank,BYTE(trial),sizeof(bank));std::memset(&owner,0xa5,sizeof(owner));
  BYTE legacy[188];for(auto& b:legacy)b=BYTE(trial);Translate(legacy,reinterpret_cast<BYTE*>(&bank.model));
  bank.model.Meshs=reinterpret_cast<BYTE*>(uintptr_t(sizeof(void*)==8?0x123456780ull:0x23456780u));
  bank.model.Bones=bank.model.Actions=bank.model.Textures=bank.model.LightMaps=bank.model.Meshs;
  bank.model.IndexTexture=reinterpret_cast<short*>(bank.model.Meshs);
  for(unsigned axis=0;axis<3;++axis){std::memcpy(owner.Position+axis,bits+(trial+axis)%10,4);}
  std::memcpy(&owner.Scale,bits+(trial/7)%10,4);owner.CurrentAction=BYTE(trial);
  const short kinds[]={0,330,331,390,-1,32767};owner.Type=kinds[trial%6];
  auto expected=bank;Legacy::PrepareContext(&owner,legacy);Translate(legacy,reinterpret_cast<BYTE*>(&expected.model));
  PrepareContext(&owner,&bank.model);Require(std::memcmp(&expected,&bank,sizeof(bank))==0);
  Hash(legacy,sizeof(legacy));
  // Dos ámbitos del mismo modelo deben restaurar también los últimos bytes.
  const auto original=bank;
  {EquipmentModelContext outer(&bank.model);std::memset(&bank.model,0x6d,sizeof(BMD));const auto middle=bank;
   {EquipmentModelContext inner(&bank.model);std::memset(&bank.model,0x9e,sizeof(BMD));}
   Require(std::memcmp(&bank,&middle,sizeof(bank))==0);
  }
  Require(std::memcmp(&bank,&original,sizeof(bank))==0);
  // Transformaciones con origen/escala finitos; iguales bytes y aliases de salida.
  float scale=float(int(trial%9)-4)*.25f,origin[3]={-3.f,float(trial%11),.125f};
  bank.model.BodyScale=scale;std::memcpy(bank.model.BodyOrigin,origin,12);std::memcpy(legacy+104,&scale,4);std::memcpy(legacy+108,origin,12);
  for(char flag:{char(0),char(1),char(-1)}){
   float current[24],reference[24];for(unsigned i=0;i<24;++i)current[i]=float(int((trial+i*7)%41)-20)*.125f;std::memcpy(reference,current,sizeof(current));
   unsigned out=trial%2?16:12;
   BMD_TransformPosition(flag?&bank.model:nullptr,current,current+12,current+out,flag);
   Legacy::BMD_TransformPosition(flag?legacy:nullptr,reference,reference+12,reference+out,flag);
   Require(std::memcmp(current,reference,sizeof(current))==0);Hash(reference,sizeof(reference));++calls;
   for(unsigned i=0;i<24;++i)current[i]=float(int((trial+i*7)%41)-20)*.125f;std::memcpy(reference,current,sizeof(current));
   const unsigned overlap[]={16,12,13,0};out=overlap[trial%4];
   BMD__TransformPosition(flag?&bank.model:nullptr,reinterpret_cast<float(*)[4]>(current),current+12,current+out,flag!=0);
   Legacy::BMD__TransformPosition(flag?legacy:nullptr,reinterpret_cast<float(*)[4]>(reference),reference+12,reference+out,flag!=0);
   Require(std::memcmp(current,reference,sizeof(current))==0);++calls;

  }
 }

 // Rotación del equipo: conservar orden de stores, aliases y matriz padre.
 for(unsigned trial=0;trial<32768;++trial){
  float current[24],reference[24];for(unsigned i=0;i<24;++i)current[i]=float(int((trial+i*7)%41)-20)*.125f;std::memcpy(reference,current,sizeof(current));
  float scale=float(int(trial%9)-4)*.25f;bank.model.BodyScale=scale;BYTE legacy[188]{};std::memcpy(legacy+104,&scale,4);
  const unsigned overlaps[]={16,12,13,0};const unsigned out=overlaps[trial%4];
  for(unsigned i=0;i<12;++i)rotationParent[i]=Legacy::referenceParent[i]=float(i+77);
  const auto before=bank;
  BMD__RotationPosition(trial%3?&bank.model:nullptr,current,current+12,current+out);
  Legacy::BMD__RotationPosition(trial%3?legacy:nullptr,reference,reference+12,reference+out);
  Require(std::memcmp(current,reference,sizeof(current))==0);
  Require(std::memcmp(rotationParent,Legacy::referenceParent,sizeof(rotationParent))==0);
  Require(std::memcmp(&bank,&before,sizeof(bank))==0);
 }
 std::puts("RotationPosition: 32768 casos, aliases y matriz padre equivalentes");
 for(unsigned trial=0;trial<1024;++trial){
  std::memset(&owner,0,sizeof(owner));owner.Type=short(trial%512);owner.EnableBoneMatrix=BYTE(trial%3);owner.BoneMatrices=trial%5?privateBones:nullptr;
  const float alpha[]={0,.0001f,.5f,1.f};owner.Alpha=alpha[trial%4];DAT_07e11d30=int(trial%3);wantedTranslate=char(trial%2);
  DAT_05828d58=reinterpret_cast<uintptr_t>(models);wantedOwner=&owner;wantedModel=models+owner.Type;wantedBones=owner.EnableBoneMatrix?reinterpret_cast<float*>(owner.BoneMatrices):globalBones;
  wantedModel->BodyHeight=99;const bool skip=owner.Alpha<_DAT_005524f8;const bool editor=DAT_07e11d30==2;
  bonesCalls=boundsCalls=editorCalls=0;ObjectPose_PrepareSimulation(&owner,wantedTranslate);
  Require(bonesCalls==unsigned(!skip&&!editor)&&boundsCalls==bonesCalls&&editorCalls==unsigned(!skip&&editor));
  Require(wantedModel->BodyHeight==(!skip&&!editor?0:99));
 }
 std::printf("ModelContext: 32768 casos bits/rollback, %u transforms, 1024 selecciones; ptr=%zu BMD=%zu guard=%zu hash=%016llx\n",calls,sizeof(void*),sizeof(BMD),sizeof(EquipmentModelContext),(unsigned long long)hash);
}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("--architecture",choices=("Win32","x64"),default="x64");parser.add_argument("--generator");parser.add_argument("--contracts-only",action="store_true");args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-model-context-") as temp:print(build_and_run(Path(temp)/"current",harness(args.contracts_only),args),end="")


if __name__=="__main__":main()
