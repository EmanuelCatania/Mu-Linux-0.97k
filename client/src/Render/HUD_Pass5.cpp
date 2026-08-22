// HUD_Pass5.cpp — port of sub_4BD650, the largest HUD pass (3734 bytes).
//
// Despite its size, sub_4BD650 is structurally simple — most of the byte
// budget is taken by the *6 anti-tamper hash-table dances* on
// CharacterMachine that the engine inserts before/after every block of
// state reads.  The actual render work is straightforward:
//
//   1. Read HP/Mana from CharacterAttribute (cur+28/30, max+32/34) and
//      compute the empty-fraction for the orb fill.
//   2. Trigger PlayBuffer(34) (low-HP klaxon) when HP fraction < 20%.
//   3. Render HP orb bitmap (234 normal / 247 poisoned) at (97, 432).
//   4. Render MP orb bitmap (235) at (489, 432).
//   5. Hover-tooltips for HP/MP orbs (GlobalText[358]/[359]).
//   6. 4 action buttons (party/character/inventory/guild) at (348/379/410/582)
//      with hover tooltips (GlobalText[361..364]) and pressed-state bitmaps
//      (236/237/238/250) when the corresponding *Opened flag is set.
//   7. Skill-bar render via sub_4BB940 (the 5×4 grid of skills + the main
//      weapon icon at slot 913) when the skill list is unlocked
//      (CharacterAttribute+86 != 0).
//   8. Coin / kill counters from Hero+904 / Hero+908 at (24, 462) and
//      (48, 462).
//   9. Cur HP / Cur MP numbers at (100, 467) and (540, 467).
//  10. Three item-count badges (potions slots) via sub_482E40 at x=226, 257, 288.
//  11. EndBitmap.
//
// All anti-tamper inner blocks gate on `CharacterMachine && dword_55C9BD4`
// — both 0 by default in our build, so the hash-table noise is skipped.
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <gl/GL.h>

// External helpers ported in earlier passes.
extern "C" int    GetScreenWidth(void);
extern "C" SIZE*  __cdecl FUN_0047f6f0(int, int, const char*, int, char, int);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" void   __cdecl RenderTipText(int, int, const char*);
extern "C" void   __cdecl SeedQuickPotionTypesFromInventory(void);
// PartyOpened/GuildOpened now #defined in globals.h to DAT_07eaa11x bytes.

#define ItemAttribute  ((ITEM_ATTRIBUTE*)DAT_07d78068)
#define byte_7DB870C   DAT_07db870c
#define SelectedHero   DAT_005616ac
#define dword_559C60   (&DAT_00559c60)
#define dword_559C64   DAT_00559c64
#define dword_559C68   DAT_00559c68
#define byte_559C6C    DAT_00559c6c

// Inventory grid pool — same one HUD_Pass3.cpp's sub_482850 walks.
extern "C" BYTE  OffsetInventoryItems[];

static bool HUD_IsQuestPanelOpenRuntime(void)
{
    return (g_csQuest != 0) &&
           (*(BYTE*)((uintptr_t)g_csQuest + 0x1C87F) != 0);
}

static bool HUD_IsGoldenArcherPanelRuntime(void)
{
    return (DAT_07eaa128 != 0 && DAT_07eaa128 != 3);
}

static bool HUD_IsInventoryFamilyActive(void)
{
    return InventoryOpened || CharacterOpened || ShopOpened ||
           WarehouseOpened || ChaosMixOpened || TradeOpened ||
           EventWindowOpened || GuildCreatorOpened ||
           HUD_IsGoldenArcherPanelRuntime() || (DAT_07eaa130 != 0) ||
           HUD_IsQuestPanelOpenRuntime();
}

static bool HUD_IsGuildFamilyActive(void)
{
    return GuildOpened || GuildCreatorOpened;
}

