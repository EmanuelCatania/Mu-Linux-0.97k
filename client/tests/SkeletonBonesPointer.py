#!/usr/bin/env python3
"""Skeleton real en editor secuencial: paleta nativa, vértices/normales/bounds."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT, build_and_run, function, read_source
SERVICES=r'''
#include <algorithm>
#include <cmath>
using float10=long double;
static inline double fsin(double x){return sin(x);}
#define fcos cosf
TRANSFORM_HEADER
float* positions;float* normals;float* lighting;
#define DAT_0584621c (*positions)
#define DAT_05846224 (*(positions+2))
#define DAT_06f433bc (*normals)
#define DAT_077e298c (*lighting)
int SceneFlag=0,DAT_07e11d30=2;char DAT_005597c4;
unsigned invalidations,captures,expectedSources;const void* expectedModel;const float* expectedBones;
namespace Platform{constexpr int TruncateText=0;template<class...T>void TextFormatTruncated(T...){Require(false);}}
void DbgLogPublic(const char*){Require(false);}
template<class...T>bool ObjectPose_RestoreGeometry(T...){Require(false);return false;}
namespace TransformWorkers{struct Team{template<class...T>void Run(T...){Require(false);}};Team& SharedTeam(){static Team team;return team;}}
namespace Skinning{
SOURCE
void Invalidate(){Require(!invalidations && !captures);++invalidations;}
void Capture(const void* model,const float* bones,int count,float rotationScale,float bodyScale,const float* origin,const float* direction,const float* coefficients,const Source* sources,std::size_t sourceCount){
 Require(invalidations==1 && !captures && model==expectedModel && bones==expectedBones && count==3 && sourceCount==expectedSources);for(std::size_t i=0;i<sourceCount;++i){Require(sources[i].vertices && sources[i].normals && sources[i].world && sources[i].lighting && sources[i].vertexCount>0 && sources[i].normalCount>=0);}++captures;
}
}
template<class T>void Store(BYTE* p,unsigned offset,T value){std::memcpy(p+offset,&value,sizeof(value));}
void Ptr32(BYTE* p,unsigned offset,const void* pointer){const auto address=reinterpret_cast<uintptr_t>(pointer);Require(address<=0xffffffffull);Store(p,offset,static_cast<std::uint32_t>(address));}
'''
MAIN=r'''
void Invoke(BYTE* model,float* palette,float* bounds){float minimum[3]={-1,-2,-3},maximum[3]={1,2,3};Skeleton_Transform(model,ARG,minimum,maximum,bounds,charTranslate);}
GLOBAL_TRANSLATE
void Hash(std::uint64_t& h,const void* p,unsigned size){for(unsigned i=0;i<size;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* data,BYTE* modelMem,BYTE* boneMem,BYTE* pMem,BYTE* nMem,BYTE* lMem){
 auto* model=modelMem+16;auto* bones=reinterpret_cast<float*>(boneMem+16);positions=reinterpret_cast<float*>(pMem+16);normals=reinterpret_cast<float*>(nMem+16);lighting=reinterpret_cast<float*>(lMem+16);
 constexpr unsigned pSize=360064,lSize=120064;std::vector<BYTE> expectedP(pSize),expectedN(pSize),expectedL(lSize);std::uint64_t hash=14695981039346656037ull;
 const float scales[]={.5f,1,2},origin[]={4,-2,8};
 for(unsigned trial=0;trial<512;++trial){
  std::memset(data,0xc3,8192);std::memset(modelMem,0xa5,224);std::memset(boneMem,0x3c,192);std::memset(pMem,0x5a,pSize);std::memset(nMem,0x6b,pSize);std::memset(lMem,0x7c,lSize);
  const short meshes=trial%3;const bool lit=(trial/45)%2;DAT_005597c4=(trial/90)%2;charTranslate=(trial/180)%2;const float bodyScale=scales[(trial/7)%3];_DAT_005597c8=scales[(trial/5)%3];
  short vc[2]={},nc[2]={};expectedSources=0;Ptr32(model,40,data+16);Store(model,34,short(3));Store(model,36,meshes);model[68]=lit;Store(model,104,bodyScale);std::memcpy(model+108,origin,12);std::memset(model+140,0,12);
  for(unsigned b=0;b<3;++b){for(unsigned y=0;y<3;++y)for(unsigned x=0;x<4;++x)bones[b*12+y*4+x]=x==3?float(int(b)*(int(y)-1)):x==y?(y==0?float(b+1):y==1?2.0f:.5f):0.0f;}
  for(int mesh=0;mesh<meshes;++mesh){auto* header=data+16+mesh*40;auto* vertex=data+512+mesh*256;auto* normal=data+1536+mesh*256;vc[mesh]=(trial/3+mesh)%5;nc[mesh]=(trial/15+mesh)%3;Store(header,4,vc[mesh]);Store(header,6,nc[mesh]);Ptr32(header,16,vertex);Ptr32(header,20,normal);if(vc[mesh]>0)++expectedSources;
   for(int i=0;i<vc[mesh];++i){Store(vertex,i*16,short((trial+i)%3));for(unsigned a=0;a<3;++a)Store(vertex,i*16+4+a*4,float((i+1)*(a+1)+mesh));}
   for(int i=0;i<nc[mesh];++i){Store(normal,i*20,short((trial+i+1)%3));for(unsigned a=0;a<3;++a)Store(normal,i*20+4+a*4,float(int((i+a)%3)-1));}
  }
  BYTE dataBefore[8192],modelExpected[224],bonesBefore[192];std::memcpy(dataBefore,data,8192);std::memcpy(modelExpected,modelMem,224);std::memcpy(bonesBefore,boneMem,192);std::memcpy(expectedP.data(),pMem,pSize);std::memcpy(expectedN.data(),nMem,pSize);std::memcpy(expectedL.data(),lMem,lSize);
  float bounds[16];std::memset(bounds,0x5a,sizeof(bounds));BYTE boundsExpected[sizeof(bounds)];std::memcpy(boundsExpected,bounds,sizeof(bounds));invalidations=captures=0;expectedModel=model;expectedBones=bones;Invoke(model,bones,bounds+1);
  float lo[3]={999999,999999,999999},hi[3]={-999999,-999999,-999999};
  for(int mesh=0;mesh<meshes;++mesh){for(int i=0;i<vc[mesh];++i){const unsigned b=(trial+i)%3;float translated[3];for(unsigned a=0;a<3;++a){const float input=float((i+1)*(a+1)+mesh),diagonal=a==0?float(b+1):a==1?2.0f:.5f,offset=float(int(b)*(int(a)-1));float expected=input*diagonal*_DAT_005597c8+offset;if(charTranslate)expected*=bodyScale;lo[a]=(std::min)(lo[a],expected);hi[a]=(std::max)(hi[a],expected);if(charTranslate)expected+=origin[a];translated[a]=expected;Require(positions[mesh*45000+i*3+a]==expected);}std::memcpy(expectedP.data()+16+mesh*180000+i*12,positions+mesh*45000+i*3,12);}
   for(int i=0;i<nc[mesh];++i){const unsigned b=(trial+i+1)%3;float rotated[3];for(unsigned a=0;a<3;++a){const float diagonal=a==0?float(b+1):a==1?2.0f:.5f;rotated[a]=float(int((i+a)%3)-1)*diagonal;Require(normals[mesh*45000+i*3+a]==rotated[a]);}std::memcpy(expectedN.data()+16+mesh*180000+i*12,normals+mesh*45000+i*3,12);
    if(lit){const float dir[3]={DAT_005597c4?1.3f:0,DAT_005597c4?0:-1.5f,DAT_005597c4?2.0f:0};const double value=(double(dir[0])*rotated[0]+double(dir[1])*rotated[1]+double(dir[2])*rotated[2])*_DAT_00552530+_DAT_005528b4;const double expected=(std::max)(double(_DAT_005526e4),value);Require(std::fabs(lighting[mesh*15000+i]-expected)<0.00001);std::memcpy(expectedL.data()+16+mesh*60000+i*4,lighting+mesh*15000+i,4);}
   }
  }
  float expectedBounds[12]={};for(unsigned a=0;a<3;++a){expectedBounds[a]=lo[a]+origin[a];expectedBounds[3+a*4]=hi[a]-lo[a];}for(unsigned i=0;i<12;++i)Require(bounds[1+i]==expectedBounds[i]);std::memcpy(boundsExpected+4,bounds+1,48);const float radius=(std::max)(hi[0]-lo[0],hi[1]-lo[1]);Store(modelExpected,16+180,radius);
  Require(invalidations==1 && captures==1 && std::memcmp(data,dataBefore,8192)==0 && std::memcmp(boneMem,bonesBefore,192)==0 && std::memcmp(modelMem,modelExpected,224)==0 && std::memcmp(bounds,boundsExpected,sizeof(bounds))==0 && std::memcmp(pMem,expectedP.data(),pSize)==0 && std::memcmp(nMem,expectedN.data(),pSize)==0 && std::memcmp(lMem,expectedL.data(),lSize)==0);
  Hash(hash,modelMem,224);Hash(hash,bounds,sizeof(bounds));for(unsigned mesh=0;mesh<2;++mesh){Hash(hash,positions+mesh*45000,64);Hash(hash,normals+mesh*45000,64);Hash(hash,lighting+mesh*15000,32);}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* p=static_cast<BYTE*>(Reserve(0x28000000u));auto* n=static_cast<BYTE*>(Reserve(0x29000000u));auto* l=static_cast<BYTE*>(Reserve(0x2a000000u));Require(low && p && n && l);const auto expected=Run(low,low+8192,low+16384,p,n,l);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){std::memset(low+8192,0x7b,224);std::memset(low+16384,0x7b,192);Require(Run(low,(mask&1?high:low)+8192,(mask&2?high:low)+16384,p,n,l)==expected);if(mask&1)for(unsigned i=0;i<224;++i)Require(low[8192+i]==0x7b);if(mask&2)for(unsigned i=0;i<192;++i)Require(low[16384+i]==0x7b);}std::puts("cuatro combinaciones modelo/paleta altos; tablas y salidas legacy bajas");}
 std::printf("Skeleton: 512 poses editor secuencial, vertices/normales/bounds %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 source=read_source('src/Render/Entity_AnimationLegacy.cpp',revision);body=function(source,'Skeleton_Transform');math=read_source('src/Math/Math_3D.cpp',revision)
 bodies=''.join(function(math,n)for n in ('Matrix_BuildFromEuler','Vector_Transform','Vector_Rotate','Vector_InverseRotate'))+function(read_source('src/Entity/ObjectBounds.h',revision),'ObjectBounds_Build')+body
 names=sorted(set(re.findall(r'\b(?:_DAT_[0-9a-f]+|Math_DegreesToRadians)\b',bodies)));globals=read_source('src/globals.cpp',revision);constants=''.join(re.search(r'^float\s+'+n+r'\s*=\s*[^;]+;',globals,re.M).group()for n in names)
 skin=re.search(r'struct Source\s*\{[\s\S]*?\};',read_source('src/Render/Skinning.h',revision)).group()
 service=SERVICES.replace('TRANSFORM_HEADER','#include "'+(ROOT/'src/Render/TransformMath.h').as_posix()+'"').replace('SOURCE',skin)
 tail=MAIN.replace('ARG','palette' if 'float* boneMatrices' in body else 'static_cast<int>(reinterpret_cast<uintptr_t>(palette))').replace('GLOBAL_TRANSLATE','')
 return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x200000')+service+constants+'\nchar charTranslate;\n'+bodies+tail
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-skeleton-bones-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
