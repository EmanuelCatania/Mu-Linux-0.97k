// GL_2D.cpp
// 2D overlay rendering via gluOrtho2D + GL_TRIANGLE_FAN (glBegin(6)).
// NOTA: glBegin(6) es GL_TRIANGLE_FAN, no GL_TRIANGLE_STRIP (confusion previa en comentarios).
//
// FUN_005123c0 @ 0x005123c0 — GL_Begin2D
// FUN_005124b0 @ 0x005124b0 — GL_End2D
// FUN_005124c0 @ 0x005124c0 — GL_DrawRect
// FUN_005125a0 @ 0x005125a0 — GL_DrawTexture
// FUN_00511c10 @ 0x00511c10 — GL_DrawBillboard
//
// Globals used:
//   DAT_0056156c / DAT_00561570 — viewport width / height
//   DAT_00561554               — perspective FOV angle
//   DAT_0056154c / DAT_00561550 — near / far clip planes

#include "stdafx.h"

extern "C" { void DbgLogPublic(const char*); }

// FUN_005123C0 @ 0x005123C0 — GL_Begin2D
// Sets up a 2D orthographic projection over the current viewport.
// Calls GL_DisableDepthTest to configure GL state for 2D (disable depth, etc.).
void GL_Begin2D(void)
{
  // BUG-FIX: DAT_00561554/4c/50 son DWORDs que ALMACENAN bits de float (FOV/near/far).
  // El decompile de Ghidra los castea como (double)DWORD (interpretando como int) →
  // 45.0f bit-pattern (0x42340000 = 1110704128) se convierte en FOV=1.1e9 → matriz
  // degenerada (NaN) → el driver NVIDIA crashea en la siguiente llamada GL de estado.
  // Leer correctamente como float via puntero float*.
  float fov  = *(float*)&DAT_00561554;
  float near_ = *(float*)&DAT_0056154c;
  float far_ = *(float*)&DAT_00561550;
  // Defensa: si el bloque init no corrió aún, fallback a valores razonables.
  if (!(fov > 0.0f && fov < 180.0f))   fov   = 45.0f;
  if (!(near_ > 0.0f))                  near_ = 10.0f;
  if (!(far_ > near_))                  far_  = 10000.0f;
  DWORD vw = DAT_0056156c ? DAT_0056156c : 640;
  DWORD vh = DAT_00561570 ? DAT_00561570 : 480;

  // BUG-FIX 2026-06-28 (5.2 source ZzzOpenglUtil.cpp:1117): el 0.97k empuja la
  // 1ª matriz sobre el modo de ENTRADA (no explícito) y la 2ª sobre PROJECTION,
  // dejando el balance dependiente del modo actual.  5.2 empuja explícitamente
  // PROJECTION luego MODELVIEW.  Combinado con el fix de EndBitmap, balancea
  // exacto (1 PROJECTION + 1 MODELVIEW) y elimina el leak de PROJECTION.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glViewport(0,0,vw,vh);
  gluPerspective((double)fov, (double)vw / (double)vh,
                 (double)near_, (double)far_);
  glLoadIdentity();
  gluOrtho2D(0,(double)vw,0,(double)vh);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  GL_DisableDepthTest();
  return;
}


// FUN_005124B0 @ 0x005124B0 — GL_End2D
// Pops both matrix stacks pushed by GL_Begin2D.
void GL_End2D(void)
{
  // BUG-FIX 2026-06-28 (5.2 source ZzzOpenglUtil.cpp:1136): el 0.97k original
  // hacía `glPopMatrix(); glPopMatrix();` SIN cambiar de modo → ambos pops caían
  // sobre MODELVIEW (el modo activo al salir de BeginBitmap).  Resultado: nunca
  // se popeaba PROJECTION (que BeginBitmap había empujado) → PROJECTION acumula
  // +1/frame → GL_STACK_OVERFLOW (0x503); y EndOpengl, al popear MODELVIEW de
  // nuevo, generaba GL_STACK_UNDERFLOW (0x504).  Corrige popeando explícito
  // 1 PROJECTION + 1 MODELVIEW, balanceando exacto con BeginBitmap.
  // NOTA vs 5.2: 5.2 popea MODELVIEW→PROJECTION (queda en modo PROJECTION).
  // Acá popeamos PROJECTION→MODELVIEW para DEJAR el modo en MODELVIEW, porque
  // nuestro EndOpengl (0.97k IDA) asume MODELVIEW como modo activo de entrada
  // (su 1er glPopMatrix es sobre el modo actual).  Balance idéntico (1+1).
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  return;
}


