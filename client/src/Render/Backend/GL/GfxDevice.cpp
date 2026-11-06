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
