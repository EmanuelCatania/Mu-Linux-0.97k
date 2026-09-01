// BMD_SetupRender.cpp
// FUN_00504130 @ 0x00504130 — BMD_SetupRenderByType
// Sets up bone color and animation state for BMD models based on effect/entity type.
// Called from entity rendering pipeline to apply type-specific color/anim overrides.
//
// param_1: model pointer (BMD model struct)
// param_2: entity/data pointer (entity struct base)
// param_3: model slot / type id
// param_4: alpha
// param_5: flags / blend mode
//
// Uses:
//   FUN_00440d50 — BMD_SetAnimFrame (model, frame_type, action, scale, phase, x, y, z, color)
//   FUN_00441e00 — BMD_SetColorAnim (model, flags, f1..f6, color_mask)
//   FUN_00440d30 — glPushMatrix wrapper

#include "stdafx.h"


void __cdecl FUN_00504130(void *param_1, int param_2, int param_3, float param_4, uint param_5)
{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  float10 fVar4;
  uint uVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;

  if ((((param_3 == 0x29f) || (param_3 == 0x2df)) || (param_3 == 0x2bf)) || (param_3 == 0x2ff)) {
    // White knight / dark knight variant — full brightness
    *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;   // R=1.0
    *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;   // G=1.0
    *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;   // B=1.0
    fVar9  = *(float *)(param_2 + 0x58);
    fVar11 = *(float *)(param_2 + 0x70);
    fVar10 = *(float *)(param_2 + 0x6c);
    fVar8  = *(float *)(param_2 + 0x68);
    fVar7  = *(float *)(param_2 + 100);
    fVar6  = *(float *)(param_2 + 0x168);
    uVar5  = 6;
  }
  else {
    if (((param_3 == 0x2a4) || (param_3 == 0x2e4)) || ((param_3 == 0x2c4 || (param_3 == 0x304)))) {
      // Variant with custom color anim
      *(undefined4 *)((int)param_1 + 0x48) = 0x3f59999a;   // R≈0.85
      *(undefined4 *)((int)param_1 + 0x4c) = 0x3f59999a;   // G≈0.85
      *(undefined4 *)((int)param_1 + 0x50) = 0x3f99999a;   // B≈1.2
      FUN_00441e00(param_1, 6, *(float *)(param_2 + 0x168), *(int *)(param_2 + 100),
                   *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                   *(float *)(param_2 + 0x70), *(float *)(param_2 + 0x58), 0x493);
      goto LAB_00504925;
    }
    if (param_3 != 0x316) {
      if (param_3 == 0x1a3) {
        *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
        FUN_00440d50(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        FUN_00440d50(param_1, 0.0f, 0x12, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     DAT_05826e08 * _DAT_00552868, 0x492);
        FUN_00440d50(param_1, 1.4013e-45f, 2, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x23a) {
        *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
        FUN_00440d50(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        FUN_00440d50(param_1, 0.0f, 0x12, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     DAT_05826e08 * _DAT_00552920, 0x492);
        FUN_00440d50(param_1, 1.4013e-45f, 2, param_4, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x222) {
        *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
        FUN_00440d50(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        FUN_00440d50(param_1, 0.0f, 0x12, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     DAT_05826e08 * _DAT_00552920, 0x492);
        FUN_00440d50(param_1, 1.4013e-45f, 2, param_4, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x1af) {
        *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
        FUN_00440d30();
        // BUG-FIX: glColor3f espera float; *(undefined4*) lee bits y los pasa como int → C castea int→float = basura.
        glColor3f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),
                  *(float *)((int)param_1 + 0x50));
        FUN_00440d50(param_1, 4.2039e-45f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        FUN_00440d50(param_1, 1.4013e-45f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        fVar2 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524f8);
        FUN_00440d50(param_1, 1.4013e-45f, 2, (float)fVar2, 1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        FUN_00440d50(param_1, 0.0f, 6, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     DAT_05826e08 * _DAT_00552500, 0x492);
        fVar2 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552500);
        fVar2 = (fVar2 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
        *(float *)((int)param_1 + 0x48) = (float)fVar2;
        *(float *)((int)param_1 + 0x4c) = (float)fVar2;
        *(float *)((int)param_1 + 0x50) = (float)fVar2;
        fVar11 = DAT_05826e08 * _DAT_0055291c;
        fVar10 = DAT_05826e08 * _DAT_00552868;
        fVar9 = *(float *)(param_2 + 0x68);
      }
      else {
        if (param_3 == 0x1fa) {
          *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
          *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
          *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
          FUN_00441e00(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), *(float *)(param_2 + 0x58), 0xffffffff);
          FUN_00440d50(param_1, 1.4013e-45f, 2, 1.0f, 1,
                       *(float *)(param_2 + 0x68),
                       DAT_05826e08 * _DAT_00552868, DAT_05826e08 * _DAT_00552a40, 0xffffffff);
          return;
        }
        if (param_3 == 0x1d7) {
          *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
          *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
          *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
          FUN_00441e00(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), *(float *)(param_2 + 0x58), 0xffffffff);
          fVar3 = (float10)DAT_05826e08;
          fVar2 = (float10)_DAT_00552500;
          fVar4 = (float10)fsin(fVar3 * fVar2);
          fVar4 = fVar4 * (float10)_DAT_00552504 + (float10)_DAT_005528b8;
          *(float *)((int)param_1 + 0x48) = (float)fVar4;
          *(float *)((int)param_1 + 0x4c) = (float)fVar4;
          *(float *)((int)param_1 + 0x50) = (float)fVar4;
          FUN_00440d50(param_1, 2.8026e-45f, 2, 1.0f, 2,
                       *(float *)(param_2 + 0x68),
                       (float)(fVar3 * fVar2), DAT_05826e08 * _DAT_0055291c, 0xffffffff);
          return;
        }
        if (param_3 != 0x25f) {
          if (param_3 == 0x260) {
            *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
            *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
            *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
            FUN_00440d30();
            // BUG-FIX: leer como float, no como undefined4 (int). Alpha 0x3f4ccccd=0.8f, 0x3f000000=0.5f.
            glColor4f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),
                      *(float *)((int)param_1 + 0x50), 0.8f);
            FUN_00440d50(param_1, 1.4013e-45f, 2, 0.8f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            glColor4f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),
                      *(float *)((int)param_1 + 0x50), 0.5f);
            FUN_00440d50(param_1, 4.2039e-45f, 2, 0.5f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            glColor3f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),
                      *(float *)((int)param_1 + 0x50));
            FUN_00440d50(param_1, 0.0f, 2, 1.0f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            FUN_00440d50(param_1, 2.8026e-45f, 2, 1.0f, 2,
                         *(float *)(param_2 + 0x68),
                         DAT_05826e08 * _DAT_00552a40, *(float *)(param_2 + 0x70), 0xffffffff);
            uVar5 = 0xffffffff;
            iVar1 = _rand();
            fVar9 = (float)(iVar1 % 10) * _DAT_005524f4;
            iVar1 = _rand();
            FUN_00440d50(param_1, 4.2039e-45f, 2, 1.0f, 3,
                         *(float *)(param_2 + 0x68),
                         (float)(iVar1 % 10) * _DAT_005524f4, fVar9, uVar5);
            glPopMatrix();
            return;
          }
          FUN_00441e00(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), -1.0f, 0xffffffff);
          return;
        }
        // param_3 == 0x25f
        FUN_00440d30();
        *(undefined4 *)((int)param_1 + 0x48) = 0x3e99999a;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3e99999a;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3e99999a;
        // BUG-FIX: 0x3e99999a = bits de 0.3f (≈ gris oscuro)
        glColor3f(0.3f, 0.3f, 0.3f);
        FUN_00440d50(param_1, 2.8026e-45f, 1, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
        // BUG-FIX: 0x3f800000 = bits de 1.0f (blanco)
        glColor3f(1.0f, 1.0f, 1.0f);
        FUN_00440d50(param_1, 2.8026e-45f, 0x44, 1.0f, 2,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     DAT_05826e08 * _DAT_005524f8, 0x492);
        FUN_00440d50(param_1, 0.0f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        uVar5 = 0xffffffff;
        iVar1 = _rand();
        fVar9 = (float)(iVar1 % 10) * _DAT_005524f4;
        iVar1 = _rand();
        FUN_00440d50(param_1, 1.4013e-45f, 2, 1.0f, 1,
                     *(float *)(param_2 + 0x68),
                     (float)(iVar1 % 10) * _DAT_005524f4, fVar9, uVar5);
        fVar2 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552500);
        fVar2 = fVar2 * (float10)_DAT_005528b4 + (float10)_DAT_00552534;
        fVar9 = (float)fVar2;
        *(float *)((int)param_1 + 0x48) = (float)fVar2;
        *(float *)((int)param_1 + 0x4c) = (float)fVar2;
        *(float *)((int)param_1 + 0x50) = (float)fVar2;
        glColor3f(fVar9, fVar9, fVar9);
        fVar11 = DAT_05826e08 * _DAT_00552a40;
        fVar9  = *(float *)(param_2 + 0x68);
        fVar10 = DAT_05826e08 * _DAT_00552868;
      }
      FUN_00440d50(param_1, 2.8026e-45f, 0x42, 1.0f, 2, fVar9, fVar10, fVar11, 0xffffffff);
      glPopMatrix();
      return;
    }
    // param_3 == 0x316
    *(undefined4 *)((int)param_1 + 0x48) = 0x3f4ccccd;   // R≈0.8
    *(undefined4 *)((int)param_1 + 0x4c) = 0x3f19999a;   // G≈0.6
    *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;   // B=1.0
    fVar9  = *(float *)(param_2 + 0x58);
    fVar11 = *(float *)(param_2 + 0x70);
    fVar10 = *(float *)(param_2 + 0x6c);
    fVar7  = *(float *)(param_2 + 100);
    fVar6  = *(float *)(param_2 + 0x168);
    fVar8  = 0.5f;
    uVar5  = 0x44;
  }
  FUN_00441e00(param_1, uVar5, fVar6, fVar7, fVar8, fVar10, fVar11, fVar9, 0x493);
LAB_00504925:
  *(undefined4 *)((int)param_1 + 0x48) = 0x3f800000;
  *(undefined4 *)((int)param_1 + 0x4c) = 0x3f800000;
  *(undefined4 *)((int)param_1 + 0x50) = 0x3f800000;
  FUN_00441e00(param_1, 2, *(float *)(param_2 + 0x168), *(int *)(param_2 + 100),
               *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
               *(float *)(param_2 + 0x70), *(float *)(param_2 + 0x58), 0xffffffff);
  return;
}

// FUN_00440a30 @ 0x00440a30 — BoneTransformOffset (sub_440A30 en IDA)
// Transforms pos_in through bone rotation, scales by model scale (this[+0x68]),
// stores result in pos_out, AND COPIES the bone matrix into the global root
// matrix DAT_06989c9c so that BMD_Animation (FUN_00440060), cuando procesa el
// root bone del ala/arma con parentIdx=-1 y param_7=='\x01', use esta matriz
// del bone padre del player como su "parent transform" → el modelo linked
// queda renderizado en la posición del hueso del player en lugar del origen.
//
// IDA original (sub_440A30 @ 0x00440A30):
//   void __thiscall sub_440A30(float *this, float in2[3][4], float in1[3], float *a4) {
//       float out[3];
//       VectorRotate(in1, in2, out);                  // pure rotation
//       *a4   = this[26] * out[0];                    // scale by this[+0x68] (=this[26])
//       a4[1] = this[26] * out[1];
//       a4[2] = this[26] * out[2];
//       memcpy(matrix, in2, 0x30);                    // copy 12 floats
//   }
//
// BUGFIX 2026-04-26: el Ghidra port había stripped tanto el escalado como
// (crítico) la copia de matriz. Sin la copia, las alas/armas renderizaban
// en (entity_pos + rotated_offset_de_15) ≈ pies del char en lugar del back.
void __cdecl FUN_00440a30(void *model, float *bone_mat, float *pos_in, float *pos_out)
{
  // 1. Rotate-only transform: pos_out = bone_rotation_3x3 * pos_in (no translation)
  Vector_Rotate(pos_in, bone_mat, pos_out);
  // 2. Scale by model scale (model[+0x68])
  if (model) {
    float sc = *(float*)((char*)model + 0x68);
    pos_out[0] *= sc;
    pos_out[1] *= sc;
    pos_out[2] *= sc;
  }
  // 3. Copy 12 floats (3x4 matrix) from bone_mat into global root matrix
  //    DAT_06989c9c — used by BMD_Animation root bone path.
  float* dst = (float*)&DAT_06989c9c;
  for (int i = 0; i < 12; ++i) dst[i] = bone_mat[i];
}

// FUN_004553c0 @ 0x004553c0 — Model_BoneParticle
// Emits a particle at the world position of a specific bone.
// model: BMD model ptr, type: entity type, bone_idx: bone index,
// scale: particle scale, color: float[3] RGB, entity: entity index
void __cdecl FUN_004553c0(void *model, int type, int bone_idx, float scale, float *color, int entity)
{
  float world_pos[3];
  float world_col[3];
  BMD_TransformPosition(model, (float *)((int)model + bone_idx * 0x30), world_pos, world_col, 1);
  FUN_004795c0((unsigned short)type, world_pos, scale, color, entity, 0.0f, 0);
}
