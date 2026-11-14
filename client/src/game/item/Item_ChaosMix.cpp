// Item_ChaosMix.cpp
//
// Validación de recetas del chaos mix y su helper del panel de inventario.

#include "stdafx.h"
#include "assets/data/ContentCatalog.h"
#include "globals.h"
#include "functions.h"

// CheckMixRecipe @ 0x004E40F0
// Validates chaos/mix recipe ingredients in the mix inventory grid.
// Scans all items, counts by category (wings, jewels, excellent items, +level items).
// Sets DAT_0055a3fc (max height), DAT_0055a3f8 (max width).

int __cdecl CheckMixRecipe(short* param_1, int param_2, int param_3) {
    char counts[77];
    memset(counts + 1, 0, 0x4C);


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
        BYTE* rowBase = reinterpret_cast<BYTE*>(param_1);

        for (int row = 0; row < param_3; ++row) {
            BYTE* itemPtr = rowBase;
            for (int col = 0; col < param_2; ++col) {
                short itemType = *(short*)itemPtr;
                if (itemType != (short)0xFFFF && *(int *)&AsItem(itemPtr)->Key > 0) {
                    ++*(int*)(counts + 1);

                    std::uintptr_t attrBase = DAT_07d78068 + itemType * 0x40;
                    const int kind = ItemBehaviorType(itemType);   // 0.97.20: rangos y recetas por comportamiento
                    int attrW = *(unsigned char*)(attrBase + 0x20);
                    int attrH = *(unsigned char*)(attrBase + 0x21);
                    // 004E40F0 compares directly against ITEM_ATTRIBUTE; it
                    // does not clamp these dimensions to the usual 1..8 grid.
                    if (DAT_0055a3f8 <= attrW) DAT_0055a3f8 = attrW;
                    if (DAT_0055a3fc <= attrH) DAT_0055a3fc = attrH;

                    int itemLevel = (AsItem(itemPtr)->Level >> 3) & 0xF;
                    char* socketFlagPtr = 0;
                    bool scanSocketFlag = false;

                    if (kind < ITEM_WING_ELF || kind <= ITEM_WING_DARKNESS) {
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

                    switch (kind) {
                    case ITEM_WING_ELF:
                    case ITEM_WING_HEAVEN:
                    case ITEM_WING_SATAN:
                    {
                        ++*(int*)(counts + 53);
                        DAT_07eaa170 = 0;
                        wingValue = ItemValue((ITEM*)itemPtr, 0);
                        int socketCount = AsItem(itemPtr)->SpecialNum;
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
                    case ITEM_JEWEL_CHAOS:
                        ++*(int*)(counts + 5);
                        break;
                    case ITEM_HELPER_UNIRIA:
                        if (AsItem(itemPtr)->Durability == 0xFF) {
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
                    case ITEM_JEWEL_BLESS:
                        ++*(int*)(counts + 29);
                        break;
                    case ITEM_JEWEL_SOUL:
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
                    case ITEM_JEWEL_CREATION:
                        ++*(int*)(counts + 45);
                        break;
                    default:
                    {
                        bool foundExcellent = false;
                        if (kind < ITEM_WING_ELF && itemLevel >= 4) {
                            DAT_07eaa178 = 0;
                            scanSocketFlag = true;
                            socketFlagPtr = (char*)&DAT_07eaa178;
                        }

                        int socketCount = AsItem(itemPtr)->SpecialNum;
                        for (int i = 0; i < socketCount; ++i) {
                            unsigned char v = *(unsigned char*)(itemPtr + 37 + i);
                            if (v >= 0x3C && v <= 0x3F && !foundExcellent) {
                                ++*(int*)(counts + 9);
                                if (kind == 70 || kind == 134 || kind == 167) {
                                    ++*(int*)(counts + 65);
                                }
                                foundExcellent = true;
                            }
                        }

                        if (!foundExcellent) {
                            ++*(int*)(counts + 41);
                        }

                        if ((AsItem(itemPtr)->Option1 & 0x3F) != 0 && itemLevel >= 4) {
                            ++*(int*)(counts + 57);
                            excellentValue += ItemValue((ITEM*)itemPtr, 0);
                        }
                        break;
                    }
                    }

                    if (scanSocketFlag) {
                        int socketCount = AsItem(itemPtr)->SpecialNum;
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

// RenderInventoryInterface vive en Render/HUD_Pass6.cpp.
