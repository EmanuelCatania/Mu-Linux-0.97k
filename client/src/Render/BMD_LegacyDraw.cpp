// BMD_LegacyDraw.cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.

// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

// IDA Hex-Rays intrinsic shims.
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


// FUN_004414d0 @ 0x004414D0 — BMD_DrawBoneSlot_Anim
// Renders polygons for a mesh slot using bone-animated vertex positions.
// unaff_EBX in original = mesh entry = model.Actions[frame], same as iVar7 computed below.
void __cdecl FUN_004414d0(void *model, char a, int b, float frame, int flags,
                           float f3, int f4, float f5, float f6, float f7, unsigned int rgba)
{
    if (!model) return;

    // Mesh entry = Actions[frame] at stride 0x28
    int meshEntry = *(int *)((int)model + 0x28) + (int)frame * 0x28;

    // Texture index from bone index lookup
    unsigned int texIdx = (unsigned int)*(short *)(*(int *)((int)model + 0x38) +
                           *(short *)(meshEntry + 2) * 2);
    if (texIdx == 300) return;  // BITMAP_HIDE
    if (*(short *)(meshEntry + 10) == 0) return;  // no polygons

    // Override texture with rgba param if not 0xffffffff
    if (rgba != 0xffffffff) texIdx = rgba;

    int bVar3 = (int)(unsigned char)((unsigned int)flags & 0xFF);
    // BlendMesh viaja en los BITS del float (los callers pasan *(float*)(o+100),
    // que es un int). -1 = ninguna; Queen Rainer usa -2.
    int blendMeshInt = f4;   // IDA a5: entero (indice de malla o -1)

    // v58 de IDA: LightEnable del modelo; algunas ramas lo apagan.
    int lightEnable = *(unsigned char *)((int)model + 0x44);
    {
        int v14 = *(signed char *)((int)model + 0x88);
        if ((int)frame == v14) {
            glColor3fv((const float *)((int)model + 0x48));
            lightEnable = 0;
        }
    }

    // a7 de IDA: el MODO que fija el bloque de estado de abajo y que despues
    // gatea la emision de texcoords. NO es `flags`.
    //
    // El port gateaba con `flags == 2` / `flags == 4`, pero a esta funcion solo
    // se entra con (flags & 0x400) puesto, asi que esas comparaciones NUNCA eran
    // ciertas: no se emitia glTexCoord2f y toda malla dibujada por aca salia de
    // color plano. En IDA el default del bloque de estado es `a7 = 2`.
    int mode = 2;

    // GL state setup
    if ((bVar3 & 1) == 1) {
        mode = 1;
        if ((bVar3 & 0x40) == 0x40)      GL_SetBlendAdditive();
        else if ((bVar3 & 0x80) == 0x80) GL_SetBlendSrcAlpha();
        else                             GL_ResetState();
        GL_SetAlphaTest('\0');
        glColor3fv((float *)((int)model + 0x48));
    } else if (blendMeshInt <= -2 || *(short *)(meshEntry + 2) == blendMeshInt) {
        // IDA sub_4414D0: rama que faltaba entera, y va ANTES de la de (flags & 2).
        //
        //   else if ( a7 <= -2 || *(__int16 *)(v13 + 2) == a7 ) {
        //       a7 = 2;
        //       BindTexture(tex);
        //       (v21 & 0x80) ? EnableAlphaBlendMinus() : EnableAlphaBlend();
        //       glColor3f(a8 * this[72], a8 * this[76], a8 * this[80]);
        //   }
        //
        // a7 = BlendMesh (entero, llega en los BITS del float) y a8 =
        // BlendMeshLight. Sin esta rama la malla caia en la de (flags & 2), que
        // usa blending NORMAL y no setea color: quedaba una silueta oscura.
        // Aca va ADITIVO y con color = BlendMeshLight * BodyLight.
        //
        // Queen Rainer (ModelID 321) tiene BlendMesh = -2 (CreateMonster case 70),
        // asi que su malla 1 — el vestido — entra por aca.
        GL_BindTextureSlot(texIdx);
        if ((bVar3 & 0x80) == 0x80) GL_SetBlendSrcAlpha();   // EnableAlphaBlendMinus (0x511790)
        else                        GL_SetBlendAdditive();   // EnableAlphaBlend    (0x511710)
        {
            const float *bodyLight = (const float *)((int)model + 0x48);
            glColor3f(f7 * bodyLight[0], f7 * bodyLight[1], f7 * bodyLight[2]);
        }
        mode = 2;
        lightEnable = 0;          // IDA: v58 = 0 en esta rama
    } else if ((bVar3 & 2) == 2) {
        mode = 2;
        GL_BindTextureSlot(texIdx);
        if ((bVar3 & 0x40) == 0x40)      GL_SetBlendAdditive();
        else if ((bVar3 & 0x80) == 0x80) GL_SetBlendSrcAlpha();
        else                             GL_ResetState();
    } else if ((bVar3 & 0x40) == 0x40) {
        if (texIdx == 4) return;  // (&DAT_083a7cc8)[local_24 * 0x38] == 4 early-out
        mode = 64;
        GL_SetBlendAdditive();
        GL_SetAlphaTest('\0');
        GL_DisableDepthWrites();
    }
    // else param_6 = 2.8026e-45 — no extra state

    // (HashTable obfuscation block skipped — pure ref-count noise)

    glBegin(GL_TRIANGLES);

    int polyCount = *(short *)(meshEntry + 10);
    int param_5_i = 0;
    for (int local_20 = 0; local_20 < polyCount; local_20++) {
        char *pcVar10 = (char *)(param_5_i + *(int *)(meshEntry + 0x1c));
        if (*pcVar10 > 0) {
            int deformFlag = b & 1;
            short *psVar15 = (short *)(pcVar10 + 10);
            for (int vi = 0; vi < (int)*pcVar10; vi++, psVar15++) {
                int iVar7 = (int)psVar15[-4];

                if (mode == 2) {
                    // Textured: UV from UV array
                    float *uvPtr = (float *)(*(int *)(meshEntry + 0x18) + (int)psVar15[4] * 8);
                    float uCoord, vCoord;
                    if (f5 == 0.0f) {
                        uCoord = *uvPtr;
                        vCoord = uvPtr[1];
                    } else {
                        uCoord = f5 + *uvPtr;
                        vCoord = f6 + uvPtr[1];
                    }
                    glTexCoord2f(uCoord, vCoord);
                    if (lightEnable) {
                        int iVar13 = ((int)*psVar15 + (int)frame * 15000) * 0xc;
                        if (f3 < _DAT_00552544) {
                            glColor4f(*(float *)(&DAT_060db65c + iVar13),
                                      *(float *)(&DAT_060db65c + iVar13 + 4),
                                      *(float *)(&DAT_060db65c + iVar13 + 8), f3);
                        } else {
                            glColor3fv((float *)(&DAT_060db65c + iVar13));
                        }
                    }
                } else if (mode == 4) {
                    // Chrome UV
                    if (f3 < _DAT_00552544) {
                        glColor4f(*(float *)((int)model + 0x48), *(float *)((int)model + 0x4c),
                                  *(float *)((int)model + 0x50), f3);
                    } else {
                        glColor3fv((float *)((int)model + 0x48));
                    }
                    // BUG-FIX 2026-07-15: el V leía `&DAT_05828d5c + (idx*2+1)*4`
                    // (stride ×4, Ghidra float→byte mis-decompile) → out-of-bounds /
                    // UV degenerado. La tabla es {U,V} contigua: V = índice idx*2+1.
                    glTexCoord2f((&DAT_05828d5c)[*psVar15 * 2],
                                 (&DAT_05828d5c)[*psVar15 * 2 + 1]);
                }

                // Vertex position
                float *pfVar5;
                float afStack_14[3];
                if (deformFlag) {
                    // Sin-wave deformation
                    // IDA: v56 = (__int64)WorldTime + 931 * v33;  (v33 = indice
                    // de vertice). El port ponia `frame` — el indice de MALLA —
                    // donde va WorldTime, asi que la onda quedaba congelada.
                    int iVar13 = (long long)DAT_05826e08 + 0x3a3 * iVar7;
                    float sinVal = (float)fsin((double)iVar13 * (double)_DAT_005528c4);
                    float *pfVar12 = (float *)((char*)&DAT_0584621c + ((int)frame * 15000 + iVar7) * 3 * 4);
                    int normBase = ((int)*psVar15 + (int)frame * 15000) * 0xc;
                    for (int k = 0; k < 3; k++) {
                        afStack_14[k] = sinVal * *(float *)(&DAT_06f433bc + normBase + k * 4)
                                        * _DAT_00552644 + pfVar12[k];
                    }
                    pfVar5 = afStack_14;
                } else {
                    pfVar5 = (float *)((char*)&DAT_0584621c + (iVar7 + (int)frame * 15000) * 3 * 4);
                }
                glVertex3fv(pfVar5);
            }
        }
        param_5_i += 0x24;
    }

    glEnd();
    // f4 = BlendMesh y f7 = BlendMeshLight: ya se consumen en la rama de estado.
}