// FUN_005124C0 @ 0x005124C0 — GL_DrawRect
// Draws a 2D filled rectangle (no texture) using GL_TRIANGLE_FAN (glBegin(6)).
// Vertices TL, BL, BR, TR forman un abanico (fan) con v0=TL como pivote:
//   tri1 = (TL, BL, BR), tri2 = (TL, BR, TR).
// Coordinates are in screen pixels; Y is flipped relative to viewport height.
// param_1: x,  param_2: y,  param_3: width,  param_4: height
//
// 2026-05-04 BUG-FIX: Ghidra decompile splittered the original contiguous
// stack array of 8 floats into `local_20[5] + local_c + local_8 + local_4`.
// The walker `for(i=0;i<4;i++) { glVertex2f(*p, p[1]); p+=2; }` assumed
// 8 contiguous floats, but MSVC is free to reorder/separate the named
// locals → vertex 4 (and possibly 3) read garbage from stack → 4th corner
// degenerated → rect renders as a triangle (visible as the yellow EXP bar
// and hover highlights showing as triangles instead of bars).
// Fix: explicit contiguous float[8].
void __cdecl GL_DrawRect(float param_1,float param_2,float param_3,float param_4)
{
  float verts[8];   // 4 vertices × 2 floats = 8

  GL_SetAlphaTest('\0');
  float x      = (float)Screen_ToGLX(param_1);
  float y_off  = (float)Screen_ToGLY(param_2);
  float w      = (float)Screen_ToGLX(param_3);
  float h      = (float)Screen_ToGLY(param_4);
  float y_top    = (float)DAT_00561570 - y_off;
  float y_bottom = y_top - h;

  verts[0] = x;       verts[1] = y_top;     // TL
  verts[2] = x;       verts[3] = y_bottom;  // BL
  verts[4] = x + w;   verts[5] = y_bottom;  // BR
  verts[6] = x + w;   verts[7] = y_top;     // TR

  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < 4; ++i) {
      glVertex2f(verts[i*2 + 0], verts[i*2 + 1]);
  }
  glEnd();
}


// FUN_005125A0 @ 0x005125A0 — GL_DrawTexture
// Draws a textured 2D quad (GL_TRIANGLE_FAN, glBegin(6)) using GL_BindTextureSlot to bind texture.
// Vertices: TL, BL, BR, TR — UVs (u0,v0), (u0,v0+dv), (u0+du,v0+dv), (u0+du,v0).
// param_1:  texture slot index
// param_2/3: x, y position (screen pixels)
// param_4/5: width, height (screen pixels)
// param_6/7: tex U0, V0 (top-left UV)
// param_8/9: tex dU, dV (UV size)
// param_10: if non-zero, scale x/y by screen mapping helpers
// param_11: if non-zero, scale width/height by screen mapping helpers
void __cdecl
GL_DrawTexture(int param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7,float param_8,float param_9,char param_10,char param_11)
{
  int iVar1;
  float10 fVar2;
  float local_40 [16];

  if (param_11 != '\0') {
    fVar2 = Screen_ToGLX(param_2);
    param_2 = (float)fVar2;
    fVar2 = Screen_ToGLY(param_3);
    param_3 = (float)fVar2;
  }
  if (param_10 != '\0') {
    fVar2 = Screen_ToGLX(param_4);
    param_4 = (float)fVar2;
    fVar2 = Screen_ToGLY(param_5);
    param_5 = (float)fVar2;
  }
  GL_BindTextureSlot(param_1);
  // DIAG: log first 40 UI quad draws to see what 2D elements appear on screen
  {
      static int s_q = 0;
      if (s_q < 40) {
          char m[160];
          _snprintf_s(m, sizeof(m), _TRUNCATE,
              "UI_Quad[%d] tex=0x%x xy=(%.0f,%.0f) wh=(%.0f,%.0f)",
              s_q, param_1, param_2, param_3, param_4, param_5);
          DbgLogPublic(m); s_q++;
      }
  }
  local_40[10] = param_2;
  local_40[9] = (float)DAT_00561570 - param_3;
  local_40[8] = param_2;
  local_40[0] = param_6;
  local_40[1] = param_7;
  local_40[2] = param_6;
  local_40[0xb] = local_40[9] - param_5;
  local_40[0xc] = param_2 + param_4;
  local_40[4] = param_6 + param_8;
  local_40[7] = param_7;
  local_40[3] = param_7 + param_9;
  local_40[5] = local_40[3];
  local_40[6] = local_40[4];
  local_40[0xd] = local_40[0xb];
  local_40[0xe] = local_40[0xc];
  local_40[0xf] = local_40[9];
  // BUG-FIX: la decompile original usaba *(undefined4*) = *(unsigned int*) lo
  // cual al pasar a glTexCoord2f/glVertex2f hacia conversion int→float,
  // corrompiendo las coords (0x3f800000 → 1065353216.0f en vez de 1.0f).
  // Leer como float via puntero float*.
  glBegin(6);   // GL_TRIANGLE_FAN
  iVar1 = 0;
  do {
    float *uv  = (float *)((char *)local_40 + iVar1);
    float *pos = (float *)((char *)local_40 + iVar1 + 0x20);
    glTexCoord2f(uv[0], uv[1]);
    glVertex2f(pos[0], pos[1]);
    iVar1 = iVar1 + 8;
  } while (iVar1 < 0x20);
  glEnd();
  return;
}


