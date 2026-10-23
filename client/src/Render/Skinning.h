#pragma once
#include <cstddef>
#include "Render/BufferedDraw.h"
// DESVIACION: pose de presentación capturada al terminar Skeleton_Transform.
// La copia CPU permanece disponible para picking, bounds, tela y efectos.
namespace Skinning {
// vertices/normals apuntan a los arrays del BMD (estables mientras el modelo
// está cargado): no se copian por pose.
struct Source { const void* key;const unsigned char* vertices;const unsigned char* normals;
    const float* world;const float* lighting;int vertexCount,normalCount; };
// Vértice residente de una malla BMD: posición y normal en el espacio del
// hueso, índice de hueso y UV. Se sube una vez; por cuadro sólo viaja la paleta.
struct StaticVertex { float position[3],bone,normal[4],uv[2]; };
struct ResidentDraw {
    const void* model;const void* key;
    unsigned array;std::size_t indices;int maxBone;
    const float* world;int positionCount;
    const float* colors;int normalCount;
    bool lit,textured;
    const float* bodyLight;
    const float* color;   // color constante del envío (RGBA)
    const float* uv;      // UV constante cuando la malla no se texturiza
    float uOffset,vOffset;
};
void Invalidate();
void Capture(const void* model,const float* bones,int boneCount,float rotationScale,float bodyScale,
    const float* origin,const float* lightDirection,const float* lightCoefficients,
    const Source* sources,std::size_t count);
bool Draw(const void* model,const void* mesh,unsigned elements,const BufferedDraw::Vertex* attributes,
    const int* positions,const int* normals,std::size_t count,std::size_t indices,
    const float* world,int positionCount,const float* colors,bool lit,const float* bodyLight);
// Configura en el VAO enlazado el formato de StaticVertex sobre `buffer`.
void ResidentLayout(unsigned buffer);
// Dibuja una malla residente con la pose capturada. Devuelve false si la
// pose no corresponde, si la CPU la deformó o si el color no es el de la luz.
bool DrawResident(const ResidentDraw& draw);
void Release();
}
