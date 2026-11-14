// Render_LegacyTrails.cpp

#include "stdafx.h"
#include "render/BufferedDraw.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    ClothNode_ScalarDtor(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones definidas en otros modulos -----------------
// Cloth_Integrate vive en Physics/Cloth_Simulation.cpp y Cloth_Solve en
// Net/Crypto.cpp.
void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "net/Net.h"

extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

// Particle_Update @ 0x0046C3E0 — Trail_RenderAll: render weapon/beam trails in pool.
// Pool: g_RenderPool_07c608a8 (= shared joint/trail pool, 100 slots × 0x2f0);
// iteration count is 100 (matching IDA bound `< 0x7c72e74` = base + 100*0x2f0).
void __cdecl Particle_Update(void)
{
    int *slot = (int *)&DAT_07c608b4;
    for (int slotIdx = 0; slotIdx < 100; ++slotIdx, slot = reinterpret_cast<int*>(reinterpret_cast<char*>(slot) + 0x2f0)) {
        if ((char)slot[-3] != '\0') {
            int mode = slot[-2];
            if (*(short *)(g_TrailOwner[slotIdx] + EntityOffset(0x1be)) == 0 && mode < 3)
                GL_SetBlendAdditive();
            else
                GL_SetBlendSrcAlpha();
            if (mode > 2) mode -= 3;
            int seg = slot[1];
            if (seg > 1) {
                GL_BindTextureSlot(mode + 0x48d);
                int *vp = slot + 0x5f;
                for (int i = 0; i < seg - 1; i++, vp += 3) {
                    BufferedDraw::Begin(BufferedDraw::Primitive::TriangleFan);
                    float alpha = _DAT_0055256c;
                    if (*(short *)(g_TrailOwner[slotIdx] + EntityOffset(0x1be)) == 0)
                        alpha = (float)(seg - i) / (float)seg;
                    BufferedDraw::Color3f(alpha*(float)slot[2], alpha*(float)slot[3], alpha*(float)slot[4]);
                    BufferedDraw::TexCoord2f((float)i / (float)seg, 1.0f);
                    BufferedDraw::Vertex3fv((float *)(vp - 0x5a));
                    BufferedDraw::TexCoord2f((float)i / (float)seg, 0.0f);
                    BufferedDraw::Vertex3fv((float *)vp);
                    float alpha2 = _DAT_0055256c;
                    if (*(short *)(g_TrailOwner[slotIdx] + EntityOffset(0x1be)) == 0)
                        alpha2 = (float)(seg - i - 1) / (float)seg;
                    BufferedDraw::Color3f(alpha2*(float)slot[2], alpha2*(float)slot[3], alpha2*(float)slot[4]);
                    BufferedDraw::TexCoord2f((float)(i+1) / (float)seg, 0.0f);
                    BufferedDraw::Vertex3fv((float *)(vp + 3));
                    BufferedDraw::TexCoord2f((float)(i+1) / (float)seg, 1.0f);
                    BufferedDraw::Vertex3fv((float *)(vp - 0x57));
                    BufferedDraw::End();
                }
            }
        }
    }  // end of explicit count loop (advance happens in for-statement)
}
