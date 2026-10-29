#!/usr/bin/env python3
"""Ciclo real de grilla x86 contra referencia congelada: alta, bajas y descarga."""
import argparse
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, function, read_source


def production():
    source = read_source("src/Scene/Scene_EntityLifecycle.cpp")
    prefix = source.split("// common initialisation", 1)[0] + "return puVar3; }\n"
    create = function(prefix, "CreateObject")
    unlink = function(read_source("src/Scene/Scene_LegacyHelpers.cpp"), "Entity_GridUnlink")
    unload = function(read_source("src/Render/SMD_Parser.cpp"), "DeleteObjects")
    return create + unlink + unload


PREAMBLE = r'''
#include <array>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#define __cdecl
using BYTE = std::uint8_t; using UINT = std::uint32_t; using DWORD = std::uint32_t;
void Require(bool value) { if (!value) std::abort(); }
alignas(16) static unsigned char nodes[16][444];
static unsigned nextSlot;
static bool registered[16], freed[16];
static std::vector<unsigned> events;
static float _DAT_00552d20 = 0.000625f;
static DWORD DAT_05828d58 = 0x1000;
static unsigned char DAT_083a2370[120];
unsigned Slot(const void* p) {
    const auto address = reinterpret_cast<std::uintptr_t>(p);
    const auto base = reinterpret_cast<std::uintptr_t>(nodes);
    Require(address >= base);
    const auto n = address - base;
    Require(n % 444 == 0 && n / 444 < 16);
    return static_cast<unsigned>(n / 444);
}
void* operator_new(unsigned size) {
    Require(size == 444 && nextSlot < 16);
    return nodes[nextSlot++];
}
void operator_delete(void* p) {
    unsigned slot = Slot(p); Require(!registered[slot] && !freed[slot]);
    freed[slot] = true; events.push_back(0x2000 + slot);
}
namespace SceneObjects {
void Register(void* p) { unsigned s=Slot(p); Require(!registered[s]); registered[s]=true; events.push_back(0x1000+s); }
void Unregister(void* p) { unsigned s=Slot(p); Require(registered[s]); registered[s]=false; events.push_back(0x3000+s); }
}
void BMD__Release(DWORD value) { events.push_back(0x100000 + value); }
void UnloadImage(int value) { events.push_back(0x200000 + value); }
using Snapshot = std::vector<unsigned char>;
std::vector<Snapshot> snapshots;
void Save(const void* grid) {
    Snapshot s;
    auto Add=[&](const void* p, std::size_t n) { if(!n)return; auto* b=static_cast<const unsigned char*>(p); s.insert(s.end(), b, b+n); };
    Add(grid,4096); Add(nodes,sizeof(nodes)); Add(registered,sizeof(registered));
    Add(freed,sizeof(freed)); Add(DAT_083a2370,sizeof(DAT_083a2370));
    Add(events.data(), events.size()*sizeof(unsigned)); snapshots.push_back(s);
}
void Reset() {
    std::memset(nodes,0xa5,sizeof(nodes)); std::memset(registered,0,sizeof(registered));
    std::memset(freed,0,sizeof(freed)); std::memset(DAT_083a2370,0xa5,sizeof(DAT_083a2370));
    nextSlot=0;events.clear();snapshots.clear();
}
'''

RUN = r'''
void Run(unsigned cell, unsigned order) {
    Reset(); auto* grid = reinterpret_cast<unsigned char*>(g_ObjectBucketGrid);
    std::memset(grid,0,4096);
    for(unsigned i=0;i<256;++i) {
        grid[i*16]=static_cast<unsigned char>(i); grid[i*16+12]=static_cast<unsigned char>(255-i);
        std::memset(grid+i*16+13,0x5a,3);
    }
    Save(grid);
    float angle[3]={0,0,0};
    float pos[3]={float((cell/16)*1600+400),float((cell%16)*1600+400),0};
    void* pointers[7];
    for(unsigned i=0;i<7;++i) { pointers[i]=CreateObject(i,pos,angle,1); Require(pointers[i]!=nullptr); Save(grid); }
    Entity_GridUnlink(nullptr,grid+cell*16); Save(grid);
    // Cabeza, medio, cola y único; el resto se descarga de cola a cabeza.
    const unsigned orders[3][6]={{0,1,2,3,4,5},{6,5,4,3,2,1},{3,0,6,2,4,1}};
    for(unsigned i=0;i<(order<3?6u:0u);++i) { Entity_GridUnlink(pointers[orders[order][i]],grid+cell*16); Save(grid); }
    // Un nodo en otra celda comprueba el límite y el orden entre celdas.
    unsigned other=(cell+127)%256;
    pos[0]=float((other/16)*1600+400);pos[1]=float((other%16)*1600+400);
    Require(CreateObject(8,pos,angle,1)!=nullptr); Save(grid);
    DeleteObjects(); Save(grid);
    for(unsigned i=0;i<nextSlot;++i) Require(!registered[i] && freed[i]);
    for(unsigned i=0;i<256;++i) {
        for(unsigned b=4;b<12;++b) Require(grid[i*16+b]==0);
        Require(grid[i*16]==static_cast<unsigned char>(i));
        Require(grid[i*16+12]==static_cast<unsigned char>(255-i));
        for(unsigned b=13;b<16;++b) Require(grid[i*16+b]==0x5a);
    }
}
'''


def harness():
    reference = read_source("tests/fixtures/scene-grid-lifecycle-x86.inc")
    header = read_source("src/Scene/SceneObjectCell.h").replace("#pragma once", "")
    return PREAMBLE + header + "\nnamespace Reference {\n" + reference + RUN + r'''
}
namespace Current {
SceneObjectCell g_ObjectBucketGrid[256];
''' + production() + RUN + r'''
}
int main() {
    static_assert(sizeof(void*)==4, "El ciclo completo conserva enlaces de nodos x86 pendientes de B2");
    unsigned cases=0;
    for(unsigned cell=0;cell<256;++cell) for(unsigned order=0;order<4;++order) {
        Reference::Run(cell,order); auto expected=snapshots;
        Current::Run(cell,order);
        if(expected!=snapshots) { std::fprintf(stderr,"Diferencia celda=%u orden=%u\n",cell,order); return 1; }
        ++cases;
    }
    std::printf("SceneGridLifecycle: %u ciclos reales, bytes/registro/borrado/descarga idénticos OK\n",cases);
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32",), default="Win32")
    parser.add_argument("--generator")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-scene-grid-") as temp:
        print(build_and_run(Path(temp) / "current", harness(), args), end="")


if __name__ == "__main__":
    main()
