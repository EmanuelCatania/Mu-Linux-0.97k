// Entity_DrawSetup.cpp
// FUN_00504b50 @ 0x00504b50
//
// Entity_SetColorAndRender — resolves anim-mode, sets model color from light
// array, dispatches to FUN_00504130 / FUN_00504960 / FUN_00504ac0, applies
// optional "flashing" effect on buffed entities.
//
// Signature (faithful Ghidra port):
//   void __cdecl FUN_00504b50(int param_1, int param_2, float *param_3,
//                              float param_4, uint param_5, byte param_6,
//                              undefined4 param_7, uint param_8)
//
// param_1 — entity pointer
// param_2 — model slot index → this = DAT_05828d58 + param_2 * 0xbc
// param_3 — float[3] light color (R,G,B)
// param_4 — animation frame / scale
// param_5 — raw flags (anim_mode = (param_5 >> 3) & 0xf)
// param_6 — status bits; (param_6 & 0x3f) != 0 → buff flashing
// param_7 — pass-through (unused in default path)
// param_8 — draw flags (bit 0x400 forces anim_mode 0; bit 0x100 passed to subs)
//
// ── BUG-FIX (2026-04-20) ───────────────────────────────────────────────────────
// La port anterior pasaba `(int)(uintptr_t)param_3` (puntero heap) como 2do
// argumento `int flags` de FUN_00441e00. Resultado en log:
//   BMD_Draw flags=0xa0b5790 bodyLight=(0,0,0)
// La función real de Ghidra NO llama FUN_00441e00 en el default path — sólo
// FUN_00504130. Además FUN_00504130 toma 5 args (this, entity, model_slot,
// scale, flags), no 3. Esta re-port arregla ambos.
//
// ── Entity-type switches (Ghidra) ──────────────────────────────────────────────
// Hay ~15 branches para tipos especiales (0x1f9, 0x33f, 0x3be, 0x341, 0x342,
// 0x315, 0x316, 0x361-0x365, etc.). Para Player (0x186) y body-parts
// (0x390-0x3ac) NINGUNA coincide → todos caen al default path implementado aquí.
// Los branches especiales se dejan fuera; si aparecen sprites con tipo raro
// (dragones, phoenixes, bosses específicos), habrá que portarlos.
//
// Called from: Entity_DrawAt @ 0x00505A10

#include "stdafx.h"
extern "C" { void DbgLogPublic(const char* msg); }

