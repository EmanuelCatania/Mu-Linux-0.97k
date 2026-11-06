#pragma once
#include "Render/BufferedDraw.h"
#include "Render/MeshDraw.h"
#include "Render/Skinning.h"
#include <cstddef>

// Funciones de las capas de dibujo que implementa cada backend. La misma lista
// declara la versión GL (namespace X::GL), la WebGPU (X::WGPU) y genera las
// funciones públicas que despachan al backend activo (Backend/Dispatch.cpp).
// Las firmas tienen que coincidir con las de los encabezados públicos, sin los
// argumentos por defecto.
// BufferedDraw arma las primitivas en código común (Render/BufferedDraw.cpp);
// el backend sólo recibe lotes listos. El despacho vive en BufferedDraw::Backend.
#define MU_BUFFERED_DRAW_API(X) \
    X(bool, Initialize, (), ()) \
    X(void, Release, (), ()) \
    X(void, Submit, (unsigned mode, const Vertex* vertices, std::size_t count), (mode, vertices, count)) \
    X(void, Suspend, (), ()) \
    X(void, Activate, (unsigned shader, unsigned array, unsigned buffer), (shader, array, buffer)) \
    X(void, DrawIndexed, (unsigned array, unsigned buffer, const Vertex* vertices, std::size_t count, std::size_t indices), (array, buffer, vertices, count, indices))

#define MU_MESH_DRAW_API(X) \
    X(bool, DrawShadow, (const void* model, const char* mesh, const float* world, const float* origin), (model, mesh, world, origin)) \
    X(void, Release, (), ()) \
    X(void, ToggleResident, (), ()) \
    X(bool, Draw, (const Input& input), (input))

#define MU_SKINNING_API(X) \
    X(bool, Draw, (const void* model, const void* mesh, unsigned elements, const BufferedDraw::Vertex* attributes, const int* positions, const int* normals, std::size_t count, std::size_t indices, const float* world, int positionCount, const float* colors, bool lit, const float* bodyLight), (model, mesh, elements, attributes, positions, normals, count, indices, world, positionCount, colors, lit, bodyLight)) \
    X(void, ResidentLayout, (unsigned buffer), (buffer)) \
    X(bool, DrawResident, (const ResidentDraw& draw), (draw)) \
    X(void, Release, (), ())

#ifndef MU_BACKEND_DECLARE
#define MU_BACKEND_DECLARE(ret, name, params, args) ret name params;
#endif
namespace BufferedDraw {
namespace Backend { MU_BUFFERED_DRAW_API(MU_BACKEND_DECLARE) }
namespace GL { MU_BUFFERED_DRAW_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_BUFFERED_DRAW_API(MU_BACKEND_DECLARE) }
}
namespace MeshDraw {
namespace GL { MU_MESH_DRAW_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_MESH_DRAW_API(MU_BACKEND_DECLARE) }
}
namespace Skinning {
namespace GL { MU_SKINNING_API(MU_BACKEND_DECLARE) }
namespace WGPU { MU_SKINNING_API(MU_BACKEND_DECLARE) }
}
