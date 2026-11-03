// Backend GL de la capa Gfx. El estado del pipeline fijo (alpha test, niebla,
// texture env, matrices, color actual) se guarda en RenderState y lo aplican
// los shaders; al dispositivo sólo va el estado que existe en 3.3 core.
// Antes de cada cambio se envía el lote pendiente, para no alterar el orden.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#include "Render/BufferedDraw.h"
#include "Render/Gfx.h"
#include "Render/RenderProfile.h"
#include "Render/RenderState.h"
#include "Render/TerrainDraw.h"
#include <cstring>

namespace Gfx {
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
bool MaterialCap(Cap cap) { return cap == Cap::Texture2D || cap == Cap::AlphaTest || cap == Cap::Fog; }
void CountCap(Cap cap) {
    if (cap == Cap::Blend) RenderProfile::Change(RenderProfile::State::Blend);
    if (cap == Cap::DepthTest) RenderProfile::Change(RenderProfile::State::Depth);
    if (cap == Cap::Texture2D) RenderProfile::Change(RenderProfile::State::Texture);
}
void SetCap(Cap cap, bool on) {
    auto& s = RenderState::Mutable();
    bool& current = s.caps[RenderState::CapIndex(cap)];
    if (current == on) return;
    BufferedDraw::Flush();
    current = on;CountCap(cap);
    if (const GLenum device = DeviceCap(cap)) { if (on) glEnable(device); else glDisable(device); }
    if (MaterialCap(cap)) RenderState::TouchMaterial();
}
void MaterialChange() { BufferedDraw::Flush();RenderState::TouchMaterial(); }
void MatrixChange() { BufferedDraw::Flush(); }
// Lleva al dispositivo lo que una restauración de atributos cambió.
void ApplyDevice(const RenderState::Fixed& before) {
    const auto& s = RenderState::Get();
    for (int i = 0; i < RenderState::CapCount; ++i) {
        if (before.caps[i] == s.caps[i]) continue;
        CountCap(static_cast<Cap>(i));
        if (const GLenum device = DeviceCap(static_cast<Cap>(i))) { if (s.caps[i]) glEnable(device); else glDisable(device); }
    }
    if (before.blendSource != s.blendSource || before.blendDestination != s.blendDestination) {
        RenderProfile::Change(RenderProfile::State::Blend);glBlendFunc(ToGL(s.blendSource), ToGL(s.blendDestination));
    }
    if (before.depthFunction != s.depthFunction) { RenderProfile::Change(RenderProfile::State::Depth);glDepthFunc(ToGL(s.depthFunction)); }
    if (before.depthWrite != s.depthWrite) { RenderProfile::Change(RenderProfile::State::Depth);glDepthMask(s.depthWrite ? GL_TRUE : GL_FALSE); }
    if (before.texture != s.texture) { RenderProfile::Change(RenderProfile::State::Texture);glBindTexture(GL_TEXTURE_2D, s.texture); }
    if (std::memcmp(before.viewport, s.viewport, sizeof(s.viewport)))
        glViewport(s.viewport[0], s.viewport[1], s.viewport[2], s.viewport[3]);
}
}

void ResetState() {
    RenderState::Reset();
    for (int i = 0; i < RenderState::CapCount; ++i)
        if (const GLenum device = DeviceCap(static_cast<Cap>(i))) { if (RenderState::Get().caps[i]) glEnable(device); else glDisable(device); }
    glBlendFunc(GL_ONE, GL_ZERO);glDepthFunc(GL_LESS);glDepthMask(GL_TRUE);glBindTexture(GL_TEXTURE_2D, 0);
    GLint viewport[4]{};glGetIntegerv(GL_VIEWPORT, viewport);
    std::memcpy(RenderState::Mutable().viewport, viewport, sizeof(viewport));
}

