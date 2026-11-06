// Pose de skinning capturada al terminar Skeleton_Transform, común a los
// backends: paleta de huesos, escalas, origen, luz y la posición/luz CPU de
// cada malla para reconocer dibujos que la CPU deformó después de la pose.
#include "Render/Skinning.h"
#include <algorithm>
#include <cstring>

namespace Skinning {
namespace {
const void* poseModel=nullptr;
std::vector<float> palette;
std::vector<Pose::Mesh> poseMeshes;
std::size_t poseMeshCount=0;
float rotationScale=1,bodyScale=1,origin[3]{},direction[3]{},coefficients[3]{};
unsigned long long revision=0;
}
void Invalidate() { poseModel=nullptr; }
void Capture(const void* model,const float* bones,int boneCount,float rotation,float scale,
    const float* translation,const float* lightDirection,const float* lightCoefficients,
    const Source* sources,std::size_t count) {
    Invalidate();if(!model || !bones || boneCount<=0) return;
    palette.assign(bones,bones+std::size_t(boneCount)*12);rotationScale=rotation;bodyScale=scale;
    std::copy(translation,translation+3,origin);std::copy(lightDirection,lightDirection+3,direction);
    std::copy(lightCoefficients,lightCoefficients+3,coefficients);
    // DESVIACION: conservar capacidad por orden de malla entre objetos.
    // No liberar y recrear todos los vectores al cambiar de modelo.
    if(poseMeshes.size()<count) poseMeshes.resize(count);
    poseMeshCount=0;
    for(std::size_t i=0;i<count;++i) { const auto& s=sources[i];
        if(s.vertexCount<=0 || !s.vertices || !s.world || s.normalCount<0 || (s.normalCount && (!s.normals || !s.lighting))) continue;
        auto& m=poseMeshes[poseMeshCount++];m.key=s.key;
        m.vertices=s.vertices;m.normals=s.normals;m.vertexCount=s.vertexCount;m.normalCount=s.normalCount;
        m.world.assign(s.world,s.world+s.vertexCount*3);
        m.lighting.assign(s.lighting,s.lighting+s.normalCount);
    }
    poseModel=model;++revision;
}
namespace Pose {
int BoneOf(const unsigned char* base,int stride,int index) { short bone;std::memcpy(&bone,base+index*stride,2);return bone; }
const float* XyzOf(const unsigned char* base,int stride,int index) { return reinterpret_cast<const float*>(base+index*stride+4); }
const Mesh* Find(const void* model,const void* key,const float* world,int positionCount) {
    if(!model || model!=poseModel) return nullptr;
    for(std::size_t i=0;i<poseMeshCount;++i) if(poseMeshes[i].key==key) {
        const auto& m=poseMeshes[i];
        // Las pasadas deformadas modifican BoneVertex después de la pose. Deben
        // usar esos datos CPU, nunca una pose previa aunque coincida el modelo.
        if(positionCount!=m.vertexCount || std::memcmp(world,m.world.data(),m.world.size()*sizeof(float))) return nullptr;
        return &m;
    }
    return nullptr;
}
bool LightMatches(const Mesh& m,const float* colors,int normalCount,const float* bodyLight) {
    // El shader calcula la luz por normal; si la CPU dejó otro color (un
    // efecto que lo modificó), hay que dibujar con los colores CPU.
    if(normalCount!=m.normalCount || !colors) return false;
    for(int n=0;n<m.normalCount;++n)
        for(int k=0;k<3;++k) if(colors[n*3+k]!=m.lighting[n]*bodyLight[k]) return false;
    return true;
}
const std::vector<float>& Palette() { return palette; }
unsigned long long Revision() { return revision; }
float RotationScale() { return rotationScale; }
float BodyScale() { return bodyScale; }
const float* Origin() { return origin; }
const float* Direction() { return direction; }
const float* Coefficients() { return coefficients; }
}
}
