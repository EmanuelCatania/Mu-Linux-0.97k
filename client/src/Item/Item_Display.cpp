// Item_Display.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetMixItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE Inventory[];
extern "C" int dword_7EAA0C8;
extern "C" int dword_7EAA0CC;

static BYTE InventoryPoolToMoveFlag(const BYTE* poolBase) {
    if (poolBase == &OffsetTradeItems[0] || poolBase == &Inventory[0]) return 1;
    if (poolBase == &OffsetWarehouseItems[0]) return 2;
    if (poolBase == &OffsetMixItems[0]) return 3;
    return 0;
}

static void InventoryMove_SetPendingPools(const BYTE* sourcePoolBase,
                                          const BYTE* targetPoolBase) {
    g_ItemMoveSourcePool = (DWORD)(uintptr_t)sourcePoolBase;
    g_ItemMoveTargetPool = (DWORD)(uintptr_t)targetPoolBase;
}

// ConvertGold64 @ 0x004C3E10 (~23 lines) — formats gold with thousand separators
int __cdecl ConvertGold64_stub(int Zen, char* Buffer) {
    if (Zen > 999999999)
        return sprintf(Buffer, "%d,%03d,%03d,%03d", Zen / 1000000000, (Zen / 1000000) % 1000, (Zen / 1000) % 1000, Zen % 1000);
    if (Zen > 999999)
        return sprintf(Buffer, "%d,%03d,%03d", Zen / 1000000, (Zen / 1000) % 1000, Zen % 1000);
    if (Zen > 999)
        return sprintf(Buffer, "%d,%03d", Zen / 1000, Zen % 1000);
    return sprintf(Buffer, "%d", Zen);
}

