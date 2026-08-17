// Terrain_Light.cpp
// Terrain light map computation and per-entity light colour lookup.
//
// FUN_004f7960 @ 0x004f7960 — Terrain_GetLightColor
// FUN_004fa930 @ 0x004fa930 — Entity_GetLightScale
// FUN_004f95e0 @ 0x004f95e0 — Terrain_ComputeLightMap
//
// Terrain light map:
//   DAT_07eab200 — float[256][256] per-tile brightness (wave-modulated)
//   DAT_081cb608 — float[256][256][3] per-tile RGB light colour (bilinear)
//   DAT_0828b608 — source RGB light colour table (read-only)
//   DAT_0839bc84 — flag: light map enabled
//   DAT_0839bc90/94 — current dirty rect corners

#include "stdafx.h"


// FUN_004f7960 — Terrain_GetLightColor
// Samples the light colour at world position (param_1, param_2) using
// bilinear interpolation over the 256x256 tile RGB map at DAT_081cb608.
// Requires map mode 5 (DAT_005615c0==5) and DAT_0839bc84 != 0.
// param_1: world X  param_2: world Y  param_3: output float[3] RGB
// On failure (out-of-bounds or wrong mode), writes (0,0,0).
void __cdecl FUN_004f7960(float param_1,float param_2,float *param_3)
{
  float fVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  float *pfVar5;
  int iVar6;
  float *pfVar7;
  float *pfVar8;
  int iVar9;
  float *pfVar10;
  longlong lVar11;

  if ((DAT_005615c0 == 5) && (DAT_0839bc84 != '\0')) {
    fVar3 = param_1 * _DAT_00552594;
    fVar2 = param_2 * _DAT_00552594;
    lVar11 = (longlong)fVar3;   // IDA RequestTerrainLight: v3 = (__int64)xfa  (xf*0.01)
    iVar4 = (int)lVar11;
    lVar11 = (longlong)fVar2;   // IDA RequestTerrainLight: v4 = (__int64)yfa  (yf*0.01)
    iVar9 = (int)lVar11;
    if ((-1 < iVar4) && (((-1 < iVar9 && (iVar4 < 0xff)) && (iVar9 < 0xff)))) {
      iVar6 = iVar9 * 0x100 + iVar4;
      fVar2 = fVar2 - (float)iVar9;
      // BUG-FIX: DAT_081cb608 es DWORD → &DAT_081cb608 + N*0xc hace aritmética
      // DWORD* (= +N*0xc*4 = +N*48 bytes). Disasm @ 0x004f7a13-28 muestra
      //   LEA <reg>,[<idx>*0x4 + 0x81cb608]  donde <idx> ya viene * 3
      // → byte offset = idx*3*4 = idx*12. Castear base a char* para byte arith.
      pfVar5 = (float *)((char*)&DAT_081cb608 + ((iVar9 + 1) * 0x100 + iVar4) * 0xc);
      iVar9 = 3;
      pfVar7 = (float *)((char*)&DAT_081cb608 + iVar6 * 0xc);
      pfVar8 = (float *)((char*)&DAT_081cb608 + (iVar6 * 3 + 3) * 4);
      pfVar10 = (float *)((char*)&DAT_081cb608 + (iVar6 * 3 + 0x303) * 4);
      do {
        fVar1 = *pfVar5;
        pfVar5 = pfVar5 + 1;
        iVar9 = iVar9 + -1;
        fVar1 = (fVar1 - *pfVar7) * fVar2 + *pfVar7;
        *param_3 = (((*pfVar10 - *pfVar8) * fVar2 + *pfVar8) - fVar1) * (fVar3 - (float)iVar4) +
                   fVar1;
        pfVar7 = pfVar7 + 1;
        pfVar8 = pfVar8 + 1;
        pfVar10 = pfVar10 + 1;
        param_3 = param_3 + 1;
      } while (iVar9 != 0);
      return;
    }
  }
  *param_3 = 0.0;
  param_3[1] = 0.0;
  param_3[2] = 0.0;
  return;
}


