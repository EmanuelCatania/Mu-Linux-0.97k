#include "stdafx.h"
#include "Render/Materials.h"
#include "Render/BMDDevice.h"
#include "Render/MeshDraw.h"
#include "Render/ResourceTables.h"
#include <array>
#include <cstring>
#include <vector>

extern "C" void DbgLogPublic(const char*);

namespace Materials {
namespace {
bool enabled=false;
unsigned twoSidedDepth=0;
template<class T> void Cached(Switch value,T& cache,unsigned cap) {
    if(value==Switch::Keep)return;
    const bool next=value==Switch::On;
    if((cache!=0)==next)return;
    cache=next?1:0;if(next)glEnable(cap);else glDisable(cap);
}
void DepthWrite(Switch value) {
    if(value==Switch::Keep)return;
    const bool next=value==Switch::On;
    if((DAT_083a42e8!=0)==next)return;
    DAT_083a42e8=next?1:0;glDepthMask(next?1:0);
}
}
bool Enabled(){return enabled;}
void Toggle() {
    // Vaciar antes del cambio: no se difiere ni reordena geometrÃ­a transparente.
    BufferedDraw::Flush();enabled=!enabled;RenderProfile::RestartCapture();
    DbgLogPublic(enabled?"Materiales R4a: activo":"Materiales R4a: referencia");
}
bool Apply(const Material& m) {
    if(!enabled)return false;
    if(m.blend!=Blend::Keep&&DAT_083a412c!=static_cast<int>(m.blend)) {
        DAT_083a412c=static_cast<int>(m.blend);
        if(m.blend==Blend::Opaque)glDisable(GL_BLEND);
        else {
            glEnable(GL_BLEND);
            switch(m.blend) {
            case Blend::LightMap:glBlendFunc(GL_ZERO,GL_SRC_COLOR);break;
            case Blend::Alpha:glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);break;
            case Blend::Add:glBlendFunc(GL_ONE,GL_ONE);break;
            // Los nombres histÃ³ricos no describen sus factores reales.
            case Blend::Attenuate:glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);break;
            case Blend::InverseAdd:glBlendFunc(GL_ONE_MINUS_SRC_COLOR,GL_ONE);break;
            default:break;
            }
        }
    }
    Cached(twoSidedDepth?Switch::Off:m.cull,DAT_083a411c,GL_CULL_FACE);DepthWrite(m.depthWrite);
    Cached(m.alphaTest,DAT_083a411d,GL_ALPHA_TEST);Cached(m.texture,DAT_083a4125,GL_TEXTURE_2D);
    if(m.depthTest==Switch::Off) {
        // Conservar las dos lecturas/barreras del helper de referencia.
        glGetError();Cached(m.depthTest,DAT_083a411e,GL_DEPTH_TEST);glGetError();
    } else Cached(m.depthTest,DAT_083a411e,GL_DEPTH_TEST);
    // FogEnable expresa polÃ­tica de escena, no un cachÃ© del estado real.
    // Si vale cero, los helpers histÃ³ricos NO cambian GL_FOG.
    if(DAT_083a42ea&&m.fog!=Fog::Keep) {
        if(m.fog==Fog::Scene)glEnable(GL_FOG);else glDisable(GL_FOG);
    }
    return true;
}
bool ApplySequence(const MaterialSequence& sequence){
    if(!enabled||sequence.count==0||sequence.count>3)return false;
    for(unsigned i=0;i<sequence.count;++i)Apply(sequence.steps[i]);
    return true;
}
TwoSidedScope::TwoSidedScope(bool twoSided){
    managed=enabled;if(!managed||!twoSided)return;
    overrideCull=true;previousCull=glIsEnabled(GL_CULL_FACE)!=0;++twoSidedDepth;
    // Corregir tambiÃ©n una entrada con cachÃ© incoherente; la propiedad de
    // dos caras es explÃ­cita y se mantiene en los materiales interiores.
    glDisable(GL_CULL_FACE);DAT_083a411c=0;
}
TwoSidedScope::~TwoSidedScope(){
    if(!overrideCull)return;
    --twoSidedDepth;const bool restore=twoSidedDepth?false:previousCull;
    if(restore)glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);
    DAT_083a411c=restore?1:0;
}
bool BeginText(){
    if(!enabled)return false;
    // COLOR_BUFFER conserva factores RGB/alpha y ecuaciones, ademÃ¡s del enable.
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIST_BIT|GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    return true;
}
void EndText(){glPopAttrib();}
InheritedBlendScope::InheritedBlendScope(){
    if(!enabled)return;
    active=true;wasEnabled=glIsEnabled(GL_BLEND)!=0;previousBlend=DAT_083a412c;
    glEnable(GL_BLEND);
    // No representa ningÃºn preset: BMD__RenderBodyShadow no selecciona mezcla.
    // Invalidar temporalmente evita anunciar factores que no se establecieron.
    DAT_083a412c=-1;
}
InheritedBlendScope::~InheritedBlendScope(){
    if(!active)return;
    // La sombra no cambia factores. La barrera envÃ­a sus triÃ¡ngulos antes
    // de recuperar activaciÃ³n; los demÃ¡s estados quedan como en referencia.
    if(wasEnabled)glEnable(GL_BLEND);else glDisable(GL_BLEND);
    DAT_083a412c=previousBlend;
}
bool BeginOverlay(const OverlayState& s){
    if(!enabled)return false;
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
    glViewport(0,0,s.width,s.height);
    // Conservar la secuencia de referencia, incluida la perspectiva temporal.
    gluPerspective(s.fov,double(s.width)/double(s.height),s.nearClip,s.farClip);
    glLoadIdentity();gluOrtho2D(0,double(s.width),0,double(s.height));
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    Material entry;entry.depthTest=Switch::Off;Apply(entry);
    return true;
}
bool EndOverlay(){
    if(!enabled)return false;
    glMatrixMode(GL_PROJECTION);glPopMatrix();
    glMatrixMode(GL_MODELVIEW);glPopMatrix();
    return true;
}
bool BeginView(const ViewState& s){
    if(!enabled)return false;
    // La entrada de referencia escribe el estado fÃ­sico incluso cuando el
    // cachÃ© legacy difiere. No usar Cached para omitir esta reparaciÃ³n.
    if(s.alphaTest)glEnable(GL_ALPHA_TEST);else glDisable(GL_ALPHA_TEST);
    if(s.texture)glEnable(GL_TEXTURE_2D);else glDisable(GL_TEXTURE_2D);
    if(s.depthTest)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
    if(s.cull)glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);
    glDepthMask(s.depthWrite?1:0);
    DAT_083a411d=s.alphaTest?1:0;DAT_083a4125=s.texture?1:0;
    DAT_083a411e=s.depthTest?1:0;DAT_083a411c=s.cull?1:0;DAT_083a42e8=s.depthWrite?1:0;
    const auto compare=[](Compare c)->unsigned {
        switch(c){
        case Compare::Never:return GL_NEVER;case Compare::Less:return GL_LESS;
        case Compare::Equal:return GL_EQUAL;case Compare::LessEqual:return GL_LEQUAL;
        case Compare::Greater:return GL_GREATER;case Compare::NotEqual:return GL_NOTEQUAL;
        case Compare::GreaterEqual:return GL_GEQUAL;case Compare::Always:return GL_ALWAYS;
        }return GL_ALWAYS;
    };
    glDepthFunc(compare(s.depthFunction));glAlphaFunc(compare(s.alphaFunction),s.alphaReference);
    if(s.fog==ViewFog::Off)glDisable(GL_FOG);
    else {
        glEnable(GL_FOG);
        if(s.fog==ViewFog::Linear){glFogi(GL_FOG_MODE,GL_LINEAR);glFogf(GL_FOG_START,s.fogStart);glFogf(GL_FOG_END,s.fogEnd);}
        else {glFogi(GL_FOG_MODE,GL_EXP2);glFogf(GL_FOG_DENSITY,s.fogDensity);}
        glFogfv(GL_FOG_COLOR,s.fogColor);
    }
    return true;
}
bool BindTexture(TextureSlot texture){
    if(!enabled||texture.index<0)return false;
    const unsigned slot=static_cast<unsigned>(texture.index);
    // Conservar el cachÃ© por slot y el orden de referencia: publicar el Ã­ndice
    // antes de asegurar la tabla, y resolver el handle sÃ³lo si cambia el slot.
    if(DAT_00561574!=slot){
        DAT_00561574=slot;ResourceTables::EnsureTextures(std::size_t(slot)+1);
        const unsigned native=(&DAT_083a7ccc)[std::size_t(slot)*0xe];
        glBindTexture(GL_TEXTURE_2D,native);
    }
    return true;
}
bool ApplyTextured(const TexturedMaterial& m){
    // Rechazar antes de aplicar estado, tambiÃ©n con material primero.
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

namespace FragmentPipelineGL {
LegacyState ReadLegacyState(){
    return {DAT_083a412c,DAT_083a411c!=0,DAT_083a411e!=0,DAT_083a42e8!=0,DAT_083a411d!=0,DAT_083a4125!=0};
}
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
        const bool alpha=(s.flags&0xc0)==0&&(s.alpha||(&DAT_083a7cc8)[std::size_t(s.textureSlot)*0x38]=='\x04');
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
    case P::Shadow:{
        Materials::Material noWrite;noWrite.depthWrite=Materials::Switch::Off;
        const Materials::MaterialSequence sequence{{Materials::Describe(Materials::Preset::Untextured,false),noWrite},2};
        if(!Materials::ApplySequence(sequence)){GL_SetAlphaTest('\0');GL_DisableDepthWrites();}break;
    }
    }
}
MeshScope::MeshScope(bool twoSided):cull(twoSided){
    FragmentPipelineGL::EnterModel();
    if(!cull&&twoSided&&glIsEnabled(GL_CULL_FACE)){glDisable(GL_CULL_FACE);legacyCull=true;}
}
MeshScope::~MeshScope(){FragmentPipelineGL::LeaveModel();if(legacyCull)glEnable(GL_CULL_FACE);}
namespace {
struct ModelFrame { int mode=0;std::array<double,16> matrix{};bool reference=false,captured=false; };
std::vector<ModelFrame> modelFrames;
unsigned MatrixKey(int mode){return mode==GL_PROJECTION?GL_PROJECTION_MATRIX:mode==GL_TEXTURE?GL_TEXTURE_MATRIX:GL_MODELVIEW_MATRIX;}
}
void BeforeMatrixChange(){
    // Copia al escribir: las pasadas BMD habituales no modifican matrices y
    // no necesitan consultas ni copias al cerrar cada modelo.
    for(auto& frame:modelFrames)if(!frame.reference&&!frame.captured){
        glGetDoublev(MatrixKey(frame.mode),frame.matrix.data());frame.captured=true;
    }
}
void BeginModel(){
    BufferedDraw::Flush();ModelFrame frame;frame.reference=!Materials::Enabled();
    if(frame.reference)glPushMatrix();
    else glGetIntegerv(GL_MATRIX_MODE,&frame.mode);
    modelFrames.push_back(frame);FragmentPipelineGL::EnterModel();
}
void EndModel(){
    BufferedDraw::Flush();
    if(modelFrames.empty()){DbgLogPublic("PerfilR4 BMD: error_ambito_matriz=sin_entrada");return;}
    const auto frame=modelFrames.back();modelFrames.pop_back();
    if(frame.reference)glPopMatrix();
    else if(frame.captured){
        // Pila propia por valor. La frontera con productores aÃºn compatibles
        // sÃ³lo restaura la matriz si un callee la modificÃ³; no usa glPush/Pop.
        int current=0;glGetIntegerv(GL_MATRIX_MODE,&current);
        if(current!=frame.mode)glMatrixMode(frame.mode);
        double matrix[16];glGetDoublev(MatrixKey(frame.mode),matrix);
        if(std::memcmp(matrix,frame.matrix.data(),sizeof(matrix)))glLoadMatrixd(frame.matrix.data());
        if(current!=frame.mode){glMatrixMode(current);DbgLogPublic("PerfilR4 BMD: error_ambito_matriz=modo_no_balanceado");}
    }
    FragmentPipelineGL::LeaveModel();
}

}
