#include <cstdint>
// Animación de modelos (Calc_RenderObject, BMD_Animation, Skeleton_Transform)
// y algunos helpers sueltos (CIsin, Screen_ToGLx/y).

#include "stdafx.h"
#include "Platform/Text.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Entity/ObjectBounds.h"
#include "Entity/ObjectPose.h"
#include "Render/Skinning.h"
#include "Render/TransformMath.h"
#include "Render/TransformWorkers.h"
#include <vector>

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl operator_delete(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// FUN_004FAA70 @ 0x004FAA70 — Entity_PrepareRenderData(entity_ptr, shadow_pass, lod).
// Copies entity data into the class render struct at DAT_05828D58 + entity_type*0xBC.
// Guards on entity visibility (entity+0x168 >= DAT_005524F8).
// Calls Sprite_Draw (BMD_Animation) and shadow/bone pass (Skeleton_Transform, BMD__RenderBody).
// Returns 1 on success, 0 if out of range.
// IDA: Calc_RenderObject (0x004FAA70)
int __cdecl Calc_RenderObject(OBJECT* object, char param_2, int param_3) {
    BYTE* param_1 = reinterpret_cast<BYTE*>(object);
    float fVar2 = AsObject(param_1)->Alpha;
    if (fVar2 < _DAT_005524f8)
        return 0;
    BYTE* this_ = reinterpret_cast<BYTE*>(DAT_05828d58 + AsObject(param_1)->Type * 0xbc);
    *(DWORD*)(this_ + 0x84) = 0;
    *(BYTE *)(this_ + 0x45) = AsObject(param_1)->ContrastEnable;
    Entity_GetLightScale(reinterpret_cast<uintptr_t>(param_1), reinterpret_cast<uintptr_t>(this_));
    void* puVar3 = AsObject(param_1)->EnableBoneMatrix == 0 ? (void*)&DAT_06970a9c
                                                                  : AsObject(param_1)->BoneMatrices;
    ObjectPose_PrepareBones(object, this_, (float*)puVar3, param_2);
    short cls = AsObject(param_1)->Type;
    _DAT_005597c8 = 1.0f;
    if      (param_3 == 3) _DAT_005597c8 = 1.4f;
    else if (param_3 == 2) _DAT_005597c8 = 1.2f;
    else if (param_3 == 1) {
        // Contorno de seleccion (hover sobre monstruo/NPC).  IDA 0x4FAA70
        // case 1: DOS pasadas en RENDER_COLOR (flag 64), primero un contorno
        // ancho y oscuro y despues uno mas fino y claro, con un color por
        // pasada que depende de Kind (4 = NPC), y el caso Kind == 4 en el
        // factor de escala.
        *(BYTE *)(this_+0x44) = 0;
        const bool npc = (*(char *)&AsObject(param_1)->Kind == '\x04');
        for (int pass = 0; pass < 2; ++pass) {
            if (npc) {
                *(float*)(this_+0x48) = pass ? 0.16f : 0.02f;
                *(float*)(this_+0x4c) = pass ? 0.7f  : 0.1f;
                _DAT_005597c8 = pass ? 1.08f : 1.2f;
            } else {
                *(float*)(this_+0x48) = pass ? 0.7f  : 0.1f;
                *(float*)(this_+0x4c) = pass ? 0.07f : 0.01f;
                if (cls == 0x12e)
                    _DAT_005597c8 = pass ? 1.08f : 1.2f;
                else
                    _DAT_005597c8 = (pass ? 0.039999999f : 0.1f) / AsObject(param_1)->Scale + 1.0f;
            }
            *(float*)(this_+0x50) = 0.0f;
            puVar3 = (*(char *)&AsObject(param_1)->EnableBoneMatrix=='\0') ? &DAT_06970a9c : *(void**)(param_1+0x114);
            // IDA pasa `Translate` tal cual a sub_4404E0 (BMD_Animation recibe !Translate).
            Skeleton_Transform(this_, reinterpret_cast<float*>(puVar3), (float*)(param_1+0x118),
                         (float*)(param_1+0x124), (float*)(param_1+0x130), param_2);
            BMD__RenderBody(this_, 0x40,
                         AsObject(param_1)->Alpha, AsObject(param_1)->BlendMesh,
                         AsObject(param_1)->BlendMeshLight,  AsObject(param_1)->BlendMeshTexCoordU,
                         AsObject(param_1)->BlendMeshTexCoordV,  AsObject(param_1)->HiddenMesh, 0xffffffff);
        }
        Entity_GetLightScale(reinterpret_cast<uintptr_t>(param_1), reinterpret_cast<uintptr_t>(this_));
        _DAT_005597c8 = 1.0f;
    }
    puVar3 = (*(char *)&AsObject(param_1)->EnableBoneMatrix=='\0') ? (void*)&DAT_06970a9c : *(void**)(param_1+0x114);
    Skeleton_Transform(this_, reinterpret_cast<float*>(puVar3), (float*)(param_1+0x118),
                 (float*)(param_1+0x124), (float*)(param_1+0x130), param_2);
    return 1;
}
// Skeleton_Transform @ 0x004404e0 — Skeleton_Transform (thiscall: bone→vertex pipeline)
// Applies bone matrices from param_1 to all mesh vertices; stores results in
// DAT_05846224 (vertex buffer, stride 3 floats per vertex, indexed by sub-mesh).
// param_1  = bone transform array (param_1[bone*0x30] = 3x4 matrix)
// param_2  = AABB min ptr (in), param_3 = AABB max ptr (in)
// param_4  = AABB output ptr [12 floats: min.xyz, size.xyz, 3 zeros, size.w]
// param_5  = if non-zero: also applies entity scale+offset (this+0x68/6c/70/74)
// Uses _DAT_005597c8 scale flag: if != 1.0 → calls Vector_Rotate (scaled transform).
void __cdecl Skeleton_Transform(void* this_, float* boneMatrices, float* param_2, float* param_3,
                           float* param_4, char param_5) {
    // [0-2]=AABB min, [3-5]=AABB max / work dir, [6..8]=transformed dir.
    // In the Ghidra decompile local_54[7]/[8] appear as adjacent stack slots
    // local_38/local_34. A previous port declared those as separate floats, so
    // Vector_InverseRotate writing 3 floats at &local_54[6] overflowed past local_54's
    // 7-elem bound → /GS stack-cookie corruption. Keep them as one 9-elem array
    // and reference [7]/[8] directly where the Ghidra names appear.
    BYTE* param_1 = reinterpret_cast<BYTE*>(boneMatrices);
    Skinning::Invalidate();
    float local_54[9];
    // Inicializar sólo la dirección capturada cuando la iluminación está desactivada.
    local_54[6]=local_54[7]=local_54[8]=0.f;
    float local_30[12];
    int   local_58 = 0;
    int   local_64 = 0, local_6c = 0;
    // El original (IDA sub_4404E0 lines 84/172/206) mantiene DOS punteros — v37
    // es la BASE del mesh actual y v35 = v37 al entrar a cada mesh, para que ++v35
    // itere por normales y v37 += 15000 al final del mesh avance a la base del
    // siguiente mesh ABSOLUTAMENTE. Con un único puntero se acumularía el offset
    // de normales y se desalinearían los slots que BMD_DrawMesh lee en
    // DAT_077e298c + meshIdx*15000.
    float* local_68_base = (float*)&DAT_077e298c;  // base del mesh actual
    float* local_68      = local_68_base;          // puntero de trabajo
    intptr_t local_74;

    // ── DIAG: log Translate flag + scale + world offset for first N calls ────
    // Restrict to in-game and Lorencia-area entities to avoid log flood.
    static int s_xform_dbg = 0;
    int  diag_this_call = s_xform_dbg;
    bool diagInGame = (SceneFlag == 5);
    if (diagInGame && s_xform_dbg < 200) {
        char b[256];
        float sc = *(float*)((char*)this_ + 0x68);
        float px = *(float*)((char*)this_ + 0x6c);
        float py = *(float*)((char*)this_ + 0x70);
        float pz = *(float*)((char*)this_ + 0x74);
        short nm = *(short*)((char*)this_ + 0x24);
    }

    // optional lighting direction compute
    if (*(char*)((char*)this_ + 0x44) != '\0') {
        if (DAT_005597c4 == '\0') {
            local_54[3] = 0.0f; local_54[4] = -1.5f; local_54[5] = 0.0f;
        } else {
            local_54[3] = 1.3f; local_54[4] = 0.0f; local_54[5] = 2.0f;
        }
        Matrix_BuildFromEuler((float*)((char*)this_ + 0x8c), local_30);
        Vector_InverseRotate(local_54 + 3, local_30, local_54 + 6);
    }
    // init AABB for bounding-box mode
    if (DAT_07e11d30 == 2) {
        local_54[0]=999999.0f; local_54[1]=999999.0f; local_54[2]=999999.0f;
        local_54[3]=-999999.0f; local_54[4]=-999999.0f; local_54[5]=-999999.0f;
    }
    // per-sub-mesh bone transform loop
    int meshCount = (int)(short)*(short*)((char*)this_ + 0x24);
    // GUARD (desviación): si el modelo (this_) tiene el puntero Meshs (+0x28)
    // NULL/basura o un meshCount insano (preview char del panel crear-personaje sin
    // modelo válido), abortar antes de deferenciar.
    {
        unsigned int meshsPtr = (unsigned int)*(int*)((char*)this_ + 0x28);
        if (meshsPtr < 0x10000u || meshCount < 0 || meshCount > 200) {
            static bool s_loggedNoMeshs = false;
            if (!s_loggedNoMeshs) {
                s_loggedNoMeshs = true;
                char _nb[120];
                Platform::TextFormatTruncated(_nb, sizeof(_nb), Platform::TruncateText,
                    "XFORM_NO_MESHS etype=%d meshsPtr=%08x meshCount=%d",
                    (int)*(short*)((char*)this_ + 2), meshsPtr, meshCount);
                DbgLogPublic(_nb);
            }
            return;
        }
    }
    // DESVIACION: agrupar sólo matemática independiente (IDA 0x004404E0).
    // Editor y datos inválidos conservan el recorrido original y sus diagnósticos.
    std::size_t transformCount = 0;
    bool parallelSafe = DAT_07e11d30 != 2;
    for (int i=0; parallelSafe && i<meshCount; ++i) {
        const char* mesh=(const char*)(uintptr_t)*(DWORD*)((char*)this_+0x28)+i*0x28;
        const int vc=*(const short*)(mesh+4), nc=*(const short*)(mesh+6);
        if(vc<0 || nc<0) { parallelSafe=false; break; }
        transformCount+=vc+nc;
    }
    // No despertar trabajadores para las numerosas mallas pequeñas del juego.
    const float transformCoefficients[3]={_DAT_00552530,_DAT_005528b4,_DAT_005526e4};
    bool transformed = parallelSafe && ObjectPose_RestoreGeometry(this_,(const float*)(uintptr_t)param_1,
        _DAT_005597c8,param_5,local_54+6,transformCoefficients,(float*)&DAT_0584621c,
        (float*)&DAT_06f433bc,(float*)&DAT_077e298c);
    if (!transformed && parallelSafe && transformCount >= 4096) {
        static std::vector<TransformMath::Mesh> transformMeshes;
        transformMeshes.clear();
        for(int i=0;i<meshCount;++i) {
            const char* mesh=(const char*)(uintptr_t)*(DWORD*)((char*)this_+0x28)+i*0x28;
            const int vc=*(const short*)(mesh+4), nc=*(const short*)(mesh+6);
            const auto* vp=(const unsigned char*)(uintptr_t)*(DWORD*)(mesh+0x10);
            const auto* np=(const unsigned char*)(uintptr_t)*(DWORD*)(mesh+0x14);
            // Si se superponen slots de salida, conservar el orden secuencial.
            if((uintptr_t)vp<0x10000u || (uintptr_t)np<0x10000u ||
               vc>15000 || nc>15000) { parallelSafe=false; break; }
            transformMeshes.push_back({vp,np,(float*)((char*)&DAT_0584621c+i*180000),
                (float*)((char*)&DAT_06f433bc+i*180000),(float*)&DAT_077e298c+i*15000,
                (std::size_t)vc,(std::size_t)nc});
        }
        const int nBones=*(const short*)((char*)this_+0x22);
        for (const auto& mesh : transformMeshes) {
            for(std::size_t i=0;i<mesh.vertexCount;++i) {
                const int bone=*(const short*)(mesh.vertices+i*16);
                if(bone<0 || bone>=200 || (nBones>0 && bone>=nBones)) parallelSafe=false;
            }
            for(std::size_t i=0;i<mesh.normalCount;++i) {
                const int bone=*(const short*)(mesh.normals+i*20);
                if(bone<0 || bone>=200 || (nBones>0 && bone>=nBones)) parallelSafe=false;
            }
        }
        if(parallelSafe) {
            TransformMath::Batch batch{transformMeshes.data(),transformMeshes.size(),
                (const float*)(uintptr_t)param_1,_DAT_005597c8,*(float*)((char*)this_+0x68),
                {*(float*)((char*)this_+0x6c),*(float*)((char*)this_+0x70),*(float*)((char*)this_+0x74)},
                {local_54[6],local_54[7],local_54[8]},
                {_DAT_00552530,_DAT_005528b4,_DAT_005526e4},
                _DAT_005597c8==_DAT_0055256c,param_5!='\0',*(char*)((char*)this_+0x44)!='\0'};
            auto& workers=TransformWorkers::SharedTeam();
            workers.Run(transformCount,&batch,TransformMath::Run);
            transformed=true;
        }
    }
    if (!transformed)
    for (local_58 = 0; local_58 < meshCount; local_58++) {
        int meshPtr = *(int*)((char*)this_ + 0x28) + local_6c;
        int vertCount = (int)(short)*(short*)(meshPtr + 4);
        // GUARD (desviación): el preview char del panel crear-personaje puede quedar
        // con una malla cuyo array de vértices/normales es NULL (modelo sin cargar).
        // Si la malla es inválida, se saltea (avanzando los offsets per-mesh) y se
        // loguea una vez el entity_type.
        {
            unsigned int vArr = (unsigned int)*(int*)(meshPtr + 0x10);
            unsigned int nArr = (unsigned int)*(int*)(meshPtr + 0x14);
            int nCount = (int)(short)*(short*)(meshPtr + 6);
            bool bad = (vArr < 0x10000u ||
                        nArr < 0x10000u ||
                        vertCount < 0 || vertCount > 20000 ||
                        nCount   < 0 || nCount   > 20000);
            if (bad) {
                static bool s_loggedInvMesh = false;
                if (!s_loggedInvMesh) {
                    s_loggedInvMesh = true;
                    char _ib[160];
                    Platform::TextFormatTruncated(_ib, sizeof(_ib), Platform::TruncateText,
                        "XFORM_INVALID_MESH etype=%d meshsPtr=%p mesh=%d/%d vArr=%08x nArr=%08x vc=%d nc=%d",
                        (int)*(short*)((char*)this_ + 2), (void*)(intptr_t)*(int*)((char*)this_ + 0x28),
                        local_58, meshCount, vArr, nArr, vertCount, nCount);
                    DbgLogPublic(_ib);
                }
                local_6c += 0x28;
                local_68_base += 15000;
                local_64 += 180000;
                continue;
            }
        }
        local_74 = -(reinterpret_cast<intptr_t>(&DAT_0584621c) + local_64);
        float* pfOut = (float*)((char*)&DAT_05846224 + local_64);
        // GUARD (desviación): el indice de hueso del vertice (`*vs`) indexa
        // `BoneTransform` con stride 0x30, y ese bloque lo aloca
        // CreateCharacterPointer como `operator_new(48 * numBones)`.  Un vertice
        // que referencie un hueso fuera de rango lee fuera del bloque -> AV dentro de
        // Vector_Transform.
        //
        // El original no acota; aca se saltea el vertice y se loguea UNA vez con
        // el tipo de entidad, el modelo y los dos numeros, para poder atacar la
        // causa (que el modelo y el BoneTransform no correspondan) con datos.
        const int nBonesGuard = (int)(short)*(short*)((char*)this_ + 0x22);
        for (int vi = 0; vi < vertCount; vi++) {
            short* vs = (short*)((char*)(vi * 0x10) + *(int*)(meshPtr + 0x10));
            const int boneIdx = (int)*vs;
            if (boneIdx < 0 || (nBonesGuard > 0 && boneIdx >= nBonesGuard) || boneIdx >= 200) {
                static bool s_loggedBadBone = false;
                if (!s_loggedBadBone) {
                    s_loggedBadBone = true;
                    char _bb[180];
                    Platform::TextFormatTruncated(_bb, sizeof(_bb), Platform::TruncateText,
                        "XFORM_BAD_BONE etype=%d mesh=%d/%d vert=%d/%d bone=%d nBones=%d bt=%p",
                        (int)*(short*)((char*)this_ + 2), local_58, meshCount,
                        vi, vertCount, boneIdx, nBonesGuard, (void*)param_1);
                    DbgLogPublic(_bb);
                }
                pfOut += 3;
                continue;
            }
            float* pfDst = pfOut - 2;
            if (_DAT_005597c8 == _DAT_0055256c) {
                Vector_Transform((float*)(vs + 2), (float*)(*vs * 0x30 + param_1), pfDst);
                if (param_5 != '\0') {
                    float sc = *(float*)((char*)this_ + 0x68);
                    pfDst[0] *= sc; pfOut[-1] *= sc; *pfOut *= sc;
                }
            } else {
                Vector_Rotate((float*)(vs + 2), (float*)(*vs * 0x30 + param_1), pfDst);
                pfDst[0]  = _DAT_005597c8 * pfDst[0]  + *(float*)(*vs * 0x30 + 0x0c + param_1);
                pfOut[-1] = _DAT_005597c8 * pfOut[-1] + *(float*)(*vs * 0x30 + 0x1c + param_1);
                *pfOut    = _DAT_005597c8 * *pfOut    + *(float*)(*vs * 0x30 + 0x2c + param_1);
                if (param_5 != '\0') {
                    float sc = *(float*)((char*)this_ + 0x68);
                    pfDst[0] *= sc; pfOut[-1] *= sc; *pfOut *= sc;
                }
            }
            if (DAT_07e11d30 == 2) {
                // update AABB (min/max per axis)
                if (pfDst[0] < local_54[0]) local_54[0] = pfDst[0];
                if (local_54[3] < pfDst[0]) local_54[3] = pfDst[0];
                if (pfOut[-1] < local_54[1]) local_54[1] = pfOut[-1];
                if (local_54[4] < pfOut[-1]) local_54[4] = pfOut[-1];
                if (*pfOut < local_54[2]) local_54[2] = *pfOut;
                if (local_54[5] < *pfOut) local_54[5] = *pfOut;
            }
            if (param_5 != '\0') {
                pfDst[0]  += *(float*)((char*)this_ + 0x6c);
                pfOut[-1] += *(float*)((char*)this_ + 0x70);
                *pfOut    += *(float*)((char*)this_ + 0x74);
            }
            pfOut += 3;
        }
        // normal transform + per-vertex lighting (IDA sub_4404E0 L172-206).
        // Son DOS buffers separados:
        //   • normal transformada → NORMAL buffer (DAT_06f433bc + local_64,
        //     stride 3 floats), que lee el chrome env-map en BMD_DrawMesh.
        //   • intensidad de luz (dot con la dir de luz) → LIGHT buffer
        //     (DAT_077e298c, stride 1 float).
        // `v20 = &unk_6F433BC + v9` (normal) y `v35 = v37` (light) son punteros
        // independientes, fiel a IDA.
        local_68 = local_68_base;   // v35 — light buffer (1 float/vertex)
        float* pfNrmBase = (float*)((char*)&DAT_06f433bc + local_64);  // v20 — normal buffer (3 floats/normal)
        int normCount = (int)(short)*(short*)(meshPtr + 6);
        for (int ni = 0; ni < normCount; ni++) {
            short* ns = (short*)(ni * 0x14 + *(int*)(meshPtr + 0x14));
            // VectorRotate(normal_src, boneMatrix, dst=normal buffer)
            Vector_Rotate((float*)(ns + 2), (float*)(*ns * 0x30 + param_1), pfNrmBase);
            if (*(char*)((char*)this_ + 0x44) != '\0') {
                // dot(transformed_light_dir, transformed_normal):
                // Ghidra local_38 == local_54[7] (Y), local_34 == local_54[8] (Z).
                float lit = (local_54[7] * pfNrmBase[1] + local_54[6] * pfNrmBase[0] + local_54[8] * pfNrmBase[2])
                            * _DAT_00552530 + _DAT_005528b4;
                if (lit < _DAT_005526e4) lit = _DAT_005526e4;
                *local_68 = lit;
            }
            local_68++;       // light buffer += 1 float
            pfNrmBase += 3;   // normal buffer += 3 floats
        }
        // ── DIAG: dump first transformed vertex of mesh 0 on first call ──
        if (diagInGame && diag_this_call < 200 && local_58 == 0 && vertCount > 0) {
            float* pfFirst = (float*)((char*)&DAT_05846224 + 0);
        }
        local_6c += 0x28;
        // Avanzar la BASE del mesh 15000 floats (60000 bytes)
        // ABSOLUTAMENTE — equivale a `v37 += 15000` en IDA line 206.
        local_68_base += 15000;
        local_64 += 180000;
    }
    // DESVIACION: capturar huesos/atributos de esta pose antes de que otro
    // objeto reutilice los scratch CPU. No retirar los consumidores lógicos.
    static std::vector<Skinning::Source> skinSources;
    skinSources.clear();
    for(int i=0;i<meshCount;++i) {
        const char* mesh=(const char*)(uintptr_t)*(DWORD*)((char*)this_+0x28)+i*0x28;
        const int vc=*(const short*)(mesh+4),nc=*(const short*)(mesh+6);
        const auto* vp=(const unsigned char*)(uintptr_t)*(DWORD*)(mesh+0x10);
        const auto* np=(const unsigned char*)(uintptr_t)*(DWORD*)(mesh+0x14);
        if((uintptr_t)vp<0x10000u || (uintptr_t)np<0x10000u || vc<=0 || vc>20000 || nc<0 || nc>20000) continue;
        skinSources.push_back({mesh,vp,np,(const float*)((const char*)&DAT_0584621c+i*180000),
            (const float*)&DAT_077e298c+i*15000,vc,nc});
    }
    const float zero[3]{},coefficients[3]{_DAT_00552530,_DAT_005528b4,_DAT_005526e4};
    Skinning::Capture(this_,(const float*)(uintptr_t)param_1,(std::min)(200,int(*(short*)((char*)this_+0x22))),
        _DAT_005597c8,param_5?*(float*)((char*)this_+0x68):1.f,
        param_5?(float*)((char*)this_+0x6c):zero,local_54+6,coefficients,skinSources.data(),skinSources.size());
    if (diagInGame && s_xform_dbg < 200) s_xform_dbg++;
    // El editor usa los extremos de la malla; la ruta normal, los bounds locales.
    const float* boundsMin = DAT_07e11d30 == 2 ? local_54 : param_2;
    const float* boundsMax = DAT_07e11d30 == 2 ? local_54 + 3 : param_3;
    ObjectBounds_Build(boundsMin, boundsMax, (float*)((char*)this_ + 0x6c), param_4);
    // compute largest dimension for bounding radius
    float dx=local_54[3]-local_54[0], dy=local_54[4]-local_54[1], dz=local_54[5]-local_54[2];
    float maxD = (dy > dx ? dy : dx);
    if (dz > maxD) maxD = (dx <= dy ? dy : dx);
    *(float*)((char*)this_ + 0xb4) = maxD;
}
// BMD_TransformPosition — implemented in src/Math/Math_3D.cpp (Bone_TransformVertex)
// Triangle_ComputeNormal — implemented in src/Math/Math_3D.cpp
// Particle_Spawn — implemented in src/Render/Particle_Spawn.cpp (Particle_Spawn)
// CreateSprite — implemented in src/Render/Particle.cpp (Effect_Spawn, returns int)
// PressKey — implemented in src/Input/Input.cpp
// UIChatLogWindow_AddText — UIChatLogWindow_AddText — implemented above as UIChatLogWindow_AddText
// TestFrustrum2D — implemented in src/Terrain/Terrain_Utils.cpp
// OpenJPG — implemented in src/Render/Texture/Texture.cpp (Texture_Load)
// ═════════════════════════════════════════════════════════════════════════════

// CSimpleModulus crypto (CSimpleModulus_Encode/cd20/cca0/ce30 + helpers) vive en
// src/Net/Crypto.cpp.

// Chat_ValidateInputCommand — implemented in src/UI/Chat.cpp
// GL_DrawTexture — implemented in src/Render/GL_2D.cpp
// RenderSprite_0 — implemented in src/Render/Sprite.cpp (Sprite_DrawTexturedQuad)

// CIsin @ 0x005433b0 — GetTickCount-based time accumulator
// Takes x87 FPU float10 (in_ST0) from the FPU stack, converts to double, passes to
// checkTOS_withFB (store low DWORD) + FUN_005433cd (return via x87). Returns float10.
// Ghidra shows void; real calling convention returns value on x87 stack.
float10 __cdecl CIsin(void) { return 0.0L; }

// ── Screen coordinate converters (@ 0x00511950 / 0x00511980) ─────────────────
float __cdecl Screen_ToGLx(float x) { return x; }
float __cdecl Screen_ToGLy(float y) { return y; }

// ── AttackEffect / UseSkillWarrior ────────────────────────────────────────────
