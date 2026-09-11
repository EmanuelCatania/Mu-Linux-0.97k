// Effect_Create.cpp
// Effect_Create @ 0x00460dc0
//
// CreateEffect -- spawns a visual effect by type ID.

#include "stdafx.h"

extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// IDA: FUN_00460dc0
float * __cdecl
Effect_Create(int param_1,float *param_2,float *param_3,float *param_4,float *param_5,float *param_6,
            float *param_7,float *param_8,byte param_9)

{
  // [DIAG FORGE] entry-point CreateEffect (param_4 = Light/color)
  if (param_2)
      DbgForge("CreateEffect", param_1, -1, -1, -1, -1, -1,
               param_2[0], param_2[1], param_2[2],
               param_4 ? param_4[0] : 0.0f, param_4 ? param_4[1] : 0.0f,
               param_4 ? param_4[2] : 0.0f, -1.0f);
  byte bVar1;
  float fVar2;
  float *pfVar3;
  float *pfVar4;
  char cVar5;
  byte bVar6;
  void *pvVar7;
  float *pfVar8;
  int iVar9;
  uint uVar10;
  undefined4 *puVar11;
  float *pfVar12;
  byte *pbVar13;
  float *pfVar14;
  float *_param_9;
  int iVar15;
  uint uVar16;
  float *pfVar17;
  float **ppfVar18;
  undefined4 *puVar19;
  undefined4 *puVar20;
  byte *pbVar21;
  bool bVar22;
  bool bVar23;
  float10 fVar24;
  float10 fVar25;
  float10 fVar26;
  undefined4 *unaff_retaddr;
  float fVar27;
  short sVar28;
  // 2026-08-10 FIX (mismo patrón que MoveJoint): estos "locales" sueltos son en
  // realidad un bloque CONTIGUO del frame original (ebp-0x6C .. ebp), y el
  // código depende de esa contigüidad — MSVC no la garantiza:
  //   Vector_Rotate(&local_6c, local_3c + 3, &local_60)
  //     → entrada  = {local_6c, local_68, local_64}
  //     → salida   = {local_60, local_5c, local_58}
  //   Vector_Rotate(&local_6c, local_3c + 3, &local_48)
  //     → salida   = {local_48, local_44, local_40}
  // Además `Matrix_BuildFromEuler(ang, local_3c + 3)` escribe una matriz 3x4 (12 floats)
  // en local_3c[3..14], o sea un float FUERA del `float local_3c[14]` original.
  // Mapeo por offset de frame: -0x6C=[0] … -0x3C=[12]; a local_3c se le dan 16
  // slots para que la matriz entre completa.
  float __frame[28] = {0};
  float &local_6c = __frame[0];
  float &local_68 = __frame[1];
  float &local_64 = __frame[2];
  float &local_60 = __frame[3];
  float &local_5c = __frame[4];
  float &local_58 = __frame[5];
  float &local_54 = __frame[6];
  float &local_50 = __frame[7];
  float &local_4c = __frame[8];
  float &local_48 = __frame[9];
  float &local_44 = __frame[10];
  float &local_40 = __frame[11];
  float *const local_3c = &__frame[12];
  undefined4 *puStack_4;

  pfVar12 = param_3;
LAB_00460dd8:
  pfVar8 = (float *)0x0;
  pfVar17 = (float *)&DAT_07b11670[0];
  // BUG-FIX 2026-05-03: was `if (0x7b2714f < (int)pfVar17)` — absolute source-
  // binary address. Pool DAT_07b11670 is sized 200 × 0x1bc bytes; walk by
  // explicit iteration count.
  {
    bool found_slot = false;
    for (int i = 0; i < 200; ++i, pfVar17 = pfVar17 + 0x6f) {
      if (*(char *)pfVar17 == '\0') { found_slot = true; break; }
    }
    if (!found_slot) return (float *)0x0;
  }
  *(undefined1 *)pfVar17 = 1;
  *(short *)((int)pfVar17 + 2) = (short)param_1;
  // 00460DC0 writes SubType and Owner as raw DWORD fields.  The reconstructed
  // prototype still carries them as float pointers, so numeric casts corrupt
  // every non-zero subtype/owner (notably all skill effects) instead of
  // preserving the original bit pattern.
  *(DWORD *)(pfVar17 + 1) = (DWORD)(uintptr_t)param_5;
  *(undefined1 *)(pfVar17 + 0x37) = 1;
  *(int*)&pfVar17[0x16] = -1;
  *(int*)&pfVar17[0x19] = -1;
  pfVar17[0x1a] = 1.0;
  pfVar17[0x1b] = 0.0;
  pfVar17[0x1c] = 0.0;
  pfVar17[0x42] = 0.0;
  *(undefined1 *)((int)pfVar17 + 0x161) = 0;
  pfVar17[0x5a] = 1.0;
  pfVar17[0x43] = 0.0;
  pfVar17[3] = 0.9;
  *(DWORD *)(pfVar17 + 0x3f) = (DWORD)(uintptr_t)param_6;
  pfVar17[0x33] = 0.3;
  *(short *)((int)pfVar17 + 0x86) = (short)(uintptr_t)param_7;
  *(byte *)(pfVar17 + 0x21) = param_9;
  fVar27 = *param_4;
  // CreateEffect's eighth argument is SkillIndex (an integer), not a pointer.
  // IDA writes its low byte directly to effect+0x85.
  *(undefined1 *)((int)pfVar17 + 0x85) = (byte)(uintptr_t)param_8;
  _param_9 = pfVar17 + 0x3a;
  *(undefined1 *)(pfVar17 + 0x1d) = 0;
  pfVar17[0x3d] = 0.0;
  *_param_9 = fVar27;
  fVar27 = param_4[2];
  pfVar17[0x3b] = param_4[1];
  pfVar17[0x3c] = fVar27;
  pfVar14 = pfVar17 + 4;
  pfVar17[7] = *pfVar12;
  pfVar17[8] = pfVar12[1];
  pfVar17[9] = pfVar12[2];
  *pfVar14 = *param_2;
  pfVar17[5] = param_2[1];
  pfVar17[6] = param_2[2];
  pfVar17[0x30] = 0.0;
  pfVar17[0x31] = 0.0;
  pfVar17[0x32] = 0.0;
  pfVar4 = (float*)DAT_07cf1ffc;
  pfVar3 = (float*)DAT_07abf5d8;
  iVar9 = DAT_055c9bc8;
  fVar27 = _DAT_00552580;
  if (0xf6 < param_1) {
    if (param_1 < 0x1f1) {
      if (param_1 == 0x1f0) goto LAB_00464ed8;
      switch(param_1) {
      case 0xf7:
        pfVar17[0x18] = 4.90454e-44;
        pfVar17[3] = (float)(int)(short)param_7 * _DAT_00552594;
        return (float *)(int)(short)param_7;
      case 0xf8:
        pfVar17[0x18] = 4.90454e-44;
        pfVar17[3] = (float)(int)(short)param_7 * _DAT_00552594;
        pfVar17[0x19] = 0.0;
        return pfVar8;
      case 0xf9:
        pfVar17[0x18] = 5.60519e-44;
        fVar27 = (float)(int)(short)param_7;
        break;
      case 0xfa:
        pfVar17[0x18] = 4.90454e-44;
        pfVar17[3] = (float)(int)(short)param_7 * _DAT_00552594;
        return (float *)(int)(short)param_7;
      case 0xfb:
        pfVar17[0x18] = 5.60519e-44;
        pfVar17[3] = (float)(int)(short)param_7 * _DAT_00552594;
        pfVar17[0x19] = 0.0;
        return pfVar8;
      case 0xfc:
        pfVar17[0x18] = 5.60519e-44;
        fVar27 = (float)(int)(short)param_7;
        break;
      case 0xfd:
        pfVar17[0x19] = 0.0;
        pfVar17[0x1a] = 1.5;
        pfVar17[3] = 0.5;
        pfVar17[0x18] = 2.10195e-44;
        pfVar17[6] = pfVar17[6] - _DAT_00552834;
        goto LAB_004658be;
      case 0xfe:
        *(int*)&pfVar17[0x19] = -2;
        pfVar17[0x18] = 8.40779e-45;
        pfVar17[3] = 1.0;
        pfVar17[0x36] = 80.0;
        pfVar17[0x3a] = 0.5;
        pfVar17[0x3b] = 0.5;
        pfVar17[0x3c] = 0.5;
        pfVar17[7] = 0.0;
        pfVar17[8] = 0.0;
        pfVar17[9] = 45.0;
        return pfVar8;
      case 0xff:
        pfVar17[0x18] = 1.4013e-43;
        pfVar17[0x19] = 0.0;
        pfVar17[0x1a] = 1.0;
        pfVar17[0x36] = 0.0;
        if (param_5 == (float *)0x0) {
          pfVar17[3] = 12.0;
          pfVar17[0x5c] = *(float *)(*(int*)&pfVar17[0x3f] + 0x10);
          pfVar17[0x5d] = *(float *)(*(int*)&pfVar17[0x3f] + 0x14);
          pfVar17[0x5e] = *(float *)(*(int*)&pfVar17[0x3f] + 0x18);
        }
        else if (param_5 == (float *)0x1) {
          pfVar17[0x16] = 0.0;
          pfVar17[3] = 24.0;
        }
        pfVar12 = pfVar17 + 7;
        pfVar8 = pfVar17 + 4;
        bVar6 = (byte)param_5;
        Joint_Create(0x4e2,pfVar8,pfVar8,pfVar12,0,(int)pfVar17,pfVar17[3],0x1e,bVar6);
        Joint_Create(0x4e2,pfVar8,pfVar8,pfVar12,1,(int)pfVar17,pfVar17[3],0x1e,bVar6);
        Joint_Create(0x4e2,pfVar8,pfVar8,pfVar12,2,(int)pfVar17,pfVar17[3],0x1e,bVar6);
        Joint_Create(0x4e2,pfVar8,pfVar8,pfVar12,3,(int)pfVar17,pfVar17[3],0x1e,bVar6);
        pfVar12 = (float *)FUN_00404bc0(0x5c,0,0);
        pfVar17[3] = 1.0;
        return pfVar12;
      case 0x100:
        goto switchD_00460f25_caseD_100;
      default:
        return (float *)0x0;
      case 0x103:
        pfVar17[0x18] = 2.8026e-44;
        pfVar17[0x19] = 0.0;
        pfVar17[0x1a] = 1.0;
        return pfVar8;
      case 0x104:
      case 0x105:
        local_6c = 0.0;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x8000007f;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
        }
        param_8 = (float *)(uVar10 + 0x20);
        local_64 = 0.0;
        local_68 = (float)(int)param_8 * _DAT_005524f4;
        pfVar17[6] = pfVar17[6] + _DAT_00552598;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x8000000f;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffff0) + 1;
        }
        *(int*)&pfVar17[0x18] = (int)(uVar10 + 0x20);   // lifetime: DWORD, no float (fix 2026-08-16)
        uVar10 = _rand();
        uVar10 = uVar10 & 0x80000003;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
        }
        param_8 = (float *)(uVar10 + 8);
        pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[9] = (float)(int)param_8;
        Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
        Vector_Rotate(&local_6c,local_3c + 3,pfVar17 + 0x30);
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 5 + 2);
        pfVar17[0x36] = (float)(int)param_8;
        if ((*(short *)((int)pfVar17 + 2) == 0x105) && (pfVar17[1] == 0.0)) {
          pfVar17[1] = 1.4013e-45;
          iVar9 = _rand();
          param_8 = (float *)(iVar9 % 5);
          pfVar17[0x36] = (float)(int)param_8 + pfVar17[0x36];
        }
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[7] = (float)(int)param_8;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[8] = (float)(int)param_8;
        iVar9 = _rand();
        pfVar17[9] = (float)(iVar9 % 0x168);
        return (float *)(iVar9 / 0x168);
      case 0x106:
      case 0x107:
        local_6c = 0.0;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x8000007f;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
        }
        param_8 = (float *)(uVar10 + 0x40);
        local_64 = 0.0;
        local_68 = (float)(int)param_8 * _DAT_005524f4;
        pfVar17[6] = pfVar17[6] + _DAT_00552598;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x8000000f;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffff0) + 1;
        }
        *(int*)&pfVar17[0x18] = (int)(uVar10 + 0x20);   // lifetime: DWORD, no float (fix 2026-08-16)
        uVar10 = _rand();
        uVar10 = uVar10 & 0x80000003;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
        }
        param_8 = (float *)(uVar10 + 8);
        pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[9] = (float)(int)param_8;
        Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
        Vector_Rotate(&local_6c,local_3c + 3,pfVar17 + 0x30);
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 5 + 2);
        pfVar17[0x36] = (float)(int)param_8;
        if ((*(short *)((int)pfVar17 + 2) == 0x106) && (pfVar17[1] == 0.0)) {
          pfVar17[1] = 1.4013e-45;
          iVar9 = _rand();
          param_8 = (float *)(iVar9 % 5);
          pfVar17[0x36] = (float)(int)param_8 + pfVar17[0x36];
        }
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[7] = (float)(int)param_8;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[8] = (float)(int)param_8;
        iVar9 = _rand();
        pfVar17[9] = (float)(iVar9 % 0x168);
        return (float *)(iVar9 / 0x168);
      case 0x109:
        pfVar17[0x18] = 8.40779e-44;
        return pfVar8;
      case 0x10a:
        pfVar17[0x18] = 2.8026e-44;
        pfVar17[3] = 1.5;
        fVar24 = (float10)fsin((float10)pfVar17[9] * (float10)_DAT_0055253c);
        pfVar17[0x30] = (float)(fVar24 * (float10)_DAT_00552660);
        fVar24 = (float10)fcos((float10)pfVar17[9] * (float10)_DAT_0055253c);
        pfVar17[0x31] = (float)(fVar24 * (float10)_DAT_00552828);
        return pfVar8;
      case 0x10c:
        pfVar17[0x18] = 1.4013e-45;
        pfVar17[0x19] = 0.0;
        pfVar17[0x1a] = 1.0;
        pfVar17[3] = 5.0;
        return pfVar8;
      }
