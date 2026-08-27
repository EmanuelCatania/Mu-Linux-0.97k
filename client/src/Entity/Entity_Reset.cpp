// Entity_Reset.cpp
// FUN_00502ba0 @ 0x00502BA0 — Entity_Reset
// Initializes/resets an effect-entity slot with default field values,
// then applies type-specific overrides via a switch on entity type.
// Returns an integer state value (type-dependent, usually 0).
//
// Called from: Particle_Spawn (Particle_Spawn) when allocating a new slot.
// param_1: pointer to the effect entity struct (stride 0x1bc in the effect pool).
//
// Field layout (relevant offsets):
//   +0x02 short  entity type
//   +0x0c float  scale / color multiplier
//   +0x10 float  world X (also used as phase/timer)
//   +0x14 float  unused / random offset Y
//   +0x58 uint   render type (0xffffffff=default, 1=add, 2=blend)
//   +0x64 uint   lifetime/frame count (-1=no limit)
//   +0x68 float  size / scale value
//   +0x6c float  spin speed X
//   +0x70 float  spin speed Y
//   +0x78 uint   flags
//   +0x8c byte   extra flag
//   +0xcc uint   color flags
//   +0xd0 float  max lifetime
//   +0xd8 uint   ???
//   +0xdc byte   visible flag
//   +0xe8..0xf0 float[3] color (R,G,B) — default 0.3f each
//   +0x105 byte  anim_state_prev
//   +0x106 byte  anim_state
//   +0x108 uint  ???
//   +0x10c uint  ???
//   +0x161 byte  active flag
//   +0x168 float alpha

#include "stdafx.h"


