// Item_Inventory.cpp — port directo desde MU 0.52 (sub_404830 / sub_425850 / sub_428C30)
//
// 2026-04-30: la versión 0.97 tenía estos handlers como stubs vacíos / no-op,
// por lo que el server enviaba el packet F3/10 (inventory-snapshot) y el cliente
// lo descartaba — ningún item aparecía en el grid.
//
// El código de 0.52 está libre de anti-tamper / hash-table noise. Estructuras
// y offsets son idénticos a 0.97 (ITEM stride 68 B, grid 8×8 = 64 slots).
//
// Diferencias con 0.52 portadas a 0.97:
//   - 0.52 tenía 12 equipment slots + 64 inventory slots. 0.97 mantiene los 64
//     slots de inventario en OffsetInventoryItems[], pero las slots de
//     equipment viven en CharacterMachine (cm[0x10e]=helm, cm[0xfd]=armor, …).
//   - 0.52 stride per packet item: 4 bytes [type, level, gridX, gridY].
//     0.97 expande con durabilidad / serial / option byte; el server-emu mu-emu
//     que usamos manda 12 bytes per item — formato compatible verifica byte 0
//     antes de leer cells extras.
//   - Item dims (W×H): 0.52 los lee de byte_7A5999E/F (stride 45). 0.97 los
//     guarda en ItemAttribute (struct stride 64). Aquí accedemos via
//     `ItemAttribute[type].Width / .Height`.

#include "stdafx.h"
#include "globals.h"
#include "structs.h"        // ITEM, ITEM_ATTRIBUTE (también en GameStructs.h — usar uno solo)
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
int __cdecl FUN_0047b110(BYTE* Item);

// ── External handles -─────────────────────────────────────────────────────────
//
// OffsetInventoryItems[64 * 68] está en HUD_Pass3.cpp como `BYTE` array sized
// para el grid 8×8.  Cada slot es un ITEM (struct, 68 B).
extern "C" BYTE OffsetInventoryItems[];
extern "C" void HeroEquipWatchdog(int c);
extern int DAT_00559c60;
extern int DAT_00559c64;
extern int DAT_00559c68;
extern "C" void __cdecl InsertInventoryItem(BYTE* Inv, int Width, int Height,
                                            int Index, BYTE* Item, bool First);

// ItemAttribute base — `(ITEM_ATTRIBUTE*)DAT_07d78068` per HUD_Pass2.cpp:60.
// La tabla la carga `Item_LoadFromBmd` desde Data\Local\Item.bmd al startup.
// Si no está cargada (ItemAttribute=0 o type fuera de rango), defaulteamos
// a 1×1 (worst case visualmente, pero al menos el item aparece).
extern "C" int DAT_07d78068;
extern "C" DWORD g_ItemAttribute_Backup;   // src/globals.cpp — recovery pointer

static inline ITEM_ATTRIBUTE* Item_GetAttribute(int type)
{
    // 2026-07-27 CRASH-FIX (0xC0000005 en Item_GetWidth): algún writer corrompe
    // DAT_07d78068 (ItemAttribute base) a un valor chico (~0/1). El check `!tbl`
    // NO lo atrapaba (tbl=1 != null) → devolvía `1 + type*0x40` = puntero NULL+off
    // → los callers (Item_GetWidth/Height, ItemData_FillStats) crasheaban al
    // dereferenciar. Pasaba al comprar/mover con el inventario lleno (más
    // inserts = más chances de pegarle a la ventana corrupta). Mismo watchdog
    // que FUN_004d23b0 / Inventory_DropItemEx: restauramos desde el backup.
    unsigned int p = (unsigned int)(uintptr_t)DAT_07d78068;
    if ((p < 0x100000u || p >= 0x80000000u)
        && g_ItemAttribute_Backup >= 0x100000u
        && g_ItemAttribute_Backup < 0x80000000u)
    {
        DAT_07d78068 = (int)g_ItemAttribute_Backup;
        p = g_ItemAttribute_Backup;
    }
    if (p < 0x100000u || p >= 0x80000000u) return nullptr;  // sin backup válido → no crash
    if (type < 0 || type > 0xFFF) return nullptr;
    return &((ITEM_ATTRIBUTE*)(uintptr_t)p)[type];
}

