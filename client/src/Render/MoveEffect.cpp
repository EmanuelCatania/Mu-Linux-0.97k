// MoveEffect.cpp
// FUN_00466ad0 @ 0x00466AD0  [Kayito: MoveEffect]
//
// Per-frame particle/effect spawner for active effect slots.
// Called by Effect_TickAll (FUN_0046b790) once per slot per frame.
//
// param_1 = float* pointing to effect slot (stride 0x1bc, base DAT_07b11670)
// param_2 = slot index
//
// Effect slot layout (float offsets):
//   [+0x00/char] = active flag
//   [+0x02/short]= entity type id (iVar9)
//   [+0x04]      = position X
//   [+0x05]      = position Y
//   [+0x06]      = position Z
//   [+0x07..09]  = angle/dir XYZ
//   [+0x0a..0c]  = angle2 XYZ
//   [+0x10]      = cached pos X
//   [+0x11]      = cached pos Y
//   [+0x12]      = cached pos Z
//   [+0x16]      = some float state
//   [+0x18]      = lifetime counter (fVar13)
//   [+0x1a]      = alpha/scale
//   [+0x1b]      = secondary scale
//   [+0x1e]      = flags word
//   [+0x21/byte] = byte flag
//   [+0x22/byte] = byte flag2
//   [+0x30..32]  = velocity XYZ
//   [+0x33]      = some float
//   [+0x36]      = Z velocity / bounce
//   [+0x3a..3c]  = color RGB
//   [+0x3d]      = float flag
//   [+0x3f]      = entity ptr (target entity)
//   [+0x42..43]  = float counters
//   [+0x5a]      = alpha override
//   [+0x5c..5e]  = secondary position XYZ
//   [+0x85/byte] = byte param
//   [+0x86/short]= short param
//   [+0x105/byte]= anim state
//   [+0x106/byte]= anim state prev
//
// External functions:
//   FUN_00475220 = Particle_Spawn(type, pos, dir, color, mode, size, entity_ptr)
//   FUN_00460dc0 = CreateEffect(type, pos, dir, color, v1, v2, v3, v4, flag)
//   FUN_0046d840 = CreateJoint(type, pos, target, dir, mode, entity, size, ...)
//   FUN_004f9db0 = AngleMatrix(angles, matrix)
//   FUN_004fa0b0 = EulerToMatrix3x4(v, matrix, out)
//   FUN_004f7500 = RequestTerrainHeight(x, y)
//   FUN_004f76c0 = RequestTerrainLight(x, y, light_ptr, mode, addr)
//   FUN_004795c0 = FUN_004795c0 (Flare_Spawn?)
//   FUN_00465fe0 = FUN_00465fe0 (effect color update)
//   FUN_00465e60 = FUN_00465e60 (effect deactivate?)
//   FUN_00466440 = FUN_00466440 (effect helper)
//   FUN_004660f0 = FUN_004660f0 (effect helper2)
//   FUN_004661f0 = FUN_004661f0 (effect helper3)
//   FUN_0046c3e0 = Trail_RenderAll?
//   FUN_00440aa0 = BMD_SetAnim?
//   FUN_0045fec0 = Entity_SpawnImpact?
//   FUN_00404bc0 = Sound_Play(id, slot, flag)
//   FUN_00473d90 = FUN_00473d90 (ring?)
//   FUN_004f6c40 = Terrain_GetTileAttr(x, y)
//   FUN_004f6c30 = FUN_004f6c30 (terrain helper)

#include "stdafx.h"

