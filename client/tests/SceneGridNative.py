#!/usr/bin/env python3
"""Enlazado real de escena con punteros nativos contra un modelo de listas independiente."""
import argparse
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, read_source
from SceneGridLifecycle import PREAMBLE, production
from ObjectLayoutContract import record


MAIN = r'''
SceneObjectCell g_ObjectBucketGrid[256];
constexpr unsigned CellBytes=8+2*sizeof(void*);
constexpr unsigned Prev=sizeof(void*)==4?436:444, Next=sizeof(void*)==4?440:452;
static unsigned char expectedNodes[16][sizeof(OBJECT)], expectedGrid[256*CellBytes];
static bool expectedRegistered[16],expectedFreed[16];
static std::vector<unsigned> lists[256], expectedEvents;
void* Pointer(unsigned id){return nodes[id];}
void NodeLink(unsigned id,unsigned offset,void* pointer){std::memcpy(expectedNodes[id]+offset,&pointer,sizeof(pointer));}
void CellLink(unsigned cell,unsigned offset,void* pointer){std::memcpy(expectedGrid+cell*CellBytes+offset,&pointer,sizeof(pointer));}
void ModelInsert(unsigned cell,unsigned id){
    std::memset(expectedNodes[id],0,sizeof(OBJECT));
    expectedRegistered[id]=true;expectedEvents.push_back(0x1000+id);
    if(lists[cell].empty())CellLink(cell,4,Pointer(id));
    else {unsigned previous=lists[cell].back();NodeLink(previous,Next,Pointer(id));NodeLink(id,Prev,Pointer(previous));}
    lists[cell].push_back(id);CellLink(cell,4+sizeof(void*),Pointer(id));
}
void ModelRemove(unsigned cell,unsigned id){
    auto& list=lists[cell];auto it=std::find(list.begin(),list.end(),id);Require(it!=list.end());
    const bool first=it==list.begin(),last=it+1==list.end();
    void* previous=first?nullptr:Pointer(*(it-1));void* next=last?nullptr:Pointer(*(it+1));
    if(!first)NodeLink(*(it-1),Next,next);else CellLink(cell,4,next);
    if(!last)NodeLink(*(it+1),Prev,previous);else CellLink(cell,4+sizeof(void*),previous);
    list.erase(it);expectedRegistered[id]=false;expectedFreed[id]=true;
    expectedEvents.push_back(0x3000+id);expectedEvents.push_back(0x2000+id);
}
void Check(){
    Require(std::memcmp(expectedNodes,nodes,sizeof(nodes))==0);
    Require(std::memcmp(expectedGrid,g_ObjectBucketGrid,sizeof(expectedGrid))==0);
    Require(std::memcmp(expectedRegistered,registered,sizeof(registered))==0);
    Require(std::memcmp(expectedFreed,freed,sizeof(freed))==0);
    Require(expectedEvents==events);
}
void Reset(){
    nextSlot=0;events.clear();expectedEvents.clear();
    std::memset(nodes,0xa5,sizeof(nodes));std::memset(expectedNodes,0xa5,sizeof(expectedNodes));
    std::memset(registered,0,sizeof(registered));std::memset(freed,0,sizeof(freed));
    std::memset(expectedRegistered,0,sizeof(expectedRegistered));std::memset(expectedFreed,0,sizeof(expectedFreed));
    std::memset(g_ObjectBucketGrid,0,sizeof(g_ObjectBucketGrid));std::memset(expectedGrid,0,sizeof(expectedGrid));
    std::memset(DAT_083a2370,0xa5,sizeof(DAT_083a2370));
    for(unsigned cell=0;cell<256;++cell){
        lists[cell].clear();
        const std::uint32_t reserved=0x12340000+cell;
        std::memcpy(expectedGrid+cell*CellBytes,&reserved,4);
        expectedGrid[cell*CellBytes+4+2*sizeof(void*)]=static_cast<BYTE>(255-cell);
        std::memset(expectedGrid+cell*CellBytes+5+2*sizeof(void*),0x5a,3);
    }
    std::memcpy(g_ObjectBucketGrid,expectedGrid,sizeof(expectedGrid));Check();
}
void ModelUnload(){
    for(unsigned i=0;i<0x7580;i+=0xbc)expectedEvents.push_back(0x100000+i+DAT_05828d58);
    for(unsigned cell=0;cell<256;++cell)while(!lists[cell].empty())ModelRemove(cell,lists[cell].back());
    for(unsigned i=0x23;i<0x68;++i)expectedEvents.push_back(0x200000+i);
}
int main(){
    static_assert(sizeof(nodes)==16*sizeof(OBJECT));
    if(sizeof(void*)==8)Require(reinterpret_cast<std::uintptr_t>(nodes)>0xffffffffull);
    unsigned cases=0;
    for(unsigned cell=0;cell<256;++cell)for(unsigned order=0;order<4;++order){
        Reset();float angle[3]={0,0,0};float position[3]={float((cell/16)*1600+400),float((cell%16)*1600+400),0};
        for(unsigned id=0;id<7;++id){ModelInsert(cell,id);Require(CreateObject(id,position,angle,1)==Pointer(id));Check();}
        Entity_GridUnlink(nullptr,nullptr);Check();
        const unsigned orders[3][6]={{0,1,2,3,4,5},{6,5,4,3,2,1},{3,0,6,2,4,1}};
        for(unsigned step=0;step<(order<3?6u:0u);++step){
            unsigned id=orders[order][step];ModelRemove(cell,id);Entity_GridUnlink(Pointer(id),&g_ObjectBucketGrid[cell]);Check();
        }
        unsigned other=(cell+127)%256;position[0]=float((other/16)*1600+400);position[1]=float((other%16)*1600+400);
        ModelInsert(other,7);Require(CreateObject(8,position,angle,1)==Pointer(7));Check();
        ModelUnload();DeleteObjects();Check();for(BYTE b:DAT_083a2370)Require(b==0);
        ++cases;
    }
    std::printf("SceneGridNative: %u ciclos, ptr=%zu OBJECT=%zu, modelo/bytes/eventos OK\n",cases,sizeof(void*),sizeof(OBJECT));
}
'''


def harness():
    header = read_source("src/Entity/EntityTypes.h")
    types = ("#include <cstdint>\n#include <cstddef>\n#include <algorithm>\n"
             "using BYTE=std::uint8_t; using vec3_t=float[3]; using vec34_t=float[3][4];\n"
             "#pragma pack(push,1)\n" + record(header, "OBJECT") +
             "\n#pragma pack(pop)\nOBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}\n")
    cell = read_source("src/Scene/SceneObjectCell.h").replace("#pragma once", "")
    services = PREAMBLE.split("using Snapshot", 1)[0].replace("444", "sizeof(OBJECT)")
    return types + cell + services + "\nextern SceneObjectCell g_ObjectBucketGrid[256];\n" + production() + MAIN


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-scene-native-") as temp:
        print(build_and_run(Path(temp) / "current", harness(), args), end="")


if __name__ == "__main__":
    main()
