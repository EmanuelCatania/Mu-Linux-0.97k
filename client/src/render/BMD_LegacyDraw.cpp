#include <cstdint>
// BMD_LegacyDraw.cpp
// BMD__RenderMeshTranslate y RenderObjectScreen (ítems 3D de las grillas).

#include "stdafx.h"
#include "assets/model/ModelTypes.h"
#include "render/BufferedDraw.h"
#include "render/MeshDraw.h"
#include "render/BMDDevice.h"
#include "render/LooseWingVisuals.h"
#include "render/MaterialVisuals.h"
#include "platform/ITimer.h"
#include <array>
#include "assets/data/ContentCatalog.h"
#include "globals.h"
#include "functions.h"

extern void __cdecl operator_delete(void* ptr);

// IDA Hex-Rays intrinsic shims.

// IDA: sub_4414D0 (0x004414D0) -- dibujado de una malla con deformacion
// senoidal por vertice.  sub_440D50 delega aca cuando (flags & 0x400).
//
// Orden de parametros: IDA es (this, a2, a3, a4 = malla, a5 = flags, alpha,
// a7 = BlendMesh, a8 = BlendMeshLight, a9 = U, a10 = V, a11 = textura).  El
// port usa (model, a, b, frame = malla, flags, f3 = alpha, f4 = BlendMesh,
// f5 = U, f6 = V, f7 = BlendMeshLight, rgba = textura), o sea U/V van antes
// que la luz.  El unico caller (BMD__RenderMesh) ya pasa en este orden.
void __cdecl BMD__RenderMeshTranslate(void *model, char a, int b, float frame, int flags,
                           float f3, int f4, float f5, float f6, float f7, unsigned int rgba)
{
    (void)b;
    if (!model) return;

    auto* bmd = AsModel(model);
    const int meshIdx  = (int)frame;
    const auto* mesh = reinterpret_cast<const ClothModelMesh*>(bmd->Meshs) + meshIdx;
    if (mesh->TriangleCount == 0) return;             // sin triangulos

    int tex = bmd->IndexTexture[mesh->TextureIndex];
    if (tex == 300) return;                                   // BITMAP_HIDE
    if (rgba != 0xffffffff) tex = (int)rgba;

    const int blendMesh = f4;
    const float *bodyLight = bmd->BodyLight;

    // UV animadas (LOBYTE(a11) de IDA): la malla es la de BlendMesh o la de
    // StreamMesh (model+0x88, o la propia si el script de la malla lo pide),
    // y hay algun desplazamiento de UV.
    const int streamMesh = static_cast<signed char>(bmd->StreamMesh);
    int waveMesh = streamMesh;
    {
        const auto* script = static_cast<const char*>(mesh->TextureFlags);
        if (script && script[2]) waveMesh = meshIdx;
    }
    const bool uvAnim = (meshIdx == blendMesh || meshIdx == waveMesh) &&
                        (f5 != 0.0f || f6 != 0.0f);

    // v58: LightEnable del modelo.  Con StreamMesh el color es plano; si no,
    // se arma la luz por vertice = IntensityTransform * BodyLight (IDA
    // escribe el mismo buffer que sub_440D50 antes de delegar).
    int lightEnable = bmd->LightEnable;
    if (meshIdx == streamMesh) {
        BufferedDraw::Color3fv(bodyLight);
        lightEnable = 0;
    } else if (lightEnable) {
        const int nNormals = mesh->NormalCount;
        const float *src = (const float *)(&DAT_077e298c + meshIdx * 15000);
        float *dst = (float *)(&DAT_060db65c + meshIdx * 180000);
        for (int i = 0; i < nNormals; ++i) {
            dst[i * 3 + 0] = src[i] * bodyLight[0];
            dst[i * 3 + 1] = src[i] * bodyLight[1];
            dst[i * 3 + 2] = src[i] * bodyLight[2];
        }
    }

    // Estado GL; `mode` es el a7 de IDA reasignado, que despues decide si se
    // emiten texcoords (2) o no (1, 64).
    const int fl = flags & 0xFF;
    int mode;
    if ((fl & 1) == 1) {
        mode = 1;
        BMDDevice::Select({BMDDevice::Pass::ColorAnimated,static_cast<unsigned>(fl)});
        BufferedDraw::Color3fv(bodyLight);
    } else if (blendMesh <= -2 || mesh->TextureIndex == blendMesh) {
        mode = 2;
        BMDDevice::Select({BMDDevice::Pass::Blend,static_cast<unsigned>(fl),tex});
        BufferedDraw::Color3f(f7 * bodyLight[0], f7 * bodyLight[1], f7 * bodyLight[2]);
        lightEnable = 0;
    } else if ((fl & 2) == 2) {
        mode = 2;
        BMDDevice::Select({BMDDevice::Pass::Surface,static_cast<unsigned>(fl),tex,f3 < _DAT_00552544});
    } else if ((fl & 0x40) == 0x40) {
        if ((&DAT_083a7cc8)[tex * TEXTURE_STRIDE] == '\x04') return;    // Bitmaps[tex].Components == 4
        mode = 64;
        BMDDevice::Select({BMDDevice::Pass::Metal});
    } else {
        mode = 2;
    }

    // (bloque de hash-table anti-tamper omitido)

    // v55 = a2 & 1: la deformacion depende del PRIMER argumento, que
    // sub_440D50 pasa siempre en 1.  El port miraba el segundo (siempre 0),
    // asi que la onda nunca se aplicaba.
    const bool deform = (a & 1) != 0;

    BufferedDraw::Begin(BufferedDraw::Primitive::Triangles);
    const int polyCount = mesh->TriangleCount;
    for (int poly = 0; poly < polyCount; ++poly) {
        const char *tri = reinterpret_cast<const char*>(mesh->Triangles + poly * sizeof(ModelRuntimeTriangle));
        const int nv = *tri;
        const short *idx = (const short *)(tri + 10);
        for (int vi = 0; vi < nv; ++vi, ++idx) {
            const int vtx = idx[-4];                          // indice de vertice
            if (mode == 2) {
                const float *uv = static_cast<const float*>(mesh->TexCoords) + idx[4] * 2;
                if (uvAnim) BufferedDraw::TexCoord2f(f5 + uv[0], f6 + uv[1]);
                else        BufferedDraw::TexCoord2f(uv[0], uv[1]);
                if (lightEnable) {
                    const float *c = (const float *)(&DAT_060db65c + ((int)idx[0] + meshIdx * 15000) * 12);
                    if (f3 < _DAT_00552544) BufferedDraw::Color4f(c[0], c[1], c[2], f3);
                    else                    BufferedDraw::Color3fv(c);
                }
            }

            const float *pos = (const float *)((char *)&DAT_0584621c + (vtx + meshIdx * 15000) * 12);
            float moved[3];
            if (deform) {
                // IDA: v56 = (std::int64_t)WorldTime + 931 * vtx;
                //      pos + sin(v56 * 0.007) * normal * 28
                const int phase = (int)(long long)WorldTime + 0x3a3 * vtx;
                const float sn = (float)sin((double)phase * (double)_DAT_005528c4);
                const float *nrm = (const float *)(&DAT_06f433bc + ((int)idx[0] + meshIdx * 15000) * 12);
                for (int k = 0; k < 3; ++k)
                    moved[k] = sn * nrm[k] * _DAT_00552644 + pos[k];
                pos = moved;
            }
            BufferedDraw::Vertex3fv(pos);
        }
    }
    BufferedDraw::End();
}

