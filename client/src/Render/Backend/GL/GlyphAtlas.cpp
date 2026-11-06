// Sin PCH: las funciones modernas se resuelven con glad.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#undef glTexImage2D
#define glTexImage2D(...) ::ProfileUploadGL::Image2D(glad_glTexImage2D,__VA_ARGS__)

#include "Render/BufferedDraw.h"
#include "Render/Gfx.h"
#include "Render/RenderState.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/GlyphAtlas.h"
#include "Render/Backend/Api.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace GlyphAtlas::GL {
namespace {
struct Glyph { int x=0,y=0,w=0,h=0,left=0,bottom=0; float advanceX=0,advanceY=0; };
struct Atlas { GLuint texture=0; int width=0,height=0; std::vector<Glyph> glyphs; };
std::unordered_map<unsigned,Atlas> atlases;

// glBitmap pasa sus fragmentos por el alpha test con el alpha del color raster.
bool AlphaPasses(Gfx::Compare f,float a,float r) {
    switch(f){
    case Gfx::Compare::Never:return false;case Gfx::Compare::Less:return a<r;case Gfx::Compare::Equal:return a==r;
    case Gfx::Compare::LessEqual:return a<=r;case Gfx::Compare::Greater:return a>r;case Gfx::Compare::NotEqual:return a!=r;
    case Gfx::Compare::GreaterEqual:return a>=r;case Gfx::Compare::Always:return true;
    }
    return true;
}
}
void Forget(unsigned key) {
    auto it=atlases.find(key);if (it==atlases.end()) return;
    BufferedDraw::Suspend();glDeleteTextures(1,&it->second.texture);atlases.erase(it);
}
void Release() { while (!atlases.empty()) Forget(atlases.begin()->first); }

bool Build(unsigned key,const Bitmap* glyphs,unsigned count) {
    Forget(key);
    if (!glyphs || !count) return false;
    Atlas atlas;atlas.glyphs.resize(count);
    int cellW=1,cellH=1;
    for (unsigned i=0;i<count;++i) {
        const auto& b=glyphs[i];auto& g=atlas.glyphs[i];
        g.advanceX=b.advanceX;g.advanceY=b.advanceY;
        if (b.width<=0 || b.height<=0 || b.coverage.size()<size_t(b.width)*b.height) continue;
        g.w=b.width;g.h=b.height;g.left=b.left;g.bottom=b.bottom;
        cellW=std::max(cellW,g.w+2);cellH=std::max(cellH,g.h+2);
    }
    const unsigned columns=16,rows=(count+columns-1)/columns;
    atlas.width=cellW*int(columns);atlas.height=cellH*int(rows);
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
    if (atlas.width>limit || atlas.height>limit) return false;
    // Blanco con la máscara en alpha: el color sale del texture env Modulate.
    std::vector<unsigned char> pixels(size_t(atlas.width)*atlas.height*4,255);
    for (size_t p=3;p<pixels.size();p+=4) pixels[p]=0;
    for (unsigned i=0;i<count;++i) {
        auto& g=atlas.glyphs[i];g.x=int(i%columns)*cellW+1;g.y=int(i/columns)*cellH+1;
        for (int y=0;y<g.h;++y) for (int x=0;x<g.w;++x)
            pixels[(size_t(g.y+y)*atlas.width+g.x+x)*4+3]=glyphs[i].coverage[size_t(y)*g.w+x];
    }
    BufferedDraw::Suspend();
    GLint active=0,bound=0,alignment=0,rowLength=0,skipRows=0,skipPixels=0,unpack=0;
    glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&unpack);
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);glGetIntegerv(GL_UNPACK_ROW_LENGTH,&rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS,&skipRows);glGetIntegerv(GL_UNPACK_SKIP_PIXELS,&skipPixels);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,0);glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
    glGenTextures(1,&atlas.texture);RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,atlas.texture);
    glBindTexture(GL_TEXTURE_2D,atlas.texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,atlas.width,atlas.height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    const bool ok=atlas.texture && glGetError()==GL_NO_ERROR;
    glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);glPixelStorei(GL_UNPACK_ROW_LENGTH,rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,skipRows);glPixelStorei(GL_UNPACK_SKIP_PIXELS,skipPixels);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack);
    glBindTexture(GL_TEXTURE_2D,bound);glActiveTexture(active);
    if (!ok) { if (atlas.texture) glDeleteTextures(1,&atlas.texture);return false; }
    atlases.emplace(key,std::move(atlas));return true;
}

