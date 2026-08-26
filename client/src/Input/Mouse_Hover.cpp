// Mouse_Hover.cpp — FUN_004b0310 @ 0x004b0310
// Per-frame mouse cursor billboard render + hover target determination.
//
// Called every frame. Two responsibilities:
//   1. Render the cursor as a 2D billboard sprite.
//   2. Determine which entity/item is under the cursor and store hover targets.
//
// ── Cursor billboard ──────────────────────────────────────────────────────────
// FUN_004f8480(DAT_080ab288, DAT_080ab28c, screenX, screenY, 1.0f, 1, 1):
//   Returns nonzero if cursor is visible/active.
// If visible, calls FUN_00511710() (hide char anim sprite for cursor area),
// then FUN_004f8bb0(type=8, x, y, sx, sy, color, 0, alpha) to draw the quad.
//   - States 2/4/5 (login/charselect/ingame): fixed size based on DAT_07e11d5c
//   - States 1/3 (intro/loading): animated size using DAT_07e11d5c oscillation
//
// ── Hover targets ─────────────────────────────────────────────────────────────
// After cursor render, sets hover indices:
//   SelectedItem = item on ground (-1=none)
//   SelectedNpc = NPC / shop entity (-1=none)
//   SelectedCharacter = mob or player entity (-1=none)
//   SelectedOperate = special object (-1=none)
//   DAT_00559c58 = secondary hover (cleared if SelectedCharacter resets)
//
// Priority with Alt held (VK_MENU):
//   item-ground (FUN_004afa40) → NPC type 4 → mob type 0x22 → player type 1 → special
//
// Priority without Alt + swim/idle:
//   mob/player type 0x22 → type 1 → NPC type 4 → item-ground → special
//
// HashTable section (lines ~131-350 in original):
//   Anti-tamper XOR encode/decode block operating on DAT_07cf1ffc (0x584-byte
//   char-data buffer). Per CLAUDE.md policy, hash table operations are
//   reference-count obfuscation — not game logic. Omitted from implementation.
//
// After hover detection: if hover target found and DAT_00559c58 != -1,
//   calls FUN_004afb00() to process the pending click action.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>



// ─────────────────────────────────────────────────────────────────────────────

