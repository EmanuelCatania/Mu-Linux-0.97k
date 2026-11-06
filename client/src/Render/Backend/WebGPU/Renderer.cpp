// Backend WebGPU de GfxDevice y BufferedDraw.
//
// Cada lote (Submit) guarda sus vértices, una foto del estado de RenderState
// (pipeline, uniforms, textura, viewport) y el dibujo. Gfx::Clear abre una
// pasada nueva. En Present se sube todo de una vez y se codifica en un único
// envío: así un buffer o una textura que se reescribe a mitad de cuadro no
// cambia los dibujos ya grabados (una textura usada en el cuadro que se vuelve
// a escribir pasa a una copia nueva; los dibujos anteriores conservan la vieja).
#include "Render/Backend/Api.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/Backend/WebGPU/Device.h"
#include "Render/Backend/WebGPU/Renderer.h"
#include "Render/MeshTopology.h"
#include "Render/RenderState.h"
#include "Render/Skinning.h"
#include "Render/ShadowLight.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" void DbgLogPublic(const char*);

namespace WebGPU {
namespace {
using Gfx::Cap;
using BufferedDraw::Vertex;

// Modos de BufferedDraw (numeración de OpenGL 1.1). Abanicos, polígonos y
// quad strips ya llegan convertidos.
enum Mode : unsigned { Points=0, Lines=1, LineLoop=2, LineStrip=3, Triangles=4, TriangleStrip=5, Quads=7 };

const char* const Shader = R"WGSL(
struct Uniforms {
    mvp: mat4x4f, modelView: mat4x4f, textureMatrix: mat4x4f,
    flags: vec4i,     // textured, texEnv, alphaFunction, fog
    params: vec4f,    // alphaReference, fogStart, fogEnd, fogDensity
    fogColor: vec4f,
    extra: vec4i,     // bicúbico, recibe sombra
};
@group(0) @binding(0) var<uniform> u: Uniforms;
@group(1) @binding(0) var tex: texture_2d<f32>;
@group(1) @binding(1) var smp: sampler;
// Sombras proyectadas: params = activa, intensidad, tamaño del texel, sesgo.
struct Shadow { lightViewProj: mat4x4f, inverseView: mat4x4f, params: vec4f };
@group(2) @binding(0) var<uniform> shadow: Shadow;
@group(2) @binding(1) var shadowMap: texture_depth_2d;
@group(2) @binding(2) var shadowSampler: sampler_comparison;
struct VertexOut {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec4f,
    @location(2) fogDistance: f32,
    @location(3) world: vec3f,
};
// Posición en el mundo: la cámara de la escena deshace la vista del dibujo.
fn worldOf(position: vec4f) -> vec3f {
    return (shadow.inverseView * (u.modelView * position)).xyz;
}
@vertex fn vs(@location(0) p: vec3f, @location(1) color: vec4f, @location(2) uv: vec4f) -> VertexOut {
    var out: VertexOut;
    let position = vec4f(p, 1.0);
    var clip = u.mvp * position;
    // Las matrices son de OpenGL (z de -1 a 1); WebGPU usa z de 0 a 1.
    clip.z = (clip.z + clip.w) * 0.5;
    out.position = clip;
    out.color = clamp(color, vec4f(0.0), vec4f(1.0));
    out.uv = u.textureMatrix * uv;
    out.fogDistance = abs((u.modelView * position).z);
    out.world = worldOf(position);
    return out;
}
// Mallas residentes: posición y normal en el espacio del hueso; la paleta
// tiene tres filas por hueso (rotación en xyz, traslación en w).
struct Skin {
    paletteBase: i32, flags: u32, pad0: i32, pad1: i32,   // flags: luz, sombra, textura
    scales: vec4f, origin: vec4f, direction: vec4f, coefficients: vec4f,
    bodyLight: vec4f, uvOffset: vec4f, shadowOrigin: vec4f, color: vec4f, uv: vec4f,
};
@group(0) @binding(1) var<uniform> skin: Skin;
@group(0) @binding(2) var<storage, read> palette: array<vec4f>;
fn rotateBone(p: vec3f, b: i32) -> vec3f {
    let r = skin.paletteBase + b * 3;
    return vec3f(dot(palette[r].xyz, p), dot(palette[r + 1].xyz, p), dot(palette[r + 2].xyz, p));
}
fn skinPosition(p0: vec3f, b: i32) -> vec3f {
    let r = skin.paletteBase + b * 3;
    var p = rotateBone(p0, b) * skin.scales.x;
    p = p + vec3f(palette[r].w, palette[r + 1].w, palette[r + 2].w);
    return p * skin.scales.y + skin.origin.xyz;
}
@vertex fn vs_skin(@location(0) p0: vec3f, @location(1) bone: f32, @location(2) normal: vec4f, @location(3) uv2: vec2f) -> VertexOut {
    var out: VertexOut;
    var p = skinPosition(p0, i32(bone));
    if ((skin.flags & 2u) != 0u) {
        // Sombra proyectada de BMD__RenderBodyShadow.
        let l = p - skin.shadowOrigin.xyz;
        p = vec3f(l.x + (l.x + 2000.0) * l.z / (l.z - 4000.0) + skin.shadowOrigin.x, l.y + skin.shadowOrigin.y, skin.shadowOrigin.z + 5.0);
    }
    let position = vec4f(p, 1.0);
    var clip = u.mvp * position;
    clip.z = (clip.z + clip.w) * 0.5;
    out.position = clip;
    var color = skin.color;
    if ((skin.flags & 1u) != 0u) {
        let n = rotateBone(normal.xyz, i32(normal.w));
        let light = max((skin.direction.y * n.y + skin.direction.x * n.x + skin.direction.z * n.z) * skin.coefficients.x + skin.coefficients.y, skin.coefficients.z);
        color = vec4f(light * skin.bodyLight.xyz, color.a);
    }
    var uv = skin.uv;
    if ((skin.flags & 4u) != 0u) { uv = vec4f(uv2, 0.0, 1.0); }
    out.color = clamp(color, vec4f(0.0), vec4f(1.0));
    out.uv = u.textureMatrix * (uv + vec4f(skin.uvOffset.xy, 0.0, 0.0));
    out.fogDistance = abs((u.modelView * position).z);
    out.world = worldOf(position);
    return out;
}
// Pase de sombras: los mismos dibujos vistos desde la luz, sólo profundidad.
struct CasterOut {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec4f,
};
@vertex fn vs_caster(@location(0) p: vec3f, @location(1) color: vec4f, @location(2) uv: vec4f) -> CasterOut {
    var out: CasterOut;
    out.position = shadow.lightViewProj * vec4f(worldOf(vec4f(p, 1.0)), 1.0);
    out.color = clamp(color, vec4f(0.0), vec4f(1.0));
    out.uv = u.textureMatrix * uv;
    return out;
}
@vertex fn vs_skin_caster(@location(0) p0: vec3f, @location(1) bone: f32, @location(2) normal: vec4f, @location(3) uv2: vec2f) -> CasterOut {
    var out: CasterOut;
    let p = skinPosition(p0, i32(bone));
    out.position = shadow.lightViewProj * vec4f(worldOf(vec4f(p, 1.0)), 1.0);
    out.color = clamp(skin.color, vec4f(0.0), vec4f(1.0));
    var uvs = skin.uv;
    if ((skin.flags & 4u) != 0u) { uvs = vec4f(uv2, 0.0, 1.0); }
    out.uv = u.textureMatrix * (uvs + vec4f(skin.uvOffset.xy, 0.0, 0.0));
    return out;
}
// No proyecta lo que se dibuja semitransparente por su color (un techo que
// se desvanece al entrar a un edificio, un objeto que aparece) ni lo
// transparente de la textura (hojas, rejas).
@fragment fn fs_caster(in: CasterOut) {
    let t = textureSample(tex, smp, in.uv.xy / in.uv.w);
    if (in.color.a < 0.99) { discard; }
    if (u.flags.x != 0 && t.a < 0.25) { discard; }
}
// Fracción iluminada (PCF 3x3), con borde suave al final del mapa.
fn shadowFactor(world: vec3f) -> f32 {
    let lp = shadow.lightViewProj * vec4f(world, 1.0);
    let uv = vec2f(lp.x * 0.5 + 0.5, 0.5 - lp.y * 0.5);
    if (any(uv < vec2f(0.0)) || any(uv > vec2f(1.0)) || lp.z > 1.0) { return 1.0; }
    var lit = 0.0;
    for (var y = -1; y <= 1; y++) {
        for (var x = -1; x <= 1; x++) {
            lit += textureSampleCompareLevel(shadowMap, shadowSampler, uv + vec2f(f32(x), f32(y)) * shadow.params.z, lp.z - shadow.params.w);
        }
    }
    lit = lit / 9.0;
    let edge = smoothstep(0.0, 0.1, min(min(uv.x, uv.y), min(1.0 - uv.x, 1.0 - uv.y)));
    return mix(1.0, mix(1.0 - shadow.params.y, 1.0, lit), edge);
}
// Catmull-Rom con los 16 texels del nivel 0 (textureLoad: el 0.97k usa casi
// siempre filtro nearest, así que el sampler no sirve para interpolar).
fn wrapTexel(c: i32, size: i32, repeat: bool) -> i32 {
    if (repeat) { return ((c % size) + size) % size; }
    return clamp(c, 0, size - 1);
}
fn cubicWeights(f: f32) -> vec4f {
    return vec4f(f * (-0.5 + f * (1.0 - 0.5 * f)), 1.0 + f * f * (-2.5 + 1.5 * f),
                 f * (0.5 + f * (2.0 - 1.5 * f)), f * f * (-0.5 + 0.5 * f));
}
fn catmullRom(uv: vec2f, size: vec2f) -> vec4f {
    let isize = vec2i(size);
    let p = uv * size - 0.5;
    let i = floor(p);
    let f = p - i;
    let wx = cubicWeights(f.x);
    let wy = cubicWeights(f.y);
    let repeatS = (u.extra.x & 2) != 0;
    let repeatT = (u.extra.x & 4) != 0;
    var r = vec4f(0.0);
    for (var y = 0; y < 4; y++) {
        let ty = wrapTexel(i32(i.y) + y - 1, isize.y, repeatT);
        var row = vec4f(0.0);
        for (var x = 0; x < 4; x++) {
            let tx = wrapTexel(i32(i.x) + x - 1, isize.x, repeatS);
            row += textureLoad(tex, vec2i(tx, ty), 0) * wx[x];
        }
        r += row * wy[y];
    }
    return clamp(r, vec4f(0.0), vec4f(1.0));
}
@fragment fn fs(in: VertexOut) -> @location(0) vec4f {
    // textureProj: divide por q. Se muestrea siempre (control uniforme).
    let uv = in.uv.xy / in.uv.w;
    var t = textureSample(tex, smp, uv);
    // DESVIACION: bicúbico (opción de calidad), sólo al ampliar la textura.
    let size = vec2f(textureDimensions(tex, 0));
    // Texels por píxel en el eje mayor (como el LOD del hardware, no fwidth).
    let st = uv * size;
    let footprint = max(length(dpdx(st)), length(dpdy(st)));
    if (u.extra.x != 0 && footprint < 1.0) { t = catmullRom(uv, size); }
    var c = in.color;
    if (u.flags.x != 0) {
        let env = u.flags.y;
        if (env == 1) { c = t; }
        else if (env == 2) { c = vec4f(mix(in.color.rgb, t.rgb, t.a), in.color.a); }
        else if (env == 4) { c = vec4f(in.color.rgb + t.rgb, in.color.a * t.a); }
        else { c = t * in.color; }
    }
    let f = u.flags.z;
    if (f != 0) {
        let a = c.a;
        let r = u.params.x;
        var passed = true;
        if (f == 1) { passed = false; }
        else if (f == 2) { passed = a < r; }
        else if (f == 3) { passed = a == r; }
        else if (f == 4) { passed = a <= r; }
        else if (f == 5) { passed = a > r; }
        else if (f == 6) { passed = a != r; }
        else if (f == 7) { passed = a >= r; }
        if (!passed) { discard; }
    }
    // DESVIACION: sombras proyectadas (opción), antes de la niebla.
    if (u.extra.y != 0 && shadow.params.x > 0.5) {
        c = vec4f(c.rgb * shadowFactor(in.world), c.a);
    }
    let fog = u.flags.w;
    if (fog != 0) {
        let d = in.fogDistance;
        var k = 1.0;
        if (fog == 1) {
            if (u.params.z == u.params.y) { k = select(0.0, 1.0, d <= u.params.z); }
            else { k = (u.params.z - d) / (u.params.z - u.params.y); }
        }
        if (fog == 2) { k = exp(-u.params.w * d); }
        if (fog == 3) { let v = u.params.w * d; k = exp(-v * v); }
        c = vec4f(mix(u.fogColor.rgb, c.rgb, clamp(k, 0.0, 1.0)), c.a);
    }
    return c;
}
)WGSL";

struct Uniforms {
    float mvp[16], modelView[16], textureMatrix[16];
    int32_t flags[4];
    float params[4];
    float fogColor[4];
    int32_t extra[4];
};
struct Skin {
    int32_t paletteBase; uint32_t flags; int32_t pad[2];
    float scales[4], origin[4], direction[4], coefficients[4], bodyLight[4], uvOffset[4], shadowOrigin[4], color[4], uv[4];
};
struct ShadowUniform {
    float lightViewProj[16], inverseView[16];
    float params[4];   // activa, intensidad, tamaño del texel, sesgo
};
constexpr uint32_t ShadowSize = ShadowLight::Size;
constexpr uint64_t UniformStride = 256;
static_assert(sizeof(Skin) <= UniformStride, "Skin no entra en su tramo");
static_assert(sizeof(Uniforms) <= UniformStride, "Uniforms no entra en su tramo");

// ── Texturas ────────────────────────────────────────────────────────────────
struct Sampling {
    Gfx::TexValue minFilter = Gfx::TexValue::NearestMipmapLinear, magFilter = Gfx::TexValue::Linear;
    Gfx::TexValue wrapS = Gfx::TexValue::Repeat, wrapT = Gfx::TexValue::Repeat;
    uint32_t Key() const {
        return uint32_t(minFilter) | uint32_t(magFilter) << 8 | uint32_t(wrapS) << 16 | uint32_t(wrapT) << 24;
    }
};
struct TextureSlot {
    WGPUTexture texture = nullptr;
    WGPUTextureView view = nullptr;
    int width = 0, height = 0;
    Sampling sampling;
    // Bind groups (vista + sampler) por combinación de filtro y wrap: el
    // cliente vuelve a fijar el filtro al ligar la textura, y alternar entre
    // dos no debe crear un bind group por dibujo.
    struct Group { uint32_t key; WGPUBindGroup group; };
    std::vector<Group> groups;
    uint64_t usedFrame = 0;          // último cuadro grabado que la usa
};

// ── Grabación del cuadro ────────────────────────────────────────────────────
// Clase de vértice del pipeline, en los bits altos de PipelineKey::topology.
enum Kind : uint8_t { Immediate = 0, Resident = 1 };
struct PipelineKey {
    uint8_t topology = 0, blend = 0, src = 0, dst = 0, depthTest = 0, depthFunc = 0, depthWrite = 0, cull = 0;
    uint64_t Value() const {
        uint64_t v = 0;
        std::memcpy(&v, this, sizeof(*this));
        return v;
    }
};
static_assert(sizeof(PipelineKey) == 8, "PipelineKey ocupa 8 bytes");
struct Draw {
    WGPURenderPipeline pipeline;
    WGPUBindGroup textureGroup;
    uint32_t uniformOffset, skinOffset;
    uint32_t firstVertex, vertexCount;
    bool quads;
    bool caster;   // dentro de la escena de sombras: se dibuja también desde la luz
    uint8_t topology;   // WGPUPrimitiveTopology del dibujo inmediato
    int viewport[4];
    // Malla residente: sus propios buffers (nulos en el dibujo inmediato).
    WGPUBuffer vertexBuffer, indexBuffer;
    uint32_t indexCount;
};
// Buffers de una malla BMD residente; se rehacen al cambiar su topología.
struct GpuMesh {
    WGPUBuffer indices = nullptr, vertices = nullptr;
    uint64_t revision = 0, residentRevision = 0, usedFrame = 0;
};
struct Pass {
    bool clearColor = false, clearDepth = false;
    // Posproceso sobre postViewport antes de la pasada: FXAA y nitidez (> 0).
    bool antialias = false;
    float sharpen = 0.0f;
    int postViewport[4]{};
    bool Post() const { return antialias || sharpen > 0.0f; }
    bool atmosphere = false;   // niebla por profundidad antes de los dibujos de la pasada
    float color[4]{};
    std::vector<Draw> draws;
};

struct State {
    bool ready = false;
    WGPUShaderModule module = nullptr;
    WGPUBindGroupLayout uniformLayout = nullptr, textureLayout = nullptr;
    WGPUPipelineLayout pipelineLayout = nullptr;
    WGPUBuffer vertexBuffer = nullptr, uniformBuffer = nullptr, indexBuffer = nullptr;
    uint64_t vertexCapacity = 0, uniformCapacity = 0, indexQuads = 0;
    WGPUBindGroup uniformGroup = nullptr;
    WGPUBuffer skinBuffer = nullptr, paletteBuffer = nullptr;
    uint64_t skinCapacity = 0, paletteCapacity = 0;
    std::unordered_map<const void*, GpuMesh> meshes;
    bool residentEnabled = true;
    WGPUTexture depth = nullptr;
    WGPUTextureView depthView = nullptr;
    unsigned depthWidth = 0, depthHeight = 0;
    // MSAA: color multisample que cada pasada resuelve sobre la textura del cuadro.
    WGPUTexture msaa = nullptr;
    WGPUTextureView msaaView = nullptr;
    unsigned msaaWidth = 0, msaaHeight = 0;
    WGPUTextureFormat msaaFormat = WGPUTextureFormat_Undefined;
    std::unordered_map<uint64_t, WGPURenderPipeline> pipelines;
    std::unordered_map<uint32_t, WGPUSampler> samplers;
    std::unordered_map<Gfx::Texture, TextureSlot> textures;
    TextureSlot white;
    Gfx::Texture nextTexture = 1;
    // Cuadro en grabación.
    std::vector<Vertex> vertices;
    std::vector<uint8_t> uniforms;
    std::vector<Pass> passes;
    std::vector<uint8_t> skins;
    std::vector<float> palette;
    uint64_t paletteRevision = ~0ull;
    int32_t paletteBase = 0;
    uint64_t matrixRevision = ~0ull, materialRevision = ~0ull;
    uint32_t lastUniform = 0;
    bool lastReceiver = false;
    uint64_t frame = 1;
    // Recursos que siguen en uso por el cuadro grabado y se liberan al enviarlo.
    std::vector<WGPUTexture> retiredTextures;
    std::vector<WGPUTextureView> retiredViews;
    std::vector<WGPUBindGroup> retiredGroups;
    std::vector<WGPUBuffer> retiredBuffers;
    float clearColor[4]{};
    int unpackAlignment = 4;
    // Nitidez (CAS): copia del cuadro y su pipeline.
    WGPUShaderModule postModule = nullptr;
    WGPUBindGroupLayout postLayout = nullptr;
    WGPUPipelineLayout postPipelineLayout = nullptr;
    WGPURenderPipeline postPipeline = nullptr, fxaaPipeline = nullptr;
    WGPUSampler postSampler = nullptr;
    WGPUTexture postTexture = nullptr;
    WGPUTextureView postView = nullptr;
    WGPUBuffer postUniform = nullptr;
    WGPUBindGroup postGroup = nullptr;
    unsigned postWidth = 0, postHeight = 0;
    WGPUTextureFormat postFormat = WGPUTextureFormat_Undefined;
    unsigned errors = 0;
    // Sombras proyectadas: escena en grabación y recursos.
    bool shadowScene = false, shadowFrame = false, shadowCasting = true;
    float shadowStrength = 0.5f;
    ShadowUniform shadow{};
    WGPUTexture shadowMap = nullptr;
    WGPUTextureView shadowView = nullptr;
    WGPUSampler shadowSampler = nullptr;
    WGPUBuffer shadowBuffer = nullptr;
    WGPUBindGroupLayout shadowLayout = nullptr, casterLayout = nullptr;
    WGPUBindGroup shadowGroup = nullptr, casterGroup = nullptr;
    WGPUPipelineLayout casterPipelineLayout = nullptr;
    std::unordered_map<uint8_t, WGPURenderPipeline> casterPipelines;
    // Niebla por profundidad: uniform del cuadro y recursos.
    struct Atmosphere {
        float inverseViewProj[16];
        float camera[4];     // posición de la cámara en el mundo
        float color[4];
        float params[4];     // modo, densidad, distancia inicial
        float viewport[4];   // x, y (desde arriba), ancho, alto en píxeles
    } atmosphere{};
    int atmosphereViewport[4]{};
    WGPUShaderModule atmosphereModule = nullptr;
    WGPUBindGroupLayout atmosphereLayout = nullptr;
    WGPUPipelineLayout atmospherePipelineLayout = nullptr;
    WGPURenderPipeline atmosphereMultiply = nullptr, atmosphereAdd = nullptr;
    WGPUBuffer atmosphereBuffer = nullptr;
    WGPUTextureFormat atmosphereFormat = WGPUTextureFormat_Undefined;
    uint32_t atmosphereSamples = 0;
} g;

uint32_t Samples() { return Gfx::Multisample() > 1 ? 4u : 1u; }   // WebGPU garantiza 1 y 4

WGPUStringView View(const char* text) { return WGPUStringView{text, WGPU_STRLEN}; }

void Log(const char* text) { DbgLogPublic(text); }

WGPUBlendFactor Blend(Gfx::Factor f) {
    switch (f) {
    case Gfx::Factor::Zero: return WGPUBlendFactor_Zero;
    case Gfx::Factor::One: return WGPUBlendFactor_One;
    case Gfx::Factor::SrcColor: return WGPUBlendFactor_Src;
    case Gfx::Factor::OneMinusSrcColor: return WGPUBlendFactor_OneMinusSrc;
    case Gfx::Factor::SrcAlpha: return WGPUBlendFactor_SrcAlpha;
    case Gfx::Factor::OneMinusSrcAlpha: return WGPUBlendFactor_OneMinusSrcAlpha;
    case Gfx::Factor::DstAlpha: return WGPUBlendFactor_DstAlpha;
    case Gfx::Factor::OneMinusDstAlpha: return WGPUBlendFactor_OneMinusDstAlpha;
    case Gfx::Factor::DstColor: return WGPUBlendFactor_Dst;
    case Gfx::Factor::OneMinusDstColor: return WGPUBlendFactor_OneMinusDst;
    case Gfx::Factor::SrcAlphaSaturate: return WGPUBlendFactor_SrcAlphaSaturated;
    }
    return WGPUBlendFactor_One;
}
WGPUCompareFunction Compare(Gfx::Compare c) {
    switch (c) {
    case Gfx::Compare::Never: return WGPUCompareFunction_Never;
    case Gfx::Compare::Less: return WGPUCompareFunction_Less;
    case Gfx::Compare::Equal: return WGPUCompareFunction_Equal;
    case Gfx::Compare::LessEqual: return WGPUCompareFunction_LessEqual;
    case Gfx::Compare::Greater: return WGPUCompareFunction_Greater;
    case Gfx::Compare::NotEqual: return WGPUCompareFunction_NotEqual;
    case Gfx::Compare::GreaterEqual: return WGPUCompareFunction_GreaterEqual;
    case Gfx::Compare::Always: return WGPUCompareFunction_Always;
    }
    return WGPUCompareFunction_Always;
}
int AlphaCode(Gfx::Compare c) {
    switch (c) {
    case Gfx::Compare::Never: return 1; case Gfx::Compare::Less: return 2; case Gfx::Compare::Equal: return 3;
    case Gfx::Compare::LessEqual: return 4; case Gfx::Compare::Greater: return 5; case Gfx::Compare::NotEqual: return 6;
    case Gfx::Compare::GreaterEqual: return 7; case Gfx::Compare::Always: return 0;
    }
    return 0;
}
int EnvCode(Gfx::TexEnv e) {
    switch (e) { case Gfx::TexEnv::Replace: return 1; case Gfx::TexEnv::Decal: return 2; case Gfx::TexEnv::Add: return 4; default: return 0; }
}
bool Linear(Gfx::TexValue v) {
    return v == Gfx::TexValue::Linear || v == Gfx::TexValue::LinearMipmapNearest || v == Gfx::TexValue::LinearMipmapLinear;
}
WGPUAddressMode Address(Gfx::TexValue v) { return v == Gfx::TexValue::Repeat ? WGPUAddressMode_Repeat : WGPUAddressMode_ClampToEdge; }

WGPUBuffer CreateBuffer(uint64_t size, WGPUBufferUsage usage) {
    WGPUBufferDescriptor d = WGPU_BUFFER_DESCRIPTOR_INIT;
    d.size = size;
    d.usage = usage | WGPUBufferUsage_CopyDst;
    return wgpuDeviceCreateBuffer(GetDevice(), &d);
}

WGPUSampler Sampler(const Sampling& s) {
    auto it = g.samplers.find(s.Key());
    if (it != g.samplers.end()) return it->second;
    WGPUSamplerDescriptor d = WGPU_SAMPLER_DESCRIPTOR_INIT;
    d.addressModeU = Address(s.wrapS);
    d.addressModeV = Address(s.wrapT);
    d.magFilter = Linear(s.magFilter) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
    d.minFilter = Linear(s.minFilter) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
    if (Gfx::TextureMipmaps()) {
        // DESVIACION: con la opción de calidad, reducción trilineal con
        // anisotropía, también para la nítida (la UI se dibuja a su tamaño).
        d.minFilter = WGPUFilterMode_Linear;
        d.mipmapFilter = WGPUMipmapFilterMode_Linear;
        if (Linear(s.magFilter)) d.maxAnisotropy = 16;
    } else {
        // El cliente sube sólo el nivel 0: un filtro nítido no usa mipmaps.
        d.lodMaxClamp = 0.0f;
    }
    WGPUSampler sampler = wgpuDeviceCreateSampler(GetDevice(), &d);
    g.samplers.emplace(s.Key(), sampler);
    return sampler;
}

void Retire(TextureSlot& slot) {
    for (auto& entry : slot.groups) g.retiredGroups.push_back(entry.group);
    slot.groups.clear();
    if (slot.view) g.retiredViews.push_back(slot.view);
    if (slot.texture) g.retiredTextures.push_back(slot.texture);
    slot.view = nullptr; slot.texture = nullptr;
}
void Destroy(TextureSlot& slot) {
    for (auto& entry : slot.groups) wgpuBindGroupRelease(entry.group);
    slot.groups.clear();
    if (slot.view) wgpuTextureViewRelease(slot.view);
    if (slot.texture) wgpuTextureRelease(slot.texture);
    slot.view = nullptr; slot.texture = nullptr;
}

uint32_t MipLevels(int width, int height) {
    if (!Gfx::TextureMipmaps()) return 1;
    uint32_t levels = 1;
    for (int size = std::max(width, height); size > 1; size /= 2) ++levels;
    return levels;
}
WGPUTexture NewTexture(int width, int height) {
    WGPUTextureDescriptor d = WGPU_TEXTURE_DESCRIPTOR_INIT;
    d.size = {uint32_t(width), uint32_t(height), 1};
    d.mipLevelCount = MipLevels(width, height);
    d.format = WGPUTextureFormat_RGBA8Unorm;
    d.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
    return wgpuDeviceCreateTexture(GetDevice(), &d);
}

// Píxeles del cliente (RGB o RGBA, filas alineadas como pide el unpack) a RGBA.
std::vector<uint8_t> ToRGBA(int width, int height, Gfx::Format format, const void* pixels) {
    std::vector<uint8_t> out(size_t(width) * height * 4, 255);
    if (!pixels) return out;
    const int channels = format == Gfx::Format::RGB ? 3 : 4;
    const size_t align = size_t(std::max(1, g.unpackAlignment));
    const size_t stride = (size_t(width) * channels + align - 1) / align * align;
    const auto* src = static_cast<const uint8_t*>(pixels);
    for (int y = 0; y < height; ++y) {
        const uint8_t* row = src + y * stride;
        uint8_t* dst = &out[size_t(y) * width * 4];
        if (channels == 4) std::memcpy(dst, row, size_t(width) * 4);
        else for (int x = 0; x < width; ++x) { dst[x * 4] = row[x * 3]; dst[x * 4 + 1] = row[x * 3 + 1]; dst[x * 4 + 2] = row[x * 3 + 2]; }
    }
    return out;
}

void Write(WGPUTexture texture, int x, int y, int width, int height, const std::vector<uint8_t>& rgba, uint32_t level = 0) {
    WGPUTexelCopyTextureInfo dst = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
    dst.texture = texture;
    dst.mipLevel = level;
    dst.origin = {uint32_t(x), uint32_t(y), 0};
    WGPUTexelCopyBufferLayout layout = WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;
    layout.bytesPerRow = uint32_t(width) * 4;
    layout.rowsPerImage = uint32_t(height);
    const WGPUExtent3D size{uint32_t(width), uint32_t(height), 1};
    wgpuQueueWriteTexture(GetQueue(), &dst, rgba.data(), rgba.size(), &layout, &size);
}

// Niveles de mipmap de una imagen RGBA completa (promedio de 2x2), como
// glGenerateMipmap. WebGPU no los genera solo.
void WriteMipmaps(WGPUTexture texture, int width, int height, std::vector<uint8_t> rgba) {
    const uint32_t levels = MipLevels(width, height);
    for (uint32_t level = 1; level < levels; ++level) {
        const int w = std::max(1, width / 2), h = std::max(1, height / 2);
        std::vector<uint8_t> next(size_t(w) * h * 4);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 4; ++c) {
                    const int x0 = std::min(x * 2, width - 1), x1 = std::min(x * 2 + 1, width - 1);
                    const int y0 = std::min(y * 2, height - 1), y1 = std::min(y * 2 + 1, height - 1);
                    const int sum = rgba[(size_t(y0) * width + x0) * 4 + c] + rgba[(size_t(y0) * width + x1) * 4 + c] +
                                    rgba[(size_t(y1) * width + x0) * 4 + c] + rgba[(size_t(y1) * width + x1) * 4 + c];
                    next[(size_t(y) * w + x) * 4 + c] = uint8_t((sum + 2) / 4);
                }
        Write(texture, 0, 0, w, h, next, level);
        rgba.swap(next); width = w; height = h;
    }
}

