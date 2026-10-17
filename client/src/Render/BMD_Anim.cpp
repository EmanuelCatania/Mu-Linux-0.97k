// BMD_Anim.cpp
//
// BMD animation tick + per-character animation dispatch.
//
//   BMD__PlayAnimation @ 0x00440AA0 — BMD::PlayAnimation (a.k.a. BMD_AnimTick).
//                                Advances frame counter on a model's current
//                                action. Wraps for looping actions, clamps for
//                                non-looping. Called by CharacterAnimation +
//                                others.
//   CharacterAnimation @ 0x00448600 — per-character per-frame animation tick.
//                                Reads model action speed, applies multipliers
//                                (fast-walk + action-63 slowdown), then calls
//                                BMD__PlayAnimation. Without this, character entities
//                                stay frozen in their initial frame.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Entity/EntityView.h"

// BMD__PlayAnimation @ 0x00440AA0 — BMD::PlayAnimation (__thiscall, 7 args in original)
// Advances animation frame counter (*AnimationFrame) by Speed each call. If the frame
// crosses an integer boundary, saves PriorAction/PriorFrame. Wraps or clamps based on
// the action's loop flag (byte at action+0). Returns true if still playing, false when
// a non-looping action has ended.
//
// Per-frame logic (from Ghidra @ 0x00440aa0):
//   this+0x26 (short) : hasAnimations flag
//   this+0xa0 (byte)  : current action index
//   this+0x30 (int)   : actions array base (stride 0x10, frameCount at +8, loop at +0)
//   this+0xa4 (float) : current frame (duplicated)
//   this+0xa8 (short) : current frame rounded
//   DAT_0055256c / DAT_00552580 : frame/time constants (typically 1.0f)
//   DAT_005524f8                : terminal offset (0.001f)
//
// NOTE: original is __thiscall with extra Origin[3]/Angle[3] params that are unused in
// the body. Our forward-decl in functions.h takes 5 args (__cdecl, this passed as arg1).
bool __cdecl BMD__PlayAnimation(void *This, float *AnimationFrame, float *PriorAnimationFrame,
                          void *PriorAction, float Speed) {
    // 00440AA0 initializes its return to true and changes it to false only
    // when the frame reaches a terminal/loop boundary.
    bool stillPlaying = true;
    if (!This || !AnimationFrame) return true;
    BYTE* self = (BYTE*)This;
    if (AsModel(self)->NumActions == 0) return true;  // model has no animations
    BYTE action = AsModel(self)->CurrentAction;
    int actionsBase = *(int *)&AsModel(self)->Actions;
    // Guarda propia: actionsBase puede ser basura si el slot fue parcialmente
    // inicializado (ej. particle pool con stale data). Sólo se descartan
    // direcciones bajas: en Linux x86 el heap puede estar por encima de 2 GB.
    if ((uintptr_t)(unsigned int)actionsBase < 0x100000u) return true;
    short* pAction = (short*)(uintptr_t)((unsigned int)actionsBase + action * 0x10u);
    short frameCount = *(short*)((BYTE*)pAction + 8);
    if (frameCount <= 1) return true;

    int priorIntFrame = (int)*AnimationFrame;
    *AnimationFrame += Speed;
    int newIntFrame = (int)*AnimationFrame;
    if (priorIntFrame != newIntFrame) {
        if (PriorAction) *(BYTE*)PriorAction = action;
        if (PriorAnimationFrame) *PriorAnimationFrame = (float)priorIntFrame;
    }
    if (*AnimationFrame <= 0.0f) {
        *AnimationFrame = ((float)frameCount - 1.0f) + *AnimationFrame;
    }
    char* actFlags = (char*)pAction;
    if (actFlags[0] == '\0') {
        // Looping action
        int loopLen = (int)frameCount - (actFlags[10] != '\0' ? 1 : 0);
        if ((float)loopLen <= *AnimationFrame) {
            stillPlaying = false;
            int intF = (int)*AnimationFrame;
            *AnimationFrame = (*AnimationFrame - (float)intF) + (float)(intF % loopLen);
        }
    } else {
        // Non-looping action: clamp near end
        if ((float)frameCount <= *AnimationFrame) {
            stillPlaying = false;
            *AnimationFrame = (float)frameCount - 0.01f;
        }
    }
    AsModel(self)->CurrentAnimation = *AnimationFrame;
    AsModel(self)->CurrentAnimationFrame = (short)(int)*AnimationFrame;
    return stillPlaying;
}

