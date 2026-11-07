#!/usr/bin/env python3
"""Quad real de sprite: arrays locales nativos; geometría y backend controlados."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
using undefined4=std::uint32_t;
float CameraMatrix[12];
char textureFlags[1208*56];
#define DAT_083a7cc8 textureFlags[0]
float expectedPosition[3],expectedLight[3],expectedRotation;
int expectedTexture;unsigned phase,rotations,matrices,vertexCount,uvCount;
float vertices[12],uvs[8],color[4];unsigned colorSize;
void GL_BindTextureSlot(int texture){Require(!phase && texture==expectedTexture);phase=1;}
void Vector_Transform(float* p,float* m,float* out){Require(phase==1 && m==CameraMatrix && std::memcmp(p,expectedPosition,12)==0);for(unsigned i=0;i<3;++i)out[i]=p[i]+float((i+1)*4);phase=2;}
void Matrix_BuildFromEuler(float* angle,float* matrix){Require(phase==2 && !matrices && angle[0]==0 && angle[1]==0 && angle[2]==expectedRotation);matrix[0]=37;matrices=1;}
void Vector_Rotate(float* in,float* matrix,float* out){Require(phase==2 && matrices==1 && matrix[0]==37 && rotations<4);out[0]=-in[1];out[1]=in[0];out[2]=in[2];++rotations;}
namespace BufferedDraw{
enum class Primitive{Quads};
void Begin(Primitive p){Require(phase==2 && p==Primitive::Quads);phase=3;}
void Color3fv(float* p){Require(phase==3 && std::memcmp(p,expectedLight,12)==0);std::memcpy(color,p,12);colorSize=3;phase=4;}
void Color4f(float r,float g,float b,float a){Require(phase==3);color[0]=r;color[1]=g;color[2]=b;color[3]=a;colorSize=4;phase=4;}
void TexCoord2f(float u,float v){Require(phase==4 && uvCount==vertexCount && uvCount<4);uvs[uvCount*2]=u;uvs[uvCount*2+1]=v;++uvCount;}
void Vertex3fv(float* p){Require(phase==4 && uvCount==vertexCount+1 && vertexCount<4);std::memcpy(vertices+vertexCount*3,p,12);++vertexCount;}
void End(){Require(phase==4 && vertexCount==4 && uvCount==4);phase=5;}
}
'''
MAIN=r'''
void Hash(std::uint64_t& h,const void* p,unsigned n){for(unsigned i=0;i<n;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* positions,BYTE* lights){
 std::uint64_t hash=14695981039346656037ull;
 const float angles[]={0.0f,-0.0f,45.0f,-90.0f,180.0f,359.5f};const int textures[]={0,1,0x4b6,0x4b7};
 for(unsigned trial=0;trial<32768;++trial){
  std::memset(positions,0xc3,64);std::memset(lights,0xa5,64);auto* p=reinterpret_cast<float*>(positions+16);auto* light=reinterpret_cast<float*>(lights+16);
  for(unsigned i=0;i<3;++i){p[i]=float(int((trial>>(i*3))%129)-64)*0.25f;light[i]=float((trial>>(i*2))%17)*0.0625f;}
  std::memcpy(expectedPosition,p,12);std::memcpy(expectedLight,light,12);expectedRotation=angles[trial%6];expectedTexture=textures[(trial/6)%4];textureFlags[expectedTexture*56]=(trial/24)%2?3:4;
  const float width=float(int((trial/48)%65)-32)*0.5f,height=float(int((trial/3120)%17)-8)*0.5f;
  const float u=float(trial%9)*0.125f,v=float((trial/9)%9)*0.125f,uw=float(int((trial/81)%9)-4)*0.25f,vh=float(int((trial/729)%9)-4)*0.25f;
  const float x=p[0]+4,y=p[1]+8,z=p[2]+12,hw=width*0.5f,hh=height*0.5f;
  const float offsets[8]={-hw,-hh,hw,-hh,hw,hh,-hw,hh};float expectedVertices[12];
  for(unsigned i=0;i<4;++i){expectedVertices[i*3]=x+(expectedRotation==0?offsets[i*2]:-offsets[i*2+1]);expectedVertices[i*3+1]=y+(expectedRotation==0?offsets[i*2+1]:offsets[i*2]);expectedVertices[i*3+2]=z;}
  const float expectedUV[8]={u,v+vh,u+uw,v+vh,u+uw,v,u,v};
  BYTE positionBefore[64],lightBefore[64];std::memcpy(positionBefore,positions,64);std::memcpy(lightBefore,lights,64);
  phase=rotations=matrices=vertexCount=uvCount=colorSize=0;std::memset(color,0,sizeof(color));
  RenderSprite_0(expectedTexture,p,width,height,light,expectedRotation,u,v,uw,vh);
  Require(phase==5 && rotations==(expectedRotation==0?0u:4u) && matrices==(expectedRotation==0?0u:1u) && std::memcmp(vertices,expectedVertices,48)==0 && std::memcmp(uvs,expectedUV,32)==0 && std::memcmp(positions,positionBefore,64)==0 && std::memcmp(lights,lightBefore,64)==0);
  Require(std::memcmp(color,expectedLight,12)==0 && colorSize==(textureFlags[expectedTexture*56]==3?3u:4u));
  if(colorSize==4)Require(color[3]==(expectedTexture==0x4b6?1.0f:expectedLight[0]));
  Hash(hash,vertices,48);Hash(hash,uvs,32);Hash(hash,color,sizeof(color));Hash(hash,&colorSize,4);
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+1024);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,1152);Require(Run(mask&1?high:low,(mask&2?high:low)+1024)==expected);if(mask&1)for(unsigned i=0;i<64;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<64;++i)Require(low[1024+i]==0x7b);}std::puts("arrays locales nativos y cuatro combinaciones posicion/color altos y bajos");}
 std::printf("Sprite quad: 32768 casos, vertices/UV/color/orden %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/Render/Sprite.cpp',revision),'RenderSprite_0')
 globals_source=read_source('src/globals.cpp',revision)
 constants='\n'.join(re.search(r'^float\s+'+n+r'\s*=\s*[^;]+;',globals_source,re.M).group()for n in ('_DAT_00552504','_DAT_00552580'))
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+constants+code+MAIN
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-sprite-quad-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
