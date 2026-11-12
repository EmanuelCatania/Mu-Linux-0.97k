#!/usr/bin/env python3
"""Fauna ambiental real (IDA Ambient_ParticleUpdate 0x00502320): spawn por mapa,
peces de Atlans por mitad del mapa y pool/Hero en direcciones bajas, bit31 y altas.

Compila el Ambient_Particles.cpp de producción entero y lo corre muchos frames
con RNG y terreno controlados. Comprueba los tipos que nacen en cada mapa
(Atlans sur = Fish04..07, norte = Fish08/09), que fuera de Atlans/Tarkan sólo se
usen los 3 primeros slots, que nada se escriba fuera del pool y que el resultado
sea idéntico byte a byte en cualquier dirección del pool y del héroe.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, read_source
from X86Views import object_view

FIELDS = ['Live', 'Type', 'SubType', 'Scale', 'Position', 'Angle', 'HiddenMesh', 'LifeTime',
          'BlendMesh', 'BlendMeshLight', 'Timer', 'Direction', 'Velocity', 'Gravity',
          'LightEnable', 'Light', 'CurrentAction', 'PriorAction', 'AnimationFrame',
          'PriorAnimationFrame', 'AlphaEnable', 'AlphaTarget', 'Alpha']

SERVICES = r'''
#include <cmath>
using uint = unsigned int;
char* DAT_07abf5d8;
std::uintptr_t DAT_05828d58;
BYTE* DAT_083a2e90;
BYTE* pool;
unsigned char layer1[65536], wall[65536];
unsigned char* TerrainMappingLayer1 = layer1;
unsigned char* TerrainWall = wall;
int world;
std::uint32_t seed;
std::uint64_t calls;
void Mix(std::uint64_t v){ calls ^= v; calls *= 1099511628211ull; }
int ControlledRand(){ seed = seed * 1103515245u + 12345u; return int((seed >> 16) & 0x7fff); }
#define rand ControlledRand
struct Map { int GetCurrentMap(){ return world; } } gMapManager;
namespace ResourceTables { std::size_t ModelCount(){ return 1200; } }
bool InPool(const void* p){ auto* b = static_cast<const BYTE*>(p); return b >= pool && b < pool + 10 * OBJECT_STRIDE; }
int Terrain_GetTileIndex(unsigned int x, unsigned int y){ Mix(x); Mix(y); return int((x & 255) + (y & 255) * 256); }
float RequestTerrainHeight(float x, float y){ return float(int(x + y) & 63); }
void RenderFxSnapshots_New(const void* p){ Require(InPool(p)); Mix(1); }
void* Joint_Create(int type, float* p, float* q, float* a, unsigned int sub, intptr_t owner, float scale, short key, unsigned char flag){
    Require(type == 1258 && InPool(p) && p == q && InPool(a) && sub == 4 && InPool(reinterpret_cast<void*>(owner)) && scale == 30.0f && key == -1 && flag == 0);
    Mix(2); return nullptr;
}
bool BMD__PlayAnimation(void* model, float* frame, float* prior, void* action, float speed){
    Require(InPool(frame) && InPool(prior) && InPool(action)); *frame += speed; Mix(3); return true;
}
void* SetAction(OBJECT* o, int anim){ Require(InPool(o)); o->CurrentAction = BYTE(anim); return nullptr; }
void Particle_PathUpdate(OBJECT* o, int slot, const void* table, int count){ Require(InPool(o) && table == pool && count == 10 && slot >= 0 && slot < 10); Mix(4); }
void Matrix_BuildFromEuler(float* angles, float* out){ Require(InPool(angles) && InPool(out)); for(int i = 0; i < 12; ++i) out[i] = (i % 5 == 0) ? 1.0f : 0.0f; }
float* Vector_Rotate(float* pt, float* mat, float* out){ Require(InPool(mat)); out[0] = pt[0]; out[1] = pt[1] * 0.5f; out[2] = pt[2]; return out; }
long PlayBuffer(int buffer, const OBJECT* o, int looped){ Require(buffer == 15 && InPool(o) && looped == 0); Mix(5); return 0; }
void Alpha(OBJECT* o){ Require(InPool(o)); o->Alpha = o->AlphaTarget; }
'''

MAIN = r'''
template<class T> void Put(BYTE* p, int off, T v){ std::memcpy(p + off, &v, sizeof(v)); }
constexpr unsigned poolSize = 10 * OBJECT_STRIDE;
unsigned typeSeen[1200];
std::uint64_t Run(BYTE* poolMemory, BYTE* heroMemory){
    pool = poolMemory + 16; DAT_083a2e90 = pool; DAT_07abf5d8 = reinterpret_cast<char*>(heroMemory + 16);
    static BYTE models[1200 * BMD_STRIDE]; DAT_05828d58 = reinterpret_cast<std::uintptr_t>(models);   // escribe CurrentAction en el modelo
    std::uint64_t hash = 14695981039346656037ull;
    std::memset(typeSeen, 0, sizeof(typeSeen));
    for(int i = 0; i < 65536; ++i){ layer1[i] = 5; wall[i] = BYTE((i * 7) % 3 == 1 ? 2 : 0); }
    struct Case { int world; float heroY; };
    const Case cases[] = { {0, 6400}, {1, 6400}, {6, 6400}, {7, 2400}, {7, 20000}, {8, 6400}, {9, 6400}, {-1, 6400} };
    for(const Case& c : cases){
        std::memset(poolMemory, 0x5a, poolSize + 32); std::memset(pool, 0, poolSize);
        std::memset(heroMemory, 0x33, 64); Put(heroMemory, 16 + 0x10, 12800.0f); Put(heroMemory, 16 + 0x14, c.heroY); Put(heroMemory, 16 + 0x18, 100.0f);
        world = c.world; seed = 77u + unsigned(c.world) * 1000u + unsigned(c.heroY); calls = 0;
        for(int frame = 0; frame < 400; ++frame){
            AmbientParticles_Update();
            for(int s = 0; s < 10; ++s){
                OBJECT* o = AsObject(pool + s * OBJECT_STRIDE);
                if(!o->Live) continue;
                const int t = o->Type;
                Require(t >= 0 && t < 1200); ++typeSeen[t];
                if(c.world != 7 && c.world != 8) Require(s < 3);
                switch(c.world){
                case 0: Require(t == 181); break;
                case 1: Require(t == 177); break;
                case 6: Require(t == 178); break;
                case 7: if(c.heroY * 0.01f < 128.0f) Require(t >= 184 && t <= 187);
                        else { Require(t == 188 || t == 189); Require(o->BlendMesh == 0 && o->BlendMeshLight >= 0.09f && o->BlendMeshLight <= 0.91f); }
                        break;
                case 8: Require(t == 179); break;
                default: Require(false);
                }
            }
        }
        for(unsigned i = 0; i < 16; ++i) Require(poolMemory[i] == 0x5a && poolMemory[16 + poolSize + i] == 0x5a);
        for(unsigned i = 0; i < 16; ++i) Require(heroMemory[i] == 0x33);
        for(unsigned i = 0; i < poolSize; ++i){ hash ^= pool[i]; hash *= 1099511628211ull; }
        hash ^= calls; hash *= 1099511628211ull;
    }
    for(int t = 184; t <= 189; ++t) Require(typeSeen[t] > 0);
    Require(typeSeen[181] && typeSeen[177] && typeSeen[178] && typeSeen[179]);
    return hash;
}
int main(){
    auto* low = static_cast<BYTE*>(Reserve(0x26000000u)); auto* hero = static_cast<BYTE*>(Reserve(0x28000000u)); Require(low && hero);
    const auto expected = Run(low, hero);
    if(sizeof(void*) > 4){
        auto* high = static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull))); auto* highHero = static_cast<BYTE*>(Reserve(std::uintptr_t(0x128000000ull)));
        auto* sign = static_cast<BYTE*>(Reserve(0x8e000000u)); auto* signHero = static_cast<BYTE*>(Reserve(0x90000000u));
        Require(high && highHero && sign && signHero);
        Require(Run(high, highHero) == expected); Require(Run(sign, signHero) == expected); Require(Run(high, hero) == expected);
        std::puts("pool/Hero bajos, bit31 y altos; canarios intactos");
    }
    std::printf("Ambient fish: 8 mapas x 400 frames, Atlans sur 184..187 y norte 188/189 %016llx\n", (unsigned long long)expected);
}
'''


def harness(revision=None):
    source = read_source('src/Render/Ambient_Particles.cpp', revision)
    source = re.sub(r'^\s*#include[^\n]*\n', '', source, flags=re.M)
    source = re.sub(r'^extern "C"[^\n]*\n', '', source, flags=re.M)
    assert 'void __cdecl AmbientParticles_Update(void)' in source
    source = source.replace('__cdecl ', '')
    return PREAMBLE.split('char DAT_00559bb4', 1)[0] + 'typedef float vec3_t[3];' + chr(10) + object_view(FIELDS, revision) + SERVICES + source + MAIN


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--architecture', choices=('Win32', 'x64'), default='x64')
    p.add_argument('--generator')
    p.add_argument('--reference-ref')
    args = p.parse_args()
    if args.reference_ref and (os.name != 'nt' or args.architecture != 'Win32'):
        p.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-ambient-fish-') as temp:
        directory = Path(temp)
        current = build_and_run(directory / 'current', harness(), args)
        print(current, end='')
        if args.reference_ref:
            reference = build_and_run(directory / 'reference', harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__ == '__main__':
    main()