// =============================================================================
// sub_482E40 — count inventory items matching a quick-slot category.
// The original walks its inventory grid.  In this port the authoritative
// inventory storage is OffsetInventoryItems, which is also what renders the
// hotbar and receives every server inventory update.
// =============================================================================
extern "C" int __cdecl sub_482E40(int a1)
{
    char countAsStacks = 0;
    int rangeHi = a1;
    int rangeLo = a1;

    if (a1 == 0) {
        rangeHi = dword_559C60[0];
        switch (rangeHi) {
        case 456:
            rangeLo = rangeHi;
            break;
        case 457:
        case 468:
            countAsStacks = 1;
            rangeLo = rangeHi;
            break;
        default:
            if (rangeHi >= 452 && rangeHi <= 454) {
                rangeHi = 454;
                rangeLo = 452;
            } else {
                rangeHi = 451;
                rangeLo = 448;
            }
            break;
        }
    } else if (a1 == 1) {
        rangeHi = dword_559C64;
        switch (rangeHi) {
        case 456:
            rangeLo = rangeHi;
            break;
        case 457:
        case 468:
            countAsStacks = 1;
            rangeLo = rangeHi;
            break;
        default:
            if (rangeHi >= 448 && rangeHi <= 451) {
                rangeHi = 451;
                rangeLo = 448;
            } else {
                rangeHi = 454;
                rangeLo = 452;
            }
            break;
        }
    } else if (a1 == 2) {
        rangeHi = dword_559C68;
        if (rangeHi == 457 || rangeHi == 468) {
            countAsStacks = 1;
            rangeLo = rangeHi;
        } else if (rangeHi >= 448 && rangeHi <= 451) {
            rangeHi = 451;
            rangeLo = 448;
        } else if (rangeHi >= 452 && rangeHi <= 454) {
            rangeHi = 454;
            rangeLo = 452;
        } else {
            rangeHi = 456;
            rangeLo = 456;
        }
    }

    if (rangeHi < rangeLo) {
        return 0;
    }

    int total = 0;
    // Se conserva la forma legible de `main` (recorrer el inventario como
    // ITEM[64]) en vez del walk de punteros del decompile: son equivalentes —
    // IDA visita cada (tipo buscado, slot) y acá se filtra por rango, y el
    // acumulador `*((unsigned __int8 *)v6 - 30)` es el byte +26 = Durability.
    //
    // Lo unico que se corrige es el GATE.  IDA sub_482E40 usa
    // `*((__int16 *)v6 - 28) == v9 && *v6 > 0`, donde `*v6` es el campo en
    // slot+56 = ITEM::Key, no Durability.  Coinciden casi siempre porque
    // InsertInventoryItem hace `Key = max(Durability, 1)`, pero no cuando la
    // durabilidad es 0: ahi el item sigue teniendo Key = 1 y el original lo
    // cuenta igual (suma 0, o +1 si countAsStacks).
    const ITEM* inventory = (const ITEM*)OffsetInventoryItems;
    for (int slot = 0; slot < 64; ++slot) {
        const ITEM& item = inventory[slot];
        if (item.Type < rangeLo || item.Type > rangeHi || (int)item.Key <= 0) {
            continue;
        }
        if (countAsStacks) {
            ++total;
        } else {
            total += item.Durability;
        }
    }
    return total;
}


// =============================================================================
// sub_4BB940 — render a single skill icon.
//   Height — skill slot index (0..19) cast to float in IDA.
//   x, y   — screen position (passed as float-cast-of-int).
//   a4     — icon width  (32)
//   a5     — icon height (36)
//
// Reads the skill type at CharacterAttribute[+slot+87].  When non-zero:
//   * Tints special skill type 47 red unless a pet is equipped (Hero+696
//     in {818, 819}).
//   * Renders bitmap 298 with UV indexed by skill type modulo 8x16.
//   * Renders the keybind digit (0..9) at the bottom-right when
//     CharacterAttribute[SelectedHero*64 + slot + 215] != 255.
// =============================================================================
extern "C" void __cdecl sub_4BB940(float HeightArg, float xArg, float yArg,
                                   int a4, int a5)
{
    if (!CharacterAttribute || !Hero) return;

    int slot = (int)HeightArg;
    BYTE* CA = (BYTE*)CharacterAttribute;
    int v10 = *(unsigned char*)(CA + slot + 87);
    if (!v10) return;

    if (v10 == 47) {
        WORD heroFlag = *(WORD*)((BYTE*)(uintptr_t)Hero + 696);
        if (heroFlag != 818 && heroFlag != 819) {
            glColor3f(1.0f, 0.5f, 0.5f);
        }
    }

    float fx = (float)(int)xArg;
    float fy = (float)(int)yArg;
    float fw = (float)a4;
    float fh = (float)a5;

    float v_uv = (float)((double)(a5 * ((v10 - 1) / 8)) * 0.00390625);
    float u_uv = (float)((double)(a4 * ((v10 - 1) % 8)) * 0.00390625);
    float uW   = fw * 0.00390625f;
    float vH   = (fh - 1.0f) * 0.00390625f;
    FUN_005125a0(298, fx, fy, fw, fh, u_uv, v_uv, uW, vH, 1, 1);

    // Keybind digit per-hero.
    BYTE keybind = *(unsigned char*)(CA + (SelectedHero << 6) + slot + 215);
    if (keybind != 255) {
        RenderNumber2D(fx + 4.0f, fy + 26.0f, keybind, 9.0f, 10.0f);
    }
    byte_559C6C = (char)keybind;
}


