#pragma once
#include "Render/Materials.h"

namespace MeshDraw {
// DESVIACION R4l: misma prioridad de selección que BMD__RenderMesh y su
// variante animada. Alpha incluye el formato RGBA sólo en la rama Surface.
constexpr Materials::Material SurfaceMaterial(unsigned flags,bool alpha) {
    return Materials::Describe((flags&0x40)?Materials::Preset::Add:
        (flags&0x80)?Materials::Preset::Attenuate:
        alpha?Materials::Preset::Alpha:Materials::Preset::Opaque);
}
constexpr Materials::Material ChromeMaterial(unsigned flags,bool alpha) {
    return Materials::Describe((flags&0x40)?Materials::Preset::Add:
        (flags&0x80)?Materials::Preset::Attenuate:
        (flags&0x10)?Materials::Preset::LightMap:
        alpha?Materials::Preset::Alpha:Materials::Preset::Opaque);
}
constexpr Materials::Material BlendMaterial(unsigned flags) {
    return Materials::Describe((flags&0x80)?Materials::Preset::Attenuate:Materials::Preset::Add);
}

struct Input {
    const void* key;
    const unsigned char* faces;
    int faceCount,positionCount,normalCount,uvCount;
    const float* positions;
    const float* colors;
    const float* uv;
    const float* chromeUV;
    int mode;
    bool light,offset;
    float alpha,u,v;
    const float* bodyLight;
    const void* model=nullptr;
};
void Release();
// DESVIACION: vértices únicos e índices persistentes; no expandir cada
// esquina a posición/color/UV en CPU. El skinning y los datos lógicos siguen allí.
bool Draw(const Input& input);
}
