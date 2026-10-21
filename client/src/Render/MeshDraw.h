#pragma once

namespace MeshDraw {
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
