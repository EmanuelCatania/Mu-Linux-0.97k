#!/usr/bin/env python3
"""Preparación real de cámara: Hero nativo y orden de servicios controlados."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
// Vista escalar mínima del fixture; no construye OBJECT nativo de producción.
struct OBJECT{BYTE prefix[16];float Position[3];};static_assert(offsetof(OBJECT,Position)==16);
ACCESSORS
char* DAT_07abf5d8;int SceneFlag;bool CameraTopViewEnabled,wide,antilag,move;unsigned frame;int screenWidth,depth;DWORD DAT_083a427c,DAT_083a4278;float ray[3];
#define DAT_083a4110 ray[0]
constexpr int ANTILAG_INTERFACE=1;std::vector<DWORD> events;
DWORD Bits(float value){DWORD bits;std::memcpy(&bits,&value,4);return bits;}void Value(float value){events.push_back(Bits(value));}
DWORD PositionBits(unsigned axis){const DWORD values[]={0,0x80000000u,0x3f800000,0xbf800000u,0x7fc12345,0x7f800000};return values[(frame+axis+(move?2:0))%6];}
void MoveMainCamera(){Require(SceneFlag==5 && !depth);events.push_back(1);if(move && DAT_07abf5d8)for(unsigned i=0;i<3;++i){const DWORD bits=PositionBits(i);std::memcpy(DAT_07abf5d8+16+i*4,&bits,4);}}
struct Window{bool IsWide(){events.push_back(2);return wide;}float ViewportFraction(int width){events.push_back(5);events.push_back(width);return float(width)*.125f;}}gWindow;
int GetScreenWidth(){events.push_back(3);return screenWidth;}
struct Settings{bool GetAntilag(int id){Require(id==ANTILAG_INTERFACE);events.push_back(8);return antilag;}}gUserSettings;
void GL_BeginViewport(int x,int y,int w,int h){Require(!depth++);events.push_back(4);events.push_back(x);events.push_back(y);events.push_back(w);events.push_back(h);}void GL_EndOpenGL(){Require(depth--==1);events.push_back(7);}
void Camera_SetupFrustum(float fraction,float* origin){Require(depth==1 && fraction==float(wide?640:screenWidth)*.125f);events.push_back(6);Value(fraction);for(unsigned i=0;i<3;++i){DWORD expected=0;if(DAT_07abf5d8)std::memcpy(&expected,DAT_07abf5d8+16+i*4,4);Require(Bits(origin[i])==expected);Value(origin[i]);}}
void Camera_BuildMouseRay(int x,int y,float* out){Require(depth==1 && out==ray && x==static_cast<int>(DAT_083a427c) && y==static_cast<int>(DAT_083a4278));events.push_back(9);events.push_back(static_cast<DWORD>(x));events.push_back(static_cast<DWORD>(y));for(unsigned i=0;i<3;++i)out[i]=float(frame+i)*.25f;}
void CaptureCamera(){Require(depth==1);for(unsigned i=0;i<3;++i)Require(ray[i]==float(frame+i)*.25f);events.push_back(10);}
'''
MAIN=r'''
void Viewport(std::vector<DWORD>& out,int x,int y,int w,int h){out.insert(out.end(),{4,static_cast<DWORD>(x),static_cast<DWORD>(y),static_cast<DWORD>(w),static_cast<DWORD>(h)});}
std::uint64_t Run(BYTE* memory){std::uint64_t hash=14695981039346656037ull;const int scenes[]={0,1,2,3,4,5,7,8},widths[]={640,800,1024,1920};unsigned queries=0;
 for(frame=0;frame<64;++frame)for(int scene:scenes)for(unsigned flags=0;flags<32;++flags){SceneFlag=scene;wide=(flags&1)!=0;CameraTopViewEnabled=(flags&2)!=0;antilag=(flags&4)!=0;const bool absent=(flags&8)!=0;move=(flags&16)!=0;screenWidth=widths[frame%4];std::memset(memory,0xa5,512);DAT_07abf5d8=absent?nullptr:reinterpret_cast<char*>(memory+16);for(unsigned axis=0;axis<3;++axis){const DWORD values[]={0,0x80000000u,0x3f800000,0xbf800000u,0x7fc12345,0x7f800000};const DWORD bits=values[(frame+axis)%6];std::memcpy(memory+32+axis*4,&bits,4);}BYTE expectedMemory[512];std::memcpy(expectedMemory,memory,512);if(scene==5 && move && !absent)for(unsigned axis=0;axis<3;++axis){const DWORD bits=PositionBits(axis);std::memcpy(expectedMemory+32+axis*4,&bits,4);}DAT_083a427c=frame*123456789u;DAT_083a4278=frame*987654321u;std::vector<DWORD> expected;
  if(scene==5){expected.push_back(1);expected.push_back(2);if(!wide)expected.push_back(3);const int width=wide?640:screenWidth;Viewport(expected,0,0,width,48);expected.push_back(5);expected.push_back(width);expected.push_back(6);expected.push_back(Bits(float(width)*.125f));for(unsigned axis=0;axis<3;++axis){DWORD bits=0;if(!absent)std::memcpy(&bits,expectedMemory+32+axis*4,4);expected.push_back(bits);}expected.push_back(7);if(!CameraTopViewEnabled){expected.push_back(8);if(!antilag)expected.push_back(2);}Viewport(expected,0,0,width,(CameraTopViewEnabled||antilag||wide)?480:432);}else if(scene==4)Viewport(expected,0,0,640,480);else Viewport(expected,0,80,640,320);
  expected.insert(expected.end(),{9,DAT_083a427c,DAT_083a4278,10,7});events.clear();depth=0;PrepareCamera();Require(events==expected && !depth && std::memcmp(memory,expectedMemory,512)==0);for(auto value:events)for(unsigned byte=0;byte<4;++byte){hash^=(value>>(byte*8))&255;hash*=1099511628211ull;}++queries;
 }Require(queries==16384);return hash;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high);std::memset(low,0x7b,512);Require(Run(high)==expected);for(unsigned i=0;i<512;++i)Require(low[i]==0x7b);std::puts("Hero alto; lectura tras movimiento, origen/orden/viewports y alias intactos");}std::printf("Camera Hero: 16384 consultas por ruta %016llx\n",static_cast<unsigned long long>(expected));}
'''

def harness(revision=None):
    header=read_source('src/Entity/EntityTypes.h',revision)
    accessors='\n'.join(re.findall(r'^inline (?:const )?OBJECT\* AsObject\([^\n]+',header,re.M))
    if len(accessors.splitlines())!=3:raise ValueError('Se esperaban tres overloads AsObject')
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES.replace('ACCESSORS',accessors)+function(read_source('src/Game/SimulationPreparation.cpp',revision),'PrepareCamera')+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64');parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-camera-hero-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
