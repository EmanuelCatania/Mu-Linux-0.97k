#!/usr/bin/env python3
"""La confirmación de movimiento debe publicar el color normal antes del draw."""
import argparse
from pathlib import Path
import tempfile
from SceneObjectPointer import build_and_run,function,read_source

def harness(revision=None):
    header=read_source('src/Item/ItemTypes.h')
    services=r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using BYTE=unsigned char;
#ifndef _WIN32
#define __cdecl
#endif
ITEM* AsItem(void* p){return static_cast<ITEM*>(p);}
namespace Gfx {
float color[4];
void Color3f(float r,float g,float b){color[0]=r;color[1]=g;color[2]=b;color[3]=1;}
void Color4f(float r,float g,float b,float a){Color3f(r,g,b);color[3]=a;}
}
void Require(bool ok){if(!ok){std::fputs("Destello de grilla o bytes alterados\n",stderr);std::exit(1);}}
'''
    main=r'''
int main(){
    const short types[]={-32768,-1,0,12,512,8191};
    const BYTE flags[]={0,1,2,3,4,99,255};
    alignas(ITEM) static BYTE memory[120*sizeof(ITEM)+32];
    if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(memory)>0xffffffffull);
    BYTE* pool=memory+16;unsigned cases=0;
    ItemMove_ClearPoolPreview(nullptr,120);
    for(int count:{0,32,64,120})for(BYTE flag:flags)for(int shift=0;shift<6;++shift){
        std::memset(memory,0xa5,sizeof(memory));
        for(int i=0;i<count;++i){ITEM& item=AsItem(pool)[i];item.Type=types[(i+shift)%6];item.Color=flag;}
        BYTE expected[sizeof(memory)];std::memcpy(expected,memory,sizeof(memory));
        for(int i=0;i<count;++i)expected[16+i*sizeof(ITEM)+64]=AsItem(pool)[i].Type!=-1?1:0;
        // Entre el ack y el siguiente tick puede haber varios dibujos.
        ItemMove_ClearPoolPreview(pool,count);
        Require(std::memcmp(expected,memory,sizeof(memory))==0);
        for(int frame=0;frame<5;++frame)for(int i=0;i<count;++i){
            InventoryColor(&AsItem(pool)[i]);
            const float normal=AsItem(pool)[i].Type!=-1?.8f:1.f;
            Require(Gfx::color[0]==normal&&Gfx::color[1]==normal&&Gfx::color[2]==normal&&Gfx::color[3]==1);
        }
        // Referencia IDA sub_4E6550: el siguiente tick publica estos colores.
        for(int i=0;i<count;++i)AsItem(pool)[i].Color=AsItem(pool)[i].Type!=-1?1:0;
        Require(std::memcmp(expected,memory,sizeof(memory))==0);
        // Una segunda confirmación tampoco debe producir un salto.
        ItemMove_ClearPoolPreview(pool,count);
        Require(std::memcmp(expected,memory,sizeof(memory))==0);++cases;
    }
    std::printf("ItemMovePreviewColor: %u secuencias ack/draw/tick sin destello\n",cases);
}
'''
    return header+services+function(read_source('src/Net/Recv/Recv_Item.cpp',revision),'ItemMove_ClearPoolPreview')+function(read_source('src/Item/Item_InventoryRender.cpp'),'InventoryColor')+main

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-item-preview-') as temp:
        print(build_and_run(Path(temp)/'probe',harness(args.reference_ref),args),end='')
if __name__=='__main__':main()