LAB_004649e0:
      pfVar17[3] = fVar27 * _DAT_00552594;
      pfVar17[0x19] = 0.0;
      if (param_5 != (float *)0x1) {
        return param_5;
      }
      *(undefined1 *)(pfVar17 + 0x1d) = 0x80;
      return (float *)0x1;
    }
    if (0x4b0 < param_1) {
      switch(param_1) {
      case 0x4ba:
      case 0x4bb:
      case 0x4bc:
        goto switchD_00465549_caseD_4ba;
      default:
        return (float *)0x0;
      case 0x4df:
        pfVar17[0x3a] = 1.0;
        pfVar17[0x3b] = 1.0;
        pfVar17[0x3c] = 1.0;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 9 + -4);
        pfVar17[0x30] = (float)(int)param_8;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 9 + -4);
        pfVar17[0x31] = (float)(int)param_8;
        pfVar17[0x32] = 26.0;
        pfVar17[0x18] = 1.68156e-44;
        pfVar12 = (float *)FUN_00404bc0(0x44,(int)pfVar17,0);
        return pfVar12;
      case 0x4e0:
        pfVar17[7] = 0.0;
        pfVar17[8] = 0.0;
        pfVar17[9] = 0.0;
        pfVar17[6] = 100.0;
        pfVar17[0x18] = 1.05097e-43;
        return pfVar8;
      case 0x4f0:
      case 0x4f1:
        pfVar17[0x18] = 2.8026e-44;
        // 2026-08-15: IDA `if (*((_DWORD *)i + 1) == 4)` — SubType es un
        // DWORD. Leerlo como float y convertir (`(int)pfVar17[1]`) daba 0 para
        // cualquier SubType chico, asi que la rama NUNCA se ejecutaba.
        if (*(int*)&pfVar17[1] != 4) {
          return (float*)(uintptr_t)*(int*)&pfVar17[1];
        }
        pfVar17[0x18] = 8.40779e-44;
        iVar9 = _rand();
        pfVar17[3] = (float)(iVar9 % 0x32 + 0x32) * _DAT_0055295c;
        return (float *)(iVar9 / 0x32);
      case 0x4f3:
        pfVar17[0x18] = 1.4013e-44;
        if (pfVar17[1] == 0.0) {
          local_6c = 0.0;
          local_68 = -100.0;
          local_64 = 0.0;
          Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
          pfVar12 = (float *)Vector_Rotate(&local_6c,local_3c + 3,&local_60);
          pfVar17[4] = local_60 + pfVar17[4];
          pfVar17[5] = local_5c + pfVar17[5];
          pfVar17[6] = local_58 + pfVar17[6] + _DAT_0055297c;
          return pfVar12;
        }
        if (*(int*)&pfVar17[1] != 1) {
          return (float *)0x0;
        }
        pfVar12 = (float*)(uintptr_t)(int)param_2[1];
        fVar27 = param_2[2];
        pfVar17[0x5c] = *param_2;
        // 00460DC0 case 1267/subtype 1 copies Position[1] as a float.
        // It is not a pointer-as-float field.
        pfVar17[0x5d] = param_2[1];
        pfVar17[0x5e] = fVar27;
        pfVar17[0x18] = 2.8026e-44;
        return pfVar12;
      case 0x4f7:
        iVar9 = _rand();
        *(int*)&pfVar17[0x18] = (int)(iVar9 % 0xf + 0xf);   // lifetime: DWORD, no float (fix 2026-08-16)
        pfVar17[0x36] = -20.0;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x168);
        pfVar17[0x33] = (float)(int)param_8;
        pfVar17[0x3a] = 0.0;
        pfVar17[0x3b] = 0.0;
        pfVar17[0x3c] = 0.0;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 200);
        pfVar17[4] = ((float)(int)param_8 + pfVar17[4]) - _DAT_005524f0;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 200);
        pfVar17[5] = ((float)(int)param_8 + pfVar17[5]) - _DAT_005524f0;
        pfVar17[6] = pfVar17[6] + _DAT_005528e8;
        fVar27 = pfVar17[4] + _DAT_005524f0;
        pfVar17[4] = fVar27;
        pfVar17[0x5c] = fVar27;
        pfVar17[0x5d] = pfVar17[5];
        pfVar17[0x5e] = pfVar17[6];
        pfVar12 = (float *)FUN_00404bc0(0x2e,0,0);
        return pfVar12;
      case 0x4fa:
        pfVar17[0x18] = 1.4013e-42;
        pfVar12 = (float*)(uintptr_t)*(int*)&pfVar17[1];   // SubType (DWORD)
        if (pfVar12 == (float *)0x1) {
          pfVar17[0x18] = 1.31722e-42;
          return pfVar12;
        }
        if (pfVar12 != (float *)0x2) {
          return pfVar12;
        }
        pfVar17[0x18] = 1.12104e-44;
        pfVar17[6] = pfVar17[6] + _DAT_0055297c;
        return pfVar12;
      case 0x566:
        pfVar17[0x18] = 1.4013e-44;
        pfVar17[0x33] = 1.0;
        pfVar12 = pfVar17 + 7;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = -30.0;
        pfVar17[0x32] = 0.0;
        Matrix_BuildFromEuler(pfVar12,local_3c + 3);
        local_6c = 0.0;
        local_68 = -20.0;
        local_64 = 50.0;
        Vector_Rotate(&local_6c,local_3c + 3,&local_60);
        pfVar17[4] = local_60 + pfVar17[4];
        local_6c = -20.0;
        local_68 = -20.0;
        local_64 = 60.0;
        pfVar17[5] = local_5c + pfVar17[5];
        pfVar17[6] = local_58 + pfVar17[6];
        Vector_Rotate(&local_6c,local_3c + 3,&local_60);
        local_48 = local_60 + *param_2;
        local_54 = *pfVar12;
        local_50 = pfVar17[8];
        local_4c = pfVar17[9];
        local_44 = local_5c + param_2[1];
        param_8 = (float *)0x0;
        local_40 = local_58 + param_2[2];
        do {
          pfVar8 = param_8;
          iVar9 = _rand();
          _param_9 = (float *)(iVar9 % 0x14);
          local_54 = (float)(int)_param_9 + *pfVar12 + _DAT_00552660;
          local_50 = (float)(int)param_8 + local_50;
          Joint_Create(0x4e9,&local_48,&local_48,&local_54,1,0,10.0,-1,0);
          param_8 = (float *)((int)pfVar8 + 0x12);
        } while ((int)param_8 < 0x168);
        local_3c[0] = 30.0;
        local_3c[1] = -20.0;
        local_3c[2] = 60.0;
        Vector_Rotate(local_3c,local_3c + 3,&local_60);
        local_48 = local_60 + *param_2;
        local_54 = *pfVar12;
        local_50 = pfVar17[8];
        local_4c = pfVar17[9];
        local_44 = local_5c + param_2[1];
        local_40 = local_58 + param_2[2];
        param_8 = (float *)0x0;
        do {
          pfVar8 = param_8;
          iVar9 = _rand();
          _param_9 = (float *)(iVar9 % 0x14);
          local_54 = (float)(int)_param_9 + *pfVar12 + _DAT_00552660;
          local_50 = (float)(int)param_8 + local_50;
          pfVar17 = (float *)Joint_Create(0x4e9,&local_48,&local_48,&local_54,1,0,10.0,-1,0);
          param_8 = (float *)((int)pfVar8 + 0x12);
        } while ((int)param_8 < 0x168);
        return pfVar17;
      case 0x596:
        pfVar17[0x18] = 4.2039e-44;
        pfVar17[3] = 1.0;
