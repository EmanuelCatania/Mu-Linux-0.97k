// BMD_SetupRender.cpp
// BMD_SetupRenderByType @ 0x00504130 — BMD_SetupRenderByType
// Sets up bone color and animation state for BMD models based on effect/entity type.
// Called from entity rendering pipeline to apply type-specific color/anim overrides.
//
// param_1: model pointer (BMD model struct)
// param_2: entity/data pointer (entity struct base)
// param_3: model slot / type id
// param_4: alpha
// param_5: flags / blend mode
//
// Uses:
//   BMD__RenderMesh  (0x00440D50) — dibuja una malla del modelo
//   BMD__RenderBody  (0x00441E00) — recorre las mallas y llama a RenderMesh
//   BMD__BeginRender (0x00440D30) — glPushMatrix

#include "stdafx.h"
#include "Render/BMDDevice.h"
#include "Render/BufferedDraw.h"
#include "Render/MaterialVisuals.h"


void __cdecl BMD_SetupRenderByType(void *model, uintptr_t param_2, int param_3, float param_4, uint param_5)
{
  auto* param_1 = AsModel(model);
  const auto* owner = AsObject(param_2);
  float10 fVar2;
  float10 fVar3;
  float10 fVar4;
  uint uVar5;
  float fVar6;
  float fVar7;  // Conversión heredada en dos ramas, conservada para equivalencia x86.
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;

  if ((((param_3 == 0x29f) || (param_3 == 0x2df)) || (param_3 == 0x2bf)) || (param_3 == 0x2ff)) {
    // White knight / dark knight variant — full brightness
    { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }   // R=1.0
    { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }   // G=1.0
    { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }   // B=1.0

    fVar11 = owner->BlendMeshTexCoordV;
    fVar10 = owner->BlendMeshTexCoordU;
    fVar8  = owner->BlendMeshLight;
    memcpy(&fVar7, &owner->BlendMesh, sizeof(fVar7));
    fVar6  = owner->Alpha;
    uVar5  = 6;
  }
  else {
    if (((param_3 == 0x2a4) || (param_3 == 0x2e4)) || ((param_3 == 0x2c4 || (param_3 == 0x304)))) {
      // Variant with custom color anim
      { const std::uint32_t bits = 0x3f59999a; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }   // R≈0.85
      { const std::uint32_t bits = 0x3f59999a; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }   // G≈0.85
      { const std::uint32_t bits = 0x3f99999a; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }   // B≈1.2
      BMD__RenderBody(param_1, 6, owner->Alpha, owner->BlendMesh,
                   owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                   owner->BlendMeshTexCoordV, owner->HiddenMesh, 0x493);
      goto LAB_00504925;
    }
    if (param_3 != 0x316) {
      if (param_3 == 0x1a3) {
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, param_4, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     WorldTime * _DAT_00552868, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        return;
      }
      if (param_3 == 0x23a) {
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, 1.0f, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     WorldTime * _DAT_00552920, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        return;
      }
      if (param_3 == 0x222) {
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        BMD__RenderMesh(param_1, 0.0f, 10, param_4, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 0x12, 1.0f, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     WorldTime * _DAT_00552920, 0x492);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, param_4, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        return;
      }
      if (param_3 == 0x1af) {
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        BMD__BeginRender();
        // glColor3f espera float: leer como float, no como undefined4 (int → float = basura).
        BufferedDraw::Color3f(param_1->BodyLight[0], param_1->BodyLight[1],
                  param_1->BodyLight[2]);
        BMD__RenderMesh(param_1, 4.2039e-45f, 2, 1.0f, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_005524f8);
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, (float)fVar2, 1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        BMD__RenderMesh(param_1, 0.0f, 6, 1.0f, 0,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     WorldTime * _DAT_00552500, 0x492);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_00552500);
        fVar2 = (fVar2 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
        param_1->BodyLight[0] = (float)fVar2;
        param_1->BodyLight[1] = (float)fVar2;
        param_1->BodyLight[2] = (float)fVar2;
        fVar11 = WorldTime * _DAT_0055291c;
        fVar10 = WorldTime * _DAT_00552868;
        fVar9 = owner->BlendMeshLight;
      }
      else {
        if (param_3 == 0x1fa) {
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
          BMD__RenderBody(param_1, param_5, param_4, owner->BlendMesh,
                       owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                       owner->BlendMeshTexCoordV, owner->HiddenMesh, 0xffffffff);
          BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, 1,
                       owner->BlendMeshLight,
                       WorldTime * _DAT_00552868, WorldTime * _DAT_00552a40, 0xffffffff);
          return;
        }
        if (param_3 == 0x1d7) {
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
          { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
          BMD__RenderBody(param_1, param_5, param_4, owner->BlendMesh,
                       owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                       owner->BlendMeshTexCoordV, owner->HiddenMesh, 0xffffffff);
          fVar3 = (float10)WorldTime;
          fVar2 = (float10)_DAT_00552500;
          fVar4 = (float10)fsin(fVar3 * fVar2);
          fVar4 = fVar4 * (float10)_DAT_00552504 + (float10)_DAT_005528b8;
          param_1->BodyLight[0] = (float)fVar4;
          param_1->BodyLight[1] = (float)fVar4;
          param_1->BodyLight[2] = (float)fVar4;
          BMD__RenderMesh(param_1, 2.8026e-45f, 2, 1.0f, 2,
                       owner->BlendMeshLight,
                       (float)(fVar3 * fVar2), WorldTime * _DAT_0055291c, 0xffffffff);
          return;
        }
        if (param_3 != 0x25f) {
          if (param_3 == 0x260) {
            { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
            { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
            { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
            BMD__BeginRender();
            // Leer como float, no como undefined4 (int). Alpha 0x3f4ccccd=0.8f, 0x3f000000=0.5f.
            BufferedDraw::Color4f(param_1->BodyLight[0], param_1->BodyLight[1],
                      param_1->BodyLight[2], 0.8f);
            BMD__RenderMesh(param_1, 1.4013e-45f, 2, 0.8f, -1,
                         owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                         owner->BlendMeshTexCoordV, 0xffffffff);
            BufferedDraw::Color4f(param_1->BodyLight[0], param_1->BodyLight[1],
                      param_1->BodyLight[2], 0.5f);
            BMD__RenderMesh(param_1, 4.2039e-45f, 2, 0.5f, -1,
                         owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                         owner->BlendMeshTexCoordV, 0xffffffff);
            BufferedDraw::Color3f(param_1->BodyLight[0], param_1->BodyLight[1],
                      param_1->BodyLight[2]);
            BMD__RenderMesh(param_1, 0.0f, 2, 1.0f, -1,
                         owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                         owner->BlendMeshTexCoordV, 0xffffffff);
            BMD__RenderMesh(param_1, 2.8026e-45f, 2, 1.0f, 2,
                         owner->BlendMeshLight,
                         WorldTime * _DAT_00552a40, owner->BlendMeshTexCoordV, 0xffffffff);
            uVar5 = 0xffffffff;
            const auto &sample=MaterialVisuals_DrawSample((void *)param_2,param_3);
            fVar9 = sample.v;
            BMD__RenderMesh(param_1, 4.2039e-45f, 2, 1.0f, 3,
                         owner->BlendMeshLight,
                         sample.u, fVar9, uVar5);
            BMDDevice::EndModel();
            return;
          }
          BMD__RenderBody(param_1, param_5, param_4, owner->BlendMesh,
                       owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                       owner->BlendMeshTexCoordV, -1, 0xffffffff);
          return;
        }
        // param_3 == 0x25f
        BMD__BeginRender();
        { const std::uint32_t bits = 0x3e99999a; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3e99999a; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3e99999a; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        // 0x3e99999a = bits de 0.3f (≈ gris oscuro)
        BufferedDraw::Color3f(0.3f, 0.3f, 0.3f);
        BMD__RenderMesh(param_1, 2.8026e-45f, 1, 1.0f, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
        { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
        // 0x3f800000 = bits de 1.0f (blanco)
        BufferedDraw::Color3f(1.0f, 1.0f, 1.0f);
        BMD__RenderMesh(param_1, 2.8026e-45f, 0x44, 1.0f, 2,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     WorldTime * _DAT_005524f8, 0x492);
        BMD__RenderMesh(param_1, 0.0f, 2, 1.0f, -1,
                     owner->BlendMeshLight, owner->BlendMeshTexCoordU,
                     owner->BlendMeshTexCoordV, 0xffffffff);
        uVar5 = 0xffffffff;
        const auto &sample=MaterialVisuals_DrawSample((void *)param_2,param_3);
        fVar9 = sample.v;
        BMD__RenderMesh(param_1, 1.4013e-45f, 2, 1.0f, 1,
                     owner->BlendMeshLight,
                     sample.u, fVar9, uVar5);
        fVar2 = (float10)fsin((float10)WorldTime * (float10)_DAT_00552500);
        fVar2 = fVar2 * (float10)_DAT_005528b4 + (float10)_DAT_00552534;
        fVar9 = (float)fVar2;
        param_1->BodyLight[0] = (float)fVar2;
        param_1->BodyLight[1] = (float)fVar2;
        param_1->BodyLight[2] = (float)fVar2;
        BufferedDraw::Color3f(fVar9, fVar9, fVar9);
        fVar11 = WorldTime * _DAT_00552a40;
        fVar9  = owner->BlendMeshLight;
        fVar10 = WorldTime * _DAT_00552868;
      }
      BMD__RenderMesh(param_1, 2.8026e-45f, 0x42, 1.0f, 2, fVar9, fVar10, fVar11, 0xffffffff);
      BMDDevice::EndModel();
      return;
    }
    // param_3 == 0x316
    { const std::uint32_t bits = 0x3f4ccccd; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }   // R≈0.8
    { const std::uint32_t bits = 0x3f19999a; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }   // G≈0.6
    { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }   // B=1.0

    fVar11 = owner->BlendMeshTexCoordV;
    fVar10 = owner->BlendMeshTexCoordU;
    memcpy(&fVar7, &owner->BlendMesh, sizeof(fVar7));
    fVar6  = owner->Alpha;
    fVar8  = 0.5f;
    uVar5  = 0x44;
  }
  // fVar9 es reuso de registro de Hex-Rays: en los dos caminos que llegan aca
  // vale `unusedHiddenMeshBits` (HiddenMesh, bit-pattern).  Se pasa el
  // entero directo para no depender de esa reinterpretacion.
  BMD__RenderBody(param_1, uVar5, fVar6, fVar7, fVar8, fVar10, fVar11,
               owner->HiddenMesh, 0x493);
LAB_00504925:
  { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[0], &bits, sizeof(bits)); }
  { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[1], &bits, sizeof(bits)); }
  { const std::uint32_t bits = 0x3f800000; memcpy(&param_1->BodyLight[2], &bits, sizeof(bits)); }
  BMD__RenderBody(param_1, 2, owner->Alpha, owner->BlendMesh,
               owner->BlendMeshLight, owner->BlendMeshTexCoordU,
               owner->BlendMeshTexCoordV, owner->HiddenMesh, 0xffffffff);
  return;
}

// BMD__RotationPosition @ 0x00440a30 — BoneTransformOffset (sub_440A30 en IDA)
// Transforms pos_in through bone rotation, scales by model scale (this[+0x68]),
// stores result in pos_out, AND COPIES the bone matrix into the global root
// matrix DAT_06989c9c so that BMD_Animation, cuando procesa el
// root bone del ala/arma con parentIdx=-1 y param_7=='\x01', use esta matriz
// del bone padre del player como su "parent transform" → el modelo linked
// queda renderizado en la posición del hueso del player en lugar del origen.
//
// IDA original (sub_440A30 @ 0x00440A30):
//   void __thiscall sub_440A30(float *this, float in2[3][4], float in1[3], float *a4) {
//       float out[3];
//       VectorRotate(in1, in2, out);                  // pure rotation
//       *a4   = this[26] * out[0];                    // scale by this[+0x68] (=this[26])
//       a4[1] = this[26] * out[1];
//       a4[2] = this[26] * out[2];
//       memcpy(matrix, in2, 0x30);                    // copy 12 floats
//   }
//
// Sin la copia de matriz, las alas/armas se dibujarían en
// (entity_pos + rotated_offset_de_15) ≈ pies del char en lugar de la espalda.
void __cdecl BMD__RotationPosition(void *model, float *bone_mat, float *pos_in, float *pos_out)
{
  // 1. Rotate-only transform: pos_out = bone_rotation_3x3 * pos_in (no translation)
  Vector_Rotate(pos_in, bone_mat, pos_out);
  // 2. Scale by model scale (model[+0x68])
  if (model) {
    float sc = AsModel(model)->BodyScale;
    pos_out[0] *= sc;
    pos_out[1] *= sc;
    pos_out[2] *= sc;
  }
  // 3. Copy 12 floats (3x4 matrix) from bone_mat into global root matrix
  //    DAT_06989c9c — used by BMD_Animation root bone path.
  float* dst = (float*)&DAT_06989c9c;
  for (int i = 0; i < 12; ++i) dst[i] = bone_mat[i];
}

// Model_BoneParticle @ 0x004553c0 — Model_BoneParticle
// Emits a particle at the world position of a specific bone.
// model: BMD model ptr, type: entity type, bone_idx: bone index,
// scale: particle scale, color: float[3] RGB, entity: entity index
void __cdecl Model_BoneParticle(void *model, int type, int bone_idx, float scale, float *color, int entity)
{
  // IDA 0x004553C0 (la usan nueve sitios: brillo de arcos y bastones de
  // RenderLinkObject, el equipo del jugador y AttackEffect):
  //
  //   memset(v7, 0, sizeof(v7));
  //   TransformPosition(This, (float (*)[4])BoneMatrix[3 * a3], v7, Position, 1);
  //   return CreateSprite(Type, Position, Scale, Light, Owner, 0.0, 0);
  //
  // 1) La matriz sale del buffer global de huesos (`BoneMatrix` = 0x06970A9C,
  //    confirmado por xrefs), no del struct del BMD.  `BoneMatrix[3*a3]` con
  //    filas de float[4] son 48 bytes por hueso = bone_idx * 0x30.
  // 2) Los argumentos 3 y 4 de TransformPosition son ENTRADA y SALIDA: el sprite
  //    se emite con la posición transformada, no con la entrada.
  float in[3]  = { 0.0f, 0.0f, 0.0f };
  float world_pos[3];
  BMD_TransformPosition(model,
                        (float *)((char *)&DAT_06970a9c + bone_idx * 0x30),
                        in, world_pos, 1);
  CreateSprite((unsigned short)type, world_pos, scale, color, entity, 0.0f, 0);
}
