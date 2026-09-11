// Combat_AttackStage.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// AttackStage @ 0x00448930 (~866 lines) — main attack animation/effect handler
// Decompiled from Ghidra. Anti-tamper hash table ops (FUN_00403f80/4041e0/404280/404330) skipped.
// Phantom params: unaff_EBX/ESI/EDI/EBP/retaddr are anti-tamper artifacts, not real args.
// The function reads c->Skill (c+0x302), dispatches on its value to spawn
// effects, joints, sounds per weapon/skill type. Each case creates visual FX
// via Effect_Create (CreateEffect) or Joint_Create (CreateJoint).
static bool __cdecl AttackStage_legacy_mismatched(DWORD c, DWORD o) {
    int Hand = GetHandOfWeapon((int)o);

    // anti-tamper hash table — skipped (encrypt/decrypt c->Skill)
    BYTE skillByte = *(BYTE*)(c + 0x302);

    DAT_00559858 = 15; // g_iLimitAttackTime = 15

    // Models base
    DWORD modelsBase = DAT_05828d58;

    switch (skillByte) {

    // ── case '+' (0x2B) — sword trail / CreateJoint(0x10a) ──────────────
    case 0x2B: {
        short oType = *(short*)(o + 0x02);
        int modelIdx = (int)oType;
        // BMD* = Models + type * sizeof(BMD_entry)
        // model stride = type*0x2C (from LEA EDX,[EAX+EAX*2]; SHL EDX,4; SUB EDX,EAX => type*47 => *4 bytes)
        DWORD pModel = modelsBase + modelIdx * 0xBC;

        BYTE attackTime = *(BYTE*)(c + 0x2F5);

        if (attackTime == 8) {
            PlayBuffer(0x53, 0, 0);
        }

        // Spawn 3 CreateJoint(0x10a) lightning/beam joints when attackTime in [2..8]
        if (attackTime > 1 && attackTime < 9) {
            for (int i = 0; i < 3; i++) {
                float pos[3];
                pos[0] = *(float*)(o + 0x10);
                pos[1] = *(float*)(o + 0x14);
                pos[2] = *(float*)(o + 0x18) + _DAT_00552908;

                // Random offsets (-300..+300)
                pos[0] += (float)(rand() % 601 - 300);
                pos[1] += (float)(rand() % 601 - 300);
                pos[2] += (float)(rand() % 601 - 300);

                // Sin/cos offset from facing angle
                float angleZ = *(float*)(o + 0x24);
                float rad = angleZ * _DAT_0055253c;
                pos[0] -= sinf(rad) * _DAT_00552904;
                pos[1] += cosf(rad) * _DAT_00552904;

                float* oAngle = (float*)(o + 0x1C);
                Joint_Create(0x10a, pos, oAngle, (float*)(o + 0x1C),
                             (unsigned int)o, 0, 40.0f, (short)oType, 0);
            }
        }

        // BMD::TransformPosition for sword trail (attackTime < 9)
        if (attackTime < 9) {
            float localPos[3] = {0.0f, 0.0f, 0.0f};
            // ExtOption index: c->Weapon[Hand].ExtOption = *(BYTE*)(c + Hand*0x18 + 0x274)
            BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
            DWORD boneTransform2 = *(DWORD*)(o + 0x114);
            float (*boneMat)[4] = (float(*)[4])(boneTransform2 + extOpt * 0x30);
            float worldPos[3];
            float* swordPos = (float*)(o + 0x188); // o->m_vPosSword
            BMD__TransformPosition((void*)pModel, boneMat, localPos, worldPos, true);
            // Result stored in worldPos — used by trail system
            (void)swordPos; // referenced by original but result consumed internally
        }

        // Second transform for extended trail (attackTime in [6..12])
        if (attackTime > 5 && attackTime < 13) {
            float localPos2[3] = {0.0f, 0.0f, 0.0f};
            BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
            DWORD boneTransform2 = *(DWORD*)(o + 0x114);
            float (*boneMat)[4] = (float(*)[4])(boneTransform2 + extOpt * 0x30);
            float worldPos2[3];
            BMD__TransformPosition((void*)pModel, boneMat, localPos2, worldPos2, true);
        }
        break;
    }

    // ── default — early return checks, then set attackTime=15 ────────────
    default: {
        // Various early-return checks based on AnimationFrame, Type, CurrentAction
        float animFrame = *(float*)(o + 0x108);
        short oType = *(short*)(o + 0x02);
        BYTE curAction = *(BYTE*)(o + 0x105);

        if (animFrame < _DAT_0055256c && oType == 0x186 && curAction == '>') {
            // pass through to set attackTime = 15
        }
        else if (animFrame < _DAT_00552660) {
            return true;
        }
        else {
            if (oType == 0x186 && curAction >= 0x22 && curAction <= 0x5B) {
                // pass through
            }
            else if (oType >= 0x10E && oType <= 0x14E && curAction >= 3 && curAction <= 4) {
                // pass through (monster attack actions)
            }
            else {
                return true;
            }
        }
        // Fall through: set attackTime = 15
        *(BYTE*)(c + 0x2F5) = 0x0F;
        break;
    }

    // ── case '/' (0x2F) — slash effect with CreateEffect(0x1F0) + projectiles ──
    case 0x2F: {
        BYTE attackTime = *(BYTE*)(c + 0x2F5);
        short oType = *(short*)(o + 0x02);
        DWORD pModel = modelsBase + (int)oType * 0xBC;

        if (attackTime == 0x0A) {
            PlayBuffer(0x5F, 0, 0);
        }
        else {
            // Transform at attackTime == 4 (light color = 1.0, 1.0, 0.5)
            if (attackTime == 0x04) {
                float light[3] = {1.0f, 1.0f, 0.5f};
                float localPos[3] = {0.0f, 0.0f, 0.0f};
                BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
                DWORD bt2 = *(DWORD*)(o + 0x114);
                float (*boneMat)[4] = (float(*)[4])(bt2 + extOpt * 0x30);
                float worldPos[3];
                BMD__TransformPosition((void*)pModel, boneMat, localPos, worldPos, true);
                (void)light; // consumed by transform chain
            }

            // Spawn 2x CreateEffect(0x1F0) at attackTime == 8
            if (attackTime == 0x08) {
                float angleZ = *(float*)(o + 0x24);
                float rad = angleZ * _DAT_0055253c;
                float pos[3];
                pos[0] = *(float*)(o + 0x10) + sinf(rad) * _DAT_00552598;
                pos[1] = *(float*)(o + 0x14) - cosf(rad) * _DAT_00552598;
                pos[2] = *(float*)(o + 0x18) + _DAT_005528fc;
                float light[3] = {1.0f, 1.0f, 1.0f};

                Effect_Create(0x1F0, pos, (float*)(o + 0x1C), light,
                             NULL, (float*)o, NULL, NULL, 0);
                Effect_Create(0x1F0, pos, (float*)(o + 0x1C), light,
                             NULL, (float*)o, NULL, NULL, 0);
            }
        }

        // Spawn 3x CreateEffect(0x10A) when attackTime in [13..14]
        if (attackTime > 0x0C && attackTime < 0x0F) {
            // Weapon type from c->Weapon[Hand].Type
            int weapType = (int)*(short*)(c + Hand * 0x18 + 0x270);

            for (int i = 0; i < 3; i++) {
                float rad = *(float*)(o + 0x24) * _DAT_0055253c;
                float light[3] = {0.3f, 0.3f, 0.3f};
                float pos[3];
                pos[0] = *(float*)(o + 0x10) + sinf(rad) * _DAT_005528f8;
                pos[1] = *(float*)(o + 0x14) - cosf(rad) * _DAT_005528f8;
                pos[2] = *(float*)(o + 0x18) + _DAT_005528fc;

                // Random scatter when attackTime != 11
                if (attackTime != 0x0B) {
                    pos[0] += (float)(rand() % 60 - 30);
                    pos[1] += (float)(rand() % 60 - 30);
                    pos[2] += (float)(rand() % 60 - 30);
                }

                Effect_Create(0x10A, pos, (float*)(o + 0x1C), light,
                             (float*)(intptr_t)weapType, (float*)o, NULL, NULL, 0);
            }
            return true;
        }
        break;
    }

    // ── case '0' (0x30) — early attackTime check ────────────────────────
    case 0x30: {
        if (*(BYTE*)(c + 0x2F5) > 9
            && *(short*)(o + 0x02) == 0x186
            && *(BYTE*)(o + 0x105) == 0x3F) {
            *(BYTE*)(c + 0x2F5) = 0x0F;
            return true;
        }
        break;
    }

    // ── case '1' (0x31) — set attackTime to 15 if conditions met ────────
    case 0x31: {
        float animFrame = *(float*)(o + 0x108);
        if (animFrame < _DAT_00552660) return true;
        if (*(short*)(o + 0x02) != 0x186) return true;
        BYTE curAction = *(BYTE*)(o + 0x105);
        if (curAction == 0x40 || curAction == 0x41) {
            *(BYTE*)(c + 0x2F5) = 0x0F;
            return true;
        }
        // Fall through to default: attackTime = 15
        *(BYTE*)(c + 0x2F5) = 0x0F;
        break;
    }

    // ── case '4' (0x34) — combo effect CreateEffect(0x4F3) ─────────────
    case 0x34: {
        short oType = *(short*)(o + 0x02);
        BYTE curAction = *(BYTE*)(o + 0x105);

        // Force animation frame when in attack action range
        if (oType == 0x186 && curAction > 0x21 && curAction < 0x5C) {
            if (*(float*)(o + 0x108) >= _DAT_00552660) {
                *(float*)(o + 0x10C) = 4.0f; // PriorAnimationFrame
                *(float*)(o + 0x108) = 5.0f; // AnimationFrame
            }
        }

        if (*(BYTE*)(c + 0x2F5) == 0x03) {
            Effect_Create(0x4F3, (float*)(o + 0x10), (float*)(o + 0x1C),
                         (float*)(o + 0xE8), NULL, (float*)o, NULL, NULL, 0);
            PlayBuffer(100, (DWORD)o, 0);
        }

        DAT_00559858 = 5; // g_iLimitAttackTime = 5
        return true;
    }

    // ── case '7' (0x37) — summoner/special skill: CreateEffect(0x4F3) + CreateEffect(0x490) ──
    case 0x37: {
        if (*(short*)(o + 0x02) != 0x186) {
            DAT_00559858 = 15;
            return true;
        }
        if (*(BYTE*)(o + 0x105) != 0x3D) {
            DAT_00559858 = 15;
            return true;
        }

        BYTE attackTime = *(BYTE*)(c + 0x2F5);

        // Spawn effect when attackTime in [1..2]
        if (attackTime != 0 && attackTime < 3) {
            Effect_Create(0x4F3, (float*)(o + 0x10), (float*)(o + 0x1C),
                         (float*)(o + 0xE8), (float*)1, (float*)o, NULL, NULL, 0);
        }

        if (*(float*)(o + 0x108) < _DAT_00552540) {
            DAT_00559858 = 15;
            return true;
        }

        PlayBuffer(0x54, 0, 0);

        // FindHotKey(0x37) — find hotkey slot for skill '7'
        int hotKey = FindHotKey_stub(0x37);
        WORD pkKey = *(WORD*)(o + 0x86);

        Effect_Create(0x490, (float*)(o + 0x10), (float*)(o + 0x1C),
                     (float*)(o + 0xE8), (float*)(intptr_t)hotKey, (float*)o,
                     (float*)(intptr_t)(int)pkKey, NULL, NULL);

        // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt)

        // Final: update Models[0x186] bone data and set attackTime = 15
        DWORD modelData = *(DWORD*)(modelsBase + 0x186 * 0xBC + 0x30);
        *(float*)(modelData + 0x3D4) = (float)(int)c + _DAT_005528f4;
        *(BYTE*)(c + 0x2F5) = 0x0F;
        DAT_00559858 = 15;
        return true;
    }

    // ── case '8' (0x38) — five-fold energy blast: 5x CreateEffect(0xCB) ──
    case 0x38: {
        if (*(short*)(o + 0x02) == 0x186 && *(BYTE*)(o + 0x105) == 0x51) {
            float angleDir[3];
            angleDir[0] = *(float*)(o + 0x1C);
            angleDir[1] = *(float*)(o + 0x20);
            angleDir[2] = *(float*)(o + 0x24) - _DAT_005528e4;

            float* oPos = (float*)(o + 0x10);
            float* oLight = (float*)(o + 0xE8);

            // 5x CreateEffect(0xCB) at different angle offsets
            Effect_Create(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            Effect_Create(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            Effect_Create(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            Effect_Create(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            Effect_Create(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);

            PlayBuffer(0x54, 0, 0);
            *(BYTE*)(c + 0x2F5) = 0x0F;
        }
        DAT_00559858 = 15;
        return true;
    }

    } // end switch

    return true;
}

// IDA: FUN_00451f30 @ 0x00451F30 (~52 lines) — death particle burst (20 dust particles)
// When entity is dead (anim==6) and animation frame is in range [_DAT_00552658, _DAT_00552830),
// spawn 20 dust particles (type 0x4c5) at random offsets (-32..+31) from entity position.
// Sets entity light to (1.0, 1.0, 1.0) before each particle spawn.
// Combat_SpawnDeathDustParticles (IDA-activated, was Ghidra stub)
// 00448930 AttackStage — direct IDA switch (raw/00448930_AttackStage.c).
// The older AttackStage_legacy_mismatched above is intentionally not called.
bool __cdecl AttackStage_stub(DWORD c, DWORD o)
{
    const int hand = GetHandOfWeapon((int)o);
    const BYTE skill = *(BYTE*)(c + 770), stage = *(BYTE*)(c + 757);
    const short type = *(short*)(o + 2);
    const DWORD model = DAT_05828d58 + type * 188;
    DAT_00559858 = 15;
    if (skill == 43) {
        if (stage == 8) PlayBuffer(83, 0, 0);
        if (stage >= 2 && stage <= 8) {
            const float r = *(float*)(o + 36) * 0.017453292f;
            for (int i = 0; i < 3; ++i) {
                float p[3] = {*(float*)(o+16)+(float)(rand()%601-300)-sinf(r)*1400.0f, *(float*)(o+20)+(float)(rand()%601-300)+cosf(r)*1400.0f, *(float*)(o+24)+120.0f+(float)(rand()%601-300)};
                Joint_Create(266, p, p, (float*)(o+28), 2, (int)o, 40.0f, -1, 0);
            }
        }
        if (stage <= 8) {
            float z[3] = {};
            BMD_TransformPosition((void*)model, (float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)), z, (float*)(o+392), 1);
            const float r = *(float*)(o+36)*0.017453292f; *(float*)(o+392)+=sinf(r)*300.0f; *(float*)(o+396)-=cosf(r)*300.0f;
        }
        if (stage >= 6 && stage <= 12) {
            float z[3]={}, p[3], l[3]={1,1,1};
            BMD_TransformPosition((void*)model, (float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)), z, p, 1);
            const float r=*(float*)(o+36)*0.017453292f, d=(float)(stage-8)*10.0f+100.0f; p[0]+=sinf(r)*d; p[1]-=cosf(r)*d;
            Effect_Create(496,p,(float*)(o+28),l,(float*)1,(float*)o,(float*)-1,nullptr,0); Effect_Create(496,p,(float*)(o+28),l,(float*)1,(float*)o,(float*)-1,nullptr,0);
            const short target=*(short*)(c+784); if(target!=-1 && stage>=10) { BYTE* v=(BYTE*)(uintptr_t)CharactersClient+target*916; if(*v) v[404]=35; }
        }
    } else if (skill == 47) {
        if(stage==10) PlayBuffer(95,0,0);
        else if(stage==4) { float z[3]={},p[3],l[3]={1,1,0.5f}; BMD_TransformPosition((void*)model,(float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)),z,p,1); Effect_Create(497,p,(float*)(o+28),l,(float*)(intptr_t)*(short*)(c+8*(3*hand+78)),(float*)o,(float*)-1,nullptr,0); }
        else if(stage==8) { const float r=*(float*)(o+36)*0.017453292f; float p[3]={*(float*)(o+16)+sinf(r)*50.0f,*(float*)(o+20)-cosf(r)*50.0f,*(float*)(o+24)+110.0f},l[3]={1,1,1}; Effect_Create(496,p,(float*)(o+28),l,nullptr,(float*)o,(float*)-1,nullptr,0); Effect_Create(496,p,(float*)(o+28),l,nullptr,(float*)o,(float*)-1,nullptr,0); }
        if(stage>=13 && stage<=14) { const float r=*(float*)(o+36)*0.017453292f; for(int i=0;i<3;++i) { float p[3]={*(float*)(o+16)+sinf(r)*145.0f+(float)(rand()%60-30),*(float*)(o+20)-cosf(r)*145.0f+(float)(rand()%60-30),*(float*)(o+24)+110.0f+(float)(rand()%60-30)},l[3]={0.3f,0.3f,0.3f}; Effect_Create(266,p,(float*)(o+28),l,(float*)(intptr_t)*(short*)(c+8*(3*hand+78)),(float*)o,(float*)-1,nullptr,0); } }
    } else if(skill==48) { if(stage>9 && type==390 && *(BYTE*)(o+261)==63) *(BYTE*)(c+757)=15; }
    else if(skill==49) { if(*(float*)(o+264)>=5.0f && type==390 && (*(BYTE*)(o+261)==64 || *(BYTE*)(o+261)==65)) *(BYTE*)(c+757)=15; }
    else if(skill==52) { if(type==390 && *(BYTE*)(o+261)>=34 && *(BYTE*)(o+261)<=91 && *(float*)(o+264)>=5.0f){*(float*)(o+268)=4.0f;*(float*)(o+264)=5.0f;} if(stage==3){Effect_Create(1267,(float*)(o+16),(float*)(o+28),(float*)(o+232),nullptr,(float*)o,(float*)-1,nullptr,0);PlayBuffer(100,o,0);} DAT_00559858=5; }
    else if(skill==55) { if(type==390 && *(BYTE*)(o+261)==61) { if(stage && stage<=2) Effect_Create(1267,(float*)(o+16),(float*)(o+28),(float*)(o+232),(float*)1,(float*)o,(float*)-1,nullptr,0); if(*(float*)(o+264)>=3.0f){PlayBuffer(84,0,0); Effect_Create(1168,(float*)(o+16),(float*)(o+28),(float*)(o+232),nullptr,(float*)o,(float*)(intptr_t)*(short*)(o+134),(float*)(intptr_t)FindHotKey_stub(55),0); const DWORD modelState=DAT_05828d58?*(DWORD*)(DAT_05828d58+390*188+48):0; if(modelState && CharacterAttribute) *(float*)(modelState+980)=*(WORD*)((BYTE*)CharacterAttribute+56)*0.0040000002f+0.54000002f; *(BYTE*)(c+757)=15;} } }
    else if(skill==56) { if(type==390 && *(BYTE*)(o+261)==81){float a[3]={*(float*)(o+28),*(float*)(o+32),*(float*)(o+36)-40.0f};for(int i=0;i<5;++i){Effect_Create(203,(float*)(o+16),a,(float*)(o+232),(float*)2,(float*)o,(float*)-1,nullptr,0);a[2]+=20.0f;}PlayBuffer(84,0,0);*(BYTE*)(c+757)=15;} }
    // -- PENDIENTE: grupo de skills de magia del DLL (mejora, NO esta en IDA) --
    //
    // El `else if` de abajo es el `default:` literal de IDA (0x00448930 L356-364):
    // pone `c+757 = 15` -- o sea deja que el skill dispare -- cuando el frame de
    // la animacion llega a 5.0 con la accion en 0x22..0x5B.
    //
    // PROBLEMA MEDIDO (2026-09-03, sonda ANIMSPD): `SetAttackSpeed` (0x00443E70)
    // le da a las acciones de casteo (82-85) una PlaySpeed de
    //     AttackSpeed * 0.004 + 0.29
    // Con el AttackSpeed de este server eso da 3.97 frames por tick y la accion
    // tiene 6 frames, asi que el frame tras avanzar va 3.97 -> 0.97 -> 3.97 ...
    // y NUNCA cae en [5, 6).  La condicion no se cumple, `c+757` no llega a 15 y
    // el efecto del skill no se crea; ademas cada eco `0x1E` del server lo
    // resetea a 1.  Sintoma: manteniendo el click derecho no aparece animacion ni
    // efecto hasta soltar, y sale una sola vez.
    //
    // LO QUE HACE EL DLL (Source/Client/Main/Patchs.cpp, CPatchs::AttackStage,
    // enganchado con SetCompleteHook(0xE9, 0x00448930)): agrega un grupo de cases
    // que el binario NO tiene, y que fuerza el disparo sin esperar el frame 5.0:
    //
    //     case SKILL_POISON: case SKILL_METEORITE: case SKILL_LIGHTNING:
    //     case SKILL_FIRE_BALL: case SKILL_FLAME: case SKILL_ICE:
    //     case SKILL_TWISTER: case SKILL_EVIL_SPIRIT: case SKILL_POWER_WAVE:
    //     case SKILL_AQUA_BEAM: case SKILL_BLAST: case SKILL_INFERNO:
    //     case SKILL_ENERGY_BALL:
    //         *(BYTE*)(c + 0x2F5) = 15;   // c->AttackTime = 15
    //         break;
    //
    // Para implementarlo aca alcanza con un `else if` sobre esos 13 ids ANTES
    // del default, poniendo `*(BYTE*)(c+757) = 15`.  Queda como mejora del DLL
    // pendiente de decision (politica del proyecto: IDA manda, las mejoras del
    // DLL van al final).
    //
    // Dato del usuario para tener en cuenta al implementarlo: en versiones
    // viejas de MU este mismo problema de velocidad de ataque se evita usando
    // MONTURA, que cambia el set de acciones del casteo (y por lo tanto su
    // cuenta de frames).  Conviene verificar el caso montado antes de dar el
    // fix por completo.
    else if((*(float*)(o+264)>=1.0f && type==390 && *(BYTE*)(o+261)==62) || (*(float*)(o+264)>=5.0f && ((type==390 && *(BYTE*)(o+261)>=34 && *(BYTE*)(o+261)<=91) || (type>=270 && type<335 && *(BYTE*)(o+261)>=3 && *(BYTE*)(o+261)<=4)))) *(BYTE*)(c+757)=15;
    return true;
}

void __cdecl Combat_SpawnDeathDustParticles(int a1)
{
  int v1; // ebx
  float Position[3]; // [esp+4h] [ebp-Ch] BYREF

  if ( *(BYTE *)(a1 + 261) == 6 && *(float *)(a1 + 264) >= 8.0 && *(float *)(a1 + 264) < 9.0 )
  {
    v1 = 20;
    do
    {
      *(DWORD *)(a1 + 232) = 1065353216;
      *(DWORD *)(a1 + 236) = 1065353216;
      *(DWORD *)(a1 + 240) = 1065353216;
      Position[0] = (double)(rand() % 64 - 32) + *(float *)(a1 + 16);
      Position[1] = (double)(rand() % 64 - 32) + *(float *)(a1 + 20);
      Position[2] = (double)(rand() % 32 - 16) + *(float *)(a1 + 24);
      Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 1, 1.0, 0);
      --v1;
    }
    while ( v1 );
  }
}


// IDA: FUN_00452030 @ 0x00452030 (~30 lines) — idle ambient particle (single spark)
// When entity anim state == 2 (combat/active), spawn one particle (type 0x4c5)
// at random offset (0..199) minus grid scale from entity X/Y position.
void __cdecl Combat_SpawnIdleAmbientParticle(int param_1) {
    if (*(char*)(param_1 + 0x105) != '\x02') return;

    int r1 = rand();
    float fX = ((float)(r1 % 200) + *(float*)(param_1 + 0x10)) - _DAT_005524f0;

    int r2 = rand();
    float fZ = ((float)(r2 % 200) + *(float*)(param_1 + 0x14)) - _DAT_005524f0;

    float pos[3] = { fX, *(float*)(param_1 + 0x14), *(float*)(param_1 + 0x18) };
    float scale[3] = { 1.0f, 1.0f, 0.0f };
    float light[3] = { fX, fZ, 0.0f };
    Particle_Spawn(0x4c5, (float*)pos, (float*)scale, (float*)light,
                 *(int*)(param_1 + 0x18), 0.0f, r2 % 200);
}