LAB_004658be:
        pfVar17[0x3a] = 1.0;
        pfVar17[0x3b] = 1.0;
        pfVar17[0x3c] = 1.0;
        return (float *)0x3f800000;
      }
    }
    if (param_1 == 0x4b0) {
      pfVar17[0x18] = 5.60519e-44;
      param_5 = pfVar4;
      uVar10 = (**(code **)(iVar9 + 0xc))(pfVar4);
      param_8 = (float *)0x0;
      param_7 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_00465082;
      goto LAB_00465011;
    }
    if (0x498 < param_1) {
      switch(param_1) {
      case 0x49c:
        pfVar17[0x18] = 2.8026e-44;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = -60.0;
        pfVar17[0x32] = 0.0;
        pfVar17[6] = pfVar17[6] + _DAT_005524f0;
        if (*(int*)&pfVar17[1] != 1) {
          return (float *)0x0;
        }
        pfVar17[3] = 1.5;
        return pfVar8;
      default:
        return (float *)0x0;
      case 0x4a7:        pfVar17[0x18] = 1.4013e-44;
        pfVar17[3] = 1.5;
        return pfVar8;
      case 0x4ab:
        goto switchD_00464efe_caseD_4ab;
      case 0x4ac:
        pfVar17[0x18] = 1.4013e-44;
        Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
        local_6c = 0.0;
        local_68 = -60.0;
        local_64 = 0.0;
        pfVar12 = (float *)Vector_Rotate(&local_6c,local_3c + 3,&local_60);
        pfVar17[4] = local_60 + pfVar17[4];
        pfVar17[5] = local_5c + pfVar17[5];
        pfVar17[6] = local_58 + pfVar17[6] + _DAT_00552980;
        return pfVar12;
      }
    }
    if (param_1 == 0x498) {
LAB_00464ed8:
      pfVar17[0x18] = 1.4013e-44;
      return pfVar8;
    }
    if (param_1 < 0x47f) {
      if (param_1 == 0x47e) {
        if (pfVar17[1] == 0.0) {
          pfVar17[0x3a] = 0.3;
          pfVar17[0x3b] = 0.3;
          pfVar17[0x3c] = 0.3;
          iVar9 = _rand();
          fVar24 = (float10)(iVar9 % 5 + 9) * (float10)_DAT_00552504;
          fVar25 = (float10)fcos((float10)_DAT_00552968);
          fVar26 = (float10)fsin((float10)_DAT_00552960);
          pfVar17[0x30] = (float)(fVar26 * fVar25 * fVar24);
          fVar26 = (float10)fcos((float10)_DAT_00552960);
          pfVar17[0x31] = (float)(fVar26 * fVar25 * fVar24);
          fVar25 = (float10)fsin((float10)_DAT_00552968);
          pfVar17[0x32] = (float)(fVar25 * fVar24);
          pfVar17[3] = 3.0;
          pfVar17[0x18] = 5.60519e-43;
          return (float *)(iVar9 / 5);
        }
        if (*(int*)&pfVar17[1] != 1) {
          return (float *)0x0;
        }
switchD_00464efe_caseD_4ab:
        pfVar17[0x18] = 1.4013e-42;
        return pfVar8;
      }
      if (param_1 == 0x1f1) {
        pfVar17[9] = 0.0;
        pfVar17[8] = 0.0;
        pfVar17[7] = 0.0;
        pfVar17[0x18] = 7.00649e-45;
        return pfVar8;
      }
      if (param_1 != 0x238) {
        return (float *)0x0;
      }
      pfVar17[0x18] = 4.2039e-44;
      *(int*)&pfVar17[0x19] = -2;
      pfVar17[3] = 1.0;
      pfVar17[6] = pfVar17[6] + _DAT_00552970;
      pfVar17[7] = pfVar17[7] + _DAT_005524fc;
      pfVar17[0x30] = 0.0;
      pfVar17[0x31] = -80.0;
      pfVar17[0x32] = -10.0;
      return pfVar8;
    }
    if (param_1 != 0x490) {
      return (float *)0x0;
    }
    fVar27 = pfVar12[2] + _DAT_00552844;
    pfVar17[0x18] = 4.2039e-44;
    pfVar17[0x3a] = 0.8;
    pfVar17[0x3b] = 0.8;
    pfVar17[0x3c] = 0.8;
    pfVar17[10] = 0.0;
    pfVar17[0xb] = 0.0;
    pfVar17[0xc] = fVar27;
    goto LAB_00463e3d;
  }
  if (param_1 == 0xf6) {
    pfVar17[0x18] = 2.8026e-44;
    fVar27 = (float)(int)(short)param_7;
    goto LAB_004649e0;
  }
  switch(param_1) {
  case 0xb6:
    pfVar17[0x18] = 2.8026e-45;
    pfVar17[0x19] = 0.0;
    pfVar17[3] = 10.0;
    pfVar17[5] = pfVar17[5] + _DAT_0055285c;
    pfVar17[6] = pfVar17[6] - _DAT_00552974;
    *(undefined1 *)(pfVar17 + 0x37) = 0;
    return pfVar8;
  default:
    goto switchD_00460f25_caseD_101;
  case 0xb9:
    pfVar17[0x18] = 1.4013e-45;
    pfVar17[0x5a] = 0.5;
    pfVar17[3] = 1.5;
    return pfVar8;
  case 0xbe:
    fVar27 = pfVar17[1];
    if (fVar27 == 0.0) {
      pfVar17[0x18] = 7.00649e-44;
      pfVar17[3] = 0.8;
      pfVar17[0x33] = 1.0;
      pfVar17[7] = 0.0;
      pfVar17[0x19] = 0.0;
      return pfVar8;
    }
    if ((int)fVar27 < 1) {
      return (float *)0x0;
    }
    if (2 < (int)fVar27) {
      return (float *)0x0;
    }
    pfVar17[0x18] = 2.8026e-44;
    pfVar17[3] = 0.8;
    pfVar17[7] = -20.0;
    pfVar17[0x19] = 0.0;
    pfVar17[0x1a] = 0.5;
    pfVar17[0x36] = 5.0;
    pfVar17[10] = 0.0;
    pfVar17[0xb] = 0.0;
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 0x168);
    iVar9 = 3;
    pfVar17[0xc] = (float)(int)param_8;
    do {
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000003f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffffc0) + 1;
      }
      param_8 = (float *)(uVar10 - 0x20);
      local_48 = (float)(int)param_8 + pfVar17[4];
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000003f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffffc0) + 1;
      }
      param_8 = (float *)(uVar10 - 0x20);
      local_44 = (float)(int)param_8 + pfVar17[5];
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000007f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
      }
      param_8 = (float *)(uVar10 + 0x20);
      local_40 = (float)(int)param_8 + pfVar17[6];
      Particle_Spawn(0x4c4,&local_48,pfVar17 + 7,pfVar17 + 0x3a,0,1.0,0);
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    fVar27 = pfVar17[4];
    *param_4 = 0.4;
    param_4[1] = 0.3;
    param_4[2] = 0.2;
    FUN_004f76c0(fVar27,pfVar17[5],(int)param_4,2,(int)&DAT_081cb608[0]);
LAB_00463e3d:
    pfVar17[0x5c] = pfVar17[4];
    pfVar17[0x5d] = pfVar17[5];
    pfVar17[0x5e] = pfVar17[6];
    return (float*)(uintptr_t)(int)pfVar17[5];
  case 0xbf:
    pfVar17[0x19] = 1.4013e-45;
    fVar27 = pfVar17[1];
    if (fVar27 == 0.0) {
      pfVar17[0x18] = 5.60519e-44;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x80000007;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xfffffff8) + 1;
      }
      param_8 = (float *)(uVar10 + 10);
      pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000001f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffffe0) + 1;
      }
    }
    else {
      if (*(int*)&fVar27 == 2) {
        pfVar17[0x18] = 5.60519e-44;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x80000007;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffff8) + 1;
        }
        pfVar17[3] = (float)(int)(uVar10 + 10) * _DAT_005524f4;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = 0.0;
        pfVar17[0x32] = -50.0;
        return (float *)0x0;
      }
      if (*(int*)&fVar27 == 3) {
        pfVar17[0x18] = 1.12104e-43;
        pfVar17[3] = 0.3;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = -12.0;
        pfVar17[0x32] = 0.0;
        return pfVar8;
      }
      if (*(int*)&fVar27 == 4) {
        pfVar17[0x18] = 5.60519e-44;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 10 + 0xf);
        pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
        uVar10 = _rand();
        param_8 = (float *)(uVar10 & 0x8000001f);
        if ((int)param_8 < 0) {
          param_8 = (float *)(((int)param_8 - 1U | 0xffffffe0) + 1);
        }
        pfVar17[4] = (float)(int)param_8 + pfVar17[4] + _DAT_00552980;
        pfVar17[6] = pfVar17[6] + _DAT_00552850;
        pfVar17[0x30] = 0.0;
        iVar9 = _rand();
        param_8 = (float *)(iVar9 % 0x14);
        pfVar17[0x31] = -((float)(int)param_8 + _DAT_00552488);
        iVar9 = _rand();
        pfVar17[0x32] = -((float)(iVar9 % 10) + _DAT_005524fc);
        pfVar17[7] = 0.0;
        pfVar17[8] = 20.0;
        pfVar17[9] = 0.0;
        return (float *)(iVar9 / 10);
      }
      if (*(int*)&fVar27 == 5) {
        pfVar17[0x18] = 5.60519e-44;
        pfVar17[0x36] = 5.0;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = -30.0;
        pfVar17[0x32] = 0.0;
        return pfVar8;
      }
      if (*(int*)&fVar27 != 6) {
        pfVar17[0x18] = 8.40779e-44;
        uVar10 = _rand();
        uVar10 = uVar10 & 0x80000003;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
        }
        pfVar17[3] = (float)(int)(uVar10 + 8) * _DAT_005524f4;
        pfVar17[6] = pfVar17[6] + _DAT_00552908;
        pfVar17[0x30] = 0.0;
        pfVar17[0x31] = -50.0;
        pfVar17[0x32] = 0.0;
        return (float *)0x0;
      }
      pfVar17[0x18] = 5.60519e-44;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x80000007;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xfffffff8) + 1;
      }
      param_8 = (float *)(uVar10 + 0xf);
      pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000001f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffffe0) + 1;
      }
    }
    pfVar17[4] = (float)(int)uVar10 + pfVar17[4] + _DAT_00552980;
    pfVar17[6] = pfVar17[6] + _DAT_00552850;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = 0.0;
    pfVar17[0x32] = -50.0;
    pfVar17[7] = 0.0;
    pfVar17[8] = 20.0;
    pfVar17[9] = 0.0;
    return (float *)0x0;
  case 0xc0:
    pfVar17[0x19] = 1.4013e-45;
    pfVar17[0x18] = 5.60519e-44;
    pfVar17[3] = 1.0;
    return pfVar8;
  case 0xc2:
    pfVar17[0x18] = 7.00649e-44;
    pfVar17[3] = 0.7;
    if (param_5 != (float *)0x0) {
      *(int*)&pfVar17[0x19] = -4;
      pfVar17[7] = 0.0;
      pfVar17[8] = 0.0;
      pfVar17[9] = 45.0;
      return pfVar8;
    }
    *(int*)&pfVar17[0x19] = -3;
    pfVar17[7] = 0.0;
    pfVar17[8] = 0.0;
    pfVar17[9] = 45.0;
    return pfVar8;
  case 0xc4:
    pfVar17[0x18] = 1.4013e-44;
    pfVar17[0x19] = 0.0;
    return pfVar8;
  case 0xc5:
  case 0xc6:
  case 0xd5:
  case 0xd6:
    if (*(int*)&pfVar17[1] == 5) {
      pfVar17[0x18] = 8.40779e-44;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x80000003;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
      }
      pfVar17[3] = (float)(int)(uVar10 + 8) * _DAT_005524f4;
      return (float *)(uVar10 + 8);
    }
  case 199:
  case 0xe7:
  case 0xe8:
    goto switchD_00461001_caseD_c7;
  case 200:
    pfVar17[0x18] = 6.30584e-44;
    pfVar17[0x19] = 0.0;
    if ((float*)(uintptr_t)*(int*)&pfVar17[0x3f] != pfVar3) {
      return (float*)(uintptr_t)*(int*)&pfVar17[0x3f];
    }
    FUN_00423040(&DAT_055c9bc8,DAT_07cf1ffc);
    pfVar12 = (float*)DAT_07cf1ffc;
    cVar5 = *(char *)((char*)DAT_07cf1ffc + 0x160);
    *(char *)(pfVar17 + 0x22) = cVar5;
    *(char *)(pfVar12 + 0x160) = cVar5 + '\x01';
    FUN_0043d1d0(&DAT_055c9bc8,pfVar12);
    sVar28 = *(short *)((int)pfVar17 + 0x86);
    bVar6 = *(byte *)(pfVar17 + 0x22);
    fVar27 = 300.0;
    goto LAB_00463ca2;
  case 0xc9:
    pfVar17[0x18] = 5.60519e-44;
    pfVar17[0x19] = 0.0;
    if (param_5 != (float *)0x1) {
      return (float *)0x0;
    }
    *(undefined1 *)(pfVar17 + 0x1d) = 0x80;
    return pfVar8;
  case 0xca:
    pfVar17[0x18] = 2.8026e-44;
    pfVar17[0x19] = 0.0;
    return pfVar8;
  case 0xcb:
    pfVar17[0x19] = 0.0;
    pfVar17[0x18] = 2.8026e-44;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = -60.0;
    pfVar17[0x32] = 0.0;
    if (*(int*)&pfVar17[1] != 2) {                      // SubType (DWORD)
      return (float*)(uintptr_t)*(int*)&pfVar17[1];
    }
    break;
  case 0xcc:
    pfVar17[0x18] = 8.26766e-44;
    pfVar17[0x19] = 0.0;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = -10.0;
    pfVar17[0x32] = 0.0;
    // BUG-FIX 2026-04-28: pass explicit (xf, yf) — effect pos at pfVar17[4]/[5]
    fVar24 = (float10)FUN_004f7500(pfVar17[4], pfVar17[5]);
    pfVar17[6] = (float)fVar24;
    break;
  case 0xcd:
    if ((pfVar17[1] == 0.0) || (*(int*)&pfVar17[1] == 3)) {
      pfVar17[0x18] = 1.4013e-45;
    }
    else {
      pfVar17[0x18] = 4.2039e-44;
      pfVar17[6] = pfVar17[6] + _DAT_0055297c;
      pfVar17[0x30] = 0.0;
      pfVar17[0x31] = -80.0;
      pfVar17[0x32] = 0.0;
      pfVar17[0x3a] = 1.0;
      pfVar17[0x3b] = 0.0;
      pfVar17[0x3c] = 0.0;
    }
    pfVar17[0x19] = 0.0;
    pfVar17[0x1a] = pfVar17[0x3a];
    pfVar17[3] = 1.3;
    *(undefined1 *)(pfVar17 + 0x1d) = 0x80;
    return pfVar8;
  case 0xd1:
    pfVar17[0x18] = 1.4013e-44;
    pfVar17[6] = pfVar17[6] + _DAT_00552980;
    Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
    local_6c = 0.0;
    local_68 = -60.0;
    local_64 = 0.0;
    pfVar12 = (float *)Vector_Rotate(&local_6c,local_3c + 3,pfVar17 + 0x30);
    return pfVar12;
  case 0xd2:
    pfVar17[6] = pfVar17[6] + _DAT_00552598;
  case 0xd3:
    pfVar17[6] = pfVar17[6] + _DAT_005524f0;
