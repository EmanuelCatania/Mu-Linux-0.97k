// Sin PCH: las funciones modernas se resuelven con glad.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#undef glTexImage2D
#define glTexImage2D(...) ::ProfileUploadGL::Image2D(glad_glTexImage2D,__VA_ARGS__)
#undef glTexSubImage2D
#define glTexSubImage2D(...) ::ProfileUploadGL::Sub2D(glad_glTexSubImage2D,__VA_ARGS__)
#undef glTexImage3D
#define glTexImage3D(...) ::ProfileUploadGL::Image3D(glad_glTexImage3D,__VA_ARGS__)
#undef glTexSubImage3D
#define glTexSubImage3D(...) ::ProfileUploadGL::Sub3D(glad_glTexSubImage3D,__VA_ARGS__)
#undef glBufferData
#define glBufferData(...) ::ProfileUploadGL::BufferData(glad_glBufferData,__VA_ARGS__)
#undef glBufferSubData
#define glBufferSubData(...) ::ProfileUploadGL::BufferSubData(glad_glBufferSubData,__VA_ARGS__)

#include "Render/BufferedDraw.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/GlyphAtlas.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace GlyphAtlas {
namespace {
struct Glyph { int x=0,y=0,w=0,h=0,left=0,bottom=0; float advanceX=0,advanceY=0; };
struct Atlas { GLuint texture=0; int width=0,height=0; std::array<Glyph,256> glyphs; };
std::unordered_map<unsigned,Atlas> atlases;

// Incluye la posición raster y las matrices, pero los bindings modernos no
// pertenecen a PushAttrib: se guardan explícitamente para no contaminar la UI.
struct State {
    GLint program,draw,read,active,mode,pack,unpack;
    State() {
        BufferedDraw::Suspend();
        glGetIntegerv(GL_CURRENT_PROGRAM,&program);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);
        glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_MATRIX_MODE,&mode);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&pack);glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&unpack);
        glPushAttrib(GL_ALL_ATTRIB_BITS);glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glActiveTexture(GL_TEXTURE0);RenderProfileGL::ObserveProgram(0);glUseProgram(0);
        glMatrixMode(GL_TEXTURE);glPushMatrix();glLoadIdentity();
        glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
        glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
        glPixelStorei(GL_PACK_ALIGNMENT,1);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glPixelStorei(GL_PACK_ROW_LENGTH,0);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
        glPixelStorei(GL_PACK_SKIP_ROWS,0);glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
        glPixelStorei(GL_PACK_SKIP_PIXELS,0);glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
    }
    ~State() {
        BufferedDraw::Suspend();
        glMatrixMode(GL_MODELVIEW);glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();
        glMatrixMode(GL_TEXTURE);glPopMatrix();glMatrixMode(mode);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);glBindFramebuffer(GL_READ_FRAMEBUFFER,read);
        glPopClientAttrib();RenderProfileGL::PopAttributes();RenderProfileGL::ObserveProgram(program);glUseProgram(program);glActiveTexture(active);
        glBindBuffer(GL_PIXEL_PACK_BUFFER,pack);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack);
    }
};
// El dibujo no toca FBO, pixel store ni buffers de transferencia. Evitar el
// guardado completo de la preparación por cada tramo de texto del HUD.
struct DrawState {
    GLint active,mode,texture,environment;
    DrawState(int width,int height) {
        BufferedDraw::Suspend();
        glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_MATRIX_MODE,&mode);
        glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_VIEWPORT_BIT|GL_POLYGON_BIT);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,&environment);
        glMatrixMode(GL_TEXTURE);glPushMatrix();glLoadIdentity();
        glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(0,width,0,height,-1,1);
        glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();glViewport(0,0,width,height);
    }
    ~DrawState() {
        BufferedDraw::Suspend();
        glMatrixMode(GL_MODELVIEW);glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();
        glMatrixMode(GL_TEXTURE);glPopMatrix();glMatrixMode(mode);
        RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,texture);glBindTexture(GL_TEXTURE_2D,texture);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,environment);
        RenderProfileGL::PopAttributes();glActiveTexture(active);
    }
};
}
void Forget(unsigned base) {
    auto it=atlases.find(base);if (it==atlases.end()) return;
    BufferedDraw::Suspend();glDeleteTextures(1,&it->second.texture);atlases.erase(it);
}
void Release() { while (!atlases.empty()) Forget(atlases.begin()->first); }

