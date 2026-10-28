#!/usr/bin/env python3
"""Preparación del modelo y tres lecturas reales de MoveCharacterVisual.

Extrae el prefijo hasta SafeZone y las expresiones escalares de huesos.
AsCharacter/ObjectBounds controlados; no ejecuta el tick visual completo ni
certifica CHARACTER/BMD nativos, matrices internas o render real.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
// Proyección semántica para el servicio controlado, sin representar CHARACTER.
struct OBJECT{short Type;float Position[3],Scale;BYTE CurrentAction;float BoundingBoxMin[3],BoundingBoxMax[3],OBB[12];};
struct Character{OBJECT Object;}entity;
std::uintptr_t DAT_05828d58;unsigned queries,boundsCalls;
Character* AsCharacter(int key){Require(key==7);++queries;return &entity;}
void ObjectBounds_Build(const float* a,const float* b,const float* p,float* out){Require(a==entity.Object.BoundingBoxMin && b==entity.Object.BoundingBoxMax && p==entity.Object.Position && out==entity.Object.OBB && boundsCalls++==0);}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,unsigned at,T value){std::memcpy(p+at,&value,sizeof(value));}
constexpr unsigned bytes=1024*188+64;
std::uint64_t Run(BYTE* storage){
 for(unsigned i=0;i<bytes;++i)storage[i]=static_cast<BYTE>(i*13+5);
 const unsigned slots[]={0,1,31,1023};const short actions[]={-32768,-1,0,1,32767};
 const short bones[]={-32768,-1,0,1,255,32767};const int roots[]={static_cast<int>(0x80000000u),-1,0,1,15,0x7fffffff};
 const DWORD bits[]={0,0x80000000u,0x7f800000u,0xff800000u,0x7fc12345u,1,0x3f800000u,0xffffffffu};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned profile=0;profile<256;++profile)for(short action:actions)for(unsigned slot:slots){
  DAT_05828d58=reinterpret_cast<std::uintptr_t>(storage+16+(profile%2)*4);BYTE* model=reinterpret_cast<BYTE*>(DAT_05828d58)+slot*188;
  for(unsigned i=0;i<188;++i)model[i]=static_cast<BYTE>(profile+i*37);
  Put(model,38,action);Put(model,34,bones[profile%6]);Put(model,84,roots[profile%6]);
  std::memset(&entity,0x55,sizeof(entity));entity.Object.Type=static_cast<short>(slot);entity.Object.CurrentAction=static_cast<BYTE>(profile);
  for(unsigned i=0;i<3;++i)std::memcpy(&entity.Object.Position[i],&bits[(profile+i)%8],4);std::memcpy(&entity.Object.Scale,&bits[(profile+3)%8],4);
  BYTE entityBefore[sizeof(entity)];std::memcpy(entityBefore,&entity,sizeof(entity));std::vector<BYTE> before(storage,storage+bytes),expected=before;
  if(action!=0){const unsigned at=unsigned(model-storage);for(unsigned i=0;i<3;++i)std::memcpy(expected.data()+at+108+i*4,&bits[(profile+i)%8],4);std::memcpy(expected.data()+at+104,&bits[(profile+3)%8],4);expected[at+160]=static_cast<BYTE>(profile);}
  // El objetivo null sale antes de consultar tipo o modelo.
  queries=boundsCalls=0;ModelPrefix(0);Require(queries==0 && boundsCalls==0 && std::memcmp(storage,before.data(),bytes)==0);
  queries=boundsCalls=0;ModelPrefix(7);
  Require(queries==(action==0?2u:6u) && boundsCalls==(action==0?1u:0u));
  Require(ReadBone0(model)==bones[profile%6] && ReadBone1(model)==bones[profile%6] && ReadRoot(model)==roots[profile%6]);
  Require(std::memcmp(storage,expected.data(),bytes)==0 && std::memcmp(&entity,entityBefore,sizeof(entity))==0);
  coverage|=action==0?1u:2u;
  for(unsigned i=0;i<188;++i){hash^=model[i];hash*=1099511628211ull;}hash^=queries+16*boundsCalls;hash*=1099511628211ull;++cases;
 }
 Require(cases==5120 && coverage==3);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,bytes);Require(Run(high)==expected);for(unsigned i=0;i<bytes;++i)Require(low[i]==0x7b);std::puts("modelos altos; preparacion, lecturas escalares, bits y alias intactos");}
 std::printf("Character visual model: 5120 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Net/SecondPassword.cpp',revision)
    start=source.index('void __cdecl MoveCharacterVisual(int entity_ptr)')
    end=source.index('// SafeZone update por frame',start)
    prefix=source[start:end].replace('MoveCharacterVisual(','ModelPrefix(',1)+'\n}\n'
    tail=source[end:];base=r'(?:\(int\)model|static_cast<BYTE\*>\(model\))'
    bones=re.findall(r'\*\(short \*\)\('+base+r' \+ 34\)',tail)
    roots=re.findall(r'\*\(int \*\)\('+base+r' \+ 84\)',tail)
    if len(bones)!=2 or len(roots)!=1:raise ValueError('Lecturas de modelo inesperadas')
    reads='\n'.join(f'short ReadBone{i}(void* model){{return {expression};}}' for i,expression in enumerate(bones))+'\nint ReadRoot(void* model){return '+roots[0]+';}\n'
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x40000')+SERVICES+prefix+reads+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-character-model-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