bool Draw(unsigned key,float x,float y,const char* text,int count,int width,int height) {
    const auto it=atlases.find(key);if (it==atlases.end()) return false;
    RenderState::SetRasterPosition(x,y,0);
    const auto s=RenderState::Get();
    const auto& atlas=it->second;
    float advanceX=0,advanceY=0;
    for (int i=0;i<count;++i) {
        const unsigned c=static_cast<unsigned char>(text[i]);
        if (c<atlas.glyphs.size()) { advanceX+=atlas.glyphs[c].advanceX;advanceY+=atlas.glyphs[c].advanceY; }
    }
    using Cap=Gfx::Cap;
    const bool visible=s.rasterValid && count>0 && width>0 && height>0 &&
        (!RenderState::Enabled(Cap::AlphaTest) || AlphaPasses(s.alphaFunction,s.rasterColor[3],s.alphaReference));
    if (visible) {
        BufferedDraw::Flush();
        const auto mode=RenderState::MatrixMode();
        Gfx::PushAttrib(Gfx::AttribEnable|Gfx::AttribCurrent|Gfx::AttribViewport|Gfx::AttribTexture|Gfx::AttribColorBuffer);
        Gfx::MatrixMode(Gfx::Matrix::Texture);Gfx::PushMatrix();Gfx::LoadIdentity();
        Gfx::MatrixMode(Gfx::Matrix::Projection);Gfx::PushMatrix();Gfx::LoadIdentity();Gfx::Ortho2D(0,width,0,height);
        Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::PushMatrix();Gfx::LoadIdentity();Gfx::Viewport(0,0,width,height);
        Gfx::Enable(Cap::Texture2D);Gfx::BindTexture(atlas.texture);
        // glBitmap no depende del culling ni del modo de relleno de polígonos.
        Gfx::Disable(Cap::CullFace);Gfx::Disable(Cap::PolygonSmooth);
        Gfx::TexEnvMode(Gfx::TexEnv::Modulate);
        // Sólo los bits encendidos generan fragmentos; el resto se descarta.
        Gfx::Enable(Cap::AlphaTest);Gfx::AlphaFunc(Gfx::Compare::Greater,0.5f);
        // Ortho2D mapea z del ojo a -z normalizado: se conserva la profundidad raster.
        const float z=1-2*s.raster[2];
        BufferedDraw::Color4fv(s.rasterColor);BufferedDraw::Begin(BufferedDraw::Primitive::Quads);
        float penX=0,penY=0;
        for (int i=0;i<count;++i) {
            const unsigned c=static_cast<unsigned char>(text[i]);
            if (c>=atlas.glyphs.size()) continue;
            const auto& g=atlas.glyphs[c];
            if (g.w && g.h) {
                const float gx=std::floor(s.raster[0]+penX)+g.left,gy=std::floor(s.raster[1]+penY)+g.bottom;
                const float u=float(g.x)/atlas.width,v=float(g.y)/atlas.height;
                const float u1=float(g.x+g.w)/atlas.width,v1=float(g.y+g.h)/atlas.height;
                BufferedDraw::TexCoord2f(u,v);BufferedDraw::Vertex3f(gx,gy,z);
                BufferedDraw::TexCoord2f(u1,v);BufferedDraw::Vertex3f(gx+g.w,gy,z);
                BufferedDraw::TexCoord2f(u1,v1);BufferedDraw::Vertex3f(gx+g.w,gy+g.h,z);
                BufferedDraw::TexCoord2f(u,v1);BufferedDraw::Vertex3f(gx,gy+g.h,z);
            }
            penX+=g.advanceX;penY+=g.advanceY;
        }
        BufferedDraw::End();
        Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::PopMatrix();
        Gfx::MatrixMode(Gfx::Matrix::Projection);Gfx::PopMatrix();
        Gfx::MatrixMode(Gfx::Matrix::Texture);Gfx::PopMatrix();
        Gfx::MatrixMode(mode);Gfx::PopAttrib();
    }
    // Como glCallLists: la posición raster avanza aunque no se dibuje nada.
    RenderState::AdvanceRaster(advanceX,advanceY);return true;
}
}
