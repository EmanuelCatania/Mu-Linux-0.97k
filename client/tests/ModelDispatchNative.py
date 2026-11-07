#!/usr/bin/env python3
"""Despacho real de RenderBody y sonidos: slots, scripts, traza y punteros completos."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from AnimationBonesPointer import layouts
from ObjectLayoutContract import record
from SceneObjectPointer import function,read_source,build_and_run
SERVICES=r'''
#include <array>
#include <limits>
uintptr_t DAT_05828d58;float _DAT_00552544=.99f;
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}
std::size_t slotCount;namespace ResourceTables{std::size_t ModelCount(){return slotCount;}}
std::vector<unsigned> events;
unsigned Bits(float f){unsigned u;std::memcpy(&u,&f,4);return u;}
void Event(unsigned id,std::initializer_list<unsigned> args={}){events.push_back(id);events.insert(events.end(),args.begin(),args.end());}
void BMD__BeginRender(){Event(1);}
namespace BMDDevice{void EndModel(){Event(2);}}
namespace BufferedDraw{Stream stream;void NativeColor(float r,float g,float b,float a){Event(3,{Bits(r),Bits(g),Bits(b),Bits(a)});}}
BMD* wantedModel;
void BMD__RenderMesh(void* model,float index,int flags,float alpha,int blend,float light,float u,float v,unsigned texture){Require(model==wantedModel);Event(4,{Bits(index),unsigned(flags),Bits(alpha),unsigned(blend),Bits(light),Bits(u),Bits(v),texture});}
'''
MAIN=r'''
void Gold(std::vector<unsigned>& out,unsigned id,std::initializer_list<unsigned> args={}){out.push_back(id);out.insert(out.end(),args.begin(),args.end());}
void Hash(std::uint64_t& h,unsigned n){for(unsigned b=0;b<4;++b){h^=BYTE(n>>(8*b));h*=1099511628211ull;}}
std::uint64_t Run(BYTE* pool,BYTE* tables,BYTE* scripts){const std::size_t bytes=17*sizeof(BMD)+32;std::uint64_t hash=14695981039346656037ull;const int slots[]={0,1,16},meshCounts[]={-1,0,1,2,4},blends[]={-2,-1,0,3};const float alphas[]={0,.5f,.99f,1};unsigned rendered=0,rejected=0;
 for(unsigned trial=0;trial<4096;++trial){std::memset(pool,0xa5,bytes);std::memset(tables,0x6d,1024);std::memset(scripts,0x37,128);auto* base=reinterpret_cast<BMD*>(pool+16);const int slot=slots[trial%3];auto* model=base+slot;wantedModel=model;DAT_05828d58=reinterpret_cast<uintptr_t>(base);slotCount=17;
  model->NumMeshs=short(meshCounts[(trial/3)%5]);model->Meshs=tables+16;model->LightEnable=BYTE((trial/7)%3);model->BodyLight[0]=.25f;model->BodyLight[1]=.5f;model->BodyLight[2]=.75f;
  const unsigned scriptVariants[]={0,1,2,3,4,5};unsigned variants[4];auto* meshes=reinterpret_cast<ClothModelMesh*>(tables+16);
  for(unsigned m=0;m<4;++m){variants[m]=scriptVariants[(trial/11+m)%6];meshes[m].TextureFlags=variants[m]?scripts+m*16:nullptr;const BYTE first[]={0,0,1,0,1,255},second[]={0,0,0,1,255,0};scripts[m*16]=first[variants[m]];scripts[m*16+1]=second[variants[m]];}
  const unsigned scenario=trial%15;bool valid=true;void* argument=model;
  switch(scenario){case 0:DAT_05828d58=0;valid=false;break;case 1:argument=nullptr;valid=false;break;case 2:argument=pool+15;valid=false;break;case 3:argument=base+17;valid=false;break;case 4:argument=reinterpret_cast<BYTE*>(model)+1;valid=false;break;case 5:slotCount=slot;valid=false;break;case 6:slotCount=0;valid=false;break;case 7:model->NumMeshs=0;break;case 8:model->Meshs=nullptr;break;case 9:model->Meshs=reinterpret_cast<BYTE*>(uintptr_t(0x80000));break;default:break;}
  valid=valid&&model->NumMeshs!=0&&model->Meshs&&uintptr_t(model->Meshs)>=0x100000;
  const int flags=int(trial%2048),blend=blends[(trial/17)%4],hidden=int((trial/23)%6)-1,texture=trial%2?-1:1170;const float alpha=alphas[(trial/31)%4],light=.75f,u=trial%2?.125f:0,v=trial%3?-.25f:0;
  std::vector<unsigned> expected;events.clear();
  if(valid){Gold(expected,1);if(!model->LightEnable)Gold(expected,3,{Bits(.25f),Bits(.5f),Bits(.75f),Bits(alpha<.99f?alpha:1.f)});for(int m=0;m<model->NumMeshs;++m){const unsigned variant=variants[m];if(m==hidden||variant==3||variant==4)continue;const int chosenBlend=variant==2||variant==5?m:blend;Gold(expected,4,{Bits(float(m)),unsigned(flags),Bits(alpha),unsigned(chosenBlend),Bits(light),Bits(u),Bits(v),unsigned(texture)});++rendered;}Gold(expected,2);}else ++rejected;
  const std::vector<BYTE> beforePool(pool,pool+bytes),beforeTables(tables,tables+1024),beforeScripts(scripts,scripts+128);
  BMD__RenderBody(argument,flags,alpha,blend,light,u,v,hidden,texture);Require(events==expected);Require(std::memcmp(pool,beforePool.data(),bytes)==0&&std::memcmp(tables,beforeTables.data(),1024)==0&&std::memcmp(scripts,beforeScripts.data(),128)==0);for(unsigned e:events)Hash(hash,e);
  // Sonidos: verificar sólo los diez bytes del slot elegido y todas las fronteras.
  DAT_05828d58=reinterpret_cast<uintptr_t>(base);const int values[]={0,-1,32768,0x1234ffff,(std::numeric_limits<int>::min)(),(std::numeric_limits<int>::max)()};int sounds[5];auto wanted=beforePool;
  for(unsigned s=0;s<5;++s){sounds[s]=values[(trial+s)%6];const std::uint16_t bits=std::uint16_t(sounds[s]);std::memcpy(wanted.data()+16+slot*sizeof(BMD)+offsetof(BMD,Sounds)+s*2,&bits,2);Hash(hash,bits);}
  Model_SetAnimationSlots(slot,sounds[0],sounds[1],sounds[2],sounds[3],sounds[4]);Require(std::memcmp(pool,wanted.data(),bytes)==0);
 }Require(rendered>0&&rejected>0);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto hash=Run(low+256,low+16384,low+32768);
 if(sizeof(void*)==8){auto* high=static_cast<BYTE*>(Reserve(0x126000000ull));Require(high);for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,65536);Require(Run((mask&1?high:low)+256,(mask&2?high:low)+16384,(mask&4?high:low)+32768)==hash);if(mask&1)for(unsigned i=0;i<17*sizeof(BMD)+32;++i)Require(low[256+i]==0x7b);if(mask&2)for(unsigned i=0;i<1024;++i)Require(low[16384+i]==0x7b);if(mask&4)for(unsigned i=0;i<128;++i)Require(low[32768+i]==0x7b);}auto* sign=static_cast<BYTE*>(Reserve(0x86000000u));Require(sign);Require(Run(sign+256,sign+16384,sign+32768)==hash);}
 std::printf("ModelDispatch: 4096 despachos/sonidos por ruta, guardas/scripts/bytes %016llx\n",(unsigned long long)hash);
}
'''

def harness(revision=None):
    headers='\n'.join(re.sub(r'#include "[^\"]+"|#pragma once','',read_source(p))for p in ['src/Render/BufferedDraw.h'])
    source=read_source('src/Render/SMD_Parser.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+layouts()+'\n#pragma pack(push,1)\n'+record(read_source('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\n'+headers+SERVICES+function(source,'BMD__RenderBody')+function(source,'Model_SetAnimationSlots')+MAIN

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');a=p.parse_args()
    if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-model-dispatch-')as t:
        result=build_and_run(Path(t)/'current',harness(),a);print(result,end='')
        if a.reference_ref:
            old=build_and_run(Path(t)/'reference',harness(a.reference_ref),a)
            if old!=result:raise RuntimeError('Diferencia frente a x86')
            print('Equivalencia con '+a.reference_ref+': OK')
if __name__=='__main__':main()
