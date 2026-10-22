#pragma once
#include <cstdint>
#include <vector>
// DESVIACION R3: puente temporal de los recursos 0.97k, sin tipos de GL.
// El backend importa el handle existente; no cambia texturas ni sus píxeles.
namespace TerrainDraw {
enum class Layer { None, Alpha, Water };
struct Texture { std::uint32_t handle=0;int slot=0,width=0,height=0,channels=0;const void* pixels=nullptr; };
struct Tile {
    int x=0,y=0;unsigned indices[4]{};
    float positions[4][3]{},uv[4][4]{};
    Texture base,upper;Layer layer=Layer::None;
};
struct FrameData {
    int map=-1;float scale=100;
    const float* heights=nullptr;const float* light=nullptr;const float* alpha=nullptr;
    bool fogPolicy=false;std::uint64_t textureEpoch=0;
};
using Reference=void(*)(const Tile&,const FrameData&,bool draw);
struct Result { bool drawn=false;std::uint64_t draws=0,referenceTiles=0,geometryBytes=0,lightBytes=0,dataBytes=0;const char* reason=""; };
void Toggle();bool Requested();bool Collecting();const char* Status();
void InvalidateTextures();void InvalidateBoundTexture();void InvalidateTexture(std::uint32_t handle);void Release();
void Begin(FrameData frame,Reference reference);
void Submit(const Tile& tile);
bool End();
}
