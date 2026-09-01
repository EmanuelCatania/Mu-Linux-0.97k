// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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
// Calls Sprite_Draw (FUN_00440060) and shadow/bone pass (FUN_004404E0, FUN_00441E00).
// Returns 1 on success, 0 if out of range.
int __cdecl FUN_004faa70(int param_1, char param_2, int param_3) {
    float fVar2 = *(float*)(param_1 + 0x168);
    if (fVar2 < _DAT_005524f8)
        return 0;
    void* this_ = (void*)(DAT_05828d58 + *(short*)(param_1 + 2) * 0xbc);
    *(DWORD*)((int)this_ + 0x84) = 0;
    *(BYTE *)((int)this_ + 0x45) = *(BYTE*)(param_1 + 0xe4);
    FUN_004fa930(param_1, (int)this_);
    *(DWORD*)((int)this_ + 0x68) = *(DWORD*)(param_1 + 0x0c);
    *(BYTE *)((int)this_ + 0xa0) = *(BYTE*)(param_1 + 0x105);
    *(DWORD*)((int)this_ + 0x6c) = *(DWORD*)(param_1 + 0x10);
    *(DWORD*)((int)this_ + 0x70) = *(DWORD*)(param_1 + 0x14);
    *(DWORD*)((int)this_ + 0x74) = *(DWORD*)(param_1 + 0x18);
    // Special-case position Y offset for Elf (0x14B) and variant (0x14A)
    short cls = *(short*)(param_1 + 2);
    if (cls == 0x14b) {
        *(DWORD*)((int)this_ + 0x6c) = *(DWORD*)(param_1 + 0x10);
        *(float *)((int)this_ + 0x70) = *(float*)(param_1 + 0x14) + _DAT_0055290c;
        *(DWORD*)((int)this_ + 0x74) = *(DWORD*)(param_1 + 0x18);
    } else if (cls == 0x14a) {
        *(DWORD*)((int)this_ + 0x6c) = *(DWORD*)(param_1 + 0x10);
        *(float *)((int)this_ + 0x70) = *(float*)(param_1 + 0x14) + _DAT_00552908;
        *(DWORD*)((int)this_ + 0x74) = *(DWORD*)(param_1 + 0x18);
    }
    BYTE  bVar4  = *(BYTE*)(param_1 + 0x106);
    DWORD uVar1  = *(DWORD*)(param_1 + 0x10c);
    float fVar2b = *(float*)(param_1 + 0x108);
    void* puVar3 = (*(char*)(param_1 + 0x110) == '\0') ? &DAT_06970a9c
                                                         : *(void**)(param_1 + 0x114);
    FUN_00440060(this_, (int)puVar3, fVar2b, uVar1, bVar4,
                 (unsigned int*)(param_1+0x1c), (float*)(param_1+0x28), '\0', param_2=='\0');
    _DAT_005597c8 = 1.0f;
    if      (param_3 == 3) _DAT_005597c8 = 1.4f;
    else if (param_3 == 2) _DAT_005597c8 = 1.2f;
    else if (param_3 == 1) {
        // shadow pass: set tint colors, draw twice with bone animation
        *(BYTE *)((int)this_+0x44) = 0;
        bool dark = (*(char*)(param_1+0x84) == '\x04');
        *(float*)((int)this_+0x48) = dark ? 0.02f   : 0.1f;
        *(float*)((int)this_+0x4c) = dark ? 0.4f    : 0.02f;
        *(float*)((int)this_+0x50) = 0.0f;
        _DAT_005597c8 = (cls == 0x12e) ? 1.2f
                        : (_DAT_005524f4 / *(float*)(param_1+0x0c) + _DAT_0055256c);
        puVar3 = (*(char*)(param_1+0x110)=='\0') ? &DAT_06970a9c : *(void**)(param_1+0x114);
        // BUG-FIX CRÍTICO (2026-04-21): el original IDA @ 0x004FAA70 pasa
        // `Translate` RAW (no !Translate) al último arg de Skeleton_Transform
        // (sub_4404E0, líneas 112/151/173). En cambio BMD_Animation recibe
        // `!Translate`. La sesión previa invirtió ambos → doble aplicación
        // de entity scale+origin (root matrix Y per-vertex) → ships en
        // (-1260,1260,0) en vez de (-700,700,0). Volvemos a `param_2` raw.
        FUN_004404e0(this_, (int)puVar3, (float*)(param_1+0x118),
                     (float*)(param_1+0x124), (float*)(param_1+0x130), param_2);
        FUN_00441e00(this_, 0x40,
                     *(float*)(param_1+0x168), *(int *)(param_1+100),
                     *(float*)(param_1+0x68),  *(float*)(param_1+0x6c),
                     *(float*)(param_1+0x70),  *(int *)(param_1+0x58), 0xffffffff);
        FUN_004fa930(param_1, (int)this_);
        _DAT_005597c8 = 1.0f;
    }
    puVar3 = (*(char*)(param_1+0x110)=='\0') ? (void*)&DAT_06970a9c : *(void**)(param_1+0x114);
    FUN_004404e0(this_, (int)puVar3, (float*)(param_1+0x118),
                 (float*)(param_1+0x124), (float*)(param_1+0x130), param_2);
    return 1;
}
// FUN_00440060 @ 0x00440060 — Sprite_Draw (thiscall: model animation interpolation)
// Computes animated bone transforms for the current frame.
// param_1 = bone transform array (param_1[bone * 0x30] = 3x4 matrix per bone)
// param_2 = anim speed (float), param_3 = RGBA color, param_4 = current anim index,
// param_5 = RGBA ptr (root-bone position hint), param_6 = position offset float[3],
// param_7/8 = flags for root-bone translation.
// Output: bone quaternion+position stored in DAT_05826E18 (bone*0x10 stride).
//         Final bone matrices written to param_1 (3x4 per bone).
void __cdecl FUN_00440060(void* this_, int param_1, float param_2, unsigned int param_3,
                           unsigned char param_4, unsigned int* param_5,
                           float* param_6, char param_7, char param_8) {
    int*  piModel = (int*)this_;
    // clamp anim indices to bone count
    short boneCount = *(short*)((char*)this_ + 0x26);
    if (boneCount <= (short)(unsigned short)param_4)
        param_4 = 0;
    if (boneCount <= (short)(unsigned short)*(unsigned char*)((char*)this_ + 0xa0))
        *(unsigned char*)((char*)this_ + 0xa0) = 0;
    // store current color/frame in model header
    *(unsigned int*)((char*)this_ + 0x78) = param_5[0];
    *(unsigned int*)((char*)this_ + 0x7c) = param_5[1];
    *(unsigned int*)((char*)this_ + 0x80) = param_5[2];
    *(float*)((char*)this_ + 0xa4) = param_2;
    // compute fractional frame and frame indices
    long long lv = (long long)(int)param_2;
    short curFrame = (short)lv;
    *(short*)((char*)this_ + 0xa8) = curFrame;
    float fFrac = param_2 - (float)(int)curFrame;
    float fFrac2 = _DAT_0055256c - fFrac;
    // BUG-FIX CRÍTICO: param_3 es realmente `float PriorFrame` (IDA firma),
    // no un entero. La firma C nuestra lo declara `unsigned int` porque el
    // caller en FUN_004faa70 lo carga con *(DWORD*)(entity+0x10c) y los bits
    // del float caben en un DWORD. Hay que reinterpretar las bits → float
    // y truncar a int (== IDA: v12 = (__int64)PriorFrame; v39 = v12).
    // Antes calculábamos v39 = (int)(1.0 - fFrac) que es 0 ó 1 siempre →
    // cada bone muestreaba el MISMO keyframe → geometría explotada.
    float priorFrame;
    memcpy(&priorFrame, &param_3, 4);
    int v39 = (int)priorFrame;
    if (boneCount > 0) {
        int* animA = (int*)(*(int*)((char*)this_ + 0x30) + (unsigned int)param_4 * 0x10);
        if (*(short*)(animA + 2) <= v39) v39 = 0;
        // IDA también clamp this[+0xA8] contra anim[this+0xA0]'s length:
        //   if ( v11 >= *(__int16 *)(16 * *(unsigned __int8*)(This+160) + v13 + 8) )
        //       *(_WORD *)(This + 168) = 0;
        int animBIdxClamp = *(unsigned char*)((char*)this_ + 0xa0);
        int* animBHdr = (int*)(*(int*)((char*)this_ + 0x30) + animBIdxClamp * 0x10);
        if (*(short*)(animBHdr + 2) <= *(short*)((char*)this_ + 0xa8))
            *(short*)((char*)this_ + 0xa8) = 0;
        // per-bone loop: interpolate quaternions and build transforms
        for (int boneIdx = 0; boneIdx < (int)(short)*(short*)((char*)this_+0x22); boneIdx++) {
            int bonePtr = *(int*)((char*)this_ + 0x2c) + boneIdx * 0x8c;
            if (*(char*)(bonePtr + 0x22) == '\0') {
                int* animPtrA = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)param_4 * 0xc);
                int* animPtrB = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)*(unsigned char*)((char*)this_ + 0xa0) * 0xc);
                float quatA[4], quatB[4];
                if (boneIdx == *(int*)((char*)this_ + 0x54)) {
                    // root bone: leer Euler angles (3 floats, stride 0xc) y
                    // convertir a quat via FUN_004fa1d0 (EulerToQuat).
                    // BUG-FIX: antes faltaba el componente Y (posA[1]/posB[1]).
                    // FUN_004fa1d0 lee los 3 componentes (línea 25: v5 = a1[1]*0.5)
                    // → con Y=stack garbage el quat salía arbitrario.
                    // IDA: v43[1] = *(_DWORD *)(v21 + 4); (Y sin ajuste HeadAngle)
                    float posA[4], posB[4];
                    int   curFrameB = *(short*)((char*)this_ + 0xa8);
                    posA[0] = *(float*)(animPtrA[1] + v39 * 0xc)     - param_6[0] * _DAT_005528b0;
                    posA[1] = *(float*)(animPtrA[1] + v39 * 0xc + 4);
                    posA[2] = *(float*)(animPtrA[1] + v39 * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    posB[0] = *(float*)(animPtrB[1] + curFrameB * 0xc)     - param_6[0] * _DAT_005528b0;
                    posB[1] = *(float*)(animPtrB[1] + curFrameB * 0xc + 4);
                    posB[2] = *(float*)(animPtrB[1] + curFrameB * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    FUN_004fa1d0((int)&posA[0], (int)&quatA[0], 0, 0);
                    FUN_004fa1d0((int)&posB[0], (int)&quatB[0], 0, 0);
                } else {
                    // regular bone: read quaternion from anim table
                    int frameOff = v39 * 0x10;
                    quatA[0] = *(float*)(animPtrA[2] + frameOff);
                    quatA[1] = *(float*)(animPtrA[2] + frameOff + 4);
                    quatA[2] = *(float*)(animPtrA[2] + frameOff + 8);
                    quatA[3] = *(float*)(animPtrA[2] + frameOff + 0xc);
                    int frameOffB = *(short*)((char*)this_ + 0xa8) * 0x10;
                    quatB[0] = *(float*)(animPtrB[2] + frameOffB);
                    quatB[1] = *(float*)(animPtrB[2] + frameOffB + 4);
                    quatB[2] = *(float*)(animPtrB[2] + frameOffB + 8);
                    quatB[3] = *(float*)(animPtrB[2] + frameOffB + 0xc);
                }
                // slerp between quatA and quatB
                // IDA: `(char *)&unk_5826E18 + 16 * boneIdx` — paso de 16 BYTES.
                // Con `&DAT_05826e18` (DWORD*) el paso salía de 64 bytes.
                int quatBone = boneIdx;
                if (quatBone < 0 || quatBone >= 200) quatBone = 0;   // guard: la tabla es de 200 huesos
                float* outQuat = (float*)(DAT_05826e18 + quatBone * 0x10);
                int same = FUN_004f9c70((int)&quatA, (int)&quatB, 0, 0);
                if (!same)
                    FUN_004fa350((int)&quatA, (int)&quatB, *(int*)&fFrac, (int)outQuat); // BUG-FIX: pass float bits, not truncated int
                else { outQuat[0]=quatA[0]; outQuat[1]=quatA[1]; outQuat[2]=quatA[2]; outQuat[3]=quatA[3]; }
                // quaternion to 3x3 rotation matrix, embedded in a 3x4.
                // BUG-FIX: FUN_004fa270 solo escribe posiciones [0,1,2,4,5,6,8,9,10]
                // (9 floats de 3x3). Las posiciones 3,7,11 (columna de translación)
                // quedaban sin inicializar. FUN_004f9f70 las lee como translation,
                // así que generaba bone matrices con translate = stack garbage →
                // vértices astronómicos. Zero-init + inyectar tX/tY/tZ abajo.
                float rot33[12] = {0};
                FUN_004fa270((int)outQuat, (int)rot33, 0, 0);
                // get translation from frame keys (3 floats × priorFrameInt)
                float* ptransA = (float*)(*animPtrA + v39 * 0xc);
                float* ptransB = (float*)(*animPtrB + *(short*)((char*)this_ + 0xa8) * 0xc);
                float tX, tY, tZ;
                if (boneIdx == 0 &&
                    (*(char*)((unsigned int)param_4 * 0x10 + 10 + *(int*)((char*)this_+0x30)) != '\0' ||
                     *(char*)((unsigned int)*(unsigned char*)((char*)this_+0xa0)*0x10+10+*(int*)((char*)this_+0x30)) != '\0')) {
                    // root-bone override: IDA toma frame 0 de animB (NO animA)
                    //   v52 = **(float **)v17;              // v17 = animB
                    //   v53 = *(float *)(*(_DWORD *)v17+4);
                    // Antes usábamos *animPtrA → offset origen equivocado en anim
                    // de transición entre acciones (animA≠animB).
                    tX = *(float*)*animPtrB;
                    tY = *(float*)(*animPtrB + 4);
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2] + *(float*)((char*)this_ + 0x84);
                } else {
                    tX = fFrac2 * ptransA[0] + fFrac * ptransB[0];
                    tY = fFrac * ptransB[1] + fFrac2 * ptransA[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2];
                }
                // apply parent transform
                short parentIdx = *(short*)(bonePtr + 0x20);
                float* parentMat;
                if (parentIdx == -1) {
                    if (param_7 == '\0') {
                        Matrix_BuildFromEuler((float*)((char*)this_ + 0x78), &DAT_06989c9c);
                        if (param_8 != '\0') {
                            // BUG-FIX 2026-05-03 (cross-ref con 5.2 ZzzBMD.cpp:153-159):
                            // El IDA decompile mostraba solo 3 escalas de diagonales
                            // (matrix[0][0], [1][1], [2][2] = offsets 0/5/10) — eso
                            // se LEÍA mal. El source 5.2 limpio muestra que se
                            // escalan los 9 elementos rotacionales (3x3 completa):
                            //   for (y=0; y<3; y++)
                            //     for (x=0; x<3; x++)
                            //       ParentMatrix[y][x] *= BodyScale;
                            // Sin escalar los 6 off-diagonal, la rotación queda
                            // deformada → bone matrices torcidas → vertices con
                            // posiciones distorsionadas → "imp-like" body parts.
                            // Translate flag escribe BodyOrigin en columna [3].
                            float sc = *(float*)((char*)this_ + 0x68);
                            float* pm = &DAT_06989c9c_matrix[0];
                            // Scale full 3x3 rotation block (rows 0..2, cols 0..2).
                            // Layout: pm[y*4 + x] for row y, col x.
                            for (int y = 0; y < 3; ++y) {
                                for (int x = 0; x < 3; ++x) {
                                    pm[y * 4 + x] *= sc;
                                }
                            }
                            // Translation column [y][3] = BodyOrigin[y].
                            pm[3]  = *(float*)((char*)this_ + 0x6c);  // BodyOrigin[0]
                            pm[7]  = *(float*)((char*)this_ + 0x70);  // BodyOrigin[1]
                            pm[11] = *(float*)((char*)this_ + 0x74);  // BodyOrigin[2]
                        }
                    }
                    parentMat = &DAT_06989c9c;
                } else {
                    parentMat = (float*)(parentIdx * 0x30 + param_1);
                }
                // Inject bone translation into rot33's 3x4 translation column.
                rot33[3]  = tX;
                rot33[7]  = tY;
                rot33[11] = tZ;
                FUN_004f9f70(parentMat, rot33, (float*)(boneIdx * 0x30 + param_1));
            }
        }
    }
}