// La textura ligada (RenderState) para escribirla. Si ya la usa el cuadro en
// grabación, pasa a una textura nueva; keep copia el contenido anterior.
TextureSlot* Writable(bool keep) {
    auto it = g.textures.find(RenderState::Get().texture);
    if (it == g.textures.end()) return nullptr;
    TextureSlot& slot = it->second;
    if (slot.texture && slot.usedFrame == g.frame) {
        WGPUTexture fresh = NewTexture(slot.width, slot.height);
        if (keep) {
            WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
            WGPUTexelCopyTextureInfo from = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT, to = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
            from.texture = slot.texture; to.texture = fresh;
            const WGPUExtent3D size{uint32_t(slot.width), uint32_t(slot.height), 1};
            wgpuCommandEncoderCopyTextureToTexture(encoder, &from, &to, &size);
            WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
            wgpuQueueSubmit(GetQueue(), 1, &commands);
            wgpuCommandBufferRelease(commands);
            wgpuCommandEncoderRelease(encoder);
        }
        Retire(slot);
        slot.texture = fresh;
        slot.view = wgpuTextureCreateView(fresh, nullptr);
    }
    return &slot;
}

// Con mipmaps, lo 3D (depth test) amplía con filtro lineal aunque la textura
// sea nítida: si no, el límite entre ampliación y reducción se ve al moverse.
// La interfaz conserva el filtro original.
WGPUBindGroup TextureGroup(TextureSlot& slot) {
    Sampling sampling = slot.sampling;
    if (Gfx::TextureMipmaps() && RenderState::Get().caps[RenderState::CapIndex(Cap::DepthTest)])
        sampling.magFilter = Gfx::TexValue::Linear;
    const uint32_t key = sampling.Key();
    for (const auto& entry : slot.groups) if (entry.key == key) return entry.group;
    WGPUBindGroupEntry entries[2] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
    entries[0].binding = 0; entries[0].textureView = slot.view;
    entries[1].binding = 1; entries[1].sampler = Sampler(sampling);
    WGPUBindGroupDescriptor d = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    d.layout = g.textureLayout;
    d.entryCount = 2;
    d.entries = entries;
    WGPUBindGroup group = wgpuDeviceCreateBindGroup(GetDevice(), &d);
    slot.groups.push_back({key, group});
    return group;
}

