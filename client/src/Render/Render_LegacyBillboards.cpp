// Render_LegacyBillboards.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

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


// FUN_00475170 @ 0x00475170 — ItemDrop_SetupRenderRef(slot_ptr)
// Resolves the entity reference at slot+0x3c, copies its world position to the
// model render slot, selects the target bone via equip-flags, then calls
// BMD_TransformPosition to animate/position it.
void __cdecl FUN_00475170(int param_1) {
    int iVar1 = *(int*)(param_1 + 0x3c);
    float local_c[3] = {0.0f, 0.0f, 0.0f};
    void *this_ = (void*)(DAT_05828d58 + *(short*)(iVar1 + 2) * 0xbc);
    if (*(int*)(param_1 + 4) == 0x4d0)
        local_c[1] = -120.0f;
    *(unsigned int*)((int)this_ + 0x6c) = *(unsigned int*)(iVar1 + 0x10);
    *(unsigned int*)((int)this_ + 0x70) = *(unsigned int*)(iVar1 + 0x14);
    *(unsigned int*)((int)this_ + 0x74) = *(unsigned int*)(iVar1 + 0x18);
    uint uVar2 = *(uint*)(param_1 + 8) & 0x80000001;
    if ((int)uVar2 < 0) uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
    BMD_TransformPosition(this_,
        (float*)((unsigned int)(*(byte*)(DAT_07abf5d8 + 0x274 + uVar2 * 0x18)) * 0x30
                 + *(int*)(iVar1 + 0x114)),
        local_c,
        (float*)(param_1 + 0x10),
        '\x01');
}
// FUN_00474f90 @ 0x00474F90 — Player_DrawInstance
// Renders a billboard quad at pos[], scaled by sc, rotated around Z by rot[0] angle,
// using texture slot param_1.
void __cdecl FUN_00474f90(int cls, float *pos, float *rot, float sc) {
    GL_BindTextureSlot(cls);
    GL_SetBlendAdditive();
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    float angle = rot ? rot[0] : 0.0f;
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    // Quad: half-size sc, CCW, full UV
    float q[4][3] = {
        {-sc, -sc, 0.0f},
        {-sc,  sc, 0.0f},
        { sc,  sc, 0.0f},
        { sc, -sc, 0.0f}
    };
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3fv(q[0]);
    glTexCoord2f(0.0f, 0.0f); glVertex3fv(q[1]);
    glTexCoord2f(1.0f, 0.0f); glVertex3fv(q[2]);
    glTexCoord2f(1.0f, 1.0f); glVertex3fv(q[3]);
    glEnd();
    glPopMatrix();
    GL_ResetState();
}
// FUN_00440aa0 (BMD::PlayAnimation / BMD_AnimTick) — moved to src/Render/BMD_Anim.cpp
// CharacterAnimation @ 0x00448600       — moved to src/Render/BMD_Anim.cpp
// (B3 refactor 2026-05-07)

// ── Weapon/Entity color helpers ───────────────────────────────────────────────

