// Item_InventoryGrid.cpp
//
// Extracted from stubs_game.cpp.  This module owns the item-grid operations
// used by inventory, warehouse, trade, and chaos-mix panels.
//
// Every entry point retains its original IDA symbol/address in its leading
// comment.  No 5.2 logic was imported during this extraction.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// InsertWarehouseItem @ 0x004CC0E0 (~52 lines) — fills warehouse grid cells for an item
// ConvertItemType(param_2) -> get Width/Height from ItemAttribute[type].
// For each cell in WxH: set item type, grid coords, call ItemConvert.
// Stride 0x44 per cell at OffsetWarehouseItems base (DAT_07ea5b30).
// ItemAttribute = DAT_07d78068 (ITEM_ATTRIBUTE array, stride 0x40).
void __cdecl InsertWarehouseItem(unsigned int param_1, unsigned char* param_2) {
    // ConvertItemType: item_type = param_2[0] + (param_2[3] & 0x80) * 2
    int iVar3 = (unsigned int)param_2[0] + (param_2[3] & 0x80) * 2;
    ITEM_ATTRIBUTE* pAttr = (ITEM_ATTRIBUTE*)DAT_07d78068;

    if (iVar3 == 0xff) return;

    int iVar5 = 0;
    if (pAttr[iVar3].Height != '\0') {
        do {
            int iVar6 = 0;
            if (pAttr[iVar3].Width != '\0') {
                // Grid column of item origin (param_1 % 8)
                unsigned int uVar4 = param_1 & 0x80000007;
                if ((int)uVar4 < 0) {
                    uVar4 = (uVar4 - 1 | 0xfffffff8) + 1;
                }

                do {
                    int iVar1 = (param_1 + iVar5 * 8 + iVar6) * 0x44;
                    char* warehouseBase = &DAT_07ea5b30;

                    // Set item type
                    *(short*)(warehouseBase + iVar1) = (short)iVar3;
                    // Grid X origin
                    *(warehouseBase + 0x3e + iVar1) = (char)uVar4;
                    // Grid Y origin (param_1 / 8)
                    *(warehouseBase + 0x3f + iVar1) = (char)((int)(param_1 + ((int)param_1 >> 0x1f & 7U)) >> 3);
                    // Copy Attribute1 and Attribute2 from packet
                    *(warehouseBase + 0x1a + iVar1) = param_2[2];
                    unsigned char attr2 = param_2[3];
                    *(warehouseBase + 0x1b + iVar1) = attr2;

                    // Mark first cell (0,0) with flag=1, others with 0
                    if ((iVar5 == 0) && (iVar6 == 0)) {
                        *(DWORD*)(warehouseBase + 0x38 + iVar1) = 1;
                    } else {
                        *(DWORD*)(warehouseBase + 0x38 + iVar1) = 0;
                    }

                    // ItemConvert — applies level/option to the ITEM slot (minimal stub body).
                    ItemConvert((int)(warehouseBase + iVar1), (int)param_2[1], (int)attr2);

                    iVar6 = iVar6 + 1;
                } while (iVar6 < (int)(unsigned int)pAttr[iVar3].Width);
            }
            iVar5 = iVar5 + 1;
        } while (iVar5 < (int)(unsigned int)pAttr[iVar3].Height);
    }
}

// SetItemGridFlag @ 0x004CC1E0 (~35 lines) — marks cells occupied/free in inventory grid
// Reads item type at (param_2 + param_1*0x44), gets Width/Height from ItemAttribute.
// Reads grid origin from offsets 0x3e (x) and 0x3f (y).
// Writes param_3 to each cell at (param_2 + 0x40 + (gx + gy*8)*0x44).
void __cdecl SetItemGridFlag(int param_1, int param_2, unsigned char param_3) {
    // 0x004CC1E0 — marks cells occupied/free in inventory grid
    // param_1 = item slot index, param_2 = inventory base ptr, param_3 = flag value to write
    short itemType = *(short*)(param_2 + param_1 * 0x44);
    int itemBase = param_2 + param_1 * 0x44;
    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)DAT_07d78068;
    BYTE width  = attr[itemType].Width;
    BYTE height = attr[itemType].Height;
    unsigned int originX = (unsigned int)*(BYTE*)(itemBase + 0x3e);
    unsigned int originY = (unsigned int)*(BYTE*)(itemBase + 0x3f);
    unsigned int gy = originY;
    if (gy < originY + height) {
        do {
            unsigned int gx = originX;
            if (gx < originX + width) {
                unsigned char* pCell = (unsigned char*)(param_2 + 0x40 + (gx + gy * 8) * 0x44);
                do {
                    *pCell = param_3;
                    gx++;
                    pCell += 0x44;
                } while ((int)gx < (int)(originX + (unsigned int)width));
            }
            gy++;
        } while ((int)gy < (int)(originY + (unsigned int)height));
    }
}

