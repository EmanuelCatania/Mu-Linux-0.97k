#pragma once
#include "Render/BufferedDraw.h"
#include "Render/RenderProfile.h"
#include "Render/TerrainDraw.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/Backend/GL/ProfileQueries.h"

// DESVIACION: antes de cambiar el pipeline o leer/escribir su salida, enviar
// la geometría pendiente. Texto/listas/lecturas y stacks de atributos devuelven
// además el control al pipeline de entrada. Los setters/cache no cambian.
namespace BMDDevice { void BeforeMatrixChange(); }
namespace BufferedDraw {
template<class F,class... A> inline auto StateChange(F function,A... args) -> decltype(function(args...)) {
    Flush();BMDDevice::BeforeMatrixChange();FragmentPipelineGL::MatricesChanged();return function(args...);
}
template<class F,class... A> inline auto MaterialChange(FragmentPipelineGL::MaterialPart part,F function,A... args) -> decltype(function(args...)) {
    Flush();FragmentPipelineGL::MaterialChanged(part);return function(args...);
}

template<class F,class... A> inline auto TextureChange(F function,A... args) -> decltype(function(args...)) {
    Flush();FragmentPipelineGL::InvalidateTextures();TerrainDraw::InvalidateBoundTexture();return function(args...);
}
template<class F> inline void DeleteTexturesState(F function,int count,const unsigned* names) {
    Flush();FragmentPipelineGL::InvalidateTextures();for(int i=0;i<count;++i)TerrainDraw::InvalidateTexture(names[i]);function(count,names);
}
template<class F,class... A> inline auto LegacyDraw(F function,A... args) -> decltype(function(args...)) {
    Suspend();RenderProfile::Legacy();return function(args...);
}

template<class F,class... A> inline auto LegacyCall(F function,A... args) -> decltype(function(args...)) {
    Suspend();return function(args...);
}
// No vaciar un lote cuando el comando mantiene exactamente el estado actual.
// Ejecutarlo igualmente conserva su registro dentro de display lists.
template<class F> inline void SetEnabled(F function,unsigned cap,bool enabled) {
    if ((::glIsEnabled(cap)!=0)!=enabled) {
        Flush();
        FragmentPipelineGL::CapabilityChanged(cap);
        if(cap==GL_BLEND)RenderProfile::Change(RenderProfile::State::Blend);
        if(cap==GL_DEPTH_TEST)RenderProfile::Change(RenderProfile::State::Depth);
        if(cap==GL_TEXTURE_2D)RenderProfile::Change(RenderProfile::State::Texture);
    }
    function(cap);
}
template<class F> inline void BindTextureState(F function,unsigned target,unsigned name) {
    if (target==GL_TEXTURE_2D) {
        GLint current=0;::glGetIntegerv(GL_TEXTURE_BINDING_2D,&current);
        if (static_cast<unsigned>(current)!=name) { Flush();FragmentPipelineGL::MaterialChanged(FragmentPipelineGL::MaterialPart::Texture);RenderProfile::Change(RenderProfile::State::Texture); }
    } else Flush();
    function(target,name);
}

inline void ProfilePopAttrib() { Suspend();RenderProfileGL::PopAttributes(); }
inline void ProfileBlend(unsigned source,unsigned destination) {
    Flush();
    if(RenderProfile::active)RenderProfileGL::BlendFunction(source,destination);
    ::glBlendFunc(source,destination);
}
inline void ProfileDepthFunction(unsigned value) {
    Flush();if(RenderProfile::active)RenderProfileGL::DepthFunction(value);::glDepthFunc(value);
}
inline void ProfileDepthMask(unsigned char enabled) {
    Flush();
    if(RenderProfile::active)RenderProfileGL::DepthWrite(enabled!=0);
    ::glDepthMask(enabled);
}
}
#define glAlphaFunc(...) ::BufferedDraw::MaterialChange(::FragmentPipelineGL::MaterialPart::Alpha,::glAlphaFunc,__VA_ARGS__)
#define glBindTexture(target,name) ::BufferedDraw::BindTextureState(::glBindTexture,target,name)
#define glBitmap(...) ::BufferedDraw::LegacyDraw(::glBitmap,__VA_ARGS__)
#define glBlendFunc(source,destination) ::BufferedDraw::ProfileBlend(source,destination)
#define glCallLists(...) ::BufferedDraw::LegacyDraw(::glCallLists,__VA_ARGS__)
#define glClear(...) ::BufferedDraw::StateChange(::glClear,__VA_ARGS__)
#define glClearColor(...) ::BufferedDraw::StateChange(::glClearColor,__VA_ARGS__)
#define glDeleteLists(...) ::BufferedDraw::LegacyCall(::glDeleteLists,__VA_ARGS__)
#define glDeleteTextures(...) ::BufferedDraw::DeleteTexturesState(::glDeleteTextures,__VA_ARGS__)
#define glDepthFunc(value) ::BufferedDraw::ProfileDepthFunction(value)
#define glDepthMask(mask) ::BufferedDraw::ProfileDepthMask(mask)
#define glDisable(cap) ::BufferedDraw::SetEnabled(::glDisable,cap,false)
#define glEnable(cap) ::BufferedDraw::SetEnabled(::glEnable,cap,true)
#define glEndList() ::BufferedDraw::LegacyCall(::glEndList)
#define glFlush() ::BufferedDraw::LegacyCall(::glFlush)
#define glFogf(...) ::BufferedDraw::MaterialChange(::FragmentPipelineGL::MaterialPart::Fog,::glFogf,__VA_ARGS__)
#define glFogfv(...) ::BufferedDraw::MaterialChange(::FragmentPipelineGL::MaterialPart::Fog,::glFogfv,__VA_ARGS__)
#define glFogi(...) ::BufferedDraw::MaterialChange(::FragmentPipelineGL::MaterialPart::Fog,::glFogi,__VA_ARGS__)
#define glNewList(...) ::BufferedDraw::LegacyCall(::glNewList,__VA_ARGS__)
#define glPixelStorei(...) ::BufferedDraw::StateChange(::glPixelStorei,__VA_ARGS__)
#define glPopAttrib() ::BufferedDraw::ProfilePopAttrib()
#define glPopClientAttrib() ::BufferedDraw::LegacyCall(::glPopClientAttrib)
#define glPopMatrix() ::BufferedDraw::StateChange(::glPopMatrix)
#define glPushAttrib(...) ::BufferedDraw::LegacyCall(::glPushAttrib,__VA_ARGS__)
#define glPushClientAttrib(...) ::BufferedDraw::LegacyCall(::glPushClientAttrib,__VA_ARGS__)
#define glPushMatrix() ::BufferedDraw::StateChange(::glPushMatrix)
#define glRasterPos2f(...) ::BufferedDraw::LegacyCall(::glRasterPos2f,__VA_ARGS__)
#define glReadPixels(...) ::BufferedDraw::LegacyCall(::glReadPixels,__VA_ARGS__)
#define glRotatef(...) ::BufferedDraw::StateChange(::glRotatef,__VA_ARGS__)
#define glTexEnvf(...) ::BufferedDraw::MaterialChange(::FragmentPipelineGL::MaterialPart::Texture,::glTexEnvf,__VA_ARGS__)
#define glTexImage2D(...) ::BufferedDraw::TextureChange(::glTexImage2D,__VA_ARGS__)
#define glTexSubImage2D(...) ::BufferedDraw::TextureChange(::glTexSubImage2D,__VA_ARGS__)
#define glTexParameterfv(...) ::BufferedDraw::TextureChange(::glTexParameterfv,__VA_ARGS__)
#define glTexParameteri(...) ::BufferedDraw::TextureChange(::glTexParameteri,__VA_ARGS__)
#define glTranslatef(...) ::BufferedDraw::StateChange(::glTranslatef,__VA_ARGS__)
#define glViewport(...) ::BufferedDraw::StateChange(::glViewport,__VA_ARGS__)
#define glOrtho(...) ::BufferedDraw::StateChange(::glOrtho,__VA_ARGS__)
#define glFrustum(...) ::BufferedDraw::StateChange(::glFrustum,__VA_ARGS__)
#define glLoadIdentity() ::BufferedDraw::StateChange(::glLoadIdentity)
#define glLoadMatrixf(...) ::BufferedDraw::StateChange(::glLoadMatrixf,__VA_ARGS__)
#define glMultMatrixf(...) ::BufferedDraw::StateChange(::glMultMatrixf,__VA_ARGS__)
#define glMatrixMode(...) ::BufferedDraw::StateChange(::glMatrixMode,__VA_ARGS__)
#define glScalef(...) ::BufferedDraw::StateChange(::glScalef,__VA_ARGS__)
#define glScissor(...) ::BufferedDraw::StateChange(::glScissor,__VA_ARGS__)
#define glFinish() ::BufferedDraw::LegacyCall(::glFinish)
#define glDrawPixels(...) ::BufferedDraw::LegacyDraw(::glDrawPixels,__VA_ARGS__)
#define glCopyTexImage2D(...) ::BufferedDraw::TextureChange(::glCopyTexImage2D,__VA_ARGS__)
#define glCopyTexSubImage2D(...) ::BufferedDraw::TextureChange(::glCopyTexSubImage2D,__VA_ARGS__)
#define glGetError() ::BufferedDraw::LegacyCall(::glGetError)
#define glCallList(...) ::BufferedDraw::LegacyDraw(::glCallList,__VA_ARGS__)
#define glListBase(...) ::BufferedDraw::StateChange(::glListBase,__VA_ARGS__)

