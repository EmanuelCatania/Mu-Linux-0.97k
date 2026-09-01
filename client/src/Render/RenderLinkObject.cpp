// RenderLinkObject.cpp
// FUN_00455430 @ 0x00455430  (642 decompiled lines, COMPLETO)
//
// Renders an equipped item / weapon model attached to a bone on a parent entity.
// Called from Player_Render / Entity_Render for each equipped part slot.
//
// Signature (Offsets.h canonical):
//   RenderLinkObject(float x, float y, float z, DWORD c, DWORD f,
//                    int Type, int Level, int Option1, bool Link,
//                    bool Translate, int RenderType)
//
// Parameters (mapped from Ghidra):
//   param_1/2/3  = x,y,z   — local offset position (used when Link==0)
//   param_4      = c        — entity OBJECT* (parent character)
//   param_5      = f        — PART slot pointer (equipped item desc)
//   param_6      = Type     — model index (DAT_05828d58 + Type*0xbc)
//   param_7      = Level
//   param_8      = Option1
//   param_9      = Link     — if nonzero: linked-bone path (builds angle matrix);
//                              if zero: plain bone-to-world transform path
//   param_10     = Translate — passed to BMD_Transform
//   param_11     = RenderType — render flags OR'd into FUN_00504b50 call
//
// PART slot offsets (param_5):
//   +0x04  BYTE  LinkBone       — bone index on parent model
//   +0x05  BYTE  (CurrentAction carried at param_5+5 → model+0xa0)
//   +0x06  BYTE  PriorAction
//   +0x08  float AnimationFrame
//   +0x0c  float PriorAnimationFrame
//   +0x10  float PlaySpeed
//
// OBJECT offsets (param_4):
//   +0x02  short Type (model index of parent)
//   +0x0c  DWORD (scale field → model+0x68)
//   +0x10  float world_x
//   +0x14  float world_y
//   +0x18  float world_z
//   +0x2a0 short char_class
//   +0x2eb BYTE  magic_channel_flag  (0x84-0x86 = cloaking/magic channel)
//   +0x105 BYTE  CurrentAction
//   +0x108 float AnimationFrame
//   +0x114 DWORD BoneTransform ptr   (float[][3][4], stride 0x30 per bone)
//   +0x34e char  SafeZone
//   +0xe4  BYTE  ContrastEnable → model+0x45
//   +0x168 float (alpha, passed to FUN_00504b50)
//   +0x24  float facing_angle
//   +0x302 BYTE  (param_4+0x302 = hash-table key / ref-count target)
//
// Model offsets (iVar7 = DAT_05828d58 + Type*0xbc):
//   +0x44  BYTE  LightEnable (set 0)
//   +0x45  BYTE  ContrastEnable (copied from entity +0xe4)
//   +0x54  DWORD (cleared for TYPE=0xea boss-head case)
//   +0x68  DWORD (Scale copied from entity +0xc)
//   +0x6c  float BodyOrigin.x
//   +0x70  float BodyOrigin.y
//   +0x74  float BodyOrigin.z
//   +0x84  DWORD (cleared = BodyHeight 0)
//   +0xa0  BYTE  CurrentAction (from PART +5)
//
// Key globals:
//   DAT_05828d58       — Models base pointer (stride 0xBC per BMD)
//   DAT_06970a9c       — BoneTransform global scratch array (stride 0x30)
//   DAT_06989c9c       — ParentMatrix scratch (root bone combine output)
//   DAT_05826e08       — g_AnimTick (boss-head angle spin)
//   DAT_07abf5c4/c8/cc — Boss-head position offset (x/y/z added to BodyOrigin)
//   DAT_0055a7ac       — g_GameSubState (wing particle suppression when ==10)
//   _DAT_00552580      — 0.0f guard constant
//   _DAT_00552940      — luminosity scale factor (rand%30+70)*factor
//
// Hash-table blocks (obfuscation, NOT game logic):
//   DAT_055c9bc8/bcc/bd0/bd4/bf0 — ref-count hash table on param_4+0x302
//   DAT_00559050 — XOR key used in ref-count manipulation
//   FUN_00405540 — hash-table error reporter ("Hash table full")
//   operator_new / operator_delete — used only in ref-count alloc/free
//   This entire block manipulates param_4+0x302 but the net effect on game
//   behavior is zero (anti-tamper). Faithfully transcribed from decompile.
//
// ─────────────────────────────────────────────────────────────────────────────
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <math.h>   // fsin

extern "C" void DbgLogPublic(const char*);
extern "C" int g_BackItemHand;   // 0/1: mano del item colgado en la espalda

// ─────────────────────────────────────────────────────────────────────────────