// FUN_004e13a0 @ 0x004E13A0 — RenderObjectScreen
// Renders a 3D item/object at world position param_4[0..2].
// Sets rotation globals per type, builds a stack entity, calls BMD_Animation + Entity_DrawAt.
static bool ApplyInventoryExactPoseLate(int param_1, int level, float* outPos)
{
    switch (param_1) {
    case MODEL_SWORD + 0:
        outPos[0] -= 0.02f; outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 0:
        outPos[1] += 0.05f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_BOW + 7:
    case MODEL_BOW + 15:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 17:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 20:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 21:
        outPos[1] += 0.12f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 22:
    case MODEL_BOW + 23:
        outPos[0] -= 0.10f; outPos[1] += 0.08f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 10:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELM + 30:
        outPos[0] -= 0.03f; outPos[1] += 0.07f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 31:
        outPos[0] += 0.03f; outPos[1] -= 0.06f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 35:
        outPos[0] -= 0.02f; outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 5:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 6:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 7:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 10:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 11:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = -20.0f; return true;
    case MODEL_EVENT + 12:
        _DAT_07ea952c = 250.0f; _DAT_07ea9530 = 140.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 14:
        _DAT_07ea952c = 255.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 15:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 16:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 18:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 3:
    case MODEL_HELPER + 4:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 5:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -35.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 16:
    case MODEL_HELPER + 17:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 18:
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 21:
    case MODEL_HELPER + 22:
    case MODEL_HELPER + 23:
    case MODEL_HELPER + 24:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELPER + 29:
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 30:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 12:
        if (level == 0) { _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 1) { _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 2) { _DAT_07ea952c = 90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        return true;
    case MODEL_STAFF + 7:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 205.0f; return true;
    case MODEL_STAFF + 12:
        outPos[0] += 0.025f; outPos[1] -= 0.10f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_STAFF + 13:
        outPos[0] += 0.02f; outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_POTION + 20:
    case MODEL_POTION + 27:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 13:
    case MODEL_POTION + 14:
    case MODEL_POTION + 22:
        outPos[0] += 0.005f; outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 21:
        outPos[0] += 0.005f; outPos[1] -= 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 41:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 42:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 43:
    case MODEL_POTION + 44:
        outPos[0] -= 0.04f; outPos[1] += 0.02f; outPos[2] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = -45.0f; return true;
    case MODEL_POTION + 63:
        outPos[1] += 0.08f;
        _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_SWORD + 26:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_SWORD + 27:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SWORD + 28:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_MACE + 16:
        outPos[0] -= 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 17:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 18:
        outPos[0] -= 0.03f; outPos[1] += 0.06f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 2.0f; return true;
    case MODEL_MACE + 14:
        outPos[0] -= 0.01f; outPos[1] += 0.10f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_MACE + 15:
        outPos[1] += 0.05f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_SPEAR + 11:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_STAFF + 30:
    case MODEL_STAFF + 31:
    case MODEL_STAFF + 32:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 33:
        outPos[0] += 0.02f; outPos[1] -= 0.06f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 34:
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_BOW + 24:
        outPos[0] -= 0.07f; outPos[1] += 0.07f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_HELPER + 39:
    case MODEL_HELPER + 40:
    case MODEL_HELPER + 41:
    case MODEL_HELPER + 42:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    default:
        if (param_1 >= MODEL_HELPER + 12 && param_1 < MODEL_HELPER + 512 &&
            param_1 != MODEL_HELPER + 12 && param_1 != MODEL_HELPER + 13 &&
            param_1 != MODEL_HELPER + 14 && param_1 != MODEL_HELPER + 15) {
            _DAT_07ea952c = 360.0f;
            _DAT_07ea9530 = 0.0f;
            _DAT_07ea9534 = 0.0f;
            return true;
        }
        if (param_1 == MODEL_ARMOR + 29) {
            outPos[1] += 0.07f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 30) {
            outPos[1] += 0.10f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 34) {
            outPos[1] += 0.03f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 35) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 36 || param_1 == MODEL_ARMOR + 37) {
            outPos[1] -= 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_HELM + 39 && param_1 <= MODEL_HELM + 44) {
            outPos[1] -= 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_ARMOR + 38 && param_1 <= MODEL_ARMOR + 44) {
            outPos[1] -= 0.08f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_STAFF + 21 && param_1 <= MODEL_STAFF + 29) {
            _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 130 && param_1 <= MODEL_POTION + 132) {
            outPos[1] += 0.06f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 133) {
            outPos[0] += 0.01f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 134 && param_1 <= MODEL_POTION + 139) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 140) {
            outPos[1] += 0.09f;
            _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 52) {
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 63) {
            outPos[1] += 0.08f;
            _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 160 || param_1 == MODEL_POTION + 161) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 145 && param_1 <= MODEL_POTION + 150) {
            outPos[0] += 0.01f; outPos[1] += 0.04f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
    return false;
}

}

#if 0
#if 0 // Superseded by the literal 0.97k RenderObjectScreen port below.
void __cdecl FUN_004e13a0(int param_1, unsigned int param_2, unsigned char param_3, unsigned char param_4, float *param_5, int param_6, char param_7)
{
    // 2026-05-08: per-call recovery. Esta función se llama MUCHAS veces por
    // frame (una por cada item 3D del inventario). El watchdog en Render_GameFrame
    // sólo recupera 1 vez por frame; si la corrupción de DAT_07d78068 ocurre
    // entre dos calls del mismo frame, las posteriores crashean. Recuperar
    // inline antes del read.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if (p < 0x100000u || p >= 0x80000000u) {
            if (g_ItemAttribute_Backup >= 0x100000u && g_ItemAttribute_Backup < 0x80000000u) {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            } else {
                return;   // can't recover — bail
        }
}

