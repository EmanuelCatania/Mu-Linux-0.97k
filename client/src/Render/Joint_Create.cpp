// Joint_Create.cpp
// Joint_Create @ 0x0046d840
//
// CreateJoint - creates a lightning/beam visual joint by type.

#include "stdafx.h"

extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// IDA: FUN_0046d840
void * __cdecl
Joint_Create(int param_1,float *param_2,float *param_3,float *param_4,undefined4 param_5,int param_6
            ,float param_7,short param_8,byte param_9)

{
  // [DIAG FORGE] entry-point CreateJoint (a = param_7)
  if (param_2)
      DbgForge("CreateJoint", param_1, -1, -1, -1, -1, param_6,
               param_2[0], param_2[1], param_2[2], 0.0f, 0.0f, 0.0f, param_7);
  float *pfVar1;
  float *pfVar2;
  float *pfVar3;
  char cVar4;
  byte bVar5;
  int iVar6;
  undefined4 *puVar7;
  byte *pbVar8;
  uint uVar9;
  byte *pbVar10;
  int iVar11;
  uint uVar12;
  undefined4 *unaff_EBX;
  undefined4 *puVar13;
  char *pcVar14;
  byte **ppbVar15;
  undefined4 *puVar16;
  byte *pbVar17;
  bool bVar18;
  bool bVar19;
  float10 fVar20;
  float10 fVar21;
  float *pfVar22;
  undefined4 uVar23;
  float fVar24;
  undefined4 uVar25;
  byte *pbVar26;
  // 2026-08-10 FIX (haces oscuros de las alas) — tercera instancia del patrón
  // "locales que Ghidra separó y el código asume contiguos". Acá el código hace:
  //     local_90 = Scale * -0.5f;  local_8c = 0;  local_88 = 0;
  //     Vector_Rotate(&local_90, local_6c, &local_78);   // in y out son vec3
  //     *(float*)(slot+0x58) = local_78 + Position.x;   // out[0]
  //     *(float*)(slot+0x5c) = local_74 + Position.y;   // out[1]
  //     *(float*)(slot+0x60) = local_70 + Position.z;   // out[2]
  // o sea depende de que {local_90,local_8c,local_88} y {local_78,local_74,
  // local_70} sean tríos contiguos del frame. Con locales sueltos sólo out[0]
  // caía donde el código lo lee → la esquina X del segmento salía bien y la Y/Z
  // quedaban con basura, que es justo lo que mostró el probe JOINTDBG
  // (v0=(13480.6, 1.08e9, 3.86e10)).
  // Varios de estos slots son dual-use (float o byte*), así que se respalda todo
  // con un bloque de bytes y los nombres quedan como referencias tipadas al
  // offset correcto: ebp-0x98 → __fr[0x00] … ebp-0x3C → __fr[0x5C], total 0x98.
  alignas(8) unsigned char __fr[0x98] = {0};
  byte *&local_98 = *(byte **)(__fr + 0x00);
  byte *&local_94 = *(byte **)(__fr + 0x04);
  float &local_90 = *(float  *)(__fr + 0x08);
  byte *&local_8c = *(byte **)(__fr + 0x0c);   // dual-use: float coord OR ptr
  byte *&local_88 = *(byte **)(__fr + 0x10);   // dual-use: float coord OR ptr
  byte *&local_84 = *(byte **)(__fr + 0x14);
  byte *&local_80 = *(byte **)(__fr + 0x18);
  byte *&local_7c = *(byte **)(__fr + 0x1c);
  float &local_78 = *(float  *)(__fr + 0x20);
  float &local_74 = *(float  *)(__fr + 0x24);
  float &local_70 = *(float  *)(__fr + 0x28);
  float *const local_6c = (float *)(__fr + 0x2c);   // 12 floats
  float *const local_3c = (float *)(__fr + 0x5c);   // 15 floats
  // 2026-08-10 — vistas FLOAT de los slots que Ghidra tipó como `byte*`.
  // Ghidra los llamó punteros, pero cuando forman el vec3 de Vector_Rotate
  // guardan FLOATS. El port los leía con `local_8cf`, que
  // CONVIERTE el valor entero en vez de reinterpretar los bits: un 2.16
  // (bits 0x400B0000 = 1074413568) salía como 1074413568.0 ≈ 1.07e9 — la
  // causa exacta de los haces (x usaba local_90, que sí era float, y salía
  // sana; y/z pasaban por esta conversión y explotaban).
  float &local_94f = *(float *)(__fr + 0x04);
  float &local_8cf = *(float *)(__fr + 0x0c);
  float &local_88f = *(float *)(__fr + 0x10);
  // Ghidra ghost stack-slot variables (used as byte* for 4-byte comparisons)
  undefined4 stack0xffffff60 = 0;
  undefined4 stack0xffffff64 = 0;
  undefined4 stack0xffffff5c = 0;

  pcVar14 = DAT_07b27150;
  const uintptr_t __jp_end = (uintptr_t)DAT_07b27150 + sizeof(DAT_07b27150);
  while (*pcVar14 != '\0') {
    pcVar14 = pcVar14 + 0x9d8;
    if ((uintptr_t)pcVar14 >= __jp_end) {
      return nullptr;
    }
  }
  *pcVar14 = '\x01';
  *(int *)(pcVar14 + 4) = param_1;
  *(undefined4 *)(pcVar14 + 8) = param_5;
  pcVar14[0x9bc] = '\0';
  *(short *)(pcVar14 + 0x9d0) = param_8;
  pcVar14[0x9d2] = param_9;
  pcVar14[0x9c0] = '\0';
  pcVar14[0x9c1] = '\0';
  pcVar14[0x9c2] = '\0';
  pcVar14[0x9c3] = '\0';
  pcVar14[0x40] = '\0';
  pcVar14[0x41] = '\0';
  pcVar14[0x42] = '\0';
  pcVar14[0x43] = '\0';
  pfVar1 = (float *)(pcVar14 + 0x10);
  pfVar2 = (float *)(pcVar14 + 0x28);
  *pfVar1 = *param_2;
  *(float *)(pcVar14 + 0x14) = param_2[1];
  *(float *)(pcVar14 + 0x18) = param_2[2];
  *pfVar2 = *param_4;
  *(float *)(pcVar14 + 0x2c) = param_4[1];
  *(float *)(pcVar14 + 0x30) = param_4[2];
  pfVar3 = (float *)(pcVar14 + 0x34);
  *pfVar3 = 1.0;
  pcVar14[0x38] = '\0';
  pcVar14[0x39] = '\0';
  pcVar14[0x3a] = -0x80;
  pcVar14[0x3b] = '?';
  pcVar14[0x3c] = '\0';
  pcVar14[0x3d] = '\0';
  pcVar14[0x3e] = -0x80;
  pcVar14[0x3f] = '?';
  if (param_6 == 0) {
    *(float *)(pcVar14 + 0x44) = *param_3;
    *(float *)(pcVar14 + 0x48) = param_3[1];
    *(float *)(pcVar14 + 0x4c) = param_3[2];
  }
  else {
    if ((param_1 == 0x10a) && (*(int *)(pcVar14 + 8) == 2)) {
      *(float *)(pcVar14 + 0x44) = *param_3;
      *(float *)(pcVar14 + 0x48) = param_3[1];
      *(float *)(pcVar14 + 0x4c) = param_3[2];
    }
    *(int *)(pcVar14 + 0x40) = param_6;
  }
  pcVar14[0x50] = '\0';
  pcVar14[0x51] = '\0';
  pcVar14[0x52] = '\0';
  pcVar14[0x53] = '\0';
  Matrix_BuildFromEuler(pfVar2,local_6c);
  local_90 = *(float *)(pcVar14 + 0xc) * _DAT_00552a14;
  local_8c = (byte*)0x0;
  local_88 = (byte*)0x0;
  Vector_Rotate(&local_90,local_6c,&local_78);
  *(float *)(pcVar14 + 0x58) = local_78 + *pfVar1;
  local_8c = (byte*)0x0;
  local_88 = (byte*)0x0;
  *(float *)(pcVar14 + 0x5c) = local_74 + *(float *)(pcVar14 + 0x14);
  *(float *)(pcVar14 + 0x60) = local_70 + *(float *)(pcVar14 + 0x18);
  local_90 = *(float *)(pcVar14 + 0xc) * _DAT_00552504;
  Vector_Rotate(&local_90,local_6c,&local_78);
  *(float *)(pcVar14 + 100) = local_78 + *pfVar1;
  local_90 = 0.0f;
  local_8c = (byte*)0x0;
  *(float *)(pcVar14 + 0x68) = local_74 + *(float *)(pcVar14 + 0x14);
  *(float *)(pcVar14 + 0x6c) = local_70 + *(float *)(pcVar14 + 0x18);
  local_88f = *(float *)(pcVar14 + 0xc) * _DAT_00552a14;
  Vector_Rotate(&local_90,local_6c,&local_78);
  *(float *)(pcVar14 + 0x70) = local_78 + *pfVar1;
  local_90 = 0.0f;
  local_8c = (byte*)0x0;
  *(float *)(pcVar14 + 0x74) = local_74 + *(float *)(pcVar14 + 0x14);
  *(float *)(pcVar14 + 0x78) = local_70 + *(float *)(pcVar14 + 0x18);
  local_88f = *(float *)(pcVar14 + 0xc) * _DAT_00552504;
  Vector_Rotate(&local_90,local_6c,&local_78);
  *(float *)(pcVar14 + 0x7c) = local_78 + *pfVar1;
  *(float *)(pcVar14 + 0x80) = local_74 + *(float *)(pcVar14 + 0x14);
  *(float *)(pcVar14 + 0x84) = local_70 + *(float *)(pcVar14 + 0x18);
  pbVar10 = (byte*)DAT_07cf1ffc;
  // 0046D840, Type 1253 / SubType 0 (Evil Spirit): this is the joint
  // initialisation that feeds the per-frame 00470030 branch.  Without these
  // exact counters the slot is allocated but expires as an unconfigured beam.
  if (param_1 == 1253 && *(int *)(pcVar14 + 8) == 0) {
    pcVar14[0x9d3] = pbVar10 ? pbVar10[1408] : 0; // CharacterMachine+1408
    *(float *)(pcVar14 + 0x9c0) = 70.0f;
    *(int *)(pcVar14 + 0x9b8) = 49;
    *(float *)(pcVar14 + 0x0c) = param_7;
    *(int *)(pcVar14 + 0x54) = 6;
  }
  if (param_1 < 0x4e9) {
    if (param_1 == 0x4e8) {
      fVar24 = *(float *)(pcVar14 + 0x4c) + _DAT_00552980;
      pcVar14[0xc] = '\0';
      pcVar14[0xd] = '\0';
      pcVar14[0xe] = -0x74;
      pcVar14[0xf] = 'B';
      pcVar14[0x9c0] = '\0';
      pcVar14[0x9c1] = '\0';
      pcVar14[0x9c2] = 'H';
      pcVar14[0x9c3] = 'B';
      pcVar14[0x54] = '\b';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x9b8] = '\x14';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      *(float *)(pcVar14 + 0x4c) = fVar24;
      *(undefined4 *)(pcVar14 + 0x44) = *(undefined4 *)(pcVar14 + 0x44);
      *(undefined4 *)(pcVar14 + 0x48) = *(undefined4 *)(pcVar14 + 0x48);
      goto switchD_0046dee7_default;
    }
    if (param_1 < 0x4e3) {
      if (param_1 != 0x4e2) {
        if (0x498 < param_1) {
          if (param_1 != 0x4e1) goto switchD_0046dee7_default;
          goto switchD_0046f0e4_caseD_4fd;
        }
        if (param_1 == 0x498) {
          pcVar14[0x9b8] = 'd';
          pcVar14[0x9b9] = '\0';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          pcVar14[0x54] = '\x14';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          pcVar14[0xc] = '\0';
          pcVar14[0xd] = '\0';
          pcVar14[0xe] = ' ';
          pcVar14[0xf] = 'A';
          if (*(int *)(pcVar14 + 8) == 0) {
            iVar6 = _rand();
            local_94 = (byte *)(iVar6 % 0x14 + 0x23);
            *(float *)(pcVar14 + 0x9cc) = (float)(int)local_94;
            iVar6 = _rand();
            pcVar14[0x9b8] = '\x19';
            pcVar14[0x9b9] = '\0';
            pcVar14[0x9ba] = '\0';
            pcVar14[0x9bb] = '\0';
            *(float *)(pcVar14 + 0xc) = (float)(iVar6 % 0x14) + _DAT_005524fc;
          }
          *(float *)(pcVar14 + 0x44) = *pfVar1;
          *pfVar3 = 1.0;
          pcVar14[0x38] = '\0';
          pcVar14[0x39] = '\0';
          pcVar14[0x3a] = -0x80;
          pcVar14[0x3b] = '?';
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = -0x80;
          pcVar14[0x3f] = '?';
          *(undefined4 *)(pcVar14 + 0x48) = *(undefined4 *)(pcVar14 + 0x14);
          *(undefined4 *)(pcVar14 + 0x4c) = *(undefined4 *)(pcVar14 + 0x18);
          goto switchD_0046dee7_default;
        }
        if (param_1 != 0x10a) {
          if (param_1 != 0x48e) goto switchD_0046dee7_default;
          goto switchD_0046f0e4_caseD_4ed;
        }
        iVar6 = *(int *)(pcVar14 + 0x40);
        *(undefined4 *)(pcVar14 + 0x44) = *(undefined4 *)(iVar6 + 0x10);
        *(undefined4 *)(pcVar14 + 0x48) = *(undefined4 *)(iVar6 + 0x14);
        *(undefined4 *)(pcVar14 + 0x4c) = *(undefined4 *)(iVar6 + 0x18);
        switch(*(undefined4 *)(pcVar14 + 8)) {
        case 0:
          *pfVar3 = 1.0;
          pcVar14[0x38] = '\0';
          pcVar14[0x39] = '\0';
          pcVar14[0x3a] = -0x80;
          pcVar14[0x3b] = '?';
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = -0x80;
          pcVar14[0x3f] = '?';
          break;
        case 1:
          *pfVar3 = 0.2;
          pcVar14[0x38] = -0x33;
          pcVar14[0x39] = -0x34;
          pcVar14[0x3a] = 'L';
          pcVar14[0x3b] = '>';
          pcVar14[0x3c] = -0x33;
          pcVar14[0x3d] = -0x34;
          pcVar14[0x3e] = 'L';
          pcVar14[0x3f] = '>';
          break;
        case 2:
          *(float *)(pcVar14 + 0x44) = *param_3;
          *(float *)(pcVar14 + 0x48) = param_3[1];
          *(float *)(pcVar14 + 0x4c) = param_3[2];
          *pfVar3 = 1.0;
          pcVar14[0x38] = -0x66;
          pcVar14[0x39] = -0x67;
          pcVar14[0x3a] = -0x67;
          pcVar14[0x3b] = '>';
          pcVar14[0x3c] = -0x66;
          pcVar14[0x3d] = -0x67;
          pcVar14[0x3e] = -0x67;
          pcVar14[0x3f] = '>';
          pcVar14[0x9b8] = '\x14';
          pcVar14[0x9b9] = '\0';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          pcVar14[0x54] = '\x05';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          *(float *)(pcVar14 + 0xc) = param_7;
          goto switchD_0046dee7_default;
        case 3:
          if (iVar6 == 0) {
            *pfVar3 = 0.5;
            pcVar14[0x38] = '\0';
            pcVar14[0x39] = '\0';
            pcVar14[0x3a] = '\0';
            pcVar14[0x3b] = '\0';
            pcVar14[0x3c] = '\0';
            pcVar14[0x3d] = '\0';
            pcVar14[0x3e] = '\0';
            pcVar14[0x3f] = '\0';
          }
          else {
            *pfVar3 = *(float *)(iVar6 + 0xe8);
            *(undefined4 *)(pcVar14 + 0x38) = *(undefined4 *)(iVar6 + 0xec);
            *(undefined4 *)(pcVar14 + 0x3c) = *(undefined4 *)(iVar6 + 0xf0);
          }
          *pfVar2 = -90.0;
          pcVar14[0x2c] = '\0';
          pcVar14[0x2d] = '\0';
          pcVar14[0x2e] = '\0';
          pcVar14[0x2f] = '\0';
          pcVar14[0x30] = '\0';
          pcVar14[0x31] = '\0';
          pcVar14[0x32] = '\0';
          pcVar14[0x33] = '\0';
          pcVar14[0x9c4] = '\0';
          pcVar14[0x9c5] = '\0';
          pcVar14[0x9c6] = '\0';
          pcVar14[0x9c7] = '\0';
          iVar6 = _rand();
          pcVar14[0x9cc] = '\0';
          pcVar14[0x9cd] = '\0';
          pcVar14[0x9ce] = '\0';
          pcVar14[0x9cf] = '\0';
          local_94 = (byte *)(iVar6 % 500);
          *(float *)(pcVar14 + 0x9c8) = (float)(int)local_94;
          iVar6 = _rand();
          *(float *)(pcVar14 + 0x9c0) = (float)(iVar6 % 5) + _DAT_00552660;
          break;
        case 4:
          *pfVar3 = 0.4;
          pcVar14[0x38] = -0x33;
          pcVar14[0x39] = -0x34;
          pcVar14[0x3a] = 'L';
          pcVar14[0x3b] = '?';
          pcVar14[0x3c] = -0x33;
          pcVar14[0x3d] = -0x34;
          pcVar14[0x3e] = 'L';
          pcVar14[0x3f] = '>';
          pcVar14[0x9b8] = '\x10';
          pcVar14[0x9b9] = '\'';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          pcVar14[0x54] = '\x1e';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          *(float *)(pcVar14 + 0xc) = param_7;
          goto switchD_0046dee7_default;
        default:
          goto switchD_0046db06_default;
        }
        pcVar14[0x9b8] = '?';
        pcVar14[0x9b9] = 'B';
        pcVar14[0x9ba] = '\x0f';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x54] = '\x1e';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
switchD_0046db06_default:
        *(float *)(pcVar14 + 0xc) = param_7;
        goto switchD_0046dee7_default;
      }
      pcVar14[0x9d0] = '\0';
      pcVar14[0x9d1] = '\0';
      *(int *)(pcVar14 + 0x9b8) = (int)param_8;
      pcVar14[0x54] = '2';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      if (pcVar14[0x9d2] == '\0') {
        pcVar14[0xc] = '\0';
        pcVar14[0xd] = '\0';
        pcVar14[0xe] = -0x60;
        pcVar14[0xf] = 'A';
        *pfVar3 = 0.5;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = '\0';
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = '\0';
        pcVar14[0x3f] = '?';
      }
      else if (pcVar14[0x9d2] == '\x01') {
        pcVar14[0xc] = '\0';
        pcVar14[0xd] = '\0';
        pcVar14[0xe] = ' ';
        pcVar14[0xf] = 'B';
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = '\0';
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = '\0';
        pcVar14[0x3f] = '\0';
      }
      switch(*(undefined4 *)(pcVar14 + 8)) {
      case 0:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = '\0';
        pcVar14[0x9c3] = '\0';
        break;
      case 1:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = -0x4c;
        pcVar14[0x9c3] = 'B';
        break;
      case 2:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = '4';
        pcVar14[0x9c3] = 'C';
        break;
      case 3:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = 'p';
        pcVar14[0x9c3] = 'C';
        break;
      case 4:
        *pfVar2 = 0.0;
        pcVar14[0x9b8] = '\0';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x2c] = '\0';
        pcVar14[0x2d] = '\0';
        pcVar14[0x2e] = '\0';
        pcVar14[0x2f] = '\0';
        pcVar14[0x30] = '\0';
        pcVar14[0x31] = '\0';
        pcVar14[0x32] = '\0';
        pcVar14[0x33] = '\0';
        *(float *)(pcVar14 + 0xc) = param_7;
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = -0x80;
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        *(float *)(pcVar14 + 0x1c) = (*param_3 - *pfVar1) * _DAT_00552a00;
        *(float *)(pcVar14 + 0x20) = (param_3[1] - *(float *)(pcVar14 + 0x14)) * _DAT_00552a00;
        *(float *)(pcVar14 + 0x24) = (param_3[2] - *(float *)(pcVar14 + 0x18)) * _DAT_00552a00;
        Matrix_BuildFromEuler(pfVar2,local_6c);
        iVar6 = 0;
        if (0 < *(int *)(pcVar14 + 0x54)) {
          do {
            *pfVar1 = *pfVar1 + *(float *)(pcVar14 + 0x1c);
            *(float *)(pcVar14 + 0x14) = *(float *)(pcVar14 + 0x14) + *(float *)(pcVar14 + 0x20);
            *(float *)(pcVar14 + 0x18) = *(float *)(pcVar14 + 0x24) + *(float *)(pcVar14 + 0x18);
            FUN_0046fe90((int)pcVar14,local_6c);
            iVar6 = iVar6 + 1;
          } while (iVar6 < *(int *)(pcVar14 + 0x54));
        }
      default:
        goto switchD_0046dd50_default;
      }
      fVar24 = param_7 * _DAT_005528f0;
      pcVar14[0x9c8] = '\0';
      pcVar14[0x9c9] = '\0';
      pcVar14[0x9ca] = '\0';
      pcVar14[0x9cb] = '\0';
      pcVar14[0x9cc] = '\0';
      pcVar14[0x9cd] = '\0';
      pcVar14[0x9ce] = '\0';
      pcVar14[0x9cf] = '\0';
      *(float *)(pcVar14 + 0x9c4) = fVar24;
