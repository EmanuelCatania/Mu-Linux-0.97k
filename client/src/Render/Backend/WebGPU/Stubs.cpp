// Backend WebGPU, etapa 1: el cuadro se limpia con el color del juego y se
// presenta, pero todavía no se dibuja nada. Cada capa se reemplaza por su
// implementación real en las etapas siguientes; mientras tanto devuelven
// valores neutros para que el juego corra igual que con OpenGL.
#include "Render/Backend/Api.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/Backend/WebGPU/Device.h"
#include "Render/RenderState.h"
#include <cstring>

namespace GfxDevice::WGPU {
using namespace Gfx;
namespace {
float clearColor[4]{0, 0, 0, 0};
int unpackAlignment = 4;
Texture nextTexture = 1;
}
void Reset() {}
void ApplyCap(Cap, bool) {}
void ApplyBlend(Factor, Factor) {}
void ApplyDepthFunc(Compare) {}
void ApplyDepthMask(bool) {}
void ApplyTexture(Texture) {}
void ApplyViewport(int, int, int, int) {}
void ClearColor(float r, float g, float b, float a) { clearColor[0] = r; clearColor[1] = g; clearColor[2] = b; clearColor[3] = a; }
void Clear(unsigned clearMask) {
    if (clearMask & ClearColorBuffer) ::WebGPU::Clear(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
}
void Flush() {}
void ClearErrors() {}
void ReadPixelsRGB(int, int, int width, int height, void* out) {
    if (out && width > 0 && height > 0) std::memset(out, 0, size_t(width) * size_t(height) * 3);
}
int MaxTextureSize() { return 8192; }
void MaxViewportDims(int out[2]) { out[0] = out[1] = 8192; }
const char* GetDeviceInfo(DeviceInfo info) {
    switch (info) {
    case DeviceInfo::Vendor: return "WebGPU";
    case DeviceInfo::Renderer: return ::WebGPU::Description();
    default: return "Dawn";
    }
}
Texture CreateTexture() { return nextTexture++; }
void DeleteTexture(Texture) {}
void TexImage2D(int, int, int, Format, const void*) {}
void TexSubImage2D(int, int, int, int, Format, const void*) {}
void TexParameter(TexParam, TexValue) {}
void TexBorderColor(const float*) {}
void SetUnpackAlignment(int alignment) { unpackAlignment = alignment; }
int UnpackAlignment() { return unpackAlignment; }
}

namespace BufferedDraw::WGPU {
bool Initialize() { return true; }
void Release() {}
void Submit(unsigned, const Vertex*, std::size_t) {}
void Suspend() {}
void Activate(unsigned, unsigned, unsigned) {}
void DrawIndexed(unsigned, unsigned, const Vertex*, std::size_t, std::size_t) {}
}

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
