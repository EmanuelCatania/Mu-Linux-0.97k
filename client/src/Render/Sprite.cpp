// Sprite.cpp
// RenderSprite_0 @ 0x00511d00  — Sprite_DrawTexturedQuad
//
// Draws a textured billboard quad (GL_QUADS) at a world position.
//
// Steps:
//  1. Bind texture slot via GL_BindTextureSlot(param_1)
//  2. Project the world position param_2 to screen via Vector_Transform
//     (stores result in local_ac / local_a8)
//  3. Build 4 corner positions (local_60[0..11]):
//     - If param_6 == 0.0: axis-aligned rect:
//         corners = center ± (param_3, param_4) in screen space
//     - If param_6 != 0.0: rotated rect:
//         local offsets at ±param_3/param_4, rotated by param_6 (degrees)
//         via Matrix_BuildFromEuler + Vector_Rotate
//  4. Build UV coords (local_9c[0..9]):
//     u0=param_7, v0=param_8+param_10, u1=param_9+param_7, v1=param_8
//     (arranged for 4 vertices of the quad)
//  5. glBegin(GL_QUADS=7)
//  6. glColor: if channel type == 3 → glColor3fv(param_5)
//             if channel == 0x4b6   → glColor4f(r,g,b,1.0)
//             otherwise             → glColor4f(r,g,b,r)
//  7. For each of the 4 corners: glTexCoord2f + glVertex3fv
//  8. glEnd()
//
// Parameters:
//   param_1  — texture/channel index (used for GL_BindTextureSlot + glColor mode)
//   param_2  — world position (float[3])
//   param_3  — half-width in world units
//   param_4  — half-height in world units
//   param_5  — colour (float[3])
//   param_6  — rotation angle (degrees, 0 = axis-aligned)
//   param_7  — UV offset U
//   param_8  — UV offset V
//   param_9  — UV size U
//   param_10 — UV size V
//
// Sub-functions:
//   GL_BindTextureSlot — GL_BindTextureSlot
//   Vector_Transform — World_ToScreen (projects param_2 using CameraMatrix matrix)
//   Matrix_BuildFromEuler — EulerToMatrix3x4
//   Vector_Rotate — Vec3_TransformByMatrix
//
// Globals:
//   CameraMatrix  — current view/projection matrix
//   DAT_083a7cc8  — per-channel flags table (stride 0x38, byte at +0 = type)
//   _DAT_00552504 — degrees-to-radians or scale constant
//   _DAT_00552580 — float 0.0

#include "stdafx.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
RenderSprite_0(int param_1,float *param_2,float param_3,float param_4,float *param_5,float param_6,
            float param_7,float param_8,float param_9,float param_10)

