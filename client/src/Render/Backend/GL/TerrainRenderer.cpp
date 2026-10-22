// Sin PCH: todo acceso nuevo a GL queda dentro del backend.
#include "Platform/GL/glad.h"
#include "Platform/GLFunctions.h"
#include "Render/Backend/GL/TerrainRenderer.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderProfile.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <map>
#include <tuple>
extern "C" void DbgLogPublic(const char*);
namespace TerrainRendererGL {
namespace {
// DESVIACION R3a: conservar las dos pasadas antes de fusionarlas permite
// verificar geometría, arrays, agua y estado sin mezclar errores de redondeo.
GLuint program=0,vao=0,ebo=0,geometry=0,lighting=0,metadata=0;
int lightWidth=0,metadataRows=0;
int cachedMap=-1;float cachedScale=0;std::uint64_t epoch=0;
std::vector<float> heights,positions;
struct Page { GLuint name=0;int width=0,height=0,channels=0,filterMin=0,filterMag=0,wrapS=0,wrapT=0;std::vector<TerrainDraw::Texture> sources; };
std::vector<Page> pages;
struct Location { int page=0,layer=0; };
std::map<GLuint,Location> locations;
struct Operation { const TerrainDraw::Tile* tile;Location texture;int mode; };
const char* vertex=R"GLSL(#version 330 compatibility
uniform sampler2D geometryData,lightData,drawData;
uniform int firstInstance;
out vec4 shade;
out vec2 uv;
out float fogDistance;
flat out float layer;
vec4 fetchDraw(int i) { return texelFetch(drawData,ivec2(i%1024,i/1024),0); }
void main() {
    int corner=gl_VertexID;
    int record=(firstInstance+gl_InstanceID)*5;
    vec4 info=fetchDraw(record);
    int tile=int(info.x), mode=int(info.z);
    vec4 p=texelFetch(geometryData,ivec2((tile%256)*4+corner,tile/256),0);
    int grid=int(p.w);
    vec4 light=texelFetch(lightData,ivec2(grid%256,grid/256),0);
    shade=mode==2?vec4(light.aaa,1):vec4(light.rgb,mode==1?light.a:1);
    // El pipeline compatible limita el color primario antes de interpolarlo.
    shade=clamp(shade,0,1);
    vec4 tex=fetchDraw(record+1+corner);
    uv=(gl_TextureMatrix[0]*vec4(tex.xy,0,1)).xy;
    layer=info.y;
    vec4 pos=vec4(p.xyz,1);
    gl_Position=gl_ModelViewProjectionMatrix*pos;
    vec4 eye=gl_ModelViewMatrix*pos;
    fogDistance=abs(eye.z);gl_ClipVertex=eye;
}
)GLSL";
const char* fragment=R"GLSL(#version 330 compatibility
uniform sampler2DArray tiles;
uniform int fogMode;
in vec4 shade;
in vec2 uv;
in float fogDistance;
flat in float layer;
out vec4 color;
void main() {
    color=texture(tiles,vec3(uv,layer))*shade;
    float f=1;
    if(fogMode==1) f=(gl_Fog.end-fogDistance)*gl_Fog.scale;
    if(fogMode==2) f=exp(-gl_Fog.density*fogDistance);
    if(fogMode==3) { float d=gl_Fog.density*fogDistance;f=exp(-d*d); }
    if(fogMode!=0) color.rgb=mix(gl_Fog.color.rgb,color.rgb,clamp(f,0,1));
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
    glGenTextures(1,&geometry);glGenTextures(1,&lighting);glGenTextures(1,&metadata);
    Program(program);
    glUniform1i(glGetUniformLocation(program,"tiles"),0);glUniform1i(glGetUniformLocation(program,"geometryData"),1);
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
    auto wrap=[](int v){return v==GL_REPEAT||v==GL_CLAMP_TO_EDGE||v==GL_MIRRORED_REPEAT;};
    return wrap(p.wrapS)&&wrap(p.wrapT);
}
bool Same(const Page&a,const Page&b) { return std::tie(a.width,a.height,a.channels,a.filterMin,a.filterMag,a.wrapS,a.wrapT)==std::tie(b.width,b.height,b.channels,b.filterMin,b.filterMag,b.wrapS,b.wrapT); }
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
        Page description;if(!ReadSource(t,description))return false;
        int index=-1;for(int i=0;i<(int)pages.size();++i)if(Same(pages[i],description)&&(int)pages[i].sources.size()<limit){index=i;break;}
        if(index<0) {index=(int)pages.size();pages.push_back(description);}
        auto& page=pages[index];locations[t.handle]={index,(int)page.sources.size()};page.sources.push_back(t);dirty.push_back(index);
    }
    std::sort(dirty.begin(),dirty.end());dirty.erase(std::unique(dirty.begin(),dirty.end()),dirty.end());
    glActiveTexture(GL_TEXTURE0);
    for(int i:dirty) { auto& p=pages[i];if(!p.name)glGenTextures(1,&p.name);Bind(GL_TEXTURE_2D_ARRAY,p.name);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,p.filterMin);glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,p.filterMag);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,p.wrapS);glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,p.wrapT);
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
    if(guard.units[0].sampler || guard.active!=GL_TEXTURE0)return result;
    if(depth!=GL_LEQUAL||env!=GL_MODULATE||glIsEnabled(GL_LIGHTING)||glIsEnabled(GL_FRAMEBUFFER_SRGB)||sampleBuffers||matrix[3]!=0||matrix[7]!=0||matrix[15]!=1)return result;
    GLint textureUnits=0;glGetIntegerv(GL_MAX_TEXTURE_UNITS,&textureUnits);
    for(int i=1;i<textureUnits;++i) { glActiveTexture(GL_TEXTURE0+i);if(glIsEnabled(GL_TEXTURE_2D)||glIsEnabled(GL_TEXTURE_1D)||glIsEnabled(GL_TEXTURE_3D)||glIsEnabled(GL_TEXTURE_CUBE_MAP))return result; }
    if(!Init()){result.reason="Terreno: referencia (shader no disponible)";return result;}
    std::vector<Operation> ops;ops.reserve(tiles.size()*2);
    for(const auto& t:tiles) {
        if(t.x<0||t.y<0||t.x>=255||t.y>=255)return result;
        const unsigned ids[]{(unsigned)(t.y*256+t.x),(unsigned)(t.y*256+t.x+1),(unsigned)((t.y+1)*256+t.x+1),(unsigned)((t.y+1)*256+t.x)};
        const float x=t.x*f.scale,y=t.y*f.scale;
        const float xy[4][2]{{x,y},{x+f.scale,y},{x+f.scale,y+f.scale},{x,y+f.scale}};
        for(int c=0;c<4;++c)if(t.indices[c]!=ids[c]||t.positions[c][0]!=xy[c][0]||t.positions[c][1]!=xy[c][1]||t.positions[c][2]!=f.heights[ids[c]])return result;
        ops.push_back({&t,{},0});if(t.layer!=TerrainDraw::Layer::None)ops.push_back({&t,{},t.layer==TerrainDraw::Layer::Water?2:1});
    }
    if(cachedMap!=f.map){heights.clear();cachedMap=f.map;ClearPages();}
    if(epoch!=f.textureEpoch){ClearPages();epoch=f.textureEpoch;}
    if(!EnsureSources(ops,result.dataBytes)){ClearPages();result.reason="Terreno: referencia (textura no admitida)";return result;}
    for(auto& op:ops)op.texture=locations.at(op.mode==0?op.tile->base.handle:op.tile->upper.handle);
    if(heights.size()!=65536||cachedScale!=f.scale||std::memcmp(heights.data(),f.heights,65536*sizeof(float))) {
        heights.assign(f.heights,f.heights+65536);cachedScale=f.scale;positions.assign(65536*16,0);
        for(int y=0;y<255;++y)for(int x=0;x<255;++x) {
            const int ids[]{y*256+x,y*256+x+1,(y+1)*256+x+1,(y+1)*256+x};
            const float xx=x*f.scale,yy=y*f.scale;const float xy[4][2]{{xx,yy},{xx+f.scale,yy},{xx+f.scale,yy+f.scale},{xx,yy+f.scale}};
            for(int c=0;c<4;++c){float* p=&positions[(y*256+x)*16+c*4];p[0]=xy[c][0];p[1]=xy[c][1];p[2]=heights[ids[c]];p[3]=(float)ids[c];}
        }
        if(!TextureData(1,geometry,1024,256,positions.data(),true)){heights.clear();result.reason="Terreno: referencia (memoria geometria)";return result;}result.geometryBytes=positions.size()*sizeof(float);
    }
    // Luz/alpha son datos de la SIM ya congelada: no se modifican sus pools.
    int firstRow=255,lastRow=0;
    for(const auto& tile:tiles){firstRow=std::min(firstRow,tile.y);lastRow=std::max(lastRow,tile.y+1);}
    std::vector<float> lights((lastRow-firstRow+1)*256*4);
    for(int i=firstRow*256;i<(lastRow+1)*256;++i){std::copy_n(f.light+i*3,3,lights.data()+(i-firstRow*256)*4);lights[(i-firstRow*256)*4+3]=f.alpha[i];}
    glActiveTexture(GL_TEXTURE1);Bind(GL_TEXTURE_2D,geometry);
    glActiveTexture(GL_TEXTURE2);Bind(GL_TEXTURE_2D,lighting);
    if(!lightWidth){if(!TextureData(2,lighting,256,256,nullptr,true)){result.reason="Terreno: referencia (memoria luz)";return result;}lightWidth=256;}
    glTexSubImage2D(GL_TEXTURE_2D,0,0,firstRow,256,lastRow-firstRow+1,GL_RGBA,GL_FLOAT,lights.data());result.lightBytes=lights.size()*sizeof(float);
    const int rows=(int)((ops.size()*5+1023)/1024);std::vector<float> records(rows*1024*4);
    for(std::size_t i=0;i<ops.size();++i) {const auto& op=ops[i];float* p=records.data()+i*20;p[0]=(float)(op.tile->y*256+op.tile->x);p[1]=(float)op.texture.layer;p[2]=(float)op.mode;
        for(int c=0;c<4;++c){p[4+c*4]=op.tile->uv[c][op.mode?2:0];p[5+c*4]=op.tile->uv[c][op.mode?3:1];}
    }
    if(!TextureData(3,metadata,1024,rows,records.data(),metadataRows!=rows)){metadataRows=0;result.reason="Terreno: referencia (memoria instancias)";return result;}metadataRows=rows;result.dataBytes+=records.size()*sizeof(float);
    glActiveTexture(GL_TEXTURE0);Program(program);glBindVertexArray(vao);
    const bool originalFog=glIsEnabled(GL_FOG)!=0;glGetIntegerv(GL_FOG_MODE,&mode);
    const int fogMode=!originalFog?0:mode==GL_LINEAR?1:mode==GL_EXP?2:3;
    const GLint first=glGetUniformLocation(program,"firstInstance"),fog=glGetUniformLocation(program,"fogMode");
    for(std::size_t begin=0;begin<ops.size();) {
        std::size_t end=begin+1;const auto& op=ops[begin];while(end<ops.size()&&ops[end].mode==op.mode&&ops[end].texture.page==op.texture.page)++end;
        RenderProfileGL::SetEnabled(GL_BLEND,op.mode!=0);RenderProfileGL::SetEnabled(GL_ALPHA_TEST,op.mode==1);RenderProfileGL::SetEnabled(GL_CULL_FACE,op.mode==0);
        if(RenderProfile::active)RenderProfileGL::DepthWrite(op.mode!=2);glDepthMask(op.mode!=2);
        if(op.mode) {const GLenum src=op.mode==1?GL_SRC_ALPHA:GL_ONE,dst=op.mode==1?GL_ONE_MINUS_SRC_ALPHA:GL_ONE;RenderProfileGL::BlendFunction(src,dst);glBlendFunc(src,dst);}
        glUniform1i(fog,op.mode==2&&f.fogPolicy?0:fogMode);Bind(GL_TEXTURE_2D_ARRAY,pages[op.texture.page].name);
        glUniform1i(first,(int)begin);glDrawElementsInstanced(GL_TRIANGLES,6,GL_UNSIGNED_INT,nullptr,(GLsizei)(end-begin));RenderProfile::Draw(6*(end-begin),end-begin);
        ++result.draws;begin=end;
    }
    result.drawn=true;return result;
}
void Release() {
    if(!program)return;
    BufferedDraw::Suspend();ClearPages();glDeleteProgram(program);glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&ebo);
    const GLuint textures[]{geometry,lighting,metadata};glDeleteTextures(3,textures);program=vao=ebo=geometry=lighting=metadata=0;
    heights.clear();positions.clear();cachedMap=-1;epoch=0;lightWidth=metadataRows=0;
}
}