static int FindFirstInventoryTypeInRange(int typeMin, int typeMax)
{
    if (typeMin > typeMax) {
        int tmp = typeMin;
        typeMin = typeMax;
        typeMax = tmp;
    }
    ITEM* g = (ITEM*)OffsetInventoryItems;
    for (int i = 0; i < 64; ++i) {
        if (g[i].Type >= typeMin && g[i].Type <= typeMax) {
            return g[i].Type;
        }
    }
    return -1;
}

static int FindFirstInventoryTypeExact(const int* types, int count)
{
    ITEM* g = (ITEM*)OffsetInventoryItems;
    for (int idx = 0; idx < count; ++idx) {
        int wanted = types[idx];
        for (int i = 0; i < 64; ++i) {
            if (g[i].Type == wanted) {
                return wanted;
            }
        }
    }
    return -1;
}

extern "C" void __cdecl SeedQuickPotionTypesFromInventory(void);
void __cdecl SeedQuickPotionTypesFromInventory()
{
    // 97k HUD order in IDA (item group 14):
    //   slot 0 -> life group (449..451)
    //   slot 1 -> mana group (452..454)
    //   slot 2 -> auxiliary group
    DAT_00559c60 = 449;
    DAT_00559c64 = 452;
    DAT_00559c68 = 456;

    int hpType = FindFirstInventoryTypeInRange(451, 449);
    if (hpType == -1) hpType = FindFirstInventoryTypeInRange(448, 448);
    if (hpType != -1) DAT_00559c60 = hpType;

    int mpType = FindFirstInventoryTypeInRange(454, 452);
    if (mpType == -1) mpType = FindFirstInventoryTypeInRange(451, 449);
    if (mpType != -1) DAT_00559c64 = mpType;

    // Keep the auxiliary slot on real consumables only.
    // Town Portal Scroll (458) is intentionally excluded from the HUD hotbar.
    static const int kAuxTypes[] = { 468, 457, 456 };
    int auxType = FindFirstInventoryTypeExact(kAuxTypes, 3);
    if (auxType != -1) DAT_00559c68 = auxType;
}

extern "C" {
    extern short g_HeroEquipStash_LH, g_HeroEquipStash_RH;
    extern short g_HeroEquipStash_Wing, g_HeroEquipStash_Helper, g_HeroEquipStash_Pendant;
    extern short g_HeroEquipStash_RingR, g_HeroEquipStash_RingL;
    extern short g_HeroEquipStash_Body[6];
    extern unsigned char g_HeroEquipStash_LHLvl, g_HeroEquipStash_RHLvl;
    extern unsigned char g_HeroEquipStash_WingLvl, g_HeroEquipStash_HelperLvl, g_HeroEquipStash_PendantLvl;
    extern unsigned char g_HeroEquipStash_RingRLvl, g_HeroEquipStash_RingLLvl;
    extern unsigned char g_HeroEquipStash_LHDur, g_HeroEquipStash_RHDur;
    extern unsigned char g_HeroEquipStash_WingDur, g_HeroEquipStash_HelperDur, g_HeroEquipStash_PendantDur;
    extern unsigned char g_HeroEquipStash_RingRDur, g_HeroEquipStash_RingLDur;
    extern unsigned char g_HeroEquipStash_BodyDur[6];
    extern unsigned char g_HeroEquipStash_BodyLvl[6];
    extern unsigned char g_HeroEquipStash_BodyOpt1[6];
    extern unsigned char g_HeroEquipStash_BodyOpt2[6];
    extern unsigned char g_HeroEquipStash_BodyOpt3[6];
    extern unsigned char g_HeroEquipStash_LHOpt, g_HeroEquipStash_RHOpt;
    extern unsigned char g_HeroEquipStash_WingOpt, g_HeroEquipStash_HelperOpt, g_HeroEquipStash_PendantOpt;
    extern unsigned char g_HeroEquipStash_RingROpt, g_HeroEquipStash_RingLOpt;
    extern unsigned char g_HeroEquipStash_LHOpt2, g_HeroEquipStash_RHOpt2;
    extern unsigned char g_HeroEquipStash_WingOpt2, g_HeroEquipStash_HelperOpt2, g_HeroEquipStash_PendantOpt2;
    extern unsigned char g_HeroEquipStash_RingROpt2, g_HeroEquipStash_RingLOpt2;
    extern unsigned char g_HeroEquipStash_LHOpt3, g_HeroEquipStash_RHOpt3;
    extern unsigned char g_HeroEquipStash_WingOpt3, g_HeroEquipStash_HelperOpt3, g_HeroEquipStash_PendantOpt3;
    extern unsigned char g_HeroEquipStash_RingROpt3, g_HeroEquipStash_RingLOpt3;
    extern int g_HeroEquipStash_Valid;
}

