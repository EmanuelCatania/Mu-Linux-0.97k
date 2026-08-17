// Joint_Render.cpp
// FUN_0046c3e0 @ 0x0046c3e0
//
// Joint_RenderAll — renders all active joints as textured triangle strips.
//
// Iterates the joint render pool starting at DAT_07c608b4 (stride 0x2f0
// bytes = 0xbc int-words).  For each active slot (piVar2[-3] != 0):
//   - Selects blend mode based on joint type and blink flag (+0x1be):
//       type < 3 and !blink → FUN_00511710 (additive)
//       otherwise           → FUN_00511790 (src-alpha)
//   - Binds the texture via FUN_00511480(type + 0x48d)
//   - Draws (segCount - 1) quad strips using GL_TRIANGLE_STRIP (glBegin 6):
//       Each segment has 4 vertices (top/bottom of current and next node)
//       with UV coords interpolated along the strip.
//       Colour fades from 1.0 → 0.0 along the strip when !blink.
//
// Pool slot layout (relative to piVar2, which is &slot[3]):
//   piVar2[-3] (byte) — active flag
//   piVar2[-2] (int)  — joint sub-type (0–5)
//   piVar2[-1]        — unused / padding
//   piVar2[0]  (int*) — pointer to model/entity base (for blink flag)
//   piVar2[1]  (int)  — segment count
//   piVar2[2]  (int)  — colour R (float reinterpreted as int)
//   piVar2[3]  (int)  — colour G
//   piVar2[4]  (int)  — colour B
//   piVar2[0x5f..]    — vertex positions (stride 3 floats = 12 bytes)
//                       relative to piVar2+0x5f, each vertex 3 floats
//
// Sub-functions:
//   FUN_00511710 — GL_SetBlendAdditive (blend type 3)
//   FUN_00511790 — GL_SetBlendSrcAlpha (blend type 4)
//   FUN_00511480 — GL_BindTexture (texture slot)
//
// Globals:
//   DAT_07c608b4 — joint render pool base (ends at 0x7c72e74)
//   _DAT_0055256c — float 1.0

#include "stdafx.h"

extern "C" void DbgLogPublic(const char* msg);


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// 2026-05-03: AUTO-SKIP removed. Pool now properly sized in globals.cpp
// (g_RenderPool_07c608a8 = 100 slots × 0x2f0). DAT_07c608b4 is the +12
// anchor inside slot[0]. Walk replaced with explicit count.
void FUN_0046c3e0(void)

{
  float fVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int local_8;

  piVar2 = (int*)&DAT_07c608b4;
  for (int slotIdx = 0; slotIdx < 100; ++slotIdx, piVar2 += 0xbc) {
    if ((char)piVar2[-3] != '\0') {
      iVar3 = piVar2[-2];
      {
        char trailLog[192];
        wsprintfA(trailLog,
                  "TRAIL slot=%d owner=%08X type=%d seg=%d base=%08X hero=%08X chars=%08X",
                  slotIdx, (unsigned)*piVar2, iVar3, piVar2[1],
                  (unsigned)(uintptr_t)g_RenderPool_07c608a8,
                  (unsigned)(uintptr_t)DAT_07abf5d8, (unsigned)DAT_07abf5d0);
        DbgLogPublic(trailLog);
      }
      if ((*(short *)(*piVar2 + 0x1be) == 0) && (iVar3 < 3)) {
        FUN_00511710();
      }
      else {
        FUN_00511790();
      }
      if (2 < iVar3) {
        iVar3 = iVar3 + -3;
      }
      if (1 < piVar2[1]) {
        FUN_00511480(iVar3 + 0x48d);
        local_8 = 0;
        if (piVar2[1] != 1 && -1 < piVar2[1] + -1) {
          piVar4 = piVar2 + 0x5f;
          do {
            glBegin(6);
            fVar1 = _DAT_0055256c;
            if (*(short *)(*piVar2 + 0x1be) == 0) {
              fVar1 = (float)(piVar2[1] - local_8) / (float)piVar2[1];
            }
            // BUG-FIX 2026-07-15: IDA Trail_RenderAll lee el color como FLOAT
            // (`*((float*)v0+2)`). Leerlo como `(float)piVar2[N]` (cast int de
            // los bits float, ej. 0.1f=0x3DCCCCCD → 1.03e9) hacía glColor clampear
            // a 1.0 → los beams/crackles salían a brillo MÁXIMO dorado en vez del
            // color tenue → haz dorado brillante sobre la espada del +11 set.
            glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2[4]);
            iVar3 = piVar2[1];
            glTexCoord2f((float)local_8 / (float)iVar3,1.0f);   // BUG-FIX: era 0x3f800000 (int=1e9), IDA usa 1.0
            glVertex3fv((const GLfloat*)(piVar4 + -0x5a));
            glTexCoord2f((float)local_8 / (float)iVar3,0);
            glVertex3fv((const GLfloat*)piVar4);
            fVar1 = _DAT_0055256c;
            if (*(short *)(*piVar2 + 0x1be) == 0) {
              fVar1 = (float)((piVar2[1] - local_8) + -1) / (float)piVar2[1];
            }
            // BUG-FIX 2026-07-15: IDA Trail_RenderAll lee el color como FLOAT
            // (`*((float*)v0+2)`). Leerlo como `(float)piVar2[N]` (cast int de
            // los bits float, ej. 0.1f=0x3DCCCCCD → 1.03e9) hacía glColor clampear
            // a 1.0 → los beams/crackles salían a brillo MÁXIMO dorado en vez del
            // color tenue → haz dorado brillante sobre la espada del +11 set.
            glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2[4]);
            local_8 = local_8 + 1;
            iVar3 = piVar2[1];
            glTexCoord2f((float)local_8 / (float)iVar3,0);
            glVertex3fv((const GLfloat*)(piVar4 + 3));
            glTexCoord2f((float)local_8 / (float)iVar3,1.0f);   // BUG-FIX: era 0x3f800000 (int=1e9), IDA usa 1.0
            glVertex3fv((const GLfloat*)(piVar4 + -0x57));
            glEnd();
            piVar4 = piVar4 + 3;
          } while (local_8 < piVar2[1] + -1);
        }
      }
    }
  }  // end of explicit count loop (advance happens in for-statement)
  return;
}
