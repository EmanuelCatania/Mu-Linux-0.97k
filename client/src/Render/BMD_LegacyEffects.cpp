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
    // 2026-09-04 -- BUG-FIX: el port recorria TODAS las mallas del modelo.
    // IDA (sub_441BE0) trabaja sobre UNA sola, la de indice `a2`:
    //     result = this[10] + 40 * a2;        // this + 0x28 = array de mallas
    //     if ( *(__int16 *)(result + 10) > 0 ) ...
    // y el mismo `a2` es el que elige el bloque de 15000 vertices del pool
    // BoneVertex.  Los dos call sites pasan a2 = 0.  Con el bucle sobre todas
    // las mallas se spawneaban varias veces mas efectos de los que corresponde.
    char *this_ = (char*)model;
    const int meshIdx = param_1;
    if (meshIdx < 0 || meshIdx >= *(short*)(this_ + 0x24)) return;

    char *mesh = (char*)(*(int*)(this_ + 0x28) + meshIdx * 0x28);
    const int triCount = *(short*)(mesh + 10);
    if (triCount <= 0) return;

    float Angle[3] = { 0.0f, 0.0f, 0.0f };
    float Light[3] = { 1.0f, 1.0f, 1.0f };

    for (int tri = 0; tri < triCount; tri++) {
        short *psVar6 = (short*)(*(int*)(mesh + 0x1c) + tri * 36);
        const int nIdx = (int)(char)*psVar6;
        for (int k = 0; k < nIdx; k++) {
            const int vertIdx = psVar6[k + 1];
            float *vpos = (float*)(g_BoneVertexBuf + 12 * (vertIdx + 15000 * meshIdx));
            if (param_2 == 260) {
                if (_rand() % 2 == 0)
                    Effect_Create(261, vpos, Angle, Light, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                if (_rand() % 10 == 0)
                    Effect_Create(260, vpos, Angle, Light, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
            } else if (param_2 == 262) {
                Light[0] = Light[1] = Light[2] = 0.2f;
                if (_rand() % 12 == 0)
                    Effect_Create(263, vpos, Angle, Light, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                if (_rand() % 50 == 0)
                    Effect_Create(262, vpos, Angle, Light, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
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