// FUN_00511C10 @ 0x00511C10 — GL_DrawBillboard
// Draws a world-space billboard quad of half-extents (param_1, param_2).
// param_3: view matrix (float[12]) used to transform the 4 corner vertices
//          via Vector_Transform (Vec3_TransformByMatrix_WithTranslate).
// Emits GL_QUADS (glBegin(7)) with UV corners (0,1), (1,1), (1,0), (0,0).
void __cdecl GL_DrawBillboard(float param_1,float param_2,float *param_3)
{
  // ── 2026-08-16: patron [[locales-contiguos-ghidra]] (5ta instancia) ────────
  // IDA `sub_511C10` recorre `in1[0..11]` y `v[0..11]` como 4 vec3 cada uno:
  //     for (i = 0; i < 12; i += 3) VectorTransform(&in1[i], in2, &v[i]);
  // Ghidra emitio ese frame como escalares SUELTOS (local_60[4] + local_50,
  // local_4c, local_48, local_44, local_40, local_3c, local_38, local_34 /
  // local_30[3] + local_24 + local_18 + local_c). MSVC no garantiza que queden
  // contiguos, asi que los vertices 2, 3 y 4 salian de memoria basura y se
  // escribian en lugares arbitrarios => quads desbocados = los CUADROS BLANCOS
  // de los efectos de skill (este es el billboard 3D que usa SkillEffect_Render
  // en todos los mapas salvo World 2).
  // Mapeo por offset de frame (ebp):
  //   in1[0..2]  = -0x60,-0x5c,-0x58   in1[3..5]  = -0x54,-0x50,-0x4c
  //   in1[6..8]  = -0x48,-0x44,-0x40   in1[9..11] = -0x3c,-0x38,-0x34
  //   out[0..2]  = -0x30 (v)   [3..5] = -0x24 (v16)
  //   out[6..8]  = -0x18 (v17) [9..11]= -0x0c (v18)
  float in1[12], out[12];
  const float nx = -param_1;
  const float nz = -param_2;
  in1[0] = nx;      in1[1]  = nx;      in1[2]  = nz;
  in1[3] = param_1; in1[4]  = param_1; in1[5]  = nz;
  in1[6] = param_1; in1[7]  = param_1; in1[8]  = param_2;
  in1[9] = nx;      in1[10] = nx;      in1[11] = param_2;

  for (int i = 0; i < 12; i += 3) {
    Vector_Transform(&in1[i], param_3, &out[i]);
  }

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);  glVertex3fv(&out[0]);
  glTexCoord2f(1.0f, 1.0f);  glVertex3fv(&out[3]);
  glTexCoord2f(1.0f, 0.0f);  glVertex3fv(&out[6]);
  glTexCoord2f(0.0f, 0.0f);  glVertex3fv(&out[9]);
  glEnd();
}