// ─────────────────────────────────────────────────────────────────────────────
// RenderItemName @ 0x004C9E70 — nombre flotante de un item del suelo
// ─────────────────────────────────────────────────────────────────────────────
// Port 1:1 del decompile (mu97k-src-IDA/raw/004C9E70_RenderItemName.c).  Lo
// llama sub_4CB6F0 (Target_Render): para el item bajo el cursor con Sort=0 y
// para todos los del suelo con Sort=1 mientras Alt esté activo.
//
//   o          = &Items[i][72]  → o+2 = índice de modelo (tipo + 400)
//   ItemLevel  = Items+8        → para el Zen (modelo 863) es la CANTIDAD
//   ItemOption = Items+31
//
// 2026-08-21: antes era un resumen escrito a ojo.  Divergencias que tenía y que
// este port corrige:
//   · Zen (863): hacía `sprintf(buf, DAT_0055a608, name)` con DAT_0055a608 = ""
//     → cadena vacía, o sea el Zen del suelo no mostraba NADA.  IDA es
//     `sprintf(String, "%s %d", name, ItemLevel)` = "Zen <cantidad>".
//   · Los colores de nivel 3-4 y de la rama (v5 & 0x87) estaban invertidos en
//     RGB (IDA llena v38[2],v38[1],v38[0] y llama glColor3f(v38[0],v38[1],v38[2])).
//   · Faltaban por completo las ramas 860 (Event), 831 (alas), 951-958
//     (flechas/bolts), 795 (pergamino de skill), 826 (piedra de invocación) y
//     los sufijos Excellent/Luck/Skill (GlobalText[176..179]).
//
// Ruido anti-tamper omitido por policy: las ramas 795 y 826 del binario están
// envueltas en lookups de hash-table (ref-count + XOR sobre la entrada de
// SkillAttribute).  Con nuestra tabla en claro el resultado es el mismo.
//
// Desviación: donde IDA hace `sprintf(String, GlobalText[N])` (una cadena de
// datos usada como formato) nosotros usamos `sprintf(String, "%s", GlobalText[N])`.
void __cdecl RenderItemName_stub(int i, DWORD o, int ItemLevel, int ItemOption, bool Sort) {
    (void)i;

    const int v5 = ItemLevel;
    const int v6 = (ItemLevel >> 3) & 0xF;
    char  String[52];
    float v38[3];
    String[0] = '\0';

    SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);   // g_hFont
    const short v7 = *(short*)(o + 2);

    // &ItemAttribute[v7 - 400] — ITEM_ATTRIBUTE stride 0x40, Name en offset 0.
    // Guard defensivo (no está en IDA): DAT_07d78068 se corrompe a valores
    // chicos en algunos caminos, igual que en los guards del tooltip.
    const char* name = "";
    {
        int attrIdx = (int)v7 - 400;
        unsigned int abase = (unsigned int)(uintptr_t)DAT_07d78068;
        if (abase >= 0x100000u && abase < 0x80000000u && attrIdx >= 0 && attrIdx < 1024)
            name = (const char*)(uintptr_t)(abase + (unsigned int)attrIdx * 64u);
    }

    switch (v7) {
    case 863:                                   // Zen / dinero
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s %d", name, v5);
        goto renderLabel;

    case 861: case 862: case 864:               // joyas
    case 799: case 870: case 830:
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", name);
        goto renderLabel;

    case 865: case 866: case 867:               // Devil (Chaos) items
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        glColor3f(1.0f, 0.8f, 0.1f);
        if ((v5 & 0x78) != 0) sprintf(String, "%s +%d", name, v6);
        else                  sprintf(String, "%s", name);
        goto renderLabel;
    }

    if (v7 == 859 && v6 == 7) {
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", GlobalText[111]);
        goto renderLabel;
    }

    switch (v7) {
    case 860:                                   // Event item
        if (v6 == 0)      sprintf(String, "%s", GlobalText[100]);
        else if (v6 == 1) sprintf(String, "%s", GlobalText[101]);
        else if (v6 == 2) sprintf(String, "%s", GlobalText[104]);
        goto renderLabel;

    case 831:                                   // alas: prefijo por sub-tipo
        glColor3f(1.0f, 0.8f, 0.1f);
        switch (v6) {
        case 0: sprintf(String, "%s %s", GlobalText[168], name); break;
        case 1: sprintf(String, "%s %s", GlobalText[169], name); break;
        case 2: sprintf(String, "%s %s", GlobalText[167], name); break;
        case 3: sprintf(String, "%s %s", GlobalText[166], name); break;
        default: break;
        }
        goto renderLabel;

    case 951: sprintf(String, "%s", GlobalText[105]); goto renderLabel;
    case 952: sprintf(String, "%s", GlobalText[106]); goto renderLabel;
    case 953: sprintf(String, "%s", GlobalText[107]); goto renderLabel;
    case 954: sprintf(String, "%s", GlobalText[108]); goto renderLabel;
    case 955: sprintf(String, "%s", GlobalText[109]); goto renderLabel;
    case 956: sprintf(String, "%s", GlobalText[110]); goto renderLabel;
    case 957: sprintf(String, "%s +%d", GlobalText[115], v6 - 7); goto renderLabel;
    case 958:
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", GlobalText[810]);
        goto renderLabel;

    case 795: {                                 // pergamino de skill
        // IDA: v45 = 8 * (5 * v6 + 150); sprintf("%s %s", &SkillAttribute[v45],
        // GlobalText[102]).  Las entradas de SkillAttribute son de 40 bytes
        // (WinMain reserva 0xA00 = 64 entradas y el anti-tamper copia de a
        // 0x28), así que el índice es 40 * (v6 + 30) y el nombre va en offset 0.
        int v45 = 8 * (5 * v6 + 150);
        const char* skillName = (const char*)((const char*)&SkillAttribute + v45);
        sprintf(String, "%s %s", skillName, GlobalText[102]);
        goto renderLabel;
    }
    }

    if (v7 != 826) {
        if (v7 == 570 || v7 == 419 || v7 == 546) {
            v38[2] = 1.0f;
            v38[1] = 0.1f;
        } else {
            if ((ItemOption & 0x3F) != 0 && (v7 < 787 || v7 > 790)) {
                v38[2] = 0.5f; v38[1] = 1.0f; v38[0] = 0.1f;
                goto LABEL_118;
            }
            if (v6 < 7) {
                if ((v5 & 0x87) != 0) goto LABEL_115;
                if (v6 == 0) {
                    glColor3f(0.7f, 0.7f, 0.7f);
                    goto LABEL_119;
                }
                if (v6 < 3) {
                    v38[0] = 0.9f; v38[1] = 0.9f; v38[2] = 0.9f;
                    goto LABEL_118;
                }
                if (v6 >= 5) {
LABEL_115:
                    v38[2] = 1.0f; v38[1] = 0.7f; v38[0] = 0.4f;
                    goto LABEL_118;
                }
                v38[2] = 0.2f; v38[1] = 0.5f;
            } else {
                v38[2] = 0.1f; v38[1] = 0.8f;
            }
        }
        v38[0] = 1.0f;
LABEL_118:
        glColor3f(v38[0], v38[1], v38[2]);
        if (v6 != 0) {
            sprintf(String, "%s +%d", name, v6);
            goto LABEL_121;
        }
LABEL_119:
        sprintf(String, "%s", name);
LABEL_121:
        if ((v5 & 0x80) != 0) {
            // IDA escribe " +" (word_55A6B0) sobre el NUL y luego concatena
            // GlobalText[179] para el tipo 819; el resto usa GlobalText[176].
            if (v7 == 819) {
                strcat(String, " +");
                strcat(String, GlobalText[179]);
            } else {
                strcat(String, GlobalText[176]);
            }
        }
        if ((v5 & 3) != 0 || (ItemOption & 0x40) != 0) strcat(String, GlobalText[177]);
        if ((v5 & 4) != 0)                             strcat(String, GlobalText[178]);
        goto renderLabel;
    }

    {   // v7 == 826: piedra de invocación — nombre del monstruo + GlobalText[103]
        static const int SommonTable[6] = { 2, 7, 14, 8, 9, 41 };
        if (v6 < 6) {
            const MONSTER_SCRIPT* ms = (const MONSTER_SCRIPT*)&MonsterScript;
            int target = SommonTable[v6];
            for (int m = 0; m < MAX_MONSTER; ++m) {
                if (ms[m].Type == target) {
                    sprintf(String, "%s %s", ms[m].Name, GlobalText[103]);
                    break;
                }
            }
        }
    }