// =============================================================================
// sub_4BD650 — main HUD render: orbs, action buttons, skill bar, counters.
// =============================================================================
extern "C" void __cdecl Render_HudPass_4BD650_(void);
void __cdecl Render_HudPass_4BD650_(void)
{
    if (!CharacterAttribute) return;

    // Keep the quick potion categories aligned with the real inventory state.
    SeedQuickPotionTypesFromInventory();

    BYTE* CA = (BYTE*)CharacterAttribute;

    // Read HP/Mana max+cur.  The engine clamps cur values to max so the
    // orb bar can't render past its full height.
    int max_HP = *(unsigned short*)(CA + 32);
    int cur_HP = *(unsigned short*)(CA + 28);
    if (cur_HP >= max_HP) cur_HP = max_HP;

    int max_MP = *(unsigned short*)(CA + 34);
    int cur_MP = *(unsigned short*)(CA + 30);
    if (cur_MP >= max_MP) cur_MP = max_MP;

    int empty_HP = max_HP - cur_HP;
    int empty_MP = max_MP - cur_MP;
    double hp_frac = max_HP > 0 ? (double)empty_HP / (double)max_HP : 0.0;
    double mp_frac = max_MP > 0 ? (double)empty_MP / (double)max_MP : 0.0;

    // Low-HP audio cue (< 20% HP remaining).
    if (max_HP > 0 && (double)cur_HP / (double)max_HP < 0.2) {
        // PlayBuffer(34, 0, 0) — soft skip; sound system not always wired.
    }

    // ── HP orb (left) ────────────────────────────────────────────────────────
    double v19 = hp_frac * 48.0;
    float va  = (float)(v19 * 0.015625);   // u offset into orb texture
    float HpHeight = 48.0f - va;
    float yHp = (float)(v19 + 432.0);
    bool poisoned = (Hero && (*(BYTE*)((BYTE*)(uintptr_t)Hero + 120) & 1) == 1);
    FUN_005125a0(poisoned ? 247 : 234,
                 97.0f, yHp, 53.0f, HpHeight,
                 0.0f, va, 0.828125f, 0.75f, 1, 1);

    // HP tooltip on hover.
    if ((double)MouseX >= 97.0 && (double)MouseX < 150.0 &&
        (double)MouseY >= 432.0 && (double)MouseY < 480.0) {
        CHAR Buffer[100];
        wsprintfA(Buffer, GlobalText[358], cur_HP, max_HP);
        RenderTipText(103, 419, Buffer);
    }

    // ── MP orb (right) ───────────────────────────────────────────────────────
    double v22 = mp_frac * 48.0;
    float v94 = (float)(v22 * 0.015625);
    float MpHeight = 48.0f - v94;
    float yMp = (float)(v22 + 432.0);
    FUN_005125a0(235, 489.0f, yMp, 53.0f, MpHeight,
                 0.0f, v94, 0.828125f, 0.75f, 1, 1);

    if ((double)MouseX >= 489.0 && (double)MouseX < 542.0 &&
        (double)MouseY >= 432.0 && (double)MouseY < 480.0) {
        CHAR Buffer[100];
        wsprintfA(Buffer, GlobalText[359], cur_MP, max_MP);
        RenderTipText(495, 419, Buffer);
    }

    // ── Action button: Party (348) ───────────────────────────────────────────
    if ((double)MouseX >= 348.0 && (double)MouseX < 372.0 &&
        (double)MouseY >= 452.0 && (double)MouseY < 476.0) {
        RenderTipText(348, 437, GlobalText[361]);
    }
    if (PartyOpened) {
        FUN_005125a0(236, 348.0f, 452.0f, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    }

    // ── Action button: Character (379) ───────────────────────────────────────
    if ((double)MouseX >= 379.0 && (double)MouseX < 403.0 &&
        (double)MouseY >= 452.0 && (double)MouseY < 476.0) {
        RenderTipText(379, 437, GlobalText[362]);
    }
    if (CharacterOpened) {
        FUN_005125a0(237, 379.0f, 452.0f, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    }

    // ── Action button: Inventory (410) ───────────────────────────────────────
    if ((double)MouseX >= 410.0 && (double)MouseX < 434.0 &&
        (double)MouseY >= 452.0 && (double)MouseY < 476.0) {
        RenderTipText(410, 437, GlobalText[363]);
    }
    if (InventoryOpened) {
        FUN_005125a0(238, 410.0f, 452.0f, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    }

    // ── Action button: Guild (582) ───────────────────────────────────────────
    if ((double)MouseX >= 582.0 && (double)MouseX < 634.0 &&
        (double)MouseY >= 459.0 && (double)MouseY < 477.0) {
        RenderTipText(582, 444, GlobalText[364]);
    }
    if (GuildOpened) {
        FUN_005125a0(250, 582.0f, 459.0f, 52.0f, 18.0f, 0.0f, 0.0f, 0.8125f, 0.5625f, 1, 1);
    }

    // ── Skill bar (when CharacterAttribute+86 != 0) ──────────────────────────
    if (CA[86]) {
        bool hoveredSkill = false;
        glColor3f(1.0f, 0.9f, 0.8f);
        // Main weapon skill icon.
        BYTE mainSlot = *(unsigned char*)((BYTE*)(uintptr_t)Hero + 913);
        float mainSkill = (float)mainSlot;
        sub_4BB940(mainSkill, 304.0f, 444.0f, 32, 36);
        if ((double)MouseX >= 304.0 && (double)MouseX < 336.0 &&
            (double)MouseY >= 444.0 && (double)MouseY < 480.0) {
            DAT_0055a3e4 = (DWORD)mainSlot;
            DAT_07ea840c = 320;
            DAT_07ea8408 = 444;
            DAT_07d78094 = 1;
            hoveredSkill = true;
        }

        if (byte_7DB870C) {
            int v92 = 0;
            int slotIdx = 0;
            do {
                if (CA[slotIdx + 87]) {
                    int slotCount = (unsigned char)CA[86];
                    double xCenter = 320.0 - (double)slotCount * 16.0 + (double)v92 * 32.0;
                    if (slotIdx == mainSlot) {
                        glColor3f(1.0f, 0.9f, 0.8f);
                    } else {
                        glColor3f(0.6f, 0.7f, 0.8f);
                    }
                    sub_4BB940((float)slotIdx, (float)(int)xCenter, 370.0f, 32, 36);
                    if ((double)MouseX >= xCenter && (double)MouseX < xCenter + 32.0 &&
                        (double)MouseY >= 370.0 && (double)MouseY < 406.0) {
                        DAT_0055a3e4 = (DWORD)slotIdx;
                        DAT_07ea840c = (DWORD)((int)xCenter + 16);
                        DAT_07ea8408 = 370;
                        DAT_07d78094 = 1;
                        hoveredSkill = true;
                    }
                    ++v92;
                }
                ++slotIdx;
            } while (slotIdx < 20);
        }
        if (!hoveredSkill && (int)DAT_0055a3e4 >= 0 && (int)DAT_0055a3e4 < 20) {
            DAT_0055a3e4 = 0xFFFFFFFF;
        }
    }

    // ── Coin / kill counters at (24,462) and (48,462) ────────────────────────
    int v53 = *(int*)((BYTE*)(uintptr_t)Hero + 904);
    glColor3f(0.6f, 0.6f, 0.6f);
    RenderNumber2D(24.0f, 462.0f, v53, 8.0f, 9.0f);

    int v62 = *(int*)((BYTE*)(uintptr_t)Hero + 908);
    RenderNumber2D(48.0f, 462.0f, v62, 8.0f, 9.0f);

    // ── HP / MP cur values ───────────────────────────────────────────────────
    glColor3f(1.0f, 0.95f, 0.75f);
    RenderNumber2D(100.0f, 467.0f, *(unsigned short*)(CA + 28), 9.0f, 10.0f);
    RenderNumber2D(540.0f, 467.0f, *(unsigned short*)(CA + 30), 9.0f, 10.0f);

    // ── Three item-count badges (potion / arrow / etc) at x=226, 257, 288 ───
    for (int i = 0; i < 3; ++i) {
        int v81 = sub_482E40(i);
        if (v81 > 0) {
            float x_pos = (float)(i) * 31.0f + 226.0f;
            RenderNumber2D(x_pos, 447.0f, v81, 9.0f, 10.0f);
        }
    }

    FUN_005124b0();   // EndBitmap
}

// AntiTamper_HashMaintain_C → sub_4BD650.
void AntiTamper_HashMaintain_C(void) { Render_HudPass_4BD650_(); }