bool Build(unsigned base,int height,int maxWidth) {
    Forget(base);
    if (!glGenFramebuffers || !glBindFramebuffer || !glFramebufferTexture2D ||
        !glCheckFramebufferStatus || !glDeleteFramebuffers) return false;
    if (height<=0 || maxWidth<0) return false;
    State state;
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
    const int side=std::max(height,maxWidth)*4+32;
    if (side>limit) return false;
    GLuint target=0,framebuffer=0;
    glGenTextures(1,&target);RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,target);glBindTexture(GL_TEXTURE_2D,target);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,side,side,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glGenFramebuffers(1,&framebuffer);glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,target,0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);glReadBuffer(GL_COLOR_ATTACHMENT0);
    bool ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    Atlas atlas;
    if (ok) {
        glViewport(0,0,side,side);glMatrixMode(GL_PROJECTION);glOrtho(0,side,0,side,-1,1);
        glMatrixMode(GL_MODELVIEW);
        for (GLenum cap : {GL_BLEND,GL_ALPHA_TEST,GL_DEPTH_TEST,GL_STENCIL_TEST,GL_SCISSOR_TEST,
                          GL_FOG,GL_LIGHTING,GL_TEXTURE_2D,GL_DITHER,GL_COLOR_LOGIC_OP}) RenderProfileGL::SetEnabled(cap,false);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glClearColor(0,0,0,0);glColor4f(1,1,1,1);
        const int origin=side/2;
        std::vector<unsigned char> pixels(size_t(side)*side*4);
        std::array<std::vector<unsigned char>,256> masks;
        int cellW=1,cellH=1;
        for (unsigned i=0;i<256 && ok;++i) {
            glClear(GL_COLOR_BUFFER_BIT);glRasterPos2i(origin,origin);glCallList(base+i);
            float raster[4];glGetFloatv(GL_CURRENT_RASTER_POSITION,raster);
            auto& g=atlas.glyphs[i];g.advanceX=raster[0]-origin;g.advanceY=raster[1]-origin;
            glReadPixels(0,0,side,side,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            int left=side,right=-1,bottom=side,top=-1;
            for (int y=0;y<side;++y) for (int x=0;x<side;++x) if (pixels[(size_t(y)*side+x)*4+3]) {
                left=std::min(left,x);right=std::max(right,x);bottom=std::min(bottom,y);top=std::max(top,y);
            }
            if (right<left) continue;
            // Si el bitmap excede el lienzo, conservar el camino anterior.
            if (!left || !bottom || right==side-1 || top==side-1) { ok=false;break; }
            g.w=right-left+1;g.h=top-bottom+1;g.left=left-origin;g.bottom=bottom-origin;
            masks[i].resize(size_t(g.w)*g.h);
            for (int y=0;y<g.h;++y) for (int x=0;x<g.w;++x)
                masks[i][size_t(y)*g.w+x]=pixels[(size_t(y+bottom)*side+x+left)*4+3];
            cellW=std::max(cellW,g.w+2);cellH=std::max(cellH,g.h+2);
        }
        atlas.width=cellW*16;atlas.height=cellH*16;
        ok=ok && atlas.width<=limit && atlas.height<=limit;
        if (ok) {
            std::vector<unsigned char> pixelsAtlas(size_t(atlas.width)*atlas.height*4,255);
            for (size_t p=3;p<pixelsAtlas.size();p+=4) pixelsAtlas[p]=0;
            for (unsigned i=0;i<256;++i) {
                auto& g=atlas.glyphs[i];g.x=(i%16)*cellW+1;g.y=(i/16)*cellH+1;
                for (int y=0;y<g.h;++y) for (int x=0;x<g.w;++x)
                    pixelsAtlas[(size_t(g.y+y)*atlas.width+g.x+x)*4+3]=masks[i][size_t(y)*g.w+x];
            }
            glGenTextures(1,&atlas.texture);RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,atlas.texture);glBindTexture(GL_TEXTURE_2D,atlas.texture);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,atlas.width,atlas.height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixelsAtlas.data());
            ok=atlas.texture && glGetError()==GL_NO_ERROR;
        }
    }
    glDeleteFramebuffers(1,&framebuffer);glDeleteTextures(1,&target);
    if (!ok) { if (atlas.texture) glDeleteTextures(1,&atlas.texture);return false; }
    atlases.emplace(base,std::move(atlas));return true;
}

