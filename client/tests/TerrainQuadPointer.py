#!/usr/bin/env python3
"""Quad real de terreno: UVs tipados, alturas/luz y orden de backend verificados."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
#include <cmath>
float* DAT_080cb2cc;float* DAT_081cb608;
unsigned phase,vertices,texcoords,colors,components[4];bool expectLighting;float xyz[12],uv[8],rgba[16];
namespace BufferedDraw{
enum class Primitive{TriangleFan};
void Begin(Primitive mode){Require(!phase && mode==Primitive::TriangleFan);phase=1;}
void Color3fv(float* value){Require(phase==1 && expectLighting && vertices==texcoords && colors==vertices && colors<4);std::memcpy(rgba+colors*4,value,12);components[colors++]=3;}
void Color4f(float r,float g,float b,float a){Require(phase==1 && expectLighting && vertices==texcoords && colors==vertices && colors<4);const float values[]={r,g,b,a};std::memcpy(rgba+colors*4,values,16);components[colors++]=4;}
void TexCoord2f(float u,float v){Require(phase==1 && texcoords==vertices && texcoords<4 && colors==(expectLighting?vertices+1:0));uv[texcoords*2]=u;uv[texcoords*2+1]=v;++texcoords;}
void Vertex3fv(float* value){Require(phase==1 && texcoords==vertices+1 && vertices<4);std::memcpy(xyz+vertices*3,value,12);++vertices;}
void End(){Require(phase==1 && vertices==4 && texcoords==4 && colors==(expectLighting?4:0));phase=2;}
}
'''
MAIN=r'''
float Height(unsigned index){return float(int(index%37)-18)*.25f;}
float Light(unsigned index,unsigned channel){return float(int((index*3+channel)%17)-8)*.0625f;}
void Draw(float x,float y,float scale,int step,const float (*corners)[3],char enabled,float alpha){Terrain_RenderQuad(x,y,scale,step,ARG,enabled,alpha);}
void Hash(std::uint64_t& h,const void* p,unsigned size){for(unsigned i=0;i<size;++i){h^=static_cast<const BYTE*>(p)[i];h*=1099511628211ull;}}
std::uint64_t Run(BYTE* uvMemory,BYTE* heightMemory,BYTE* lightMemory){
 constexpr unsigned heightSize=65536*4+32,lightSize=65536*3*4+32;DAT_080cb2cc=reinterpret_cast<float*>(heightMemory+16);DAT_081cb608=reinterpret_cast<float*>(lightMemory+16);std::memset(heightMemory,0x5a,heightSize);std::memset(lightMemory,0x6b,lightSize);for(unsigned i=0;i<65536;++i){DAT_080cb2cc[i]=Height(i);for(unsigned a=0;a<3;++a)DAT_081cb608[i*3+a]=Light(i,a);}
 std::vector<BYTE> expectedHeight(heightMemory,heightMemory+heightSize),expectedLight(lightMemory,lightMemory+lightSize);std::uint64_t hash=14695981039346656037ull;
 const int coordinates[]={-2,-1,0,1,2,64,127,253,254,255,256,300};const float fractions[]={-.75f,0,.5f},sizes[]={-1,0,.5f,1,2};const std::uint32_t alphaBits[]={0x3f800000u,0u,0x80000000u,0x3f000000u,0x7fc12345u};
 for(unsigned trial=0;trial<32768;++trial){
  std::memset(uvMemory,0xa5,80);auto* corners=reinterpret_cast<float(*)[3]>(uvMemory+16);for(unsigned i=0;i<4;++i){corners[i][0]=float(int((trial+i*3)%17)-8)*.25f;corners[i][1]=float(int((trial/3+i*7)%19)-9)*.125f;const auto bits=0x7fc12345u+i;std::memcpy(&corners[i][2],&bits,4);}
  const float x=float(coordinates[trial%12])+fractions[(trial/144)%3],y=float(coordinates[(trial/12)%12])+fractions[(trial/432)%3],size=sizes[(trial/2592)%5];const int step=(trial/1296)%2;const char enabled=trial%3==0?0:trial%3==1?1:-1;float alpha;std::memcpy(&alpha,&alphaBits[(trial/7)%5],4);
  const int ix=int(x<0?std::ceil(x):std::floor(x)),iy=int(y<0?std::ceil(y):std::floor(y));const bool valid=ix>=0 && iy>=0 && ix<255 && iy<255;
  phase=vertices=texcoords=colors=0;expectLighting=enabled!=0;std::memset(xyz,0,sizeof(xyz));std::memset(uv,0,sizeof(uv));std::memset(rgba,0,sizeof(rgba));std::memset(components,0,sizeof(components));BYTE before[80];std::memcpy(before,uvMemory,80);
  Draw(x,y,size,step,valid?corners:nullptr,enabled,alpha);Require(std::memcmp(before,uvMemory,80)==0);
  if(!valid){Require(!phase && !vertices && !texcoords && !colors);}else{
   const unsigned indices[]={unsigned(iy*256+ix),unsigned(iy*256+ix+step),unsigned((iy+step)*256+ix+step),unsigned((iy+step)*256+ix)};const float x0=x*100.0f,y0=y*100.0f,x1=x0+size*100.0f,y1=y0+size*100.0f;const float expectedXYZ[]={x0,y0,Height(indices[0]),x1,y0,Height(indices[1]),x1,y1,Height(indices[2]),x0,y1,Height(indices[3])};Require(phase==2 && std::memcmp(xyz,expectedXYZ,48)==0);
   for(unsigned i=0;i<4;++i){Require(uv[i*2]==corners[i][0] && uv[i*2+1]==corners[i][1]);if(enabled){Require(components[i]==(alpha==1?3u:4u));for(unsigned a=0;a<3;++a)Require(rgba[i*4+a]==Light(indices[i],a));if(alpha!=1)Require(std::memcmp(rgba+i*4+3,&alpha,4)==0);}}
  }
  Hash(hash,&phase,4);Hash(hash,xyz,sizeof(xyz));Hash(hash,uv,sizeof(uv));Hash(hash,rgba,sizeof(rgba));Hash(hash,components,sizeof(components));
 }
 Require(std::memcmp(heightMemory,expectedHeight.data(),heightSize)==0 && std::memcmp(lightMemory,expectedLight.data(),lightSize)==0);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* heights=static_cast<BYTE*>(Reserve(0x28000000u));auto* lights=static_cast<BYTE*>(Reserve(0x29000000u));Require(low && heights && lights);const auto expected=Run(low,heights,lights);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* highHeights=static_cast<BYTE*>(Reserve(std::uint64_t(0x128000000ull)));auto* highLights=static_cast<BYTE*>(Reserve(std::uint64_t(0x129000000ull)));Require(high && highHeights && highLights);for(unsigned mask=1;mask<8;++mask){std::memset(low,0x7b,80);std::memset(heights,0x7b,65536*4+32);std::memset(lights,0x7b,65536*3*4+32);Require(Run(mask&1?high:low,mask&2?highHeights:heights,mask&4?highLights:lights)==expected);if(mask&1)for(unsigned i=0;i<80;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<65536*4+32;++i)Require(heights[i]==0x7b);if(mask&4)for(unsigned i=0;i<65536*3*4+32;++i)Require(lights[i]==0x7b);}std::puts("ocho combinaciones UV/alturas/luz altas y bajas; aliases intactos");}
 std::printf("Terrain quad: 32768 casos, indices/vertices/UV/color %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/Terrain/Terrain_LegacyRender.cpp',revision),'Terrain_RenderQuad');constant=re.search(r'^float\s+_DAT_005524f0\s*=\s*[^;]+;',read_source('src/globals.cpp',revision),re.M).group()
 arg='corners' if 'const float (*p5)[3]' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(corners))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')+SERVICES+constant+code+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-terrain-quad-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
