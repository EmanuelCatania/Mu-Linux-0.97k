#!/usr/bin/env python3
"""Los destellos efímeros no adquieren identidad por ordinal entre pasos."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def read_source(path, revision):
    if revision:
        return subprocess.check_output(['git','show',f'{revision}:{path}'],cwd=ROOT,text=True,encoding='utf-8')
    return (ROOT/path).read_text(encoding='utf-8')

def build_and_run(directory, source, args):
    directory.mkdir()
    (directory/'main.cpp').write_text(source,encoding='utf-8')
    (directory/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.25)
project(EquipmentSpriteReplay LANGUAGES CXX)
add_executable(replay main.cpp)
target_compile_features(replay PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(replay PRIVATE /fp:precise /utf-8)
else()
  target_compile_options(replay PRIVATE -fno-strict-aliasing)
endif()
''',encoding='utf-8')
    command=['cmake','-S',str(directory),'-B',str(directory/'build')]
    if args.generator:
        command+=['-G',args.generator]
    if os.name=='nt':
        command+=['-A',args.architecture,'-T','v143']
    subprocess.run(command,check=True)
    subprocess.run(['cmake','--build',str(directory/'build'),'--config','Release'],check=True)
    binary=directory/'build'/'Release'/'replay.exe' if os.name=='nt' else directory/'build'/'replay'
    return subprocess.check_output([str(binary)],text=True)

def harness(revision, coupled_only=False):
    source = read_source('src/Render/EquipmentVisuals.cpp', revision)
    draw = source[source.index('void Equipment_Draw('):]
    start = draw.index('        if (!command.mesh) {')
    end = draw.index('        const Mesh &mesh', start)
    block = draw[start:end]
    source = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
struct Sprite { unsigned short type; float position[3],scale,light[3],rotation; int owner,mode,stage; };
struct Command { bool mesh; unsigned index; int stage; };
struct Frame { std::vector<Sprite> sprites; std::vector<Command> commands; };
float displacement[3], emitted[3]; unsigned calls;
bool known;
float RenderBlend(float a,float b,float t) { return a+(b-a)*t; }
bool RenderInterpolation_OwnerOffset(const void*,float* out) { if(known) std::memcpy(out,displacement,12); return known; }
void CreateSprite(unsigned short,float* p,float,float*,int,float,int) { std::memcpy(emitted,p,12); ++calls; }
void Replay(Frame& frame,const Frame& previous,float alpha,bool continuous,const void* entity) {
    for(const Command& command:frame.commands) {
''' + block + r'''
    }
}
int main() {
    // Dos tiradas posibles de los diez emisores de 0x236: igual cantidad,
    // mismo bitmap/dueño/etapa, pero sobreviven huesos/puntos distintos.
    Frame previous,frame;
    for(int point: {0,2}) { Sprite s{}; s.type=1150;s.owner=7;s.position[1]=60.f-20.f*point;previous.sprites.push_back(s);previous.commands.push_back({false,(unsigned)previous.commands.size(),0}); }
    for(int point: {1,3}) { Sprite s{}; s.type=1150;s.owner=7;s.position[1]=60.f-20.f*point;frame.sprites.push_back(s);frame.commands.push_back({false,(unsigned)frame.commands.size(),0}); }
    unsigned cases=0;
    for(int moving=0;moving<2;++moving) for(int valid=0;valid<2;++valid) for(int step=FIRST_STEP;step<=40;++step) {
        const float remaining=1.f-step/40.f;
        known=valid!=0;displacement[0]=moving?-8.f*remaining:0.f;displacement[1]=moving?4.f*remaining:0.f;displacement[2]=moving?2.f*remaining:0.f;
        const Frame before=frame;calls=0;
        Replay(frame,previous,step/40.f,true,&frame);
        if(calls!=2) return 1;
        for(int axis=0;axis<3;++axis) {
            const float expected=frame.sprites.back().position[axis]+(known?displacement[axis]:0.f);
            if(emitted[axis]!=expected) { std::fprintf(stderr,"Cruce entre emisiones: paso=%d eje=%d recibido=%g esperado=%g\n",step,axis,emitted[axis],expected); return 1; }
        }
        if(std::memcmp(before.sprites.data(),frame.sprites.data(),frame.sprites.size()*sizeof(Sprite))) return 1;
        ++cases;
    }
    std::printf("EquipmentSpriteReplay: %u casos OK\n",cases);
}
'''
    return source.replace('FIRST_STEP', '40' if coupled_only else '0')

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator',default=None)
    parser.add_argument('--reference-ref',default=None)
    parser.add_argument('--coupled-only',action='store_true')
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-equipment-replay-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args.reference_ref,args.coupled_only),args),end='')

if __name__=='__main__':
    main()
