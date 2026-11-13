#pragma once
// Programas GLSL 3.3 core del backend. Cada productor aporta su vertex shader
// (con el prólogo común) y comparte la etapa de fragmentos que reproduce el
// pipeline fijo del 0.97k: texture env, alpha test y niebla, con los valores
// de RenderState como uniforms.
namespace FragmentPipelineGL {
// Prólogo de los vertex shaders: versión, matrices y salidas hacia la etapa
// de fragmentos (vColor ya limitado a [0,1], vUV y vFogDistance).
extern const char* const VertexPrologue;
// Compila prólogo + cuerpo y la etapa de fragmentos común. 0 si falla.
unsigned Create(const char* vertexBody);
void Release(unsigned program);
// Con el programa ya ligado, sube los uniforms que cambiaron desde el envío
// anterior de ese programa.
void Apply(unsigned program);
// Ubicación de un uniform propio del productor.
int Uniform(unsigned program,const char* name);
}