void Enable(Cap cap) { SetCap(cap, true); }
void Disable(Cap cap) { SetCap(cap, false); }
bool IsEnabled(Cap cap) { return RenderState::Enabled(cap); }
void BlendFunc(Factor source, Factor destination) {
    auto& s = RenderState::Mutable();
    if (s.blendSource == source && s.blendDestination == destination) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Blend);
    s.blendSource = source;s.blendDestination = destination;
    glBlendFunc(ToGL(source), ToGL(destination));
}
void DepthFunc(Compare function) {
    auto& s = RenderState::Mutable();
    if (s.depthFunction == function) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Depth);
    s.depthFunction = function;glDepthFunc(ToGL(function));
}
void DepthMask(bool write) {
    auto& s = RenderState::Mutable();
    if (s.depthWrite == write) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Depth);
    s.depthWrite = write;glDepthMask(write ? GL_TRUE : GL_FALSE);
}
void AlphaFunc(Compare function, float reference) {
    auto& s = RenderState::Mutable();
    if (s.alphaFunction == function && s.alphaReference == reference) return;
    MaterialChange();s.alphaFunction = function;s.alphaReference = reference;
}
void PushAttrib(unsigned attribMask) { BufferedDraw::Flush();RenderState::PushAttrib(attribMask); }
void PopAttrib() {
    BufferedDraw::Flush();
    RenderState::Fixed before;
    if (RenderState::PopAttrib(before)) ApplyDevice(before);
}

void Color3f(float r, float g, float b) { BufferedDraw::Color4f(r, g, b, 1.0f); }
void Color4f(float r, float g, float b, float a) { BufferedDraw::Color4f(r, g, b, a); }
void Color4fv(const float* rgba) { BufferedDraw::Color4f(rgba[0], rgba[1], rgba[2], rgba[3]); }
void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { BufferedDraw::Color4ub(r, g, b, a); }
void CurrentColor(float rgba[4]) { std::memcpy(rgba, RenderState::Get().color, 4 * sizeof(float)); }

void MatrixMode(Matrix mode) { RenderState::SetMatrixMode(mode); }
void PushMatrix() { MatrixChange();RenderState::PushMatrix(); }
void PopMatrix() { MatrixChange();RenderState::PopMatrix(); }
void LoadIdentity() { MatrixChange();RenderState::LoadIdentity(); }
void Translate(float x, float y, float z) { MatrixChange();RenderState::Translate(x, y, z); }
void Rotate(float degrees, float x, float y, float z) { MatrixChange();RenderState::Rotate(degrees, x, y, z); }
void Ortho2D(double left, double right, double bottom, double top) { MatrixChange();RenderState::Ortho(left, right, bottom, top, -1, 1); }
void Perspective(double fovy, double aspect, double zNear, double zFar) { MatrixChange();RenderState::Perspective(fovy, aspect, zNear, zFar); }
void GetMatrix(Matrix which, float out[16]) { std::memcpy(out, RenderState::Top(which), 16 * sizeof(float)); }

void Viewport(int x, int y, int width, int height) {
    auto& v = RenderState::Mutable().viewport;
    if (v[0] == x && v[1] == y && v[2] == width && v[3] == height) return;
    BufferedDraw::Flush();v[0] = x;v[1] = y;v[2] = width;v[3] = height;
    glViewport(x, y, width, height);
}
void ClearColor(float r, float g, float b, float a) { BufferedDraw::Flush();glClearColor(r, g, b, a); }
void Clear(unsigned clearMask) {
    BufferedDraw::Flush();
    GLbitfield bits = 0;
    if (clearMask & ClearColorBuffer) bits |= GL_COLOR_BUFFER_BIT;
    if (clearMask & ClearDepthBuffer) bits |= GL_DEPTH_BUFFER_BIT;
    if (clearMask & ClearStencilBuffer) bits |= GL_STENCIL_BUFFER_BIT;
    glClear(bits);
}
void Flush() { BufferedDraw::Flush();glFlush(); }
void ClearErrors() { RenderState::TakeStackError();while (glGetError() != GL_NO_ERROR) {} }
void ReadPixelsRGB(int x, int y, int width, int height, void* out) {
    BufferedDraw::Flush();
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, out);
}
int MaxTextureSize() { GLint value = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value); return value; }
void MaxViewportDims(int out[2]) { GLint value[2]{}; glGetIntegerv(GL_MAX_VIEWPORT_DIMS, value); out[0] = value[0]; out[1] = value[1]; }
const char* GetDeviceInfo(DeviceInfo info) {
    const GLenum name = info == DeviceInfo::Vendor ? GL_VENDOR : info == DeviceInfo::Renderer ? GL_RENDERER : GL_VERSION;
    return reinterpret_cast<const char*>(glGetString(name));
}