switchD_0046dd50_default:
      *(float *)(pcVar14 + 0x44) = *param_3;
      *(float *)(pcVar14 + 0x48) = param_3[1];
      *(float *)(pcVar14 + 0x4c) = param_3[2];
      goto switchD_0046dee7_default;
    }
    if (param_1 == 0x4e5) {
      switch(*(undefined4 *)(pcVar14 + 8)) {
      case 0:
        local_98 = (byte*)DAT_07cf1ffc;
        local_80 = (byte*)DAT_07cf1ffc;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
        local_98 = (byte *)0x0;
        local_88 = (byte*)0x0;
        if (DAT_055c9bd4 != 0) {
          do {
            bVar18 = false;
            iVar11 = 0;
            bVar19 = true;
            local_80 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
            iVar6 = 4;
            ppbVar15 = &local_98;
            pbVar26 = local_80;
            do {
              if (iVar6 == 0) break;
              iVar6 = iVar6 + -1;
              bVar18 = *(byte *)ppbVar15 < *pbVar26;
              bVar19 = *(byte *)ppbVar15 == *pbVar26;
              ppbVar15 = (byte **)((int)ppbVar15 + 1);
              pbVar26 = pbVar26 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar11 == 0) goto LAB_0046e550;
            iVar6 = 4;
            bVar18 = false;
            iVar11 = 0;
            bVar19 = true;
            ppbVar15 = &local_84;
            pbVar26 = local_80;
            do {
              if (iVar6 == 0) break;
              iVar6 = iVar6 + -1;
              bVar18 = *(byte *)ppbVar15 < *pbVar26;
              bVar19 = *(byte *)ppbVar15 == *pbVar26;
              ppbVar15 = (byte **)((int)ppbVar15 + 1);
              pbVar26 = pbVar26 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);

            }
            if (iVar11 == 0) {
              if (uVar9 == 0xffffffff) goto LAB_0046e550;
              local_80 = pbVar10;
              uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(pbVar10);
              local_88 = (byte*)0x0;
              local_8c = (byte*)0x0;
              if (DAT_055c9bd4 != 0) goto LAB_0046e62c;
              goto LAB_0046e687;
            }
            uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
            local_88 = local_88 + 1;
          } while ((uint)(uintptr_t)local_88 < DAT_055c9bd4);
        }
        FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046e550:
        local_98 = (byte*)operator_new(0x585);
        local_98[0x584] = 1;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(pbVar10);
        local_84 = (byte *)0x0;
        local_8c = (byte*)0x0;
        if (DAT_055c9bd4 != 0) {
          do {
            bVar18 = false;
            iVar6 = 0;
            bVar19 = true;
            local_88 = (byte*)(uintptr_t)(int)(DAT_055c9bd0 + uVar9 * 4);
            iVar11 = 4;
            pbVar26 = (byte*)&stack0xffffff60;
            pbVar8 = local_88;
            do {
              if (iVar11 == 0) break;
              iVar11 = iVar11 + -1;
              bVar18 = *pbVar26 < *pbVar8;
              bVar19 = *pbVar26 == *pbVar8;
              pbVar26 = pbVar26 + 1;
              pbVar8 = pbVar8 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar6 == 0) {
LAB_0046e742:
              *(byte **)(DAT_055c9bcc + uVar9 * 4) = pbVar10;
              *(byte **)(DAT_055c9bd0 + uVar9 * 4) = pbVar10;
              goto LAB_0046e76a;
            }
            iVar11 = 4;
            bVar18 = false;
            iVar6 = 0;
            bVar19 = true;
            ppbVar15 = &local_84;
            pbVar26 = local_88;
            do {
              if (iVar11 == 0) break;
              iVar11 = iVar11 + -1;
              bVar18 = *(byte *)ppbVar15 < *pbVar26;
              bVar19 = *(byte *)ppbVar15 == *pbVar26;
              ppbVar15 = (byte **)((int)ppbVar15 + 1);
              pbVar26 = pbVar26 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar6 == 0) goto LAB_0046e742;
            uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
            local_8c = local_8c + 1;
          } while ((uint)(uintptr_t)local_8c < DAT_055c9bd4);
        }
        FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
        goto LAB_0046e76a;
      case 1:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = -0x74;
        pcVar14[0x9c3] = 'B';
        pcVar14[0x9b8] = '1';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        *(float *)(pcVar14 + 0xc) = param_7;
        pcVar14[0x54] = '\x06';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        FUN_00404bc0(0x2a,0,0);
        break;
      case 2:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = 'H';
        pcVar14[0x9c3] = 'B';
        *(float *)(pcVar14 + 0xc) = param_7;
        *pfVar3 = 0.5;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = '\0';
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = '\0';
        pcVar14[0x3f] = '?';
        pcVar14[0x9b8] = '\x14';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x54] = '\x03';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        *(float *)(pcVar14 + 0x1c) = *pfVar1;
        *(undefined4 *)(pcVar14 + 0x20) = *(undefined4 *)(pcVar14 + 0x14);
        *(undefined4 *)(pcVar14 + 0x24) = *(undefined4 *)(pcVar14 + 0x18);
        break;
      case 3:
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = '\f';
        pcVar14[0x9c3] = 'C';
        pcVar14[0x9b8] = '1';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        *(float *)(pcVar14 + 0xc) = param_7;
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = '\0';
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = -0x33;
        pcVar14[0x3d] = -0x34;
        pcVar14[0x3e] = -0x34;
        pcVar14[0x3f] = '=';
        FUN_00404bc0(0x49,0,0);
        break;
      case 4:
        *pfVar2 = 0.0;
        pcVar14[0x9b8] = '\0';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x2c] = '\0';
        pcVar14[0x2d] = '\0';
        pcVar14[0x2e] = '\0';
        pcVar14[0x2f] = '\0';
        pcVar14[0x30] = '\0';
        pcVar14[0x31] = '\0';
        pcVar14[0x32] = '\0';
        pcVar14[0x33] = '\0';
        *(float *)(pcVar14 + 0xc) = param_7;
        *pfVar3 = 0.3;
        pcVar14[0x38] = -0x66;
        pcVar14[0x39] = -0x67;
        pcVar14[0x3a] = -0x67;
        pcVar14[0x3b] = '>';
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *(float *)(pcVar14 + 0x1c) = (*param_3 - *pfVar1) * _DAT_00552a00;
        local_90 = *(float *)(pcVar14 + 0x1c);
        *(float *)(pcVar14 + 0x20) = (param_3[1] - *(float *)(pcVar14 + 0x14)) * _DAT_00552a00;
        local_8c = *(byte **)(pcVar14 + 0x20);
        local_88f = (param_3[2] - *(float *)(pcVar14 + 0x18)) * _DAT_00552a00;
        *(byte **)(pcVar14 + 0x24) = local_88;
        Matrix_BuildFromEuler(pfVar2,local_6c);
        iVar6 = 0;
        if (*(int *)(pcVar14 + 0x54) != 1 && -1 < *(int *)(pcVar14 + 0x54) + -1) {
          do {
            if (*(int *)(pcVar14 + 0x40) == 0) {
              local_90 = *(float *)(pcVar14 + 0x1c);
              local_8c = *(byte **)(pcVar14 + 0x20);
            }
            else {
              local_90 = *(float *)(pcVar14 + 0x1c);
              local_8cf = *(float *)(pcVar14 + 0x20);
            }
            *pfVar1 = local_90 + *pfVar1;
            *(float *)(pcVar14 + 0x14) = local_8cf + *(float *)(pcVar14 + 0x14);
            *(float *)(pcVar14 + 0x18) = local_88f + *(float *)(pcVar14 + 0x18);
            FUN_0046fe90((int)pcVar14,local_6c);
            iVar6 = iVar6 + 1;
            *pfVar1 = (*pfVar1 - local_90) + *(float *)(pcVar14 + 0x1c);
            *(float *)(pcVar14 + 0x14) =
                 (*(float *)(pcVar14 + 0x14) - local_8cf) + *(float *)(pcVar14 + 0x20);
          } while (iVar6 < *(int *)(pcVar14 + 0x54) + -1);
        }
        goto LAB_0046e37e;
      case 5:
        local_84 = (byte*)DAT_07cf1ffc;
        local_7c = (byte*)DAT_07cf1ffc;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
        local_98 = (byte *)0x0;
        if (DAT_055c9bd4 != 0) {
          do {
            bVar18 = false;
            iVar11 = 0;
            bVar19 = true;
            local_84 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
            iVar6 = 4;
            pbVar26 = (byte*)&stack0xffffff64;
            pbVar8 = local_84;
            do {
              if (iVar6 == 0) break;
              iVar6 = iVar6 + -1;
              bVar18 = *pbVar26 < *pbVar8;
              bVar19 = *pbVar26 == *pbVar8;
              pbVar26 = pbVar26 + 1;
              pbVar8 = pbVar8 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar11 == 0) goto LAB_0046ea53;
            iVar6 = 4;
            bVar18 = false;
            iVar11 = 0;
            bVar19 = true;
            ppbVar15 = &local_80;
            pbVar26 = local_84;
            do {
              if (iVar6 == 0) break;
              iVar6 = iVar6 + -1;
              bVar18 = *(byte *)ppbVar15 < *pbVar26;
              bVar19 = *(byte *)ppbVar15 == *pbVar26;
              ppbVar15 = (byte **)((int)ppbVar15 + 1);
              pbVar26 = pbVar26 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar11 == 0) {
              if (uVar9 == 0xffffffff) goto LAB_0046ea53;
              local_84 = pbVar10;
              uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(pbVar10);
              pbVar10 = (byte *)0x0;
              if (DAT_055c9bd4 != 0) goto LAB_0046eb2f;
              goto LAB_0046eb8a;
            }
            uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
            local_98 = local_98 + 1;
          } while ((uint)(uintptr_t)local_98 < DAT_055c9bd4);
        }
        FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046ea53:
        local_88 = (byte*)operator_new(0x585);
        local_88[0x584] = 1;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))(pbVar10);
        local_84 = (byte *)0x0;
        pbVar26 = (byte *)0x0;
        if (DAT_055c9bd4 != 0) {
          do {
            bVar18 = false;
            iVar6 = 0;
            bVar19 = true;
            local_88 = (byte*)(uintptr_t)(int)(DAT_055c9bd0 + uVar9 * 4);
            iVar11 = 4;
            pbVar8 = (byte*)&stack0xffffff60;
            pbVar17 = local_88;
            do {
              if (iVar11 == 0) break;
              iVar11 = iVar11 + -1;
              bVar18 = *pbVar8 < *pbVar17;
              bVar19 = *pbVar8 == *pbVar17;
              pbVar8 = pbVar8 + 1;
              pbVar17 = pbVar17 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar6 == 0) {
LAB_0046ec45:
              *(byte **)(DAT_055c9bcc + uVar9 * 4) = local_8c;
              *(byte **)(DAT_055c9bd0 + uVar9 * 4) = pbVar10;
              goto LAB_0046ec6d;
            }
            iVar11 = 4;
            bVar18 = false;
            iVar6 = 0;
            bVar19 = true;
            ppbVar15 = &local_84;
            pbVar8 = local_88;
            do {
              if (iVar11 == 0) break;
              iVar11 = iVar11 + -1;
              bVar18 = *(byte *)ppbVar15 < *pbVar8;
              bVar19 = *(byte *)ppbVar15 == *pbVar8;
              ppbVar15 = (byte **)((int)ppbVar15 + 1);
              pbVar8 = pbVar8 + 1;
            } while (bVar19);
            if (!bVar19) {
              iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
            }
            if (iVar6 == 0) goto LAB_0046ec45;
            uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
            pbVar26 = pbVar26 + 1;
          } while ((uint)(uintptr_t)pbVar26 < DAT_055c9bd4);
        }
        FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______Insert_005580e8);
        goto LAB_0046ec6d;
      }
      goto switchD_0046dee7_default;
    }
    if (param_1 != 0x4e6) {
      if (param_1 != 0x4e7) goto switchD_0046dee7_default;
      pcVar14[0x54] = '2';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      *(float *)(pcVar14 + 0xc) = param_7;
      iVar6 = _rand();
      pcVar14[0x9b8] = '\x14';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      *(float *)(pcVar14 + 0x9c0) = (float)(iVar6 % 0x14) + _DAT_00552878;
      *(float *)(pcVar14 + 0x3c) = (float)*(int *)(pcVar14 + 0x54);
      goto LAB_0046e043;
    }
    pcVar14[0x9c0] = '\0';
    pcVar14[0x9c1] = '\0';
    pcVar14[0x9c2] = 'H';
    pcVar14[0x9c3] = 'B';
    *(float *)(pcVar14 + 0xc) = param_7;
    pcVar14[0x54] = '2';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    switch(*(undefined4 *)(pcVar14 + 8)) {
    case 1:
      iVar6 = _rand();
      local_94 = (byte *)(iVar6 % 10);
      *(float *)(pcVar14 + 0x9c0) = (float)(int)local_94 + _DAT_005524fc;
      uVar9 = _rand();
      uVar9 = uVar9 & 0x80000007;
      if ((int)uVar9 < 0) {
        uVar9 = (uVar9 - 1 | 0xfffffff8) + 1;
      }
      *(uint *)(pcVar14 + 0x9b8) = uVar9 + 8;
      break;
    case 2:
      pcVar14[0x9b8] = '\x02';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      *pfVar3 = 1.0;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = -0x34;
      pcVar14[0x3b] = '=';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = '\0';
      pcVar14[0x3f] = '\0';
      local_3c[0] = 0.0;
      local_3c[1] = -150.0;
      local_3c[2] = 0.0;
      // DAT_05828d58 + 0x11e68 = dirección absoluta 0x0583AB40 (artefacto Ghidra).
      // TransformPosition con base bogus; omitimos hasta resolver bone matrix real.
      // BMD_TransformPosition((void *)(DAT_05828d58 + 0x11e68),
      //              (float *)(*(int *)(*(int *)(pcVar14 + 0x40) + 0x114) + 0x630),local_3c,
      //              (float *)(pcVar14 + 0x44),'\x01');
      break;
    case 3:
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      *(int *)(pcVar14 + 0x9b8) = (int)param_8;
      *(uint *)(pcVar14 + 0x54) = (uint)param_9;
      *pfVar3 = 0.5;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = '\0';
      pcVar14[0x3b] = '?';
      *(float *)(pcVar14 + 0x9c0) = (float)param_9;
      pcVar14[0x9d0] = -1;
      pcVar14[0x9d1] = -1;
      goto LAB_0046e043;
    case 4:
    case 5:
      pcVar14[0x9c0] = '\0';
      pcVar14[0x9c1] = '\0';
      pcVar14[0x9c2] = 'p';
      pcVar14[0x9c3] = 'B';
      pcVar14[0x9c4] = '\0';
      pcVar14[0x9c5] = '\0';
      pcVar14[0x9c6] = '\0';
      pcVar14[0x9c7] = '\0';
      pcVar14[0x9c8] = '\0';
      pcVar14[0x9c9] = '\0';
      pcVar14[0x9ca] = '\0';
      pcVar14[0x9cb] = '\0';
      pcVar14[0x9cc] = '\0';
      pcVar14[0x9cd] = '\0';
      pcVar14[0x9ce] = '\0';
      pcVar14[0x9cf] = '\0';
      *pfVar3 = 0.5;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = '\0';
      pcVar14[0x3b] = '?';
      pcVar14[0x9b8] = '2';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[0x54] = '\n';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      *(float *)(pcVar14 + 0x44) = *param_3;
      *(float *)(pcVar14 + 0x48) = param_3[1];
      *(float *)(pcVar14 + 0x4c) = param_3[2];
      *(float *)(pcVar14 + 0x1c) = *pfVar1;
      *(undefined4 *)(pcVar14 + 0x20) = *(undefined4 *)(pcVar14 + 0x14);
      *(undefined4 *)(pcVar14 + 0x24) = *(undefined4 *)(pcVar14 + 0x18);
      break;
    case 6:
      iVar6 = _rand();
      *(int *)(pcVar14 + 0x9b8) = iVar6 % 0x14 + 6;
      iVar6 = _rand();
      local_94 = (byte *)(iVar6 % 10);
      *(float *)(pcVar14 + 0x9c0) = (float)(int)local_94 + _DAT_00552834;
      iVar6 = _rand();
      fVar24 = (float)(iVar6 % 10) * _DAT_00552a48 + _DAT_005524f4;
      *pfVar3 = fVar24;
      *(float *)(pcVar14 + 0x38) = fVar24;
      *(float *)(pcVar14 + 0x3c) = fVar24;
      goto LAB_0046e043;
    case 7:
      pcVar14[0x9c0] = '\0';
      pcVar14[0x9c1] = '\0';
      pcVar14[0x9c2] = -0x60;
      pcVar14[0x9c3] = '@';
      *(float *)(pcVar14 + 0x1c) = *pfVar1;
      *pfVar3 = 0.5;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = '\0';
      pcVar14[0x3b] = '?';
      pcVar14[0x54] = '\x03';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x9b8] = '\x01';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      *(undefined4 *)(pcVar14 + 0x20) = *(undefined4 *)(pcVar14 + 0x14);
      *(undefined4 *)(pcVar14 + 0x24) = *(undefined4 *)(pcVar14 + 0x18);
      FUN_0043e4a0(pfVar1,pfVar2,(float *)(pcVar14 + 0x44),360.0);
      break;
    case 8:
      pcVar14[0xc] = '\0';
      pcVar14[0xd] = '\0';
      pcVar14[0xe] = -0x60;
      pcVar14[0xf] = '@';
      iVar6 = _rand();
      pbVar10 = (byte *)(iVar6 % 10 + 6);
      local_94 = pbVar10;
      goto LAB_0046f415;
    case 9:
      iVar6 = _rand();
      pcVar14[0x9b8] = '\x1e';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      local_94 = (byte *)(iVar6 % 0x14);
      *(float *)(pcVar14 + 0x9c0) = (float)(int)local_94 + _DAT_00552878;
      iVar6 = _rand();
      fVar24 = (float)(iVar6 % 10) * _DAT_00552a48 + _DAT_005524f4;
      *pfVar3 = fVar24;
      *(float *)(pcVar14 + 0x38) = fVar24;
      *(float *)(pcVar14 + 0x3c) = fVar24;
