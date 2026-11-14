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
#include "render/InventoryPoseTable.h"
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

// IDA: RenderObjectScreen (0x004E13A0).
// Dibuja un item 3D en las grillas: inventario, equipo, tienda, baúl, trade y
// Chaos. Fija los ángulos compartidos, arma un OBJECT en la pila y llama a
// BMD_Animation + RenderPartObject.
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
    // Ángulo, altura y escala por modelo: Content/item/inventory_pose.toml.
    const InventoryPose& inventoryPose = InventoryPose_Get(Type);
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
    AsCharacter(object)->Class = 2;

    if (drawType==790) LooseWingVisuals_SubmitPreview(*(OBJECT *)object,renderAngle,headAngle,
        inventoryPose.BodyHeight,ItemLevel,Select==1);

    float light[3] = { 1.0f, 1.0f, 1.0f };
    // Entity_DrawAt's visibility argument is 1.0 in the native UI path used
    // by this client; all OpenGL state setup/teardown stays inside that renderer.
    RenderPartObject((intptr_t)object, drawType, 0, light, 1.0f, ItemLevel, Option1,
                 '\x01', 1, '\x01', 0, 2);
}