void FUN_00466ad0(float *param_1, int param_2)
{
  void *pvVar1;
  char cVar2;
  short sVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  uint uVar8;
  int iVar9;
  float *pfVar10;
  char *pcVar11;
  int iVar12;
  float fVar13;
  undefined2 extraout_var;
  float *pfVar14;
  float *pfVar15;
  bool bVar16;
  float10 fVar17;
  float10 fVar18;
  longlong lVar19;
  longlong lVar20;
  float *pfVar21;
  float *pfVar22;
  float *pfVar23;
  int iVar24;
  float *pfVar25;
  float *pfVar26;
  float *pfVar27;
  byte bVar28;
  undefined4 uVar29;
  int iVar30;
  float local_36c;
  float local_368;
  float local_364;
  float local_360;
  float *local_35c;
  float local_358;
  float local_354;
  float local_350;
  float local_34c;
  float local_348;
  float local_344;
  float local_340;
  float local_33c;
  float local_338;
  float local_334;
  float local_330;
  float local_32c;
  float local_328;
  float local_324;
  float local_320;
  float local_31c;
  undefined4 local_318;
  undefined4 local_314;
  float local_310;
  float local_30c;
  undefined4 local_308;
  float local_304;
  float local_300;
  float local_2fc;
  float local_2f8;
  float local_2f4;
  float local_2f0;
  float local_2ec;
  float local_2e8;
  float local_2e4;
  float local_2e0;
  float local_2dc;
  float local_2d8;
  float local_2d4;
  float local_2d0;
  float local_2cc;
  float local_2c8;
  float local_2c4;
  float local_2c0;
  float local_2bc;
  float local_2b8;
  float local_2b4;
  float local_2b0;
  float local_2ac;
  float local_2a8;
  float local_2a4;
  float local_2a0;
  float local_29c;
  float local_298;
  float local_294;
  float local_290;
  float local_28c;
  uint local_288;
  float local_284;
  float local_280;
  float local_27c;
  float local_278;
  undefined4 local_274;
  float local_270;
  undefined4 local_26c;
  undefined4 local_268;
  undefined4 local_264;
  float local_260;
  float local_25c;
  float local_258;
  float local_254;
  undefined4 local_250;
  undefined4 local_24c;
  float local_248;
  float local_244;
  float local_240;
  float local_23c;
  float local_238;
  float local_234;
  float local_230;
  float local_22c;
  undefined4 local_228;
  float local_224;
  float local_220;
  float local_21c;
  float local_218;
  float local_214;
  undefined4 local_210;
  float local_20c;
  undefined4 local_208;
  undefined4 local_204;
  float local_200;
  float local_1fc;
  undefined4 local_1f8;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  float local_1e8;
  undefined4 local_1e4;
  float local_1e0;
  float local_1dc;
  undefined4 local_1d8;
  float local_1d4;
  float local_1d0;
  float local_1cc;
  float local_1c8;
  float local_1c4;
  undefined4 local_1c0;
  float local_1bc;
  float local_1b8;
  undefined4 local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  float local_1a4[18];
  float local_15c[12];
  float local_12c[12];
  float local_fc[12];
  float local_cc[12];
  float local_9c[12];
  float local_6c[13];
  float local_38[14];

// SECTION_BODY_START
  do {
    uVar8 = _rand();
    uVar8 = uVar8 & 0x80000003;
    if ((int)uVar8 < 0) {
      uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
    }
    local_35c = (float *)(uVar8 + 7);
    // 2026-08-15: el counter (+96) es un DWORD. IDA: `v4 = *(int *)(o + 96)`
    // y compara `v4`; `v356 = *(float *)&v4` es solo la copia de los BITS.
    // El port hacia `(int)fVar13`, o sea convertia el float numericamente -> 0.
    const int __cnt = *(int*)&param_1[0x18];
    fVar13 = param_1[0x18];
    local_36c = (float)(int)local_35c * _DAT_005524f4;
    if (__cnt < 5) {
      local_35c = (float *)(5 - __cnt);
      local_36c = local_36c - (float)(int)local_35c * _DAT_005526e4;
      if (local_36c < _DAT_00552580) {
        local_36c = 0.0;
      }
    }
    sVar3 = *(short *)((int)param_1 + 2);
    local_368 = 1.0;
    iVar9 = (int)sVar3;
    local_364 = 1.0;
    local_360 = 1.0;
    local_358 = fVar13;
    if (0x1f0 < iVar9) {
      if (iVar9 < 0x4b1) {
        if (iVar9 == 0x4b0) {
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000007;
          bVar16 = uVar8 == 0;
          if ((int)uVar8 < 0) {
            bVar16 = (uVar8 - 1 | 0xfffffff8) == 0xffffffff;
          }
          if (bVar16) {
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar14 = param_1 + 0x3a;
            pfVar22 = (float *)0x0;
            pfVar10 = param_1 + 7;
            pfVar15 = param_1 + 4;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000001;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
            }
            FUN_00460dc0(uVar8 + 0xc5,pfVar15,pfVar10,pfVar14,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
          pfVar10 = param_1 + 4;
          iVar9 = 6;
          do {
            iVar12 = _rand();
            local_354 = (float)(iVar12 % 0x32 + -0x19);
            iVar12 = _rand();
            local_34c = param_1[6];
            local_354 = local_354 + *pfVar10;
            local_350 = (float)(iVar12 % 0x32 + -0x19) + param_1[5];
            FUN_00475220(0x4b0,&local_354,param_1 + 7,&local_368,0,1.0,0);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
          local_364 = local_36c * _DAT_005528b4;
          local_368 = local_36c;
          local_360 = 0.0;
          FUN_004f76c0(*pfVar10,param_1[5],(int)&local_368,3,(int)DAT_081cb608);
          if (((char *)*(int*)&param_1[0x3f] == DAT_07abf5d8) && ((*(int*)&param_1[0x18]) % 0x14 == 0)) {
            FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),pfVar10,150.0,
                         *(byte *)(param_1 + 0x22),*(short *)((int)param_1 + 0x86));
          }
        }
        else if (iVar9 < 0x499) {
          if (iVar9 == 0x498) {
            local_294 = param_1[4];
            local_290 = param_1[5];
            local_28c = param_1[6];
            local_368 = (float)(int)fVar13 * _DAT_005524f4;
            iVar9 = 0;
            local_364 = local_368;
            local_360 = local_368;
            do {
              local_28c = local_28c + _DAT_005529a8;
              if (iVar9 == 0) {
                fVar13 = 12.0;
              }
              else {
                fVar13 = 6.0;
              }
              FUN_00475220(0x498,&local_294,param_1 + 7,&local_368,1,fVar13,0);
              iVar9 = iVar9 + 1;
            } while (iVar9 < 0x12);
          }
          else if (iVar9 < 0x47f) {
            if (iVar9 == 0x47e) {
              if (param_1[1] == 0.0) {
                param_1[4] = param_1[0x30] + param_1[4];
                param_1[5] = param_1[0x31] + param_1[5];
                param_1[6] = param_1[0x32] + param_1[6];
                param_1[0x32] = param_1[0x32] - _DAT_005524f8;
                FUN_00475220(0x47e,param_1 + 4,param_1 + 7,param_1 + 0x3a,1,param_1[3],0);
                if (param_1[0x32] < _DAT_005529ac) goto LAB_0046a366;
              }
              else if (param_1[1] == 1.4013e-45) {
                pcVar11 = (char *)*(int*)&param_1[0x3f];
                if (((pcVar11 == (char *)0x0) || (*pcVar11 != '\x01')) ||
                   (((byte)*(undefined4 *)(pcVar11 + 0x78) & 0x10) != 0x10)) {
                  param_1[0x18] = 0.0;
                }
                else {
                  param_1[0x18] = 1.4013e-44;
                }
                local_368 = 1.0;
                local_364 = 0.5;
                local_360 = 0.1;
                iVar9 = _rand();
                FUN_00475220(0x47e,param_1 + 4,&local_334,&local_368,4,
                             (float)(byte)(DAT_00559b78)[iVar9 % 7],*(int*)&param_1[0x3f]);
                FUN_00475220(0x47e,param_1 + 4,&local_334,&local_368,4,
                             (float)(byte)(DAT_00559b7f)[-(iVar9 % 7)],*(int*)&param_1[0x3f]);
              }
            }
            else if (iVar9 == 0x1f1) {
              local_254 = 0.0;
              local_250 = 0xc2c80000;
              local_24c = 0;
              iVar9 = 3;
              do {
                iVar12 = _rand();
                local_274 = 0;
                local_278 = (float)(iVar12 % 0x5a);
                iVar12 = _rand();
                local_270 = (float)(iVar12 % 0x168);
                FUN_004f9db0(&local_278,local_fc);
                FUN_004fa0b0(&local_254,local_fc,&local_2a0);
                local_2a0 = param_1[4] - local_2a0;
                local_29c = param_1[5] - local_29c;
                local_298 = param_1[6] - local_298;
                FUN_0046d840(0x4eb,&local_2a0,param_1 + 4,&local_278,6,0,5.0,-1,0);
                iVar9 = iVar9 + -1;
              } while (iVar9 != 0);
            }
            else if (iVar9 == 0x238) {
              pfVar10 = param_1 + 4;
              fVar17 = FUN_004f7500(param_1[4], param_1[5]);
              if ((float10)param_1[6] < fVar17) {
                local_354 = *pfVar10;
                local_350 = param_1[5];
                local_34c = param_1[6] + _DAT_00552878;
                FUN_00475220(0x4bf,&local_354,param_1 + 7,&local_368,0,1.0,0);
                iVar9 = 6;
                do {
                  bVar28 = 0;
                  pfVar27 = (float *)0x0;
                  pfVar25 = (float *)0xffffffff;
                  pfVar23 = (float *)0x0;
                  pfVar15 = param_1 + 0x3a;
                  pfVar21 = (float *)0x0;
                  pfVar14 = pfVar10;
                  pfVar22 = param_1 + 7;
                  uVar8 = _rand();
                  uVar8 = uVar8 & 0x80000001;
                  if ((int)uVar8 < 0) {
                    uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
                  }
                  FUN_00460dc0(uVar8 + 0xc5,pfVar14,pfVar22,pfVar15,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
                  iVar9 = iVar9 + -1;
                } while (iVar9 != 0);
                *(undefined1 *)param_1 = 0;
              }
              local_368 = local_36c * _DAT_005526e4;
              local_360 = local_36c;
              local_364 = local_36c * _DAT_005528b4;
              FUN_004f76c0(*pfVar10,param_1[5],(int)&local_368,2,(int)DAT_081cb608);
            }
          }
          else if ((iVar9 == 0x490) && (fVar13 == 4.2039e-44)) {
            local_354 = param_1[4];
            local_350 = param_1[5];
            local_34c = param_1[6] + _DAT_005524f0;
            FUN_0046d840(0x4ee,&local_354,&local_354,param_1 + 10,0,*(int*)&param_1[0x3f],150.0,
                         *(short *)((int)param_1 + 0x86),*(byte *)((int)param_1 + 0x85));
          }
        }
        else {
          switch(iVar9) {
          case 0x49c:
            fVar13 = (float)(int)fVar13 * _DAT_005526e4;
            fVar4 = fVar13;
            fVar5 = fVar13;
            fVar6 = fVar13;
            if ((param_1[1] != 0.0) &&
               (fVar4 = local_368, fVar5 = local_364, fVar6 = local_360, param_1[1] == 1.4013e-45))
            {
              local_368 = 1.0;
              local_364 = 0.8;
              local_360 = 0.2;
              fVar4 = local_368;
              fVar5 = local_364;
              fVar6 = local_360;
            }
            local_360 = fVar6;
            local_364 = fVar5;
            local_368 = fVar4;
            pfVar10 = param_1 + 4;
            FUN_00475220(0x49c,pfVar10,param_1 + 7,&local_368,0,1.0,0);
            FUN_00475220(0x498,pfVar10,param_1 + 7,&local_368,0,4.0,0);
            local_368 = fVar13 * _DAT_005526e4;
            local_364 = fVar13 * _DAT_005528b4;
            local_360 = fVar13;
            FUN_004f76c0(*pfVar10,param_1[5],(int)&local_368,2,(int)DAT_081cb608);
            FUN_00465e60((int)param_1);
            break;
          case 0x4a7:
            local_360 = (float)(int)fVar13 * _DAT_005526e4;
            local_368 = local_360 * _DAT_005526e4;
            local_364 = local_360 * _DAT_00552504;
            goto LAB_004695b5;
          case 0x4ab:
            FUN_00475220(0x4ab,param_1 + 4,param_1 + 7,&local_368,9,1.0,*(int*)&param_1[0x3f]);
            break;
          case 0x4ac:
            local_368 = 1.0;
            local_364 = 1.0;
            local_360 = 1.0;
            FUN_00475220(0x4ac,param_1 + 4,param_1 + 7,&local_368,1,1.0,0);
            local_364 = local_36c * _DAT_00552534;
            local_368 = local_36c;
            local_360 = local_36c * _DAT_005528b8;
            FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
          }
        }
      }
// SECTION_2
      else {
        switch(iVar9) {
        case 0x4df:
          if (fVar13 == 1.4013e-45) {
            iVar9 = _rand();
            iVar12 = _rand();
            local_368 = (float)(iVar12 % 3) * _DAT_005528b8 + _DAT_005528b4;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000003;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
            }
            local_360 = 0.0;
            iVar12 = 0x50;
            local_364 = (float)(int)uVar8 * _DAT_005524f4;
            do {
              FUN_00475220(0x4df,param_1 + 4,param_1 + 7,&local_368,iVar9 % 0x1e,1.0,0);
              iVar12 = iVar12 + -1;
            } while (iVar12 != 0);
            FUN_00404bc0(0x45,(unsigned int)(uintptr_t)param_1,0);
          }
          FUN_00475220(0x4df,param_1 + 4,param_1 + 7,param_1 + 0x3a,-1,1.0,0);
          local_368 = local_36c * _DAT_005528b4;
          local_364 = local_36c * _DAT_005528b8;
          local_360 = local_36c * _DAT_005526e4;
          FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,1,(int)DAT_081cb608);
          break;
        case 0x4e0:
          if (((int)fVar13 % 5 == 0) && (iVar9 = _rand(), iVar9 % 3 == 0)) {
            FUN_00460dc0(0x4df,param_1 + 4,param_1 + 7,param_1 + 0x3a,(float *)0x0,(float *)0x0,
                         (float *)0xffffffff,(float *)0x0,0);
          }
          break;
        case 0x4f0:
        case 0x4f1:
          if ((0 < (int)param_1[1]) && (param_1[1] != 5.60519e-45)) {
            FUN_00460c30((int)param_1);
          }
          break;
        case 0x4f3:
          if (param_1[1] == 1.4013e-45) {
            local_31c = 0.0;
            local_318 = 0;
            local_314 = 0x41200000;
            iVar9 = *(int *)(*(int*)&param_1[0x3f] + 0x114);
            pvVar1 = (void *)(DAT_05828d58 + *(short *)(*(int*)&param_1[0x3f] + 2) * 0xbc);
            *(float *)((int)pvVar1 + 0x6c) = param_1[0x5c];
            *(float *)((int)pvVar1 + 0x70) = param_1[0x5d];
            *(float *)((int)pvVar1 + 0x74) = param_1[0x5e];
            FUN_004409a0(pvVar1,(float *)(iVar9 + 0x630),&local_31c,param_1 + 4,'\x01');
          }
          pfVar10 = param_1 + 4;
          iVar9 = 3;
          do {
            local_31c = 0.0;
            local_318 = 0x42f00000;
            local_314 = 0;
            iVar12 = _rand();
            local_330 = 0.0;
            local_334 = (float)(iVar12 % 0x168);
            iVar12 = _rand();
            local_32c = (float)(iVar12 % 0x168);
            FUN_004f9db0(&local_334,local_1a4 + 6);
            FUN_004fa0b0(&local_31c,local_1a4 + 6,&local_354);
            local_354 = local_354 + *pfVar10;
            local_350 = local_350 + param_1[5];
            local_34c = local_34c + param_1[6];
            if (param_1[1] == 1.4013e-45) {
              uVar8 = (*(uint*)&param_1[0x18]) & 0x80000001;
              bVar16 = uVar8 == 0;
              if ((int)uVar8 < 0) {
                bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
              }
              if (bVar16) {
                FUN_0046d840(0x4e6,&local_354,pfVar10,&local_334,3,0,10.0,10,10);
              }
              else {
                pfVar15 = param_1;
                iVar12 = _rand();
                FUN_00475220(0x498,&local_354,&local_334,&local_368,2,
                             (float)(iVar12 % 0x32 + 10) * _DAT_00552594,(int)pfVar15);
              }
              uVar29 = 0;
              iVar12 = _rand();
              pfVar15 = param_1 + 0x3a;
              fVar13 = (float)(iVar12 % 0x168);
              pfVar14 = param_1;
              uVar8 = _rand();
              uVar8 = uVar8 & 0x80000007;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
              }
              FUN_004795c0(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005526e4,pfVar15,(int)pfVar14,fVar13,uVar29);
            }
            else {
              uVar29 = 0;
              iVar12 = _rand();
              pfVar15 = param_1 + 0x3a;
              fVar13 = (float)(iVar12 % 0x168);
              pfVar14 = param_1;
              uVar8 = _rand();
              uVar8 = uVar8 & 0x80000007;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
              }
              FUN_004795c0(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005528b8,pfVar15,(int)pfVar14,fVar13,uVar29);
              pfVar15 = param_1;
              iVar12 = _rand();
              FUN_00475220(0x498,&local_354,&local_334,&local_368,2,
                           (float)(iVar12 % 0x32 + 10) * _DAT_00552594,(int)pfVar15);
            }
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
          break;
        case 0x4f7:
          if ((int)fVar13 < 0x10) {
            iVar9 = _rand();
            pfVar10 = param_1 + 4;
            fVar17 = (float10)fsin((float10)(iVar9 % 1000) * (float10)_DAT_005524f8);
            *pfVar10 = (float)(fVar17 * (float10)_DAT_00552488 + (float10)param_1[0x5c]);
            iVar9 = _rand();
            fVar17 = (float10)fsin((float10)(iVar9 % 1000) * (float10)_DAT_005524f8);
            param_1[5] = (float)(fVar17 * (float10)_DAT_00552488 + (float10)param_1[0x5d]);
            param_1[6] = param_1[6] + param_1[0x36];
            param_1[0x36] = param_1[0x36] - _DAT_0055264c;
            param_1[0x5c] = param_1[0x5c] - _DAT_00552488;
            FUN_00475220(0x4ad,pfVar10,param_1 + 7,&local_368,7,1.0,0);
            fVar13 = param_1[0x3a] + _DAT_005524f4;
            pfVar15 = param_1 + 0x3a;
            uVar29 = 0;
            *pfVar15 = fVar13;
            param_1[0x3b] = fVar13;
            param_1[0x3c] = fVar13;
            iVar9 = _rand();
            fVar13 = (float)(iVar9 % 0x168);
            pfVar14 = pfVar15;
            pfVar22 = param_1;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000003;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
            }
            FUN_004795c0(0x4cf,pfVar10,(float)(int)(uVar8 + 4) * _DAT_005526e4,pfVar14,(int)pfVar22,fVar13,uVar29);
            uVar29 = 0;
            iVar9 = _rand();
            FUN_004795c0(0x47e,pfVar10,0x3f800000,pfVar15,(int)param_1,(float)(iVar9 % 0x168),uVar29);
            if ((param_1[1] == 1.4013e-45) &&
               (fVar17 = FUN_004f7500(param_1[4], param_1[5]), (float10)param_1[6] < fVar17)) {
              local_354 = *pfVar10;
              local_350 = param_1[5];
              local_34c = param_1[6] + _DAT_00552598;
              iVar9 = 0;
              local_368 = 1.0;
              local_364 = 1.0;
              local_360 = 1.0;
              uVar8 = _rand();
              uVar8 = uVar8 & 0x8000001f;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
              }
              FUN_00475220(0x4c4,&local_354,param_1 + 7,&local_368,0xb,
                           (float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar9);
            }
          }
          break;
        case 0x4fa:
          if (*(char *)*(int*)&param_1[0x3f] == '\0') {
            param_1[0x18] = 0.0;
            goto LAB_0046a85e;
          }
          switch(*(int*)&param_1[1]) {
          case 0:   // 0.0f
            if ((*(byte*)&param_1[0x1e] & 0x40) == 0x40) {
              param_1[0x18] = 1.4013e-44f;
            }
          case 1:   // 1.4013e-45f
          case 3:   // 4.2039e-45f
            local_35c = (float *)0x0;
            do {
              pfVar10 = local_35c;
              fVar13 = param_1[1];
              fVar17 = (float10)(int)local_35c * (float10)_DAT_005529a0 +
                       (float10)(*(int*)&param_1[0x18]) * (float10)_DAT_005529a4;
              if (fVar13 == 4.2039e-45f) {
                fVar17 = -fVar17;
              }
              fVar4 = param_1[0x3f];
              fVar18 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552998 +
                                     (float10)(int)local_35c * (float10)_DAT_0055299c);
              local_35c = (float *)(int)(float)(fVar18 * (float10)_DAT_005524fc + (float10)_DAT_00552598);
              fVar18 = (float10)fsin(fVar17);
              local_260 = (float)(fVar18 * (float10)(int)local_35c +
                                 (float10)*(float *)((int)fVar4 + 0x10));
              fVar17 = (float10)fcos(fVar17);
              local_25c = (float)(fVar17 * (float10)(int)local_35c +
                                 (float10)*(float *)((int)fVar4 + 0x14));
              fVar5 = _DAT_0055285c;
              if (fVar13 == 4.2039e-45f) {
                fVar5 = _DAT_00552994;
              }
              local_258 = fVar5 * *(float *)((int)fVar4 + 0xc) + *(float *)((int)fVar4 + 0x18);
              local_1a4[0] = 0.65f;
              local_1a4[1] = 0.65f;
              local_1a4[2] = 0.65f;
              local_1a4[3] = 0.1f;
              local_1a4[4] = 1.0f;
              local_1a4[5] = 1.0f;
              FUN_00475220(0x4b0,&local_260,param_1 + 7,local_1a4 + (uint)(fVar13 == 4.2039e-45f) * 3,
                           5,0.4f,0);
              FUN_004795c0(0x4fa,&local_260,0x3f800000,&local_368,*(int*)&param_1[0x3f],0,0);
              local_35c = (float *)((int)pfVar10 + 1);
            } while ((int)local_35c < 3);
            break;
          case 2:   // 2.8026e-45f
            local_1f4 = (float)(int)fVar13 * _DAT_005524f4;
            local_1f0 = local_1f4;
            local_1ec = local_1f4;
            FUN_004795c0(0x4fa,param_1 + 4,0x3fc00000,&local_1f4,*(int*)&param_1[0x3f],0,0);
          }
          break;
        case 0x566:
          local_368 = local_36c * _DAT_00552504;
          pfVar10 = param_1 + 4;
          local_360 = local_36c * _DAT_00552530;
          local_364 = local_368;
          FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
          local_334 = param_1[7];
          local_330 = param_1[8];
          local_32c = param_1[9] + _DAT_00552848;
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          iVar9 = _rand();
          param_1[3] = (float)(0xf - (*(int*)&param_1[0x18])) * _DAT_00552a10 * (float)(iVar9 % 3 + 2);
          FUN_004f9db0(&local_334,local_1a4 + 6);
          local_31c = 0.0;
          local_318 = 0x41a00000;
          local_314 = 0;
          FUN_004fa0b0(&local_31c,local_1a4 + 6,&local_354);
          pfVar15 = param_1 + 0x5c;
          *pfVar15 = local_354 + *pfVar10;
          param_1[0x5d] = local_350 + param_1[5];
          param_1[0x5e] = local_34c + param_1[6];
          FUN_00475220(0x4ad,pfVar15,param_1 + 7,&local_368,10,param_1[3],0);
          local_31c = 0.0;
          local_318 = 0xc1a00000;
          local_314 = 0;
          FUN_004fa0b0(&local_31c,local_1a4 + 6,&local_354);
          *pfVar15 = local_354 + *pfVar10;
          param_1[0x5d] = local_350 + param_1[5];
          param_1[0x5e] = local_34c + param_1[6];
          FUN_00475220(0x4ad,pfVar15,param_1 + 7,&local_368,10,param_1[3],0);
          if (param_1[0x18] == 1.4013e-45) {
            FUN_004661f0((undefined4 *)pfVar10,'\x01');
          }
          break;
        case 0x596:
          pcVar11 = (char *)*(int*)&param_1[0x3f];
          if ((*pcVar11 == '\0') || (((byte)*(undefined4 *)(pcVar11 + 0x78) & 2) != 2)) {
LAB_0046a366:
            param_1[0x18] = 0.0;
          }
          else if ((int)fVar13 < 0x14) {
            param_1[0x18] = 4.2039e-44;
          }
          else {
            fVar4 = *(float *)(pcVar11 + 0x15c);
            fVar5 = *(float *)(pcVar11 + 0x18);
            param_1[4] = *(float *)(pcVar11 + 0x10);
            param_1[5] = *(float *)(pcVar11 + 0x14);
            param_1[6] = fVar4 + fVar5 + _DAT_00552878;
            FUN_00475220(0x596,param_1 + 4,param_1 + 7,param_1 + 0x3a,(0x1e - (int)fVar13) / 2,1.0,0);
          }
        }
      }
      goto switchD_00466b93_caseD_c1;
    }
    if (iVar9 == 0x1f0) {
      if (param_1[1] == 1.4013e-45) {
        FUN_0046d840(0x4e1,param_1 + 4,param_1 + 4,param_1 + 7,0xc,(int)param_1,100.0,-1,0);
      }
      else if (param_1[1] == 0.0) {
        FUN_0046d840(0x4e1,param_1 + 4,param_1 + 4,param_1 + 7,4,(int)param_1,50.0,-1,0);
      }
      goto switchD_00466b93_caseD_c1;
    }