static void SyncHeroEquipStashFromEquipmentSlot(int slotIdx, int type, BYTE optByte, BYTE durability, BYTE byteHi, BYTE extByte)
{
    switch (slotIdx) {
    case 0:
        g_HeroEquipStash_LH = (short)type;
        g_HeroEquipStash_LHLvl = optByte;
        g_HeroEquipStash_LHDur = durability;
        g_HeroEquipStash_LHOpt = optByte;
        g_HeroEquipStash_LHOpt2 = byteHi;
        g_HeroEquipStash_LHOpt3 = extByte;
        break;
    case 1:
        g_HeroEquipStash_RH = (short)type;
        g_HeroEquipStash_RHLvl = optByte;
        g_HeroEquipStash_RHDur = durability;
        g_HeroEquipStash_RHOpt = optByte;
        g_HeroEquipStash_RHOpt2 = byteHi;
        g_HeroEquipStash_RHOpt3 = extByte;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6: {
        int bodyIdx = slotIdx - 2; // 0..4 => helm..boots
        g_HeroEquipStash_Body[bodyIdx] = (short)type;
        g_HeroEquipStash_BodyLvl[bodyIdx] = optByte;
        g_HeroEquipStash_BodyDur[bodyIdx] = durability;
        g_HeroEquipStash_BodyOpt1[bodyIdx] = optByte;
        g_HeroEquipStash_BodyOpt2[bodyIdx] = byteHi;
        g_HeroEquipStash_BodyOpt3[bodyIdx] = extByte;
        break;
    }
    case 7:
        g_HeroEquipStash_Wing = (short)type;
        g_HeroEquipStash_WingLvl = optByte;
        g_HeroEquipStash_WingDur = durability;
        g_HeroEquipStash_WingOpt = optByte;
        g_HeroEquipStash_WingOpt2 = byteHi;
        g_HeroEquipStash_WingOpt3 = extByte;
        break;
    case 8:
        g_HeroEquipStash_Helper = (short)type;
        g_HeroEquipStash_HelperLvl = optByte;
        g_HeroEquipStash_HelperDur = durability;
        g_HeroEquipStash_HelperOpt = optByte;
        g_HeroEquipStash_HelperOpt2 = byteHi;
        g_HeroEquipStash_HelperOpt3 = extByte;
        break;
    case 9:
        g_HeroEquipStash_Pendant = (short)type;
        g_HeroEquipStash_PendantLvl = optByte;
        g_HeroEquipStash_PendantDur = durability;
        g_HeroEquipStash_PendantOpt = optByte;
        g_HeroEquipStash_PendantOpt2 = byteHi;
        g_HeroEquipStash_PendantOpt3 = extByte;
        break;
    case 10:
        g_HeroEquipStash_RingR = (short)type;
        g_HeroEquipStash_RingRLvl = optByte;
        g_HeroEquipStash_RingRDur = durability;
        g_HeroEquipStash_RingROpt = optByte;
        g_HeroEquipStash_RingROpt2 = byteHi;
        g_HeroEquipStash_RingROpt3 = extByte;
        break;
    case 11:
        g_HeroEquipStash_RingL = (short)type;
        g_HeroEquipStash_RingLLvl = optByte;
        g_HeroEquipStash_RingLDur = durability;
        g_HeroEquipStash_RingLOpt = optByte;
        g_HeroEquipStash_RingLOpt2 = byteHi;
        g_HeroEquipStash_RingLOpt3 = extByte;
        break;
    default:
        break;
    }
}

static inline int Item_GetWidth(int type)
{
    ITEM_ATTRIBUTE* a = Item_GetAttribute(type);
    return (a && a->Width  > 0 && a->Width  <= 8) ? (int)a->Width  : 1;
}