bool Draw(unsigned base,const char* text,int count,int width,int height) {
    const auto it=atlases.find(base);if (it==atlases.end() || width<=0 || height<=0) return false;
    // El HUD usa SRC_ALPHA/ONE_MINUS_SRC_ALPHA y no escribe profundidad.
    // Otros usos de glBitmap conservan su semántica hasta migrarlos también.
    GLint source=0,destination=0;
    glGetIntegerv(GL_BLEND_SRC,&source);glGetIntegerv(GL_BLEND_DST,&destination);
    if (!glIsEnabled(GL_BLEND) || source!=GL_SRC_ALPHA || destination!=GL_ONE_MINUS_SRC_ALPHA ||
        glIsEnabled(GL_DEPTH_TEST) || glIsEnabled(GL_STENCIL_TEST) || glIsEnabled(GL_FOG) ||
        glIsEnabled(GL_COLOR_LOGIC_OP)) return false;
    GLboolean valid=GL_FALSE;glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
    if (!valid || count<=0) return true;
    float raster[4],color[4];glGetFloatv(GL_CURRENT_RASTER_POSITION,raster);glGetFloatv(GL_CURRENT_RASTER_COLOR,color);
    float advanceX=0,advanceY=0;
    {
        DrawState state(width,height);
        const auto& atlas=it->second;
        RenderProfileGL::SetEnabled(GL_TEXTURE_2D,true);RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,atlas.texture);glBindTexture(GL_TEXTURE_2D,atlas.texture);
        // glBitmap no depende del culling ni del modo de relleno de polígonos.
        RenderProfileGL::SetEnabled(GL_CULL_FACE,false);RenderProfileGL::SetEnabled(GL_POLYGON_SMOOTH,false);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        BufferedDraw::Color4fv(color);BufferedDraw::Begin(GL_QUADS);
        for (int i=0;i<count;++i) {
            const auto& g=atlas.glyphs[static_cast<unsigned char>(text[i])];
            if (g.w && g.h) {
                const float x=std::floor(raster[0]+advanceX)+g.left,y=std::floor(raster[1]+advanceY)+g.bottom;
                const float u=float(g.x)/atlas.width,v=float(g.y)/atlas.height;
                const float u1=float(g.x+g.w)/atlas.width,v1=float(g.y+g.h)/atlas.height;
                BufferedDraw::TexCoord2f(u,v);BufferedDraw::Vertex2f(x,y);
                BufferedDraw::TexCoord2f(u1,v);BufferedDraw::Vertex2f(x+g.w,y);
                BufferedDraw::TexCoord2f(u1,v1);BufferedDraw::Vertex2f(x+g.w,y+g.h);
                BufferedDraw::TexCoord2f(u,v1);BufferedDraw::Vertex2f(x,y+g.h);
            }
            advanceX+=g.advanceX;advanceY+=g.advanceY;
        }
        BufferedDraw::End();
    }
    // Conservar el avance raster que observaban los callers de glCallLists.
    glBitmap(0,0,0,0,advanceX,advanceY,nullptr);return true;
}
}