switchD_00461001_caseD_e2:
    if (*(int*)&pfVar17[1] == 5) {
      pfVar17[0x18] = 8.40779e-44;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x80000003;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
      }
      pfVar17[3] = (float)(int)(uVar10 + 8) * _DAT_005524f4;
      pfVar17[0x5c] = pfVar17[4];
      pfVar17[0x5d] = pfVar17[5];
      pfVar17[0x5e] = pfVar17[6];
      pfVar17[4] = *(float *)(*(int*)&pfVar17[0x3f] + 0x10);
      pfVar17[5] = *(float *)(*(int*)&pfVar17[0x3f] + 0x14);
      pfVar17[6] = ((float*)(uintptr_t)*(int*)&pfVar17[0x3f])[6];
      return (float*)(uintptr_t)*(int*)&pfVar17[0x3f];
    }
switchD_00461001_caseD_c7:
    if ((*(short *)((int)pfVar17 + 2) == 0xe2) || (*(short *)((int)pfVar17 + 2) == 0xe3)) {
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000007f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
      }
      param_8 = (float *)(uVar10 - 0x40);
      local_6c = (float)(int)param_8;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x8000007f;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
      }
      param_8 = (float *)(uVar10 - 0x40);
      local_68 = (float)(int)param_8;
      iVar9 = _rand();
      param_8 = (float *)(iVar9 % 0xb4);
      local_64 = (float)(int)param_8;
      pfVar17[4] = local_6c + pfVar17[4];
      pfVar17[5] = local_68 + pfVar17[5];
      pfVar17[6] = local_64 + pfVar17[6];
    }
    if (param_1 == 199) {
      pfVar17[0x19] = 0.0;
      pfVar17[0x1a] = 0.3;
      local_6c = 0.0;
      uVar10 = _rand();
      uVar10 = uVar10 & 0x800000ff;
      if ((int)uVar10 < 0) {
        uVar10 = (uVar10 - 1 | 0xffffff00) + 1;
      }
      param_8 = (float *)(uVar10 + 0x40);
      local_68 = (float)(int)param_8 * _DAT_005524f4;
      pfVar17[6] = pfVar17[6] + _DAT_00552598;
    }
    else {
      local_6c = 0.0;
      if (*(int*)&pfVar17[1] == 1) {
        local_68 = 0.0;
      }
      else {
        uVar10 = _rand();
        uVar10 = uVar10 & 0x800000ff;
        if ((int)uVar10 < 0) {
          uVar10 = (uVar10 - 1 | 0xffffff00) + 1;
        }
        param_8 = (float *)(uVar10 + 0x40);
        local_68 = (float)(int)param_8 * _DAT_005524f4;
      }
    }
    local_64 = 0.0;
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000000f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffff0) + 1;
    }
    *(int*)&pfVar17[0x18] = (int)(uVar10 + 0x20);   // lifetime: DWORD, no float (fix 2026-08-16)
    uVar10 = _rand();
    uVar10 = uVar10 & 0x80000003;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffffc) + 1;
    }
    param_8 = (float *)(uVar10 + 8);
    pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 0x168);
    pfVar17[9] = (float)(int)param_8;
    Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
    Vector_Rotate(&local_6c,local_3c + 3,pfVar17 + 0x30);
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000000f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffff0) + 1;
    }
    pfVar17[0x36] = (float)(int)(uVar10 + 8);
    return (float *)(uVar10 + 8);
  case 0xd4:
    Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
    local_6c = 0.0;
    local_68 = -40.0;
    local_64 = 150.0;
    pfVar12 = (float *)Vector_Rotate(&local_6c,local_3c + 3,&local_60);
    pfVar17[4] = local_60 + pfVar17[4];
    pfVar17[5] = local_5c + pfVar17[5];
    pfVar17[6] = local_58 + pfVar17[6];
    pfVar17[0x31] = -40.0;
    pfVar17[0x32] = 10.0;    pfVar17[0x18] = 2.8026e-44;
    pfVar17[3] = 1.2;
    return pfVar12;
  case 0xd7:
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000000f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffff0) + 1;
    }
    *(int*)&pfVar17[0x18] = (int)(uVar10 + 0x18);   // lifetime: DWORD, no float (fix 2026-08-16)
    uVar10 = _rand();
    uVar10 = uVar10 & 0x80000007;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffff8) + 1;
    }
    param_8 = (float *)(uVar10 + 6);
    pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
    uVar10 = _rand();
    param_8 = (float *)(uVar10 & 0x80000003);
    if ((int)param_8 < 0) {
      param_8 = (float *)(((int)param_8 - 1U | 0xfffffffc) + 1);
    }
    pfVar17[0x36] = -(float)(int)param_8;
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000003f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xffffffc0) + 1;
    }
    param_8 = (float *)(uVar10 - 0x20);
    local_6c = (float)(int)param_8;
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000001f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xffffffe0) + 1;
    }
    param_8 = (float *)(uVar10 + 0x32);
    local_68 = -(float)(int)param_8;
    uVar10 = _rand();
    uVar10 = uVar10 & 0x8000007f;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xffffff80) + 1;
    }
    param_8 = (float *)(uVar10 + 200);
    local_64 = (float)(int)param_8;
    Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
    pfVar12 = (float *)Vector_Rotate(&local_6c,local_3c + 3,&local_60);
    pfVar17[4] = local_60 + pfVar17[4];
    pfVar17[5] = local_5c + pfVar17[5];
    pfVar17[6] = local_58 + pfVar17[6];
    return pfVar12;
  case 0xd8:
  case 0xd9:
  case 0xda:
  case 0xdb:
  case 0xdc:
  case 0xdd:
  case 0xde:
  case 0xdf:
  case 0xe0:
    if (param_1 == 0xd8) {
      pfVar17[0x19] = 1.4013e-45;
    }
    else if (param_1 == 0xde) {
      *(int*)&pfVar17[0x19] = -2;
    }
    else if ((((param_1 == 0xda) || (param_1 == 0xdb)) || (param_1 == 0xdc)) ||
            ((param_1 == 0xe0 || (param_1 == 0xdf)))) {
      pfVar17[0x19] = 0.0;
    }
    pfVar12 = pfVar17 + 7;
    pfVar17[0x33] = 1.0;
    pfVar17[0x18] = 4.2039e-44;
    Matrix_BuildFromEuler(pfVar12,local_3c + 3);
    local_6c = -10.0;
    local_68 = -60.0;
    local_64 = 135.0;
    Vector_Rotate(&local_6c,local_3c + 3,&local_60);
    pfVar17[4] = local_60 + pfVar17[4];
    pfVar17[5] = local_5c + pfVar17[5];
    pfVar17[6] = local_58 + pfVar17[6];
    if (param_1 == 0xe0) {
      pfVar17[3] = 1.8;
      pfVar17[0x31] = -50.0;
      pfVar17[0x36] = -10.0;
    }
    else if (param_1 == 0xdf) {
      pfVar17[3] = 1.0;
      pfVar17[0x31] = -30.0;
      pfVar17[0x36] = -10.0;
      pfVar17[0x18] = 5.60519e-44;
    }
    else {
      pfVar17[3] = 0.8;
      pfVar17[0x31] = -70.0;
    }
    // IDA CreateEffect L685-693 (cases 216-224):
    //     if ( *((_DWORD *)i + 1) == 2 )        // SubType == 2 (flecha de Penetration)
    //     {
    //         if ( Type != 223 ) CreateEffect(255, ...);
    //         *((_DWORD *)i + 61) = 0;
     //        i[132] = 1;
    //     }
    // Este es el bloque que hace que la flecha de Penetration entre por el path A
    // de sub_466440 (que exige `Target + 132 != 0`) y por lo tanto reporte sus
    // blancos al server con el 0x1D.  El original compara el campo como DWORD;
    // el port lo hacia contra el denormal 2.8026e-45, o sea leyendo el slot como
    // float.  Se lee por BITS, igual que ya se hace en la linea 386 de este mismo
    // archivo (`*(int*)&pfVar17[1] != 4`).
    if (*(int*)&pfVar17[1] == 2) {
      if (param_1 != 0xdf) {
        Effect_Create(0xff,pfVar17 + 4,pfVar12,pfVar17 + 0x3a,(float *)0x0,pfVar17,
                     (float *)0xffffffff,(float *)0x0,0);
      }
      pfVar17[0x3d] = 0.0;
      *(undefined1 *)(pfVar17 + 0x21) = 1;
    }
    pfVar8 = (float*)DAT_07abf5d8;
    if ((param_1 == 0xd8) && (*(int*)&pfVar17[1] == 3)) {
      pfVar17[0x18] = 5.60519e-44;
      fVar27 = pfVar8[5];
      pfVar17[0x5c] = pfVar8[4];
      fVar2 = pfVar8[6];
      pfVar17[0x5d] = fVar27;
      pfVar17[0x5e] = fVar2;
      iVar9 = _rand();
      param_8 = (float *)(iVar9 % 0x1e);
      pfVar17[0x31] = -((float)(int)param_8 + _DAT_0055284c);
      iVar9 = _rand();
      param_8 = (float *)(iVar9 % 0x14);
      *pfVar12 = -((float)(int)param_8 + _DAT_00552844);
    }
    pfVar12 = (float*)DAT_07cf1ffc;
    param_5 = (float*)DAT_07cf1ffc;
    uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_00462bfb;
    goto LAB_00462b8a;
  case 0xe1:
    pfVar17[0x18] = 1.4013e-41;
    pfVar17[0x33] = 0.3;
    pfVar17[0x19] = 0.0;
    return pfVar8;
  case 0xe2:
  case 0xe3:
    goto switchD_00461001_caseD_e2;
  case 0xe4:
  case 0xe5:
  case 0xe6:
    pfVar17[0x18] = 1.4013e-43;
    iVar9 = _rand();
    pfVar17[3] = (float)(iVar9 % 10 + 4) * _DAT_005524f4;
    fVar27 = _DAT_00552988;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = fVar27 / pfVar17[3];
    pfVar17[0x32] = _DAT_00552984 / pfVar17[3];
    pfVar17[1] = 1.4013e-45;
    return (float *)(iVar9 / 10);
  case 0xe9:
    pfVar17[0x18] = 4.2039e-44;
    pfVar17[3] = 0.7;
    *(int*)&pfVar17[0x19] = -2;
    if (*(int*)&pfVar17[1] != 1) {
      pfVar17[0x33] = 0.1;
      return pfVar8;
    }
    pfVar17[0x18] = 2.8026e-44;
    pfVar17[0x33] = 0.1;
    pfVar17[0x16] = 0.0;
    return pfVar8;
  case 0xed:
    pfVar17[0x18] = 7.00649e-44;
    pfVar17[3] = 0.8;
    pfVar17[0x33] = 1.0;
    pfVar17[7] = 0.0;
    pfVar17[0x19] = 0.0;
    pfVar17[0x31] = -50.0;
    pfVar17[0x36] = -10.0;
    return pfVar8;
  case 0xee:
    pfVar17[0x18] = 7.00649e-45;
    param_8 = pfVar4;
    param_7 = pfVar4;
    uVar10 = (**(code **)(iVar9 + 0xc))(pfVar4);
    pfVar12 = (float *)0x0;
    param_8 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_0046320b;
    goto LAB_004631b2;
  case 0xef:
    pfVar17[0x18] = 3.50325e-44;
    param_5 = pfVar4;
    uVar10 = (**(code **)(iVar9 + 0xc))(pfVar4);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_0046371e;
    goto LAB_004636ad;
  case 0xf0:
    pfVar17[0x18] = 4.2039e-44;
    pfVar17[0x19] = 0.0;
    uVar10 = _rand();
    uVar10 = uVar10 & 0x80000007;
    if ((int)uVar10 < 0) {
      uVar10 = (uVar10 - 1 | 0xfffffff8) + 1;
    }
    param_8 = (float *)(uVar10 + 10);
    pfVar12 = pfVar17 + 4;
    pfVar17[3] = (float)(int)param_8 * _DAT_005524f4;
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 100 + 200);
    *pfVar12 = (float)(int)param_8 + *pfVar12;
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 100 + -0x32);
    pfVar17[5] = (float)(int)param_8 + pfVar17[5];
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 500 + 300);
    pfVar17[6] = (float)(int)param_8 + pfVar17[6];
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = 0.0;
    iVar9 = _rand();
    param_8 = (float *)(iVar9 % 0x32);
    pfVar17[0x32] = _DAT_0055298c - (float)(int)param_8;
    pfVar17[7] = 0.0;
    pfVar17[8] = 20.0;
    pfVar17[9] = 0.0;
    pfVar17[0x10] = *pfVar12;
    pfVar17[0x11] = pfVar17[5];
    pfVar17[0x12] = pfVar17[6];
    Joint_Create(0x4ea,pfVar12,pfVar12,pfVar17 + 7,5,(int)pfVar17,100.0,-1,0);
    pfVar12 = (float*)DAT_07cf1ffc;
    param_5 = (float*)DAT_07cf1ffc;
    uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_004611cb;
    goto LAB_0046115a;
  case 0xf1:
    *(int*)&pfVar17[0x19] = -2;
    pfVar17[3] = 0.9;
    pfVar17[0x33] = 0.5;
    // SubType es un DWORD: leer los BITS, no convertir el float (daba 0 siempre).
    switch(*(int*)&pfVar17[1]) {
    case 0:
      pfVar17[0x3a] = 0.8;
      pfVar17[0x3b] = 0.8;
      pfVar17[0x3c] = 0.8;
      pfVar17[0x18] = 2.10195e-44;
      break;
    case 1:
      pfVar17[0x3a] = 1.0;
      pfVar17[0x3b] = 0.5;
      pfVar17[0x3c] = 0.2;
      pfVar17[0x18] = 4.90454e-44;
      break;
    case 2:
      pfVar17[0x3a] = 0.8;
      pfVar17[0x3b] = 0.8;
      pfVar17[0x3c] = 0.8;
      pfVar17[0x18] = 1.68156e-44;
      pfVar17[0x16] = 0.0;
      pfVar17[3] = 0.5;
      break;
    case 3:
      pfVar17[0x3a] = 0.8;
      pfVar17[0x3b] = 0.8;
      pfVar17[0x3c] = 0.8;
      pfVar17[0x18] = 5.60519e-45;
      pfVar17[0x16] = 1.4013e-45;
      pfVar17[3] = 0.02;
      pfVar17[0x33] = 1.0;
      pfVar17[7] = 90.0;
      pfVar17[8] = 0.0;
      pfVar17[9] = 0.0;
      break;
    case 4:
      pfVar17[0x3a] = 0.8;
      pfVar17[0x3b] = 0.8;
      pfVar17[0x3c] = 0.8;
      pfVar17[0x18] = 4.90454e-44;
      pfVar17[0x16] = 1.4013e-45;
      pfVar17[3] = 0.1;
      pfVar17[0x36] = 0.0;
    }
    pfVar8 = (float*)DAT_07cf1ffc;
    // pfVar17[0x3f] (= slot+252) es el PUNTERO al owner: hay que reinterpretar
    // los bits, no convertir el float (eso destruia el puntero y la comparacion
    // con Hero nunca daba true).
    pfVar12 = (float*)(uintptr_t)*(unsigned int*)&pfVar17[0x3f];
    if (pfVar12 != (float*)DAT_07abf5d8) {
      return pfVar12;
    }
    if (1 < *(int*)&pfVar17[1]) {                      // SubType (DWORD)
      return pfVar12;
    }
    param_5 = (float*)DAT_07cf1ffc;
    uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_00461843;
    goto LAB_004617d3;
  case 0xf2:
    pfVar17[0x18] = 4.2039e-44;
    *(int*)&pfVar17[0x19] = -2;
    pfVar17[3] = 1.0;
    pfVar12 = pfVar17 + 4;
    pfVar17[6] = pfVar17[6] + _DAT_00552980;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = -70.0;
    pfVar17[0x32] = 0.0;
    pfVar17[0x10] = *pfVar12;
    pfVar17[0x11] = pfVar17[5];
    pfVar17[0x12] = pfVar17[6];
    Joint_Create(0x4ea,pfVar12,pfVar12,pfVar17 + 7,5,(int)pfVar17,100.0,-1,0);
    if (pfVar17[1] != 0.0) {
      Effect_Create(0xff,pfVar12,pfVar17 + 7,pfVar17 + 0x3a,(float *)0x0,pfVar17,(float *)0xffffffff,
                   (float *)0x0,0);
      pfVar17[0x3d] = 0.0;
      *(undefined1 *)(pfVar17 + 0x21) = 1;
    }
    pfVar12 = (float*)DAT_07cf1ffc;
    param_5 = (float*)DAT_07cf1ffc;
    uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 == 0) goto LAB_00461f5f;
    goto LAB_00461eee;
  case 0xf3:
    pfVar17[0x18] = 4.2039e-44;
    *(int*)&pfVar17[0x19] = -2;
    pfVar17[3] = 1.0;
    iVar9 = 0;
    pfVar17[6] = pfVar17[6] + _DAT_00552980;
    pfVar17[0x30] = 0.0;
    pfVar17[0x31] = -30.0;
    pfVar17[0x32] = 0.0;
    Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
    local_6c = -10.0;
    local_68 = -100.0;
    local_64 = 15.0;
    Vector_Rotate(&local_6c,local_3c + 3,&local_60);
    pfVar8 = pfVar17 + 0x5c;
    *pfVar8 = pfVar17[4];
    pfVar17[0x5d] = pfVar17[5];
    pfVar17[0x5e] = pfVar17[6];
    fVar27 = *pfVar8;
    *pfVar12 = 0.0;
    pfVar12[1] = 0.0;
    *pfVar8 = local_60 + fVar27;
    pfVar17[0x5d] = local_5c + pfVar17[0x5d];
    pfVar17[0x5e] = local_58 + pfVar17[0x5e];
    pfVar12[2] = pfVar17[9];
    do {
      Joint_Create(0x4e1,pfVar8,pfVar8,pfVar12,0xb,(int)pfVar17,50.0,-1,iVar9 == 1);
      pfVar14 = (float*)DAT_07cf1ffc;
      iVar9 = iVar9 + 1;
    } while (iVar9 < 4);
    param_5 = (float*)DAT_07cf1ffc;
    uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
    param_8 = (float *)0x0;
    param_7 = (float *)0x0;
    if (DAT_055c9bd4 != 0) {
      do {
        bVar22 = false;
        iVar15 = 0;
        bVar23 = true;
        param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
        iVar9 = 4;
        ppfVar18 = &param_8;
        pfVar12 = param_6;
        do {
          if (iVar9 == 0) break;
          iVar9 = iVar9 + -1;
          bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
          bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
          ppfVar18 = (float **)((int)ppfVar18 + 1);
          pfVar12 = (float *)((int)pfVar12 + 1);
        } while (bVar23);
        if (!bVar23) {
          iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
        }
        if (iVar15 == 0) goto LAB_004625aa;
        iVar9 = 4;
        bVar22 = false;
        iVar15 = 0;
        bVar23 = true;
        ppfVar18 = &param_4;
        pfVar12 = param_6;
        do {
          if (iVar9 == 0) break;
          iVar9 = iVar9 + -1;
          bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
          bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
          ppfVar18 = (float **)((int)ppfVar18 + 1);
          pfVar12 = (float *)((int)pfVar12 + 1);
        } while (bVar23);
        if (!bVar23) {
          iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
        }
        if (iVar15 == 0) {
          if (uVar10 == 0xffffffff) goto LAB_004625aa;
          param_6 = pfVar14;
          uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar14);
          param_8 = (float *)0x0;
          param_7 = (float *)0x0;
          if (DAT_055c9bd4 == 0) goto LAB_00462722;
          goto LAB_004626bb;
        }
        uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
        param_7 = (float *)((int)param_7 + 1);
        if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
      } while( true );
    }
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004625aa:
    pvVar7 = operator_new(0x585);
    *(undefined1 *)((int)pvVar7 + 0x584) = 1;
    FUN_00403f80(&DAT_055c9bc8,pvVar7,pfVar14);
    goto LAB_004625ca;
  case 0xf4:
    pfVar17[10] = pfVar17[7];
    pfVar17[0xb] = pfVar17[8];
    pfVar17[0xc] = pfVar17[9];
    pfVar17[0x5c] = pfVar17[4];
    pfVar17[0x5d] = pfVar17[5];
    pfVar17[0x5e] = pfVar17[6];
    pfVar17[0x18] = 2.8026e-44;
    iVar9 = _rand();
    pfVar17[1] = (float)(iVar9 % 100);
    pfVar17[9] = pfVar17[9] + _DAT_00552978;
    pfVar17[10] = pfVar17[10] + _DAT_00552878;
    pfVar17[0xc] = pfVar17[0xc] + _DAT_005524ec;
    pfVar17[0x36] = 50.0;
    break;
  case 0xf5:
    pfVar17[0x18] = 4.90454e-44;
    pfVar17[3] = (float)(int)(short)param_7 * _DAT_00552594;
    pfVar17[0x19] = 0.0;
    return pfVar8;
  }
  FUN_00423040(&DAT_055c9bc8,DAT_07cf1ffc);
  pfVar12 = (float*)DAT_07cf1ffc;
  cVar5 = *(char *)((char*)DAT_07cf1ffc + 0x160);
  *(char *)(pfVar17 + 0x22) = cVar5;
  *(char *)(pfVar12 + 0x160) = cVar5 + '\x01';
  pfVar12 = (float *)FUN_0043d1d0(&DAT_055c9bc8,pfVar12);
  return pfVar12;