renderLabel:
    {
        int  textLen = lstrlenA(String);
        SIZE sz;
        GetTextExtentPointA(m_hFontDC, String, textLen, &sz);
        if (Sort) {
            // Centrado sobre la posición de pantalla del item (o+0x5c / o+0x5e)
            int x = (int)*(short*)(o + 0x5c) - 640 * (sz.cx / 2) / (int)WindowWidth;
            int y = (int)*(short*)(o + 0x5e) - 15;
            RenderText(x, y, String, 0, 0, (SIZE*)3);
        } else {
            int x = (int)MouseX - 640 * (sz.cx / 2) / (int)WindowWidth;
            int y = (int)MouseY - 15;
            RenderText(x, y, String, 0, 0, (SIZE*)3);
        }
    }
}

unsigned int __stdcall Inventory_DropItemEx(int origin_x, int origin_y,
                                            BYTE* invBase, int gridW,
                                            int gridH, int slotType);

unsigned int __stdcall Inventory_DropItem_stub(void) {
    BYTE* invBase = (BYTE*)(uintptr_t)DAT_07ea9800;
    int gridW = 8;
    int gridH = (invBase == &OffsetMixItems[0]) ? 4 : 8;
    int originX = 0;
    int originY = 0;

    if (invBase == &OffsetInventoryItems[0]) {
        originX = InventoryStartX + 15;
        originY = InventoryStartY + 200;
    } else if (invBase == &OffsetTradeItems[0]) {
        originX = TradeInventoryStartX + 15;
        originY = TradeInventoryStartY + 270;
        gridH = 4;
    } else if (invBase == &Inventory[0]) {
        originX = TradeInventoryStartX + 15;
        originY = TradeInventoryStartY + 70;
        gridH = 4;
    } else if (invBase == &OffsetWarehouseItems[0]) {
        originX = dword_7EAA0C8 + 15;
        originY = dword_7EAA0CC + 50;
        gridH = 15;
    } else if (invBase == &OffsetMixItems[0]) {
        originX = dword_7EAA0C8 + 15;
        originY = dword_7EAA0CC + 110;
        gridH = 4;
    }

    return Inventory_DropItemEx(originX, originY, invBase, gridW, gridH, 0);
}