int __cdecl FUN_00502ba0(int param_1)
{
  short sVar1;
  int iVar2;
  int iVar3;
  float10 fVar4;
  ulonglong uVar5;

  *(undefined4 *)(param_1 + 100) = 0xffffffff;          // lifetime = -1 (no limit)
  *(undefined4 *)(param_1 + 0x58) = 0xffffffff;          // render type = default
  sVar1 = *(short *)(param_1 + 2);                       // entity type
  *(undefined4 *)(param_1 + 0xe8) = 0x3e99999a;          // color R = 0.3f
  *(undefined4 *)(param_1 + 0xec) = 0x3e99999a;          // color G = 0.3f
  *(undefined4 *)(param_1 + 0xf0) = 0x3e99999a;          // color B = 0.3f
  iVar2 = 0;
  *(undefined1 *)(param_1 + 0xdc) = 1;                   // visible = true
  *(undefined1 *)(param_1 + 0x161) = 0;                  // active flag = 0
  *(undefined1 *)(param_1 + 0x8c) = 0;                   // extra flag = 0
  *(undefined4 *)(param_1 + 0xcc) = 0;                   // color flags = 0
  *(undefined4 *)(param_1 + 0xd0) = 0x42480000;          // max lifetime = 50.0f
  *(undefined4 *)(param_1 + 0x10c) = 0;
  *(undefined4 *)(param_1 + 0x108) = 0;
  *(undefined1 *)(param_1 + 0x106) = 0;                  // anim state = 0
  *(undefined1 *)(param_1 + 0x105) = 0;                  // anim state prev = 0
  *(undefined4 *)(param_1 + 0xd8) = 0;
  *(undefined4 *)(param_1 + 0x168) = 0x3f800000;         // alpha = 1.0f
  *(undefined4 *)(param_1 + 0x68) = 0x3f800000;          // scale = 1.0f
  *(undefined4 *)(param_1 + 0x6c) = 0;                   // spin X = 0
  *(undefined4 *)(param_1 + 0x70) = 0;                   // spin Y = 0
  *(undefined4 *)(param_1 + 0xc) = 0x3f4ccccd;           // color mul = 0.8f
  *(undefined4 *)(param_1 + 0x78) = 0;                   // flags = 0

  // Reduce color multiplier for entity types 0x1f0..0x24f
  if ((0x1ef < sVar1) && (sVar1 < 0x250)) {
    *(undefined4 *)(param_1 + 0xc) = 0x3f333333;         // color mul = 0.7f
  }

  iVar3 = (int)sVar1;
  if (iVar3 < 0x231) {
    if (iVar3 == 0x230) {
      *(undefined4 *)(param_1 + 100) = 2;
      return iVar2;
    }
    switch (iVar3) {
    case 0x195:
    case 0x19d:
      goto switchD_00502c73_caseD_195;
    default:
      goto switchD_00502c73_caseD_196;
    case 0x19a:
    case 0x1d6:
    case 0x1f0:
      goto switchD_00502c73_caseD_19a;
    case 0x19e:
      *(undefined4 *)(param_1 + 100) = 1;
      iVar3 = _rand();
      iVar2 = iVar3 / 10;
      *(float *)(param_1 + 0x6c) = (float)(iVar3 % 10) * _DAT_005524f4;
      break;
    case 0x1af:
    case 0x1d7:
switchD_00502c73_caseD_1af:
      *(undefined4 *)(param_1 + 0x58) = 2;
      return iVar2;
    case 0x1d4:
      *(undefined4 *)(param_1 + 100) = 1;
      fVar4 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524bc);
      *(float *)(param_1 + 0x68) = (float)(fVar4 * (float10)_DAT_005526e4 + (float10)_DAT_00552530);
      return iVar2;
    case 0x1d5:
      goto switchD_00502c73_caseD_1d5;
    case 0x1f9:
      uVar5 = (longlong)DAT_05826e08;   // IDA ItemObjectAttribute case 505: (__int64)WorldTime % 2000
      uVar5 = (ulonglong)(uint)((int)uVar5 >> 0x1f) << 0x20 | uVar5 & 0xffffffff;
      *(float *)(param_1 + 0x70) = (float)(int)((longlong)uVar5 % 2000) * _DAT_0055291c;
      return (int)((longlong)uVar5 / 2000);
    case 0x1fa:
      *(undefined4 *)(param_1 + 0x58) = 1;
      return iVar2;
    case 0x216:
    case 0x21d:
    case 0x21e:
      *(undefined4 *)(param_1 + 100) = 0xfffffffe;
      break;
    case 0x220:
switchD_00502c73_caseD_220:
      *(undefined4 *)(param_1 + 100) = 0xfffffffe;
      fVar4 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524bc);
      *(float *)(param_1 + 0x68) = (float)(fVar4 * (float10)_DAT_005526e4 + (float10)_DAT_005526e8);
      return iVar2;
    }
  }
  else {
    if (0x25d < iVar3) {
      if (0x31b < iVar3) {
        iVar2 = iVar3 + -0x330;
        switch (iVar3) {
        case 0x330:
          goto switchD_00502c73_caseD_195;
        default:
switchD_00502c73_caseD_196:
          return iVar2;
        case 0x351:
          *(undefined4 *)(param_1 + 0xc) = 0x3f800000;   // color mul = 1.0f
          return iVar2;
        case 0x365:
          *(undefined4 *)(param_1 + 0xc) = 0x3f000000;   // color mul = 0.5f
          return iVar2;
        case 0x370:
          *(undefined4 *)(param_1 + 0xc) = 0x3f333333;   // color mul = 0.7f
          return iVar2;
        }
      }
      if (iVar3 < 0x318) {
        switch (iVar3) {
        case 0x25e:
          *(undefined4 *)(param_1 + 100) = 1;
          iVar2 = _rand();
          *(float *)(param_1 + 0x6c) = (float)(iVar2 % 10) * _DAT_005524f4;
          iVar2 = _rand();
          *(float *)(param_1 + 0x70) = (float)(iVar2 % 10) * _DAT_005524f4;
          return iVar2 / 10;
        default:
          goto switchD_00502c73_caseD_196;
        case 0x260:
          goto switchD_00502c73_caseD_1af;
        case 0x310:
          break;
        case 0x313:
          *(undefined4 *)(param_1 + 0xc) = 0x3f000000;   // color mul = 0.5f
        }
      }
switchD_00502c73_caseD_1d5:
      *(undefined4 *)(param_1 + 100) = 0;
      return iVar2;
    }
    if (iVar3 < 0x25b) {
      iVar2 = iVar3 + -0x235;
      switch (iVar2) {
      case 0:
        *(undefined4 *)(param_1 + 100) = 2;
        iVar2 = _rand();
        *(float *)(param_1 + 0x6c) = (float)(iVar2 % 10) * _DAT_005524f4;
        iVar2 = _rand();
        *(float *)(param_1 + 0x70) = (float)(iVar2 % 10) * _DAT_005524f4;
        return iVar2 / 10;
      case 1:
        *(undefined4 *)(param_1 + 100) = 0xfffffffe;
        fVar4 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524bc);
        *(float *)(param_1 + 0x68) =
             (float)(fVar4 * (float10)_DAT_005528b8 + (float10)_DAT_00552928);
        return iVar2;
      case 2:
        *(undefined4 *)(param_1 + 100) = 1;
        iVar2 = _rand();
        *(float *)(param_1 + 0x68) = (float)(iVar2 % 0xb) * _DAT_005524f4;
        return iVar2 / 0xb;
      case 3:
        goto switchD_00502c73_caseD_220;
      case 4:
        goto switchD_00502c73_caseD_195;
      default:
        goto switchD_00502c73_caseD_196;
      }
    }
switchD_00502c73_caseD_19a:
    *(undefined4 *)(param_1 + 100) = 1;
  }
  fVar4 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524bc);
  *(float *)(param_1 + 0x68) = (float)(fVar4 * (float10)_DAT_005528b8 + (float10)_DAT_00552928);
  return iVar2;
switchD_00502c73_caseD_195:
  *(undefined4 *)(param_1 + 100) = 1;
  return iVar2;
}
