// GL_State.cpp
// FUN_00511710 @ 0x00511710  — GL_SetBlendAdditive (blend type 3)
// FUN_00511790 @ 0x00511790  — GL_SetBlendSrcAlpha (blend type 4)
//
// These functions configure the OpenGL blending state for particle/effect
// rendering. Each one is a no-op if the blend mode is already current
// (cached in DAT_083a412c).
//
// IMPORTANT: IDA shows these wrappers do not toggle GL_DEPTH_TEST. They only
// switch blend func, cull face, depth mask, alpha test, texture-2D, and fog.
//
// Sub-functions (per 5.2 source helpers EnableCullFace/DisableCullFace/DepthMask):
//   GL_EnableDepthWrites — EnableDepthMask   (glDepthMask(GL_TRUE),  cache DAT_083a42e8)
//   GL_DisableDepthWrites — DisableDepthMask  (glDepthMask(GL_FALSE))
//   GL_EnableCullFace — EnableCullFace    (glEnable (GL_CULL_FACE=0xb44), cache DAT_083a411c)
//   GL_DisableCullFace — DisableCullFace   (glDisable(GL_CULL_FACE))
// 2D blend setters (types 2/3/4) call Disable*; 3D/reset setters (type 1 / reset)
// call Enable*, mirroring the Main 5.2 pattern where every blend-mode wrapper
// also toggles cull-face + depth-mask for the surfaces it's meant for.
//
// Globals:
//   DAT_083a412c  — current cached blend mode (int)
//   DAT_083a411d  — alpha-test enabled cache
//   DAT_083a4125  — texture-2D enabled cache
//   DAT_083a42ea  — fog enabled cache
//
// OpenGL enum constants:
//   0xbe2 = GL_BLEND
//   0xbc0 = GL_ALPHA_TEST
//   0xde1 = GL_TEXTURE_2D
//   0xb60 = GL_FOG
//   0x301 = GL_SRC_ALPHA

#include "stdafx.h"
extern "C" void DbgLogPublic(const char* msg);

void GL_SetBlendAdditive(void)

{
  if (DAT_083a412c != 3) {
    DAT_083a412c = 3;
    glEnable(0xbe2);
    glBlendFunc(1,1);
  }
  GL_DisableCullFace();
  GL_DisableDepthWrites();
  if (DAT_083a411d != '\0') {
    DAT_083a411d = '\0';
    glDisable(0xbc0);
  }
  if (DAT_083a4125 == '\0') {
    DAT_083a4125 = '\x01';
    glEnable(0xde1);
  }
  if (DAT_083a42ea != '\0') {
    glDisable(0xb60);
  }
  return;
}


void GL_SetBlendSrcAlpha(void)

{
  if (DAT_083a412c != 4) {
    DAT_083a412c = 4;
    glEnable(0xbe2);
    glBlendFunc(0,0x301);
  }
  GL_DisableCullFace();
  GL_DisableDepthWrites();
  if (DAT_083a411d != '\0') {
    DAT_083a411d = '\0';
    glDisable(0xbc0);
  }
  if (DAT_083a4125 == '\0') {
    DAT_083a4125 = '\x01';
    glEnable(0xde1);
  }
  if (DAT_083a42ea != '\0') {
    glEnable(0xb60);
  }
  return;
}


// FUN_00511480 @ 0x00511480 — GL_BindTextureSlot
// Binds texture slot param_1 (index into DAT_083a7ccc table, stride 0xe).
// Negative param_1: binds -param_1 directly as a GL texture handle.
// Caches last-bound slot in DAT_00561574 to avoid redundant rebinds.
void __cdecl GL_BindTextureSlot(int param_1)
{
  if (DAT_00561574 != param_1) {
    DAT_00561574 = param_1;
    if (-1 < param_1) {
      DWORD texture = (&DAT_083a7ccc)[param_1 * 0xe];

      // Diagnóstico sin efectos laterales: la instancia puede existir y aun
      // así resultar invisible si el slot no recibió textura/dimensiones.
      // Diagnostic only.  It is deliberately disabled in normal runs: this
      // path executes on every particle/sprite bind and is not present in the
      // original client; forwarding it to a debugger stalls the render loop.
      if (false && param_1 >= 1180 && param_1 <= 1300) {
        static DWORD lastWindow = 0;
        static unsigned count = 0;
        DWORD now = GetTickCount();
        if (now - lastWindow >= 1000) { lastWindow = now; count = 0; }
        if (count++ < 80) {
          char line[160];
          _snprintf_s(line, sizeof(line), _TRUNCATE,
                      "VIS Bind type=%d gl=%u size=(%.0f,%.0f) comp=%u",
                      param_1, (unsigned)texture,
                      *(float *)((char *)&DAT_083a7cc0 + param_1 * 0x38),
                      *(float *)((char *)&DAT_083a7cc4 + param_1 * 0x38),
                      (unsigned)(unsigned char)(&DAT_083a7cc8)[param_1 * 0x38]);
          DbgLogPublic(line);
        }
      }
      glBindTexture(0xde1, texture);
      return;
    }
    glBindTexture(0xde1,-param_1);
  }
  return;
}


