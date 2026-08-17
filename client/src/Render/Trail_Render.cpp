// Trail_Render.cpp
// FUN_0046c3e0 @ 0x0046C3E0
//
// Trail_RenderAll — renders all active beam/trail effects (weapon trails, magic beams, etc.)
//
// Trail table: starts at DAT_07c608b4, stride 0xbc (int-words) = 0x2f0 = 752 bytes per slot.
// Array ends at 0x7C72E74.
//
// Each slot layout (relative to piVar2, which points at field +0):
//   piVar2[-3]  (char)   — active flag (non-zero = active trail)
//   piVar2[-2]  (int)    — segment count (number of trail points)
//   piVar2[-1]  (int)    — entity ptr (*piVar2 = entity ptr at +0x1be → is_swimming short)
//   piVar2[0]   (int)    — entity ptr (*(short*)(entity+0x1be) checked for swim state)
//   piVar2[1]   (int)    — segment count (duplicate / display count)
//   piVar2[2..4] (float) — RGB color (r, g, b)
//   piVar2[0x5f] (int)   — ptr to head of trail positions (3 floats per point)
//   piVar2+0x5f-0x5a — trail point array: each element = 3 floats (x,y,z)
//
// Rendering:
//   - Sets GL state via FUN_00511480(type + 0x48d)
//   - For each segment pair: glBegin(GL_QUAD_STRIP=6)
//       glColor3f with fade: alpha = (count-i)/count (or 1.0 if swimming)
//       4 vertices per strip quad (two points × two sides of the trail)
//     glEnd()
//
// Type lookup: if entity.is_swimming == 0 → type = piVar2[-2], else FUN_00511790
//
// Sub-functions:
//   FUN_00511710 — GL state A (no swimming)
//   FUN_00511790 — GL state B (swimming)
//   FUN_00511480 — SetTrailTexture(type)

#include "stdafx.h"

// FUN_0046c3e0 definition MOVED to Joint_Render.cpp — this duplicate removed to avoid LNK2005
#if 0
void __cdecl FUN_0046c3e0_DISABLED(void)
{
    return;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    float fVar1;
    int  *piVar2;
    int   iVar3;
    int  *piVar4;
    int   local_8;

    piVar2 = (int*)&DAT_07c608b4;
    do {
        if ((char)piVar2[-3] != '\0') {
            iVar3 = piVar2[-2];  // segment count

            // Select GL state based on entity swim status
            if ((*(short *)(*piVar2 + 0x1be) == 0) && (iVar3 < 3)) {
                FUN_00511710();   // normal GL state
            } else {
                FUN_00511790();   // swimming GL state
            }

            if (iVar3 > 2)
                iVar3 -= 3;

            // Render trail segments as quad strip
            if (piVar2[1] > 1) {
                FUN_00511480(iVar3 + 0x48d);  // bind trail texture type

                local_8 = 0;
                if (piVar2[1] != 1 && piVar2[1] - 1 >= 0) {
                    piVar4 = piVar2 + 0x5f;   // head of trail point array
                    do {
                        glBegin(6);  // GL_QUAD_STRIP

                        // Compute per-segment fade alpha
                        fVar1 = _DAT_0055256c;  // default: 1.0
                        if (*(short *)(*piVar2 + 0x1be) == 0) {
                            // Fade from full to zero along trail length
                            fVar1 = (float)(piVar2[1] - local_8) / (float)piVar2[1];
                        }
                        glColor3f(fVar1 * (float)piVar2[2],
                                  fVar1 * (float)piVar2[3],
                                  fVar1 * (float)piVar2[4]);

                        iVar3 = piVar2[1];
                        // Vertex pair A (tail side)
                        glTexCoord2f((float)local_8 / (float)iVar3, (float)0x3f800000);  // v=1
                        glVertex3fv((float *)(piVar4 - 0x5a));
                        glTexCoord2f((float)local_8 / (float)iVar3, 0.0f);
                        glVertex3fv((float *)piVar4);

                        // Fade for next vertex pair
                        fVar1 = _DAT_0055256c;
                        if (*(short *)(*piVar2 + 0x1be) == 0) {
                            fVar1 = (float)((piVar2[1] - local_8) - 1) / (float)piVar2[1];
                        }
                        glColor3f(fVar1 * (float)piVar2[2],
                                  fVar1 * (float)piVar2[3],
                                  fVar1 * (float)piVar2[4]);

                        local_8++;
                        iVar3 = piVar2[1];
                        // Vertex pair B (head side)
                        glTexCoord2f((float)local_8 / (float)iVar3, 0.0f);
                        glVertex3fv((float *)(piVar4 + 3));
                        glTexCoord2f((float)local_8 / (float)iVar3, (float)0x3f800000);
                        glVertex3fv((float *)(piVar4 - 0x57));

                        glEnd();
                        piVar4 += 3;   // advance to next trail point (3 floats)
                    } while (local_8 < piVar2[1] - 1);
                }
            }
        }
        piVar2 = piVar2 + 0xbc;  // stride: 0x2f0 = 752 bytes
    } while ((int)piVar2 < 0x7c72e74);

    return;
}
#endif  // disabled duplicate — canonical definition in Joint_Render.cpp
