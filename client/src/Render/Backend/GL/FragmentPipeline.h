#pragma once
#include <cstddef>
namespace FragmentPipelineGL {
void Create(unsigned reference,const char* vertexSource);
void Release(unsigned reference);
void InvalidateTextures();
void InvalidateBindings();
void CapabilityChanged(unsigned capability);
// Invalidar después de vaciar el lote y antes de modificar el material nativo.
void MaterialChanged();
// Selección por envío: el objeto restaura el alpha test físico al terminar.
// Los atributos pueden proceder del stream general o de vértices BMD.
class Draw {
public:
    Draw(unsigned reference,const float* alpha,std::size_t count,std::size_t stride);
    ~Draw();
    Draw(const Draw&)=delete;
    Draw& operator=(const Draw&)=delete;
    unsigned Program()const{return program;}
    void Apply();
private:
    unsigned program=0,reference=0;
    bool modern=false,alphaEnabled=false,textured=false,reject=false;
    int fog=0,combine=0;
    float fogValues[3]{},fogColor[4]{};
};
}