void FUN_004b0310(void)
{
    // ── 1. Cursor billboard render ────────────────────────────────────────────
    // 2026-04-29 DISABLED: el cursor billboard 3D (sprite en el suelo del tile
    // hovered) requiere DAT_07eab24c (BackTerrainHeight) que no se inicializa
    // en nuestro port. Crash AV en FUN_004f8740 al acceder al buffer null.
    // El cursor 2D (FUN_004bffa0) sigue funcionando normalmente.
    #if 0
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4 || DAT_005615c0 == 5)
    {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = FUN_004f8480(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            FUN_00511710();
            int frame = (DAT_005615c0 == 2) ? 1 : (DAT_07e11d5c + 1);
            float sz = (float)frame;
            FUN_004f8bb0(8, DAT_083a4130, DAT_083a4134, sz, sz, color, 0, 1.0f);
        }
    }
    if (DAT_005615c0 == 1 || DAT_005615c0 == 3) {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = FUN_004f8480(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            FUN_00511710();
            float base = (float)DAT_07e11d5c + (float)DAT_07e11d5c + _DAT_0055256c;
            float szX = ((int)base / 100) * 100 + _DAT_00552598;
            float szY = ((int)base / 100) * 100 + _DAT_00552598;
            FUN_004f8bb0(8, szX, szY, base, base, color, 0, 1.0f);
        }
    }
    #endif

    // ── 2. Reset hover targets ────────────────────────────────────────────────

    // 2026-05-06: añadido guard `c50 >= 0` para evitar OOB read cuando
    // SelectedCharacter == -1 (initial state). Antes se leía entity[+0x2fd] con
    // c50=-1 → puntero negativo → crash latente.
    if (DAT_00559c5c == '\0' || DAT_0055a7ac == 6) {
        // Cursor disabled or spectator state
        SelectedCharacter = -1;
        DAT_00559c58 = -1;
    } else if (SelectedCharacter >= 0 &&
               *(char *)(DAT_07abf5d0 + 0x2fd + SelectedCharacter * 0x394) == '\0' &&
               *(char *)(DAT_07abf5d0 + SelectedCharacter * 0x394 + 0x84) == '\x02') {
        // Current hover target is a valid alive monster.
        // 2026-05-06: REMOVED reset on IsClickPushed/DAT_083a42c4. La lógica
        // original IDA reseteaba aquí porque la detect que sigue inmediato
        // re-poblaría. Pero en nuestro port el detect a veces falla (terrain
        // filter, etc) → c50 quedaba -1 al click time → mob attack handler
        // no disparaba. Mantener el target HASTA que detect lo reemplace.
        // Keep the original transient hover state.  Leaving this selected
        // after the cursor moves away turns later ground clicks into a basic
        // attack against the stale mob.
        if (DAT_00559c58 == -1 ||
            DAT_083a42ac != '\0' || DAT_083a42d0 != '\0' ||
            *(char *)(DAT_07abf5d8 + 0x2fd) != '\0')
        {
            SelectedCharacter = -1;
        }
    } else if (SelectedCharacter >= 0) {
        // Current target is no longer valid (died or kind changed).
        DAT_00559c58 = -1;
        SelectedCharacter = -1;
    }

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedOperate = -1;

    // ── 3. Hover target detection ─────────────────────────────────────────────

    if (DAT_07d78094 != '\0') goto process_click;

    {
        SHORT altState = GetAsyncKeyState(0x12);  // VK_MENU = Alt
        if ((char)((unsigned short)altState >> 8) == (char)0x80)
        {
            // Alt held: item-on-ground first, then NPC, mob, player
            if (DAT_07e91388 == 0)
                SelectedItem = FUN_004afa40();
            if (SelectedItem == -1) {
                SelectedNpc = FUN_004afdc0(4);
                if (SelectedNpc == -1) {
                    SelectedCharacter = FUN_004afdc0(0x22);
                    if (SelectedCharacter != -1) goto done;
                    SelectedCharacter = FUN_004afdc0(1);
                    if (SelectedCharacter != -1) goto done;
                    SelectedOperate = FUN_004b0240();
                }
                goto check_click;
            }
            goto process_click;
        }

        // No Alt: check swim/idle move state
        unsigned char moveFlags = *(unsigned char *)(DAT_07abf5d8 + 0x1bc) & 7;
        if (moveFlags == 2 || moveFlags == 0)
        {
            // HashTable encode/decode block — anti-tamper obfuscation.
            // Operates on DAT_07cf1ffc (XOR-encoded char data, 0x584 bytes).
            // Omitted per project policy (hash table operations are not game logic).
        }

        // Fallthrough to secondary hover scan
        {
            // Item range check (iStack_10 between 0x19 and 0x1c — from hash table result)
            // local_20 / local_1c adjust entity type priority based on equip slot range
        }

        // ── Secondary hover without second password ───────────────────────────
        if (FUN_004e5980() == '\0') {
            // 2026-07-27: click sobre un item del suelo = levantarlo. El IDA
            // chequea el item ÚLTIMO, pero eso hace que cualquier mob/NPC cercano
            // en pantalla robe el hover y no se pueda levantar. Comportamiento MU
            // esperado (confirmado por el usuario): el item bajo el cursor tiene
            // PRIORIDAD. Lo chequeamos primero; si hay item, es pickup.
            if (DAT_07e91388 == 0) {
                SelectedItem = FUN_004afa40();
                if (SelectedItem != -1) goto check_click;   // item bajo el cursor → pickup
            }
            if (SelectedCharacter == -1) {
                SelectedCharacter = FUN_004afdc0(0x22);  // mob type
                if (SelectedCharacter == -1) {
                    SelectedCharacter = FUN_004afdc0(1);  // player type
                    if (SelectedCharacter != -1) goto done;
                    SelectedNpc = FUN_004afdc0(4);  // NPC type
                    if (SelectedNpc == -1) {
                        SelectedOperate = FUN_004b0240();
                    }
                    goto check_click;
                }
            }
            if (DAT_00559c58 != -1) goto process_click;
        }
        else goto process_click;
    }

check_click:
    if (SelectedCharacter == -1) {
        DAT_00559c58 = -1;
    }
    goto done;

process_click:
    FUN_004afb00();

done:
    if (SelectedCharacter == -1)
        DAT_00559c58 = -1;

}