#endif
static bool ApplyInventoryExactPoseTail(int param_1, float* outPos)
{
    if (param_1 == MODEL_POTION + 96) {
        outPos[0] += 0.003f; outPos[1] -= 0.013f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 99) {
        outPos[0] += 0.02f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = -40.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 100) {
        outPos[0] += 0.01f; outPos[1] -= 0.05f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 76) {
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 80 || param_1 == MODEL_HELPER + 123) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 40.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 81 || param_1 == MODEL_HELPER + 82) {
        outPos[0] += 0.005f; outPos[1] += 0.035f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 93 || param_1 == MODEL_HELPER + 94) {
        outPos[0] += 0.005f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 97 || param_1 == MODEL_HELPER + 98) {
        outPos[0] += 0.002f; outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 99) {
        outPos[0] += 0.002f; outPos[1] += 0.025f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 103) {
        outPos[0] += 0.01f; outPos[1] += 0.01f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 104 || param_1 == MODEL_HELPER + 105) {
        outPos[0] += 0.01f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 106) {
        outPos[0] += 0.01f; outPos[1] -= 0.05f;
        _DAT_07ea952c = 255.0f; _DAT_07ea9530 = 45.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 107) {
        _DAT_07ea952c = 90.0f; _DAT_07ea9530 = 225.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 109 && param_1 <= MODEL_HELPER + 112) {
        outPos[0] += 0.025f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 25.0f; _DAT_07ea9534 = 25.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 113 && param_1 <= MODEL_HELPER + 115) {
        outPos[0] += 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 116) {
        outPos[0] += 0.005f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 121) {
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 122) {
        outPos[0] += 0.01f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 124) {
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 125 && param_1 <= MODEL_HELPER + 127) {
        outPos[0] += 0.007f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 128 || param_1 == MODEL_HELPER + 131 || param_1 == MODEL_HELPER + 133) {
        outPos[0] += 0.017f; outPos[1] -= 0.053f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 129) {
        outPos[0] += 0.012f; outPos[1] -= 0.045f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 130) {
        outPos[0] += 0.007f; outPos[1] += 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 132) {
        outPos[0] += 0.007f; outPos[1] += 0.045f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 134) {
        outPos[0] += 0.005f; outPos[1] -= 0.033f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    // 2026-08-26: acotado para no pisar el 958, que IDA define aparte
    // (`case 958: Angle[0] = -90`). Este rango son los modelos 951..961.
    if (param_1 >= MODEL_HELPER + 135 && param_1 <= MODEL_HELPER + 145 && param_1 != 958) {
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 30 || param_1 == MODEL_WING + 31 ||
        (param_1 >= MODEL_WING + 136 && param_1 <= MODEL_WING + 143)) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f;
        if (param_1 == MODEL_WING + 142 || param_1 == MODEL_WING + 143) _DAT_07ea9534 = -45.0f;
        else if (param_1 == MODEL_WING + 136 || param_1 == MODEL_WING + 137) outPos[1] -= 0.05f;
        else if (param_1 == MODEL_WING + 139) { outPos[1] -= 0.05f; _DAT_07ea9530 = 90.0f; }
        return true;
    }
    // 2026-08-26: acotado. `MODEL_WING + 60..65` son los modelos 844..849, que
    // tampoco son alas. IDA cubre 828..847 (salvo 830/831) con `Angle[0] = 360`
    // y este interceptor les ponia 10. Se dejan pasar los que IDA define y solo
    // sobreviven 848/849, que caen fuera de ese rango.
    if (param_1 >= 848 && param_1 <= MODEL_WING + 65) {
        _DAT_07ea952c = 10.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 10.0f; return true;
    }
    if (param_1 >= MODEL_WING + 70 && param_1 <= MODEL_WING + 74) {
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_WING + 100 && param_1 <= MODEL_WING + 129) {
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    // 2026-08-26: excluido el modelo 833. `MODEL_WING + 49` es 833, que no es
    // un ala (MODEL_WING son 32 slots, 784..815): es el "Blood Bone" (item 433).
    // IDA `RenderObjectScreen` lo cubre con `case 832: case 833: Angle[0]=270`,
    // y este interceptor le ponia -90 mas un ajuste de posicion que IDA no
    // tiene. Mismo error de aritmetica que tenia el +50 (Cloak of Invisibility).
    if (false && param_1 == MODEL_WING + 49) {
        outPos[0] += 0.015f; outPos[1] += 0.01f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    // REMOVIDO 2026-08-26 — `MODEL_WING + 50` es 834, que NO es un ala.
    //
    // MODEL_WING vale 784 y el grupo de alas son 32 slots (784..815), asi que
    // el +50 se pasa de su propia categoria y cae sobre el 834 = "Cloak of
    // Invisibility" (item 434, la entrada del Blood Castle). El interceptor le
    // hacia dos cosas, ninguna de las cuales esta en IDA:
    //
    //   outPos[1] += 0.15f;                          <- lo dibujaba mas arriba
    //   Angle = (270, -10, 0);                       <- IDA dice (290, 0, 0)
    //
    // y encima cortaba con `return true`, tapando el `else if (param_1 ==
    // 0x342)` de RenderObjectScreen que tiene los valores correctos. El
    // decompile de 0x4E13A0 no tiene ningun `Position[1] +=` por tipo: el 834
    // solo setea `Angle[0] = 290` y hace `goto LABEL_51`.
    //
    // Medido con la sonda OBJ3D antes del fix:
    //     OBJ3D model=834 scale=1800/1e6 bodyH=0 ang=(270,-10,0)
    // La escala (0.0018) y el BodyHeight (0) ya salian bien — el unico
    // desvio era el angulo, y el desplazamiento de posicion que no se ve en
    // ese volcado porque se aplica sobre outPos.
    //
    // OJO: quedan varios `MODEL_WING + N` con N > 31 en esta misma funcion
    // (60-65, 70-74, 100-129, 135, 136-143) que por la misma aritmetica caen
    // sobre modelos de otras categorias. No se tocan porque no hay reporte
    // sobre ellos, pero son sospechosos del mismo error.
    if (param_1 == MODEL_WING + 135) {
        outPos[0] += 0.005f; outPos[1] += 0.05f; return true;
    }
    if (param_1 == MODEL_POTION + 52) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -25.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_ETC + 19 && param_1 <= MODEL_ETC + 27) {
        outPos[0] += 0.03f; outPos[1] += 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 38) {
        outPos[1] += 0.02f;
        _DAT_07ea952c = -198.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 63) {
        outPos[1] += 0.08f;
        _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 7) {
        outPos[0] += 0.005f; outPos[1] -= 0.015f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_ETC + 30 && param_1 <= MODEL_ETC + 36) {
        outPos[0] += 0.03f; outPos[1] += 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    // 2026-08-26: excluido el modelo 958 — IDA tiene `case 958: Angle[0] = -90`
    // (con dword_7EA9530/34 = -0.5) y este interceptor le ponia 270.
    if (false && param_1 == MODEL_POTION + 110) {
        outPos[0] += 0.005f; outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 111) {
        outPos[0] += 0.01f; outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 112 && param_1 <= MODEL_POTION + 113) {
        outPos[0] += 0.05f; outPos[1] += 0.009f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 114 && param_1 <= MODEL_POTION + 119) {
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 120) {
        outPos[0] += 0.01f; outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 126 && param_1 <= MODEL_POTION + 129) {
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    return false;
}
void __cdecl FUN_004e13a0(int param_1, unsigned int param_2, unsigned char param_3, unsigned char param_4, float *param_5, int param_6, char param_7)
{
    // 2026-05-08: per-call recovery. Esta función se llama muchas veces por frame.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if (p < 0x100000u || p >= 0x80000000u) {
            if (g_ItemAttribute_Backup >= 0x100000u && g_ItemAttribute_Backup < 0x80000000u) {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            } else {
                return;
            }
        }
    }
    float direction[3];
    direction[0] = param_5[0] - _CameraRayOriginX;
    direction[1] = param_5[1] - _CameraRayOriginY;
    direction[2] = param_5[2] - _CameraRayOriginZ;

    float outPos[3];
    float camPos[3] = { _CameraRayOriginX, _CameraRayOriginY, _CameraRayOriginZ };
    FUN_004f9ce0(camPos, param_7 ? 0.07f : 0.1f, direction, outPos);

    // ── Posición fiel a IDA (2026-08-26) ────────────────────────────────────
    // En `RenderObjectScreen` (0x4E13A0) la posición se calcula UNA sola vez,
    // con este mismo VectorMA, y se lee al final sin tocarla: el decompile no
    // tiene NI UN `Position[i] +=` por tipo (verificado sobre el raw entero).
    //
    // Este port, en cambio, acumula 193 ajustes `outPos[i] += 0.0xx` repartidos
    // entre esta función y los dos helpers `ApplyInventoryExactPose*`. No salen
    // de ningún decompile — el comentario de uno de ellos lo admite ("raw ids
    // verified from asset table", o sea a ojo contra los assets). Son los que
    // producían el sesgo reportado: los ítems con offset negativo (jewels,
    // Devil's Invitation, Jewel of Life) se dibujaban más abajo y los que tenían
    // offset positivo (el Cloak of Invisibility con +0.15) más arriba.
    //
    // Se guarda la posición limpia acá y se restaura justo antes de escribirla
    // en el OBJECT, que es un único punto de control en vez de tocar 193 sitios.
    // Poner el flag en 0 devuelve el comportamiento viejo para comparar A/B.
    #define ITEM3D_FAITHFUL_POSITION 1
    const float rayPos[3] = { outPos[0], outPos[1], outPos[2] };

    // Book01..Book16 have no position adjustment in the original renderer.
    // Keep its ray-projected location before the reconstructed generic table
    // below has a chance to apply overlapping potion-model adjustments.
    const bool nativeBook = param_1 >= 880 && param_1 <= 895;
    const float nativeBookPos[3] = { outPos[0], outPos[1], outPos[2] };

    // ── ITEM3D: ver el bloque SCALE3D más abajo (después de resolver local_3bc).
    // ── (round-trip ya verificado: proj_pos == proj_tgt, cadena consistente)
#if 0
    // ── ITEM3D (temporal): prueba de ida y vuelta del descentrado de items.
    // `Projection` (0x5113F0) es el inverso exacto de `CreateScreenVector`, así
    // que re-proyectar a pantalla debe devolver las MISMAS coords que entraron.
    //   · Target y outPos deben proyectar al MISMO punto: el lerp va a lo largo
    //     del rayo de visión, así que sólo conserva la posición en pantalla si
    //     `MousePosition` (CameraRayOriginX) es de verdad el ojo de la cámara.
    //   · Si difieren, el corrimiento medido ES el bug y su magnitud dice cuánto.
    {
        static DWORD s_lastI = 0;
        DWORD nowI = GetTickCount();
        if (nowI - s_lastI > 1000) {
            s_lastI = nowI;
            int tx = 0, ty = 0, px = 0, py = 0;
            Camera_ProjectWorldToScreen(param_5, &tx, &ty);   // Target  → pantalla
            Camera_ProjectWorldToScreen(outPos,  &px, &py);   // Position→ pantalla
            char ib[240];
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "ITEM3D type=%d cam=(%.1f,%.1f,%.1f) tgt=(%.1f,%.1f,%.1f) "
                "pos=(%.1f,%.1f,%.1f) proj_tgt=(%d,%d) proj_pos=(%d,%d) d=(%d,%d)",
                (int)param_1, camPos[0], camPos[1], camPos[2],
                param_5[0], param_5[1], param_5[2],
                outPos[0], outPos[1], outPos[2],
                tx, ty, px, py, px - tx, py - ty);
            DbgLogPublic(ib);
        }
    }
