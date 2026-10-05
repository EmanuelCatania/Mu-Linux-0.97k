// Scene_ObjectTick.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Named aliases ────────────────────────────────────────────────────────────

// Object_MoveUpdate — alias for MoveObjects (FUN_004FF260, per-frame
// world-objects animation/render-update dispatcher). The CALLERS
// (Game_SceneUpdate / Game_EnterWorldTick / Game_CharSelectTick) want a
// per-frame objects tick, which IS MoveObjects (0x004FF260).
//
// Sólo con SceneFlag == 5: MoveObjects recorre la grilla de buckets de objetos
// (DAT_083a021c..), que sólo está poblada con un mundo cargado; en
// Login/CharSelect/Loading los punteros de la lista no son válidos.
extern void __stdcall MoveObjects(void);
void __cdecl Object_MoveUpdate(void) {
    if (SceneFlag == 5) {
        MoveObjects();
        return;
    }

    // Login / CharSelect: este camino queda inerte. Los objetos de la escena de
    // login no se mueven desde acá (el mover genérico crasheaba al segundo frame);
    // el logo y los brillos de los barcos salen del lado del render.
    if (SceneFlag == 2 || SceneFlag == 4) {
        return;
    }
}

