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

extern "C" DWORD g_ItemAttribute_Backup;   // DAT_07d78068 recovery pointer
// InsertWarehouseItem @ 0x004CC0E0 (~52 lines) — fills warehouse grid cells for an item
// ConvertItemType(param_2) -> get Width/Height from ItemAttribute[type].
// For each cell in WxH: set item type, grid coords, call ItemConvert.
// Stride 0x44 per cell at OffsetWarehouseItems base (DAT_07ea5b30).
// ItemAttribute = DAT_07d78068 (ITEM_ATTRIBUTE array, stride 0x40).
void __cdecl InsertWarehouseItem_stub(unsigned int param_1, unsigned char* param_2) {
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
                    FUN_0047b910((int)(warehouseBase + iVar1), (int)param_2[1], (int)attr2);

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
void __cdecl SetItemGridFlag_stub(int param_1, int param_2, unsigned char param_3) {
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
int __cdecl CompareItems_stub(short param_1, int param_2, int param_3) {
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

// SortInventory @ 0x004CC3C0 (~99 lines) — sort inventory slots by item quality
// Iterates 0x20 inventory slots, compares against DAT_07e11f78 reference array (stride 0x22 words).
// Uses CompareItems for ordering. Calls SetItemGridFlag to update grid flags.
void __cdecl SortInventory_stub(short* param_1) {
    // 0x004CC3C0 — Sort inventory slots by item quality
    // Iterates 0x20 inventory slots, compares each against DAT_07e11f78 reference array
    // (stride 0x22 words = 0x44 bytes). Uses CompareItems_stub for ordering.
    // Calls SetItemGridFlag_stub to mark cells as occupied (1) or sorted (99).
    // DAT_07eaa0e8 = sort-dirty flag.

    int matchCount = 0;
    DAT_07eaa0e8 = 0;

    // Reset the "sorted" byte at offset +0x40 (= short offset +0x20) for each of 0x20 slots
    short* pReset = param_1 + 0x20;  // offset +0x40 bytes = +0x20 shorts
    for (int i = 0x20; i != 0; i--) {
        *(unsigned char*)pReset = 1;
        pReset += 0x22;  // stride 0x44 bytes = 0x22 shorts
    }

    int matchedTypes[10];
    int* pMatchList = matchedTypes;
    int slotIdx = 0;
    short* pSlot = param_1;

    do {
        if ((*pSlot != -1) && (*(int*)(pSlot + 0x1c) != 0)) {
            // Slot is occupied and has durability > 0
            int refIdx = 0;
            short* pRef = (short*)&DAT_07e11f78;

            do {
                if ((*pRef != -1) && (*pRef == *pSlot)) {
                    // Found a matching reference item — compare quality
                    // Copy both item records to stack for CompareItems_stub
                    // (CompareItems reads 0x44 bytes from &param_1 and &param_1+0x44)
                    unsigned char itemBuf[0x88];  // two 0x44-byte records
                    memcpy(itemBuf, pSlot, 0x44);
                    memcpy(itemBuf + 0x44, pRef, 0x44);

                    int cmpResult = CompareItems_stub(
                        *(short*)itemBuf,
                        *(int*)(itemBuf + 4),
                        *(int*)(itemBuf + 8));

                    if (cmpResult == 1) {
                        // Current slot item is better than reference → mark as "excellent" (99)
                        DAT_07eaa0e8 = 1;
                        SetItemGridFlag_stub(slotIdx, (int)param_1, 99);
                        // Continue to next reference
                    } else {
                        if (cmpResult == 0) {
                            // Equal — record the matching type
                            *pMatchList = (int)(short)((short*)&DAT_07e11f78)[refIdx * 0x22];
                            matchCount++;
                            pMatchList++;
                        }
                        SetItemGridFlag_stub(slotIdx, (int)param_1, 1);
                        break;
                    }
                }
                pRef += 0x22;
                refIdx++;
                // BUG-FIX 2026-05-03: was `(int)pRef < 0x7e127f8` (absolute
                // source-binary bound). DAT_07e11f78 is 0x880 bytes / 32 entries
                // × 0x44-byte stride; cap with explicit count.
            } while (refIdx < 32);
        }
        pSlot += 0x22;
        slotIdx++;

        if (slotIdx > 0x1f) {
            // After processing all slots, re-mark slots matching recorded types
            if (matchCount > 0) {
                DAT_07eaa0e8 = 0;
                int* pType = matchedTypes;
                do {
                    int scanIdx = 0;
                    short* pScan = param_1;
                    do {
                        if ((*pScan != -1) && ((int)*pScan == *pType) && (*(int*)(pScan + 0x1c) != 0)) {
                            SetItemGridFlag_stub(scanIdx, (int)param_1, 1);
                        }
                        scanIdx++;
                        pScan += 0x22;
                    } while (scanIdx < 0x20);
                    pType++;
                    matchCount--;
                } while (matchCount != 0);
            }
            return;
        }
    } while (true);
}

// CheckInventorySpace @ 0x004D5D70 (~66 lines) — check if picked item fits at position
// Returns 1 in low byte if space available, 0 otherwise.
// Also handles stackable item merging (arrows 0x1bf..0x1c8, potions 0x87/0x8f).
unsigned long long __cdecl CheckInventorySpace_stub(int p1, int p2, unsigned short* p3, int p4, int p5) {
    // 0x004D5D70 — Check if the currently picked item fits at the mouse grid position
    // p1 = screen offset X (inventory panel left), p2 = screen offset Y (inventory panel top)
    // p3 = inventory array base (short*, stride 0x22 words = 0x44 bytes)
    // p4 = grid width, p5 = grid height
    //
    // Uses DAT_07e91350 (pPickedItem type), DAT_07d78068 (ItemAttribute base) to get item Width/Height.
    // Converts MouseX/MouseY to grid coords using _DAT_00552a10 and _DAT_00552504 scale factors.
    // Returns low byte=1 if space available, 0 otherwise.
    // Also handles stackable item merging (arrows 0x1bf..0x1c8, potions 0x87/0x8f).

    // 2026-05-09: ItemAttribute base watchdog (mirror of FUN_004d23b0 +
    // Inventory_DropItemEx). Algún writer corrompe DAT_07d78068 = 0x1
    // intermitentemente. Sin restore, attrBase = type*0x40 + 1 → itemW/itemH
    // = bytes random → spaceFree=0 SIEMPRE → drop nunca dispara.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

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
int __cdecl FindEmptySlot_stub(int param_1, int param_2, int param_3, int param_4) {
    // 0x004D5F20 — Search for empty rectangular area from bottom-right
    // param_1 = item type index (for Width/Height from ItemAttribute)
    // param_2 = inventory base pointer (short*, stride 0x44 bytes)
    // param_3 = grid width, param_4 = grid height
    // Returns low byte=1 if found, 0 otherwise.

    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

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
                                    if (*(short*)pCol != -1 && *(int*)(pCol + 0x38) > 0) {
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
#if 0
unsigned int __cdecl FindEmptySlotNearMouse_stub(int p1, int p2, int p3, int p4, int p5) {
    // 0x004D6020 — Search outward from cursor position for empty W*H block
    // p1 = screen offset X (unused in grid scan), p2 = screen offset Y (unused in grid scan)
    // p3 = inventory base pointer, p4 = grid width, p5 = grid height
    // Uses DAT_07e91350 (pPickedItem type) for Width/Height from ItemAttribute.
    // Updates MouseX/MouseY globals on success.
    // Returns low byte=1 if found, 0 otherwise.

    short pickedType = *(short*)DAT_07e91350;
    int attrBase = (int)pickedType * 0x40 + DAT_07d78068;
    unsigned int itemH = (unsigned int)*(unsigned char*)(attrBase + 0x21);  // Height
    unsigned int itemW = (unsigned int)*(unsigned char*)(attrBase + 0x20);  // Width

    int maxGX = p4 - (int)itemW;    // max valid grid X
    int maxGY = p5 - (int)itemH;    // max valid grid Y

    if (maxGX < 0) return 0;

    // Scan from bottom-right, sweeping inward
    int gx = maxGX;
    do {
        int gy = maxGY;
        if (gy >= 0) {
            int rowBase = gy * p4;
            do {
                int emptyCount = 0;
                // Only check if within valid bounds
                if (gx >= 0 && gy >= 0 && gx <= maxGX && gy <= maxGY) {
                    // Count empty cells in W*H block starting at (gx, gy)
                    for (int cx = 0; cx < (int)itemW; cx++) {
                        if (itemH != 0) {
                            short* pCell = (short*)(p3 + (rowBase + cx + gx) * 0x44);
                            unsigned int ry = itemH;
                            do {
                                if (*pCell == -1) {
                                    emptyCount++;
                                }
                                pCell += p4 * 0x22;  // next row (stride 0x44 bytes per cell)
                                ry--;
                            } while (ry != 0);
                        }
                    }

                    if (emptyCount == (int)(itemH * itemW)) {
                        // Found! Convert grid coords back to screen coords and set MouseX/MouseY
                        MouseX = (int)((float)itemW * _DAT_00552504 + (float)gx - _DAT_00552504) * _DAT_005524fc + p1;
                        MouseY = (int)((float)itemH * _DAT_00552504 + (float)gy - _DAT_00552504) * _DAT_005524fc + p2;
                        return 1;
                    }
                }
                gy--;
                rowBase -= p4;
            } while (gy >= 0);
        }
        gx--;
    } while (gx >= 0);

    return 0;  // no space found
}

// Inventory_DropItem @ 0x004D6470 (sub_4D6470, 36536 bytes / 6236 lines IDA)
// Item drag-and-drop dispatcher per inventory grid.
//
// 2026-05-08: SIGNATURE FIX — original was `void Inventory_DropItem_stub(void)`
// reading globals; per IDA `bool sub_4D6470(int a1@<edi>, int a2 origin_x,
// int a3 origin_y, DWORD* a4 invBase, int a5 gridW, int a6 gridH, int a7
// slotType)`. The mouse-to-grid conversion absolutely requires the screen
// origin (a2/a3) — without it `mouseGridX = mouseX - 0` always exceeds the
// 8-cell grid bounds → no drop ever lands.
//
// Wrapper preserved for back-compat (legacy callers that read globals).
// Real entry is `Inventory_DropItemEx` (with explicit args).
#endif

unsigned int __cdecl FindEmptySlotNearMouse_stub(int p1, int p2, int p3, int p4, int p5) {
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
    int attrBase = DAT_07d78068 + pickedType * 0x40;
    int itemH = *(unsigned char*)(attrBase + 0x21);
    int itemW = *(unsigned char*)(attrBase + 0x20);
    if (itemW <= 0 || itemW > 8 || itemH <= 0 || itemH > 8) {
        return 0;
    }
    int gx = p4 - itemW;
    int maxGX = gx;

    if (gx < 0) {
        return 0;
    }

    int maxGY = p5 - itemH;
    int gy = maxGY;

    while (true) {
        int scanY = gy;
        if (scanY < 0) {
            gx = --maxGX;
            if (gx < 0) {
                return 0;
            }
            gy = maxGY;
            continue;
        }

        int rowBase = p4 * scanY;
        while (true) {
            int emptyCount = 0;
            if (gx >= 0 && gx <= maxGX && scanY <= maxGY) {
                for (int i = 0; i < itemW; ++i) {
                    if (itemH > 0) {
                        short* cell = (short*)(p3 + 68 * (gx + rowBase + i));
                        int leftH = itemH;
                        do {
                            // Mismo fix que en CheckInventorySpace_stub: IDA sub_4D6020 L71 es
                            // `if ( *v14 == 0xFFFF )`, sin mirar Key. Con el chequeo de Key este
                            // scanner daba por libres las celdas no primarias de un item multi-celda
                            // y elegia como hueco un lugar ya ocupado.
                            if (*cell == (short)0xFFFF) {
                                ++emptyCount;
                            }
                            cell += 34 * p4;
                            --leftH;
                        } while (leftH);
                    }
                }

                if (emptyCount == itemW * itemH) {
                    MouseX = (int)(((double)itemW * 0.5 + (double)gx - 0.5) * 20.0 + (double)p1);
                    MouseY = (int)(((double)itemH * 0.5 + (double)scanY - 0.5) * 20.0 + (double)p2);
                    return 1;
                }
            }

            --scanY;
            rowBase -= p4;
            if (scanY < 0) {
                break;
            }
        }

        gx = --maxGX;
        if (gx < 0) {
            return 0;
        }
        gy = maxGY;
    }
}

// CalculateInventoryValue @ 0x004DF330 (~65 lines) — sum item values in inventory grid
// Iterates grid rows*cols, calls ItemValue for each non-empty slot.
// Special items: 0x1cd=100k, 0x1ce=70k, 399=40k, 0x1d0/0x1d6=450k zen.
long long __fastcall CalculateInventoryValue_stub(int p1, unsigned int p2, short* p3, int p4, short* p5) {
    // 0x004DF330 — Sum item values in inventory grid
    // p1 = unused (fastcall ECX), p2 = unused (fastcall EDX, but reused as temp)
    // p3 = inventory base (short*, stride 0x22 words = 0x44 bytes)
    // p4 = grid width (columns), p5 = grid height (rows, cast as int)
    //
    // Special currency items: 0x1cd=100k, 0x1ce=70k, 0x18f(399)=40k, 0x1d0/0x1d6=450k zen.
    // For regular items, calls FUN_0047c690(item_ptr, 0) to get sell value.
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
                            int itemVal = FUN_0047c690((void*)pCell, 0);
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
