#pragma once
#include <cstdint>

// Capa de render: el estado y los recursos que usa el juego, sin tipos ni
// constantes de OpenGL. El estado del pipeline fijo (alpha test, niebla,
// texture env, matrices, color actual) vive en Render/RenderState y lo
// consumen los shaders; el backend sólo envía al dispositivo el estado que
// existe en OpenGL 3.3 core. Antes de cada cambio se envía el lote pendiente,
// así el orden de dibujo no cambia. La geometría va por BufferedDraw; esto
// cubre estado, matrices, texturas, niebla y consultas.
namespace Gfx {

enum class Cap { Texture2D, Blend, AlphaTest, DepthTest, CullFace, Fog, ScissorTest, StencilTest, Lighting, Dither, PolygonSmooth, ColorLogicOp };
enum class Factor { Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha, DstColor, OneMinusDstColor, SrcAlphaSaturate };
enum class Compare { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
enum class Matrix { ModelView, Projection, Texture };
enum class Format { RGB, RGBA };
enum class TexParam { MinFilter, MagFilter, WrapS, WrapT };
enum class TexValue { Nearest, Linear, NearestMipmapNearest, LinearMipmapNearest, NearestMipmapLinear, LinearMipmapLinear, Repeat, Clamp, ClampToEdge };
enum class TexEnv { Modulate, Replace, Decal, Blend, Add };
enum class FogMode { Linear, Exp, Exp2 };
enum class DeviceInfo { Vendor, Renderer, Version };
// Máscaras combinables con |.
enum ClearMask : unsigned { ClearColorBuffer = 1, ClearDepthBuffer = 2, ClearStencilBuffer = 4 };
enum AttribMask : unsigned { AttribCurrent = 1, AttribTexture = 2, AttribEnable = 4,
    AttribColorBuffer = 16, AttribViewport = 32, AttribFog = 64, AttribDepth = 128 };

// Al crear un contexto: deja RenderState y el dispositivo con los valores
// iniciales de OpenGL.
void ResetState();

// Estado
void Enable(Cap cap);
void Disable(Cap cap);
inline void SetEnabled(Cap cap, bool on) { on ? Enable(cap) : Disable(cap); }
bool IsEnabled(Cap cap);
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
// DESVIACION: posproceso opcional sobre el viewport actual, con lo dibujado
// hasta ahora: antialiasing FXAA y después nitidez adaptativa (tipo AMD CAS).
// sharpen: 0 apagada, 1 = máximo de AMD CAS.
void PostProcess(bool antialias, float sharpen);
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

// DESVIACION: calidad de texturas, opción del jugador. Los mipmaps se generan
// al subir cada textura (se fija al arrancar); el bicúbico cambia en caliente.
void SetTextureQuality(bool mipmaps, bool bicubic);
bool TextureMipmaps();
bool TextureBicubic();
// DESVIACION: muestras de MSAA del framebuffer (0 sin MSAA); se fija al crear la ventana.
void SetMultisample(int samples);
// DESVIACION: texturas del pack HD (Data/HD); se fija al arrancar.
void SetHDTextures(bool enabled);
bool HDTextures();
int Multisample();

// Niebla
void SetFogMode(FogMode mode);
void FogStart(float value);
void FogEnd(float value);
void FogDensity(float value);
void FogColor(const float rgba[4]);

}
