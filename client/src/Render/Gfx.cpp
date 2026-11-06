// Capa Gfx, común a los backends. El estado del pipeline fijo (alpha test,
// niebla, texture env, matrices, color actual) se guarda en RenderState y lo
// aplican los shaders; lo que existe en el dispositivo va por GfxDevice.
// Antes de cada cambio se envía el lote pendiente, para no alterar el orden.
#include "Render/Gfx.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderProfile.h"
#include "Render/RenderState.h"
#include "Render/TerrainDraw.h"
#include <cstring>
#include <unordered_map>

namespace Gfx {
namespace {
bool mipmaps=false,bicubic=false;
// Wrap pedido por textura (S, T); sin entrada, el default de GL (repetir).
struct Wrap { bool s=true,t=true; };
std::unordered_map<Texture,Wrap> wraps;
void UpdateWrap() {
    auto& s=RenderState::Mutable();
    const auto it=wraps.find(s.texture);
    const Wrap w=it==wraps.end() ? Wrap{} : it->second;
    if (s.textureRepeat[0]==w.s && s.textureRepeat[1]==w.t) return;
    s.textureRepeat[0]=w.s;s.textureRepeat[1]=w.t;
    if (mipmaps || bicubic) RenderState::TouchMaterial();
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
    GfxDevice::ApplyCap(cap, on);
    // El bicúbico sólo filtra lo 3D (con depth test): ese cap también es material.
    if (MaterialCap(cap) || ((mipmaps || bicubic) && cap == Cap::DepthTest)) RenderState::TouchMaterial();
}
void MaterialChange() { BufferedDraw::Flush();RenderState::TouchMaterial(); }
void MatrixChange() { BufferedDraw::Flush(); }
// Lleva al dispositivo lo que una restauración de atributos cambió.
void ApplyDevice(const RenderState::Fixed& before) {
    const auto& s = RenderState::Get();
    for (int i = 0; i < RenderState::CapCount; ++i) {
        if (before.caps[i] == s.caps[i]) continue;
        CountCap(static_cast<Cap>(i));
        GfxDevice::ApplyCap(static_cast<Cap>(i), s.caps[i]);
        if ((mipmaps || bicubic) && static_cast<Cap>(i) == Cap::DepthTest) RenderState::TouchMaterial();
    }
    if (before.blendSource != s.blendSource || before.blendDestination != s.blendDestination) {
        RenderProfile::Change(RenderProfile::State::Blend);GfxDevice::ApplyBlend(s.blendSource, s.blendDestination);
    }
    if (before.depthFunction != s.depthFunction) { RenderProfile::Change(RenderProfile::State::Depth);GfxDevice::ApplyDepthFunc(s.depthFunction); }
    if (before.depthWrite != s.depthWrite) { RenderProfile::Change(RenderProfile::State::Depth);GfxDevice::ApplyDepthMask(s.depthWrite); }
    if (before.texture != s.texture) { RenderProfile::Change(RenderProfile::State::Texture);GfxDevice::ApplyTexture(s.texture); }
    if (std::memcmp(before.viewport, s.viewport, sizeof(s.viewport)))
        GfxDevice::ApplyViewport(s.viewport[0], s.viewport[1], s.viewport[2], s.viewport[3]);
}
}

void ResetState() {
    RenderState::Reset();
    GfxDevice::Reset();
}

void Enable(Cap cap) { SetCap(cap, true); }
void Disable(Cap cap) { SetCap(cap, false); }
bool IsEnabled(Cap cap) { return RenderState::Enabled(cap); }
void BlendFunc(Factor source, Factor destination) {
    auto& s = RenderState::Mutable();
    if (s.blendSource == source && s.blendDestination == destination) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Blend);
    s.blendSource = source;s.blendDestination = destination;
    GfxDevice::ApplyBlend(source, destination);
}
void DepthFunc(Compare function) {
    auto& s = RenderState::Mutable();
    if (s.depthFunction == function) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Depth);
    s.depthFunction = function;GfxDevice::ApplyDepthFunc(function);
}
void DepthMask(bool write) {
    auto& s = RenderState::Mutable();
    if (s.depthWrite == write) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Depth);
    s.depthWrite = write;GfxDevice::ApplyDepthMask(write);
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
    GfxDevice::ApplyViewport(x, y, width, height);
}
void ClearColor(float r, float g, float b, float a) { BufferedDraw::Flush();GfxDevice::ClearColor(r, g, b, a); }
void Clear(unsigned clearMask) { BufferedDraw::Flush();GfxDevice::Clear(clearMask); }
void Flush() { BufferedDraw::Flush();GfxDevice::Flush(); }
void ClearErrors() { RenderState::TakeStackError();GfxDevice::ClearErrors(); }
void PostProcess(bool antialias, float sharpen) {
    if (!antialias && sharpen <= 0.0f) return;
    BufferedDraw::Flush();
    GfxDevice::PostProcess(antialias, sharpen);
}
void ReadPixelsRGB(int x, int y, int width, int height, void* out) {
    BufferedDraw::Flush();
    GfxDevice::ReadPixelsRGB(x, y, width, height, out);
}
int MaxTextureSize() { return GfxDevice::MaxTextureSize(); }
void MaxViewportDims(int out[2]) { GfxDevice::MaxViewportDims(out); }
const char* GetDeviceInfo(DeviceInfo info) { return GfxDevice::GetDeviceInfo(info); }

