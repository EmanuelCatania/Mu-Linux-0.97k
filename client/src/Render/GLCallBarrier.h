#pragma once
#include "Render/BufferedDraw.h"

// DESVIACION: antes de cambiar el pipeline o leer/escribir su salida, enviar
// la geometría pendiente. Texto/listas/lecturas y stacks de atributos devuelven
// además el control al pipeline de entrada. Los setters/cache no cambian.
namespace BufferedDraw {
template<class F,class... A> inline auto StateChange(F function,A... args) -> decltype(function(args...)) {
    Flush();return function(args...);
}
template<class F,class... A> inline auto LegacyCall(F function,A... args) -> decltype(function(args...)) {
    Suspend();return function(args...);
}
// No vaciar un lote cuando el comando mantiene exactamente el estado actual.
// Ejecutarlo igualmente conserva su registro dentro de display lists.
template<class F> inline void SetEnabled(F function,unsigned cap,bool enabled) {
    if ((::glIsEnabled(cap)!=0)!=enabled) Flush();
    function(cap);
}
template<class F> inline void BindTextureState(F function,unsigned target,unsigned name) {
    if (target==GL_TEXTURE_2D) {
        GLint current=0;::glGetIntegerv(GL_TEXTURE_BINDING_2D,&current);
        if (static_cast<unsigned>(current)!=name) Flush();
    } else Flush();
    function(target,name);
}

}
#define glAlphaFunc(...) ::BufferedDraw::StateChange(::glAlphaFunc,__VA_ARGS__)
#define glBindTexture(target,name) ::BufferedDraw::BindTextureState(::glBindTexture,target,name)
#define glBitmap(...) ::BufferedDraw::LegacyCall(::glBitmap,__VA_ARGS__)
#define glBlendFunc(...) ::BufferedDraw::StateChange(::glBlendFunc,__VA_ARGS__)
#define glCallLists(...) ::BufferedDraw::LegacyCall(::glCallLists,__VA_ARGS__)
#define glClear(...) ::BufferedDraw::StateChange(::glClear,__VA_ARGS__)
#define glClearColor(...) ::BufferedDraw::StateChange(::glClearColor,__VA_ARGS__)
#define glDeleteLists(...) ::BufferedDraw::LegacyCall(::glDeleteLists,__VA_ARGS__)
#define glDeleteTextures(...) ::BufferedDraw::StateChange(::glDeleteTextures,__VA_ARGS__)
#define glDepthFunc(...) ::BufferedDraw::StateChange(::glDepthFunc,__VA_ARGS__)
#define glDepthMask(...) ::BufferedDraw::StateChange(::glDepthMask,__VA_ARGS__)
#define glDisable(cap) ::BufferedDraw::SetEnabled(::glDisable,cap,false)
#define glEnable(cap) ::BufferedDraw::SetEnabled(::glEnable,cap,true)
#define glEndList() ::BufferedDraw::LegacyCall(::glEndList)
#define glFlush() ::BufferedDraw::LegacyCall(::glFlush)
#define glFogf(...) ::BufferedDraw::StateChange(::glFogf,__VA_ARGS__)
#define glFogfv(...) ::BufferedDraw::StateChange(::glFogfv,__VA_ARGS__)
#define glFogi(...) ::BufferedDraw::StateChange(::glFogi,__VA_ARGS__)
#define glNewList(...) ::BufferedDraw::LegacyCall(::glNewList,__VA_ARGS__)
#define glPixelStorei(...) ::BufferedDraw::StateChange(::glPixelStorei,__VA_ARGS__)
#define glPopAttrib() ::BufferedDraw::LegacyCall(::glPopAttrib)
#define glPopClientAttrib() ::BufferedDraw::LegacyCall(::glPopClientAttrib)
#define glPopMatrix() ::BufferedDraw::StateChange(::glPopMatrix)
#define glPushAttrib(...) ::BufferedDraw::LegacyCall(::glPushAttrib,__VA_ARGS__)
#define glPushClientAttrib(...) ::BufferedDraw::LegacyCall(::glPushClientAttrib,__VA_ARGS__)
#define glPushMatrix() ::BufferedDraw::StateChange(::glPushMatrix)
#define glRasterPos2f(...) ::BufferedDraw::LegacyCall(::glRasterPos2f,__VA_ARGS__)
#define glReadPixels(...) ::BufferedDraw::LegacyCall(::glReadPixels,__VA_ARGS__)
#define glRotatef(...) ::BufferedDraw::StateChange(::glRotatef,__VA_ARGS__)
#define glTexEnvf(...) ::BufferedDraw::StateChange(::glTexEnvf,__VA_ARGS__)
#define glTexImage2D(...) ::BufferedDraw::StateChange(::glTexImage2D,__VA_ARGS__)
#define glTexSubImage2D(...) ::BufferedDraw::StateChange(::glTexSubImage2D,__VA_ARGS__)
#define glTexParameterfv(...) ::BufferedDraw::StateChange(::glTexParameterfv,__VA_ARGS__)
#define glTexParameteri(...) ::BufferedDraw::StateChange(::glTexParameteri,__VA_ARGS__)
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
#define glDrawPixels(...) ::BufferedDraw::LegacyCall(::glDrawPixels,__VA_ARGS__)
#define glCopyTexImage2D(...) ::BufferedDraw::StateChange(::glCopyTexImage2D,__VA_ARGS__)
#define glCopyTexSubImage2D(...) ::BufferedDraw::StateChange(::glCopyTexSubImage2D,__VA_ARGS__)
#define glGetError() ::BufferedDraw::LegacyCall(::glGetError)
#define glCallList(...) ::BufferedDraw::LegacyCall(::glCallList,__VA_ARGS__)
#define glListBase(...) ::BufferedDraw::StateChange(::glListBase,__VA_ARGS__)

// GLU cambia matrices dentro de la biblioteca; también necesita la barrera.
#define gluOrtho2D(...) ::BufferedDraw::StateChange(::gluOrtho2D,__VA_ARGS__)
#define gluPerspective(...) ::BufferedDraw::StateChange(::gluPerspective,__VA_ARGS__)
#define gluLookAt(...) ::BufferedDraw::StateChange(::gluLookAt,__VA_ARGS__)
#define gluPickMatrix(...) ::BufferedDraw::StateChange(::gluPickMatrix,__VA_ARGS__)
