// Entity_PrepareRender.cpp
// Entity_PrepareRender @ 0x004FC030  (13 lines, 3 basic blocks)
//
// Thin wrapper: comprueba si la entidad es renderable y si es así llama al setup GL.
// Llamada por Terrain_Render para cada entidad en la lista enlazada por tile.
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void __cdecl FUN_004fc030(undefined1 *entity, undefined4 slot, int flag, char mode)
//   {
//     undefined4 uVar1 = Calc_RenderObject((int)entity, (char)slot, flag);
//     if ((char)uVar1 != '\0') {
//       Draw_RenderObject(entity, slot, flag, mode);
//     }
//   }
//
// ── LÓGICA ────────────────────────────────────────────────────────────────────
//
//   1. Calc_RenderObject(entity, slot, flag) → Entity_IsRenderable(entity, slot, flag)
//        Devuelve char: '\0' = no renderizar, != '\0' = sí renderizar
//        Probablemente verifica: flag de visibilidad, distancia, estado activo, LOD
//
//   2. Si es renderable:
//        Draw_RenderObject(entity, slot, flag, mode) → Entity_SetupGL(entity, slot, flag, mode)
//        Configura el estado OpenGL para esta entidad antes de Entity_Render_3D
//
// ── PATRONES DE LLAMADA ───────────────────────────────────────────────────────
//
//   Terrain_Render llama siempre como:
//     FUN_004fc030(entity, 0, 0, '\0')   → Entity_PrepareRender(entity, 0, 0, normal)
//   Inmediatamente seguido de:
//     Entity_SpawnEffects(entity)               → Entity_Render_3D(entity)
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   Calc_RenderObject  → Entity_IsRenderable(entity, slot, flag)  — cull / LOD check
//   Draw_RenderObject  → Entity_SetupGL(entity, slot, flag, mode) — glPushMatrix, texture bind, etc.

#include "stdafx.h"
#include "Render/Entity_PrepareRender.h"

// Entity_PrepareRender @ 0x004FC030 (13 lines)
// Cull + GL setup: si la entidad es visible, llama a Entity_SetupGL.
void __cdecl Entity_PrepareRender(void *param_1, int param_2, int param_3, char param_4)
{
    int uVar1 = Calc_RenderObject((int)param_1, (char)param_2, param_3);  // Entity_IsRenderable
    if ((char)uVar1 != '\0')
        Draw_RenderObject(param_1, param_2, param_3, param_4);                   // Entity_SetupGL
}
