// Backend GL de la capa Gfx: cada función llama a la misma función GL que
// usaba el código heredado. Las macros de GLCallBarrier envían el lote
// pendiente antes de cada cambio de estado, igual que antes.
#include "stdafx.h"
#include "Render/Backend/GL/LegacyGL.h"
#include "Render/Gfx.h"

namespace Gfx {
namespace {
GLenum ToGL(Cap cap) {
    switch (cap) {
    case Cap::Texture2D: return GL_TEXTURE_2D;
    case Cap::Blend: return GL_BLEND;
    case Cap::AlphaTest: return GL_ALPHA_TEST;
    case Cap::DepthTest: return GL_DEPTH_TEST;
    case Cap::CullFace: return GL_CULL_FACE;
    case Cap::Fog: return GL_FOG;
    case Cap::ScissorTest: return GL_SCISSOR_TEST;
    case Cap::StencilTest: return GL_STENCIL_TEST;
    case Cap::Lighting: return GL_LIGHTING;
    case Cap::Dither: return GL_DITHER;
    case Cap::PolygonSmooth: return GL_POLYGON_SMOOTH;
    case Cap::ColorLogicOp: return GL_COLOR_LOGIC_OP;
    }
    return GL_TEXTURE_2D;
}
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
GLenum ToGL(Matrix mode) {
    switch (mode) {
    case Matrix::ModelView: return GL_MODELVIEW;
    case Matrix::Projection: return GL_PROJECTION;
    case Matrix::Texture: return GL_TEXTURE;
    }
    return GL_MODELVIEW;
}
GLenum ToGL(Format format) {
    switch (format) {
    case Format::Alpha: return GL_ALPHA;
    case Format::RGB: return GL_RGB;
    case Format::RGBA: return GL_RGBA;
    case Format::Luminance: return GL_LUMINANCE;
    case Format::LuminanceAlpha: return GL_LUMINANCE_ALPHA;
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
    case TexValue::Clamp: return GL_CLAMP;
    case TexValue::ClampToEdge: return GL_CLAMP_TO_EDGE;
    }
    return GL_NEAREST;
}
GLenum ToGL(TexEnv mode) {
    switch (mode) {
    case TexEnv::Modulate: return GL_MODULATE;
    case TexEnv::Replace: return GL_REPLACE;
    case TexEnv::Decal: return GL_DECAL;
    case TexEnv::Blend: return GL_BLEND;
    case TexEnv::Add: return GL_ADD;
    }
    return GL_MODULATE;
}
GLbitfield AttribBits(unsigned mask) {
    GLbitfield bits = 0;
    if (mask & AttribCurrent) bits |= GL_CURRENT_BIT;
    if (mask & AttribTexture) bits |= GL_TEXTURE_BIT;
    if (mask & AttribEnable) bits |= GL_ENABLE_BIT;
    if (mask & AttribList) bits |= GL_LIST_BIT;
    return bits;
}
}

void Enable(Cap cap) { glEnable(ToGL(cap)); }
void Disable(Cap cap) { glDisable(ToGL(cap)); }
void BlendFunc(Factor source, Factor destination) { glBlendFunc(ToGL(source), ToGL(destination)); }
void DepthFunc(Compare function) { glDepthFunc(ToGL(function)); }
void DepthMask(bool write) { glDepthMask(write ? GL_TRUE : GL_FALSE); }
void AlphaFunc(Compare function, float reference) { glAlphaFunc(ToGL(function), reference); }
void PushAttrib(unsigned attribMask) { glPushAttrib(AttribBits(attribMask)); }
void PopAttrib() { glPopAttrib(); }

void Color3f(float r, float g, float b) { glColor3f(r, g, b); }
void Color4f(float r, float g, float b, float a) { glColor4f(r, g, b, a); }
void Color4fv(const float* rgba) { glColor4fv(rgba); }
void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { glColor4ub(r, g, b, a); }
void CurrentColor(float rgba[4]) { glGetFloatv(GL_CURRENT_COLOR, rgba); }

void MatrixMode(Matrix mode) { glMatrixMode(ToGL(mode)); }
void PushMatrix() { glPushMatrix(); }
void PopMatrix() { glPopMatrix(); }
void LoadIdentity() { glLoadIdentity(); }
void Translate(float x, float y, float z) { glTranslatef(x, y, z); }
void Rotate(float degrees, float x, float y, float z) { glRotatef(degrees, x, y, z); }
void Ortho2D(double left, double right, double bottom, double top) { gluOrtho2D(left, right, bottom, top); }
void Perspective(double fovy, double aspect, double zNear, double zFar) { gluPerspective(fovy, aspect, zNear, zFar); }
void GetMatrix(Matrix which, float out[16]) {
    glGetFloatv(which == Matrix::Projection ? GL_PROJECTION_MATRIX :
                which == Matrix::Texture ? GL_TEXTURE_MATRIX : GL_MODELVIEW_MATRIX, out);
}

void Viewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }
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
void ReadPixelsRGB(int x, int y, int width, int height, void* out) { glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, out); }
int MaxTextureSize() { GLint value = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value); return value; }
void MaxViewportDims(int out[2]) { GLint value[2]{}; glGetIntegerv(GL_MAX_VIEWPORT_DIMS, value); out[0] = value[0]; out[1] = value[1]; }
const char* GetDeviceInfo(DeviceInfo info) {
    const GLenum name = info == DeviceInfo::Vendor ? GL_VENDOR : info == DeviceInfo::Renderer ? GL_RENDERER : GL_VERSION;
    return reinterpret_cast<const char*>(glGetString(name));
}