Texture CreateTexture() { return GfxDevice::CreateTexture(); }
void DeleteTexture(Texture texture) {
    wraps.erase(texture);
    BufferedDraw::Flush();TerrainDraw::InvalidateTexture(texture);RenderProfile::ForgetTexture(texture);
    auto& s = RenderState::Mutable();if (s.texture == texture) s.texture = 0;
    GfxDevice::DeleteTexture(texture);
}
void BindTexture(Texture texture) {
    auto& s = RenderState::Mutable();
    if (s.texture == texture) return;
    BufferedDraw::Flush();RenderProfile::Change(RenderProfile::State::Texture);
    s.texture = texture;GfxDevice::ApplyTexture(texture);
    UpdateWrap();
}
void TexImage2D(int components, int width, int height, Format format, const void* pixels) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    GfxDevice::TexImage2D(components, width, height, format, pixels);
}
void TexSubImage2D(int x, int y, int width, int height, Format format, const void* pixels) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    GfxDevice::TexSubImage2D(x, y, width, height, format, pixels);
}
void TexParameter(TexParam parameter, TexValue value) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    GfxDevice::TexParameter(parameter, value);
    if (parameter == TexParam::WrapS || parameter == TexParam::WrapT) {
        auto& w=wraps[RenderState::Get().texture];
        (parameter == TexParam::WrapS ? w.s : w.t) = value == TexValue::Repeat;
        UpdateWrap();
    }
}
void SetTextureQuality(bool useMipmaps, bool useBicubic) {
    if (mipmaps==useMipmaps && bicubic==useBicubic) return;
    BufferedDraw::Flush();mipmaps=useMipmaps;bicubic=useBicubic;RenderState::TouchMaterial();
}
bool TextureMipmaps() { return mipmaps; }
bool TextureBicubic() { return bicubic; }
namespace { int multisample = 0; }
void SetMultisample(int samples) { multisample = samples; }
int Multisample() { return multisample; }
namespace { bool shadowMaps = false; float shadowStrength = 0.5f; bool shadowScene = false; }
void SetShadowMaps(bool enabled, float strength) { shadowMaps = enabled; shadowStrength = strength; }
bool ShadowMaps() { return shadowMaps && GfxDevice::SupportsShadowMaps(); }
void BeginShadowScene(const float focus[3]) {
    if (!ShadowMaps() || shadowScene) return;
    BufferedDraw::Flush();
    shadowScene = true;
    GfxDevice::ShadowScene(true, focus, shadowStrength);
}
void SetShadowCasting(bool enabled) {
    if (!shadowScene) return;
    BufferedDraw::Flush();
    GfxDevice::ShadowCasting(enabled);
}
void EndShadowScene() {
    if (!shadowScene) return;
    BufferedDraw::Flush();
    shadowScene = false;
    GfxDevice::ShadowScene(false, nullptr, 0.0f);
}
namespace { bool hdTextures = false; }
void SetHDTextures(bool enabled) { hdTextures = enabled; }
bool HDTextures() { return hdTextures; }
// Valores numéricos de OpenGL que trae el binario (p. ej. OpenJPG(..., 0x2601, 0x2901)).
TexValue TexValueFromLegacy(int value) {
    switch (value) {
    case 0x2601: return TexValue::Linear;               // GL_LINEAR
    case 0x2700: return TexValue::NearestMipmapNearest; // GL_NEAREST_MIPMAP_NEAREST
    case 0x2701: return TexValue::LinearMipmapNearest;  // GL_LINEAR_MIPMAP_NEAREST
    case 0x2702: return TexValue::NearestMipmapLinear;  // GL_NEAREST_MIPMAP_LINEAR
    case 0x2703: return TexValue::LinearMipmapLinear;   // GL_LINEAR_MIPMAP_LINEAR
    case 0x2901: return TexValue::Repeat;               // GL_REPEAT
    case 0x2900: return TexValue::Clamp;                // GL_CLAMP
    case 0x812F: return TexValue::ClampToEdge;          // GL_CLAMP_TO_EDGE
    default: return TexValue::Nearest;
    }
}
void TexBorderColor(const float rgba[4]) {
    BufferedDraw::Flush();TerrainDraw::InvalidateBoundTexture();
    GfxDevice::TexBorderColor(rgba);
}
void TexEnvMode(TexEnv mode) {
    auto& s = RenderState::Mutable();
    if (s.texEnv == mode) return;
    MaterialChange();s.texEnv = mode;
}
void SetUnpackAlignment(int alignment) { BufferedDraw::Flush();GfxDevice::SetUnpackAlignment(alignment); }
int UnpackAlignment() { return GfxDevice::UnpackAlignment(); }

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