// CompareItems @ 0x004CC270 (~93 lines) — compare two items for sorting
// Returns: -1 (a<b), 0 (equal), 1 (a>b), 2 (different type)
// Compares: type, level (bits 3..6), excellent bit (bit 7), option bytes, durability
int __cdecl CompareItems(short param_1, int param_2, int param_3) {
    // 0x004CC270 — Compare two items for sorting
    // Ghidra shows phantom stack params (in_stack_*) because caller pushes two full
    // 0x44-byte item records on the stack. param_1/param_2/param_3 overlap the first record.
    // The second record starts at stack offset +0x44 from param_1.
    //
    // Stack layout (from &param_1):
    //   +0x00: typeA (short), +0x04: levelA (int, bits 3..6=level, bit7=excel)
    //   +0x08: param_3 (byte at +0x09 = durability class)
    //   +0x1a: durabilityA, +0x1b: attr2A (& 0x3f = option)
    //   +0x24: extraDurA, +0x25..+0x2c: socket slots, +0x2d..+0x34: socket vals
    //   +0x44: typeB (short), +0x48: levelB, +0x4d: durability class B, etc.
    //
    // Comparison order: type → dur_class → level → excellent → option → extra_dur → sockets → durability
    // Returns: 2=different type, 0=equal, -1=a<b, 1=a>b

    unsigned char* base = (unsigned char*)&param_1;

    // Item A fields (stack offsets from Ghidra, relative to param_1 at +0x04)
    // in_stack at +0x04 means base[0], etc. Ghidra offsets are absolute stack offsets.
    // param_1 = stack+0x04, so in_stack_0000001e = base[0x1e - 0x04] = base[0x1a]
    short typeA = *(short*)(base + 0x00);              // param_1
    int   lvlIntA = *(int*)(base + 0x04);              // param_2
    unsigned char durClassA = base[0x09];               // param_3 byte 1
    unsigned char durA = base[0x1a];                    // in_stack_0000001e
    unsigned char attr2A = base[0x1b];                  // in_stack_0000001f
    unsigned char extraDurA = base[0x24];               // in_stack_00000028

    // Item B fields (0x44 bytes after item A)
    short typeB = *(short*)(base + 0x44);              // in_stack_00000048
    int   lvlIntB = *(int*)(base + 0x48);              // in_stack_0000004c
    unsigned char durClassB = base[0x4d];               // in_stack_00000051
    unsigned char durB = base[0x5e];                    // in_stack_00000062
    unsigned char attr2B = base[0x5f];                  // in_stack_00000063
    unsigned char extraDurB = base[0x68];               // in_stack_0000006c

    // Different type → incomparable
    if (typeA != typeB) return 2;

    // Different durability class → equal (no ordering)
    if (durClassA != durClassB) return 0;

    // Level (bits 3..6)
    unsigned int levelA = (lvlIntA >> 3) & 0xf;
    unsigned int levelB = (lvlIntB >> 3) & 0xf;
    if (levelA != levelB) return (levelA < levelB) ? -1 : 1;

    // Excellent bit (bit 7)
    unsigned int excelA = (lvlIntA >> 7) & 1;
    unsigned int excelB = (lvlIntB >> 7) & 1;
    if (excelA < excelB) return -1;
    if (excelA > excelB) return 1;

    // Option (attr2 & 0x3f)
    if ((attr2A & 0x3f) < (attr2B & 0x3f)) return -1;
    if ((attr2A & 0x3f) > (attr2B & 0x3f)) return 1;

    // Extra durability
    if (extraDurA < extraDurB) return -1;
    if (extraDurA > extraDurB) return 1;

    // Socket scan (slots at +0x25+i, vals at +0x2d+i for A; +0x69+i, +0x71+i for B)
    unsigned int hasSocketA = 0, hasSocketB = 0;
    unsigned char socketValA = 0, socketValB = 0;
    for (int i = 0; i < (int)(unsigned int)extraDurB; i++) {
        if (base[0x25 + i] > 0x3b && base[0x25 + i] < 0x40) {
            hasSocketA = 1;
            socketValA = base[0x2d + i];
        }
        if (base[0x69 + i] > 0x3b && base[0x69 + i] < 0x40) {
            hasSocketB = 1;
            socketValB = base[0x71 + i];
        }
    }
    if (hasSocketA < hasSocketB) return -1;
    if (socketValA < socketValB) return -1;
    if (hasSocketA != hasSocketB && socketValA != socketValB) return 1;

    // Durability
    int result = 0;
    if (durA < durB) result = -1;
    if (durA <= durB) return result;
    return 1;
}