void __cdecl FUN_00504b50(int param_1, int param_2, float *param_3,
                           float param_4, uint param_5, byte param_6,
                           undefined4 param_7, uint param_8)
{
    void   *this_;
    short   sVar3;
    short   effectType;
    byte    bVar4;
    float10 fVar10;
    int     ItemLevelFx;

    // ── Anim-mode extraction (bits 3-6 of param_5) ────────────────────────────
    param_5 = ((int)param_5 >> 3) & 0xf;
    if ((param_8 & 0x400) != 0) {
        param_5 = 0;
    }
    ItemLevelFx = (int)param_5;
    if (param_2 > 867) {
        switch (param_2) {
        case 868:
        case 952:
        case 953:
            ItemLevelFx = 9;
            break;
        case 870:
        case 873:
        case 874:
        case 947:
        case 948:
        case 956:
            ItemLevelFx = 8;
            break;
        case 951:
        case 954:
        case 955:
        case 958:
            ItemLevelFx = 0;   // IDA LABEL_17
            break;
        case 957:
            ItemLevelFx = ItemLevelFx * 2 - 15;
            break;
        default:
            break;
        }
    }
    else if (param_2 >= 865) {
        ItemLevelFx *= 2;
    }
    else if (param_2 > 799) {
        switch (param_2) {
        case 800:
        case 801:
        case 802:
        case 803:
            ItemLevelFx = 9;
            break;
        case 819:
        case 860:
        case 861:
        case 862:
        case 864:
            ItemLevelFx = 8;
            break;
        case 826:
            ItemLevelFx = 8;
            break;
        default:
            break;
        }
    }
    else if (param_2 == 799) {
        ItemLevelFx = 8;
    }
    else if (param_2 > 791) {
        switch (param_2) {
        case 795:
            ItemLevelFx = 0;   // IDA LABEL_17
            break;
        case 796:
        case 797:
        case 798:
            ItemLevelFx = 9;
            break;
        default:
            break;
        }
    }
    else if (param_2 == 791) {
        ItemLevelFx = 9;
    }
    else if (param_2 > 543) {
        if (param_2 >= 784) {
            ItemLevelFx = 0;
        }
    }
    else if (param_2 == 543 || param_2 == 535) {
        ItemLevelFx = (ItemLevelFx < 1) ? 0 : (ItemLevelFx * 2 + 1);
    }

    // NOTE: el 0.97k (RenderPartObjectEffect @0x504B50) tiene UN SOLO switch de
    // remap de ItemLevelFx (el de arriba) y luego va directo al switch de tipo de
    // entidad (v11 = *(o+2)). Los dos switches extra que había acá (casos
    // MODEL_WING/MODEL_POTION/MODEL_HELPER/MODEL_EVENT de alto índice) provienen de
    // una versión posterior de MU y NO existen en el binario 0.97k. Colisionaban
    // con los tipos de cuerpo desnudo (armor-skin = 919+Class → 920/921/922/925 =
    // MODEL_WING+136/137/138/141) forzando ItemLevelFx=8 → glow fantasma en el
    // pecho de los personajes demo del server-select (sin armadura). Eliminados
    // para ser fieles a IDA.

    this_ = (void *)(DAT_05828d58 + param_2 * 0xbc);
    sVar3 = *(short *)(param_1 + 2);
    effectType = sVar3;

    // ─── Dead/dying entity (+0x8c != 0): black tint, special shadow draw ──────────────────
    if (*(char *)(param_1 + 0x8c) != '\0') {
        if (DAT_0055a7ac == 7) {
            GL_SetBlendSrcOver('\x01');
            glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
        }
        else {
            GL_ResetState();
            glColor3f(0.0f, 0.0f, 0.0f);
        }
        if (DAT_0055a7ac == 10) return;
        FUN_00441f00(this_, *(int *)(param_1 + 100), *(int *)(param_1 + 0x58));
        return;
    }

    if (effectType == 505) {
        *(float *)((int)this_ + 0x48) = 0.5f;
        *(float *)((int)this_ + 0x4c) = 0.5f;
        *(float *)((int)this_ + 0x50) = 1.5f;
        *(char *)((int)this_ + 0x88) = 0;
        FUN_00441e00(this_, 2, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), *(float *)(param_1 + 0x58), 1170);
        *(char *)((int)this_ + 0x88) = -1;
    }
    else if (effectType == 831) {
        if (ItemLevelFx == 0) {
            *(float *)((int)this_ + 0x48) = 0.0f;
            *(float *)((int)this_ + 0x4c) = 0.5f;
            *(float *)((int)this_ + 0x50) = 1.0f;
        }
        else if (ItemLevelFx == 1) {
            *(float *)((int)this_ + 0x48) = 1.0f;
            *(float *)((int)this_ + 0x4c) = 0.2f;
            *(float *)((int)this_ + 0x50) = 0.0f;
        }
        else if (ItemLevelFx == 2) {
            *(float *)((int)this_ + 0x48) = 1.0f;
            *(float *)((int)this_ + 0x4c) = 0.8f;
            *(float *)((int)this_ + 0x50) = 0.0f;
        }
        else if (ItemLevelFx == 3) {
            *(float *)((int)this_ + 0x48) = 0.65f;
            *(float *)((int)this_ + 0x4c) = 0.8f;
            *(float *)((int)this_ + 0x50) = 0.4f;
        }
        FUN_00441e00(this_, 8, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), *(float *)(param_1 + 0x58), 1171);
        FUN_00441e00(this_, 0x44, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), *(float *)(param_1 + 0x58), 1171);
        return;
    }
    else if (effectType == 958) {
        *(float *)((int)this_ + 0x48) = 1.0f;
        *(float *)((int)this_ + 0x4c) = 1.0f;
        *(float *)((int)this_ + 0x50) = 1.0f;
        FUN_00441e00(this_, 2, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), -1.0f, -1);
        FUN_00441e00(this_, 0x44, 0.5f, *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), -1.0f, 1171);
        return;
    }

    // ── Status-flag tints (entity +0x78 bits) ─────────────────────────────────
    else if (effectType == 833) {
        float rotU;
        *(float *)((int)this_ + 0x48) = 0.9f;
        *(float *)((int)this_ + 0x4c) = 0.1f;
        *(float *)((int)this_ + 0x50) = 0.1f;
        rotU = sinf((float)World * 0.0001f);
        *(float *)(param_1 + 0x6c) = rotU;
        *(float *)(param_1 + 0x70) = (float)WorldTime * -0.0005f;
        *(char *)(DAT_05828d58 + effectType * 0xbc + 0x88) = 0;
        FUN_00441e00(this_, 2, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), *(float *)(param_1 + 0x58), 1170);
        *(char *)(DAT_05828d58 + effectType * 0xbc + 0x88) = -1;
        *(float *)((int)this_ + 0x48) = 1.0f;
        *(float *)((int)this_ + 0x4c) = 1.0f;
        *(float *)((int)this_ + 0x50) = 1.0f;
        FUN_00441e00(this_, 2, *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68), *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70), *(float *)(param_1 + 0x58), -1);
        return;
    }
    else if (effectType == 834) {
        float pulse;
        *(float *)((int)this_ + 0x48) = 0.8f;
        *(float *)((int)this_ + 0x4c) = 0.8f;
        *(float *)((int)this_ + 0x50) = 0.8f;
        pulse = sinf((float)WorldTime * 0.002f) * 0.3f + 0.7f;
        FUN_00441e00(this_, 0x42, 1.0f, 0.0f, pulse,
                     *(float *)(param_1 + 0x6c), *(float *)(param_1 + 0x70),
                     *(float *)(param_1 + 0x58), -1);
        return;
    }
    else if (effectType == 865) {
        float wave;
        *(int *)(param_1 + 100) = 1;
        wave = sinf((float)WorldTime * 0.002f) * 10.0f + 15.65f;
        *(float *)(param_1 + 104) = wave;
        *(float *)(param_1 + 0x168) = 2.0f;
        *(float *)(param_1 + 0x70) = (float)((long long)WorldTime % 2000) * 0.0005f;
        *(float *)(param_1 + 0xe8) = wave * 0.2f;
        *(float *)(param_1 + 0xec) = wave * 0.2f;
        *(float *)(param_1 + 0xf0) = wave * 0.2f;
    }
    else if (effectType == 866) {
        float zeroPos[3] = { 0.0f, 0.0f, 0.0f };
        float targetPos[3];
        float fxLight[3];
        float lum;
        float scale;
        lum = sinf((float)WorldTime * 0.002f) * 0.35f + 0.65f;
        scale = lum * 0.8f;
        fxLight[0] = lum * 2.0f;
        fxLight[1] = lum * 0.32f;
        fxLight[2] = fxLight[1];
        BMD__TransformPosition(this_, (float (*)[4])&DAT_06970acc, zeroPos, targetPos, false);
        targetPos[0] += *(float *)(param_1 + 0x10);
        targetPos[1] += *(float *)(param_1 + 0x14);
        targetPos[2] += *(float *)(param_1 + 0x18);
        FUN_004795c0(1176, targetPos, scale, fxLight, param_1, 0.0f, 0);
        BMD__TransformPosition(this_, (float (*)[4])&DAT_06970afc, zeroPos, targetPos, false);
        targetPos[0] += *(float *)(param_1 + 0x10);
        targetPos[1] += *(float *)(param_1 + 0x14);
        targetPos[2] += *(float *)(param_1 + 0x18);
        FUN_004795c0(1176, targetPos, scale, fxLight, param_1, 0.0f, 0);
    }
    else if (effectType == 867) {
        float zeroPos[3] = { 0.0f, 0.0f, 0.0f };
        float targetPos[3];
        float fxLight[3];
        float lum;
        float scale;
        lum = sinf((float)WorldTime * 0.002f) * 0.35f + 0.65f;
        scale = lum * 0.8f;
        fxLight[0] = lum * 2.0f;
        fxLight[1] = lum * 0.32f;
        fxLight[2] = fxLight[1];
        BMD__TransformPosition(this_, (float (*)[4])&DAT_06970c4c, zeroPos, targetPos, false);
        targetPos[0] += *(float *)(param_1 + 0x10);
        targetPos[1] += *(float *)(param_1 + 0x14);
        targetPos[2] += *(float *)(param_1 + 0x18);
        FUN_004795c0(1176, targetPos, scale, fxLight, param_1, 0.0f, 0);
        BMD__TransformPosition(this_, (float (*)[4])&DAT_06970c7c, zeroPos, targetPos, false);
        targetPos[0] += *(float *)(param_1 + 0x10);
        targetPos[1] += *(float *)(param_1 + 0x14);
        targetPos[2] += *(float *)(param_1 + 0x18);
        FUN_004795c0(1176, targetPos, scale, fxLight, param_1, 0.0f, 0);
    }
    else if (effectType == 869) {
        float fxLight[3];
        float lum;
        lum = sinf((float)WorldTime * 0.002f) * 0.25f + 0.75f;
        fxLight[0] = lum;
        fxLight[1] = lum * 0.5f;
        fxLight[2] = 0.0f;
        FUN_004795c0(1176, (float *)(param_1 + 0x10), 2.5f, fxLight, param_1, 0.0f, 0);
    }
    else if (effectType == 789) {
        *(float *)(param_1 + 104) = (sinf((float)WorldTime * 0.001f) + 1.0f) * 0.25f;
    }
    else if (effectType == 788) {
        *(float *)(param_1 + 104) = sinf((float)WorldTime * 0.001f) + 1.1f;
    }
    else if (effectType == 790) {
        float fxLight[3];
        float zeroPos[3];
        float targetPos[3];
        float worldPos[3];
        float targetScale;
        float scale2;
        float spriteScale;
        const int fromA[5] = { 0x420, 0x3F0, 0x3C0, 0x390, 0x360 };
        const int toA[5]   = { 0x5A0, 0x570, 0x540, 0x510, 0x4E0 };
        const int fromB[5] = { 0x150, 0x120, 0x0F0, 0x0C0, 0x090 };
        const int toB[5]   = { 0x210, 0x240, 0x270, 0x2A0, 0x2D0 };
        int i;

        fxLight[0] = 0.60000002f;
        fxLight[1] = 0.30000001f;
        fxLight[2] = 0.80000001f;
        zeroPos[0] = 0.0f;
        zeroPos[1] = 0.0f;
        zeroPos[2] = 0.0f;

        targetScale = (sinf((float)WorldTime * 0.0040000002f) + 1.0f) * 0.30000001f * 10.0f + 20.0f;
        scale2 = targetScale + 5.0f;
        spriteScale = targetScale * 0.035714287f;

        for (i = 0; i < 5; ++i) {
            BMD__TransformPosition(this_,
                (float (*)[4])(g_BoneScratch + fromA[i]),
                zeroPos, targetPos, true);
            BMD__TransformPosition(this_,
                (float (*)[4])(g_BoneScratch + toA[i]),
                zeroPos, worldPos, true);
            Joint_Create(1254, worldPos, targetPos, (float *)(param_1 + 28), 14, param_1, targetScale, -1, 0);
            Joint_Create(1253, targetPos, worldPos, (float *)(param_1 + 28), 4, param_1, scale2, -1, 0);
            FUN_004795c0(1277, targetPos, spriteScale, fxLight, param_1, 0.0f, 0);
        }

        for (i = 0; i < 5; ++i) {
            BMD__TransformPosition(this_,
                (float (*)[4])(g_BoneScratch + fromB[i]),
                zeroPos, targetPos, true);
            BMD__TransformPosition(this_,
                (float (*)[4])(g_BoneScratch + toB[i]),
                zeroPos, worldPos, true);
            Joint_Create(1254, worldPos, targetPos, (float *)(param_1 + 28), 14, param_1, targetScale, -1, 0);
            Joint_Create(1253, targetPos, worldPos, (float *)(param_1 + 28), 4, param_1, scale2, -1, 0);
            FUN_004795c0(1277, targetPos, spriteScale, fxLight, param_1, 0.0f, 0);
        }
    }
    bVar4 = (byte)*(uint *)(param_1 + 0x78);
    if ((*(uint *)(param_1 + 0x78) & 1) == 1) {
        // bit 0 set → poisoned/slowed color (green-tinted)
        *(float *)((int)this_ + 0x48) = 0.3f;   // 0x3e99999a
        *(float *)((int)this_ + 0x4c) = 1.0f;   // 0x3f800000
        *(float *)((int)this_ + 0x50) = ((bVar4 & 2) == 2) ? 1.0f : 0.5f;
    }
    else if ((bVar4 & 2) == 2) {
        // bit 1 set → frozen color (blue-tinted)
        *(float *)((int)this_ + 0x48) = 0.3f;
        *(float *)((int)this_ + 0x4c) = 0.5f;
        *(float *)((int)this_ + 0x50) = 1.0f;
    }
    else {
        // ── BUG-FIX 2026-04-27: PORT del +N item-level glow logic IDA ───────
        // (Antes solo copiaba light directo y hacía UN render — el +9/+11 glow
        // visible del Mu Online viene de DOBLE render via RenderPartObjectBodyColor
        // con flags 0x44 + 0x48). Ver FUN_00504b50 IDA lines 415-510.
        //
        // ItemLevel se extrae de param_5 (flags con level en bits 3-6).
        // BUG-FIX: línea 52 ya hizo `param_5 = ((int)param_5 >> 3) & 0xf;` así que
        // param_5 ES el ItemLevel directamente. El shift adicional daba 0xb→1.
        uint ItemLevel = (uint)ItemLevelFx;

        bool didExtraRender = false;
        if (ItemLevel > 2 && effectType != 863) {
            if (ItemLevel < 5) {
                // +3, +4 — mezcla con factor _DAT_00552534 ≈ 0.4
                // Los 3 floats: (X, X*0.4, X*0.4) donde X = DAT_083a45d4 (anim brightness)
                float baseB = DAT_083a45d4;
                float mixR  = baseB;
                float mixG  = baseB * 0.60000002f;
                float mixB  = mixG;
                // Component-wise multiply: model.color[i] = mix[i] * light[i]
                *(float*)((int)this_ + 0x48) = mixR * param_3[0];
                *(float*)((int)this_ + 0x4c) = mixG * param_3[1];
                *(float*)((int)this_ + 0x50) = mixB * param_3[2];
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                didExtraRender = true;
            }
            else if (ItemLevel < 7) {
                // +5, +6 — mezcla diferente
                float baseB = DAT_083a45d4;
                float mixR  = baseB * 0.5f;
                float mixG  = baseB * 0.69999999f;
                float mixB  = baseB;
                *(float*)((int)this_ + 0x48) = mixR * param_3[0];
                *(float*)((int)this_ + 0x4c) = mixG * param_3[1];
                *(float*)((int)this_ + 0x50) = mixB * param_3[2];
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                didExtraRender = true;
            }
            else if (ItemLevel < 8) {
                // +7 — tint * 0.4 + 1 extra body color render
                *(float *)((int)this_ + 0x48) = param_3[0] * 0.80000001f;
                *(float *)((int)this_ + 0x4c) = param_3[1] * 0.80000001f;
                *(float *)((int)this_ + 0x50) = param_3[2] * 0.80000001f;
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                FUN_00504960(this_, param_1, param_2, param_4, 0x44, 1.0f, 0xffffffff);
                didExtraRender = true;
            }
            else if (ItemLevel == 8) {
                // +8 — same as +7 (caída por else)
                *(float *)((int)this_ + 0x48) = param_3[0] * 0.80000001f;
                *(float *)((int)this_ + 0x4c) = param_3[1] * 0.80000001f;
                *(float *)((int)this_ + 0x50) = param_3[2] * 0.80000001f;
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                FUN_00504960(this_, param_1, param_2, param_4, 0x44, 1.0f, 0xffffffff);
                didExtraRender = true;
            }
            else if (ItemLevel < 10) {
                // +9 — tint * 0.9 + DOUBLE body color render (0x44 y 0x48)
                *(float *)((int)this_ + 0x48) = param_3[0] * 0.89999998f;
                *(float *)((int)this_ + 0x4c) = param_3[1] * 0.89999998f;
                *(float *)((int)this_ + 0x50) = param_3[2] * 0.89999998f;
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                uint extraFlag = param_8 & 0x100u;
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x44, 1.0f, 0xffffffff);
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x48, 1.0f, 0xffffffff);
                didExtraRender = true;
            }
            else if (ItemLevel < 11) {
                // +10 — same pattern
                *(float *)((int)this_ + 0x48) = param_3[0] * 0.89999998f;
                *(float *)((int)this_ + 0x4c) = param_3[1] * 0.89999998f;
                *(float *)((int)this_ + 0x50) = param_3[2] * 0.89999998f;
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                uint extraFlag = param_8 & 0x100u;
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x44, 1.0f, 0xffffffff);
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x48, 1.0f, 0xffffffff);
                didExtraRender = true;
            }
            else if (ItemLevel < 12) {
                // +11 — además FUN_00504ac0 con flag 0x240 (extra mesh)
                *(float *)((int)this_ + 0x48) = param_3[0] * 0.89999998f;
                *(float *)((int)this_ + 0x4c) = param_3[1] * 0.89999998f;
                *(float *)((int)this_ + 0x50) = param_3[2] * 0.89999998f;
                FUN_00504130(this_, param_1, param_2, param_4, param_8);
                uint extraFlag = param_8 & 0x100u;
                FUN_00504ac0(this_, param_1, param_2, param_4, extraFlag | 0x240, 1.0f, 0xffffffff);
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x48, 1.0f, 0xffffffff);
                FUN_00504960(this_, param_1, param_2, param_4, extraFlag | 0x44, 1.0f, 0xffffffff);
                didExtraRender = true;
            }
        }

        if (!didExtraRender) {
            // ── Normal path (level 0-2): copy light directly ────────────────
            *(float *)((int)this_ + 0x48) = param_3[0];
            *(float *)((int)this_ + 0x4c) = param_3[1];
            *(float *)((int)this_ + 0x50) = param_3[2];
            FUN_00504130(this_, param_1, param_2, param_4, param_8);
        }
        // skip the unconditional FUN_00504130 below (we already called it)
        goto LAB_after_render;
    }

    // ── Main dispatch: setup + render ────────────────────────────────────────
    // Ghidra/IDA: FUN_00504130(this, entity, model_type, alpha, draw_flags)
    FUN_00504130(this_, param_1, param_2, param_4, param_8);
LAB_after_render: ;

    // ── Optional flashing tint for buffed entities ───────────────────────────
    // Aplica sólo si param_6 tiene bits 0..5 activos y el tipo no está en el
    // rango de skill-FX (0x310..0x316). Genera un pulso sinusoidal que pinta
    // por encima con FUN_00441e00 flags=0x42.
    short pulseType = *(short*)(param_1 + 2);
    if (((param_6 & 0x3f) != 0) && ((pulseType < 0x310) || (0x316 < pulseType))) {
        fVar10 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005528e0);
        fVar10 = (fVar10 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
        *(float *)((int)this_ + 0x48) = (float)fVar10;
        *(float *)((int)this_ + 0x4c) = (float)(fVar10 * (float10)_DAT_005528b8);
        *(float *)((int)this_ + 0x50) = (float)((float10)_DAT_0055256c - fVar10);

        // flags=0x42 (int flags, NOT a pointer!), f1=1.0, then anim data
        FUN_00441e00(this_, 0x42, 1.0f,
                     *(float*)(param_1 + 100),
                     *(float*)(param_1 + 0x68),
                     *(float*)(param_1 + 0x6c),
                     *(float*)(param_1 + 0x70),
                     -1.0f, 0xffffffff);
    }
}