// ── CharacterAnimation @ 0x00448600 (port of IDA decomp, anti-tamper stripped) ─
// Per-character animation tick: reads model action speed, applies multipliers,
// then calls BMD__PlayAnimation (BMD_AnimTick) which advances entity[+0x108] (frame).
// Without this, character entities stay frozen in their initial frame.
//
// IDA original (sub_448600): hash-table reference-count of `c+770` on entry/exit
// — pure anti-tamper obfuscation, omitted here. Real work is the speed calc
// and the sub_440AA0 call.
extern "C" bool __cdecl CharacterAnimation(int c, int o)
{
    short type = AsObject(o)->Type;
    BYTE* model = (BYTE*)(uintptr_t)DAT_05828d58 + 188 * type;
    float speed = 0.0f;

    // model+38 = actionCount (short). If the model has actions, look up speed
    // for the current action (model+160 = active action byte) in the actions
    // table (model+48 = ptr; stride 0x10; field +4 = speed).
    // Note: offset semantics here are functional — they match what
    // OpenMonsterModel writes to (and what works for monsters). Don't change.
    short actionCount = *(short*)(model + 38);
    BYTE  modelAction = AsObject(o)->CurrentAction;  // entity[+0x105] — current action
    int   actionsBase = (actionCount > 0) ? *(int*)(model + 48) : 0;
    if (actionCount > 0) {
        if (actionsBase) {
            speed = *(float*)(actionsBase + 16 * modelAction + 4);
            if (speed < 0.0f) speed = 0.0f;
        }
        // c[847] (==entity[+0x34F]): "fast walk" flag → 1.5× for actions 3..4
        if (AsCharacter(c)->Change) {
            BYTE act = AsObject(o)->CurrentAction;
            if (act >= 3 && act <= 4) speed *= 1.5f;
        }
        // Action 63 with frame > 6.0 → half speed (anim slow-down at end)
        if (AsObject(o)->CurrentAction == 63 && AsObject(o)->AnimationFrame > 6.0f) speed *= 0.5f;
    }

    // IDA: AttackStage (0x00448930) default: el ataque sale cuando el frame de una
    // acción 34..91 llega a 5, y el disparo (MoveCharacter) exige que la acción
    // siga siendo de ataque. DESVIACIÓN: con la velocidad de ataque de MuEmu
    // (F3/E1) el avance por tick puede pasar del frame 5 al final de la acción
    // en un solo tick (arco a 535: 0 -> 2.44 -> 4.88 -> fin), y el héroe vuelve
    // a quieto sin disparar. Si el avance va a cruzar el 5, en este tick avanza
    // sólo hasta 5, como IDA frena en 5 a Penetration (skill 52). Quedan fuera
    // los skills con etapas propias en AttackStage.
    if (type == 390 && EntityView((const void*)c).Kind() == EntityView::PlayerKind &&
        modelAction >= 34 && modelAction <= 91 && AsCharacter(c)->SkillStage != 0) {
        const BYTE skill = AsCharacter(c)->Skill;
        const bool ownStages = skill == 43 || skill == 47 || skill == 48 || skill == 49 ||
                               skill == 52 || skill == 55 || skill == 56;
        if (!ownStages && AsObject(o)->AnimationFrame < 5.0f &&
            AsObject(o)->AnimationFrame + speed > 5.0f) {
            speed = 5.0f - AsObject(o)->AnimationFrame;
        }
    }

    // BMD__PlayAnimation(model, &frame, &priorFrame, &priorAction, speed) — advances frame.
    // (IDA passes 7 args incl. Pos=o+16, HeadAngle=o+28; our 5-arg variant ignores them.)
    bool bPlaying = BMD__PlayAnimation((void*)model, (float*)(o + 264), (float*)(o + 268),
                        (void*)(o + 262), speed);

    return bPlaying;
}