Texture CreateTexture() { GLuint name = 0; glGenTextures(1, &name); return name; }
void DeleteTexture(Texture texture) { const GLuint name = texture; glDeleteTextures(1, &name); }
void BindTexture(Texture texture) { glBindTexture(GL_TEXTURE_2D, texture); }
void TexImage2D(int components, int width, int height, Format format, const void* pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, ToGL(format), GL_UNSIGNED_BYTE, pixels);
}
void TexParameter(TexParam parameter, TexValue value) { glTexParameteri(GL_TEXTURE_2D, ToGL(parameter), ToGL(value)); }
TexValue TexValueFromLegacy(int value) {
    switch (value) {
    case GL_LINEAR: return TexValue::Linear;
    case GL_NEAREST_MIPMAP_NEAREST: return TexValue::NearestMipmapNearest;
    case GL_LINEAR_MIPMAP_NEAREST: return TexValue::LinearMipmapNearest;
    case GL_NEAREST_MIPMAP_LINEAR: return TexValue::NearestMipmapLinear;
    case GL_LINEAR_MIPMAP_LINEAR: return TexValue::LinearMipmapLinear;
    case GL_REPEAT: return TexValue::Repeat;
    case GL_CLAMP: return TexValue::Clamp;
    case GL_CLAMP_TO_EDGE: return TexValue::ClampToEdge;
    default: return TexValue::Nearest;
    }
}
void TexBorderColor(const float rgba[4]) { glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba); }
void TexEnvMode(TexEnv mode) { glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, static_cast<float>(ToGL(mode))); }
void SetUnpackAlignment(int alignment) { glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); }
int UnpackAlignment() { GLint value = 0; glGetIntegerv(GL_UNPACK_ALIGNMENT, &value); return value; }

void SetFogMode(FogMode mode) { glFogi(GL_FOG_MODE, mode == FogMode::Linear ? GL_LINEAR : mode == FogMode::Exp ? GL_EXP : GL_EXP2); }
void FogStart(float value) { glFogf(GL_FOG_START, value); }
void FogEnd(float value) { glFogf(GL_FOG_END, value); }
void FogDensity(float value) { glFogf(GL_FOG_DENSITY, value); }
void FogColor(const float rgba[4]) { glFogfv(GL_FOG_COLOR, rgba); }

unsigned GenLists(int count) { return glGenLists(count); }
void DeleteLists(unsigned base, int count) { glDeleteLists(base, count); }
void ListBase(unsigned base) { glListBase(base); }
void RasterPos2f(float x, float y) { glRasterPos2f(x, y); }
void CallLists(int count, const unsigned char* characters) { glCallLists(count, GL_UNSIGNED_BYTE, characters); }

}