unsigned int __stdcall Inventory_DropItemEx(int origin_x, int origin_y,
                                            BYTE* invBase, int gridW,
                                            int gridH, int slotType) {
    bool actionTaken = false;
    // 0x004D6470 — Giant item drag-and-drop handler (~3011 lines decompiled).
    // Called when player releases mouse on inventory/equipment grid.
    //
    // Anti-tamper notes (all skipped in this implementation):
    //   - ~22 unreachable blocks (dead code removed by Ghidra)
    //   - ~63 phantom stack params (in_stack_00006cec..in_stack_00009d10) — fake anti-tamper,
    //     not real args. The real "params" are caller locals read via stack overlap in the
    //     original binary. Our re-implementation reads them as globals instead.
    //   - Repeated XOR key init (0x893a6de7/0x739fb2bc/0xb6fea823) + single-iteration loops
    //     around every packet byte — compiler artifact / anti-tamper padding.
    //   - Hash table operations (FUN_00403f80, FUN_004041e0, FUN_004042b0, FUN_00404280,
    //     FUN_00404330, FUN_00404400) interspersed around every packet send — anti-tamper
    //     hash table, skipped.
    //
    // The caller (FUN_004df410 @ 0x004DF410) calls this up to 4 times per frame,
    // once for each open inventory context:
    //   1. Main inventory (OffsetInventoryItems, grid 8x8, slotType=0)
    //   2. Trade window    (TradeOpened, same grid)
    //   3. Warehouse       (WarehouseOpened, same grid)
    //   4. Chaos mix       (ChaosMixOpened + MixState, grid 8x4)
    //
    // In the original binary, context params (inventoryBase, gridWidth, gridHeight, slotType,
    // gridOffsetX, gridOffsetY) are passed via stack-frame overlap. In this reimplementation
    // we read the relevant globals directly, matching the caller's setup pattern.
    //
    // Return value: low byte used as bool success flag by caller.

    // --- Alias Ghidra globals to readable names ---
    // pPickedItem       = DAT_07e91350 (short, type of item being dragged)
    // ItemAttribute     = DAT_07d78068 (ITEM_ATTRIBUTE*, stride 0x40)
    // OffsetInventoryItems = DAT_07ea8410 (inventory item array base)
    // OffsetWarehouseItems = DAT_07ea5b30 (warehouse item array base, char type in globals)
    // OffsetMixItems    = DAT_07ea9848 (chaos mix item array base)
    // MixState          = DAT_07eaa140 (chaos mix active flag)
    // EquipmentItem     = DAT_07eaa165 (equipment move in-progress guard)
    // InventoryOpened   = (extern) InventoryOpened
    // WarehouseOpened   = DAT_07eaa119
    // TradeOpened       = DAT_07eaa11b
    // MouseX            = DAT_083a427c
    // MouseY            = DAT_083a4278
    // MouseLButtonPush  = DAT_083a4124
    // DAT_083a42eb      = auto-drop trigger flag
    // DAT_07e11e78      = target inventory slot index
    // DAT_07ea5b18      = source inventory slot index (picked-from)
    // DAT_07e91354      = picked item level (Level)
    // DAT_07e9136a      = picked item durability/option byte
    // DAT_07e9136b      = picked item extra dur byte
    // DAT_07ea9800      = current inventory context pointer
    // DAT_07ea8414      = inventory item level array (offset +4 from base)
    // EnableUse         = DAT_05826d1c (extern EnableUse)

    // 2026-05-09: ItemAttribute base watchdog. Mirror of the one in
    // FUN_004d23b0. Some unknown writer sets DAT_07d78068 = 0x00000001 → all
    // attr-based reads (CheckInventorySpace_stub itemW/H, attr[type] in this
    // function) compute bogus values, making spaceFree always 0 and breaking
    // drop on actually-empty slots. Restore from backup if corrupt.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

    short pickedType = *(short*)DAT_07e91350;
    ITEM_ATTRIBUTE* pAttr = (ITEM_ATTRIBUTE*)DAT_07d78068;
    if (!pAttr || pickedType < 0) return 0;
    int itemWidth  = (int)pAttr[pickedType].Width;
    int itemHeight = (int)pAttr[pickedType].Height;
    if (itemWidth <= 0 || itemHeight <= 0) return 0;

    // Use the args directly (FIX 2026-05-08).
    BYTE* sourceInvBase = (BYTE*)(uintptr_t)DAT_07ea9800;
    BYTE  sourceMoveFlag = InventoryPoolToMoveFlag(sourceInvBase);
    BYTE  targetMoveFlag = InventoryPoolToMoveFlag(invBase);
    int   gridWidth  = gridW;
    int   gridHeight = gridH;

    // 2026-08-08 FIX "mover items DENTRO del baul los hacia desaparecer":
    // aca habia un `DAT_07ea9800 = invBase` ("update para downstream readers")
    // que es una INVENCION del port — IDA sub_4D6470 SOLO LEE dword_7EA9800,
    // nunca lo escribe (los unicos writers son sub_4D23B0 L798/L1401 y
    // Player_InputTick L711, todos en el PICKUP). dword_7EA9800 es el pool de
    // ORIGEN del item agarrado, y el dispatcher sub_4DF410 llama a esta funcion
    // hasta 4 veces por frame (main inv, trade, baul, mix). La primera llamada
    // (main inv) pisaba el origen con OffsetInventoryItems, asi que en la
    // llamada del baul `sourceMoveFlag` salia 0 (=inventario) en vez de 2
    // (=baul) -> el server recibia SourceFlag=0 con SourceSlot=101 (fuera del
    // rango de inventario) -> INVENTORY_RANGE falla -> result=0xFF y el item
    // quedaba solo borrado localmente = "desaparecio".
    // Sintoma cruzado en el log: baul->inventario (resuelto en la 1er llamada,
    // antes del clobber) SI mandaba srcF=2 y funcionaba.

    // ── Mouse-to-grid conversion (per IDA L595-597) ─────────────────────────
    //   gridX = (MouseX - origin_x) * 0.05 - itemW * 0.5 + 0.5  →  __ftol
    //   gridY = (MouseY - origin_y) * 0.05 - itemH * 0.5 + 0.5
    // The 0.05 factor = 1/20 (cells are 20 px wide). The half-width offset
    // centers the multi-cell footprint on the cursor.
    int mouseGridX = (int)(((double)((int)DAT_083a427c - origin_x)) * 0.05
                            - (double)itemWidth  * 0.5 + 0.5);
    int mouseGridY = (int)(((double)((int)DAT_083a4278 - origin_y)) * 0.05
                            - (double)itemHeight * 0.5 + 0.5);

    // Bounds check: negative grid coords = outside the grid (mouse left/up
    // of origin).
    if (mouseGridX < 0 || mouseGridY < 0) {
        return 0;
    }

    bool cursorInsideGrid = (mouseGridX < gridWidth && mouseGridY < gridHeight);

    // (Pool aliases declared at file scope above.)
    // --- Check if target is MixItems with MixState active ---
    bool spaceFree = false;
    if ((BYTE*)invBase == &OffsetMixItems[0] && DAT_07eaa140 != 0) {
        // Mix placement: space is always considered free when mix state active
        spaceFree = true;
    } else {
        // Call CheckInventorySpace to validate placement.
        // 2026-05-09 BUG-FIX: ANTES pasábamos `mouseGridX, mouseGridY` (= grid
        // coords ya calculadas como 0..7) como p1, p2. Pero la función espera
        // SCREEN OFFSETS (origin_x, origin_y) para hacer la conversión interna
        // mouseX-p1 → relative pixel → grid. Pasar grid coords daba
        // gridX = (MouseX - 1)*0.05 ≈ 30 → fuera del grid → emptyCount=0 →
        // spaceFree=0 SIEMPRE. Esto es por qué el drop nunca encontraba slots
        // libres aún con el watchdog de attr.
        unsigned long long result = CheckInventorySpace_stub(
            origin_x, origin_y,
            (unsigned short*)invBase,
            gridWidth, gridHeight
        );
        spaceFree = (char)result != 0;
    }

    // --- Mark grid cells with color state ---
    // Iterate over the item's width x height footprint
    int endY = mouseGridY + itemHeight;
    int endX = mouseGridX + itemWidth;
    for (int gy = mouseGridY; gy < endY; gy++) {
        for (int gx = mouseGridX; gx < endX; gx++) {
            if (gx >= 0 && gy >= 0 && gx < gridWidth && gy < gridHeight) {
                int cellIdx = gy * gridWidth + gx;
                BYTE* cellBase = (BYTE*)(invBase + cellIdx * 0x44);  // stride 0x22 words = 0x44 bytes
                if (!spaceFree) {
                    // Overlap: mark red
                    cellBase[0x40] = 2;  // +0x20 in short units = +0x40 bytes: colorState
                } else {
                    // Check if currency item (Jewel of Bless/Soul/Life)
                    if (pickedType == 0x1cd || pickedType == 0x1ce || pickedType == 0x1d0) {
                        cellBase[0x40] = 4;  // currency highlight
                    } else {
                        cellBase[0x40] = 3;  // normal placement ok
                    }
                }
            }
        }
    }

    // --- Check for mouse click to confirm drop ---
    if (DAT_083a42eb == '\0' && DAT_083a4124 == 0) {
        // Neither auto-drop trigger nor mouse button pressed: just return (grid preview only)
        return 0;
    }

    // === MOUSE BUTTON PRESSED OR AUTO-DROP: Execute the drop ===

    if (!spaceFree) {
        // --- Space check failed: target slot is occupied ───────────────────
        // (Currency stacking / "can't place" UI message only.)
        // Check for currency item stacking on existing inventory item
        bool canStack = false;
        if (cursorInsideGrid
            && (pickedType == 0x1cd || pickedType == 0x1ce || pickedType == 0x1d0)
            && slotType == 0 && InventoryOpened != 0)
        {
            int targetSlot = mouseGridY * gridWidth + mouseGridX;
            short targetType = *(short*)(invBase + targetSlot * 0x44);
            int targetLevel = *(int*)(invBase + targetSlot * 0x44 + 4) >> 3 & 0xf;
            bool validTarget = (targetType < 0x187);

            // Jewels 0x87 / 0x8f cannot receive upgrade
            if (targetType == 0x87 || targetType == 0x8f) {
                validTarget = false;
            }

            // Level cap per currency type
            if (pickedType == 0x1cd && targetLevel > 5) {
                // Jewel of Bless: max +5
                canStack = false;
                goto drop_done;
            }
            if (pickedType == 0x1ce && targetLevel > 8) {
                // Jewel of Soul: max +8
                canStack = false;
                goto drop_done;
            }

            if (validTarget) {
                // Record which slot the currency is being applied to
                DAT_07e11e78 = (DWORD)(
                    (DWORD)*(invBase + targetSlot * 0x44 + 0x3f) * gridWidth + 0xc
                    + (DWORD)*(invBase + targetSlot * 0x44 + 0x3e)
                );

                // Con el baul o el trade abiertos no se puede aplicar la
                // jewel: IDA salta a LABEL_807, que muestra el mensaje. Por eso
                // `canStack` queda en false en esos dos casos (antes se ponia
                // en true al final incondicionalmente y el aviso no salia).
                if (DAT_07eaa119 == '\0' && DAT_07eaa11b == '\0') {
                    // 2026-08-24 FIX (issue #15, "las jewels no se consumen"):
                    // aca se mandaba `SendRequestEquipmentItem_stub`, o sea
                    // 0x24 PMSG_ITEM_MOVE_RECV (11 bytes). El server trata eso
                    // como MOVER la jewel a una celda ocupada -> lo rechaza y
                    // el cliente la devuelve al inventario. IDA (sub_4D6470
                    // L5919-5931) manda 0x26 PMSG_ITEM_USE_RECV, que es el que
                    // dispara CharacterUseJewelOfBles/Soul/Life en el server
                    // (ItemManager.cpp:2753+) y contesta con GCItemDeleteSend +
                    // GCItemModifySend (F3/14).
                    //
                    //   struct PMSG_ITEM_USE_RECV {   // ItemManager.h:49
                    //       PBMSG_HEAD header;        // C1 : 5 : 0x26
                    //       BYTE SourceSlot;          // +3
                    //       BYTE TargetSlot;          // +4
                    //   };
                    //
                    // Va por Net_SendSmallPacket porque HackPacketCheck.txt da
                    // Encrypt=1 para el indice 38 -> el frame final tiene que
                    // ser C3 con serial. El 0xC1 que arma IDA es el texto plano
                    // previo al encriptador, no el frame que viaja.
                    if ((int)EnableUse < 1) {
                        // IDA: `if (EnableUse > 0) goto LABEL_808;` — durante el
                        // cooldown NO se manda nada. Antes se mandaba igual.
                        EnableUse = 10;

                        BYTE pkt[8];
                        memset(pkt, 0, sizeof(pkt));
                        pkt[0] = 0xC1;
                        pkt[1] = 5;                          // lo pisa el serial
                        pkt[2] = 0x26;                       // ItemUse
                        pkt[3] = (BYTE)(int)DAT_07ea5b18;    // SourceSlot (la jewel)
                        pkt[4] = (BYTE)(int)DAT_07e11e78;    // TargetSlot (el item)
                        Net_SendSmallPacket(pkt, 5);
                        actionTaken = true;

                        // Sonido segun el item de origen (IDA LABEL_802).
                        short sndItem = *(short*)((char*)OffsetInventoryItems + ((int)DAT_07ea5b18 - 0xc) * 0x44);
                        if (sndItem == 0x1c0) {
                            PlayBuffer(0x21, 0, 0);
                        } else if (sndItem > 0x1c0 && sndItem < 0x1ca) {
                            PlayBuffer(0x20, 0, 0);
                        }
                    }
                    canStack = true;
                }
            }
        }
        if (!canStack) {
            // Show "can't place here" message
            UIChatLogWindow_AddText((const char*)&DAT_07eaa190, (const char*)&DAT_07d4c89c, 2);
        }
        goto drop_done;
    }

    // === Space is free: handle the actual item placement ===

    if (slotType != 0) {
        // --- Equipment slot drop (slotType == 2 or 3) ---
        DAT_07e11e78 = (DWORD)(mouseGridY * gridWidth + mouseGridX);

        if (DAT_07eaa165 != '\0') goto drop_done;  // equipment move already in progress

        // 2026-07-27 PORT FALTANTE (no se podian meter items al baul): este
        // camino (slotType != 0 = baul/trade/mix) SOLO tenia implementado el
        // destino "inventario principal"; para el resto habia un comentario
        // "follow same pattern" y NADA de codigo -> nunca se mandaba el 0x24,
        // actionTaken quedaba en false -> el dispatcher veia dropWH=0 y trataba
        // el drop como "tirar al suelo" (mensaje rojo), y el item quedaba en un
        // estado inconsistente (desaparecia al recargar). El envio es identico
        // para todos los destinos: solo cambia targetMoveFlag (0=inv, 1=trade,
        // 2=warehouse, 3=chaos), que ya viene calculado arriba.
        if (DAT_07eaa165 == 0) {
            DAT_07eaa165 = 1;
            InventoryMove_SetPendingPools(sourceInvBase, invBase);
            SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
                (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
            actionTaken = true;
        }
        goto drop_done;
    }

    // --- slotType == 0: inventory-to-inventory placement ---
    if (InventoryOpened == 0) goto drop_done;

    // Calculate target slot in inventory grid (offset by 0xc for equipment slots 0-11)
    DAT_07e11e78 = (DWORD)(mouseGridY * gridWidth + 0xc + mouseGridX);

    if (invBase == &OffsetInventoryItems[0]) {
        // --- Target is main inventory (OffsetInventoryItems) ---
        bool isDifferentSlot = (DAT_07ea5b18 != DAT_07e11e78);
        int targetOffset = ((int)DAT_07e11e78 - 0xc) * 0x44;
        short targetType = *(short*)((char*)OffsetInventoryItems + targetOffset);

        // Check for stackable items (same type swap)
        if (pickedType == targetType) {
            // Arrows: types 0x1c0..0x1c8 can stack together
            if (pickedType > 0x1bf && pickedType < 0x1c9
                && targetType > 0x1bf && targetType < 0x1c9)
            {
                isDifferentSlot = true;  // force swap even if same slot
            }
            // Potions 0x87/0x8f: can stack only if same level
            if ((pickedType == 0x87 || pickedType == 0x8f)
                && (targetType == 0x87 || targetType == 0x8f))
            {
                int pickedLevel = ((int)DAT_07e91354 >> 3) & 0xf;
                int targetLevel = (*(int*)((char*)OffsetInventoryItems + targetOffset + 4) >> 3) & 0xf;
                if (pickedLevel != targetLevel) {
                    isDifferentSlot = true;  // different level = treat as different
                }
                // same level same type same slot = no-op, fall through
            }
        } else {
            // Different types: if same slot, call inventory reset
            if (!isDifferentSlot) {
                FUN_004cd3b0();  // Inventory_Reset / CharPreview_Refresh
                goto drop_done;
            }
        }

        // Guard: only one equipment move at a time
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped (XOR packet build + serial insertion)
        // Send the equipment swap request
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }
    else if (invBase == &OffsetMixItems[0]) {
        // --- Target is Chaos Mix inventory (OffsetMixItems) ---
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }
    else if (invBase == &OffsetWarehouseItems[0]) {
        // --- Target is Warehouse (OffsetWarehouseItems) ---
        // Check warehouse password verification state
        if (DAT_00559f5f == '\0' || DAT_07eaa148 != '\0') {
            if (DAT_07eaa165 != '\0') goto drop_done;
            DAT_07eaa165 = '\x01';

            // anti-tamper hash table — skipped
            InventoryMove_SetPendingPools(sourceInvBase, invBase);
            SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
                (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
            actionTaken = true;
        }
        else {
            // Warehouse needs password: set up second-password state machine
            DAT_07ea9808 = DAT_07ea5b18;
            DAT_07ea9804 = 2;
            DAT_07ea980c = 0;
            DAT_07ea9810 = DAT_07e11e78;

            // Initialize shuffle array for second-password input (10 digits, shuffled)
            short* shuffleArr = (short*)&DAT_07e91394;
            for (short i = 0; i < 10; i++) {
                shuffleArr[i] = i;
            }
            // Fisher-Yates shuffle (20 iterations)
            for (int n = 0; n < 20; n++) {
                int a = rand() % 10;
                int b = rand() % 10;
                if (a != b) {
                    // XOR swap
                    shuffleArr[a] ^= shuffleArr[b];
                    shuffleArr[b] ^= shuffleArr[a];
                    shuffleArr[a] ^= shuffleArr[b];
                }
            }
            DAT_07eaa14c = 1;
            DAT_07ea9814 = 0;
            DAT_07ea9818 = 0;
            DAT_07ea981c = 0;
            DAT_07ea981e = 0;
            actionTaken = true;
        }
    }
    else {
        // --- Other target context (Trade items, etc.) ---
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped
        // Build generic equipment move packet:
        // Packet fields: [C1][len][0x24][01][srcSlot][pPickedItem][Level][durOption][durExtra][slotType=3][dstSlot]
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }

drop_done:
    return actionTaken ? 1u : 0u;
}