LAB_0046e043:
      *(float *)(pcVar14 + 0x1c) = *pfVar1;
      *(undefined4 *)(pcVar14 + 0x20) = *(undefined4 *)(pcVar14 + 0x14);
      *(undefined4 *)(pcVar14 + 0x24) = *(undefined4 *)(pcVar14 + 0x18);
      break;
    case 10:
      *pfVar2 = 0.0;
      pcVar14[0x9b8] = '\0';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[0x2c] = '\0';

      pcVar14[0x2d] = '\0';
      pcVar14[0x2e] = '\0';
      pcVar14[0x2f] = '\0';
      pcVar14[0x30] = '\0';
      pcVar14[0x31] = '\0';
      pcVar14[0x32] = '\0';
      pcVar14[0x33] = '\0';
      *(float *)(pcVar14 + 0xc) = param_7;
      *pfVar3 = 0.3;
      pcVar14[0x38] = -0x66;
      pcVar14[0x39] = -0x67;
      pcVar14[0x3a] = -0x67;
      pcVar14[0x3b] = '>';
      pcVar14[0x54] = '\n';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      *(float *)(pcVar14 + 0x1c) = (*param_3 - *pfVar1) * _DAT_00552a00;
      *(float *)(pcVar14 + 0x20) = (param_3[1] - *(float *)(pcVar14 + 0x14)) * _DAT_00552a00;
      local_90 = *(float *)(pcVar14 + 0x1c);
      local_8c = *(byte **)(pcVar14 + 0x20);
      local_88f = (param_3[2] - *(float *)(pcVar14 + 0x18)) * _DAT_00552a00;
      *(byte **)(pcVar14 + 0x24) = local_88;
      Matrix_BuildFromEuler(pfVar2,local_6c);
      iVar6 = 0;
      if (*(int *)(pcVar14 + 0x54) != 1 && -1 < *(int *)(pcVar14 + 0x54) + -1) {
        do {
          if (*(int *)(pcVar14 + 0x40) == 0) {
            uVar9 = _rand();
            local_94 = (byte *)(uVar9 & 0x80000007);
            if ((int)local_94 < 0) {
              local_94 = (byte *)(((int)local_94 - 1U | 0xfffffff8) + 1);
            }
            local_90 = ((float)(int)local_94 + *(float *)(pcVar14 + 0x1c)) - _DAT_00552650;
            uVar9 = _rand();
            local_94 = (byte *)(uVar9 & 0x80000007);
            if ((int)local_94 < 0) {
              local_94 = (byte *)(((uint)(local_94 + -1) | 0xfffffff8) + 1);
            }
            local_8cf = (local_94f + *(float *)(pcVar14 + 0x20)) - _DAT_00552650;
          }
          else {
            uVar9 = _rand();
            local_94 = (byte *)(uVar9 & 0x8000000f);
            if ((int)local_94 < 0) {
              local_94 = (byte *)(((int)local_94 - 1U | 0xfffffff0) + 1);
            }
            local_90 = ((float)(int)local_94 + *(float *)(pcVar14 + 0x1c)) - _DAT_00552658;
            uVar9 = _rand();
            local_94 = (byte *)(uVar9 & 0x8000000f);
            if ((int)local_94 < 0) {
              local_94 = (byte *)(((uint)(local_94 + -1) | 0xfffffff0) + 1);
            }
            local_8cf = (local_94f + *(float *)(pcVar14 + 0x20)) - _DAT_00552658;
          }
          *pfVar1 = local_90 + *pfVar1;
          *(float *)(pcVar14 + 0x14) = local_8cf + *(float *)(pcVar14 + 0x14);
          *(float *)(pcVar14 + 0x18) = local_88f + *(float *)(pcVar14 + 0x18);
          FUN_0046fe90((int)pcVar14,local_6c);
          iVar6 = iVar6 + 1;
          *pfVar1 = (*pfVar1 - local_90) + *(float *)(pcVar14 + 0x1c);
          *(float *)(pcVar14 + 0x14) =
               (*(float *)(pcVar14 + 0x14) - local_8cf) + *(float *)(pcVar14 + 0x20);
        } while (iVar6 < *(int *)(pcVar14 + 0x54) + -1);
      }
      goto LAB_0046e37e;
    case 0xb:
      *pfVar3 = 1.0;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = '\0';
      pcVar14[0x3b] = '?';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = -0x34;
      pcVar14[0x3f] = '=';
    case 0:
      pcVar14[0x9b8] = '\x02';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      break;
    case 0xc:
      *pfVar3 = 1.0;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = -0x34;
      pcVar14[0x3b] = '=';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = -0x34;
      pcVar14[0x3f] = '=';
      pcVar14[0x9b8] = '\n';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      break;
    case 0xd:
      *pfVar3 = 1.0;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = -0x34;
      pcVar14[0x3b] = '=';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = -0x34;
      pcVar14[0x3f] = '=';
      pcVar14[0x9b8] = '\x02';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[8] = '\v';
      pcVar14[9] = '\0';
      pcVar14[10] = '\0';
      pcVar14[0xb] = '\0';
      break;
    case 0xe:
      *pfVar2 = 0.0;
      pcVar14[0x9b8] = '\0';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[0x2c] = '\0';
      pcVar14[0x2d] = '\0';
      pcVar14[0x2e] = '\0';
      pcVar14[0x2f] = '\0';
      pcVar14[0x30] = '\0';
      pcVar14[0x31] = '\0';
      pcVar14[0x32] = '\0';
      pcVar14[0x33] = '\0';
      *(float *)(pcVar14 + 0xc) = param_7;
      *pfVar3 = 0.3;
      pcVar14[0x38] = -0x66;
      pcVar14[0x39] = -0x67;
      pcVar14[0x3a] = -0x67;
      pcVar14[0x3b] = '>';
      pcVar14[0x54] = '\n';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      *(float *)(pcVar14 + 0x1c) = (*param_3 - *pfVar1) * _DAT_00552a00;
      local_90 = *(float *)(pcVar14 + 0x1c);
      *(float *)(pcVar14 + 0x20) = (param_3[1] - *(float *)(pcVar14 + 0x14)) * _DAT_00552a00;
      local_8c = *(byte **)(pcVar14 + 0x20);
      local_88f = (param_3[2] - *(float *)(pcVar14 + 0x18)) * _DAT_00552a00;
      *(byte **)(pcVar14 + 0x24) = local_88;
      Matrix_BuildFromEuler(pfVar2,local_6c);
      iVar6 = 0;
      if (*(int *)(pcVar14 + 0x54) != 1 && -1 < *(int *)(pcVar14 + 0x54) + -1) {
        do {
          if (*(int *)(pcVar14 + 0x40) == 0) {
            local_90 = *(float *)(pcVar14 + 0x1c);
            local_8c = *(byte **)(pcVar14 + 0x20);
          }
          else {
            local_90 = *(float *)(pcVar14 + 0x1c);
            local_8c = *(byte **)(pcVar14 + 0x20);
          }
          *pfVar1 = local_90 + *pfVar1;
          *(float *)(pcVar14 + 0x14) = local_8cf + *(float *)(pcVar14 + 0x14);
          *(float *)(pcVar14 + 0x18) = local_88f + *(float *)(pcVar14 + 0x18);
          FUN_0046fe90((int)pcVar14,local_6c);
          iVar6 = iVar6 + 1;
          *pfVar1 = (*pfVar1 - local_90) + *(float *)(pcVar14 + 0x1c);
          *(float *)(pcVar14 + 0x14) =
               (*(float *)(pcVar14 + 0x14) - local_8cf) + *(float *)(pcVar14 + 0x20);
        } while (iVar6 < *(int *)(pcVar14 + 0x54) + -1);
      }