// FUN_00503cf0 @ 0x00503CF0 — Weapon_SetColor: maps item type to RGB color into color[3].
// color[3] *= scale * half_scale (scaled product), direction varies by item type index.
// flag=1 or flag=8 overrides selection for type 0x129/0x1f9.
void __cdecl FUN_00503cf0(int param_1, float param_2, float param_3, float *param_4, char param_5)
{
    unsigned int uVar4 = 0;
    // flag overrides
    if (param_5 != '\0' && (param_1 == 0x129 || param_1 == 0x1f9)) { uVar4 = 8; goto apply; }
    if (param_1 == 0x129 || param_1 == 0x1f9) { uVar4 = 1; goto apply; }
    if (param_1 == 0x131 || param_1 == 0x215 || param_1 == 0x21d) { uVar4 = 5; goto apply; }
    if (param_1 == 0x19e || param_1 == 0x235 || (param_1 >= 0x369 && param_1 <= 0x36a)) goto fallthrough;
    if (param_1 == 0x221) { uVar4 = 9; goto apply; }
    if (param_1 == 0x222) { uVar4 = 10; goto apply; }
    if (param_1 == 0x239) { uVar4 = 5; goto apply; }
    if (param_1 == 0x1af) { uVar4 = 10; goto apply; }
    if (param_1 == 0x260) { uVar4 = 6; goto apply; }
    if (param_1 == 0x1fa) { uVar4 = 9; goto apply; }
    if (param_1 == 0x31e) { uVar4 = 2; goto apply; }
    // 5.2 / original monk inventory armors:
    //   MODEL_ARMORINVEN_60 -> color 16
    //   MODEL_ARMORINVEN_61 -> color 42
    //   MODEL_ARMORINVEN_62 -> color 18
    {   // range dispatch: (param_1 - 400) / 32 → slot
        unsigned int uVar5 = (unsigned int)(param_1 - 400);
        int iVar3 = (int)(uVar5 + ((int)uVar5 >> 31 & 0x1fu)) >> 5;
        if (iVar3 >= 7 && iVar3 <= 0xb) {
            unsigned int u = uVar5 & 0x8000001fu;
            if ((int)u < 0) u = (u - 1 | 0xffffffe0u) + 1;
            switch (u) {
            case 1:  uVar4 = 1; break;
            case 3:  uVar4 = 3; break;
            case 4: case 0xe: case 0x12: uVar4 = 5; break;
            case 6:  uVar4 = 6; break;
            case 9: case 0xc: uVar4 = 2; break;
            case 0xd: uVar4 = 4; break;
            case 0xf: uVar4 = 7; break;
            case 0x10: uVar4 = 10; break;
            case 0x11: case 0x13: case 0x14: uVar4 = 9; break;
            default: break;
            }
        }
    }
fallthrough:;
apply:;
    float fVar1 = param_2 * param_3;
    switch (uVar4) {
    case 0:  param_4[2]=0.0f; param_4[0]=fVar1; param_4[1]=fVar1*_DAT_00552504; return;
    case 1:  param_4[2]=0.0f; param_4[0]=fVar1; param_4[1]=fVar1*_DAT_005526e4; return;
    case 2: case 3: { float h=fVar1*_DAT_00552504; param_4[0]=0.0f; param_4[1]=h; param_4[2]=fVar1; return; }
    case 4:  { float h=fVar1*_DAT_00552530; param_4[0]=0.0f; param_4[1]=h; param_4[2]=fVar1*_DAT_005528b4; return; }
    case 5:  param_4[0]=fVar1; param_4[1]=fVar1; param_4[2]=fVar1; return;
    case 6:  param_4[0]=fVar1*_DAT_00552534; param_4[1]=fVar1*_DAT_00552530; param_4[2]=fVar1*_DAT_005528b4; return;
    case 7:  param_4[0]=fVar1*_DAT_005526e8; param_4[1]=fVar1*_DAT_00552530; param_4[2]=fVar1; return;
    case 8:  { float h=fVar1*_DAT_00552530; param_4[0]=h; param_4[1]=h; param_4[2]=fVar1; return; }
    case 9:  { float h=fVar1*_DAT_00552504; param_4[0]=h; param_4[1]=h; param_4[2]=fVar1*_DAT_00552530; return; }
    case 10: param_4[0]=fVar1*_DAT_00552adc; param_4[1]=fVar1*_DAT_00552948; param_4[2]=fVar1*_DAT_00552504; return;
    case 0xb: { float h=fVar1*_DAT_005526e4; param_4[0]=fVar1; param_4[1]=h; param_4[2]=h; return; }
    default: return;
    }
}

