#pragma once
#include "Render/Gfx.h"

// Parte de Gfx que llega al dispositivo. RenderState guarda todo el estado;
// GL aplica acá lo que existe en la API en el momento del cambio, mientras que
// WebGPU lo lee de RenderState al armar cada pipeline.
#define MU_GFX_DEVICE_API(X) \
    X(void, Reset, (), ()) \
    X(void, ApplyCap, (Gfx::Cap cap, bool on), (cap, on)) \
    X(void, ApplyBlend, (Gfx::Factor source, Gfx::Factor destination), (source, destination)) \
    X(void, ApplyDepthFunc, (Gfx::Compare function), (function)) \
    X(void, ApplyDepthMask, (bool write), (write)) \
    X(void, ApplyTexture, (Gfx::Texture texture), (texture)) \
    X(void, ApplyViewport, (int x, int y, int width, int height), (x, y, width, height)) \
    X(void, ClearColor, (float r, float g, float b, float a), (r, g, b, a)) \
    X(void, Clear, (unsigned clearMask), (clearMask)) \
    X(void, Flush, (), ()) \
    X(void, ClearErrors, (), ()) \
    X(void, PostProcess, (bool antialias, float sharpen), (antialias, sharpen))     X(bool, SupportsShadowMaps, (), ())     X(void, ShadowScene, (bool begin, const float* focus, float strength), (begin, focus, strength)) \
    X(void, ShadowCasting, (bool enabled), (enabled)) \
    X(void, ReadPixelsRGB, (int x, int y, int width, int height, void* out), (x, y, width, height, out)) \
    X(int, MaxTextureSize, (), ()) \
    X(void, MaxViewportDims, (int out[2]), (out)) \
    X(const char*, GetDeviceInfo, (Gfx::DeviceInfo info), (info)) \
    X(Gfx::Texture, CreateTexture, (), ()) \
    X(void, DeleteTexture, (Gfx::Texture texture), (texture)) \
    X(void, TexImage2D, (int components, int width, int height, Gfx::Format format, const void* pixels), (components, width, height, format, pixels)) \
    X(void, TexSubImage2D, (int x, int y, int width, int height, Gfx::Format format, const void* pixels), (x, y, width, height, format, pixels)) \
    X(void, TexParameter, (Gfx::TexParam parameter, Gfx::TexValue value), (parameter, value)) \
    X(void, TexBorderColor, (const float rgba[4]), (rgba)) \
    X(void, SetUnpackAlignment, (int alignment), (alignment)) \
    X(int, UnpackAlignment, (), ())

#ifndef MU_BACKEND_DECLARE
#define MU_BACKEND_DECLARE(ret, name, params, args) ret name params;
#endif
namespace GfxDevice {
MU_GFX_DEVICE_API(MU_BACKEND_DECLARE)
namespace GL { MU_GFX_DEVICE_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_GFX_DEVICE_API(MU_BACKEND_DECLARE) }
}
