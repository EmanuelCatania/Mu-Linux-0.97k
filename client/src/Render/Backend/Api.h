#pragma once
#include "Render/BufferedDraw.h"
#include "Render/GlyphAtlas.h"
#include "Render/MeshDraw.h"
#include "Render/Skinning.h"
#include <cstddef>

// Funciones de las capas de dibujo que implementa cada backend. La misma lista
// declara la versión GL (namespace X::GL), la WebGPU (X::WGPU) y genera las
// funciones públicas que despachan al backend activo (Backend/Dispatch.cpp).
// Las firmas tienen que coincidir con las de los encabezados públicos, sin los
// argumentos por defecto.
#define MU_BUFFERED_DRAW_API(X) \
    X(Vertex, CaptureAttributes, (bool color, bool uv), (color, uv)) \
    X(bool, Initialize, (), ()) \
    X(void, Release, (), ()) \
    X(void, Begin, (unsigned mode), (mode)) \
    X(void, Begin, (Primitive primitive), (primitive)) \
    X(void, End, (), ()) \
    X(void, Quad, (const Vertex* corners, bool fan), (corners, fan)) \
    X(void, Flush, (const Vertex* current), (current)) \
    X(void, Suspend, (), ()) \
    X(void, Activate, (unsigned shader, unsigned array, unsigned buffer), (shader, array, buffer)) \
    X(void, SetExitHook, (void (*hook)()), (hook)) \
    X(void, DrawIndexed, (unsigned array, unsigned buffer, const Vertex* vertices, std::size_t count, std::size_t indices, const Vertex* current), (array, buffer, vertices, count, indices, current)) \
    X(void, NativeColor, (float r, float g, float b, float a), (r, g, b, a)) \
    X(void, NativeUV, (float u, float v), (u, v)) \
    X(void, NativeVertex, (float x, float y, float z), (x, y, z))

#define MU_MESH_DRAW_API(X) \
    X(bool, DrawShadow, (const void* model, const char* mesh, const float* world, const float* origin), (model, mesh, world, origin)) \
    X(void, Release, (), ()) \
    X(void, ToggleResident, (), ()) \
    X(bool, Draw, (const Input& input), (input))

#define MU_GLYPH_ATLAS_API(X) \
    X(bool, Build, (unsigned key, const Bitmap* glyphs, unsigned count), (key, glyphs, count)) \
    X(bool, Draw, (unsigned key, float x, float y, const char* text, int count, int width, int height), (key, x, y, text, count, width, height)) \
    X(void, Forget, (unsigned key), (key)) \
    X(void, Release, (), ())

#define MU_SKINNING_API(X) \
    X(void, Invalidate, (), ()) \
    X(void, Capture, (const void* model, const float* bones, int boneCount, float rotationScale, float bodyScale, const float* origin, const float* lightDirection, const float* lightCoefficients, const Source* sources, std::size_t count), (model, bones, boneCount, rotationScale, bodyScale, origin, lightDirection, lightCoefficients, sources, count)) \
    X(bool, Draw, (const void* model, const void* mesh, unsigned elements, const BufferedDraw::Vertex* attributes, const int* positions, const int* normals, std::size_t count, std::size_t indices, const float* world, int positionCount, const float* colors, bool lit, const float* bodyLight), (model, mesh, elements, attributes, positions, normals, count, indices, world, positionCount, colors, lit, bodyLight)) \
    X(void, ResidentLayout, (unsigned buffer), (buffer)) \
    X(bool, DrawResident, (const ResidentDraw& draw), (draw)) \
    X(void, Release, (), ())

#ifndef MU_BACKEND_DECLARE
#define MU_BACKEND_DECLARE(ret, name, params, args) ret name params;
#endif
// Los mismos argumentos por defecto que el encabezado público, para las
// llamadas internas de cada backend (una redeclaración puede agregarlos).
#define MU_BUFFERED_DRAW_DEFAULTS \
    Vertex CaptureAttributes(bool color = true, bool uv = true); \
    void Quad(const Vertex* corners, bool fan = true); \
    void Flush(const Vertex* current = nullptr); \
    void DrawIndexed(unsigned array, unsigned buffer, const Vertex* vertices, std::size_t count, std::size_t indices, const Vertex* current = nullptr);
namespace BufferedDraw {
namespace GL { MU_BUFFERED_DRAW_API(MU_BACKEND_DECLARE) MU_BUFFERED_DRAW_DEFAULTS }
namespace WGPU { MU_BUFFERED_DRAW_API(MU_BACKEND_DECLARE) MU_BUFFERED_DRAW_DEFAULTS }
}
namespace MeshDraw {
namespace GL { MU_MESH_DRAW_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_MESH_DRAW_API(MU_BACKEND_DECLARE) }
}
namespace GlyphAtlas {
namespace GL { MU_GLYPH_ATLAS_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_GLYPH_ATLAS_API(MU_BACKEND_DECLARE) }
}
namespace Skinning {
namespace GL { MU_SKINNING_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_SKINNING_API(MU_BACKEND_DECLARE) }
}