WGPURenderPipeline Pipeline(const PipelineKey& key) {
    auto it = g.pipelines.find(key.Value());
    if (it != g.pipelines.end()) return it->second;
    const bool resident = (key.topology >> 4) == Resident;
    static const WGPUVertexAttribute attributes[3] = {
        {nullptr, WGPUVertexFormat_Float32x3, offsetof(Vertex, position), 0},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Vertex, color), 1},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Vertex, uv), 2},
    };
    using Static = Skinning::StaticVertex;
    static const WGPUVertexAttribute residentAttributes[4] = {
        {nullptr, WGPUVertexFormat_Float32x3, offsetof(Static, position), 0},
        {nullptr, WGPUVertexFormat_Float32, offsetof(Static, bone), 1},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Static, normal), 2},
        {nullptr, WGPUVertexFormat_Float32x2, offsetof(Static, uv), 3},
    };
    WGPUVertexBufferLayout buffer = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    buffer.stepMode = WGPUVertexStepMode_Vertex;
    buffer.arrayStride = resident ? sizeof(Static) : sizeof(Vertex);
    buffer.attributeCount = resident ? 4 : 3;
    buffer.attributes = resident ? residentAttributes : attributes;

    WGPUBlendState blend{};
    blend.color = {WGPUBlendOperation_Add, Blend(Gfx::Factor(key.src)), Blend(Gfx::Factor(key.dst))};
    blend.alpha = blend.color;
    WGPUColorTargetState target = WGPU_COLOR_TARGET_STATE_INIT;
    target.format = SurfaceFormat();
    target.blend = key.blend ? &blend : nullptr;
    target.writeMask = WGPUColorWriteMask_All;
    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = g.module;
    fragment.entryPoint = View("fs");
    fragment.targetCount = 1;
    fragment.targets = &target;

    // Sin depth test, OpenGL tampoco escribe profundidad.
    WGPUDepthStencilState depth = WGPU_DEPTH_STENCIL_STATE_INIT;
    depth.format = WGPUTextureFormat_Depth24Plus;
    depth.depthWriteEnabled = key.depthTest && key.depthWrite ? WGPUOptionalBool_True : WGPUOptionalBool_False;
    depth.depthCompare = key.depthTest ? Compare(Gfx::Compare(key.depthFunc)) : WGPUCompareFunction_Always;

    WGPURenderPipelineDescriptor d = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    d.layout = g.pipelineLayout;
    d.multisample.count = Samples();
    d.vertex.module = g.module;
    d.vertex.entryPoint = View(resident ? "vs_skin" : "vs");
    d.vertex.bufferCount = 1;
    d.vertex.buffers = &buffer;
    d.primitive.topology = WGPUPrimitiveTopology(key.topology & 0xF);
    d.primitive.frontFace = WGPUFrontFace_CCW;
    d.primitive.cullMode = key.cull ? WGPUCullMode_Back : WGPUCullMode_None;
    d.depthStencil = &depth;
    d.fragment = &fragment;
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &d);
    g.pipelines.emplace(key.Value(), pipeline);
    return pipeline;
}

// Pipeline del pase de sombras: sólo profundidad desde la luz, sin culling.
WGPURenderPipeline CasterPipeline(uint8_t topology) {
    auto it = g.casterPipelines.find(topology);
    if (it != g.casterPipelines.end()) return it->second;
    const bool resident = (topology >> 4) == Resident;
    static const WGPUVertexAttribute attributes[3] = {
        {nullptr, WGPUVertexFormat_Float32x3, offsetof(Vertex, position), 0},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Vertex, color), 1},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Vertex, uv), 2},
    };
    using Static = Skinning::StaticVertex;
    static const WGPUVertexAttribute residentAttributes[4] = {
        {nullptr, WGPUVertexFormat_Float32x3, offsetof(Static, position), 0},
        {nullptr, WGPUVertexFormat_Float32, offsetof(Static, bone), 1},
        {nullptr, WGPUVertexFormat_Float32x4, offsetof(Static, normal), 2},
        {nullptr, WGPUVertexFormat_Float32x2, offsetof(Static, uv), 3},
    };
    WGPUVertexBufferLayout buffer = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    buffer.stepMode = WGPUVertexStepMode_Vertex;
    buffer.arrayStride = resident ? sizeof(Static) : sizeof(Vertex);
    buffer.attributeCount = resident ? 4 : 3;
    buffer.attributes = resident ? residentAttributes : attributes;
    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = g.module;
    fragment.entryPoint = View("fs_caster");
    fragment.targetCount = 0;
    WGPUDepthStencilState depth = WGPU_DEPTH_STENCIL_STATE_INIT;
    depth.format = WGPUTextureFormat_Depth32Float;
    depth.depthWriteEnabled = WGPUOptionalBool_True;
    depth.depthCompare = WGPUCompareFunction_Less;
    // Sesgo por pendiente: evita que las superficies se sombreen a sí mismas.
    depth.depthBias = 4;
    depth.depthBiasSlopeScale = 2.0f;
    WGPURenderPipelineDescriptor d = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    d.layout = g.casterPipelineLayout;
    d.vertex.module = g.module;
    d.vertex.entryPoint = View(resident ? "vs_skin_caster" : "vs_caster");
    d.vertex.bufferCount = 1;
    d.vertex.buffers = &buffer;
    d.primitive.topology = WGPUPrimitiveTopology(topology & 0xF);
    d.primitive.cullMode = WGPUCullMode_None;
    d.depthStencil = &depth;
    d.fragment = &fragment;
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &d);
    g.casterPipelines.emplace(topology, pipeline);
    return pipeline;
}