LAB_0046e37e:
      *(undefined4 *)(pcVar14 + 0x10) = *(undefined4 *)(pcVar14 + 0x44);
      *(undefined4 *)(pcVar14 + 0x14) = *(undefined4 *)(pcVar14 + 0x48);
      *(undefined4 *)(pcVar14 + 0x18) = *(undefined4 *)(pcVar14 + 0x4c);
    }
    goto switchD_0046dee7_default;
  }
  switch(param_1) {
  case 0x4e9:
    iVar6 = *(int *)(pcVar14 + 8);
    if (iVar6 == 0) {
      pcVar14[0xc] = '\0';
      pcVar14[0xd] = '\0';
      pcVar14[0xe] = '\0';
      pcVar14[0xf] = '@';
      iVar6 = _rand();
      pbVar10 = (byte *)(iVar6 % 0x14 + 6);
      local_98 = pbVar10;
LAB_0046f415:
      *(float *)(pcVar14 + 0x9c0) = (float)(int)pbVar10;
      uVar9 = _rand();
      uVar9 = uVar9 & 0x80000007;
      if ((int)uVar9 < 0) {
        uVar9 = (uVar9 - 1 | 0xfffffff8) + 1;
      }
      pcVar14[0x54] = '\x02';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      *(uint *)(pcVar14 + 0x9b8) = uVar9 + 8;
    }
    else {
      if (iVar6 != 1) {
        if (iVar6 == 2) {
          pcVar14[0xc] = '\0';
          pcVar14[0xd] = '\0';
          pcVar14[0xe] = -0x80;
          pcVar14[0xf] = '@';
          pcVar14[0x9c0] = '\0';
          pcVar14[0x9c1] = '\0';
          pcVar14[0x9c2] = -0x10;
          pcVar14[0x9c3] = 'A';
          uVar9 = _rand();
          uVar9 = uVar9 & 0x80000003;
          if ((int)uVar9 < 0) {
            uVar9 = (uVar9 - 1 | 0xfffffffc) + 1;
          }
          pcVar14[0x54] = '\x02';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          *(uint *)(pcVar14 + 0x9b8) = uVar9 + 4;
          *pfVar3 = 0.3;
          pcVar14[0x38] = -0x66;
          pcVar14[0x39] = -0x67;
          pcVar14[0x3a] = -0x67;
          pcVar14[0x3b] = '>';
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = -0x80;
          pcVar14[0x3f] = '?';
        }
        goto switchD_0046dee7_default;
      }
      pcVar14[0xc] = '\0';
      pcVar14[0xd] = '\0';
      pcVar14[0xe] = '\0';
      pcVar14[0xf] = '@';
      iVar6 = _rand();
      local_98 = (byte *)(iVar6 % 0x14 + 0x10);
      *(float *)(pcVar14 + 0x9c0) = (float)(int)local_98;
      uVar9 = _rand();
      uVar9 = uVar9 & 0x80000003;
      if ((int)uVar9 < 0) {
        uVar9 = (uVar9 - 1 | 0xfffffffc) + 1;
      }
      pcVar14[0x54] = '\x02';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      *(uint *)(pcVar14 + 0x9b8) = uVar9 + 4;
    }
    break;
  case 0x4ea:
    iVar6 = *(int *)(pcVar14 + 8);
    *(float *)(pcVar14 + 0xc) = param_7;
    switch(iVar6) {
    case 2:
    case 3:
    case 4:
    case 5:
    case 8:
      pcVar14[0x9b8] = -1;
      pcVar14[0x9b9] = -0x37;
      pcVar14[0x9ba] = -0x66;
      pcVar14[0x9bb] = ';';
      if (iVar6 == 5) {
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
      }
      else {
        pcVar14[0x54] = '\x14';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
      }
      if (iVar6 == 3) {
        *pfVar1 = *(float *)(*(int *)(pcVar14 + 0x40) + 0x4c);
        *(undefined4 *)(pcVar14 + 0x14) = *(undefined4 *)(*(int *)(pcVar14 + 0x40) + 0x50);
        uVar23 = *(undefined4 *)(*(int *)(pcVar14 + 0x40) + 0x54);
      }
      else {
        *pfVar1 = *(float *)(*(int *)(pcVar14 + 0x40) + 0x40);
        *(undefined4 *)(pcVar14 + 0x14) = *(undefined4 *)(*(int *)(pcVar14 + 0x40) + 0x44);
        uVar23 = *(undefined4 *)(*(int *)(pcVar14 + 0x40) + 0x48);
      }
      *(undefined4 *)(pcVar14 + 0x18) = uVar23;
      break;
    case 9:
      *pfVar3 = 0.2;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = 'L';
      pcVar14[0x3b] = '>';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = 'L';
      pcVar14[0x3f] = '>';
    case 0:
    case 1:
    case 6:
    case 7:
      pcVar14[0x9b8] = 'x';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      pcVar14[0x54] = '\b';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      pcVar14[0x9c0] = '\0';
      pcVar14[0x9c1] = '\0';
      pcVar14[0x9c2] = '@';
      pcVar14[0x9c3] = '@';
    }
    switch(*(undefined4 *)(pcVar14 + 8)) {
    case 0:
    case 6:
      *pfVar3 = 0.4;
      pcVar14[0x38] = -0x66;
      pcVar14[0x39] = -0x67;
      pcVar14[0x3a] = -0x67;
      pcVar14[0x3b] = '>';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = 'L';
      pcVar14[0x3f] = '>';
      break;
    case 1:
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = '\0';
      pcVar14[0x3f] = '?';
      *pfVar3 = 0.1;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = -0x34;
      pcVar14[0x3b] = '=';
      break;
    case 2:
    case 3:
    case 5:
      *pfVar3 = 0.5;
      pcVar14[0x38] = -0x33;
      pcVar14[0x39] = -0x34;
      pcVar14[0x3a] = -0x34;
      pcVar14[0x3b] = '=';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = -0x80;
      pcVar14[0x3f] = '?';
      break;
    case 4:
      *pfVar3 = 0.3;
      pcVar14[0x38] = -0x66;
      pcVar14[0x39] = -0x67;
      pcVar14[0x3a] = '\x19';
      pcVar14[0x3b] = '>';
      pcVar14[0x3c] = -0x33;
      pcVar14[0x3d] = -0x34;
      pcVar14[0x3e] = -0x34;
      pcVar14[0x3f] = '=';
      break;
    case 8:
      *pfVar3 = 1.0;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = '\0';
      pcVar14[0x3b] = '\0';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = '\0';
      pcVar14[0x3f] = '?';
    }
    goto switchD_0046dee7_default;
  case 0x4eb:
    pcVar14[0x9b8] = '\f';
    pcVar14[0x9b9] = '\0';
    pcVar14[0x9ba] = '\0';
    pcVar14[0x9bb] = '\0';
    *(float *)(pcVar14 + 0xc) = param_7;
    pcVar14[0x9c0] = '\0';
    pcVar14[0x9c1] = '\0';
    pcVar14[0x9c2] = '\0';
    pcVar14[0x9c3] = '\0';
    iVar6 = *(int *)(pcVar14 + 8);
    pcVar14[0x54] = '\x02';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    if (iVar6 == 6) {
      pcVar14[0x54] = '\x04';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      *pfVar3 = 1.0;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = -0x80;
      pcVar14[0x3b] = '?';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = '\0';
      pcVar14[0x3f] = '?';
      goto switchD_0046dee7_default;
    }
    if (iVar6 == 7) {
      pcVar14[0x54] = '\x04';
      pcVar14[0x55] = '\0';
      pcVar14[0x56] = '\0';
      pcVar14[0x57] = '\0';
      *pfVar3 = 1.0;
      pcVar14[0x38] = '\0';
      pcVar14[0x39] = '\0';
      pcVar14[0x3a] = -0x80;
      pcVar14[0x3b] = '?';
      pcVar14[0x3c] = '\0';
      pcVar14[0x3d] = '\0';
      pcVar14[0x3e] = '\0';
      pcVar14[0x3f] = '\0';
      goto switchD_0046dee7_default;
    }
    if (iVar6 == 4) {
      pcVar14[0x9b8] = '\x1e';
      pcVar14[0x9b9] = '\0';
      pcVar14[0x9ba] = '\0';
      pcVar14[0x9bb] = '\0';
      uVar9 = _rand();
      uVar9 = uVar9 & 0x8000003f;
      if ((int)uVar9 < 0) {
        uVar9 = (uVar9 - 1 | 0xffffffc0) + 1;
      }
      pcVar14[0x48] = '\0';
      pcVar14[0x49] = '\0';
      pcVar14[0x4a] = ' ';
      pcVar14[0x4b] = -0x3f;
      pcVar14[0x4c] = '\0';
      pcVar14[0x4d] = '\0';
      pcVar14[0x4e] = '\0';
      pcVar14[0x4f] = '\0';
      *(float *)(pcVar14 + 0x44) = (float)(int)(uVar9 - 0x20);
      goto switchD_0046dee7_default;
    }
    if (iVar6 != 5) goto switchD_0046dee7_default;
    iVar6 = _rand();
    local_98 = (byte *)(iVar6 % 10 + -5);
    *(float *)(pcVar14 + 0xc) = (float)(int)local_98 + *(float *)(pcVar14 + 0xc);
    iVar6 = _rand();
    local_98 = (byte *)(iVar6 % 0x14 + 6);
    *(float *)(pcVar14 + 0x9c0) = (float)(int)local_98;
    uVar9 = _rand();
    uVar9 = uVar9 & 0x80000007;
    if ((int)uVar9 < 0) {
      uVar9 = (uVar9 - 1 | 0xfffffff8) + 1;
    }
    pcVar14[0x54] = '\b';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    *(uint *)(pcVar14 + 0x9b8) = uVar9 + 8;
    break;
  case 0x4ec:
    pcVar14[0x9c0] = '\0';
    pcVar14[0x9c1] = '\0';
    pcVar14[0x9c2] = -0x74;
    pcVar14[0x9c3] = 'B';
    pcVar14[0x9b8] = '1';
    pcVar14[0x9b9] = '\0';
    pcVar14[0x9ba] = '\0';
    pcVar14[0x9bb] = '\0';
    *(float *)(pcVar14 + 0xc) = param_7;
    pcVar14[0x54] = '\x06';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    goto switchD_0046dee7_default;
  case 0x4ed:
switchD_0046f0e4_caseD_4ed:
    pcVar14[0x9c0] = '\0';
    pcVar14[0x9c1] = '\0';
    pcVar14[0x9c2] = ' ';
    pcVar14[0x9c3] = 'B';
    pcVar14[0xc] = '\0';
    pcVar14[0xd] = '\0';
    pcVar14[0xe] = 'p';
    pcVar14[0xf] = 'B';
    pcVar14[0x54] = '2';
    pcVar14[0x55] = '\0';

    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    pcVar14[0x9b8] = '\x02';
    pcVar14[0x9b9] = '\0';
    pcVar14[0x9ba] = '\0';
    pcVar14[0x9bb] = '\0';
    if (*(int *)(pcVar14 + 8) != 0) {
      if (*(int *)(pcVar14 + 4) == 0x4ed) {
        *pfVar3 = 0.35;
        pcVar14[0x38] = -0x33;
        pcVar14[0x39] = -0x34;
        pcVar14[0x3a] = -0x34;
        pcVar14[0x3b] = '=';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
      }
      else {
        *pfVar3 = 0.0;
        pcVar14[0x38] = -0x66;
        pcVar14[0x39] = -0x67;
        pcVar14[0x3a] = -0x67;
        pcVar14[0x3b] = '>';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
      }
      goto switchD_0046dee7_default;
    }
    break;
  case 0x4ee:
    pcVar14[0x9c0] = '\0';
    pcVar14[0x9c1] = '\0';
    pcVar14[0x9c2] = '\0';
    pcVar14[0x9c3] = '\0';
    fVar24 = *(float *)(pcVar14 + 0x30) + _DAT_0055284c;
    *(float *)(pcVar14 + 0xc) = param_7;
    *(undefined4 *)(pcVar14 + 0x4c) = *(undefined4 *)(pcVar14 + 0x18);
    *(float *)(pcVar14 + 0x30) = fVar24;
    *(float *)(pcVar14 + 0x9cc) = fVar24;
    *(float *)(pcVar14 + 0x44) = *pfVar1;
    *(undefined4 *)(pcVar14 + 0x48) = *(undefined4 *)(pcVar14 + 0x14);
    pcVar14[0x54] = '\x12';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    pcVar14[0x50] = -1;
    pcVar14[0x51] = -1;
    pcVar14[0x52] = -1;
    pcVar14[0x53] = -1;
    pcVar14[0x9b8] = '\x14';
    pcVar14[0x9b9] = '\0';
    pcVar14[0x9ba] = '\0';
    pcVar14[0x9bb] = '\0';
    local_78 = 0.0;
    local_74 = -180.0;
    local_70 = 0.0;
    *(float *)(pcVar14 + 0x9c4) = *pfVar2;
    *(undefined4 *)(pcVar14 + 0x9c8) = *(undefined4 *)(pcVar14 + 0x2c);
    Matrix_BuildFromEuler(pfVar2,local_6c);
    Vector_Rotate(&local_78,local_6c,(float *)&local_90);
    *pfVar1 = (float)local_90 + *pfVar1;
    *(float *)(pcVar14 + 0x14) = *(float*)&local_8c + *(float *)(pcVar14 + 0x14);
    *(float *)(pcVar14 + 0x18) = *(float*)&local_88 + *(float *)(pcVar14 + 0x18);
    break;
  default:
    goto switchD_0046dee7_default;
  case 0x4fd:
switchD_0046f0e4_caseD_4fd:
    iVar6 = *(int *)(pcVar14 + 8);
    pcVar14[0x9b8] = 'd';
    pcVar14[0x9b9] = '\0';
    pcVar14[0x9ba] = '\0';
    pcVar14[0x9bb] = '\0';
    pcVar14[0x54] = '\x14';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
    pcVar14[0xc] = '\0';
    pcVar14[0xd] = '\0';
    pcVar14[0xe] = ' ';
    pcVar14[0xf] = 'A';
    if (iVar6 == 0) {
      *(float *)(pcVar14 + 0xc) = param_7;
      iVar6 = _rand();
      local_98 = (byte *)(iVar6 % 0x96);
      *(float *)(pcVar14 + 0x9cc) = (float)(int)local_98 * _DAT_00552594;
      iVar6 = _rand();
      local_98 = (byte *)(iVar6 % 500 + -0xfa);
      *(float *)(pcVar14 + 0x9c8) = (float)(int)local_98;
      if (_DAT_00552488 < *(float *)(pcVar14 + 0xc)) {
        pcVar14[0x9b8] = '2';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        iVar6 = _rand();
        fVar24 = (float)(iVar6 % 0xfa + 200);
LAB_0046f54e:
        *(float *)(pcVar14 + 0x9cc) = fVar24 * _DAT_00552594;
      }
LAB_0046f55a:
      iVar6 = *(int *)(pcVar14 + 0x40);
      *pfVar3 = *(float *)(iVar6 + 0xe8);
      *(undefined4 *)(pcVar14 + 0x38) = *(undefined4 *)(iVar6 + 0xec);
      *(undefined4 *)(pcVar14 + 0x3c) = *(undefined4 *)(iVar6 + 0xf0);
    }
    else {
      if (iVar6 == 0xe) {
        *(float *)(pcVar14 + 0xc) = param_7;
        iVar6 = _rand();
        local_98 = (byte *)(iVar6 % 0x96);
        *(float *)(pcVar14 + 0x9cc) = (float)(int)local_98 * _DAT_00552594;
        iVar6 = _rand();
        local_98 = (byte *)(iVar6 % 500 + -0xfa);
        *(float *)(pcVar14 + 0x9c8) = (float)(int)local_98;
        iVar6 = _rand();
        local_98 = (byte *)(iVar6 % 0x32);
        *(float *)(pcVar14 + 0x18) = (float)(int)local_98 + *(float *)(pcVar14 + 0x18);
        if (_DAT_00552488 < *(float *)(pcVar14 + 0xc)) {
          pcVar14[0x9b8] = '2';
          pcVar14[0x9b9] = '\0';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          iVar6 = _rand();
          fVar24 = (float)(iVar6 % 0xfa + 200);
          goto LAB_0046f54e;
        }
        goto LAB_0046f55a;
      }
      if (iVar6 == 2) {
        iVar6 = _rand();
        local_98 = (byte *)(iVar6 % 0x14 + 0x23);
        *(float *)(pcVar14 + 0xc) = param_7;
        *(float *)(pcVar14 + 0x9cc) = (float)(int)local_98;
        iVar6 = _rand();
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = -0x80;
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *(int *)(pcVar14 + 0x9b8) = iVar6 % 0x32 + 0x19;
      }
      else if (iVar6 == 3) {
        pcVar14[0x9c0] = '\0';
        pcVar14[0x9c1] = '\0';
        pcVar14[0x9c2] = 'H';
        pcVar14[0x9c3] = 'B';
        *pfVar2 = 0.0;
        *(float *)(pcVar14 + 0xc) = param_7;
        pcVar14[0x9b8] = '\x05';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        pcVar14[0x2c] = '\0';
        pcVar14[0x2d] = '\0';
        pcVar14[0x2e] = '4';
        pcVar14[0x2f] = 'B';
        pcVar14[0x30] = '\0';
        pcVar14[0x31] = '\0';
        pcVar14[0x32] = -0x4c;
        pcVar14[0x33] = -0x3e;
        Matrix_BuildFromEuler(pfVar2,local_3c + 3);
        local_78 = 0.0;
        local_74 = 100.0;
        local_70 = 0.0;
        Vector_Rotate(&local_78,local_3c + 3,(float *)&local_90);
        uVar25 = 0;
        *pfVar1 = (float)local_90 + *pfVar1;
        *(float *)(pcVar14 + 0x14) = *(float*)&local_8c + *(float *)(pcVar14 + 0x14);
        *(float *)(pcVar14 + 0x18) = *(float*)&local_88 + *(float *)(pcVar14 + 0x18);
        iVar6 = _rand();
        local_98 = (byte *)(iVar6 % 0x168);
        fVar24 = (float)(int)local_98;
        uVar23 = 0;
        pfVar22 = pfVar3;
        uVar9 = _rand();
        uVar9 = uVar9 & 0x80000007;
        if ((int)uVar9 < 0) {
          uVar9 = (uVar9 - 1 | 0xfffffff8) + 1;
        }
        local_98 = (byte *)(uVar9 + 8);
        FUN_004795c0(0x4cf,pfVar1,(float)(int)local_98 * _DAT_005528b8,pfVar22,uVar23,fVar24,uVar25)
        ;
        fVar24 = *pfVar2 * _DAT_005526d8;
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = -0x80;
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *pfVar2 = fVar24;
        *(float *)(pcVar14 + 0x2c) = *(float *)(pcVar14 + 0x2c) * _DAT_005526d8;
        *(float *)(pcVar14 + 0x30) = *(float *)(pcVar14 + 0x30) * _DAT_005526d8;
      }
      else if (((iVar6 == 4) || (iVar6 == 6)) || (iVar6 == 0xc)) {
        *(float *)(pcVar14 + 0xc) = param_7;
        if (iVar6 == 0xc) {
          pcVar14[0x9b8] = 'F';
          pcVar14[0x9b9] = '\0';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          pcVar14[0x54] = '2';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          *pfVar3 = 0.1;
          pcVar14[0x38] = -0x33;
          pcVar14[0x39] = -0x34;
          pcVar14[0x3a] = -0x34;
          pcVar14[0x3b] = '=';
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = -0x80;
          pcVar14[0x3f] = '?';
          iVar6 = _rand();
          *(float *)(pcVar14 + 0x9c4) = (float)(iVar6 % 0x168);
          fVar21 = (float10)_DAT_00552a44;
          fVar20 = (float10)fsin((float10)*(float *)(pcVar14 + 0x30) * fVar21);
          *(float *)(pcVar14 + 0x9c8) = (float)(fVar20 * (float10)_DAT_00552a34);
          fVar21 = (float10)fcos((float10)*(float *)(pcVar14 + 0x30) * fVar21);
          fVar21 = fVar21 * (float10)_DAT_00552a34;
        }
        else {
          if (iVar6 == 6) {
            pcVar14[0x9b8] = '\x14';
            pcVar14[0x9b9] = '\0';
            pcVar14[0x9ba] = '\0';
            pcVar14[0x9bb] = '\0';
            pcVar14[0x54] = '\x1e';
            pcVar14[0x55] = '\0';
            pcVar14[0x56] = '\0';
            pcVar14[0x57] = '\0';
            *pfVar3 = 1.0;
            pcVar14[0x38] = '\0';
            pcVar14[0x39] = '\0';
            pcVar14[0x3a] = -0x80;
            pcVar14[0x3b] = '?';
            pcVar14[0x3c] = '\0';
            pcVar14[0x3d] = '\0';
            pcVar14[0x3e] = -0x80;
            pcVar14[0x3f] = '?';
          }
          else {
            iVar6 = *(int *)(pcVar14 + 0x40);
            pcVar14[0x9b8] = 'n';
            pcVar14[0x9b9] = '\0';
            pcVar14[0x9ba] = '\0';
            pcVar14[0x9bb] = '\0';
            pcVar14[0x54] = -0x38;
            pcVar14[0x55] = '\0';
            pcVar14[0x56] = '\0';
            pcVar14[0x57] = '\0';
            *pfVar3 = *(float *)(iVar6 + 0xe8);
            *(undefined4 *)(pcVar14 + 0x38) = *(undefined4 *)(iVar6 + 0xec);
            *(undefined4 *)(pcVar14 + 0x3c) = *(undefined4 *)(iVar6 + 0xf0);
          }
          iVar6 = _rand();
          *(float *)(pcVar14 + 0x9c4) = (float)(iVar6 % 0x168);
          fVar21 = (float10)_DAT_00552a44;
          fVar20 = (float10)fsin((float10)*(float *)(pcVar14 + 0x30) * fVar21);
          *(float *)(pcVar14 + 0x9c8) = (float)(fVar20 * (float10)_DAT_005529ac);
          fVar21 = (float10)fcos((float10)*(float *)(pcVar14 + 0x30) * fVar21);
          fVar21 = fVar21 * (float10)_DAT_005529ac;
        }
        *(float *)(pcVar14 + 0x9cc) = (float)fVar21;
      }
      else if (iVar6 == 5) {
        pcVar14[0x9b8] = '\x02';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x54] = '\x03';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        iVar6 = _rand();
        *pfVar3 = 1.0;
        pcVar14[0x38] = '\0';
        pcVar14[0x39] = '\0';
        pcVar14[0x3a] = -0x80;
        pcVar14[0x3b] = '?';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *(float *)(pcVar14 + 0x9cc) = -(float)(iVar6 % 3 + 0x28);
      }
      else if (iVar6 == 7) {
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *pfVar3 = 0.2;
        pcVar14[0x38] = -0x33;
        pcVar14[0x39] = -0x34;
        pcVar14[0x3a] = 'L';
        pcVar14[0x3b] = '>';
        iVar6 = _rand();
        pcVar14[0x54] = '\x0f';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        *(int *)(pcVar14 + 0x9d4) = iVar6 % 10;
        *(int *)(pcVar14 + 0x9b8) = iVar6 % 10 + 0x1e;
        iVar6 = _rand();
        pcVar14[0xc] = '\0';
        pcVar14[0xd] = '\0';
        pcVar14[0xe] = -0x10;
        pcVar14[0xf] = 'A';
        *(float *)(pcVar14 + 0x9c4) = (float)(iVar6 % 3000);
      }
      else if (iVar6 == 8) {
        local_78 = 0.0;
        *pfVar2 = 0.0;
        pcVar14[0x2c] = '\0';
        pcVar14[0x2d] = '\0';
        pcVar14[0x2e] = '\0';
        pcVar14[0x2f] = '\0';
        pcVar14[0x30] = '\0';
        pcVar14[0x31] = '\0';
        pcVar14[0x32] = '\0';
        pcVar14[0x33] = '\0';
        pcVar14[0x9c4] = '\0';
        pcVar14[0x9c5] = '\0';
        pcVar14[0x9c6] = '\0';
        pcVar14[0x9c7] = '\0';
        pcVar14[0x9c8] = '\0';
        pcVar14[0x9c9] = '\0';
        pcVar14[0x9ca] = '\0';
        pcVar14[0x9cb] = '\0';
        pcVar14[0x9cc] = '\0';
        pcVar14[0x9cd] = '\0';
        pcVar14[0x9ce] = '\0';
        pcVar14[0x9cf] = '\0';
        *(float *)(pcVar14 + 0x1c) = *pfVar1;
        *(undefined4 *)(pcVar14 + 0x20) = *(undefined4 *)(pcVar14 + 0x14);
        *(undefined4 *)(pcVar14 + 0x24) = *(undefined4 *)(pcVar14 + 0x18);
        local_74 = -50.0;
        local_70 = 0.0;
        Matrix_BuildFromEuler((float *)(pcVar14 + 0x44),local_6c);
        Vector_Rotate(&local_78,local_6c,(float *)&local_90);
        *pfVar1 = (float)local_90 + *(float *)(pcVar14 + 0x1c);
        *(float *)(pcVar14 + 0x14) = *(float*)&local_8c + *(float *)(pcVar14 + 0x20);
        *(float *)(pcVar14 + 0x18) = *(float*)&local_88 + *(float *)(pcVar14 + 0x24);
      }
      else if (iVar6 == 9) {
        *pfVar2 = 0.0;
        pcVar14[0x9b8] = '\0';
        pcVar14[0x9b9] = '\0';
        pcVar14[0x9ba] = '\0';
        pcVar14[0x9bb] = '\0';
        pcVar14[0x2c] = '\0';
        pcVar14[0x2d] = '\0';
        pcVar14[0x2e] = '\0';
        pcVar14[0x2f] = '\0';
        pcVar14[0x30] = '\0';
        pcVar14[0x31] = '\0';
        pcVar14[0x32] = '\0';
        pcVar14[0x33] = '\0';
        *(float *)(pcVar14 + 0xc) = param_7;
        *pfVar3 = 0.3;
        pcVar14[0x38] = -0x66;
        pcVar14[0x39] = -0x67;
        pcVar14[0x3a] = -0x67;
        pcVar14[0x3b] = '>';
        pcVar14[0x54] = '\n';
        pcVar14[0x55] = '\0';
        pcVar14[0x56] = '\0';
        pcVar14[0x57] = '\0';
        pcVar14[0x3c] = '\0';
        pcVar14[0x3d] = '\0';
        pcVar14[0x3e] = -0x80;
        pcVar14[0x3f] = '?';
        *(float *)(pcVar14 + 0x1c) = (*param_3 - *pfVar1) * _DAT_00552a00;
        *(float *)(pcVar14 + 0x20) = (param_3[1] - *(float *)(pcVar14 + 0x14)) * _DAT_00552a00;
        *(float *)(pcVar14 + 0x24) = (param_3[2] - *(float *)(pcVar14 + 0x18)) * _DAT_00552a00;
        Matrix_BuildFromEuler(pfVar2,local_6c);
        iVar6 = 0;
        if (0 < *(int *)(pcVar14 + 0x54)) {
          do {
            *pfVar1 = *pfVar1 + *(float *)(pcVar14 + 0x1c);
            *(float *)(pcVar14 + 0x14) = *(float *)(pcVar14 + 0x14) + *(float *)(pcVar14 + 0x20);
            *(float *)(pcVar14 + 0x18) = *(float *)(pcVar14 + 0x24) + *(float *)(pcVar14 + 0x18);
            FUN_0046fe90((int)pcVar14,local_6c);
            iVar6 = iVar6 + 1;
          } while (iVar6 < *(int *)(pcVar14 + 0x54));
        }
      }
      else {
        if (iVar6 == 10) {
          *(float *)(pcVar14 + 0xc) = param_7;
          iVar6 = _rand();
          local_98 = (byte *)(iVar6 % 0x96);
          *(float *)(pcVar14 + 0x9cc) = (float)(int)local_98 * _DAT_00552594;
          iVar6 = _rand();
          local_98 = (byte *)(iVar6 % 500 + -0xfa);
          *(float *)(pcVar14 + 0x9c8) = (float)(int)local_98;
          if (_DAT_00552488 < *(float *)(pcVar14 + 0xc)) {
            pcVar14[0x9b8] = '2';
            pcVar14[0x9b9] = '\0';
            pcVar14[0x9ba] = '\0';
            pcVar14[0x9bb] = '\0';
            iVar6 = _rand();
            fVar24 = (float)(iVar6 % 0xfa + 200);
            goto LAB_0046f54e;
          }
          goto LAB_0046f55a;
        }
        if (iVar6 == 0xb) {
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = -0x80;
          pcVar14[0x3f] = '?';
          *pfVar3 = 0.2;
          pcVar14[0x38] = -0x33;
          pcVar14[0x39] = -0x34;
          pcVar14[0x3a] = 'L';
          pcVar14[0x3b] = '>';
          pcVar14[0x9d4] = '\0';
          pcVar14[0x9d5] = '\0';
          pcVar14[0x9d6] = '\0';
          pcVar14[0x9d7] = '\0';
          pcVar14[0x9b8] = '\x1e';
          pcVar14[0x9b9] = '\0';
          pcVar14[0x9ba] = '\0';
          pcVar14[0x9bb] = '\0';
          pcVar14[0x54] = '\x0f';
          pcVar14[0x55] = '\0';
          pcVar14[0x56] = '\0';
          pcVar14[0x57] = '\0';
          iVar6 = _rand();
          pcVar14[0xc] = '\0';
          pcVar14[0xd] = '\0';
          pcVar14[0xe] = -0x10;
          pcVar14[0xf] = 'A';
          *(float *)(pcVar14 + 0x9c4) = (float)(iVar6 % 3000);
        }
        else if (iVar6 == 0xd) {
          iVar6 = _rand();
          local_98 = (byte *)(iVar6 % 0x14 + 0x23);
          *(float *)(pcVar14 + 0xc) = param_7;
          *(float *)(pcVar14 + 0x9cc) = (float)(int)local_98;
          iVar6 = _rand();
          *pfVar3 = 0.5;
          pcVar14[0x38] = '\0';
          pcVar14[0x39] = '\0';
          pcVar14[0x3a] = '\0';
          pcVar14[0x3b] = '?';
          pcVar14[0x3c] = '\0';
          pcVar14[0x3d] = '\0';
          pcVar14[0x3e] = '\0';
          pcVar14[0x3f] = '?';
          *(int *)(pcVar14 + 0x9b8) = iVar6 % 0x32 + 0x19;
        }
      }
    }
    *(float *)(pcVar14 + 0x44) = *param_3;
    *(float *)(pcVar14 + 0x48) = param_3[1];
    *(float *)(pcVar14 + 0x4c) = param_3[2];
    goto switchD_0046dee7_default;
  }
  *pfVar3 = 1.0;
  pcVar14[0x38] = '\0';
  pcVar14[0x39] = '\0';
  pcVar14[0x3a] = -0x80;
  pcVar14[0x3b] = '?';
  pcVar14[0x3c] = '\0';
  pcVar14[0x3d] = '\0';
  pcVar14[0x3e] = -0x80;
  pcVar14[0x3f] = '?';