// IDA: sub_4CC3C0 (0x004CC3C0).  No ordena nada: marca los items de un pool
// del trade (32 celdas) contra el historial de items valiosos (word_7E11F78,
// 32 entradas).  Si un item es PEOR que uno del historial del mismo tipo, su
// footprint queda en 99 (color de advertencia) y byte_7EAA0E8 = 1; si es igual
// o mejor, en 1.  Los tipos que empataron (resultado 0) vuelven a 1 en todas
// sus celdas al final.
// (El port anterior llamaba a CompareItems con tipo/nivel/durabilidad en
// vez de los dos registros, asi que la comparacion era basura.)
extern "C" int __cdecl Item_CompareForTradeHistory(const BYTE* p, const BYTE* n);
void __cdecl SortInventory(short* param_1) {
    BYTE* const pool = (BYTE*)param_1;
    DAT_07eaa0e8 = 0;                                   // byte_7EAA0E8
    for (int i = 0; i < 32; ++i)
        pool[i * 68 + 64] = 1;

    int tied[10];                                       // char v19[40]
    int tiedCount = 0;
    const BYTE* const history = (const BYTE*)&DAT_07e11f78[0];

    for (int slot = 0; slot < 32; ++slot) {
        const BYTE* item = pool + slot * 68;
        if (*(const short*)item == -1 || *(const DWORD*)(item + 56) == 0)
            continue;
        for (int h = 0; h < 32; ++h) {
            const BYTE* entry = history + h * 68;
            if (*(const short*)entry == -1 || *(const short*)entry != *(const short*)item)
                continue;
            const int r = Item_CompareForTradeHistory(entry, item);
            if (r == 1) {                               // peor que el historial
                DAT_07eaa0e8 = 1;
                SetItemGridFlag(slot, (int)(uintptr_t)pool, 99);
                continue;
            }
            if (r == 0 && tiedCount < 10)               // el original no acota
                tied[tiedCount++] = *(const short*)entry;
            SetItemGridFlag(slot, (int)(uintptr_t)pool, 1);
            break;
        }
    }

    if (tiedCount > 0) {
        DAT_07eaa0e8 = 0;
        for (int t = 0; t < tiedCount; ++t)
            for (int i = 0; i < 32; ++i) {
                const BYTE* item = pool + i * 68;
                if (*(const short*)item != -1 && *(const short*)item == tied[t] &&
                    *(const DWORD*)(item + 56) != 0)
                    SetItemGridFlag(i, (int)(uintptr_t)pool, 1);
            }
    }
}