// Uniforms del lote: se reutiliza el tramo anterior si no cambió nada.
uint32_t UniformOffset() {
    const uint64_t matrices = RenderState::MatrixRevision(), material = RenderState::MaterialRevision();
    const auto& state = RenderState::Get();
    const bool additive = state.caps[RenderState::CapIndex(Cap::Blend)] && state.blendDestination == Gfx::Factor::One;
    const bool receiver = g.shadowScene && g.shadowFrame && state.caps[RenderState::CapIndex(Cap::DepthTest)] && !additive;
    if (!g.uniforms.empty() && matrices == g.matrixRevision && material == g.materialRevision &&
        receiver == g.lastReceiver) return g.lastUniform;
    Uniforms u{};
    std::memcpy(u.mvp, RenderState::ModelViewProjection(), sizeof(u.mvp));
    std::memcpy(u.modelView, RenderState::Top(Gfx::Matrix::ModelView), sizeof(u.modelView));
    std::memcpy(u.textureMatrix, RenderState::Top(Gfx::Matrix::Texture), sizeof(u.textureMatrix));
    const auto& s = RenderState::Get();
    u.flags[0] = s.caps[RenderState::CapIndex(Cap::Texture2D)] ? 1 : 0;
    u.flags[1] = EnvCode(s.texEnv);
    u.flags[2] = s.caps[RenderState::CapIndex(Cap::AlphaTest)] ? AlphaCode(s.alphaFunction) : 0;
    u.flags[3] = !s.caps[RenderState::CapIndex(Cap::Fog)] ? 0 : s.fogMode == Gfx::FogMode::Linear ? 1 : s.fogMode == Gfx::FogMode::Exp ? 2 : 3;
    u.params[0] = s.alphaReference; u.params[1] = s.fogStart; u.params[2] = s.fogEnd; u.params[3] = s.fogDensity;
    std::memcpy(u.fogColor, s.fogColor, sizeof(u.fogColor));
    // Bicúbico sólo en lo 3D (depth test); bits 1 y 2: repetir en S y en T.
    const bool bicubic = Gfx::TextureBicubic() && s.caps[RenderState::CapIndex(Cap::DepthTest)];
    u.extra[0] = bicubic ? 1 | (s.textureRepeat[0] ? 2 : 0) | (s.textureRepeat[1] ? 4 : 0) : 0;
    u.extra[1] = receiver ? 1 : 0;
    const uint32_t offset = uint32_t(g.uniforms.size());
    g.uniforms.resize(g.uniforms.size() + UniformStride);
    std::memcpy(&g.uniforms[offset], &u, sizeof(u));
    g.matrixRevision = matrices; g.materialRevision = material; g.lastUniform = offset; g.lastReceiver = receiver;
    return offset;
}

PipelineKey StateKey(uint8_t topology) {
    const auto& s = RenderState::Get();
    PipelineKey key;
    key.topology = topology;
    key.blend = s.caps[RenderState::CapIndex(Cap::Blend)];
    key.src = uint8_t(s.blendSource); key.dst = uint8_t(s.blendDestination);
    key.depthTest = s.caps[RenderState::CapIndex(Cap::DepthTest)];
    key.depthFunc = uint8_t(s.depthFunction);
    key.depthWrite = s.depthWrite;
    key.cull = s.caps[RenderState::CapIndex(Cap::CullFace)];
    return key;
}
// Proyecta sombra: dentro de la escena, opaco o con alpha test (escribe
// profundidad) y no aditivo; sólo triángulos.
bool Caster(const PipelineKey& key) {
    const uint8_t topology = key.topology & 0xF;
    return g.shadowScene && g.shadowFrame && g.shadowCasting && key.depthTest && key.depthWrite &&
           !(key.blend && Gfx::Factor(key.dst) == Gfx::Factor::One) &&
           (topology == WGPUPrimitiveTopology_TriangleList || topology == WGPUPrimitiveTopology_TriangleStrip);
}
// La textura ligada si el texturizado está activo; la blanca si no.
TextureSlot& BoundSlot() {
    const auto& s = RenderState::Get();
    if (s.caps[RenderState::CapIndex(Cap::Texture2D)]) {
        auto it = g.textures.find(s.texture);
        if (it != g.textures.end() && it->second.view) return it->second;
    }
    return g.white;
}


// Color multisample del tamaño de la superficie (sólo con MSAA).
void EnsureMsaa(unsigned width, unsigned height) {
    if (Samples() == 1) return;
    const WGPUTextureFormat format = SurfaceFormat();
    if (g.msaa && g.msaaWidth == width && g.msaaHeight == height && g.msaaFormat == format) return;
    if (g.msaaView) wgpuTextureViewRelease(g.msaaView);
    if (g.msaa) wgpuTextureRelease(g.msaa);
    WGPUTextureDescriptor d = WGPU_TEXTURE_DESCRIPTOR_INIT;
    d.size = {width, height, 1};
    d.format = format;
    d.usage = WGPUTextureUsage_RenderAttachment;
    d.sampleCount = Samples();
    g.msaa = wgpuDeviceCreateTexture(GetDevice(), &d);
    g.msaaView = wgpuTextureCreateView(g.msaa, nullptr);
    g.msaaWidth = width; g.msaaHeight = height; g.msaaFormat = format;
}
// Destino de color de una pasada: el multisample (resuelto en el cuadro) o el cuadro.
void ColorTarget(WGPURenderPassColorAttachment& color) {
    if (g.msaaView) { color.view = g.msaaView; color.resolveTarget = FrameView(); }
    else color.view = FrameView();
}

Pass& CurrentPass() {
    if (g.passes.empty()) g.passes.emplace_back();
    return g.passes.back();
}

void EnsureDepth(unsigned width, unsigned height) {
    if (g.depth && g.depthWidth == width && g.depthHeight == height) return;
    if (g.depthView) wgpuTextureViewRelease(g.depthView);
    if (g.depth) wgpuTextureRelease(g.depth);
    WGPUTextureDescriptor d = WGPU_TEXTURE_DESCRIPTOR_INIT;
    d.size = {width, height, 1};
    d.format = WGPUTextureFormat_Depth24Plus;
    // Se lee en la niebla por profundidad.
    d.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
    d.sampleCount = Samples();
    g.depth = wgpuDeviceCreateTexture(GetDevice(), &d);
    g.depthView = wgpuTextureCreateView(g.depth, nullptr);
    g.depthWidth = width; g.depthHeight = height;
}

// Buffer de al menos `size` bytes; si crece se recrea (el anterior ya se envió).
void Reserve(WGPUBuffer& buffer, uint64_t& capacity, uint64_t size, WGPUBufferUsage usage) {
    if (buffer && capacity >= size) return;
    if (buffer) wgpuBufferRelease(buffer);
    capacity = std::max<uint64_t>(size, std::max<uint64_t>(capacity * 2, 1u << 20));
    buffer = CreateBuffer(capacity, usage);
    const bool grouped = &buffer == &g.uniformBuffer || &buffer == &g.skinBuffer || &buffer == &g.paletteBuffer;
    if (grouped && g.uniformGroup) { wgpuBindGroupRelease(g.uniformGroup); g.uniformGroup = nullptr; }
}

void ReserveQuadIndices(uint64_t quads) {
    if (g.indexBuffer && g.indexQuads >= quads) return;
    if (g.indexBuffer) wgpuBufferRelease(g.indexBuffer);
    g.indexQuads = std::max<uint64_t>(quads, std::max<uint64_t>(g.indexQuads * 2, 4096));
    std::vector<uint32_t> indices;
    indices.reserve(g.indexQuads * 6);
    for (uint32_t q = 0; q < g.indexQuads; ++q) {
        const uint32_t v = q * 4;
        indices.insert(indices.end(), {v, v + 1, v + 2, v, v + 2, v + 3});
    }
    g.indexBuffer = CreateBuffer(indices.size() * 4, WGPUBufferUsage_Index);
    wgpuQueueWriteBuffer(GetQueue(), g.indexBuffer, 0, indices.data(), indices.size() * 4);
}

void ReleaseRetired() {
    for (auto group : g.retiredGroups) wgpuBindGroupRelease(group);
    for (auto view : g.retiredViews) wgpuTextureViewRelease(view);
    for (auto texture : g.retiredTextures) wgpuTextureRelease(texture);
    for (auto buffer : g.retiredBuffers) wgpuBufferRelease(buffer);
    g.retiredGroups.clear(); g.retiredViews.clear(); g.retiredTextures.clear(); g.retiredBuffers.clear();
}

void ResetFrame() {
    g.vertices.clear(); g.uniforms.clear(); g.passes.clear();
    g.skins.clear(); g.palette.clear(); g.paletteRevision = ~0ull;
    g.matrixRevision = g.materialRevision = ~0ull;
    g.shadowFrame = false;
    ++g.frame;
}

bool CreateObjects() {
    WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgsl.code = View(Shader);
    WGPUShaderModuleDescriptor md = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    md.nextInChain = &wgsl.chain;
    g.module = wgpuDeviceCreateShaderModule(GetDevice(), &md);

    WGPUBindGroupLayoutEntry uniform[3] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    uniform[0].binding = 0;
    uniform[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    uniform[0].buffer.type = WGPUBufferBindingType_Uniform;
    uniform[0].buffer.hasDynamicOffset = 1;
    uniform[0].buffer.minBindingSize = sizeof(Uniforms);
    uniform[1].binding = 1;
    uniform[1].visibility = WGPUShaderStage_Vertex;
    uniform[1].buffer.type = WGPUBufferBindingType_Uniform;
    uniform[1].buffer.hasDynamicOffset = 1;
    uniform[1].buffer.minBindingSize = sizeof(Skin);
    uniform[2].binding = 2;
    uniform[2].visibility = WGPUShaderStage_Vertex;
    uniform[2].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    WGPUBindGroupLayoutDescriptor ud = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    ud.entryCount = 3; ud.entries = uniform;
    g.uniformLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &ud);

    WGPUBindGroupLayoutEntry textureEntries[2] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    textureEntries[0].binding = 0;
    textureEntries[0].visibility = WGPUShaderStage_Fragment;
    textureEntries[0].texture.sampleType = WGPUTextureSampleType_Float;
    textureEntries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
    textureEntries[1].binding = 1;
    textureEntries[1].visibility = WGPUShaderStage_Fragment;
    textureEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;
    WGPUBindGroupLayoutDescriptor td = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    td.entryCount = 2; td.entries = textureEntries;
    g.textureLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &td);

    // Grupo 2: sombras. El pipeline normal lee el mapa; el del pase de
    // sombras sólo el uniform (el mapa es su destino).
    WGPUBindGroupLayoutEntry shadowEntries[3] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    shadowEntries[0].binding = 0;
    shadowEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    shadowEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
    shadowEntries[0].buffer.minBindingSize = sizeof(ShadowUniform);
    shadowEntries[1].binding = 1;
    shadowEntries[1].visibility = WGPUShaderStage_Fragment;
    shadowEntries[1].texture.sampleType = WGPUTextureSampleType_Depth;
    shadowEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;
    shadowEntries[2].binding = 2;
    shadowEntries[2].visibility = WGPUShaderStage_Fragment;
    shadowEntries[2].sampler.type = WGPUSamplerBindingType_Comparison;
    WGPUBindGroupLayoutDescriptor sd = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    sd.entryCount = 3; sd.entries = shadowEntries;
    g.shadowLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &sd);
    sd.entryCount = 1;
    g.casterLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &sd);

    WGPUBindGroupLayout layouts[3] = {g.uniformLayout, g.textureLayout, g.shadowLayout};
    WGPUPipelineLayoutDescriptor pd = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pd.bindGroupLayoutCount = 3; pd.bindGroupLayouts = layouts;
    g.pipelineLayout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pd);
    layouts[2] = g.casterLayout;
    g.casterPipelineLayout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pd);

    WGPUTextureDescriptor mapd = WGPU_TEXTURE_DESCRIPTOR_INIT;
    mapd.size = {ShadowSize, ShadowSize, 1};
    mapd.format = WGPUTextureFormat_Depth32Float;
    mapd.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
    g.shadowMap = wgpuDeviceCreateTexture(GetDevice(), &mapd);
    g.shadowView = wgpuTextureCreateView(g.shadowMap, nullptr);
    WGPUSamplerDescriptor smp = WGPU_SAMPLER_DESCRIPTOR_INIT;
    smp.magFilter = WGPUFilterMode_Linear;
    smp.minFilter = WGPUFilterMode_Linear;
    smp.compare = WGPUCompareFunction_LessEqual;
    g.shadowSampler = wgpuDeviceCreateSampler(GetDevice(), &smp);
    g.shadowBuffer = CreateBuffer(sizeof(ShadowUniform), WGPUBufferUsage_Uniform);
    WGPUBindGroupEntry shadowGroup[3] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
    shadowGroup[0].binding = 0; shadowGroup[0].buffer = g.shadowBuffer; shadowGroup[0].size = sizeof(ShadowUniform);
    shadowGroup[1].binding = 1; shadowGroup[1].textureView = g.shadowView;
    shadowGroup[2].binding = 2; shadowGroup[2].sampler = g.shadowSampler;
    WGPUBindGroupDescriptor gd = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    gd.layout = g.shadowLayout; gd.entryCount = 3; gd.entries = shadowGroup;
    g.shadowGroup = wgpuDeviceCreateBindGroup(GetDevice(), &gd);
    gd.layout = g.casterLayout; gd.entryCount = 1;
    g.casterGroup = wgpuDeviceCreateBindGroup(GetDevice(), &gd);

    // Textura blanca 1x1 para los lotes sin textura (el shader muestrea siempre).
    g.white.texture = NewTexture(1, 1);
    g.white.view = wgpuTextureCreateView(g.white.texture, nullptr);
    g.white.width = g.white.height = 1;
    Write(g.white.texture, 0, 0, 1, 1, std::vector<uint8_t>(4, 255));
    return g.module && g.uniformLayout && g.textureLayout && g.pipelineLayout && g.white.view &&
           g.shadowGroup && g.casterGroup && g.casterPipelineLayout;
}

