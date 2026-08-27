// Item_InventoryRender.cpp
//
// Extracted from stubs_game.cpp.  Owns the inventory/equipment render passes.
// Every entry point retains its original IDA symbol/address in its leading
// comment; this is a file reorganization only.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
// InventoryColor @ 0x004E2420 (~38 lines) — sets GL color by item Color field
void __cdecl InventoryColor_stub(ITEM* p) {
    if (!p) return;
    switch (p->Color) {
    case 0:  glColor3f(1.0f, 1.0f, 1.0f); return;        // white (normal)
    case 1:  glColor3f(0.8f, 0.8f, 0.8f); return;         // light gray
    case 2:  glColor3f(0.6f, 0.7f, 1.0f); return;         // blue (magic)
    case 3:
    case 99: glColor3f(1.0f, 0.2f, 0.1f); break;          // red (excellent)
    case 4:  glColor3f(0.5f, 1.0f, 0.6f); return;         // green (set)
    case 5:  glColor4f(0.8f, 0.7f, 0.0f, 1.0f); return;   // gold
    case 6:  glColor4f(0.8f, 0.5f, 0.0f, 1.0f); return;   // orange
    case 7:  glColor4f(0.8f, 0.3f, 0.3f, 1.0f); return;   // pink
    case 8:  glColor4f(1.0f, 0.0f, 0.0f, 1.0f); return;   // bright red
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

static int GetEquipmentSlotOffsetBytes(int index)
{
    if (index < EQUIPMENT_WEAPON_RIGHT || index > EQUIPMENT_RING_LEFT)
        return -1;
    return 536 + 68 * index;
}

// 2026-08-08: devolvía `((ITEM*)OffsetInventoryItems) + index` — la QUINTA copia
// del mismo error. `OffsetInventoryItems` es el pool del grid 8×8 y su índice de
// celda es `slot - 12` (AddItemToGrid:396): `[0..11]` son las CELDAS 0..11 del
// inventario visible, NO los wear slots. Los 12 slots de equipo viven sólo en
// `CharacterMachine + 536 + 68*slot`.
// Era el fallback de `GetPanelEquipmentSourceItem`, así que se disparaba justo
// al desequipar (slot de CM en -1) y dibujaba en la caja lo que el jugador
// tuviera en las primeras celdas del inventario — la poción en los pants, un
// casco en los anillos, etc.
static ITEM* GetEquippedInventoryItem(int /*index*/)
{
    return nullptr;
}

static ITEM* GetCharacterMachineEquipItem(int index)
{
    if (!CharacterMachine) return nullptr;
    if (index < EQUIPMENT_WEAPON_RIGHT || index > EQUIPMENT_RING_LEFT)
        return nullptr;
    return (ITEM*)((BYTE*)CharacterMachine + 536 + 68 * index);
}

static ITEM* GetCharacterMachineSlotAtOffset(int byteOffset)
{
    if (!CharacterMachine) return nullptr;
    return (ITEM*)((BYTE*)CharacterMachine + byteOffset);
}

static ITEM* GetCharacterMachineEquipSlotByIndex(int slotIdx)
{
    if (!CharacterMachine) return nullptr;
    if (slotIdx < EQUIPMENT_WEAPON_RIGHT || slotIdx > EQUIPMENT_RING_LEFT)
        return nullptr;
    return (ITEM*)((BYTE*)CharacterMachine + 536 + 68 * slotIdx);
}

static ITEM* GetCharacterMachineValidEquipSlot(int slotIdx, bool requireGate)
{
    if (CharacterMachine) {
        ITEM* cm = GetCharacterMachineEquipSlotByIndex(slotIdx);
        if (cm && cm->Type != -1) {
            if (!requireGate || *(DWORD*)((BYTE*)cm + 56) > 0) {
                return cm;
            }
        }
    }
    return nullptr;
}

static bool IsCharacterMachineEquipPointer(const ITEM* slot)
{
    if (!slot || !CharacterMachine) {
        return false;
    }
    const BYTE* p = (const BYTE*)slot;
    const BYTE* base = (const BYTE*)CharacterMachine + 536;
    const BYTE* end = base + 68 * 12;
    return p >= base && p < end;
}

static bool IsVisualEquipSlotOccupied(const ITEM* slot)
{
    if (!slot || slot->Type == -1) {
        return false;
    }
    if (slot->Key > 0) {
        return true;
    }
    if (slot->Durability > 0) {
        return true;
    }
    if (*(const DWORD*)((const BYTE*)slot + 56) > 0) {
        return true;
    }
    return false;
}

static ITEM* GetPanelEquipmentSourceItem(int slotIdx)
{
    ITEM* cm = GetCharacterMachineValidEquipSlot(slotIdx, true);
    if (cm) {
        return cm;
    }

    ITEM* inv = GetEquippedInventoryItem(slotIdx);
    if (IsVisualEquipSlotOccupied(inv)) {
        return inv;
    }

    if (CharacterMachine) {
        cm = GetCharacterMachineEquipSlotByIndex(slotIdx);
        if (IsVisualEquipSlotOccupied(cm)) {
            return cm;
        }
    }
    return inv;
}

static void SetEquipmentSlotPlaceholderColorForIndex(int slotIdx)
{
    ITEM* slot = GetPanelEquipmentSourceItem(slotIdx);
    if (IsVisualEquipSlotOccupied(slot)) {
        InventoryColor_stub(slot);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

// RenderEquipmentBox @ 0x004E25A0 — port FIEL desde IDA (2026-05-02 v2).
//
// Renders los slot decoration backgrounds (placeholder icons como helmet
// outline, weapon outline, etc.) en la INVENTORY panel. Cada slot dibuja
// un bitmap a la posición correcta. Las posiciones y texturas vienen del
// IDA decompile de RenderEquipmentBox.
//
// Layout verificado contra IDA (mismas posiciones que RenderEquipment3D):
//   byte 1080 → (15, 46)   40×40   tex 275  (Pendant)
//   byte 1012 → (115, 46)  60×40   tex 274  (Wings)
//   byte 672  → (75, 46)   40×40   tex 263  (Helmet, skip if class==3)
//   byte 740  → (75, 89)   40×60   tex 264  (Armor)
//   byte 808  → (75, 152)  40×40   tex 265  (Pants/Boots — bottom-CENTER)
//   byte 536  → (15, 89)   40×60   tex 266  (WeaponL)
//   byte 604  → (134, 89)  40×60   tex 276  (WeaponR)
//   byte 876  → (15, 152)  40×40   tex 267  (Gloves — bottom-LEFT)
//   byte 944  → (134, 152) 40×40   tex 268  (Boots — bottom-RIGHT)
//   byte 1148 → (55, 89)   20×20   tex 269  (Ring1)
//   byte 1216 → (55, 152)  20×20   tex 270  (Ring2)
//   byte 1284 → (115, 152) 20×20   tex 270  (Necklace, same tex as Ring2)
//
// FIX 2026-05-02 v2: la versión vieja usaba `_DAT_00552c04/c10/...` globals
// que no estaban inicializados con los valores correctos → boxes se pintaban
// en posiciones equivocadas (cuadro gris al lado de armor que el user reportó).
// Ahora todas las posiciones son hardcoded literales matching IDA exactamente.
void __stdcall RenderEquipmentBox_stub(void) {
    EnableAlphaTest(true);

    float sx = (float)(int)DAT_07ea5288;  // InventoryStartX
    float sy = (float)(int)DAT_07ea5284;  // InventoryStartY
    float rowTop = sy + _DAT_00552c18;
    float rowMid = sy + _DAT_00552c0c;
    float rowBottom = sy + _DAT_00552c08;
    float colLeft = sx + _DAT_00552834;
    float colRing = sx + _DAT_005524a0;
    float colBody = sx + _DAT_00552c10;
    float colPendant = sx + _DAT_00552c14;
    float colRight = sx + _DAT_00552c04;

    // Helper/Pet slot 8.
    SetEquipmentSlotPlaceholderColorForIndex(8);
    GL_DrawTexture(0x113, colLeft, rowTop, 40.0f, 40.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(7);
    GL_DrawTexture(0x112, colPendant, rowTop, 60.0f, 40.0f, 0.0f, 0.0f, 0.9375f, 0.625f, 1, 1);

    // 2026-08-08 FIX (el MG seguía mostrando la caja del casco): el gate leía
    // `*(short*)CharacterAttribute` — o sea un SHORT en el offset 0 — en vez del
    // byte de clase. Per IDA RenderEquipmentBox (0x4E25A0 L178) es
    //     if ( (*(BYTE *)(CharacterAttribute + 11) & 7) != 3 )
    // Mismo campo que usa RenderEquipment3D para saltear el ITEM del casco.
    if (!CharacterAttribute ||
        ((*(BYTE*)((BYTE*)CharacterAttribute + 11)) & 7) != 3) {
        SetEquipmentSlotPlaceholderColorForIndex(2);
        GL_DrawTexture(0x107, colBody, rowTop, 40.0f, 40.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);
    }

    // 2026-08-08 FIX (el recuadro de la armadura salía 10 px más arriba y las
    // pants parecían caerse por debajo): acá se restaba `_DAT_00552488` (=10) a
    // la Y de la CAJA. Ese -10 es del ITEM, no del recuadro:
    //   RenderEquipmentBox  (0x4E25A0 L248-252): `v57 = v49 + 89.0;`
    //                                            RenderBitmap(264, v56, v57, 40, 60)
    //   RenderEquipment3D   (0x4E3100):          `syc = v45 + 89.0 - 10.0;`
    // O sea la caja va en +89 y el item se dibuja 10 px más arriba dentro de
    // ella (que es lo que hace RenderEquipmentPart3D_stub, y eso queda igual).
    SetEquipmentSlotPlaceholderColorForIndex(3);
    GL_DrawTexture(0x108, colBody, rowMid, 40.0f, 60.0f, 0.0f, 0.0f, 0.625f, 0.9375f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(4);
    GL_DrawTexture(0x109, colBody, rowBottom, 40.0f, 40.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(0);
    GL_DrawTexture(0x10A, colLeft, rowMid, 40.0f, 60.0f, 0.0f, 0.0f, 0.625f, 0.9375f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(1);
    GL_DrawTexture(0x114, colRight, rowMid, 40.0f, 60.0f, 0.0f, 0.0f, 0.625f, 0.9375f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(5);
    GL_DrawTexture(0x10B, colLeft, rowBottom, 40.0f, 40.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(6);
    GL_DrawTexture(0x10C, colRight, rowBottom, 40.0f, 40.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(9);
    GL_DrawTexture(0x10D, colRing, rowMid, 20.0f, 20.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(10);
    GL_DrawTexture(0x10E, colRing, rowBottom, 20.0f, 20.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    SetEquipmentSlotPlaceholderColorForIndex(11);
    GL_DrawTexture(0x10E, colPendant, rowBottom, 20.0f, 20.0f, 0.0f, 0.0f, 0.625f, 0.625f, 1, 1);

    glColor3f(1.0f, 1.0f, 1.0f);
}

// RenderEquipmentPart3D @ 0x004E2E40 (~201 lines) — renders one 3D equipment piece
void __cdecl RenderEquipmentPart3D_stub(int Index, float sx, float sy, float Width, float Height) {
    if (Index < EQUIPMENT_WEAPON_RIGHT || Index > EQUIPMENT_RING_LEFT) return;

    ITEM* src = GetPanelEquipmentSourceItem(Index);
    if (!IsVisualEquipSlotOccupied(src)) return;

    if (IsCharacterMachineEquipPointer(src)) {
        int itemType = (int)src->Type;
        int level = *(int*)((BYTE*)src + 4);
        unsigned int option = *(unsigned char*)((BYTE*)src + 27);
        RenderItem3D(sx, sy, Width, Height, itemType, level, (int)option, 0, false);
    } else {
        RenderItem3D(sx, sy, Width, Height,
                     (int)src->Type, src->Level, src->Option1,
                     0, false);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

// RenderEquipment3D @ 0x004E3100 — port FIEL desde IDA decompile (2026-05-01).
//
// Layout en CharacterMachine (byte offsets verificados contra IDA + capturas
// del cliente original 2026-05-01 v2 — labels corregidos):
//   536  WeaponL    (15, 89)  40×60
//   604  WeaponR    (134,89)  40×60
//   672  HELMET     (75, 46)  40×40   ← antes mal-labeled "Pendant"
//   740  Armor      (75, 79)  40×60   ← y = 89-10 = 79
//   808  PANTS      (75,152)  40×40   ← antes mal-labeled "Boots"
//   876  GLOVES     (15,152)  40×40   ← antes mal-labeled "Pants"
//   944  BOOTS      (134,152) 40×40   ← antes mal-labeled "Gloves"
//   1012 Wings      (115,46)  60×40
//   1080 PENDANT/Pet (15, 46) 40×40   ← antes mal-labeled "Helmet"
//   1148 Ring1      (55, 89)  20×20   via RenderEquipmentPart3D(9)
//   1216 Ring2      (55,152)  20×20   via RenderEquipmentPart3D(10)
//   1284 Necklace   (115,152) 20×20   via RenderEquipmentPart3D(11)
//
// Cada ITEM (68 bytes) en CM:
//   +0  Type (short),  +4 Level (DWORD),  +27 Option (byte),  +56 Durability (int)
//
// Nota: la versión previa tenía labels mezclados (decía "Armor 0xfd" pero leía
// byte 1012 = Wings real, etc) y screen positions desde DAT_ globals con valores
// distintos a los de IDA. Esto causaba que en la pantalla aparecieran items en
// posiciones equivocadas (helmet apilado con rings, pendant donde casco, etc).
void __stdcall RenderEquipment3D_stub(void) {
    if (!CharacterAttribute) return;

    float sx = (float)(int)DAT_07ea5288;
    float sy = (float)(int)DAT_07ea5284;
    float rowTop = sy + _DAT_00552c18;
    float rowMid = sy + _DAT_00552c0c;
    float rowBottom = sy + _DAT_00552c08;
    float colLeft = sx + _DAT_00552834;
    float colRing = sx + _DAT_005524a0;
    float colBody = sx + _DAT_00552c10;
    float colPendant = sx + _DAT_00552c14;
    float colRight = sx + _DAT_00552c04;

    InventoryColor_stub(GetPanelEquipmentSourceItem(8));
    RenderEquipmentPart3D_stub(8, colLeft, rowTop, 40.0f, 40.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(7));
    RenderEquipmentPart3D_stub(7, colPendant, rowTop, 60.0f, 40.0f);

    if (((*(BYTE*)((BYTE*)CharacterAttribute + 11)) & 7) != 3) {
        InventoryColor_stub(GetPanelEquipmentSourceItem(2));
        RenderEquipmentPart3D_stub(2, colBody, rowTop, 40.0f, 40.0f);
    }

    InventoryColor_stub(GetPanelEquipmentSourceItem(3));
    RenderEquipmentPart3D_stub(3, colBody, rowMid - _DAT_00552488, 40.0f, 60.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(4));
    RenderEquipmentPart3D_stub(4, colBody, rowBottom, 40.0f, 40.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(0));
    RenderEquipmentPart3D_stub(0, colLeft, rowMid, 40.0f, 60.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(1));
    RenderEquipmentPart3D_stub(1, colRight, rowMid, 40.0f, 60.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(5));
    RenderEquipmentPart3D_stub(5, colLeft, rowBottom, 40.0f, 40.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(6));
    RenderEquipmentPart3D_stub(6, colRight, rowBottom, 40.0f, 40.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(9));
    RenderEquipmentPart3D_stub(9, colRing, rowMid, 20.0f, 20.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(10));
    RenderEquipmentPart3D_stub(10, colRing, rowBottom, 20.0f, 20.0f);

    InventoryColor_stub(GetPanelEquipmentSourceItem(11));
    RenderEquipmentPart3D_stub(11, colPendant, rowBottom, 20.0f, 20.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
}

// RenderItemsBoxes @ 0x004E37B0 (~49 lines) — render 2D item grid boxes
void __cdecl RenderItemsBoxes_stub(float fPosX, float fPosY, DWORD Inventory, int iMaxWidth, int iMaxHeight) {
    // 0x004E37B0 — render 2D item grid boxes
    // For each cell: InventoryColor sets GL color, then RenderBitmap draws the cell background.
    // Empty cell (Type==-1) uses texture 0x115 with UV 1.0x1.0.
    // Occupied cell uses texture 0x116 with UV 0.625x0.667.
    // Cell size: 20x20 px, stride 0x14 (20) px per cell.
    // Row stride in Inventory: each row is iMaxWidth ITEMs = iMaxWidth * sizeof(ITEM).
    // Ghidra shows Inventory += 0x220 per row => sizeof(ITEM)*iMaxWidth varies but Ghidra
    // hardcodes 0x220 based on a specific grid width. We use pointer arithmetic from ITEM*.

    if (iMaxHeight <= 0) return;

    int pixelY = 0;
    int rowsLeft = iMaxHeight;
    do {
        if (0 < iMaxWidth) {
            int pixelX = 0;
            int colsLeft = iMaxWidth;
            ITEM* p = (ITEM*)Inventory;
            int gridCol = 0;
            do {
                int Texture;
                float uWidth, vHeight;
                if (p->Type == -1) {
                    InventoryColor_stub(p);
                    vHeight = 1.0f;
                    uWidth = 1.0f;
                    Texture = 0x115;
                } else {
                    InventoryColor_stub(p);
                    vHeight = 0.6666667f;
                    uWidth = 0.625f;
                    Texture = 0x116;
                }
                GL_DrawTexture(Texture, (float)pixelX + fPosX, (float)pixelY + fPosY,
                             20.0f, 20.0f, 0.0f, 0.0f, uWidth, vHeight, '\x01', '\x01');
                p = p + 1;
                ++gridCol;
                pixelX = pixelX + 0x14;
                colsLeft = colsLeft - 1;
            } while (colsLeft != 0);
        }
        pixelY = pixelY + 0x14;
        Inventory = Inventory + (iMaxWidth * (int)sizeof(ITEM));
        rowsLeft = rowsLeft - 1;
    } while (rowsLeft != 0);
}

// RenderItems3D @ 0x004E38B0 (~130 lines) — render 3D item models in inventory grid
// For each non-empty cell: call RenderItem3D with item dimensions from ItemAttribute.
// If MixItems + _MixState active (1..0x32): render sparkle effects (textures 0x4ce/0x4cf/0x47e).
// Also renders "new item" cyan glow for items marked Color=='c'.
void __cdecl RenderItems3D_stub(float p1, float p2, short* p3, int p4, int p5, char p6) {
    // 0x004E38B0 — Render 3D item models in inventory grid
    // p1=posX, p2=posY, p3=inventory array (short*, stride 0x22 words=0x44 bytes)
    // p4=gridWidth, p5=gridHeight, p6=mode (0=normal 3D render, 1=show arrow count + mix sparkles)
    // ItemAttribute = (ITEM_ATTRIBUTE*)DAT_07d78068, stride 0x40
    // _DAT_005524fc = cell size (20.0f), _DAT_005527dc = arrow count X offset
    // OffsetMixItems = DAT_07ea9848, MixState = DAT_07eaa140
    // Inventory = DAT_07ea5298, WorldTime = DAT_05826e08
    // TradeInventoryStartX = DAT_07ea5290, TradeInventoryStartY = DAT_07ea528c

    ITEM_ATTRIBUTE* itemAttr = (ITEM_ATTRIBUTE*)DAT_07d78068;

    if (p5 > 0) {
        int yOff = 0;
        short* rowPtr = p3;
        for (int row = 0; row < p5; row++) {
            if (p4 > 0) {
                int xOff = 0;
                float y = (float)yOff + p2;
                short* cellPtr = rowPtr;
                for (int col = 0; col < p4; col++) {
                    ITEM* cell = (ITEM*)cellPtr;
                    short itemType = cell->Type;
                    float x = (float)xOff + p1;
                    bool primaryOccupied =
                        (itemType != -1 &&
                         (int)cell->Key > 0 &&
                         (int)cell->x == col &&
                         (int)cell->y == row);
                    if (primaryOccupied) {
                        if (0 < (int)cell->Key) {
                            int idx = (int)itemType;
                            RenderItem3D(x, y,
                                (float)itemAttr[idx].Width * _DAT_005524fc,
                                (float)itemAttr[idx].Height * _DAT_005524fc,
                                idx, cell->Level,
                                (unsigned int)cell->Option1,
                                (int)cell->byColorState, false);

                            if (p6 != '\0' && itemType > 0x1bf && itemType < 0x1c9 &&
                                cell->Durability > 1) {
                                glColor3f(1.0f, 0.9f, 0.7f);
                                RenderNumber2D_stub(x + _DAT_005527dc, y,
                                    (unsigned int)cell->Durability, 9.0f, 10.0f);
                            }
                        }
                        // Mix sparkle effects when mixing
                        if (p6 != '\0' && p3 == (short*)&DAT_07ea9848 &&
                            DAT_07eaa140 > 0 && DAT_07eaa140 < 0x33) {
                            EnableAlphaBlend();
                            unsigned int r1 = rand();
                            unsigned int rMod = r1 & 0x80000003;
                            if ((int)rMod < 0) rMod = (rMod - 1 | 0xfffffffc) + 1;
                            float colorG = (float)(int)(rMod + 4) * _DAT_005524f4;
                            int r2 = rand();
                            float colorR = (float)(r2 % 6 + 6) * _DAT_005524f4;
                            // random offset size
                            int r3 = rand();
                            float sparkSize = (float)(r3 % 10) + _DAT_00552660;
                            int r4 = rand();
                            float fx = (float)(r4 % 0x14) + x;
                            int r5 = rand();
                            float fy = (float)(r5 % 0x14) + y;
                            float mixRot = (float)((__int64)DAT_05826e08 % 100) * 20.0f;
                            glColor3f(colorR, colorG, 0.2f);
                            FUN_005126e0(0x4ce, fx, fy, sparkSize, sparkSize, 0);
                            FUN_005126e0(0x4ce, fx, fy, sparkSize, sparkSize, mixRot);
                            FUN_005126e0(0x4cf, fx, fy, sparkSize * _DAT_00552540, sparkSize * _DAT_00552540, mixRot);
                            FUN_005126e0(0x47e, fx, fy, sparkSize * _DAT_005527d0, sparkSize * _DAT_005527d0, 0);
                            GL_ResetState();  // DisableAlphaBlend
                        }
                    }
                    cellPtr += 0x22;  // stride 0x44 bytes
                    xOff += 0x14;     // 20 pixels per cell
                }
            }
            yOff += 0x14;
            rowPtr += 0x22 * p4;
        }
    }

    // "New item" cyan glow: scan Inventory array for items with Color=='c' (0x63)
    if (p3 == (short*)&DAT_07ea5298) {
        short* scanPtr = (short*)&DAT_07ea5298;
        for (int s = 0; s < 64; s++) {
            ITEM* cell = (ITEM*)scanPtr;
            if (cell->Type != -1 && (int)cell->Key > 0 &&
                (int)cell->x == (s % 8) && (int)cell->y == (s / 8) &&
                cell->Color == 'c') {
                glColor3f(0.0f, 1.0f, 1.0f);
                float glowX = (float)((unsigned int)cell->x * 0x14)
                              + (float)DAT_07ea5290 + _DAT_00552834;
                float sinVal = (float)sin((double)((float)DAT_05826e08 * _DAT_005529cc));
                float glowY = (float)((unsigned int)cell->y * 0x14)
                              + (float)DAT_07ea528c + _DAT_00552938 + sinVal + _DAT_00552660;
                GL_DrawTexture(9, glowX, glowY, 24.0f, 24.0f, 0.0f, 0.4f, 1.0f, 1.0f, 1, 1);
                glColor3f(1.0f, 1.0f, 1.0f);
                // Text rendering for new item label — simplified
                // (original uses SelectObject, RenderText with bold font)
            }
            scanPtr += 0x22;
        }
    }

    // Advance MixState counter
    if (DAT_07eaa140 > 1) {
        DAT_07eaa140 = DAT_07eaa140 + 1;
    }
}
