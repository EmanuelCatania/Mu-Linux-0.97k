#pragma once
// Texture.h — OZJ/OZT load pipeline, GL_BindTextureSlot, Texture_Unload, Texture_Draw2D.
int   __cdecl Texture_Load(const char* path, int id, int w, int h, int flags, char mipmap);
void  __cdecl Texture_Unload(int id);
// Sube a la GPU las texturas HD que terminó de decodificar el hilo de carga.
void  Texture_PumpHD();
// DESVIACION (pack HD): con la opción de texturas HD, "Data\X\archivo" se toma de
// "Data\HD\X\archivo" si existe (modelos .bmd y texturas .ozj/.ozt con el mismo
// nombre). Devuelve la ruta a abrir: `out` o la original.
#include <cstddef>
const char* Texture_HDAssetPath(const char* path, char* out, std::size_t size);
void  __cdecl Texture_BindGL(int id);
void  __cdecl Texture_Draw2D(int id, float x, float y, float w, float h,
                              float u0, float v0, float u1, float v1,
                              char fx, char fy);