void __cdecl FUN_00455430(float param_1, float param_2, float param_3,
                          int param_4, int param_5, int param_6,
                          char param_7, unsigned int param_8, char param_9,
                          char param_10, unsigned int param_11)
{
    // BUGFIX 2026-04-26: el `return;` AUTO-SKIP estaba mal copiado del template
    // de Player_Render. Esta función NO tiene end-bound loop — es el render de
    // items linked al hueso (alas, armas, helper, escudo). Sin esto las alas
    // (model_idx=0x313..0x316) y armas no se dibujan en char-select ni in-game.
    // Ghidra emitía 642 líneas válidas que sí están en el cuerpo abajo.
    // ── local storage ─────────────────────────────────────────────────────────
    // afStack_264 layout used for various vec3/matrix temps throughout:
    //   [0..2]  = Light color vec3
    //   [3..5]  = Angle vec3  (input to Matrix_BuildFromEuler / BMD_TransformPosition)
    //   [6..8]  = Position vec3 (world pos scratch / BodyOrigin temp)
    //   [9..11] = extra (matches IDA `Position[3]` at ebp-240h)
    // BUGFIX 2026-04-26: era float[7] pero los callees (BMD_TransformPosition,
    // FUN_004795c0, Joint_Create) leen/escriben 3 floats desde
    // `afStack_264 + 6` → [6][7][8] OOB. /GS canary check tripeaba al return.
    // local_248/local_244 eran las falsas vars que Ghidra emitió por las
    // posiciones [7] y [8].
    float afStack_264[12];
    #define local_248 afStack_264[7]
    #define local_244 afStack_264[8]
    // BUGFIX 2026-04-27: local_240/_23c_f/_238_f eran 3 vars separadas (void* +
    // float + float). FUN_00440a30 escribe 3 floats consecutivos a través de
    // (float*)&local_240. Si el compilador NO ubica las 3 vars contiguas (no
    // está obligado), los writes 4-11 caen en stack canary u otros locales →
    // comportamiento NO determinístico entre builds (flicker variable, locales
    // pisados). Ahora un solo array contiguo. local_23c_f/_238_f redirigidos
    // vía macro al uso "como float", local_240 mantiene su uso "as void*" en
    // el bloque hash-table (lee/escribe los 4 bytes como pointer).
    float local_240_buf[3];  // pos_out: [0]=x, [1]=y, [2]=z (FUN_00440a30 target)
    #define local_240 (*(void**)&local_240_buf[0])
    #define local_23c_f local_240_buf[1]
    #define local_238_f local_240_buf[2]

    // BUGFIX 2026-04-26: era float[3] con local_21c/20c/1fc separadas; pero
    // Matrix_BuildFromEuler escribe 12 floats (matriz 3×4 [0..0xb]) → overflow masivo
    // dentro del propio buffer. Ahora declarado como matriz completa y los
    // accesos legacy local_21c/20c/1fc redirigen vía macro.
    float local_228[12];     // AngleMatrix output: 3×4 matrix, row-major
    #define local_21c local_228[3]   // matrix[0][3] — translation X
    #define local_20c local_228[7]   // matrix[1][3] — translation Y
    #define local_1fc local_228[11]  // matrix[2][3] — translation Z

    // BUGFIX 2026-04-26: afStack_204 era float[2], pero IDA `v70[3]` y los
    // callees (sub_4404E0 anim1/anim2) leen 3 floats. → OOB read garbage.
    float afStack_204[3];    // = IDA v70[3] — anim param scratch
    // BUGFIX 2026-04-26: el OBJECT local era unsigned char[2] + un short suelto.
    // `FUN_00502ba0` (ItemObjectAttribute) escribe hasta offset 0x168 (360 bytes)
    // → smasheaba TODO el frame, devolvía a Entity_RenderAll con param_1
    // corrupto a la siguiente lectura (+0xae). Ahora dimensionado al stride
    // del effect-entity pool (0x1bc = 444 bytes) y `local_1ea` redirigido al
    // offset +2 dentro del buffer (campo Type según IDA `o[1]`).
    unsigned char local_1ec[0x1c0];     // local OBJECT — ItemObjectAttribute target
    #define local_1ea (*(unsigned short*)(local_1ec + 2))
    float afStack_1dc[3];    // position scratch (passed to FUN_00440060/FUN_004404e0)

    float local_1c8;         // boss-head BoneHead flag
    float afStack_3c[15];    // BoneTransform output scratch (passed to FUN_004404e0)

    int   local_22c;         // cached model pointer = DAT_05828d58 + param_6*0xbc
    int   iVar7;             // model pointer (same as local_22c for most of fn)

    // ── 1. look up model, copy entity fields ──────────────────────────────────
    iVar7 = DAT_05828d58 + param_6 * 0xbc;
    *(unsigned char*)(iVar7 + 0x45) = *(unsigned char*)(param_4 + 0xe4);  // ContrastEnable
    *(unsigned int* )(iVar7 + 0x68) = *(unsigned int* )(param_4 + 0x0c);  // Scale
    *(unsigned char*)(iVar7 + 0xa0) = *(unsigned char*)(param_5 + 5);     // CurrentAction from PART
    *(unsigned int* )(iVar7 + 0x84) = 0;                                   // BodyHeight = 0

    // BUGFIX 2026-04-27: zero local_1ec antes de ItemObjectAttribute. La stack
    // tiene garbage cada llamada → ItemObjectAttribute no escribe TODOS los
    // campos del OBJECT struct, sólo los que le importan. Los bytes uninit
    // pueden cambiar comportamiento de FUN_00504b50 / FUN_00504130 entre frames
    // → flicker visible en weapons.
    memset(local_1ec, 0, sizeof(local_1ec));
    local_1ea = (unsigned short)param_6;  // OBJECT.Type = item type index
    local_22c = iVar7;

    // FUN_00502ba0: initialise local_1ec as a minimal OBJECT for the item.
    // It sets LightEnable, reads item attributes.
    FUN_00502ba0((int)local_1ec);

    *(unsigned char*)(iVar7 + 0x44) = 0;  // LightEnable = false

    // ── 2. bone position resolution ───────────────────────────────────────────
    if (param_9 == '\0')
    {
        // ── Non-linked path: transform local offset through owner bone ────────
        // afStack_264[6..8] = {param_1, param_2, param_3} = local offset vec3
        afStack_264[6] = param_1;
        local_248      = param_2;
        local_244      = param_3;

        // BoneTransform for LinkBone: (byte)(param_5+4) * 0x30 + *(int*)(param_4+0x114)
        // = BoneTransform base + LinkBone * stride
        float* pBoneMat = (float*)((unsigned int)*(unsigned char*)(param_5 + 4) * 0x30
                                   + *(int*)(param_4 + 0x114));

        void* ownerModel = (void*)(DAT_05828d58
                                   + (int)*(short*)(param_4 + 2) * 0xbc);

        // FUN_00440a30 = RotationPosition: transforms pos_in through bone → pos_out
        FUN_00440a30(ownerModel, pBoneMat, afStack_264 + 6, (float*)&local_240);

        // BodyOrigin = TransformedPosition + entity world position
        // BUGFIX 2026-04-26: era `(float)(int)local_240` que tomaba la
        // representación entera de los bits del float y la convertía a float
        // (devolvía 1065353216.0f para un 1.0f real). Ghidra había tipado
        // local_240 como void* y aplicó el cast equivocado. IDA línea 173
        // usa `Position[0]` directo. Reinterpretamos correctamente.
        *(float*)(iVar7 + 0x6c) = *(float*)&local_240 + *(float*)(param_4 + 0x10);
        *(float*)(iVar7 + 0x70) = local_23c_f         + *(float*)(param_4 + 0x14);
        *(float*)(iVar7 + 0x74) = local_238_f         + *(float*)(param_4 + 0x18);

    }
    else
    {
        // ── Linked path: build angle matrix based on weapon type ──────────────
        // Check magic-channel / cloaking flag: byte at param_4+0x2eb
        unsigned char magicFlag = *(unsigned char*)(param_4 + 0x2eb);
        bool isMagicChannel = (magicFlag >= 0x84) && (magicFlag <= 0x86);

        if (!isMagicChannel)
        {
            // Weapon type range dispatch for angle setup
            if ((param_6 >= 0x218 && param_6 <= 0x21e) ||
                (param_6 == 0x220) || (param_6 == 0x222))
            {
                // BOW range 0x218-0x21e / 0x220 / 0x222
                afStack_264[3] = _DAT_00552580;  // 0.0f
                afStack_264[4] = 20.0f;
                afStack_264[5] = 180.0f;
                Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                local_21c = -10.0f;
                local_20c =   8.0f;
                local_1fc =  40.0f;
            }
            else if (param_6 == 0x239)
            {
                // Special case 0x239
                afStack_264[3] = 110.0f;
                afStack_264[4] = 180.0f;
                afStack_264[5] =  90.0f;
                Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                local_21c = -10.0f;
                local_20c =   5.0f;
                local_1fc = -10.0f;
            }
            else
            {
                // General weapon / staff / shield path
                afStack_264[4] = _DAT_00552580;  // 0.0f
                afStack_264[3] = 70.0f;
                afStack_264[5] = 90.0f;
                Matrix_BuildFromEuler(afStack_264 + 3, local_228);

                if ((param_6 >= 0x210 && param_6 <= 0x22f) || param_6 == 0x238)
                {
                    // BOW sub-range
                    local_21c = -10.0f;
                    local_20c =   5.0f;
                    local_1fc =  10.0f;
                }
            else
            {
                // ── Tabla de poses de escudo / segundo item ─────────────────
                // PORT DEL DLL (CWeaponView::SecondWeaponViewFix), 2026-09-01.
                // El DLL engancha en 0x0045568B (la rama generica de abajo, que
                // vanilla resuelve con trans (-20, 5, 40)) y salta de vuelta a
                // 0x004556AA.  Constantes decodificadas de su bloque _asm:
                //   0xC20C0000=-35  0x41200000=10   0x41F00000=30  0xC1200000=-10
                //   0x43070000=135  0x42B40000=90   0xC1A00000=-20 0x42480000=50
                //   0xC1E00000=-28  0xC1C80000=-25  0xC2DC0000=-110 0x43340000=180
                //   0x41A00000=20   0x41700000=15   0x42200000=40  0x40A00000=5
                // Sin esto un escudo cae en la pose generica de arma
                // (Angle 70,0,90 / trans -20,5,40) y queda flotando por encima
                // de la cabeza.
                bool poseSet = false;

                if (param_6 >= 592 && param_6 < 624)     // GET_ITEM_MODEL(6,0)..(7,0)
                {
                    poseSet = true;
                    if (param_6 == 598) {                // Skull Shield
                        // sin AngleMatrix: conserva el angulo generico (70,0,90)
                        local_21c = -35.0f; local_20c = 10.0f; local_1fc = 0.0f;
                    }
                    else if (param_6 == 605) {           // Dragon Shield
                        afStack_264[3] =  30.0f;
                        afStack_264[4] = -10.0f;
                        afStack_264[5] = 135.0f;
                        Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                        local_21c = -10.0f; local_20c = 10.0f; local_1fc = 0.0f;
                    }
                    else if (param_6 == 608) {           // Elemental Shield
                        afStack_264[3] = 30.0f;
                        afStack_264[4] =  0.0f;
                        afStack_264[5] = 90.0f;
                        Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                        local_21c = -20.0f; local_20c = 0.0f; local_1fc = -20.0f;
                    }
                    else if (param_6 == 607 || param_6 == 606) {  // Grand Soul / Legendary
                        afStack_264[3] = 50.0f;
                        afStack_264[4] =  0.0f;
                        afStack_264[5] = 90.0f;
                        Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                        local_21c = -28.0f; local_20c = 10.0f; local_1fc = -25.0f;
                    }
                    else {                               // resto de escudos
                        local_21c = -10.0f; local_20c = 10.0f; local_1fc = 0.0f;
                    }
                }
                else if (g_BackItemHand != 0)            // = SecondWeaponFixVal
                {
                    poseSet = true;
                    afStack_264[3] = -110.0f;
                    afStack_264[4] =  180.0f;
                    afStack_264[5] =   90.0f;
                    Matrix_BuildFromEuler(afStack_264 + 3, local_228);
                    local_21c = 20.0f; local_20c = 15.0f; local_1fc = 40.0f;
                }

                if (!poseSet)
                {
                    // Vanilla (0x0045568B): arma en la mano 0.
                    local_21c = -20.0f;
                    local_20c =   5.0f;
                    local_1fc =  40.0f;
                }
            }
            }
        }
        else if ((param_6 == 0x23a) || (param_6 == 0x1a3))
        {
            // Cloaking/magic channel special types
            afStack_264[4] = _DAT_00552580;  // 0.0f
            afStack_264[3] = 90.0f;
            afStack_264[5] = 90.0f;
            Matrix_BuildFromEuler(afStack_264 + 3, local_228);
            local_21c =   0.0f;
            local_20c =  80.0f;
            local_1fc = 120.0f;
        }
        else if (param_6 == 0x222)
        {
            afStack_264[4] = _DAT_00552580;
            afStack_264[5] = _DAT_00552580;
            afStack_264[3] = 10.0f;
            Matrix_BuildFromEuler(afStack_264 + 3, local_228);
            local_21c =   0.0f;
            local_20c = 110.0f;
            local_1fc =  80.0f;
        }
        // else: local_228 (and offsets) left uninitialized — matches decompile
        // (the isMagicChannel path falls through without angle init for other types)

        // Translation column already written via local_21c/20c/1fc macros, which
        // alias local_228[3]/[7]/[11] (BUGFIX 2026-04-26 — see decl block).
        // FUN_004f9f70 takes: (bone_mat, angle_mat_12, out_parentmat)

        // BoneTransform for LinkBone
        float* pBoneMat = (float*)((unsigned int)*(unsigned char*)(param_5 + 4) * 0x30
                                   + *(int*)(param_4 + 0x114));

        // FUN_004f9f70: combine bone matrix with local angle matrix → ParentMatrix
        FUN_004f9f70(pBoneMat, local_228, (float*)&DAT_06989c9c);

        // BodyOrigin = entity world position (copy from entity, no local offset added)
        *(unsigned int*)(iVar7 + 0x6c) = *(unsigned int*)(param_4 + 0x10);
        *(unsigned int*)(iVar7 + 0x70) = *(unsigned int*)(param_4 + 0x14);
        *(unsigned int*)(iVar7 + 0x74) = *(unsigned int*)(param_4 + 0x18);
    }

    // ── 3. Boss-head special case (TYPE == 0xea = MODEL_BOSS_HEAD) ────────────
    local_1c8 = 0.0f;
    if (param_6 == 0xea)
    {
        *(float*)(iVar7 + 0x6c) += _DAT_07abf5c4;
        *(float*)(iVar7 + 0x70) += _DAT_07abf5c8;
        *(float*)(iVar7 + 0x74) += _DAT_07abf5cc;
        *(unsigned int*)(iVar7 + 0x54) = 0;      // BoneHead = 0
        local_1c8 = (float)DAT_05826e08;         // spinning angle = g_AnimTick
    }

    // ── 4. Hash-table ref-count block on param_4+0x302 (anti-tamper) ─────────
    // This is the obfuscation pattern (see CLAUDE.md). It manipulates a reference
    // count stored at param_4+0x302 via a hash table keyed on the pointer.
    // The block is transcribed faithfully; net game effect = zero.
    {
        unsigned char* pbVar1 = (unsigned char*)(param_4 + 0x302);
        unsigned int uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
        unsigned int uVar16 = 0;
        unsigned char* pbStack_26c = nullptr;
        unsigned char* pbStack_270 = pbVar1;

        // -- GetIndex loop --
        if (DAT_055c9bd4 != 0)
        {
            bool found = false;
            do {
                // compare pbStack_26c with hash slot key
                unsigned char* slotKey = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                int cmp = 0;
                bool eq = true;
                unsigned char* lhs = (unsigned char*)&pbStack_26c;
                for (int k = 4; k; --k)
                {
                    if (*lhs != *slotKey) { eq = false; cmp = (*lhs < *slotKey) ? -1 : 1; break; }
                    ++lhs; ++slotKey;
                }
                if (cmp == 0) { found = true; break; }

                // compare local_238 slot
                unsigned char* lhs2 = (unsigned char*)&local_238_f;
                unsigned char* slotKey2 = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                int cmp2 = 0; bool eq2 = true;
                for (int k = 4; k; --k)
                {
                    if (*lhs2 != *slotKey2) { eq2 = false; cmp2 = (*lhs2 < *slotKey2) ? -1 : 1; break; }
                    ++lhs2; ++slotKey2;
                }
                if (cmp2 == 0)
                {
                    if (uVar3 == 0xffffffff) { found = true; break; }
                    uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
                    pbStack_26c = nullptr;
                    uVar16 = 0;
                    if (DAT_055c9bd4 != 0) goto hash_getindex_loop2;
                    goto hash_getindex_done;
                }
                uVar3 = (uVar3 + 1) % DAT_055c9bd4;
                uVar16++;
            } while (uVar16 < DAT_055c9bd4);
            if (!false) // loop exhausted
                FUN_00405540((void*)&DAT_055c9bf0, "Hash table full, GetIndex");
            found = true;
        }

        // -- Insert ref slot --
        pbStack_26c = (unsigned char*)operator new(2);
        pbStack_26c[1] = 1;
        uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
        local_23c_f = 0.0f; // local_23c = null
        uVar16 = 0;

        if (DAT_055c9bd4 != 0)
        {
        hash_getindex_loop2:
            do {
                unsigned char* slotKey = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                int cmp = 0; bool eq = true;
                unsigned char* lhs = (unsigned char*)&pbStack_26c; // &stack0xfffffd8c equiv
                for (int k = 4; k; --k)
                {
                    if (*lhs != *slotKey) { eq = false; cmp = (*lhs < *slotKey) ? -1 : 1; break; }
                    ++lhs; ++slotKey;
                }
                if (cmp == 0)
                {
                    // LAB_00455ab1
                    *(unsigned char**)(DAT_055c9bcc + uVar3 * 4) = pbStack_270;
                    *(unsigned char**)(DAT_055c9bd0 + uVar3 * 4) = pbVar1;
                    goto hash_insert_done;
                }
                unsigned char* slotKey2 = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                int cmp2 = 0; bool eq2 = true;
                unsigned char* lhs2 = (unsigned char*)&local_23c_f;
                for (int k = 4; k; --k)
                {
                    if (*lhs2 != *slotKey2) { eq2 = false; cmp2 = (*lhs2 < *slotKey2) ? -1 : 1; break; }
                    ++lhs2; ++slotKey2;
                }
                if (cmp2 == 0)
                {
                    *(unsigned char**)(DAT_055c9bcc + uVar3 * 4) = pbStack_270;
                    *(unsigned char**)(DAT_055c9bd0 + uVar3 * 4) = pbVar1;
                    goto hash_insert_done;
                }
                uVar3 = (uVar3 + 1) % DAT_055c9bd4;
                uVar16++;
            } while (uVar16 < DAT_055c9bd4);
            FUN_00405540((void*)&DAT_055c9bf0, "Hash table full, Insert");
        }

    hash_insert_done:
        // Encode value and write back to param_4+0x302
        {
            // local_238 = CONCAT (first byte of *pbVar1 | prev local_238 low 3 bytes)
            // then local_23c = pbVar1
            // followed by a GetIndex + ref-count increment on local_23c / local_238

            // Simplified: increment ref-count slot [1] and conditionally obfuscate *pbVar1
            unsigned char* pbVar12 = nullptr;
            uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
            uVar16 = 0;

            // GetIndex for the newly inserted slot
            if (DAT_055c9bd4 != 0)
            {
                do {
                    unsigned char* slotKey = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                    // compare pbStack_270 equivalent
                    int cmp = 0; bool eq = true;
                    unsigned char* lhs = pbStack_270; // &stack0xfffffd88 equiv
                    unsigned char* rhs = slotKey;
                    for (int k = 4; k; --k)
                    {
                        if (*lhs != *rhs) { eq = false; cmp = (*lhs < *rhs) ? -1 : 1; break; }
                        ++lhs; ++rhs;
                    }
                    if (cmp == 0)
                    {
                        // LAB_00455bc1
                        goto hash_getindex2_found;
                    }
                    unsigned char* slotKey2 = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                    int cmp2 = 0; bool eq2 = true;
                    unsigned char* lhs2 = (unsigned char*)&local_240;
                    for (int k = 4; k; --k)
                    {
                        if (*lhs2 != *slotKey2) { eq2 = false; cmp2 = (*lhs2 < *slotKey2) ? -1 : 1; break; }
                        ++lhs2; ++slotKey2;
                    }
                    if (cmp2 == 0)
                    {
                        if (uVar3 == 0xffffffff) goto hash_getindex2_found;
                        *(void**)&local_240 = pbVar1;
                        uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
                        uVar16 = 0;
                        if (DAT_055c9bd4 != 0) goto hash_getindex2_loop;
                        goto hash_getindex2_exhaust;
                    }
                    uVar3 = (uVar3 + 1) % DAT_055c9bd4;
                    uVar16++;
                hash_getindex2_loop:;
                } while (uVar16 < DAT_055c9bd4);
                FUN_00405540((void*)&DAT_055c9bf0, "Hash table full, GetIndex");
            }

        hash_getindex_done:
        hash_getindex2_exhaust:
            pbVar12 = nullptr;

        hash_getindex2_found:
            // Increment ref-count and optionally decode/obfuscate *pbVar1
            {
                unsigned char bVar6 = pbVar12 ? pbVar12[1] : 0;
                if (pbVar12) pbVar12[1] = bVar6 + 1;
                if (pbVar12 && (unsigned char)(bVar6 + 1) < 2)
                {
                    unsigned char* pbVar4 = (unsigned char*)operator new(1);
                    unsigned char v = *pbVar12;
                    v = v - 0x23;
                    v = (v ^ PacketXorKey16[0]) + 0xb9;
                    *pbVar1 = v;
                    operator delete(pbVar4);
                }
            }
        }

        // -- Decrement ref-count (matching release) --
        {
            unsigned char* pbVar12 = nullptr;
            uVar3 = (**(unsigned int(__cdecl**)(unsigned char*))(DAT_055c9bc8 + 0x0c))(pbVar1);
            uVar16 = 0;
            if (DAT_055c9bd4 != 0)
            {
                do {
                    unsigned char* slotKey = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                    int cmp = 0;
                    unsigned char* lhs = pbStack_270;
                    unsigned char* rhs = slotKey;
                    for (int k = 4; k; --k)
                    {
                        if (*lhs != *rhs) { cmp = (*lhs < *rhs) ? -1 : 1; break; }
                        ++lhs; ++rhs;
                    }
                    if (cmp == 0) goto hash_decref_found;
                    unsigned char* slotKey2 = (unsigned char*)(DAT_055c9bd0 + uVar3 * 4);
                    int cmp2 = 0;
                    unsigned char* lhs2 = (unsigned char*)&local_240;
                    unsigned char* rhs2 = slotKey2;
                    for (int k = 4; k; --k)
                    {
                        if (*lhs2 != *rhs2) { cmp2 = (*lhs2 < *rhs2) ? -1 : 1; break; }
                        ++lhs2; ++rhs2;
                    }
                    if (cmp2 == 0)
                    {
                        if (uVar3 == 0xffffffff) goto hash_decref_found;
                        pbVar12 = *(unsigned char**)(DAT_055c9bcc + uVar3 * 4);
                        goto hash_decref_action;
                    }
                    uVar3 = (uVar3 + 1) % DAT_055c9bd4;
                    uVar16++;
                } while (uVar16 < DAT_055c9bd4);
                FUN_00405540((void*)&DAT_055c9bf0, "Hash table full, GetIndex");
            }
        hash_decref_found:
            pbVar12 = nullptr;
        hash_decref_action:
            if (pbVar12)
            {
                unsigned char bVar6 = pbVar12[1];
                pbVar12[1] = bVar6 - 1;
                if ((unsigned char)(bVar6 - 1) == 0)
                {
                    unsigned char* pbVar4 = (unsigned char*)operator new(1);
                    unsigned char v = *pbVar1 + 0x47;
                    v = (v ^ PacketXorKey16[0]) + 0x23;
                    int rval = rand();
                    *pbVar1 = (unsigned char)rval;
                    *pbVar12 = v;
                    operator delete(pbVar4);
                }
            }
        }
    } // end hash-table block

    // ── 5. Post-hash: animation-frame override for bow firing (link path) ─────
    // Check: local_23c high byte == '4' (0x34), entity type == 0x186, anim in range,
    //        anim frame in [_DAT_00552660 .. _DAT_00552488]
    // Maps to: if piercing skill active + bow-wielding + attack anim → force frame
    {
        // local_23c._3_1_ == '4' check: byte 3 of local_23c_f (the float storage)
        unsigned char highByte = ((unsigned char*)&local_23c_f)[3];
        bool cond = (highByte == '4')
                 && (*(short*)(param_4 + 2) == 0x186)
                 && (*(unsigned char*)(param_4 + 0x105) > 0x21)
                 && (*(unsigned char*)(param_4 + 0x105) < 0x5c)
                 && (_DAT_00552660 <= *(float*)(param_4 + 0x108))
                 && (*(float*)(param_4 + 0x108) <= _DAT_00552488);
        if (cond)
        {
            // Override PART animation frames: PriorAnimFrame = 2.0, AnimFrame = 3.2
            *(float*)((int)param_5 + 0x0c) = 2.0f;   // PriorAnimationFrame
            *(float*)((int)param_5 + 0x08) = 3.2f;   // AnimationFrame
        }
    }

    // ── 6. Ala 790 en zona segura → CurrentAction = 1 ────────────────────────
    // IDA L380: `if (c->Wing.Type == 790 && c->SafeZone) This->CurrentAction = 1`.
    // OJO: el gate mira el tipo del ALA (c+0x2A0) pero escribe sobre el modelo
    // que se está renderizando AHORA (iVar7), que puede ser el arma en la
    // espalda o el helper. Es un bug del 0.97k original: el DLL lo parchea en
    // 0x00455C19 (`FixImpAnimationWithWingDarknessInSafeZone`) agregando la
    // condición `Type == 790` para que sólo afecte al ala.
    // Se deja FIEL a IDA; si aparece el artefacto, el fix es esa condición.
    if ((*(short*)(param_4 + 0x2a0) > 0x315) && (*(short*)(param_4 + 0x2a0) < 0x317)
        && (*(char*)(param_4 + 0x34e) != '\0'))
    {
        *(unsigned char*)((int)iVar7 + 0xa0) = 1;
    }

    // ── 7. Animation tick (BMD_AnimTick) ─────────────────────────────────────
    // Condition: Link==0 OR Type not in BOW range 0x210..0x22f (excl. 0x238 exception)
    if ((param_9 == '\0') || (param_6 < 0x210) || (param_6 > 0x22f))
    {
        // FUN_00440aa0 = BMD_AnimTick: advance animation frame
        // Args: (model, AnimFrame*, PriorAnimFrame*, PriorAction*, PlaySpeed, Position, Angle)
        FUN_00440aa0((void*)iVar7,
                     (float*)((int)param_5 + 0x08),  // AnimationFrame
                     (float*)((int)param_5 + 0x0c),  // PriorAnimationFrame
                     (unsigned char*)((int)param_5 + 0x06),  // PriorAction
                     *(float*)((int)param_5 + 0x10)); // PlaySpeed
    }

    // ── 8. BMD_Animation (bone interpolation) ────────────────────────────────
    // FUN_00440060: __thiscall (model, BoneTransformArray, AnimFrame, PriorAnimFrame,
    //               PriorAction, Angle, HeadAngle, Parent=1, Translate=1)
    void* pModel = (void*)iVar7;
    FUN_00440060(pModel, (int)&DAT_06970a9c,
                 *(float*)((int)param_5 + 0x08),        // AnimationFrame
                 *(unsigned int*)((int)param_5 + 0x0c), // PriorAnimationFrame (as uint)
                 *(unsigned char*)((int)param_5 + 0x06), // PriorAction
                 (unsigned int*)afStack_1dc,             // Angle (reinterpreted)
                 afStack_1dc, '\x01', '\x01');

    // ── 9. BMD_Transform (compute world-space vertices) ──────────────────────
    // FUN_004404e0: (model, BoneTransformBase, anim1, anim2, bbox, Translate)
    FUN_004404e0(pModel, (int)&DAT_06970a9c,
                 afStack_204, afStack_204, afStack_3c, param_10);

    // ── 10. RenderPartObject ─────────────────────────────────────────────────
    // FUN_00504b50: (localObj, Type, Light*, alpha, 8*Level, Option1, 0, flags)
    {
        unsigned char monsterIndex = *(unsigned char*)(param_4 + 0x2eb);
        unsigned int baseRenderFlags = ((monsterIndex == 67) ? 0x102u : 0x2u);
        FUN_00504b50((int)local_1ec, (int)param_6,
                     (float*)(param_4 + 800),     // Light offset +0x320
                     *(float*)(param_4 + 0x168),  // alpha
                     8u * (unsigned int)(unsigned char)param_7,  // 8*Level
                     (unsigned char)param_8,                     // Option1
                     0,
                     baseRenderFlags | param_11);
    }

    // ── 11. Per-weapon-type particle effects ─────────────────────────────────
    int   iVar_rand = rand();
    void* pModel2 = (void*)iVar7;
    // Luminosity = (float)(rand()%30 + 70) * _DAT_00552940
    float fLum = (float)(iVar_rand % 0x1e + 0x46) * _DAT_00552940;
    // BUGFIX 2026-09-01: locales no contiguos que Ghidra separo.  Los 9 cases
    // del switch de abajo construian el `Light[3]` de IDA como TRES escalares
    // sueltos (ebp-270h / -26Ch / -268h) y pasaban `&pbStack_270_f` como vec3.
    // MSVC no garantiza ese layout, asi que CreateSprite leia G y B de basura:
    // el brillo de cada arma/escudo salia con color arbitrario.  El Grand Soul
    // Shield (tipo 607) deberia tirar a azul (Light[2] = fLum*2, ~3x el R/G) y
    // se veia blanco.
    float Light[3] = { 0.0f, 0.0f, 0.0f };

    // The switch is on param_6 (Type), Ghidra mis-typed as float; actual int cases:
    // 0x1a3=419, 0x1af=431, 0x1d7=471, 0x1fa=506,
    // 0x214=532, 0x215=533, 0x216=534, 0x221=545, 0x23a=570, 0x25f=607
    switch (param_6)
    {
    case 0x1a3:  // MODEL_STAFF+10 equivalent — fire particles on bones 0..9
    {
        Light[1] = fLum * _DAT_005528b8;
        afStack_264[3] = 0.0f;
        afStack_264[4] = 0.0f;
        afStack_264[5] = 0.0f;
        Light[2] = fLum * _DAT_005524f4;
        float* pfVar10 = (float*)&DAT_06970afc;
        Light[0] = fLum;
        do {
            afStack_264[0] = 0.4f;
            afStack_264[1] = 0.4f;
            afStack_264[2] = 0.4f;
            BMD_TransformPosition(pModel2, pfVar10, afStack_264 + 3, afStack_264 + 6, '\x01');
            int r1 = rand() % 3;
            if (r1 == 0)
            {
                int r2 = rand();
                FUN_004795c0(0x4cf, afStack_264 + 6, 0.6f, afStack_264, param_4,
                             (float)(r2 % 0x168), r1);
            }
            FUN_004795c0(0x47e, afStack_264 + 6, 2.0f, Light, param_4, 0, 0);
            pfVar10 += 0xc;
        } while ((int)pfVar10 < 0x6970c4c);

        unsigned char bVar6 = *(unsigned char*)(param_4 + 0x105);
        if (((bVar6 < 0x0d) || (bVar6 > 0x21)) && ((bVar6 < 0x38) || (bVar6 > 0x3c)))
        {
            afStack_264[3] = 0.0f;
            afStack_264[4] = 0.0f;
            afStack_264[5] = 0.0f;
            rand();
            BMD_TransformPosition(pModel2, (float*)&DAT_06970b2c, afStack_264 + 3, afStack_264, '\x01');
            BMD_TransformPosition(pModel2, (float*)&DAT_06970afc, afStack_264 + 3, afStack_264 + 6, '\x01');
            return;
        }
        break;
    }

    case 0x1af:  // MODEL_BOW+18 equivalent — wing trail particles
    {
        if ((*(char*)(param_4 + 0x105) == 'P') && (DAT_0055a7ac != 10))
        {
            unsigned int uv = (unsigned int)rand() & 0x80000001u;
            bool bEven = (uv == 0);
            if ((int)uv < 0)
                bEven = ((uv - 1) | 0xfffffffeu) == 0xffffffffu;
            if (bEven)
            {
                Light[0] = 1.0f; Light[1] = 1.0f; Light[2] = 1.0f;
                afStack_264[3] = 0.0f;
                afStack_264[4] = 0.0f;
                afStack_264[5] = 0.0f;
                BMD_TransformPosition(pModel2, (float*)&DAT_06970acc, afStack_264 + 3, afStack_264 + 6, '\x01');
                int r1 = rand();
                afStack_264[6] += (float)(r1 % 0x1e) - _DAT_00552834;
                int r2 = rand();
                int iVar8 = 4;
                local_248 += (float)(r2 % 0x1e) - _DAT_00552834;
                local_244 += _DAT_005524fc;
                do {
                    int r3 = rand();
                    afStack_264[1] = 0.0f;
                    afStack_264[2] = *(float*)(param_4 + 0x24);
                    afStack_264[0] = (float)(r3 % 0x3c + 0x96);
                    Joint_Create(0x4e9, afStack_264 + 6, afStack_264 + 6, afStack_264, 0, 0, 10.0f, -1, 0);
                    Particle_Spawn(0x497, afStack_264 + 6, afStack_264, Light, 0, 1.0f, 0);
                    iVar8--;
                } while (iVar8 != 0);
            }
        }
        break;
    }

    case 0x1d7:  // blue light pulse on bone
    {
        Light[1] = fLum * _DAT_005526e8;
        Light[2] = 0.0f;
        afStack_264[3] = 0.0f;
        afStack_264[4] = 0.0f;
        afStack_264[5] = 0.0f;
        Light[0] = fLum;
        BMD_TransformPosition(pModel2, (float*)&DAT_06970acc, afStack_264 + 3, afStack_264 + 6, '\x01');
        FUN_004795c0(0x47e, afStack_264 + 6, 2.0f, Light, param_4, 0, 0);
        Light[0] = 0.5f;
        Light[1] = 0.5f;
        float fVar15 = (float)sin((double)((float)DAT_05826e08 * _DAT_005528e0));
        Light[2] = 0.5f;
        FUN_004795c0(0x47e, afStack_264 + 6, fVar15 + _DAT_00552504, Light, param_4, 0, 0);
        return;
    }

    case 0x1fa:  // orbit sprite loop
    {
        Light[0] = fLum * _DAT_005526e4;
        afStack_264[3] = 0.0f;
        afStack_264[4] = 0.0f;
        afStack_264[5] = 0.0f;
        float* pfVar10 = (float*)&DAT_06970acc;
        Light[1] = fLum * _DAT_005524f4;
        Light[2] = fLum * _DAT_00552530;
        do {
            BMD_TransformPosition(pModel2, pfVar10, afStack_264 + 3, afStack_264 + 6, '\x01');
            FUN_004795c0(0x47e, afStack_264 + 6, 1.3f, Light, param_4, 0, 0);
            pfVar10 += 0xc;
        } while ((int)pfVar10 < 0x6970c4c);
        return;
    }

    case 0x214:  // BOW+4
    case 0x215:  // BOW+5
    case 0x216:  // BOW+6
    case 0x221:  // BOW+17
    {
        if (param_6 == 0x216)
        {
            Light[0] = fLum * _DAT_00552534;
            int iVar8 = 0x0d;
            Light[2] = fLum * _DAT_00552530;
            Light[1] = fLum;
            do {
                FUN_004553c0(pModel2, 0x4cf, iVar8, 1.0f, Light, param_4);
                iVar8++;
            } while (iVar8 < 0x13);
            return;
        }
        if (param_6 == 0x221)
        {
            Light[0] = fLum * _DAT_00552504;
            int iVar8 = 0x0d;
            Light[2] = fLum * _DAT_00552530;
            Light[1] = Light[0];
            do {
                FUN_004553c0(pModel2, 0x4cf, iVar8, 1.0f, Light, param_4);
                iVar8++;
            } while (iVar8 < 0x13);
            iVar8 = 5;
            do {
                FUN_004553c0(pModel2, 0x4cf, iVar8, 1.0f, Light, param_4);
                iVar8++;
            } while (iVar8 < 9);
            return;
        }
        // cases 0x214 and 0x215
        Light[1] = fLum * _DAT_00552534;
        Light[2] = fLum * _DAT_005526e4;
        Light[0] = fLum;
        FUN_004553c0(pModel2, 0x4cf, 2, 1.0f, Light, param_4);
        FUN_004553c0(pModel2, 0x4cf, 6, 1.0f, Light, param_4);
        return;
    }

    case 0x23a:  // MODEL_STAFF+10 alternate — orbit loop on DAT_06970a9c
    {
        Light[1] = fLum * _DAT_005528b8;
        int iVar7_loc = 0;
        Light[2] = fLum * _DAT_005524f4;
        Light[0] = fLum;
        do {
            afStack_264[0] = 0.4f;
            afStack_264[3] = (float)iVar7_loc * _DAT_0055284c - _DAT_005524ec;
            afStack_264[1] = 0.4f;
            afStack_264[2] = 0.4f;
            afStack_264[4] = -40.0f;
            afStack_264[5] = 0.0f;
            BMD_TransformPosition(pModel2, (float*)&DAT_06970a9c, afStack_264 + 3, afStack_264 + 6, '\x01');
            int r1 = rand() % 3;
            if (r1 == 0)
            {
                int r2 = rand();
                FUN_004795c0(0x4cf, afStack_264 + 6, 0.6f, afStack_264, param_4,
                             (float)(r2 % 0x168), r1);
            }
            FUN_004795c0(0x47e, afStack_264 + 6, 2.0f, Light, param_4, 0, 0);
            iVar7_loc++;
        } while (iVar7_loc < 10);
        return;
    }

    case 0x25f:  // electric charge effect
    {
        Light[0] = fLum * _DAT_00552534;
        Light[2] = fLum + fLum;
        afStack_264[3] = 15.0f;
        afStack_264[4] = -15.0f;
        afStack_264[5] = 0.0f;
        Light[1] = Light[0];
        BMD_TransformPosition(pModel2, (float*)&DAT_06970acc, afStack_264 + 3, afStack_264 + 6, '\x01');
        FUN_004795c0(0x4cf, afStack_264 + 6, 1.5f, Light, param_4, 0, 0);
        FUN_004795c0(0x47e, afStack_264 + 6, fLum + _DAT_005528f0, Light, param_4, 0, 0);
        return;
    }

    default:
        break;
    }

    return;
}
