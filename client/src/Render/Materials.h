#pragma once
// DESVIACION R4a: descripción semántica del estado heredado. Keep se usa sólo
// durante la transición; los materiales completos de cada pasada lo retirarán.
namespace Materials {
enum class Switch { Keep, Off, On };
enum class Blend { Keep=-1, Opaque=0, LightMap=1, Alpha=2, Add=3, Attenuate=4, InverseAdd=5 };
enum class Fog { Keep, Scene, OffWhenScene };
struct Material {
    Blend blend=Blend::Keep;
    Switch cull=Switch::Keep,depthWrite=Switch::Keep,alphaTest=Switch::Keep,texture=Switch::Keep,depthTest=Switch::Keep;
    Fog fog=Fog::Keep;
};
// Entrada de la vista 3D: sólo el estado que establece GL_BeginViewport.
// Blend, bindings y orientación de caras se conservan como en la referencia.
enum class Compare { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
enum class ViewFog { Off, Linear, ExpSquared };
struct ViewState {
    bool alphaTest=false,texture=true,depthTest=true,cull=true,depthWrite=true;
    Compare depthFunction=Compare::LessEqual,alphaFunction=Compare::Greater;
    float alphaReference=.25f;
    ViewFog fog=ViewFog::Off;
    float fogStart=0,fogEnd=1,fogDensity=1,fogColor[4]{};
};
bool BeginView(const ViewState& state);
// Entrada 2D en píxeles. Sólo declara proyección/viewport y depth test Off;
// las ventanas con modelos pueden abrir después su propia vista 3D.
struct OverlayState {
    unsigned width=640,height=480;
    float fov=45,nearClip=10,farClip=10000;
};
bool BeginOverlay(const OverlayState& state);
bool EndOverlay();

enum class Preset { Opaque, LightMap, Alpha, Add, Attenuate, InverseAdd, Untextured };
constexpr Material Describe(Preset preset,bool depthWrite=true) {
    using S=Switch;using B=Blend;
    switch(preset) {
    case Preset::Opaque:return {B::Opaque,S::On,S::On,S::Off,S::On,S::Keep,Fog::Scene};
    case Preset::LightMap:return {B::LightMap,S::On,S::On,S::Off,S::On,S::Keep,Fog::Scene};
    case Preset::Alpha:return {B::Alpha,S::Off,depthWrite?S::On:S::Keep,S::On,S::On,S::Keep,Fog::Scene};
    case Preset::Add:return {B::Add,S::Off,S::Off,S::Off,S::On,S::Keep,Fog::OffWhenScene};
    case Preset::Attenuate:return {B::Attenuate,S::Off,S::Off,S::Off,S::On,S::Keep,Fog::Scene};
    case Preset::InverseAdd:return {B::InverseAdd,S::Off,S::Off,S::Off,S::On,S::Keep,Fog::Scene};
    case Preset::Untextured:return {B::Keep,S::Keep,S::On,depthWrite?S::On:S::Off,S::Off,S::Keep,Fog::Keep};
    }
    return {};
}
// La referencia sigue disponible hasta validar la nueva ruta en el cliente.
bool Enabled();
void Toggle();
bool Select(Preset preset,bool depthWrite=true);
bool Apply(const Material& material);
// El slot es un recurso del cliente; el identificador nativo se resuelve en GL.
// Los índices negativos heredados conservan el camino de referencia.
struct TextureSlot { int index=0; };
struct TexturedMaterial { TextureSlot texture;Material material; };
bool BindTexture(TextureSlot texture);
bool ApplyTextured(const TexturedMaterial& material);
}
