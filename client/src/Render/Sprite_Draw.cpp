// Sprite_Draw.cpp
// Sprite_Draw @ 0x00440060  (159 lines, decompile completo)
//
// Renderiza un sprite 3D animado en el mundo: interpola animación entre dos frames/bones,
// transforma los vértices con la matriz de vista, y llama al draw final.
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   void __thiscall FUN_00440060(
//     void       *this,      // model data ptr (DAT_05828d58 + class*0xbc)
//     int         param_1,   // flags: 0x6970a9c = root bone offset constant
//     float       param_2,   // rotation angle (float, animation frame)
//     undefined4  param_3,   // pos Y packed
//     byte        param_4,   // animation index A (slot A)
//     undefined4 *param_5,   // position pointer XYZ (world pos from entity)
//     float      *param_6,   // extra params (terrain slope offsets)
//     char        param_7,   // flag A
//     char        param_8    // flag B
//   )
//
// ── ANIMATION CLAMP ──────────────────────────────────────────────────────────
//
//   sVar6 = this[+0x26]   (total animation frame count for this model)
//   if param_4 >= sVar6: param_4 = 0
//   if this[+0xa0] >= sVar6: this[+0xa0] = 0
//
//   Copia param_5 → this[+0x78/7c/80]  (world position XYZ)
//   this[+0xa4] = param_2               (rotation angle float)
//
// ── FRAME INTERPOLATION ───────────────────────────────────────────────────────
//
//   sVar10 = (short)ftol(param_2)       (frame entero)
//   fVar8 = param_2 - sVar10            (fracción de frame: blending factor)
//   fVar9 = 1.0 - fVar8                 (complemento)
//   local_78 = ftol(fVar9)              (frame secundario)
//
//   Frame validation:
//     if local_78 >= frame_count: local_78 = 0
//     if sVar10 >= anim_frame_count: sVar10 = 0
//
// ── BONE LOOP ─────────────────────────────────────────────────────────────────
//
//   Loop para cada hueso (0..this[+0x22]-1):
//     iVar1 = this[+0x2c] + bone_idx * 0x8c    (bone descriptor)
//
//     if bone[+0x22] == '\0' (bone activo):
//       piVar2 = bone[+0x24] + param_4 * 0xc   (anim data para slot A)
//       piVar3 = bone[+0x24] + this[+0xa0] * 0xc (anim data para slot B)
//
//       if bone_idx == this[+0x54] (root/special bone):
//         // Frame-to-frame interpolation (quaternion slerp via FUN_004fa350)
//         iVar7 = piVar2[1] + frame * 0xc       (frame A keyframe offset)
//         iVar4 = piVar3[1] + sVar10 * 0xc      (frame B keyframe offset)
//         local_68 = *(float*)(iVar7)    - *param_6 * _DAT_005528b0
//         local_5c = *(float*)(iVar4)    - *param_6 * _DAT_005528b0
//         local_60 = *(float*)(iVar7+8)  - param_6[1] * _DAT_005528b0
//         local_54 = *(float*)(iVar4+8)  - param_6[1] * _DAT_005528b0
//         FUN_004fa1d0(&local_68, &local_50)   → Quat_Normalize(q_A, out)
//         FUN_004fa1d0(&local_5c, &local_40)   → Quat_Normalize(q_B, out)
//         iVar11 = FUN_004f9c70(&local_50, &local_40)  → Quat_Dot(A, B)
//         if iVar11 == 0: FUN_004fa350(&local_50, &local_40, fVar8, pfVar13) → Quat_Slerp
//         else: pfVar13 = &DAT_05826e18 + bone_idx*0x10; *pfVar13 = local_50 (copy)
//       else:
//         // Sample directly from keyframe arrays (no slerp for non-root bones)
//         local_50 = piVar2[2][frame*0x10]     (quat A)
//         local_40 = piVar3[2][sVar10*0x10]    (quat B)
//
//       // Blend result stored in DAT_05826e18 (bone result table, stride 0x10)
//       FUN_004fa270(pfVar13, local_30)     → Quat_ToMatrix(quat, mat3x3)
//
//       pfVar13 = piVar2[0] + local_78 * 0xc   (translation keyframe A)
//       ... (lerp translation)
//
// ── DRAW FINAL ────────────────────────────────────────────────────────────────
//
//   FUN_004fa930(entity, this) → Sprite_SetupTransform (aplica bone results a model)
//   FUN_004404e0(this, bone_ptr, anim_A, anim_B, anim_C, blend) → Sprite_DrawPass
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_05826e18  — bone result buffer: blended quaternions (stride 0x10, max 32 bones)
//   DAT_005528b0  — animation blend scale constant
//   this[+0x22]   — bone count for this model
//   this[+0x26]   — total frame count
//   this[+0x2c]   — bone descriptor array ptr
//   this[+0x30]   — animation data ptr
//   this[+0x54]   — root/special bone index (uses slerp)
//   this[+0xa0]   — current animation slot B index
//   this[+0xa4]   — current animation angle/frame float
//   this[+0xa8]   — current frame short
//   this[+0x78/7c/80] — world position XYZ
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_004f9c70  → Quat_Dot(A, B) — retorna 0 si quats paralelos (skip slerp)
//   FUN_004fa350  → Quat_Slerp(A, B, t, out)
//   FUN_004fa1d0  → Quat_Normalize(quat, out)
//   FUN_004fa270  → Quat_ToMatrix(quat, out_mat3x3)
//   FUN_004fa930  → Sprite_SetupTransform(entity, model)
//   FUN_004404e0  → Sprite_DrawPass(model, bone, anim_A, anim_B, anim_C, blend)

#include "stdafx.h"
#include "Render/Sprite_Draw.h"
