// Sin PCH: estado de render independiente de la API gráfica.
#include "Render/RenderState.h"
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace RenderState {
namespace {
Fixed state;
struct Stack { std::array<std::array<float,16>,StackDepth> m{};int top=0; };
Stack stacks[3];
Gfx::Matrix mode=Gfx::Matrix::ModelView;
std::uint64_t matrixRevision=1,materialRevision=1,mvpRevision=0;
float mvp[16]{};
bool stackError=false;
struct Saved { unsigned mask;Fixed fixed; };
std::vector<Saved> attributes;

constexpr float Identity[16]{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
Stack& Current() { return stacks[static_cast<int>(mode)]; }
float* CurrentTop() { auto& s=Current();return s.m[s.top].data(); }
// r = a × b, por columnas (convención de OpenGL).
void Multiply(const float* a,const float* b,float* r) {
    float t[16];
    for(int c=0;c<4;++c)for(int row=0;row<4;++row) {
        float v=0;for(int k=0;k<4;++k)v+=a[k*4+row]*b[c*4+k];
        t[c*4+row]=v;
    }
    std::memcpy(r,t,sizeof(t));
}
void Changed() { ++matrixRevision; }
void MultiplyCurrent(const double* m) {
    float f[16];for(int i=0;i<16;++i)f[i]=static_cast<float>(m[i]);
    float* top=CurrentTop();Multiply(top,f,top);Changed();
}
}

const Fixed& Get() { return state; }
Fixed& Mutable() { return state; }
std::uint64_t MatrixRevision() { return matrixRevision; }
std::uint64_t MaterialRevision() { return materialRevision; }
void TouchMaterial() { ++materialRevision; }

void SetMatrixMode(Gfx::Matrix next) { mode=next; }
Gfx::Matrix MatrixMode() { return mode; }
bool PushMatrix() {
    auto& s=Current();
    if(s.top+1>=StackDepth) { stackError=true;return false; }
    s.m[s.top+1]=s.m[s.top];++s.top;return true;
}
bool PopMatrix() {
    auto& s=Current();
    if(s.top==0) { stackError=true;return false; }
    --s.top;Changed();return true;
}
void LoadIdentity() { std::memcpy(CurrentTop(),Identity,sizeof(Identity));Changed(); }
void LoadMatrix(const float m[16]) { std::memcpy(CurrentTop(),m,16*sizeof(float));Changed(); }
void MultiplyMatrix(const float m[16]) { float* top=CurrentTop();Multiply(top,m,top);Changed(); }
void Translate(float x,float y,float z) {
    const float t[16]{1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1};
    MultiplyMatrix(t);
}
void Rotate(float degrees,float x,float y,float z) {
    const double length=std::sqrt(double(x)*x+double(y)*y+double(z)*z);
    if(length==0)return;
    const double a=double(degrees)*3.14159265358979323846/180.0,c=std::cos(a),s=std::sin(a);
    const double nx=x/length,ny=y/length,nz=z/length,t=1-c;
    const double r[16]{
        nx*nx*t+c,    ny*nx*t+nz*s, nx*nz*t-ny*s, 0,
        nx*ny*t-nz*s, ny*ny*t+c,    ny*nz*t+nx*s, 0,
        nx*nz*t+ny*s, ny*nz*t-nx*s, nz*nz*t+c,    0,
        0,0,0,1};
    MultiplyCurrent(r);
}
void Ortho(double l,double r,double b,double t,double n,double f) {
    if(l==r||b==t||n==f)return;
    const double m[16]{2/(r-l),0,0,0, 0,2/(t-b),0,0, 0,0,-2/(f-n),0, -(r+l)/(r-l),-(t+b)/(t-b),-(f+n)/(f-n),1};
    MultiplyCurrent(m);
}
// Misma construcción que gluPerspective: se descarta si el campo es degenerado.
void Perspective(double fovy,double aspect,double zNear,double zFar) {
    const double radians=fovy/2*3.14159265358979323846/180.0,deltaZ=zFar-zNear,sine=std::sin(radians);
    if(deltaZ==0||sine==0||aspect==0)return;
    const double cotangent=std::cos(radians)/sine;
    const double m[16]{cotangent/aspect,0,0,0, 0,cotangent,0,0, 0,0,-(zFar+zNear)/deltaZ,-1, 0,0,-2*zNear*zFar/deltaZ,0};
    MultiplyCurrent(m);
}
const float* Top(Gfx::Matrix which) { const auto& s=stacks[static_cast<int>(which)];return s.m[s.top].data(); }
const float* ModelViewProjection() {
    if(mvpRevision!=matrixRevision) {
        Multiply(Top(Gfx::Matrix::Projection),Top(Gfx::Matrix::ModelView),mvp);mvpRevision=matrixRevision;
    }
    return mvp;
}

void SetRasterPosition(float x,float y,float z) {
    const float* m=ModelViewProjection();
    float clip[4];
    for(int row=0;row<4;++row)clip[row]=m[row]*x+m[4+row]*y+m[8+row]*z+m[12+row];
    const float w=clip[3];
    std::memcpy(state.rasterColor,state.color,sizeof(state.rasterColor));
    state.rasterValid=w>0&&std::fabs(clip[0])<=w&&std::fabs(clip[1])<=w&&std::fabs(clip[2])<=w;
    if(!state.rasterValid)return;
    const auto* v=state.viewport;
    state.raster[0]=v[0]+(clip[0]/w+1)*v[2]*0.5f;
    state.raster[1]=v[1]+(clip[1]/w+1)*v[3]*0.5f;
    state.raster[2]=(clip[2]/w+1)*0.5f;state.raster[3]=w;
}
void AdvanceRaster(float dx,float dy) { if(state.rasterValid){state.raster[0]+=dx;state.raster[1]+=dy;} }

bool PushAttrib(unsigned mask) {
    if(attributes.size()>=StackDepth) { stackError=true;return false; }
    attributes.push_back({mask,state});return true;
}
bool PopAttrib(Fixed& previous) {
    if(attributes.empty()) { stackError=true;return false; }
    const auto saved=attributes.back();attributes.pop_back();
    previous=state;const auto& s=saved.fixed;const unsigned m=saved.mask;
    auto cap=[&](Gfx::Cap c){state.caps[CapIndex(c)]=s.caps[CapIndex(c)];};
    if(m&Gfx::AttribCurrent) {
        std::memcpy(state.color,s.color,sizeof(state.color));std::memcpy(state.uv,s.uv,sizeof(state.uv));
        std::memcpy(state.raster,s.raster,sizeof(state.raster));state.rasterValid=s.rasterValid;
        std::memcpy(state.rasterColor,s.rasterColor,sizeof(state.rasterColor));
    }
    if(m&Gfx::AttribEnable)std::memcpy(state.caps,s.caps,sizeof(state.caps));
    if(m&Gfx::AttribTexture) { state.texture=s.texture;state.texEnv=s.texEnv; }
    if(m&Gfx::AttribList)state.listBase=s.listBase;
    if(m&Gfx::AttribColorBuffer) {
        cap(Gfx::Cap::Blend);cap(Gfx::Cap::AlphaTest);cap(Gfx::Cap::Dither);cap(Gfx::Cap::ColorLogicOp);
        state.blendSource=s.blendSource;state.blendDestination=s.blendDestination;
        state.alphaFunction=s.alphaFunction;state.alphaReference=s.alphaReference;
    }
    if(m&Gfx::AttribViewport)std::memcpy(state.viewport,s.viewport,sizeof(state.viewport));
    if(m&Gfx::AttribFog) {
        cap(Gfx::Cap::Fog);state.fogMode=s.fogMode;state.fogStart=s.fogStart;state.fogEnd=s.fogEnd;
        state.fogDensity=s.fogDensity;std::memcpy(state.fogColor,s.fogColor,sizeof(state.fogColor));
    }
    if(m&Gfx::AttribDepth) { cap(Gfx::Cap::DepthTest);state.depthFunction=s.depthFunction;state.depthWrite=s.depthWrite; }
    TouchMaterial();return true;
}
bool TakeStackError() { const bool e=stackError;stackError=false;return e; }
void Reset() {
    state=Fixed{};attributes.clear();mode=Gfx::Matrix::ModelView;stackError=false;
    for(auto& s:stacks){s.top=0;std::memcpy(s.m[0].data(),Identity,sizeof(Identity));}
    // OpenGL arranca con dither activado.
    state.caps[CapIndex(Gfx::Cap::Dither)]=true;
    ++matrixRevision;++materialRevision;
}
namespace { struct Startup { Startup() { Reset(); } } startup; }
}
