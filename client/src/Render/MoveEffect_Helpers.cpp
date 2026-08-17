// MoveEffect_Helpers.cpp
// Helper functions called from FUN_00466AD0 (MoveEffect).
//
// FUN_004660f0 @ 0x004660F0  — Effect_SmokeBurst   (smoke + optional sparkle)
// FUN_004661f0 @ 0x004661F0  — Effect_SmokeExplosion (int-coord variant)
// FUN_00460c30 @ 0x00460C30  — Effect_LightningBurst (3 random lightning beams)
// FUN_00465e60 @ 0x00465E60  — Effect_OnHitProximity (proximity hit fx by entity type)
// FUN_00473d90 @ 0x00473D90  — Ring_ComputeOrbit    (Lissajous ring position calculator)
//
// NOTE: FUN_00466440 @ 0x00466440 — NOT implemented here.
//   Uses unaff_retaddr + unaff_EBP (phantom return-address / frame-pointer params)
//   and heavy HashTable reference-count obfuscation — cannot be ported safely.
//   Kept as empty stub in stubs.cpp.

#include "stdafx.h"


// FUN_004660f0 — Effect_SmokeBurst
// Spawns 20 particles of type 0x497 (smoke/debris) at param_1 position,
// offset upward by _DAT_00552878 in Z. If param_2 != 0, also spawns
// a type 0x4bf (sparkle) particle at the same position.
// param_1: world position float[3]
// param_2: flag — non-zero = also emit sparkle
void __cdecl FUN_004660f0(float *param_1, char param_2)
{
  // 004660F0 (CreateBomb): preserve the original particle types, angles and
  // position offset. The older body below was a smoke approximation.
  float light[3] = { 1.0f, 1.0f, 1.0f };
  float angle[3];
  float position[3] = { param_1[0], param_1[1], param_1[2] + 80.0f };
  for (int i = 20; i != 0; --i) {
    angle[0] = (float)(_rand() % 60 + 150);
    angle[1] = 0.0f;
    angle[2] = (float)(_rand() % 30);
    FUN_00475220(1175, position, angle, light, 2, 1.0f, 0);
  }
  light[0] = 0.7f;
  light[1] = 0.7f;
  light[2] = 0.7f;
  if (param_2 != 0)
    FUN_00475220(1215, position, angle, light, 0, 1.0f, 0);
  return;

#if 0 // superseded approximation kept only as historical reference
  int iVar1;
  int iVar2;
  float local_24[4];   // color/alpha: [0]=R,[1]=G,[2]=B,[3]=size
  undefined4 local_14; // unused/padding
  float local_10;      // position[0] copy
  float local_c;       // position[1] copy (unused output of rand)
  float local_8;       // position[2] copy
  float local_4;       // Z offset position

  local_24[0] = 1.0f;
  local_24[1] = 1.0f;
  local_24[2] = 1.0f;
  local_c = *param_1;
  local_8 = param_1[1];
  local_4 = param_1[2] + _DAT_00552878;
  iVar2 = 0x14;  // spawn 20 smoke particles
  do {
    iVar1 = _rand();
    local_14 = 0;
    local_24[3] = (float)(iVar1 % 0x3c + 0x96);  // size: 150-209
    iVar1 = _rand();
    local_10 = (float)(iVar1 % 0x1e);             // random float (0..29, unused by spawn)
    FUN_00475220(0x497, &local_c, local_24 + 3, local_24, 2, 1.0f, 0);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);

  local_24[0] = 0.7f;
  local_24[1] = 0.7f;
  local_24[2] = 0.7f;
  if (param_2 != '\0') {
    FUN_00475220(0x4bf, &local_c, local_24 + 3, local_24, 0, 1.0f, 0);
  }
  return;
#endif
}