switchD_00460f25_caseD_100:
  pfVar12 = pfVar17 + 7;
  pfVar17[0x33] = 1.0;
  pfVar17[0x18] = 4.2039e-44;
  Matrix_BuildFromEuler(pfVar12,local_3c + 3);
  local_6c = -10.0;
  local_68 = -60.0;
  local_64 = 135.0;
  Vector_Rotate(&local_6c,local_3c + 3,&local_60);
  param_4 = _param_9;
  param_9 = 0;
  param_8 = (float *)((uint)param_8 & 0xffffff00);
  param_7 = (float *)0xffffffff;
  *pfVar14 = local_60 + *pfVar14;
  param_1 = 0xff;
  pfVar17[5] = local_5c + pfVar17[5];
  pfVar17[6] = local_58 + pfVar17[6];
  pfVar17[3] = 0.8;
  pfVar17[0x31] = -70.0;
  param_2 = pfVar14;
  param_6 = pfVar17;
  goto LAB_00460dd8;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_00465011:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pfVar12 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00465094;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar12 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00465094;
      param_6 = pfVar4;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar4);
      param_7 = (float *)0x0;
      param_6 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_004651f7;
      goto LAB_00465190;
    }  }
LAB_00465082:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00465094:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = pfVar4;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar4);
  param_5 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_004652b0:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = pfVar4;
        goto LAB_004652dd;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_004652b0;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_6 = (float *)((int)param_6 + 1);
    } while ((uint)(uintptr_t)param_6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_004652dd:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_5 = pfVar12;
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      iVar9 = 4;
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = pfVar12;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
        param_5 = (float *)0x0;
        param_4 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00465448;
        goto LAB_004653e2;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_5 = (float *)((int)param_5 + 1);
    } while ((uint)(uintptr_t)param_5 < DAT_055c9bd4);
  }
  goto LAB_00465388;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_6 = (float *)((int)param_6 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_6) break;