Texture CreateTexture() { GLuint name = 0; glGenTextures(1, &name); return name; }
void DeleteTexture(Texture texture) {
    BufferedDraw::Flush();TerrainDraw::InvalidateTexture(texture);RenderProfile::ForgetTexture(texture);
    auto& s = RenderState::Mutable();if (s.texture == texture) s.texture = 0;
    const GLuint name = texture; glDeleteTextures(1, &name);
}
void BindTexture(Texture texture) {
    auto& s = RenderState::Mutable();
    if (s.texture == texture) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Texture);
    s.texture = texture;glBindTexture(GL_TEXTURE_2D, texture);
}
void TexImage2D(int components, int width, int height, Format format, const void* pixels) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    // Core no acepta la cantidad de componentes como formato interno.
    const GLint internal = components == 3 ? GL_RGB8 : GL_RGBA8;
    ProfileUploadGL::Image2D(glad_glTexImage2D, GL_TEXTURE_2D, 0, internal, width, height, 0, ToGL(format), GL_UNSIGNED_BYTE, pixels);
}
void TexSubImage2D(int x, int y, int width, int height, Format format, const void* pixels) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    ProfileUploadGL::Sub2D(glad_glTexSubImage2D, GL_TEXTURE_2D, 0, x, y, width, height, ToGL(format), GL_UNSIGNED_BYTE, pixels);
}
void TexParameter(TexParam parameter, TexValue value) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    glTexParameteri(GL_TEXTURE_2D, ToGL(parameter), ToGL(value));
}
TexValue TexValueFromLegacy(int value) {
    switch (value) {
    case GL_LINEAR: return TexValue::Linear;
    case GL_NEAREST_MIPMAP_NEAREST: return TexValue::NearestMipmapNearest;
    case GL_LINEAR_MIPMAP_NEAREST: return TexValue::LinearMipmapNearest;
    case GL_NEAREST_MIPMAP_LINEAR: return TexValue::NearestMipmapLinear;
    case GL_LINEAR_MIPMAP_LINEAR: return TexValue::LinearMipmapLinear;
    case GL_REPEAT: return TexValue::Repeat;
    case 0x2900: return TexValue::Clamp; // GL_CLAMP del binario
    case GL_CLAMP_TO_EDGE: return TexValue::ClampToEdge;
    default: return TexValue::Nearest;
    }
}
void TexBorderColor(const float rgba[4]) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba);
}
void TexEnvMode(TexEnv mode) {
    auto& s = RenderState::Mutable();
    if (s.texEnv == mode) return;
    MaterialChange();s.texEnv = mode;
}
void SetUnpackAlignment(int alignment) { BufferedDraw::Flush();glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); }
int UnpackAlignment() { GLint value = 0; glGetIntegerv(GL_UNPACK_ALIGNMENT, &value); return value; }

void SetFogMode(FogMode mode) { auto& s = RenderState::Mutable();if (s.fogMode == mode) return;MaterialChange();s.fogMode = mode; }
void FogStart(float value) { auto& s = RenderState::Mutable();if (s.fogStart == value) return;MaterialChange();s.fogStart = value; }
void FogEnd(float value) { auto& s = RenderState::Mutable();if (s.fogEnd == value) return;MaterialChange();s.fogEnd = value; }
void FogDensity(float value) { auto& s = RenderState::Mutable();if (s.fogDensity == value) return;MaterialChange();s.fogDensity = value; }
void FogColor(const float rgba[4]) {
    auto& s = RenderState::Mutable();
    if (!std::memcmp(s.fogColor, rgba, sizeof(s.fogColor))) return;
    MaterialChange();std::memcpy(s.fogColor, rgba, sizeof(s.fogColor));
}

}
