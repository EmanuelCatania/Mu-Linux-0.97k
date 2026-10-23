#pragma once
#include <cstddef>
namespace FragmentPipelineGL {
// Estado lógico legacy para contrastar en el diagnóstico, sin tipos GL.
struct LegacyState { int blend=0;bool cull=false,depth=false,write=false,alpha=false,texture=false; };
LegacyState ReadLegacyState();
void Create(unsigned reference,const char* vertexSource);
void Release(unsigned reference);
void InvalidateTextures();
void InvalidateBindings();
void MatricesChanged();
void EnterModel();
void LeaveModel();
void CapabilityChanged(unsigned capability);
// Invalidar después de vaciar el lote y antes de modificar el material nativo.
enum class MaterialPart : unsigned { Texture=1, Alpha=2, Fog=4, All=7 };
void MaterialChanged(MaterialPart part=MaterialPart::All);
// Selección por envío: el objeto restaura el alpha test físico al terminar.
// Los atributos pueden proceder del stream general o de vértices BMD.
class Draw {
public:
    Draw(unsigned reference,const float* alpha,std::size_t count,std::size_t stride,bool model=false);
    ~Draw();
    Draw(const Draw&)=delete;
    Draw& operator=(const Draw&)=delete;
    unsigned Program()const{return program;}
    void Apply();
private:
    unsigned program=0,reference=0;
    bool modern=false,alphaEnabled=false,textured=false,reject=false,modelVariant=false,modelRequest=false;
    int alphaFunction=0;float alphaReference=0;
    int fog=0,combine=0;
    float fogValues[3]{},fogColor[4]{};
};
}
