// Sin PCH: todo acceso nuevo a GL queda dentro del backend.
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

#include "Platform/GLFunctions.h"
#include "Render/Backend/GL/TerrainRenderer.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderProfile.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <tuple>
extern "C" void DbgLogPublic(const char*);
namespace TerrainRendererGL {
namespace {
// DESVIACION R3b: componer sólo cuando base y overlay cubren los mismos
// fragmentos. Las caras ocultas y estados especiales conservan dos pasadas.
GLuint program=0,vao=0,ebo=0,geometry=0,lighting=0,metadata=0;
int lightWidth=0,metadataRows=0;
int cachedMap=-1;float cachedScale=0;std::uint64_t epoch=0;
std::vector<float> heights,positions;
struct Page { GLuint name=0;int width=0,height=0,channels=0,filterMin=0,filterMag=0,wrapS=0,wrapT=0;std::array<float,4> border{};std::vector<TerrainDraw::Texture> sources; };
std::vector<Page> pages;
char textureFailure[160]="Terreno: referencia (textura no admitida)";
struct Location { int page=0,layer=0; };
std::map<GLuint,Location> locations;
struct Operation { const TerrainDraw::Tile* tile;Location texture;int mode;Location upper{}; };
const char* vertex=R"GLSL(#version 330 compatibility
layout(location=0) in vec4 position0;
layout(location=1) in vec4 position1;
layout(location=2) in vec4 position2;
layout(location=3) in vec4 position3;
uniform sampler2D lightData,drawData;
uniform int firstInstance;
out vec4 shade,upperShade;
out vec2 uv,upperUV;
out float fogDistance;
flat out vec2 layers;
flat out int operation;
vec4 fetchDraw(int i) { return texelFetch(drawData,ivec2(i%1024,i/1024),0); }
void main() {
    int corner=gl_VertexID;
    int record=(firstInstance+gl_InstanceID)*5;
    vec4 info=fetchDraw(record);
    operation=int(info.z);layers=info.xy;
    vec4 p=corner==0?position0:corner==1?position1:corner==2?position2:position3;
    int grid=int(p.w);
    vec4 light=texelFetch(lightData,ivec2(grid%256,grid/256),0);
    shade=operation==2?vec4(light.aaa,1):vec4(light.rgb,operation==1?light.a:1);
    upperShade=operation==4?vec4(light.aaa,1):light;
    // Limitar color primario antes de interpolar, igual que R3a.
    shade=clamp(shade,0,1);upperShade=clamp(upperShade,0,1);
    vec4 tex=fetchDraw(record+1+corner);
    uv=(gl_TextureMatrix[0]*vec4(tex.xy,0,1)).xy;
    upperUV=(gl_TextureMatrix[0]*vec4(tex.zw,0,1)).xy;
    vec4 pos=vec4(p.xyz,1);
    gl_Position=gl_ModelViewProjectionMatrix*pos;
    vec4 eye=gl_ModelViewMatrix*pos;
    // El cliente no activa planos de recorte de usuario (issue #240).
    fogDistance=abs(eye.z);
}
)GLSL";
const char* fragment=R"GLSL(#version 330 compatibility
uniform sampler2DArray tiles,upperTiles;
uniform int fogMode;
uniform bool waterNoFog;
uniform vec4 framebufferLevels;
in vec4 shade,upperShade;
in vec2 uv,upperUV;
in float fogDistance;
flat in vec2 layers;
flat in int operation;
out vec4 color;
vec4 fogged(vec4 c) {
    float f=1;
    if(fogMode==1) f=(gl_Fog.end-fogDistance)*gl_Fog.scale;
    if(fogMode==2) f=exp(-gl_Fog.density*fogDistance);
    if(fogMode==3) { float d=gl_Fog.density*fogDistance;f=exp(-d*d); }
    if(fogMode!=0)c.rgb=mix(gl_Fog.color.rgb,c.rgb,clamp(f,0,1));
    return c;
}

void main() {
    color=fogged(texture(tiles,vec3(uv,layers.x))*shade);
    if(operation<3)return;
    // La base ya habría pasado por UNORM antes del blend de la segunda capa.
    vec4 base=floor(clamp(color,0,1)*framebufferLevels+0.5)/max(framebufferLevels,vec4(1));
    if(framebufferLevels.a==0)base.a=1;
    vec4 upper=texture(upperTiles,vec3(upperUV,layers.y))*upperShade;
    if(operation==4) {
        if(!waterNoFog)upper=fogged(upper);
        color=base+upper;
    } else {
        upper=fogged(upper);
        // El CPU sólo admite esta variante cuando toda la capa pasa el test.
        // Si el umbral cruza el tile, se conserva el alpha test nativo R3a.
        color=upper*upper.a+base*(1-upper.a);
    }
}
)GLSL";
void Bind(unsigned target,GLuint name) { RenderProfileGL::ObserveTexture(target,name);glBindTexture(target,name); }
void Program(GLuint name) { RenderProfileGL::ObserveProgram(name);glUseProgram(name); }
struct Guard {
    GLint oldProgram=0,oldArray=0,oldBuffer=0,active=0,unpack=0,pack=0;
    struct Unit { GLint two=0,array=0,sampler=0; } units[4];
    Guard() {
        BufferedDraw::Suspend();
        glGetIntegerv(GL_CURRENT_PROGRAM,&oldProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&unpack);glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&pack);
        glPushAttrib(GL_ALL_ATTRIB_BITS);glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        for(int i=0;i<4;++i) { glActiveTexture(GL_TEXTURE0+i);
            glGetIntegerv(GL_TEXTURE_BINDING_2D,&units[i].two);glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY,&units[i].array);
            glGetIntegerv(GL_SAMPLER_BINDING,&units[i].sampler);glBindSampler(i,0);
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glPixelStorei(GL_PACK_ALIGNMENT,1);glPixelStorei(GL_PACK_ROW_LENGTH,0);glPixelStorei(GL_PACK_SKIP_PIXELS,0);glPixelStorei(GL_PACK_SKIP_ROWS,0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,0);glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,0);glPixelStorei(GL_UNPACK_SKIP_IMAGES,0);
        glPixelStorei(GL_UNPACK_SWAP_BYTES,GL_FALSE);
    }
    ~Guard() {
        for(int i=0;i<4;++i) { glActiveTexture(GL_TEXTURE0+i);Bind(GL_TEXTURE_2D,units[i].two);Bind(GL_TEXTURE_2D_ARRAY,units[i].array);glBindSampler(i,units[i].sampler); }
        glActiveTexture(active);Program(oldProgram);glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack);glBindBuffer(GL_PIXEL_PACK_BUFFER,pack);glPopClientAttrib();RenderProfileGL::PopAttributes();
    }
};
bool Init() {
    if(program)return true;
    GLuint shaders[2]{glCreateShader(GL_VERTEX_SHADER),glCreateShader(GL_FRAGMENT_SHADER)};
    const char* sources[2]{vertex,fragment};bool success=true;
    for(int i=0;i<2;++i) { glShaderSource(shaders[i],1,&sources[i],nullptr);glCompileShader(shaders[i]);GLint ok=0;glGetShaderiv(shaders[i],GL_COMPILE_STATUS,&ok);
        if(!ok) { char log[2048]{};glGetShaderInfoLog(shaders[i],sizeof(log),nullptr,log);DbgLogPublic(log);success=false; }
    }
    if(success) { program=glCreateProgram();for(auto s:shaders)glAttachShader(program,s);glLinkProgram(program);GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);success=ok!=0;
        if(!success) { char log[2048]{};glGetProgramInfoLog(program,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteProgram(program);program=0; }
    }
    for(auto s:shaders)glDeleteShader(s);if(!success)return false;
    glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenBuffers(1,&ebo);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    const unsigned indices[]{0,1,2,0,2,3};glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    glGenBuffers(1,&geometry);glBindBuffer(GL_ARRAY_BUFFER,geometry);
    for(GLuint c=0;c<4;++c){glEnableVertexAttribArray(c);glVertexAttribDivisor(c,1);}
    glGenTextures(1,&lighting);glGenTextures(1,&metadata);
    Program(program);
    glUniform1i(glGetUniformLocation(program,"tiles"),0);glUniform1i(glGetUniformLocation(program,"upperTiles"),1);
    glUniform1i(glGetUniformLocation(program,"lightData"),2);glUniform1i(glGetUniformLocation(program,"drawData"),3);
    return true;
}
bool TextureData(int unit,GLuint name,int w,int h,const float* data,bool create) {
    glActiveTexture(GL_TEXTURE0+unit);Bind(GL_TEXTURE_2D,name);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    if(create)glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,w,h,0,GL_RGBA,GL_FLOAT,data);
    else glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,GL_RGBA,GL_FLOAT,data);
    GLint actualW=0,actualH=0;glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&actualW);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&actualH);
    return actualW==w&&actualH==h;
}
void ClearPages() { for(auto& page:pages)glDeleteTextures(1,&page.name);pages.clear();locations.clear(); }
bool ReadSource(const TerrainDraw::Texture& source,Page& p) {
    if(!source.handle||!source.pixels||source.width<=0||source.height<=0||(source.channels!=3&&source.channels!=4))return false;
    glActiveTexture(GL_TEXTURE0);Bind(GL_TEXTURE_2D,source.handle);
    GLint w=0,h=0,r=0,g=0,b=0,a=0,base=0,internal=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_RED_SIZE,&r);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_GREEN_SIZE,&g);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_BLUE_SIZE,&b);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_ALPHA_SIZE,&a);
    glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,&base);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT,&internal);
    if(w!=source.width||h!=source.height||r!=8||g!=8||b!=8||a!=(source.channels==4?8:0)||base!=0)return false;
    if(internal!=3&&internal!=4&&internal!=GL_RGB8&&internal!=GL_RGBA8)return false;
    // Las texturas anisotrópicas requieren incorporar ese sampler; no degradarlo.
    GLint extensions=0;glGetIntegerv(GL_NUM_EXTENSIONS,&extensions);
    for(int i=0;i<extensions;++i)if(std::strcmp((const char*)glGetStringi(GL_EXTENSIONS,i),"GL_EXT_texture_filter_anisotropic")==0) {
        GLfloat anisotropy=1;glGetTexParameterfv(GL_TEXTURE_2D,0x84FE,&anisotropy);if(anisotropy!=1)return false;break;
    }
    p.width=w;p.height=h;p.channels=source.channels;
    glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&p.filterMin);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&p.filterMag);
    glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,&p.wrapS);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,&p.wrapT);
    GLint compare=0,swizzle[4]{};glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,&compare);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_SWIZZLE_RGBA,swizzle);
    const int identity[]{GL_RED,GL_GREEN,GL_BLUE,GL_ALPHA};
    if(compare!=GL_NONE||std::memcmp(identity,swizzle,sizeof(identity)))return false;
    if(p.filterMin!=GL_NEAREST&&p.filterMin!=GL_LINEAR)return false;
    glGetTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,p.border.data());
    auto wrap=[](int v){return v==GL_CLAMP||v==GL_REPEAT||v==GL_CLAMP_TO_EDGE||v==GL_MIRRORED_REPEAT;};
    return wrap(p.wrapS)&&wrap(p.wrapT);
}
// Orden residente idéntico al recorrido de bloques 4x4 del 0.97k. Los huecos
// de culling cortan el lote: no se compactan/reenvían posiciones por cuadro.
std::size_t ResidentIndex(int x,int y) { return ((y/4)*64+x/4)*16+(y%4)*4+x%4; }
bool Same(const Page&a,const Page&b) { return std::tie(a.width,a.height,a.channels,a.filterMin,a.filterMag,a.wrapS,a.wrapT,a.border)==std::tie(b.width,b.height,b.channels,b.filterMin,b.filterMag,b.wrapS,b.wrapT,b.border); }
bool EnsureSources(const std::vector<Operation>& ops,std::uint64_t& bytes) {
    GLint limit=0;glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS,&limit);
    std::vector<int> dirty;
    for(const auto& op:ops) {
        const auto& t=op.mode==0?op.tile->base:op.tile->upper;
        auto found=locations.find(t.handle);
        if(found!=locations.end()) {
            const auto& old=pages[found->second.page].sources[found->second.layer];
            if(old.pixels!=t.pixels||old.width!=t.width||old.height!=t.height||old.channels!=t.channels)return false;
            continue;
        }
        Page description;if(!ReadSource(t,description)) {
            std::snprintf(textureFailure,sizeof(textureFailure),"Terreno: referencia (slot %d %dx%d C%d min=%d wrap=%d/%d)",t.slot,t.width,t.height,t.channels,description.filterMin,description.wrapS,description.wrapT);return false;
        }
        int index=-1;for(int i=0;i<(int)pages.size();++i)if(Same(pages[i],description)&&(int)pages[i].sources.size()<limit){index=i;break;}
        if(index<0) {index=(int)pages.size();pages.push_back(description);}
        auto& page=pages[index];locations[t.handle]={index,(int)page.sources.size()};page.sources.push_back(t);dirty.push_back(index);
    }
    std::sort(dirty.begin(),dirty.end());dirty.erase(std::unique(dirty.begin(),dirty.end()),dirty.end());
    glActiveTexture(GL_TEXTURE0);
    for(int i:dirty) { auto& p=pages[i];if(!p.name)glGenTextures(1,&p.name);Bind(GL_TEXTURE_2D_ARRAY,p.name);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,p.filterMin);glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,p.filterMag);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,p.wrapS);glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,p.wrapT);
        glTexParameterfv(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_BORDER_COLOR,p.border.data());
        glTexImage3D(GL_TEXTURE_2D_ARRAY,0,p.channels==4?GL_RGBA8:GL_RGB8,p.width,p.height,(int)p.sources.size(),0,p.channels==4?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,nullptr);
        GLint layers=0;glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY,0,GL_TEXTURE_DEPTH,&layers);if(layers!=(int)p.sources.size())return false;
        bytes+=(std::uint64_t)p.width*p.height*p.channels*p.sources.size();
        // Importar la imagen real, incluso si otro emisor la actualizó sin
        // modificar el Buffer del bitmap. Sólo al crear/invalidar una página;
        // no hay readback en el camino estable de cada cuadro.
        std::vector<unsigned char> image((std::size_t)p.width*p.height*p.channels);
        for(int j=0;j<(int)p.sources.size();++j) {
            Bind(GL_TEXTURE_2D,p.sources[j].handle);
            glGetTexImage(GL_TEXTURE_2D,0,p.channels==4?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,image.data());
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY,0,0,0,j,p.width,p.height,1,p.channels==4?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,image.data());
        }
    }
    return true;
}
// Las proyecciones dudosas no se aproximan: conservan el dibujo de dos capas.
bool FrontTile(const TerrainDraw::Tile& t,const float* view,const float* projection,int front,int cull) {
    if(cull==GL_FRONT_AND_BACK)return false;
    double clip[4][2]{};
    for(int c=0;c<4;++c) {
        double eye[4]{};for(int row=0;row<4;++row) {
            eye[row]=view[12+row];for(int k=0;k<3;++k)eye[row]+=view[k*4+row]*double(t.positions[c][k]);
        }
        double p[4]{};for(int row=0;row<4;++row)for(int k=0;k<4;++k)p[row]+=projection[k*4+row]*eye[k];
        if(!std::isfinite(p[3])||p[3]<=1e-6)return false;
        clip[c][0]=p[0]/p[3];clip[c][1]=p[1]/p[3];
    }
    const int triangles[2][3]{{0,1,2},{0,2,3}};
    for(const auto& tri:triangles) {
        const auto* a=clip[tri[0]];const auto* b=clip[tri[1]];const auto* c=clip[tri[2]];
        double area=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);
        if(!std::isfinite(area)||std::abs(area)<1e-6)return false;
        const bool facing=(area>0)==(front==GL_CCW);
        if((cull==GL_BACK&&!facing)||(cull==GL_FRONT&&facing))return false;
    }
    return true;
}
// Resultado constante del alpha test en todo el tile. El margen conserva
// el camino nativo alrededor del umbral, incluso con redondeo de 8 bits.
// Las texturas RGBA usan el intervalo conservador [0,1], sin leer la GPU.
int AlphaCoverage(const TerrainDraw::Tile& t,const TerrainDraw::FrameData& frame,int function,float reference) {
    if(function==GL_ALWAYS)return 1;if(function==GL_NEVER)return -1;
    float lo=1,hi=0;
    for(unsigned i:t.indices){const float a=std::max(0.f,std::min(1.f,frame.alpha[i]));lo=std::min(lo,a);hi=std::max(hi,a);}
    if(t.upper.channels!=3)lo=0;
    constexpr float margin=2.f/255.f;
    const bool below=hi<reference-margin,above=lo>reference+margin;
    switch(function) {
    case GL_LESS:case GL_LEQUAL:return below?1:above?-1:0;
    case GL_GREATER:case GL_GEQUAL:return above?1:below?-1:0;
    case GL_EQUAL:return below||above?-1:0;
    case GL_NOTEQUAL:return below||above?1:0;
    default:return 0;
    }
}
int Pipeline(int mode) { return mode>=3?3:mode; }

}
TerrainDraw::Result Draw(const TerrainDraw::FrameData& f,const std::vector<TerrainDraw::Tile>& tiles) {
    TerrainDraw::Result result;result.reason="Terreno: referencia (estado no admitido)";
    if(!Platform::GL33Ready()||!f.heights||!f.light||!f.alpha)return result;
    if(tiles.empty()){result.drawn=true;return result;}
    Guard guard;
    // DESVIACION: los estados especiales conservan la pasada completa actual;
    // nunca se redibuja una pasada que ya haya enviado una parte nueva.
    GLint depth=0,env=0,mode=0,sampleBuffers=0;GLfloat matrix[16]{};
    glGetIntegerv(GL_DEPTH_FUNC,&depth);glGetIntegerv(GL_SAMPLE_BUFFERS,&sampleBuffers);
    glActiveTexture(GL_TEXTURE0);glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,&env);glGetFloatv(GL_TEXTURE_MATRIX,matrix);
    if(guard.units[0].sampler){result.reason="Terreno: referencia (sampler ajeno)";return result;}
    if(guard.active!=GL_TEXTURE0){result.reason="Terreno: referencia (unidad activa)";return result;}
    if(depth!=GL_LEQUAL){result.reason="Terreno: referencia (depth distinto)";return result;}
    if(env!=GL_MODULATE){result.reason="Terreno: referencia (texture env)";return result;}
    if(glIsEnabled(GL_LIGHTING)){result.reason="Terreno: referencia (lighting)";return result;}
    if(glIsEnabled(GL_FRAMEBUFFER_SRGB)){result.reason="Terreno: referencia (sRGB)";return result;}
    if(sampleBuffers){result.reason="Terreno: referencia (multisampling)";return result;}
    if(matrix[3]!=0||matrix[7]!=0||matrix[15]!=1){result.reason="Terreno: referencia (UV proyectiva)";return result;}
    GLint textureUnits=0;glGetIntegerv(GL_MAX_TEXTURE_UNITS,&textureUnits);
    for(int i=1;i<textureUnits;++i) { glActiveTexture(GL_TEXTURE0+i);if(glIsEnabled(GL_TEXTURE_2D)||glIsEnabled(GL_TEXTURE_1D)||glIsEnabled(GL_TEXTURE_3D)||glIsEnabled(GL_TEXTURE_CUBE_MAP)){result.reason="Terreno: referencia (multitextura)";return result;} }
    if(!Init()){result.reason="Terreno: referencia (shader no disponible)";return result;}
    std::vector<Operation> ops;ops.reserve(tiles.size()*2);
    for(const auto& t:tiles) {
        if(t.x<0||t.y<0||t.x>=255||t.y>=255){result.reason="Terreno: referencia (borde de grilla)";return result;}
        const unsigned ids[]{(unsigned)(t.y*256+t.x),(unsigned)(t.y*256+t.x+1),(unsigned)((t.y+1)*256+t.x+1),(unsigned)((t.y+1)*256+t.x)};
        const float x=t.x*f.scale,y=t.y*f.scale;
        const float xy[4][2]{{x,y},{x+f.scale,y},{x+f.scale,y+f.scale},{x,y+f.scale}};
        for(int c=0;c<4;++c)if(t.indices[c]!=ids[c]||t.positions[c][0]!=xy[c][0]||t.positions[c][1]!=xy[c][1]||t.positions[c][2]!=f.heights[ids[c]]){result.reason="Terreno: referencia (geometria distinta)";return result;}
        ops.push_back({&t,{},0});if(t.layer!=TerrainDraw::Layer::None)ops.push_back({&t,{},t.layer==TerrainDraw::Layer::Water?2:1});
    }
    if(cachedMap!=f.map){heights.clear();cachedMap=f.map;ClearPages();}
    if(epoch!=f.textureEpoch){ClearPages();epoch=f.textureEpoch;}
    if(!EnsureSources(ops,result.dataBytes)){ClearPages();result.reason=textureFailure;return result;}
    GLint bits[4]{},equationRGB=0,equationAlpha=0,polygon[2]{},front=0,cull=0,alphaFunction=0;
    const GLenum bitNames[]{GL_RED_BITS,GL_GREEN_BITS,GL_BLUE_BITS,GL_ALPHA_BITS};
    for(int i=0;i<4;++i)glGetIntegerv(bitNames[i],bits+i);
    glGetIntegerv(GL_BLEND_EQUATION_RGB,&equationRGB);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&equationAlpha);
    glGetIntegerv(GL_POLYGON_MODE,polygon);glGetIntegerv(GL_FRONT_FACE,&front);glGetIntegerv(GL_CULL_FACE_MODE,&cull);
    glGetIntegerv(GL_ALPHA_TEST_FUNC,&alphaFunction);GLfloat alphaReference=0;glGetFloatv(GL_ALPHA_TEST_REF,&alphaReference);
    float view[16]{},projection[16]{};glGetFloatv(GL_MODELVIEW_MATRIX,view);glGetFloatv(GL_PROJECTION_MATRIX,projection);
    const bool fusion=f.compose&&bits[0]==8&&bits[1]==8&&bits[2]==8&&(bits[3]==0||bits[3]==8)
        &&equationRGB==GL_FUNC_ADD&&equationAlpha==GL_FUNC_ADD&&polygon[0]==GL_FILL&&polygon[1]==GL_FILL
        &&!glIsEnabled(GL_STENCIL_TEST)&&!glIsEnabled(GL_POLYGON_SMOOTH)&&!glIsEnabled(GL_COLOR_LOGIC_OP)
        &&!glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE)&&!glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE);
    ops.clear();
    for(const auto& t:tiles) {
        const auto base=locations.at(t.base.handle);
        if(t.layer==TerrainDraw::Layer::None){ops.push_back({&t,base,0});continue;}
        const auto upper=locations.at(t.upper.handle);
        const int coverage=t.layer==TerrainDraw::Layer::Water?1:AlphaCoverage(t,f,alphaFunction,alphaReference);
        if(fusion&&coverage<0) {
            ops.push_back({&t,base,0});++result.rejectedTiles;
        } else if(fusion&&coverage>0&&FrontTile(t,view,projection,front,cull)) {
            ops.push_back({&t,base,t.layer==TerrainDraw::Layer::Water?4:3,upper});++result.composedTiles;
        } else {
            ops.push_back({&t,base,0});ops.push_back({&t,upper,t.layer==TerrainDraw::Layer::Water?2:1});++result.splitTiles;
            if(!fusion)++result.splitState;else if(coverage==0)++result.splitAlpha;else ++result.splitFace;
        }
    }
    if(heights.size()!=65536||cachedScale!=f.scale||std::memcmp(heights.data(),f.heights,65536*sizeof(float))) {
        heights.assign(f.heights,f.heights+65536);cachedScale=f.scale;positions.assign(65536*16,0);
        for(int y=0;y<255;++y)for(int x=0;x<255;++x) {
            const int ids[]{y*256+x,y*256+x+1,(y+1)*256+x+1,(y+1)*256+x};
            const float xx=x*f.scale,yy=y*f.scale;const float xy[4][2]{{xx,yy},{xx+f.scale,yy},{xx+f.scale,yy+f.scale},{xx,yy+f.scale}};
            for(int c=0;c<4;++c){float* p=&positions[ResidentIndex(x,y)*16+c*4];p[0]=xy[c][0];p[1]=xy[c][1];p[2]=heights[ids[c]];p[3]=(float)ids[c];}
        }
        glBindBuffer(GL_ARRAY_BUFFER,geometry);glBufferData(GL_ARRAY_BUFFER,positions.size()*sizeof(float),positions.data(),GL_STATIC_DRAW);
        GLint size=0;glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&size);
        if(size!=(GLint)(positions.size()*sizeof(float))){heights.clear();result.reason="Terreno: referencia (memoria geometria)";return result;}
        result.geometryBytes=positions.size()*sizeof(float);
    }
    // Luz/alpha son datos de la SIM ya congelada: no se modifican sus pools.
    int firstRow=255,lastRow=0;
    for(const auto& tile:tiles){firstRow=std::min(firstRow,tile.y);lastRow=std::max(lastRow,tile.y+1);}
    std::vector<float> lights((lastRow-firstRow+1)*256*4);
    for(int i=firstRow*256;i<(lastRow+1)*256;++i){std::copy_n(f.light+i*3,3,lights.data()+(i-firstRow*256)*4);lights[(i-firstRow*256)*4+3]=f.alpha[i];}
    glActiveTexture(GL_TEXTURE2);Bind(GL_TEXTURE_2D,lighting);
    if(!lightWidth){if(!TextureData(2,lighting,256,256,nullptr,true)){result.reason="Terreno: referencia (memoria luz)";return result;}lightWidth=256;}
    glTexSubImage2D(GL_TEXTURE_2D,0,0,firstRow,256,lastRow-firstRow+1,GL_RGBA,GL_FLOAT,lights.data());result.lightBytes=lights.size()*sizeof(float);
    const int rows=(int)((ops.size()*5+1023)/1024);std::vector<float> records(rows*1024*4);
    for(std::size_t i=0;i<ops.size();++i) {const auto& op=ops[i];float* p=records.data()+i*20;p[0]=(float)op.texture.layer;p[1]=(float)op.upper.layer;p[2]=(float)op.mode;
        for(int c=0;c<4;++c){
            const int offset=op.mode>0&&op.mode<3?2:0;
            p[4+c*4]=op.tile->uv[c][offset];p[5+c*4]=op.tile->uv[c][offset+1];
            if(op.mode>=3){p[6+c*4]=op.tile->uv[c][2];p[7+c*4]=op.tile->uv[c][3];}
        }
    }
    if(!TextureData(3,metadata,1024,rows,records.data(),metadataRows!=rows)){metadataRows=0;result.reason="Terreno: referencia (memoria instancias)";return result;}metadataRows=rows;result.dataBytes+=records.size()*sizeof(float);
    glActiveTexture(GL_TEXTURE0);Program(program);glBindVertexArray(vao);
    const bool originalFog=glIsEnabled(GL_FOG)!=0;glGetIntegerv(GL_FOG_MODE,&mode);
    const int fogMode=!originalFog?0:mode==GL_LINEAR?1:mode==GL_EXP?2:3;
    const GLint first=glGetUniformLocation(program,"firstInstance"),fog=glGetUniformLocation(program,"fogMode");
    glUniform1i(glGetUniformLocation(program,"waterNoFog"),f.fogPolicy?1:0);
    glUniform4f(glGetUniformLocation(program,"framebufferLevels"),255,255,255,bits[3]?255.f:0.f);
    int lastMode=-1,lastPage=-1,lastUpper=-1;
    for(std::size_t begin=0;begin<ops.size();) {
        std::size_t end=begin+1;const auto& op=ops[begin];while(end<ops.size()&&Pipeline(ops[end].mode)==Pipeline(op.mode)&&ops[end].texture.page==op.texture.page
            &&(op.mode<3||ops[end].upper.page==op.upper.page)
            &&ResidentIndex(ops[end].tile->x,ops[end].tile->y)==ResidentIndex(op.tile->x,op.tile->y)+(end-begin))++end;
        // Este estado sólo vive dentro de la pasada; no toca el caché legacy.
        if(lastMode!=Pipeline(op.mode)) {
            RenderProfileGL::SetEnabled(GL_BLEND,op.mode==1||op.mode==2);RenderProfileGL::SetEnabled(GL_ALPHA_TEST,op.mode==1);RenderProfileGL::SetEnabled(GL_CULL_FACE,op.mode==0||op.mode>=3);
            if(RenderProfile::active)RenderProfileGL::DepthWrite(op.mode!=2);glDepthMask(op.mode!=2);
            if(op.mode==1||op.mode==2) {const GLenum src=op.mode==1?GL_SRC_ALPHA:GL_ONE,dst=op.mode==1?GL_ONE_MINUS_SRC_ALPHA:GL_ONE;
                if(RenderProfile::active)RenderProfileGL::BlendFunction(src,dst);glBlendFunc(src,dst);}
            glUniform1i(fog,op.mode==2&&f.fogPolicy?0:fogMode);lastMode=Pipeline(op.mode);
        }
        if(lastPage!=op.texture.page){Bind(GL_TEXTURE_2D_ARRAY,pages[op.texture.page].name);lastPage=op.texture.page;}
        if(op.mode>=3&&lastUpper!=op.upper.page){
            glActiveTexture(GL_TEXTURE1);Bind(GL_TEXTURE_2D_ARRAY,pages[op.upper.page].name);glActiveTexture(GL_TEXTURE0);lastUpper=op.upper.page;
        }
        glBindBuffer(GL_ARRAY_BUFFER,geometry);
        for(GLuint c=0;c<4;++c)glVertexAttribPointer(c,4,GL_FLOAT,GL_FALSE,16*sizeof(float),
            reinterpret_cast<const void*>((ResidentIndex(op.tile->x,op.tile->y)*16+c*4)*sizeof(float)));
        glUniform1i(first,(int)begin);glDrawElementsInstanced(GL_TRIANGLES,6,GL_UNSIGNED_INT,nullptr,(GLsizei)(end-begin));RenderProfile::Draw(6*(end-begin),end-begin);
        ++result.draws;begin=end;
    }
    result.drawn=true;return result;
}
// DESVIACION: los uploads de texto/UI no invalidan las páginas del terreno.
// El caché vacío no consulta GL: la ruta actual apagada no paga este costo.
bool SourceTexture(std::uint32_t handle) { return locations.find(handle)!=locations.end(); }
bool BoundSourceTexture() {
    if(locations.empty())return false;
    GLint handle=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&handle);return SourceTexture(handle);
}
void Release() {
    if(!program)return;
    BufferedDraw::Suspend();ClearPages();glDeleteProgram(program);glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&ebo);
    glDeleteBuffers(1,&geometry);const GLuint textures[]{lighting,metadata};glDeleteTextures(2,textures);program=vao=ebo=geometry=lighting=metadata=0;
    heights.clear();positions.clear();cachedMap=-1;epoch=0;lightWidth=metadataRows=0;
}
}
