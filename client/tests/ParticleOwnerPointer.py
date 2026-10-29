#!/usr/bin/env python3
"""Case1176/sub2 real: pool nativo y owner legado32 independiente de su base.

Extrae sólo el case;no ejecuta MoveParticles ni su decremento/movimiento previo.
No certifica owners altos,layout nativo del pool,renderer o tick completo.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source

SERVICES=r'''
static_assert(sizeof(int)==4);
char* DAT_07abf5f0;
MACROS
CONSTANTS
void Segment(int iVar9){switch(2){
CASE
}}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,unsigned at,T value){std::memcpy(p+at,&value,sizeof(value));}
constexpr unsigned poolBytes=32*112+32;
std::uint64_t Run(BYTE* pool,BYTE* owner,BYTE* alias){
 DAT_07abf5f0=reinterpret_cast<char*>(pool+16);
 const int lives[]={static_cast<int>(0x80000000u),-2,-1,0,1,4,5,6,7,0x7fffffff};const BYTE flags[]={0,1,2,127,128,255};const unsigned slots[]={0,1,3,31};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned profile=0;profile<64;++profile)for(int life:lives)for(unsigned slot:slots)for(BYTE active:flags){
  for(unsigned i=0;i<poolBytes;++i)pool[i]=static_cast<BYTE>(profile+i*37);
  for(unsigned i=0;i<128;++i){owner[i]=static_cast<BYTE>(profile+i*13);alias[i]=static_cast<BYTE>(profile+i*7);}
  owner[16]=active;alias[16]=active==0?1:0;
  BYTE* particle=pool+16+slot*112;const bool nullOwner=life<=5 && (profile&1);
  const BYTE liveValues[]={1,127,128,255};particle[0]=liveValues[profile%4];Put(particle,4,1176);Put(particle,8,2);
  const auto ownerAddress=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(owner+16));Put(particle,60,nullOwner?std::uint32_t(0):ownerAddress);Put(particle,56,life);
  const float scale=float(int(profile)-32)*0.125f;Put(particle,12,scale);
  const float x=float(profile)*0.25f,y=-float(profile)*0.5f,z=float(int(profile)-20)*0.125f;
  const float vx=float(int(profile)-17)*0.125f,vy=float(int(profile)-9)*0.25f,vz=-float(profile)*0.125f;
  Put(particle,100,x);Put(particle,104,y);Put(particle,108,z);Put(particle,76,vx);Put(particle,80,vy);Put(particle,84,vz);
  std::vector<BYTE> expected(pool,pool+poolBytes);BYTE beforeOwner[128],beforeAlias[128];std::memcpy(beforeOwner,owner,128);std::memcpy(beforeAlias,alias,128);
  BYTE* out=expected.data()+16+slot*112;const int wantedLife=life>5 && active==0?5:life;
  if(life<1)out[0]=0;Put(out,56,wantedLife);
  const float light=float(wantedLife)*0.2f;Put(out,40,light);Put(out,44,light);Put(out,48,light);Put(out,12,float(scale+0.05f));
  Put(out,16,float(x+vx));Put(out,20,float(y+vy));Put(out,24,float(z+vz));Put(out,100,float(x+vx));Put(out,104,float(y+vy));Put(out,108,float(z+vz));
  Segment(int(slot*112));
  Require(std::memcmp(pool,expected.data(),poolBytes)==0 && std::memcmp(owner,beforeOwner,128)==0 && std::memcmp(alias,beforeAlias,128)==0);
  coverage|=life<1?1u:2u;coverage|=life>5?(active==0?4u:8u):16u;if(nullOwner)coverage|=32;
  // Sólo se normalizan los bytes de dirección;el oráculo completo los conserva.
  for(unsigned i=0;i<112;++i)if(i<60||i>=64){hash^=particle[i];hash*=1099511628211ull;}++cases;
 }
 Require(cases==15360 && coverage==63);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* owner=static_cast<BYTE*>(Reserve(0x28000000u));auto* alias=static_cast<BYTE*>(Reserve(sizeof(void*)>4?std::uintptr_t(0x128000000ull):std::uintptr_t(0x2a000000u)));Require(low&&owner&&alias);
 const auto expected=Run(low,owner,alias);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,poolBytes);Require(Run(high,owner,alias)==expected);for(unsigned i=0;i<poolBytes;++i)Require(low[i]==0x7b);
  auto* ownerBit31=static_cast<BYTE*>(Reserve(0x8e000000u));auto* aliasBit31=static_cast<BYTE*>(Reserve(std::uintptr_t(0x18e000000ull)));Require(ownerBit31&&aliasBit31);
  Require(Run(low,ownerBit31,aliasBit31)==expected);std::memset(low,0x7b,poolBytes);Require(Run(high,ownerBit31,aliasBit31)==expected);for(unsigned i=0;i<poolBytes;++i)Require(low[i]==0x7b);
  std::puts("pool alto;owner32 bajo/bit31,alias opuesto,case y bytes correctos");
 }
 std::printf("Particle owner: 15360 casos por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def harness(revision=None):
    source=read_source('src/Render/Particle_Move.cpp',revision)
    begin=source.index('case 2:',source.index('if (iVar5 == 0x498)'));end=source.index('case 3:',begin)
    macros='\n'.join(re.findall(r'^#define P_[A-Z0-9]+(?:\([^)]*\))?\s+[^\n]+',source,re.M))
    if '#define P_ENT(o)' not in macros:raise ValueError('Macros de partículas incompletas')
    globals_source=read_source('src/globals.cpp',revision)
    constants='\n'.join(re.search(r'^float\s+'+name+r'\s*=\s*[^;]+;',globals_source,re.M).group(0) for name in ('_DAT_00552b28','_DAT_00552874'))
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('MACROS',macros).replace('CONSTANTS',constants).replace('CASE',source[begin:end])+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-particle-owner-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