#endif

    int level = ((int)param_2 >> 3) & 0xf;
    short modelType = (short)param_1;

    // Default rotation values (overridden below)
    _DAT_07ea952c = 0.0f;
    _DAT_07ea9530 = 0.0f;
    _DAT_07ea9534 = 0.0f;

    DWORD local_3bc = 0;  // model scale (raw IEEE 754 float bits)
    bool exactPose = false;

    if (param_1 == 0x190) { // MODEL_SWORD+0
        outPos[0] -= 0.02f;
        outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x1f0) { // MODEL_SPEAR+0
        outPos[1] += 0.05f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 20.0f;
        exactPose = true;
    } else if (param_1 == 0x224) { // MODEL_BOW+20
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x225) { // MODEL_BOW+21
        outPos[1] += 0.12f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x226 || param_1 == 0x227) { // MODEL_BOW+22/+23
        outPos[0] -= 0.10f;
        outPos[1] += 0.08f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x23c) { // MODEL_STAFF+12
        outPos[1] -= 0.10f;
        outPos[0] += 0.025f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 8.0f;
        exactPose = true;
    } else if (param_1 == 0x23d) { // MODEL_STAFF+13
        outPos[0] += 0.02f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 8.0f;
        exactPose = true;
    } else if (param_1 >= 0x245 && param_1 <= 0x24d) { // MODEL_STAFF+21..29
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x1de) { // MODEL_MACE+14
        outPos[1] += 0.10f;
        outPos[0] -= 0.01f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 13.0f;
        exactPose = true;
    } else if (param_1 == 0x1df) { // MODEL_MACE+15
        outPos[1] += 0.05f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 13.0f;
        exactPose = true;
    } else if (param_1 == 0x28e) { // MODEL_HELM+30
        outPos[1] += 0.07f;
        outPos[0] -= 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x28f) { // MODEL_HELM+31
        outPos[1] -= 0.06f;
        outPos[0] += 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2ad) { // MODEL_ARMOR+29
        outPos[1] += 0.07f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2ae) { // MODEL_ARMOR+30
        outPos[1] += 0.10f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x293) { // MODEL_HELM+35
        outPos[0] -= 0.02f;
        outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x297 && param_1 <= 0x29c) { // MODEL_HELM+39..44
        outPos[1] -= 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 25.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b2) { // MODEL_ARMOR+34
        outPos[1] += 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b3) { // MODEL_ARMOR+35
        outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b4 || param_1 == 0x2b5) { // MODEL_ARMOR+36/+37
        outPos[1] -= 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x2b6 && param_1 <= 0x2bc) { // MODEL_ARMOR+38..44
        outPos[1] -= 0.08f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x1a8) { // MODEL_SWORD+24
        outPos[0] -= 0.02f;
        outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x24b) { // MODEL_EVENT+10
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x24c) { // MODEL_EVENT+11
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -20.0f;
        _DAT_07ea9534 = -20.0f;
        exactPose = true;
    } else if (param_1 == 0x24d) { // MODEL_EVENT+12
        _DAT_07ea952c = 250.0f;
        _DAT_07ea9530 = 140.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x24f) { // MODEL_EVENT+14
        _DAT_07ea952c = 255.0f;
        _DAT_07ea9530 = 160.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x250) { // MODEL_EVENT+15
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x251) { // MODEL_EVENT+16
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_STAFF + 7) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 205.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 0) {
        outPos[0] += 0.002f;
        outPos[1] += 0.010f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 1 || param_1 == MODEL_HELPER + 2) {
        outPos[0] += 0.002f;
        outPos[1] += 0.008f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    // 2026-08-26 — DESACTIVADO. `MODEL_HELPER + 12/13` son los modelos 828/829
    // (Pendant of Lightning / of Fire). IDA los cubre con
    // `Type >= 828 && Type < 848 && Type != 830 && Type != 831 -> Angle[0] = 360`,
    // que en este archivo está más abajo (rama `0x33c..0x34f`) y es fiel; este
    // interceptor corría antes, ponía 270 y marcaba `exactPose = true`, así que
    // la rama buena nunca se alcanzaba. Medido con la sonda OBJ3D: `ang=(270,0,0)`
    // donde IDA da 360.
    } else if (false && (param_1 == MODEL_HELPER + 12 || param_1 == MODEL_HELPER + 13)) {
        outPos[0] += 0.002f;
        outPos[1] += 0.010f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if ((param_1 >= MODEL_POTION + 1 && param_1 <= MODEL_POTION + 10) ||
               param_1 == MODEL_POTION + 13 || param_1 == MODEL_POTION + 14 || param_1 == MODEL_POTION + 22) {
        outPos[0] += 0.005f;
        outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 21) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.005f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x34e) { // MODEL_HELPER+30
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x34f) { // MODEL_HELPER+31
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x350) { // MODEL_HELPER+32
        outPos[0] += 0.01f;
        outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x351) { // MODEL_HELPER+33
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x352 || param_1 == 0x353) { // MODEL_HELPER+34/+35
        outPos[0] += 0.01f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x354) { // MODEL_HELPER+36
        outPos[0] += 0.01f;
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x355) { // MODEL_HELPER+37
        outPos[0] += 0.01f;
        outPos[1] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x35b || param_1 == 0x35d) { // raw ids verified from asset table
        outPos[1] += (param_1 == 0x35b) ? -0.027f : -0.02f;
        outPos[0] += 0.005f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x35e || param_1 == 0x35f || param_1 == 0x360 || param_1 == 0x361 || param_1 == 0x362 || param_1 == 0x363) { // MODEL_HELPER+46..51
        outPos[1] += (param_1 == 0x361) ? -0.04f : (param_1 == 0x362 ? -0.03f : (param_1 == 0x363 ? -0.02f : -0.04f));
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x364) { // MODEL_HELPER+52
        outPos[1] += 0.045f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x365) { // MODEL_HELPER+53
        outPos[1] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 120.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x366 && param_1 <= 0x36a) { // MODEL_HELPER+54..58
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36b) { // MODEL_HELPER+59
        outPos[0] += 0.01f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36c) { // MODEL_HELPER+60
        outPos[1] -= 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36d) { // MODEL_HELPER+61
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36e) { // MODEL_HELPER+62
        outPos[0] += 0.01f;
        outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36f) { // MODEL_HELPER+63
        outPos[0] += 0.01f;
        outPos[1] += 0.082f;
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x385) { // MODEL_POTION+53
        outPos[1] += 0.042f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x386) { // MODEL_POTION+54
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x38a) { // MODEL_POTION+58
        outPos[1] += 0.07f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x38b && param_1 <= 0x38e) { // MODEL_POTION+59..62
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x390) { // MODEL_POTION+64
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x396 && param_1 <= 0x397) { // MODEL_POTION+70..71
        outPos[0] += 0.01f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x398 && param_1 <= 0x39d) { // MODEL_POTION+72..77
        outPos[1] += 0.08f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x39e && param_1 <= 0x3a2) { // MODEL_POTION+78..82
        outPos[1] += 0.01f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x3a3) { // MODEL_POTION+83
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x3a4 && param_1 <= 0x3aa) { // MODEL_POTION+84..90
        if (param_1 == 0x3a4 || param_1 == 0x3a6 || param_1 == 0x3a7) outPos[1] += 0.01f;
        else if (param_1 == 0x3a5) outPos[1] -= 0.01f;
        else outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x3ab && param_1 <= 0x3af) { // MODEL_POTION+91..95
        if (param_1 == 0x3ae) outPos[0] += 0.01f; // +94
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x3b1 || param_1 == 0x3b2) { // MODEL_POTION+97..98
        outPos[1] += 0.09f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 37 || param_1 == MODEL_WING + 38 || param_1 == MODEL_WING + 40) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 39) {
        outPos[1] += 0.08f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 42) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 2.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 44 || param_1 == MODEL_WING + 45 || param_1 == MODEL_WING + 46 || param_1 == MODEL_WING + 47) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 64) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 65) {
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 66) {
        outPos[0] += 0.01f;
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 67) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 40.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 68) {
        outPos[0] += 0.02f;
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 300.0f;
        _DAT_07ea9530 = 10.0f;
        _DAT_07ea9534 = 20.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 69) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -30.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 70) {
        outPos[0] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 70.0f;
        exactPose = true;
    } else if (param_1 >= MODEL_HELPER + 71 && param_1 <= MODEL_HELPER + 75) {
        outPos[1] += 0.07f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = (param_6 == 1) ? 180.0f : (DAT_05826e08 * 0.2f);
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_ARMOR + 10 || param_1 == MODEL_ARMOR + 11) {
        outPos[1] -= 0.10f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_PANTS + 10 || param_1 == MODEL_PANTS + 11) {
        outPos[1] -= 0.08f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 65) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 66 || param_1 == MODEL_POTION + 67) {
        outPos[1] += 0.11f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose)
        exactPose = ApplyInventoryExactPoseTail(param_1, outPos);

    if (!exactPose && param_1 == MODEL_EVENT + 5) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 180.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && param_1 == MODEL_EVENT + 6) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && param_1 == MODEL_EVENT + 7) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose && (param_1 >= MODEL_POTION + 32 && param_1 <= MODEL_POTION + 34)) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && (param_1 >= MODEL_EVENT + 21 && param_1 <= MODEL_EVENT + 23)) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose)
        exactPose = ApplyInventoryExactPoseLate(param_1, (int)param_2, outPos);

    // ── BUG-FIX (2026-04-20) ─────────────────────────────────────────────────
    // Las líneas siguientes antes asignaban `_DAT_07ea952c = 0x42b40000` etc.
    // Como esos globals están tipados `float` en globals.h/cpp, C hace conversión
    // int→float: 0x42b40000 == 1'119'748'096, no 90.0f. El patrón es idéntico al
    // bug que tuvimos en _DAT_005597c8 — producía rotaciones locas (~1e9°).
    if (exactPose) {
    } else if (param_1 == 0x217 || param_1 == 0x21f) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 15.0f;
    } else if (param_1 == 0x221) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 15.0f;
    } else if (param_1 >= 0x218 && param_1 <= 0x22f) {
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 180.0f;
        _DAT_07ea9534 = 20.0f;
    } else if (param_1 == 0x1fa) {
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 20.0f;
    } else if (param_1 >= 0x250 && param_1 <= 0x26f) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 >= 0x190 && param_1 < 0x250) {
        switch (param_1) {
        case 0x23e: outPos[1] += 0.04f; break;                 // MODEL_STAFF+14
        case 0x241: outPos[0] += 0.02f; outPos[1] += 0.03f; break; // MODEL_STAFF+17
        case 0x242: outPos[0] += 0.02f; break;                 // MODEL_STAFF+18
        case 0x243: outPos[0] -= 0.02f; outPos[1] -= 0.02f; break; // MODEL_STAFF+19
        case 0x244: outPos[0] += 0.01f; outPos[1] -= 0.01f; break; // MODEL_STAFF+20
        default: break;
        }
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        if (*(char *)(param_1 * 0x40 + -0x63e2 + DAT_07d78068) != '\0')
            _DAT_07ea9534 = 25.0f;
        else
            _DAT_07ea9534 = 15.0f;
    } else if (param_1 == 0x333) {
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x342) {
        _DAT_07ea952c = 290.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x3be) {
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -20.0f;
        _DAT_07ea9534 = -20.0f;    // 0xc1a00000
    } else if (param_1 == 0x35c) {
        if (level == 0) { _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f;   _DAT_07ea9534 = 0.0f; }
        else if (level == 1) { _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 2) { _DAT_07ea952c =  90.0f; _DAT_07ea9530 = 0.0f;  _DAT_07ea9534 = 0.0f; }
    } else if (param_1 == 0x3b8 || param_1 == 0x3ba || param_1 == 0x364) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x3b9) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f;
    } else if ((param_1 >= 0x33c && param_1 <= 0x34f) &&
               param_1 != 0x33e && param_1 != 0x33f) {
        _DAT_07ea952c = 360.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f;
    } else {
        // FIX 2026-05-01: IDA fall-through (verified disasm @0x4E16FC/1728):
        // Angle=270, Y=-10, Z=0. Antes Y=0 → items rendered upside-down.
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f;
    }

    if (param_6 == 1)
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;

    _DAT_07ea9512 = modelType;
    if (modelType >= 0x270 && modelType < 0x310) {
        modelType = 390;
        _DAT_07ea9512 = 390;
    } else if (modelType == 0x35c) {
        if (level == 0) {
            param_1 = 947;
            modelType = 947;
            _DAT_07ea9512 = 947;
        } else if (level == 2) {
            param_1 = 948;
            modelType = 948;
            _DAT_07ea9512 = 948;
        }
    }

    // Set model height offset based on type range
    void *modelThis = (void *)(DAT_05828d58 + (int)modelType * 0xbc);

    // FIX 2026-05-02: faltaba `*(BYTE*)(modelThis + 0xa0) = 0`. IDA hace esto
    // al inicio para resetear el action index. Sin esto, el player pose usa el
    // action index del último frame del mundo (e.g. running) → body parts
    // posan con bones en pose de correr en lugar de idle.
    *(unsigned char *)((char *)modelThis + 0xa0) = 0;

    // BodyHeight (model + 0x84) — desplazamiento vertical por rango de modelo.
    // Valores exactos de IDA `RenderObjectScreen` (0x4E13A0), decodificados de
    // los literales del decompile:
    //   [624,656) HELM   → -1021313024 = 0xC31C0000 = -156.0
    //   [656,688) ARMOR  → -1027080192 = 0xC2C00000 =  -96.0
    //   [688,720) PANTS  → -1035468800 = 0xC2400000 =  -48.0
    //   [720,752) GLOVES → -1031012352 = 0xC2900000 =  -72.0
    //   resto (incl. BOOTS [752,784)) → 0
    //
    // 2026-08-11 FIX: esta copia (la VIVA) tenía DOS errores encadenados —
    //   (a) los cuatro valores eran los viejos -160/-100/-50/-70, y
    //   (b) PANTS y GLOVES estaban CRUZADOS (MODEL_PANTS=688, MODEL_GLOVES=720;
    //       el código le daba a GLOVES el valor de pants y viceversa).
    // Combinados: pants recibía -70 en vez de -48 (22 unidades DE MÁS hacia
    // abajo) y guantes -50 en vez de -72 (22 hacia arriba) — exactamente los
    // dos síntomas opuestos reportados. El comentario original ya listaba los
    // rangos de IDA bien y el código los contradecía.
    if (param_1 >= MODEL_HELM && param_1 < MODEL_HELM + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC31C0000;   // -156.0
    else if (param_1 >= MODEL_ARMOR && param_1 < MODEL_ARMOR + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2C00000;   //  -96.0
    else if (param_1 >= MODEL_PANTS && param_1 < MODEL_PANTS + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2400000;   //  -48.0
    else if (param_1 >= MODEL_GLOVES && param_1 < MODEL_GLOVES + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2900000;   //  -72.0
    else
        *(unsigned int *)((char *)modelThis + 0x84) = 0;

    if (param_1 == MODEL_HELM + 65 || param_1 == MODEL_HELM + 70)
        outPos[0] += 0.04f;

    // Resolve per-type model scale (local_3bc)
    auto rawf = [](float f) -> DWORD {
        DWORD d;
        memcpy(&d, &f, sizeof(d));
        return d;
    };

    if (param_1 >= MODEL_HELM && param_1 < MODEL_HELM + 32) local_3bc = rawf(0.0039f);
    else if (param_1 >= MODEL_ARMOR && param_1 < MODEL_ARMOR + 32) local_3bc = rawf(0.0039f);
    else if (param_1 >= MODEL_GLOVES && param_1 < MODEL_GLOVES + 32) local_3bc = rawf(0.0038f);
    else if (param_1 >= MODEL_PANTS && param_1 < MODEL_PANTS + 32) local_3bc = rawf(0.0033f);
    else if (param_1 >= MODEL_BOOTS && param_1 < MODEL_BOOTS + 32) local_3bc = rawf(0.0032f);
    if (param_1 == MODEL_POTION + 45 || param_1 == MODEL_POTION + 49) local_3bc = rawf(0.003f);
    else if (param_1 >= MODEL_POTION + 46 && param_1 <= MODEL_POTION + 48) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_POTION + 50) local_3bc = rawf(0.001f);
        else if (param_1 >= MODEL_POTION + 32 && param_1 <= MODEL_POTION + 34) {
            outPos[1] += 0.05f;
            local_3bc = rawf(0.002f);
        }
        else if (param_1 >= MODEL_EVENT + 21 && param_1 <= MODEL_EVENT + 23) {
            outPos[1] += (param_1 == MODEL_EVENT + 21) ? 0.08f : 0.06f;
            local_3bc = rawf(0.002f);
        }
    else if (param_1 == MODEL_POTION + 21) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_EVENT + 11) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 4) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 5) local_3bc = rawf(0.005f);
    else if (param_1 == MODEL_HELPER + 30 || param_1 == MODEL_EVENT + 16 || param_1 == MODEL_HELPER + 16) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_HELPER + 17 || param_1 == MODEL_HELPER + 18) local_3bc = rawf(0.0018f);
    else if (param_1 >= MODEL_HELPER + 43 && param_1 <= MODEL_HELPER + 45) local_3bc = rawf(0.0021f);
    else if (param_1 >= MODEL_HELPER + 46 && param_1 <= MODEL_HELPER + 48) local_3bc = rawf(0.0018f);
    else if (param_1 == MODEL_POTION + 53) local_3bc = rawf(0.00078f);
    else if (param_1 == MODEL_POTION + 54) local_3bc = rawf(0.0024f);
    else if (param_1 == MODEL_POTION + 58) local_3bc = rawf(0.0012f);
    else if (param_1 == MODEL_POTION + 59 || param_1 == MODEL_POTION + 60) local_3bc = rawf(0.0010f);
    else if (param_1 == MODEL_POTION + 61 || param_1 == MODEL_POTION + 62) local_3bc = rawf(0.0009f);
    else if (param_1 >= MODEL_POTION + 70 && param_1 <= MODEL_POTION + 71) local_3bc = rawf(0.0028f);
    else if (param_1 >= MODEL_POTION + 72 && param_1 <= MODEL_POTION + 77) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_HELPER + 59) local_3bc = rawf(0.0008f);
    else if (param_1 >= MODEL_HELPER + 54 && param_1 <= MODEL_HELPER + 58) local_3bc = rawf(0.004f);
    else if (param_1 >= MODEL_POTION + 78 && param_1 <= MODEL_POTION + 82) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_HELPER + 60) local_3bc = rawf(0.005f);
    else if (param_1 == MODEL_HELPER + 61) local_3bc = rawf(0.0018f);
    else if (param_1 == MODEL_POTION + 83) local_3bc = rawf(0.0009f);
    else if (param_1 == MODEL_POTION + 91) local_3bc = rawf(0.0034f);
    else if (param_1 == MODEL_POTION + 92 || param_1 == MODEL_POTION + 93 || param_1 == MODEL_POTION + 95) local_3bc = rawf(0.0024f);
    else if (param_1 == MODEL_POTION + 94) local_3bc = rawf(0.0022f);
    else if (param_1 == MODEL_POTION + 84) local_3bc = rawf(0.0031f);
    else if (param_1 == MODEL_POTION + 85) local_3bc = rawf(0.0044f);
    else if (param_1 == MODEL_POTION + 86) local_3bc = rawf(0.0031f);
    else if (param_1 == MODEL_POTION + 87) local_3bc = rawf(0.0061f);
    else if (param_1 == MODEL_POTION + 88 || param_1 == MODEL_POTION + 89 || param_1 == MODEL_POTION + 90) local_3bc = rawf(0.0035f);
    else if (param_1 == MODEL_HELPER + 62 || param_1 == MODEL_HELPER + 63) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_POTION + 97 || param_1 == MODEL_POTION + 98) local_3bc = rawf(0.003f);
    else if (param_1 == MODEL_POTION + 96) local_3bc = rawf(0.0028f);
    else if (param_1 == MODEL_HELPER + 0) local_3bc = rawf(0.0022f);
    else if (param_1 == MODEL_HELPER + 1 || param_1 == MODEL_HELPER + 2) local_3bc = rawf(0.0020f);
    else if (param_1 == MODEL_HELPER + 12 || param_1 == MODEL_HELPER + 13) local_3bc = rawf(0.0014f);
    else if (param_1 == MODEL_HELPER + 64) local_3bc = rawf(0.0005f);
    else if (param_1 == MODEL_HELPER + 65) local_3bc = rawf(0.0016f);
    else if (param_1 == MODEL_HELPER + 67) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 80) local_3bc = rawf(0.0020f);
    else if (param_1 == MODEL_HELPER + 68 || param_1 == MODEL_HELPER + 76) local_3bc = rawf(0.0026f);
    else if (param_1 == MODEL_HELPER + 69) local_3bc = rawf(0.0023f);
    else if (param_1 == MODEL_HELPER + 70) local_3bc = rawf(0.0018f);
    else if (param_1 >= MODEL_HELPER + 71 && param_1 <= MODEL_HELPER + 75) local_3bc = rawf(0.0019f);
    else if (param_1 == MODEL_HELPER + 81 || param_1 == MODEL_HELPER + 82) local_3bc = rawf(0.0012f);
    else if (param_1 == MODEL_HELPER + 93 || param_1 == MODEL_HELPER + 94) local_3bc = rawf(0.0021f);

    if (local_3bc == 0) {
        if (param_1 == MODEL_HELPER + 97 || param_1 == MODEL_HELPER + 98 || param_1 == MODEL_POTION + 91) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_HELPER + 99) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_POTION + 101) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_POTION + 102) local_3bc = rawf(0.005f);
        else if (param_1 >= MODEL_POTION + 103 && param_1 <= MODEL_POTION + 108) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_POTION + 109) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 110 || param_1 == MODEL_POTION + 111) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_HELPER + 105) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_HELPER + 106) local_3bc = rawf(0.0015f);
        else if (param_1 == MODEL_HELPER + 107) local_3bc = rawf(0.0034f);
        else if (param_1 >= MODEL_HELPER + 109 && param_1 <= MODEL_HELPER + 112) local_3bc = rawf(0.0045f);
        else if (param_1 >= MODEL_HELPER + 113 && param_1 <= MODEL_HELPER + 115) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_POTION + 112 && param_1 <= MODEL_POTION + 113) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 116) local_3bc = rawf(0.0021f);
        else if (param_1 >= MODEL_POTION + 114 && param_1 <= MODEL_POTION + 119) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_POTION + 120) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_HELPER + 121) local_3bc = rawf(0.0018f);
        else if (param_1 == MODEL_HELPER + 122) local_3bc = rawf(0.0033f);
        else if (param_1 == MODEL_HELPER + 123) local_3bc = rawf(0.0009f);
        else if (param_1 == MODEL_HELPER + 124) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_HELPER + 125 && param_1 <= MODEL_HELPER + 127) local_3bc = rawf(0.0013f);
        else if (param_1 == MODEL_HELPER + 128 || param_1 == MODEL_HELPER + 129) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_HELPER + 130) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 131) local_3bc = rawf(0.0033f);
        else if (param_1 == MODEL_HELPER + 132) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 133 || param_1 == MODEL_HELPER + 134) local_3bc = rawf(0.0033f);
        else if (param_1 >= MODEL_POTION + 126 && param_1 <= MODEL_POTION + 129) local_3bc = rawf(0.0038f);
        else if (param_1 >= MODEL_POTION + 130 && param_1 <= MODEL_POTION + 132) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_POTION + 133) local_3bc = rawf(0.0030f);
        else if (param_1 >= MODEL_POTION + 134 && param_1 <= MODEL_POTION + 139) local_3bc = rawf(0.0050f);
        else if (param_1 == MODEL_POTION + 140) local_3bc = rawf(0.0026f);
        else if (param_1 >= MODEL_POTION + 145 && param_1 <= MODEL_POTION + 150) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_HELPER + 135 && param_1 <= MODEL_HELPER + 145) local_3bc = rawf(0.0010f);
        else if (param_1 == MODEL_SWORD + 19) { if ((int)param_2 >= 0) local_3bc = rawf(0.0025f); else { local_3bc = rawf(0.001f); param_2 = 0; } }
        else if (param_1 == MODEL_STAFF + 10) { if ((int)param_2 >= 0) local_3bc = rawf(0.0019f); else { local_3bc = rawf(0.001f); param_2 = 0; } }
        else if (param_1 == MODEL_BOW + 18) { if ((int)param_2 >= 0) local_3bc = rawf(0.0025f); else { local_3bc = rawf(0.0015f); param_2 = 0; } }
        else if (param_1 >= MODEL_MACE + 8 && param_1 <= MODEL_MACE + 11) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_MACE + 12) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_MACE + 18) local_3bc = rawf(0.0024f);
        else if (param_1 == MODEL_EVENT + 12) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_EVENT + 13) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_EVENT + 14) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_EVENT + 15) local_3bc = rawf(0.0023f);
        else if (param_1 >= MODEL_POTION + 22 && param_1 < MODEL_POTION + 25) local_3bc = rawf(0.0025f);
        else if (param_1 >= MODEL_POTION + 25 && param_1 < MODEL_POTION + 27) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_POTION + 63) local_3bc = rawf(0.007f);
        else if (param_1 == MODEL_POTION + 99) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_POTION + 52) local_3bc = rawf(0.0014f);
        else if (param_1 == MODEL_BOW + 19) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_POTION + 41) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_POTION + 42) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_POTION + 43) {
            outPos[1] -= 0.005f;
            local_3bc = rawf(0.0035f);
        }
        else if (param_1 == MODEL_POTION + 44) {
            outPos[1] -= 0.005f;
            local_3bc = rawf(0.004f);
        }
        else if (param_1 == MODEL_POTION + 7 || param_1 == MODEL_HELPER + 7 || param_1 == MODEL_HELPER + 11) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_EVENT + 18) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 38) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 32) local_3bc = rawf(0.0019f);
        else if (param_1 == MODEL_HELPER + 33 || param_1 == MODEL_HELPER + 34 || param_1 == MODEL_HELPER + 35) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_HELPER + 36) local_3bc = rawf(0.007f);
        else if (param_1 == MODEL_HELPER + 37) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_BOW + 21) local_3bc = rawf(0.0022f);
        else if (param_1 == MODEL_BOW + 24) local_3bc = rawf(0.0023f);
        else if (param_1 == MODEL_HELPER + 49) local_3bc = rawf(0.0013f);
        else if (param_1 == MODEL_HELPER + 50 || param_1 == MODEL_HELPER + 51 || param_1 == MODEL_POTION + 64 || param_1 == MODEL_POTION + 65) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 66 || param_1 == MODEL_POTION + 67) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_POTION + 68) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_HELPER + 52 || param_1 == MODEL_HELPER + 53) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_HELPER + 66) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_SWORD + 24) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_BOW + 22) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_BOW + 23) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 14 || param_1 == MODEL_HELPER + 15) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 100) local_3bc = rawf(0.0040f);
        else if (param_1 == MODEL_EVENT + 10) local_3bc = rawf(0.0010f);
        else if (param_1 >= MODEL_ETC + 19 && param_1 <= MODEL_ETC + 27) local_3bc = rawf(0.0023f);
        else if (param_1 == MODEL_WING + 6) local_3bc = rawf(0.0015f);
        else if (param_1 >= MODEL_WING + 32 && param_1 <= MODEL_WING + 34) {
            outPos[1] -= 0.05f;
            local_3bc = rawf(0.0010f);
        }
        else if (param_1 >= MODEL_WING + 60 && param_1 <= MODEL_WING + 65) local_3bc = rawf(0.0022f);
        else if (param_1 >= MODEL_WING + 70 && param_1 <= MODEL_WING + 74) local_3bc = rawf(0.0017f);
        else if (param_1 >= MODEL_WING + 100 && param_1 <= MODEL_WING + 129) local_3bc = rawf(0.0017f);
        else if (param_1 == MODEL_WING + 49 || param_1 == MODEL_WING + 50) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_WING + 130) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_WING + 135) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_WING + 30 || param_1 == MODEL_WING + 31 ||
                 (param_1 >= MODEL_WING + 136 && param_1 <= MODEL_WING + 143)) {
            local_3bc = rawf(0.0040f);
            if (param_1 == MODEL_WING + 142) {
                outPos[0] -= 0.05f;
                local_3bc = rawf(0.0030f);
            } else if (param_1 == MODEL_WING + 143) {
                outPos[0] -= 0.05f;
                local_3bc = rawf(0.0040f);
            } else if (param_1 == MODEL_WING + 137) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0025f);
            } else if (param_1 == MODEL_WING + 141) {
                outPos[1] += 0.025f;
                local_3bc = rawf(0.0020f);
            } else if (param_1 == MODEL_WING + 138) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0036f);
            } else if (param_1 == MODEL_WING + 136) {
                outPos[1] += 0.025f;
                local_3bc = rawf(0.0035f);
            } else if (param_1 == MODEL_WING + 139) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0035f);
            } else if (param_1 == MODEL_WING + 140) {
                local_3bc = rawf(0.0050f);
            }
        }
        else if (param_1 >= MODEL_WING && param_1 < MODEL_WING + 6) local_3bc = rawf(0.0025f);
        else if (param_1 >= MODEL_WING && param_1 < MODEL_WING + 512) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_POTION + 160 || param_1 == MODEL_POTION + 161) local_3bc = rawf(0.0010f);
        else if (param_1 >= MODEL_POTION && param_1 < MODEL_POTION + 512) local_3bc = rawf(0.0035f);
        else if (param_1 >= MODEL_SPEAR && param_1 < MODEL_SPEAR + 512) {
            if (param_1 == MODEL_SPEAR + 10) local_3bc = rawf(0.0018f);
            else if (param_1 == MODEL_SPEAR + 11) local_3bc = rawf(0.0025f);
            else local_3bc = rawf(0.0021f);
        }
        else if (param_1 >= MODEL_STAFF && param_1 < MODEL_STAFF + 512) {
            if (param_1 >= MODEL_STAFF + 14 && param_1 <= MODEL_STAFF + 20) local_3bc = rawf(0.0028f);
            else if (param_1 >= MODEL_STAFF + 21 && param_1 <= MODEL_STAFF + 29) local_3bc = rawf(0.004f);
            else if (param_1 == MODEL_STAFF + 33 || param_1 == MODEL_STAFF + 34) local_3bc = rawf(0.0028f);
            else local_3bc = rawf(0.0022f);
        }
        else if (param_1 == MODEL_BOW + 15) local_3bc = rawf(0.0011f);
        else if (param_1 == MODEL_BOW + 7) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_EVENT + 6) local_3bc = rawf(0.0039f);
        else if (param_1 == MODEL_EVENT + 8) local_3bc = rawf(0.0015f);
        else if (param_1 == MODEL_EVENT + 9) local_3bc = rawf(0.0019f);
    }

    if (local_3bc == 0 && (param_1 == MODEL_HELM + 62 || param_1 == MODEL_HELM + 63 || param_1 == MODEL_HELM + 65 || param_1 == MODEL_HELM + 70)) local_3bc = rawf(0.0010f);
    else if (local_3bc == 0 && param_1 == MODEL_HELM + 31) local_3bc = rawf(0.0070f);
    else if (local_3bc == 0 && param_1 >= MODEL_HELM + 39 && param_1 <= MODEL_HELM + 44) local_3bc = rawf(0.0070f);
    else if (local_3bc == 0 && (param_1 == MODEL_ARMOR + 30 || param_1 == MODEL_ARMOR + 32)) local_3bc = rawf(0.0035f);
    else if (local_3bc == 0 && param_1 == MODEL_ARMOR + 29) local_3bc = rawf(0.0033f);
    else if (local_3bc == 0 && (param_1 == MODEL_ARMOR + 34 || param_1 == MODEL_ARMOR + 35 || param_1 == MODEL_GLOVES + 38)) local_3bc = rawf(0.0032f);
    else if (local_3bc == 0 && param_1 >= 0x270 && param_1 < 0x310) {
        if (param_1 < 0x290)       local_3bc = rawf(0.0039f);
        else if (param_1 < 0x2b0)  local_3bc = rawf(0.0039f);
        else if (param_1 < 0x2d0)  local_3bc = rawf(0.0033f);
        else if (param_1 < 0x2f0)  local_3bc = rawf(0.0038f);
        else                        local_3bc = rawf(0.0032f);
    } else if (local_3bc == 0 && param_1 == 0x316)  local_3bc = 0x3ac49ba6;
    else if (param_1 >= 0x310 && param_1 < 0x330) local_3bc = 0x3b03126f;
    else if (param_1 == 0x340)    local_3bc = 0x3b03126f;
    else if (param_1 == 0x341 || param_1 == 0x342) local_3bc = 0x3aebedfa;
    else if (param_1 == 0x365)    local_3bc = 0x3b03126f;
    else if (param_1 == 0x3be)    local_3bc = 0x3ac49ba6;
    else if (param_1 == 0x1a3)    { if ((int)param_2 >= 0) local_3bc = 0x3b23d70a; else { local_3bc = 0x3a83126f; param_2 = 0; } }
    else if (param_1 == 0x23a)    { if ((int)param_2 >= 0) local_3bc = 0x3af9096c; else { local_3bc = 0x3a83126f; param_2 = 0; } }
    else if (param_1 == 0x222)  {
        if ((int)param_2 < 0) {
            local_3bc = 0x3ac49ba6;
            param_2 = 0;
        } else {
            local_3bc = 0x3b378034;
        }
    }
    else if (param_1 >= 0x366 && param_1 <= 0x368) local_3bc = 0x3b23d70a;
    else if (param_1 == 0x369 || param_1 == 0x36a) local_3bc = 0x3b378034;
    else if (param_1 == 0x33e || param_1 == 0x33f) local_3bc = 0x3b449ba6;
    else if (param_1 >= 0x350 && param_1 < 0x370)  local_3bc = 0x3b656042;
    else if (param_1 >= 0x1f0 && param_1 < 0x210)  local_3bc = 0x3aebedfa;
    else if (param_1 >= 0x230 && param_1 < 0x250)  local_3bc = 0x3b102de0;
    else if (param_1 == 0x21f) local_3bc = 0x3a902de0;
    else if (param_1 == 0x217) local_3bc = 0x3a9d4952;
    else if (param_1 == 0x3b9) local_3bc = 0x3b7f9724;
    else if (param_1 == 0x3bb) local_3bc = 0x3ac49ba6;
    else if (param_1 == 0x3bc) local_3bc = 0x3af9096c;
    else if (param_1 == 0x3bd) local_3bc = 0x3a83126f;
    // 2026-08-11 FIX (items 3D chicos y descentrados): este `else` NO estaba
    // gateado, así que pisaba la escala ya calculada. Las líneas ~1881-1885
    // asignan bien la escala de los body-parts ([624,784): casco/armadura 0.0039,
    // guantes 0.0038, pants 0.0033, botas 0.0032), pero después vienen dos
    // cascadas más cuyas ramas llevan `local_3bc == 0 &&`; al no matchear
    // ninguna, la ejecución caía en este `else` final y clavaba TODO en el
    // default 0.0025. Medido en runtime (probe SCALE3D): los modelos 642/674/
    // 706/738/770 salían todos con 0.0025 en vez de su valor → items ~1.56×
    // más chicos y, como el ancla queda por debajo del centro de la casilla,
    // visualmente corridos. En IDA no puede pasar: la rama de [624,784) termina
    // con `goto LABEL_142`, saltándose el resto de la lógica de escala.
    else if (local_3bc == 0) local_3bc = 0x3b23d70a;

    // OpenMu/IDA 0x4E13A0 treats Book01..Book16 (models 880..895) as plain
    // item models: fixed pose (270,-10,0), 0.0025 scale, and Y rotation only
    // while the item is selected.  The reconstructed generic item table above
    // reuses this numeric range for potion-specific poses, which overwrote the
    // native Book state after it had been established.
    if (nativeBook) {
        outPos[0] = nativeBookPos[0];
        outPos[1] = nativeBookPos[1];
        outPos[2] = nativeBookPos[2];
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = param_6 == 1 ? (float)DAT_05826e08 * 0.45f : -10.0f;
        _DAT_07ea9534 = 0.0f;
        local_3bc = 0x3b23d70a; // 0.0025f
    }

    // Reset render state globals
    _DAT_07ea9618 = 0;
    _DAT_07ea961c = 0;
    DAT_07ea9616 = 0;

    // Pose model with BMD_Animation
    float angleArr[3] = { _DAT_07ea952c, _DAT_07ea9530, _DAT_07ea9534 };
    float headAngle[3] = { _DAT_07ea9538, 0.0f, 0.0f };
    FUN_00440060(modelThis, (int)&DAT_06970a9c, 0.0f, 0, 0, (unsigned int *)angleArr, headAngle, '\0', '\0');

    // Build stack entity and draw
    // entity_type at [+2], scale at [+0x0c], world_pos at [+0x10..+0x18]
    char ent[0x200];
    memset(ent, 0, sizeof(ent));
    *(short *)(ent + 2) = (short)param_1;
    *(DWORD *)(ent + 0x0c) = local_3bc;  // write raw float bits into entity+0x0c
    *(unsigned char *)(ent + 0x3d) = param_4; // preserve raw ExtOption for later render passes

    FUN_00502ba0((int)ent);

    // FIX 2026-05-01 (BUG REAL): ItemObjectAttribute (FUN_00502ba0) sobreescribe
    // ent[+0xC] con un valor default (0x3F4CCCCD = 0.8f para items en mundo).
    // En IDA, después de ItemObjectAttribute hay un `v16 = v11;` que reasigna el
    // scale (v16 = ent+0xC). Sin esa reasignación, RenderPartObject lee scale=0.8
    // (80%) en vez de los valores correctos (0.0025 para items, etc.) → modelos
    // renderizados a tamaño gigante.
    //
    // 2026-08-08 FIX (items 3D descentrados / más bajos que su casilla): el
    // mismo patrón, pero con la POSICIÓN. El port escribía outPos en
    // ent+0x10/0x14/0x18 ANTES de ItemObjectAttribute y no los reescribía.
    // En IDA (0x4E13A0 LABEL_142) el orden es:
    //     ItemObjectAttribute(&o);
    //     v16 = v11;              // o + 12 = Scale
    //     v17 = Position[0];      // o + 16
    //     v18 = Position[1];      // o + 20
    //     v19 = Position[2];      // o + 24
    //     v20 = 0;                // o + 0xDC
    //     v22 = 2;                // o + 0x1BC
    // o sea Position (y los otros dos campos) se escriben DESPUÉS, porque
    // ItemObjectAttribute también los pisa — igual que hace con el scale.
    // Verificado que el resto de la cadena (anchor, CreateScreenVector, lerp
    // 0.1, tabla de escala, viewport/perspectiva) ya era fiel, y que el DLL de
    // inyección usa los mismos multiplicadores con su tabla de ajuste VACÍA:
    // era este orden.
    *(DWORD *)(ent + 0x0c) = local_3bc;