LAB_00465190:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00465209;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00465209;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_0046520b;
    }
  }
LAB_004651f7:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00465209:
  puVar19 = (undefined4 *)0x0;
LAB_0046520b:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)((int)puVar11 + uVar10) =
             *(byte *)((int)puVar11 + uVar10) ^ *(byte *)((int)puVar11 + uVar10 + 1);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)((int)puVar11 + uVar10) =
           (PacketXorKey16[uVar16] ^ *(char *)((int)puVar11 + uVar10) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puStack_4 = *puVar19;
      puVar19 = puVar19 + 1;
      puStack_4 = puStack_4 + 1;
    }
    operator_delete((undefined *)puVar11);
  }
  goto LAB_004652dd;
LAB_004653e2:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_0046545a;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_0046545a;
    param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_0046546c;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) {
LAB_00465448:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_0046545a:
    param_4 = (float *)0x0;
LAB_0046546c:
    cVar5 = *(char *)(param_4 + 0x161) + -1;
    *(char *)(param_4 + 0x161) = cVar5;
    if (cVar5 != '\0') {
      return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    }
    pfVar14 = (float*)operator_new(0x584);
    pfVar8 = pfVar12;
    pfVar17 = pfVar14;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar17 = *pfVar8;
      pfVar8 = pfVar8 + 1;
      pfVar17 = pfVar17 + 1;
    }
    uVar10 = 0;
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar14);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {
        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
      }
      iVar9 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar14;
    pfVar8 = param_4;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar14); pfVar12 = NULL;
    return pfVar12;
  }
  goto LAB_004653e2;
LAB_004626bb:
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  iVar9 = 4;
  ppfVar18 = &param_8;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00462734;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_6;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar13;
    bVar23 = *(byte *)ppfVar18 == *pbVar13;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar13 = pbVar13 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_00462734;
    puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_00462736;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_7 = (float *)((int)param_7 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) goto LAB_00462722;
  goto LAB_004626bb;
LAB_00462722:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00462734:
  puVar19 = (undefined4 *)0x0;
LAB_00462736:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)((int)puVar11 + uVar10) =
             *(byte *)((int)puVar11 + uVar10) ^ *(byte *)((int)puVar11 + uVar10 + 1);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)((int)puVar11 + uVar10) =
           (PacketXorKey16[uVar16] ^ *(char *)((int)puVar11 + uVar10) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *unaff_retaddr = *puVar19;
      puVar19 = puVar19 + 1;
      unaff_retaddr = unaff_retaddr + 1;
    }
    operator_delete((undefined *)puVar11);
  }
LAB_004625ca:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_6 = pfVar12;
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 == 0) goto LAB_00465388;
  do {
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    ppfVar18 = &param_6;
    pfVar17 = pfVar8;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar17 = (float *)((int)pfVar17 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      return pfVar8;
    }
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar17 = pfVar8;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar17 = (float *)((int)pfVar17 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) {
        return pfVar8;
      }
      param_4 = pfVar12;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
      param_5 = (float *)0x0;
      param_4 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_00462884;
      break;
    }
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_5 = (float *)((int)param_5 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_5) goto LAB_00465388;
  } while( true );
LAB_0046281e:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00462896;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 != 0xffffffff) {
      param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_004628a8;
    }
    goto LAB_00462896;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) goto LAB_00462884;
  goto LAB_0046281e;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_00461eee:
    bVar22 = false;
    iVar15 = 0;    bVar23 = true;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00461f71;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00461f71;
      param_6 = pfVar12;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
      param_7 = (float *)0x0;
      param_6 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_004620d4;
      goto LAB_0046206d;
    }
  }
LAB_00461f5f:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00461f71:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = pfVar12;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
  param_5 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_0046218d:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = pfVar12;
        goto LAB_004621ba;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_0046218d;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_6 = (float *)((int)param_6 + 1);
    } while ((uint)(uintptr_t)param_6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_004621ba:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_5 = pfVar12;
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      iVar9 = 4;
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = pfVar12;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
        param_5 = (float *)0x0;
        param_4 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00462314;
        goto LAB_004622ae;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_5 = (float *)((int)param_5 + 1);
    } while ((uint)(uintptr_t)param_5 < DAT_055c9bd4);
  }
  goto LAB_00465388;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_6 = (float *)((int)param_6 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_6) break;
LAB_0046206d:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_004620e6;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_004620e6;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_004620e8;
    }
  }
LAB_004620d4:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004620e6:
  puVar19 = (undefined4 *)0x0;
LAB_004620e8:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)(uVar10 + (int)puVar11) =
             *(byte *)(uVar10 + (int)puVar11) ^ *(byte *)(uVar10 + 1 + (int)puVar11);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar10 + (int)puVar11) =
           (PacketXorKey16[uVar16] ^ *(char *)(uVar10 + (int)puVar11) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puStack_4 = *puVar19;
      puVar19 = puVar19 + 1;
      puStack_4 = puStack_4 + 1;
    }
    operator_delete((undefined *)puVar11);
  }
  goto LAB_004621ba;
LAB_004622ae:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00462326;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_00462326;
    param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_00462338;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) {
LAB_00462314:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_00462326:
    param_4 = (float *)0x0;
LAB_00462338:
    cVar5 = *(char *)(param_4 + 0x161) + -1;
    *(char *)(param_4 + 0x161) = cVar5;
    if (cVar5 != '\0') {
      return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    }
    pfVar14 = (float*)operator_new(0x584);
    pfVar8 = pfVar12;
    pfVar17 = pfVar14;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar17 = *pfVar8;
      pfVar8 = pfVar8 + 1;
      pfVar17 = pfVar17 + 1;
    }
    uVar10 = 0;
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar14);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {
        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
      }
      iVar9 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar14;
    pfVar8 = param_4;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar14); pfVar12 = NULL;
    return pfVar12;
  }
  goto LAB_004622ae;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_6 = (float *)((int)param_6 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_6) break;
LAB_00461951:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_004619ca;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_004619ca;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_004619cc;
    }
  }
LAB_004619b8:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004619ca:
  puVar19 = (undefined4 *)0x0;
LAB_004619cc:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)(uVar10 + (int)puVar11) =
             *(byte *)(uVar10 + (int)puVar11) ^ *(byte *)(uVar10 + 1 + (int)puVar11);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar10 + (int)puVar11) =
           (PacketXorKey16[uVar16] ^ *(char *)(uVar10 + (int)puVar11) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puStack_4 = *puVar19;
      puVar19 = puVar19 + 1;
      puStack_4 = puStack_4 + 1;
    }
    operator_delete((undefined *)puVar11);
  }
  goto LAB_00461a9e;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    _param_9 = (float *)((int)_param_9 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)_param_9) break;
LAB_00461ba9:
    iVar15 = 4;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    bVar22 = false;
    iVar9 = 0;
    bVar23 = true;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar15 == 0) break;
      iVar15 = iVar15 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar9 == 0) goto LAB_00461c23;
    iVar15 = 4;
    bVar22 = false;
    iVar9 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar15 == 0) break;
      iVar15 = iVar15 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar9 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00461c23;
      _param_9 = *(float **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_00461c25;
    }
  }
LAB_00461c11:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00461c23:
  _param_9 = (float *)0x0;
LAB_00461c25:
  cVar5 = *(char *)(_param_9 + 0x161);
  *(char *)(_param_9 + 0x161) = cVar5 + -1;
  if ((char)(cVar5 + -1) == '\0') {
    pfVar14 = (float*)operator_new(0x584);
    pfVar12 = param_8;
    pfVar8 = pfVar14;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    uVar10 = 0;
    param_8 = (float *)((int)param_8 - (int)pfVar14);
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar14);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
      }
      iVar9 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[(int)param_8] = (byte)iVar9;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar14;
    pfVar8 = _param_9;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar14);
  }
  goto LAB_00461ce3;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_0046115a:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_004611dd;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_004611dd;
      param_6 = pfVar12;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
      param_7 = (float *)0x0;
      param_6 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_00461340;
      goto LAB_004612d9;
    }
  }
LAB_004611cb:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004611dd:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = pfVar12;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
  param_5 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_004613f9:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = pfVar12;
        goto LAB_00461426;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_004613f9;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_6 = (float *)((int)param_6 + 1);
    } while ((uint)(uintptr_t)param_6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_00461426:
  pfVar12 = (float*)DAT_07cf1ffc;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_5 = pfVar12;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      iVar9 = 4;
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = pfVar12;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
        param_5 = (float *)0x0;
        param_4 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00461580;
        goto LAB_0046151a;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_5 = (float *)((int)param_5 + 1);
    } while ((uint)(uintptr_t)param_5 < DAT_055c9bd4);
  }
  goto LAB_00465388;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_6 = (float *)((int)param_6 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_6) break;
LAB_004612d9:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00461352;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00461352;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_00461354;
    }
  }
LAB_00461340:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00461352:
  puVar19 = (undefined4 *)0x0;
LAB_00461354:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)(uVar10 + (int)puVar11) =
             *(byte *)(uVar10 + (int)puVar11) ^ *(byte *)(uVar10 + 1 + (int)puVar11);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar10 + (int)puVar11) =
           (PacketXorKey16[uVar16] ^ *(char *)(uVar10 + (int)puVar11) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puStack_4 = *puVar19;
      puVar19 = puVar19 + 1;
      puStack_4 = puStack_4 + 1;
    }
    operator_delete((undefined *)puVar11);
  }
  goto LAB_00461426;
