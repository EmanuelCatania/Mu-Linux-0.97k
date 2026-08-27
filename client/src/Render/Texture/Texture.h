#pragma once
// Texture.h — OZJ/OZT load pipeline, GL_BindTextureSlot, Texture_Unload, Texture_Draw2D.
int   __cdecl Texture_Load(const char* path, int id, int w, int h, int flags, char mipmap);
void  __cdecl Texture_Unload(int id);
void  __cdecl Texture_BindGL(int id);
void  __cdecl Texture_Draw2D(int id, float x, float y, float w, float h,
                              float u0, float v0, float u1, float v1,
                              char fx, char fy);
