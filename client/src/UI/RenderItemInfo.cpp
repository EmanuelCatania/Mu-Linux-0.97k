#include "stdafx.h"

extern "C" BYTE ShopItems[];   // pool dedicado de la tienda
// RenderItemInfo.cpp  @0x004C4650 / 0x004C8D70
// Populates the item info tooltip string buffer (lpString_07e90798, stride 100, ~0x18 slots)
// and the color-flag array (DAT_07e91708). Called from inventory/shop tooltip draw path.

// 2026-05-08: SEH wrapper. Múltiples crashes recurrentes en este path
// (addr=0x74F3DBCC en ucrtbase.dll desde snprintf con `%s` reading bogus
// itemName pointer). Aunque agregamos guards extensivos, hay paths
// internos que pueden seguir tropezando con punteros corruptos. SEH
// silencia cualquier AV interno en lugar de matar el proceso — la
// tooltip simplemente no aparece esa frame.
extern "C" void __cdecl FUN_004c4650_impl(void*, void*, void*, int);
extern "C" void __cdecl FUN_004c8d70_impl(void*, int, void*);
extern "C" void DbgLogPublic(const char* msg);
char* __cdecl GetMapName(int iMap);

static bool IsRenderRepairInfoBlocked(short type)
{
    if ((type >= 416 + 103 && type <= 416 + 107) ||
        (type >= 416 + 109 && type <= 416 + 115) ||
        type == 416 + 121 ||
        type == 448 + 112 || type == 448 + 113 ||
        type == 448 + 120 || type == 448 + 121 || type == 448 + 122 ||
        type == 448 + 123 || type == 448 + 124 ||
        type == 448 + 133 ||
        (type >= 448 + 114 && type <= 448 + 119) ||
        (type >= 448 + 126 && type <= 448 + 132) ||
        (type >= 448 + 134 && type <= 448 + 139) ||
        (type >= 384 + 130 && type <= 384 + 135)) {
        return true;
    }
    return false;
}

static BYTE GetHeroTooltipClassRaw()
{
    if (!Hero || (uintptr_t)Hero < 0x100000)
        return 0;
    return *(BYTE*)(Hero + 0x1BC);
}

static BYTE GetHeroTooltipBaseClass()
{
    return GetHeroTooltipClassRaw() & 7;
}

static BYTE GetHeroTooltipStepClass()
{
    const BYTE raw = GetHeroTooltipClassRaw();
    if ((raw >> 4) & 1)
        return 3;
    if ((raw >> 3) & 1)
        return 2;
    return 1;
}

struct INVENTORY_ITEM_ADD_OPTION
{
    BYTE  m_byOption1;
    WORD  m_byValue1;
    BYTE  m_byOption2;
    WORD  m_byValue2;
    BYTE  m_Type;
    DWORD m_Time;
};

static bool GetInventoryItemAddOption(short type, INVENTORY_ITEM_ADD_OPTION& out)
{
    static bool s_attempted = false;
    static bool s_loaded = false;
    static INVENTORY_ITEM_ADD_OPTION s_table[1024] = {};

    if (!s_attempted) {
        s_attempted = true;

        char exePath[MAX_PATH] = {};
        char localPath[MAX_PATH] = {};
        char relPath1[MAX_PATH] = "Data\\Local\\ItemAddOption.bmd";
        char relPath2[MAX_PATH] = "bin\\Client\\Data\\Local\\ItemAddOption.bmd";

        if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) > 0) {
            char* slash = strrchr(exePath, '\\');
            if (slash) {
                *slash = '\0';
                snprintf(localPath, MAX_PATH, "%s\\Data\\Local\\ItemAddOption.bmd", exePath);
            }
        }

        const char* tryPaths[3] = {
            localPath[0] ? localPath : nullptr,
            relPath1,
            relPath2,
        };

        for (const char* path : tryPaths) {
            if (!path || !path[0]) {
                continue;
            }

            FILE* fp = fopen(path, "rb");
            if (!fp) {
                continue;
            }

            const size_t want = sizeof(s_table);
            const size_t got = fread(s_table, 1, want, fp);
            fclose(fp);

            if (got == want) {
                BuxConvert_0((int)s_table, (int)want);
                s_loaded = true;
                break;
            }

            memset(s_table, 0, sizeof(s_table));
        }
    }

    if (!s_loaded || type < 0 || type >= (short)(sizeof(s_table) / sizeof(s_table[0]))) {
        return false;
    }

    out = s_table[type];
    return true;
}

static void RenderItemTooltipTextList(int sx, int sy, int itemHeightCells, int count)
{
    if (count <= 0)
        return;

    if (count > 20)
        count = 20;

    HDC hdc = (HDC)(uintptr_t)DAT_055c9fec;
    TEXTMETRICA tm = {};
    GetTextMetricsA(hdc, &tm);
    const int lineH = tm.tmHeight + tm.tmExternalLeading;
    const int padX = 4;
    const int padY = 3;
    const int gapY = 1;

    int maxWidth = 0;
    int visibleLines = 0;
    for (int i = 0; i < count; ++i) {
        const char* pText = lpString_07e90798 + i * 100;
        if (!pText[0] || (pText[0] == '\n' && pText[1] == '\0')) {
            ++visibleLines;
            continue;
        }
        SIZE sz = {};
        GetTextExtentPointA(hdc, pText, (int)strlen(pText), &sz);
        if (sz.cx > maxWidth)
            maxWidth = sz.cx;
        ++visibleLines;
    }

    if (visibleLines <= 0)
        return;

    const int boxW = maxWidth + padX * 2;
    const int boxH = visibleLines * lineH + (visibleLines - 1) * gapY + padY * 2;

    int drawX = sx - boxW / 2;
    int drawY = sy - boxH;
    if (drawY < 0)
        drawY = sy + itemHeightCells * 20;
    if (drawX < 0)
        drawX = 0;
    if (drawX + boxW > (int)WindowWidth)
        drawX = (int)WindowWidth - boxW;
    if (drawX < 0)
        drawX = 0;

    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY - 1), (float)(boxW + 2), 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY + boxH), (float)(boxW + 2), 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    FUN_005124c0((float)(drawX + boxW), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    glColor4f(0.0f, 0.0f, 0.0f, 0.82f);
    FUN_005124c0((float)drawX, (float)drawY, (float)boxW, (float)boxH);

    static const DWORD colorTable[7] = {
        0xFFFFFFFF, 0xFFFFFF00, 0xFF00FF00,
        0xFFFF0000, 0xFF00FFFF, 0xFFFFC040, 0xFFFF80FF
    };

    int lineY = drawY + padY;
    for (int i = 0; i < count; ++i) {
        char* pText = lpString_07e90798 + i * 100;
        int colorType = DAT_07e91708[i];
        int fontIdx = (&DAT_07ea7b10)[i];

        SelectObject(hdc, fontIdx ? g_hFontBold : g_hFont);

        if (pText[0] && !(pText[0] == '\n' && pText[1] == '\0')) {
            int colorIdx = (colorType >= 0 && colorType < 7) ? colorType : 0;
            m_dwBackColor = 0;
            DAT_00559c78 = colorTable[colorIdx];
            FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, drawX + padX, lineY, pText, 0);
        }

        lineY += lineH + gapY;
    }
}

