// Particle.cpp
#include "stdafx.h"
extern "C" { void DbgLogPublic(const char* msg); }
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]
// FUN_0043e5c0 @ 0x0043e5c0  — Particle_FinalizeAlpha
// FUN_0043e680 @ 0x0043e680  — Particle_PathUpdate
// FUN_0043e820 @ 0x0043e820  — Particle_SetAnimation
//
// Particle_FinalizeAlpha (FUN_0043e5c0):
//   Smoothly blends particle alpha (field +0x168) toward target (field
//   +0x164). Behavior depends on flag at +0x161:
//     0 — lerp toward target using _DAT_005524f4 factor
//     non-zero — step toward target by _DAT_00552874; clamp to [0,1]
//   Clamps display alpha (+0x68) to the computed value.
//
// Particle_PathUpdate (FUN_0043e680):
//   Steers a particle toward the average direction of nearby active
//   particles in the pool (stride 0x1bc = 0x6f*4).  Uses a proximity
//   threshold (_DAT_00552850) and a half-way distance (_DAT_00552878).
//   Writes the resulting steering angle to field +0x24 via
//   FUN_0043e430 / FUN_0043e120.
//
// Particle_SetAnimation (FUN_0043e820):
//   Sets the current animation index (+0x105) for a particle/entity.
//   Validates range against the model's animation count
//   (DAT_05828d58 + 0x26 + type*0xbc).  Special values 0x4c and 0x4d
//   bypass the range check.  Saves the previous animation to +0x106
//   and resets the frame counter (+0x108) when the animation changes.
//
// Globals / constants:
//   _DAT_005524f4 — alpha lerp factor
//   _DAT_00552580 — float 0.0
//   _DAT_0055256c — float 1.0
//   _DAT_00552874 — alpha step delta
//   _DAT_00552850 — proximity threshold (squared)
//   _DAT_00552878 — half-way distance
//   DAT_05828d58  — monster/entity base table

#include "stdafx.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_0043e5c0(int param_1)