// FUN_004fa930 — Entity_GetLightScale
// Reads the terrain light at entity param_1's world position and writes the
// RGB scale into render object param_2 at offsets +0x48/+0x4c/+0x50.
// Special cases:
//   type 0x145 → fixed white (0.6, 0.6, 0.6)
//   type 0xe1 with flag 0x200 → animated sine wave brightness
//   dark-map flag at param_1+0xdc → full light from map
//   otherwise → map light * 0.2 scale
void __cdecl FUN_004fa930(int param_1,int param_2)
{
  float10 fVar1;
  // PORT FIX: Ghidra decompile produced three separate locals (local_c/8/4)
  // where the original binary had a contiguous float[3] on the stack.
  // FUN_004f7960 writes 3 floats starting at its output pointer, so the
  // locals MUST be contiguous. In MSVC, separate `float` declarations are
  // NOT guaranteed to be adjacent — so local_8/local_4 ended up reading
  // uninitialised stack slots, producing huge/subnormal values that were
  // added to the entity tint (+0xe8/+0xec/+0xf0) and written to the model's
  // bodyLight (+0x48/+0x4c/+0x50). Logo01/Logo03 at the login scene rendered
  // as corrupted coloured triangles because of this.  Using a proper float[3]
  // array guarantees contiguity and eliminates the uninitialised reads.
  float rgb[3] = {0.0f, 0.0f, 0.0f};
  float &local_c = rgb[0];
  float &local_8 = rgb[1];
  float &local_4 = rgb[2];

  if (*(short *)(param_1 + 2) == 0x145) {
    *(undefined4 *)(param_2 + 0x48) = 0x3f19999a;
    *(undefined4 *)(param_2 + 0x4c) = 0x3f19999a;
    *(undefined4 *)(param_2 + 0x50) = 0x3f19999a;
    return;
  }
  if ((*(short *)(param_1 + 2) == 0xe1) &&
     ((*(uint *)(*(int *)(param_1 + 0xfc) + 0x78) & 0x200) == 0x200)) {
    fVar1 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552a08);
    fVar1 = (fVar1 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
    *(float *)(param_2 + 0x48) = (float)fVar1;
    *(float *)(param_2 + 0x4c) = (float)fVar1;
    *(float *)(param_2 + 0x50) = (float)fVar1;
    return;
  }
  *(undefined1 *)(param_2 + 0x44) = *(undefined1 *)(param_1 + 0xdc);
  if (*(char *)(param_1 + 0xdc) != '\0') {
    FUN_004f7960(*(float *)(param_1 + 0x10),*(float *)(param_1 + 0x14),rgb);
    // Login banner (`0xa2` / MODEL_MUGAME): the overlay path in sub_440D50
    // multiplies the animated blend light by model BodyLight. In gameplay the
    // terrain sampler feeds that base color, but our login scene has no world
    // lightmap path, so rgb comes back (0,0,0) and every banner layer stays
    // black even though sub_4FDC00 ramps OBJECT.Light as expected.
    //
    // Keep this narrowly scoped to the login banner so we preserve the world
    // renderer semantics and avoid brightening unrelated objects.
    if ((DAT_005615c0 == 2 || DAT_005615c0 == 4) &&
        *(short *)(param_1 + 2) == 0xA2 &&
        rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
        rgb[0] = rgb[1] = rgb[2] = 1.0f;
    }
    // IDA-faithful nuance: en login/char-select (g_GameState 2/4) el banner/logo
    // depende de que el terrain-light cero NO se reemplace por (1,1,1), para que
    // el fade venga puramente de OBJECT.Light (+0xe8..+0xf0). Mantener el fallback
    // fuera de esas escenas evita volver negro el mundo cuando el lightmap aún no
    // propagó, sin contaminar el intro.
    if (DAT_005615c0 != 2 && DAT_005615c0 != 4 &&
        rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
        rgb[0] = rgb[1] = rgb[2] = 1.0f;
    }
    *(float *)(param_2 + 0x48) = local_c + *(float *)(param_1 + 0xe8);
    *(float *)(param_2 + 0x4c) = local_8 + *(float *)(param_1 + 0xec);
    *(float *)(param_2 + 0x50) = local_4 + *(float *)(param_1 + 0xf0);
    return;
  }
  FUN_004f7960(*(float *)(param_1 + 0x10),*(float *)(param_1 + 0x14),rgb);
  if ((DAT_005615c0 == 2 || DAT_005615c0 == 4) &&
      *(short *)(param_1 + 2) == 0xA2 &&
      rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
    rgb[0] = rgb[1] = rgb[2] = 1.0f;
  }
  if (DAT_005615c0 != 2 && DAT_005615c0 != 4 &&
      rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
    rgb[0] = rgb[1] = rgb[2] = 1.0f;
  }
  local_8 = local_8 * _DAT_005524f4;
  local_4 = local_4 * _DAT_005524f4;
  *(float *)(param_2 + 0x48) = local_c * _DAT_005524f4 + *(float *)(param_1 + 0xe8);
  *(float *)(param_2 + 0x4c) = local_8 + *(float *)(param_1 + 0xec);
  *(float *)(param_2 + 0x50) = local_4 + *(float *)(param_1 + 0xf0);
  return;
}