// FUN_004404e0 @ 0x004404e0 — Skeleton_Transform (thiscall: bone→vertex pipeline)
// Applies bone matrices from param_1 to all mesh vertices; stores results in
// DAT_05846224 (vertex buffer, stride 3 floats per vertex, indexed by sub-mesh).
// param_1  = bone transform array (param_1[bone*0x30] = 3x4 matrix)
// param_2  = AABB min ptr (in), param_3 = AABB max ptr (in)
// param_4  = AABB output ptr [12 floats: min.xyz, size.xyz, 3 zeros, size.w]
// param_5  = if non-zero: also applies entity scale+offset (this+0x68/6c/70/74)
// Uses _DAT_005597c8 scale flag: if != 1.0 → calls Vector_Rotate (scaled transform).
void __cdecl FUN_004404e0(void* this_, int param_1, float* param_2, float* param_3,
                           float* param_4, char param_5) {
    // [0-2]=AABB min, [3-5]=AABB max / work dir, [6..8]=transformed dir.
    // In the Ghidra decompile local_54[7]/[8] appear as adjacent stack slots
    // local_38/local_34. A previous port declared those as separate floats, so
    // Vector_InverseRotate writing 3 floats at &local_54[6] overflowed past local_54's
    // 7-elem bound → /GS stack-cookie corruption. Keep them as one 9-elem array
    // and reference [7]/[8] directly where the Ghidra names appear.
    float local_54[9];
    float local_30[12];
    int   local_58 = 0;
    int   local_64 = 0, local_6c = 0;
    // BUG-FIX: el original (IDA sub_4404E0 lines 84/172/206) mantiene DOS
    // punteros — v37 es la BASE del mesh actual y v35 = v37 al entrar a
    // cada mesh, para que ++v35 itere por normales y v37 += 15000 al
    // final del mesh avance a la base del siguiente mesh ABSOLUTAMENTE.
    // Usar un único puntero (como hacía el port anterior) acumulaba el
    // offset de normales a la suma de bases, desalineando los slots que
    // BMD_DrawMesh lee en DAT_077e298c + meshIdx*15000. Resultado: mesh 0
    // OK, mesh 1+ leía basura (ceros) → per-vertex intensity=0 → vertex
    // colors=0 → velas del ship y otros meshes se ven NEGROS.
    float* local_68_base = (float*)&DAT_077e298c;  // base del mesh actual
    float* local_68      = local_68_base;          // puntero de trabajo
    int   local_74;

    // ── DIAG: log Translate flag + scale + world offset for first N calls ────
    // 2026-05-03 bumped 16→200 to capture in-game frames after login.
    // Restrict to in-game and Lorencia-area entities to avoid log flood.
    static int s_xform_dbg = 0;
    int  diag_this_call = s_xform_dbg;
    bool diagInGame = (DAT_005615c0 == 5);
    if (diagInGame && s_xform_dbg < 200) {
        char b[256];
        float sc = *(float*)((char*)this_ + 0x68);
        float px = *(float*)((char*)this_ + 0x6c);
        float py = *(float*)((char*)this_ + 0x70);
        float pz = *(float*)((char*)this_ + 0x74);
        short nm = *(short*)((char*)this_ + 0x24);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "BMD_Xform #%d  translate=%d  scale=%.3f  pos=(%.1f,%.1f,%.1f)  numMesh=%d  _DAT_5597c8=%.3f",
            s_xform_dbg, (int)(unsigned char)param_5, sc, px, py, pz, (int)nm, _DAT_005597c8);
        DbgLogPublic(b);
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
    // GUARD 2026-07-16: si el modelo (this_) tiene el puntero Meshs (+0x28) NULL/
    // garbage o un meshCount insano (preview char del panel crear-personaje sin
    // modelo válido), abortar antes de deferenciar → evita crash en FUN_004404e0.
    {
        unsigned int meshsPtr = (unsigned int)*(int*)((char*)this_ + 0x28);
        if (meshsPtr < 0x10000u || meshsPtr >= 0x80000000u || meshCount < 0 || meshCount > 200) {
            static bool s_loggedNoMeshs = false;
            if (!s_loggedNoMeshs) {
                s_loggedNoMeshs = true;
                char _nb[120];
                _snprintf_s(_nb, sizeof(_nb), _TRUNCATE,
                    "XFORM_NO_MESHS etype=%d meshsPtr=%08x meshCount=%d",
                    (int)*(short*)((char*)this_ + 2), meshsPtr, meshCount);
                DbgLogPublic(_nb);
            }
            return;
        }
    }
    for (local_58 = 0; local_58 < meshCount; local_58++) {
        int meshPtr = *(int*)((char*)this_ + 0x28) + local_6c;
        int vertCount = (int)(short)*(short*)(meshPtr + 4);
        // GUARD 2026-07-16: el preview char del panel crear-personaje puede quedar
        // con una malla cuyo array de vértices/normales es NULL (modelo sin cargar)
        // → deref de near-null en el loop → crash 0x5A3797 en FUN_004404e0. Si la
        // malla es inválida, se saltea (avanzando los offsets per-mesh) para no
        // crashear. Loguea una vez el entity_type para diagnosticar la raíz.
        {
            unsigned int vArr = (unsigned int)*(int*)(meshPtr + 0x10);
            unsigned int nArr = (unsigned int)*(int*)(meshPtr + 0x14);
            int nCount = (int)(short)*(short*)(meshPtr + 6);
            bool bad = (vArr < 0x10000u || vArr >= 0x80000000u ||
                        nArr < 0x10000u || nArr >= 0x80000000u ||
                        vertCount < 0 || vertCount > 20000 ||
                        nCount   < 0 || nCount   > 20000);
            if (bad) {
                static bool s_loggedInvMesh = false;
                if (!s_loggedInvMesh) {
                    s_loggedInvMesh = true;
                    char _ib[160];
                    _snprintf_s(_ib, sizeof(_ib), _TRUNCATE,
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
        local_74 = -((int)(char*)&DAT_0584621c + local_64);
        float* pfOut = (float*)((char*)&DAT_05846224 + local_64);
        for (int vi = 0; vi < vertCount; vi++) {
            short* vs = (short*)((char*)(vi * 0x10) + *(int*)(meshPtr + 0x10));
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
        // BUG-FIX 2026-07-15: son DOS buffers separados —
        //   • normal transformada → NORMAL buffer (DAT_06f433bc + local_64,
        //     stride 3 floats), que lee el chrome env-map en BMD_DrawMesh.
        //   • intensidad de luz (dot con la dir de luz) → LIGHT buffer
        //     (DAT_077e298c, stride 1 float).
        // El port anterior escribía la normal al LIGHT buffer (local_68) y la
        // sobreescribía con la luz → la luz salía bien pero el NORMAL buffer
        // quedaba en cero → todos los UV chrome colapsaban a un texel → armas
        // y sets con glow chrome se veían como barra/relleno sólido dorado.
        // Ahora `v20 = &unk_6F433BC + v9` (normal) y `v35 = v37` (light) son
        // punteros independientes, fiel a IDA.
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
            char b[200];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "BMD_Xform #%d  mesh0_vert0=(%.1f,%.1f,%.1f)  vertCount=%d",
                diag_this_call, pfFirst[-2], pfFirst[-1], pfFirst[0], vertCount);
            DbgLogPublic(b);
        }
        local_6c += 0x28;
        // FIX: avanzar la BASE del mesh 15000 floats (60000 bytes)
        // ABSOLUTAMENTE — equivale a `v37 += 15000` en IDA line 206.
        local_68_base += 15000;
        local_64 += 180000;
    }
    if (diagInGame && s_xform_dbg < 200) s_xform_dbg++;
    // output AABB
    if (DAT_07e11d30 == 2) {
        param_4[0]=local_54[0]; param_4[1]=local_54[1]; param_4[2]=local_54[2];
        param_4[3]=local_54[3]-local_54[0]; param_4[7]=local_54[4]-local_54[1];
        float szZ = local_54[5]-local_54[2];
        param_4[0xb] = szZ;
    } else {
        param_4[0]=*param_2;   param_4[1]=param_2[1]; param_4[2]=param_2[2];
        param_4[3]=*param_3-*param_2; param_4[7]=param_3[1]-param_2[1];
        param_4[0xb]=param_3[2]-param_2[2];
    }
    // compute largest dimension for bounding radius
    float dx=local_54[3]-local_54[0], dy=local_54[4]-local_54[1], dz=local_54[5]-local_54[2];
    float maxD = (dy > dx ? dy : dx);
    if (dz > maxD) maxD = (dx <= dy ? dy : dx);
    *(float*)((char*)this_ + 0xb4) = maxD;
    // apply entity translation offset
    param_4[0] += *(float*)((char*)this_ + 0x6c);
    param_4[1] += *(float*)((char*)this_ + 0x70);
    param_4[2] += *(float*)((char*)this_ + 0x74);
    param_4[4]=param_4[5]=param_4[6]=0.0f;
    param_4[8]=param_4[9]=param_4[10]=0.0f;
}
// BMD_TransformPosition — implemented in src/Math/Math_3D.cpp (Bone_TransformVertex)
// Triangle_ComputeNormal — implemented in src/Math/Math_3D.cpp
// Particle_Spawn — implemented in src/Render/Particle_Spawn.cpp (Particle_Spawn)
// FUN_004795c0 — implemented in src/Render/Particle.cpp (Effect_Spawn, returns int)
// Input_IsKeyJustPressed — implemented in src/Input/Input.cpp
// FUN_00480620 — UIChatLogWindow_AddText — implemented above as UIChatLogWindow_AddText
// FUN_004f8ff0 — implemented in src/Terrain/Terrain_Utils.cpp
// FUN_00529740 — implemented in src/Render/Texture/Texture.cpp (Texture_Load)
// ═════════════════════════════════════════════════════════════════════════════

// CSimpleModulus crypto (FUN_0053cc30/cd20/cca0/ce30 + helpers) moved to
// src/Net/Crypto.cpp (B3 refactor 2026-05-07, 282 lines).

// Chat_ValidateInputCommand — implemented in src/UI/Chat.cpp
// GL_DrawTexture — implemented in src/Render/GL_2D.cpp
// FUN_00511d00 — implemented in src/Render/Sprite.cpp (Sprite_DrawTexturedQuad)

// FUN_005433b0 @ 0x005433b0 — GetTickCount-based time accumulator
// Takes x87 FPU float10 (in_ST0) from the FPU stack, converts to double, passes to
// FUN_00549ae8 (store low DWORD) + FUN_005433cd (return via x87). Returns float10.
// Ghidra shows void; real calling convention returns value on x87 stack.
float10 __cdecl FUN_005433b0(void) { return 0.0L; }

// ── Screen coordinate converters (@ 0x00511950 / 0x00511980) ─────────────────
float __cdecl Screen_ToGLx(float x) { return x; }
float __cdecl Screen_ToGLy(float y) { return y; }

// ── AttackEffect / UseSkillWarrior ────────────────────────────────────────────