{
  int iVar1;
  int iVar2;
  float *pfVar3;
  float *pfVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float local_ac;
  float local_a8;
  undefined4 local_a4;
  float local_a0;
  float local_9c [5];
  float local_88;
  float local_84;
  float local_80;
  undefined4 local_7c;
  float local_78;
  float local_74;
  undefined4 local_70;
  float local_6c [3];
  // Las 12 vars (local_60[0..4] + local_4c..local_34) se iteran como 12 floats
  // consecutivos vía pfVar4 = local_60; pfVar4 += 3 (4× glVertex3fv). MSVC no
  // garantiza contigüidad: UN SOLO array de 12 floats con macros para mantener
  // los nombres originales como aliases.
  float local_corners_buf[12];
  #define local_60 local_corners_buf
  #define local_4c (*(undefined4*)&local_corners_buf[5])
  #define local_48 local_corners_buf[6]
  #define local_44 local_corners_buf[7]
  #define local_40 (*(undefined4*)&local_corners_buf[8])
  #define local_3c local_corners_buf[9]
  #define local_38 local_corners_buf[10]
  #define local_34 (*(undefined4*)&local_corners_buf[11])
  float local_30 [12];

  GL_BindTextureSlot(param_1);
  // local_ac/_a8/_a4: Vector_Transform escribe 3 floats consecutivos, así que se
  // usa un array TPos_buf[3] contiguo y se copia a las vars.
  float TPos_buf[3];
  Vector_Transform(param_2,(float *)&CameraMatrix, TPos_buf);
  local_ac = TPos_buf[0];
  local_a8 = TPos_buf[1];
  *(float*)&local_a4 = TPos_buf[2];
  local_9c[3] = param_3 * _DAT_00552504;
  local_a0 = local_a8;
  local_80 = param_4 * _DAT_00552504;
  // local_a4 guarda el bit-pattern de TPos[2]: reinterpretar los bits (el
  // decompile hace `(float)local_a4`, que sería un CAST int→float).
  float depth_eye = *(float*)&local_a4;
  if (param_6 == _DAT_00552580) {
    local_60[0] = local_ac - local_9c[3];
    local_60[2] = depth_eye;
    local_60[1] = local_a8 - local_80;
    local_4c = local_a4;
    local_40 = local_a4;
    local_60[3] = local_ac + local_9c[3];
    local_44 = local_a8 + local_80;
    local_34 = local_a4;
    local_60[4] = local_60[1];
    local_48 = local_60[3];
    local_3c = local_60[0];
    local_38 = local_44;
  }
  else {
    // Caso ROTADO (param_6 != 0): los offsets de las 4 esquinas se leen como 12
    // floats contiguos; array contiguo con los 4 offsets en el orden de IDA:
    //   BL(-hw,-hh) BR(+hw,-hh) TR(+hw,+hh) TL(-hw,+hh), z = depth_eye.
    // TODOS los sprites/flares/glows con rotación pasan por acá (partículas
    // 0x4e1 con frame≠0, weapon glow, etc.), no solo el char-select.
    float halfW = local_9c[3];   // param_3 * _DAT_00552504
    float halfH = local_80;      // param_4 * _DAT_00552504
    float rotOffsets[12] = {
        -halfW, -halfH, depth_eye,   // BL
         halfW, -halfH, depth_eye,   // BR
         halfW,  halfH, depth_eye,   // TR
        -halfW,  halfH, depth_eye    // TL
    };
    local_6c[0] = 0.0f;
    local_6c[1] = 0.0f;
    local_6c[2] = param_6;
    Matrix_BuildFromEuler(local_6c,local_30);
    iVar1 = 0;
    do {
      pfVar4 = (float *)((int)local_60 + iVar1);
      Vector_Rotate((float *)((int)rotOffsets + iVar1),local_30,pfVar4);
      iVar2 = iVar1 + 0xc;
      *pfVar4 = local_ac + *pfVar4;                        // + centerX
      *(float *)((int)local_60 + iVar1 + 4) = local_a0 + *(float *)((int)local_60 + iVar1 + 4);  // + centerY
      iVar1 = iVar2;
    } while (iVar2 < 0x30);
  }
  local_9c[2] = param_7 + param_9;
  local_84 = param_7;
  local_80 = param_8;
  local_9c[1] = param_8 + param_10;
  local_88 = param_8;
  local_9c[0] = param_7;
  local_9c[3] = local_9c[1];
  local_9c[4] = local_9c[2];
  // Los 8 texcoords se iteran como 8 floats consecutivos vía pfVar3 += 2
  // (4× glTexCoord2f): array contiguo con el layout de IDA:
  //   corner BL=(u, v+vH), BR=(u+uW, v+vH), TR=(u+uW, v), TL=(u, v)
  float local_tex[8] = {
      param_7,              // BL u
      param_8 + param_10,   // BL v
      param_7 + param_9,    // BR u
      param_8 + param_10,   // BR v
      param_7 + param_9,    // TR u
      param_8,              // TR v
      param_7,              // TL u
      param_8               // TL v
  };
  // ── DEPTH TEST: dejar habilitado. Mu usa GL_LEQUAL + glDepthMask(GL_FALSE)
  // (set por GL_DisableDepthWrites en blend setup), así sprites pueden ser ocluidos
  // por geometría más cercana pero NO escriben profundidad. Esto da forma
  // correcta a glows sobre armas/armaduras (siguen el contorno), en vez de
  // cuadrados blanco-fluo cubriendo todo.
  glBegin(7);
  if ((&DAT_083a7cc8)[param_1 * 0x38] == '\x03') {
    glColor3fv(param_5);
  }
  else {
    if (param_1 == 0x4b6) {
      fVar7 = param_5[2];
      fVar6 = param_5[1];
      fVar5 = *param_5;
      fVar8 = 1.0;
    }
    else {
      fVar5 = *param_5;
      fVar7 = param_5[2];
      fVar6 = param_5[1];
      fVar8 = fVar5;
      local_a0 = fVar5;
    }
    glColor4f(fVar5,fVar6,fVar7,fVar8);
  }
  pfVar4 = local_60;
  pfVar3 = local_tex;
  iVar1 = 4;
  do {
    glTexCoord2f(*pfVar3,pfVar3[1]);
    glVertex3fv(pfVar4);
    pfVar3 = pfVar3 + 2;
    pfVar4 = pfVar4 + 3;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  glEnd();
  return;
}
