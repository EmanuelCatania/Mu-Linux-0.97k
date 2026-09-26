#pragma once
// Terrain_Render (0x004FD800) -- el walker de objetos del mundo por chunk.
//
// 2026-09-26: aca habia una declaracion `void Terrain_Render(int, int, int)` sin
// definicion y sin un solo call site, mientras la funcion real vivia como
// Terrain_Render(void).  Dos firmas para la misma direccion son dos simbolos.
void Terrain_Render(void);