// FUN_004661f0 — Effect_SmokeExplosion
// Same as Effect_SmokeBurst but takes an integer-coordinate position array
// (undefined4*) rather than float*. Spawns 20 type-0x497 particles.
// If param_2 != 0, spawns a larger type-0x4c0 (white explosion) particle at scale 4.0.
// param_1: world position int[3] (cast from float array by caller)
// param_2: flag — non-zero = also emit white explosion
void __cdecl FUN_004661f0(undefined4 *param_1, char param_2)
{
  // 004661F0: same bomb trail, with the 30-unit offset and 1216 explosion.
  float* input = (float*)param_1;
  float light[3] = { 1.0f, 1.0f, 1.0f };
  float angle[3];
  float position[3] = { input[0], input[1], input[2] + 30.0f };
  for (int i = 20; i != 0; --i) {
    angle[0] = (float)(_rand() % 60 + 150);
    angle[1] = 0.0f;
    angle[2] = (float)(_rand() % 30);
    FUN_00475220(1175, position, angle, light, 2, 1.0f, 0);
  }
  if (param_2 != 0) {
    angle[0] = angle[1] = angle[2] = 0.0f;
    (void)FUN_00475220(1216, position, angle, light, 0, 4.0f, 0);
  }
  return;

#if 0 // superseded approximation kept only as historical reference
  int iVar1;
  int iVar2;
  float local_24[7];   // [0-2]=color, [3-5]=velocity/direction, [6]=pos[0]
  undefined4 local_8;  // pos[1]
  float local_4;       // pos[2] + Z offset

  local_24[0] = 1.0f;
  local_24[1] = 1.0f;
  local_24[2] = 1.0f;
  local_24[6] = (float)*param_1;
  local_8 = param_1[1];
  local_4 = (float)param_1[2] + _DAT_0055284c;
  iVar2 = 0x14;  // spawn 20 smoke particles
  do {
    iVar1 = _rand();
    local_24[4] = 0.0f;
    local_24[3] = (float)(iVar1 % 0x3c + 0x96);  // size: 150-209
    iVar1 = _rand();
    local_24[5] = (float)(iVar1 % 0x1e);
    FUN_00475220(0x497, local_24 + 6, local_24 + 3, local_24, 2, 1.0f, 0);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);

  local_24[0] = 1.0f;
  local_24[1] = 1.0f;
  local_24[2] = 1.0f;
  local_24[3] = 0.0f;
  local_24[4] = 0.0f;
  local_24[5] = 0.0f;
  if (param_2 != '\0') {
    FUN_00475220(0x4c0, local_24 + 6, local_24 + 3, local_24, 0, 4.0f, 0);
  }
  return;
#endif
}