// SECTION_3
    switch(iVar9) {
    case 0xbe:
      fVar13 = param_1[1];
      if (fVar13 == 0.0) {
        if (_DAT_00552660 <= param_1[0x42]) {
          param_1[0x33] = 0.0;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000001;
          bVar16 = uVar8 == 0;
          if ((int)uVar8 < 0) {
            bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
          }
          if (bVar16) {
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000003f;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
            }
            local_1d0 = (float)(int)(uVar8 - 0x20) + param_1[4];
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000003f;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
            }
            local_1cc = (float)(int)(uVar8 - 0x20) + param_1[5];
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000007f;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xffffff80) + 1;
            }
            local_1c8 = (float)(int)(uVar8 + 0x20) + param_1[6];
            FUN_00475220(0x4c4,&local_1d0,param_1 + 7,param_1 + 0x3a,0,1.0,0);
          }
          fVar13 = param_1[0x5a] - _DAT_00552874;
          param_1[0x5a] = fVar13;
          if (fVar13 < _DAT_00552580) {
            *(undefined1 *)param_1 = 0;
          }
        }
        local_368 = local_36c * _DAT_00552990;
        local_364 = local_36c * _DAT_005529c0;
        local_360 = local_36c * _DAT_00552570;
        goto LAB_004695b5;
      }
      if (((int)fVar13 < 1) || (2 < (int)fVar13)) break;
      pcVar11 = (char *)*(int*)&param_1[0x3f];
      if (pcVar11 == (char *)0x0) goto LAB_0046a85e;
      if ((*pcVar11 == '\0') || (((byte)*(undefined4 *)(pcVar11 + 0x78) & 0x20) != 0x20)) {
        param_1[0x5a] = param_1[0x5a] - _DAT_00552874;
      }
      else {
        bVar28 = pcVar11[0x105];
        if (*(short *)(pcVar11 + 2) == 0x186) {
          if (((0xc < bVar28) && (bVar28 < 0x22)) || ((0x4b < bVar28 && (bVar28 < 0x4e)))) {
            param_1[0x42] = 0.0;
            param_1[0x43] = 0.0;
            goto LAB_00469366;
          }
LAB_0046934f:
          param_1[0x16] = -NAN;
        }
        else {
          if (bVar28 != 2) goto LAB_0046934f;
          param_1[0x43] = 0.0;
          param_1[0x42] = 0.0;
LAB_00469366:
          param_1[0x16] = -NAN;
        }
        bVar16 = _DAT_00552650 <= param_1[0x42];
        param_1[4] = *(float *)(pcVar11 + 0x10);
        fVar4 = *(float *)(pcVar11 + 0x18);
        param_1[5] = *(float *)(pcVar11 + 0x14);
        param_1[6] = fVar4;
        if (bVar16) {
          param_1[0x42] = 4.0;
        }
        param_1[0x18] = 1.4013e-44;
      }
      if (fVar13 == 1.4013e-45) {
        local_31c = 60.0;
        local_318 = 0;
        param_1[0xc] = param_1[0xc] + _DAT_005524fc;
        local_314 = 0;
        fVar17 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524f8);
        param_1[0x36] = (float)(fVar17 * (float10)_DAT_005524fc + (float10)_DAT_0055284c);
        FUN_004f9db0(param_1 + 10,local_1a4 + 6);
        FUN_004fa0b0(&local_31c,local_1a4 + 6,&local_354);
        fVar13 = param_1[0x3f];
        local_354 = local_354 + *(float *)((int)fVar13 + 0x10);
        local_350 = local_350 + *(float *)((int)fVar13 + 0x14);
        local_368 = 1.0;
        local_34c = local_34c + *(float *)((int)fVar13 + 0x18) + param_1[0x36];
        local_364 = 1.0;
        local_360 = 1.0;
        FUN_00475220(0x4ad,&local_354,param_1 + 7,&local_368,10,2.0,0);
      }
      break;
    case 0xbf:
      fVar13 = param_1[1];
      if ((fVar13 != 0.0) && (fVar13 != 2.8026e-45)) {
        if (fVar13 == 5.60519e-45) goto LAB_00469772;
        if (fVar13 == 8.40779e-45) goto LAB_00469769;
        if (fVar13 == 7.00649e-45) {
          param_1[6] = param_1[6] + param_1[0x36];
          param_1[0x36] = param_1[0x36] - _DAT_0055264c;
          fVar17 = FUN_004f7500(param_1[4], param_1[5]);
          local_348 = (float)fVar17;
          if (param_1[6] < local_348) {
            fVar13 = param_1[6] + _DAT_00552488;
            param_1[0x36] = 10.0;
            param_1[6] = fVar13;
            param_1[0x31] = param_1[0x31] * _DAT_00552504;
            param_1[3] = param_1[3] * _DAT_005529b4;
          }
          lVar19 = (longlong)(param_1[4] * 0.0099999998f);   // IDA Terrain_Load((__int64)(x*0.01),...)
          lVar20 = (longlong)(param_1[5] * 0.0099999998f);   // IDA Terrain_Load(...,(__int64)(y*0.01))
          iVar9 = FUN_004f6c40((uint)lVar19,(uint)lVar20);
          if ((param_1[0x18] == 1.4013e-45) || (((DAT_0838bc70)[iVar9] & 8) == 8)) {
            local_224 = param_1[4];
            local_21c = local_348 + _DAT_00552878;
            param_1[6] = local_348;
            local_220 = param_1[5];
            param_1[0x30] = 0.0;
            param_1[0x31] = 0.0;
            param_1[0x32] = 0.0;
            FUN_00475220(0x4bf,&local_224,param_1 + 7,&local_368,0,1.0,0);
            iVar9 = 6;
            do {
              bVar28 = 0;
              pfVar25 = (float *)0x0;
              pfVar23 = (float *)0xffffffff;
              pfVar21 = (float *)0x0;
              pfVar10 = param_1 + 0x3a;
              pfVar22 = (float *)0x0;
              pfVar15 = param_1 + 4;
              pfVar14 = param_1 + 7;
              uVar8 = _rand();
              uVar8 = uVar8 & 0x80000001;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
              }
              FUN_00460dc0(uVar8 + 0xc5,pfVar15,pfVar14,pfVar10,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
              iVar9 = iVar9 + -1;
            } while (iVar9 != 0);
            *(undefined1 *)param_1 = 0;
          }
        }
        goto LAB_00469a2f;
      }