switchD_0046dee7_default:
  if (0x32 < *(int *)(pcVar14 + 0x54)) {
    pcVar14[0x54] = '2';
    pcVar14[0x55] = '\0';
    pcVar14[0x56] = '\0';
    pcVar14[0x57] = '\0';
  }
  {
    // 2026-09-03 -- DESVIACION DOCUMENTADA (no esta en IDA 0x46D840).
    //
    // El binario inicializa UNICAMENTE la fila 0 del anillo de segmentos
    // (`v11+22..v11+33` = 0x58..0x87, los 4 vertices) y deja el resto como
    // estaba.  En el juego original eso no se nota porque el pool de joints se
    // recicla sin parar: las filas altas conservan las coordenadas del joint
    // anterior, que son valores de mundo plausibles, asi que los quads de mas
    // salen diminutos o degenerados.
    //
    // En nuestro build el pool es un global en BSS: la PRIMERA vez que se usa
    // un slot esas filas valen 0, y el renderer (0x00473710) dibuja un quad
    // entre la ultima fila con datos y una fila en el origen -- la banda que
    // cruza la pantalla desde el personaje.  La muestran justo los tres joints
    // reportados: aura del Soul Barrier (266), efecto de subir de nivel y halo
    // del set +11 (1249).  Medido con la sonda JROWS: el vertice lejano salia
    // en (1046, 0, 295) con el cercano en (1112, 1489, 381).  Y verificado por
    // contraste: cuando el slot venia RECICLADO (con datos del joint anterior)
    // el anillo se comportaba perfecto durante 200 muestras seguidas.
    //
    // Replicar la fila 0 en todo el anillo reproduce la condicion que el
    // original obtiene gratis por reciclaje: los quads sobrantes quedan
    // degenerados sobre la propia posicion del joint en vez de barrer el mapa.
    // Solo toca filas que en un slot fresco valen 0, asi que no puede alterar
    // ningun joint que ya se estuviera dibujando bien.
    const int __rowBytes = 0x30;
    const int __maxRows  = (0x9d8 - 0x58) / __rowBytes;
    int __n = *(int *)(pcVar14 + 0x54);
    if (__n > __maxRows) __n = __maxRows;
    for (int __r = 1; __r < __n; ++__r)
        memcpy(pcVar14 + 0x58 + __r * __rowBytes, pcVar14 + 0x58, __rowBytes);
  }
  return (void*)pcVar14;
  while( true ) {
    uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
    pbVar10 = pbVar10 + 1;
    if ((uint)(uintptr_t)pbVar10 >= DAT_055c9bd4) break;
LAB_0046eb2f:
    bVar18 = false;
    iVar11 = 0;
    bVar19 = true;
    pbVar26 = (byte *)(DAT_055c9bd0 + uVar9 * 4);

    iVar6 = 4;
    pbVar8 = (byte*)&stack0xffffff60;
    pbVar17 = pbVar26;
    do {
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      bVar18 = *pbVar8 < *pbVar17;
      bVar19 = *pbVar8 == *pbVar17;
      pbVar8 = pbVar8 + 1;
      pbVar17 = pbVar17 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar11 == 0) goto LAB_0046eb9c;
    iVar6 = 4;
    bVar18 = false;
    iVar11 = 0;
    bVar19 = true;
    ppbVar15 = &local_88;
    do {
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      bVar18 = *(byte *)ppbVar15 < *pbVar26;
      bVar19 = *(byte *)ppbVar15 == *pbVar26;
      ppbVar15 = (byte **)((int)ppbVar15 + 1);
      pbVar26 = pbVar26 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar11 == 0) {
      if (uVar9 == 0xffffffff) goto LAB_0046eb9c;
      puVar13 = *(undefined4 **)(DAT_055c9bcc + uVar9 * 4);
      goto LAB_0046eb9e;
    }
  }