// FUN_00460c30 — Effect_LightningBurst
// Spawns 3 lightning beam joints (type 0x4eb) from entity param_1 to random
// directions. Each beam has a random horizontal angle (0-89 deg) and random
// elevation (0-359 deg), transformed into world space via Matrix_FromEuler +
// Matrix_TransformPoint. Used for electric/thunder death effects.
// param_1: entity pointer (float* to entity struct at stride 0x394)
void __cdecl FUN_00460c30(int param_1)
{
  int iVar1;
  int iVar2;
  float position[3];     // beam endpoint world XYZ (IDA Position[3])
  float local_48[3];     // Euler angles for random direction
  float local_3c[15];    // rotation matrix (12) + endpoint (3)

  local_3c[0] = 0.0f;
  local_3c[1] = -200.0f;  // beam length 200 units downward in local space
  local_3c[2] = 0.0f;
  iVar2 = 3;  // spawn 3 beams
  do {
    iVar1 = _rand();
    local_48[1] = 0.0f;
    local_48[0] = (float)(iVar1 % 0x5a);   // horizontal angle 0-89 deg
    iVar1 = _rand();
    local_48[2] = (float)(iVar1 % 0x168);  // elevation 0-359 deg
    FUN_004f9db0(local_48, local_3c + 3);  // build rotation matrix from Euler angles
    FUN_004fa0b0(local_3c, local_3c + 3, position);  // transform beam endpoint to world
    // Compute world endpoint relative to entity position
    position[0] = *(float *)(param_1 + 0x10) - position[0];
    position[1] = *(float *)(param_1 + 0x14) - position[1];
    position[2] = (*(float *)(param_1 + 0x18) - position[2]) + _DAT_00552908;
    // Spawn lightning joint: type=0x4eb, from entity pos, width 5.0, persistent
    FUN_0046d840(0x4eb, position, (float *)(param_1 + 0x10), local_48,
                 *(undefined4 *)(param_1 + 4),
                 *(int *)(param_1 + 0xfc), 5.0f, -1, 0);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


// FUN_00465e60 — Effect_OnHitProximity
// Checks if the entity's target (at +0xfc) is within _DAT_005524f0 distance.
// If within range, spawns hit effects based on entity type (+0x02):
//   0x00BF: spawns 2 random type 0xC5/0xC6 effects (physical hit variants)
//   0x00D4: spawns 2 random type 0xD5/0xD6 effects + particle 0x4C4 + sound 0x30
//   0x049C: spawns particle 0x498 at scale 6.0 (area-of-effect indicator)
// param_1: entity pointer (int, base of entity struct)
void __cdecl FUN_00465e60(int param_1)
{
  float *pfVar1;
  short sVar2;
  char *pcVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  int iVar7;
  float *pfVar8;
  float *pfVar9;
  float *pfVar10;
  float *pfVar11;
  float *pfVar12;
  float *pfVar13;
  float *pfVar14;
  byte bVar15;
  float local_c[3];

  pcVar3 = *(char **)(param_1 + 0xfc);
  // Check: target valid, active, alive, and within distance threshold
  if ((((pcVar3 != (char *)0x0) && (*pcVar3 != '\0')) && (pcVar3[0x160] != '\0')) &&
      (fVar4 = *(float *)(param_1 + 0x10) - *(float *)(pcVar3 + 0x10),
       pfVar1 = (float *)(param_1 + 0x10),
       fVar5 = *(float *)(param_1 + 0x14) - *(float *)(pcVar3 + 0x14),
       SQRT(fVar4 * fVar4 + fVar5 * fVar5) <= _DAT_005524f0)) {
    sVar2 = *(short *)(param_1 + 2);
    *(undefined4 *)(param_1 + 0x60) = 1;  // mark hit flag
    local_c[0] = 1.0f;
    local_c[1] = 1.0f;
    local_c[2] = 1.0f;
    if (sVar2 == 0xbf) {  // entity type: physical attacker
      if (*(int *)(param_1 + 4) == 1) {
        iVar7 = 2;
        do {
          bVar15 = 0;
          pfVar14 = (float *)0x0;
          pfVar13 = (float *)0xffffffff;
          pfVar12 = (float *)0x0;
          pfVar11 = (float *)0x0;
          pfVar8 = pfVar1;
          pfVar9 = (float *)(param_1 + 0x1c);
          pfVar10 = (float *)(param_1 + 0xe8);
          uVar6 = _rand();
          uVar6 = uVar6 & 0x80000001;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
          }
          // Random hit effect: type 0xC5 or 0xC6
          FUN_00460dc0(uVar6 + 0xc5, pfVar8, pfVar9, pfVar10, pfVar11, pfVar12, pfVar13, pfVar14, bVar15);
          iVar7 = iVar7 + -1;
        } while (iVar7 != 0);
      }
    }
    else {
      if (sVar2 == 0xd4) {  // entity type: magic attacker
        iVar7 = 2;
        do {
          bVar15 = 0;
          pfVar14 = (float *)0x0;
          pfVar13 = (float *)0xffffffff;
          pfVar12 = (float *)0x0;
          pfVar11 = (float *)0x0;
          pfVar8 = pfVar1;
          pfVar9 = (float *)(param_1 + 0x1c);
          pfVar10 = (float *)(param_1 + 0xe8);
          uVar6 = _rand();
          uVar6 = uVar6 & 0x80000001;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
          }
          // Random magic hit effect: type 0xD5 or 0xD6
          FUN_00460dc0(uVar6 + 0xd5, pfVar8, pfVar9, pfVar10, pfVar11, pfVar12, pfVar13, pfVar14, bVar15);
          FUN_00475220(0x4c4, pfVar1, (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 0, 1.0f, 0);
          iVar7 = iVar7 + -1;
        } while (iVar7 != 0);
        FUN_00404bc0(0x30, 0, 0);  // play sound 0x30 (magic hit sfx)
        return;
      }
      if (sVar2 == 0x49c) {  // entity type: area-of-effect skill
        FUN_00475220(0x498, pfVar1, (float *)(param_1 + 0x1c), local_c, 1, 6.0f, 0);
        return;
      }
    }
  }
  return;
}


// FUN_00473d90 — Ring_ComputeOrbit
// Computes a point on a 3-axis Lissajous orbit curve for ring/trail effects.
// Uses three independent sin/cos pairs with different frequency scales
// (_DAT_00552a9c, _DAT_00552aa0, _DAT_00552aa8) to produce smooth 3D orbital motion.
// param_1: phase index (combined with frame counter from HashTable)
// param_2: output float[3] — computed orbit position
// param_3: scale / frequency multiplier
//
// NOTE: The HashTable block at entry (FUN_0043d3e0/HashTable_GetIndex/FUN_00404280)
// reads a frame counter (DAT_083a7c00) used as a phase seed — it's anti-tamper
// ref-count obfuscation around the real value. Here we use DAT_083a7c00 directly.
void __cdecl FUN_00473d90(int param_1, float *param_2, float param_3)
{
  // 00473D90: the hash-table operations only protect the scene-frame read.
  // The visual orbit itself is the following direct trigonometric sequence.
  const int frame = DAT_083a7c00;
  const int phase = frame + param_1;
  const double a = (double)param_3 * 0.048 * (double)(phase + 55555);
  const double b = (double)phase * ((double)param_3 * 0.061299998);
  const double c = (double)(phase + 11111) * ((double)param_3 * 0.1113);
  const float xyCos = (float)(fcos(b) * fsin(a));
  const float xySin = (float)(fsin(b) * fsin(a));
  const float zCos = (float)fcos(a);
  const float cSin = (float)fsin(c);
  const float cCos = (float)fcos(c);
  param_2[2] = xyCos;
  param_2[1] = cCos * zCos + cSin * xySin;
  param_2[0] = cCos * xySin - cSin * zCos;
}
