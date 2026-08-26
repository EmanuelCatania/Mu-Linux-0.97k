// Combat_Targeting.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// CheckTarget @ 0x0049CAE0 (~51 lines) — validates attack target
// If SelectedCharacter!=-1: use character position. Otherwise: terrain pick.
bool __cdecl CheckTarget_stub(DWORD c) {
    // 0x0049CAE0 — Validates attack target
    // If SelectedCharacter != -1: use selected character's position as target.
    // Otherwise: do terrain ray pick to find target position.
    // c = entity base pointer (character struct)

    DWORD base = DAT_07abf5d0;  // CharactersClient
    int selIdx = SelectedCharacter;  // SelectedCharacter

    if (selIdx != -1) {
        // Use selected character's position
        DWORD selEntity = base + selIdx * 0x394;
        // Position at Object.Position: +0x10 (x), +0x14 (y), +0x18 (z)
        // 0049CAE0 stores target *grid* coordinates, not world coordinates.
        // The native calculation is `(int)(world * 0.01f)`.
        DAT_07e016c0 = (DWORD)(int)(*(float*)(selEntity + 0x10) * 0.01f);
        DAT_07e016c4 = (DWORD)(int)(*(float*)(selEntity + 0x14) * 0.01f);
        // Copy selected character position to entity's TargetPosition
        // TargetPosition at some offset in CHARACTER struct
        float posX = *(float*)(selEntity + 0x10);
        float posY = *(float*)(selEntity + 0x14);
        float posZ = *(float*)(selEntity + 0x18);
        // c->TargetPosition[0..2] — need to find offset
        // From Ghidra: c->TargetPosition[0], [1], [2]
        // These are at c + some offset; let's use the field offsets from Ghidra struct
        // In the decompile, it writes directly to c->TargetPosition which is member access
        // For raw pointer: typical target position offsets
        // IDA 0049CAE0 stores TargetPosition at +788/+792/+796, not in
        // Object.Angle (+0x28..+0x30).  The old offsets corrupt the model
        // transform whenever a character is selected as a skill target.
        *(float*)(c + 788) = posX;
        *(float*)(c + 792) = posY;
        *(float*)(c + 796) = posZ;
        return true;
    }

    // Terrain pick path
    FUN_004f9ac0('\x01');  // RenderTerrain(true) — init ray cast

    int gridX = (int)*(float*)&DAT_080ab288;     // SelectXF
    int gridY = (int)*(float*)&DAT_080ab28c;     // SelectYF
    bool hit = (bool)FUN_004f8480(
        *(int*)&DAT_080ab288,  // SelectXF as int (float bits)
        *(int*)&DAT_080ab28c,  // SelectYF as int (float bits)
        // 0049CAE0: RenderTerrainTile(SelectXF, SelectYF,
        //                             (int)SelectXF, (int)SelectYF, ...).
        // The previous port inverted these integer tile indices, so this
        // final verification used the height/map data from a different tile.
        gridX, gridY, 1.0f, 1, 1
    );

    if (hit) {
        *(float*)(c + 788) = DAT_083a4130;
        *(float*)(c + 792) = DAT_083a4134;
        *(float*)(c + 796) = DAT_083a4138;
        DAT_07e016c0 = (DWORD)(int)(DAT_083a4130 * 0.01f);
        DAT_07e016c4 = (DWORD)(int)(DAT_083a4134 * 0.01f);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 17
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 18 — Final 5 game functions (99.4% → 100% game code coverage)
// ═══════════════════════════════════════════════════════════════════════════════
