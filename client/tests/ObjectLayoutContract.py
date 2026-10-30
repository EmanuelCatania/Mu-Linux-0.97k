#!/usr/bin/env python3
"""Contrato declarado de OBJECT y núcleo real de CreateObject, antes del cambio de pools.

El tamaño x64 mide sólo los punteros ya declarados; no define el ABI final de
la cola opaca ni habilita los pools legacy en x64.
"""
import argparse
from pathlib import Path
import re
import tempfile
from SceneObjectPointer import build_and_run,read_source

def record(source,name):
    return re.search(r'struct '+name+r'\s*\{.*?\n\};',source,re.S).group()

def object_types():
    header=read_source('src/Entity/EntityTypes.h')
    return '#pragma pack(push,1)\n'+''.join(record(header,name) for name in ['OBJECT','PART_t','CHARACTER'])+'\n#pragma pack(pop)\n'

def core(source):
    return source.split('// common initialisation',1)[1].split('// login/char-select scene type overrides',1)[0]

def harness(args):
    header=read_source('src/Entity/EntityTypes.h')
    fields=re.findall(r'MU_ASSERT_FIELD\(OBJECT,\s*(\w+),\s*(0x[0-9a-fA-F]+)\)',header)
    assertions='\n'.join(f'static_assert(offsetof(OBJECT,{name})==NativeOffset({offset}),"{name}");' for name,offset in fields)
    prefix=r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;using UINT=std::uint32_t;
using vec3_t=float[3];using vec34_t=float[3][4];
'''+object_types()+read_source('src/Item/ItemTypes.h')+record(read_source('src/structs.h'),'ITEM_t')+r'''
constexpr std::size_t NativeOffset(std::size_t offset){return offset+(sizeof(void*)==8?(offset>=0x1b8?16:offset>=0x188?12:offset>=0x118?8:offset>=0x100?4:0):0);}
static_assert(sizeof(OBJECT)==(sizeof(void*)==4?0x1bc:0x1d0));
static_assert(sizeof(CHARACTER)==(sizeof(void*)==4?0x394:0x3d4));
static_assert(sizeof(ITEM_t)==(sizeof(void*)==4?0x204:0x218));
static_assert(offsetof(OBJECT,SceneCell)==NativeOffset(0x1b0));
static_assert(offsetof(OBJECT,ScenePrevious)==NativeOffset(0x1b4));
static_assert(offsetof(OBJECT,SceneNext)==NativeOffset(0x1b8));
static_assert(offsetof(ITEM_t,Object)==0x48 && sizeof(PART_t)==20+sizeof(void*));
static_assert(offsetof(PART_t,Cloth)==20);
static_assert(alignof(OBJECT)==1 && std::is_standard_layout<OBJECT>::value && std::is_trivially_copyable<OBJECT>::value);
static_assert(sizeof(float)==4 && sizeof(int)==4 && sizeof(short)==2 && sizeof(vec34_t)==48);
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}
void Require(bool ok){if(!ok){std::fputs("Contrato OBJECT o escritura incorrecta\n",stderr);std::exit(1);}}
'''+assertions
    parameters='int param_1,float* param_2,float* param_3,float param_4,BYTE bVar4'
    current=core(read_source('src/Scene/Scene_EntityLifecycle.cpp',args.current_ref))
    reference=core(read_source('src/Scene/Scene_EntityLifecycle.cpp',args.reference_ref)) if args.reference_ref else read_source('tests/fixtures/object-create-core-x86.inc')
    functions='\nvoid Current(UINT* puVar3,'+parameters+'){'+current+'}\nvoid Reference(UINT* puVar3,'+parameters+'){'+reference+'}\n'
    main=r'''
int main(){
    std::printf("OBJECT declarado: ptr=%zu size=%zu Owner=%zu BoneMatrices=%zu Alpha=%zu CHARACTER=%zu ITEM_t=%zu\n",sizeof(void*),sizeof(OBJECT),offsetof(OBJECT,Owner),offsetof(OBJECT,BoneMatrices),offsetof(OBJECT,Alpha),sizeof(CHARACTER),sizeof(ITEM_t));
    if(CONTRACTS_ONLY)return 0;
    alignas(OBJECT) static BYTE native[sizeof(OBJECT)+32];alignas(UINT) BYTE legacy[444];
    if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(native)>0xffffffffull);
    const std::uint32_t values[]={0,0x80000000u,0x3f800000u,0xbf000000u,0x7f800000u,0xff800000u,0x7fc12345u,0x7f800001u,0x00000001u,0x55555555u};
    std::uint64_t hash=14695981039346656037ull;
    for(unsigned trial=0;trial<32768;++trial){
        const BYTE fill=BYTE(trial);
        std::memset(native,fill,sizeof(native));std::memset(legacy,fill,sizeof(legacy));
        auto* object=reinterpret_cast<OBJECT*>(native+16);
        object->Owner=reinterpret_cast<OBJECT*>(uintptr_t(sizeof(void*)==8?0x123456780ull:0x23456780ull));
        object->BoneMatrices=reinterpret_cast<vec34_t*>(uintptr_t(sizeof(void*)==8?0x154321080ull:0x54321080ull));
        object->Cloth=reinterpret_cast<void*>(uintptr_t(sizeof(void*)==8?0x165432100ull:0x65432100ull));
        object->ScenePrevious=reinterpret_cast<OBJECT*>(uintptr_t(sizeof(void*)==8?0x176543210ull:0x76543210ull));
        object->SceneNext=reinterpret_cast<OBJECT*>(uintptr_t(sizeof(void*)==8?0x187654320ull:0x87654320ull));
        BYTE expected[sizeof(native)];std::memcpy(expected,native,sizeof(native));
        float position[3],angle[3],scale;
        for(unsigned axis=0;axis<3;++axis){const auto p=values[(trial+axis)%10],a=values[(trial/7+axis)%10];std::memcpy(position+axis,&p,4);std::memcpy(angle+axis,&a,4);}
        const auto scaleBits=values[(trial/11)%10];std::memcpy(&scale,&scaleBits,4);
        const int type=int(trial*65537u);const BYTE cell=BYTE(trial);
        Reference(reinterpret_cast<UINT*>(legacy),type,position,angle,scale,cell);
        for(unsigned i=0;i<444;++i)if(legacy[i]!=fill){Require(!(i>=252&&i<256)&&!(i>=276&&i<280));expected[16+NativeOffset(i)]=legacy[i];}
        Current(reinterpret_cast<UINT*>(object),type,position,angle,scale,cell);
        if(std::memcmp(expected,native,sizeof(native))){std::fprintf(stderr,"CreateObject comun: trial=%u ptr=%zu\n",trial,sizeof(void*));return 1;}
        for(unsigned i=0;i<444;++i){hash^=legacy[i];hash*=1099511628211ull;}
    }
    std::printf("ObjectLayoutContract: 32768 inicializaciones, bytes/punteros/canarios %016llx\n",(unsigned long long)hash);
}
'''
    return prefix+functions+main.replace('CONTRACTS_ONLY','true' if args.contracts_only else 'false')

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');p.add_argument('--current-ref');p.add_argument('--contracts-only',action='store_true');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-object-contract-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args),args),end='')
if __name__=='__main__':main()
