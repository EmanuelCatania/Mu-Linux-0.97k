#pragma once
#include <cstdint>

// Capa de render: el estado y los recursos que usa el juego, sin tipos ni
// constantes de OpenGL. El backend GL (Render/Backend/GL/Gfx.cpp) traduce
// cada llamada a la misma función GL que usaba el código heredado, con las
// mismas barreras del lote (GLCallBarrier), así el orden de envío no cambia.
// La geometría va por BufferedDraw; esto cubre estado, matrices, texturas,
// niebla, consultas y las listas de glifos del texto.
namespace Gfx {

enum class Cap { Texture2D, Blend, AlphaTest, DepthTest, CullFace, Fog, ScissorTest, StencilTest, Lighting, Dither, PolygonSmooth, ColorLogicOp };
enum class Factor { Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha, DstColor, OneMinusDstColor, SrcAlphaSaturate };
enum class Compare { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
enum class Matrix { ModelView, Projection, Texture };
enum class Format { Alpha, RGB, RGBA, Luminance, LuminanceAlpha };
enum class TexParam { MinFilter, MagFilter, WrapS, WrapT };
enum class TexValue { Nearest, Linear, NearestMipmapNearest, LinearMipmapNearest, NearestMipmapLinear, LinearMipmapLinear, Repeat, Clamp, ClampToEdge };
enum class TexEnv { Modulate, Replace, Decal, Blend, Add };
enum class FogMode { Linear, Exp, Exp2 };
enum class DeviceInfo { Vendor, Renderer, Version };
// Máscaras combinables con |.
enum ClearMask : unsigned { ClearColorBuffer = 1, ClearDepthBuffer = 2, ClearStencilBuffer = 4 };
enum AttribMask : unsigned { AttribCurrent = 1, AttribTexture = 2, AttribEnable = 4, AttribList = 8 };

// Estado
void Enable(Cap cap);
void Disable(Cap cap);
inline void SetEnabled(Cap cap, bool on) { on ? Enable(cap) : Disable(cap); }
void BlendFunc(Factor source, Factor destination);
void DepthFunc(Compare function);
void DepthMask(bool write);
void AlphaFunc(Compare function, float reference);
void PushAttrib(unsigned attribMask);
void PopAttrib();

// Color actual (el que toma BufferedDraw al empezar una primitiva)
void Color3f(float r, float g, float b);
void Color4f(float r, float g, float b, float a);
void Color4fv(const float* rgba);
void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void CurrentColor(float rgba[4]);

// Matrices
void MatrixMode(Matrix mode);
void PushMatrix();
void PopMatrix();
void LoadIdentity();
void Translate(float x, float y, float z);
void Rotate(float degrees, float x, float y, float z);
void Ortho2D(double left, double right, double bottom, double top);
void Perspective(double fovy, double aspect, double zNear, double zFar);
// Matriz actual de la pila indicada, por columnas (16 floats).
void GetMatrix(Matrix which, float out[16]);

// Marco
void Viewport(int x, int y, int width, int height);
void ClearColor(float r, float g, float b, float a);
void Clear(unsigned clearMask);
void Flush();
// Descarta los errores pendientes del dispositivo (p. ej. tras pops de más).
void ClearErrors();
// Lee el framebuffer en RGB de 8 bits, filas de abajo hacia arriba.
void ReadPixelsRGB(int x, int y, int width, int height, void* out);
int MaxTextureSize();
void MaxViewportDims(int out[2]);
const char* GetDeviceInfo(DeviceInfo info);

// Texturas (handles propios; hoy coinciden con los nombres del backend)
using Texture = unsigned;
Texture CreateTexture();
void DeleteTexture(Texture texture);
void BindTexture(Texture texture);
// components: cantidad de canales almacenados (3 o 4), como el original.
void TexImage2D(int components, int width, int height, Format format, const void* pixels);
// Actualiza un rectángulo de la textura ligada (filas contiguas, sin relleno).
void TexSubImage2D(int x, int y, int width, int height, Format format, const void* pixels);
void TexParameter(TexParam parameter, TexValue value);
// Los cargadores del 0.97k reciben filtro y wrap con los valores numéricos
// del binario (p. ej. OpenJPG(..., 0x2601, 0x2901)); esto los traduce.
TexValue TexValueFromLegacy(int value);
void TexBorderColor(const float rgba[4]);
void TexEnvMode(TexEnv mode);
void SetUnpackAlignment(int alignment);
int UnpackAlignment();

// Niebla
void SetFogMode(FogMode mode);
void FogStart(float value);
void FogEnd(float value);
void FogDensity(float value);
void FogColor(const float rgba[4]);

// Listas de glifos del texto heredado (wglUseFontBitmaps / glCallLists)
unsigned GenLists(int count);
void DeleteLists(unsigned base, int count);
void ListBase(unsigned base);
void RasterPos2f(float x, float y);
// Dibuja una lista por carácter (bytes), desde la base de ListBase.
void CallLists(int count, const unsigned char* characters);

}
