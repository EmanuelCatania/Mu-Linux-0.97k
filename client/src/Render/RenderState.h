#pragma once
#include "Render/Gfx.h"
#include <cstdint>

// Estado de render del cliente, guardado en CPU. Reproduce lo que el 0.97k
// dejaba en el pipeline fijo de OpenGL (capacidades, mezcla, alpha test,
// niebla, texture env, color/UV actuales, posición raster, pilas de matrices
// y de atributos). Los backends lo leen para armar sus uniforms; ninguno
// consulta el estado al driver.
namespace RenderState {
constexpr int CapCount=12;
constexpr int StackDepth=32;

struct Fixed {
    bool caps[CapCount]{};
    Gfx::Factor blendSource=Gfx::Factor::One,blendDestination=Gfx::Factor::Zero;
    Gfx::Compare depthFunction=Gfx::Compare::Less;
    bool depthWrite=true;
    Gfx::Compare alphaFunction=Gfx::Compare::Always;
    float alphaReference=0;
    Gfx::FogMode fogMode=Gfx::FogMode::Exp;
    float fogStart=0,fogEnd=1,fogDensity=1,fogColor[4]{};
    Gfx::TexEnv texEnv=Gfx::TexEnv::Modulate;
    unsigned texture=0;
    float color[4]{1,1,1,1},uv[4]{0,0,0,1};
    // glRasterPos guarda también el color actual en el momento de fijarla.
    float raster[4]{0,0,0,1},rasterColor[4]{1,1,1,1};bool rasterValid=true;
    int viewport[4]{};
};

const Fixed& Get();
Fixed& Mutable();
inline int CapIndex(Gfx::Cap cap) { return static_cast<int>(cap); }
inline bool Enabled(Gfx::Cap cap) { return Get().caps[CapIndex(cap)]; }

// Revisiones: cambian cada vez que se modifica algo que viaja como uniform.
std::uint64_t MatrixRevision();
std::uint64_t MaterialRevision();
void TouchMaterial();

// Pilas de matrices por columnas, con la semántica de OpenGL: un push con la
// pila llena o un pop con un solo elemento no hacen nada (y queda un error).
void SetMatrixMode(Gfx::Matrix mode);
Gfx::Matrix MatrixMode();
bool PushMatrix();
bool PopMatrix();
void LoadIdentity();
void LoadMatrix(const float m[16]);
void MultiplyMatrix(const float m[16]);
void Translate(float x,float y,float z);
void Rotate(float degrees,float x,float y,float z);
void Ortho(double left,double right,double bottom,double top,double zNear,double zFar);
void Perspective(double fovy,double aspect,double zNear,double zFar);
const float* Top(Gfx::Matrix which);
// Proyección × modelview, recalculada sólo cuando cambia alguna.
const float* ModelViewProjection();

// glRasterPos: transforma el punto con las matrices actuales y el viewport.
void SetRasterPosition(float x,float y,float z);
void AdvanceRaster(float dx,float dy);

// Pila de atributos. Pop devuelve el estado previo para que el backend
// reaplique al dispositivo lo que cambió.
bool PushAttrib(unsigned mask);
bool PopAttrib(Fixed& previous);
// Error pendiente de las pilas (push lleno / pop vacío), como glGetError.
bool TakeStackError();
void Reset();
}
