// BMD_LegacyEffects.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_00440d30 @ 0x00440D30 — BMD_PushMatrix: calls glPushMatrix().
void __cdecl FUN_00440d30(void) { glPushMatrix(); }
// FUN_00440d50 @ 0x00440D50 — BMD_DrawMesh: implemented in Render/BMD_DrawMesh.cpp
// FUN_00441be0 @ 0x00441BE0 — Model_KillAnim
// Iterates bone meshes; for each vertex in bone 'b', spawns kill/death particle effects.
void __cdecl FUN_00441be0(void *model, int param_1, int param_2)
{
    char *this_ = (char*)model;
    if (*(short*)(this_ + 0x24) == 0) return;
    float scale[6] = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    int meshCount = *(short*)(this_ + 0x24);
    int iVar3 = param_1;
    for (int local_14 = 0; local_14 < meshCount; local_14++) {
        int iVar1 = *(int*)(this_ + 0x28) + local_14 * 0x28;
        int meshVerts = *(short*)(iVar1 + 10);
        if (meshVerts <= 0) continue;
        for (int local_20 = 0; local_20 < meshVerts; local_20++) {
            short *psVar6 = (short*)(local_20 * 0x24 + *(int*)(iVar1 + 0x1c));
            if ((char)*psVar6 <= 0) continue;
            for (int iVar7 = 0; iVar7 < (char)*psVar6; iVar7++) {
                int vertIdx = psVar6[iVar7 + 1];
                float *vpos = (float*)((char*)(char*)&DAT_0584621c + (vertIdx + iVar3 * 15000) * 3 * 4);
                if (param_2 == 0x104) {
                    if (_rand() % 2 == 0)
                        FUN_00460dc0(0x105, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 10 == 0)
                        FUN_00460dc0(0x104, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                } else if (param_2 == 0x106) {
                    scale[0] = scale[1] = scale[2] = 0.2f;
                    if (_rand() % 12 == 0)
                        FUN_00460dc0(0x107, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 50 == 0)
                        FUN_00460dc0(0x106, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                }
            }
        }
    }
}

// FUN_00441f00 @ 0x00441F00 — Model_RenderShadow
// Renders shadow triangles for all mesh bones except bone param_2 (using anim param_1).
void __cdecl FUN_00441f00(void *model, int param_1, int param_2)
{
    char *this_ = (char*)model;
    if (*(short*)(this_ + 0x24) == 0) return;
    FUN_00511590('\0');
    FUN_00511530();
    FUN_00440d30();
    int meshCount = *(short*)(this_ + 0x24);
    int local_20 = 0;
    for (int local_14 = 0; local_14 < meshCount; local_14++, local_20 += 15000) {
        int iVar5 = *(int*)(this_ + 0x28) + local_14 * 0x28;
        if (*(short*)(iVar5 + 10) <= 0) { continue; }
        if (*(short*)(iVar5 + 2) == (short)param_1) { continue; }
        if (local_14 == param_2) { continue; }
        glBegin(GL_TRIANGLES);
        int polyCount = *(short*)(iVar5 + 10);
        for (int local_1c = 0; local_1c < polyCount; local_1c++) {
            short *psVar1 = (short*)(local_1c * 0x24 + *(int*)(iVar5 + 0x1c));
            for (int iVar7 = 0; iVar7 < (char)*psVar1; iVar7++) {
                int vertIdx = (int)psVar1[iVar7 + 1] + local_20;
                float *vp = (float*)((char*)(char*)&DAT_0584621c + vertIdx * 3 * 4);
                float lc = vp[0] - *(float*)(this_ + 0x6c);
                float lz = vp[2] - *(float*)(this_ + 0x74);
                float shadow_x = lc + *(float*)(this_ + 0x6c);
                float shadow_y = *(float*)(this_ + 0x70);
                float shadow_z = *(float*)(this_ + 0x74);
                glVertex3f(shadow_x, shadow_y, shadow_z);
            }
        }
        glEnd();
    }
    glPopMatrix();
    FUN_00511510();
}