// RenderObjectScreen @ 0x004E13A0 — RenderObjectScreen
// Renders a 3D item/object at world position param_4[0..2].
// Sets rotation globals per type, builds a stack entity, calls BMD_Animation + Entity_DrawAt.
static bool ApplyInventoryExactPoseLate(int param_1, int level, float* outPos)
{
    switch (param_1) {
    case MODEL_SWORD + 0:
        outPos[0] -= 0.02f; outPos[1] += 0.03f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 0:
        outPos[1] += 0.05f;
        ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_BOW + 7:
    case MODEL_BOW + 15:
        ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 17:
        ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 20:
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 21:
        outPos[1] += 0.12f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 22:
    case MODEL_BOW + 23:
        outPos[0] -= 0.10f; outPos[1] += 0.08f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 10:
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELM + 30:
        outPos[0] -= 0.03f; outPos[1] += 0.07f;
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 31:
        outPos[0] += 0.03f; outPos[1] -= 0.06f;
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 35:
        outPos[0] -= 0.02f; outPos[1] += 0.05f;
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 5:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 6:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 7:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 10:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 11:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = -20.0f; return true;
    case MODEL_EVENT + 12:
        ObjectSelect_Angle = 250.0f; _DAT_07ea9530 = 140.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 14:
        ObjectSelect_Angle = 255.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 15:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 16:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 18:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 3:
    case MODEL_HELPER + 4:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 5:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = -35.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 16:
    case MODEL_HELPER + 17:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 18:
        ObjectSelect_Angle = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 21:
    case MODEL_HELPER + 22:
    case MODEL_HELPER + 23:
    case MODEL_HELPER + 24:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELPER + 29:
        ObjectSelect_Angle = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 30:
        ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 12:
        if (level == 0) { ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 1) { ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 2) { ObjectSelect_Angle = 90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        return true;
    case MODEL_STAFF + 7:
        ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 205.0f; return true;
    case MODEL_STAFF + 12:
        outPos[0] += 0.025f; outPos[1] -= 0.10f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_STAFF + 13:
        outPos[0] += 0.02f; outPos[1] += 0.02f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_POTION + 20:
    case MODEL_POTION + 27:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 13:
    case MODEL_POTION + 14:
    case MODEL_POTION + 22:
        outPos[0] += 0.005f; outPos[1] += 0.015f;
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 21:
        outPos[0] += 0.005f; outPos[1] -= 0.005f;
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 41:
        outPos[1] += 0.02f;
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 42:
        outPos[1] += 0.02f;
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 43:
    case MODEL_POTION + 44:
        outPos[0] -= 0.04f; outPos[1] += 0.02f; outPos[2] += 0.02f;
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = -45.0f; return true;
    case MODEL_POTION + 63:
        outPos[1] += 0.08f;
        ObjectSelect_Angle = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_SWORD + 26:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_SWORD + 27:
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SWORD + 28:
        outPos[1] += 0.02f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_MACE + 16:
        outPos[0] -= 0.02f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 17:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 18:
        outPos[0] -= 0.03f; outPos[1] += 0.06f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 2.0f; return true;
    case MODEL_MACE + 14:
        outPos[0] -= 0.01f; outPos[1] += 0.10f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_MACE + 15:
        outPos[1] += 0.05f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_SPEAR + 11:
        outPos[1] += 0.02f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_STAFF + 30:
    case MODEL_STAFF + 31:
    case MODEL_STAFF + 32:
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 33:
        outPos[0] += 0.02f; outPos[1] -= 0.06f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 34:
        outPos[1] -= 0.05f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_BOW + 24:
        outPos[0] -= 0.07f; outPos[1] += 0.07f;
        ObjectSelect_Angle = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_HELPER + 39:
    case MODEL_HELPER + 40:
    case MODEL_HELPER + 41:
    case MODEL_HELPER + 42:
        ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    default:
        if (param_1 >= MODEL_HELPER + 12 && param_1 < MODEL_HELPER + 512 &&
            param_1 != MODEL_HELPER + 12 && param_1 != MODEL_HELPER + 13 &&
            param_1 != MODEL_HELPER + 14 && param_1 != MODEL_HELPER + 15) {
            ObjectSelect_Angle = 360.0f;
            _DAT_07ea9530 = 0.0f;
            _DAT_07ea9534 = 0.0f;
            return true;
        }
        if (param_1 == MODEL_ARMOR + 29) {
            outPos[1] += 0.07f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 30) {
            outPos[1] += 0.10f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 34) {
            outPos[1] += 0.03f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 35) {
            outPos[1] += 0.05f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 36 || param_1 == MODEL_ARMOR + 37) {
            outPos[1] -= 0.05f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_HELM + 39 && param_1 <= MODEL_HELM + 44) {
            outPos[1] -= 0.05f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_ARMOR + 38 && param_1 <= MODEL_ARMOR + 44) {
            outPos[1] -= 0.08f;
            ObjectSelect_Angle = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_STAFF + 21 && param_1 <= MODEL_STAFF + 29) {
            ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 130 && param_1 <= MODEL_POTION + 132) {
            outPos[1] += 0.06f;
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 133) {
            outPos[0] += 0.01f;
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 134 && param_1 <= MODEL_POTION + 139) {
            outPos[1] += 0.05f;
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 140) {
            outPos[1] += 0.09f;
            ObjectSelect_Angle = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 52) {
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = -25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 63) {
            outPos[1] += 0.08f;
            ObjectSelect_Angle = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 160 || param_1 == MODEL_POTION + 161) {
            outPos[1] += 0.05f;
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 145 && param_1 <= MODEL_POTION + 150) {
            outPos[0] += 0.01f; outPos[1] += 0.04f;
            ObjectSelect_Angle = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
    return false;
}

}

namespace {
// IDA: RenderObjectScreen (0x004E13A0). Rangos inclusivos; las excepciones
// posteriores reemplazan al grupo. La tabla se resuelve al compilar.
enum class InventoryAngleMode : unsigned char { Fixed, ItemAttribute, Level };

struct InventoryPoseDefinition {
    float Angle[3] = { 270.0f, -10.0f, 0.0f };
    float BodyHeight = 0.0f;
    float Scale = 0.0025f;
    float NegativeLevelScale = 0.0025f;
    InventoryAngleMode AngleMode = InventoryAngleMode::Fixed;
};

constexpr std::array<InventoryPoseDefinition, MODEL_MAX_VANILLA> MakeInventoryPoses()
{
    std::array<InventoryPoseDefinition, MODEL_MAX_VANILLA> poses{};
    struct AngleRule {
        int First, Last;
        float X, Y, Z;
        InventoryAngleMode Mode = InventoryAngleMode::Fixed;
    };
    constexpr AngleRule angles[] = {
        { MODEL_SWORD, MODEL_SHIELD - 1, 180.0f, 270.0f, 15.0f, InventoryAngleMode::ItemAttribute },
        { MODEL_SHIELD, MODEL_HELM - 1, 270.0f, 270.0f, 0.0f },
        { 536, MODEL_STAFF - 1, 90.0f, 180.0f, 20.0f },
        { 506, 506, 180.0f, 270.0f, 20.0f },
        { 535, 535, 0.0f, 270.0f, 15.0f },
        { 543, 543, 0.0f, 270.0f, 15.0f },
        { 545, 545, 0.0f, 90.0f, 15.0f },
        { MODEL_HELPER_DINORANT, MODEL_HELPER_DINORANT, -90.0f, -20.0f, 0.0f },
        { 828, MODEL_POTION - 1, 360.0f, 0.0f, 0.0f },
        { 830, 833, 270.0f, -10.0f, 0.0f }, // Anillos, Blood Bone y Scroll of Archangel.
        { 834, 834, 290.0f, 0.0f, 0.0f },   // Cloak of Invisibility: no es un ala.
        { 860, 860, 90.0f, 0.0f, 0.0f, InventoryAngleMode::Level },
        { 868, 868, 270.0f, 0.0f, 0.0f },
        { 952, 954, 270.0f, 0.0f, 0.0f },
        { 953, 953, 270.0f, 90.0f, 0.0f },
        { 958, 958, -90.0f, -20.0f, -20.0f },
    };
    for (const auto& rule : angles) {
        for (int type = rule.First; type <= rule.Last; ++type) {
            poses[type].Angle[0] = rule.X;
            poses[type].Angle[1] = rule.Y;
            poses[type].Angle[2] = rule.Z;
            poses[type].AngleMode = rule.Mode;
        }
    }

    struct SizeRule { int First, Last; float Height, Scale; };
    constexpr SizeRule sizes[] = {
        { MODEL_HELM, MODEL_ARMOR - 1, -156.0f, 0.0039f },
        { MODEL_ARMOR, MODEL_PANTS - 1, -96.0f, 0.0039f },
        { MODEL_PANTS, MODEL_GLOVES - 1, -48.0f, 0.0038f },
        { MODEL_GLOVES, MODEL_BOOTS - 1, -72.0f, 0.0032f },
        { MODEL_BOOTS, MODEL_WING_ELF - 1, 0.0f, 0.0033f },
        { MODEL_WING_ELF, MODEL_HELPER_ANGEL - 1, 0.0f, 0.0020f },
        { MODEL_WING_DARKNESS, MODEL_WING_DARKNESS, 0.0f, 0.0015f },
        { MODEL_SPEAR, MODEL_BOW - 1, 0.0f, 0.0018f },
        { MODEL_STAFF, MODEL_SHIELD - 1, 0.0f, 0.0022f },
        { MODEL_POTION, MODEL_ETC - 1, 0.0f, 0.0035f },
        { 535, 535, 0.0f, 0.0012f },
        { 543, 543, 0.0f, 0.0011f },
        { 570, 570, 0.0f, 0.0019f },
        { 830, 831, 0.0f, 0.0030f },
        { 832, 832, 0.0f, 0.0020f },
        { 833, 834, 0.0f, 0.0018f },
        { 869, 869, 0.0f, 0.0020f },
        { 870, 872, 0.0f, 0.0025f },
        { 873, 874, 0.0f, 0.0028f },
        { 953, 953, 0.0f, 0.0039f },
        { 955, 955, 0.0f, 0.0015f },
        { 956, 956, 0.0f, 0.0019f },
        { 957, 957, 0.0f, 0.0010f },
        { 958, 958, 0.0f, 0.0015f },
    };
    for (const auto& rule : sizes) {
        for (int type = rule.First; type <= rule.Last; ++type) {
            poses[type].BodyHeight = rule.Height;
            poses[type].Scale = rule.Scale;
            poses[type].NegativeLevelScale = rule.Scale;
        }
    }
    poses[419].NegativeLevelScale = 0.0010f;
    poses[570].NegativeLevelScale = 0.0010f;
    poses[546].NegativeLevelScale = 0.0015f;
    return poses;
}

constexpr auto InventoryPoses = MakeInventoryPoses();
constexpr InventoryPoseDefinition DefaultInventoryPose{};
} // namespace

// IDA: RenderObjectScreen (0x004E13A0).
// Pose común a inventario, equipo, tienda, baúl, trade y Chaos.
void __cdecl RenderObjectScreen(int Type, unsigned int ItemLevel, unsigned char Option1,
                          unsigned char ExtOption, float* Target, int Select, char PickUp)
{
    // 0.97.20: un modelo dinámico del catálogo (agregado >= 512) toma la pose,
    // escala y altura del vanilla que imita; se dibuja con su propio modelo.
    MaterialVisualScope material(MaterialVisuals_UiSample(Type));
    const int drawType = Type;
    if (Type >= MODEL_MAX_VANILLA) {
        const int itemType = gContentCatalog.GetModelItemType(Type);
        if (itemType >= 0) Type = ItemEntityModel(itemType);
    }

    float camera[3] = { _CameraRayOriginX, _CameraRayOriginY, _CameraRayOriginZ };
    float direction[3] = {
        Target[0] - camera[0], Target[1] - camera[1], Target[2] - camera[2]
    };
    float position[3];
    VectorMA(camera, PickUp ? 0.07f : 0.1f, direction, position);

    const int level = ((int)ItemLevel >> 3) & 0x0F;
    const auto& inventoryPose = (Type >= 0 && Type < MODEL_MAX_VANILLA)
        ? InventoryPoses[Type] : DefaultInventoryPose;
    float angle[3] = { inventoryPose.Angle[0], inventoryPose.Angle[1], inventoryPose.Angle[2] };
    if (inventoryPose.AngleMode == InventoryAngleMode::ItemAttribute) {
        angle[2] = *(BYTE*)((BYTE*)(uintptr_t)DAT_07d78068 + (Type - 399) * 0x40 - 34) ? 25.0f : 15.0f;
    } else if (inventoryPose.AngleMode == InventoryAngleMode::Level) {
        if (level == 0) angle[0] = 180.0f;
        else if (level == 1) { angle[0] = 270.0f; angle[1] = 90.0f; }
    }

    // DESVIACION (DLL ItemPosition, ahora catálogo 0.97.20): pose propia del
    // item en el inventario.  Desplaza la posición, reemplaza los ángulos y la
    // escala; el giro del item seleccionado se mantiene.
    const CatalogItemPose* pose = gContentCatalog.GetItemPose(gContentCatalog.GetModelItemType(drawType));
    if (pose) {
        position[0] += pose->PositionX;
        position[1] += pose->PositionY;
        angle[0] = pose->RotationX;
        angle[1] = pose->RotationY;
        angle[2] = pose->RotationZ;
    }

    if (Select == 1)
        angle[1] = (float)WorldTime * 0.45f;

    // IDA stores the pose in these shared angle slots before BMD_Animation;
    // later item passes also observe that state.
    ObjectSelect_Angle = angle[0];
    _DAT_07ea9530 = angle[1];
    _DAT_07ea9534 = angle[2];

    short modelType = (short)drawType;
    if (drawType == Type && modelType >= MODEL_HELM && modelType < MODEL_WING_ELF) {
        modelType = 390;
    } else if (modelType == 860) {
        if (level == 0) modelType = 947;
        else if (level == 2) modelType = 948;
    }
    ObjectSelect_Type = modelType;

    void* model = (void*)(DAT_05828d58 + (int)modelType * BMD_STRIDE);
    *(BYTE*)&((BMD*)(uintptr_t)(model))->CurrentAction = 0;
    *(float*)&((BMD*)(uintptr_t)(model))->BodyHeight = inventoryPose.BodyHeight;
    float scale = (int)ItemLevel < 0 ? inventoryPose.NegativeLevelScale : inventoryPose.Scale;
    if (pose && pose->Scale > 0.0f) scale = pose->Scale;

    ObjectSelect_AnimationFrame = 0;
    ObjectSelect_PriorAnimationFrame = 0;
    ObjectSelect_PriorAction = 0;
    float renderAngle[3] = { ObjectSelect_Angle, _DAT_07ea9530, _DAT_07ea9534 };
    float headAngle[3] = { ObjectSelect_HeadAngle, 0.0f, 0.0f };
    BMD_Animation(model, reinterpret_cast<float*>(&DAT_06970a9c), 0.0f, 0, 0,
                 (unsigned int*)renderAngle, headAngle, '\0', '\0');

    char object[0x200] = {};
    *(short*)(object + 2) = (short)drawType;
    ItemObjectAttributeForRender(AsObject(object));
    *(float*)(object + 0x0C) = scale;
    *(float*)(object + 0x10) = position[0];
    *(float*)(object + 0x14) = position[1];
    *(float*)(object + 0x18) = position[2];
    *(BYTE*)(object + 0x3D) = ExtOption;
    *(int*)(object + 0xDC) = 0;
    *(BYTE*)(object + EntityOffset(0x1BC)) = 2;

    if (drawType==790) LooseWingVisuals_SubmitPreview(*(OBJECT *)object,renderAngle,headAngle,
        inventoryPose.BodyHeight,ItemLevel,Select==1);

    float light[3] = { 1.0f, 1.0f, 1.0f };
    // Entity_DrawAt's visibility argument is 1.0 in the native UI path used
    // by this client; all OpenGL state setup/teardown stays inside that renderer.
    RenderPartObject((intptr_t)object, drawType, 0, light, 1.0f, ItemLevel, Option1,
                 '\x01', 1, '\x01', 0, 2);
}
