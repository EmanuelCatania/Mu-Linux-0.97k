// Skinning con WebGPU: las mallas residentes las graba MeshDraw directamente
// (Backend/WebGPU/MeshDraw.cpp). Estas entradas son del camino GL (VAO y
// vértices armados en CPU) y no se usan: devuelven false.
#include "Render/Backend/Api.h"

namespace Skinning::WGPU {
bool Draw(const void*, const void*, unsigned, const BufferedDraw::Vertex*, const int*, const int*, std::size_t, std::size_t,
    const float*, int, const float*, bool, const float*) { return false; }
void ResidentLayout(unsigned) {}
bool DrawResident(const ResidentDraw&) { return false; }
void Release() {}
}