LAB_00469769:
      if (fVar13 == 5.60519e-45) {
LAB_00469772:
        lVar19 = (longlong)(param_1[4] * 0.0099999998f);   // IDA Terrain_Load((__int64)(x*0.01),...)
        lVar20 = (longlong)(param_1[5] * 0.0099999998f);   // IDA Terrain_Load(...,(__int64)(y*0.01))
        iVar9 = FUN_004f6c40((uint)lVar19,(uint)lVar20);
        if (((DAT_0838bc70)[iVar9] & 8) == 8) {
          if (param_1[6] < _DAT_005529b0) {
            fVar17 = (float10)param_1[6];
            goto LAB_004698ca;
          }
        }
        else {
          fVar17 = FUN_004f7500(param_1[4], param_1[5]);
          local_344 = (float)fVar17;
          if (param_1[6] < local_344) {
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000003;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
            }
            iVar9 = 0;
            _DAT_083a0210 = (float)(int)(uVar8 - 4) * _DAT_005524f4;
            do {
              local_348 = param_1[4] - *(float *)(iVar9 + 0x10 + DAT_07abf5d0);
              pcVar11 = (char *)(iVar9 + DAT_07abf5d0);
              local_35c = (float *)(int)(param_1[5] - *(float *)(pcVar11 + 0x14));
              if ((((*pcVar11 != '\0') && (pcVar11[0x160] != '\0')) && (pcVar11 != DAT_07abf5d8)) &&
                 ((pcVar11[0x2fd] == '\0' &&
                  (SQRT(local_348 * local_348 + (float)(int)local_35c * (float)(int)local_35c) <=
                   _DAT_0055285c)))) {
                if (*(short *)(pcVar11 + 2) == 0x186) {
                  uVar8 = 0x82;
                }
                else {
                  uVar8 = 5;
                }
                FUN_0043e820((int)pcVar11,uVar8);
              }
              iVar9 = iVar9 + 0x394;
            } while (iVar9 < 0x59740);
            fVar17 = (float10)local_344;
            goto LAB_004698ca;
          }
        }
      }
      else {
        fVar17 = FUN_004f7500(param_1[4], param_1[5]);
        if (fVar17 <= (float10)param_1[6]) goto LAB_00469a2f;
LAB_004698ca:
        pfVar15 = param_1 + 4;
        local_2b8 = *pfVar15;
        local_2b4 = param_1[5];
        param_1[6] = (float)fVar17;
        fVar18 = (float10)_DAT_00552878;
        param_1[0x30] = 0.0;
        local_2b0 = (float)(fVar17 + fVar18);
        param_1[0x31] = 0.0;
        param_1[0x32] = 0.0;
        pfVar10 = param_1 + 7;
        FUN_00475220(0x4bf,&local_2b8,pfVar10,&local_368,0,1.0,0);
        iVar9 = 6;
        do {
          bVar28 = 0;
          pfVar26 = (float *)0x0;
          pfVar27 = (float *)0xffffffff;
          pfVar25 = (float *)0x0;
          pfVar14 = param_1 + 0x3a;
          pfVar23 = (float *)0x0;
          pfVar22 = pfVar15;
          pfVar21 = pfVar10;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000001;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
          }
          FUN_00460dc0(uVar8 + 0xc5,pfVar22,pfVar21,pfVar14,pfVar23,pfVar25,pfVar27,pfVar26,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        if (param_1[1] == 8.40779e-45) {
          iVar9 = 0x1e;
          do {
            iVar12 = _rand();
            local_2b8 = (float)(iVar12 % 0xa0 + -0x50) + *pfVar15;
            iVar12 = _rand();
            iVar24 = 0;
            local_368 = 0.2;
            local_364 = 1.0;
            local_360 = 0.2;
            local_2b4 = (float)(iVar12 % 0xa0 + -100) + param_1[5];
            local_2b0 = param_1[6] + _DAT_00552598;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000001f;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
            }
            FUN_00475220(0x4c4,&local_2b8,pfVar10,&local_368,0xb,
                         (float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar24);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }
        *(undefined1 *)param_1 = 0;
      }
LAB_00469a2f:
      fVar13 = param_1[1];
      if (fVar13 == 7.00649e-45) {
        local_364 = local_36c * _DAT_005524f4;
        local_368 = local_36c;
        param_1[0x16] = 0.0;
        param_1[0x1a] = 0.0;
        local_360 = 0.0;
        FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
        iVar9 = _rand();
        if (iVar9 % 5 != 0) {
          FUN_00475220(0x567,param_1 + 4,param_1 + 7,&local_368,0,1.0,0);
        }
      }
      else {
        if (fVar13 == 4.2039e-45) {
          param_1[0x16] = 0.0;
LAB_00469ab9:
          param_1[0x1a] = 0.0;
        }
        else if (fVar13 == 0.0) {
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000003;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
          }
          param_1[0x1a] = (float)(int)(uVar8 + 4) * _DAT_005524f4;
        }
        else {
          if (fVar13 != 8.40779e-45) goto LAB_00469ab9;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000003;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
          }
          param_1[0x1a] = (float)(int)(uVar8 + 4) * _DAT_005524f4;
        }
        if (param_1[1] == 8.40779e-45) {
          local_368 = local_36c * _DAT_005524f4;
          local_364 = local_36c;
        }
        else {
          local_364 = local_36c * _DAT_005524f4;
          local_368 = local_36c;
        }
        pfVar10 = param_1 + 4;
        local_360 = 0.0;
        FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
        if (param_1[1] == 8.40779e-45) {
          FUN_00475220(0x4ab,pfVar10,param_1 + 7,&local_368,5,1.0,0);
          FUN_00475220(0x4ab,pfVar10,param_1 + 7,&local_368,0,1.0,0);
        }
        else if (param_1[1] == 4.2039e-45) {
          FUN_00473d90(param_2 * 0x14fb,&local_2f8,1.0);
          local_2f8 = local_2f8 * _DAT_00552598;
          local_2f4 = local_2f4 * _DAT_00552598;
          local_334 = 0.0;
          local_330 = 0.0;
          local_2f0 = local_2f0 * _DAT_00552598;
          local_32c = 0.0;
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          local_2ec = *pfVar10 + local_2f8;
          local_2e8 = param_1[5] + local_2f4;
          local_2e4 = param_1[6] + local_2f0;
          FUN_00475220(0x4ab,&local_2ec,&local_334,&local_368,8,param_1[3] * _DAT_00552950,0);
          uVar29 = 0;
          iVar9 = _rand();
          FUN_004795c0(0x4cf,&local_2ec,param_1[3] * _DAT_00552540,&local_368,0,
                       (float)(iVar9 % 0x168),uVar29);
        }
        else {
          FUN_00475220(0x4ab,pfVar10,param_1 + 7,&local_368,5,1.0,0);
        }
        if (param_1[1] == 1.4013e-45) {
          FUN_00465e60((int)param_1);
        }
      }
      break;
    case 0xc0:
      local_364 = local_36c;
      fVar13 = (float)(int)fVar13 * _DAT_005524f4;
      param_1[0x1a] = fVar13;
      param_1[0x5a] = fVar13;
      local_368 = local_36c * _DAT_005528b8;
      local_360 = local_36c * _DAT_00552534;
LAB_004695b5:
      iVar9 = 2;
      goto LAB_004695c0;
    case 0xc2:
    case 0x103:
      fVar13 = param_1[0x3f];
      fVar4 = *(float *)((int)fVar13 + 0x14);
      fVar5 = *(float *)((int)fVar13 + 0x18);
      param_1[4] = *(float *)((int)fVar13 + 0x10);
      param_1[5] = fVar4;
      param_1[6] = fVar5;
      break;
    case 0xc5:
    case 0xc6:
    case 0xd5:
    case 0xd6:
      if (param_1[1] != 7.00649e-45) goto switchD_00466b93_caseD_c7;
      pcVar11 = (char *)*(int*)&param_1[0x3f];
LAB_00468772:
      param_1[4] = *(float *)(pcVar11 + 0x10) + param_1[0x5c];
      param_1[5] = *(float *)(pcVar11 + 0x14) + param_1[0x5d];
      param_1[6] = *(float *)(pcVar11 + 0x18) + param_1[0x5e];
      break;
    case 0xc9:
      // IDA uses the DWORD at o+0x60 as an integer counter here
      // (SLODWORD(v356)); fVar13 only carries those same bits as a float.
      // Converting fVar13 numerically turns e.g. counter 40 into zero.
      iVar12 = *(int *)((char *)param_1 + 0x60);
      iVar9 = iVar12;
      if (0x1d < iVar12) {
        iVar9 = 0x28 - iVar12;
      }
      param_1[0x1a] = (float)iVar9 * _DAT_005524f4;
      param_1[0x1b] = (float)iVar12 * _DAT_005529d0;
      iVar9 = _rand();
      _DAT_083a0210 = (float)(iVar9 % 6 + -6) * _DAT_005524f4;
      uVar8 = _rand();
      uVar8 = uVar8 & 0x80000003;
      bVar16 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar16 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar16) {
        local_200 = 0.0;
        iVar9 = _rand();
        local_1f8 = 0;
        local_1e8 = 0.0;
        local_1e4 = 0;
        local_1fc = (float)(iVar9 % 300);
        iVar9 = _rand();
        local_1e0 = (float)(iVar9 % 0x168);
        FUN_004f9db0(&local_1e8,local_6c);
        FUN_004fa0b0(&local_200,local_6c,&local_2d0);
        local_2d0 = local_2d0 + param_1[4];
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 7;
        pfVar14 = &local_2d0;
        bVar28 = 0;
        pfVar25 = (float *)0x0;
        local_2cc = local_2cc + param_1[5];
        pfVar23 = (float *)0xffffffff;
        pfVar21 = (float *)0x0;
        pfVar22 = (float *)0x1;
        local_2c8 = local_2c8 + param_1[6];
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000001;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
        }
        FUN_00460dc0(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
      }
      local_364 = local_36c * _DAT_00552530;
      iVar9 = 4;
      local_360 = local_36c * _DAT_005526e4;
      goto LAB_00468736;
    case 0xca:
      fVar4 = param_1[0x3f];
      fVar6 = *(float *)((int)fVar4 + 0x18) + _DAT_005524f0;
      fVar5 = *(float *)((int)fVar4 + 0x14);
      param_1[4] = *(float *)((int)fVar4 + 0x10);
      param_1[5] = fVar5;
      param_1[6] = fVar6;
      param_1[8] = param_1[8] + _DAT_005524fc;
    case 200:
      // LABEL_36 in IDA: scale = integer lifetime * 0.1.
      param_1[0x1a] = (float)*(int *)((char *)param_1 + 0x60) * _DAT_005524f4;
      break;
    case 0xcb:
      pfVar10 = param_1 + 7;
      pfVar15 = param_1 + 4;
      iVar9 = 4;
      param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4;
      param_1[0x1b] = (float)(int)fVar13 * _DAT_00552570;
      do {
        FUN_00475220(0x4c4,pfVar15,pfVar10,param_1 + 0x3a,3,1.0,0);
        iVar9 = iVar9 + -1;
      } while (iVar9 != 0);
      local_368 = local_36c * _DAT_005528b8;
      local_360 = local_36c;
      local_364 = local_36c * _DAT_00552534;
      FUN_004f76c0(*pfVar15,param_1[5],(int)&local_368,3,(int)DAT_081cb608);
      if (param_1[1] == 2.8026e-45) {
        local_334 = *pfVar10;
        local_330 = param_1[8];
        local_32c = param_1[9];
        local_354 = *pfVar15;
        local_350 = param_1[5];
        local_34c = param_1[6];
        param_1[0x16] = 0.0;
        iVar9 = _rand();
        iVar12 = 0;
        local_32c = (float)(iVar9 % 10 + -5) + local_32c;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        }
        FUN_00475220(0x4c4,pfVar15,pfVar10,&local_368,0xb,(float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar12);
        local_34c = local_34c + _DAT_00552598;
        local_26c = 0x3f800000;
        local_268 = 0x3f800000;
        local_264 = 0x3f800000;
        uVar29 = 0;
        iVar9 = _rand();
        FUN_004795c0(0x4cf,&local_354,0x3fc00000,(float*)&local_26c,0,(float)(iVar9 % 0x168),uVar29);
        uVar29 = 0;
        iVar9 = _rand();
        FUN_004795c0(0x4cf,&local_354,0x3fc00000,(float*)&local_26c,0,(float)(iVar9 % 0x168),uVar29);
        uVar29 = 0;
        iVar9 = _rand();
        FUN_004795c0(0x47e,&local_354,0x40600000,&local_368,0,(float)(iVar9 % 0x168),uVar29);
      }
      break;
    case 0xcc:
      pfVar14 = param_1 + 0x3a;
      pfVar10 = param_1 + 7;
      pfVar15 = param_1 + 4;
      param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4;
      param_1[0x1b] = (float)(int)fVar13 * _DAT_005529c8;
      FUN_00475220(0x4c4,pfVar15,pfVar10,pfVar14,3,1.0,0);
      local_32c = param_1[9];
      local_334 = 90.0;
      local_330 = 0.0;
      uVar8 = _rand();
      uVar8 = uVar8 & 0x80000001;
      bVar16 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
      }
      if (bVar16) {
        local_354 = *pfVar15 - _DAT_0055285c;
        local_350 = param_1[5];
        local_34c = param_1[6] + _DAT_005529c4;
        FUN_0046d840(0x4e6,&local_354,pfVar15,&local_334,0,(int)param_1,10.0,-1,0);
      }
      uVar8 = _rand();
      uVar8 = uVar8 & 0x80000001;
      bVar16 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
      }
      if (bVar16) {
        local_354 = *pfVar15 + _DAT_0055285c;
        local_350 = param_1[5];
        local_34c = param_1[6] + _DAT_005529c4;
        FUN_0046d840(0x4e6,&local_354,pfVar15,&local_334,0,(int)param_1,10.0,-1,0);
      }
      fVar17 = FUN_004f7500(param_1[4], param_1[5]);
      param_1[6] = (float)fVar17;
      uVar8 = _rand();
      uVar8 = uVar8 & 0x80000003;
      bVar16 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar16 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar16) {
        bVar28 = 0;
        pfVar27 = (float *)0x0;
        pfVar25 = (float *)0xffffffff;
        pfVar23 = (float *)0x0;
        pfVar21 = (float *)0x2;
        pfVar22 = pfVar15;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000001;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
        }
        FUN_00460dc0(uVar8 + 0xc5,pfVar22,pfVar10,pfVar14,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
      }
      local_368 = local_36c * _DAT_00552990;
      local_364 = local_36c * _DAT_005529c0;
      local_360 = local_36c * _DAT_00552570;
      FUN_004f76c0(*pfVar15,param_1[5],(int)&local_368,5,(int)DAT_081cb608);
      if (((*(int*)&param_1[0x18]) % 0xf == 0) && ((char *)*(int*)&param_1[0x3f] == DAT_07abf5d8)) {
        FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),pfVar15,150.0,*(byte *)(param_1 + 0x22),
                     *(short *)((int)param_1 + 0x86));
      }
      break;
    case 0xd1:
      param_1[9] = param_1[9] - _DAT_0055284c;
      break;
    case 0xd2:
    case 0xd3:
    case 0xe2:
    case 0xe3:
      if (param_1[1] == 7.00649e-45) {
        pcVar11 = (char *)*(int*)&param_1[0x3f];
        if (*pcVar11 == '\0') {
          *(undefined1 *)param_1 = 0;
        }
        goto LAB_00468772;
      }
    case 199:
    case 0xe7:
    case 0xe8:
    case 0xed:
