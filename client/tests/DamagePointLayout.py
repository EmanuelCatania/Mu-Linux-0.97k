#!/usr/bin/env python3
"""Piloto B2: contrato y consumidores reales del pool de daño, comparados byte a byte."""
import argparse,os,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,function,read_source

def block(source,marker):
 source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S);start=source.index(marker);brace=source.index('{',start);end=brace+1;depth=1
 while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
 return source[start:end]+(';' if marker.startswith('struct ') else '')+'\n'
SERVICES=r'''
struct Draw{float position[3],color[3],alpha,scale;int value;};std::vector<Draw> draws;std::uint64_t pointGeneration[100];int created,lastSlot,alphaCalls,depthCalls,snapshotCalls;float interpolation;
void RenderSnapshots_NewDamagePoint(int slot){++created;lastSlot=slot;++pointGeneration[slot];}
void EnableAlphaTest(bool value){Require(value);++alphaCalls;}void GL_DisableDepthTest(){++depthCalls;}
float RenderInterpolation_Alpha(){return interpolation;}
bool RenderSnapshots_DamagePoint(int slot,RenderPosePair& pair){++snapshotCalls;if(slot%2)return false;pair.current.position[0]=1000.f+slot;pair.current.position[1]=2000;pair.current.position[2]=3000;pair.current.scale=9;pair.current.alpha=.7f;return true;}
RenderPose RenderSnapshots_Interpolate(const RenderPosePair& pair,float){return pair.current;}
void RenderNumber(float* p,int value,float* color,float alpha,float scale){Draw d;std::memcpy(d.position,p,12);std::memcpy(d.color,color,12);d.alpha=alpha;d.scale=scale;d.value=value;draws.push_back(d);}
'''
MAIN=r'''
template<class T>void Put(BYTE* bytes,int offset,T value){std::memcpy(bytes+offset,&value,sizeof(value));}template<class T>T Get(const BYTE* bytes,int offset){T value;std::memcpy(&value,bytes+offset,sizeof(value));return value;}
int main(){BYTE* pool=reinterpret_cast<BYTE*>(DAT_07c80110);Require(sizeof(DAT_07c80110)==11200);if(sizeof(void*)==8)Require(reinterpret_cast<std::uintptr_t>(pool)>0xffffffffull);BYTE wanted[11200],before[11200];std::uint64_t hash=14695981039346656037ull;auto Hash=[&](const void* p,std::size_t n){for(auto* b=static_cast<const BYTE*>(p);n--;++b){hash^=*b;hash*=1099511628211ull;}};unsigned cases=0;
 float position[3]={12,-20,33},color[3]={.25f,.5f,.75f};const int values[]={0,1,-1,65535,2147483647,(-2147483647-1),0x7fc12345,0x7f800001};
 for(int slot=0;slot<=100;++slot)for(int value:values)for(float scale:{15.f,50.f}){
  std::memset(pool,0xa5,11200);for(int i=0;i<100;++i)pool[i*112]=1;if(slot<100)pool[slot*112]=0;std::memcpy(wanted,pool,11200);created=0;lastSlot=-1;std::fill(pointGeneration,pointGeneration+100,0);
  if(slot<100){BYTE* p=wanted+slot*112;p[0]=1;Put(p,4,value);Put(p,12,scale);std::memcpy(p+16,position,12);Put(p,24,position[2]+140.f);std::memcpy(p+28,color,12);Put(p,56,int(0));Put(p,72,10.f);}
  CreatePoint(position,value,color,scale);Require(std::memcmp(wanted,pool,11200)==0&&created==int(slot<100)&&lastSlot==(slot<100?slot:-1));for(int i=0;i<100;++i)Require(pointGeneration[i]==std::uint64_t(slot<100&&i==slot));Hash(pool,11200);++cases;
 }
 std::memset(pool,0x6d,11200);for(int i=0;i<100;++i){BYTE* p=pool+i*112;p[0]=i%3?1:0;Put(p,4,i%5?i:-1);Put(p,12,float(5+(i%4)*15));for(int j=0;j<3;++j){Put(p,16+j*4,float(i*3+j));Put(p,28+j*4,float(j+1)*.25f);}Put(p,56,(i%4)-2);Put(p,72,float(i%4)*3.25f);pointGeneration[i]=i+1;}std::memcpy(wanted,pool,11200);
 for(int tick=0;tick<48;++tick){for(int i=0;i<100;++i){BYTE* p=wanted+i*112;if(!p[0])continue;int delay=Get<int>(p,56)-1;Put(p,56,delay);if(delay>=0)continue;Put(p,24,Get<float>(p,72)+Get<float>(p,24));float life=Get<float>(p,72)-.3f;Put(p,72,life);if(life<=0)p[0]=0;float scale=Get<float>(p,12)-5.f;Put(p,12,scale<15.f?15.f:scale);}
  DamageNumbers_Tick();Require(std::memcmp(wanted,pool,11200)==0);Hash(pool,11200);
  for(int i=0;i<100;++i){Sample sample=PointSample(i);BYTE* p=wanted+i*112;Require(sample.live==bool(p[0]));if(sample.live){Require(sample.key==Get<int>(p,4)&&sample.generation==pointGeneration[i]&&sample.pose.scale==Get<float>(p,12)&&sample.pose.alpha==Get<float>(p,72)*.40000001f&&std::memcmp(sample.pose.position,p+16,12)==0);}}
  for(float alpha:{0.f,.5f,1.f}){interpolation=alpha;draws.clear();alphaCalls=depthCalls=snapshotCalls=0;std::memcpy(before,pool,11200);RenderPoints(0,0,0,0);std::size_t draw=0;int live=0;for(int i=0;i<100;++i){BYTE* p=wanted+i*112;if(!p[0])continue;++live;Require(draw<draws.size());const Draw& d=draws[draw++];float expectedPosition[3];std::memcpy(expectedPosition,p+16,12);float expectedScale=Get<float>(p,12),expectedAlpha=Get<float>(p,72)*.40000001f;if(alpha<1&&i%2==0){expectedPosition[0]=1000.f+i;expectedPosition[1]=2000;expectedPosition[2]=3000;expectedScale=9;expectedAlpha=.7f;}Require(d.value==Get<int>(p,4)&&std::memcmp(d.color,p+28,12)==0&&std::memcmp(d.position,expectedPosition,12)==0&&d.scale==expectedScale&&d.alpha==expectedAlpha);Hash(d.position,12);Hash(&d.value,4);Hash(&d.alpha,4);Hash(&d.scale,4);}Require(draw==draws.size()&&alphaCalls==1&&depthCalls==1&&snapshotCalls==(alpha<1?live:0)&&std::memcmp(before,pool,11200)==0);++cases;}
 }
 Require(cases==1760);std::printf("Damage layout:1760 escenarios,100 slots %016llx\n",(unsigned long long)hash);}
'''
def harness(revision=None):
 header='' if revision else read_source('src/Render/DamagePointTypes.h');globals_source=read_source('src/globals.cpp',revision);declaration=re.search(r'(?:char|DamagePoint)\s+DAT_07c80110\[[^\n]+;',globals_source);assert declaration
 poses=read_source('src/Game/RenderSnapshots.h',revision);samples=read_source('src/Game/RenderSnapshots.cpp',revision);structs=''.join(block(poses,'struct '+name+' {') for name in ['RenderPose','RenderPosePair'])+block(samples,'struct Sample {')
 constants='';g=read_source('src/globals.cpp',revision)
 for name in ['_DAT_00552958','_DAT_005528b8','_DAT_00552580','_DAT_00552660','_DAT_00552834']:
  match=re.search(r'float\s+'+name+r'\s*=\s*[^;]+;',g);assert match;constants+=match.group()+'\n'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#include <algorithm>\nusing undefined1=unsigned char;\n'+header+structs+declaration.group()+constants+SERVICES+function(read_source('src/Combat/Skills_WarriorLegacy.cpp',revision),'Entity_TeleportAnim')+function(read_source('src/Entity/Entity_LegacyTeleport.cpp',revision),'CreatePoint')+function(read_source('src/Render/Effect_Tick.cpp',revision),'DamageNumbers_Tick')+block(samples,'Sample PointSample(')+function(read_source('src/Render/Render_LegacyDamageNumbers.cpp',revision),'RenderPoints')+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-damage-layout-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
