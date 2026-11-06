// Funciones públicas de las capas de dibujo: despachan al backend activo.
// Sin MU_RENDER_WEBGPU sólo existe el backend GL y el despacho es directo.
#include "Render/Backend/Api.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/RenderBackend.h"

namespace RenderBackend { Kind active = Kind::OpenGL; }

#ifdef MU_RENDER_WEBGPU
#define MU_BACKEND_DISPATCH(ret, name, params, args) \
    ret name params { if (RenderBackend::IsWebGPU()) return WGPU::name args; return GL::name args; }
#else
#define MU_BACKEND_DISPATCH(ret, name, params, args) \
    ret name params { return GL::name args; }
#endif

namespace GfxDevice { MU_GFX_DEVICE_API(MU_BACKEND_DISPATCH) }
namespace BufferedDraw { MU_BUFFERED_DRAW_API(MU_BACKEND_DISPATCH) }
namespace MeshDraw { MU_MESH_DRAW_API(MU_BACKEND_DISPATCH) }
namespace GlyphAtlas { MU_GLYPH_ATLAS_API(MU_BACKEND_DISPATCH) }
namespace Skinning { MU_SKINNING_API(MU_BACKEND_DISPATCH) }