// FUN_00503fe0 @ 0x00503FE0 — Weapon_SetColorAlt: simpler color selector, no flag param.
// Scales existing color[3] in place by (scale * half_scale).
void __cdecl FUN_00503fe0(int param_1, float param_2, float param_3, float *param_4)
{
    int iVar4 = 0;
    if ((param_1==0x215)||(param_1==0x21d)||(param_1==0x19e)||(param_1==0x235)) { iVar4=2; }
    else if (param_1==0x1a2) { iVar4=0; }
    else if (param_1==0x221||param_1==0x239) { iVar4=0; }
    else if (param_1==0x1a4) { iVar4=1; }
    else {
        unsigned int uVar3 = (unsigned int)(param_1 - 400);
        int iVar2 = (int)(uVar3 + ((int)uVar3 >> 31 & 0x1fu)) >> 5;
        if (iVar2 >= 7 && iVar2 <= 0xb) {
            unsigned int u = uVar3 & 0x8000001fu;
            if ((int)u < 0) u = (u - 1 | 0xffffffe0u) + 1;
            switch (u) {
            case 0x12: iVar4=2; break;
            case 4: case 0xe: case 0xf: case 0x11: iVar4=0; break; // (break → iVar4=1 after)
            default: iVar4=0; break;
            }
        }
    }
    float fVar1 = param_2 * param_3;
    if (iVar4 == 0) {
        param_4[0] = fVar1 * param_4[0];
        param_4[1] = fVar1 * param_4[1];
        param_4[2] = fVar1 * param_4[2];
    } else if (iVar4 == 1) {
        param_4[2] = 0.0f;
        param_4[0] = fVar1 * param_4[0];
        param_4[1] = fVar1 * param_4[1] * _DAT_00552504;
    } else if (iVar4 == 2) {
        param_4[0] = 0.0f;
        param_4[1] = fVar1 * param_4[1] * _DAT_00552504;
        param_4[2] = fVar1 * param_4[2];
    }
}

// FUN_00504960 @ 0x00504960 — Entity_SetModelColor: weapon type → color/alpha → render.
// Sets model color at +0x48..+0x50. Special cases for type 0x144 (two-tone), 0x1d7
// (sets entity +0x58=2 then resets to -1 afterward), 0x235 (FUN_00441e00 with extra arg).
// Falls through to FUN_00441e00 for bone rendering.
void* __cdecl FUN_00504960(void *model, int entity, int etype, float scale,
                            int flags, float alpha, int rgba)
{
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else if (etype == 0x144) {
        if ((flags & 0x100) == 0) {
            color[0] = 0.2f; color[1] = 0.2f; color[2] = 0.8f;
        } else {
            flags -= 0x100;
            color[0] = 1.0f; color[1] = 0.1f; color[2] = 0.1f;
        }
        FUN_00441e00(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), -1, (uint)rgba);
        return nullptr;
    } else {
        FUN_00503cf0(etype, scale, alpha, color, (char)((flags >> 8) & 1));
    }
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = 2;
    } else if (etype == 0x235) {
        FUN_00441e00(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), 1, (uint)rgba);
        return nullptr;
    } else if (etype != 0x1af && etype != 0x1fa && etype != 0x260) {
        FUN_00441e00(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), -1, (uint)rgba);
        return nullptr;
    }
    int fVar1 = *(int*)(entity + 0x58);
    FUN_00441e00(model, (uint)flags, scale,
                 *(int *)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = -1;
    }
    return nullptr;
}

