#pragma once
#include "Render/Materials.h"
// Contrato de estado por pasada BMD; no contiene enums ni handles de GL.
namespace BMDDevice {
enum class Pass { Color, ColorAnimated, Surface, Chrome, Blend, Metal, Glow, Exit, ShadowBlend, Shadow, ShadowExit };
struct PassState { Pass pass;unsigned flags=0;int textureSlot=0;bool alpha=false; };
// El switch sólo alterna esta etapa; los materiales anteriores se conservan.
bool Enabled();
void Toggle();
const char* Status();
void Select(const PassState& state);
void BeginModel();
void EndModel();
// Frontera para escrituras de matrices de productores aún compatibles.
void BeforeMatrixChange();
class MeshScope {
    Materials::TwoSidedScope cull;
    bool legacyCull=false;
public:
    explicit MeshScope(bool twoSided);
    ~MeshScope();
    MeshScope(const MeshScope&)=delete;
    MeshScope& operator=(const MeshScope&)=delete;
};
}