// CheckInventorySpace @ 0x004D5D70 (~66 lines) — check if picked item fits at position
// Returns 1 in low byte if space available, 0 otherwise.
// Also handles stackable item merging (arrows 0x1bf..0x1c8, potions 0x87/0x8f).
unsigned long long __cdecl CheckInventorySpace(int p1, int p2, unsigned short* p3, int p4, int p5) {
    // 0x004D5D70 — Check if the currently picked item fits at the mouse grid position
    // p1 = screen offset X (inventory panel left), p2 = screen offset Y (inventory panel top)
    // p3 = inventory array base (short*, stride 0x22 words = 0x44 bytes)
    // p4 = grid width, p5 = grid height
    //
    // Uses DAT_07e91350 (pPickedItem type), DAT_07d78068 (ItemAttribute base) to get item Width/Height.
    // Converts MouseX/MouseY to grid coords using _DAT_00552a10 and _DAT_00552504 scale factors.
    // Returns low byte=1 if space available, 0 otherwise.
    // Also handles stackable item merging (arrows 0x1bf..0x1c8, potions 0x87/0x8f).


    // Get picked item dimensions from ItemAttribute table
    short pickedType = *(short*)DAT_07e91350;
    int attrBase = (int)pickedType * 0x40 + DAT_07d78068;
    unsigned int itemW = (unsigned int)*(unsigned char*)(attrBase + 0x20);  // Width
    unsigned int itemH = (unsigned int)*(unsigned char*)(attrBase + 0x21);  // Height

    // Sanity-clamp itemW/itemH — if even with restore the read is bogus,
    // bail safely instead of looping over garbage cell counts.
    if (itemW == 0 || itemW > 8 || itemH == 0 || itemH > 8) {
        return 0;
    }

    // Convert mouse position to grid coords
    // gridX = ftol((MouseX - p1) * _DAT_00552a10 * _DAT_00552504 - ... + _DAT_00552504)
    // From assembly: FILD(MouseX - p1), FMUL(DAT_00552a10), FILD(itemW), FMUL(DAT_00552504), FSUB, FADD(DAT_00552504)
    int gridX = (int)((float)(MouseX - p1) * _DAT_00552a10 - (float)itemW * _DAT_00552504 + _DAT_00552504);
    int gridY = (int)((float)(MouseY - p2) * _DAT_00552a10 - (float)itemH * _DAT_00552504 + _DAT_00552504);

    // Count empty cells in the item's footprint
    int emptyCount = 0;
    if (gridY < (int)(gridY + itemH)) {
        int rowOffset = gridY * p4;
        int gy = gridY;
        do {
            int gx = gridX;
            if (gridX < (int)(itemW + gridX)) {
                do {
                    if (gx >= 0 && gy >= 0 && gx < p4 && gy < p5) {
                        // Check if cell is empty (-1)
                        BYTE* cell = (BYTE*)p3 + (rowOffset + gx) * 0x44;
                        // 2026-08-24 FIX (issue #15, "la jewel solo aplicaba en la 1er celda"):
                        // aca decia `|| *(int*)(cell + 0x38) <= 0`, o sea contaba la celda como
                        // VACIA cuando su Key era 0. Pero AddItemToGrid deja Key=0 en todas las
                        // celdas NO primarias de un item multi-celda (usa Key=1 solo para marcar
                        // la primaria), asi que de un item 2x2 tres de sus cuatro celdas se
                        // reportaban libres. IDA sub_4D5D70 L47 mira UNICAMENTE el Type:
                        //     if ( a3[34 * v13 + 34 * v14] == -1 )  ++v20;
                        // El campo Key solo gatea el RENDER (sub_4E38B0 L60), no la ocupacion.
                        if (*(short*)cell == -1) {
                            emptyCount++;
                        }
                    }
                    gx++;
                } while (gx < (int)(itemW + gridX));
            }
            gy++;
            rowOffset += p4;
        } while (gy < (int)(itemH + gridY));
    }

    // All cells empty → item fits
    if (emptyCount == (int)(itemH * itemW)) {
        return 1;  // success (low byte = 1)
    }

    // Check for stackable item merging (only in main inventory)
    // (Pool aliases declared at file scope above.)
    if (p3 == (unsigned short*)&OffsetInventoryItems[0]) {
        if (gridX < 0 || gridY < 0 || gridX >= p4 || gridY >= p5) {
            return 0;
        }
        int cellIdx = gridY * p4 + gridX;
        int cellType = (int)*(short*)((char*)OffsetInventoryItems + cellIdx * 0x44);
        int picked = (int)*(short*)DAT_07e91350;

        if (picked == cellType) {
            // Arrows (0x1bf < type < 0x1c9) stack with same range
            if (picked > 0x1bf && picked < 0x1c9 && cellType > 0x1bf && cellType < 0x1c9) {
                return 1;
            }
            // Potions (0x87 or 0x8f) stack if same type and same level
            if ((picked == 0x87 || picked == 0x8f) &&
                (cellType == 0x87 || cellType == 0x8f)) {
                int cellLevel = (*(int*)((char*)OffsetInventoryItems + cellIdx * 0x44 + 4) >> 3) & 0xf;
                int pickedLevel = ((int)DAT_07e91354 >> 3) & 0xf;
                if (pickedLevel == cellLevel) {
                    return 1;
                }
            }
        }
    }

    return 0;  // doesn't fit
}