void ReleaseObjects() {
    ResetFrame();
    ReleaseRetired();
    for (auto& [handle, slot] : g.textures) Destroy(slot);
    Destroy(g.white);
    for (auto& [key, pipeline] : g.pipelines) wgpuRenderPipelineRelease(pipeline);
    for (auto& [key, pipeline] : g.casterPipelines) wgpuRenderPipelineRelease(pipeline);
    if (g.atmosphereMultiply) wgpuRenderPipelineRelease(g.atmosphereMultiply);
    if (g.atmosphereAdd) wgpuRenderPipelineRelease(g.atmosphereAdd);
    if (g.atmospherePipelineLayout) wgpuPipelineLayoutRelease(g.atmospherePipelineLayout);
    if (g.atmosphereLayout) wgpuBindGroupLayoutRelease(g.atmosphereLayout);
    if (g.atmosphereModule) wgpuShaderModuleRelease(g.atmosphereModule);
    if (g.atmosphereBuffer) wgpuBufferRelease(g.atmosphereBuffer);
    if (g.shadowGroup) wgpuBindGroupRelease(g.shadowGroup);
    if (g.casterGroup) wgpuBindGroupRelease(g.casterGroup);
    if (g.shadowBuffer) wgpuBufferRelease(g.shadowBuffer);
    if (g.shadowSampler) wgpuSamplerRelease(g.shadowSampler);
    if (g.shadowView) wgpuTextureViewRelease(g.shadowView);
    if (g.shadowMap) wgpuTextureRelease(g.shadowMap);
    if (g.casterPipelineLayout) wgpuPipelineLayoutRelease(g.casterPipelineLayout);
    if (g.casterLayout) wgpuBindGroupLayoutRelease(g.casterLayout);
    if (g.shadowLayout) wgpuBindGroupLayoutRelease(g.shadowLayout);
    for (auto& [key, sampler] : g.samplers) wgpuSamplerRelease(sampler);
    for (auto& [key, mesh] : g.meshes) {
        if (mesh.indices) wgpuBufferRelease(mesh.indices);
        if (mesh.vertices) wgpuBufferRelease(mesh.vertices);
    }
    if (g.skinBuffer) wgpuBufferRelease(g.skinBuffer);
    if (g.paletteBuffer) wgpuBufferRelease(g.paletteBuffer);
    if (g.uniformGroup) wgpuBindGroupRelease(g.uniformGroup);
    if (g.vertexBuffer) wgpuBufferRelease(g.vertexBuffer);
    if (g.uniformBuffer) wgpuBufferRelease(g.uniformBuffer);
    if (g.indexBuffer) wgpuBufferRelease(g.indexBuffer);
    if (g.depthView) wgpuTextureViewRelease(g.depthView);
    if (g.msaaView) wgpuTextureViewRelease(g.msaaView);
    if (g.msaa) wgpuTextureRelease(g.msaa);
    if (g.postGroup) wgpuBindGroupRelease(g.postGroup);
    if (g.postView) wgpuTextureViewRelease(g.postView);
    if (g.postTexture) wgpuTextureRelease(g.postTexture);
    if (g.postUniform) wgpuBufferRelease(g.postUniform);
    if (g.postPipeline) wgpuRenderPipelineRelease(g.postPipeline);
    if (g.fxaaPipeline) wgpuRenderPipelineRelease(g.fxaaPipeline);
    if (g.postSampler) wgpuSamplerRelease(g.postSampler);
    if (g.postPipelineLayout) wgpuPipelineLayoutRelease(g.postPipelineLayout);
    if (g.postLayout) wgpuBindGroupLayoutRelease(g.postLayout);
    if (g.postModule) wgpuShaderModuleRelease(g.postModule);
    if (g.depth) wgpuTextureRelease(g.depth);
    if (g.pipelineLayout) wgpuPipelineLayoutRelease(g.pipelineLayout);
    if (g.textureLayout) wgpuBindGroupLayoutRelease(g.textureLayout);
    if (g.uniformLayout) wgpuBindGroupLayoutRelease(g.uniformLayout);
    if (g.module) wgpuShaderModuleRelease(g.module);
    g = State{};
}
WGPUBuffer UploadBuffer(const void* data, uint64_t size, WGPUBufferUsage usage) {
    WGPUBuffer buffer = CreateBuffer((size + 3) & ~uint64_t(3), usage);
    wgpuQueueWriteBuffer(GetQueue(), buffer, 0, data, size);
    return buffer;
}
void RetireMesh(GpuMesh& mesh, bool vertices) {
    WGPUBuffer& buffer = vertices ? mesh.vertices : mesh.indices;
    if (!buffer) return;
    if (mesh.usedFrame == g.frame) g.retiredBuffers.push_back(buffer); else wgpuBufferRelease(buffer);
    buffer = nullptr;
}
} // namespace

// Graba una malla residente con la pose capturada. false si la pose no
// corresponde al dibujo: el caller dibuja por el camino de referencia.
bool DrawResidentMesh(const void* key, MeshTopology::Mesh& topology, const Skinning::ResidentDraw& d) {
    if (!g.ready) return false;
    const Skinning::Pose::Mesh* pose = Skinning::Pose::Find(d.model, d.key, d.world, d.positionCount);
    if (!pose) return false;
    const auto& bones = Skinning::Pose::Palette();
    if (d.maxBone < 0 || std::size_t(d.maxBone) * 12 >= bones.size()) return false;
    if (d.lit && !Skinning::Pose::LightMatches(*pose, d.colors, d.normalCount, d.bodyLight)) return false;
    BufferedDraw::Flush();

    GpuMesh& mesh = g.meshes[key];
    if (!mesh.indices || mesh.revision != topology.revision) {
        RetireMesh(mesh, false); RetireMesh(mesh, true);
        mesh.indices = UploadBuffer(topology.indices.data(), topology.indices.size() * sizeof(unsigned), WGPUBufferUsage_Index);
        mesh.revision = topology.revision; mesh.residentRevision = 0;
    }
    if (!mesh.vertices || mesh.residentRevision != topology.residentRevision) {
        RetireMesh(mesh, true);
        mesh.vertices = UploadBuffer(topology.resident.data(), topology.resident.size() * sizeof(Skinning::StaticVertex), WGPUBufferUsage_Vertex);
        mesh.residentRevision = topology.residentRevision;
    }
    mesh.usedFrame = g.frame;

    // La paleta de la pose se agrega una vez por cuadro y por pose.
    if (g.paletteRevision != Skinning::Pose::Revision()) {
        g.paletteBase = int32_t(g.palette.size() / 4);
        g.palette.insert(g.palette.end(), bones.begin(), bones.end());
        g.paletteRevision = Skinning::Pose::Revision();
    }
    Skin skin{};
    skin.paletteBase = g.paletteBase;
    skin.flags = (d.lit ? 1u : 0u) | (d.shadow ? 2u : 0u) | (d.textured ? 4u : 0u);
    skin.scales[0] = Skinning::Pose::RotationScale(); skin.scales[1] = Skinning::Pose::BodyScale();
    std::memcpy(skin.origin, Skinning::Pose::Origin(), 3 * sizeof(float));
    std::memcpy(skin.direction, Skinning::Pose::Direction(), 3 * sizeof(float));
    std::memcpy(skin.coefficients, Skinning::Pose::Coefficients(), 3 * sizeof(float));
    if (d.bodyLight) std::memcpy(skin.bodyLight, d.bodyLight, 3 * sizeof(float));
    skin.uvOffset[0] = d.uOffset; skin.uvOffset[1] = d.vOffset;
    if (d.shadow && d.shadowOrigin) std::memcpy(skin.shadowOrigin, d.shadowOrigin, 3 * sizeof(float));
    if (d.color) std::memcpy(skin.color, d.color, 4 * sizeof(float));
    if (d.uv) std::memcpy(skin.uv, d.uv, 4 * sizeof(float));
    const uint32_t skinOffset = uint32_t(g.skins.size());
    g.skins.resize(g.skins.size() + UniformStride);
    std::memcpy(&g.skins[skinOffset], &skin, sizeof(skin));

    TextureSlot& slot = BoundSlot();
    Draw draw{};
    const PipelineKey residentKey = StateKey(uint8_t(WGPUPrimitiveTopology_TriangleList | (Resident << 4)));
    draw.pipeline = Pipeline(residentKey);
    draw.caster = Caster(residentKey) && !d.shadow;   // la sombra plana no proyecta
    draw.textureGroup = TextureGroup(slot);
    slot.usedFrame = g.frame;
    draw.uniformOffset = UniformOffset();
    draw.skinOffset = skinOffset;
    std::memcpy(draw.viewport, RenderState::Get().viewport, sizeof(draw.viewport));
    draw.vertexBuffer = mesh.vertices;
    draw.indexBuffer = mesh.indices;
    draw.indexCount = uint32_t(d.indices);
    CurrentPass().draws.push_back(draw);
    return true;
}

void SetResident(bool enabled) { g.residentEnabled = enabled; }
bool ResidentEnabled() { return g.residentEnabled; }
void ReleaseMeshes() {
    for (auto& [key, mesh] : g.meshes) { RetireMesh(mesh, false); RetireMesh(mesh, true); }
    g.meshes.clear();
}

namespace {
const char* const PostShader = R"WGSL(
struct Post { amount: f32, pad0: f32, pad1: f32, pad2: f32 };
@group(0) @binding(0) var src: texture_2d<f32>;
@group(0) @binding(1) var<uniform> post: Post;
@group(0) @binding(2) var smp: sampler;
@vertex fn vs_post(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
    let p = vec2f(select(-1.0, 3.0, i == 1u), select(-1.0, 3.0, i == 2u));
    return vec4f(p, 0.0, 1.0);
}
fn at(p: vec2i) -> vec3f {
    let size = vec2i(textureDimensions(src, 0));
    return textureLoad(src, clamp(p, vec2i(0), size - 1), 0).rgb;
}
@fragment fn fs_post(@builtin(position) position: vec4f) -> @location(0) vec4f {
    let p = vec2i(position.xy);
    let a = at(p + vec2i(-1, -1)); let b = at(p + vec2i(0, -1)); let c = at(p + vec2i(1, -1));
    let d = at(p + vec2i(-1, 0));  let e = at(p);                let f = at(p + vec2i(1, 0));
    let g = at(p + vec2i(-1, 1));  let h = at(p + vec2i(0, 1));  let i = at(p + vec2i(1, 1));
    var mn = min(min(min(d, e), min(f, b)), h); mn = mn + min(mn, min(min(a, c), min(g, i)));
    var mx = max(max(max(d, e), max(f, b)), h); mx = mx + max(mx, max(max(a, c), max(g, i)));
    let amp = sqrt(clamp(min(mn, 2.0 - mx) / max(mx, vec3f(1e-5)), vec3f(0.0), vec3f(1.0)));
    // Realce del laplaciano, atenuado por amp en las zonas de mucho contraste.
    // Las diferencias muy chicas (ruido, transición entre mipmaps) se ignoran.
    var l = 4.0 * e - b - d - f - h;
    l = sign(l) * max(abs(l) - vec3f(0.06), vec3f(0.0));
    return vec4f(clamp(e + post.amount * amp * l, vec3f(0.0), vec3f(1.0)), 1.0);
}
fn lum(c: vec3f) -> f32 { return dot(c, vec3f(0.299, 0.587, 0.114)); }
fn sampleAt(uv: vec2f) -> vec3f { return textureSampleLevel(src, smp, uv, 0.0).rgb; }
// FXAA (versión de consola de Timothy Lottes).
@fragment fn fs_fxaa(@builtin(position) position: vec4f) -> @location(0) vec4f {
    let texel = 1.0 / vec2f(textureDimensions(src, 0));
    let uv = position.xy * texel;
    let rgbM = sampleAt(uv);
    let nw = lum(sampleAt(uv + vec2f(-1.0, -1.0) * texel)); let ne = lum(sampleAt(uv + vec2f(1.0, -1.0) * texel));
    let sw = lum(sampleAt(uv + vec2f(-1.0, 1.0) * texel));  let se = lum(sampleAt(uv + vec2f(1.0, 1.0) * texel));
    let m = lum(rgbM);
    let lmin = min(m, min(min(nw, ne), min(sw, se)));
    let lmax = max(m, max(max(nw, ne), max(sw, se)));
    // Sólo bordes con contraste real: las texturas quedan intactas.
    if (lmax - lmin < max(0.0625, lmax * 0.125)) { return vec4f(rgbM, 1.0); }
    var dir = vec2f(-((nw + ne) - (sw + se)), (nw + sw) - (ne + se));
    let reduce = max((nw + ne + sw + se) * 0.03125, 1.0 / 128.0);
    dir = clamp(dir / (min(abs(dir.x), abs(dir.y)) + reduce), vec2f(-8.0), vec2f(8.0)) * texel;
    let a = 0.5 * (sampleAt(uv + dir * (1.0 / 3.0 - 0.5)) + sampleAt(uv + dir * (2.0 / 3.0 - 0.5)));
    let b = a * 0.5 + 0.25 * (sampleAt(uv - dir * 0.5) + sampleAt(uv + dir * 0.5));
    let lb = lum(b);
    return vec4f(select(b, a, lb < lmin || lb > lmax), 1.0);
}
)WGSL";

