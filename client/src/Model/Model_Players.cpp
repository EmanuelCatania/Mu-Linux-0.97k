// Model_LoadPlayers @ 0x005079d0
// Loads all weapon, shield, item, wing, helper and miscellaneous item models
// into slots 0x190-0x3bd.  Uses FUN_00506050 (numbered prefix loader) and
// FUN_00505e90 (named SMD loader) for Korean-locale files, then
// FUN_005060b0 (BMD index loader) for compressed-asset paths.
// Note: "Players" here is a misnomer — this function loads item/weapon geometry.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

void __cdecl FUN_005079d0(void)
{
    // ── Korean SMD item models ────────────────────────────────────────────────
    if (DAT_0055a7c4 == '\0') {
        // Swords (slots 0x190-0x1a0, numbered prefix)
        for (int i = 0x190; i-0x18e < 3; i++)
            FUN_00506050((int)i, "Data2\\Item\\Sword\\Sword", i - 0x18e);
        FUN_00505e90((int)0x193, "Data2\\Item\\Sword\\", "sword05.smd");
        for (int i = 0x194; i-0x18e < 0x10; i++)
            FUN_00506050((int)i, "Data2\\Item\\Sword\\Sword", i - 0x18e);
        FUN_00505e90((int)0x1a0, "Data2\\Item\\", "\xB5\xE6\xB0\xFC.smd"); // µæ°ü.smd

        // Axes (0x1b0-0x1b9)
        for (int i = 0x1b0; i-0x1af < 9; i++)
            FUN_00506050((int)i, "Data2\\Item\\Axe\\axe", i - 0x1ae);

        // Maces (0x1d0-0x1d6)
        for (int i = 0x1d0; (int)i < 0x1d2; i++)
            FUN_00506050((int)i, "Data2\\Item\\Mace\\Mace", i - 0x1ce);
        FUN_00505e90((int)0x1d2, "Data2\\Item\\Mace\\", "flail02.smd");
        FUN_00506050((int)0x1d3, "Data2\\Item\\Mace\\Maul", 3);
        FUN_00505e90((int)0x1d4, "Data2\\Item\\Axe\\", "\xBF\xB9\xBD\xBA\xC5\xC0.smd");  // ¿¹½ºÅÀ
        FUN_00505e90((int)0x1d5, "Data2\\Item\\Axe\\", "\xC7\xD9.smd");                   // Çù
        FUN_00505e90((int)0x1d6, "Data2\\Item\\",      "\xB5\xE6\xB0\xFC\xB5\xBF.smd");  // µæ°üµ¿

        // Spears (0x1f0-0x1f9)
        for (int i = 0x1f0; i-0x1ef < 5; i++)
            FUN_00506050((int)i, "Data2\\Item\\Spear\\Spear", i - 0x1ef);
        for (int i = 0x1f5; i-0x7d < 4; i++)
            FUN_00506050((int)i, "Data2\\Item\\Spear\\Polearms", i - 499);
        FUN_00505e90((int)0x1f9, "Data2\\Item\\Spear\\", "\xC3\xA3\xC0\xDC.smd"); // ÃãÀÜ

        // Shields (0x250-0x25e)
        for (int i = 0x250; i-0x24f < 0xe; i++)
            FUN_00506050((int)i, "Data2\\Item\\Shield\\Shield", i - 0x24f);
        FUN_00505e90((int)0x25e, "Data2\\Item\\Shield\\", "\xBD\xBA\xC5\xC0\xBD\xBA.smd"); // ½ºÅÀ½º

        // Staffs/Wands (0x230-0x238)
        for (int i = 0x230; i-0x22f < 5; i++)
            FUN_00506050((int)i, "Data2\\Item\\Staff\\wand", i - 0x22e);
        FUN_00505e90((int)0x235, "Data2\\Item\\staff\\", "\xBD\xBA\xC5\xC0\xBD\xBA.smd");
        FUN_00505e90((int)0x236, "Data2\\Item\\staff\\", "\xBD\xBA\xC5\xC0\xBD\xBA.smd");
        FUN_00505e90((int)0x237, "Data2\\Item\\",        "\xBD\xBA\xC5\xC0\xBD\xBA.smd");
        FUN_00505e90((int)0x238, "Data2\\Item\\",        "\xBD\xBA\xC5\xC0\xBD\xBA.smd");

        // Bows (0x210-0x220)
        FUN_00505e90((int)0x210, "Data2\\Item\\Bow\\", "bow01.smd");
        FUN_00505e90((int)0x211, "Data2\\Item\\Bow\\", "bow02.smd");
        FUN_00505e90((int)0x212, "Data2\\Item\\Bow\\", "bow03.smd");
        FUN_00505e90((int)0x213, "Data2\\Item\\Bow\\", "bow04.smd");
        FUN_00505e90((int)0x214, "Data2\\Item\\Bow\\", "bow05.smd");
        FUN_00505e90((int)0x215, "Data2\\Item\\Bow\\", "bow06.smd");
        FUN_00505e90((int)0x216, "Data2\\Item\\",      "\xBD\xBA\xC5\xC0\xBD\xBA.smd"); // arrows(?)
        FUN_00505e90((int)0x218, "Data2\\Item\\Bow\\", "crossbow01.smd");
        FUN_00505e90((int)0x219, "Data2\\Item\\Bow\\", "crossbow02.smd");
        FUN_00505e90((int)0x21a, "Data2\\Item\\Bow\\", "crossbow03.smd");
        FUN_00505e90((int)0x21b, "Data2\\Item\\Bow\\", "crossbow04.smd");
        FUN_00505e90((int)0x21c, "Data2\\Item\\Bow\\", "crossbow05.smd");
        FUN_00505e90((int)0x21d, "Data2\\Item\\Bow\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x21e, "Data2\\Item\\Bow\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x217, "Data2\\Item\\Bow\\", "\xBD\xBA\xC5\xC0.smd"); // arrows
        FUN_00505e90((int)0x21f, "Data2\\Item\\Bow\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x220, "Data2\\Item\\",      "\xBD\xBA\xC5\xC0.smd");

        // Helper items (familiars: fairy, satan, etc.)
        FUN_00505e90((int)0x330, "Data2\\Item\\Helper\\", "fairy.smd");
        FUN_00505e90((int)0x331, "Data2\\Item\\Helper\\", "satan.smd");
        FUN_00505e90((int)0x332, "Data2\\Item\\Helper\\", "\xBD\xBA\xC5\xC0.smd");

        // Rings (0x338-0x339)
        for (int i = 0x338; i-0x337 < 2; i++)
            FUN_00506050((int)i, "Data2\\Item\\Helper\\Ring", i - 0x337);

        // Necklaces (0x33c-0x33d)
        for (int i = 0x33c; i-0x33b < 2; i++)
            FUN_00506050((int)i, "Data2\\Item\\Helper\\Necklace", i - 0x33b);

        // Potions / consumables (0x350-0x360+)
        FUN_00505e90((int)0x350, "Data2\\Item\\Etc\\", "Apple.smd");
        for (int i = 0x351; (int)(i-0xd4) < 3; i++)
            FUN_00506050((int)i, "Data2\\Item\\Etc\\Red_", (int)(i-0xd4));
        for (int i = 0x354; i-0x353 < 3; i++)
            FUN_00506050((int)i, "Data2\\Item\\Etc\\Blue_", i-0x353);
        FUN_00505e90((int)0x358, "Data2\\Item\\Etc\\", "Green_01.smd");
        FUN_00505e90((int)0x359, "Data2\\Item\\Etc\\", "Bottle.smd");
        FUN_00505e90((int)0x35a, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x35b, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x35c, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x3b7, "Data2\\Item\\Etc\\", "star.smd");
        FUN_00505e90((int)0x3bb, "Data2\\Item\\Etc\\", "medal.smd");
        FUN_00505e90((int)0x3bc, "Data2\\Item\\Etc\\", "goldmedal.smd");
        FUN_00505e90((int)0x3bd, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x3b8, "Data2\\Item\\Etc\\", "poket.smd");
        FUN_00505e90((int)0x3b9, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x3ba, "Data2\\Item\\Helper\\", "olive.smd");
        FUN_00505e90((int)0x3b3, "Data2\\Item\\Etc\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x3b4, "Data2\\Item\\",      "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x35d, "Data2\\Item\\Etc\\", "blue.smd");
        FUN_00505e90((int)0x35e, "Data2\\Item\\Etc\\", "red.smd");
        FUN_00505e90((int)0x35f, "Data2\\Item\\Etc\\", "Gold.smd");
        FUN_00505e90((int)0x360, "Data2\\Item\\",      "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x361, "Data2\\Item\\Etc\\", "devileye.smd");
        FUN_00505e90((int)0x362, "Data2\\Item\\Etc\\", "devilkey.smd");
        FUN_00505e90((int)0x363, "Data2\\Item\\Etc\\", "devilinvitation.smd");
        FUN_00505e90((int)0x364, "Data2\\Item\\Etc\\", "loveb.smd");

        // Wings (0x310-0x313)
        FUN_00505e90((int)0x310, "Data2\\Item\\Wing\\", "elfin_wing.smd");
        FUN_00505e90((int)0x311, "Data2\\Item\\Wing\\", "angel.smd");
        FUN_00505e90((int)0x312, "Data2\\Item\\Wing\\", "devil.smd");
        FUN_00505e90((int)0x313, "Data2\\Item\\Wing\\", "elfin_wing2.smd");

        // Gem / pearls (0x317-0x323, excluding 0x31f)
        for (int i = 0x317; i-0x316 < 0xd; i++) {
            if (i != 0x31f)
                FUN_00506050((int)i, "Data2\\Item\\Etc\\pearl", 1);
        }
        FUN_00505e90((int)0x31f, "Data2\\Item\\", "\xBD\xBA\xC5\xC0.smd");

    }

    // ── BMD compressed asset loads (always run) ───────────────────────────────
    // BUGFIX 2026-04-27: TODOS los loops de items tenían off-by-one Ghidra
    // (`i-(start-1)<N` en vez de `i-start<N`), así que el último índice de cada
    // tipo de arma quedaba sin cargar. SM (Staff 568) y DK (Spear 505) crasheaban
    // silenciosamente al renderizar arma → invisibles. Mismo patrón que los
    // wing/armor loaders que ya fixeamos. Ver IDA 0x005079D0_OpenItems.
    //
    // Swords (0x190-0x1a0) — 17 iter (400-416)
    for (int i = 400; i-400 < 17; i++)
        FUN_005060b0(i, "Data\\Item\\", "Sword", i - 399);

    if (DAT_0055a7c4 == '\0') {
        FUN_00505e90((int)0x1a1, "Data2\\Item\\Sword\\", "DarkBreaker.smd");
        FUN_00505e90((int)0x1a2, "Data2\\Item\\Sword\\", "ThunderBlade.smd");
    }
    FUN_005060b0(0x1a1, "Data\\Item\\", "Sword", 0x12);
    FUN_005060b0(0x1a2, "Data\\Item\\", "Sword", 0x13);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x1a3, "Data2\\Item\\Sword\\", "UniqueSword.smd");
    FUN_005060b0(0x1a3, "Data\\Item\\", "Sword", 0x14);
    DAT_0055a7c4 = '\x01';

    FUN_005060b0(0x1af, "Data\\Item\\", "Sword", 0x20);

    // Axes BMD (0x1b0-0x1b8) — 9 iter (432-440)
    for (int i = 0x1b0; i-0x1b0 < 9; i++)
        FUN_005060b0(i, "Data\\Item\\", "Axe", i - 0x1af);

    // Maces BMD (0x1d0-0x1d6) — 7 iter (464-470)
    for (int i = 0x1d0; i-0x1d0 < 7; i++)
        FUN_005060b0(i, "Data\\Item\\", "Mace", i - 0x1cf);
    FUN_005060b0(0x1d7, "Data\\Item\\", "Mace", 8);

    // Spears BMD (0x1f0-0x1f9) — 10 iter (496-505)  ← DK weapon=505 vivía aquí
    for (int i = 0x1f0; i-0x1f0 < 10; i++)
        FUN_005060b0(i, "Data\\Item\\", "Spear", i - 0x1ef);
    FUN_005060b0(0x1fa, "Data\\Item\\", "Spear", 0xb);

    // Shields BMD (0x250-0x260) — 15 iter (592-606)
    for (int i = 0x250; i-0x250 < 15; i++)
        FUN_005060b0(i, "Data\\Item\\", "Shield", i - 0x24f);
    FUN_005060b0(0x25f, "Data\\Item\\", "Shield", 0x10);
    FUN_005060b0(0x260, "Data\\Item\\", "Shield", 0x11);

    // Staffs BMD (0x230-0x238) — 9 iter (560-568)  ← SM weapon=568 vivía aquí
    for (int i = 0x230; i-0x230 < 9; i++)
        FUN_005060b0(i, "Data\\Item\\", "Staff", i - 0x22f);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x239, "Data2\\Item\\Staff\\", "SpiritStaff.smd");
    FUN_005060b0(0x239, "Data\\Item\\", "Staff", 10);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x23a, "Data2\\Item\\Staff\\", "UniqueStaff.smd");
    FUN_005060b0(0x23a, "Data\\Item\\", "Staff", 0xb);
    DAT_0055a7c4 = '\x01';

    // Bows BMD (0x210-0x216) — 7 iter (528-534)
    for (int i = 0x210; i-0x210 < 7; i++)
        FUN_005060b0(i, "Data\\Item\\", "Bow", i - 0x20f);

    // Crossbows BMD (0x218-0x21e) — 7 iter (536-542)
    for (int i = 0x218; i-0x218 < 7; i++)
        FUN_005060b0(i, "Data\\Item\\", "CrossBow", i - 0x217);

    FUN_005060b0(0x217, "Data\\Item\\", "Arrows",   1);
    FUN_005060b0(0x21f, "Data\\Item\\", "Arrows",   2);
    FUN_005060b0(0x220, "Data\\Item\\", "CrossBow", 0x11);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x221, "Data2\\Item\\Bow\\", "\xBD\xBA\xC5\xC0.smd");
    FUN_005060b0(0x221, "Data\\Item\\", "Bow", 0x12);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x222, "Data2\\Item\\Bow\\", "UniqueBow.smd");
    FUN_005060b0(0x222, "Data\\Item\\", "Bow", 0x13);
    DAT_0055a7c4 = '\x01';

    // Helpers BMD (familiars 0x330-0x332) — 3 iter (816-818)
    for (int i = 0x330; i-0x330 < 3; i++)
        FUN_005060b0(i, "Data\\Player\\", "Helper", i - 0x32f);

    // Rings BMD — 2 iter (824-825)
    for (int i = 0x338; i-0x338 < 2; i++)
        FUN_005060b0(i, "Data\\Item\\", "Ring", i - 0x337);
    FUN_005060b0(0x33a, "Data\\Item\\", "Ring", 1);

    // Necklaces BMD — 2 iter (828-829)
    for (int i = 0x33c; i-0x33c < 2; i++)
        FUN_005060b0(i, "Data\\Item\\", "Necklace", i - 0x33b);

    if (DAT_0055a7c4 == '\0')
        FUN_00505e90((int)0x333, "Data2\\Item\\Helper\\", "\xBD\xBA\xC5\xC0.smd");
    FUN_005060b0(0x333, "Data\\Player\\", "Helper", 4);

    // Potions / consumables BMD (0x350-0x356)
    for (int i = 0x350; i-0x34f <= 7; i++)
        FUN_005060b0(i, "Data\\Item\\", "Potion", i - 0x34f);

    FUN_005060b0(0x358, "Data\\Item\\", "Antidote", 1);
    FUN_005060b0(0x359, "Data\\Item\\", "Beer",     1);
    FUN_005060b0(0x35a, "Data\\Item\\", "Scroll",   1);
    FUN_005060b0(0x35b, "Data\\Item\\", "MagicBox", 1);
    FUN_005060b0(0x35c, "Data\\Item\\", "Event",    1);

    // Jewels (0x35d-0x35e)
    for (int i = 0x35d; i-0x35c < 2; i++)
        FUN_005060b0(i, "Data\\Item\\", "Jewel", i - 0x35c);

    AccessModel(MODEL_POTION + 13, (char*)"Data\\Item\\", (char*)"Jewel", 1);
    AccessModel(MODEL_POTION + 14, (char*)"Data\\Item\\", (char*)"Jewel", 2);
    OpenTexture(MODEL_POTION + 13, (void*)"Item\\", 0x2600, true);
    OpenTexture(MODEL_POTION + 14, (void*)"Item\\", 0x2600, true);

    FUN_005060b0(0x35f, "Data\\Item\\", "Gold",  1);
    FUN_005060b0(0x360, "Data\\Item\\", "Jewel", 3);

    // Devil items (0x361-0x363)
    for (int i = 0; i < 3; i++)
        FUN_005060b0(i + 0x361, "Data\\Item\\", "Devil", i);

    FUN_005060b0(0x364, "Data\\Item\\", "Drink",   0);
    FUN_005060b0(0x365, "Data\\Item\\", "ConChip", 0);
    FUN_005060b0(0x3b7, "Data\\Item\\", "MagicBox", 2);
    FUN_005060b0(0x3b9, "Data\\Item\\", "MagicBox", 5);
    FUN_005060b0(0x3ba, "Data\\Item\\", "Beer",     2);
    FUN_005060b0(0x3bb, "Data\\Item\\", "MagicBox", 6);
    FUN_005060b0(0x3bc, "Data\\Item\\", "MagicBox", 7);
    FUN_005060b0(0x3bd, "Data\\Item\\", "MagicBox", 8);
    FUN_005060b0(0x3b8, "Data\\Item\\", "MagicBox", 4);
    FUN_005060b0(0x3b3, "Data\\Item\\", "Event",    2);
    FUN_005060b0(0x3b4, "Data\\Item\\", "Event",    3);

    // Blood Castle event items (Korean SMD)
    if (DAT_0055a7c4 == '\0') {
        FUN_00505e90((int)0x340, "Data2\\Item\\Etc\\", "AngelScroll.smd");
        FUN_00505e90((int)0x341, "Data2\\Item\\Etc\\", "BloodBone.smd");
        FUN_00505e90((int)0x342, "Data2\\Item\\Etc\\", "InvisibleCloak.smd");
        FUN_00505e90((int)0x3be, "Data2\\Item\\Etc\\", "BloodStone.smd");
    }
    // BloodCastle items BMD (0x340-0x342 + 0x3be mapped to 0-3)
    for (int i = 0; i < 4; i++) {
        int slot = (i < 3) ? (i + 0x340) : (i + 0x3bb);
        FUN_005060b0(slot, "Data\\Item\\", "EventBloodCastle", i);
    }

    DAT_0055a7c4 = '\x01';

    // Quest items (0x367-0x36a)
    for (int i = 0; i < 4; i++)
        FUN_005060b0(i + 0x367, "Data\\Item\\", "Quest", i);

    // Extra ETC items (Korean SMD)
    if (DAT_0055a7c4 == '\0') {
        FUN_00505e90((int)0x366, "Data2\\Item\\ETC\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x33e, "Data2\\Item\\ETC\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x33f, "Data2\\Item\\ETC\\", "\xBD\xBA\xC5\xC0.smd");
    }
    FUN_005060b0(0x366, "Data\\Item\\", "jewel", 0x16);
    for (int i = 0x33e; i-0x33d < 2; i++)
        FUN_005060b0(i, "Data\\Item\\", "Quest", i - 0x33a);

    // Wings BMD (0x310-0x312)
    // BUGFIX 2026-04-26: era `i-0x30f < 3` → 2 iter (perdía Wing03 idx 0x312).
    // IDA 0x005079D0 línea 216: `while (v31 - 784 < 3)` con v31=784 → 3 iter.
    for (int i = 0x310; i-0x310 < 3; i++)
        FUN_005060b0(i, "Data\\Item\\", "Wing", i - 0x30f);

    // Wings SMD (Korean)
    if (DAT_0055a7c4 == '\0') {
        FUN_00505e90((int)0x313, "Data2\\Item\\Wing\\", "elfin_wing2.smd");
        FUN_00505e90((int)0x314, "Data2\\Item\\Wing\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x315, "Data2\\Item\\Wing\\", "\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x316, "Data2\\Item\\Wing\\", "\xBD\xBA\xC5\xC0.smd");
    }

    // Wings BMD extended (0x313-0x316) — Wing04..Wing07
    // BUGFIX 2026-04-26: era `i-0x312 < 4` → 3 iter (perdía Wing07 idx 0x316,
    // ala que usa MG/0x316). IDA línea 224: `while (v32 - 787 < 4)` con v32=787 → 4 iter.
    for (int i = 0x313; i-0x313 < 4; i++)
        FUN_005060b0(i, "Data\\Item\\", "Wing", i - 0x30f);

    DAT_0055a7c4 = 1;

    // Gem slots (0x317-0x322, skip 0x31f=9)
    for (int i = 1; i <= 0xd; i++) {
        if (i != 9)
            FUN_005060b0(i + 0x316, "Data\\Item\\", "Gem", i);
    }

    DAT_0055a7c4 = 1;

    FUN_005060b0(799, "Data\\Item\\", "Jewel", 0xf);

    // Magic books BMD (Book01..Book16 / models 0x370..0x37f).
    // IDA OpenItems loads the inclusive range; stopping before 0x37f leaves
    // Book16 unloaded, which is Soul Barrier (type 495/model 895).
    for (int i = 0x370; i <= 0x37f; i++)
        FUN_005060b0(i, "Data\\Item\\", "Book", i - 0x36f);

    // NoneBlendMesh flags — RE-HABILITADO 2026-07-16 (fix del filo glowing).
    // Marca el mesh del FILO (emisivo) de estas armas como NoneBlendMesh=1 (mesh+0),
    // que BMD_DrawMesh saltea en el path chrome/oil/blur (línea "if (*pcVar1) return").
    // Efecto: el glow +N chrome NO se pinta sobre el filo (que conserva su luz propia);
    // va al hilt/mango. Sin esto, el chrome cubría el filo → "barra dorada".
    //
    // Fiel a IDA OpenItems (0x005079D0 L245-249). Los offsets son BYTES dentro del
    // array de modelos (Models = DAT_05828d58, stride 0xBC, campo Meshs en +0x28), NO
    // direcciones absolutas ni índices ×4 — una sesión previa les agregó un ×4 espurio
    // y comentó todo. `Models + 0x16c68` = Model[496].Meshs; `+0x28` = mesh[1].
    //   0x16c68 = 496*0xBC+0x28 → Light Spear (MODEL_SPEAR)    mesh[1]
    //   0x12d40 = 410*0xBC+0x28 → Light Saber (MODEL_SWORD+10) mesh[1]
    //   0x19fd0 = 566*0xBC+0x28 → Staff+6                      mesh[2] (+0x50)
    //   0x15950 = 470*0xBC+0x28 → Mace+6                       mesh[1]
    //   0x2be38 = 956*0xBC+0x28 → Event+9                      mesh[1]
    {
        unsigned int m;
        m = *(unsigned int*)((unsigned char*)(uintptr_t)DAT_05828d58 + 0x16c68); if (m) *(unsigned char*)(uintptr_t)(m + 0x28) = 1;
        m = *(unsigned int*)((unsigned char*)(uintptr_t)DAT_05828d58 + 0x12d40); if (m) *(unsigned char*)(uintptr_t)(m + 0x28) = 1;
        m = *(unsigned int*)((unsigned char*)(uintptr_t)DAT_05828d58 + 0x19fd0); if (m) *(unsigned char*)(uintptr_t)(m + 0x50) = 1;
        m = *(unsigned int*)((unsigned char*)(uintptr_t)DAT_05828d58 + 0x15950); if (m) *(unsigned char*)(uintptr_t)(m + 0x28) = 1;
        m = *(unsigned int*)((unsigned char*)(uintptr_t)DAT_05828d58 + 0x2be38); if (m) *(unsigned char*)(uintptr_t)(m + 0x28) = 1;
    }
}
