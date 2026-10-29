#!/usr/bin/env python3
"""Variantes reales de CreateObject: bytes, retornos y orden del registro."""
import argparse
from pathlib import Path
import tempfile
from SceneObjectPointer import build_and_run,function,read_source
from ObjectLayoutContract import object_types

def variants(source):
    return source.split('// login/char-select scene type overrides',1)[1].split('\n',1)[1].split('\n}\n// IDA: DeleteBug',1)[0]

def harness(args):
    source=read_source('src/Scene/Scene_EntityLifecycle.cpp',args.current_ref)
    reference=variants(read_source('src/Scene/Scene_EntityLifecycle.cpp',args.reference_ref)) if args.reference_ref else read_source('tests/fixtures/object-scene-variants-x86.inc')
    helper=function(source,'SceneObject_StoreFloatBits') if 'void SceneObject_StoreFloatBits(' in source else ''
    prefix=r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;using UINT=std::uint32_t;
using vec3_t=float[3];using vec34_t=float[3][4];
'''+object_types()+r'''
constexpr unsigned Offset(unsigned i){return i+(sizeof(void*)==8?(i>=280?8:i>=256?4:0):0);}
static_assert(sizeof(OBJECT)==(sizeof(void*)==4?444:452));
int SceneFlag;
struct Map{int current;int GetCurrentMap()const{return current;}}gMapManager;
std::vector<std::vector<BYTE>> calls;bool nativePhase;
void Entity_InitRenderState(void* p){auto* bytes=static_cast<BYTE*>(p);calls.emplace_back(bytes,bytes+(nativePhase?sizeof(OBJECT):444));}
void Require(bool ok){if(!ok){std::fputs("Variante OBJECT o registro incorrecto\n",stderr);std::exit(1);}}
void Translate(const BYTE* legacy,BYTE* native){
 for(unsigned i=0;i<444;++i)if(!(i>=252&&i<256)&&!(i>=276&&i<280))native[Offset(i)]=legacy[i];
}
'''+helper
    functions='\nvoid* Current(UINT* puVar3,int param_1){OBJECT& object=*reinterpret_cast<OBJECT*>(puVar3);'+variants(source)+'}\nvoid* Reference(UINT* puVar3,int param_1){'+reference+'}\n'
    main=r'''
int main(){
 alignas(OBJECT) static BYTE native[sizeof(OBJECT)+32];alignas(UINT) BYTE legacy[444];
 if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(native)>0xffffffffull);
 const float rotations[]={-720.75f,-360.1f,-1.9f,-0.f,0.f,89.9f,90.f,180.f,270.f,359.9f,360.f,720.5f};
 std::vector<int> types;for(int type=0;type<256;++type)types.push_back(type);for(int type:{-1,1000,(-2147483647-1),2147483647})types.push_back(type);
 unsigned cases=0;std::uint64_t hash=14695981039346656037ull;
 auto Hash=[&](const BYTE* p,unsigned n){while(n--){hash^=*p++;hash*=1099511628211ull;}};
 for(int scene:{0,2,4,5})for(int map=-1;map<=8;++map)for(int type:types)for(float rotation:rotations){
  BYTE fill=BYTE(cases);std::memset(native,fill,sizeof(native));std::memset(legacy,fill,sizeof(legacy));
  float position[]={float(type%127),-50.25f,19.f},angle[]={-0.f,45.25f,rotation};
  const std::uint32_t payload=cases%2?0x7fc12345u:0x7f800001u;
  if(cases%3==0)std::memcpy(angle,&payload,4);
  if(cases%5==0)std::memcpy(position+2,&payload,4);
  std::memcpy(legacy+16,position,12);std::memcpy(legacy+28,angle,12);
  auto* object=reinterpret_cast<OBJECT*>(native+16);Translate(legacy,native+16);
  object->Owner=reinterpret_cast<OBJECT*>(uintptr_t(sizeof(void*)==8?0x123456780ull:0x23456780ull));
  object->BoneMatrices=reinterpret_cast<vec34_t*>(uintptr_t(sizeof(void*)==8?0x154321080ull:0x54321080ull));
  BYTE baseline[sizeof(native)],expected[sizeof(native)];std::memcpy(baseline,native,sizeof(native));std::memcpy(expected,native,sizeof(native));
  SceneFlag=scene;gMapManager.current=map;nativePhase=false;calls.clear();
  Require(Reference(reinterpret_cast<UINT*>(legacy),type)==legacy);
  const auto referenceCalls=calls;Translate(legacy,expected+16);
  nativePhase=true;calls.clear();Require(Current(reinterpret_cast<UINT*>(object),type)==object);
  if(std::memcmp(expected,native,sizeof(native))||calls.size()!=referenceCalls.size()){
   std::fprintf(stderr,"CreateObject variante: scene=%d map=%d type=%d rot=%g ptr=%zu\n",scene,map,type,rotation,sizeof(void*));return 1;
  }
  for(unsigned i=0;i<calls.size();++i){BYTE event[sizeof(OBJECT)];std::memcpy(event,baseline+16,sizeof(event));Translate(referenceCalls[i].data(),event);Require(calls[i].size()==sizeof(OBJECT)&&std::memcmp(event,calls[i].data(),sizeof(event))==0);Hash(referenceCalls[i].data(),444);}
  Hash(legacy,444);const unsigned count=unsigned(calls.size());Hash(reinterpret_cast<const BYTE*>(&count),sizeof(count));++cases;
 }
 std::printf("ObjectSceneVariants: %u casos bytes/retorno/registro %016llx\n",cases,(unsigned long long)hash);
}
'''
    return prefix+functions+main

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');p.add_argument('--current-ref');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-object-variants-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args),args),end='')
if __name__=='__main__':main()
