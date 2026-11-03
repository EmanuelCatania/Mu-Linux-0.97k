#include "stdafx.h"
#include "Render/Gfx.h"
#include "Render/Materials.h"
#include "Render/BMDDevice.h"
#include "Render/MeshDraw.h"
#include "Render/RenderState.h"
#include "Render/ResourceTables.h"
#include <cstring>
#include <vector>

extern "C" void DbgLogPublic(const char*);

namespace Materials {
namespace {
bool enabled=false;
unsigned twoSidedDepth=0;
template<class T> void Cached(Switch value,T& cache,Gfx::Cap cap) {
    if(value==Switch::Keep)return;
    const bool next=value==Switch::On;
    if((cache!=0)==next)return;
    cache=next?1:0;Gfx::SetEnabled(cap,next);
}
void DepthWrite(Switch value) {
    if(value==Switch::Keep)return;
    const bool next=value==Switch::On;
    if((DAT_083a42e8!=0)==next)return;
    DAT_083a42e8=next?1:0;Gfx::DepthMask(next);
}
}
bool Enabled(){return enabled;}
void Toggle() {
    // Vaciar antes del cambio: no se difiere ni reordena geometría transparente.
    BufferedDraw::Flush();enabled=!enabled;RenderProfile::RestartCapture();
    DbgLogPublic(enabled?"Materiales R4a: activo":"Materiales R4a: referencia");
}
bool Apply(const Material& m) {
    if(!enabled)return false;
    if(m.blend!=Blend::Keep&&DAT_083a412c!=static_cast<int>(m.blend)) {
        DAT_083a412c=static_cast<int>(m.blend);
        if(m.blend==Blend::Opaque)Gfx::Disable(Gfx::Cap::Blend);
        else {
            using F=Gfx::Factor;
            Gfx::Enable(Gfx::Cap::Blend);
            switch(m.blend) {
            case Blend::LightMap:Gfx::BlendFunc(F::Zero,F::SrcColor);break;
            case Blend::Alpha:Gfx::BlendFunc(F::SrcAlpha,F::OneMinusSrcAlpha);break;
            case Blend::Add:Gfx::BlendFunc(F::One,F::One);break;
            // Los nombres históricos no describen sus factores reales.
            case Blend::Attenuate:Gfx::BlendFunc(F::Zero,F::OneMinusSrcColor);break;
            case Blend::InverseAdd:Gfx::BlendFunc(F::OneMinusSrcColor,F::One);break;
            default:break;
            }
        }
    }
    Cached(twoSidedDepth?Switch::Off:m.cull,DAT_083a411c,Gfx::Cap::CullFace);DepthWrite(m.depthWrite);
    Cached(m.alphaTest,DAT_083a411d,Gfx::Cap::AlphaTest);Cached(m.texture,DAT_083a4125,Gfx::Cap::Texture2D);
    Cached(m.depthTest,DAT_083a411e,Gfx::Cap::DepthTest);
    // FogEnable expresa política de escena, no un caché del estado real.
    // Si vale cero, los helpers históricos NO cambian la niebla.
    if(DAT_083a42ea&&m.fog!=Fog::Keep)Gfx::SetEnabled(Gfx::Cap::Fog,m.fog==Fog::Scene);
    return true;
}
bool ApplySequence(const MaterialSequence& sequence){
    if(!enabled||sequence.count==0||sequence.count>3)return false;
    for(unsigned i=0;i<sequence.count;++i)Apply(sequence.steps[i]);
    return true;
}
TwoSidedScope::TwoSidedScope(bool twoSided){
    managed=enabled;if(!managed||!twoSided)return;
    overrideCull=true;previousCull=Gfx::IsEnabled(Gfx::Cap::CullFace);++twoSidedDepth;
    // Corregir también una entrada con caché incoherente; la propiedad de
    // dos caras es explícita y se mantiene en los materiales interiores.
    Gfx::Disable(Gfx::Cap::CullFace);DAT_083a411c=0;
}
TwoSidedScope::~TwoSidedScope(){
    if(!overrideCull)return;
    --twoSidedDepth;const bool restore=twoSidedDepth?false:previousCull;
    Gfx::SetEnabled(Gfx::Cap::CullFace,restore);
    DAT_083a411c=restore?1:0;
}
bool BeginText(){
    if(!enabled)return false;
    // COLOR_BUFFER conserva factores y activación de la mezcla.
    Gfx::PushAttrib(Gfx::AttribEnable|Gfx::AttribCurrent|Gfx::AttribColorBuffer);
    Gfx::Disable(Gfx::Cap::Texture2D);Gfx::Disable(Gfx::Cap::DepthTest);Gfx::Enable(Gfx::Cap::Blend);
    Gfx::BlendFunc(Gfx::Factor::SrcAlpha,Gfx::Factor::OneMinusSrcAlpha);
    return true;
}
void EndText(){Gfx::PopAttrib();}
InheritedBlendScope::InheritedBlendScope(){
    if(!enabled)return;
    active=true;wasEnabled=Gfx::IsEnabled(Gfx::Cap::Blend);previousBlend=DAT_083a412c;
    Gfx::Enable(Gfx::Cap::Blend);
    // No representa ningún preset: BMD__RenderBodyShadow no selecciona mezcla.
    // Invalidar temporalmente evita anunciar factores que no se establecieron.
    DAT_083a412c=-1;
}
InheritedBlendScope::~InheritedBlendScope(){
    if(!active)return;
    // La sombra no cambia factores. La barrera envía sus triángulos antes
    // de recuperar activación; los demás estados quedan como en referencia.
    Gfx::SetEnabled(Gfx::Cap::Blend,wasEnabled);
    DAT_083a412c=previousBlend;
}
bool BeginOverlay(const OverlayState& s){
    if(!enabled)return false;
    Gfx::MatrixMode(Gfx::Matrix::Projection);Gfx::PushMatrix();Gfx::LoadIdentity();
    Gfx::Viewport(0,0,s.width,s.height);
    // Conservar la secuencia de referencia, incluida la perspectiva temporal.
    Gfx::Perspective(s.fov,double(s.width)/double(s.height),s.nearClip,s.farClip);
    Gfx::LoadIdentity();Gfx::Ortho2D(-double(s.left),double(s.width)-double(s.left),0,double(s.height));
    Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::PushMatrix();Gfx::LoadIdentity();
    Material entry;entry.depthTest=Switch::Off;Apply(entry);
    return true;
}
bool EndOverlay(){
    if(!enabled)return false;
    Gfx::MatrixMode(Gfx::Matrix::Projection);Gfx::PopMatrix();
    Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::PopMatrix();
    return true;
}
bool BeginView(const ViewState& s){
    if(!enabled)return false;
    // La entrada de referencia escribe el estado incluso cuando el caché
    // legacy difiere. No usar Cached para omitir esta reparación.
    Gfx::SetEnabled(Gfx::Cap::AlphaTest,s.alphaTest);Gfx::SetEnabled(Gfx::Cap::Texture2D,s.texture);
    Gfx::SetEnabled(Gfx::Cap::DepthTest,s.depthTest);Gfx::SetEnabled(Gfx::Cap::CullFace,s.cull);
    Gfx::DepthMask(s.depthWrite);
    DAT_083a411d=s.alphaTest?1:0;DAT_083a4125=s.texture?1:0;
    DAT_083a411e=s.depthTest?1:0;DAT_083a411c=s.cull?1:0;DAT_083a42e8=s.depthWrite?1:0;
    // Materials::Compare comparte el orden de Gfx::Compare.
    Gfx::DepthFunc(static_cast<Gfx::Compare>(s.depthFunction));
    Gfx::AlphaFunc(static_cast<Gfx::Compare>(s.alphaFunction),s.alphaReference);
    if(s.fog==ViewFog::Off)Gfx::Disable(Gfx::Cap::Fog);
    else {
        Gfx::Enable(Gfx::Cap::Fog);
        if(s.fog==ViewFog::Linear){Gfx::SetFogMode(Gfx::FogMode::Linear);Gfx::FogStart(s.fogStart);Gfx::FogEnd(s.fogEnd);}
        else {Gfx::SetFogMode(Gfx::FogMode::Exp2);Gfx::FogDensity(s.fogDensity);}
        Gfx::FogColor(s.fogColor);
    }
    return true;
}
bool BindTexture(TextureSlot texture){
    if(!enabled||texture.index<0)return false;
    const unsigned slot=static_cast<unsigned>(texture.index);
    // Conservar el caché por slot y el orden de referencia: publicar el índice
    // antes de asegurar la tabla, y resolver el handle sólo si cambia el slot.
    if(DAT_00561574!=slot){
        DAT_00561574=slot;ResourceTables::EnsureTextures(std::size_t(slot)+1);
        const unsigned native=(&DAT_083a7ccc)[std::size_t(slot)* TEXTURE_STRIDE_DWORDS];
        Gfx::BindTexture(native);
    }
    return true;
}
bool ApplyTextured(const TexturedMaterial& m){
    // Rechazar antes de aplicar estado, también con material primero.
    if(!enabled||m.texture.index<0)return false;
    if(m.order==BindingOrder::MaterialFirst){
        if(!Apply(m.material))return false;
        return BindTexture(m.texture);
    }
    if(!BindTexture(m.texture))return false;
    return Apply(m.material);
}
bool Select(Preset preset,bool depthWrite){return Apply(Describe(preset,depthWrite));}
}

