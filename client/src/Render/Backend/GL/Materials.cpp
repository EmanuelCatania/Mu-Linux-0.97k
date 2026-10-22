#include "stdafx.h"
#include "Render/Materials.h"

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
bool Select(Preset preset,bool depthWrite){return Apply(Describe(preset,depthWrite));}
}

namespace FragmentPipelineGL {
LegacyState ReadLegacyState(){
    return {DAT_083a412c,DAT_083a411c!=0,DAT_083a411e!=0,DAT_083a42e8!=0,DAT_083a411d!=0,DAT_083a4125!=0};
}
}
