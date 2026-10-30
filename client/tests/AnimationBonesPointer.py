#!/usr/bin/env python3
"""Animación real: salida de huesos nativa y composición padre-hijo verificable."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
from ObjectLayoutContract import record
SERVICES=r'''
#include <cmath>
using float10=long double;
static inline double fsin(double x){return sin(x);}
#define fcos cosf
char* quaternionGlobal;float* parentGlobal;
#define DAT_05826e18 quaternionGlobal
#define DAT_06989c9c (*parentGlobal)
template<class T>void Store(BYTE* p,unsigned offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
void PtrNative(BYTE* p,unsigned offset,const void* pointer){Store(p,offset,reinterpret_cast<uintptr_t>(pointer));}
'''
MAIN=r'''
void Invoke(BYTE* model,float* bones,float frame,unsigned prior,unsigned char action,unsigned* angles,float* head,char special,char translate,bool direct,char* quats,float* parent){if(direct)AnimateWithScratch(model,DIRECT,frame,prior,action,angles,head,special,translate,quats,parent);else BMD_Animation(model,PUBLIC,frame,prior,action,angles,head,special,translate);}
float Translation(unsigned bone,unsigned action,unsigned frame,unsigned axis){return float((bone+1)*4+action*3+frame*(axis+1))+float(axis)*0.5f;}
void Identity(float* matrix,float scale,const float* origin){for(unsigned y=0;y<3;++y)for(unsigned x=0;x<4;++x)matrix[y*4+x]=x==3?origin[y]:x==y?scale:0.0f;}
std::uint64_t Run(BYTE* data,BYTE* modelMem,BYTE* bonesMem,BYTE* scratch){
 std::uint64_t hash=14695981039346656037ull;auto* model=modelMem+16;auto* bones=reinterpret_cast<float*>(bonesMem+16);auto* quats=reinterpret_cast<char*>(scratch+16);auto* parent=reinterpret_cast<float*>(scratch+128);quaternionGlobal=quats;parentGlobal=parent;
 const float frames[]={0,.25f,1.5f,2.75f,3,4.5f},priorFrames[]={0,.5f,1.75f,2.25f,3.5f},scales[]={.5f,1,2};const float origin[]={-4,0,8},initialOrigin[]={2,4,8},zeroOrigin[]={0,0,0};unsigned cases=0;
 for(unsigned trial=0;trial<4096;++trial)for(unsigned route=0;route<2;++route){
  std::memset(data,0xc3,8192);std::memset(modelMem,0xa5,ModelBytes);std::memset(bonesMem,0x3c,192);std::memset(scratch,0x5a,256);
  const short numBones=trial%4,actions=(trial/32)%3;const BYTE current=trial%4;const BYTE prior=(trial/4)%4;const unsigned clampedCurrent=current>=actions?0:current,clampedPrior=prior>=actions?0:prior;
  const float frame=frames[(trial/3)%6],priorFrame=priorFrames[(trial/5)%5],scale=scales[(trial/7)%3];unsigned priorBits;std::memcpy(&priorBits,&priorFrame,4);const int root=int((trial/512)%4)-1;const char special=trial&1,translate=(trial>>1)&1;const bool skipLast=(trial>>8)&1,secondRoot=(trial>>7)&1;
  auto* headers=data+512;const BYTE flags[2]={static_cast<BYTE>((trial>>8)&1),static_cast<BYTE>((trial>>9)&1)};
  for(unsigned action=0;action<2;++action){Store(headers,action*sizeof(ModelAnimationAction)+8,short(3));headers[action*sizeof(ModelAnimationAction)+10]=flags[action];}
  for(unsigned b=0;b<3;++b){auto* descriptor=data+16+b*sizeof(ModelAnimationBone);auto* keys=data+768+b*(2*sizeof(ModelAnimationKeys));Store(descriptor,32,short(b==0 || (b==2 && secondRoot)?-1:b-1));descriptor[34]=b==2 && skipLast;PtrNative(descriptor,36,keys);
   for(unsigned action=0;action<2;++action){auto* positions=data+1024+b*512+action*128;auto* eulers=data+3072+b*256+action*64;auto* rotations=data+4096+b*256+action*64;PtrNative(keys,action*sizeof(ModelAnimationKeys),positions);PtrNative(keys,action*sizeof(ModelAnimationKeys)+sizeof(void*),eulers);PtrNative(keys,action*sizeof(ModelAnimationKeys)+2*sizeof(void*),rotations);
    for(unsigned f=0;f<3;++f){for(unsigned a=0;a<3;++a){Store(positions,f*12+a*4,Translation(b,action,f,a));Store(eulers,f*12+a*4,0.0f);}for(unsigned a=0;a<4;++a)Store(rotations,f*16+a*4,a==3?((b+action)%2?-1.0f:1.0f):0.0f);}
   }
  }
  PtrNative(model,M(44),data+16);PtrNative(model,M(48),headers);Store(model,M(34),numBones);Store(model,M(38),actions);Store(model,M(84),root);Store(model,M(104),scale);std::memcpy(model+M(108),origin,12);Store(model,M(132),2.0f);model[M(160)]=current;
  auto* angles=reinterpret_cast<unsigned*>(data+6000);auto* head=reinterpret_cast<float*>(data+6020);std::memset(angles,0,12);std::memset(head,0,12);Identity(parent,1,initialOrigin);
  BYTE dataBefore[8192],modelExpected[ModelBytes],bonesExpected[192],scratchExpected[256];std::memcpy(dataBefore,data,8192);std::memcpy(modelExpected,modelMem,ModelBytes);std::memcpy(bonesExpected,bonesMem,192);std::memcpy(scratchExpected,scratch,256);
  modelExpected[16+M(160)]=static_cast<BYTE>(clampedCurrent);std::memset(modelExpected+16+M(120),0,12);Store(modelExpected,16+M(164),frame);const short curFrame=actions>0 && int(frame)>=3?0:short(int(frame));Store(modelExpected,16+M(168),curFrame);
  const float frac=frame-float(int(frame)),other=1-frac;const unsigned oldFrame=int(priorFrame)>=3?0:unsigned(priorFrame);float expected[3][12];bool written[3]={false,false,false};bool parentChanged=false;
  for(unsigned b=0;actions>0 && b<static_cast<unsigned>(numBones);++b){if(b==2 && skipLast)continue;written[b]=true;float t[3];for(unsigned a=0;a<3;++a)t[a]=other*Translation(b,clampedPrior,oldFrame,a)+frac*Translation(b,clampedCurrent,curFrame,a);
   if(b==0 && (flags[clampedPrior] || flags[clampedCurrent])){t[0]=Translation(b,clampedCurrent,0,0);t[1]=Translation(b,clampedCurrent,0,1);t[2]+=2;}
   const bool isRoot=b==0 || (b==2 && secondRoot);float parentMatrix[12];if(isRoot){Identity(parentMatrix,special?1:translate?scale:1,special?initialOrigin:translate?origin:zeroOrigin);if(!special)parentChanged=true;}else std::memcpy(parentMatrix,expected[b-1],48);
   std::memcpy(expected[b],parentMatrix,48);for(unsigned a=0;a<3;++a)expected[b][a*4+3]=parentMatrix[a*4+a]*t[a]+parentMatrix[a*4+3];
  }
  Invoke(model,bones,frame,priorBits,prior,angles,head,special,translate,route!=0,quats,parent);
  for(unsigned b=0;b<3;++b)if(written[b]){for(unsigned i=0;i<12;++i)Require(bones[b*12+i]==expected[b][i]);std::memcpy(bonesExpected+16+b*48,bones+b*12,48);const auto* q=reinterpret_cast<const float*>(quats+b*16);Require(q[0]==0 && q[1]==0 && q[2]==0 && (q[3]==1 || q[3]==-1));std::memcpy(scratchExpected+16+b*16,q,16);}
  if(parentChanged){float expectedParent[12];const float zero[]={0,0,0};Identity(expectedParent,translate?scale:1,translate?origin:zero);for(unsigned i=0;i<12;++i)Require(parent[i]==expectedParent[i]);std::memcpy(scratchExpected+128,parent,48);}
  Require(std::memcmp(data,dataBefore,8192)==0 && std::memcmp(modelMem,modelExpected,ModelBytes)==0 && std::memcmp(bonesMem,bonesExpected,192)==0 && std::memcmp(scratch,scratchExpected,256)==0);
  for(const auto pair:{std::pair<const BYTE*,unsigned>{modelMem,ModelBytes},{bonesMem,192},{scratch,256}})for(unsigned i=0;i<pair.second;++i){if(pair.first==modelMem && ((i>=16+M(44)&&i<16+M(44)+sizeof(void*))||(i>=16+M(48)&&i<16+M(48)+sizeof(void*))))continue;hash^=pair.first[i];hash*=1099511628211ull;}++cases;
 }
 Require(cases==8192);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+8192,low+16384,low+24576);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<16;++mask){std::memset(low+8192,0x7b,16640);Require(Run((mask&8?high:low),(mask&1?high:low)+8192,(mask&2?high:low)+16384,(mask&4?high:low)+24576)==expected);if(mask&1)for(unsigned i=0;i<ModelBytes;++i)Require(low[8192+i]==0x7b);if(mask&2)for(unsigned i=0;i<192;++i)Require(low[16384+i]==0x7b);if(mask&4)for(unsigned i=0;i<256;++i)Require(low[24576+i]==0x7b);}std::puts("dieciséis combinaciones tablas/modelo/salida/scratch altos y bajos; claves nativas, con direcciones altas/bajas");}
 std::printf("Animation bones: 8192 poses, jerarquia/clamp/bytes %016llx\n",static_cast<unsigned long long>(expected));}
'''
def layouts():
 header=read_source('src/Model/ModelTypes.h')
 code='#include <type_traits>\nusing WORD=std::uint16_t;using vec3_t=float[3];\n#pragma pack(push,1)\n'+'\n'.join(record(header,n)for n in ('ModelAnimationKeys','ModelAnimationBone','ModelAnimationAction','ModelRuntimeVertex','ModelRuntimeTriangle','BMD'))+'\n#pragma pack(pop)\n'
 code+=r"""
