#!/usr/bin/env python3
"""Pool de cien sprites y RenderPlane activos: tipo signed y dirección nativa."""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
DWORD* DAT_07c74f54;BYTE* pool;bool materialResult;unsigned cursor,currentSlot,completed,vertex,depth;std::vector<std::uint32_t> events;
#define Ff(sym) (*(float*)&(sym))
std::uint32_t Bits(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}float GetFloat(unsigned offset){float value;std::memcpy(&value,pool+currentSlot*444+offset,4);return value;}int GetType(){short value;std::memcpy(&value,pool+currentSlot*444+2,2);return value;}void Value(float value){events.push_back(Bits(value));}
namespace Materials{enum class Preset{Add};struct Texture{int slot;};struct State{Preset preset;};struct TexturedMaterial{Texture texture;State state;};State Describe(Preset preset){return {preset};}bool ApplyTextured(const TexturedMaterial& material){Require(material.texture.slot==GetType() && material.state.preset==Preset::Add);events.push_back(2);events.push_back(material.texture.slot);return materialResult;}}
void GL_BindTextureSlot(int type){Require(!materialResult && type==GetType());events.push_back(3);events.push_back(type);}void GL_SetBlendAdditive(){Require(!materialResult);events.push_back(4);}void GL_ResetState(){Require(!depth && vertex==4);events.push_back(13);++completed;}
namespace Gfx{
void Color3f(float r,float g,float b){Require(completed==cursor || !depth);while(cursor<100 && !pool[cursor*444])++cursor;Require(cursor<100 && !depth);currentSlot=cursor++;Require(Bits(r)==Bits(GetFloat(232)) && Bits(g)==Bits(GetFloat(236)) && Bits(b)==Bits(GetFloat(240)));events.push_back(1);Value(r);Value(g);Value(b);}
void PushMatrix(){Require(!depth++);vertex=0;events.push_back(5);}void PopMatrix(){Require(depth--==1 && vertex==4);events.push_back(12);}void Translate(float x,float y,float z){Require(depth==1 && Bits(x)==Bits(GetFloat(16)) && Bits(y)==Bits(GetFloat(20)) && Bits(z)==Bits(GetFloat(24)));events.push_back(6);Value(x);Value(y);Value(z);}void Rotate(float angle,float x,float y,float z){Require(depth==1 && Bits(angle)==Bits(GetFloat(12)) && x==0 && y==0 && z==1);events.push_back(7);Value(angle);}
}
namespace BufferedDraw{
enum class Primitive{Quads};void Begin(Primitive p){Require(depth==1 && p==Primitive::Quads);events.push_back(8);}void End(){Require(vertex==4);events.push_back(11);}void TexCoord2f(float u,float v){const float uv[4][2]={{0,1},{0,0},{1,0},{1,1}};Require(vertex<4 && u==uv[vertex][0] && v==uv[vertex][1]);events.push_back(9);Value(u);Value(v);}void Vertex3fv(const float* p){Require(vertex<4 && depth==1);const int signs[4][2]={{-1,-1},{-1,1},{1,1},{1,-1}};const float scale=GetFloat(36);const float x=signs[vertex][0]<0?-scale:scale,y=signs[vertex][1]<0?-scale:scale;Require(Bits(p[0])==Bits(x) && Bits(p[1])==Bits(y) && p[2]==0);events.push_back(10);for(unsigned i=0;i<3;++i)Value(p[i]);++vertex;}
}
'''
MAIN=r'''
void Put32(BYTE* p,std::uint32_t bits){std::memcpy(p,&bits,4);}void PutFloat(BYTE* p,float value){std::memcpy(p,&value,4);}
std::uint64_t Run(BYTE* memory){pool=memory+16;DAT_07c74f54=reinterpret_cast<DWORD*>(pool+236);std::uint64_t hash=14695981039346656037ull;const short types[]={-32768,-1,0,1,494,1023,32767};const std::uint32_t colors[]={0,0x80000000u,0x3f800000,0xbf800000u,0x3f000000,0x7fc12345,0xffc54321u};const float scales[]={0,-0.0f,.25f,-1,1,2},angles[]={0,-0.0f,-360,45,90,720};unsigned total=0;
 for(unsigned frame=0;frame<1024;++frame){std::memset(memory,0xa5,44432);unsigned active=0;for(unsigned slot=0;slot<100;++slot){BYTE* record=pool+slot*444;record[0]=frame%16==0?0:frame%16==1?1:static_cast<BYTE>((frame+slot)%4==0?0:(frame+slot)%4==1?1:(frame+slot)%4==2?0x80:0xff);if(record[0])++active;const short type=types[(frame+slot)%7];std::memcpy(record+2,&type,2);PutFloat(record+12,angles[(frame+slot)%6]);PutFloat(record+36,scales[(frame/7+slot)%6]);for(unsigned axis=0;axis<3;++axis){PutFloat(record+16+axis*4,float(int((frame+slot*3+axis)%257)-128)*.25f);Put32(record+232+axis*4,colors[(frame+slot+axis)%7]);}}
  BYTE before[44432];std::memcpy(before,memory,44432);materialResult=frame%2;cursor=currentSlot=completed=vertex=depth=0;events.clear();Player_Render();Require(completed==active && !depth && std::memcmp(memory,before,44432)==0);if(!active)Require(events.empty());for(auto value:events)for(unsigned i=0;i<4;++i){hash^=(value>>(i*8))&255;hash*=1099511628211ull;}total+=completed;
 }Require(total==73600);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,44432);Require(Run(high)==expected);for(unsigned i=0;i<44432;++i)Require(low[i]==0x7b);std::puts("pool alto; tipo signed, quad/UV/color y alias bajo intactos");}std::printf("Player pool: 1024 frames/73600 draws %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    plane=function(read_source('src/Render/Render_LegacyBillboards.cpp',revision),'RenderPlane')
    player=function(read_source('src/Render/Player_Render.cpp',revision),'Player_Render')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+plane+player+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-player-pool-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