LAB_0046151a:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00461592;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_00461592;
    param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_004615a4;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) {
LAB_00461580:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_00461592:
    param_4 = (float *)0x0;
LAB_004615a4:
    cVar5 = *(char *)(param_4 + 0x161) + -1;
    *(char *)(param_4 + 0x161) = cVar5;
    if (cVar5 != '\0') {
      return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    }
    pfVar14 = (float*)operator_new(0x584);
    pfVar8 = pfVar12;
    pfVar17 = pfVar14;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar17 = *pfVar8;
      pfVar8 = pfVar8 + 1;
      pfVar17 = pfVar17 + 1;
    }
    uVar10 = 0;
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar14);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {
        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
      }
      iVar9 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar14;
    pfVar8 = param_4;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar14); pfVar12 = NULL;
    return pfVar12;
  }
  goto LAB_0046151a;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_004636ad:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pfVar12 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00463730;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar12 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00463730;
      param_6 = pfVar4;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar4);
      param_8 = (float *)0x0;
      param_7 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_004638a8;
      goto LAB_00463841;
    }
  }
LAB_0046371e:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00463730:
  pvVar7 = operator_new(0x585);
  *(undefined1 *)((int)pvVar7 + 0x584) = 1;
  FUN_00403f80(&DAT_055c9bc8,pvVar7,pfVar4);
  goto LAB_00463750;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_00463841:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_004638ba;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_004638ba;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_004638bc;
    }
  }
LAB_004638a8:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004638ba:
  puVar19 = (undefined4 *)0x0;
LAB_004638bc:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)(uVar10 + (int)puVar11) =
             *(byte *)(uVar10 + (int)puVar11) ^ *(byte *)(uVar10 + 1 + (int)puVar11);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar10 + (int)puVar11) =
           (PacketXorKey16[uVar16] ^ *(char *)(uVar10 + (int)puVar11) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {      *unaff_retaddr = *puVar19;
      puVar19 = puVar19 + 1;
      unaff_retaddr = unaff_retaddr + 1;
    }
    operator_delete((undefined *)puVar11);
  }
LAB_00463750:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_6 = pfVar12;
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = pfVar12;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
        param_5 = (float *)0x0;
        param_4 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00463a0a;
        goto LAB_004639a4;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_5 = (float *)((int)param_5 + 1);
    } while ((uint)(uintptr_t)param_5 < DAT_055c9bd4);
  }
  goto LAB_00465388;
LAB_004639a4:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00463a1c;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_00463a1c;
    param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_00463a2e;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) {
LAB_00463a0a:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_00463a1c:
    param_4 = (float *)0x0;
LAB_00463a2e:
    cVar5 = *(char *)(param_4 + 0x161) + -1;
    *(char *)(param_4 + 0x161) = cVar5;
    if (cVar5 != '\0') {
      return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    }
    pfVar14 = (float*)operator_new(0x584);
    pfVar8 = pfVar12;
    pfVar17 = pfVar14;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar17 = *pfVar8;
      pfVar8 = pfVar8 + 1;
      pfVar17 = pfVar17 + 1;
    }
    uVar10 = 0;
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar14);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {
        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
      }
      iVar9 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar14;
    pfVar8 = param_4;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar14); pfVar12 = NULL;
    return pfVar12;
  }
  goto LAB_004639a4;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    pfVar12 = (float *)((int)pfVar12 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)pfVar12) break;
LAB_004631b2:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_0046321d;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_0046321d;
      param_6 = param_7;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(param_7);
      pfVar12 = (float *)0x0;
      param_7 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_00463363;
      goto LAB_0046330a;
    }
  }
LAB_0046320b:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046321d:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = param_7;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(param_7);
  pfVar12 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar21 = pbVar13;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar21;
        bVar23 = *(byte *)ppfVar18 == *pbVar21;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar21 = pbVar21 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_0046341d:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = param_6;
        goto LAB_00463451;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_0046341d;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while ((uint)(uintptr_t)pfVar12 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_00463451:
  pfVar12 = (float*)DAT_07cf1ffc;
  param_6 = (float*)DAT_07cf1ffc;
  param_5 = (float*)DAT_07cf1ffc;
  *(char *)((char*)DAT_07cf1ffc + 0x160) = *(char *)((char*)DAT_07cf1ffc + 0x160) + '\x01';
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
  pfVar12 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = param_5;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(param_5);
        pfVar12 = (float *)0x0;
        param_5 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_0046358c;
        goto LAB_00463534;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while ((uint)(uintptr_t)pfVar12 < DAT_055c9bd4);
  }
  goto LAB_00465388;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    pfVar12 = (float *)((int)pfVar12 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)pfVar12) break;
LAB_0046330a:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00463375;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00463375;
      pfVar12 = *(float **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_00463377;
    }
  }
LAB_00463363:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00463375:
  pfVar12 = (float *)0x0;
LAB_00463377:
  cVar5 = *(char *)(pfVar12 + 0x161);
  *(byte *)(pfVar12 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    pfVar17 = (float*)operator_new(0x584);
    pfVar8 = pfVar17;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)(uVar10 + (int)pfVar17) =
             *(byte *)(uVar10 + (int)pfVar17) ^ *(byte *)(uVar10 + 1 + (int)pfVar17);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar10 + (int)pfVar17) =
           (PacketXorKey16[uVar16] ^ *(char *)(uVar10 + (int)pfVar17) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    pfVar12 = pfVar17;
    pfVar8 = param_6;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar17);
  }
  goto LAB_00463451;
LAB_00463534:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_0046359e;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_0046359e;
    param_5 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_004635a0;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  pfVar12 = (float *)((int)pfVar12 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)pfVar12) {
LAB_0046358c:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_0046359e:
    param_5 = (float *)0x0;
LAB_004635a0:
    cVar5 = *(char *)(param_5 + 0x161) + -1;
    *(char *)(param_5 + 0x161) = cVar5;
    if (cVar5 != '\0') {
      return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    }
    pfVar17 = (float*)operator_new(0x584);
    pfVar12 = param_4;
    pfVar8 = pfVar17;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    uVar10 = 0;
    iVar9 = (int)param_4 - (int)pfVar17;
    do {
      pbVar13 = (byte *)(uVar10 + (int)pfVar17);
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
      *pbVar13 = bVar6;
      if (uVar10 < 0x583) {
        *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar17) ^ bVar6;
      }
      iVar15 = _rand();
      uVar10 = uVar10 + 1;
      pbVar13[iVar9] = (byte)iVar15;
    } while (uVar10 < 0x584);
    pfVar12 = pfVar17;
    pfVar8 = param_5;    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *pfVar8 = *pfVar12;
      pfVar12 = pfVar12 + 1;
      pfVar8 = pfVar8 + 1;
    }
    operator_delete((undefined *)pfVar17); pfVar12 = NULL;
    return pfVar12;
  }
  goto LAB_00463534;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_6 = (float *)((int)param_6 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_6) break;
LAB_00462d09:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_7;
    pbVar21 = pbVar13;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar21;
      bVar23 = *(byte *)ppfVar18 == *pbVar21;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar21 = pbVar21 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00462d82;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_5;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *pbVar13;
      bVar23 = *(byte *)ppfVar18 == *pbVar13;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pbVar13 = pbVar13 + 1;
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00462d82;
      puVar19 = *(undefined4 **)(DAT_055c9bcc + uVar10 * 4);
      goto LAB_00462d84;
    }
  }
LAB_00462d70:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00462d82:
  puVar19 = (undefined4 *)0x0;
LAB_00462d84:
  cVar5 = *(char *)(puVar19 + 0x161);
  *(byte *)(puVar19 + 0x161) = cVar5 + 1U;
  if ((byte)(cVar5 + 1U) < 2) {
    puVar11 = (undefined4*)operator_new(0x584);
    puVar20 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar20 = *puVar19;
      puVar19 = puVar19 + 1;
      puVar20 = puVar20 + 1;
    }
    uVar10 = 0x583;
    iVar9 = 0x584;
    do {
      if (uVar10 < 0x583) {
        *(byte *)((int)puVar11 + uVar10) =
             *(byte *)((int)puVar11 + uVar10) ^ *(byte *)((int)puVar11 + uVar10 + 1);
      }
      uVar16 = uVar10 & 0x8000000f;
      if ((int)uVar16 < 0) {
        uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)((int)puVar11 + uVar10) =
           (PacketXorKey16[uVar16] ^ *(char *)((int)puVar11 + uVar10) - 0x23U) + 0xb9;
      uVar10 = uVar10 - 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar19 = puVar11;
    for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puStack_4 = *puVar19;
      puVar19 = puVar19 + 1;
      puStack_4 = puStack_4 + 1;
    }
    operator_delete((undefined *)puVar11);
  }
  goto LAB_00462e56;
LAB_00462f4a:
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
  ppfVar18 = &param_5;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) goto LAB_00462fc2;
  iVar9 = 4;
  bVar22 = false;
  iVar15 = 0;
  bVar23 = true;
  ppfVar18 = &param_3;
  pbVar21 = pbVar13;
  do {
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    bVar22 = *(byte *)ppfVar18 < *pbVar21;
    bVar23 = *(byte *)ppfVar18 == *pbVar21;
    ppfVar18 = (float **)((int)ppfVar18 + 1);
    pbVar21 = pbVar21 + 1;
  } while (bVar23);
  if (!bVar23) {
    iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
  }
  if (iVar15 == 0) {
    if (uVar10 == 0xffffffff) goto LAB_00462fc2;
    param_4 = *(float **)(DAT_055c9bcc + uVar10 * 4);
    goto LAB_00462fd4;
  }
  uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
  param_4 = (float *)((int)param_4 + 1);
  if (DAT_055c9bd4 <= (uint)(uintptr_t)param_4) {
LAB_00462fb0:
    FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_00462fc2:
    param_4 = (float *)0x0;
LAB_00462fd4:
    cVar5 = *(char *)(param_4 + 0x161) + -1;
    pfVar8 = (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
    *(char *)(param_4 + 0x161) = cVar5;
    if (cVar5 == '\0') {
      pfVar14 = (float*)operator_new(0x584);
      pfVar8 = pfVar12;
      pfVar17 = pfVar14;
      for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
        *pfVar17 = *pfVar8;
        pfVar8 = pfVar8 + 1;
        pfVar17 = pfVar17 + 1;
      }
      uVar10 = 0;
      do {
        pbVar13 = (byte *)(uVar10 + (int)pfVar14);
        uVar16 = uVar10 & 0x8000000f;
        if ((int)uVar16 < 0) {
          uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
        }
        bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
        *pbVar13 = bVar6;
        if (uVar10 < 0x583) {
          *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
        }
        iVar9 = _rand();
        uVar10 = uVar10 + 1;
        pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
      } while (uVar10 < 0x584);
      pfVar12 = pfVar14;
      pfVar8 = param_4;
      for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
        *pfVar8 = *pfVar12;
        pfVar12 = pfVar12 + 1;
        pfVar8 = pfVar8 + 1;
      }
      operator_delete((undefined *)pfVar14); pfVar12 = NULL;
      return pfVar12;
    }
switchD_00460f25_caseD_101:
    return pfVar8;
  }
  goto LAB_00462f4a;
