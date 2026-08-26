// Scene_ObjectTick.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Named aliases ────────────────────────────────────────────────────────────

// Object_MoveUpdate — alias for MoveObjects_stub (FUN_004FF260, per-frame
// world-objects animation/render-update dispatcher). The historical naming
// "Object_MoveUpdate" came from a Ghidra mis-id of FUN_0043E050 which is
// actually Movement_Tick (angle math). The CALLERS (Game_SceneUpdate /
// Game_EnterWorldTick / Game_CharSelectTick) want a per-frame objects tick,
// which IS MoveObjects (0x004FF260). Wire them here so the world-objects
// pool actually advances each frame.
//
// 2026-05-07 (revert): MoveObjects_stub iterates the world-objects bucket
// grid (DAT_083a021c..) which is only properly populated when a world is
// loaded (state 5 = in-game). During Login/CharSelect/Loading the bucket
// linked-list pointers are uninitialized garbage → AV in FUN_004fdc00 →
// FUN_0043e5c0 reading param_1 + 0x161. Gate on g_GameState == 5 so this
// is only active in-game where the pool is real.
extern void __stdcall MoveObjects_stub(void);
void __cdecl Object_MoveUpdate(void) {
    if (DAT_005615c0 == 5) {
        MoveObjects_stub();
        return;
    }

    // Login / CharSelect:
    // Keep this path inert for now. Multiple attempts to drive the login scene
    // objects from here ended in second-frame crashes, which strongly suggests
    // the original scene uses a narrower update path than the generic object
    // mover. We'll recover the logo / ship glows from the render side instead
    // of mutating login objects here.
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4) {
        return;
    }
}