// GLU cambia matrices dentro de la biblioteca; también necesita la barrera.
#define gluOrtho2D(...) ::BufferedDraw::StateChange(::gluOrtho2D,__VA_ARGS__)
#define gluPerspective(...) ::BufferedDraw::StateChange(::gluPerspective,__VA_ARGS__)
#define gluLookAt(...) ::BufferedDraw::StateChange(::gluLookAt,__VA_ARGS__)
#define gluPickMatrix(...) ::BufferedDraw::StateChange(::gluPickMatrix,__VA_ARGS__)

// Las matrices heredadas de otros productores sólo se traducen en la frontera
// de la escena; BMD consume datos uniformes, no matrices implícitas del shader.
#define glLoadIdentity() ::BufferedDraw::StateChange(::glLoadIdentity)
#define glLoadMatrixf(...) ::BufferedDraw::StateChange(::glLoadMatrixf,__VA_ARGS__)
#define glLoadMatrixd(...) ::BufferedDraw::StateChange(::glLoadMatrixd,__VA_ARGS__)
#define glMultMatrixd(...) ::BufferedDraw::StateChange(::glMultMatrixd,__VA_ARGS__)
#define glTranslatef(...) ::BufferedDraw::StateChange(::glTranslatef,__VA_ARGS__)
#define glTranslated(...) ::BufferedDraw::StateChange(::glTranslated,__VA_ARGS__)
#define glScalef(...) ::BufferedDraw::StateChange(::glScalef,__VA_ARGS__)
#define glScaled(...) ::BufferedDraw::StateChange(::glScaled,__VA_ARGS__)
#define glRotated(...) ::BufferedDraw::StateChange(::glRotated,__VA_ARGS__)
#define glFrustum(...) ::BufferedDraw::StateChange(::glFrustum,__VA_ARGS__)
#define glOrtho(...) ::BufferedDraw::StateChange(::glOrtho,__VA_ARGS__)
