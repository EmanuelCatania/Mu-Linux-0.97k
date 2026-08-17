// Sprite.cpp
// FUN_00511d00 @ 0x00511d00  — Sprite_DrawTexturedQuad
//
// Draws a textured billboard quad (GL_QUADS) at a world position.
//
// Steps:
//  1. Bind texture slot via FUN_00511480(param_1)
//  2. Project the world position param_2 to screen via FUN_004fa170
//     (stores result in local_ac / local_a8)
//  3. Build 4 corner positions (local_60[0..11]):
//     - If param_6 == 0.0: axis-aligned rect:
//         corners = center ± (param_3, param_4) in screen space
//     - If param_6 != 0.0: rotated rect:
//         local offsets at ±param_3/param_4, rotated by param_6 (degrees)
//         via FUN_004f9db0 + FUN_004fa0b0
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
//   param_1  — texture/channel index (used for FUN_00511480 + glColor mode)
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
//   FUN_00511480 — GL_BindTexture
//   FUN_004fa170 — World_ToScreen (projects param_2 using DAT_083a4140 matrix)
//   FUN_004f9db0 — EulerToMatrix3x4
//   FUN_004fa0b0 — Vec3_TransformByMatrix
//
// Globals:
//   DAT_083a4140  — current view/projection matrix
//   DAT_083a7cc8  — per-channel flags table (stride 0x38, byte at +0 = type)
//   _DAT_00552504 — degrees-to-radians or scale constant
//   _DAT_00552580 — float 0.0

#include "stdafx.h"
extern "C" { void DbgLogPublic(const char* msg); }
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
FUN_00511d00(int param_1,float *param_2,float param_3,float param_4,float *param_5,float param_6,
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
  // ── BUG-FIX 2026-04-27: las 12 vars (local_60[0..4] + local_4c..local_34)
  // se iteraban como 12 floats consecutivos vía pfVar4 = local_60; pfVar4 += 3
  // 4× para emitir glVertex3fv. MSVC no garantiza contigüidad → corners 1-3
  // leían stack basura → sprites se veían como streaks horizontales en lugar
  // de quads. Fix: declarar como UN SOLO array de 12 floats con macros para
  // mantener nombres originales como aliases.
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

  FUN_00511480(param_1);
  // ── BUG-FIX 2026-04-27: local_ac/_a8/_a4 son 3 variables locales separadas;
  // MSVC no garantiza layout contiguo. Pasar &local_ac a FUN_004fa170 (que
  // escribe 3 floats consecutivos) puede dejar TPos[1]/TPos[2] en memoria
  // unrelated → sprite quad corner depth basura → sprites invisibles o
  // mal-clipeados. Usamos un array TPos_buf[3] contiguo y copiamos a las vars.
  float TPos_buf[3];
  FUN_004fa170(param_2,(float *)&DAT_083a4140, TPos_buf);
  local_ac = TPos_buf[0];
  local_a8 = TPos_buf[1];
  *(float*)&local_a4 = TPos_buf[2];
  local_9c[3] = param_3 * _DAT_00552504;
  local_a0 = local_a8;
  local_80 = param_4 * _DAT_00552504;
  // ── BUG-FIX 2026-04-27: local_a4 está declarado `undefined4` (int32). El
  // decompile original hace `(float)local_a4` que es CAST int→float (NO
  // reinterpret-bits). Como almacenamos el bit-pattern de TPos[2] (e.g.
  // -290.0f → 0xC3910000), el cast int→float lo lee como -1.01e9 → quad
  // corners con depth astronómica → fuera del frustum → invisible.
  // Usamos directamente el bit-pattern almacenado vía reinterpret.
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
    // ── BUG-FIX 2026-07-15: el caso ROTADO (param_6 != 0) leía los offsets de
    // las 4 esquinas de local_9c[0..4] + local_88/84/80/7c/78/74/70 — locales
    // SEPARADOS — como 12 floats contiguos vía `(float*)((int)local_9c+iVar1)`.
    // MSVC NO garantiza ese layout → esquinas 2-4 leían stack basura → el sprite
    // rotado se estiraba a posiciones random (haces/beams saliendo de los bordes
    // de la pantalla). Mismo patrón que los corners axis-aligned y los texcoords
    // (ya arreglados). Fix: array contiguo con los 4 offsets en el orden de IDA:
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
    FUN_004f9db0(local_6c,local_30);
    iVar1 = 0;
    do {
      pfVar4 = (float *)((int)local_60 + iVar1);
      FUN_004fa0b0((float *)((int)rotOffsets + iVar1),local_30,pfVar4);
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
  // ── BUG-FIX 2026-07-12: los 8 texcoords (local_9c[0..4] + local_80/84/88)
  // se iteraban como 8 floats consecutivos vía pfVar3 = local_9c; pfVar3 += 2
  // 4× para emitir glTexCoord2f. Igual que los corners, MSVC NO garantiza que
  // local_80/84/88 sigan contiguos a local_9c[4] → los texcoords de los corners
  // 3-4 leían stack basura → la textura del sprite muestreaba coords fuera de
  // rango (parte negra) → sprites/flares/glows INVISIBLES (samplean negro).
  // Fix: array contiguo de 8 floats con el layout correcto del IDA:
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
  // (set por FUN_00511530 en blend setup), así sprites pueden ser ocluidos
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
  pfVar3 = local_tex;   // BUG-FIX: array contiguo (era local_9c → texcoords basura)
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
