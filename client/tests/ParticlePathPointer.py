#!/usr/bin/env python3
"""Vecindad real de partículas; giro controlado y tabla444 sin punteros leídos."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <cmath>
struct OBJECT;
using longlong=std::int64_t;
#define SQRT std::sqrt
unsigned mathCalls,clampCalls;
float expectedX,expectedY,expectedMeanX,expectedMeanY;
int expectedAngle,expectedStep,resultAngle;
int Math_GetAngleFromPoints(float x1,float y1,float x2,float y2){Require(mathCalls==0 && x1==expectedX && y1==expectedY && x2==expectedMeanX && y2==expectedMeanY);++mathCalls;return 170;}
int Angle_Clamp(int angle,int target,int step){Require(mathCalls==1 && clampCalls==0 && angle==expectedAngle && target==170 && step==expectedStep);++clampCalls;return resultAngle;}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
void Update(BYTE* object,int index,BYTE* table,int count){Particle_PathUpdate(OBJECT_ARG,index,TABLE_ARG,count);}
std::uint64_t Run(BYTE* objectMemory,BYTE* tableMemory){
    std::uint64_t digest=14695981039346656037ull;
    const float distances[]={1,50,79,80,81,399,400,401,200};const float angles[]={-360.75f,-90.5f,-0.0f,0.0f,22.5f,90.75f,359.5f};const float steps[]={-4.75f,0.0f,1.5f,9.99f,18.0f};
    constexpr unsigned tableBytes=40*444+32;std::vector<BYTE> beforeTable(tableBytes);
    for(unsigned trial=0;trial<32768;++trial){
        std::memset(objectMemory,0x5a,476);std::memset(tableMemory,0xa5,tableBytes);BYTE *object=objectMemory+16,*table=tableMemory+16;
        expectedX=static_cast<float>(static_cast<int>(trial%17)-8)*0.5f;expectedY=-expectedX;Put(object+16,expectedX);Put(object+20,expectedY);Put(object+36,angles[trial%7]);Put(object+216,steps[(trial/7)%5]);
        expectedAngle=static_cast<int>(angles[trial%7]);expectedStep=static_cast<int>(steps[(trial/7)%5]);resultAngle=static_cast<int>(trial%721)-360;
        const int count=static_cast<int>(trial%42)-1,index=(trial/42)%4==0?-1:(trial/42)%4==1?0:(trial/42)%4==2?count-1:count;
        float sumX=0,sumY=0;unsigned neighbours=0;
        for(int i=0;i<40;++i){
            const float distance=distances[(trial+i*7)%9];const int axis=i%4;const float ax=axis==0?1.0f:axis==1?-1.0f:0.0f,ay=axis==2?1.0f:axis==3?-1.0f:0.0f;
            const float x=expectedX+ax*distance,y=expectedY+ay*distance;
            const float vx=axis==0?0.0f:axis==1?1.0f:axis==2?0.0f:-1.0f,vy=axis==0?1.0f:axis==1?0.0f:axis==2?-1.0f:0.0f;
            BYTE* row=table+i*444;row[0]=trial%17==0?0:(trial/3+i)%4==0?0:(trial/3+i)%4==1?1:(trial/3+i)%4==2?128:255;
            Put(row+16,x);Put(row+20,y);Put(row+192,(expectedX+x+vx)*0.5f);Put(row+196,(expectedY+y+vy)*0.5f);
            if(i<count && row[0] && i!=index && distance<400){++neighbours;sumX+=distance>=80?vx:-ax;sumY+=distance>=80?vy:-ay;}
        }
        expectedMeanX=expectedX+(neighbours?sumX/static_cast<float>(neighbours):0);expectedMeanY=expectedY+(neighbours?sumY/static_cast<float>(neighbours):0);
        BYTE expected[476];std::memcpy(expected,objectMemory,476);std::memcpy(beforeTable.data(),tableMemory,tableBytes);if(neighbours){const float angle=static_cast<float>(resultAngle);std::memcpy(expected+16+36,&angle,4);}
        mathCalls=clampCalls=0;Update(trial%17==0?nullptr:object,index,count<=0?nullptr:table,count);
        Require(mathCalls==(neighbours?1u:0u) && clampCalls==mathCalls && std::memcmp(objectMemory,expected,476)==0 && std::memcmp(tableMemory,beforeTable.data(),tableBytes)==0);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}digest^=mathCalls;digest*=1099511628211ull;
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,low+0x8000);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);
        for(unsigned mask=1;mask<4;++mask){std::memset(low,0x7b,0x10000);Require(Run(mask&1?high:low,(mask&2?high:low)+0x8000)==expected);
            if(mask&1)for(unsigned i=0;i<476;++i)Require(low[i]==0x7b);if(mask&2)for(unsigned i=0;i<40*444+32;++i)Require(low[0x8000+i]==0x7b);
        }std::puts("cuatro combinaciones objeto/tabla altos y bajos; aliases intactos");
    }std::printf("Particle path: 32768 casos, vecindad/giro/readonly %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    code=function(read_source('src/Render/Particle.cpp',revision),'Particle_PathUpdate')
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES
    gh,gd=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in sorted(set(re.findall(r'_DAT_[0-9a-f]+',code))):
        prefix+=re.search(rf'extern [^;\n]*\b{name}\s*;',gh).group();prefix+=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',gd,re.M).group()
    current='OBJECT* object' in code
    obj='reinterpret_cast<OBJECT*>(object)' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(object))';table='table' if current else 'static_cast<int>(reinterpret_cast<uintptr_t>(table))'
    return prefix+code+MAIN.replace('OBJECT_ARG',obj).replace('TABLE_ARG',table)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-particle-path-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
