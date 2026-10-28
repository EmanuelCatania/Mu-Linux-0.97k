#!/usr/bin/env python3
"""Buffer real de vértices de tela: las dos caras sin truncar su dirección."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#ifndef _WIN32
#define __fastcall
#endif
BYTE *object,*nodes,*scratch;unsigned count,allocations,deletions,visited;bool failAllocation;
std::vector<std::uint32_t> events;
std::uint32_t Bits(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
void* operator_new(unsigned n){Require(!allocations++ && n==12*count);events.insert(events.end(),{10,n});return failAllocation?nullptr:scratch;}
void operator_delete(BYTE* p){Require(p==scratch && allocations==1 && !deletions++);events.push_back(9);}
GET_POS
void SpringNode_GetPos(void* p,float* out){Require(p==nodes+60*visited && reinterpret_cast<BYTE*>(out)==scratch+12*visited);events.insert(events.end(),{11,visited});ActualGetPos(p,out);}
void ClothPresentation_Vertex(void* p,int index,float* out){Require(p==object && unsigned(index)==visited && reinterpret_cast<BYTE*>(out)==scratch+12*visited);events.insert(events.end(),{12,visited});out[2]+=visited%2?.25f:-.25f;++visited;}
void GL_ResetState(){events.push_back(1);}
void GL_SetBlendSrcOver(char enabled){Require(enabled==1);events.push_back(2);}
void GL_BindTextureSlot(int texture){events.push_back(4);events.push_back(static_cast<std::uint32_t>(texture));}
namespace BufferedDraw{
enum class Primitive{Quads};
void Color3f(float a,float b,float c){Require(a==1 && b==1 && c==1);events.push_back(3);}
void Begin(Primitive p){Require(p==Primitive::Quads);events.push_back(5);}
void End(){events.push_back(8);}
void TexCoord2f(float u,float v){events.insert(events.end(),{6,Bits(u),Bits(v)});}
void Vertex3f(float a,float b,float c){events.insert(events.end(),{7,Bits(a),Bits(b),Bits(c)});}
}
'''
MAIN=r'''
void Put(BYTE* p,int value){std::memcpy(p,&value,4);}
void Hash(std::uint64_t& h,std::uint32_t value){for(unsigned b=0;b<4;++b){h^=(value>>(b*8))&255;h*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory,BYTE* storage){
 object=memory+16;scratch=storage+16;
 const int grids[][2]={{-1,3},{3,-1},{0,0},{1,4},{4,1},{2,2},{2,3},{3,2},{3,3},{7,5}};
 const std::uint32_t textures[]={0,1,0x7fffffff,0x80000000u,0xffffffffu,0x3f800000,0x7fc01234,0x80000001u};
 std::uint64_t hash=14695981039346656037ull;unsigned queries=0;
 for(unsigned pattern=0;pattern<32;++pattern)for(auto& grid:grids)for(unsigned mode=0;mode<4;++mode)for(unsigned failed=0;failed<2;++failed){
  std::memset(memory,0xa5,128);std::memset(storage,0x6b,1024);std::memset(nodes-16,0x5c,4096);
  const int cols=grid[0],rows=grid[1],total=cols>=2 && rows>=2?cols*rows:4;
  const int capacity=mode==0?-1:mode==1?0:total+(mode==3?2:0);count=capacity>0?unsigned(capacity):0;
  const unsigned flags=(pattern%4)*0x1000u|((pattern&1)?0x80u:0);
  Put(object+40,cols);Put(object+44,rows);Put(object+48,capacity);Put(object+52,static_cast<int>(reinterpret_cast<std::uintptr_t>(nodes)));Put(object+20,static_cast<int>(flags));
  const auto front=textures[pattern%8],back=textures[(pattern+3)%8];std::memcpy(object+12,&front,4);std::memcpy(object+16,&back,4);
  std::vector<float> vertices(total*3);
  for(int i=0;i<total;++i)for(unsigned axis=0;axis<3;++axis){
   const float value=static_cast<float>(int(pattern)*3-i*7+int(axis))* .125f;
   std::memcpy(nodes+60*i+28+axis*4,&value,4);vertices[i*3+axis]=value+(axis==2?(i%2?.25f:-.25f):0);
  }
  const std::vector<BYTE> beforeObject(memory,memory+128),beforeNodes(nodes-16,nodes-16+4096);
  std::vector<BYTE> expectedStorage(storage,storage+1024);std::vector<std::uint32_t> expected;
  const bool valid=cols>=2 && rows>=2 && capacity>0;
  const bool drawing=valid && !failed;
  if(valid){expected.insert(expected.end(),{10,12*count});}
  if(drawing){
   for(int i=0;i<total;++i){expected.insert(expected.end(),{11,unsigned(i),12,unsigned(i)});}
   std::memcpy(expectedStorage.data()+16,vertices.data(),total*12);
   if((flags&0x3000u)==0)expected.push_back(1);else if((flags&0x3000u)==0x1000)expected.push_back(2);
   expected.push_back(3);
   auto emit=[&](int col,int row){
    const float u=static_cast<float>(double(col)/double(cols-1));
    float v=static_cast<float>(double(row)/double(rows-1));if(row==rows-1)v=.99f;
    const unsigned index=3*(row*cols+col);
    expected.insert(expected.end(),{6,Bits(u),Bits(v),7,Bits(vertices[index]),Bits(vertices[index+1]),Bits(vertices[index+2])});
   };
   for(unsigned face=0;face<2;++face){
    expected.insert(expected.end(),{4,face?back:front,5});
    for(int row=0;row<rows-1;++row)for(int col=0;col<cols-1;++col){
     emit(col,row);
     if(!face){emit(col+1,row);emit(col+1,row+1);}else{emit(col,row+1);emit(col+1,row+1);}
     if(!face)emit(col,row+1);else emit(col+1,row);
    }
    expected.push_back(8);
   }
   expected.push_back(9);
  }
  failAllocation=failed!=0;allocations=deletions=visited=0;events.clear();FUN_00408ff0(object);
  Require(events==expected && allocations==unsigned(valid) && deletions==unsigned(drawing) && visited==(drawing?unsigned(total):0) && std::memcmp(memory,beforeObject.data(),128)==0 && std::memcmp(nodes-16,beforeNodes.data(),4096)==0 && std::memcmp(storage,expectedStorage.data(),1024)==0);
  for(auto value:events)Hash(hash,value);++queries;
 }
 Require(queries==2560);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);nodes=low+16400;
 const auto expected=Run(low,low+4096);
 if(sizeof(void*)>4){
  auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);
  for(unsigned mask=1;mask<4;++mask){
   std::memset(low,0x7b,8192);Require(Run(mask&1?high:low,(mask&2?high:low)+4096)==expected);
   if(mask&1)for(unsigned i=0;i<128;++i)Require(low[i]==0x7b);
   if(mask&2)for(unsigned i=0;i<1024;++i)Require(low[4096+i]==0x7b);
  }
  std::puts("objeto/buffer altos; nodos32 bajos, dos caras, UV y aliases intactos");
 }
 std::printf("Cloth vertices: 2560 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''
def harness(revision=None):
    source=read_source('src/Net/Crypto.cpp',revision)
    getter=function(read_source('src/Physics/Cloth_Simulation.cpp',revision),'SpringNode_GetPos').replace('SpringNode_GetPos','ActualGetPos')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('GET_POS',getter)+function(source,'FUN_004091d0')+function(source,'FUN_004090b0')+function(source,'FUN_00408ff0')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-cloth-vertices-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