static inline int Item_GetHeight(int type)
{
    ITEM_ATTRIBUTE* a = Item_GetAttribute(type);
    return (a && a->Height > 0 && a->Height <= 8) ? (int)a->Height : 1;
}

// ── Item helper: ItemData_FillStats (0.52 sub_42DE30) ─────────────────────────
//
// En 0.52 esta función calcula damage/defense del item según su level y tabla
// base.  Aquí copiamos los stats directamente desde el ItemAttribute table.
static void ItemData_FillStats(ITEM* slot, int level)
{
    if (!slot) return;
    // Pre-compute durability si tenemos el ItemAttribute.
    ITEM_ATTRIBUTE* a = Item_GetAttribute(slot->Type);
    if (a) {
        slot->Defense       = a->Defense;
        slot->MagicDefense  = a->MagicDefense;
        slot->DamageMin     = a->DamageMin;
        slot->DamageMax     = a->DamageMax;
        slot->SuccessfulBlocking = a->DefenseRate;
        slot->WalkSpeed     = a->WalkSpeed;
        slot->WeaponSpeed   = a->AttackSpeed;
        slot->RequireLevel  = a->RequireLevel;
        slot->RequireStrength = a->RequireStrength;
        slot->RequireDexterity = a->RequireAgility;
        slot->RequireEnergy = a->RequireEnergy;
        slot->TwoHand       = a->TwoHand ? 1 : 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AddItemToGrid (port directo de 0.52 sub_425850)
// ─────────────────────────────────────────────────────────────────────────────
//
//   gridBase  — `OffsetInventoryItems` (8×8 grid)  ó  Inventory (otro buffer)
//   gridW/H   — dims del grid (8/8 para inventario; otros valores para
//                trade/warehouse)
//   slotIdx   — celda objetivo:
//                 < 0          → invalid, skip
//                 [0..11]      → equipment slot (en 0.97 va a CharacterMachine,
//                                ver MapEquipmentSlot abajo)
//                 [12..N+11]   → cell index = slotIdx-12 → gridY*W + gridX
//   type      — item type (-1 = empty, no escribe nada)
//   level     — item level
//   optByte   — option byte (el 0.52 usa 1 valor; 0.97 mete 4 bytes en
//                Option1+SpecialNum+SpecialValue, aquí tomamos el primero)
//
// En 0.52 el equipment se guardaba en el mismo gridBase con índice <12. En
// nuestro 0.97 los slots de equipment NO viven en gridBase sino en
// CharacterMachine — los manejamos aparte vía MapEquipmentSlot.
// ─────────────────────────────────────────────────────────────────────────────

// CharacterMachine equipment layout in the client source is contiguous:
//   CharacterMachine + 536 + 68 * slot
// for slot 0..11.  This is the real 97k path used by the client source and
// matches the render-side helpers in ItemManager.cpp / PrintPlayer.cpp.
// Slot order:
//   0 WeaponR, 1 WeaponL, 2 Helm, 3 Armor, 4 Pants, 5 Gloves,
//   6 Boots, 7 Wing, 8 Helper, 9 Amulet, 10 RingR, 11 RingL.
static inline int GetEquipSlotByteOffset(int slotIdx)
{
    return 536 + (68 * slotIdx);
}

static void WriteEquipmentSlot(int slotIdx, int type, int level, BYTE optByte, BYTE durability, BYTE byteHi, BYTE extByte)
{
    // BUG-FIX 2026-05-01: el código previo mezclaba DWORD indexing y byte
    // offsets. Resultado: Type se escribía bien (al inicio), pero Durability
    // (cm[off + 0xE] como DWORD index) caía en posición wrong dentro del
    // siguiente slot. RenderBrokenItem leía durability=0 → "Light Saber (0/69)"
    // siempre.
    //
    // Cada slot es un struct ITEM (68 bytes) en CharacterMachine.
    // Layout real: CharacterMachine + 536 + 68*slot.
    // Los offsets de campo dentro de ITEM (en bytes desde slot start):
    //   +0  Type (short)
    //   +4  Level (int)
    //   +26 Durability (BYTE)
    //   +27 Option1 (BYTE)
    BYTE* cm = (BYTE*)(uintptr_t)DAT_07cf1ffc;
    if (!cm) return;
    if (slotIdx < 0 || slotIdx >= 12) return;

    int byteOff = GetEquipSlotByteOffset(slotIdx);
    BYTE* slot = cm + byteOff;

    if (type < 0) {
        memset(slot, 0, sizeof(ITEM));
        *(short*)(slot + 0)  = -1;
        *(DWORD*)(slot + 56) = 0;
        SyncHeroEquipStashFromEquipmentSlot(slotIdx, -1, 0, 0, 0, 0);
        if (DAT_005615c0 == 5 && DAT_07abf5d8) {
            HeroEquipWatchdog((int)(uintptr_t)DAT_07abf5d8);
        }
        return;
    }

    memset(slot, 0, sizeof(ITEM));
    *(short*)(slot + 0)  = (short)type;
    // 2026-05-08: BUG-FIX +N glow on EQUIPMENT slots.
    // Same fix as AddItemToGrid: store the RAW Option byte (level<<3 encoded)
    // so the render-side shift `(Level >> 3) & 0xF` extracts the correct +N.
    // Previously we stored decoded `level` (0-15) → render shifted again →
    // +9 → +1, no glow. Equipment uses the same render path (sub_4E38B0 /
    // Render_HotbarItems3D → RenderItem3D → RenderObjectScreen).
    *(int*)(slot + 4)    = (int)optByte;
    *(BYTE*)(slot + 26)  = durability;
    // FIX 2026-07-21 — Option1 llevaba `optByte` (= Attribute1, el byte de
    // nivel).  Los flags de EXCELLENT viven en Attribute2 (= byteHi = Item[3]):
    // ItemConvert (0x47B910) hace `iItemExcel = Attribute2 & 63`, y esa es la
    // MISMA mascara 0x3F que testean CalcMaxDurability (0x4C45C0) y
    // RenderItemInfo sobre ip->Option1.
    // Sintomas: los anillos/pendants excellent mostraban durabilidad maxima 15
    // puntos MENOS (no se aplicaba el bonus +15 de excellent) y les faltaba el
    // prefijo "Excelente" en el nombre.
    // Las opciones excellent SI se veian porque salen de Special[]/SpecialValue[],
    // que ItemConvert llena aparte — y a esa funcion los argumentos le llegaban
    // bien (ver la llamada FUN_0047b910(slot, optByte, byteHi) mas abajo).
    *(BYTE*)(slot + 27)  = byteHi;
    *(DWORD*)(slot + 56) = durability ? (DWORD)durability : 1u;
    *(BYTE*)(slot + 60)  = byteHi;
    *(BYTE*)(slot + 61)  = extByte;
    ItemData_FillStats((ITEM*)slot, level);
    ((ITEM*)slot)->Level = (int)optByte;
    FUN_0047b910((int)(uintptr_t)slot, (int)optByte, (int)byteHi);
    // Slot ocupado real del equipo. En la base más nueva esto vive en Number;
    // en nuestra estructura 97k lo espejamos en Key.
    ITEM* equip = (ITEM*)slot;
    equip->Key = (*(DWORD*)(slot + 56) > 0) ? 1 : 0;
    equip->x = (BYTE)slotIdx;
    equip->y = 0;
    equip->Color = 0;
    SyncHeroEquipStashFromEquipmentSlot(slotIdx, type, optByte, durability, byteHi, extByte);
    g_HeroEquipStash_Valid = 1;
    if (DAT_005615c0 == 5 && DAT_07abf5d8) {
        HeroEquipWatchdog((int)(uintptr_t)DAT_07abf5d8);
    }
}

extern "C" void __cdecl AddItemToGrid(BYTE* gridBase, int gridW, int gridH,
                                       int slotIdx, int type, int level,
                                       BYTE optByte, BYTE fillFlag, BYTE durability, BYTE byteHi, BYTE extByte)
{
    if (type == 0xFF || type < 0) return;            // empty marker
    if (!gridBase) return;
    if (gridW <= 0 || gridH <= 0) return;

    bool isMainInventory = (gridBase == &OffsetInventoryItems[0]);

    // Equipment slot path
    if (isMainInventory && slotIdx >= 0 && slotIdx < 12) {
        WriteEquipmentSlot(slotIdx, type, level, optByte, durability, byteHi, extByte);
        return;
    }

    // Inventory cell path
    int cellIdx = isMainInventory ? (slotIdx - 12) : slotIdx;
    if (cellIdx < 0 || cellIdx >= gridW * gridH) return;

    int gridX = cellIdx % gridW;
    int gridY = cellIdx / gridW;

    int width  = Item_GetWidth(type);
    int height = Item_GetHeight(type);

    if (gridX + width  > gridW) width  = gridW - gridX;
    if (gridY + height > gridH) height = gridH - gridY;
    if (width <= 0 || height <= 0) return;

    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int cx = gridX + dx;
            int cy = gridY + dy;
            ITEM* slot = (ITEM*)(gridBase + (cy * gridW + cx) * sizeof(ITEM));

            memset(slot, 0, sizeof(ITEM));
            slot->Type       = (short)type;
            slot->Level      = (int)optByte;
            // BUG-FIX 2026-05-01: Durability era el byteOpt (level/option byte)
            // → broken-item warning falso. Ahora usa el byte real del packet.
            slot->Durability = durability;
            slot->Option1    = byteHi;   // Attribute2 — ver nota en WriteEquipmentSlot
            slot->Unknown    = byteHi;
            slot->byColorState = extByte;
            slot->x          = (BYTE)gridX;
            slot->y          = (BYTE)gridY;
            slot->Key        = (dx == 0 && dy == 0) ? 1u : 0u;  // primary cell flag
            slot->Color      = 0;

            ItemData_FillStats(slot, level);
            slot->Level      = (int)optByte;
            FUN_0047b910((int)(uintptr_t)slot, (int)optByte, (int)byteHi);

            // 2026-05-08: BUG-FIX item +N glow.
            // sub_4E38B0 pasa `*(int*)(slot+4)` (= slot->Level int) como param_6
            // a FUN_004e1be0 → RenderObjectScreen, que extrae el level con
            // `(param_6 >> 3) & 0xF`. Si Level está pre-decoded (0-15), el
            // shift en RenderObjectScreen produce (9>>3)=1 → no glow.
            // ItemData_FillStats(level) acaba de setear Level = decoded —
            // override aquí con el byteOpt RAW para preservar el shift chain.
        }
    }

    if (fillFlag) {
        // Equivalent of 0.52's `sub_434E60(off_46F064)` — char stat recompute.
        // We don't have a direct port; trigger CharData_RecalcStats if available.
        // (Función ya existe en UI/UI.cpp como FUN_0042xxxx; safe no-op si falta.)
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Recv_Inventory — port de 0.52 sub_404830 (opcode F3/10)
// ─────────────────────────────────────────────────────────────────────────────
//
// Layout 0.52: [C1][len][F3][10][reserved][count]{[type][level][gridX][gridY]}*count
//   stride por item = 4 bytes.
//
// Layout 0.97 (mu-emu / season-1 server):
//   [C1][len][F3][10][reserved][count]{[slotIdx][itemBytes...]}*count
//   itemBytes = 12 bytes típicamente:
//     [0]  = type (low)
//     [1]  = level | option1
//     [2]  = durability
//     [3]  = special bytes / serial
//     [4-11] = options / set / harmony / excellent
//
// Detección híbrida: si len-6 = count*4 → 0.52 layout; si = count*12 → 0.97.
// Si el server manda otro stride hacemos best-effort skip.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void __cdecl Recv_Inventory(const BYTE* Msg)
{
    if (!Msg) return;

    // Reset full inventory grid (8×8).
    {
        ITEM* g = (ITEM*)OffsetInventoryItems;
        for (int i = 0; i < 64; ++i) {
            memset(&g[i], 0, sizeof(ITEM));
            g[i].Type = -1;
        }
    }

    // Reset equipment slots in CharacterMachine.
    {
        BYTE* cm = (BYTE*)(uintptr_t)DAT_07cf1ffc;
        if (cm) {
            for (int s = 0; s < 12; ++s) {
                int off = GetEquipSlotByteOffset(s);
                BYTE* slot = cm + off;
                memset(slot, 0, sizeof(ITEM));
                *(short*)slot = -1;
            }
        }
    }

    // Parse header — check both C1 and C2 framing.
    //
    // Real layout (verificado vía hex dump del server-emu del user):
    //   [C1][len][F3][10][count]{[slotIdx][typeLo][levelOpts][durability][typeHi|exc]}*count
    //   stride = 5 bytes/item, count at Msg[4] (sin byte reserved)
    //
    // Encoded item (mu-emu/Webzen 0.97D format):
    //   byte 0: type & 0xFF              ← low 8 bits del type
    //   byte 1: (level<<3) | (skill?0x80:0) | (luck?0x04:0) | (options & 0x03)
    //   byte 2: durability
    //   byte 3: ((type>>4) & 0xF0) | (excellent & 0x0F)
    bool isC2 = (Msg[0] == 0xC2);
    int  hdrLen = isC2 ? 4 : 3;                      // [C1][len][F3] = 3, [C2][len_hi][len_lo][F3] = 4
    int  totalLen = isC2
                  ? ((Msg[1] << 8) | Msg[2])
                  :  Msg[1];

    // Body starts AFTER F3 + sub-opcode byte.
    int  bodyStart  = hdrLen + 1;                    // skip sub-opcode (0x10) → Msg[4]
    BYTE count      = Msg[bodyStart];                // count at Msg[4]
    int  itemsStart = bodyStart + 1;                 // first item byte
    int  stride     = 5;                             // mu-emu 0.97D stride

    {
        char b[160];
        wsprintfA(b, "NET:    F3/10 Inventory count=%d totalLen=%d stride=%d hdrLen=%d isC2=%d",
                  (int)count, totalLen, stride, hdrLen, (int)isC2);
        DbgLogPublic(b);
    }

    // [DIAG] Raw packet hex dump (first 64 bytes) to figure out actual server layout.
    {
        char b[400];
        int p = 0;
        int dumpLen = totalLen > 64 ? 64 : totalLen;
        if (dumpLen > 0) {
            p += wsprintfA(b + p, "NET:    F3/10 RAW: ");
            for (int i = 0; i < dumpLen && p < 380; ++i) {
                p += wsprintfA(b + p, "%02X ", Msg[i]);
            }
            DbgLogPublic(b);
        }
    }

    if (count == 0) return;

    for (int i = 0; i < (int)count; ++i) {
        BYTE* rec = (BYTE*)(Msg + itemsStart + i * stride);
        int slotIdx = rec[0];
        BYTE* item = rec + 1;
        if (item[0] == 0xFF)
            continue;
        InsertInventoryItem(OffsetInventoryItems, 8, 8, slotIdx, item, true);
    }

    SeedQuickPotionTypesFromInventory();

    if (DAT_005615c0 == 5 && DAT_07abf5d8) {
        HeroEquipWatchdog((int)(uintptr_t)DAT_07abf5d8);
    }

    // Ahora que el grid está poblado, abrir la ventana lógicamente: el
    // cliente original setea InventoryOpened=1 al recibir este packet (el
    // server lo manda solo cuando el server lo decide).  Lo dejamos OFF —
    // el user abre con V/I keys.  Solo logueamos para confirmar arrival.
    DbgLogPublic("NET:    F3/10 done - inventory populated");
}

// ─────────────────────────────────────────────────────────────────────────────
// Inventory_Open / Inventory_Close — opcodes 0x55 / 0x54 (server-driven)
// ─────────────────────────────────────────────────────────────────────────────
//
// En 0.52:
//   sub_407BD0 (op 0x55): byte_46F378=0; byte_7A51AFE=1; dword_7A51B00=1;
//                          byte_79CD7AB=1; sub_419DF0(); dword_46F388=8;
//                          dword_46F37C=0; *(WORD*)(off_46F064+388)=999;
//   sub_407BA0 (op 0x54): clears every flag, dword_7A51B00=0
//
// En 0.97 el flag InventoryOpened está unificado con DAT_07eaa117 (per
// nuestra refactor).  Estos handlers simplemente lo flippean.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void __cdecl Recv_InventoryOpen(const BYTE* /*Msg*/)
{
    DAT_07eaa117 = 1;                               // InventoryOpened = true
    DbgLogPublic("NET:    0x55 server requested Inventory_Open");
}

extern "C" void __cdecl Recv_InventoryClose(const BYTE* /*Msg*/)
{
    DAT_07eaa117 = 0;                               // InventoryOpened = false
    DbgLogPublic("NET:    0x54 server requested Inventory_Close");
}