switchD_00466b93_caseD_c7:
      if (param_1[1] == 0.0) {
        param_1[4] = param_1[0x30] + param_1[4];
        param_1[5] = param_1[0x31] + param_1[5];
        fVar13 = param_1[0x32];
      }
      else {
        FUN_004f9db0(param_1 + 7,local_1a4 + 6);
        FUN_004fa0b0(param_1 + 0x30,local_1a4 + 6,&local_354);
        param_1[4] = local_354 + param_1[4];
        param_1[5] = local_350 + param_1[5];
        fVar13 = local_34c;
      }
      param_1[6] = fVar13 + param_1[6];
      param_1[0x30] = param_1[0x30] * _DAT_005526e8;
      param_1[0x31] = param_1[0x31] * _DAT_005526e8;
      param_1[0x32] = param_1[0x32] * _DAT_005526e8;
      param_1[6] = param_1[6] + param_1[0x36];
      if (param_1[1] == 0.0) {
        param_1[0x36] = param_1[0x36] - _DAT_00552540;
        fVar17 = FUN_004f7500(param_1[4], param_1[5]);
        if (fVar17 <= (float10)param_1[6]) {
          fVar13 = param_1[3] * _DAT_005529b8;
        }
        else {
          param_1[6] = (float)fVar17;
          fVar13 = param_1[0x36] * _DAT_00552a14;
          *(int*)&param_1[0x18] = *(int*)&param_1[0x18] + -4;   // counter DWORD
          param_1[0x36] = fVar13;
          fVar13 = param_1[3] * _DAT_005529bc;
        }
        pfVar10 = param_1 + 7;
        *pfVar10 = *pfVar10 - fVar13;
        if ((param_1[1] == 0.0) && (iVar9 = _rand(), iVar9 % 10 == 0)) {
          sVar3 = *(short *)((int)param_1 + 2);
          if ((sVar3 == 199) || ((sVar3 == 0xe7 || (sVar3 == 0xe8)))) {
            FUN_00475220(0x4c4,param_1 + 4,pfVar10,param_1 + 0x3a,0,1.0,0);
          }
          else if ((sVar3 == 0xc5) || (sVar3 == 0xc6)) {
            iVar12 = 0;
            fVar13 = 1.0;
            iVar9 = _rand();
            FUN_00475220(0x4ab,param_1 + 4,pfVar10,param_1 + 0x3a,iVar9 % 3 + 1,fVar13,iVar12);
          }
        }
      }
      else {
        if (param_1[1] != 1.4013e-45) {
          param_1[9] = param_1[9] + _DAT_005524fc;
        }
        param_1[0x36] = param_1[0x36] + _DAT_00552504;
      }
      break;
    case 0xd4:
      param_1[0x32] = param_1[0x32] - _DAT_00552950;
      FUN_00465e60((int)param_1);
      break;
    case 0xd7:
      param_1[6] = param_1[6] + param_1[0x36];
      param_1[0x36] = param_1[0x36] - _DAT_0055256c;
      fVar17 = FUN_004f7500(param_1[4], param_1[5]);
      if ((float10)param_1[6] < fVar17) {
        param_1[6] = (float)fVar17;
        fVar13 = param_1[0x36] * _DAT_00552990;
        *(int*)&param_1[0x18] = *(int*)&param_1[0x18] + -4;   // counter DWORD
        param_1[0x36] = fVar13;
        param_1[0x31] = param_1[0x31] - _DAT_0055264c;
      }
      break;
    case 0xd8:
      local_368 = local_36c * _DAT_00552530;
      local_364 = local_36c * _DAT_00552504;
      local_360 = local_36c * _DAT_005526e4;
      FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
      pfVar10 = param_1 + 7;
      FUN_00475220(0x4ab,param_1 + 4,pfVar10,&local_368,0,1.0,0);
      FUN_00466440((int)param_1);
      if (param_1[1] == 4.2039e-45) {
        fVar13 = *pfVar10 + _DAT_00552488;
        *pfVar10 = fVar13;
        if (_DAT_00552844 < fVar13) {
          *pfVar10 = 45.0;
        }
        lVar19 = (longlong)(param_1[4] * 0.0099999998f);   // IDA Terrain_Load((__int64)(x*0.01),...)
        lVar20 = (longlong)(param_1[5] * 0.0099999998f);   // IDA Terrain_Load(...,(__int64)(y*0.01))
        iVar9 = FUN_004f6c40((uint)lVar19,(uint)lVar20);
        if ((((DAT_0838bc70)[iVar9] & 8) != 8) &&
           (fVar17 = FUN_004f7500(param_1[4], param_1[5]), (float10)param_1[6] < fVar17)) {
          fVar18 = (float10)_DAT_00552488;
          param_1[0x30] = 0.0;
          param_1[0x31] = 0.0;
          param_1[0x32] = 0.0;
          param_1[6] = (float)(fVar17 + fVar18);
        }
      }
      break;
    case 0xd9:
    case 0xda:
    case 0xdb:
    case 0xdc:
    case 0xdd:
    case 0xde:
    case 0xdf:
    case 0xe0:
      if (sVar3 == 0xde) {
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (__int64)WorldTime * 0.1
        FUN_004795c0(0x4a7,pfVar10,0x3f000000,pfVar15,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(__int64)WorldTime * 0.1
        FUN_004795c0(0x4a7,pfVar10,0x3f800000,pfVar15,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        iVar9 = 4;
        do {
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000001f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
          }
          local_354 = (float)(int)(uVar8 - 0x10);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000003f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
          }
          local_350 = (float)(int)(uVar8 - 0x20);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000001f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
          }
          iVar30 = 0;
          fVar13 = 1.0;
          local_354 = local_354 + *pfVar10;
          iVar24 = 0;
          pfVar14 = param_1 + 7;
          pfVar22 = &local_354;
          local_350 = local_350 + param_1[5];
          local_34c = (float)(int)(uVar8 - 0x10) + param_1[6];
          pfVar21 = pfVar15;
          iVar12 = _rand();
          FUN_00475220(iVar12 % 3 + 0x4da,pfVar22,pfVar14,pfVar21,iVar24,fVar13,iVar30);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        param_1[8] = param_1[8] + _DAT_0055284c;
      }
      else if (sVar3 == 0xe0) {
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (__int64)WorldTime * 0.1
        FUN_004795c0(0x4a7,pfVar10,0x3f000000,pfVar15,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(__int64)WorldTime * 0.1
        FUN_004795c0(0x4a7,pfVar10,0x3f800000,pfVar15,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        local_35c = (float *)0x4;
        do {
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_354 = (float)(int)(uVar8 - 8);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_350 = (float)(int)(uVar8 - 8);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_354 = local_354 + *pfVar10;
          local_350 = local_350 + param_1[5];
          local_34c = (float)(int)(uVar8 - 8) + param_1[6];
          FUN_00475220(0x4d9,&local_354,param_1 + 7,pfVar15,1,1.0,0);
          local_35c = (float *)((int)local_35c + -1);
        } while (local_35c != (float *)0x0);
        FUN_00475220(0x4c4,pfVar10,param_1 + 7,pfVar15,0,1.0,0);
      }
      else if (sVar3 == 0xdf) {
        local_368 = 1.0;
        local_364 = 0.6;
        local_360 = 0.4;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (__int64)WorldTime * 0.1
        FUN_004795c0(0x47e,pfVar10,0x3f800000,&local_368,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(__int64)WorldTime * 0.1
        FUN_004795c0(0x47e,pfVar10,0x40000000,&local_368,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        iVar9 = 4;
        do {
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_354 = (float)(int)(uVar8 - 8);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_350 = (float)(int)(uVar8 - 8);
          uVar8 = _rand();
          uVar8 = uVar8 & 0x8000000f;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
          }
          local_354 = local_354 + *pfVar10;
          local_350 = local_350 + param_1[5];
          local_34c = (float)(int)(uVar8 - 8) + param_1[6];
          FUN_00475220(0x4d9,&local_354,param_1 + 7,param_1 + 0x3a,1,1.0,0);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        param_1[6] = param_1[6] + param_1[0x36];
        param_1[0x36] = param_1[0x36] - _DAT_0055256c;
        fVar17 = FUN_004f7500(param_1[4], param_1[5]);
        if ((float10)param_1[6] < fVar17) {
          param_1[6] = (float)fVar17;
          param_1[0x36] = param_1[0x36] * _DAT_00552990;
          param_1[0x31] = param_1[0x31] * _DAT_005528b4;
        }
        if ((param_1[0x18] == 1.4013e-45) &&
           (FUN_004660f0(pfVar10,'\x01'), (char *)*(int*)&param_1[0x3f] == DAT_07abf5d8)) {
          FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),pfVar10,100.0,*(byte *)(param_1 + 0x22),
                       *(short *)((int)param_1 + 0x86));
        }
      }
      local_368 = local_36c * _DAT_00552534;
      local_364 = local_36c * _DAT_00552530;
      local_360 = local_364;
      FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,2,(int)DAT_081cb608);
      FUN_00466440((int)param_1);
      break;
    case 0xe1:
      pcVar11 = (char *)*(int*)&param_1[0x3f];
      param_1[0x1a] = 1.3;
      fVar13 = *(float *)(pcVar11 + 0x14);
      fVar4 = param_1[9] + _DAT_00552488;
      param_1[4] = *(float *)(pcVar11 + 0x10);
      param_1[6] = *(float *)(pcVar11 + 0x18);
      cVar2 = *pcVar11;
      param_1[9] = fVar4;
      param_1[5] = fVar13;
      if ((cVar2 == '\0') || (pcVar11[0x160] == '\0')) {
        *(undefined1 *)param_1 = 0;
      }
      break;
    case 0xe4:
    case 0xe5:
    case 0xe6:
      pfVar10 = param_1 + 4;
      lVar19 = (longlong)param_1[5];   // IDA Terrain_Load(x/100, y/100): este es y
      uVar8 = (int)lVar19 / 100;
      lVar19 = (longlong)param_1[4];   // IDA Terrain_Load(x/100, y/100): este es x
      iVar9 = FUN_004f6c40((int)lVar19 / 100,uVar8);
      if ((((DAT_0838bc70)[iVar9] & 8) != 8) &&
         (fVar17 = FUN_004f7500(param_1[4], param_1[5]), (float10)param_1[6] < fVar17)) {
        param_1[6] = (float)fVar17;
        *(undefined1 *)param_1 = 0;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000007;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
        }
        iVar9 = 5;
        _DAT_083a0210 = (float)(int)(uVar8 - 8) * _DAT_005524f4;
        do {
          bVar28 = 0;
          pfVar27 = (float *)0x0;
          pfVar25 = (float *)0xffffffff;
          pfVar23 = (float *)0x0;
          pfVar21 = (float *)0x0;
          pfVar15 = param_1 + 7;
          pfVar14 = pfVar10;
          pfVar22 = param_1 + 0x3a;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000001;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
          }
          FUN_00460dc0(uVar8 + 0xe7,pfVar14,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
      }
      pfVar15 = param_1 + 7;
      *pfVar10 = param_1[0x30] + *pfVar10;
      param_1[5] = param_1[0x31] + param_1[5];
      param_1[6] = param_1[0x32] + param_1[6];
      *pfVar15 = _DAT_00552488 / param_1[3] + *pfVar15;
      FUN_00475220(0x4c4,pfVar10,pfVar15,param_1 + 0x3a,3,1.0,0);
      return;
    case 0xe9:
      if (param_1[1] == 1.4013e-45) {
        local_368 = local_36c;
        param_1[3] = param_1[3] + _DAT_005524f8;
        local_364 = 0.0;
        local_360 = 0.0;
        FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,3,(int)DAT_081cb608);
        param_1[0x1a] = (float)(*(int*)&param_1[0x18]) * _DAT_00552a00;
      }
      else {
        fVar13 = param_1[0x3f];
        fVar4 = *(float *)((int)fVar13 + 0x10);
        fVar5 = *(float *)((int)fVar13 + 0x14);
        param_1[3] = param_1[3] + _DAT_005524f8;
        fVar13 = *(float *)((int)fVar13 + 0x18);
        param_1[4] = fVar4;
        param_1[5] = fVar5;
        param_1[6] = fVar13;
        param_1[0x1a] = (float)(*(int*)&param_1[0x18]) * _DAT_00552a00;
      }
      break;
    case 0xee:
      FUN_00460dc0(0xef,param_1 + 4,param_1 + 7,param_1 + 0x3a,(float *)(4 - (int)fVar13),
                   (float *)*(int*)&param_1[0x3f],(float *)(uint)*(ushort *)((int)param_1 + 0x86),
                   (float *)CONCAT31((int3)(CONCAT22(extraout_var,sVar3) >> 8),
                                     *(undefined1 *)((int)param_1 + 0x85)),*(byte *)(param_1 + 0x21));
      break;
    case 0xef:
      switch(*(int*)&param_1[1]) {
      case 1:   // 1.4013e-45f
        param_1[0x5a] = 0.6f;
        break;
      case 2:   // 2.8026e-45f
        param_1[0x5a] = 0.5f;
        break;
      case 3:   // 4.2039e-45f
        param_1[0x5a] = 0.4f;
        break;
      case 4:   // 5.60519e-45f
        param_1[0x5a] = 0.3f;
      }
      if ((*(byte *)(*(int*)&param_1[0x3f] + 0x88) < 0x60) ||
         (0x7f < *(byte *)(*(int*)&param_1[0x3f] + 0x88))) {
        local_318 = 0xc3160000;
      }
      else {
        local_318 = 0xc3340000;
      }
      local_31c = 0.0;
      pfVar10 = param_1 + 7;
      local_314 = 0;
      FUN_004f9db0(pfVar10,local_1a4 + 6);
      FUN_004fa0b0(&local_31c,local_1a4 + 6,&local_354);
      fVar13 = param_1[0x3f];
      pfVar15 = param_1 + 4;
      *pfVar15 = local_354 + *(float *)((int)fVar13 + 0x10);
      param_1[5] = local_350 + *(float *)((int)fVar13 + 0x14);
      param_1[6] = local_34c + *(float *)((int)fVar13 + 0x18);
      param_1[9] = param_1[9] - _DAT_005529fc;
      FUN_00475220(0x4c4,pfVar15,pfVar10,param_1 + 0x3a,3,1.0,0);
      local_368 = local_36c * _DAT_005528b8;
      local_364 = local_368;
      local_360 = local_368;
      FUN_004f76c0(*pfVar15,param_1[5],(int)&local_368,3,(int)DAT_081cb608);
      local_368 = 1.0;
      local_364 = 1.0;
      local_360 = 1.0;
      iVar9 = 0;
      do {
        iVar12 = _rand();
        local_330 = 0.0;
        local_35c = (float *)(iVar12 % 0x3c + -0x1e);
        local_334 = (float)(int)local_35c;
        iVar12 = _rand();
        local_350 = param_1[5];
        local_34c = param_1[6];
        local_35c = (float *)(iVar12 % 0x1e + 0x5a);
        local_354 = *pfVar15;
        local_334 = local_334 + *pfVar10;
        local_330 = local_330 + param_1[8];
        local_32c = (float)(int)local_35c + param_1[9];
        iVar12 = _rand();
        local_35c = (float *)(iVar12 % 0x14 + -10);
        local_354 = (float)(int)local_35c + local_354;
        iVar12 = _rand();
        local_35c = (float *)(iVar12 % 0x14 + -10);
        local_350 = (float)(int)local_35c + local_350;
        FUN_0046d840(0x4e9,&local_354,&local_354,&local_334,0,0,10.0,-1,0);
        if (iVar9 == 0) {
          FUN_00475220(0x497,&local_354,&local_334,&local_368,0,1.0,0);
        }
        iVar9 = iVar9 + 1;
      } while (iVar9 < 4);
      local_354 = *pfVar15;
      local_350 = param_1[5];
      local_34c = param_1[6] + _DAT_005524fc;
      local_368 = 1.0;
      local_364 = 0.8;
      local_360 = 0.6;
      FUN_004795c0(0x47e,&local_354,0x40000000,&local_368,(int)param_1,0,0);
      if (param_1[1] == 0.0) {
        uVar8 = (*(uint*)&param_1[0x18]) & 0x80000007;
        bVar16 = uVar8 == 0;
        if ((int)uVar8 < 0) {
          bVar16 = (uVar8 - 1 | 0xfffffff8) == 0xffffffff;
        }
        if ((bVar16) && ((char *)*(int*)&param_1[0x3f] == DAT_07abf5d8)) {
          FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),pfVar15,150.0,*(byte *)(param_1 + 0x22),
                       *(short *)((int)param_1 + 0x86));
        }
      }
      break;
    case 0xf0:
      pfVar10 = param_1 + 4;
      fVar17 = FUN_004f7500(param_1[4], param_1[5]);
      if ((float10)param_1[6] < fVar17) {
        local_284 = *pfVar10;
        local_280 = param_1[5];
        param_1[6] = (float)fVar17;
        pfVar15 = param_1 + 7;
        local_27c = (float)(fVar17 + (float10)_DAT_00552878);
        param_1[0x30] = 0.0;
        param_1[0x31] = 0.0;
        param_1[0x32] = 0.0;
        FUN_00475220(0x4bf,&local_284,pfVar15,&local_368,0,1.0,0);
        FUN_00475220(0x4d2,&local_284,pfVar15,&local_368,0,1.0,0);
        iVar9 = 6;
        do {
          bVar28 = 0;
          pfVar26 = (float *)0x0;
          pfVar27 = (float *)0xffffffff;
          pfVar25 = (float *)0x0;
          pfVar14 = param_1 + 0x3a;
          pfVar23 = (float *)0x0;
          pfVar22 = pfVar10;
          pfVar21 = pfVar15;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000001;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
          }
          FUN_00460dc0(uVar8 + 0xc5,pfVar22,pfVar21,pfVar14,pfVar23,pfVar25,pfVar27,pfVar26,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        bVar16 = (char *)*(int*)&param_1[0x3f] == DAT_07abf5d8;
        *(undefined1 *)param_1 = 0;
        if (bVar16) {
          FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),pfVar10,150.0,*(byte *)(param_1 + 0x22),
                       *(short *)((int)param_1 + 0x86));
        }
      }
      local_344 = *pfVar10;
      param_1[0x10] = local_344;
      local_348 = param_1[5];
      param_1[0x12] = param_1[6];
      param_1[0x11] = local_348;
      local_368 = local_36c * _DAT_005526e4;
      local_360 = local_36c;
      local_364 = local_36c * _DAT_005528b4;
      FUN_004f76c0(local_344,local_348,(int)&local_368,2,(int)DAT_081cb608);
      break;
    case 0xf1:
      fVar4 = param_1[1];
      if (fVar4 == 2.8026e-45) {
        param_1[3] = param_1[3] + _DAT_00552a0c;
LAB_00466e5e:
        if (fVar4 == 5.60519e-45) break;
      }
      else {
        if (fVar4 == 4.2039e-45) {
          fVar5 = param_1[0x3f];
          param_1[5] = *(float *)((int)fVar5 + 0x14);
          iVar9 = *(int *)((int)fVar5 + 0xfc);
          param_1[6] = *(float *)((int)fVar5 + 0x18);
          param_1[4] = *(float *)(iVar9 + 0x10) + _DAT_0055297c;
          goto LAB_00466e5e;
        }
        if (fVar4 == 5.60519e-45) {
          fVar7 = param_1[3] + _DAT_00552a08;
          fVar5 = param_1[0x3f];
          fVar6 = *(float *)((int)fVar5 + 0x10);
          param_1[3] = fVar7;
          fVar7 = fVar7 * _DAT_00552a04;
          param_1[4] = fVar6;
          param_1[5] = *(float *)((int)fVar5 + 0x14);
          fVar7 = fVar7 + param_1[0x36];
          param_1[0x36] = fVar7;
          param_1[6] = fVar7 + *(float *)((int)fVar5 + 0x18);
          goto LAB_00466e5e;
        }
      }
      param_1[0x1a] = (float)(int)fVar13 * _DAT_00552a10;
      goto LAB_00466d90;
    case 0xf2:
      fVar13 = param_1[8] + _DAT_0055284c;
      param_1[0x12] = param_1[6];
      local_360 = local_36c;
      param_1[8] = fVar13;
      local_344 = param_1[4];
      param_1[0x10] = local_344;
      local_348 = param_1[5];
      param_1[0x11] = local_348;
      local_368 = local_36c * _DAT_005526e4;
      local_364 = local_36c * _DAT_005528b4;
      FUN_004f76c0(local_344,local_348,(int)&local_368,2,(int)DAT_081cb608);
      FUN_00466440((int)param_1);
      break;
    case 0xf3:
      local_360 = local_36c;
      param_1[8] = param_1[8] + _DAT_0055284c;
      local_368 = local_36c * _DAT_005526e4;
      local_364 = local_36c * _DAT_005528b4;
      FUN_004f76c0(param_1[0x5c],param_1[0x5d],(int)&local_368,2,(int)DAT_081cb608);
      if (param_1[1] != 0.0) {
        if (param_1[0x18] == 1.82169e-44) {
          FUN_00460dc0(0xff,param_1 + 4,param_1 + 7,param_1 + 0x3a,(float *)0x0,param_1,
                       (float *)0xffffffff,(float *)0x0,0);
        }
        else if (param_1[0x18] == 4.2039e-44) {
          param_1[0x3d] = 0.0;
          *(undefined1 *)(param_1 + 0x21) = 1;
        }
      }
      break;
    case 0xf4:
      local_340 = 0.0;
      local_33c = 0.0;
      local_338 = 0.0;
      local_310 = 0.0;
      local_30c = 0.0;
      local_308 = 0;
      if (fVar13 == 4.2039e-45) {
        local_20c = 1.0;
        local_208 = 0x3f800000;
        local_204 = 0x3f800000;
        local_340 = 0.0;
        local_33c = 0.0;
        local_338 = 0.0;
        local_310 = -25.0;
        local_30c = -80.0;
        local_308 = 0;
        FUN_004f9db0((float *)(*(int*)&param_1[0x3f] + 0x1c),local_15c);
        FUN_004fa0b0(&local_310,local_15c,&local_328);
        local_348 = local_328 + param_1[4];
        pfVar10 = param_1 + 0x5c;
        *pfVar10 = local_348;
        local_35c = (float *)(int)(local_324 + param_1[5]);
        param_1[0x5d] = (float)(int)local_35c;
        param_1[0x5e] = local_320 + param_1[6];
        fVar17 = FUN_004f7500(param_1[4], param_1[5]);
        param_1[0x5e] = (float)(fVar17 + (float10)_DAT_00552464);
        FUN_00475220(0x4bf,pfVar10,&local_340,&local_20c,0,0.5,0);
        iVar9 = 0;
        do {
          iVar12 = _rand();
          local_33c = 0.0;
          local_35c = (float *)(iVar12 % 0x3c);
          local_340 = (float)(int)local_35c - _DAT_0055290c;
          iVar12 = _rand();
          local_324 = param_1[0x5d];
          local_320 = param_1[0x5e];
          local_35c = (float *)(iVar12 % 0x1e + 0x5a);
          local_328 = *pfVar10;
          local_340 = local_340 + param_1[7];
          local_33c = local_33c + param_1[8];
          local_338 = (float)(int)local_35c + param_1[9];
          iVar12 = _rand();
          local_35c = (float *)(iVar12 % 0x14 + -10);
          local_328 = (float)(int)local_35c + local_328;
          iVar12 = _rand();
          local_35c = (float *)(iVar12 % 0x14 + -10);
          local_324 = (float)(int)local_35c + local_324;
          FUN_0046d840(0x4e9,&local_328,&local_328,&local_340,0,0,10.0,-1,0);
          if (iVar9 == 0) {
            FUN_00475220(0x497,&local_328,&local_340,&local_368,0,1.0,0);
          }
          iVar9 = iVar9 + 1;
        } while (iVar9 < 8);
        pfVar15 = param_1 + 0x3a;
        local_340 = 0.0;
        local_33c = 0.0;
        local_338 = 0.0;
        FUN_00460dc0(0xfd,pfVar10,&local_340,pfVar15,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
        param_1[0x5e] = param_1[0x5e] - _DAT_005524a8;
        FUN_00460dc0(0xf7,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        FUN_00460dc0(0xf5,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        FUN_00460dc0(0xf6,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        local_358 = 0.0;
        do {
          local_310 = 0.0;
          iVar9 = _rand();
          local_308 = 0;
          local_340 = 0.0;
          local_33c = 0.0;
          local_35c = (float *)(iVar9 % 0x96 + 100);
          local_30c = (float)(int)local_35c;
          local_338 = (float)(int)local_358 * _DAT_005529f8 + (float)(int)param_1[1];
          FUN_004f9db0(&local_340,local_15c);
          FUN_004fa0b0(&local_310,local_15c,&local_328);
          local_328 = local_328 + *pfVar10;
          local_324 = local_324 + param_1[0x5d];
          local_320 = local_320 + param_1[0x5e] + _DAT_00552540;
          iVar9 = _rand();
          local_340 = 0.0;
          local_33c = 0.0;
          pfVar14 = (float *)(iVar9 % 0x32 + 0x28);
          iVar9 = _rand();
          local_35c = (float *)(iVar9 % 0x1e + -0xf);
          local_338 = (float)(int)local_35c + _DAT_00552844;
          lVar19 = (longlong)(local_328 * 0.0099999998f);   // IDA TERRAIN_INDEX((__int64)(TargetPosition[0]*0.01),...)
          lVar20 = (longlong)(local_324 * 0.0099999998f);   // IDA TERRAIN_INDEX(...,(__int64)(TargetPosition[1]*0.01))
          iVar9 = FUN_004f6c30((int)lVar19,(int)lVar20);
          bVar28 = (DAT_0838bc70)[iVar9];
          if ((((bVar28 & 4) != 4) || ((bVar28 & 8) != 8)) || ((bVar28 & 0x10) != 0x10)) {
            fVar17 = FUN_004f7500(param_1[4], param_1[5]);
            local_320 = (float)(fVar17 + (float10)_DAT_00552540);
            FUN_00460dc0(0xf8,&local_328,&local_340,pfVar15,(float *)0x0,param_1,pfVar14,(float *)0x0,0);
            FUN_00460dc0(0xf9,&local_328,&local_340,pfVar15,(float *)0x0,param_1,pfVar14,(float *)0x0,0);
          }
          local_358 = (float)((int)local_358 + 1);
        } while ((int)local_358 < 5);
      }
      else if (fVar13 == 2.8026e-45) {
        pfVar10 = local_38;
        local_1b8 = 0.0;
        local_1b4 = 0;
        local_1b0 = 0;
        local_1ac = 0;
        local_1a8 = 0;
        iVar9 = 5;
        do {
          pfVar10[-1] = param_1[0x5c];
          *pfVar10 = param_1[0x5d];
          pfVar10[1] = param_1[0x5e];
          pfVar10 = pfVar10 + 3;
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        local_358 = 0.0;
        local_348 = 0.0;
        do {
          local_310 = 0.0;
          iVar9 = _rand();
          local_308 = 0;
          local_35c = (float *)(iVar9 % 0xf);
          local_30c = (float)(int)local_35c + _DAT_00552854;
          if (2 < (int)local_348) {
            local_358 = (float)_rand();
          }
          local_344 = 0.0;
          local_288 = (uint)local_358 & 0x80000001;
          if ((int)local_288 < 0) {
            local_288 = (local_288 - 1 | 0xfffffffe) + 1;
          }
          pfVar10 = &local_1b8;
          pfVar15 = local_38;
          do {
            local_35c = pfVar10;
            if (local_288 == 0) {
              iVar9 = _rand();
              local_36c = (float)(iVar9 % 0x1e + 0x32);
            }
            else {
              iVar9 = _rand();
              local_36c = (float)(-0x32 - iVar9 % 0x1e);
            }
            fVar13 = *pfVar10;
            *pfVar10 = (float)(int)local_36c + fVar13;
            iVar9 = _rand();
            local_338 = (float)((iVar9 % 10 + 0x3e) * (int)local_344) +
                        (float)(int)local_36c + fVar13;
            FUN_004f9db0(&local_340,local_15c);
            FUN_004fa0b0(&local_310,local_15c,&local_328);
            pfVar14 = pfVar15 + -1;
            *pfVar14 = local_328 + pfVar15[-1];
            *pfVar15 = local_324 + *pfVar15;
            pfVar15[1] = local_320 + pfVar15[1];
            lVar19 = (longlong)(pfVar15[-1] * 0.0099999998f);   // IDA TERRAIN_INDEX((__int64)(*(v74-1)*0.01),...)
            lVar20 = (longlong)(pfVar15[0] * 0.0099999998f);   // IDA TERRAIN_INDEX(...,(__int64)(*v74*0.01))
            iVar9 = FUN_004f6c30((int)lVar19,(int)lVar20);
            bVar28 = (DAT_0838bc70)[iVar9];
            if ((((bVar28 & 4) != 4) || ((bVar28 & 8) != 8)) || ((bVar28 & 0x10) != 0x10)) {
              fVar17 = FUN_004f7500(param_1[4], param_1[5]);
              pfVar15[1] = (float)(fVar17 + (float10)_DAT_00552540);
              local_338 = local_338 + _DAT_00552864;
              FUN_00460dc0(0xfb,pfVar14,&local_340,param_1 + 0x3a,(float *)0x0,param_1,(float *)0x64,(float *)0x0,0);
              FUN_00460dc0(0xfc,pfVar14,&local_340,param_1 + 0x3a,(float *)0x0,param_1,(float *)0x64,(float *)0x0,0);
              pfVar10 = local_35c;
            }
            pfVar10 = pfVar10 + 1;
            local_344 = (float)((int)local_344 + 1);
            pfVar15 = pfVar15 + 3;
          } while ((int)local_344 < 5);
          local_358 = (float)((int)local_358 + 1);
          local_348 = (float)((int)local_348 + 1);
          local_35c = pfVar10;
        } while ((int)local_348 < 4);
        bVar16 = (char *)*(int*)&param_1[0x3f] == DAT_07abf5d8;
        param_1[0x18] = 0.0;
        if (bVar16) {
          FUN_0045fec0((uint)*(byte *)((int)param_1 + 0x85),param_1 + 0x5c,300.0,
                       *(byte *)(param_1 + 0x22),*(short *)((int)param_1 + 0x86));
        }
      }
      else {
        local_358 = (float)(int)fVar13;
        local_2e0 = 15.0;
        if ((9 < (int)fVar13) && ((int)fVar13 < 0x10)) {
          local_358 = 12.5;
          local_2e0 = 18.0;
          if (fVar13 == 2.10195e-44) {
            param_1[0x36] = param_1[0x36] * _DAT_005526d8;
          }
        }
        if (fVar13 == 1.82169e-44) {
          local_340 = 0.0;
          local_33c = 0.0;
          local_338 = 0.0;
          local_310 = -25.0;
          local_30c = -40.0;
          local_308 = 0;
          FUN_004f9db0((float *)(*(int*)&param_1[0x3f] + 0x1c),local_15c);
          FUN_004fa0b0(&local_310,local_15c,&local_328);
          local_304 = local_328 + param_1[4];
          local_35c = (float *)0x0;
          local_300 = local_324 + param_1[5];
          local_2fc = local_320 + param_1[6];
          do {
            pfVar10 = local_35c;
            local_2bc = local_2fc - (float)(int)local_35c;
            local_2c4 = local_304;
            local_2c0 = local_300;
            FUN_00460dc0(0xfe,&local_2c4,&local_340,param_1 + 0x3a,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
            local_35c = (float *)((int)pfVar10 + 0x32);
          } while ((int)local_35c < 200);
          iVar9 = _rand();
          local_304 = (float)(iVar9 % 0x1e + 0x14) + local_304;
          iVar9 = _rand();
          local_35c = (float *)0x0;
          local_2fc = (float)(iVar9 % 500 + -0xfa) + local_2fc;
          do {
            pfVar10 = local_35c;
            local_2bc = local_2fc - (float)(int)local_35c;
            local_2c4 = local_304;
            local_2c0 = local_300;
            FUN_00460dc0(0xfe,&local_2c4,&local_340,param_1 + 0x3a,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
            local_35c = (float *)((int)pfVar10 + 0x1e);
          } while ((int)local_35c < 0x78);
          FUN_00404bc0(0x59,(unsigned int)(uintptr_t)param_1,0);
        }
        fVar13 = param_1[7] + _DAT_00552878;
        param_1[4] = param_1[0x5c];
        param_1[5] = param_1[0x5d];
        param_1[6] = param_1[0x5e];
        param_1[7] = fVar13;
        fVar17 = (float10)fsin(((float10)_DAT_005524fc - (float10)local_358) * (float10)local_2e0 *
                               (float10)_DAT_005529f0 * (float10)_DAT_005529e8);
        param_1[0x31] = (float)(fVar17 * (float10)_DAT_005529e0);
        if ((local_358 < (float)_DAT_005529d8) || ((float)_DAT_005529d8 < local_358)) {
          fVar13 = param_1[0x36] + _DAT_00552658;
        }
        else {
          fVar13 = param_1[0x36] - _DAT_00552658;
        }
        param_1[0x36] = fVar13;
        FUN_004f9db0(param_1 + 10,local_15c);
        FUN_004fa0b0(param_1 + 0x30,local_15c,&local_328);
        param_1[4] = local_328 + param_1[0x5c];
        param_1[5] = local_324 + param_1[0x5d];
        param_1[6] = local_320 + param_1[0x5e] + param_1[0x36] + _DAT_0055285c;
      }
      break;
    case 0xf5:
      param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4 * _DAT_005529d4;
      if ((int)fVar13 < 10) {
        param_1[6] = param_1[6] - _DAT_00552504;
      }
      if ((0xf < (int)fVar13) && ((int)fVar13 % 3 == 0)) {
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000007;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
        }
        _DAT_083a0210 = (float)(int)(uVar8 - 4) * _DAT_005524f4;
      }
      break;
    case 0xf6:
      if (_DAT_00552598 < param_1[3]) {
        param_1[0x18] = 0.0;
      }
      local_358 = param_1[0x18];
      if ((int)local_358 < 10) {
        if ((int)local_358 < 5) {
          param_1[6] = param_1[6] - _DAT_00552504;
        }
        param_1[0x1a] = (float)(int)local_358 * _DAT_005524f4;
        if (4 < (int)local_358) {
          iVar9 = _rand();
          if ((float)(iVar9 % 10) == 0.0) {
            local_230 = (float)(iVar9 % 10);
            iVar9 = _rand();
            local_228 = 0;
            local_1dc = 0.0;
            local_1d8 = 0;
            local_22c = (float)(iVar9 % 0x96);
            iVar9 = _rand();
            local_1d4 = (float)(iVar9 % 0x168);
            FUN_004f9db0(&local_1dc,local_cc);
            FUN_004fa0b0(&local_230,local_cc,&local_2ac);
            local_2ac = local_2ac + param_1[4];
            pfVar15 = param_1 + 0x3a;
            pfVar10 = param_1 + 7;
            pfVar14 = &local_2ac;
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            local_2a8 = local_2a8 + param_1[5];
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar22 = (float *)0x0;
            local_2a4 = local_2a4 + param_1[6];
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000001;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
            }
            FUN_00460dc0(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
        }
      }
      else {
        param_1[0x1a] = (float)(0x14 - (int)local_358) * _DAT_005524f4;
      }
      goto LAB_00467f78;
    case 0xf7:
    case 0xfa:
      param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4 * _DAT_00552a00;
      if ((int)fVar13 < 0xd) {
        param_1[6] = param_1[6] - _DAT_00552504;
      }
      break;
    case 0xf8:
    case 0xfb:
      param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4 * _DAT_005529d4;
      if ((int)fVar13 < 10) {
        param_1[6] = param_1[6] - _DAT_00552504;
      }
      break;
    case 0xf9:
      if ((int)fVar13 < 0x1e) {
        if ((int)fVar13 < 0xf) {
          param_1[6] = param_1[6] - _DAT_00552504;
        }
        param_1[0x1a] = (float)(int)fVar13 * _DAT_005524f4;
        if (4 < (int)fVar13) {
          iVar9 = _rand();
          if ((float)(iVar9 % 0xf) == 0.0) {
            local_218 = (float)(iVar9 % 0xf);
            iVar9 = _rand();
            local_210 = 0;
            local_1c4 = 0.0;
            local_1c0 = 0;
            local_214 = (float)(iVar9 % 0x96);
            iVar9 = _rand();
            local_1bc = (float)(iVar9 % 0x168);
            FUN_004f9db0(&local_1c4,local_12c);
            FUN_004fa0b0(&local_218,local_12c,&local_2dc);
            local_2dc = local_2dc + param_1[4];
            pfVar15 = param_1 + 0x3a;
            pfVar10 = param_1 + 7;
            pfVar14 = &local_2dc;
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            local_2d8 = local_2d8 + param_1[5];
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar22 = (float *)0x0;
            local_2d4 = local_2d4 + param_1[6];
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000001;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
            }
            FUN_00460dc0(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
        }
      }
      else {
        param_1[0x1a] = (float)(0x28 - (int)fVar13) * _DAT_005524f4;
      }
LAB_00467f78:
      local_364 = 0.0;
      local_360 = 0.0;
      iVar9 = 1;
      param_1[0x1b] = (float)(*(int*)&param_1[0x18]) * _DAT_005529d0;
LAB_00468736:
      local_368 = local_36c;
      FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,iVar9,(int)DAT_081cb608);
      break;
    case 0xfc:
      fVar4 = fVar13;
      if (0x1d < (int)fVar13) {
        fVar4 = (float)(0x28 - (int)fVar13);
      }
      param_1[0x1a] = (float)(int)fVar4 * _DAT_005524f4;
      if ((int)fVar13 < 0xf) {
        param_1[6] = param_1[6] - _DAT_00552504;
      }
      if (0xc < (int)fVar13) {
        _rand();
      }
      local_368 = local_36c;
      param_1[0x1b] = (float)(*(int*)&param_1[0x18]) * _DAT_005529d0;
      local_364 = 0.0;
      local_360 = 0.0;
      FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,1,(int)DAT_081cb608);
      break;
    case 0xfd:
      if (param_1[3] <= _DAT_0055264c) {
        param_1[3] = param_1[3] + _DAT_00552a1c;
        param_1[4] = param_1[4] - _DAT_00552a1c;
        param_1[6] = param_1[6] - _DAT_00552a18;
      }
      else {
        param_1[3] = param_1[3] + _DAT_005524f4;
        param_1[4] = param_1[4] - _DAT_0055256c;
        param_1[6] = param_1[6] - _DAT_005528f0;
        param_1[0x1a] = (float)(int)fVar13 * _DAT_00552a20;
      }
LAB_00466d90:
      local_368 = local_36c * _DAT_00552a14;
      iVar9 = 5;
      local_364 = local_368;
      local_360 = local_368;
LAB_004695c0:
      FUN_004f76c0(param_1[4],param_1[5],(int)&local_368,iVar9,(int)DAT_081cb608);
      break;
    case 0xfe:
      param_1[6] = param_1[6] - param_1[0x36];
      param_1[0x36] = param_1[0x36] + _DAT_0055290c;
      param_1[0x1a] = (float)(int)fVar13 * _DAT_00552a10;
      break;
    case 0xff:
      pcVar11 = (char *)*(int*)&param_1[0x3f];
      if (*pcVar11 == '\0') {
        param_1[0x18] = -NAN;
      }
      else {
        pfVar10 = param_1 + 4;
        param_1[7] = *(float *)(pcVar11 + 0x1c);
        param_1[8] = *(float *)(pcVar11 + 0x20);
        param_1[9] = *(float *)(pcVar11 + 0x24);
        param_1[10] = *(float *)(pcVar11 + 0x1c);
        param_1[0xb] = *(float *)(pcVar11 + 0x20);
        param_1[0xc] = *(float *)(pcVar11 + 0x24);
        param_1[0x5c] = *pfVar10;
        param_1[0x5d] = param_1[5];
        fVar4 = param_1[0x36] + _DAT_00552848;
        param_1[0x5e] = param_1[6];
        *pfVar10 = *(float *)(pcVar11 + 0x10);
        fVar13 = *(float *)(pcVar11 + 0x18);
        param_1[5] = *(float *)(pcVar11 + 0x14);
        param_1[0x36] = fVar4;
        param_1[6] = fVar13;
        if (param_1[1] == 1.4013e-45) {
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          FUN_0046d840(0x4e6,pfVar10,pfVar10,param_1 + 7,3,0,20.0,7,0);
          uVar29 = 0;
          iVar9 = _rand();
          pfVar15 = &local_368;
          fVar13 = (float)(iVar9 % 0x168);
          pfVar14 = param_1;
          uVar8 = _rand();
          uVar8 = uVar8 & 0x80000007;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
          }
          FUN_004795c0(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005526e4,pfVar15,(int)pfVar14,fVar13,uVar29);
        }
        param_1[0xb] = param_1[0x36];
      }
      break;
    case 0x100:
      uVar8 = _rand();
      uVar8 = uVar8 & 0x80000001;
      bVar16 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
      }
      if (bVar16) {
        FUN_00475220(0x4c4,param_1 + 4,param_1 + 7,&local_368,0,1.0,0);
      }
      pfVar10 = param_1 + 4;
      FUN_00475220(0x4ab,pfVar10,param_1 + 7,&local_368,5,1.0,0);
      FUN_00475220(0x4ab,pfVar10,param_1 + 7,&local_368,5,1.0,0);
      local_368 = local_36c * _DAT_00552534;
      local_364 = local_36c * _DAT_00552530;
      local_360 = local_364;
      FUN_004f76c0(*pfVar10,param_1[5],(int)&local_368,2,(int)DAT_081cb608);
      break;
    case 0x104:
    case 0x105:
      param_1[4] = param_1[0x30] + param_1[4];
      param_1[5] = param_1[5] + param_1[0x31];
      param_1[6] = param_1[0x32] + param_1[6];
      fVar13 = param_1[0x30] * _DAT_005526e8;
      param_1[0x30] = fVar13;
      local_348 = param_1[0x31] * _DAT_005526e8;
      param_1[0x31] = local_348;
      local_344 = param_1[0x32] * _DAT_005526e8;
      param_1[0x32] = local_344;
      param_1[6] = param_1[6] + param_1[0x36];
      if (param_1[1] == 1.4013e-45) {
        param_1[0x30] = fVar13 * _DAT_005529b4;
        param_1[0x31] = local_348 * _DAT_005529b4;
        param_1[0x32] = local_344 * _DAT_005529b4;
      }
      param_1[0x36] = param_1[0x36] - _DAT_00552540;
      fVar17 = FUN_004f7500(param_1[4], param_1[5]);
      if ((float10)param_1[6] < fVar17) {
        param_1[6] = (float)fVar17;
        fVar13 = param_1[0x36] * _DAT_00552570;
        goto LAB_0046a000;
      }
LAB_0046a01d:
      fVar13 = param_1[3] * _DAT_005526e0;
      goto LAB_0046a02b;
    case 0x106:
    case 0x107:
      param_1[4] = param_1[0x30] + param_1[4];
      param_1[5] = param_1[5] + param_1[0x31];
      param_1[6] = param_1[0x32] + param_1[6];
      fVar13 = param_1[0x30] * _DAT_005526e8;
      param_1[0x30] = fVar13;
      local_348 = param_1[0x31] * _DAT_005526e8;
      param_1[0x31] = local_348;
      local_344 = param_1[0x32] * _DAT_005526e8;
      param_1[0x32] = local_344;
      param_1[6] = param_1[6] + param_1[0x36];
      if (param_1[1] == 1.4013e-45) {
        param_1[0x30] = fVar13 * _DAT_005529b4;
        param_1[0x31] = local_348 * _DAT_005529b4;
        param_1[0x32] = local_344 * _DAT_005529b4;
      }
      param_1[0x36] = param_1[0x36] - _DAT_00552650;
      fVar17 = FUN_004f7500(param_1[4], param_1[5]);
      if (fVar17 <= (float10)param_1[6]) goto LAB_0046a01d;
      param_1[6] = (float)fVar17;
      fVar13 = param_1[0x36] * _DAT_00552a14;
LAB_0046a000:
      param_1[0x36] = fVar13;
      fVar13 = param_1[3] * _DAT_005529bc;
      *(int*)&param_1[0x18] = *(int*)&param_1[0x18] + -5;   // counter DWORD
LAB_0046a02b:
      pfVar10 = param_1 + 7;
      *pfVar10 = *pfVar10 - fVar13;
      iVar9 = _rand();
      if (iVar9 % 10 == 0) {
        FUN_00475220(0x4c5,&local_354,pfVar10,&local_368,0,1.0,0);
      }
      break;
    case 0x109:
      if ((((byte)*(undefined4 *)((char *)*(int*)&param_1[0x3f] + 0x78) & 0x20) == 0x20) &&
         (*(char *)*(int*)&param_1[0x3f] != '\0')) {
        if (0x32 < (int)fVar13) {
          iVar9 = 3;
          do {
            iVar12 = _rand();
            local_354 = (float)(iVar12 % 0x28 + -0x14);
            iVar12 = _rand();
            local_350 = (float)(iVar12 % 0x28 + -0x14);
            iVar12 = _rand();
            local_334 = 0.0;
            local_330 = 0.0;
            local_34c = (float)(iVar12 % 100);
            iVar12 = _rand();
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x5;
            pfVar10 = &local_354;
            pfVar15 = &local_334;
            local_32c = (float)(iVar12 % 0x168);
            pfVar14 = param_1 + 0x3a;
            pfVar22 = param_1;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000001;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
            }
            FUN_00460dc0(uVar8 + 0xe2,pfVar10,pfVar15,pfVar14,pfVar21,pfVar22,pfVar23,pfVar25,bVar28);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }
      }
      else {
LAB_0046a85e:
        *(undefined1 *)param_1 = 0;
      }
    } // end switch

switchD_00466b93_caseD_c1:
    sVar3 = *(short *)((int)param_1 + 2);
    if ((((sVar3 != 0xee) && (sVar3 != 0xef)) && (sVar3 != 0xf4)) &&
       (((sVar3 != 0xc5 && (sVar3 != 0xc6)) || (param_1[1] != 7.00649e-45)))) {
      if ((0xad < sVar3) && (sVar3 < 0x10d)) {
        pvVar1 = (void *)(DAT_05828d58 + sVar3 * 0xbc);
        fVar13 = param_1[0x33];
        *(undefined1 *)((int)pvVar1 + 0xa0) = *(undefined1 *)((int)param_1 + 0x105);
        FUN_00440aa0(pvVar1,param_1 + 0x42,param_1 + 0x43,(undefined1 *)((int)param_1 + 0x106),fVar13);
      }
      sVar3 = *(short *)((int)param_1 + 2);
      if ((sVar3 < 0x4ba) || (0x4bc < sVar3)) {
        if (sVar3 == 0xd1) {
LAB_0046b3bf:
          iVar9 = 0;
        }
        else {
          if (sVar3 == 0xf3) {
            local_248 = param_1[7];
            local_244 = param_1[8];
            local_240 = param_1[9];
            FUN_004f9db0(&local_248,local_9c);
            FUN_004fa0b0(param_1 + 0x30,local_9c,&local_23c);
            param_1[0x5c] = local_23c + param_1[0x5c];
            param_1[0x5d] = local_238 + param_1[0x5d];
            param_1[0x5e] = local_234 + param_1[0x5e];
            goto LAB_0046b3ca;
          }
          if (sVar3 == 0x10a) goto LAB_0046b3bf;
          iVar9 = 1;
        }
        FUN_00465fe0((int)param_1,iVar9);
      }
    }
LAB_0046b3ca:
    // IDA decrements the raw DWORD at o+0x60.  It is not an IEEE float:
    // the original tests the resulting signed integer and writes it back as
    // a DWORD, preserving the effect's full lifetime.
    iVar9 = *(int *)((char *)param_1 + 0x60) - 1;
    *(int *)((char *)param_1 + 0x60) = iVar9;
    if (iVar9 < 1) {
      *(undefined1 *)param_1 = 0;
      return;
    }
    if (*(short *)((int)param_1 + 2) == 0xbf) {
      if (param_1[1] != 4.2039e-45) {
        return;
      }
    }
    else {
      if (*(short *)((int)param_1 + 2) != 0x47e) {
        return;
      }
      if (param_1[1] != 0.0) {
        return;
      }
    }
    bVar16 = ((uint)iVar9 & 0x80000001) == 0;
    if ((int)((uint)iVar9 & 0x80000001) < 0) {
      bVar16 = (((uint)iVar9 & 0x80000001) - 1 | 0xfffffffe) == 0xffffffff;
    }
    if (bVar16) {
      return;
    }
  } while( true );
}
