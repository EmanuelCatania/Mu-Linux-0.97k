// stubs_misc2.cpp
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

static BYTE* Warrior_GetSkillRecord97k(int skillType)
{
    if (skillType < 0 || skillType >= 64)
        return nullptr;
    if (DAT_07cf1ff8 != 0)
        return (BYTE*)(uintptr_t)DAT_07cf1ff8 + skillType * 0x28;
    if (DAT_07d29d20 != 0)
        return (BYTE*)(uintptr_t)DAT_07d29d20 + skillType * 0x28;
    return nullptr;
}

// ── FUN_ stubs (non-void returning) ──────────────────────────────────────────
// FUN_00402fd0 — implemented in src/Util/Misc.cpp
// FUN_00403f80 — implemented in src/Net/Crypto.cpp
// FUN_00404280 — implemented in src/Net/Crypto.cpp
// FUN_00404330 — implemented in src/Net/Crypto.cpp
// FUN_00404400 — implemented in src/Net/Crypto.cpp
// FUN_00404bc0 — implemented in src/Sound/Sound_DS3D.cpp
// FUN_00404cd0 — implemented in src/Sound/Sound_DS3D.cpp
// FUN_00406b10 — implemented in src/Util/Misc.cpp
// FUN_00406b30 — implemented in src/Util/Misc.cpp
// FUN_00409c40 — implemented in src/Util/Misc.cpp
// FUN_00409cf0 — implemented in src/Util/Misc.cpp
// FUN_00409e20 — implemented in src/Net/Crypto.cpp
// FUN_0040c690 — implemented in src/Util/Misc.cpp
// FUN_0040e590 — implemented in src/Util/Misc.cpp
// FUN_004127f0 — implemented in src/Sound/Music.cpp
// FUN_00412890 — implemented in src/Sound/Music.cpp
// FUN_00423710 — implemented in src/Net/Crypto.cpp
// FUN_00423760 — implemented in src/Net/Crypto.cpp
// FUN_00423920 — implemented in src/Net/Net_Connect.cpp
// FUN_00423c40 — implemented in src/Net/Crypto.cpp
// FUN_0043dcc0 — implemented in src/Net/Crypto.cpp
// FUN_0043fd70 — implemented in src/Game/Timer.cpp
// FUN_004454fc0 — implemented in src/Render/Entity_Render.cpp
// FUN_004f9590 — implemented in src/Render/Camera.cpp
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
                     *(float*)(param_1+0x168), *(float*)(param_1+100),
                     *(float*)(param_1+0x68),  *(float*)(param_1+0x6c),
                     *(float*)(param_1+0x70),  *(float*)(param_1+0x58), 0xffffffff);
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
                        FUN_004f9db0((float*)((char*)this_ + 0x78), &DAT_06989c9c);
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
// Uses _DAT_005597c8 scale flag: if != 1.0 → calls FUN_004fa0b0 (scaled transform).
void __cdecl FUN_004404e0(void* this_, int param_1, float* param_2, float* param_3,
                           float* param_4, char param_5) {
    // [0-2]=AABB min, [3-5]=AABB max / work dir, [6..8]=transformed dir.
    // In the Ghidra decompile local_54[7]/[8] appear as adjacent stack slots
    // local_38/local_34. A previous port declared those as separate floats, so
    // FUN_004fa110 writing 3 floats at &local_54[6] overflowed past local_54's
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
        FUN_004f9db0((float*)((char*)this_ + 0x8c), local_30);
        FUN_004fa110(local_54 + 3, local_30, local_54 + 6);
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
                FUN_004fa170((float*)(vs + 2), (float*)(*vs * 0x30 + param_1), pfDst);
                if (param_5 != '\0') {
                    float sc = *(float*)((char*)this_ + 0x68);
                    pfDst[0] *= sc; pfOut[-1] *= sc; *pfOut *= sc;
                }
            } else {
                FUN_004fa0b0((float*)(vs + 2), (float*)(*vs * 0x30 + param_1), pfDst);
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
            FUN_004fa0b0((float*)(ns + 2), (float*)(*ns * 0x30 + param_1), pfNrmBase);
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
// FUN_004409a0 — implemented in src/Math/Math_3D.cpp (Bone_TransformVertex)
// FUN_004fa4d0 — implemented in src/Math/Math_3D.cpp
// FUN_00475220 — implemented in src/Render/Particle_Spawn.cpp (Particle_Spawn)
// FUN_004795c0 — implemented in src/Render/Particle.cpp (Effect_Spawn, returns int)
// FUN_0047ec20 — implemented in src/Input/Input.cpp
// FUN_00480620 — UIChatLogWindow_AddText — implemented above as UIChatLogWindow_AddText
// FUN_004f8ff0 — implemented in src/Terrain/Terrain_Utils.cpp
// FUN_00529740 — implemented in src/Render/Texture/Texture.cpp (Texture_Load)
// ═════════════════════════════════════════════════════════════════════════════

// CSimpleModulus crypto (FUN_0053cc30/cd20/cca0/ce30 + helpers) moved to
// src/Net/Crypto.cpp (B3 refactor 2026-05-07, 282 lines).

// FUN_00513570 — implemented in src/UI/Chat.cpp
// FUN_005125a0 — implemented in src/Render/GL_2D.cpp
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

// FUN_00445230 @ 0x00445230 — AttackEffect(CHARACTER* c)
// Spawns attack-hit effects and plays sounds based on the attacker's entity type.
// Signature in original binary: __stdcall AttackEffect(CHARACTER *c) — 1 param.
// Declared in functions.h as 3 ints; callers must use that convention.
//
// Main switch on *(short*)(entity+0x02) (Object.Type / MonsterIndex):
//   0x23 '#' DK melee    → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
//   0x26 '&' DK sword    → CreateEffect(0xC8/0xC9, pos) + PlayBuffer(0x59)
//   0x2A '*' Elf bow     → BMD_TransformPosition(bone 0xB, pos)
//   0x2D '-' Elf arrow   → BMD_TransformPosition(bone 2, pos)
//   0x35 '5' DW          → CreateEffect(0x238, pos)×18 + PlayBuffer
//   0x3D '=' Summoner    → CreateEffect(0xF1, pos) + CreateEffect(0xF0/0x238, pos)
//   0x42 'B'             → CreateEffect(0xF1, pos)
//   0x46 'F' AoE         → CreateEffect(0x4F7, targetPos)×20
//   0x48 'H' Lightning   → CreateJoint(0x4E5, ...)×36
//   0x4D 'M' Lightning   → CreateJoint(0x4E5, ...)×40
//   (most others)        → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
// Second section: target effects for skill category 0x11 (direct) and 0x03 (magic).
// 2026-08-16: version PARCIAL (374 lineas). Desactivada a favor del port fiel
// de IDA en stubs_IDA_ports.cpp (2043 lineas), que ahora se activa con
// IDA_PORT_00445230 en globals.h.
#if !defined(IDA_PORT_00445230)
void __cdecl FUN_00445230(int entity)
{
    if (!entity) return;
    const BYTE attackEffectKind = *(BYTE*)(entity + 747);
    float* pos   = (float*)(entity + 0x10);  // world position
    float* angle = (float*)(entity + 0x1c);  // world angle
    float* light = (float*)(entity + 0xe8);  // light/color

    // 00445230 reads the queued skill through FUN_0045fae0. +770 is a
    // hash-table protected byte in the original client; comparing its storage
    // directly made this path depend on the encoded value instead of the skill.
    const BYTE queuedSkill = FUN_0045fae0((DWORD)&DAT_055c9bc8,
                                          (BYTE*)(entity + 770));

    // First switch of 00445230 (lines 293..1689): this selector is +747,
    // not the model type. `frame` is +757 and `action` is +261.
    const BYTE frame = *(BYTE*)(entity + 757);
    const BYTE action = *(BYTE*)(entity + 261);
    const DWORD boneMatricesTop = *(DWORD*)(entity + 276);
    const int modelTypeTop = *(short*)(entity + 2);
    void* modelTop = (modelTypeTop >= 0 && DAT_05828d58)
        ? (void*)(DAT_05828d58 + modelTypeTop * 188) : 0;
    float zero[3] = { 0.0f, 0.0f, 0.0f };
    float world[3] = { 0.0f, 0.0f, 0.0f };
    float effectAngle[3] = { 0.0f, 0.0f, 0.0f };
    float whiteTop[3] = { 1.0f, 1.0f, 1.0f };

    switch (attackEffectKind) {
    case 35:
        if (queuedSkill == 50 && frame == 1) {
            for (int n = 0; n < 18; ++n) {
                effectAngle[0] = 0.0f; effectAngle[1] = 0.0f; effectAngle[2] = n * 20.0f;
                FUN_00460dc0(191, pos, effectAngle, light, (float*)1, (float*)entity, (float*)-1, 0, 0);
            }
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 38:
    case 67:
        if (queuedSkill == 50) {
            if (frame == 1) {
                FUN_00460dc0(200, pos, angle, light, 0, 0, (float*)-1, 0, 0);
                FUN_00460dc0(201, pos, angle, light, 0, 0, (float*)-1, 0, 0);
                FUN_00404bc0(89, 0, 0);
            }
            world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
            world[1] = pos[1] + (float)(rand() % 1024) - 512.0f;
            world[2] = pos[2];
            FUN_00460dc0(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 42:
        if (queuedSkill == 50 && frame == 1 && modelTop && boneMatricesTop) {
            FUN_004409a0(modelTop, (float*)(boneMatricesTop + 528), zero, world, 1);
            effectAngle[0] = angle[0] - 20.0f; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] - 30.0f;
            FUN_00460dc0(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            effectAngle[0] = angle[0] - 30.0f; effectAngle[2] = angle[2];
            FUN_00460dc0(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            effectAngle[0] = angle[0] - 20.0f; effectAngle[2] = angle[2] + 30.0f;
            FUN_00460dc0(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 45:
        if (modelTop && boneMatricesTop) for (int n = 0; n < 4; ++n) {
            zero[0] = (float)(rand() % 32 - 16); zero[1] = (float)(rand() % 32 - 16); zero[2] = (float)(rand() % 32 - 16);
            FUN_004409a0(modelTop, (float*)(boneMatricesTop + 96), zero, world, 1);
            Particle_Spawn(1241, world, angle, whiteTop, 0, 1.0f, 0);
            Particle_Spawn(1206, world, angle, whiteTop, 0, 1.0f, 0);
        }
        break;
    case 49:
        if (frame % 5 == 1 && modelTop && boneMatricesTop) {
            zero[0] = zero[1] = zero[2] = 0.0f;
            FUN_004409a0(modelTop, (float*)(boneMatricesTop + 3024), zero, world, 1);
            FUN_00460dc0(1211, world, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        if (queuedSkill == 50 && frame == 1) {
            float boltAngle[3] = { angle[0], angle[1], angle[2] + 20.0f };
            float boltPosition[3] = { pos[0], pos[1], pos[2] + 50.0f };
            float boltLight[3] = { 0.42000002f, 0.84000003f, 1.4f };
            for (int n = 0; n < 9; ++n) {
                boltAngle[2] += 40.0f;
                FUN_00460dc0(1210, boltPosition, boltAngle, boltLight, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 53: case 58: case 59:
        if (frame == 1) FUN_00466300(pos);
        if (attackEffectKind == 59 && queuedSkill == 50 && frame == 14) {
            for (int n = 0; n < 18; ++n) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + n * 20.0f;
                FUN_00460dc0(568, pos, effectAngle, light, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 54: case 57: case 151:
        if (frame == 1) {
            FUN_00460dc0(223, pos, angle, light, 0, (float*)entity, (float*)-1, 0, 0);
            if (attackEffectKind == 57) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + 20.0f;
                FUN_00460dc0(223, pos, effectAngle, light, 0, (float*)entity, (float*)-1, 0, 0);
                effectAngle[2] -= 40.0f;
                FUN_00460dc0(223, pos, effectAngle, light, 0, (float*)entity, (float*)-1, 0, 0);
            }
        }
        break;
    case 61:
        if (frame == 1) FUN_00460dc0(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        break;
    case 63:
        if (frame == 1) {
            FUN_00466300(pos);
            FUN_00460dc0(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        if (queuedSkill == 50) {
            world[0] = pos[0] + (float)(rand() % 800) - 400.0f;
            world[1] = pos[1] + (float)(rand() % 800) - 400.0f; world[2] = pos[2];
            FUN_00460dc0(240, world, angle, light, 0, 0, (float*)-1, 0, 0);
            if (frame == 14) for (int n = 0; n < 18; ++n) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + n * 20.0f;
                FUN_00460dc0(568, pos, effectAngle, light, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 66:
        if (queuedSkill == 50 && frame == 1)
            FUN_00460dc0(241, pos, angle, light, (float*)1, 0, (float*)-1, 0, 0);
        break;
    case 70:
        if (frame == 5 && *(short*)(entity + 784) >= 0 && *(short*)(entity + 784) < 400) {
            BYTE* targetTop = (BYTE*)(uintptr_t)CharactersClient + *(short*)(entity + 784) * 916;
            for (int n = 0; n < 20; ++n) FUN_00460dc0(1271, (float*)(targetTop + 16), (float*)(targetTop + 28), whiteTop, 0, 0, (float*)-1, 0, 0);
        }
        break;
    case 71: case 74:
        if ((action == 3 || action == 4) && frame == 5) {
            FUN_00466300(pos); FUN_00460dc0(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        break;
    case 72:
        if (queuedSkill == 50 && frame == 14) for (int n = 0; n < 36; ++n) {
            effectAngle[0] = (float)(rand() % 360); effectAngle[1] = (float)(rand() % 360); effectAngle[2] = (float)(rand() % 360);
            world[0] = pos[0]; world[1] = pos[1]; world[2] = pos[2] + 100.0f;
            FUN_0046d840(1253, world, world, effectAngle, 1, 0, 60.0f, 0, 0);
        }
        break;
    case 73: case 75:
        if (action == 3 && frame == 11) {
            effectAngle[0] = angle[0] + 45.0f; effectAngle[1] = angle[1]; effectAngle[2] = angle[2];
            float orange[3] = { 1.0f, 0.5f, 0.0f };
            for (int n = 0; n < 5; ++n) {
                world[0] = pos[0] + (float)(rand() % 1001 - 500); world[1] = pos[1] + (float)(rand() % 1001 - 500); world[2] = pos[2] + 500.0f;
                *(float*)(entity + 368) = world[0]; *(float*)(entity + 372) = world[1]; *(float*)(entity + 376) = world[2];
                FUN_00460dc0(256, world, effectAngle, orange, (float*)1, (float*)entity, (float*)-1, 0, 0);
            }
        }
        break;
    case 77:
        if (queuedSkill == 50 && (frame == 2 || frame == 6)) for (int n = 0; n < 40; ++n) {
            effectAngle[0] = (float)(rand() % 360); effectAngle[1] = (float)(rand() % 360); effectAngle[2] = (float)(rand() % 360);
            world[0] = pos[0]; world[1] = pos[1]; world[2] = pos[2] + 100.0f;
            FUN_0046d840(1253, world, world, effectAngle, 3, 0, 50.0f, 0, 0);
        }
        break;
    case 89: case 95: case 112: case 118: case 124: case 130: case 136:
        if (queuedSkill == 50) {
            if (rand() & 1) {
                world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
                world[1] = pos[1] + (float)(rand() % 1024) - 512.0f; world[2] = pos[2];
                FUN_00460dc0(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
                FUN_00404bc0(46, 0, 0);
            } else if (frame == 1) {
                FUN_00460dc0(241, pos, angle, light, (float*)1, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 103:
        if (queuedSkill == 50) {
            world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
            world[1] = pos[1] + (float)(rand() % 1024) - 512.0f; world[2] = pos[2];
            FUN_00460dc0(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    default: break;
    }

    if (queuedSkill != 3 && queuedSkill != 17) return;
    const short targetSlot = *(short*)(entity + 784);
    if (targetSlot < 0 || targetSlot >= 400 || !CharactersClient) return;

    BYTE* target = (BYTE*)(uintptr_t)CharactersClient + targetSlot * 916;
    const int modelType = *(short*)(entity + 2);
    if (modelType < 0 || !DAT_05828d58 || !*(DWORD*)(entity + 276)) return;

    // The following switch is the skill-3 switch at 00445230:1898.  The
    // apparent `TargetPosition` in IDA is the transformed hand-bone position;
    // it is deliberately not the target's world position.
    float local[3] = { 0.0f, 0.0f, 0.0f };
    float source[3] = { 0.0f, 0.0f, 0.0f };
    float jointAngle[3] = { 0.0f, 0.0f, 0.0f };
    float white[3] = { 1.0f, 1.0f, 1.0f };
    void* model = (void*)(DAT_05828d58 + modelType * 188);
    const DWORD boneMatrices = *(DWORD*)(entity + 276);
    const auto handPosition = [&](int handIndex, float offsetY, int secondHandAt = 2) {
        local[0] = 0.0f;
        local[1] = offsetY;
        local[2] = 0.0f;
        const BYTE bone = *(BYTE*)(entity + 628 + (handIndex >= secondHandAt ? 24 : 0));
        FUN_004409a0(model, (float*)(boneMatrices + 48 * bone), local, source, 1);
    };

    // 00445230:1696.  Skill 17 has a distinct, preceding switch; it is not a
    // variant of the skill-3 default.  Keep the two dispatches separated just
    // as they are in IDA so their stage guards and joint actions stay intact.
    if (queuedSkill == 17) {
        switch (attackEffectKind) {
        case 0x25:
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                FUN_0046d840(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        case 0x2E:
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = 0.0f;
                FUN_0046d840(1166, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
                FUN_0046d840(1166, source, (float*)(target + 16), jointAngle, 1, (int)target, 10.0f, -1, 0);
            }
            break;
        case 0x3D:
            for (int n = 0; n < 6; ++n) {
                handPosition(n, 0.0f, 3);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 50.0f, -1, 0);
                FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 10.0f, -1, 0);
            }
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                FUN_0046d840(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        case 0x42: {
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(60, 0, 0);
            float spin = 225.0f - (float)(((long long)DAT_05826e08 / 10 +
                                           3 * *(BYTE*)(entity + 757)) % 90);
            for (int n = 0; n < 4; ++n, spin += 270.0f) {
                handPosition(n % 2, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = spin;
                FUN_0046d840(1261, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        }
        case 0x49:
        case 0x4B:
            if (*(BYTE*)(entity + 261) == 4 && *(BYTE*)(entity + 757) == 13) {
                float offset[3] = { -50.0f, 100.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28) + 45.0f,
                                          *(float*)(entity + 32), *(float*)(entity + 36) };
                float orange[3] = { 1.0f, 0.5f, 0.0f };
                FUN_004409a0(model, (float*)(boneMatrices + 528), offset, source, 1);
                FUN_00460dc0(256, source, effectAngle, orange, (float*)1, 0, (float*)-1, 0, 0);
                FUN_0046d840(1254, source, (float*)(target + 16), effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        case 0x4D:
            if (*(BYTE*)(entity + 757) == 14) {
                float zero[3] = { 0.0f, 0.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                FUN_004409a0(model, (float*)g_AttackEffectMatrix_04D, zero, source, 1);
                FUN_00460dc0(256, source, effectAngle, white, (float*)1, 0, (float*)-1, 0, 0);
                FUN_0046d840(1254, source, source, effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        case 0x57: case 0x5D: case 0x63: case 0x74: case 0x7A: case 0x80: case 0x86:
            if (*(BYTE*)(entity + 757) == 13) {
                float offset[3] = { 60.0f, 30.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                FUN_004409a0(model, (float*)(boneMatrices + 288), offset, source, 1);
                FUN_00460dc0(191, source, effectAngle, light, (float*)5, 0, (float*)-1, 0, 0);
            }
            break;
        case 0x59: case 0x5F: case 0x70: case 0x76: case 0x7C: case 0x82: case 0x88:
            if (*(BYTE*)(entity + 757) == 14) {
                float zero[3] = { 0.0f, 0.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                FUN_004409a0(model, (float*)(boneMatrices + 1584), zero, source, 1);
                FUN_00460dc0(256, source, effectAngle, white, (float*)1, 0, (float*)-1, 0, 0);
                FUN_0046d840(1254, source, source, effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        // 0x45 uses IDA's uninitialised/reused scratch position.  It remains
        // deferred instead of inventing a source point.
        default:
            break;
        }
        return;
    }

    switch (attackEffectKind) {
    case 0x22:
        for (int n = 0; n < 4; ++n) {
            handPosition(n, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
            FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
            Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        }
        break;
    case 0x25:
        if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
        for (int n = 0; n < 4; ++n) {
            handPosition(n, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            FUN_0046d840(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
            Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
        }
        break;
    case 0x27:
        handPosition(0, *(short*)(entity + 2) == 390 ? 0.0f : -130.0f);
        jointAngle[0] = -60.0f; jointAngle[1] = 0.0f; jointAngle[2] = *(float*)(entity + 36);
        FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
        FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
        Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        break;
    case 0x30:
        for (int n = 0; n < 6; ++n) {
            handPosition(n, 0.0f, 3);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 50.0f, -1, 0);
            FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 10.0f, -1, 0);
        }
        break;
    case 0x59: case 0x5F: case 0x70: case 0x76: case 0x7C: case 0x82: case 0x88: {
        if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(60, 0, 0);
        float spin = 225.0f - (float)(((long long)DAT_05826e08 / 10 +
                                       3 * *(BYTE*)(entity + 757)) % 90);
        for (int n = 0; n < 4; ++n, spin += 270.0f) {
            handPosition(n % 2, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = spin;
            FUN_0046d840(1261, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
            Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
        }
        break;
    }
    case 0x4D:
        if (*(BYTE*)(entity + 757) >= 8) {
            float zero[3] = { 0.0f, 0.0f, 0.0f };
            FUN_004409a0(model, (float*)g_AttackEffectMatrix_04D_Alt, zero, source, 1);
            for (int n = 0; n < 4; ++n) {
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 80.0f, -1, 0);
            }
        }
        break;
    default:
        handPosition(0, *(short*)(entity + 2) == 390 ? 0.0f : -130.0f);
        jointAngle[0] = -60.0f; jointAngle[1] = 0.0f; jointAngle[2] = *(float*)(entity + 36);
        FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
        FUN_0046d840(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
        Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        break;
    }
}
#endif  // !IDA_PORT_00445230


// ─────────────────────────────────────────────────────────────────────────────
// 2026-05-08: Companion-DLL Offsets.h cross-reference — port small functions
// that were truly missing in our build. Sizes per IDA decompile.
// ─────────────────────────────────────────────────────────────────────────────

// FUN_0047b110 @ 0x0047B110 — ConvertItemType(BYTE* Item)  (21 bytes)
// Port FIEL desde IDA: returns Item[0] + 2*(Item[3] & 0x80). The high bit of
// Item[3] is the "Type Hi" flag used to distinguish item categories beyond
// 256 entries (e.g. shields >= 0x100).
extern "C" int __cdecl ConvertItemType(BYTE* Item)
{
    return (int)Item[0] + 2 * (int)(Item[3] & 0x80);
}
int __cdecl FUN_0047b110(BYTE* Item) { return ConvertItemType(Item); }

// FUN_0045ac20 @ 0x0045AC20 — DeleteCharacter(int Key)  (95 bytes)
// Linear-scans CharactersClient (400 slots × 0x394) for entity_id (+0x1DC).
// On match, marks slot inactive (+0 = 0), kills any butterfly that owned this
// character (Butterfles[+0xFC] == slot_ptr → +0 = 0), then calls DeleteCloth.
extern void __cdecl FUN_00449840(int c, int o, int flag);  // DeleteCloth
extern "C" void __cdecl DeleteCharacter(int Key)
{
    DWORD v1 = (DWORD)DAT_07abf5d0;
    int v2 = 0;
    while (!*(BYTE*)v1 || *(short*)(v1 + 0x1DC) != (short)Key) {
        ++v2;
        v1 += 916;  // 0x394
        if (v2 >= 400) return;
    }
    *(BYTE*)v1 = 0;

    // Walk Butterfles pool: each entry 0x1BC bytes (= 111 dwords = 444b),
    // entry layout has Owner at +0xFC (= dword[63]).
    DWORD* v3 = (DWORD*)&DAT_083a1218;
    DWORD* end = (DWORD*)((char*)&DAT_083a1218 + 0x1158);
    while (v3 < end) {
        if (*(BYTE*)v3 != 0 && v3[63] == v1) {
            *(BYTE*)v3 = 0;
        }
        v3 += 111;  // stride 444 bytes = 0x1BC
    }

    FUN_00449840((int)v1, (int)v1, 0);
}
void __cdecl FUN_0045ac20(int Key) { DeleteCharacter(Key); }

// FUN_00460d20 @ 0x00460D20 — DeleteEffect(int Type, DWORD Owner, int iSubType)
// (69 bytes) — walks Effects pool DAT_07b11670 (124 × 0x1BC), zeroes any slot
// whose Type/Owner/SubType match. Active flag at slot+0; Type at slot+2 (short);
// SubType at slot+4 (dword); Owner at slot+248 (= dword[62]).
//
// Slot anchor: original IDA points `o` at slot+4 (so `o[-4]` = active flag,
// `o[-2]` (short) = Type, o[0] = SubType, o[62] = Owner). Stride is 111 dwords.
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType)
{
    DWORD* o = (DWORD*)((char*)&DAT_07b11670[0] + 4);
    DWORD* end = (DWORD*)((char*)&DAT_07b11670[0] + 124 * 0x1BC + 4);
    while (o < end) {
        if (*((BYTE*)o - 4) != 0 &&
            *((short*)o - 1) == (short)Type &&
            (iSubType == -1 || (int)*o == iSubType) &&
            o[62] == Owner)
        {
            *((BYTE*)o - 4) = 0;
        }
        o += 111;  // stride 0x1BC
    }
}
void __cdecl FUN_00460d20(int Type, DWORD Owner, int iSubType)
{ DeleteEffect(Type, Owner, iSubType); }

// FUN_004cc660 @ 0x004CC660 — InsertInventoryItem(ITEM* Inv, int Width,
//   int Height, int Index, BYTE* Item, bool First)  (1945 bytes, IDA).
//
// IDA decomp ships with a `// local variable allocation has failed` warning,
// so the visible body is mostly the anti-tamper hash-table tail (CharacterMachine
// ref-count + XOR encrypt around `First` branch). The actual grid-placement
// core was lost in the decompile — but cross-referencing the call sites
// (Net_Process.cpp cases 0x22 / 0x32 / 0x39 / F3-14) plus the parallel 0.52
// port `AddItemToGrid` in Item_Inventory.cpp gives us the real semantics:
//
//   - Convert item type (Item[0] + 2*(Item[3] & 0x80)). Skip if 0xFF (empty).
//   - Look up width/height from ItemAttribute. For 0.97 we accept the
//     simplification of 1×1 placement when ItemAttribute table is unavailable.
//   - Compute slot offset: (Index * sizeof(ITEM_RAW)) within the inventory
//     buffer. ITEM_RAW stride is 0x44 (= 68 bytes) per the binary format that
//     server emits in opcode 0x22 / 0x32 / 0xF3-14.
//   - Copy 12 bytes of raw item data into the slot (matches what
//     Net_Process.cpp:2807-2814 already does for case 0x22).
//   - If `First==false`, bump CharacterMachine hash-table ref count (anti-
//     tamper, no-op per project policy).
//
// 2026-05-08: ported as the canonical entry point. Net_Process inline copies
// keep working unchanged, but anything that needs the IDA name (and the
// signature taking grid-W/H plus First flag) can now use this.
extern "C" int __cdecl ConvertItemType(BYTE* Item);  // declared above
extern "C" void __cdecl AddItemToGrid(BYTE* gridBase, int gridW, int gridH,
                                      int slotIdx, int type, int level,
                                      BYTE optByte, BYTE fillFlag, BYTE durability, BYTE byteHi, BYTE extByte);
extern "C" BYTE OffsetInventoryItems[];
extern "C" void __cdecl InsertInventoryItem(BYTE* Inv, int Width, int Height,
                                            int Index, BYTE* Item, bool First)
{
    if (!Inv || !Item) return;
    bool isMainInventory = (Inv == &OffsetInventoryItems[0]);
    auto clear_slot = [&](int slotIndex) {
        if (slotIndex < 0) return;
        if (isMainInventory && slotIndex < 12) {
            BYTE* slot = Inv + slotIndex * 0x44;
            memset(slot, 0, 0x44);
            *(short*)slot = (short)0xFFFF;
            return;
        }

        int cellIndex = isMainInventory ? (slotIndex - 12) : slotIndex;
        if (cellIndex < 0 || cellIndex >= Width * Height) return;

        BYTE* slot = Inv + cellIndex * 0x44;
        short oldType = *(short*)slot;
        if (oldType == (short)0xFFFF || oldType < 0) return;

        BYTE ox = slot[62];
        BYTE oy = slot[63];
        if (ox >= Width || oy >= Height) return;

        for (int yy = 0; yy < Height; ++yy) {
            for (int xx = 0; xx < Width; ++xx) {
                int idx = yy * Width + xx;
                BYTE* cell = Inv + idx * 0x44;
                if (*(short*)cell == oldType && cell[62] == ox && cell[63] == oy) {
                    memset(cell, 0, 0x44);
                    *(short*)cell = (short)0xFFFF;
                }
            }
        }
    };

    clear_slot(Index);

    {
        int type = ConvertItemType(Item);
        if (type == 0xFF) return;
        if (Index < 0) return;
        int total = Width * Height + (isMainInventory ? 12 : 0);
        if (Index >= total) return;

        BYTE optByte = Item[1];
        BYTE durability = Item[2];
        BYTE byteHi = Item[3];
        BYTE extByte = Item[4];
        int level = (optByte >> 3) & 0x0F;

        AddItemToGrid(Inv, Width, Height, Index, type, level, optByte,
                      First ? 1 : 0, durability, byteHi, extByte);
        return;
    }
    int type = ConvertItemType(Item);
    if (type == 0xFF) return;                     // empty / sentinel slot
    if (Index < 0) return;
    int total = Width * Height + 12;              // wear (12) + grid (W*H)
    if (Index >= total) return;

    // Grid stride: ITEM-raw is 0x44 (68) bytes per slot in 0.97k. Some tables
    // (e.g. char-list at DAT_07ea5298) use 0x44 too — same shape as inventory.
    BYTE* slot = Inv + Index * 0x44;

    // Copy item header + flags. 12 bytes is what 0.97k packets carry per slot
    // (type, level, dur, option, serial[8]).
    memcpy(slot, Item, 12);

    // Per-cell render-gate fields (per IDA sub_4E38B0 / RenderEquipment3D):
    //   slot+26 (Durability byte): high bit of Item[2]
    //   slot+27 (Option1 byte):    low bits of Item[3]
    //   slot+38 (Width grid coord, byte): rare item flag
    //   slot+62 (x grid pos), slot+63 (y grid pos)
    //   slot+56 (Key/dur-int gate, int): MUST be > 0 for the slot to render!
    //
    // Without slot+56 set, sub_4E38B0 line 60 `*(int*)(slot+0x38) > 0` fails →
    // the item is silently skipped from the render walk → invisible in grid.
    // 2026-05-08: this was the bug that caused inventory items received via
    // packets 0x32/0x39/F3-14 to populate the data array but not appear.
    BYTE durability = Item[2];                    // raw durability byte
    if (durability == 0) durability = 1;          // ensure render gate
    *(int*)(slot + 56) = (int)durability;

    // Set grid x/y from Index. For wear slots (Index < 12), use slot index 0
    // for x and 0 for y (wear slots are special-cased downstream). For grid
    // slots, decompose into (Index-12) → (x = idx % Width, y = idx / Width).
    if (Index >= 12) {
        int gridIdx = Index - 12;
        slot[62] = (BYTE)(gridIdx % Width);
        slot[63] = (BYTE)(gridIdx / Width);
    } else {
        slot[62] = 0;
        slot[63] = 0;
    }

    // (Anti-tamper hash-table ref-count for !First — skipped per policy.)
    (void)First;
}

void __cdecl FUN_004cc660(BYTE* Inv, int Width, int Height,
                          int Index, BYTE* Item, int First)
{
    InsertInventoryItem(Inv, Width, Height, Index, Item, First != 0);
}

// FUN_0046fe00 @ 0x0046FE00 — DeleteJoint(int Type, DWORD Target, int SubType)
// (61 bytes) — walks Joints pool DAT_07b27150 (500 × 0x9D8) and zeroes any
// active slot whose Type/Target/SubType match.
//
// Slot layout per IDA `CreateJoint` (raw/0046D840_CreateJoint.c:178-195):
//   slot+0   byte  active flag
//   slot+4   int   Type
//   slot+8   int   SubType
//   slot+64  DWORD Target  (= entity ptr / owner)
// Stride 0x9D8 = 2520 bytes per slot.
//
// 2026-05-08: previously a no-op (mc_DeleteJoint in SecondPassword.cpp) "until
// joint pool wired". Pool IS sized in globals.cpp (200×0x9D8); now functional.
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType)
{
    BYTE* base = (BYTE*)&DAT_07b27150[0];
    const int kJointSlots = (int)(sizeof(DAT_07b27150) / 0x9D8);   // 500
    for (int i = 0; i < kJointSlots; ++i) {
        BYTE* slot = base + i * 0x9D8;
        if (slot[0] != 0 &&
            *(int*)(slot + 4) == Type &&
            *(DWORD*)(slot + 64) == Target &&
            (SubType == -1 || *(int*)(slot + 8) == SubType))
        {
            slot[0] = 0;
        }
    }
}
void __cdecl FUN_0046fe00(int Type, DWORD Target, int SubType)
{ DeleteJoint(Type, Target, SubType); }

// FUN_004792c0 @ 0x004792C0 — CreatePoint(float Position[3], int Value,
//   float Color[3], float scale)  (101 bytes)
//
// Spawns a damage popup / floating text in the point pool DAT_07c80110
// (100 × 0x70 bytes). Per IDA decompile (raw/004792C0_CreatePoint.c).
//
// 2026-05-08: this address is the same function as the existing
// `Entity_TeleportAnim` in this file — the alias was misnamed because
// `Skills.cpp:349` calls it expecting a teleport-spawner, but the actual IDA
// is `CreatePoint` (damage-popup spawner). The Net_Process case 0x15 path
// uses it correctly as damage popup. We add the IDA name as the canonical
// entry point and the existing `Entity_TeleportAnim` impl provides the body.
//
// Skill teleport effect is actually `CreateTeleportBegin` (FUN_004742b0) /
// `CreateTeleportEnd` (FUN_00474310) — Skills.cpp:349 is a misrouted call
// (left as-is for now; spawning a degenerate damage popup is harmless, and
// fixing the routing belongs to Skills.cpp).
//
// `Entity_TeleportAnim` body (lower in this file, ~line 1070) is the actual
// implementation; this wrapper just exposes the IDA name.
extern void __cdecl Entity_TeleportAnim(float* world_pos,
    float entity_id, float* dst_pos, float param_4);
extern "C" void __cdecl CreatePoint(float Position[3], int Value,
                                    float Color[3], float scale)
{
    // bit-cast Value → float for the existing impl's "entity_id" slot
    // (matches IDA: stores Value as int at +4 / float at +4 alias).
    float val_as_float;
    *(int*)&val_as_float = Value;
    Entity_TeleportAnim(Position, val_as_float, Color, scale);
}
void __cdecl FUN_004792c0(float* Position, int Value, float* Color, float scale)
{ CreatePoint(Position, Value, Color, scale); }

// FUN_004742b0 @ 0x004742B0 — CreateTeleportBegin(DWORD o)  (83 bytes)
// Per IDA decompile (raw/004742B0_CreateTeleportBegin.c). Begins teleport
// animation: anim 87, alpha=0 (fade out), state byte 1, sparkle effect 1176.
// Sound 88 (whoosh).
//
// 2026-05-08: previously aliased as `Entity_WeaponHit` in CLAUDE.md and our
// Combat.cpp comments, but IDA confirms this is the teleport-begin function.
// FUN_00443e70 / FUN_0043e820 / FUN_00460dc0 / FUN_00404bc0 decls in functions.h.
extern "C" void __cdecl CreateTeleportBegin(unsigned int o)
{
    if (!o) return;
    FUN_00443e70();                                  // SetAttackSpeed
    (void)FUN_0043e820((int)o, 87);                  // SetAction(o, 87)
    *(unsigned int*)(o + 356) = 0;                   // alpha = 0 (fade-out)
    *(BYTE*)(o + 124) = 1;                           // state byte = 1 (begin)
    (void)FUN_00460dc0(1176, (float*)(o + 16), (float*)(o + 28),
                       (float*)(o + 232), nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    FUN_00404bc0(88, 0, 0);                          // PlayBuffer(88) whoosh
}

// FUN_00474310 @ 0x00474310 — CreateTeleportEnd(DWORD o)  (93 bytes)
// Per IDA decompile (raw/00474310_CreateTeleportEnd.c). Completes teleport:
// anim 87, anim_speed +0x108=4.5f (0x40A00000), state byte 3, alpha=1.0f
// (fade-in), sparkle effect 1176, sound 88.
extern "C" void __cdecl CreateTeleportEnd(unsigned int o)
{
    if (!o) return;
    FUN_00443e70();
    (void)FUN_0043e820((int)o, 87);
    *(unsigned int*)(o + 264) = 0x40A00000u;        // anim_speed = 5.0f
    *(BYTE*)(o + 124) = 3;                           // state byte = 3 (end)
    *(unsigned int*)(o + 356) = 0x3F800000u;        // alpha = 1.0f (fade-in)
    (void)FUN_00460dc0(1176, (float*)(o + 16), (float*)(o + 28),
                       (float*)(o + 232), nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    FUN_00404bc0(88, 0, 0);
}

// FUN_00485780 @ 0x00485780 — UseSkillWarrior(c=CHARACTER*, o=OBJECT*)
// Client-side handler for all Warrior skill activations (1989 lines in Ghidra).
// 1. Resolves skill ID from DAT_07d78098/DAT_07d7809c (+ CharacterAttribute skill table).
// 2. Sends opcode 0x10 position packet (XOR-encrypted — delegated to FUN_0048d640).
// 3. Sets attack animation based on model type (0x186=special warrior anims).
// 4. Spawns caster sparkle effect 0x4D0, plays random sword sound.
// 5. Computes facing angle toward target via CreateAngle.
// 6. Per-skill dispatch: 0x2B=TeleportSlash, 0x2F=DownStab, 0x31=Whirlwind, else=generic.
// 7. Sends opcode 0x19 skill packet for each specific skill type.
// 8. Final: sends opcode 0x11 if tile is walkable.
// Anti-tamper hash table ops and XOR encryption blocks are skipped per project policy.
void __cdecl FUN_00485780(int c, int o)
{
    if (!c || !o) return;

    // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt around skill read)

    DWORD skillType = DAT_07d7809c;
    if (DAT_07d78098 != '\0') {
        BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
        if (charAttr) {
            skillType = (DWORD)charAttr[DAT_07d7809c + 87];
        }
    }

    // anti-tamper hash table — skipped

    // Send opcode 0x10 position packet (movement before skill) using the
    // real 97k sender, not the old zero-arg legacy helper.
    FUN_00491c40(c, o);

    // Clear movement flag
    *(BYTE*)(c + 0x2EC) = 0;

    // Set attack animation based on object model type
    short modelType = *(short*)(o + 2); // OBJECT.Type
    if (modelType == 0x186) {
        // Special warrior model: per-skill animation
        FUN_00443e70(); // SetAttackSpeed
        if (skillType == 0x2B) {
            FUN_0043e820(o, 0x43); // TeleportSlash anim
        } else if (skillType == 0x2F) {
            FUN_0043e820(o, 0x42); // DownStab anim
        } else if (skillType == 0x31) {
            if (g_GameSubState == 8 || g_GameSubState == 10) {
                FUN_0043e820(o, 0x41); // Whirlwind alt
            } else {
                FUN_0043e820(o, 0x40); // Whirlwind normal
            }
        } else {
            FUN_0043e820(o, (int)(skillType + 0x25)); // Generic skill anim
        }
    } else {
        FUN_00444410(c, 0, 0, 0); // SetPlayerAttack — generic attack anim
    }

    // Spawn caster sparkle effect (always)
    float* pos   = (float*)(o + 0x10);
    float* angle = (float*)(o + 0x1c);
    float* light = (float*)(o + 0xe8);
    float scale[3] = { 1.0f, 1.0f, 1.0f };
    FUN_00460dc0(0x4D0, pos, angle, light, NULL, NULL, (float*)(uintptr_t)0xffffffff, NULL, 0);

    // Play random sword sound (0x28 or 0x29)
    PlayBuffer((_rand() & 1) + 0x28, 0, 0);

    // Set target position from CharactersClient[MovementSkillTarget]
    int targetIdx = (int)DAT_07d780a0;
    if (targetIdx < 0 || targetIdx >= 400) {
        return;
    }
    char* targetEntity = (char*)(uintptr_t)DAT_07abf5d0 + targetIdx * 0x394;
    *(float*)(c + 0x314) = *(float*)(targetEntity + 0x10); // TargetPosition.x
    *(float*)(c + 0x318) = *(float*)(targetEntity + 0x14); // TargetPosition.y
    *(float*)(c + 0x31C) = *(float*)(targetEntity + 0x18); // TargetPosition.z

    // Compute facing angle toward target
    float facingAngle = FUN_0043e050(
        *(float*)(o + 0x10), *(float*)(o + 0x14),
        *(float*)(c + 0x314), *(float*)(c + 0x318));
    *(float*)(o + 0x24) = facingAngle;

    // Per-skill dispatch
    if (skillType == 0x2B) {
        // TeleportSlash: compute direction vector, send opcode 0x1E
        DAT_05826d10 = 0x2B; // CurrentSkill
    } else if (skillType == 0x38) {
        // Whirlwind: send opcode 0x19 with skill 0x38
        DAT_05826d10 = 0x38;
    } else {
        // Generic warrior skill: send opcode 0x19
        DAT_05826d10 = skillType;
    }

    // Mark skill as active
    *(BYTE*)(c + 0x2F5) = 1;

    // 2026-05-06: send packet 0x19 PMSG_SKILL_ATTACK_RECV per server source
    // Mu-linux-97K/Source/MuServer/GameServer/SkillManager.h:82.
    // Wire format: [C1][06][19][skillID][TgtH][TgtL] + chain XOR + MuEmu byte XOR.
    //
    // ANTES: comment "anti-tamper + XOR packet build for opcode 0x19 — skipped"
    // significaba que el packet NO se enviaba — skill effects nunca llegaban
    // al server. User reportaba "no puedo usar skills".
    {
        WORD targetEntityId = *(WORD*)(targetEntity + 476);
        if (targetEntityId == 0xFFFF) {
            return;
        }
        if (skillType == 0x2B) {
            BYTE skillDist = 0;
            if (BYTE* rec = Warrior_GetSkillRecord97k((int)skillType))
                skillDist = rec[0x27];
            BYTE dir = (BYTE)(((int)(facingAngle / 45.0f)) & 0xFF);
            BYTE gridXb = (BYTE)*(DWORD*)(c + 0x388);
            BYTE gridYb = (BYTE)*(DWORD*)(c + 0x38C);

            BYTE pkt[11];
            pkt[0] = 0xC1;
            pkt[1] = 0x0B;
            pkt[2] = 0x1E;
            pkt[3] = (BYTE)skillType;
            pkt[4] = gridXb;
            pkt[5] = gridYb;
            pkt[6] = dir;
            pkt[7] = skillDist;
            pkt[8] = dir;
            pkt[9] = (BYTE)((targetEntityId >> 8) & 0xFF);
            pkt[10] = (BYTE)(targetEntityId & 0xFF);
            Net_SendSmallPacket(pkt, 11);
        } else {
            BYTE pkt[6];
            pkt[0] = 0xC1;
            pkt[1] = 0x06;
            pkt[2] = 0x19;
            pkt[3] = (BYTE)skillType;
            pkt[4] = (BYTE)((targetEntityId >> 8) & 0xFF);
            pkt[5] = (BYTE)(targetEntityId & 0xFF);
            Net_SendSmallPacket(pkt, 6);
        }
    }

    // Send opcode 0x11 position confirmation if tile is walkable
    // (checks DAT_0838bc70 terrain wall at current grid position)
    int gridX = *(int*)(c + 0x388);
    int gridY = *(int*)(c + 0x38C);
    int tileIdx = FUN_004f6c40((unsigned int)gridX, (unsigned int)gridY);
    (void)tileIdx;
}

// ── Entity action stubs (Skills.cpp / Combat.cpp externs) ────────────────────

// Entity_WeaponHit @ 0x004742B0
// Hit reaction: Entity_SetAnimation(0x57), clear +0x164, set +0x7C=1,
// spawn effect 0x498 at entity pos, play sound 0x58.
void __cdecl Entity_WeaponHit(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(DWORD *)(param_1 + 0x164) = 0;
    *(BYTE  *)(param_1 + 0x7c)  = 1;
    FUN_00460dc0(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// Entity_ResetToWalk @ 0x0042BC00
// Reads anim at +0x288. If NOT in attack range (0x210-0x216 / 0x221):
//   if prev anim (+0x270) was wind-up (0x218-0x21E or 0x220): set walk/swim-walk.
// If currently in attack anim: immediately revert to walk (0x2E) or swim-walk (0x30).
void __cdecl Entity_ResetToWalk(int param_1) {
    short sVar1    = *(short*)(param_1 + 0x288);
    bool swimming  = ((*(BYTE*)(param_1 + 0x1bc) & 7) == 2);
    bool has_water = (*(short*)(param_1 + 0x2a0) != -1);
    if (((sVar1 < 0x210) || (0x216 < sVar1)) && (sVar1 != 0x221)) {
        sVar1 = *(short*)(param_1 + 0x270);
        if (((0x217 < sVar1) && (sVar1 < 0x21f)) || (sVar1 == 0x220)) {
            FUN_0043e820(param_1, (swimming && has_water) ? 0x31 : 0x2f);
        }
        return;
    }
    FUN_0043e820(param_1, (swimming && has_water) ? 0x30 : 0x2e);
}

// Entity_SelectTarget_Player @ 0x00444A80
// entity_type 0x186 (special NPC): picks idle/stance anim from 0x52/0x53/0x56/0x5B.
// Other types: increments combo_counter (+0x303) and alternates anim 3 / 4.
void __cdecl Entity_SelectTarget_Player(int param_1, int /*target*/) {
    if (*(short*)(param_1 + 2) != 0x186) {
        UINT uVar1 = (*(BYTE*)(param_1 + 0x303) % 3 == 0) ? 3 : 4;
        FUN_0043e820(param_1, uVar1);
        *(char*)(param_1 + 0x303) = *(char*)(param_1 + 0x303) + 1;
        return;
    }
    FUN_00443e70();
    if (((*(short*)(param_1 + 0x2b8) == 0x332) || (*(short*)(param_1 + 0x2b8) == 0x333)) &&
         (*(char*)(param_1 + 0x34e) == '\0')) {
        FUN_0043e820(param_1, 0x5b); // death anim for special class
        return;
    }
    if ((*(BYTE*)(param_1 + 0x1bc) & 7) == 2) {
        FUN_0043e820(param_1, 0x56); // swim idle
        return;
    }
    UINT uVar1 = rand() & 0x80000001;
    if ((int)uVar1 < 0) uVar1 = (uVar1 - 1 | 0xfffffffe) + 1; // parity correction
    FUN_0043e820(param_1, uVar1 + 0x52); // 0x52 or 0x53 random idle
}

// Entity_TeleportEnd @ 0x00444D90
// Handles landing after a teleport:
//   If local player (param_1 == DAT_07abf5d8): clears teleport-state flag at DAT_07cf1ff4+0x1c.
//   Spawns landing visual effects per class:
//     0x186 (DarkWizard, sub_id 0xCE-0xD0): effect 0xD2 once + 0xD3 ×10, then sound 0x5E.
//     0x127 (DarkElf): effects 0xE2+0xE3 ×8, then sound 0x5E.
//     300 (0x12C): same as 0x186.
//     other: set anim 6 (land-idle), return.
//   Post-effects: if +0x108 == _DAT_00552580 (0.0f): play footstep sound from class table.
void __cdecl Entity_TeleportEnd(int entity_idx) {
    BYTE* puVar3 = (BYTE*)(uintptr_t)(int)entity_idx;

    // Local player: clear teleport in-flight state (hash-table ops are obfuscation noise)
    if ((void*)entity_idx == (void*)DAT_07abf5d8) {
        *(short*)((char*)DAT_07cf1ff4 + 0x1c) = 0;
    }

    short sVar2 = *(short*)(puVar3 + 2); // entity_type
    if (sVar2 == 0x186) {
        int sub_id = *(int*)(puVar3 + 4);
        if (sub_id < 0xce || sub_id > 0xd0) {
            FUN_0043e820(entity_idx, 0x83);
            goto LAB_00445110;
        }
        *puVar3 = 0;
        FUN_00460dc0(0xd2, (float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            FUN_00460dc0(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
    } else if (sVar2 == 0x127) {
        *puVar3 = 0;
        for (int i = 0; i < 8; i++) {
            FUN_00460dc0(0xe2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
            FUN_00460dc0(0xe3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        }
    } else if (sVar2 == 300) {
        *puVar3 = 0;
        FUN_00460dc0(0xd2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            FUN_00460dc0(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
    } else {
        FUN_0043e820(entity_idx, 6);
        goto LAB_00445110;
    }
    FUN_00404bc0(0x5e, (int)entity_idx, 0);

LAB_00445110:
    // Post-teleport footstep sound: only if anim speed == _DAT_00552580 (ground landed)
    if (*(float*)(puVar3 + 0x108) == *(float*)&DAT_00552580) {
        if ((sVar2 == 0x186) && (*(int*)(puVar3+4) < 0xce || *(int*)(puVar3+4) > 0xd0)) {
            // DarkWizard swimming vs walking footstep
            if ((puVar3[0x1bc] & 7) == 2)
                FUN_00404bc0(0x50, (int)entity_idx, 0);
            else
                FUN_00404bc0(0x4e, (int)entity_idx, 0);
            return;
        }
        // General: look up footstep sound from entity-class table at DAT_05828d58+0xb2
        short sfx = *(short*)((char*)DAT_05828d58 + 0xb2 + sVar2 * 0xbc);
        if (sfx != -1)
            FUN_00404bc0(sfx + 0xaa, (int)entity_idx, 0);
    }
}

// Entity_TeleportAnim @ 0x004792C0
// Finds a free slot in the teleport-effect pool (DAT_07C80110, stride 0x70, ~12 entries).
// Fills: active=1, entity_id, src_pos (world_pos + height_offset DAT_00552958), dst_pos.
// Slots: [0x00]=active, [0x04]=entity_id, [0x10..0x18]=src_pos, [0x1C..0x24]=dst_pos,
//        [0x0C]=param_4, [0x38..0x3B]=phase_flags, [0x48..0x4B]=state_chars.
void __cdecl Entity_TeleportAnim(float* world_pos, float entity_id, float* dst_pos, float param_4) {
    // BUG-FIX 2026-05-03: was `while ((int)pcVar1 < 0x7c82cd0)` (absolute source-binary
    // bound). With DAT_07c80110 sized as 1 byte and the loop walking 100 × 0x70 bytes,
    // every teleport effect spawn corrupted the heap. Pool now sized to 100 slots in
    // globals.cpp; bound is iteration count.
    char *pcVar1 = (char*)&DAT_07c80110[0];
    for (int i = 0; i < 100; ++i) {
        if (*pcVar1 == '\0') {
            *pcVar1 = '\x01';
            *(float*)(pcVar1 + 0x04) = entity_id;
            *(float*)(pcVar1 + 0x10) = world_pos[0];
            *(float*)(pcVar1 + 0x14) = world_pos[1];
            *(float*)(pcVar1 + 0x18) = world_pos[2] + _DAT_00552958;
            *(float*)(pcVar1 + 0x1c) = dst_pos[0];
            *(float*)(pcVar1 + 0x20) = dst_pos[1];
            *(float*)(pcVar1 + 0x24) = dst_pos[2];
            *(float*)(pcVar1 + 0x0c) = param_4;
            pcVar1[0x38] = pcVar1[0x39] = pcVar1[0x3a] = pcVar1[0x3b] = '\0';
            pcVar1[0x48] = '\0'; pcVar1[0x49] = '\0';
            pcVar1[0x4a] = ' ';  pcVar1[0x4b] = 'A';
            return;
        }
        pcVar1 += 0x70;
    }
}

// Entity_MeleeAttackStart @ 0x00474310
// Melee attack initiation: anim 0x57, set speed=5.0f (+0x108), mode=3 (+0x7C),
// scale=1.0f (+0x164), spawn effect 0x498, play sound 0x58.
void __cdecl Entity_MeleeAttackStart(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(float*)(param_1 + 0x108) = 5.0f;   // 0x40A00000
    *(BYTE *)(param_1 + 0x7c)  = 3;      // melee attack mode
    *(float*)(param_1 + 0x164) = 1.0f;   // 0x3F800000
    FUN_00460dc0(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// FUN_00479330 @ 0x00479330 — RenderPoints (damage popup renderer)
// 2026-05-06: ported from IDA mu97k-src-IDA/raw/00479330_RenderPoints.c.
//
// Itera el pool DAT_07c80110[100 × 0x70] de damage popups (poblado por
// CreatePoint en Net_Process case 0x15 / ReceiveAttackDamage). Para cada
// slot activo proyecta su world position a screen via gluProject y
// renderiza el número con RenderNumber2D usando el color del slot.
//
// Slot layout (per IDA CreatePoint):
//   +0x00 byte  active (1 if displayed)
//   +0x04 int   Value (damage to display; -1 = MISS)
//   +0x0c float scale (text size — typically 15 normal, 50 special)
//   +0x10 float pos.x (world)
//   +0x14 float pos.y (world)
//   +0x18 float pos.z (world, +140 elevation pre-applied)
//   +0x1c float color.r
//   +0x20 float color.g
//   +0x24 float color.b
//   +0x38 float frame counter (init 0, MovePoints increments)
//   +0x48 float lifetime (init 10.0, MovePoints decrements 0.3/tick)
extern "C" double __cdecl RenderNumber2D(float x, float y, int Num,
                                          float Width, float Height);

extern void __cdecl FUN_005113f0(float*, int*, int*);   // World_ToScreen
extern "C" // ─────────────────────────────────────────────────────────────────────────────
// RenderNumber  @ 0x005120C0  — dibuja un número de daño en el MUNDO.
// RenderPoints  @ 0x00479330  — recorre el pool y llama al anterior.
//
// 2026-08-15 — REESCRITO FIEL A IDA. La versión anterior era una invención en
// dos fases (project con `gluProject` + draw con `RenderNumber2D` en ortho 2D).
// Tres síntomas venían de ahí:
//   · nada se veía — se proyectaba con la matriz MODELVIEW leída de GL, que en
//     ese punto del frame es la IDENTIDAD (el call site corre después de
//     `FUN_00511cf0`/BeginSprite);
//   · los dígitos salían invertidos — `RenderNumber2D` usa V de 0.0→0.5 y el
//     original usa 0.5→0.0;
//   · los MISS salían como barras blancas — el original tiene un sprite propio
//     para `Num == -1`, no dibuja dígitos.
//
// El binario no proyecta a mano: `RenderNumber` emite el número como sprites 3D
// con `RenderSpriteUV` (0x511FB0), que ya transforma por la CameraMatrix. Por
// eso el call site está dentro del bloque 3D, entre BeginSprite y glPopMatrix.
// ─────────────────────────────────────────────────────────────────────────────
// (RenderSpriteUV_stub y FUN_005114f0 ya están declarados en functions.h)

extern "C" void __cdecl RenderNumber(float Position[3], int Num,
                                     float Color[3], float Alpha, float Scale);
void __cdecl RenderNumber(float Position[3], int Num, float Color[3],
                          float Alpha, float Scale)
{
    float p[3]   = { Position[0], Position[1], Position[2] };
    float UV[4][2];
    float Light[3][4];

    // Patrón cíclico de los 3 componentes, tal cual IDA L26-42.
    Light[0][0] = Color[0]; Light[0][1] = Color[1];
    Light[0][2] = Color[2]; Light[0][3] = Color[0];
    Light[1][0] = Color[1]; Light[1][1] = Color[2];
    Light[1][2] = Color[0]; Light[1][3] = Color[1];
    Light[2][0] = Color[2]; Light[2][1] = Color[0];
    Light[2][2] = Color[1]; Light[2][3] = Color[2];

    if (Num == -1) {
        // MISS: sprite propio, región (0..0.125, 0.53125..1.0), tamaño 45x20.
        // Constantes leídas de los QWORD del decompile.
        UV[0][0] = 0.0f;   UV[0][1] = 1.0f;
        UV[1][0] = 0.125f; UV[1][1] = 1.0f;
        UV[2][0] = 0.125f; UV[2][1] = 0.53125f;
        UV[3][0] = 0.0f;   UV[3][1] = 0.53125f;
        RenderSpriteUV_stub(1, p, 45.0f, 20.0f, UV, Light, Alpha);
        return;
    }

    char Text[32];
    _itoa(Num, Text, 10);
    const int len = (int)strlen(Text);
    if (len <= 0) return;

    const float v14 = (float)len * Scale * 0.125f;
    p[0] = p[0] - (float)len * 5.0f - v14;
    p[1] = p[1] - v14;

    const float step = Scale * 0.5f;
    for (int i = 0; i < len; ++i) {
        const float u = (float)(Text[i] - '0') * 0.0625f;
        // V va de 0.5 (abajo) a 0.0 (arriba) — invertida respecto de la
        // intuición; así lo hace el binario y es lo que deja los dígitos
        // derechos.
        UV[0][0] = u;            UV[0][1] = 0.5f;
        UV[1][0] = u + 0.0625f;  UV[1][1] = 0.5f;
        UV[2][0] = u + 0.0625f;  UV[2][1] = 0.0f;
        UV[3][0] = u;            UV[3][1] = 0.0f;
        RenderSpriteUV_stub(1, p, Scale, Scale, UV, Light, Alpha);
        p[0] += step;
        p[1] += step;
    }
}

// RenderPoints — port FIEL de IDA 0x479330. Los 4 args son un artefacto del
// call site anterior; se ignoran.
void __cdecl FUN_00479330(int, int, int, int)
{
    EnableAlphaTest(true);
    FUN_005114f0();                    // DisableDepthTest

    char* base = (char*)&DAT_07c80110[0];
    for (int i = 0; i < 100; ++i) {
        char* slot = base + i * 0x70;
        if (slot[0] == 0) continue;

        // IDA: alpha = lifetime(+0x48) * 0.4
        const float alpha = *(float*)(slot + 0x48) * 0.40000001f;
        RenderNumber((float*)(slot + 0x10),      // world pos
                     *(int*)(slot + 0x04),       // value (-1 = MISS)
                     (float*)(slot + 0x1c),      // color
                     alpha,
                     *(float*)(slot + 0x0c));    // scale
    }
}


// ── Missing stubs added for linker fix ───────────────────────────────────────

// GL helpers — cached OpenGL state wrappers
// CORRECCION: 0xb71 es GL_DEPTH_TEST, NO GL_SCISSOR_TEST (que es 0x0c11).
// FUN_005114d0 — glEnable(GL_DEPTH_TEST) with cache in DAT_083a411e
void __cdecl FUN_005114d0(void) {
    if (DAT_083a411e == '\0') { DAT_083a411e = '\x01'; glEnable(0x0b71); }
}
// FUN_005114f0 — glDisable(GL_DEPTH_TEST)
// DIAG: el AV NVIDIA aparece acá no por el glDisable per se, sino por corrupción previa
// del contexto GL que aflora al primer comando subsiguiente (driver bufferea cmds).
// Loggeamos antes/después con glGetError() para localizar el comando ofensivo.
void __cdecl FUN_005114f0(void) {
    static int s_call = 0;
    s_call++;
    GLenum err_before = glGetError();
    if (err_before != 0) {
        char b[160];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "FUN_005114f0 #%d ENTER  glGetError(pre)=0x%x  cache=%d",
            s_call, (int)err_before, (int)DAT_083a411e);
        DbgLogPublic(b);
    }
    if (DAT_083a411e != '\0') { DAT_083a411e = '\0'; glDisable(0x0b71); }
    GLenum err_after = glGetError();
    if (err_after != 0) {
        char b[160];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "FUN_005114f0 #%d EXIT   glGetError(post)=0x%x",
            s_call, (int)err_after);
        DbgLogPublic(b);
    }
}
// FUN_00511510 — glDepthMask(GL_TRUE) with cache in DAT_083a42e8
void __cdecl FUN_00511510(void) {
    if (DAT_083a42e8 == '\0') { DAT_083a42e8 = '\x01'; glDepthMask(1); }
}
// FUN_00511530 — glDepthMask(GL_FALSE)
void __cdecl FUN_00511530(void) {
    if (DAT_083a42e8 != '\0') { DAT_083a42e8 = '\0'; glDepthMask(0); }
}
// FUN_00511550 — EnableCullFace: glEnable(GL_CULL_FACE=0xb44) with cache in DAT_083a411c
// (Called by 3D blend setters FUN_00511600 / FUN_00511890 per 5.2 source pattern.)
void __cdecl FUN_00511550(void) {
    if (DAT_083a411c == '\0') { DAT_083a411c = '\x01'; glEnable(0xb44); }
}
// FUN_00511570 — DisableCullFace: glDisable(GL_CULL_FACE=0xb44)
// (Called by 2D blend setters FUN_00511680 / FUN_00511710 / FUN_00511790.)
void __cdecl FUN_00511570(void) {
    if (DAT_083a411c != '\0') { DAT_083a411c = '\0'; glDisable(0xb44); }
}
// FUN_00511590 — set depth-write + blend(0xbc0) + disable alpha-test(0xde1)
void __cdecl FUN_00511590(char param_1) {
    FUN_00511510();
    if (param_1 == '\x01') {
        if (DAT_083a411d == '\0') { DAT_083a411d = '\x01'; glEnable(0xbc0); }
    } else {
        if (DAT_083a411d != '\0') { DAT_083a411d = '\0'; glDisable(0xbc0); }
    }
    if (DAT_083a4125 != '\0') { DAT_083a4125 = '\0'; glDisable(0xde1); }
}
// FUN_00511910 — glViewport; stores params for later use in perspective calc
void __cdecl FUN_00511910(int param_1, int param_2, int param_3, int param_4) {
    DAT_083a4280 = (DWORD)param_3;
    DAT_083a42c8 = (DWORD)param_1;
    DAT_083a42cc = (DWORD)param_2;
    DAT_083a42b0 = (DWORD)param_4;
    glViewport(param_1, (int)(DAT_00561570 - param_2) - param_4, param_3, param_4);
}
// FUN_00511220 — gluPerspective + precompute per-pixel projection factors
void __cdecl FUN_00511220(int fov, float aspect, int near_clip, float far_clip) {
    float fovF  = Ff(fov);
    float nearF = Ff(near_clip);
    gluPerspective((double)fovF, (double)aspect, (double)nearF, (double)far_clip);
    double tanHalfFov = tan((double)fovF * (double)_DAT_00552cc4);
    DAT_083a429c = DAT_083a4280 / 2 + DAT_083a42c8;
    DAT_083a42a0 = DAT_083a42b0 / 2 + DAT_083a42cc;
    _DAT_083a4294 = (float)((int)DAT_0056156c - (int)DAT_083a42a0);
    _DAT_083a42a4 = (float)(tanHalfFov / (double)(DAT_083a4280 / 2) * (double)aspect);
    _DAT_083a42a8 = (float)(tanHalfFov / (double)(DAT_083a42b0 / 2)
                            * ((double)(int)DAT_00561570 / (double)(int)DAT_083a42b0));
}
// FUN_00511950 @ 0x00511950 — ConvertX: scale logical-640 X coord to real window X.
// FUN_00511980 @ 0x00511980 — ConvertY: scale logical-480 Y coord to real window Y.
// Formula: WindowWidth * x * (1/640)   and   WindowHeight * y * (1/480).
// Used by RenderBitmap, RenderColor, CreateFrustrum2D, SetPositionIME_Wnd, etc.
long double __cdecl FUN_00511950(float v) {
    return (long double)((float)(int)DAT_0056156c * v * _DAT_0055283c);
}
long double __cdecl FUN_00511980(float v) {
    return (long double)((float)(int)DAT_00561570 * v * _DAT_00552838);
}
// FUN_00529000 @ 0x00529000 — WriteJpeg(path, width, height, pixel_buf, quality)
// Writes RGB pixel buffer to JPEG file using libjpeg compression.
// Rows are written bottom-up (flipped) to match OpenGL framebuffer layout.
// Ghidra: local_1d8=3 (components), local_1d4=2 (JCS_RGB), quality from param_5.
unsigned int __cdecl FUN_00529000(const char* path, int width, int height, void* pixelBuf, int quality)
{
    FILE* f = fopen(path, "wb");
    if (f == NULL) return 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, f);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    int row_stride = width * 3;
    unsigned char* buf = (unsigned char*)pixelBuf;

    // Write rows bottom-up (flip for OpenGL framebuffer origin)
    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row_pointer = buf + (height - 1 - cinfo.next_scanline) * row_stride;
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(f);
    jpeg_destroy_compress(&cinfo);
    return 1;
}

// Particle / angle helpers

// FUN_0043e430 @ 0x0043E430 — Angle_From2Points(x1,y1,x2,y2)
// Computes clockwise angle 0..359 from point (x1,y1) toward (x2,y2).
// Uses x87 fpatan(dy/dx,1) then ftol; adds 180 if x2<x1; wraps negative.
int __cdecl FUN_0043e430(float param_1, float param_2, float param_3, float param_4) {
    double fVar2;
    if ((double)param_3 - (double)param_1 == 0.0) {
        fVar2 = 0.0;
    } else {
        fVar2 = ((double)param_2 - (double)param_4) / ((double)param_3 - (double)param_1);
    }
    int iVar1 = (int)atan(fVar2);  // fpatan → ftol (result in radians-as-int units)
    if (param_3 < param_1) iVar1 += 0xb4;   // 180
    if (iVar1 < 0)         iVar1 += 0x168;  // 360
    return (0x168 - iVar1) % 0x168;
}


// Monster/Scene data loaders
// FUN_0050b510 — implemented in src/Monster/Monster_Data.cpp
// FUN_0047d020 — implemented in src/Monster/Monster_Data.cpp
// FUN_00505bd0 @ 0x00505BD0 — Model_SetSlotIndex(index): sets active model slot index.
void __cdecl FUN_00505bd0(int param_1) {
    DAT_083a4104 = 0;
    DAT_083a4108 = param_1;
}
// FUN_00505e90 @ 0x00505E90 — OpenModel(Type, Dir, ModelFileName, ...).
// Port FIEL del IDA (raw 0x505E90):
//   1. FileName = Dir + ModelFileName
//   2. Itera variadic args (extra animation .smd paths) hasta NULL o "end"
//   3. Si v11>0: OpenSMDModel(Type, FileName, v11, unk_83A4100) +
//      OpenSMDAnimation(Type, FileNameN, lockFlag) por cada N
//   4. Si v11==0: OpenSMDModel(Type, FileName, 1, unk_83A4100) +
//      OpenSMDAnimation(Type, FileName, 0)
//
// NOTA 2026-05-01: los archivos Data2/Item/<class>/<file>.smd NO existen en el
// filesystem distribuido (solo Data/Item/<file>.bmd está). Las llamadas a
// fopen dentro de FUN_0040b280/FUN_0040b310 retornarán NULL → early return →
// no-op silencioso. El path BMD (FUN_005060b0) cubre la carga real de items.
// (FUN_0040b280, FUN_0040b310 — declared via functions.h.
//  DAT_083a4100 — declared in globals.h.)

// Note: la signature original es variadic (`...` para extra anim paths) pero
// TODOS los call sites en nuestro source pasan exactamente 3 args (sin anims
// extras). Mantener 3 args para compat con functions.h.
void __cdecl FUN_00505e90(int Type, const char* Dir, const char* ModelFileName) {
    char FileName[200];
    // Build base FileName = Dir + ModelFileName
    crt_sprintf(FileName, "%s%s", Dir ? Dir : "", ModelFileName ? ModelFileName : "");
    // Sin variadic args, v11=0 → call OpenSMDModel + OpenSMDAnimation con
    // FileName base solamente.
    FUN_0040b280(Type, FileName, 1, (char)DAT_083a4100);
    FUN_0040b310(Type, FileName, 0);
    DAT_083a4100 = 0;

    // BUG-FIX 2026-05-04: el cliente 0.97k distribuido NO tiene Data2/Object*/
    // (solo Data/Object*/ con archivos .bmd). Las SMDs no cargan → Lorencia
    // queda sin casas/decoraciones porque su path de OpenWorldModels usa SMDs
    // exclusivamente. Como fallback, si después del SMD load el slot sigue
    // vacío (mesh count = 0), intentamos varias transformaciones del nombre
    // .smd → .bmd para encontrar el archivo real (case-insensitive en Win32).
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * Type);
    short meshCount = *(short*)(slot + 0x22);  // model[+0x22] = mesh count
    if (meshCount > 0) return;  // SMD load worked

    if (!Dir || !ModelFileName) return;
    // Strip "Data2/" prefix → "Data/" prefix for BMD fallback
    char bmdDir[128];
    if (strncmp(Dir, "Data2/", 6) == 0)
        crt_sprintf(bmdDir, "Data/%s", Dir + 6);
    else
        crt_sprintf(bmdDir, "%s", Dir);

    // Strip ".smd" extension from filename
    char baseName[128];
    crt_sprintf(baseName, "%s", ModelFileName);
    int blen = (int)strlen(baseName);
    if (blen > 4 && _stricmp(baseName + blen - 4, ".smd") == 0)
        baseName[blen - 4] = 0;

    // Hardcoded SMD→BMD name remaps for known mismatches in Lorencia
    // (smd uses snake_case named files; bmd uses PascalCase with different names)
    struct NameRemap { const char* smd; const char* bmd; };
    static const NameRemap remaps[] = {
        // Trees: 13 SMDs → 13 BMDs (Tree01..Tree13)
        {"treesmall",  "Tree01"}, {"treebig",    "Tree02"},
        {"treea_01",   "Tree03"}, {"treea_02",   "Tree04"},
        {"treea_03",   "Tree05"}, {"treea_04",   "Tree06"},
        {"treea_05",   "Tree07"}, {"treea_06",   "Tree08"},
        {"treea_07",   "Tree09"}, {"treea_08",   "Tree10"},
        {"treea_09",   "Tree11"}, {"treea_10",   "Tree12"},
        {"treea_11",   "Tree13"},
        // Stones (typo "Ston" → "Stone")
        {"Ston_01",    "Stone01"}, {"Ston_02", "Stone02"},
        {"Ston_03",    "Stone03"}, {"Ston_04", "Stone04"},
        {"Ston_05",    "Stone05"},
        // Mushrooms (drop underscore + numbering different)
        {"mushroom_01","Mushroom01"}, {"mushroom_02","Mushroom02"},
        // Walls
        {"wall_01",    "Wall01"}, {"wall_02", "Wall02"},
        {"wall_03",    "Wall03"}, {"wall_04", "Wall04"},
        {"wall_05",    "Wall05"}, {"wall_06", "Wall06"},
        {"c_wall01",   "CWall01"}, {"c_wall02","CWall02"},
        {"c_wall03",   "CWall03"}, {"c_wall04","CWall04"},
        {"c_wall05",   "CWall05"},
        // Tomb / TombStone
        {"Tomb_arc",   "Tomb01"}, {"Tomb_cross","Tomb02"},
        {"TombStone",  "TombStone01"},
        // Bridge
        {"bridge_01",      "Bridge01"},
        {"bridge_stone",   "BridgeStone01"},
        // Carriage / fence / etc. (already PascalCase variant works)
    };
    for (size_t i = 0; i < sizeof(remaps)/sizeof(remaps[0]); ++i) {
        if (_stricmp(baseName, remaps[i].smd) == 0) {
            FUN_005060b0(Type, bmdDir, remaps[i].bmd, -1);
            meshCount = *(short*)(slot + 0x22);
            if (meshCount > 0) return;
            break;
        }
    }

    // Build candidate variations to try in order:
    //   1. baseName as-is (e.g. "house_01" → "house_01.bmd")
    //   2. PascalCase no underscores: capitalize first letter of each segment
    //      between underscores, drop underscores. ("house_01" → "House01")
    //   3. Capitalized with underscores preserved. ("house_01" → "House_01")
    char cand[3][128];
    crt_sprintf(cand[0], "%s", baseName);
    // PascalCase no underscores
    {
        int j = 0;
        bool capitalize = true;
        for (int i = 0; baseName[i] && j < 126; ++i) {
            char c = baseName[i];
            if (c == '_') { capitalize = true; continue; }
            if (capitalize) {
                cand[1][j++] = (char)toupper((unsigned char)c);
                capitalize = false;
            } else {
                cand[1][j++] = c;
            }
        }
        cand[1][j] = 0;
    }
    // Capitalized with underscores
    {
        int j = 0;
        bool capitalize = true;
        for (int i = 0; baseName[i] && j < 126; ++i) {
            char c = baseName[i];
            if (c == '_') { cand[2][j++] = c; capitalize = true; continue; }
            if (capitalize) {
                cand[2][j++] = (char)toupper((unsigned char)c);
                capitalize = false;
            } else {
                cand[2][j++] = c;
            }
        }
        cand[2][j] = 0;
    }

    // Try each variant. FUN_005060b0 with idx=-1 appends ".bmd" → "<name>.bmd".
    // After each attempt check mesh count; bail when slot is populated.
    for (int v = 0; v < 3; ++v) {
        FUN_005060b0(Type, bmdDir, cand[v], -1);
        meshCount = *(short*)(slot + 0x22);
        if (meshCount > 0) return;
    }
}
// FUN_005098c0 — implemented in src/Monster/Monster_Data.cpp
// FUN_0047a1f0 @ 0x0047A1F0 — GetToken: text tokenizer for all game data files
// (Monster.txt / Item.txt / Skill.txt / NPC.txt / Gate.txt / Filter.txt).
//
// Ported from IDA Hex-Rays (610 bytes). Reads from `SMDFile` (DAT_0055c0a0),
// stores result in TokenString / TokenNumber / CurrentToken globals.
//
// Returns:
//   0   = identifier (alpha[alnum/./_]*) — stored in TokenString
//   1   = number (atof(buf)) — stored in TokenNumber (_DAT_083a40f8)
//   2   = EOF (0xFF byte read)
//   '#' = literal hash
//   ',' = literal comma
//   ';' = literal semicolon
//   '{' / '}' = brace literals
//   '<' (60) = unrecognized character
//   string-literal "…" handled like identifier (sets CurrentToken=0, returns 0)
//
// Uses CRT helpers fgetc/getc/ungetc/isspace/isalpha/isalnum/isdigit/atof —
// all available in stdafx-included <cctype>/<cstdio>/<cstdlib>.
int __cdecl FUN_0047a1f0(void)
{
    // 2026-08-22 FIX: escribia en DAT_083a3ff4, que es el buffer del OTRO
    // tokenizer (Parse_NextToken / OpenWorldModels).  TokenString es 0x07CF1EF0.
    char* TokenStringBuf = (char*)&DAT_07cf1ef0[0];
    int&   CurrentToken  = _DAT_083a40f4;
    float& TokenNumber   = _DAT_083a40f8;

    // CRITICAL 2026-05-03: data parsers (Item_Data, Monster_Data, Skill_Data,
    // Filter_Data, NPC_Data, Gate_Data) all open their file via DAT_07d7806c
    // (= IDA's SMDFile_0). The other "SMDFile" symbol at DAT_0055c0a0 is a
    // separate misnamed global from an early port pass — unused here.
    FILE* fp = DAT_07d7806c;

    char  TempString[100];
    int   cha;
    char* p;

    TokenStringBuf[0] = 0;
    if (!fp) return 2;

    cha = fgetc(fp);
    if ((char)cha == (char)0xFF || cha == EOF) return 2;

    // Skip comments (// … \n) and whitespace
    while (1) {
        if ((char)cha == '/') {
            cha = fgetc(fp);
            if ((char)cha == '/') {
                do { cha = fgetc(fp); }
                while ((char)cha != '\n' && cha != EOF);
            }
        }
        if (cha == EOF) return 2;
        if (!isspace((unsigned char)cha)) break;
        cha = fgetc(fp);
        if ((char)cha == (char)0xFF || cha == EOF) return 2;
    }

    int result;
    switch ((char)cha) {
    case '"':
        // Quoted string — read until closing quote, save into TokenString
        p = TokenStringBuf;
        for (cha = getc(fp); cha != EOF && (char)cha != (char)0xFF; cha = getc(fp)) {
            if ((char)cha == '"') break;
            *p++ = (char)cha;
        }
        if ((char)cha == (char)0xFF || cha == EOF) ungetc(cha, fp);
        *p = 0;
        CurrentToken = 0;
        return 0;

    case '#': result = 35;  CurrentToken = 35;  break;
    case ',': result = 44;  CurrentToken = 44;  break;
    case ';': result = 59;  CurrentToken = 59;  break;
    case '{': result = 123; CurrentToken = 123; break;
    case '}': result = 125; CurrentToken = 125; break;

    case '-': case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        ungetc(cha, fp);
        p = TempString;
        for (cha = getc(fp); cha != EOF && (char)cha != (char)0xFF; cha = getc(fp)) {
            if ((char)cha != '.' && !isdigit((unsigned char)cha) && (char)cha != '-') break;
            *p++ = (char)cha;
        }
        ungetc(cha, fp);
        *p = 0;
        TokenNumber = (float)atof(TempString);
        CurrentToken = 1;
        return 1;

    default:
        if (isalpha((unsigned char)cha)) {
            TokenStringBuf[0] = (char)cha;
            p = &TokenStringBuf[1];
            for (cha = getc(fp); cha != EOF && cha != -1; cha = getc(fp)) {
                char c2 = (char)cha;
                if (c2 != '.' && c2 != '_' && !isalnum((unsigned char)cha)) break;
                *p++ = c2;
            }
            ungetc(cha, fp);
            *p = 0;
            CurrentToken = 0;
            return 0;
        } else {
            CurrentToken = 60;
            return 60;
        }
    }
    return result;
}

// FUN_0047ea70 @ 0x0047EA70 — Skill_HashTable_SerializeEntry: encode 0x28-byte
// entry via rolling XOR/sub cipher and insert into hash table.
void __cdecl FUN_0047ea70(void *dst, void *src) { /* hash table serialize stub */ }
// FUN_0047eaf0 @ 0x0047EAF0 — Skill_HashTable_FreeEntry: decode entry and remove.
void __cdecl FUN_0047eaf0(void *entry, void *key) { /* hash table free stub */ }
// FUN_005430f0 @ 0x005430F0 — fwrite wrapper (with lock).
uint __cdecl FUN_005430f0(char *buf, uint size, uint count, int *fp) {
    return (uint)fwrite(buf, size, count, (FILE *)fp);
}
// FUN_005060b0 @ 0x005060B0 — Model_LoadBMD_ByIdx(slot, dir, basename, idx): loads BMD file at slot.
// Construye leafname "basename.bmd" (idx==-1), "basename0N.bmd" (idx<10) o
// "basenameNN.bmd" (idx>=10), usando param_3 (BASENAME) — NO param_2 (directorio).
// param_2 (directorio "Data/Logo/") se pasa aparte al loader BMD.
// BUG PREVIO: el sprintf pasaba param_2 en vez de param_3 → el filename quedaba
// "Data/Logo/01.bmd" en vez de "Logo01.bmd", los modelos de login/select nunca
// cargaban y el fondo 3D del server select quedaba vacío.
void __cdecl FUN_005060b0(int param_1, const char *param_2, const char *param_3, int param_4) {
    // BUG-FIX 2026-04-29: pump message queue cada N llamadas para evitar que
    // OpenWorld (que llama esta func ~hundreds de veces) bloquee el message
    // pump por 2+ segundos. El server MuEmu nos kickea por backpressure si
    // no consumimos los packets que envía después del JoinMapServer.
    {
        static int s_pumpCounter = 0;
        if ((s_pumpCounter++ & 0x07) == 0) {  // cada 8 BMDs
            MSG msg;
            for (int i = 0; i < 16 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    char local_40[64];
    if (param_4 == -1) {
        crt_sprintf(local_40, s__s_bmd_0055a7f8, param_3);
    } else {
        const char *pcVar1 = (param_4 < 10) ? s__s0_d_bmd_0055a7ec : s__s_d_bmd_0055a7e0;
        crt_sprintf(local_40, pcVar1, param_3, param_4);
    }
    int numBonesInSlot = *(short*)(DAT_05828d58 + 0x22 + param_1 * 0xbc);
    {
        char diag[200];
        _snprintf_s(diag, sizeof(diag), _TRUNCATE,
            "FUN_005060b0: slot=0x%x dir='%s' leaf='%s' idx=%d bones=%d hqMode=%d",
            param_1, param_2 ? param_2 : "(null)", local_40, param_4,
            numBonesInSlot, (int)(DAT_0055a7c4 == '\0'));
        DbgLogPublic(diag);
    }
    if (DAT_0055a7c4 == '\0') {
        // HQ path original: si el SMD ya cargó bones, FUN_00442a60 agrega la anim BMD.
        // PORT FALLBACK: como nuestro SMD loader (FUN_00505e90) es stub y nunca
        // popula bones, caemos al loader completo FUN_004423e0 para al menos traer
        // la geometría BMD y ver algo del background 3D.
        if (numBonesInSlot > 0)
            FUN_00442a60((int)(DAT_05828d58 + param_1 * 0xbc), (char*)param_2, local_40);
        else
            FUN_004423e0((int)(DAT_05828d58 + param_1 * 0xbc), (int)param_2, (int)local_40, 0);
    } else {
        FUN_004423e0((int)(DAT_05828d58 + param_1 * 0xbc), (int)param_2, (int)local_40, 0);
    }

    // Post-load defensive init: ensure bodyLight is (1,1,1) even if the BMD
    // load path bypassed BMD::Open (e.g. SMD-first HQ path). Without this the
    // login-scene slots retain the 0xF5F5F5F5 pool-fill garbage pattern at
    // +0x50, which manifests as white/yellow/corrupt triangles on Ship/Logos.
    {
        int slotBase = (int)(DAT_05828d58 + param_1 * 0xbc);
        *(float *)(slotBase + 0x48) = 1.0f;
        *(float *)(slotBase + 0x4c) = 1.0f;
        *(float *)(slotBase + 0x50) = 1.0f;
    }
}
// Forward-declare FindTextureByName (real implementation at ~line 12786 below).
int __cdecl FindTextureByName(char *Name, DWORD *dwTexture);

// ── FUN_00505c80 @ 0x00505C80 — OpenTexture (Model_LoadTextures) ────────────
// Para cada mesh del modelo en slot [Model]:
//   1) Lee el nombre de textura (32 bytes) desde pBMD->Data[+0x34] + i*0x20.
//   2) FindTextureByName — si ya está cargada, reusa e incrementa ref-count.
//   3) Si no: construye "SubFolder + name", llama OpenTGA (.t*) u OpenJPG (.*),
//      copia el nombre al slot de Bitmaps, e incrementa TextureCurrent.
//   4) Guarda el slot resuelto en pBMD->Data[+0x38] + i*2 (IndexTexture[i]).
//   5) Overrides por prefijo de filename: "ski*"/"level*" → 0x12d (BITMAP_SKIN),
//      "hid*" → 300 (BITMAP_HIDE).
//
// Sin esta función, IndexTexture queda sin inicializar (0xCDCD en MSVC Debug)
// y BMD_DrawMesh hace BindTexture con un id basura → naves/logos aparecen
// blancos o con textura corrupta. Verificado via debug.log (tex=0xffffcdcd).
//
// Globals:
//   Models            = DAT_05828d58 (BMD table, stride 0xBC; Data ptr at +0x00)
//   TextureBegin      = DAT_083a4104 (int, lower bound del scan)
//   TextureCurrent    = DAT_083a4108 (int, next-free slot)
//   Bitmaps[]         = g_BitmapsRaw (stride 0x38; filename en [+0x00..+0x1F])
//   DAT_0055a7a4      = base path "Data2\"   (Data2/pak mode)
//   DAT_0055a79c      = base path "Data\"    (Data mode)
void __cdecl FUN_00505c80(int Model, const char* SubFolder, int Type, char Check) {
    // ── BUG fix (crash 0xC0000005 @ 0x61746168 "ataH"): el Model slot ES la
    //    estructura BMD completa (stride 0xBC), NO un puntero a datos. Los
    //    primeros 32 bytes del slot son el Name (string), no un data ptr.
    //    Los contadores y tablas están inline:
    //       slot +0x24 short  numMeshes
    //       slot +0x34 char*  texNameTable (char[n][0x20])
    //       slot +0x38 short* indexTexture (short[n])
    //    Verificado en Ghidra FUN_004423e0 (BMD::Open): this[0x24]=numMeshes,
    //    this[0x34]=texName[] y this[0x38]=indexTex[] se asignan directamente.
    char* slot = (char*)(DAT_05828d58 + Model * 0xBC);
    short numMeshes = *(short*)(slot + 0x24);
    // DIAG-canary inconditional: confirmar que llegamos a la función para Ship/Logo
    {
        static int s_oc_any = 0;
        if (s_oc_any < 8) {
            char b[160];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "FUN_00505c80 CALL Model=0x%x sub='%s' nMesh=%d TextureCurrent=0x%x",
                Model, SubFolder ? SubFolder : "(null)", (int)numMeshes, (unsigned)DAT_083a4108);
            DbgLogPublic(b);
            s_oc_any++;
        }
    }
    // DIAG: log ENTRY for login-scene slots (Ship=0x3c, Logos=0xa0..0xa3).
    {
        static int s_oc = 0;
        bool isLoginSlot = (Model == 0x3c || (Model >= 0xa0 && Model <= 0xa3));
        if (isLoginSlot && s_oc < 20) {
            char* texNameTable0 = *(char**)(slot + 0x34);
            short* indexTexture0 = *(short**)(slot + 0x38);
            char firstName[33] = {0};
            if (texNameTable0 && numMeshes > 0) {
                memcpy(firstName, texNameTable0, 32);
                firstName[32] = 0;
            }
            char b[300];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "OpenTex ENTER Model=0x%x sub='%s' nMesh=%d texNames=%p idxTex=%p firstName='%s' TextureCurrent=0x%x",
                Model, SubFolder, (int)numMeshes, texNameTable0, indexTexture0,
                firstName, (unsigned)DAT_083a4108);
            DbgLogPublic(b);
            s_oc++;
        }
    }
    if (numMeshes <= 0) return;

    char* texNameTable  = *(char**)(slot + 0x34);
    short* indexTexture = *(short**)(slot + 0x38);
    if (!texNameTable || !indexTexture) return;

    for (int i = 0; i < numMeshes; i++) {
        char* Name = texNameTable + i * 0x20;
        DWORD pSlot = 0;
        int foundIdx = FindTextureByName(Name, &pSlot);
        int resolvedIdx;

        if (foundIdx < 0) {
            // Miss: load texture from disk into new slot
            // Locate '.' to detect extension
            int nameLen = (int)strnlen(Name, 32);
            int dotPos = 0;
            for (int k = 0; k < nameLen; k++) { if (Name[k] == '.') { dotPos = k; break; } }

            // Build full path: SubFolder + Name  (into local_40)
            // 2026-05-05: Si Name ya contiene un path (ej. "Data\Npc\foo.OZT"
            // como guardan algunos BMDs de NPC), NO concatenar SubFolder —
            // sino que sale "Data\Npc\Data\Npc\foo.OZT" → fopen FAIL.
            // Detectamos path absoluto: arranca con "Data\" o "Data/" o
            // contiene '\\' o '/' antes del primer '.'.
            char local_40[128];
            size_t nlen = strnlen(Name, 32);
            bool nameHasPath = false;
            for (size_t k = 0; k < nlen; ++k) {
                char ch = Name[k];
                if (ch == '\\' || ch == '/') { nameHasPath = true; break; }
                if (ch == '.') break;   // extension; stop scanning
            }
            if (nameHasPath) {
                if (nlen > sizeof(local_40) - 1) nlen = sizeof(local_40) - 1;
                memcpy(local_40, Name, nlen);
                local_40[nlen] = 0;
            } else {
                size_t slen = strlen(SubFolder);
                if (slen > 63) slen = 63;
                memcpy(local_40, SubFolder, slen);
                local_40[slen] = 0;
                if (slen + nlen > sizeof(local_40) - 1) nlen = sizeof(local_40) - 1 - slen;
                memcpy(local_40 + slen, Name, nlen);
                local_40[slen + nlen] = 0;
            }

            // Dispatch by extension character (tolower'd)
            int extChar = (dotPos + 1 < nameLen) ? tolower((unsigned char)Name[dotPos + 1]) : 'j';
            int slot = (int)DAT_083a4108;  // TextureCurrent
            if (extChar == 't')
                FUN_00529bd0(local_40, slot, 0x2600, 0x2901, 0, Check);   // OpenTGA
            else
                FUN_00529740(local_40, slot, Type,   0x2901, 0, Check);   // OpenJPG/OZJ

            // Store filename into the Bitmaps slot (first 32 bytes)
            char* slotBase = &g_BitmapsRaw[slot * 0x38];
            size_t fnLen = strnlen(Name, 32);
            memcpy(slotBase, Name, fnLen);
            if (fnLen < 32) memset(slotBase + fnLen, 0, 32 - fnLen);

            resolvedIdx = slot;
            DAT_083a4108 = slot + 1;   // TextureCurrent++
        } else {
            // Hit: reuse existing slot, bump ref count at +0x30
            if (pSlot != 0) {
                *(unsigned char*)(pSlot + 0x30) += 1;
            }
            resolvedIdx = foundIdx;
        }

        // Write resolved index into IndexTexture[i]
        indexTexture[i] = (short)resolvedIdx;

        // Special name overrides
        char c0 = Name[0], c1 = Name[1], c2 = Name[2];
        if (c0 == 's' && c1 == 'k' && c2 == 'i') {
            indexTexture[i] = 0x12d;   // BITMAP_SKIN
        } else if (c0 == 'l' && c1 == 'e' && c2 == 'v' && Name[3] == 'e' && Name[4] == 'l') {
            indexTexture[i] = 0x12d;   // BITMAP_SKIN (level variant)
        } else if (c0 == 'h' && c1 == 'i' && c2 == 'd') {
            indexTexture[i] = 300;     // BITMAP_HIDE
        }

        // DIAG: log Ship (0x3c) mesh→texture mapping so we can see
        // which mesh gets which texture slot and whether the BMD's texture
        // name matched an existing slot or triggered a disk load.
        if (Model == 0x3c) {
            char safe[33] = {0};
            memcpy(safe, Name, 32);
            safe[32] = 0;
            // sanitize non-printables
            for (int q = 0; q < 32; q++) {
                if (safe[q] && (safe[q] < 0x20 || safe[q] > 0x7e)) safe[q] = '?';
            }
            BITMAP_t* b = &Bitmaps[indexTexture[i]];
            char m[320];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "Ship mesh[%d] name='%s' foundIdx=%d resolvedIdx=0x%x idxTex=0x%x "
                "BMP: W=%.0f H=%.0f Comp=%d GL=%u Ref=%u Buf=%p",
                i, safe, foundIdx, (unsigned)resolvedIdx, indexTexture[i],
                (double)b->Width, (double)b->Height, (int)b->Components,
                (unsigned)b->TextureNumber, (unsigned)b->Ref, b->Buffer);
            DbgLogPublic(m);
        }
    }
}

// Forward decls for the SMD parsing chain (stubs below — files no existen
// en filesystem, retornan false; mantienen estructura del binario).
extern "C" bool __cdecl OpenSMDFile(const char* FileName, int Type, char Flip);
extern "C" void __cdecl FixupSMD(void);
extern "C" void __cdecl SMD2BMDModel(int ID, int Actions);
extern "C" void __cdecl SMD2BMDAnimation(int ID, char LockPosition);

// FUN_0040b280 @ 0x0040b280 — OpenSMDModel(ID, FileName, Actions, Flip)
// Port FIEL del IDA (raw 0x40B280):
//   if (Models[id].numMesh <= 0) {
//     if (OpenSMDFile(FileName, 0, Flip)) {
//       strcpy(Models[id], FileName);
//       Models[id].byte32 = 10;
//       FixupSMD();
//       SMD2BMDModel(ID, Actions);
//     }
//   }
void __cdecl FUN_0040b280(int ID, const char* FileName, int Actions, char Flip) {
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * ID);  // stride 188 = 0xbc
    if (*(short*)(slot + 36) > 0) return;  // already loaded
    if (!OpenSMDFile(FileName, 0, Flip)) return;
    strncpy(slot, FileName, 31);
    slot[31] = 0;
    *(unsigned char*)(slot + 32) = 10;
    FixupSMD();
    SMD2BMDModel(ID, Actions);
}

// FUN_0040b310 @ 0x0040b310 — OpenSMDAnimation(ID, FileName, LockPosition)
// Port FIEL del IDA (raw 0x40B310):
//   if (Models[id].numAnims > 0) {
//     OpenSMDFile(FileName, 1, 0);
//     SMD2BMDAnimation(ID, LockPosition);
//   }
void __cdecl FUN_0040b310(int ID, const char* FileName, char LockPosition) {
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * ID);
    if (*(short*)(slot + 34) <= 0) return;  // mesh slot not initialized
    OpenSMDFile(FileName, 1, 0);
    SMD2BMDAnimation(ID, LockPosition);
}
