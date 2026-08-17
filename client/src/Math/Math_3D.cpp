// Math_3D.cpp
// FUN_004fa0b0 @ 0x004fa0b0  — Vec3_TransformByMatrix
// FUN_004f9db0 @ 0x004f9db0  — EulerToMatrix3x4
//
// Vec3_TransformByMatrix:
//   Transforms a 3-component vector (param_1) by a 3x4 column-major
//   rotation/scale matrix (param_2) and stores the result in param_3.
//   param_2 layout (float[]): [0,1,2] col0, [4,5,6] col1, [8,9,10] col2
//
// EulerToMatrix3x4:
//   Builds a 3x4 rotation matrix from Euler angles (param_1[0]=X,
//   param_1[1]=Y, param_1[2]=Z, in degrees).
//   Multiplies each angle by _DAT_00552ce8 (degrees-to-radians: π/180).
//   Translation column (indices 3,7,11) is zeroed.
//
// Globals:
//   _DAT_00552ce8 — degrees-to-radians constant (π/180)

#include "stdafx.h"
extern "C" { void DbgLogPublic(const char* msg); }


float* __cdecl FUN_004fa0b0(float *param_1,float *param_2,float *param_3)

{
  *param_3 = *param_1 * *param_2 + param_2[2] * param_1[2] + param_2[1] * param_1[1];
  param_3[1] = param_2[6] * param_1[2] + param_2[5] * param_1[1] + param_2[4] * *param_1;
  param_3[2] = param_2[10] * param_1[2] + param_2[9] * param_1[1] + param_2[8] * *param_1;
  return param_3;
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_004f9db0(float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float10 fVar6;
  float10 fVar7;
  float10 fVar8;

  fVar6 = (float10)fsin((float10)param_1[2] * (float10)_DAT_00552ce8);
  fVar1 = (float)fVar6;
  fVar7 = (float10)fcos((float10)param_1[2] * (float10)_DAT_00552ce8);
  fVar6 = (float10)fsin((float10)param_1[1] * (float10)_DAT_00552ce8);
  fVar2 = (float)fVar6;
  fVar6 = (float10)fcos((float10)param_1[1] * (float10)_DAT_00552ce8);
  fVar3 = (float)fVar6;
  fVar4 = *param_1;
  fVar6 = (float10)_DAT_00552ce8;
  fVar8 = (float10)fsin((float10)fVar4 * fVar6);
  param_2[3] = 0.0;
  param_2[7] = 0.0;
  param_2[0xb] = 0.0;
  fVar5 = (float)fVar8;
  fVar6 = (float10)fcos((float10)fVar4 * fVar6);
  fVar4 = (float)fVar6;
  *param_2 = (float)((float10)fVar3 * fVar7);
  param_2[4] = fVar3 * fVar1;
  param_2[8] = -fVar2;
  param_2[1] = (float)((float10)fVar5 * (float10)fVar2 * fVar7 - (float10)fVar4 * (float10)fVar1);
  param_2[5] = (float)((float10)fVar4 * fVar7 + (float10)fVar5 * (float10)fVar2 * (float10)fVar1);
  param_2[9] = fVar5 * fVar3;
  param_2[2] = (float)((float10)fVar5 * (float10)fVar1 + (float10)fVar4 * (float10)fVar2 * fVar7);
  param_2[6] = (float)((float10)fVar4 * (float10)fVar2 * (float10)fVar1 - (float10)fVar5 * fVar7);
  param_2[10] = fVar4 * fVar3;
  return;
}


// FUN_004fa110 @ 0x004fa110
//
// Vec3_TransformByMatrix_RowMajor — transforms a 3-component vector by a
// row-major 3x4 matrix (contrast with FUN_004fa0b0 which uses column-major).
// param_2 layout: [0,1,2] row0, [4,5,6] row1, [8,9,10] row2
//   out[0] = dot(in, col0) = m[0]*x + m[4]*y + m[8]*z
//   out[1] = dot(in, col1) = m[1]*x + m[5]*y + m[9]*z
//   out[2] = dot(in, col2) = m[2]*x + m[6]*y + m[10]*z

void __cdecl FUN_004fa110(float *param_1,float *param_2,float *param_3)

{
  *param_3 = *param_1 * *param_2 + param_2[4] * param_1[1] + param_2[8] * param_1[2];
  param_3[1] = param_2[5] * param_1[1] + param_2[9] * param_1[2] + param_2[1] * *param_1;
  param_3[2] = param_2[6] * param_1[1] + param_2[10] * param_1[2] + param_2[2] * *param_1;
  return;
}


// FUN_004fa4d0 @ 0x004fa4d0
//
// Vec3_TriangleNormal — computes the normalized surface normal of a triangle
// defined by three points (param_1, param_2, param_3), storing the result in
// param_4.  Uses the cross product of the two edge vectors:
//   edge1 = param_2 - param_1
//   edge2 = param_3 - param_1
//   normal = normalize(cross(edge2, edge1))
// No-op if the cross product length is zero (degenerate triangle).
//
// Globals:
//   _DAT_005528a0 — float constant 0.0 (degenerate-length guard)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_004fa4d0(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;

  fVar1 = (param_3[2] - param_1[2]) * (param_2[1] - param_1[1]) -
          (param_3[1] - param_1[1]) * (param_2[2] - param_1[2]);
  fVar2 = (*param_3 - *param_1) * (param_2[2] - param_1[2]) -
          (*param_2 - *param_1) * (param_3[2] - param_1[2]);
  fVar3 = (*param_2 - *param_1) * (param_3[1] - param_1[1]) -
          (*param_3 - *param_1) * (param_2[1] - param_1[1]);
  fVar4 = SQRT(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
  if (fVar4 != (float)_DAT_005528a0) {
    *param_4 = fVar1 / fVar4;
    param_4[1] = fVar2 / fVar4;
    param_4[2] = fVar3 / fVar4;
    return;
  }
  return;
}


// FUN_004fa170 @ 0x004fa170 — Vec3_TransformByMatrix_WithTranslate
// Transforms a 3-component vector (param_1) by a 3x4 matrix (param_2)
// where the translation column is included (indices 3, 7, 11).
// Contrast with FUN_004fa0b0 which uses pure rotation (no translation term).
// Used by Sprite_DrawTexturedQuad (FUN_00511d00) to project world→screen.
void __cdecl FUN_004fa170(float *param_1,float *param_2,float *param_3)
{
  *param_3 = *param_1 * *param_2 + param_2[2] * param_1[2] + param_2[1] * param_1[1] + param_2[3];
  param_3[1] = param_2[6] * param_1[2] + param_2[5] * param_1[1] + param_2[4] * *param_1 +
               param_2[7];
  param_3[2] = param_2[10] * param_1[2] + param_2[9] * param_1[1] + param_2[8] * *param_1 +
               param_2[0xb];
  return;
}


// FUN_004409a0 @ 0x004409a0 — Matrix_TransformPoint (thiscall)
// Transforms param_2 by matrix param_1 (via FUN_004fa170).
// If param_4 != 0: applies scale (this->+0x68) + offset (this->+0x6c/70/74).
// Otherwise: pure transform, result in param_3.
//
// BUG-FIX 2026-04-27: el decompile original usaba `float local_c; float local_8;
// float local_4;` como 3 vars separadas y pasaba `&local_c` a FUN_004fa170 que
// escribe 3 floats contiguos. MSVC no garantiza contigüidad → TPos[1]/[2] iban
// a stack slots non-relacionados → sprite spawn positions basura → glow +9,
// wing FX, particles invisibles porque proyectaban fuera del frustum. Mismo
// patrón ya corregido en Names/RenderLinkObject/Sprite_DrawTexturedQuad.
void __cdecl FUN_004409a0(void *this_,float *param_1,float *param_2,float *param_3,char param_4)
{
  if (param_4 != '\0') {
    float local_buf[3];   // contiguo
    FUN_004fa170(param_2,param_1, local_buf);
    float scale = *(float *)((int)this_ + 0x68);
    // ── DIAG: log first call per second per slot to see this_ + 0x6c..74
    {
      static DWORD s_lastTP = 0;
      DWORD now = GetTickCount();
      if (now - s_lastTP > 1000) {
        s_lastTP = now;
        char b[200];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
          "TP this=%p scale=%.3f m6c=(%.1f,%.1f,%.1f) in=(%.1f,%.1f,%.1f) "
          "matT=(%.2f,%.2f,%.2f) matResult=(%.2f,%.2f,%.2f)",
          this_, scale,
          *(float*)((int)this_+0x6c), *(float*)((int)this_+0x70), *(float*)((int)this_+0x74),
          param_2[0], param_2[1], param_2[2],
          param_1[3], param_1[7], param_1[11],
          local_buf[0], local_buf[1], local_buf[2]);
        DbgLogPublic(b);
      }
    }
    param_3[0] = scale * local_buf[0] + *(float *)((int)this_ + 0x6c);
    param_3[1] = scale * local_buf[1] + *(float *)((int)this_ + 0x70);
    param_3[2] = scale * local_buf[2] + *(float *)((int)this_ + 0x74);
    return;
  }
  FUN_004fa170(param_2,param_1,param_3);
  return;
}
