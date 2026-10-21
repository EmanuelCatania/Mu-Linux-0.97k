#pragma once
#include <vector>

// DESVIACION: captura de atributos y envío por VBO sin cambiar el orden de
// las primitivas ni el estado de materiales que establecen los callers.
namespace BufferedDraw {
struct Vertex { float position[3],color[4],uv[4]; };
struct Stream {
    bool active=false;
    unsigned mode=0;
    float color[4]{1,1,1,1},uv[4]{0,0,0,1};
    std::vector<Vertex> vertices;
};
extern Stream stream;
bool Initialize();
void Release();
void Begin(unsigned mode);
void End();
void Flush();
void NativeColor(float r,float g,float b,float a);
void NativeUV(float u,float v);
void NativeVertex(float x,float y,float z);
inline void Color4f(float r,float g,float b,float a) {
    if (!stream.active) { NativeColor(r,g,b,a); return; }
    stream.color[0]=r;stream.color[1]=g;stream.color[2]=b;stream.color[3]=a;
}
inline void Color3f(float r,float g,float b) { Color4f(r,g,b,1); }
inline void Color3fv(const float* p) { Color3f(p[0],p[1],p[2]); }
inline void Color4fv(const float* p) { Color4f(p[0],p[1],p[2],p[3]); }
inline void Color4ub(unsigned char r,unsigned char g,unsigned char b,unsigned char a) { Color4f(r/255.f,g/255.f,b/255.f,a/255.f); }
inline void TexCoord2f(float u,float v) {
    if (!stream.active) { NativeUV(u,v); return; }
    stream.uv[0]=u;stream.uv[1]=v;stream.uv[2]=0;stream.uv[3]=1;
}
inline void Vertex3f(float x,float y,float z) {
    if (!stream.active) { NativeVertex(x,y,z); return; }
    const auto* c=stream.color;const auto* t=stream.uv;
    stream.vertices.push_back({{x,y,z},{c[0],c[1],c[2],c[3]},{t[0],t[1],t[2],t[3]}});
}
inline void Vertex3fv(const float* p) { Vertex3f(p[0],p[1],p[2]); }
inline void Vertex2f(float x,float y) { Vertex3f(x,y,0); }
}