#if ITEM3D_FAITHFUL_POSITION
    // Descarta los ajustes por tipo que el port fue acumulando — ver la nota
    // junto al VectorMA, arriba. IDA escribe acá la posición del rayo, sin más.
    *(float *)(ent + 0x10) = rayPos[0];
    *(float *)(ent + 0x14) = rayPos[1];
    *(float *)(ent + 0x18) = rayPos[2];
#else
    *(float *)(ent + 0x10) = outPos[0];
    *(float *)(ent + 0x14) = outPos[1];
    *(float *)(ent + 0x18) = outPos[2];
#endif
    *(int   *)(ent + 0xdc) = 0;
    *(unsigned char *)(ent + 0x1bc) = 2;


    float light[3] = { 1.0f, 1.0f, 1.0f };
    // FIX 2026-05-01: param_5 era 0.0f → fallaba el gate
    // `if (_DAT_005524f8 < param_5)` (threshold = 0.01) y RenderPartObject
    // entero se saltaba → items del inventario no se veían. Pasamos 1.0f
    // (distancia "siempre visible") como hace el path de mundo (que pasa
    // entity scale/distance).

    // FIX confirmado 2026-05-01: el bug de "items rendering huge" venía de
    // ItemObjectAttribute(FUN_00502ba0) sobreescribiendo ent[+0xc] con 0.8f.
    // La reasignación post-ItemObjectAttribute arreglo el problema.

    FUN_00505a10((int)ent, param_1, 0, light, 1.0f, param_2, param_3, '\x01', 1, '\x01', 0, 2);
    (void)param_6;
}
#endif