// FindEmptySlot @ 0x004D5F20 (~62 lines) — search for empty rectangular area from bottom-right
// Scans grid from (maxH-itemH, maxW-itemW) upward/leftward for a W*H block of -1 cells.
int __cdecl FindEmptySlot(int param_1, int param_2, int param_3, int param_4) {
    // 0x004D5F20 — Search for empty rectangular area from bottom-right
    // param_1 = item type index (for Width/Height from ItemAttribute)
    // param_2 = inventory base pointer (short*, stride 0x44 bytes)
    // param_3 = grid width, param_4 = grid height
    // Returns low byte=1 if found, 0 otherwise.


    int attrBase = param_1 * 0x40 + DAT_07d78068;
    unsigned int itemH = (unsigned int)*(unsigned char*)(attrBase + 0x21);  // Height
    unsigned int itemW = (unsigned int)*(unsigned char*)(attrBase + 0x20);  // Width
    if (itemW == 0 || itemW > 8 || itemH == 0 || itemH > 8) {
        return 0;
    }

    // Scan from bottom-right corner upward/leftward
    int startY = param_4 - (int)itemH;
    if (startY < 0) return 0;

    int rowBase = startY * param_3;
    do {
        int gx = param_3 - (int)itemW;
        if (gx >= 0) {
            BYTE* pCell = (BYTE*)(uintptr_t)(param_2 + (rowBase + gx) * 0x44);
            do {
                if (*(short*)pCell == -1) {
                    // Check if full W*H block is empty
                    int ry = 0;
                    BYTE* pRow = pCell;
                    bool fits = true;
                    if (itemH != 0) {
                        do {
                            int rx = 0;
                            BYTE* pCol = pRow;
                            if (itemW != 0) {
                                do {
                            // IDA 0x004D5F20 only tests the item sentinel.  The
                            // former +0x38 condition belongs to a different UI
                            // occupancy convention and made Action's pickup gate
                            // accept partially occupied inventory rectangles.
                            if (*(short*)pCol != -1) {
                                        fits = false;
                                        goto next_cell;
                                    }
                                    rx++;
                                    pCol += 0x44;
                                } while (rx < (int)itemW);
                            }
                            ry++;
                            pRow += param_3 * 0x44;
                        } while (ry < (int)itemH);
                    }
                    if (fits) return 1;  // found empty space
                }
next_cell:
                gx--;
                pCell -= 0x44;  // move left one cell
            } while (gx >= 0);
        }
        startY--;
        rowBase -= param_3;
    } while (startY >= 0);

    return 0;  // no space found
}

// FindEmptySlotNearMouse @ 0x004D6020 (~69 lines) — search outward from cursor position
// Spirals outward from mouse grid pos, checking W*H blocks of empty cells.
// Updates MouseX/MouseY globals on success.

// CalculateInventoryValue @ 0x004DF330 (~65 lines) — sum item values in inventory grid
// Iterates grid rows*cols, calls ItemValue for each non-empty slot.
// Special items: 0x1cd=100k, 0x1ce=70k, 399=40k, 0x1d0/0x1d6=450k zen.
long long __fastcall CalculateInventoryValue(int p1, unsigned int p2, short* p3, int p4, short* p5) {
    // 0x004DF330 — Sum item values in inventory grid
    // p1 = unused (fastcall ECX), p2 = unused (fastcall EDX, but reused as temp)
    // p3 = inventory base (short*, stride 0x22 words = 0x44 bytes)
    // p4 = grid width (columns), p5 = grid height (rows, cast as int)
    //
    // Special currency items: 0x1cd=100k, 0x1ce=70k, 0x18f(399)=40k, 0x1d0/0x1d6=450k zen.
    // For regular items, calls Item_CalculateValue(item_ptr, 0) to get sell value.
    // Returns total value as long long. If item 0x18f (399) is present, returns value in EAX;
    // otherwise returns 0.

    (void)p1;

    short* pRow = p3;
    int totalValue = 0;
    bool hasItem399 = false;
    int rows = (int)p5;

    if (rows <= 0) {
        return 0;  // no rows
    }

    int rowStride = p4 * 0x22;  // stride per row in shorts (p4 cells * 0x44 bytes / 2)

    do {
        int cols = p4;
        short* pCell = pRow;
        if (cols > 0) {
            do {
                short itemType = *pCell;
                if (itemType != -1) {
                    int durability = *(int*)(pCell + 0x1c);  // offset +0x38 bytes = +0x1c shorts
                    if (durability > 0) {
                        if (itemType == 0x1cd) {
                            totalValue += 100000;
                        } else if (itemType == 0x1ce) {
                            totalValue += 70000;
                        } else if (itemType == 0x18f) {  // 399
                            totalValue += 40000;
                        } else if (itemType == 0x1d0) {
                            totalValue += 450000;
                        } else if (itemType == 0x1d6) {
                            totalValue += 450000;
                        } else {
                            // BUG-FIX 2026-04-26 (audit #3): ItemValue(item, sellMode=0).
                            // Antes el stub recibía (durability, 0, item, 0) → arg order roto.
                            int itemVal = Item_CalculateValue((void*)pCell, 0);
                            totalValue += itemVal;
                            (void)durability;
                        }
                        if (*pCell == 0x18f) {  // 399
                            hasItem399 = true;
                        }
                    }
                }
                cols--;
                pCell += 0x22;  // next cell (0x44 bytes)
            } while (cols != 0);
        }
        pRow += rowStride;
        rows--;
    } while (rows != 0);

    if (hasItem399) {
        return (long long)totalValue;
    }
    return 0;
}
