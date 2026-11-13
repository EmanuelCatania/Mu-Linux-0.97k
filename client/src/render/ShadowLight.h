#pragma once
// DESVIACION: sombras proyectadas (opción). Parámetros y matriz de la luz,
// comunes a los backends. Matrices column-major, como OpenGL.
namespace ShadowLight {
constexpr unsigned Size = 2048;          // texels del mapa de sombras
constexpr float Radius = 1800.0f;        // medio lado del área cubierta (unidades del mundo)
constexpr float Depth = 4000.0f;         // profundidad a cada lado del centro
constexpr float Bias = 0.0015f;          // sesgo de la comparación (en profundidad 0..1)
// Vista + proyección ortográfica de la luz centrada en focus, alineada a
// texels. x e y salen de -1 a 1 y z de 0 a 1 (convención de WebGPU).
void ViewProj(const float focus[3], float out[16]);
bool Invert(const float m[16], float out[16]);
void Multiply(const float a[16], const float b[16], float out[16]);   // out = a * b
}