// FUN_004e13a0 @ 0x004E13A0 — RenderObjectScreen.
// Literal control-flow port of the 0.97k IDA routine.  This is the common 3D
// item path for inventory, equipment, shop, warehouse, trade and Chaos grids.
void __cdecl FUN_004e13a0(int Type, unsigned int ItemLevel, unsigned char Option1,
                          unsigned char ExtOption, float* Target, int Select, char PickUp)
{
    // This recovery is infrastructure-only: the original dereferences
    // ItemAttribute below, while this client has a known external pointer
    // watchdog.  It does not alter any item rendering state.
    unsigned int attrAddress = (unsigned int)DAT_07d78068;
    if (attrAddress < 0x100000u || attrAddress >= 0x80000000u) {
        if (g_ItemAttribute_Backup < 0x100000u || g_ItemAttribute_Backup >= 0x80000000u)
            return;
        DAT_07d78068 = (int)g_ItemAttribute_Backup;
    }

    float camera[3] = { _CameraRayOriginX, _CameraRayOriginY, _CameraRayOriginZ };
    float direction[3] = {
        Target[0] - camera[0], Target[1] - camera[1], Target[2] - camera[2]
    };
    float position[3];
    FUN_004f9ce0(camera, PickUp ? 0.07f : 0.1f, direction, position);

    const int level = ((int)ItemLevel >> 3) & 0x0F;
    float angle[3];

    if (Type == 535 || Type == 543) {
        angle[0] = 0.0f; angle[1] = 270.0f; angle[2] = 15.0f;
    } else if (Type == 545) {
        angle[0] = 0.0f; angle[1] = 90.0f; angle[2] = 15.0f;
    } else if (Type >= 536 && Type < 560) {
        angle[0] = 90.0f; angle[1] = 180.0f; angle[2] = 20.0f;
    } else if (Type == 506) {
        angle[0] = 180.0f; angle[1] = 270.0f; angle[2] = 20.0f;
    } else if (Type >= 400 && Type < 592) {
        angle[0] = 180.0f;
        angle[1] = 270.0f;
        angle[2] = *(BYTE*)((BYTE*)(uintptr_t)DAT_07d78068 + (Type - 399) * 0x40 - 34) ? 25.0f : 15.0f;
    } else if (Type >= 592 && Type < 624) {
        angle[0] = 270.0f; angle[1] = 270.0f; angle[2] = 0.0f;
    } else if (Type == 819) {
        angle[0] = -90.0f; angle[1] = -20.0f; angle[2] = 0.0f;
    } else if (Type == 832 || Type == 833) {
        angle[0] = 270.0f; angle[1] = -10.0f; angle[2] = 0.0f;
    } else if (Type == 834) {
        angle[0] = 290.0f; angle[1] = 0.0f; angle[2] = 0.0f;
    } else if (Type == 958) {
        angle[0] = -90.0f; angle[1] = -20.0f; angle[2] = -20.0f;
    } else if (Type >= 828 && Type < 848 && Type != 830 && Type != 831) {
        angle[0] = 360.0f; angle[1] = 0.0f; angle[2] = 0.0f;
    } else if (Type == 860) {
        if (level == 0)      { angle[0] = 180.0f; angle[1] = 0.0f;  angle[2] = 0.0f; }
        else if (level == 1) { angle[0] = 270.0f; angle[1] = 90.0f; angle[2] = 0.0f; }
        else                 { angle[0] = 90.0f;  angle[1] = 0.0f;  angle[2] = 0.0f; }
    } else if (Type == 952 || Type == 954) {
        angle[0] = 270.0f; angle[1] = 0.0f; angle[2] = 0.0f;
    } else if (Type == 953) {
        angle[0] = 270.0f; angle[1] = 90.0f; angle[2] = 0.0f;
    } else if (Type == 868) {
        angle[0] = 270.0f; angle[1] = 0.0f; angle[2] = 0.0f;
    } else {
        angle[0] = 270.0f; angle[1] = -10.0f; angle[2] = 0.0f;
    }

    if (Select == 1)
        angle[1] = (float)DAT_05826e08 * 0.45f;

    // IDA stores the pose in these shared angle slots before BMD_Animation;
    // later item passes also observe that state.
    _DAT_07ea952c = angle[0];
    _DAT_07ea9530 = angle[1];
    _DAT_07ea9534 = angle[2];

    short modelType = (short)Type;
    if (modelType >= 624 && modelType < 784) {
        modelType = 390;
    } else if (modelType == 860) {
        if (level == 0) modelType = 947;
        else if (level == 2) modelType = 948;
    }
    _DAT_07ea9512 = modelType;

    void* model = (void*)(DAT_05828d58 + (int)modelType * 0xBC);
    *(BYTE*)((BYTE*)model + 0xA0) = 0;
    if (Type >= 624 && Type < 656)      *(float*)((BYTE*)model + 0x84) = -156.0f;
    else if (Type >= 656 && Type < 688) *(float*)((BYTE*)model + 0x84) = -96.0f;
    else if (Type >= 688 && Type < 720) *(float*)((BYTE*)model + 0x84) = -48.0f;
    else if (Type >= 720 && Type < 752) *(float*)((BYTE*)model + 0x84) = -72.0f;
    else                                 *(float*)((BYTE*)model + 0x84) = 0.0f;

    float scale = 0.0025f;
    if (Type >= 624 && Type < 784) {
        if (Type < 688) scale = 0.0039f;
        else if (Type < 720) scale = 0.0038f;
        else if (Type < 752) scale = 0.0032f;
        else scale = 0.0033f;
    } else if (Type == 790 || Type == 958) scale = 0.0015f;
    else if ((Type >= 784 && Type < 816) || Type == 869 || Type == 832) scale = 0.0020f;
    else if (Type == 833 || Type == 834 || (Type >= 496 && Type < 528)) scale = 0.0018f;
    else if (Type == 419) scale = (int)ItemLevel >= 0 ? 0.0025f : 0.0010f;
    else if (Type == 570) scale = (int)ItemLevel >= 0 ? 0.0019f : 0.0010f;
    else if (Type == 546) scale = (int)ItemLevel < 0 ? 0.0015f : 0.0025f;
    else if (Type >= 870 && Type < 873) scale = 0.0025f;
    else if (Type >= 873 && Type < 875) scale = 0.0028f;
    else if (Type == 830 || Type == 831) scale = 0.0030f;
    else if (Type >= 848 && Type < 880) scale = 0.0035f;
    else if (Type >= 560 && Type < 592) scale = 0.0022f;
    else if (Type == 543) scale = 0.0011f;
    else if (Type == 535) scale = 0.0012f;
    else if (Type == 953) scale = 0.0039f;
    else if (Type == 955) scale = 0.0015f;
    else if (Type == 956) scale = 0.0019f;
    else if (Type == 957) scale = 0.0010f;

    _DAT_07ea9618 = 0;
    _DAT_07ea961c = 0;
    DAT_07ea9616 = 0;
    float renderAngle[3] = { _DAT_07ea952c, _DAT_07ea9530, _DAT_07ea9534 };
    float headAngle[3] = { _DAT_07ea9538, 0.0f, 0.0f };
    FUN_00440060(model, (int)&DAT_06970a9c, 0.0f, 0, 0,
                 (unsigned int*)renderAngle, headAngle, '\0', '\0');

    char object[0x200] = {};
    *(short*)(object + 2) = (short)Type;
    FUN_00502ba0((int)object);
    *(float*)(object + 0x0C) = scale;
    *(float*)(object + 0x10) = position[0];
    *(float*)(object + 0x14) = position[1];
    *(float*)(object + 0x18) = position[2];
    *(BYTE*)(object + 0x3D) = ExtOption;
    *(int*)(object + 0xDC) = 0;
    *(BYTE*)(object + 0x1BC) = 2;

    float light[3] = { 1.0f, 1.0f, 1.0f };
    // Entity_DrawAt's visibility argument is 1.0 in the native UI path used
    // by this client; all OpenGL state setup/teardown stays inside that renderer.
    FUN_00505a10((int)object, Type, 0, light, 1.0f, ItemLevel, Option1,
                 '\x01', 1, '\x01', 0, 2);
}
