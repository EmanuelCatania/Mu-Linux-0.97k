// Backend WebGPU: mallas BMD residentes y skinning en GPU, todavía sin
// implementar. MeshDraw devuelve false y el juego dibuja la malla por el
// camino de referencia (vértices transformados en CPU, por BufferedDraw), que
// ya funciona con WebGPU.
#include "Render/Backend/Api.h"

namespace MeshDraw::WGPU {
bool DrawShadow(const void*, const char*, const float*, const float*) { return false; }
void Release() {}
void ToggleResident() {}
bool Draw(const Input&) { return false; }
}

namespace Skinning::WGPU {
bool Draw(const void*, const void*, unsigned, const BufferedDraw::Vertex*, const int*, const int*, std::size_t, std::size_t,
    const float*, int, const float*, bool, const float*) { return false; }
void ResidentLayout(unsigned) {}
bool DrawResident(const ResidentDraw&) { return false; }
void Release() {}
}