// FUN_004f95e0 — MOVED TO src/Render/Terrain_Water.cpp (canonical location)
// This duplicate definition is disabled to avoid LNK2005.
#if 0
int FUN_004f95e0_DISABLED(void)
{
  float fVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  float *pfVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  float10 fVar11;
  float10 fVar12;
  float10 fVar13;
  ulonglong uVar14;

  iVar10 = DAT_0055a778 + 3;
  uVar8 = DAT_0839bc94;
  if ((int)DAT_0839bc94 <= iVar10) {
    iVar4 = DAT_0055a774 + 3;
    uVar7 = DAT_0839bc94;
    do {
      if ((int)DAT_0839bc90 <= iVar4) {
        uVar5 = DAT_0839bc90;
        do {
          iVar2 = (uVar5 & 0xff) + (uVar7 & 0xff) * 0x100;
          iVar3 = iVar2 * 0xc;
          uVar5 = uVar5 + 1;
          *(undefined4 *)(&DAT_081cb608 + iVar3) = DAT_0828b608[iVar2 * 3];
          *(undefined4 *)(&DAT_081cb60c + iVar3) = (&DAT_0828b60c)[iVar2 * 3];
          *(undefined4 *)(&DAT_081cb610 + iVar3) = (&DAT_0828b610)[iVar2 * 3];
          uVar8 = DAT_0839bc94;
        } while ((int)uVar5 <= iVar4);
      }
      uVar7 = uVar7 + 1;
    } while ((int)uVar7 <= iVar10);
  }
  if (DAT_083a3ff0 == 0) {
    uVar14 = (longlong)DAT_05826e08;   // IDA Terrain_Water: (__int64)WorldTime
    uVar14 = (ulonglong)(uint)((int)uVar14 >> 0x1f) << 0x20 | uVar14 & 0xffffffff;
    iVar4 = (int)((longlong)uVar14 / 720000);
    fVar11 = (float10)(int)((longlong)uVar14 % 720000) * (float10)_DAT_005528e0;
  }
  else {
    uVar14 = (longlong)DAT_05826e08;   // IDA Terrain_Water: (__int64)WorldTime
    uVar14 = (ulonglong)(uint)((int)uVar14 >> 0x1f) << 0x20 | uVar14 & 0xffffffff;
    iVar4 = (int)((longlong)uVar14 / 36000);
    fVar11 = (float10)(int)((longlong)uVar14 % 36000) * (float10)_DAT_005524f8;
  }
  iVar2 = DAT_0055a7ac;
  if ((int)uVar8 <= iVar10) {
    iVar9 = uVar8 << 8;
    iVar3 = DAT_0055a774 + 3;
    iVar10 = (iVar10 - uVar8) + 1;
    do {
      fVar12 = (float10)(int)DAT_0839bc90;
      if ((int)DAT_0839bc90 <= iVar3) {
        iVar4 = (iVar3 - DAT_0839bc90) + 1;
        pfVar6 = (float *)(&DAT_07eab200 + (iVar9 + DAT_0839bc90) * 4);
        do {
          fVar1 = _DAT_00552660;
          if (iVar2 == 8) {
            fVar1 = _DAT_00552598;
          }
          iVar4 = iVar4 + -1;
          fVar13 = (float10)fsin(fVar12 * (float10)fVar1 + fVar11);
          *pfVar6 = (float)(fVar13 * (float10)_DAT_00552488);
          fVar12 = fVar12 + (float10)_DAT_0055256c;
          pfVar6 = pfVar6 + 1;
        } while (iVar4 != 0);
      }
      iVar9 = iVar9 + 0x100;
      iVar10 = iVar10 + -1;
    } while (iVar10 != 0);
  }
  return iVar4;
}
#endif  // disabled FUN_004f95e0 — see Terrain_Water.cpp
