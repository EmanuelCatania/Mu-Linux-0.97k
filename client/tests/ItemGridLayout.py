#!/usr/bin/env python3
"""Contrato ITEM y productores reales de grilla, con oráculo por bytes."""
import argparse
from pathlib import Path
import tempfile
from SceneObjectPointer import build_and_run,function,read_source

def harness(revision=None):
    structs=read_source('src/structs.h',revision)
    if 'struct ITEM  //' in structs:
        item=structs[structs.index('struct ITEM  //'):structs.index('// Acceso tipado a un item')]
    else:
        item=read_source('src/Item/ItemTypes.h',revision)
    preamble=r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;
#define MAX_SPECIAL_OPTION 9
#ifndef _WIN32
#define __cdecl
#endif
'''
    services=r'''
ITEM* AsItem(void* p){return static_cast<ITEM*>(p);}
struct ITEM_ATTRIBUTE { BYTE Width,Height; };
ITEM_ATTRIBUTE attributes[512];uintptr_t DAT_07d78068;
alignas(ITEM) char warehouse[240*68];
#define DAT_07ea5b30 warehouse[0]
unsigned conversions; int lastLevel,lastOption;
void ItemConvert(ITEM* p,int level,int option) {
    ++conversions;lastLevel=level;lastOption=option;
    p->Level=level;
}
void Require(bool ok){if(!ok){std::fprintf(stderr,"Contrato de grilla incumplido\n");std::exit(1);}}
'''
    main=r'''
int main(){
    static_assert(sizeof(ITEM)==68 && alignof(ITEM)==4);
    static_assert(sizeof(ITEM::Type)==2 && sizeof(ITEM::Level)==4 && sizeof(ITEM::Key)==4);
    static_assert(sizeof(ITEM::Special)==9 && sizeof(ITEM::SpecialValue)==9);
    DAT_07d78068=reinterpret_cast<uintptr_t>(attributes);
    if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(warehouse)>0xffffffffull);
    std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
    auto Hash=[&](const void* p,std::size_t n){auto b=static_cast<const BYTE*>(p);while(n--){hash^=*b++;hash*=1099511628211ull;}};
    BYTE expected[sizeof(DAT_07ea5b30)];
    for(unsigned width=0;width<=4;++width)for(unsigned height=0;height<=4;++height)
    for(unsigned origin: {0u,1u,7u,8u,25u,120u})for(unsigned variant=0;variant<4;++variant){
        if(origin%8+width>8)continue;
        BYTE packet[5]={static_cast<BYTE>(variant==3?255:12),static_cast<BYTE>(variant*67),static_cast<BYTE>(variant*85),static_cast<BYTE>(variant*17),0};
        const unsigned type=packet[0];attributes[type]={static_cast<BYTE>(width),static_cast<BYTE>(height)};
        std::memset(DAT_07ea5b30,0xa5,sizeof(DAT_07ea5b30));std::memcpy(expected,DAT_07ea5b30,sizeof(expected));conversions=0;
        if(type!=255)for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){
            BYTE* p=expected+(origin+y*8+x)*68;const short id=static_cast<short>(type);const int level=packet[1];const DWORD key=x==0&&y==0?1:0;
            std::memcpy(p,&id,2);std::memcpy(p+4,&level,4);p[26]=packet[2];p[27]=packet[3];std::memcpy(p+56,&key,4);p[62]=origin%8;p[63]=origin/8;
        }
        InsertWarehouseItem(origin,packet);
        Require(std::memcmp(expected,DAT_07ea5b30,sizeof(expected))==0);
        Require(conversions==(type==255?0:width*height));
        if(conversions)Require(lastLevel==packet[1]&&lastOption==packet[3]);
        Hash(DAT_07ea5b30,sizeof(DAT_07ea5b30));++cases;
        // El consumidor de flags sólo puede escribir Color en el rectángulo.
        alignas(ITEM) BYTE grid[240*68];std::memset(grid,0x5a,sizeof(grid));std::memcpy(expected,grid,sizeof(grid));
        const short id=12;std::memcpy(grid+origin*68,&id,2);grid[origin*68+62]=origin%8;grid[origin*68+63]=origin/8;
        std::memcpy(expected,grid,sizeof(grid));attributes[12]={static_cast<BYTE>(width),static_cast<BYTE>(height)};
        for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x)expected[(origin+y*8+x)*68+64]=packet[2];
        SetItemGridFlag(static_cast<int>(origin),grid,packet[2]);
        Require(std::memcmp(expected,grid,sizeof(grid))==0);Hash(grid,sizeof(grid));++cases;
    }
    std::printf("ItemGridLayout: %u casos, %016llx\n",cases,(unsigned long long)hash);
}
'''
    source=read_source('src/Item/Item_InventoryGrid.cpp',revision)
    return preamble+item+services+function(source,'InsertWarehouseItem')+function(source,'SetItemGridFlag')+main.replace('DAT_07ea5b30','warehouse')

def main():
    p=argparse.ArgumentParser();p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-item-grid-') as temp:
        root=Path(temp);result=build_and_run(root/'current',harness(),args);print(result,end='')
        if args.reference_ref:
            reference=build_and_run(root/'reference',harness(args.reference_ref),args)
            if result!=reference:raise RuntimeError('Diferencia con referencia de grilla')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
