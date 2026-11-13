#pragma once
// Terrain.h — Load (.att + TerrainHeight), normals, lighting, water anim, bilinear interp.
void __cdecl Terrain_Load(int map_id, int flags);
float __cdecl Terrain_GetHeight(float xf, float yf);
void  __cdecl Terrain_GetLight(float xf, float yf, float* out_light);