LAB_0046eb8a:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046eb9c:
  puVar13 = (undefined4 *)0x0;
LAB_0046eb9e:
  cVar4 = *(char *)(puVar13 + 0x161);
  *(byte *)(puVar13 + 0x161) = cVar4 + 1U;
  if ((byte)(cVar4 + 1U) < 2) {
    puVar7 = (undefined4*)operator_new(0x584);
    puVar16 = puVar7;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar16 = *puVar13;
      puVar13 = puVar13 + 1;
      puVar16 = puVar16 + 1;
    }
    uVar9 = 0x583;
    iVar6 = 0x584;
    do {
      if (uVar9 < 0x583) {
        *(byte *)(uVar9 + (int)puVar7) =
             *(byte *)(uVar9 + (int)puVar7) ^ *(byte *)(uVar9 + 1 + (int)puVar7);
      }
      bVar5 = *(char *)(uVar9 + (int)puVar7) - 0x23;
      uVar12 = uVar9 & 0x8000000f;
      *(byte *)(uVar9 + (int)puVar7) = bVar5;
      if ((int)uVar12 < 0) {
        uVar12 = (uVar12 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar9 + (int)puVar7) = (PacketXorKey16[uVar12] ^ bVar5) + 0xb9;
      uVar9 = uVar9 - 1;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
    puVar13 = puVar7;
    pbVar10 = local_8c;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *(undefined4 *)pbVar10 = *puVar13;
      puVar13 = puVar13 + 1;
      pbVar10 = pbVar10 + 4;
    }
    operator_delete((undefined *)puVar7);
  }