WGPURenderPipeline PostPipeline(const char* entry, WGPUTextureFormat format) {
    WGPUColorTargetState target = WGPU_COLOR_TARGET_STATE_INIT;
    target.format = format;
    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = g.postModule;
    fragment.entryPoint = View(entry);
    fragment.targetCount = 1;
    fragment.targets = &target;
    WGPURenderPipelineDescriptor d = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    d.layout = g.postPipelineLayout;
    d.multisample.count = Samples();
    d.vertex.module = g.postModule;
    d.vertex.entryPoint = View("vs_post");
    d.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    d.fragment = &fragment;
    return wgpuDeviceCreateRenderPipeline(GetDevice(), &d);
}

// Objetos del posproceso; los pipelines dependen del formato de la superficie.
bool EnsurePost(unsigned width, unsigned height) {
    const WGPUTextureFormat format = SurfaceFormat();
    if (!g.postModule) {
        WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
        wgsl.code = View(PostShader);
        WGPUShaderModuleDescriptor md = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
        md.nextInChain = &wgsl.chain;
        g.postModule = wgpuDeviceCreateShaderModule(GetDevice(), &md);
        WGPUBindGroupLayoutEntry entries[3] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
        entries[0].binding = 0;
        entries[0].visibility = WGPUShaderStage_Fragment;
        entries[0].texture.sampleType = WGPUTextureSampleType_Float;
        entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
        entries[1].binding = 1;
        entries[1].visibility = WGPUShaderStage_Fragment;
        entries[1].buffer.type = WGPUBufferBindingType_Uniform;
        entries[1].buffer.minBindingSize = 16;
        entries[2].binding = 2;
        entries[2].visibility = WGPUShaderStage_Fragment;
        entries[2].sampler.type = WGPUSamplerBindingType_Filtering;
        WGPUBindGroupLayoutDescriptor ld = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
        ld.entryCount = 3; ld.entries = entries;
        g.postLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &ld);
        WGPUPipelineLayoutDescriptor pd = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
        pd.bindGroupLayoutCount = 1; pd.bindGroupLayouts = &g.postLayout;
        g.postPipelineLayout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pd);
        g.postUniform = CreateBuffer(16, WGPUBufferUsage_Uniform);
        WGPUSamplerDescriptor sd = WGPU_SAMPLER_DESCRIPTOR_INIT;
        sd.magFilter = WGPUFilterMode_Linear;
        sd.minFilter = WGPUFilterMode_Linear;
        g.postSampler = wgpuDeviceCreateSampler(GetDevice(), &sd);
    }
    if (!g.postModule || !g.postLayout || !g.postPipelineLayout || !g.postUniform || !g.postSampler) return false;
    if (!g.postPipeline || g.postFormat != format) {
        if (g.postPipeline) wgpuRenderPipelineRelease(g.postPipeline);
        if (g.fxaaPipeline) wgpuRenderPipelineRelease(g.fxaaPipeline);
        g.postPipeline = PostPipeline("fs_post", format);
        g.fxaaPipeline = PostPipeline("fs_fxaa", format);
        g.postFormat = format;
        if (g.postGroup) { wgpuBindGroupRelease(g.postGroup); g.postGroup = nullptr; }
        if (g.postView) { wgpuTextureViewRelease(g.postView); g.postView = nullptr; }
        if (g.postTexture) { wgpuTextureRelease(g.postTexture); g.postTexture = nullptr; }
    }
    if (!g.postTexture || g.postWidth != width || g.postHeight != height) {
        if (g.postGroup) { wgpuBindGroupRelease(g.postGroup); g.postGroup = nullptr; }
        if (g.postView) wgpuTextureViewRelease(g.postView);
        if (g.postTexture) wgpuTextureRelease(g.postTexture);
        WGPUTextureDescriptor td = WGPU_TEXTURE_DESCRIPTOR_INIT;
        td.size = {width, height, 1};
        td.format = format;
        td.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
        g.postTexture = wgpuDeviceCreateTexture(GetDevice(), &td);
        g.postView = wgpuTextureCreateView(g.postTexture, nullptr);
        g.postWidth = width; g.postHeight = height;
    }
    if (!g.postGroup) {
        WGPUBindGroupEntry entries[3] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
        entries[0].binding = 0; entries[0].textureView = g.postView;
        entries[1].binding = 1; entries[1].buffer = g.postUniform; entries[1].size = 16;
        entries[2].binding = 2; entries[2].sampler = g.postSampler;
        WGPUBindGroupDescriptor d = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
        d.layout = g.postLayout; d.entryCount = 3; d.entries = entries;
        g.postGroup = wgpuDeviceCreateBindGroup(GetDevice(), &d);
    }
    return g.postPipeline && g.fxaaPipeline && g.postGroup;
}

// Copia el cuadro y lo redibuja con el filtro dentro del viewport de la pasada.
void EncodePost(WGPUCommandEncoder encoder, const Pass& pass, unsigned width, unsigned height) {
    const int* v = pass.postViewport;
    const float x = float(std::max(0, v[0]));
    const float w = std::min(float(v[2]), float(width) - x);
    const float top = float(int(height) - v[1] - v[3]);
    const float y = std::max(0.0f, top);
    const float h = std::min(float(v[3]) - (y - top), float(height) - y);
    if (w <= 0 || h <= 0 || !EnsurePost(width, height)) return;
    const float uniform[4] = {pass.sharpen, 0, 0, 0};
    wgpuQueueWriteBuffer(GetQueue(), g.postUniform, 0, uniform, sizeof(uniform));
    auto run = [&](WGPURenderPipeline pipeline) {
        WGPUTexelCopyTextureInfo from = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT, to = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
        from.texture = FrameTexture(); to.texture = g.postTexture;
        const WGPUExtent3D size{width, height, 1};
        wgpuCommandEncoderCopyTextureToTexture(encoder, &from, &to, &size);
        WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        ColorTarget(color);
        color.loadOp = WGPULoadOp_Load;
        color.storeOp = WGPUStoreOp_Store;
        WGPURenderPassDescriptor rp = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        rp.colorAttachmentCount = 1;
        rp.colorAttachments = &color;
        WGPURenderPassEncoder p = wgpuCommandEncoderBeginRenderPass(encoder, &rp);
        wgpuRenderPassEncoderSetViewport(p, x, y, w, h, 0, 1);
        wgpuRenderPassEncoderSetPipeline(p, pipeline);
        wgpuRenderPassEncoderSetBindGroup(p, 0, g.postGroup, 0, nullptr);
        wgpuRenderPassEncoderDraw(p, 3, 1, 0, 0);
        wgpuRenderPassEncoderEnd(p);
        wgpuRenderPassEncoderRelease(p);
    };
    if (pass.antialias) run(g.fxaaPipeline);
    if (pass.sharpen > 0.0f) run(g.postPipeline);
}

// ── Niebla por profundidad ──────────────────────────────────────────────────
// Reconstruye cada píxel desde la profundidad y mezcla con el color de la
// niebla en dos pasadas (multiplicar por la transmitancia y sumar el color),
// sin copiar la imagen.
const char* const AtmosphereShader = R"WGSL(
struct Atmosphere { inverseViewProj: mat4x4f, camera: vec4f, color: vec4f, params: vec4f, viewport: vec4f };
@group(0) @binding(0) var depth: DEPTH_TYPE;
@group(0) @binding(1) var<uniform> a: Atmosphere;
@vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
    let p = vec2f(select(-1.0, 3.0, i == 1u), select(-1.0, 3.0, i == 2u));
    return vec4f(p, 0.0, 1.0);
}
// Transmitancia por canal: 1 = sin niebla.
fn transmittance(position: vec4f) -> vec3f {
    let d = textureLoad(depth, vec2i(position.xy), 0);
    if (d >= 1.0) { return vec3f(0.0); }   // fondo: todo niebla
    let ndc = vec2f((position.x - a.viewport.x) / a.viewport.z * 2.0 - 1.0,
                    1.0 - (position.y - a.viewport.y) / a.viewport.w * 2.0);
    var w = a.inverseViewProj * vec4f(ndc, d * 2.0 - 1.0, 1.0);
    let distance = max(length(w.xyz / w.w - a.camera.xyz) - a.params.z, 0.0);
    if (a.params.x > 1.5) {
        // Bajo el agua: el rojo se absorbe primero.
        return exp(-a.params.y * vec3f(3.0, 1.8, 1.2) * distance);
    }
    let x = distance * a.params.y;
    return vec3f(exp(-x * x));
}
@fragment fn fs_multiply(@builtin(position) position: vec4f) -> @location(0) vec4f {
    return vec4f(transmittance(position), 1.0);
}
@fragment fn fs_add(@builtin(position) position: vec4f) -> @location(0) vec4f {
    return vec4f(a.color.rgb * (vec3f(1.0) - transmittance(position)), 0.0);
}
)WGSL";
bool EnsureAtmosphere() {
    const WGPUTextureFormat format = SurfaceFormat();
    const uint32_t samples = Samples();
    if (g.atmosphereMultiply && g.atmosphereFormat == format && g.atmosphereSamples == samples) return true;
    if (g.atmosphereMultiply) wgpuRenderPipelineRelease(g.atmosphereMultiply);
    if (g.atmosphereAdd) wgpuRenderPipelineRelease(g.atmosphereAdd);
    if (g.atmospherePipelineLayout) wgpuPipelineLayoutRelease(g.atmospherePipelineLayout);
    if (g.atmosphereLayout) wgpuBindGroupLayoutRelease(g.atmosphereLayout);
    if (g.atmosphereModule) wgpuShaderModuleRelease(g.atmosphereModule);
    g.atmosphereMultiply = g.atmosphereAdd = nullptr;
    std::string code = AtmosphereShader;
    const char* type = samples > 1 ? "texture_depth_multisampled_2d" : "texture_depth_2d";
    code.replace(code.find("DEPTH_TYPE"), 10, type);
    WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgsl.code = WGPUStringView{code.c_str(), code.size()};
    WGPUShaderModuleDescriptor md = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    md.nextInChain = &wgsl.chain;
    g.atmosphereModule = wgpuDeviceCreateShaderModule(GetDevice(), &md);
    WGPUBindGroupLayoutEntry entries[2] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Fragment;
    entries[0].texture.sampleType = WGPUTextureSampleType_Depth;
    entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
    entries[0].texture.multisampled = samples > 1;
    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Fragment;
    entries[1].buffer.type = WGPUBufferBindingType_Uniform;
    entries[1].buffer.minBindingSize = sizeof(State::Atmosphere);
    WGPUBindGroupLayoutDescriptor ld = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    ld.entryCount = 2; ld.entries = entries;
    g.atmosphereLayout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &ld);
    WGPUPipelineLayoutDescriptor pd = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pd.bindGroupLayoutCount = 1; pd.bindGroupLayouts = &g.atmosphereLayout;
    g.atmospherePipelineLayout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pd);
    if (!g.atmosphereBuffer) g.atmosphereBuffer = CreateBuffer(sizeof(State::Atmosphere), WGPUBufferUsage_Uniform);
    auto pipeline = [&](const char* entry, WGPUBlendFactor src, WGPUBlendFactor dst) {
        WGPUBlendState blend{};
        blend.color = {WGPUBlendOperation_Add, src, dst};
        blend.alpha = {WGPUBlendOperation_Add, WGPUBlendFactor_Zero, WGPUBlendFactor_One};
        WGPUColorTargetState target = WGPU_COLOR_TARGET_STATE_INIT;
        target.format = format;
        target.blend = &blend;
        target.writeMask = WGPUColorWriteMask_All;
        WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
        fragment.module = g.atmosphereModule;
        fragment.entryPoint = View(entry);
        fragment.targetCount = 1;
        fragment.targets = &target;
        WGPURenderPipelineDescriptor d = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
        d.layout = g.atmospherePipelineLayout;
        d.multisample.count = samples;
        d.vertex.module = g.atmosphereModule;
        d.vertex.entryPoint = View("vs");
        d.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        d.fragment = &fragment;
        return wgpuDeviceCreateRenderPipeline(GetDevice(), &d);
    };
    g.atmosphereMultiply = pipeline("fs_multiply", WGPUBlendFactor_Zero, WGPUBlendFactor_Src);
    g.atmosphereAdd = pipeline("fs_add", WGPUBlendFactor_One, WGPUBlendFactor_One);
    g.atmosphereFormat = format; g.atmosphereSamples = samples;
    return g.atmosphereMultiply && g.atmosphereAdd && g.atmosphereBuffer;
}
void EncodeAtmosphere(WGPUCommandEncoder encoder, unsigned width, unsigned height) {
    if (!g.depthView || !EnsureAtmosphere()) return;
    // Viewport de OpenGL (origen abajo) a píxeles desde arriba, dentro del cuadro.
    const int* v = g.atmosphereViewport;
    const float x = float(std::max(0, v[0]));
    const float w = std::min(float(v[2]), float(width) - x);
    const float top = float(int(height) - v[1] - v[3]);
    const float y = std::max(0.0f, top);
    const float h = std::min(float(v[3]) - (y - top), float(height) - y);
    if (w <= 0 || h <= 0) return;
    g.atmosphere.viewport[0] = float(v[0]); g.atmosphere.viewport[1] = top;
    g.atmosphere.viewport[2] = float(v[2]); g.atmosphere.viewport[3] = float(v[3]);
    wgpuQueueWriteBuffer(GetQueue(), g.atmosphereBuffer, 0, &g.atmosphere, sizeof(g.atmosphere));
    WGPUBindGroupEntry entries[2] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
    entries[0].binding = 0; entries[0].textureView = g.depthView;
    entries[1].binding = 1; entries[1].buffer = g.atmosphereBuffer; entries[1].size = sizeof(State::Atmosphere);
    WGPUBindGroupDescriptor bd = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bd.layout = g.atmosphereLayout; bd.entryCount = 2; bd.entries = entries;
    WGPUBindGroup group = wgpuDeviceCreateBindGroup(GetDevice(), &bd);
    WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    ColorTarget(color);
    color.loadOp = WGPULoadOp_Load;
    color.storeOp = WGPUStoreOp_Store;
    WGPURenderPassDescriptor rp = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    rp.colorAttachmentCount = 1;
    rp.colorAttachments = &color;
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &rp);
    wgpuRenderPassEncoderSetViewport(pass, x, y, w, h, 0, 1);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, group, 0, nullptr);
    wgpuRenderPassEncoderSetPipeline(pass, g.atmosphereMultiply);
    wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
    wgpuRenderPassEncoderSetPipeline(pass, g.atmosphereAdd);
    wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
    g.retiredGroups.push_back(group);
}

