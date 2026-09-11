// Item_ChaosMix.cpp
//
// Extracted from stubs_game.cpp.  Owns chaos-mix recipe validation and its
// inventory-panel helper.  IDA provenance remains in the function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" DWORD g_ItemAttribute_Backup;   // DAT_07d78068 recovery pointer
// CheckMixRecipe @ 0x004E40F0
// Validates chaos/mix recipe ingredients in the mix inventory grid.
// Scans all items, counts by category (wings, jewels, excellent items, +level items).
// Sets DAT_0055a3fc (max height), DAT_0055a3f8 (max width).
#if 0
int __cdecl CheckMixRecipe(short* param_1, int param_2, int param_3) {
    // 0x004E40F0 — Validates chaos/mix recipe ingredients in the mix inventory grid.
    // param_1 = inventory array base (short*, stride 0x22 words = 0x44 bytes)
    // param_2 = grid width, param_3 = grid height
    // Scans all items, counts by category, then matches known recipes.
    // Returns recipe ID (1..11) or 0 (invalid) or negative (level mismatch).

    // Category counters (local_78[0..18] in Ghidra = 19 ints)
    int counters[19];
    memset(counters, 0, sizeof(counters));
    // counters[0] = totalItems (local_78[0])
    // counters[1] = jewelOfChaos count (local_78[1])  — item 0x18F (399)
    // counters[2] = excellentItemCount (local_78[2])
    // counters[3] = devilSquareKey count (local_78[3]) — item 0x1D1 (473)

    // Named counters mapped from Ghidra locals:
    int& totalItems     = counters[0];   // local_78[0]
    int& jewelOfChaos   = counters[1];   // local_78[1]  — type 0x18F
    int& excellentItems = counters[2];   // local_78[2]
    int& dsKeyA         = counters[3];   // local_78[3]  — type 0x1D1

    int blessCount     = 0;  // local_64 — level 9 jewels
    int soulCount      = 0;  // local_60 — level 10 jewels
    int creationCount  = 0;  // local_5c — type 0x1CD
    int creationCount2 = 0;  // local_58 — type 0x1CE
    int normalCount    = 0;  // local_50 — items that aren't excellent
    int gemstoneCount  = 0;  // local_4c — type 0x1D6
    int luckCharmCount = 0;  // local_48 — type 0x1AE
    int wingCount      = 0;  // local_44 — type 0x180..0x182
    int excellentArmor = 0;  // local_40 — excellent items with level > 3
    int excellentSpecial = 0;// local_3c — (not set in decompile, stays 0)
    int ringOfXformA   = 0;  // local_38 — excellent items that are 0x46/0x86/0xA7
    int featherCount   = 0;  // local_34 — type 0x1B0
    int clawCount      = 0;  // local_30 — type 0x1B1
    int cipherCount    = 0;  // local_54 — type 0x1A2

    unsigned int levelB0 = 0;  // local_10 — level of type 0x1B0
    unsigned int levelB1 = 0;  // local_c  — level of type 0x1B1
    unsigned int levelD1 = 0;  // local_1c — level of type 0x1D1
    unsigned int levelD2 = 0;  // local_18 — level of type 0x1D2
    int dsKeyB         = 0;  // local_68 — type 0x1D2
    int totalValue     = 0;  // local_28 — accumulated item value for wing recipe
    int wingValue      = 0;  // local_24 — wing base value

    DAT_0055a3fc = 1;  // max height
    DAT_0055a3f8 = 1;  // max width

    unsigned int itemLevel = 0;
    int strideBytes = param_2 * 0x44;

    if (param_3 <= 0) goto done_scan;

    {
        short* rowBase = param_1;
        int rowsLeft = param_3;
        do {
            short* psVar12 = rowBase;
            int colsLeft = param_2;
            if (param_2 > 0) {
                do {
                    short sVar2 = *psVar12;
                    if (sVar2 != -1 && *(int*)(psVar12 + 0x1C) > 0) {
                        int iVar5 = (int)sVar2;
                        totalItems++;

                        // Update max dimensions
                        int attrBase = iVar5 * 0x40 + DAT_07d78068;
                        if (DAT_0055a3f8 <= (int)(unsigned int)*(unsigned char*)(attrBase + 0x20)) {
                            DAT_0055a3f8 = (int)(unsigned int)*(unsigned char*)(attrBase + 0x20);
                        }
                        if ((int)DAT_0055a3fc <= (int)(unsigned int)*(unsigned char*)(attrBase + 0x21)) {
                            DAT_0055a3fc = (int)(unsigned int)*(unsigned char*)(attrBase + 0x21);
                        }

                        itemLevel = *(int*)(psVar12 + 2) >> 3 & 0xF;
                        bool bVar4 = false;
                        bool hasSocket = false;

                        // Check if item is a jewel (type < 0x180 or type in 0x180..0x186)
                        if (sVar2 < 0x180 || sVar2 < 0x187) {
                            if (itemLevel == 9) {
                                blessCount++;
                            } else if (itemLevel == 10) {
                                soulCount++;
                            } else {
                                goto check_type;
                            }
                            DAT_07eaa178 = 0;
                            bVar4 = true;
                            hasSocket = true;
                        }

                    check_type:
                        if ((unsigned int)(iVar5 - 0x180) > 0x56) {
                            // Not a special item (types outside 0x180..0x1D6 range)
                            goto check_default;
                        }

                        switch (iVar5) {
                        case 0x180: case 0x181: case 0x182:
                            // Wings
                            wingCount++;
                            DAT_07eaa170 = 0;
                            wingValue = ItemValue((ITEM*)psVar12, 0);
                            {
                                unsigned int socketCount = (unsigned int)*(unsigned char*)(psVar12 + 0x12);
                                int si = 0;
                                if (socketCount != 0) {
                                    do {
                                        unsigned char bVal = *(unsigned char*)((int)psVar12 + si + 0x25);
                                        if (bVal > 0x3B) {
                                            if (bVal < 0x3E) {
                                                DAT_07eaa170 = (unsigned int)*(unsigned char*)((int)psVar12 + si + 0x2D);
                                            } else if (bVal == 0x41) {
                                                DAT_07eaa170 = (unsigned int)*(unsigned char*)((int)psVar12 + si + 0x2D) << 2;
                                            }
                                        }
                                        si++;
                                    } while (si < (int)socketCount);
                                }
                            }
                            goto scan_socket_flag;

                        case 0x18F:  // 399 — Jewel of Chaos
                            jewelOfChaos++;
                            break;

                        case 0x1A2:  // 418 — Cipher item (Devil Square piece?)
                            if ((char)psVar12[0x0D] == -1) {
                                cipherCount++;
                            }
                            break;

                        case 0x1AE:  // 430 — Luck Charm / Loch's Feather
                            luckCharmCount++;
                            break;

                        case 0x1B0:  // 432 — Feather of Condor
                            featherCount++;
                            levelB0 = itemLevel;
                            break;

                        case 0x1B1:  // 433 — Claw of Beast
                            clawCount++;
                            levelB1 = itemLevel;
                            break;

                        case 0x1CD:  // 461 — Jewel of Creation
                            creationCount++;
                            break;

                        case 0x1CE:  // 462 — Jewel of Creation 2
                            creationCount2++;
                            break;

                        case 0x1D1:  // 473 — Devil Square Key A
                            dsKeyA++;
                            levelD1 = itemLevel;
                            break;

                        case 0x1D2:  // 474 — Devil Square Key B
                            dsKeyB++;
                            levelD2 = itemLevel;
                            break;

                        case 0x1D6:  // 470 — Gemstone
                            gemstoneCount++;
                            break;

                        default:
                        check_default:
                            {
                                // Check for excellent items (option bytes 0x3C..0x3F)
                                bool foundExcellent = false;
                                if (sVar2 < 0x180 && itemLevel > 3) {
                                    DAT_07eaa178 = 0;
                                    bVar4 = true;
                                    hasSocket = true;
                                }
                                unsigned int socketCount2 = (unsigned int)*(unsigned char*)(psVar12 + 0x12);
                                if (socketCount2 != 0) {
                                    for (int si2 = 0; si2 < (int)socketCount2; si2++) {
                                        unsigned char bVal2 = *(unsigned char*)((int)psVar12 + si2 + 0x25);
                                        if (bVal2 > 0x3B && bVal2 < 0x40 && !foundExcellent) {
                                            excellentItems++;
                                            if (sVar2 == 0x46 || sVar2 == 0x86 || sVar2 == 0xA7) {
                                                ringOfXformA++;
                                            }
                                            foundExcellent = true;
                                        }
                                    }
                                    if (!foundExcellent) goto count_normal;
                                } else {
                                count_normal:
                                    normalCount++;
                                }
                                // Check excellent armor with level > 3
                                if ((*(unsigned char*)((int)psVar12 + 0x1B) & 0x3F) != 0 && itemLevel > 3) {
                                    excellentArmor++;
                                    int val = ItemValue((ITEM*)psVar12, 0);
                                    totalValue += val;
                                }
                            }
                            goto scan_socket_flag_end;
                        }

                    scan_socket_flag:
                        // Scan for socket flag '@' (0x40) in option bytes
                        {
                            unsigned int sc = (unsigned int)*(unsigned char*)(psVar12 + 0x12);
                            if (sc != 0) {
                                for (int si3 = 0; si3 < (int)sc; si3++) {
                                    if (*(char*)((int)psVar12 + si3 + 0x25) == 0x40) {
                                        DAT_07eaa170 = 1;  // _DAT_07eaa170 for wing path
                                        if (hasSocket) {
                                            DAT_07eaa178 = 1;
                                        }
                                    }
                                }
                            }
                        }
                    scan_socket_flag_end:
                        ;
                    }
                    colsLeft--;
                    psVar12 += 0x22;  // stride 0x44 bytes
                } while (colsLeft != 0);
            }
            rowBase = (short*)((int)rowBase + strideBytes);
            rowsLeft--;
        } while (rowsLeft != 0);
    }

done_scan:
    // Recipe matching logic
    if (totalItems == 3) {
        if (jewelOfChaos == 1) {
            // 3 items + 1 chaos jewel
            if (dsKeyA == 1 && dsKeyB == 1) {
                // Devil Square key recipe
                if (levelD1 == levelD2) {
                    DAT_07eaa168 = levelD1 + 1;
                    return 2;
                }
                return -2;
            }
            if (featherCount == 1 && clawCount == 1) {
                // Feather + Claw recipe
                if (levelB0 == levelB1) {
                    DAT_07eaa168 = levelB0 + 1;
                    return 8;
                }
                return -8;
            }
            goto check_wing_recipe;
        }
    } else {
        if (jewelOfChaos == 1) goto check_wing_recipe;
        goto check_other_recipes;
    }

    // Fall through to other recipes
    goto check_other_recipes;

check_wing_recipe:
    if (luckCharmCount == 1 && wingCount == 1 &&
        totalItems == excellentArmor + 3 && excellentSpecial == 0) {
        DAT_07eaa174 = wingValue / 4000000 + totalValue / 40000;
        if (DAT_07eaa174 > 0x59) {
            DAT_07eaa174 = 0x5A;
        }
        return 7;
    }
    goto check_other_recipes_detail;

check_other_recipes:
    ;
check_other_recipes_detail:
    if (totalItems == 4) {
        if (jewelOfChaos == 1) {
            if (blessCount == 1 && creationCount == 1 && creationCount2 == 1) {
                return 3;
            }
            goto check_final;
        }
    } else if (totalItems == 6) {
        if (jewelOfChaos == 1) {
            if (soulCount == 1 && creationCount == 2 && creationCount2 == 2) {
                return 4;
            }
            goto check_final;
        }
    } else if (totalItems == 0x0B) {
        if (cipherCount == 10 && jewelOfChaos == 1) {
            return 5;
        }
    } else if (totalItems == 2 && gemstoneCount == 1 && jewelOfChaos == 1) {
        return 6;
    }

check_final:
    if (jewelOfChaos < 1) {
        return 0;
    }

    // Check if all items are excellent (no other categories)
    if (excellentItems > 0 && dsKeyA == 0 && dsKeyB == 0 &&
        cipherCount == 0 && normalCount == 0 && gemstoneCount == 0 &&
        luckCharmCount == 0 && wingCount == 0 &&
        excellentSpecial == 0 && featherCount == 0 && clawCount == 0) {
        // Recipe 1 or 11 depending on whether ringOfXformA items present
        return (ringOfXformA < 1) ? 1 : 11;
    }
    return 0;
}