switchD_00465549_caseD_4ba:
  // 2026-09-03 (Aqua Beam desplazado): el vector de avance de la estela de los
  // tipos 1210/1211/1212 sale de `VectorRotate(in1, in2, (float *)i + 48)` con
  // IDA poniendo `in1 = (0, -50, 0)` (o `(0, -15, 0)` para el 1212).  El port
  // escribia `local_6c = fVar27; local_64 = fVar27;` -- o sea X y Z tomaban un
  // valor sobrante de otra rama de la funcion en vez de 0.  El campo +0xC0 es
  // el paso que `RenderEffects` usa para los 30 sprites 1176 de la estela, asi
  // que la estela avanzaba en una direccion arbitraria: el Aqua Beam nacia bien
  // pero se dibujaba corrido.
  pfVar17[0x18] = 2.8026e-44;
  if (param_1 == 0x4ba) {
    pfVar17[0x3a] = 0.5;
    pfVar17[0x3b] = 0.7;
    pfVar17[0x3c] = 1.0;
  }
  else {
    if (param_1 != 0x4bb) {
      if (param_1 == 0x4bc) {
        local_6c = 0.0f;   // IDA: in1[0] = 0.0
        local_64 = 0.0f;   // IDA: in1[2] = 0.0
        pfVar17[0x3a] = 1.0;
        pfVar17[0x3b] = 0.4;
        pfVar17[0x3c] = 0.2;
        local_68 = -15.0;
        pfVar17[3] = 5.0;
      }
      goto LAB_00465641;
    }
    pfVar17[0x3a] = 1.0;
    pfVar17[0x3b] = 0.4;
    pfVar17[0x3c] = 0.2;
  }
  local_6c = 0.0f;   // IDA: in1[0] = 0.0
  local_64 = 0.0f;   // IDA: in1[2] = 0.0
  local_68 = -50.0;
  pfVar17[3] = 16.0;
LAB_00465641:
  Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
  Vector_Rotate(&local_6c,local_3c + 3,pfVar17 + 0x30);
  if ((float*)(uintptr_t)*(int*)&pfVar17[0x3f] != (float*)DAT_07abf5d8) {
    return (float*)DAT_07abf5d8;
  }
  fVar27 = pfVar17[5];
  fVar2 = pfVar17[6];
  *param_2 = pfVar17[4];
  param_2[1] = fVar27;
  param_2[2] = fVar2;
  local_6c = 0.0;
  local_68 = -150.0;
  local_64 = 0.0;
  Matrix_BuildFromEuler(pfVar17 + 7,local_3c + 3);
  Vector_Rotate(&local_6c,local_3c + 3,&local_48);
  iVar9 = 4;
  do {
    sVar28 = *(short *)((int)pfVar17 + 0x86);
    bVar6 = *(byte *)(pfVar17 + 0x22);
    bVar1 = *(byte *)((int)pfVar17 + 0x85);
    *param_2 = local_48 + *param_2;
    param_2[1] = local_44 + param_2[1];
    param_2[2] = local_40 + param_2[2];
    pfVar12 = (float *)FUN_0045fec0((uint)bVar1,param_2,150.0,bVar6,sVar28);
    iVar9 = iVar9 + -1;
  } while (iVar9 != 0);
  return pfVar12;
LAB_00462884:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pbVar13 = NULL;
LAB_00462896:
  param_4 = (float *)0x0;
LAB_004628a8:
  cVar5 = *(char *)(param_4 + 0x161) + -1;
  *(char *)(param_4 + 0x161) = cVar5;
  if (cVar5 != '\0') {
    return (float *)CONCAT31(((uint)pbVar13 >> 8),cVar5);
  }
  pfVar14 = (float*)operator_new(0x584);
  pfVar8 = pfVar12;
  pfVar17 = pfVar14;
  for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
    *pfVar17 = *pfVar8;
    pfVar8 = pfVar8 + 1;
    pfVar17 = pfVar17 + 1;
  }
  uVar10 = 0;
  do {
    pbVar13 = (byte *)(uVar10 + (int)pfVar14);
    uVar16 = uVar10 & 0x8000000f;
    if ((int)uVar16 < 0) {
      uVar16 = (uVar16 - 1 | 0xfffffff0) + 1;
    }
    bVar6 = (PacketXorKey16[uVar16] ^ *pbVar13 + 0x47) + 0x23;
    *pbVar13 = bVar6;
    if (uVar10 < 0x583) {
      *pbVar13 = *(byte *)(uVar10 + 1 + (int)pfVar14) ^ bVar6;
    }
    iVar9 = _rand();
    uVar10 = uVar10 + 1;
    pbVar13[(int)pfVar12 - (int)pfVar14] = (byte)iVar9;
  } while (uVar10 < 0x584);
  pfVar12 = pfVar14;
  pfVar8 = param_4;
  for (iVar9 = 0x161; iVar9 != 0; iVar9 = iVar9 + -1) {
    *pfVar8 = *pfVar12;
    pfVar12 = pfVar12 + 1;
    pfVar8 = pfVar8 + 1;
  }
  operator_delete((undefined *)pfVar14); pfVar12 = NULL;
  return pfVar12;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_004617d3:
    iVar15 = 4;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    bVar22 = false;
    iVar9 = 0;
    bVar23 = true;
    ppfVar18 = &param_8;
    pfVar12 = param_6;
    do {
      if (iVar15 == 0) break;
      iVar15 = iVar15 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar9 == 0) goto LAB_00461855;
    iVar15 = 4;
    bVar22 = false;
    iVar9 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar12 = param_6;
    do {
      if (iVar15 == 0) break;
      iVar15 = iVar15 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar12;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar12;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar12 = (float *)((int)pfVar12 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar9 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00461855;
      param_6 = pfVar8;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar8);
      param_7 = (float *)0x0;
      param_6 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_004619b8;
      goto LAB_00461951;
    }
  }
LAB_00461843:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00461855:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = pfVar8;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar8);
  param_5 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_00461a71:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = pfVar8;
        goto LAB_00461a9e;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_00461a71;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_6 = (float *)((int)param_6 + 1);
    } while ((uint)(uintptr_t)param_6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_00461a9e:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  param_6 = (float*)DAT_07cf1ffc;
  param_3 = (float*)DAT_07cf1ffc;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_7 = (float *)0x0;
  _param_9 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar9 = 0;
      bVar23 = true;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      iVar15 = 4;
      ppfVar18 = &param_7;
      pbVar21 = pbVar13;
      do {
        if (iVar15 == 0) break;
        iVar15 = iVar15 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar21;
        bVar23 = *(byte *)ppfVar18 == *pbVar21;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar21 = pbVar21 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar9 == 0) goto LAB_00461ce3;
      iVar15 = 4;
      bVar22 = false;
      iVar9 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      do {
        if (iVar15 == 0) break;
        iVar15 = iVar15 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar9 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar9 == 0) {
        if (uVar10 == 0xffffffff) goto LAB_00461ce3;
        param_5 = param_8;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(param_8);
        param_7 = (float *)0x0;
        _param_9 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00461c11;
        goto LAB_00461ba9;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      _param_9 = (float *)((int)_param_9 + 1);
    } while ((uint)(uintptr_t)_param_9 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00461ce3:
  sVar28 = *(short *)((int)pfVar17 + 0x86);
  bVar6 = *(byte *)(pfVar17 + 0x22);
  fVar27 = 400.0;
LAB_00463ca2:
  pfVar12 = (float *)FUN_0045fec0((uint)*(byte *)((int)pfVar17 + 0x85),pfVar17 + 4,fVar27,bVar6,
                                  sVar28);
  return pfVar12;
  while( true ) {
    uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
    param_7 = (float *)((int)param_7 + 1);
    if (DAT_055c9bd4 <= (uint)(uintptr_t)param_7) break;
LAB_00462b8a:
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    param_6 = (float *)(DAT_055c9bd0 + uVar10 * 4);
    iVar9 = 4;
    ppfVar18 = &param_8;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);
    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) goto LAB_00462c0d;
    iVar9 = 4;
    bVar22 = false;
    iVar15 = 0;
    bVar23 = true;
    ppfVar18 = &param_4;
    pfVar8 = param_6;
    do {
      if (iVar9 == 0) break;
      iVar9 = iVar9 + -1;
      bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar8;
      bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar8;
      ppfVar18 = (float **)((int)ppfVar18 + 1);
      pfVar8 = (float *)((int)pfVar8 + 1);
    } while (bVar23);    if (!bVar23) {
      iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
    }
    if (iVar15 == 0) {
      if (uVar10 == 0xffffffff) goto LAB_00462c0d;
      param_6 = pfVar12;
      uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
      param_7 = (float *)0x0;
      param_6 = (float *)0x0;
      if (DAT_055c9bd4 == 0) goto LAB_00462d70;
      goto LAB_00462d09;
    }
  }
LAB_00462bfb:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00462c0d:
  param_4 = (float*)operator_new(0x585);
  *(undefined1 *)(param_4 + 0x161) = 1;
  param_8 = pfVar12;
  uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
  param_5 = (float *)0x0;
  param_6 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      iVar9 = 4;
      ppfVar18 = &param_7;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
LAB_00462e29:
        *(float **)(DAT_055c9bcc + uVar10 * 4) = param_3;
        *(float **)(DAT_055c9bd0 + uVar10 * 4) = pfVar12;
        goto LAB_00462e56;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_5;
      pbVar13 = (byte *)(DAT_055c9bd0 + uVar10 * 4);
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *pbVar13;
        bVar23 = *(byte *)ppfVar18 == *pbVar13;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pbVar13 = pbVar13 + 1;
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) goto LAB_00462e29;
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_6 = (float *)((int)param_6 + 1);
    } while ((uint)(uintptr_t)param_6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
LAB_00462e56:
  pfVar12 = (float*)DAT_07cf1ffc;
  iVar9 = DAT_055c9bc8;
  *(undefined1 *)(pfVar17 + 0x22) = *(undefined1 *)((char*)DAT_07cf1ffc + 0x160);
  param_5 = pfVar12;
  uVar10 = (**(code **)(iVar9 + 0xc))(pfVar12);
  param_6 = (float *)0x0;
  param_5 = (float *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      pfVar8 = (float *)(DAT_055c9bd0 + uVar10 * 4);
      iVar9 = 4;
      ppfVar18 = &param_6;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        return pfVar8;
      }
      iVar9 = 4;
      bVar22 = false;
      iVar15 = 0;
      bVar23 = true;
      ppfVar18 = &param_4;
      pfVar17 = pfVar8;
      do {
        if (iVar9 == 0) break;
        iVar9 = iVar9 + -1;
        bVar22 = *(byte *)ppfVar18 < *(byte *)pfVar17;
        bVar23 = *(byte *)ppfVar18 == *(byte *)pfVar17;
        ppfVar18 = (float **)((int)ppfVar18 + 1);
        pfVar17 = (float *)((int)pfVar17 + 1);
      } while (bVar23);
      if (!bVar23) {
        iVar15 = (1 - (uint)bVar22) - (uint)(bVar22 != 0);
      }
      if (iVar15 == 0) {
        if (uVar10 == 0xffffffff) {
          return pfVar8;
        }
        param_4 = pfVar12;
        uVar10 = (**(code **)(DAT_055c9bc8 + 0xc))(pfVar12);
        param_5 = (float *)0x0;
        param_4 = (float *)0x0;
        if (DAT_055c9bd4 == 0) goto LAB_00462fb0;
        goto LAB_00462f4a;
      }
      uVar10 = (uVar10 + 1) % (uint)DAT_055c9bd4;
      param_5 = (float *)((int)param_5 + 1);
    } while ((uint)(uintptr_t)param_5 < DAT_055c9bd4);
  }
LAB_00465388:
  param_6 = (float *)0x0;
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108); pfVar12 = NULL;
  return pfVar12;
}

// IDA compatibility bridge: stubs_IDA_ports.cpp intentionally retains this ABI name.
float* __cdecl FUN_00460dc0(int type, float* p1, float* p2, float* p3, float* p4,
                            float* p5, float* p6, float* p7, byte flag)
{
  return Effect_Create(type, p1, p2, p3, p4, p5, p6, p7, flag);
}
