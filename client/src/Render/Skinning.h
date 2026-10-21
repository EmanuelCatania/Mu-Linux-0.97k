#pragma once
#include <cstddef>
#include "Render/BufferedDraw.h"
// DESVIACION: pose de presentación capturada al terminar Skeleton_Transform.
// La copia CPU permanece disponible para picking, bounds, tela y efectos.
namespace Skinning {
struct Source { const void* key;const unsigned char* vertices;const unsigned char* normals;
    const float* world;const float* lighting;int vertexCount,normalCount; };
void Invalidate();
void Capture(const void* model,const float* bones,int boneCount,float rotationScale,float bodyScale,
    const float* origin,const float* lightDirection,const float* lightCoefficients,
    const Source* sources,std::size_t count);
bool Draw(const void* model,const void* mesh,unsigned elements,const BufferedDraw::Vertex* attributes,
    const int* positions,const int* normals,std::size_t count,std::size_t indices,
    const float* world,int positionCount,const float* colors,bool lit,const float* bodyLight);
void Release();
}
