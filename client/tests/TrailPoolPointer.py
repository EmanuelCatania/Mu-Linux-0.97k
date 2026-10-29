#!/usr/bin/env python3
"""Recorrido real de cien estelas con pool alto; owners32 y render controlado."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
BYTE* pool;
#define DAT_07c608b4 (*reinterpret_cast<DWORD*>(pool+12))
float _DAT_0055256c=1.0f;
std::vector<std::uint32_t> events;
std::uint32_t Bits(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
void GL_SetBlendAdditive(){events.push_back(1);}void GL_SetBlendSrcAlpha(){events.push_back(2);}void GL_BindTextureSlot(int texture){events.push_back(3);events.push_back(texture);}
namespace BufferedDraw{
enum class Primitive{TriangleFan};
void Begin(Primitive p){Require(p==Primitive::TriangleFan);events.push_back(4);}
void End(){events.push_back(5);}
void Color3f(float r,float g,float b){events.push_back(6);events.push_back(Bits(r));events.push_back(Bits(g));events.push_back(Bits(b));}
void TexCoord2f(float u,float v){events.push_back(7);events.push_back(Bits(u));events.push_back(Bits(v));}
void Vertex3fv(float* p){Require(reinterpret_cast<BYTE*>(p)>=pool && reinterpret_cast<BYTE*>(p)+12<=pool+75200);events.push_back(8);for(unsigned i=0;i<3;++i)events.push_back(Bits(p[i]));}
}
'''
MAIN=r'''
void PutInt(BYTE* p,int value){std::memcpy(p,&value,4);}void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}
void Color(std::vector<std::uint32_t>& out,float alpha,const int* colors){out.push_back(6);for(unsigned i=0;i<3;++i)out.push_back(Bits(alpha*static_cast<float>(colors[i])));}
void UV(std::vector<std::uint32_t>& out,float u,float v){out.push_back(7);out.push_back(Bits(u));out.push_back(Bits(v));}
void Vertex(std::vector<std::uint32_t>& out,const BYTE* p){out.push_back(8);for(unsigned i=0;i<3;++i){std::uint32_t bits;std::memcpy(&bits,p+i*4,4);out.push_back(bits);}}
std::uint64_t Run(BYTE* memory,BYTE* owners){pool=memory+16;std::uint64_t hash=14695981039346656037ull;const int counts[]={-1,0,1,2,3,7,15,30};const int modes[]={-2,0,1,2,3,4,5,8};
 for(unsigned frame=0;frame<512;++frame){std::memset(memory,0xa5,75232);std::memset(owners,0x6d,51200);std::vector<std::uint32_t> expected;
  for(unsigned slot=0;slot<100;++slot){BYTE* record=pool+slot*752;BYTE* owner=owners+slot*512;const short kind=static_cast<short>((frame+slot)%3==0?0:((frame+slot)%3==1?1:-1));std::memcpy(owner+446,&kind,2);PutInt(record+12,static_cast<int>(reinterpret_cast<std::uintptr_t>(owner)));
   const int mode=modes[(frame+slot)%8],segments=counts[(frame/8+slot)%8];record[0]=static_cast<BYTE>((frame+slot)%4==0?0:((frame+slot)%4==1?1:0x80));PutInt(record+4,mode);PutInt(record+16,segments);const int colors[]={static_cast<int>((frame+slot)%17)-8,static_cast<int>((frame*3+slot)%13),static_cast<int>(slot%5)-2};for(unsigned i=0;i<3;++i)PutInt(record+20+4*i,colors[i]);
   for(unsigned vertex=0;vertex<30;++vertex)for(unsigned axis=0;axis<3;++axis){PutFloat(record+32+vertex*12+axis*4,static_cast<float>(int(frame)-int(slot)+int(vertex*3+axis))*.25f);PutFloat(record+392+vertex*12+axis*4,static_cast<float>(int(slot)-int(frame)-int(vertex*3+axis))*.125f);}
   if(!record[0])continue;expected.push_back(kind==0 && mode<3?1:2);if(segments<=1)continue;expected.push_back(3);expected.push_back((mode>2?mode-3:mode)+0x48d);
   for(int segment=0;segment<segments-1;++segment){expected.push_back(4);Color(expected,kind==0?static_cast<float>(segments-segment)/segments:1.0f,colors);UV(expected,static_cast<float>(segment)/segments,1);Vertex(expected,record+32+segment*12);UV(expected,static_cast<float>(segment)/segments,0);Vertex(expected,record+392+segment*12);Color(expected,kind==0?static_cast<float>(segments-segment-1)/segments:1.0f,colors);UV(expected,static_cast<float>(segment+1)/segments,0);Vertex(expected,record+392+(segment+1)*12);UV(expected,static_cast<float>(segment+1)/segments,1);Vertex(expected,record+32+(segment+1)*12);expected.push_back(5);}
  }
  std::vector<BYTE> before(memory,memory+75232),ownersBefore(owners,owners+51200);events.clear();Particle_Update();Require(events==expected && std::memcmp(memory,before.data(),75232)==0 && std::memcmp(owners,ownersBefore.data(),51200)==0);for(auto value:events)for(unsigned byte=0;byte<4;++byte){hash^=(value>>(byte*8))&255;hash*=1099511628211ull;}
 }return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);BYTE* owners=low+100000;const auto expected=Run(low,owners);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,75232);Require(Run(high,owners)==expected);for(unsigned i=0;i<75232;++i)Require(low[i]==0x7b);std::puts("pool alto; owners32 bajos y alias del pool intacto");}std::printf("Trail pool: 512 frames/51200 slots %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    source=read_source('src/Render/Render_LegacyTrails.cpp',revision)
    return PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x40000')+SERVICES+function(source,'Particle_Update')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-trail-pool-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