LAB_0046ec6d:
  pbVar10 = (byte*)DAT_07cf1ffc;
  iVar6 = DAT_055c9bc8;
  local_8c = (byte*)DAT_07cf1ffc;
  local_88 = (byte*)DAT_07cf1ffc;
  pcVar14[0x9d3] = ((byte*)DAT_07cf1ffc)[0x580];
  uVar9 = (**(code **)(iVar6 + 0xc))(pbVar10);
  pbVar10 = (byte *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      iVar11 = 4;
      pbVar26 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
      bVar18 = false;
      iVar6 = 0;
      bVar19 = true;
      pbVar8 = (byte*)&stack0xffffff5c;
      pbVar17 = pbVar26;
      do {
        if (iVar11 == 0) break;
        iVar11 = iVar11 + -1;
        bVar18 = *pbVar8 < *pbVar17;
        bVar19 = *pbVar8 == *pbVar17;
        pbVar8 = pbVar8 + 1;
        pbVar17 = pbVar17 + 1;
      } while (bVar19);
      if (!bVar19) {
        iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
      }
      if (iVar6 == 0) goto LAB_0046ee73;
      iVar11 = 4;
      bVar18 = false;
      iVar6 = 0;
      bVar19 = true;
      ppbVar15 = &local_8c;
      do {
        if (iVar11 == 0) break;
        iVar11 = iVar11 + -1;
        bVar18 = *(byte *)ppbVar15 < *pbVar26;
        bVar19 = *(byte *)ppbVar15 == *pbVar26;
        ppbVar15 = (byte **)((int)ppbVar15 + 1);
        pbVar26 = pbVar26 + 1;
      } while (bVar19);
      if (!bVar19) {
        iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
      }
      if (iVar6 == 0) {
        if (uVar9 == 0xffffffff) goto LAB_0046ee73;
        local_8c = (byte*)(uintptr_t)(int)local_90;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))((void*)(uintptr_t)(int)local_90);
        pbVar10 = (byte *)0x0;
        if (DAT_055c9bd4 != 0) goto LAB_0046ed54;
        goto LAB_0046edb0;
      }
      uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
      pbVar10 = pbVar10 + 1;
    } while ((uint)(uintptr_t)pbVar10 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
  goto LAB_0046ee73;
  while( true ) {
    uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
    pbVar10 = pbVar10 + 1;
    if ((uint)(uintptr_t)pbVar10 >= DAT_055c9bd4) break;
LAB_0046ed54:
    iVar11 = 4;
    pbVar26 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
    bVar18 = false;
    iVar6 = 0;
    bVar19 = true;
    pbVar8 = (byte*)&stack0xffffff5c;
    pbVar17 = pbVar26;
    do {
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      bVar18 = *pbVar8 < *pbVar17;
      bVar19 = *pbVar8 == *pbVar17;
      pbVar8 = pbVar8 + 1;
      pbVar17 = pbVar17 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar6 == 0) goto LAB_0046edc2;
    iVar11 = 4;
    bVar18 = false;
    iVar6 = 0;
    bVar19 = true;
    ppbVar15 = &local_8c;
    do {
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      bVar18 = *(byte *)ppbVar15 < *pbVar26;
      bVar19 = *(byte *)ppbVar15 == *pbVar26;
      ppbVar15 = (byte **)((int)ppbVar15 + 1);
      pbVar26 = pbVar26 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar6 == 0) {
      if (uVar9 == 0xffffffff) goto LAB_0046edc2;
      puVar13 = *(undefined4 **)(DAT_055c9bcc + uVar9 * 4);
      goto LAB_0046edc4;
    }
  }
LAB_0046edb0:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046edc2:
  puVar13 = (undefined4 *)0x0;
LAB_0046edc4:
  cVar4 = *(char *)(puVar13 + 0x161);
  *(char *)(puVar13 + 0x161) = cVar4 + -1;
  if ((char)(cVar4 + -1) == '\0') {
    pbVar8 = (byte*)operator_new(0x584);
    pbVar10 = (byte*)(uintptr_t)(int)local_90;
    pbVar26 = pbVar8;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *(undefined4 *)pbVar26 = *(undefined4 *)pbVar10;
      pbVar10 = pbVar10 + 4;
      pbVar26 = pbVar26 + 4;
    }
    uVar9 = 0;
    iVar6 = (int)local_90 - (int)pbVar8;
    pbVar10 = pbVar8;
    do {
      bVar5 = *pbVar10;
      uVar12 = uVar9 & 0x8000000f;
      *pbVar10 = bVar5 + 0x47;
      if ((int)uVar12 < 0) {
        uVar12 = (uVar12 - 1 | 0xfffffff0) + 1;
      }
      bVar5 = (PacketXorKey16[uVar12] ^ bVar5 + 0x47) + 0x23;
      *pbVar10 = bVar5;
      if (uVar9 < 0x583) {
        *pbVar10 = pbVar8[uVar9 + 1] ^ bVar5;
      }
      iVar11 = _rand();
      uVar9 = uVar9 + 1;
      pbVar10[iVar6] = (byte)iVar11;
      pbVar10 = pbVar10 + 1;
    } while (uVar9 < 0x584);
    pbVar10 = pbVar8;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar13 = *(undefined4 *)pbVar10;
      pbVar10 = pbVar10 + 4;
      puVar13 = puVar13 + 1;
    }
    operator_delete(pbVar8);
  }