{
  float fVar1;

  if (*(char *)(param_1 + 0x161) == '\0') {
    *(float *)(param_1 + 0x168) =
         (*(float *)(param_1 + 0x164) - *(float *)(param_1 + 0x168)) * _DAT_005524f4 +
         *(float *)(param_1 + 0x168);
  }
  else if (*(float *)(param_1 + 0x164) <= *(float *)(param_1 + 0x168)) {
    if ((*(float *)(param_1 + 0x164) < *(float *)(param_1 + 0x168)) &&
       (fVar1 = *(float *)(param_1 + 0x168) - _DAT_00552874, *(float *)(param_1 + 0x168) = fVar1,
       fVar1 < _DAT_00552580)) {
      *(undefined4 *)(param_1 + 0x168) = 0;
    }
  }
  else {
    fVar1 = *(float *)(param_1 + 0x168) + _DAT_00552874;
    *(float *)(param_1 + 0x168) = fVar1;
    if (_DAT_0055256c < fVar1) {
      *(undefined4 *)(param_1 + 0x168) = 0x3f800000;
    }
  }
  if (*(float *)(param_1 + 0x168) < *(float *)(param_1 + 0x68)) {
    *(undefined4 *)(param_1 + 0x68) = *(undefined4 *)(param_1 + 0x168);
  }
  return;
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_0043e680(int param_1,int param_2,int param_3,int param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float *pfVar6;
  int iVar7;
  int iVar8;
  longlong lVar9;

  iVar8 = 0;
  iVar7 = 0;
  if (0 < param_4) {
    pfVar6 = (float *)(param_3 + 0x14);
    fVar3 = _DAT_00552580;
    fVar5 = _DAT_00552580;
    do {
      if (((*(char *)(pfVar6 + -5) != '\0') && (iVar7 != param_2)) &&
         (fVar1 = *(float *)(param_1 + 0x10) - pfVar6[-1],
         fVar2 = *(float *)(param_1 + 0x14) - *pfVar6, fVar1 = SQRT(fVar1 * fVar1 + fVar2 * fVar2),
         fVar1 < _DAT_00552850)) {
        fVar2 = pfVar6[0x2b] - *(float *)(param_1 + 0x10);
        if (_DAT_00552878 <= fVar1) {
          fVar2 = fVar2 + (pfVar6[0x2b] - pfVar6[-1]);
          fVar1 = (pfVar6[0x2c] - *(float *)(param_1 + 0x14)) + (pfVar6[0x2c] - *pfVar6);
        }
        else {
          fVar2 = (pfVar6[0x2b] - pfVar6[-1]) - fVar2;
          fVar1 = (pfVar6[0x2c] - *pfVar6) - (pfVar6[0x2c] - *(float *)(param_1 + 0x14));
        }
        iVar8 = iVar8 + 1;
        fVar4 = SQRT(fVar2 * fVar2 + fVar1 * fVar1);
        fVar3 = fVar2 / fVar4 + fVar3;
        fVar5 = fVar1 / fVar4 + fVar5;
      }
      iVar7 = iVar7 + 1;
      pfVar6 = pfVar6 + 0x6f;
    } while (iVar7 < param_4);
    if (0 < iVar8) {
      fVar1 = *(float *)(param_1 + 0x10);
      fVar2 = *(float *)(param_1 + 0x14);
      lVar9 = (longlong)(*(float *)(param_1 + 0xd8));   // IDA sub_43E680: (__int64)*(float*)(a1+216)
      iVar7 = (int)lVar9;
      iVar8 = FUN_0043e430(fVar1,fVar2,fVar3 / (float)iVar8 + fVar1,fVar5 / (float)iVar8 + fVar2);
      lVar9 = (longlong)(*(float *)(param_1 + 0x24));   // IDA sub_43E680: (__int64)*(float*)(a1+36)
      iVar7 = FUN_0043e120((int)lVar9,iVar8,iVar7);
      *(float *)(param_1 + 0x24) = (float)iVar7;
      return;
    }
  }
  return;
}


// SetAction @ 0x0043E820 — DEFINICION UNICA en stubs_externs.cpp.
// 2026-08-16: acá había una segunda definición con firma (int, uint). Como C++
// las trata como sobrecargas distintas, ambas compilaban y cada caller elegía
// por el tipo de sus argumentos — el mismo patrón que causó el bug del Magic
// Gladiator con SetPlayerStop. Las dos eran equivalentes al binario, así que
// consolidar no cambia comportamiento; sólo elimina la trampa.


// FUN_004795c0 @ 0x004795c0
//
// Effect_Spawn — allocates a free slot in the effect pool and initialises it.
//
// Scans the effect pool starting at DAT_07c85890 (stride 0x1bc = 444 bytes)
// for the first inactive slot (byte 0 == 0).  On finding one, fills fields:
//   +0x02 (short) = param_1 (effect type)
//   +0x04        = param_7
//   +0xfc        = param_5 (flags)
//   +0x0c        = param_3 (sub-type)
//   +0x24        = param_6 (scale)
//   +0x00        = 1 (mark active)
//   +0x108..10b  = 0x3f800000 (1.0f alpha)
//   +0x10..18    = position (param_2)
//   +0x170..178  = prev position (param_2 copy)
//   +0xe8..f0    = direction (param_4)
// Returns slot index (0-based), or 0 if pool is full.
//
// Globals:
//   DAT_07c85890 — effect pool base (stride 0x1bc, ends at 0x7cf1ef0)

int __cdecl
FUN_004795c0(unsigned short param_1, float *param_2, float param_3, float *param_4,
             int param_5, float param_6, int param_7)
{
  // [DIAG FORGE] entry-point CreateSprite (a = scale param_3)
  if (param_2)
      DbgForge("CreateSprite", (int)param_1, -1, (int)param_1, -1, -1, -1,
               param_2[0], param_2[1], param_2[2],
               param_4 ? param_4[0] : 0.0f, param_4 ? param_4[1] : 0.0f,
               param_4 ? param_4[2] : 0.0f, param_3);
  // Pool fix 2026-04-27: el AUTO-SKIP previo (return 0 al inicio) bloqueaba
  // TODOS los efectos (glow +9 set, wing FX, weapon sparkles, lightning).
  // Ahora con DAT_07c85890[1002*0x1bc] correctamente dimensionado, iteramos
  // por índice acotado por 1002 slots en vez de la dirección absoluta original.

  // BUG-FIX 2026-05-01: null pointer guard. El binary original asume que
  // param_2 (pos) y param_4 (dir) siempre son pointers válidos, pero algún
  // caller en el char-select pipeline pasa NULL → AV en char-select crash.
  // Loguear UNA vez para identificar al caller y eventualmente arreglar la
  // raíz. Por ahora, retornar 0 (slot inválido) para evitar el AV.
  if (!param_2 || !param_4) {
    static bool s_logged = false;
    if (!s_logged) {
      s_logged = true;
      char b[256];
      _snprintf_s(b, sizeof(b), _TRUNCATE,
        "FUN_004795c0 NULL-arg: type=0x%x scale=%.3f param_2=%p param_4=%p param_5=%d param_7=%d",
        param_1, param_3, param_2, param_4, param_5, param_7);
      DbgLogPublic(b);
    }
    return 0;
  }
  const int kPoolSlots = 1002;
  char *pcVar2 = DAT_07c85890;
  for (int iVar1 = 0; iVar1 < kPoolSlots; ++iVar1, pcVar2 += 0x1bc) {
    if (*pcVar2 == '\0') {
      *(unsigned short *)(pcVar2 + 2) = param_1;
      *(int *)(pcVar2 + 4) = param_7;
      *(int *)(pcVar2 + 0xfc) = param_5;
      *(float *)(pcVar2 + 0xc) = param_3;
      *(float *)(pcVar2 + 0x24) = param_6;
      *pcVar2 = '\x01';
      pcVar2[0x108] = '\0';
      pcVar2[0x109] = '\0';
      pcVar2[0x10a] = -0x80;
      pcVar2[0x10b] = '?';
      *(float *)(pcVar2 + 0x10) = param_2[0];
      *(float *)(pcVar2 + 0x14) = param_2[1];
      *(float *)(pcVar2 + 0x18) = param_2[2];
      *(float *)(pcVar2 + 0x170) = param_2[0];
      *(float *)(pcVar2 + 0x174) = param_2[1];
      *(float *)(pcVar2 + 0x178) = param_2[2];
      *(float *)(pcVar2 + 0xe8) = param_4[0];
      *(float *)(pcVar2 + 0xec) = param_4[1];
      *(float *)(pcVar2 + 0xf0) = param_4[2];
      return iVar1;
    }
  }
  return 0;  // pool full
}