static bool BuildInventorySpecialNameLine(ITEM* ip, ITEM_ATTRIBUTE* p, unsigned int level, char* dst, size_t dstSize)
{
    if (!ip || !p || !dst || dstSize == 0)
        return false;

    constexpr short ITEM_SWORD_BASE  = 0;
    constexpr short ITEM_MACE_BASE   = 64;
    constexpr short ITEM_BOW_BASE    = 128;
    constexpr short ITEM_STAFF_BASE  = 160;
    constexpr short ITEM_WING_BASE   = 384;
    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_POTION_BASE = 448;
    static const int kSommonTable[6] = { 2, 7, 14, 8, 9, 41 };
    static const char* kChaosEventName[10] = {
        "È÷µÅ© °íÇâ ¿©Çà±Ç",
        "ÆæÆ¼¾ö4 ÄÄÇ»ÅÍ",
        "µðÁöÅ»Ä«¸Þ¶ó",
        "·ÎÁöÅØ ¹«¼± ¸¶¿ì½º+Å°º¸µå ¼¼Æ®",
        "256M ·¥",
        "6°³¿ ÀâÁö ±¸µ¶±Ç",
        "¹®È­»óÇ°±Ç(¸¸¿ø)",
        "¹Â ¸Ó±×ÄÅ",
        "¹Â T¼ÅÃ÷",
        "¹Â 10½Ã°£ ¹«·áÀÌ¿ë±Ç"
    };

    const short type = ip->Type;

    if (type >= ITEM_POTION_BASE + 23 && type <= ITEM_POTION_BASE + 26) {
        if (type == ITEM_POTION_BASE + 23 && level == 1) {
            snprintf(dst, dstSize, "%s", GlobalText[906]);
            return true;
        }
        if (type == ITEM_POTION_BASE + 24 && level == 1) {
            snprintf(dst, dstSize, "%s", GlobalText[907]);
            return true;
        }
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 12) {
        if (level == 0) snprintf(dst, dstSize, "%s", GlobalText[100]);
        else if (level == 1) snprintf(dst, dstSize, "%s", GlobalText[101]);
        else if (level == 2 && ip->Durability < 10) snprintf(dst, dstSize, "%s", kChaosEventName[ip->Durability]);
        else snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 11) {
        switch (level) {
        case 0:  snprintf(dst, dstSize, "%s", p->Name); break;
        case 1:  snprintf(dst, dstSize, "%s", GlobalText[105]); break;
        case 2:  snprintf(dst, dstSize, "%s", GlobalText[106]); break;
        case 3:  snprintf(dst, dstSize, "%s", GlobalText[107]); break;
        case 5:  snprintf(dst, dstSize, "%s", GlobalText[109]); break;
        case 6:  snprintf(dst, dstSize, "%s", GlobalText[110]); break;
        case 7:  snprintf(dst, dstSize, "%s", GlobalText[111]); break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            snprintf(dst, dstSize, "%s +%u", GlobalText[115], level - 7);
            break;
        case 13: snprintf(dst, dstSize, "%s", GlobalText[117]); break;
        // levels 14/15 REMOVIDOS 2026-07-20: variantes de Box of Luck de
        // versiones posteriores; usaban GlobalText[1650]/[1651], fuera de las
        // 1000 filas.  Los niveles 0..12 (hasta "Box of Kundun +5") son validos
        // y quedan intactos.
        default: snprintf(dst, dstSize, "%s", p->Name); break;
        }
        return true;
    }

    if (type == ITEM_HELPER_BASE + 15) {
        switch (level) {
        case 0: snprintf(dst, dstSize, "%s %s", GlobalText[168], p->Name); break;
        case 1: snprintf(dst, dstSize, "%s %s", GlobalText[169], p->Name); break;
        case 2: snprintf(dst, dstSize, "%s %s", GlobalText[167], p->Name); break;
        case 3: snprintf(dst, dstSize, "%s %s", GlobalText[166], p->Name); break;
        // case 4 REMOVIDO: GlobalText[1900] fuera de las 1000 filas.
        default: snprintf(dst, dstSize, "%s", p->Name); break;
        }
        return true;
    }

    // Loch's Feather (430): el `level == 1` usaba GlobalText[1235], fuera de
    // rango — el item quedaba con el NOMBRE EN BLANCO en ese nivel.  Ahora cae
    // siempre a p->Name.
    if (type == ITEM_HELPER_BASE + 14) {
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 21) {
        switch (level) {
        case 0: snprintf(dst, dstSize, "%s", p->Name); break;
        case 1: snprintf(dst, dstSize, "%s", GlobalText[810]); break;
        // cases 2 y 3 REMOVIDOS: GlobalText[1098]/[1290] fuera de rango; caian
        // al default (p->Name) igual, pero con el nombre vacio.
        default: snprintf(dst, dstSize, "%s", p->Name); break;
        }
        return true;
    }

    if (type == ITEM_HELPER_BASE + 19) {
        switch (level) {
        case 0: snprintf(dst, dstSize, "%s", GlobalText[811]); break;
        case 1: snprintf(dst, dstSize, "%s", GlobalText[812]); break;
        case 2: snprintf(dst, dstSize, "%s", GlobalText[817]); break;
        default: snprintf(dst, dstSize, "%s", GlobalText[809]); break;
        }
        return true;
    }

    // REMOVIDO 2026-07-21 — HELPER+20 (type 436) no existe en el item.bmd del
    // 0.97k (slot vacio): rama muerta, nunca disparaba.  Graft de version nueva.
    // REMOVIDO 2026-07-21 — HELPER+107 (type 523 >=512): tipo imposible.
    if (type == ITEM_POTION_BASE + 9) {
        if (level == 1) snprintf(dst, dstSize, "%s", GlobalText[108]);
        else snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_WING_BASE + 11) {
        const char* skillName = (const char*)((char*)&SkillAttribute + (30 + (int)level) * 300 + 4);
        if ((uintptr_t)skillName > 0x100000 && (uintptr_t)skillName < 0x80000000 && skillName[0]) {
            snprintf(dst, dstSize, "%s %s", skillName, GlobalText[102]);
        } else {
            snprintf(dst, dstSize, "%s", p->Name);
        }
        return true;
    }

    if (type == ITEM_HELPER_BASE + 10) {
        MONSTER_SCRIPT* monsters = (MONSTER_SCRIPT*)&MonsterScript;
        bool found = false;
        if (level < 6) {
            for (int i = 0; i < MAX_MONSTER; ++i) {
                if (monsters[i].Type == kSommonTable[level]) {
                    snprintf(dst, dstSize, "%s %s", monsters[i].Name, GlobalText[103]);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            snprintf(dst, dstSize, "%s %s", p->Name, GlobalText[103]);
        }
        return true;
    }

    // REMOVIDO 2026-07-21 — HELPER+4/+5 (types 420/421) son slots vacios.
    // REMOVIDO 2026-07-21 — HELPER+30 (type 446) es slot vacio.
    // REMOVIDO 2026-07-21 — POTION+28 (type 476) es slot vacio.
    if (type == ITEM_WING_BASE + 32 || type == ITEM_WING_BASE + 33 || type == ITEM_WING_BASE + 34 || type == ITEM_WING_BASE + 35 ||
        (type >= ITEM_POTION_BASE + 45 && type <= ITEM_POTION_BASE + 50)) {
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 32) {
        // level 1 usaba GlobalText[2012], fuera de rango → nombre EN BLANCO.
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 33) {
        // level 1 usaba GlobalText[2013], fuera de rango → nombre EN BLANCO.
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_POTION_BASE + 34) {
        // level 1 usaba GlobalText[2014], fuera de rango → nombre EN BLANCO.
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type >= ITEM_HELPER_BASE + 32 && type <= ITEM_HELPER_BASE + 37) {
        // El +37 (453, Medium Mana Potion) tenia un sufijo "excellent" graft
        // ([1863]/[1864]/[1866], fuera de rango) — sin sentido en una pocion.
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    if (type == ITEM_SWORD_BASE + 19 || type == ITEM_BOW_BASE + 18 || type == ITEM_STAFF_BASE + 10 || type == ITEM_MACE_BASE + 13) {
        if (level == 0) snprintf(dst, dstSize, "%s", p->Name);
        else snprintf(dst, dstSize, "%s +%u", p->Name, level);
        return true;
    }

    if ((type >= ITEM_WING_BASE + 3 && type <= ITEM_WING_BASE + 6) ||
        type == ITEM_HELPER_BASE + 30 ||
        (type >= ITEM_WING_BASE + 36 && type <= ITEM_WING_BASE + 40) ||
        (type >= ITEM_WING_BASE + 42 && type <= ITEM_WING_BASE + 43) ||
        (type >= ITEM_WING_BASE + 49 && type <= ITEM_WING_BASE + 50)) {
        if (level == 0) snprintf(dst, dstSize, "%s", p->Name);
        else snprintf(dst, dstSize, "%s +%u", p->Name, level);
        return true;
    }

    if (type == ITEM_POTION_BASE + 41 || type == ITEM_POTION_BASE + 42 ||
        type == ITEM_POTION_BASE + 43 || type == ITEM_POTION_BASE + 44 ||
        type == ITEM_HELPER_BASE + 38 ||
        (type >= ITEM_WING_BASE + 60 && type <= ITEM_WING_BASE + 65) ||
        (type >= ITEM_WING_BASE + 70 && type <= ITEM_WING_BASE + 74) ||
        (type >= ITEM_WING_BASE + 100 && type <= ITEM_WING_BASE + 129) ||
        type == ITEM_POTION_BASE + 111 ||
        (type >= ITEM_POTION_BASE + 101 && type <= ITEM_POTION_BASE + 109)) {
        snprintf(dst, dstSize, "%s", p->Name);
        return true;
    }

    return false;
}

static int GetInventorySpecialNameColor(ITEM* ip)
{
    if (!ip) {
        return 0;
    }

    constexpr short ITEM_SWORD_BASE = 0;
    constexpr short ITEM_MACE_BASE = 64;
    constexpr short ITEM_BOW_BASE = 128;
    constexpr short ITEM_STAFF_BASE = 160;
    constexpr short ITEM_WING_BASE = 384;
    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_POTION_BASE = 448;
    constexpr int TEXT_COLOR_WHITE = 0;
    constexpr int TEXT_COLOR_BLUE = 1;
    constexpr int TEXT_COLOR_YELLOW = 3;
    constexpr int TEXT_COLOR_GREEN = 4;
    constexpr int TEXT_COLOR_DARKRED = 5;
    constexpr int TEXT_COLOR_PURPLE = 6;
    constexpr int TEXT_COLOR_VIOLET = 6;
    constexpr int TEXT_COLOR_GREEN_BLUE = 9;
    const short type = ip->Type;
    const int level = (ip->Level >> 3) & 0xF;
    const bool hasSpecial = (ip->SpecialNum != 0);
    const bool hasExcellent = ((ip->Option1 & 0x3F) != 0);
    const bool hasSet = (ip->Color == 4);
    int color = TEXT_COLOR_WHITE;

    if (type == ITEM_POTION_BASE + 13 || type == ITEM_POTION_BASE + 14 ||
        type == ITEM_WING_BASE + 15 || type == ITEM_POTION_BASE + 31 ||
        (type >= ITEM_POTION_BASE + 65 && type <= ITEM_POTION_BASE + 68) ||
        type == ITEM_HELPER_BASE + 52 || type == ITEM_HELPER_BASE + 53 ||
        type == ITEM_POTION_BASE + 100 ||
        (type >= ITEM_POTION_BASE + 141 && type <= ITEM_POTION_BASE + 144) ||
        (type >= ITEM_HELPER_BASE + 135 && type <= ITEM_HELPER_BASE + 145) ||
        (type >= ITEM_POTION_BASE + 160 && type <= ITEM_POTION_BASE + 161) ||
        type == ITEM_POTION_BASE + 16 || type == ITEM_POTION_BASE + 22 ||
        type == ITEM_POTION_BASE + 112 || type == ITEM_POTION_BASE + 113 ||
        (type >= ITEM_POTION_BASE + 114 && type <= ITEM_POTION_BASE + 124) ||
        (type >= ITEM_POTION_BASE + 126 && type <= ITEM_POTION_BASE + 140) ||
        type == ITEM_HELPER_BASE + 64 || type == ITEM_HELPER_BASE + 65 ||
        type == ITEM_HELPER_BASE + 66 || type == ITEM_HELPER_BASE + 67 ||
        type == ITEM_HELPER_BASE + 68 || type == ITEM_HELPER_BASE + 69 ||
        type == ITEM_HELPER_BASE + 70 || (type >= ITEM_HELPER_BASE + 71 && type <= ITEM_HELPER_BASE + 76) ||
        type == ITEM_HELPER_BASE + 80 || type == ITEM_HELPER_BASE + 81 || type == ITEM_HELPER_BASE + 82 ||
        type == ITEM_HELPER_BASE + 93 || type == ITEM_HELPER_BASE + 94 ||
        (type >= ITEM_HELPER_BASE + 97 && type <= ITEM_HELPER_BASE + 99) ||
        (type >= ITEM_HELPER_BASE + 103 && type <= ITEM_HELPER_BASE + 107) ||
        (type >= ITEM_HELPER_BASE + 109 && type <= ITEM_HELPER_BASE + 116) ||
        (type >= ITEM_HELPER_BASE + 121 && type <= ITEM_HELPER_BASE + 145) ||
        (type >= ITEM_WING_BASE + 130 && type <= ITEM_WING_BASE + 135)) {
        color = TEXT_COLOR_YELLOW;
    } else if (type == ITEM_STAFF_BASE + 10 || type == ITEM_SWORD_BASE + 19 ||
               type == ITEM_BOW_BASE + 18 || type == ITEM_MACE_BASE + 13) {
        color = TEXT_COLOR_PURPLE;
    } else if (type == ITEM_POTION_BASE + 17 || type == ITEM_POTION_BASE + 18 || type == ITEM_POTION_BASE + 19) {
        color = TEXT_COLOR_YELLOW;
    } else if (type == ITEM_HELPER_BASE + 16 || type == ITEM_HELPER_BASE + 17) {
        color = TEXT_COLOR_YELLOW;
    } else if (hasSet) {
        color = TEXT_COLOR_GREEN_BLUE;
    } else if (hasExcellent && hasSpecial) {
        color = TEXT_COLOR_GREEN;
    } else if (level >= 7) {
        color = TEXT_COLOR_YELLOW;
    } else if (hasSpecial) {
        color = TEXT_COLOR_BLUE;
    } else {
        color = TEXT_COLOR_WHITE;
    }

    if ((type >= ITEM_WING_BASE + 3 && type <= ITEM_WING_BASE + 6) ||
        type == ITEM_HELPER_BASE + 30 ||
        (type >= ITEM_WING_BASE + 36 && type <= ITEM_WING_BASE + 40) ||
        (type >= ITEM_WING_BASE + 42 && type <= ITEM_WING_BASE + 43) ||
        (type >= ITEM_WING_BASE + 49 && type <= ITEM_WING_BASE + 50)) {
        color = (level >= 7) ? TEXT_COLOR_YELLOW : (hasSpecial ? TEXT_COLOR_BLUE : TEXT_COLOR_WHITE);
    }

    if (type == ITEM_POTION_BASE + 21 ||
        (type >= ITEM_POTION_BASE + 23 && type <= ITEM_POTION_BASE + 26) ||
        type == ITEM_POTION_BASE + 28 ||
        (type >= ITEM_POTION_BASE + 41 && type <= ITEM_POTION_BASE + 44) ||
        (type >= ITEM_POTION_BASE + 45 && type <= ITEM_POTION_BASE + 54) ||
        (type >= ITEM_POTION_BASE + 58 && type <= ITEM_POTION_BASE + 62) ||
        (type >= ITEM_POTION_BASE + 70 && type <= ITEM_POTION_BASE + 71) ||
        (type >= ITEM_POTION_BASE + 78 && type <= ITEM_POTION_BASE + 98) ||
        (type >= ITEM_POTION_BASE + 101 && type <= ITEM_POTION_BASE + 109) ||
        type == ITEM_POTION_BASE + 111 ||
        (type >= ITEM_POTION_BASE + 145 && type <= ITEM_POTION_BASE + 156) ||
        type == ITEM_HELPER_BASE + 7 || type == ITEM_HELPER_BASE + 11 ||
        type == ITEM_HELPER_BASE + 14 || type == ITEM_HELPER_BASE + 15 ||
        type == ITEM_HELPER_BASE + 19 || type == ITEM_HELPER_BASE + 20 ||
        type == ITEM_HELPER_BASE + 29 ||
        (type >= ITEM_HELPER_BASE + 32 && type <= ITEM_HELPER_BASE + 63)) {
        color = TEXT_COLOR_YELLOW;
    }

    if ((type >= ITEM_WING_BASE + 60 && type <= ITEM_WING_BASE + 65) ||
        (type >= ITEM_WING_BASE + 70 && type <= ITEM_WING_BASE + 74) ||
        (type >= ITEM_WING_BASE + 100 && type <= ITEM_WING_BASE + 129)) {
        color = TEXT_COLOR_VIOLET;
    }

    if (type == ITEM_POTION_BASE + 41 || type == ITEM_POTION_BASE + 42 ||
        type == ITEM_POTION_BASE + 43 || type == ITEM_POTION_BASE + 44 ||
        type == ITEM_HELPER_BASE + 38) {
        color = TEXT_COLOR_YELLOW;
    }

    if (type == 19 || type == 77 || type == 146 || type == 170 ||
        type == ITEM_STAFF_BASE + 10 || type == ITEM_SWORD_BASE + 19 ||
        type == ITEM_BOW_BASE + 18 || type == ITEM_MACE_BASE + 13) {
        color = TEXT_COLOR_PURPLE;
    }

    return color;
}
// REMOVIDA 2026-07-20 — GetInventoryTooltipAddOptionData, junto con sus 5
// callers.  Pertenecia al sistema de items por PERIODO de versiones
// posteriores (mismo bloque que FormatInventoryTooltipTime, ya removida):
//   · su switch solo cubria tipos 520..535, imposibles en una tabla de 512;
//   · su fuente de datos, GetInventoryItemAddOption, lee
//     Data\Local\ItemAddOption.bmd, archivo que NO EXISTE en el 0.97k —
//     verificado en todo el proyecto.  Sin el, s_loaded queda en false y la
//     funcion devolvia false siempre.
// La struct INVENTORY_ITEM_ADD_OPTION lleva un campo m_Time (vencimiento),
// que es la firma de ese sistema.

// AUDITORIA 2026-07-20 — FormatInventoryTooltipTime REMOVIDA (10 sitios).
// Formateaba "tiempo restante" (dia/hora/minuto/segundo) para items con
// vencimiento, una feature MUY posterior al 0.97k.  Tres pruebas de que es
// injerto:
//   1. NO tenia un solo caller — ya era codigo muerto.
//   2. Usaba GlobalText[2298..2301] y [2308], fuera de las 1000 filas del
//      Text.bmd (o sea que el original no tiene esas etiquetas siquiera).
//   3. La tabla que la alimentaba (GetInventoryTooltipAddOptionData) solo
//      matchea tipos 520..535, y el item.bmd del 0.97k tiene 512 entradas:
//      esos tipos no existen ni pueden existir.

// Formatea un entero con separador de miles ("4700" -> "4,700"), que es lo que
// esperan los strings de precio de GlobalText ([62]/[63], ambos con %s).
static void FormatThousands(char* dst, size_t dstSize, int value)
{
    if (!dst || dstSize == 0) return;
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%d", value < 0 ? -value : value);
    int len = (int)strlen(tmp);
    size_t o = 0;
    if (value < 0 && o + 1 < dstSize) dst[o++] = '-';
    for (int i = 0; i < len && o + 1 < dstSize; ++i) {
        if (i > 0 && ((len - i) % 3) == 0) dst[o++] = ',';
        if (o + 1 < dstSize) dst[o++] = tmp[i];
    }
    dst[o] = 0;
}

// Copia `src` a `dst` colapsando "%%" -> "%".
// Equivale a `sprintf(dst, src)` con cero argumentos, que es lo que hace el
// binario con las lineas de GlobalText que no llevan parametros, pero sin el
// riesgo de que un %d perdido en los datos lea la pila.
static void CopyCollapsingPercent(char* dst, size_t dstSize, const char* src)
{
    if (!dst || dstSize == 0) return;
    if (!src) { dst[0] = 0; return; }
    size_t o = 0;
    for (size_t i = 0; src[i] && o + 1 < dstSize; ++i) {
        if (src[i] == '%' && src[i + 1] == '%') ++i;
        dst[o++] = src[i];
    }
    dst[o] = 0;
}

static void AppendInventorySpecialTooltipLines(ITEM* ip)
{
    if (!ip || DAT_07eaa154 >= 28)
        return;

    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_POTION_BASE = 448;
    constexpr short ITEM_WING_BASE = 384;
    const short type = ip->Type;
    constexpr int C_WHITE = 0;
    constexpr int C_BLUE = 1;
    constexpr int C_RED = 2;
    constexpr int C_YELLOW = 3;
    constexpr int C_DARKRED = 5;
    constexpr int C_PURPLE = 6;
    constexpr int C_DARKBLUE = 7;
    constexpr int C_DARKYELLOW = 8;
    const int level = (ip->Level >> 3) & 0xF;

    auto addLine = [](const char* text, int color, bool bold = false) {
        if (!text || !text[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, "%s", text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt = [](const char* fmt, int value, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt2 = [](const char* fmt, int value1, int value2, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value1, value2);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmtStr = [](const char* fmt, const char* text, int color, bool bold = false) {
        if (!fmt || !fmt[0] || !text || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };



    switch (type) {
    // AUDITORIA 2026-07-20 — case ITEM_HELPER_BASE + 38 (type 454, Large Mana
    // Potion) REMOVIDO.  Usaba GlobalText[926], que en el Text.bmd del 0.97k es
    // "Antilag" (un string del MENU DE OPCIONES: 924 "Reiniciar fuente",
    // 925 "Volver", 926 "Antilag", 927 "Eliminar Sombras"), y GlobalText[2207],
    // que esta fuera de las 1000 filas del archivo.
    // El item existe, pero el case entero es de otra version: sin el, el tipo
    // cae al camino generico de pociones y muestra "Numero de items" como debe.
    // REMOVIDOS 2026-07-20 (4 sitios) — cases HELPER+49..53, types 465..469:
    // Devil's Eye, Devil's Key, Devil's Invitation, Remedy of Love y Rena.
    // Los cuatro bloques emitian UNICAMENTE indices fuera de las 1000 filas del
    // Text.bmd ([2397]/[2398]/[2399]/[1665]) y cortaban con `return`: no
    // dibujaban nada y ademas tapaban lo que hubiera mas abajo, igual que los
    // interceptores de las joyas.
    // REMOVIDO 2026-07-20 — case HELPER+40 (type 456, Antidote).  Emitia solo
    // GlobalText[2232] y [3088], ambos fuera de las 1000 filas del Text.bmd,
    // y cortaba con `return`.
    case ITEM_HELPER_BASE + 41:
        // [2248]/[3088] removidos 2026-07-21: fuera de rango.  Se conservan
        // las lineas de buff [88]/[89] (Ale da daño adicional al consumirse).
        addFmt(GlobalText[88], 20, C_BLUE);
        addFmt(GlobalText[89], 20, C_BLUE);
        return;
    // REMOVIDOS 2026-07-20 (8 sitios) — cases HELPER+43 y +44 (types 459 Box
    // of Luck y 460 Heart).  Cuatro indices cada uno ([2256]/[2257]/[2297]/
    // [2567]/[2568]), TODOS fuera de las 1000 filas, y cortaban con `return`:
    // no dibujaban nada y ademas tapaban lo de mas abajo.
    // REMOVIDO 2026-07-20 — case HELPER+45 (type 461, Jewel of Bless).  Segundo
    // interceptor del mismo tipo que el de 462..464: cortaba con `return` antes
    // de que Bless llegara a su descripcion real ([572]).  Sus tres indices
    // ([2258]/[2297]/[2566]) estan fuera de las 1000 filas → salia vacio.
    // ── SCROLLS: descripciones REMOVIDAS 2026-07-21 ──────────────────────────
    // Bloques HELPER+64..76/+69/+70/+71..75 y POTION+42..44: emitian
    // descripciones con GlobalText>=1000 (verificado: RenderItemInfo 0x4C4650
    // NO referencia ninguno; los reales [571]/[69] SI tienen xref).  +69/+70
    // tenian ademas logica de PORTAL (coords del Hero), pero en 0.97k son
    // spells de mago (Ice/Teleport), no warp scrolls.  Sin estos bloques los
    // scrolls muestran nombre + requisitos + linea de clase (RequireClass, ya
    // funcional).  Si el cliente de referencia mostrara la skill que enseña el
    // scroll, es feature aparte (lookup a SkillAttribute como WING+11).
    default:
        break;
    }

    // REMOVIDO 2026-07-20 (3 sitios) — ESTE bloque era el que dejaba mudas a las
    // joyas.  Interceptaba los tipos 462..464 (Jewel of Soul, Zen, Jewel of
    // Life) y cortaba con `return` ANTES de que llegaran a sus descripciones
    // reales, mas abajo en esta misma funcion ([573] Soul, [621] Life).
    // Y lo que emitia salia vacio: GlobalText[2259] y [2270] estan fuera de
    // las 1000 filas del Text.bmd.  Resultado: solo el nombre.

    switch (type) {
    case ITEM_HELPER_BASE + 10:
        addFmt(GlobalText[95], (int)ip->Durability, C_WHITE);
        return;  // [3088] removido 2026-07-21: fuera de rango
    // AUDITORIA 2026-07-20 — REMOVIDOS los cases de las pociones 448..452
    // (Apple, Small/Medium/Large Healing, Small Mana).  Son items REALES, pero
    // los cinco usaban GlobalText[1181]/[1917]/[1918]/[1919], todos fuera de las
    // 1000 filas del Text.bmd → cadena vacia, y el `return` cortaba el resto.
    // Por eso la Large Healing no mostraba NADA.  Sin estos cases caen al camino
    // generico de conteo, igual que el resto de las pociones.
    // AUDITORIA 2026-07-20 — case ITEM_HELPER_BASE + 37 (type 453, Medium
    // Mana Potion) REMOVIDO: mezclaba GlobalText[70] ("Vida", incorrecto en
    // una pocion de mana) con 10 indices fuera de las 1000 filas del Text.bmd
    // ([1860]/[1861]/[1867]..[1870]...).  Cae al camino generico como sus
    // hermanas Small (452) y Large (454).
    // RESTAURADO 2026-07-20 — items de la quest de evolucion: 471 Scroll of
    // Emperor, 472 Broken Sword, 473 Tear of Elf, 474 Soul Shard of Wizard.
    // No se venden ni se depositan.  El binario emite este par CONSECUTIVO:
    // en 0x4C5D42 el push de GlobalText[733] va inmediatamente despues del
    // sprintf de GlobalText[731] — mismo run de lineas.
    // (Antes [733] colgaba por error del bloque de Box of Luck, donde el
    //  cliente de referencia no la muestra en ninguna de sus 11 variantes.)
    case ITEM_HELPER_BASE + 55:
    case ITEM_HELPER_BASE + 56:
    case ITEM_HELPER_BASE + 57:
    case ITEM_HELPER_BASE + 58:
        addLine(GlobalText[731], C_RED);
        addLine(GlobalText[733], C_RED);
        return;
    // REMOVIDO 2026-07-20 — cases HELPER+54..58 (types 470..474).  El 470 es
    // Jewel of Creation: este bloque lo interceptaba y cortaba antes de su
    // descripcion real ([619]).  Usaba GlobalText[2511]/[2510], fuera de rango.
    default:
        break;
    }

    // (switch vacio removido 2026-07-21: sus cases eran grafts inexistentes)

    // REMOVIDO 2026-07-20: colgaba de los tipos 513..516, que NO EXISTEN — el
    // item.bmd del 0.97k tiene 512 entradas.  Los indices ([730]/[731]/[732])
    // son validos, pero estaban aplicados a items imposibles.

    if (type >= ITEM_POTION_BASE + 70 && type <= ITEM_POTION_BASE + 71) {
        addFmt(GlobalText[69], (int)ip->Durability, C_BLUE);
        addLine(GlobalText[2500 + (type - (ITEM_POTION_BASE + 70))], C_BLUE);
        return;
    }

    if (type >= ITEM_POTION_BASE + 141 && type <= ITEM_POTION_BASE + 144) {
        addLine(GlobalText[571], C_WHITE);
        return;
    }

    if (type >= ITEM_POTION_BASE + 153 && type <= ITEM_POTION_BASE + 156) {
        addFmt(GlobalText[69], (int)ip->Durability, C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 13) {
        addLine(GlobalText[572], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 14) {
        addLine(GlobalText[573], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 21) {
        // levels 2/3 ([1099]/[1291]) removidos 2026-07-21: fuera de rango.
        if (level == 1)
            addLine(GlobalText[813], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 11) {
        if (level == 7) {
            addLine(GlobalText[112], C_WHITE);
            addLine(GlobalText[113], C_WHITE);
            addLine(GlobalText[114], C_WHITE);
        // (descripcion del level 14 REMOVIDA: GlobalText[1652]/[1653] fuera de rango)
        } else {
            addLine(GlobalText[571], C_WHITE);
        }
        // REMOVIDA 2026-07-20 — GlobalText[733] "No puede ser vendido." se emitia
        // INCONDICIONALMENTE para todas las variantes de Box of Luck.  Probadas
        // las 11 contra el cliente de referencia: ninguna la muestra.
        //
        // OJO / PENDIENTE: la cadena NO es un injerto — el binario la usa en
        // RenderItemInfo, en UN solo sitio (0x4C5D42).  O sea que pertenece a
        // otro item y nos quedamos sin emitirla en ningun lado.  Falta ubicar su
        // guard real desasmando alrededor de 0x4C5D42 y re-colgarla ahi.
        if (level == 13)
            addLine(GlobalText[731], C_RED);
        return;
    }

    if (type == ITEM_POTION_BASE + 16) {
        addLine(GlobalText[621], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 17 || type == ITEM_POTION_BASE + 18) {
        addLine(GlobalText[637], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 19) {
        addLine(GlobalText[638], C_WHITE);
        addLine(GlobalText[639], C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 10 && level >= 1 && level <= 8) {
        addFmt(GlobalText[157], 3, C_WHITE);
        return;
    }

    if (type == ITEM_POTION_BASE + 22) {
        addLine(GlobalText[619], C_WHITE);
        return;
    }

    if (type == 384 + 15) {
        addLine(GlobalText[574], C_WHITE);
        return;
    }

    // REMOVIDO 2026-07-21 — bloque escrito como "alas" (384+32..34) pero que en
    // realidad cae sobre los tipos 416, 417 y 418: Guardian Angel, Imp y Horn
    // of Uniria.  Les emitia GlobalText[571] ("Tiralo al suelo y podras recibir
    // zen o items"), que es la descripcion de la Box of Luck, y cortaba con
    // `return` antes del bloque propio de los pets que viene justo abajo
    // (`if (type == ITEM_HELPER_BASE + 0)` etc.).
    // Las alas de verdad son 384..390; 384+32 ya se pasa de ese rango.

    if (type == ITEM_HELPER_BASE + 0) {
        // Guardian Angel: keep these lines explicit instead of depending on
        // shifted GlobalText indices from newer/custom text tables.
        addLine("Absorbe 20% del daño", C_WHITE);
        addLine("Incrementa +50 vida máxima", C_WHITE);
        return;
    }

    if (type == ITEM_HELPER_BASE + 1) {
        addLine(GlobalText[576], C_WHITE);
        return;
    }

    // SIMPLIFICADO 2026-07-21 — quitado HELPER+30 (type 446, slot vacio); solo
    // queda 433 (Blood Bone), item real.  El absorb del 446 (10+level) era
    // codigo muerto.
    if (type == 433) {
        addFmt(GlobalText[577], 20 + level * 2, C_WHITE);
        addFmt(GlobalText[578], 10 + level * 2, C_WHITE);
        return;
    }

    if ((type >= 384 && type <= 386) || type == 425) {
        addFmt(GlobalText[577], 12 + level * 2, C_WHITE);
        addFmt(GlobalText[578], 12 + level * 2, C_WHITE);
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    if ((type >= ITEM_WING_BASE + 3 && type <= ITEM_WING_BASE + 6) || type == ITEM_WING_BASE + 42) {
        addFmt(GlobalText[577], 32 + level, C_WHITE);
        addFmt(GlobalText[578], 25 + level * 2, C_WHITE);
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    if ((type >= ITEM_WING_BASE + 36 && type <= ITEM_WING_BASE + 40) ||
        type == ITEM_WING_BASE + 43 ||
        type == ITEM_WING_BASE + 50) {
        addFmt(GlobalText[577], 39 + level * 2, C_WHITE);
        if (type == ITEM_WING_BASE + 40 || type == ITEM_WING_BASE + 50) {
            addFmt(GlobalText[578], 24 + level * 2, C_WHITE);
        } else {
            addFmt(GlobalText[578], 39 + level * 2, C_WHITE);
        }
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    if (type == ITEM_HELPER_BASE + 3) {
        addFmt(GlobalText[577], 15, C_WHITE);
        addFmt(GlobalText[578], 10, C_WHITE);
        return;
    }

    if (type == ITEM_HELPER_BASE + 14) {
        // level 1 ([1236]) removido 2026-07-21: fuera de rango (sin Dark Lord
        // en 0.97k, Loch's Feather no tiene tier).
        if (level == 0)
            addLine(GlobalText[748], C_WHITE);
        return;
    }

    if (type == ITEM_HELPER_BASE + 15) {
        // Fruit (431): descripcion = stat que sube + "Incrementa 1~3 puntos".
        // Removidos 2026-07-21: 2do switch ([1910]), linea [1908], caso level 4
        // ([1900]) y el bloque "equipable por Soul Master" — grafts (el 0.97k
        // solo tiene 4 stats: Ene/Vit/Agi/Fue = levels 0..3).
        char line[256];
        switch (level) {
        case 0: snprintf(line, sizeof(line), "%s %s", GlobalText[168], GlobalText[636]); addLine(line, C_WHITE); break;
        case 1: snprintf(line, sizeof(line), "%s %s", GlobalText[169], GlobalText[636]); addLine(line, C_WHITE); break;
        case 2: snprintf(line, sizeof(line), "%s %s", GlobalText[167], GlobalText[636]); addLine(line, C_WHITE); break;
        case 3: snprintf(line, sizeof(line), "%s %s", GlobalText[166], GlobalText[636]); addLine(line, C_WHITE); break;
        default: break;
        }
        return;
    }

    // REMOVIDO 2026-07-20: emitia GlobalText[69] ("Numero de items") para los
    // mismos rangos que ya cubre AppendInventoryDurabilityTooltipLines, asi que
    // la linea salia DOS VECES (visible en Large Mana Potion).

    if (type == ITEM_HELPER_BASE + 16 || type == ITEM_HELPER_BASE + 17) {
        addLine(GlobalText[816], C_WHITE);
        return;
    }

    if (type == ITEM_HELPER_BASE + 18) {
        addLine(GlobalText[814], C_WHITE);
        addLine("\n", C_WHITE);
        addLine(GlobalText[638], C_WHITE);
        addLine(GlobalText[639], C_WHITE);
        return;
    }

    if ((type >= 387 && type <= 390) || type == 426) {
        addFmt(GlobalText[577], 32 + level, C_WHITE);
        addFmt(GlobalText[578], 25 + level * 2, C_WHITE);
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    if ((type >= ITEM_WING_BASE && type <= ITEM_WING_BASE + 2) || type == ITEM_WING_BASE + 41) {
        addFmt(GlobalText[577], 12 + level * 2, C_WHITE);
        addFmt(GlobalText[578], 12 + level * 2, C_WHITE);
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    if ((type >= 420 && type <= 424) || type == 427 || type == 434) {
        addFmt(GlobalText[577], 39 + level * 2, C_WHITE);
        if (type == 424 || type == 434)
            addFmt(GlobalText[578], 24 + level * 2, C_WHITE);
        else
            addFmt(GlobalText[578], 39 + level * 2, C_WHITE);
        addLine(GlobalText[579], C_WHITE);
        return;
    }

    // REMOVIDO 2026-07-21 — WING+130..135 (types 514..519 >=512): tipos
    // imposibles.  "Absorb wings" de una version con mas de 512 items.
    // REMOVIDO 2026-07-21 — HELPER+42 (type 458, Town Portal Scroll).  Emitia 7
    // lineas de GlobalText[976..982] (filas VACIAS en el Text.bmd) + [3088]
    // graft.  Confirmado contra el cliente de referencia: Town Portal NO tiene
    // descripcion (las filas 974..985 quedaron vacias para completar a futuro).

    // Aca habia un `switch (type)` con los cases HELPER+110..113 (types 526..529),
    // removidos 2026-07-20 por inexistentes (la tabla del item.bmd tiene 512
    // entradas).  El switch quedaba solo con `default: break;` — warning C4065.
}

// ── RESTAURADA 2026-07-21 ────────────────────────────────────────────────────
// Esta funcion se perdio por una edicion POR NUMERO DE LINEA que borro su
// definicion dejando vivas las 2 llamadas (error C3861 en 1786 y 1921).
// Recuperada del backup del hilo paralelo y con las correcciones de la sesion
// 2026-07-20 REAPLICADAS, esta vez ancladas por texto.
// LECCION: en este archivo, editar por numero de linea es una bomba.
static void AppendInventoryDurabilityTooltipLines(ITEM* ip, ITEM_ATTRIBUTE* p, unsigned int level, int attrBase)
{
    if (!ip || !p || DAT_07eaa154 >= 28)
        return;

    constexpr short ITEM_WING_BASE = 384;
    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_POTION_BASE = 448;
    constexpr short ITEM_BOW_BASE = 128;

    const short type = ip->Type;
    constexpr int C_WHITE = 0;

    auto addLine = [](const char* text, int color, bool bold = false) {
        if (!text || !text[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, "%s", text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt = [](const char* fmt, int value, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt2 = [](const char* fmt, int value1, int value2, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value1, value2);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    bool bDurExist = false;
    if ((p->Durability || p->MagicDurability) &&
        ((((type < ITEM_WING_BASE || type >= ITEM_HELPER_BASE) && type < ITEM_POTION_BASE) ||
          (type >= ITEM_WING_BASE && type <= ITEM_WING_BASE + 6) ||
          (type >= ITEM_WING_BASE + 36 && type <= ITEM_WING_BASE + 43) ||
          (type >= ITEM_WING_BASE + 49 && type <= ITEM_WING_BASE + 50)))) {
        bDurExist = true;
    }

    bool success = false;
    if ((bDurExist || ip->Durability) &&
        (type < ITEM_HELPER_BASE + 14 || type > ITEM_HELPER_BASE + 19) &&
        !(type == ITEM_HELPER_BASE + 20 && level == 1) &&
        !(type == ITEM_HELPER_BASE + 20 && level == 2) &&
        type != ITEM_HELPER_BASE + 29 &&
        type != ITEM_POTION_BASE + 7 &&
        type != ITEM_HELPER_BASE + 7 &&
        type != ITEM_HELPER_BASE + 11 &&
        // (quitada la exclusion de HELPER+35 = 451 Large Healing Potion: era
        //  lo que la dejaba sin la linea "Numero de items")
        !(type >= ITEM_POTION_BASE + 70 && type <= ITEM_POTION_BASE + 71) &&
        !(type >= ITEM_HELPER_BASE + 54 && type <= ITEM_HELPER_BASE + 58) &&
        !(type >= ITEM_POTION_BASE + 78 && type <= ITEM_POTION_BASE + 82) &&
        type != ITEM_HELPER_BASE + 66 &&
        !(type >= ITEM_HELPER_BASE + 71 && type <= ITEM_HELPER_BASE + 75) &&
        type != ITEM_HELPER_BASE + 97 &&
        type != ITEM_HELPER_BASE + 98 &&
        type != ITEM_POTION_BASE + 91 &&
        type != ITEM_HELPER_BASE + 99 &&
        type != ITEM_POTION_BASE + 133) {
        if ((type >= ITEM_POTION_BASE && type <= ITEM_POTION_BASE + 8) ||
            (type == ITEM_POTION_BASE + 21 && level == 3) ||
            type == ITEM_BOW_BASE + 7 || type == ITEM_BOW_BASE + 15 ||
            (type >= ITEM_POTION_BASE + 35 && type <= ITEM_POTION_BASE + 40) ||
            type == ITEM_POTION_BASE + 133 ||
            (type >= ITEM_POTION_BASE + 46 && type <= ITEM_POTION_BASE + 50) ||
            (type >= ITEM_POTION_BASE + 153 && type <= ITEM_POTION_BASE + 156)) {
            addFmt(GlobalText[69], (int)ip->Durability, C_WHITE);
            success = true;
        // REMOVIDAS 2026-07-20 (2 sitios): ramas INALCANZABLES.  Cubrian los tipos
        // 448/449/450 (Apple, Small y Medium Healing Potion), que ya matchean la
        // PRIMERA condicion de esta cadena else-if (448..456).  Encima usaban
        // GlobalText[1181], fuera de las 1000 filas del Text.bmd.
        //
        // REMOVIDA tambien la rama HELPER+37 (453, Medium Mana Potion): le ponia
        // GlobalText[70] = "Vida: %d".  En el binario [70] tiene UNA sola
        // referencia en RenderItemInfo (0x4C6875) y [175] ("Mana: %d") NO la usa
        // RenderItemInfo — solo el tooltip de skills (sub_4C9730).  La pocion de
        // mana no lleva ninguna de las dos lineas; ademas la Small (452) y la
        // Large (454) tampoco la mostraban.
        } else if (type >= ITEM_HELPER_BASE && type <= ITEM_HELPER_BASE + 7) {
            addFmt(GlobalText[70], (int)ip->Durability, C_WHITE);
            success = true;
        } else if (type == ITEM_HELPER_BASE + 10) {
            addFmt(GlobalText[95], (int)ip->Durability, C_WHITE);
            success = true;
        } else if (type == ITEM_HELPER_BASE + 64 || type == ITEM_HELPER_BASE + 65) {
            addFmt(GlobalText[70], (int)ip->Durability, C_WHITE);
            success = true;
        } else if (type == ITEM_HELPER_BASE + 67 || type == ITEM_HELPER_BASE + 80 ||
                   type == ITEM_HELPER_BASE + 106 || type == ITEM_HELPER_BASE + 123) {
            addFmt(GlobalText[70], (int)ip->Durability, C_WHITE);
            success = true;
        // REMOVIDAS 2026-07-20 (6 sitios):
        //  · tipos 462..464 (Jewel of Soul, Zen, Jewel of Life): usaban
        //    GlobalText[2260], fuera de rango.  Las joyas NO se acumulan en el
        //    0.97k — verificado contra el cliente de referencia, que no muestra
        //    conteo en ninguna.  Su texto es la DESCRIPCION, que ya emite
        //    AppendInventorySpecialTooltipLines ([572]/[573]/[621]/[619]/[574]).
        //  · tipos 541..543, 501, 477 y 537: no existen en el item.bmd del 0.97k
        //    (541/543/537 pasan las 512 entradas de la tabla; 501 y 477 son
        //    slots vacios).  Usaban [2260]/[2296]/[3105]/[3106], fuera de rango.
        } else if (type == ITEM_POTION_BASE + 100) {
            addFmt(GlobalText[69], (int)ip->Durability, C_WHITE);
            success = true;
        } else if (bDurExist) {
            unsigned int maxDurability = FUN_004c45c0(ip, attrBase, (int)level) & 0xFFFF;
            addFmt2(GlobalText[71], (int)ip->Durability, (int)maxDurability, C_WHITE);
            success = true;
        }
    } else if (type == ITEM_HELPER_BASE + 10) {
        addFmt(GlobalText[95], (int)ip->Durability, C_WHITE);
        success = true;
    }
}

static void AppendInventoryLateBonusTooltipLines(ITEM* ip, ITEM_ATTRIBUTE* p)
{
    if (!ip || !p || DAT_07eaa154 >= 28)
        return;

    constexpr short ITEM_SWORD_BASE = 0;
    constexpr short ITEM_WING_BASE = 384;
    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_POTION_BASE = 448;
    constexpr short ITEM_BOW_BASE = 128;
    constexpr short ITEM_STAFF_BASE = 160;
    constexpr short ITEM_GLOVES_BASE = 320;
    constexpr short ITEM_BOOTS_BASE = 352;
    constexpr int C_WHITE = 0;
    constexpr int C_BLUE = 1;

    auto addLine = [](const char* text, int color, bool bold = false) {
        if (!text || !text[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, "%s", text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt = [](const char* fmt, int value, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    const short type = ip->Type;
    const int level = (ip->Level >> 3) & 0xF;

    if ((type == ITEM_BOW_BASE + 7 || type == ITEM_BOW_BASE + 15) && level >= 1) {
        addFmt(GlobalText[577], level * 2 + 1, C_BLUE);
        addFmt(GlobalText[88], 1, C_BLUE);
    }

    if (type >= ITEM_BOOTS_BASE && type < ITEM_BOOTS_BASE + 32) {
        if (level >= 5) {
            addLine("\n", 4);
            addLine(GlobalText[78], C_BLUE, true);
        }
    }

    if (type >= ITEM_GLOVES_BASE && type < ITEM_GLOVES_BASE + 32) {
        if (level >= 5) {
            addLine("\n", 4);
            addLine(GlobalText[93], C_BLUE, true);
        }
    }

    if ((type >= ITEM_STAFF_BASE && type < ITEM_STAFF_BASE + 32) ||
        type == ITEM_SWORD_BASE + 31 ||
        type == ITEM_SWORD_BASE + 23 ||
        type == ITEM_SWORD_BASE + 25 ||
        type == ITEM_SWORD_BASE + 21 ||
        type == ITEM_SWORD_BASE + 28) {
        addLine("\n", 4);
        const int textIndex = (type >= ITEM_STAFF_BASE + 21 && type <= ITEM_STAFF_BASE + 29) ? 1691 : 79;
        addFmt(GlobalText[textIndex], (int)p->MagicPower, C_BLUE, true);
    }

    // REMOVIDO 2026-07-20: emisor DUPLICADO de GlobalText[574] para el tipo 399
    // (Jewel of Chaos).  La descripcion ya la emite el bloque de joyas de
    // AppendInventorySpecialTooltipLines (junto a [572] Bless, [573] Soul,
    // [621] Life, [619] Creation), que ademas corta con `return`.  Al estar
    // tambien aca, la linea "Es utilizado para combinar items" salia DOS VECES.
}

static bool GetInventorySpecialOptionText(short type, BYTE option, BYTE value, int mana, char* dst, size_t dstSize)
{
    if (!dst || dstSize == 0)
        return false;

    dst[0] = '\0';

    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_WING_BASE = 384;

    switch (option) {
    case 18:
        snprintf(dst, dstSize, GlobalText[80], mana);
        return true;
    case 19:
        snprintf(dst, dstSize, GlobalText[81], mana);
        return true;
    case 20:
        snprintf(dst, dstSize, GlobalText[82], mana);
        return true;
    case 21:
        snprintf(dst, dstSize, GlobalText[83], mana);
        return true;
    case 22:
        snprintf(dst, dstSize, GlobalText[84], mana);
        return true;
    case 23:
        snprintf(dst, dstSize, GlobalText[85], mana);
        return true;
    case 24:
        snprintf(dst, dstSize, GlobalText[86], mana);
        return true;
    case 49:
        snprintf(dst, dstSize, GlobalText[745], mana);
        return true;
    case 56:
        snprintf(dst, dstSize, GlobalText[98], mana);
        return true;
    case 60:
        snprintf(dst, dstSize, GlobalText[88], value);
        return true;
    case 61:
        snprintf(dst, dstSize, GlobalText[89], value);
        return true;
    case 62:
        snprintf(dst, dstSize, GlobalText[90], value);
        return true;
    case 63:
        snprintf(dst, dstSize, GlobalText[91], value);
        return true;
    case 64:
        // GlobalText[87] = "Suerte (tasa de exito de Jewel of Soul +25%%)".
        // El %% es un ESCAPE: el original la pasa a sprintf como cadena de
        // formato SIN argumentos, y ahi %% colapsa a %.  Copiarla con "%s"
        // (como se hacia aca) dejaba el doble porcentaje visible: "+25%%".
        CopyCollapsingPercent(dst, dstSize, GlobalText[87]);
        return true;
    case 65:
        if (!(ITEM_HELPER_BASE + 14 <= type && type <= ITEM_HELPER_BASE + 18)) {
            snprintf(dst, dstSize, GlobalText[92], value);
            return true;
        }
        return false;
    case 66:
        snprintf(dst, dstSize, GlobalText[622], 4);
        return true;
    case 67:
        snprintf(dst, dstSize, GlobalText[623], 4);
        return true;
    case 68:
        snprintf(dst, dstSize, GlobalText[624], 4);
        return true;
    case 69:
        snprintf(dst, dstSize, GlobalText[625], 5);
        return true;
    case 70:
        snprintf(dst, dstSize, GlobalText[626], 10);
        return true;
    case 71:
        snprintf(dst, dstSize, GlobalText[627], 30);
        return true;
    case 72:
        snprintf(dst, dstSize, GlobalText[628], 10);
        return true;
    case 73:
        snprintf(dst, dstSize, GlobalText[629], 20);
        return true;
    case 74:
        snprintf(dst, dstSize, GlobalText[630], 2);
        return true;
    case 75:
        snprintf(dst, dstSize, GlobalText[631], 20);
        return true;
    case 76:
        snprintf(dst, dstSize, GlobalText[632], 2);
        return true;
    case 77:
        snprintf(dst, dstSize, GlobalText[633], value);
        return true;
    case 78:
        snprintf(dst, dstSize, GlobalText[634], 8);
        return true;
    case 79:
        snprintf(dst, dstSize, GlobalText[635], 8);
        return true;
    case 80:
        snprintf(dst, dstSize, GlobalText[740], value);
        return true;
    case 81:
        snprintf(dst, dstSize, GlobalText[741], value);
        return true;
    case 82:
        snprintf(dst, dstSize, GlobalText[742], value);
        return true;
    case 83:
        snprintf(dst, dstSize, GlobalText[743], value);
        return true;
    case 84:
        snprintf(dst, dstSize, GlobalText[744], value);
        return true;
    case 90:
        snprintf(dst, dstSize, "%s", GlobalText[746]);
        return true;
    default:
        return false;
    }
}

static void AppendInventorySpecialOptionLines(ITEM* ip, ITEM_ATTRIBUTE* p)
{
    if (!ip || !p || DAT_07eaa154 >= 28 || ip->SpecialNum <= 0)
        return;

    constexpr short ITEM_HELPER_BASE = 416;
    constexpr short ITEM_WING_BASE = 384;
    constexpr short ITEM_SWORD_BASE = 0;
    constexpr int C_WHITE = 0;
    constexpr int C_BLUE = 1;
    constexpr int C_DARKRED = 5;

    auto addLine = [](const char* text, int color, bool bold = false) {
        if (!text || !text[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, "%s", text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    auto addFmt = [](const char* fmt, int value, int color, bool bold = false) {
        if (!fmt || !fmt[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, value);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = bold ? 1 : 0;
        DAT_07eaa154++;
    };

    if (!(ip->Type >= ITEM_HELPER_BASE + 109 && ip->Type <= ITEM_HELPER_BASE + 115))
        addLine("\n", 4);

    for (int i = 0; i < ip->SpecialNum && DAT_07eaa154 < 28; ++i) {
        if (ip->Type >= ITEM_HELPER_BASE + 109 && ip->Type <= ITEM_HELPER_BASE + 115)
            break;

        const BYTE special = ip->Special[i];
        const int value = (int)ip->SpecialValue[i];
        int manaCost = 0;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        GetSkillInformation((int)special, 1, NULL, &manaCost, NULL, NULL);
        bool wrote = GetInventorySpecialOptionText(ip->Type, special, (BYTE)value, manaCost, dst, 100);

        if (wrote) {
            DAT_07e91708[DAT_07eaa154] = C_BLUE;
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }

        if (DAT_07eaa154 >= 28)
            break;

        if (special == 64) {
            addFmt(GlobalText[94], value, C_BLUE);
        } else if (special == 49) {
            addLine(GlobalText[179], C_DARKRED);
        } else if (ip->Type == ITEM_SWORD_BASE + 31 && special == 60) {
            addFmt(GlobalText[89], value, C_BLUE);
        }
    }

    if (DAT_07eaa154 < 28)
        addLine("\n", 4);
}

static void AppendInventoryRequirementTooltipLines(ITEM* ip)
{
    if (!ip || DAT_07eaa154 >= 28)
        return;

    constexpr int TEXT_COLOR_WHITE = 0;
    constexpr int TEXT_COLOR_RED = 2;
    BYTE* CA = (BYTE*)DAT_07cf1ff4;
    if (!CA)
        return;

    auto addRequirementLine = [&](const char* fmt, int reqVal, int currentVal, int okColor) {
        if (!fmt || !fmt[0] || reqVal <= 0 || DAT_07eaa154 >= 28)
            return;

        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, fmt, reqVal);

        const bool missing = currentVal < reqVal;
        DAT_07e91708[DAT_07eaa154] = missing ? TEXT_COLOR_RED : okColor;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
        DAT_07eaa154++;

        if (missing && DAT_07eaa154 < 28) {
            char* needDst = lpString_07e90798 + DAT_07eaa154 * 100;
            if (GlobalText[74] && GlobalText[74][0]) {
                snprintf(needDst, 100, GlobalText[74], reqVal - currentVal);
            } else {
                snprintf(needDst, 100, "Need: %d", reqVal - currentVal);
            }
            DAT_07e91708[DAT_07eaa154] = TEXT_COLOR_RED;
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }
    };

    const int level = *(WORD*)(CA + 0x0E);
    const int strength = *(WORD*)(CA + 0x14);
    const int agility = *(WORD*)(CA + 0x16);
    const int energy = *(WORD*)(CA + 0x1A);

    addRequirementLine(GlobalText[73], (int)ip->RequireStrength, strength, TEXT_COLOR_WHITE);
    addRequirementLine(GlobalText[75], (int)ip->RequireDexterity, agility, TEXT_COLOR_WHITE);
    addRequirementLine(GlobalText[77], (int)ip->RequireEnergy, energy, TEXT_COLOR_WHITE);
    if (ip->RequireLevel && ip->Type != 416 + 14) {
        addRequirementLine(GlobalText[76], (int)ip->RequireLevel, level, TEXT_COLOR_WHITE);
    }
}

static void AppendInventoryRequireClassLines(ITEM_ATTRIBUTE* pItem)
{
    if (!pItem || DAT_07eaa154 >= 28)
        return;

    const BYTE byFirstClass = GetHeroTooltipBaseClass();
    const BYTE byStepClass = GetHeroTooltipStepClass();

    int requireCount = 0;
    for (int i = 0; i < MAX_CLASS; ++i) {
        if (pItem->RequireClass[i] == 1)
            requireCount++;
    }
    if (requireCount == MAX_CLASS)
        return;

    auto addClassLine = [](const char* text, int color) {
        if (!text || !text[0] || DAT_07eaa154 >= 28)
            return;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        snprintf(dst, 100, "%s", text);
        DAT_07e91708[DAT_07eaa154] = color;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
        DAT_07eaa154++;
    };

    addClassLine("\n", 4);

    for (int i = 0; i < MAX_CLASS && DAT_07eaa154 < 28; ++i) {
        BYTE byRequireClass = pItem->RequireClass[i];
        if (byRequireClass == 0)
            continue;

        int textColor = (i == byFirstClass && byRequireClass <= byStepClass) ? 0 : 5;
        char line[100] = {};
        switch (i) {
        case CLASS_DARK_WIZARD:
            // FIX 2026-07-20 — los nombres de tier 2 estaban CORRIDOS UNO.
            // Volcado del Text.bmd desencriptado:
            //   20 Dark Wizard   21 Dark Knight  22 Fairy Elf  23 Magic Gladiator
            //   24 Soul Master   25 Blade Knight 26 Muse Elf
            //   27/28 "Reservation: job"  (el MG no tiene tier 2)
            // Se usaba 25 para el mago evolucionado, y por eso un Grand Soul
            // Armor decia "Blade Knight" en lugar de "Soul Master".
            //
            // Los tier 3 (1668..1671) NO existen en 0.97k y ademas caen FUERA
            // de GlobalText[1000][300]: leerlos es un desborde del array.
            // El Text.bmd tiene exactamente 1000 filas (300000 bytes).
            if (byRequireClass == 1)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[20]);
            else if (byRequireClass == 2)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[24]);
            break;
        case CLASS_DARK_KNIGHT:
            if (byRequireClass == 1)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[21]);
            else if (byRequireClass == 2)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[25]);
            break;
        case CLASS_FAIRY_ELF:
            if (byRequireClass == 1)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[22]);
            else if (byRequireClass == 2)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[26]);
            break;
        case CLASS_MAGIC_GLADIATOR:
            if (byRequireClass == 1)
                snprintf(line, sizeof(line), GlobalText[61], GlobalText[23]);
            break;
        default:
            break;
        }

        if (line[0]) {
            addClassLine(line, textColor);
        }
    }
}

void __cdecl FUN_004c4650(void* param_1, void* param_2, void* param_3_v, int param_4)
{
    __try {
        FUN_004c4650_impl(param_1, param_2, param_3_v, param_4);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgLogPublic("RIP CRASHED inside _impl — caught by SEH");
        extern DWORD DAT_07eaa160;
        DAT_07eaa160 = 0;
    }
}

void __cdecl FUN_004c8d70(void* param_1, int param_2, void* param_3_v)
{
    __try {
        FUN_004c8d70_impl(param_1, param_2, param_3_v);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgLogPublic("RRI CRASHED inside _impl — caught by SEH");
        extern DWORD DAT_07eaa160;
        DAT_07eaa160 = 0;
    }
}

// FUN_004c4650 @ 0x004C4650 — RenderItemInfo(int sx, int sy, ITEM* ip, bool bSell)
// param_3 = ITEM* (ushort array: [0]=type, [0x24]=options, [0x1b]=option flags, etc.)
// unaff_EBP and unaff_ESI are self-assigned locally — anti-tamper noise.
extern "C" void __cdecl FUN_004c4650_impl(void* param_1, void* param_2, void* param_3_v, int param_4)
{
    // 2026-05-08: defensive — si nos llaman antes de que WinMain initialice
    // el ItemAttribute table (DAT_07d78068), o si DAT_07d78068 fue clobbered
    // a 0x1 por un writer desconocido, ItemAttribute_Base() recupera del
    // backup. Si tampoco es válido, saltamos.
    unsigned int attrBaseOK = ItemAttribute_Base();
    if (attrBaseOK == 0) return;
    if (param_3_v == nullptr || (uintptr_t)param_3_v < 0x100000) return;
    unsigned short* param_3 = (unsigned short*)param_3_v;

    // attrBase = itemType * 0x40 + ItemAttribute_Base()
    int attrBase = (short)*param_3 * 0x40 + (int)attrBaseOK;

    // Reset counters and string buffer
    DAT_07eaa154 = 0;
    DAT_07eaa158 = 0;

    // Clear color array (0x14 DWORDs at DAT_07e91708)
    for (int i = 0; i < 0x14; i++)
        DAT_07e91708[i] = 0;

    // Clear string table (each slot = 100 bytes, 30 slots = 3000-byte buffer).
    // BUG-FIX 2026-05-03: was `< 0x7e91350` (absolute end bound from source binary).
    // In our build lpString_07e90798 is linker-placed; literal address is junk.
    for (int i = 0; i < 30; ++i)
        lpString_07e90798[i * 100] = 0;

    // Slot 0: item name
    crt_sprintf(lpString_07e90798, DAT_0055a4e0);
    DAT_07eaa154++;
    DAT_07eaa158++;

    unsigned short itemType = *param_3;
    unsigned int   level    = (*(unsigned int*)(param_3 + 2) >> 3) & 0xf;

    // Determine quality tier: 0=normal, 1=enchant, 3=set, 4=excellent, 6=ancient
    unsigned int tier = 0;
    if (itemType == 0x1cd || itemType == 0x1ce || (int)itemType == 399 ||
        itemType == 0x1d0 || itemType == 0x1d6 ||
        itemType == 0x1d1 || itemType == 0x1d2 || itemType == 0x1d3 ||
        itemType == 0x1b0 || itemType == 0x1b1) {
        tier = 3;
    } else if (itemType == 0xaa || itemType == 0x13 || itemType == 0x92) {
        tier = 6;
    } else {
        char opt1      = (char)param_3[0x12];
        unsigned char optFlags = *(unsigned char*)((char*)param_3 + 0x1b) & 0x3f;
        if (opt1 != '\0' && optFlags != 0) {
            tier = 4;
        } else {
            tier = (level > 6) ? 3u : (opt1 != '\0' ? 1u : 0u);
        }
    }
    if ((short)itemType > 0x182 && (short)itemType < 0x187) {
        tier = (level < 7) ? (unsigned int)((char)param_3[0x12] != '\0') : 3;
    }

    // REMOVIDO 2026-07-21 — bloque de "repair gold" dentro de RenderItemInfo.
    // Llamaba a ConvertRepairGold escribiendo en `lpString_07e90798` = SLOT 0,
    // o sea la linea que va ARRIBA del nombre del item.  Sintoma: cualquier
    // item con curDur < maxDur mostraba un numero suelto encima del nombre
    // ("1" en Guardian Angel/Imp, "5,200" en Horn of Dinorant), y los que
    // estaban full (Horn of Uniria, 255/255) no mostraban nada.
    //
    // NO ES DEL ORIGINAL: ConvertRepairGold (0x4C3EF0) tiene exactamente 3
    // xrefs en el binario — dos en sub_4C4080 (0x4C4206 y 0x4C42D2) y uno en
    // RenderRepairInfo (0x4C8F28).  NINGUNO en RenderItemInfo (0x4C4650).
    // El precio de reparacion lo calcula RenderRepairInfo, que es la otra rama
    // del dispatch de Scene_MapTick.
    // ── Slot: item NAME line (with +N suffix when level > 0) ────────────────
    // Per IDA L786-829: most items render `Name +N`. Exceptions: item types
    // 19/146/170 use Name only with optional Level. Items with excellent
    // options (Option1 & 0x3F != 0) prepend "Excellent " (GlobalText[620]).
    {
        ITEM* it = (ITEM*)param_3;
        ITEM_ATTRIBUTE* p = (ITEM_ATTRIBUTE*)(uintptr_t)attrBase;
        const char* itemName = (const char*)attrBase;   // p->Name at +0
        if ((uintptr_t)itemName < 0x100000 || (uintptr_t)itemName >= 0x80000000)
            itemName = "";
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        unsigned char optFlags = *(unsigned char*)((char*)param_3 + 0x1b) & 0x3f;

        if (!BuildInventorySpecialNameLine(it, p, level, dst, 100)) {
            if (optFlags != 0 && itemType != 19 && itemType != 146 && itemType != 170) {
                const char* excPrefix = GlobalText[620];
                if (level > 0) {
                    snprintf(dst, 100, "%s %s +%u",
                             excPrefix && excPrefix[0] ? excPrefix : "Excellent",
                             itemName ? itemName : "",
                             (unsigned)level);
                } else {
                    snprintf(dst, 100, "%s %s",
                             excPrefix && excPrefix[0] ? excPrefix : "Excellent",
                             itemName ? itemName : "");
                }
            } else if (level > 0) {
                snprintf(dst, 100, "%s +%u", itemName ? itemName : "", (unsigned)level);
            } else {
                snprintf(dst, 100, "%s", itemName ? itemName : "");
            }
        }
    }
    {
        int specialNameColor = GetInventorySpecialNameColor((ITEM*)param_3);
        DAT_07e91708[DAT_07eaa154] = specialNameColor;
    }
    ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 1;
    DAT_07eaa154++;

    // ── Stat lines: damage, defense, magic defense, attack/walk speed ──────
    // Per IDA L1080-1163. Each row only added if the corresponding ITEM field
    // is non-zero. Color flag set to 1 (highlight) when item has excellent
    // options on combat stats.
    auto addStatLine = [&](const char* fmt, int v1, int v2, int colorFlag) {
        if (DAT_07eaa154 >= 28) return;   // pool cap (30 slots minus tail)
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        if (v2 == INT_MIN) snprintf(dst, 100, fmt, v1);
        else               snprintf(dst, 100, fmt, v1, v2);
        DAT_07e91708[DAT_07eaa154] = colorFlag;
        ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
        DAT_07eaa154++;
    };

    {
        ITEM* it = (ITEM*)param_3;
        ITEM_ATTRIBUTE* p = (ITEM_ATTRIBUTE*)(uintptr_t)attrBase;
        BYTE excFlags = it->Option1 & 0x3F;

        // ── Damage range — for weapons (slot+0x18 = DamageMin, +0x1C = Max).
        int damageMin = (int)it->DamageMin;
        int damageMax = (int)it->DamageMax;
        if (damageMin) {
            int dmgMin = damageMin, dmgMax = damageMax;
            if (itemType >= 160 && itemType <= 192) {
                // Bow class — display half value (per-arrow).
                dmgMin /= 2;
                dmgMax /= 2;
            }
            int twoHand = (int)it->TwoHand;
            int gtIdx = (twoHand & 1) ? 41 : 40;   // GlobalText[40] = "Damage", [41] = "TwoHand"
            const char* gt = GlobalText[gtIdx];
            char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
            if (gt && gt[0]) snprintf(dst, 100, gt, dmgMin, dmgMax);
            else             snprintf(dst, 100, "Damage: %d ~ %d", dmgMin, dmgMax);
            DAT_07e91708[DAT_07eaa154] = (excFlags != 0) ? 1 : 0;
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }

        // ── Defense (armor)
        int defense = (int)it->Defense;
        if (defense) {
            const char* gt = GlobalText[65];
            char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
            if (gt && gt[0]) snprintf(dst, 100, gt, defense);
            else             snprintf(dst, 100, "Defense: %d", defense);
            int hl = (itemType >= 224 && itemType < 384 &&
                      excFlags != 0) ? 1 : 0;
            DAT_07e91708[DAT_07eaa154] = hl;
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }

        // ── Magic Defense
        int magicDef = (int)it->MagicDefense;
        if (magicDef) {
            const char* gt = GlobalText[66];
            char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
            if (gt && gt[0]) snprintf(dst, 100, gt, magicDef);
            else             snprintf(dst, 100, "Magic Defense: %d", magicDef);
            DAT_07e91708[DAT_07eaa154] = 0;
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }

        if (p) {
            // ── Defense Rate (for shields)
            if (it->SuccessfulBlocking) {
                int block = (int)it->SuccessfulBlocking;
                const char* gt = GlobalText[67];
                char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
                if (gt && gt[0]) snprintf(dst, 100, gt, block);
                else             snprintf(dst, 100, "Defense Rate: %d", block);
                DAT_07e91708[DAT_07eaa154] = (excFlags != 0) ? 1 : 0;
                ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
                DAT_07eaa154++;
            }

            // ── Attack Speed (weapons) / GlobalText[64]
            if (it->WeaponSpeed) {
                const char* gt = GlobalText[64];
                char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
                if (gt && gt[0]) snprintf(dst, 100, gt, (int)it->WeaponSpeed);
                else             snprintf(dst, 100, "Attack Speed: %d", (int)it->WeaponSpeed);
                DAT_07e91708[DAT_07eaa154] = 0;
                ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
                DAT_07eaa154++;
            }

            // ── Walk Speed (boots)
            if (it->WalkSpeed) {
                const char* gt = GlobalText[68];
                char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
                if (gt && gt[0]) snprintf(dst, 100, gt, (int)it->WalkSpeed);
                else             snprintf(dst, 100, "Walk Speed: %d", (int)it->WalkSpeed);
                DAT_07e91708[DAT_07eaa154] = 0;
                ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
                DAT_07eaa154++;
            }

        }

        // ── Sell price (when in shop sell mode = bSell != 0) ────────────────
        // FIX 2026-07-20: usaba GlobalText[78], que NO es el precio: en el
        // Text.bmd la 78 es "Incrementa velocidad de movimiento" (sin ningun
        // especificador), asi que imprimia esa frase literal.
        // Los indices reales, sacados del Text.bmd desencriptado, son:
        //     [62] "Precio de compra: %s"
        //     [63] "Precio de venta: %s"
        // Ojo: llevan %s, NO %d — el numero va pre-formateado con separador de
        // miles ("4,700").  Pasarle un int a un %s haria que snprintf lo
        // deferencie como char* → basura o AV.
        if (param_4 != 0 && DAT_07eaa154 < 28) {
            // 2026-07-27: distinguir COMPRA (item de TIENDA) vs VENTA (item del
            // inventario del jugador).  El pool de tienda es el overlay
            // Inventory[32..152].WalkSpeed (offset +24, stride 0x44).  Si el item
            // hovereado (param_3) cae en ese rango → precio de COMPRA
            // (GlobalText[62], modo 0 = valor completo).  Si no → precio de VENTA
            // (GlobalText[63], modo 1 = valor/3).  Antes SIEMPRE mostraba venta.
            unsigned char* shopLo = (unsigned char*)ShopItems;
            unsigned char* shopHi = (unsigned char*)ShopItems + 120 * 0x44;
            bool isBuy = ((unsigned char*)param_3 >= shopLo &&
                          (unsigned char*)param_3 <  shopHi);
            int price = FUN_0047c690((void*)param_3, isBuy ? 0 : 1);
            char priceStr[32];
            FormatThousands(priceStr, sizeof(priceStr), price);
            const char* gt = GlobalText[isBuy ? 62 : 63];
            char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
            if (gt && gt[0]) snprintf(dst, 100, gt, priceStr);
            else             snprintf(dst, 100, isBuy ? "Precio de compra: %s"
                                                      : "Precio de venta: %s", priceStr);
            DAT_07e91708[DAT_07eaa154] = 5;   // gold tint
            ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 0;
            DAT_07eaa154++;
        }

        AppendInventorySpecialTooltipLines(it);
        AppendInventoryDurabilityTooltipLines(it, p, level, attrBase);
        // ORDEN 2026-07-20: la línea "Puede ser equipado por X" va JUSTO DESPUÉS
        // de los requisitos, ANTES del bloque de opciones excellent.  Antes se
        // emitía última.  Verificado contra la salida del cliente de referencia
        // (mismo binario que tenemos en IDA): requisitos → clase → excellent.
        AppendInventoryRequirementTooltipLines(it);
        AppendInventoryRequireClassLines(p);
        AppendInventoryLateBonusTooltipLines(it, p);
        AppendInventorySpecialOptionLines(it, p);
    }

    // 2026-05-08: BUG-FIX — port previo populaba lpString_07e90798 pero
    // nunca llamaba al draw. Per IDA L2824:
    //   sub_4C2420(v308, sx, v309, TextNum, 0.0, 2, 1)
    // Donde sx/sy son las coords de pantalla (param_1/param_2 en nuestra sig).
    // FUN_004c2420 dibuja la lista de líneas en lpString_07e90798.
    {
        // AUDITORIA 2026-07-20 — REMOVIDO (2 sitios, uno por cada funcion de
        // render).  Anteponia una linea con GlobalText[2011] para los tipos
        // 480..482 (Scroll of Poison / Meteorite / Lightning), con un tinte
        // distinto por tipo (6/2/1).  El indice 2011 esta fuera de las 1000
        // filas del Text.bmd, asi que la linea salia VACIA pero igual
        // incrementaba el contador: el resultado visible era un renglon en
        // blanco en el tooltip de esos tres scrolls.
        // Los items son reales, pero la linea no puede ser del 0.97k — su texto
        // no existe en el archivo.  Se conserva la llamada a
        // RenderItemTooltipTextList, que es el fix real del 2026-05-08.
        int sxScreen = (int)(uintptr_t)param_1;
        int syScreen = (int)(uintptr_t)param_2;
        int itemHeightCells = ((ITEM_ATTRIBUTE*)(uintptr_t)attrBaseOK)[itemType].Height;
        if (itemHeightCells <= 0) itemHeightCells = 1;
        RenderItemTooltipTextList(sxScreen, syScreen, itemHeightCells, DAT_07eaa154);
    }
}

// FUN_004c8d70 @ 0x004C8D70 — RenderRepairInfo(param_1, param_2, ITEM* ip)    [Kayito: RenderRepairInfo]
// Shows item tooltip in the repair NPC context. unaff_EBX=DAT_07cf1ffc, unaff_ESI=1 (anti-tamper).
extern "C" void __cdecl FUN_004c8d70_impl(void* param_1, int param_2, void* param_3_v) // RenderRepairInfo
{
    // 2026-05-08: same defensive guards as FUN_004c4650 (sibling function).
    // Use the backup-aware accessor to recover DAT_07d78068 if clobbered.
    unsigned int attrBaseOK_ = ItemAttribute_Base();
    if (attrBaseOK_ == 0) return;
    if (param_3_v == nullptr || (uintptr_t)param_3_v < 0x100000) return;
    unsigned short* param_3 = (unsigned short*)param_3_v;
    short itemType = (short)*param_3;
    if (itemType < 0 || (unsigned short)itemType >= 1024) return;

    // Class-filter exclusions
    if (itemType > 0x19f && itemType < 0x1a4) return;
    if (itemType == 0x1aa) return;
    if (itemType == 0x87)  return;
    if (itemType == 0x8f)  return;
    if (itemType > 0x1bf)  return;
    if (itemType > 0x186 && itemType < 0x194) return;
    if (itemType > 0x1ad && itemType < 0x1b4) return;
    if (itemType == 0x1d5) return;
    if (IsRenderRepairInfoBlocked(itemType)) return;

    int attrBase = itemType * 0x40 + (int)attrBaseOK_;

    DAT_07eaa154 = 0;
    DAT_07eaa158 = 0;

    // BUG-FIX 2026-05-03: was `< 0x7e91350` (absolute end bound from source binary).
    for (int i = 0; i < 30; ++i)
        lpString_07e90798[i * 100] = 0;

    unsigned int level = (*(unsigned int*)(param_3 + 2) >> 3) & 0xf;

    // Quality tier
    unsigned int tier = 0;
    if (itemType == 0x1cd || itemType == 0x1ce || itemType == 399 ||
        itemType == 0x1d1 || itemType == 0x1d2 || itemType == 0x1d3) {
        tier = 3;
    } else if (itemType == 0xaa || itemType == 0x13 || itemType == 0x92) {
        tier = 6;
    } else {
        char opt1      = (char)param_3[0x12];
        unsigned char optFlags = *(unsigned char*)((char*)param_3 + 0x1b) & 0x3f;
        if (opt1 != '\0' && optFlags != 0) {
            tier = 4;
        } else {
            tier = (level > 6) ? 3u : (opt1 != '\0' ? 1u : 0u);
        }
    }
    if (itemType > 0x182 && itemType < 0x187) {
        tier = (level < 7) ? (unsigned int)((char)param_3[0x12] != '\0') : 3;
    }

    // Slot 0: name
    crt_sprintf(lpString_07e90798, DAT_0055a5f4);
    DAT_07eaa154++;
    DAT_07eaa158++;

    // Durability
    unsigned int maxDur = FUN_004c45c0(param_3, attrBase, (int)level) & 0xffff;
    unsigned int curDur = (unsigned int)*(unsigned char*)((char*)param_3 + 0x1a);
    if (curDur < maxDur) {
        // 2026-05-08: REMOVED self-perpetuating cursor flag. The original
        // IDA code wrote DAT_07eaa134 = 2 here, but that turns the mouse
        // cursor into a repair sprite (per FUN_004bffa0), and since this
        // function only runs when DAT_07eaa134 != 0, it self-locks the
        // cursor every frame. The actual repair NPC context sets
        // DAT_07eaa134 from elsewhere (Chat_InputTick B-key, NPC checkbox).
        // DAT_07eaa134 = 2;
        // BUG-FIX 2026-04-26 (audit #3): same ItemValue/ConvertRepairGold pair.
        int gold = FUN_0047c690((void*)param_3, 2);
        FUN_004c3ef0(gold, (int)curDur, (int)maxDur, (short)itemType, lpString_07e90798 + 64);
    } else {
        // DAT_07eaa134 = 1;  // Same — REMOVED.
    }

    // Slot 1: level string
    {
        ITEM* it = (ITEM*)param_3;
        ITEM_ATTRIBUTE* p = (ITEM_ATTRIBUTE*)(uintptr_t)attrBase;
        char* dst = lpString_07e90798 + DAT_07eaa154 * 100;
        if (!BuildInventorySpecialNameLine(it, p, level, dst, 100)) {
            crt_sprintf(dst, DAT_07d3b40c);
        }
    }
    {
        int specialNameColor = GetInventorySpecialNameColor((ITEM*)param_3);
        DAT_07e91708[DAT_07eaa154] = specialNameColor;
    }
    ((int*)&DAT_07ea7b10)[DAT_07eaa154] = 1;
    DAT_07eaa154++;

    // Slot 2: class/subtype
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, DAT_0055a5fc);
    DAT_07eaa154++;
    DAT_07eaa158++;

    AppendInventorySpecialTooltipLines((ITEM*)param_3);
    AppendInventoryDurabilityTooltipLines((ITEM*)param_3, (ITEM_ATTRIBUTE*)(uintptr_t)attrBase, level, attrBase);
    AppendInventoryRequirementTooltipLines((ITEM*)param_3);
    AppendInventoryRequireClassLines((ITEM_ATTRIBUTE*)(uintptr_t)attrBase);   // ver nota de orden arriba
    AppendInventoryLateBonusTooltipLines((ITEM*)param_3, (ITEM_ATTRIBUTE*)(uintptr_t)attrBase);
    AppendInventorySpecialOptionLines((ITEM*)param_3, (ITEM_ATTRIBUTE*)(uintptr_t)attrBase);

    // 2026-05-08: BUG-FIX — port previo populaba lpString_07e90798 pero
    // nunca llamaba al draw. Misma fix que FUN_004c4650.
    {
        // AUDITORIA 2026-07-20 — REMOVIDO (2 sitios, uno por cada funcion de
        // render).  Anteponia una linea con GlobalText[2011] para los tipos
        // 480..482 (Scroll of Poison / Meteorite / Lightning), con un tinte
        // distinto por tipo (6/2/1).  El indice 2011 esta fuera de las 1000
        // filas del Text.bmd, asi que la linea salia VACIA pero igual
        // incrementaba el contador: el resultado visible era un renglon en
        // blanco en el tooltip de esos tres scrolls.
        // Los items son reales, pero la linea no puede ser del 0.97k — su texto
        // no existe en el archivo.  Se conserva la llamada a
        // RenderItemTooltipTextList, que es el fix real del 2026-05-08.
        int sxScreen = (int)(uintptr_t)param_1;
        int syScreen = param_2;
        int itemHeightCells = ((ITEM_ATTRIBUTE*)(uintptr_t)attrBaseOK_)[itemType].Height;
        if (itemHeightCells <= 0) itemHeightCells = 1;
        RenderItemTooltipTextList(sxScreen, syScreen, itemHeightCells, DAT_07eaa154);
    }
}
