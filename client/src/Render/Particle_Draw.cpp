// Particle_Draw.cpp
// Particle_Draw @ 0x004F8BB0  (106 lines, decompile completo)
//
// Dibuja una partícula individual como un quad billboarded en el mundo.
// Proyecta la posición mundo a coordenadas de pantalla, rota el quad según la
// matriz de vista, y llama al draw final por cada tile de la cuadrícula.
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   void __cdecl FUN_004f8bb0(
//     int        param_1,  // particle type (0x4a7=spark, 0x4b0=snow, 0x4f0=glow, etc.)
//     float      param_2,  // world X
//     float      param_3,  // world Y
//     float      param_4,  // size width
//     float      param_5,  // size height (max(param_4, param_5) usado como tamaño efectivo)
//     undefined4*param_6,  // color ptr: RGB floats [r,g,b]
//     undefined4 param_7,  // world Z (como undefined4, reinterpretado como float)
//     float      param_8   // alpha [0.0-1.0]; si == 1.0 → glColor3fv, sino → glColor4f
//   )
//
// ── COLOR SETUP ───────────────────────────────────────────────────────────────
//
//   if param_8 == _DAT_0055256c (1.0f):
//     glColor3fv(param_6)             → color sólido sin alpha
//   else:
//     glColor4f(param_6[0], param_6[1], param_6[2], param_8)  → color con alpha
//
// ── ROTACIÓN DE VISTA ─────────────────────────────────────────────────────────
//
//   local_9c = {0.0, 0.0}; local_94 = param_7 (Z como float)
//   FUN_004f9db0(&local_9c, local_30)
//     → Matrix_FromEuler({0,0,Z}, out_mat[12])   (solo rotación en Z = spin angle)
//
//   GL_BindTextureSlot(param_1)
//     → Particle_SetTexture(type)   — bind textura según tipo de partícula
//
// ── PROYECCIÓN MUNDO→PANTALLA ─────────────────────────────────────────────────
//
//   fVar3 = param_2 * _DAT_00552594   (world_X * cam_scale → screen tile X)
//   fVar5 = param_3 * _DAT_00552594   (world_Y * cam_scale → screen tile Y)
//   lVar14 = ftol(fVar3)              (tile X entero)
//   lVar15 = ftol(fVar5)              (tile Y entero)
//
//   Tamaño efectivo: fVar2 = max(param_4, param_5)
//   fVar6 = fVar2 * _DAT_00552504     (half_size en unidades de tile)
//   fVar7 = lVar14 - fVar3 + fVar6    (offset desde esquina del tile)
//   fVar2 = 1.0 / fVar2               (reciprocal para UV calc)
//   lVar16 = ftol(fVar3)              (tile entero para loop)
//   fVar3 = float(lVar16 + 1)
//
// ── LOOP DE TILES ────────────────────────────────────────────────────────────
//
//   for fVar11 = -fVar3 to fVar3 step 1.0:    (loop Y tiles del quad)
//     for local_d0 = -fVar3 to fVar3 step 1.0: (loop X tiles del quad)
//
//       // Calcular UV y posición de este sub-tile del quad
//       local_90[1] = fVar9 = (fVar11 + lVar15 - fVar5 + fVar6) * fVar2  (UV Y top)
//       local_90[0] = (local_d0 + fVar7) * fVar2                          (UV X left)
//       local_90[2] = 0.0
//       local_90[4] = fVar9                                               (UV Y bottom = same)
//       local_90[3] = (fVar10 + local_d0) * fVar2                        (UV X right)
//       local_7c = 0; local_74 = fVar8; local_70 = 0
//       local_68 = fVar8; local_64 = 0
//       local_78 = local_90[3]; local_6c = local_90[0]
//
//       // Transformar 4 esquinas del quad con la matriz de rotación
//       for iVar12 = 0 to 0x30 step 0xc:    (4 esquinas × 3 floats = 12 bytes)
//         local_90[iVar12/4]   -= _DAT_00552504   (centrar UV)
//         local_90[iVar12/4+1] -= _DAT_00552504
//         FUN_004fa0b0(&local_90[iVar12/4], local_30, &local_60[iVar12/4])
//           → Matrix_TransformPoint(corner_uv, rotation_mat, out_corner)
//         local_60[iVar12/4] = (param_4/param_5) * local_60[iVar12/4] + _DAT_00552504
//         local_60[iVar12/4+1] += _DAT_00552504
//
//       // Draw call por sub-tile
//       FUN_004f8740(lVar14 + local_d0, lVar15 + fVar11, 1.0, 1, local_60, '\0', param_8)
//         → Particle_DrawQuad(tile_x, tile_y, scale, flag, corners[4][3], solid, alpha)
//
// ── CONSTANTES ────────────────────────────────────────────────────────────────
//
//   _DAT_00552594  — cam_scale: factor world→tile (world_pos / tile_size → tile index)
//   _DAT_00552504  — 0.5f (half para centrar)
//   _DAT_0055256c  — 1.0f (constante "full alpha")
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_004f9db0  → Matrix_FromEuler(angles[3], out_mat[12])
//   GL_BindTextureSlot  → Particle_SetTexture(type)  — bind GL texture por tipo
//   FUN_004fa0b0  → Matrix_TransformPoint(pt, mat, out)
//   FUN_004f8740  → Particle_DrawQuad(x, y, scale, flag, corners, solid, alpha)
//                   Función que hace los calls glVertex3f / glTexCoord2f reales
//   glColor3fv    → OpenGL color sin alpha
//   glColor4f     → OpenGL color con alpha

#include "stdafx.h"
#include "Render/Particle_Draw.h"