namespace BMDDevice {
namespace {
void ReferenceSurface(unsigned flags,bool alpha,bool chrome=false){
    if(flags&0x40)GL_SetBlendAdditive();
    else if(flags&0x80)GL_SetBlendSrcAlpha();
    else if(chrome&&(flags&0x10))GL_EnableLightMap();
    else if(alpha)GL_SetBlendSrcOver('\x01');
    else GL_ResetState();
}
}
bool Enabled(){return Materials::Enabled();}
const char* Status(){return Enabled()?"BMD: materiales":"BMD: referencia";}
void Select(const PassState& s){
    using P=Pass;
    switch(s.pass){
    case P::Color:case P::ColorAnimated:{
        const bool alpha=s.pass==P::Color&&s.alpha;
        if(!Materials::ApplySequence(MeshDraw::ColorMaterials(s.flags,alpha))){
            ReferenceSurface(s.flags,false);GL_SetAlphaTest('\0');
            if(alpha)GL_SetBlendSrcOver('\x01');
        }break;
    }
    case P::Surface:{
        GL_BindTextureSlot(s.textureSlot);
        const bool alpha=(s.flags&0xc0)==0&&(s.alpha||(&DAT_083a7cc8)[std::size_t(s.textureSlot)* TEXTURE_STRIDE]=='\x04');
        if(!Materials::Apply(MeshDraw::SurfaceMaterial(s.flags,alpha)))ReferenceSurface(s.flags,alpha);
        break;
    }
    case P::Chrome:{
        const Materials::TexturedMaterial material{{s.textureSlot},MeshDraw::ChromeMaterial(s.flags,s.alpha),Materials::BindingOrder::MaterialFirst};
        if(!Materials::ApplyTextured(material)){ReferenceSurface(s.flags,s.alpha,true);GL_BindTextureSlot(s.textureSlot);}break;
    }
    case P::Blend:{
        const Materials::TexturedMaterial material{{s.textureSlot},MeshDraw::BlendMaterial(s.flags)};
        if(!Materials::ApplyTextured(material)){
            GL_BindTextureSlot(s.textureSlot);
            if(s.flags&0x80)GL_SetBlendSrcAlpha();else GL_SetBlendAdditive();
        }break;
    }
    case P::Metal:
        if(!Materials::ApplySequence(MeshDraw::MetalMaterials())){GL_SetBlendAdditive();GL_SetAlphaTest('\0');GL_DisableDepthWrites();}break;
    case P::Glow:
        if(!Materials::Select(Materials::Preset::Add))GL_SetBlendAdditive();break;
    case P::Exit:case P::ShadowExit:
        if(!Materials::Apply(MeshDraw::ExitMaterial()))GL_EnableDepthWrites();break;
    case P::ShadowBlend:
        if(!Materials::Select(Materials::Preset::Alpha))GL_SetBlendSrcOver('\x01');break;
    case P::Shadow:{
        Materials::Material noWrite;noWrite.depthWrite=Materials::Switch::Off;
        const Materials::MaterialSequence sequence{{Materials::Describe(Materials::Preset::Untextured,false),noWrite},2};
        if(!Materials::ApplySequence(sequence)){GL_SetAlphaTest('\0');GL_DisableDepthWrites();}break;
    }
    }
}
MeshScope::MeshScope(bool twoSided):cull(twoSided){
    if(!cull&&twoSided&&Gfx::IsEnabled(Gfx::Cap::CullFace)){Gfx::Disable(Gfx::Cap::CullFace);legacyCull=true;}
}
MeshScope::~MeshScope(){if(legacyCull)Gfx::Enable(Gfx::Cap::CullFace);}
namespace {
// Ámbito de modelo: la matriz de entrada se repone por valor si un callee la
// modificó. Equivale al glPushMatrix/glPopMatrix de la referencia.
struct ModelFrame { Gfx::Matrix mode=Gfx::Matrix::ModelView;float matrix[16]{}; };
std::vector<ModelFrame> modelFrames;
}
void BeginModel(){
    BufferedDraw::Flush();ModelFrame frame;frame.mode=RenderState::MatrixMode();
    std::memcpy(frame.matrix,RenderState::Top(frame.mode),sizeof(frame.matrix));
    modelFrames.push_back(frame);
}
void EndModel(){
    BufferedDraw::Flush();
    if(modelFrames.empty()){DbgLogPublic("PerfilR4 BMD: error_ambito_matriz=sin_entrada");return;}
    const auto frame=modelFrames.back();modelFrames.pop_back();
    if(RenderState::MatrixMode()!=frame.mode)DbgLogPublic("PerfilR4 BMD: error_ambito_matriz=modo_no_balanceado");
    if(std::memcmp(RenderState::Top(frame.mode),frame.matrix,sizeof(frame.matrix))) {
        const auto current=RenderState::MatrixMode();
        RenderState::SetMatrixMode(frame.mode);RenderState::LoadMatrix(frame.matrix);RenderState::SetMatrixMode(current);
    }
}

}
