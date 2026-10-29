#!/usr/bin/env python3
"""Polígono/rayo reales: vértices nativos y array local de punteros sin stride32."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
float DAT_083a4120,DAT_083a4130,DAT_083a4134,DAT_083a4138;
'''
MAIN=r'''
int Invoke(const float* point,int count,float** v,int plane,float normal){return Collision_PointInPolygon(point[0],point[1],point[2],count,A0,A1,A2,A3,plane,normal);}
int Oracle(const float* point,int count,float** v,int plane,float normal){
 if(normal>0)plane*=8;int u,w;
 switch(plane){case 1:case 8:u=1;w=2;break;case 2:case 16:u=2;w=0;break;case 4:case 32:u=0;w=1;break;default:return 1;}
 for(int current=0;current<count;++current){const int previous=current?current-1:count-1;const double x=v[current][u]-double(point[u]),y=v[current][w]-double(point[w]),px=v[previous][u]-double(point[u]),py=v[previous][w]-double(point[w]);const double cross=x*py-y*px;if(plane<8?cross<=0:cross>=0)return 0;}return 1;
}
void Hash(std::uint64_t& h,unsigned value){h^=value;h*=1099511628211ull;}
void Geometry(float** v,int u,int w,float radius,bool reverse){const float xy[8]={-1,-1,-1,1,1,1,1,-1};for(unsigned i=0;i<4;++i){const unsigned source=reverse?3-i:i;for(unsigned a=0;a<3;++a)v[i][a]=0;v[i][u]=xy[2*source]*radius;v[i][w]=xy[2*source+1]*radius;}}
std::uint64_t Run(BYTE** memory){
 float* vertices[4];for(unsigned i=0;i<4;++i)vertices[i]=reinterpret_cast<float*>(memory[i]+16);std::uint64_t hash=14695981039346656037ull;
 const int planes[]={-1,0,1,2,3,4,8,16,32,33,40,64};const float normals[]={-1.0f,-0.0f,0.0f,1.0f};unsigned cases=0;
 for(unsigned frame=0;frame<32;++frame)for(int plane:planes)for(float normal:normals)for(int count=-1;count<=4;++count){
  for(unsigned i=0;i<4;++i)std::memset(memory[i],0xa5+i,64);
  const int effective=normal>0?plane*8:plane;const int u=effective==1||effective==8?1:effective==2||effective==16?2:0,w=(u+1)%3;const float radius=float(frame%5+1);
  Geometry(vertices,u,w,radius,frame%2);if(frame%8==7)std::memcpy(vertices[2],vertices[1],12);
  BYTE before[4][64];for(unsigned i=0;i<4;++i)std::memcpy(before[i],memory[i],64);
  const float points[18]={-radius,-radius,-radius,0,0,0,radius,0,radius,radius,2*radius,0,-radius*0.5f,-radius*0.5f,-radius*0.5f,radius*0.5f,0,2*radius};
  for(unsigned q=0;q<9;++q){float point[3]={0,0,0};point[u]=points[2*q];point[w]=points[2*q+1];const int expected=Oracle(point,count,vertices,plane,normal);float* nil[4]={nullptr,nullptr,nullptr,nullptr};const int actual=Invoke(point,count,count<=0?nil:vertices,plane,normal);Require(actual==expected);Hash(hash,actual);++cases;}
  for(unsigned i=0;i<4;++i)Require(std::memcmp(before[i],memory[i],64)==0);
 }
 Require(cases==82944);unsigned rayCases=0;
 for(unsigned axis=0;axis<3;++axis)for(int sign:{-1,1})for(int a=-3;a<=3;++a)for(int b=-3;b<=3;++b)for(unsigned direction=0;direction<3;++direction)for(unsigned limit=0;limit<3;++limit)for(int flag:{0,1,-1})for(int count:{3,4}){
  for(unsigned i=0;i<4;++i)std::memset(memory[i],0xc3+i,64);
  const unsigned u=(axis+1)%3,w=(axis+2)%3;Geometry(vertices,u,w,4,sign>0);
  float point[3]={0,0,0},start[3]={0,0,0},target[3]={0,0,0},normal[3]={0,0,0};point[u]=start[u]=target[u]=float(a*2);point[w]=start[w]=target[w]=float(b*2);normal[axis]=float(sign);start[axis]=direction==2?0.0f:float(sign*4);target[axis]=direction==1?float(sign*8):float(-sign*4);
  const float closest[]={0.25f,0.5f,1.0f};DAT_083a4120=closest[limit];DAT_083a4130=17;DAT_083a4134=19;DAT_083a4138=23;
  const float t=direction==2?0.0f:0.5f;const int plane=axis==0?1:axis==1?2:4;const unsigned expected=direction!=1 && t<=closest[limit] && Oracle(point,count,vertices,plane,float(sign));
  BYTE before[4][64];for(unsigned i=0;i<4;++i)std::memcpy(before[i],memory[i],64);
  const unsigned actual=CollisionDetectLineToFace(start,target,count,vertices[0],vertices[1],vertices[2],vertices[3],normal,static_cast<char>(flag));Require(actual==expected);
  Require(DAT_083a4120==(expected && flag?t:closest[limit]) && DAT_083a4130==(expected && flag?point[0]:17) && DAT_083a4134==(expected && flag?point[1]:19) && DAT_083a4138==(expected && flag?point[2]:23));
  for(unsigned i=0;i<4;++i)Require(std::memcmp(before[i],memory[i],64)==0);Hash(hash,actual);std::uint32_t bits;for(float value:{DAT_083a4120,DAT_083a4130,DAT_083a4134,DAT_083a4138}){std::memcpy(&bits,&value,4);Hash(hash,bits);}++rayCases;
 }
 Require(rayCases==15876);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);BYTE* memory[4];for(unsigned i=0;i<4;++i)memory[i]=low+1024*i;const auto expected=Run(memory);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);for(unsigned mask=1;mask<16;++mask){std::memset(low,0x7b,3200);for(unsigned i=0;i<4;++i)memory[i]=(mask&(1u<<i)?high:low)+1024*i;Require(Run(memory)==expected);for(unsigned i=0;i<4;++i)if(mask&(1u<<i))for(unsigned j=0;j<64;++j)Require(low[1024*i+j]==0x7b);}std::puts("16 combinaciones de vertices altos/bajos; array local nativo y aliases intactos");}
 std::printf("Polygon: 82944 puntos y 15876 rayos, resultados/readonly %016llx\n",static_cast<unsigned long long>(expected));}
'''
def real_double(source,name):
 return function(source.replace('double __cdecl '+name,'float10 __cdecl '+name),name).replace('float10 __cdecl '+name,'double __cdecl '+name,1)
def harness(revision=None):
 source=read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision);polygon=function(source,'Collision_PointInPolygon')
 tail=MAIN
 for i in range(4):tail=tail.replace('A'+str(i),'v['+str(i)+']' if 'const float* a5' in polygon else 'static_cast<int>(reinterpret_cast<uintptr_t>(v['+str(i)+']))')
 fabs=re.search(r'^long double\s+__cdecl Math_Fabs[^\n]+',read_source('src/Render/Render_LegacyTransforms.cpp',revision),re.M).group()
 ray=function(read_source('src/Terrain/Terrain_RayCollision.cpp',revision),'CollisionDetectLineToFace')
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+fabs+real_double(source,'Math_Fmin')+real_double(source,'Math_Fmax')+polygon+ray+tail
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-polygon-pointer-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
