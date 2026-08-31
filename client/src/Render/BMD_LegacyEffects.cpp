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
                        Effect_Create(0x105, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 10 == 0)
                        Effect_Create(0x104, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                } else if (param_2 == 0x106) {
                    scale[0] = scale[1] = scale[2] = 0.2f;
                    if (_rand() % 12 == 0)
                        Effect_Create(0x107, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 50 == 0)
                        Effect_Create(0x106, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                }
            }
        }
    }
}

// FUN_00441f00 @ 0x00441F00 — Model_RenderShadow
// Proyecta la silueta del modelo sobre el terreno como sombra.
//
// Port fiel de sub_441F00 (398 B). Por cada mesh (salvo dos que se saltean) emite
// sus triangulos con los vertices ya transformados del pool BoneVertex, pero
// aplastados contra el suelo:
//
//   lx = X - ox;  ly = Y - oy;  lz = Z - oz;          (ox/oy/oz = origen, this+0x6c/70/74)
//   vx = lx + (lx + 2000.0) * lz / (lz - 4000.0) + ox;   <- proyeccion: sesga X con la altura
//   vy = ly + oy;                                        <- identidad: queda la Y del vertice
//   vz = oz + 5.0;                                       <- plano, 5 unidades sobre el origen
//
// En IDA los tres resultados son locales CONTIGUOS ([ebp-0Ch/-8/-4]) que se emiten
// con un unico glVertex3fv(&v); aca se arma un float[3] explicito para no depender
// del layout que elija MSVC (mismo patron que ya mordio en otros ports).
//
// `vy` es algebraicamente solo Y: se conserva la resta y la suma para que la
// correspondencia con el decompile quede a la vista.
//
// El signo del divisor no se acota: si lz == 4000 el original tambien divide por
// cero. Se deja fiel.
//
// param_1 (a2): se saltea el mesh cuyo campo +2 coincide  -> los callers pasan BlendMesh.
// param_2 (a3): se saltea el mesh cuyo INDICE coincide     -> los callers pasan HiddenMesh.
// Ojo que no son el mismo criterio: uno compara un campo y el otro el indice.
void __cdecl FUN_00441f00(void *model, int param_1, int param_2)
{
    char *this_ = (char*)model;
    if (*(short*)(this_ + 0x24) == 0) return;

    // IDA: DisableTexture(0). Ese es 0x511590, que en el arbol se llama
    // GL_SetAlphaTest (nombre enganoso: termina en glDisable(GL_TEXTURE_2D)).
    // NO confundir con 0x511680 EnableAlphaTest.
    GL_SetAlphaTest('\0');
    GL_DisableDepthWrites();       // 0x511530
    FUN_00440d30();                // thunk -> glPushMatrix()

    const float ox = *(float*)(this_ + 0x6c);
    const float oy = *(float*)(this_ + 0x70);
    const float oz = *(float*)(this_ + 0x74);

    const int meshCount = *(short*)(this_ + 0x24);
    int vertBase = 0;                                  // v15: 15000 por mesh
    for (int meshIdx = 0; meshIdx < meshCount; meshIdx++, vertBase += 15000) {
        if (meshIdx == param_2) continue;              // v5 != a3

        char *mesh = (char*)(*(int*)(this_ + 0x28) + meshIdx * 0x28);
        const int polyCount = *(short*)(mesh + 10);
        if (polyCount <= 0) continue;
        if ((int)*(short*)(mesh + 2) == param_1) continue;   // *(WORD*)(v6+2) != a2

        glBegin(GL_TRIANGLES);
        for (int polyIdx = 0; polyIdx < polyCount; polyIdx++) {
            char *poly = (char*)(*(int*)(mesh + 0x1c) + polyIdx * 0x24);
            const int numVerts = (int)*(signed char*)poly;   // char en +0
            const short *idx = (const short*)(poly + 2);     // indices en +2
            for (int k = 0; k < numVerts; k++) {
                const float *vp =
                    (const float*)((char*)&DAT_0584621c + (vertBase + idx[k]) * 3 * 4);

                const float lx = vp[0] - ox;
                const float ly = vp[1] - oy;
                const float lz = vp[2] - oz;

                float v[3];
                v[0] = lx + (lx + 2000.0f) * lz / (lz - 4000.0f) + ox;
                v[1] = ly + oy;
                v[2] = oz + 5.0f;
                glVertex3fv(v);
            }
        }
        glEnd();
    }

    glPopMatrix();
    GL_EnableDepthWrites();        // 0x511510
}