// Pase de sombras: los dibujos marcados como proyectores, vistos desde la luz.
void EncodeShadows(WGPUCommandEncoder encoder) {
    WGPURenderPassDepthStencilAttachment depth = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
    depth.view = g.shadowView;
    depth.depthLoadOp = WGPULoadOp_Clear;
    depth.depthStoreOp = WGPUStoreOp_Store;
    depth.depthClearValue = 1.0f;
    WGPURenderPassDescriptor rp = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    rp.colorAttachmentCount = 0;
    rp.depthStencilAttachment = &depth;
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &rp);
    wgpuRenderPassEncoderSetBindGroup(pass, 2, g.casterGroup, 0, nullptr);
    WGPURenderPipeline lastPipeline = nullptr;
    WGPUBindGroup lastTexture = nullptr;
    WGPUBuffer lastVertices = nullptr, lastIndices = nullptr;
    uint32_t lastUniform = ~0u, lastSkin = ~0u;
    for (const auto& recorded : g.passes) {
        for (const auto& draw : recorded.draws) {
            if (!draw.caster) continue;
            const uint8_t topology = draw.vertexBuffer ? uint8_t(WGPUPrimitiveTopology_TriangleList | (Resident << 4))
                                   : uint8_t(draw.quads ? WGPUPrimitiveTopology_TriangleList : draw.topology);
            const WGPURenderPipeline pipeline = CasterPipeline(topology);
            if (pipeline != lastPipeline) { wgpuRenderPassEncoderSetPipeline(pass, pipeline); lastPipeline = pipeline; }
            if (draw.uniformOffset != lastUniform || draw.skinOffset != lastSkin) {
                const uint32_t offsets[2] = {draw.uniformOffset, draw.skinOffset};
                wgpuRenderPassEncoderSetBindGroup(pass, 0, g.uniformGroup, 2, offsets);
                lastUniform = draw.uniformOffset; lastSkin = draw.skinOffset;
            }
            const WGPUBuffer vertices = draw.vertexBuffer ? draw.vertexBuffer : g.vertexBuffer;
            if (vertices != lastVertices) { wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertices, 0, WGPU_WHOLE_SIZE); lastVertices = vertices; }
            const WGPUBuffer indices = draw.vertexBuffer ? draw.indexBuffer : draw.quads ? g.indexBuffer : nullptr;
            if (indices && indices != lastIndices) {
                wgpuRenderPassEncoderSetIndexBuffer(pass, indices, WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
                lastIndices = indices;
            }
            if (draw.textureGroup != lastTexture) { wgpuRenderPassEncoderSetBindGroup(pass, 1, draw.textureGroup, 0, nullptr); lastTexture = draw.textureGroup; }
            if (draw.vertexBuffer) wgpuRenderPassEncoderDrawIndexed(pass, draw.indexCount, 1, 0, 0, 0);
            else if (draw.quads) wgpuRenderPassEncoderDrawIndexed(pass, draw.vertexCount / 4 * 6, 1, 0, int32_t(draw.firstVertex), 0);
            else wgpuRenderPassEncoderDraw(pass, draw.vertexCount, 1, draw.firstVertex, 0);
        }
    }
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
}

// Codifica y envía lo grabado hasta ahora sobre la textura del cuadro. La
// grabación sigue después sobre la misma textura (lo usa la captura).
void EncodePasses() {
    if (g.passes.empty() || !BeginFrame()) { ResetFrame(); ReleaseRetired(); return; }
    const unsigned width = Width(), height = Height();
    EnsureDepth(width, height);
    EnsureMsaa(width, height);
    if (!g.vertices.empty()) {
        Reserve(g.vertexBuffer, g.vertexCapacity, g.vertices.size() * sizeof(Vertex), WGPUBufferUsage_Vertex);
        wgpuQueueWriteBuffer(GetQueue(), g.vertexBuffer, 0, g.vertices.data(), g.vertices.size() * sizeof(Vertex));
    }
    if (!g.uniforms.empty()) {
        // El grupo 0 siempre lleva skin y paleta, aunque el cuadro no tenga mallas.
        if (g.skins.empty()) g.skins.resize(UniformStride);
        if (g.palette.empty()) g.palette.resize(4);
        Reserve(g.uniformBuffer, g.uniformCapacity, g.uniforms.size(), WGPUBufferUsage_Uniform);
        Reserve(g.skinBuffer, g.skinCapacity, g.skins.size(), WGPUBufferUsage_Uniform);
        Reserve(g.paletteBuffer, g.paletteCapacity, g.palette.size() * sizeof(float), WGPUBufferUsage_Storage);
        wgpuQueueWriteBuffer(GetQueue(), g.uniformBuffer, 0, g.uniforms.data(), g.uniforms.size());
        wgpuQueueWriteBuffer(GetQueue(), g.skinBuffer, 0, g.skins.data(), g.skins.size());
        wgpuQueueWriteBuffer(GetQueue(), g.paletteBuffer, 0, g.palette.data(), g.palette.size() * sizeof(float));
        if (!g.uniformGroup) {
            WGPUBindGroupEntry entries[3] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
            entries[0].binding = 0; entries[0].buffer = g.uniformBuffer; entries[0].size = sizeof(Uniforms);
            entries[1].binding = 1; entries[1].buffer = g.skinBuffer; entries[1].size = sizeof(Skin);
            entries[2].binding = 2; entries[2].buffer = g.paletteBuffer; entries[2].size = g.paletteCapacity;
            WGPUBindGroupDescriptor d = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
            d.layout = g.uniformLayout; d.entryCount = 3; d.entries = entries;
            g.uniformGroup = wgpuDeviceCreateBindGroup(GetDevice(), &d);
        }
    }
    uint64_t quads = 0;
    for (const auto& pass : g.passes)
        for (const auto& draw : pass.draws) if (draw.quads) quads = std::max<uint64_t>(quads, draw.vertexCount / 4);
    if (quads) ReserveQuadIndices(quads);

    // Sombras: el uniform va siempre (el grupo 2 está en todos los pipelines);
    // sin escena en el cuadro, los receptores no leen el mapa.
    g.shadow.params[0] = g.shadowFrame ? 1.0f : 0.0f;
    wgpuQueueWriteBuffer(GetQueue(), g.shadowBuffer, 0, &g.shadow, sizeof(g.shadow));
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    if (g.shadowFrame && g.uniformGroup) EncodeShadows(encoder);
    for (const auto& pass : g.passes) {
        if (pass.atmosphere) EncodeAtmosphere(encoder, width, height);
        if (pass.Post()) EncodePost(encoder, pass, width, height);
        WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        ColorTarget(color);
        color.loadOp = pass.clearColor ? WGPULoadOp_Clear : WGPULoadOp_Load;
        color.storeOp = WGPUStoreOp_Store;
        color.clearValue = {pass.color[0], pass.color[1], pass.color[2], pass.color[3]};
        WGPURenderPassDepthStencilAttachment depth = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
        depth.view = g.depthView;
        depth.depthLoadOp = pass.clearDepth ? WGPULoadOp_Clear : WGPULoadOp_Load;
        depth.depthStoreOp = WGPUStoreOp_Store;
        depth.depthClearValue = 1.0f;
        WGPURenderPassDescriptor rp = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        rp.colorAttachmentCount = 1;
        rp.colorAttachments = &color;
        rp.depthStencilAttachment = &depth;
        WGPURenderPassEncoder encoderPass = wgpuCommandEncoderBeginRenderPass(encoder, &rp);
        wgpuRenderPassEncoderSetBindGroup(encoderPass, 2, g.shadowGroup, 0, nullptr);
        // Dawn valida cada comando: sólo se emite el estado que cambia.
        WGPURenderPipeline lastPipeline = nullptr;
        WGPUBindGroup lastTexture = nullptr;
        WGPUBuffer lastVertices = nullptr, lastIndices = nullptr;
        uint32_t lastUniform = ~0u, lastSkin = ~0u;
        int lastViewport[4]{-1, -1, -1, -1};
        for (const auto& draw : pass.draws) {
            if (std::memcmp(lastViewport, draw.viewport, sizeof(lastViewport))) {
                // Viewport de OpenGL (origen abajo) a WebGPU (origen arriba), dentro del cuadro.
                const float x = float(std::max(0, draw.viewport[0]));
                const float w = std::min(float(draw.viewport[2]), float(width) - x);
                const float top = float(int(height) - draw.viewport[1] - draw.viewport[3]);
                const float y = std::max(0.0f, top);
                const float h = std::min(float(draw.viewport[3]) - (y - top), float(height) - y);
                if (w <= 0 || h <= 0) continue;
                wgpuRenderPassEncoderSetViewport(encoderPass, x, y, w, h, 0, 1);
                std::memcpy(lastViewport, draw.viewport, sizeof(lastViewport));
            }
            if (draw.pipeline != lastPipeline) { wgpuRenderPassEncoderSetPipeline(encoderPass, draw.pipeline); lastPipeline = draw.pipeline; }
            if (draw.uniformOffset != lastUniform || draw.skinOffset != lastSkin) {
                const uint32_t offsets[2] = {draw.uniformOffset, draw.skinOffset};
                wgpuRenderPassEncoderSetBindGroup(encoderPass, 0, g.uniformGroup, 2, offsets);
                lastUniform = draw.uniformOffset; lastSkin = draw.skinOffset;
            }
            const WGPUBuffer vertices = draw.vertexBuffer ? draw.vertexBuffer : g.vertexBuffer;
            if (vertices != lastVertices) {
                wgpuRenderPassEncoderSetVertexBuffer(encoderPass, 0, vertices, 0, WGPU_WHOLE_SIZE);
                lastVertices = vertices;
            }
            const WGPUBuffer indices = draw.vertexBuffer ? draw.indexBuffer : draw.quads ? g.indexBuffer : nullptr;
            if (indices && indices != lastIndices) {
                wgpuRenderPassEncoderSetIndexBuffer(encoderPass, indices, WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
                lastIndices = indices;
            }
            if (draw.textureGroup != lastTexture) {
                wgpuRenderPassEncoderSetBindGroup(encoderPass, 1, draw.textureGroup, 0, nullptr);
                lastTexture = draw.textureGroup;
            }
            if (draw.vertexBuffer) wgpuRenderPassEncoderDrawIndexed(encoderPass, draw.indexCount, 1, 0, 0, 0);
            else if (draw.quads) wgpuRenderPassEncoderDrawIndexed(encoderPass, draw.vertexCount / 4 * 6, 1, 0, int32_t(draw.firstVertex), 0);
            else wgpuRenderPassEncoderDraw(encoderPass, draw.vertexCount, 1, draw.firstVertex, 0);
        }
        wgpuRenderPassEncoderEnd(encoderPass);
        wgpuRenderPassEncoderRelease(encoderPass);
    }
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(encoder);
    ResetFrame();
    ReleaseRetired();
}
}

// Envía el cuadro grabado y presenta. Lo llama CWindow::Present.
void SubmitFrame() {
    if (!g.ready) return;
    EncodePasses();
    if (FrameView()) Present();
}

