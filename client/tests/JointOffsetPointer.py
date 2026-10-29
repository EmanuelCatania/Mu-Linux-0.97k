#!/usr/bin/env python3
"""Desplazamiento de joints real, matriz/rotación reales y dirección nativa."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source
SERVICES=r'''
#include <cmath>
#include <algorithm>
'''
MAIN=r'''
void Invoke(BYTE* object,int flag){Joint_BoneOffsetApply(ARG,flag);}
std::uint64_t Run(BYTE* memory){
 std::uint64_t hash=14695981039346656037ull;const float angles[]={-90,-45,-0.0f,0,45,90,180};const int flags[]={0,1,-1,(-2147483647-1)};
 for(unsigned trial=0;trial<32768;++trial){
  std::memset(memory,0xa5,256);auto* object=memory+16;float position[3],offset[3],angle[3];
  for(unsigned axis=0;axis<3;++axis){position[axis]=float(int((trial>>(axis*3))%129)-64)*0.5f;offset[axis]=float(int((trial>>(axis*2))%65)-32)*0.25f;angle[axis]=angles[(trial/(axis+1))%7];std::memcpy(object+16+4*axis,position+axis,4);std::memcpy(object+192+4*axis,offset+axis,4);std::memcpy(object+28+4*axis,angle+axis,4);}
  const int flag=flags[(trial/7)%4];double expected[3];
  if(!flag)for(unsigned a=0;a<3;++a)expected[a]=double(position[a])+offset[a];
  else{
   const double x=angle[0]*double(Math_DegreesToRadians),y=angle[1]*double(Math_DegreesToRadians),z=angle[2]*double(Math_DegreesToRadians);
   const double sx=std::sin(x),cx=std::cos(x),sy=std::sin(y),cy=std::cos(y),sz=std::sin(z),cz=std::cos(z);
   const double m[9]={cy*cz,sx*sy*cz-cx*sz,sx*sz+cx*sy*cz,cy*sz,cx*cz+sx*sy*sz,cx*sy*sz-sx*cz,-sy,sx*cy,cx*cy};
   for(unsigned a=0;a<3;++a)expected[a]=position[a]+m[3*a]*offset[0]+m[3*a+1]*offset[1]+m[3*a+2]*offset[2];
  }
  BYTE before[256];std::memcpy(before,memory,256);Invoke(object,flag);
  for(unsigned a=0;a<3;++a){float actual;std::memcpy(&actual,object+16+4*a,4);Require(std::isfinite(actual) && std::fabs(actual-expected[a])<=0.00003*(std::max)(1.0,std::fabs(expected[a])));std::memcpy(before+16+16+4*a,&actual,4);}
  Require(std::memcmp(before,memory,256)==0);for(BYTE value:before){hash^=value;hash*=1099511628211ull;}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,256);Require(Run(high)==expected);for(unsigned i=0;i<256;++i)Require(low[i]==0x7b);std::puts("joint alto con matriz/rotacion reales; alias bajo intacto");}
 std::printf("Joint offset: 32768 casos, posicion/readonly %016llx\n",static_cast<unsigned long long>(expected));}
'''
def harness(revision=None):
 code=function(read_source('src/Render/SMD_Parser.cpp',revision),'Joint_BoneOffsetApply')
 arg='object' if 'BYTE* param_1' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(object))'
 constant=re.search(r'^float\s+Math_DegreesToRadians\s*=\s*[^;]+;',read_source('src/globals.cpp',revision),re.M).group()
 float_type=re.search(r'typedef\s+long double\s+float10\s*;',read_source('src/types.h',revision)).group()
 stdafx=read_source('src/stdafx.h',revision);sine=re.search(r'static inline double\s+fsin\([^\n]+',stdafx).group();cosine=re.search(r'#define fcos [^\n]+',stdafx).group()
 math=read_source('src/Math/Math_3D.cpp',revision)
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+float_type+constant+sine+'\n'+cosine+'\n'+function(math,'Matrix_BuildFromEuler')+function(math,'Vector_Rotate')+code+MAIN.replace('ARG',arg)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-joint-offset-')as temp:
  directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
