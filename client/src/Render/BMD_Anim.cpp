// BMD_Anim.cpp
//
// BMD animation tick + per-character animation dispatch.
// Moved from stubs.cpp lines 14074-14176 (B3 refactor 2026-05-07).
//
//   FUN_00440aa0 @ 0x00440AA0 — BMD::PlayAnimation (a.k.a. BMD_AnimTick).
//                                Advances frame counter on a model's current
//                                action. Wraps for looping actions, clamps for
//                                non-looping. Called by CharacterAnimation +
//                                others.
//   CharacterAnimation @ 0x00448600 — per-character per-frame animation tick.
//                                Reads model action speed, applies multipliers
//                                (fast-walk + action-63 slowdown), then calls
//                                FUN_00440aa0. Without this, character entities
//                                stay frozen in their initial frame.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_00440aa0 @ 0x00440AA0 — BMD::PlayAnimation (__thiscall, 7 args in original)
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
bool __cdecl FUN_00440aa0(void *This, float *AnimationFrame, float *PriorAnimationFrame,
                          void *PriorAction, float Speed) {
    // 00440AA0 initializes its return to true and changes it to false only
    // when the frame reaches a terminal/loop boundary.
    bool stillPlaying = true;
    if (!This || !AnimationFrame) return true;
    BYTE* self = (BYTE*)This;
    if (*(short*)(self + 0x26) == 0) return true;  // model has no animations
    BYTE action = *(BYTE*)(self + 0xa0);
    int actionsBase = *(int*)(self + 0x30);
    // BUG-FIX 2026-04-28: actionsBase puede ser garbage si el slot fue
    // parcialmente inicializado (ej. particle pool con stale data). Sanity-
    // check del rango: ptr válido en el address space del proceso (heap).
    if (actionsBase < 0x100000 || actionsBase > 0x7FFFFFFF) return true;
    short* pAction = (short*)(actionsBase + action * 0x10);
    if ((uintptr_t)pAction < 0x100000 || (uintptr_t)pAction > 0x7FFFFFFF) return true;
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
    *(float*)(self + 0xa4) = *AnimationFrame;
    *(short*)(self + 0xa8) = (short)(int)*AnimationFrame;
    return stillPlaying;
}

// ── CharacterAnimation @ 0x00448600 (port of IDA decomp, anti-tamper stripped) ─
// Per-character animation tick: reads model action speed, applies multipliers,
// then calls FUN_00440aa0 (BMD_AnimTick) which advances entity[+0x108] (frame).
// Without this, character entities stay frozen in their initial frame.
//
// IDA original (sub_448600): hash-table reference-count of `c+770` on entry/exit
// — pure obfuscation per CLAUDE.md, omitted here. Real work is the speed calc
// and the sub_440AA0 call.
extern "C" bool __cdecl CharacterAnimation(int c, int o)
{
    short type = *(short*)(o + 2);
    BYTE* model = (BYTE*)(uintptr_t)DAT_05828d58 + 188 * type;
    float speed = 0.0f;

    // model+38 = actionCount (short). If the model has actions, look up speed
    // for the current action (model+160 = active action byte) in the actions
    // table (model+48 = ptr; stride 0x10; field +4 = speed).
    // Note: offset semantics here are functional — they match what
    // FUN_005098c0 writes to (and what works for monsters). Don't change.
    short actionCount = *(short*)(model + 38);
    BYTE  modelAction = *(BYTE*)(o + 261);  // entity[+0x105] — current action
    int   actionsBase = (actionCount > 0) ? *(int*)(model + 48) : 0;
    if (actionCount > 0) {
        if (actionsBase) {
            speed = *(float*)(actionsBase + 16 * modelAction + 4);
            if (speed < 0.0f) speed = 0.0f;
        }
        // c[847] (==entity[+0x34F]): "fast walk" flag → 1.5× for actions 3..4
        if (*(BYTE*)(c + 847)) {
            BYTE act = *(BYTE*)(o + 261);
            if (act >= 3 && act <= 4) speed *= 1.5f;
        }
        // Action 63 with frame > 6.0 → half speed (anim slow-down at end)
        if (*(BYTE*)(o + 261) == 63 && *(float*)(o + 264) > 6.0f) speed *= 0.5f;
    }

    // FUN_00440aa0(model, &frame, &priorFrame, &priorAction, speed) — advances frame.
    // (IDA passes 7 args incl. Pos=o+16, HeadAngle=o+28; our 5-arg variant ignores them.)
    return FUN_00440aa0((void*)model, (float*)(o + 264), (float*)(o + 268),
                        (void*)(o + 262), speed);
}