// glReadPixels sobre el cuadro en curso: envía lo grabado, copia la región a
// un buffer y la lee esperando a la GPU. Filas de abajo hacia arriba, en RGB.
void ReadPixels(int x, int y, int width, int height, void* out) {
    auto* dst = static_cast<uint8_t*>(out);
    std::memset(dst, 0, size_t(width) * size_t(height) * 3);
    if (!g.ready) return;
    EncodePasses();
    if (!BeginFrame()) return;
    const int surfaceW = int(Width()), surfaceH = int(Height());
    const int x0 = std::max(0, x), x1 = std::min(surfaceW, x + width);
    const int y0 = std::max(0, y), y1 = std::min(surfaceH, y + height);
    if (x1 <= x0 || y1 <= y0) return;
    const uint32_t w = uint32_t(x1 - x0), h = uint32_t(y1 - y0);
    const uint32_t bytesPerRow = (w * 4 + 255) & ~255u;
    WGPUBufferDescriptor bd = WGPU_BUFFER_DESCRIPTOR_INIT;
    bd.size = uint64_t(bytesPerRow) * h;
    bd.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    WGPUBuffer readback = wgpuDeviceCreateBuffer(GetDevice(), &bd);
    WGPUTexelCopyTextureInfo from = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
    from.texture = FrameTexture();
    from.origin = {uint32_t(x0), uint32_t(surfaceH - y1), 0};   // origen de OpenGL abajo
    WGPUTexelCopyBufferInfo to = WGPU_TEXEL_COPY_BUFFER_INFO_INIT;
    to.buffer = readback;
    to.layout.bytesPerRow = bytesPerRow;
    to.layout.rowsPerImage = h;
    const WGPUExtent3D size{w, h, 1};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &from, &to, &size);
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(encoder);
    struct Result { bool ok = false; } result;
    WGPUBufferMapCallbackInfo callback = WGPU_BUFFER_MAP_CALLBACK_INFO_INIT;
    callback.mode = WGPUCallbackMode_WaitAnyOnly;
    callback.callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* user, void*) {
        static_cast<Result*>(user)->ok = status == WGPUMapAsyncStatus_Success;
    };
    callback.userdata1 = &result;
    if (WaitFuture(wgpuBufferMapAsync(readback, WGPUMapMode_Read, 0, size_t(bd.size), callback)) && result.ok) {
        const auto* src = static_cast<const uint8_t*>(wgpuBufferGetConstMappedRange(readback, 0, size_t(bd.size)));
        const bool bgra = SurfaceFormat() == WGPUTextureFormat_BGRA8Unorm;
        // Fila r del resultado (desde abajo) = fila h-1-r de la copia (desde arriba).
        for (uint32_t r = 0; src && r < h; ++r) {
            const uint8_t* row = src + size_t(h - 1 - r) * bytesPerRow;
            uint8_t* line = dst + (size_t(y0 - y + int(r)) * size_t(width) + size_t(x0 - x)) * 3;
            for (uint32_t c = 0; c < w; ++c) {
                line[c * 3 + 0] = row[c * 4 + (bgra ? 2 : 0)];
                line[c * 3 + 1] = row[c * 4 + 1];
                line[c * 3 + 2] = row[c * 4 + (bgra ? 0 : 2)];
            }
        }
        wgpuBufferUnmap(readback);
    }
    wgpuBufferRelease(readback);
}

} // namespace WebGPU

// ── GfxDevice ───────────────────────────────────────────────────────────────
namespace GfxDevice::WGPU {
using namespace Gfx;
using ::WebGPU::g;
void Reset() {
    // Como el viewport inicial de OpenGL: la superficie entera.
    auto& v = RenderState::Mutable().viewport;
    v[0] = 0; v[1] = 0; v[2] = int(::WebGPU::Width()); v[3] = int(::WebGPU::Height());
}
// El estado se lee de RenderState al grabar cada lote.
void ApplyCap(Cap, bool) {}
void ApplyBlend(Factor, Factor) {}
void ApplyDepthFunc(Compare) {}
void ApplyDepthMask(bool) {}
void ApplyTexture(Texture) {}
void ApplyViewport(int, int, int, int) {}
void ClearColor(float r, float g_, float b, float a) {
    g.clearColor[0] = r; g.clearColor[1] = g_; g.clearColor[2] = b; g.clearColor[3] = a;
}
void Clear(unsigned clearMask) {
    if (!g.ready) return;
    // Un Clear antes de dibujar se suma a la pasada; si no, abre otra.
    if (g.passes.empty() || !g.passes.back().draws.empty() || g.passes.back().Post()) g.passes.emplace_back();
    auto& pass = g.passes.back();
    if (clearMask & ClearColorBuffer) {
        pass.clearColor = true;
        std::memcpy(pass.color, g.clearColor, sizeof(pass.color));
    }
    if (clearMask & ClearDepthBuffer) pass.clearDepth = true;
}
void Flush() {}
void ClearErrors() {}
bool SupportsShadowMaps() { return g.ready; }
// Abre o cierra la escena de sombras del cuadro. Al abrirla, la vista activa
// es la de la cámara: con su inversa cada dibujo recupera su posición en el
// mundo, cualquiera sea su modelView.
void ShadowScene(bool begin, const float* focus, float strength) {
    if (!g.ready) return;
    g.matrixRevision = ~0ull;   // los uniforms siguientes cambian el flag de receptor
    if (!begin) {
        // Niebla: una pasada nueva que se aplica antes de lo que siga.
        if (g.shadowScene && g.atmosphere.params[0] > 0.5f) {
            g.passes.emplace_back();
            g.passes.back().atmosphere = true;
        }
        g.shadowScene = false;
        return;
    }
    g.shadowScene = focus != nullptr;
    g.shadowCasting = true;
    g.atmosphere.params[0] = 0.0f;
    if (!g.shadowScene) return;
    if (!ShadowLight::Invert(RenderState::Top(Gfx::Matrix::ModelView), g.shadow.inverseView)) { g.shadowScene = false; return; }
    // Para la niebla: inversa de proyección x vista, cámara y viewport.
    float viewProj[16];
    ShadowLight::Multiply(RenderState::Top(Gfx::Matrix::Projection), RenderState::Top(Gfx::Matrix::ModelView), viewProj);
    if (!ShadowLight::Invert(viewProj, g.atmosphere.inverseViewProj)) { g.shadowScene = false; return; }
    std::memcpy(g.atmosphere.camera, &g.shadow.inverseView[12], 3 * sizeof(float));
    std::memcpy(g.atmosphereViewport, RenderState::Get().viewport, sizeof(g.atmosphereViewport));
    if (strength <= 0.0f) return;   // escena sólo para la niebla
    ShadowLight::ViewProj(focus, g.shadow.lightViewProj);
    g.shadow.params[1] = strength;
    g.shadow.params[2] = 1.0f / float(::WebGPU::ShadowSize);
    g.shadow.params[3] = ShadowLight::Bias;
    g.shadowFrame = true;
}
void Atmosphere(int mode, const float* color, float density, float start) {
    if (!g.ready || !g.shadowScene) return;
    g.atmosphere.params[0] = float(mode);
    g.atmosphere.params[1] = density;
    g.atmosphere.params[2] = start;
    if (color) std::memcpy(g.atmosphere.color, color, 3 * sizeof(float));
}
void ShadowCasting(bool enabled) { g.shadowCasting = enabled; }
// El posproceso abre una pasada: al codificarla se aplica antes de sus dibujos.
void PostProcess(bool antialias, float sharpen) {
    if (!g.ready) return;
    g.passes.emplace_back();
    auto& pass = g.passes.back();
    pass.antialias = antialias;
    pass.sharpen = sharpen;
    std::memcpy(pass.postViewport, RenderState::Get().viewport, sizeof(pass.postViewport));
}
void ReadPixelsRGB(int x, int y, int width, int height, void* out) {
    if (out && width > 0 && height > 0) ::WebGPU::ReadPixels(x, y, width, height, out);
}
int MaxTextureSize() { return 8192; }
void MaxViewportDims(int out[2]) { out[0] = out[1] = 8192; }
const char* GetDeviceInfo(DeviceInfo info) {
    switch (info) {
    case DeviceInfo::Vendor: return "WebGPU";
    case DeviceInfo::Renderer: return ::WebGPU::Description();
    default: return "Dawn";
    }
}
Texture CreateTexture() {
    const Texture handle = g.nextTexture++;
    g.textures.emplace(handle, ::WebGPU::TextureSlot{});
    return handle;
}
void DeleteTexture(Texture texture) {
    auto it = g.textures.find(texture);
    if (it == g.textures.end()) return;
    if (it->second.usedFrame == g.frame) ::WebGPU::Retire(it->second);
    else ::WebGPU::Destroy(it->second);
    g.textures.erase(it);
}
void TexImage2D(int, int width, int height, Format format, const void* pixels) {
    if (!g.ready || width <= 0 || height <= 0) return;
    ::WebGPU::TextureSlot* slot = ::WebGPU::Writable(false);
    if (!slot) return;
    if (!slot->texture || slot->width != width || slot->height != height) {
        if (slot->usedFrame == g.frame) ::WebGPU::Retire(*slot); else ::WebGPU::Destroy(*slot);
        slot->texture = ::WebGPU::NewTexture(width, height);
        slot->view = wgpuTextureCreateView(slot->texture, nullptr);
        slot->width = width; slot->height = height;
    }
    auto rgba = ::WebGPU::ToRGBA(width, height, format, pixels);
    ::WebGPU::Write(slot->texture, 0, 0, width, height, rgba);
    if (Gfx::TextureMipmaps()) ::WebGPU::WriteMipmaps(slot->texture, width, height, std::move(rgba));
}
void TexSubImage2D(int x, int y, int width, int height, Format format, const void* pixels) {
    if (!g.ready || width <= 0 || height <= 0) return;
    ::WebGPU::TextureSlot* slot = ::WebGPU::Writable(true);
    if (!slot || !slot->texture || x < 0 || y < 0 || x + width > slot->width || y + height > slot->height) return;
    // Sólo el nivel 0: las texturas que se actualizan por partes (texto,
    // marcas) usan filtro nítido, que no muestrea los mipmaps.
    ::WebGPU::Write(slot->texture, x, y, width, height, ::WebGPU::ToRGBA(width, height, format, pixels));
}
void TexParameter(TexParam parameter, TexValue value) {
    auto it = g.textures.find(RenderState::Get().texture);
    if (it == g.textures.end()) return;
    auto& s = it->second.sampling;
    switch (parameter) {
    case TexParam::MinFilter: s.minFilter = value; break;
    case TexParam::MagFilter: s.magFilter = value; break;
    case TexParam::WrapS: s.wrapS = value; break;
    case TexParam::WrapT: s.wrapT = value; break;
    }
}
void TexBorderColor(const float*) {}
void SetUnpackAlignment(int alignment) { g.unpackAlignment = alignment; }
int UnpackAlignment() { return g.unpackAlignment; }
}

// ── BufferedDraw ────────────────────────────────────────────────────────────
namespace BufferedDraw::WGPU {
using ::WebGPU::g;
bool Initialize() {
    if (g.ready) return true;
    if (!::WebGPU::IsReady() || !::WebGPU::CreateObjects()) { ::WebGPU::ReleaseObjects(); return false; }
    g.ready = true;
    Gfx::ResetState();
    return true;
}
void Release() { if (g.ready) ::WebGPU::ReleaseObjects(); }
void Submit(unsigned mode, const Vertex* vertices, std::size_t count) {
    if (!g.ready || !count) return;
    const auto& s = RenderState::Get();
    const bool quads = mode == ::WebGPU::Quads;
    WGPUPrimitiveTopology topology;
    switch (mode) {
    case ::WebGPU::Points: topology = WGPUPrimitiveTopology_PointList; break;
    case ::WebGPU::Lines: topology = WGPUPrimitiveTopology_LineList; break;
    case ::WebGPU::LineLoop:
    case ::WebGPU::LineStrip: topology = WGPUPrimitiveTopology_LineStrip; break;
    case ::WebGPU::TriangleStrip: topology = WGPUPrimitiveTopology_TriangleStrip; break;
    default: topology = WGPUPrimitiveTopology_TriangleList; break;
    }
    const ::WebGPU::PipelineKey key = ::WebGPU::StateKey(uint8_t(topology));
    ::WebGPU::TextureSlot* slot = &::WebGPU::BoundSlot();
    ::WebGPU::Draw draw{};
    draw.pipeline = ::WebGPU::Pipeline(key);
    draw.textureGroup = ::WebGPU::TextureGroup(*slot);
    slot->usedFrame = g.frame;
    draw.uniformOffset = ::WebGPU::UniformOffset();
    draw.firstVertex = uint32_t(g.vertices.size());
    draw.quads = quads;
    draw.caster = ::WebGPU::Caster(key);
    draw.topology = uint8_t(topology);
    std::memcpy(draw.viewport, s.viewport, sizeof(draw.viewport));
    g.vertices.insert(g.vertices.end(), vertices, vertices + count);
    if (mode == ::WebGPU::LineLoop) g.vertices.push_back(vertices[0]);   // cerrar el lazo
    draw.vertexCount = uint32_t(g.vertices.size()) - draw.firstVertex;
    // Un lote igual al anterior y pegado a él se suma a ese dibujo (no en
    // strips: unirlos conectaría primitivas que no lo están).
    auto& draws = ::WebGPU::CurrentPass().draws;
    const bool list = key.topology != WGPUPrimitiveTopology_LineStrip && key.topology != WGPUPrimitiveTopology_TriangleStrip;
    if (list && !draws.empty()) {
        auto& last = draws.back();
        if (!last.vertexBuffer && last.pipeline == draw.pipeline && last.textureGroup == draw.textureGroup &&
            last.uniformOffset == draw.uniformOffset && last.quads == draw.quads && last.caster == draw.caster &&
            last.firstVertex + last.vertexCount == draw.firstVertex &&
            !std::memcmp(last.viewport, draw.viewport, sizeof(draw.viewport))) {
            last.vertexCount += draw.vertexCount;
            return;
        }
    }
    draws.push_back(draw);
}
void Suspend() {}
// Mallas residentes y skinning de GL: en WebGPU llegan en una etapa siguiente.
void Activate(unsigned, unsigned, unsigned) {}
void DrawIndexed(unsigned, unsigned, const Vertex*, std::size_t, std::size_t) {}
}
