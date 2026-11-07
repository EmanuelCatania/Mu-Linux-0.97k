// Núcleo x86 congelado de b0a02c59 para comparación semántica de layouts.
void __cdecl BMD_SetupRenderByType(void *model, uintptr_t param_2, int param_3, float param_4, uint param_5)
{
  BYTE* param_1 = static_cast<BYTE*>(model);
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

    *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
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

      *(undefined4 *)(param_1 + 0x48) = 0x3f59999a;
      *(undefined4 *)(param_1 + 0x4c) = 0x3f59999a;
      *(undefined4 *)(param_1 + 0x50) = 0x3f99999a;
      BMD__RenderBody(param_1, 6, *(float *)(param_2 + 0x168), *(int *)(param_2 + 100),
                   *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                   *(float *)(param_2 + 0x70), *(int *)(param_2 + 0x58), 0x493);
      goto LAB_00504925;
    }
    if (param_3 != 0x316) {
      if (param_3 == 0x1a3) {
        *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     WorldTime * _DAT_00552868, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x23a) {
        *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     WorldTime * _DAT_00552920, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x222) {
        *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     WorldTime * _DAT_00552920, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        return;
      }
      if (param_3 == 0x1af) {
        *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
        BMD__BeginRender();

        BufferedDraw::Color3f(*(float *)(param_1 + 0x48), *(float *)(param_1 + 0x4c),
                  *(float *)(param_1 + 0x50));
        BMD__RenderMesh(param_1, 4.2039e-45f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_005524f8);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, (float)fVar2, 1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 6, 1.0f, 0,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     WorldTime * _DAT_00552500, 0x492);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_00552500);
        fVar2 = (fVar2 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
        *(float *)(param_1 + 0x48) = (float)fVar2;
        *(float *)(param_1 + 0x4c) = (float)fVar2;
        *(float *)(param_1 + 0x50) = (float)fVar2;
        fVar11 = WorldTime * _DAT_0055291c;
        fVar10 = WorldTime * _DAT_00552868;
        fVar9 = *(float *)(param_2 + 0x68);
      }
      else {
        if (param_3 == 0x1fa) {
          *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
          *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
          *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
          BMD__RenderBody(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), *(int *)(param_2 + 0x58), 0xffffffff);
          BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, 1,
                       *(float *)(param_2 + 0x68),
                       WorldTime * _DAT_00552868, WorldTime * _DAT_00552a40, 0xffffffff);
          return;
        }
        if (param_3 == 0x1d7) {
          *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
          *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
          *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
          BMD__RenderBody(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), *(int *)(param_2 + 0x58), 0xffffffff);
          fVar3 = (float10)WorldTime;
          fVar2 = (float10)_DAT_00552500;
          fVar4 = (float10)fsin(fVar3 * fVar2);
          fVar4 = fVar4 * (float10)_DAT_00552504 + (float10)_DAT_005528b8;
          *(float *)(param_1 + 0x48) = (float)fVar4;
          *(float *)(param_1 + 0x4c) = (float)fVar4;
          *(float *)(param_1 + 0x50) = (float)fVar4;
          BMD__RenderMesh(param_1, 2.8026e-45f, 2, 1.0f, 2,
                       *(float *)(param_2 + 0x68),
                       (float)(fVar3 * fVar2), WorldTime * _DAT_0055291c, 0xffffffff);
          return;
        }
        if (param_3 != 0x25f) {
          if (param_3 == 0x260) {
            *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
            *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
            *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
            BMD__BeginRender();

            BufferedDraw::Color4f(*(float *)(param_1 + 0x48), *(float *)(param_1 + 0x4c),
                      *(float *)(param_1 + 0x50), 0.8f);
            BMD__RenderMesh(param_1, 1.4013e-45f, 2, 0.8f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            BufferedDraw::Color4f(*(float *)(param_1 + 0x48), *(float *)(param_1 + 0x4c),
                      *(float *)(param_1 + 0x50), 0.5f);
            BMD__RenderMesh(param_1, 4.2039e-45f, 2, 0.5f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            BufferedDraw::Color3f(*(float *)(param_1 + 0x48), *(float *)(param_1 + 0x4c),
                      *(float *)(param_1 + 0x50));
            BMD__RenderMesh(param_1, 0.0f, 2, 1.0f, -1,
                         *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                         *(float *)(param_2 + 0x70), 0xffffffff);
            BMD__RenderMesh(param_1, 2.8026e-45f, 2, 1.0f, 2,
                         *(float *)(param_2 + 0x68),
                         WorldTime * _DAT_00552a40, *(float *)(param_2 + 0x70), 0xffffffff);
            uVar5 = 0xffffffff;
            const auto &sample=MaterialVisuals_DrawSample((void *)param_2,param_3);
            fVar9 = sample.v;
            BMD__RenderMesh(param_1, 4.2039e-45f, 2, 1.0f, 3,
                         *(float *)(param_2 + 0x68),
                         sample.u, fVar9, uVar5);
            BMDDevice::EndModel();
            return;
          }
          BMD__RenderBody(param_1, param_5, param_4, *(int *)(param_2 + 100),
                       *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                       *(float *)(param_2 + 0x70), -1, 0xffffffff);
          return;
        }

        BMD__BeginRender();
        *(undefined4 *)(param_1 + 0x48) = 0x3e99999a;
        *(undefined4 *)(param_1 + 0x4c) = 0x3e99999a;
        *(undefined4 *)(param_1 + 0x50) = 0x3e99999a;

        BufferedDraw::Color3f(0.3f, 0.3f, 0.3f);
        BMD__RenderMesh(param_1, 2.8026e-45f, 1, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
        *(undefined4 *)(param_1 + 0x50) = 0x3f800000;

        BufferedDraw::Color3f(1.0f, 1.0f, 1.0f);
        BMD__RenderMesh(param_1, 2.8026e-45f, 0x44, 1.0f, 2,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     WorldTime * _DAT_005524f8, 0x492);
        BMD__RenderMesh(param_1, 0.0f, 2, 1.0f, -1,
                     *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
                     *(float *)(param_2 + 0x70), 0xffffffff);
        uVar5 = 0xffffffff;
        const auto &sample=MaterialVisuals_DrawSample((void *)param_2,param_3);
        fVar9 = sample.v;
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, 1,
                     *(float *)(param_2 + 0x68),
                     sample.u, fVar9, uVar5);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_00552500);
        fVar2 = fVar2 * (float10)_DAT_005528b4 + (float10)_DAT_00552534;
        fVar9 = (float)fVar2;
        *(float *)(param_1 + 0x48) = (float)fVar2;
        *(float *)(param_1 + 0x4c) = (float)fVar2;
        *(float *)(param_1 + 0x50) = (float)fVar2;
        BufferedDraw::Color3f(fVar9, fVar9, fVar9);
        fVar11 = WorldTime * _DAT_00552a40;
        fVar9  = *(float *)(param_2 + 0x68);
        fVar10 = WorldTime * _DAT_00552868;
      }
      BMD__RenderMesh(param_1, 2.8026e-45f, 0x42, 1.0f, 2, fVar9, fVar10, fVar11, 0xffffffff);
      BMDDevice::EndModel();
      return;
    }

    *(undefined4 *)(param_1 + 0x48) = 0x3f4ccccd;
    *(undefined4 *)(param_1 + 0x4c) = 0x3f19999a;
    *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
    fVar9  = *(float *)(param_2 + 0x58);
    fVar11 = *(float *)(param_2 + 0x70);
    fVar10 = *(float *)(param_2 + 0x6c);
    fVar7  = *(float *)(param_2 + 100);
    fVar6  = *(float *)(param_2 + 0x168);
    fVar8  = 0.5f;
    uVar5  = 0x44;
  }



  BMD__RenderBody(param_1, uVar5, fVar6, fVar7, fVar8, fVar10, fVar11,
               *(int *)(param_2 + 0x58), 0x493);
LAB_00504925:
  *(undefined4 *)(param_1 + 0x48) = 0x3f800000;
  *(undefined4 *)(param_1 + 0x4c) = 0x3f800000;
  *(undefined4 *)(param_1 + 0x50) = 0x3f800000;
  BMD__RenderBody(param_1, 2, *(float *)(param_2 + 0x168), *(int *)(param_2 + 100),
               *(float *)(param_2 + 0x68), *(float *)(param_2 + 0x6c),
               *(float *)(param_2 + 0x70), *(int *)(param_2 + 0x58), 0xffffffff);
  return;
}
