#include <cstdint>
// Terrain_Light.cpp
// Terrain light map computation and per-entity light colour lookup.
//
// RequestTerrainLight @ 0x004f7960 — Terrain_GetLightColor
// Entity_GetLightScale @ 0x004fa930 — Entity_GetLightScale
// Terrain_Water @ 0x004f95e0 — Terrain_ComputeLightMap
//
// Terrain light map:
//   DAT_07eab200 — float[256][256] per-tile brightness (wave-modulated)
//   DAT_081cb608 — float[256][256][3] per-tile RGB light colour (bilinear)
//   DAT_0828b608 — source RGB light colour table (read-only)
//   DAT_0839bc84 — flag: light map enabled
//   FrustrumBoundMinX_1/94 — current dirty rect corners

#include "stdafx.h"


// IDA: RequestTerrainLight (0x004F7960)
// Samples the light colour at world position (param_1, param_2) using
// bilinear interpolation over the 256x256 tile RGB map at DAT_081cb608.
// Requires map mode 5 (SceneFlag==5) and DAT_0839bc84 != 0.
// param_1: world X  param_2: world Y  param_3: output float[3] RGB
// On failure (out-of-bounds or wrong mode), writes (0,0,0).
void __cdecl RequestTerrainLight(float param_1,float param_2,float *param_3)
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

  if ((SceneFlag == 5) && (DAT_0839bc84 != '\0')) {
    fVar3 = param_1 * _DAT_00552594;
    fVar2 = param_2 * _DAT_00552594;
    lVar11 = (longlong)fVar3;   // IDA RequestTerrainLight: v3 = (std::int64_t)xfa  (xf*0.01)
    iVar4 = (int)lVar11;
    lVar11 = (longlong)fVar2;   // IDA RequestTerrainLight: v4 = (std::int64_t)yfa  (yf*0.01)
    iVar9 = (int)lVar11;
    if ((-1 < iVar4) && (((-1 < iVar9 && (iVar4 < 0xff)) && (iVar9 < 0xff)))) {
      iVar6 = iVar9 * 0x100 + iVar4;
      fVar2 = fVar2 - (float)iVar9;
      // DAT_081cb608 es DWORD: la aritmética va sobre char* para que el offset sea en
      // bytes (idx*3*4; disasm @ 0x004f7a13-28).
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


// Entity_GetLightScale — Entity_GetLightScale
// Reads the terrain light at entity param_1's world position and writes the
// RGB scale into render object param_2 at offsets +0x48/+0x4c/+0x50.
// Special cases:
//   type 0x145 → fixed white (0.6, 0.6, 0.6)
//   type 0xe1 with flag 0x200 → animated sine wave brightness
//   dark-map flag at param_1+0xdc → full light from map
//   otherwise → map light * 0.2 scale
void __cdecl Entity_GetLightScale(uintptr_t param_1,uintptr_t param_2)
{
  const OBJECT* object = AsObject(param_1);
  BMD* model = AsModel(param_2);
  float10 fVar1;
  // float[3] contiguo: RequestTerrainLight escribe 3 floats seguidos desde el
  // puntero de salida (Ghidra lo había partido en tres locals sueltos, que MSVC
  // no garantiza contiguos).
  float rgb[3] = {0.0f, 0.0f, 0.0f};
  float &local_c = rgb[0];
  float &local_8 = rgb[1];
  float &local_4 = rgb[2];

  if (object->Type == 0x145) {
    model->BodyLight[0] = 0.6f;
    model->BodyLight[1] = 0.6f;
    model->BodyLight[2] = 0.6f;
    return;
  }
  // El DWORD opaco +0x78 precede a los punteros: conservar su contrato de bytes.
  static_assert(offsetof(OBJECT, Teleport) == 0x7c);
  std::uint32_t ownerFlags = 0;
  if (object->Type == 0xe1)
    memcpy(&ownerFlags, reinterpret_cast<const BYTE*>(object->Owner) + 0x78, sizeof(ownerFlags));
  if ((object->Type == 0xe1) && ((ownerFlags & 0x200) == 0x200)) {
    fVar1 = (float10)fsin((float10)WorldTime * (float10)_DAT_00552a08);
    fVar1 = (fVar1 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
    model->BodyLight[0] = (float)fVar1;
    model->BodyLight[1] = (float)fVar1;
    model->BodyLight[2] = (float)fVar1;
    return;
  }
  model->LightEnable = object->LightEnable;
  if (object->LightEnable != '\0') {
    RequestTerrainLight(object->Position[0],object->Position[1],rgb);
    // Login banner (`0xa2` / MODEL_MUGAME): the overlay path in sub_440D50
    // multiplies the animated blend light by model BodyLight. In gameplay the
    // terrain sampler feeds that base color, but our login scene has no world
    // lightmap path, so rgb comes back (0,0,0) and every banner layer stays
    // black even though sub_4FDC00 ramps OBJECT.Light as expected.
    //
    // Keep this narrowly scoped to the login banner so we preserve the world
    // renderer semantics and avoid brightening unrelated objects.
    if ((SceneFlag == 2 || SceneFlag == 4) &&
        object->Type == 0xA2 &&
        rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
        rgb[0] = rgb[1] = rgb[2] = 1.0f;
    }
    // IDA-faithful nuance: en login/char-select (SceneFlag 2/4) el banner/logo
    // depende de que el terrain-light cero NO se reemplace por (1,1,1), para que
    // el fade venga puramente de OBJECT.Light (+0xe8..+0xf0). Mantener el fallback
    // fuera de esas escenas evita volver negro el mundo cuando el lightmap aún no
    // propagó, sin contaminar el intro.
    if (SceneFlag != 2 && SceneFlag != 4 &&
        rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
        rgb[0] = rgb[1] = rgb[2] = 1.0f;
    }
    model->BodyLight[0] = local_c + object->Light[0];
    model->BodyLight[1] = local_8 + object->Light[1];
    model->BodyLight[2] = local_4 + object->Light[2];
    return;
  }
  RequestTerrainLight(object->Position[0],object->Position[1],rgb);
  if ((SceneFlag == 2 || SceneFlag == 4) &&
      object->Type == 0xA2 &&
      rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
    rgb[0] = rgb[1] = rgb[2] = 1.0f;
  }
  if (SceneFlag != 2 && SceneFlag != 4 &&
      rgb[0] == 0.0f && rgb[1] == 0.0f && rgb[2] == 0.0f) {
    rgb[0] = rgb[1] = rgb[2] = 1.0f;
  }
  local_8 = local_8 * _DAT_005524f4;
  local_4 = local_4 * _DAT_005524f4;
  model->BodyLight[0] = local_c * _DAT_005524f4 + object->Light[0];
  model->BodyLight[1] = local_8 + object->Light[1];
  model->BodyLight[2] = local_4 + object->Light[2];
  return;
}


// Terrain_Water — MOVED TO src/render/Terrain_Water.cpp (canonical location)
// This duplicate definition is disabled to avoid LNK2005.