// FUN_00511680 @ 0x00511680 — GL_SetBlendSrcOver (blend type 2)
// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
// Optionally enables depth writes, enables alpha test, enables texture-2D,
// and restores fog when the scene says it is on.
void __cdecl GL_SetBlendSrcOver(char param_1)
{
  if (DAT_083a412c != 2) {
    DAT_083a412c = 2;
    glEnable(0xbe2);
    glBlendFunc(0x302,0x303);
  }
  GL_DisableCullFace();
  if (param_1 != '\0') {
    GL_EnableDepthWrites();
  }
  if (DAT_083a411d == '\0') {
    DAT_083a411d = '\x01';
    glEnable(0xbc0);
  }
  if (DAT_083a4125 == '\0') {
    DAT_083a4125 = '\x01';
    glEnable(0xde1);
  }
  if (DAT_083a42ea != '\0') {
    glEnable(0xb60);
  }
  return;
}


// FUN_005111d0 @ 0x005111d0 — GL_GetModelViewMatrix
// Reads GL_MODELVIEW_MATRIX (0xba6) via glGetFloatv and repackages the
// first 3 columns (each 4 floats) row-by-row into param_1[0..11].
void __cdecl GL_GetModelViewMatrix(unsigned int *param_1)
{
  unsigned int uVar1;
  unsigned int *puVar2;
  int iVar3;
  unsigned int *puVar4;
  int iVar5;
  unsigned int local_40 [16];

  glGetFloatv(0xba6,(GLfloat*)local_40);
  puVar4 = local_40;
  iVar5 = 3;
  do {
    iVar3 = 4;
    puVar2 = puVar4;
    do {
      uVar1 = *puVar2;
      puVar2 = puVar2 + 4;
      *param_1 = uVar1;
      param_1 = param_1 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    puVar4 = puVar4 + 1;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  return;
}


// FUN_00511cf0 @ 0x00511cf0 — GL_BeginSprite
// Pushes current matrix and loads identity.
void GL_BeginSprite(void)
{
  glPushMatrix();
  glLoadIdentity();
  return;
}


// FUN_00511bc0 @ 0x00511bc0 — GL_EndOpenGL
// Balances BeginOpengl (pushes on PROJ + MV).
// Pops current mode (MODELVIEW after BeginBitmap teardown), then pops PROJECTION,
// then restores MODELVIEW as current mode.
void GL_EndOpenGL(void)
{
  glPopMatrix();
  glMatrixMode(0x1701u);   // GL_PROJECTION
  glPopMatrix();
  glMatrixMode(0x1700u);   // GL_MODELVIEW
  return;
}


// FUN_005142D0 @ 0x005142D0 — GL_SetFogHandle (legacy export: SetErrorMessage)
// Manages fog texture handle slots DAT_083a7c24/28.
// param_1 == 0: pop current into 0x24, clear 0x28.
// param_1 != 0 and 0x24 already set: write into 0x28.
// param_1 != 0 and 0x24 empty: write into 0x24.
void __cdecl SetErrorMessage(int param_1)
{
  undefined4 uVar1;

  uVar1 = DAT_083a7c28;
  if (param_1 == 0) {
    DAT_083a7c28 = 0;
    DAT_083a7c24 = uVar1;
    return;
  }
  if (DAT_083a7c24 != 0) {
    DAT_083a7c28 = param_1;
    return;
  }
  DAT_083a7c24 = param_1;
  return;
}


// FUN_00511600 @ 0x00511600 — GL_ResetState
// Resets blend to off, re-enables texture 2D, disables fog,
// enables depth test, and re-enables alpha test if flag is set.
void GL_ResetState(void)
{
  if (DAT_083a412c != 0) {
    DAT_083a412c = 0;
    glDisable(0xbe2);
  }
  GL_EnableCullFace();
  GL_EnableDepthWrites();
  if (DAT_083a411d != '\0') {
    DAT_083a411d = '\0';
    glDisable(0xbc0);
  }
  if (DAT_083a4125 == '\0') {
    DAT_083a4125 = '\x01';
    glEnable(0xde1);
  }
  if (DAT_083a42ea != '\0') {
    glEnable(0xb60);
  }
  return;
}


// FUN_00511890 @ 0x00511890 — GL_EnableLightMap
// Sets blend mode GL_ZERO/GL_SRC_COLOR for lightmap overlay rendering.
void GL_EnableLightMap(void) {
    if (DAT_083a412c != 1) {
        DAT_083a412c = 1;
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    }
    GL_EnableCullFace();
    GL_EnableDepthWrites();
    if (DAT_083a411d != '\0') {
        DAT_083a411d = '\0';
        glDisable(GL_FOG);
    }
    if (DAT_083a4125 == '\0') {
        DAT_083a4125 = '\x01';
        glEnable(GL_TEXTURE_2D);
    }
    if (DAT_083a42ea != '\0') {
        glEnable(GL_LIGHTING);
    }
}


// FUN_00511140 @ 0x00511140 — GL_CaptureScreenshot
// Reads current framebuffer into a heap buffer via glReadPixels (GL_RGB/GL_UNSIGNED_BYTE),
// encodes it to a JPEG via FUN_00529000, then increments a screenshot counter.
// Returns 1 when counter wraps past 10000 (i.e. the Nth screenshot), else 0.
int GL_CaptureScreenshot(void)
{
  int iVar1;
  undefined *puVar2;

  DAT_083a42f4 = 1;
  puVar2 = (undefined*)operator_new(DAT_00561570 * DAT_0056156c * 3);
  glReadPixels(0,0,DAT_0056156c,DAT_00561570,0x1907,0x1401,puVar2);
  FUN_00529000((const char*)&DAT_083a4174,DAT_0056156c,DAT_00561570,puVar2,100);
  operator_delete(puVar2);
  iVar1 = DAT_083a42f0 + 1;
  DAT_083a42f0 = iVar1 % 10000;
  return iVar1 / 10000;
}