LAB_0046ee73:
  pcVar14[0x9c0] = '\0';
  pcVar14[0x9c1] = '\0';
  pcVar14[0x9c2] = -0x10;
  pcVar14[0x9c3] = 'A';
  pcVar14[0x9b8] = '1';
  pcVar14[0x9b9] = '\0';
  pcVar14[0x9ba] = '\0';
  pcVar14[0x9bb] = '\0';
    // 2026-09-02: Ghidra tipo este slot como `float**` y le asigno `param_4`,
    // que es el PUNTERO al vec3 de angulos -- una direccion de pila.  El campo
    // es la **Scale** del joint (+0x0C).  Confirmado con MU 5.2 CreateJoint,
    // case 0 de BITMAP_JOINT_SPIRIT:  Velocity = 70; LifeTime = 49;
    // Scale = Scale; MaxTails = 6  -- los otros tres valores de este mismo
    // bloque coinciden exacto.  Medido con la sonda ESPIRIT JOINT:
    // `scaleBits=001AF32C` (una direccion de stack) en vez de 42A00000 (80.0f).
    *(float *)(pcVar14 + 0xc) = param_7;
  pcVar14[0x54] = '\f';
  pcVar14[0x55] = '\0';
  pcVar14[0x56] = '\0';
  pcVar14[0x57] = '\0';
  goto switchD_0046dee7_default;
  while( true ) {
    uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
    local_8c = local_8c + 1;
    if ((uint)(uintptr_t)local_8c >= DAT_055c9bd4) break;
LAB_0046e62c:
    bVar18 = false;
    iVar11 = 0;
    bVar19 = true;
    pbVar10 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
    iVar6 = 4;
    ppbVar15 = &local_88;
    pbVar26 = pbVar10;
    do {
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      bVar18 = *(byte *)ppbVar15 < *pbVar26;
      bVar19 = *(byte *)ppbVar15 == *pbVar26;
      ppbVar15 = (byte **)((int)ppbVar15 + 1);
      pbVar26 = pbVar26 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar11 == 0) goto LAB_0046e699;
    iVar6 = 4;
    bVar18 = false;
    iVar11 = 0;
    bVar19 = true;
    ppbVar15 = &local_84;
    do {
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      bVar18 = *(byte *)ppbVar15 < *pbVar10;
      bVar19 = *(byte *)ppbVar15 == *pbVar10;
      ppbVar15 = (byte **)((int)ppbVar15 + 1);
      pbVar10 = pbVar10 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar11 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar11 == 0) {
      if (uVar9 == 0xffffffff) goto LAB_0046e699;
      puVar13 = *(undefined4 **)(DAT_055c9bcc + uVar9 * 4);
      goto LAB_0046e69b;
    }
  }
LAB_0046e687:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046e699:
  puVar13 = (undefined4 *)0x0;
LAB_0046e69b:
  cVar4 = *(char *)(puVar13 + 0x161);
  *(byte *)(puVar13 + 0x161) = cVar4 + 1U;
  if ((byte)(cVar4 + 1U) < 2) {
    puVar7 = (undefined4*)operator_new(0x584);
    puVar16 = puVar7;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar16 = *puVar13;
      puVar13 = puVar13 + 1;
      puVar16 = puVar16 + 1;
    }
    uVar9 = 0x583;
    iVar6 = 0x584;
    do {
      if (uVar9 < 0x583) {
        *(byte *)(uVar9 + (int)puVar7) =
             *(byte *)(uVar9 + (int)puVar7) ^ *(byte *)(uVar9 + 1 + (int)puVar7);
      }
      bVar5 = *(char *)(uVar9 + (int)puVar7) - 0x23;
      uVar12 = uVar9 & 0x8000000f;
      *(byte *)(uVar9 + (int)puVar7) = bVar5;
      if ((int)uVar12 < 0) {
        uVar12 = (uVar12 - 1 | 0xfffffff0) + 1;
      }
      *(byte *)(uVar9 + (int)puVar7) = (PacketXorKey16[uVar12] ^ bVar5) + 0xb9;
      uVar9 = uVar9 - 1;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
    puVar13 = puVar7;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *unaff_EBX = *puVar13;
      puVar13 = puVar13 + 1;
      unaff_EBX = unaff_EBX + 1;
    }
    operator_delete((undefined *)puVar7);
  }
LAB_0046e76a:
  pbVar10 = (byte*)DAT_07cf1ffc;
  iVar6 = DAT_055c9bc8;
  local_8c = (byte*)DAT_07cf1ffc;
  local_88 = (byte*)DAT_07cf1ffc;
  pcVar14[0x9d3] = ((byte*)DAT_07cf1ffc)[0x580];
  uVar9 = (**(code **)(iVar6 + 0xc))(pbVar10);
  pbVar10 = (byte *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      iVar11 = 4;
      pbVar26 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
      bVar18 = false;
      iVar6 = 0;
      bVar19 = true;
      pbVar8 = (byte*)&stack0xffffff5c;
      pbVar17 = pbVar26;
      do {
        if (iVar11 == 0) break;
        iVar11 = iVar11 + -1;
        bVar18 = *pbVar8 < *pbVar17;
        bVar19 = *pbVar8 == *pbVar17;
        pbVar8 = pbVar8 + 1;
        pbVar17 = pbVar17 + 1;
      } while (bVar19);
      if (!bVar19) {
        iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
      }
      if (iVar6 == 0) goto LAB_0046e970;
      iVar11 = 4;
      bVar18 = false;
      iVar6 = 0;
      bVar19 = true;
      ppbVar15 = &local_8c;
      do {
        if (iVar11 == 0) break;
        iVar11 = iVar11 + -1;
        bVar18 = *(byte *)ppbVar15 < *pbVar26;
        bVar19 = *(byte *)ppbVar15 == *pbVar26;
        ppbVar15 = (byte **)((int)ppbVar15 + 1);
        pbVar26 = pbVar26 + 1;
      } while (bVar19);
      if (!bVar19) {
        iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
      }
      if (iVar6 == 0) {
        if (uVar9 == 0xffffffff) goto LAB_0046e970;
        local_8c = (byte*)(uintptr_t)(int)local_90;
        uVar9 = (**(code **)(DAT_055c9bc8 + 0xc))((void*)(uintptr_t)(int)local_90);
        pbVar10 = (byte *)0x0;
        if (DAT_055c9bd4 != 0) goto LAB_0046e851;
        goto LAB_0046e8ad;
      }
      uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
      pbVar10 = pbVar10 + 1;
    } while ((uint)(uintptr_t)pbVar10 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
  goto LAB_0046e970;
  while( true ) {
    uVar9 = (uVar9 + 1) % (uint)DAT_055c9bd4;
    pbVar10 = pbVar10 + 1;
    if ((uint)(uintptr_t)pbVar10 >= DAT_055c9bd4) break;
LAB_0046e851:
    iVar11 = 4;
    pbVar26 = (byte *)(DAT_055c9bd0 + uVar9 * 4);
    bVar18 = false;
    iVar6 = 0;
    bVar19 = true;
    pbVar8 = (byte*)&stack0xffffff5c;
    pbVar17 = pbVar26;
    do {
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      bVar18 = *pbVar8 < *pbVar17;
      bVar19 = *pbVar8 == *pbVar17;
      pbVar8 = pbVar8 + 1;
      pbVar17 = pbVar17 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar6 == 0) goto LAB_0046e8bf;
    iVar11 = 4;
    bVar18 = false;
    iVar6 = 0;
    bVar19 = true;
    ppbVar15 = &local_8c;
    do {
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      bVar18 = *(byte *)ppbVar15 < *pbVar26;
      bVar19 = *(byte *)ppbVar15 == *pbVar26;
      ppbVar15 = (byte **)((int)ppbVar15 + 1);
      pbVar26 = pbVar26 + 1;
    } while (bVar19);
    if (!bVar19) {
      iVar6 = (1 - (uint)bVar18) - (uint)(bVar18 != 0);
    }
    if (iVar6 == 0) {
      if (uVar9 == 0xffffffff) goto LAB_0046e8bf;
      puVar13 = *(undefined4 **)(DAT_055c9bcc + uVar9 * 4);
      goto LAB_0046e8c1;
    }
  }
LAB_0046e8ad:
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_0046e8bf:
  puVar13 = (undefined4 *)0x0;
LAB_0046e8c1:
  cVar4 = *(char *)(puVar13 + 0x161);
  *(char *)(puVar13 + 0x161) = cVar4 + -1;
  if ((char)(cVar4 + -1) == '\0') {
    pbVar8 = (byte*)operator_new(0x584);
    pbVar10 = (byte*)(uintptr_t)(int)local_90;
    pbVar26 = pbVar8;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *(undefined4 *)pbVar26 = *(undefined4 *)pbVar10;
      pbVar10 = pbVar10 + 4;
      pbVar26 = pbVar26 + 4;
    }
    uVar9 = 0;
    iVar6 = (int)local_90 - (int)pbVar8;
    pbVar10 = pbVar8;
    do {
      bVar5 = *pbVar10;
      uVar12 = uVar9 & 0x8000000f;
      *pbVar10 = bVar5 + 0x47;
      if ((int)uVar12 < 0) {
        uVar12 = (uVar12 - 1 | 0xfffffff0) + 1;
      }
      bVar5 = (PacketXorKey16[uVar12] ^ bVar5 + 0x47) + 0x23;
      *pbVar10 = bVar5;
      if (uVar9 < 0x583) {
        *pbVar10 = pbVar8[uVar9 + 1] ^ bVar5;
      }
      iVar11 = _rand();
      uVar9 = uVar9 + 1;
      pbVar10[iVar6] = (byte)iVar11;
      pbVar10 = pbVar10 + 1;
    } while (uVar9 < 0x584);
    pbVar10 = pbVar8;
    for (iVar6 = 0x161; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar13 = *(undefined4 *)pbVar10;
      pbVar10 = pbVar10 + 4;
      puVar13 = puVar13 + 1;
    }
    operator_delete(pbVar8);
  }
LAB_0046e970:
  pcVar14[0x9c0] = '\0';
  pcVar14[0x9c1] = '\0';
  pcVar14[0x9c2] = -0x74;
  pcVar14[0x9c3] = 'B';
  pcVar14[0x9b8] = '1';
  pcVar14[0x9b9] = '\0';
  pcVar14[0x9ba] = '\0';
  pcVar14[0x9bb] = '\0';
    // 2026-09-02: Ghidra tipo este slot como `float**` y le asigno `param_4`,
    // que es el PUNTERO al vec3 de angulos -- una direccion de pila.  El campo
    // es la **Scale** del joint (+0x0C).  Confirmado con MU 5.2 CreateJoint,
    // case 0 de BITMAP_JOINT_SPIRIT:  Velocity = 70; LifeTime = 49;
    // Scale = Scale; MaxTails = 6  -- los otros tres valores de este mismo
    // bloque coinciden exacto.  Medido con la sonda ESPIRIT JOINT:
    // `scaleBits=001AF32C` (una direccion de stack) en vez de 42A00000 (80.0f).
    *(float *)(pcVar14 + 0xc) = param_7;
  pcVar14[0x54] = '\x06';
  pcVar14[0x55] = '\0';
  pcVar14[0x56] = '\0';
  pcVar14[0x57] = '\0';
  goto switchD_0046dee7_default;
}

// IDA compatibility bridge: stubs_IDA_ports.cpp intentionally retains this ABI name.
void* __cdecl FUN_0046d840(int type, float* p1, float* p2, float* p3, unsigned int subType,
                            int owner, float scale, short link, unsigned char flag)
{
  return Joint_Create(type, p1, p2, p3, subType, owner, scale, link, flag);
}
