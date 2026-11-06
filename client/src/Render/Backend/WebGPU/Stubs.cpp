// Backend WebGPU: capas que todavía no dibujan (mallas BMD y skinning).
// Devuelven valores neutros para que el juego corra igual que con OpenGL; se
// reemplazan por su implementación real en las etapas siguientes.
#include "Render/Backend/Api.h"

namespace MeshDraw::WGPU {
bool DrawShadow(const void*, const char*, const float*, const float*) { return true; }
void Release() {}
void ToggleResident() {}
bool Draw(const Input&) { return true; }
}

namespace Skinning::WGPU {
void Invalidate() {}
void Capture(const void*, const float*, int, float, float, const float*, const float*, const float*, const Source*, std::size_t) {}
bool Draw(const void*, const void*, unsigned, const BufferedDraw::Vertex*, const int*, const int*, std::size_t, std::size_t,
    const float*, int, const float*, bool, const float*) { return true; }
void ResidentLayout(unsigned) {}
bool DrawResident(const ResidentDraw&) { return true; }
void Release() {}
}
