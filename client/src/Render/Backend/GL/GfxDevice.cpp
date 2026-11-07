// Backend GL de GfxDevice: lleva al dispositivo el estado de Gfx que existe en
// OpenGL 3.3 core (el resto vive en RenderState y lo aplican los shaders).
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/RenderState.h"
#include <algorithm>
#include <cstring>

namespace GfxDevice::GL {
using namespace Gfx;
namespace {
GLenum ToGL(Factor factor) {
    switch (factor) {
    case Factor::Zero: return GL_ZERO;
    case Factor::One: return GL_ONE;
    case Factor::SrcColor: return GL_SRC_COLOR;
    case Factor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
    case Factor::SrcAlpha: return GL_SRC_ALPHA;
    case Factor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    case Factor::DstAlpha: return GL_DST_ALPHA;
    case Factor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
    case Factor::DstColor: return GL_DST_COLOR;
    case Factor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
    case Factor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
    }
    return GL_ONE;
}
GLenum ToGL(Compare function) {
    switch (function) {
    case Compare::Never: return GL_NEVER;
    case Compare::Less: return GL_LESS;
    case Compare::Equal: return GL_EQUAL;
    case Compare::LessEqual: return GL_LEQUAL;
    case Compare::Greater: return GL_GREATER;
    case Compare::NotEqual: return GL_NOTEQUAL;
    case Compare::GreaterEqual: return GL_GEQUAL;
    case Compare::Always: return GL_ALWAYS;
    }
    return GL_ALWAYS;
}
GLenum ToGL(Format format) {
    switch (format) {
    case Format::RGB: return GL_RGB;
    case Format::RGBA: return GL_RGBA;
    }
    return GL_RGBA;
}
GLenum ToGL(TexParam parameter) {
    switch (parameter) {
    case TexParam::MinFilter: return GL_TEXTURE_MIN_FILTER;
    case TexParam::MagFilter: return GL_TEXTURE_MAG_FILTER;
    case TexParam::WrapS: return GL_TEXTURE_WRAP_S;
    case TexParam::WrapT: return GL_TEXTURE_WRAP_T;
    }
    return GL_TEXTURE_MIN_FILTER;
}
GLint ToGL(TexValue value) {
    switch (value) {
    case TexValue::Nearest: return GL_NEAREST;
    case TexValue::Linear: return GL_LINEAR;
    case TexValue::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case TexValue::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
    case TexValue::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
    case TexValue::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
    case TexValue::Repeat: return GL_REPEAT;
    // DESVIACION: GL_CLAMP no existe en core. Con NEAREST nunca muestrea el
    // borde y los drivers actuales ya lo tratan como CLAMP_TO_EDGE.
    case TexValue::Clamp: return GL_CLAMP_TO_EDGE;
    case TexValue::ClampToEdge: return GL_CLAMP_TO_EDGE;
    }
    return GL_NEAREST;
}
// Capacidades que existen en el dispositivo; el resto sólo vive en RenderState.
GLenum DeviceCap(Cap cap) {
    switch (cap) {
    case Cap::Blend: return GL_BLEND;
    case Cap::DepthTest: return GL_DEPTH_TEST;
    case Cap::CullFace: return GL_CULL_FACE;
    case Cap::ScissorTest: return GL_SCISSOR_TEST;
    case Cap::StencilTest: return GL_STENCIL_TEST;
    case Cap::Dither: return GL_DITHER;
    case Cap::PolygonSmooth: return GL_POLYGON_SMOOTH;
    case Cap::ColorLogicOp: return GL_COLOR_LOGIC_OP;
    default: return 0;
    }
}

// Anisotropía máxima del driver (GL_EXT/ARB_texture_filter_anisotropic); 0 sin ella.
float MaxAnisotropy() {
    static float value=-1.f;
    if (value>=0) return value;
    value=0;
    GLint count=0;glGetIntegerv(GL_NUM_EXTENSIONS,&count);
    for (GLint i=0;i<count;++i) {
        const char* name=reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS,GLuint(i)));
        if (name && (!std::strcmp(name,"GL_EXT_texture_filter_anisotropic") || !std::strcmp(name,"GL_ARB_texture_filter_anisotropic"))) {
            glGetFloatv(0x84FF,&value); // GL_MAX_TEXTURE_MAX_ANISOTROPY
            value=std::min(value,16.f);
            break;
        }
    }
    return value;
}
}
void Reset() {
    for (int i = 0; i < RenderState::CapCount; ++i)
        if (const GLenum device = DeviceCap(static_cast<Cap>(i))) { if (RenderState::Get().caps[i]) glEnable(device); else glDisable(device); }
    glBlendFunc(GL_ONE, GL_ZERO);glDepthFunc(GL_LESS);glDepthMask(GL_TRUE);glBindTexture(GL_TEXTURE_2D, 0);
    GLint viewport[4]{};glGetIntegerv(GL_VIEWPORT, viewport);
    std::memcpy(RenderState::Mutable().viewport, viewport, sizeof(viewport));
}
void ApplyCap(Cap cap, bool on) {
    if (const GLenum device = DeviceCap(cap)) { if (on) glEnable(device); else glDisable(device); }
}
void ApplyBlend(Factor source, Factor destination) { glBlendFunc(ToGL(source), ToGL(destination)); }
void ApplyDepthFunc(Compare function) { glDepthFunc(ToGL(function)); }
void ApplyDepthMask(bool write) { glDepthMask(write ? GL_TRUE : GL_FALSE); }
void ApplyTexture(Texture texture) { glBindTexture(GL_TEXTURE_2D, texture); }
void ApplyViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }
void ClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
void Clear(unsigned clearMask) {
    GLbitfield bits = 0;
    if (clearMask & ClearColorBuffer) bits |= GL_COLOR_BUFFER_BIT;
    if (clearMask & ClearDepthBuffer) bits |= GL_DEPTH_BUFFER_BIT;
    if (clearMask & ClearStencilBuffer) bits |= GL_STENCIL_BUFFER_BIT;
    glClear(bits);
}
void Flush() { glFlush(); }
void ClearErrors() { while (glGetError() != GL_NO_ERROR) {} }
// Posproceso (FXAA y nitidez): copia el viewport a una textura y lo redibuja
// con el filtro. Guarda y restaura el estado de GL que toca.
namespace Post {
const char* const Vertex = R"GLSL(#version 330 core
void main(){ vec2 p=vec2(gl_VertexID==1?3.0:-1.0,gl_VertexID==2?3.0:-1.0); gl_Position=vec4(p,0.0,1.0); }
)GLSL";
const char* const Sharpen = R"GLSL(#version 330 core
uniform sampler2D uSource; uniform float uAmount; uniform ivec2 uOrigin;
out vec4 color;
vec3 at(ivec2 p){ return texelFetch(uSource,clamp(p,ivec2(0),textureSize(uSource,0)-1),0).rgb; }
void main(){
    ivec2 p=ivec2(gl_FragCoord.xy)-uOrigin;
    vec3 a=at(p+ivec2(-1,-1)),b=at(p+ivec2(0,-1)),c=at(p+ivec2(1,-1));
    vec3 d=at(p+ivec2(-1,0)),e=at(p),f=at(p+ivec2(1,0));
    vec3 g=at(p+ivec2(-1,1)),h=at(p+ivec2(0,1)),i=at(p+ivec2(1,1));
    vec3 mn=min(min(min(d,e),min(f,b)),h); mn+=min(mn,min(min(a,c),min(g,i)));
    vec3 mx=max(max(max(d,e),max(f,b)),h); mx+=max(mx,max(max(a,c),max(g,i)));
    vec3 amp=sqrt(clamp(min(mn,2.0-mx)/max(mx,vec3(1e-5)),0.0,1.0));
    // Realce del laplaciano, atenuado por amp en las zonas de mucho contraste.
    // Las diferencias muy chicas (ruido, transición entre mipmaps) se ignoran.
    vec3 l=4.0*e-b-d-f-h;
    l=sign(l)*max(abs(l)-0.06,0.0);
    color=vec4(clamp(e+uAmount*amp*l,0.0,1.0),1.0);
}
)GLSL";
const char* const Fxaa = R"GLSL(#version 330 core
uniform sampler2D uSource; uniform ivec2 uOrigin;
out vec4 color;
float lum(vec3 c){ return dot(c,vec3(0.299,0.587,0.114)); }
vec3 at(vec2 uv){ return textureLod(uSource,uv,0.0).rgb; }
void main(){
    vec2 texel=1.0/vec2(textureSize(uSource,0));
    vec2 uv=(gl_FragCoord.xy-vec2(uOrigin))*texel;
    vec3 rgbM=at(uv);
    float nw=lum(at(uv+vec2(-1.0,-1.0)*texel)),ne=lum(at(uv+vec2(1.0,-1.0)*texel));
    float sw=lum(at(uv+vec2(-1.0,1.0)*texel)),se=lum(at(uv+vec2(1.0,1.0)*texel)),m=lum(rgbM);
    float lmin=min(m,min(min(nw,ne),min(sw,se))),lmax=max(m,max(max(nw,ne),max(sw,se)));
    // Sólo bordes con contraste real: las texturas quedan intactas.
    if(lmax-lmin<max(0.0625,lmax*0.125)){ color=vec4(rgbM,1.0); return; }
    vec2 dir=vec2(-((nw+ne)-(sw+se)),(nw+sw)-(ne+se));
    float reduce=max((nw+ne+sw+se)*0.03125,1.0/128.0);
    dir=clamp(dir/(min(abs(dir.x),abs(dir.y))+reduce),vec2(-8.0),vec2(8.0))*texel;
    vec3 a=0.5*(at(uv+dir*(1.0/3.0-0.5))+at(uv+dir*(2.0/3.0-0.5)));
    vec3 b=a*0.5+0.25*(at(uv-dir*0.5)+at(uv+dir*0.5));
    float lb=lum(b);
    color=vec4((lb<lmin||lb>lmax)?a:b,1.0);
}
)GLSL";
struct Program { GLuint name=0; GLint amount=-1,origin=-1,source=-1; bool failed=false; };
GLuint Compile(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr); glCompileShader(shader);
    GLint ok = 0; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) { glDeleteShader(shader); return 0u; }
    return shader;
}
bool Ensure(Program& p, const char* fragment) {
    if (p.name) return true;
    if (p.failed) return false;
    const GLuint v = Compile(GL_VERTEX_SHADER, Vertex), f = Compile(GL_FRAGMENT_SHADER, fragment);
    if (v && f) {
        p.name = glCreateProgram(); glAttachShader(p.name, v); glAttachShader(p.name, f); glLinkProgram(p.name);
        GLint ok = 0; glGetProgramiv(p.name, GL_LINK_STATUS, &ok);
        if (!ok) { glDeleteProgram(p.name); p.name = 0; }
    }
    if (v) glDeleteShader(v);
    if (f) glDeleteShader(f);
    if (!p.name) { p.failed = true; return false; }
    p.amount = glGetUniformLocation(p.name, "uAmount");
    p.origin = glGetUniformLocation(p.name, "uOrigin");
    p.source = glGetUniformLocation(p.name, "uSource");
    return true;
}
Program sharpen, fxaa;
GLuint vao = 0, texture = 0;
int textureW = 0, textureH = 0;
// Copia el viewport y lo redibuja con el programa (estado ya preparado).
void Run(const Program& p, float amount, int x, int y, int w, int h) {
    if (textureW != w || textureH != h) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        textureW = w; textureH = h;
    }
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, w, h);
    glUseProgram(p.name);
    glUniform1i(p.source, 0); glUniform2i(p.origin, x, y);
    if (p.amount >= 0) glUniform1f(p.amount, amount);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
}
// Sombras proyectadas: Render/Backend/GL/ShadowMap.cpp.
void PostProcess(bool antialias, float sharpen) {
    const bool doFxaa = antialias && Post::Ensure(Post::fxaa, Post::Fxaa);
    const bool doSharpen = sharpen > 0.0f && Post::Ensure(Post::sharpen, Post::Sharpen);
    if (!doFxaa && !doSharpen) return;
    const int* viewport = RenderState::Get().viewport;
    const int x = viewport[0], y = viewport[1], w = viewport[2], h = viewport[3];
    if (w <= 0 || h <= 0) return;
    if (!Post::vao) glGenVertexArrays(1, &Post::vao);
    if (!Post::texture) glGenTextures(1, &Post::texture);

    GLint previousProgram = 0, previousVao = 0, previousActive = 0, previousTexture = 0, previousSampler = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActive);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    // Un sampler trilineal en la unidad 0 dejaría incompleta la copia (sin mipmaps).
    glGetIntegerv(GL_SAMPLER_BINDING, &previousSampler);
    glBindSampler(0, 0);
    const GLboolean blend = glIsEnabled(GL_BLEND), depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE), scissor = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_BLEND); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glDisable(GL_SCISSOR_TEST);
    glBindTexture(GL_TEXTURE_2D, Post::texture);
    glBindVertexArray(Post::vao);

    if (doFxaa) Post::Run(Post::fxaa, 0.0f, x, y, w, h);
    if (doSharpen) Post::Run(Post::sharpen, sharpen, x, y, w, h);

    glBindVertexArray(GLuint(previousVao));
    glUseProgram(GLuint(previousProgram));
    glBindTexture(GL_TEXTURE_2D, GLuint(previousTexture));
    glBindSampler(0, GLuint(previousSampler));
    glActiveTexture(GLenum(previousActive));
    if (blend) glEnable(GL_BLEND);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (cull) glEnable(GL_CULL_FACE);
    if (scissor) glEnable(GL_SCISSOR_TEST);
}
void ReadPixelsRGB(int x, int y, int width, int height, void* out) {
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, out);
}
int MaxTextureSize() { GLint value = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value); return value; }
void MaxViewportDims(int out[2]) { GLint value[2]{}; glGetIntegerv(GL_MAX_VIEWPORT_DIMS, value); out[0] = value[0]; out[1] = value[1]; }
const char* GetDeviceInfo(DeviceInfo info) {
    const GLenum name = info == DeviceInfo::Vendor ? GL_VENDOR : info == DeviceInfo::Renderer ? GL_RENDERER : GL_VERSION;
    return reinterpret_cast<const char*>(glGetString(name));
}
Texture CreateTexture() { GLuint name = 0; glGenTextures(1, &name); return name; }
void DeleteTexture(Texture texture) { const GLuint name = texture; glDeleteTextures(1, &name); }
void TexImage2D(int components, int width, int height, Format format, const void* pixels) {
    // Core no acepta la cantidad de componentes como formato interno.
    const GLint internal = components == 3 ? GL_RGB8 : GL_RGBA8;
    ProfileUploadGL::Image2D(glad_glTexImage2D, GL_TEXTURE_2D, 0, internal, width, height, 0, ToGL(format), GL_UNSIGNED_BYTE, pixels);
    // DESVIACION: mipmaps para el filtro de reducción (opción de calidad).
    if (Gfx::TextureMipmaps()) glGenerateMipmap(GL_TEXTURE_2D);
}
void TexSubImage2D(int x, int y, int width, int height, Format format, const void* pixels) {
    ProfileUploadGL::Sub2D(glad_glTexSubImage2D, GL_TEXTURE_2D, 0, x, y, width, height, ToGL(format), GL_UNSIGNED_BYTE, pixels);
    if (Gfx::TextureMipmaps()) glGenerateMipmap(GL_TEXTURE_2D);
}
void TexParameter(TexParam parameter, TexValue value) {
    // Con mipmaps, la reducción pasa a trilineal con anisotropía (también la
    // nítida: la UI y el texto se dibujan a su tamaño o ampliados).
    if (parameter == TexParam::MinFilter && Gfx::TextureMipmaps() && (value == TexValue::Linear || value == TexValue::Nearest)) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if (const float anisotropy = MaxAnisotropy()) glTexParameterf(GL_TEXTURE_2D, 0x84FE, anisotropy); // GL_TEXTURE_MAX_ANISOTROPY
        return;
    }
    glTexParameteri(GL_TEXTURE_2D, ToGL(parameter), ToGL(value));
}
void TexBorderColor(const float rgba[4]) { glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba); }
void SetUnpackAlignment(int alignment) { glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); }
int UnpackAlignment() { GLint value = 0; glGetIntegerv(GL_UNPACK_ALIGNMENT, &value); return value; }
}