// RenderInventoryInterface @ 0x004ECBA0 (~20 lines) — renders inventory background bitmaps
// RenderBitmap(0x104, x, y, 190, 256, ...); // top half
// RenderBitmap(0x105, x, y+offset1, 190, 177, ...); // bottom half
// if (Flag) RenderBitmap(0x117, x, y+offset2, 190, 10, ...); // separator line
#endif

int __cdecl CheckMixRecipe(short* param_1, int param_2, int param_3) {
    char counts[77];
    memset(counts + 1, 0, 0x4C);

    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

    int levelD1 = 0;
    int levelD2 = 0;
    int levelB0 = 0;
    int levelB1 = 0;
    int wingValue = 0;
    int excellentValue = 0;

    DAT_0055a3fc = 1;
    DAT_0055a3f8 = 1;

    if (param_3 > 0) {
        int rowStrideBytes = 68 * param_2;
        int rowBase = (int)param_1;

        for (int row = 0; row < param_3; ++row) {
            int itemPtr = rowBase;
            for (int col = 0; col < param_2; ++col) {
                short itemType = *(short*)itemPtr;
                if (itemType != (short)0xFFFF && *(int*)(itemPtr + 56) > 0) {
                    ++*(int*)(counts + 1);

                    int attrBase = DAT_07d78068 + itemType * 0x40;
                    int attrW = *(unsigned char*)(attrBase + 0x20);
                    int attrH = *(unsigned char*)(attrBase + 0x21);
                    // 004E40F0 compares directly against ITEM_ATTRIBUTE; it
                    // does not clamp these dimensions to the usual 1..8 grid.
                    if (DAT_0055a3f8 <= attrW) DAT_0055a3f8 = attrW;
                    if (DAT_0055a3fc <= attrH) DAT_0055a3fc = attrH;

                    int itemLevel = (*(int*)(itemPtr + 4) >> 3) & 0xF;
                    char* socketFlagPtr = 0;
                    bool scanSocketFlag = false;

                    if (itemType < 384 || itemType <= 390) {
                        if (itemLevel == 9) {
                            ++*(int*)(counts + 21);
                            DAT_07eaa178 = 0;
                            scanSocketFlag = true;
                            socketFlagPtr = (char*)&DAT_07eaa178;
                        } else if (itemLevel == 10) {
                            ++*(int*)(counts + 25);
                            DAT_07eaa178 = 0;
                            scanSocketFlag = true;
                            socketFlagPtr = (char*)&DAT_07eaa178;
                        }
                    }

                    switch (itemType) {
                    case 384:
                    case 385:
                    case 386:
                    {
                        ++*(int*)(counts + 53);
                        DAT_07eaa170 = 0;
                        wingValue = ItemValue((ITEM*)itemPtr, 0);
                        int socketCount = *(unsigned char*)(itemPtr + 36);
                        for (int i = 0; i < socketCount; ++i) {
                            unsigned char v = *(unsigned char*)(itemPtr + 37 + i);
                            if (v >= 0x3C) {
                                if (v <= 0x3D) {
                                    DAT_07eaa170 = *(unsigned char*)(itemPtr + 45 + i);
                                } else if (v == 0x41) {
                                    DAT_07eaa170 = 4 * *(unsigned char*)(itemPtr + 45 + i);
                                }
                            }
                        }
                        break;
                    }
                    case 399:
                        ++*(int*)(counts + 5);
                        break;
                    case 418:
                        if (*(unsigned char*)(itemPtr + 26) == 0xFF) {
                            ++*(int*)(counts + 37);
                        }
                        break;
                    case 430:
                        ++*(int*)(counts + 49);
                        break;
                    case 432:
                        levelB0 = itemLevel;
                        ++*(int*)(counts + 69);
                        break;
                    case 433:
                        levelB1 = itemLevel;
                        ++*(int*)(counts + 73);
                        break;
                    case 461:
                        ++*(int*)(counts + 29);
                        break;
                    case 462:
                        ++*(int*)(counts + 33);
                        break;
                    case 465:
                        levelD1 = itemLevel;
                        ++*(int*)(counts + 13);
                        break;
                    case 466:
                        levelD2 = itemLevel;
                        ++*(int*)(counts + 17);
                        break;
                    case 470:
                        ++*(int*)(counts + 45);
                        break;
                    default:
                    {
                        bool foundExcellent = false;
                        if (itemType < 384 && itemLevel >= 4) {
                            DAT_07eaa178 = 0;
                            scanSocketFlag = true;
                            socketFlagPtr = (char*)&DAT_07eaa178;
                        }

                        int socketCount = *(unsigned char*)(itemPtr + 36);
                        for (int i = 0; i < socketCount; ++i) {
                            unsigned char v = *(unsigned char*)(itemPtr + 37 + i);
                            if (v >= 0x3C && v <= 0x3F && !foundExcellent) {
                                ++*(int*)(counts + 9);
                                if (itemType == 70 || itemType == 134 || itemType == 167) {
                                    ++*(int*)(counts + 65);
                                }
                                foundExcellent = true;
                            }
                        }

                        if (!foundExcellent) {
                            ++*(int*)(counts + 41);
                        }

                        if ((*(unsigned char*)(itemPtr + 27) & 0x3F) != 0 && itemLevel >= 4) {
                            ++*(int*)(counts + 57);
                            excellentValue += ItemValue((ITEM*)itemPtr, 0);
                        }
                        break;
                    }
                    }

                    if (scanSocketFlag) {
                        int socketCount = *(unsigned char*)(itemPtr + 36);
                        for (int i = 0; i < socketCount; ++i) {
                            if (*(unsigned char*)(itemPtr + 37 + i) == 0x40) {
                                DAT_07eaa170 = 1;
                                if (socketFlagPtr) {
                                    *socketFlagPtr = 1;
                                }
                            }
                        }
                    }
                }

                itemPtr += 68;
            }
            rowBase += rowStrideBytes;
        }
    }

    int totalItems = *(int*)(counts + 1);
    int jewelOfChaos = *(int*)(counts + 5);

    if (totalItems == 3 && jewelOfChaos == 1) {
        if (*(int*)(counts + 13) == 1 && *(int*)(counts + 17) == 1) {
            if (levelD1 != levelD2) return -2;
            DAT_07eaa168 = levelD1 + 1;
            return 2;
        }
        if (*(int*)(counts + 69) == 1 && *(int*)(counts + 73) == 1) {
            if (levelB0 != levelB1) return -8;
            DAT_07eaa168 = levelB0 + 1;
            return 8;
        }
    }

    if (jewelOfChaos == 1
        && *(int*)(counts + 49) == 1
        && *(int*)(counts + 53) == 1
        && totalItems == *(int*)(counts + 57) + 3
        && !*(int*)(counts + 61)) {
        DAT_07eaa174 = wingValue / 4000000 + excellentValue / 40000;
        if (DAT_07eaa174 >= 90) DAT_07eaa174 = 90;
        return 7;
    }

    switch (totalItems) {
    case 4:
        if (jewelOfChaos == 1
            && *(int*)(counts + 21) == 1
            && *(int*)(counts + 29) == 1
            && *(int*)(counts + 33) == 1) {
            return 3;
        }
        break;
    case 6:
        if (jewelOfChaos == 1
            && *(int*)(counts + 25) == 1
            && *(int*)(counts + 29) == 2
            && *(int*)(counts + 33) == 2) {
            return 4;
        }
        break;
    case 11:
        if (*(int*)(counts + 37) == 10 && jewelOfChaos == 1) {
            return 5;
        }
        break;
    default:
        if (totalItems == 2 && *(int*)(counts + 45) == 1 && jewelOfChaos == 1) {
            return 6;
        }
        break;
    }

    if (jewelOfChaos <= 0) return 0;

    if (*(int*)(counts + 9) > 0
        && !*(int*)(counts + 13)
        && !*(int*)(counts + 17)
        && !*(int*)(counts + 37)
        && !*(int*)(counts + 41)
        && !*(int*)(counts + 45)
        && !*(int*)(counts + 49)
        && !*(int*)(counts + 53)
        && !*(int*)(counts + 61)
        && !*(int*)(counts + 69)
        && !*(int*)(counts + 73)) {
        return (*(int*)(counts + 65) <= 0) ? 1 : 11;
    }

    return 0;
}

void __cdecl RenderInventoryInterface_stub(int StartX, int StartY, int Flag) {
    // 0x004ECBA0 — renders inventory background bitmaps
    float x = (float)StartX;
    float y = (float)StartY;
    // Top half of inventory panel
    GL_DrawTexture(0x104, x, y, 190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f, true, true);
    // Bottom half (y + 256.0f)
    GL_DrawTexture(0x105, x, y + 256.0f, 190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f, true, true);
    // Separator line when Flag is set (y + 225.0f)
    if (Flag != 0) {
        GL_DrawTexture(0x117, x, y + 225.0f, 190.0f, 10.0f, 0.0f, 0.0f, 0.7421875f, 0.625f, true, true);
    }
}