// FUN_00504ac0 @ 0x00504AC0 — Entity_SetModelColorAlt: simpler version.
// No special type 0x144 path; uses FUN_00503fe0 instead of FUN_00503cf0.
void* __cdecl FUN_00504ac0(void *model, int entity, int etype, float scale,
                             int flags, float alpha, int rgba)
{
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else {
        FUN_00503fe0(etype, scale, alpha, color);
    }
    int fVar1 = (etype == 0x235) ? 1 : -1;   // HiddenMesh: malla 1 / ninguna
    FUN_00441e00(model, (uint)flags, scale,
                 *(int *)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    return nullptr;
}
// FUN_00455430 @ 0x00455430 — RenderLinkObject (COMPLETO)
// Implemented in src/Render/RenderLinkObject.cpp
// FUN_00449840 @ 0x00449840 — Entity_ClearBoneLinks(param1, param2, param3)
// Clears bone/widget link arrays on entity objects.
// For param2: iterates (+0x184, count at +0x180), calls FUN_004086e0 + vtable[0](3) per entry.
// For param1: iterates 6 weapon/equip slots (stride 0x18 at +0x1f4), calls FUN_004086e0 + vtable[0](1).
// For param3: clears one link at +0x14 via FUN_004086e0 + vtable[0](1).
// FUN_004086e0 signature: (int, int, int) — called here as (ptr, 0, 0) (3-arg form, per functions.h).
void __cdecl FUN_00449840(int param_1, int param_2, int param_3)
{
    if ((param_2 != 0) && (*(int*)(param_2 + 0x184) != 0)) {
        int count = (int)(unsigned char)*(char*)(param_2 + 0x180);
        int *puVar1 = (int*)*(int*)(param_2 + 0x184);
        for (int i = 0; i < count; i++) {
            FUN_004086e0((int)puVar1, 0, 0);
            puVar1 += 0x15;
        }
        int *base = (int*)*(int*)(param_2 + 0x184);
        if (base != nullptr) {
            FUN_0045aaa0_impl(base, 3);   // vtable[0] = sub_45AAA0
        }
        *(int*)(param_2 + 0x184) = 0;
        *(char*)(param_2 + 0x180) = '\0';
    }
    if (param_1 != 0) {
        int *piVar4 = (int*)(param_1 + 0x1f4);
        for (int i = 0; i < 6; i++) {
            int *puVar1 = (int*)*piVar4;
            if (puVar1 != nullptr) {
                FUN_004086e0((int)puVar1, 0, 0);
                (**(void (__cdecl**)(int))*puVar1)(1);
                *piVar4 = 0;
            }
            piVar4 += 6;
        }
    }
    if ((param_3 != 0) && (*(int*)(param_3 + 0x14) != 0)) {
        int *puVar1 = (int*)*(int*)(param_3 + 0x14);
        FUN_004086e0((int)puVar1, 0, 0);
        (**(void (__cdecl**)(int))*puVar1)(1);
        *(int*)(param_3 + 0x14) = 0;
    }
}
// FUN_004f8bb0 @ 0x004F8BB0 — Particle_DrawBillboard: draws a tiled billboard quad in world space.
// Loads texture (param_1), sets GL color, computes tile grid from scale/position,
// transforms each tile corner via Vector_Rotate (bone matrix), calls FUN_004f8740 per tile.
// Uses __ftol for int grid coords from float positions.
void* __cdecl FUN_004f8bb0(int type, float x, float y, float sx, float sy, float *col, float angle, float alpha)
{
    if (alpha == _DAT_0055256c)
        glColor3fv(col);
    else
        glColor4f(col[0], col[1], col[2], alpha);

    float rot_data[3] = { 0.0f, 0.0f, angle };
    float rot_mat[12];
    Matrix_BuildFromEuler(rot_data, rot_mat);
    GL_BindTextureSlot(type);

    float cx = x * _DAT_00552594;
    float cy = y * _DAT_00552594;
    int icx = (int)cx;
    int icy = (int)cy;
    float size = (sx > sy) ? sx : sy;
    if (size <= 0.0f)
        return nullptr;
    float half = size * _DAT_00552504;
    float inv  = _DAT_0055256c / size;
    float xScale = (sy != 0.0f) ? (sx / sy) : 1.0f;
    int   itiles = (int)half + 1;
    float loopRadius = (float)itiles;
    float oneTile = _DAT_0055256c;

    for (float tj = -loopRadius; tj <= loopRadius; tj += oneTile) {
        float v0 = ((tj + (float)icy - cy) + half) * inv;
        float v1 = ((tj + oneTile + (float)icy - cy) + half) * inv;
        for (float ti = -loopRadius; ti <= loopRadius; ti += oneTile) {
            float corners[4][3];
            float u0 = ((ti + (float)icx - cx) + half) * inv;
            float u1 = ((ti + oneTile + (float)icx - cx) + half) * inv;
            float inp[4][3] = {
                {u0 - _DAT_00552504, v0 - _DAT_00552504, 0.0f},
                {u1 - _DAT_00552504, v0 - _DAT_00552504, 0.0f},
                {u1 - _DAT_00552504, v1 - _DAT_00552504, 0.0f},
                {u0 - _DAT_00552504, v1 - _DAT_00552504, 0.0f},
            };
            for (int k = 0; k < 4; k++) {
                float out[3];
                Vector_Rotate(inp[k], rot_mat, out);
                corners[k][0] = xScale * out[0] + _DAT_00552504;
                corners[k][1] = out[1] + _DAT_00552504;
                corners[k][2] = out[2];
            }
            FUN_004f8740((float)icx + ti, (float)icy + tj, 1.0f, 1,
                         (int)corners, '\0', alpha);
        }
    }
    return nullptr;
}
