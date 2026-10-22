#include "stdafx.h"
#include "Render/Materials.h"
#include "Render/ResourceTables.h"

extern "C" void DbgLogPublic(const char*);

namespace Materials {
namespace {
bool enabled=false;
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
    // Vaciar antes del cambio: no se difiere ni reordena geometría transparente.
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
            // Los nombres históricos no describen sus factores reales.
            case Blend::Attenuate:glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);break;
            case Blend::InverseAdd:glBlendFunc(GL_ONE_MINUS_SRC_COLOR,GL_ONE);break;
            default:break;
            }
        }
    }
    Cached(m.cull,DAT_083a411c,GL_CULL_FACE);DepthWrite(m.depthWrite);
    Cached(m.alphaTest,DAT_083a411d,GL_ALPHA_TEST);Cached(m.texture,DAT_083a4125,GL_TEXTURE_2D);
    if(m.depthTest==Switch::Off) {
        // Conservar las dos lecturas/barreras del helper de referencia.
        glGetError();Cached(m.depthTest,DAT_083a411e,GL_DEPTH_TEST);glGetError();
    } else Cached(m.depthTest,DAT_083a411e,GL_DEPTH_TEST);
    // FogEnable expresa política de escena, no un caché del estado real.
    // Si vale cero, los helpers históricos NO cambian GL_FOG.
    if(DAT_083a42ea&&m.fog!=Fog::Keep) {
        if(m.fog==Fog::Scene)glEnable(GL_FOG);else glDisable(GL_FOG);
    }
    return true;
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
    // La entrada de referencia escribe el estado físico incluso cuando el
    // caché legacy difiere. No usar Cached para omitir esta reparación.
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
    // Conservar el caché por slot y el orden de referencia: publicar el índice
    // antes de asegurar la tabla, y resolver el handle sólo si cambia el slot.
    if(DAT_00561574!=slot){
        DAT_00561574=slot;ResourceTables::EnsureTextures(std::size_t(slot)+1);
        const unsigned native=(&DAT_083a7ccc)[std::size_t(slot)*0xe];
        glBindTexture(GL_TEXTURE_2D,native);
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

namespace FragmentPipelineGL {
LegacyState ReadLegacyState(){
    return {DAT_083a412c,DAT_083a411c!=0,DAT_083a411e!=0,DAT_083a42e8!=0,DAT_083a411d!=0,DAT_083a4125!=0};
}
}