constexpr unsigned M(unsigned off){unsigned n=off;for(unsigned p:{40u,44u,48u,52u,56u,64u})if(p<off)n+=sizeof(void*)-4;return n;}
static_assert(sizeof(ModelAnimationBone)==(sizeof(void*)==4?140:144));
static_assert(sizeof(ModelAnimationAction)==(sizeof(void*)==4?16:20));
static_assert(sizeof(ModelAnimationKeys)==3*sizeof(void*));
static_assert(offsetof(ModelAnimationBone,Keys)==36&&offsetof(ModelAnimationBone,Dummy)==34&&offsetof(ModelAnimationBone,Parent)==32);
static_assert(offsetof(ModelAnimationAction,FrameCount)==8&&offsetof(ModelAnimationAction,LockPositions)==10&&offsetof(ModelAnimationAction,Positions)==12);
static_assert(offsetof(ModelAnimationKeys,EulerAngles)==sizeof(void*)&&offsetof(ModelAnimationKeys,Quaternions)==2*sizeof(void*));
static_assert(sizeof(BMD)==(sizeof(void*)==4?188:212));
inline BMD* AsModel(void* p){return static_cast<BMD*>(p);}
constexpr unsigned ModelBytes=sizeof(void*)==4?224:256;
"""
 code+='\n'.join(re.findall(r'static_assert\([^;]+;',header[header.index('struct ModelAnimationKeys'):header.index('struct BMD {')]))
 return code

def harness(revision=None,contracts=False):
 if contracts:return PREAMBLE.split('char DAT_00559bb4',1)[0]+layouts()+'int main(){std::printf("Animation layout: bone=%zu action=%zu keys=%zu ptr=%zu OK\\n",sizeof(ModelAnimationBone),sizeof(ModelAnimationAction),sizeof(ModelAnimationKeys),sizeof(void*));}'
 pose=read_source('src/Entity/ObjectPose.cpp',revision);helper=function(pose,'AnimateWithScratch');public=function(pose,'BMD_Animation')
 render=read_source('src/Render/Render_LegacyTransforms.cpp',revision);math=read_source('src/Math/Math_3D.cpp',revision);scene=read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision)
 bodies=function(math,'Matrix_BuildFromEuler')+''.join(function(render,n)for n in ('EulerToQuat','QuatToMatrix','QuatSlerp','R_ConcatTransforms'))+function(scene,'Terrain_QuadEqual')+helper+public
 names=sorted(set(re.findall(r'\b(?:_DAT_[0-9a-f]+|Math_DegreesToRadians)\b',bodies)));globals=read_source('src/globals.cpp',revision);constants=''.join(re.search(r'^float\s+'+n+r'\s*=\s*[^;]+;',globals,re.M).group()for n in names)
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+layouts()+constants+bodies+MAIN.replace('DIRECT','bones' if 'float* param_1' in helper else 'static_cast<int>(reinterpret_cast<uintptr_t>(bones))').replace('PUBLIC','bones' if 'float* bones' in public else 'static_cast<int>(reinterpret_cast<uintptr_t>(bones))')
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--contracts-only',action='store_true');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-animation-bones-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(contracts=args.contracts_only),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
