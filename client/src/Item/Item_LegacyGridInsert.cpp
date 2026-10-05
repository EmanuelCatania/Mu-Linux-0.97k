#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void __cdecl operator_delete(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// InsertInventoryItem @ 0x004CC660 — InsertInventoryItem(ITEM* Inv, int Width,
//   int Height, int Index, BYTE* Item, bool First)  (1945 bytes, IDA).
//
// IDA decomp ships with a `// local variable allocation has failed` warning,
// so the visible body is mostly the anti-tamper hash-table tail (CharacterMachine
// ref-count + XOR encrypt around `First` branch). The actual grid-placement
// core was lost in the decompile — but cross-referencing the call sites
// (Net_Process.cpp cases 0x22 / 0x32 / 0x39 / F3-14) plus the parallel 0.52
// port `AddItemToGrid` in Item_Inventory.cpp gives us the real semantics:
//
//   - Convert item type (Item[0] + 2*(Item[3] & 0x80)). Skip if 0xFF (empty).
//   - Look up width/height from ItemAttribute. For 0.97 we accept the
//     simplification of 1×1 placement when ItemAttribute table is unavailable.
//   - Compute slot offset: (Index * sizeof(ITEM_RAW)) within the inventory
//     buffer. ITEM_RAW stride is 0x44 (= 68 bytes) per the binary format that
//     server emits in opcode 0x22 / 0x32 / 0xF3-14.
//   - Copy 12 bytes of raw item data into the slot (lo mismo que hace
//     Net_Process.cpp para el case 0x22).
//   - If `First==false`, bump CharacterMachine hash-table ref count (anti-
//     tamper, no-op per project policy).
//
// Las copias inline de Net_Process siguen igual; esta es la entrada con el
// nombre y la firma de IDA (grid W/H + flag First).
extern "C" int __cdecl ConvertItemType(BYTE* Item);  // declared above
extern "C" void __cdecl AddItemToGrid(BYTE* gridBase, int gridW, int gridH,
                                      int slotIdx, int type, int level,
                                      BYTE optByte, BYTE fillFlag, BYTE durability, BYTE byteHi, BYTE extByte);
extern "C" BYTE OffsetInventoryItems[];
// IDA: InsertInventoryItem (0x004CC660)
extern "C" void __cdecl InsertInventoryItem(BYTE* Inv, int Width, int Height,
                                            int Index, BYTE* Item, bool First)
{
    if (!Inv || !Item) return;
    bool isMainInventory = (Inv == &OffsetInventoryItems[0]);
    auto clear_slot = [&](int slotIndex) {
        if (slotIndex < 0) return;
        if (isMainInventory && slotIndex < 12) {
            // Los 12 wear slots NO tienen espejo en `OffsetInventoryItems` (ese pool es el
            // grid 8x8): viven en `CharacterMachine + 536 + 68*slot`.
            static const int kEquipOffsets[12] = {
                536, 604, 672, 740, 808, 876, 944, 1012, 1080, 1148, 1216, 1284
            };
            if (DAT_07cf1ffc) {
                BYTE* wear = (BYTE*)(uintptr_t)DAT_07cf1ffc + kEquipOffsets[slotIndex];
                memset(wear, 0, 0x44);
                *(short*)wear = (short)0xFFFF;
                *(int*)(wear + 56) = -1;      // Key: mismo estado que deja el F3/10
            }
            return;
        }

        int cellIndex = isMainInventory ? (slotIndex - 12) : slotIndex;
        if (cellIndex < 0 || cellIndex >= Width * Height) return;

        BYTE* slot = Inv + cellIndex * 0x44;
        short oldType = *(short*)slot;
        if (oldType == (short)0xFFFF || oldType < 0) return;

        BYTE ox = slot[62];
        BYTE oy = slot[63];
        if (ox >= Width || oy >= Height) return;

        for (int yy = 0; yy < Height; ++yy) {
            for (int xx = 0; xx < Width; ++xx) {
                int idx = yy * Width + xx;
                BYTE* cell = Inv + idx * 0x44;
                if (*(short*)cell == oldType && cell[62] == ox && cell[63] == oy) {
                    memset(cell, 0, 0x44);
                    *(short*)cell = (short)0xFFFF;
                }
            }
        }
    };

    clear_slot(Index);

    {
        int type = ConvertItemType(Item);
        if (type == 0xFF) return;
        if (Index < 0) return;
        int total = Width * Height + (isMainInventory ? 12 : 0);
        if (Index >= total) return;

        BYTE optByte = Item[1];
        BYTE durability = Item[2];
        BYTE byteHi = Item[3];
        BYTE extByte = Item[4];
        int level = (optByte >> 3) & 0x0F;

        AddItemToGrid(Inv, Width, Height, Index, type, level, optByte,
                      First ? 1 : 0, durability, byteHi, extByte);
        return;
    }
    // ─────────────────────────────────────────────────────────────────────────
    // CODIGO MUERTO: el bloque de arriba termina en `return` incondicional, asi
    // que nada de esto se ejecuta.  Se conserva por su documentacion de los
    // campos del slot, pero OJO antes de reactivarlo: el `Inv + Index * 0x44` de
    // abajo tiene el MISMO bug que se acaba de arreglar en `clear_slot` — para
    // `Index < 12` indexa la celda del grid en vez del wear slot de
    // CharacterMachine.
    // ─────────────────────────────────────────────────────────────────────────
    int type = ConvertItemType(Item);
    if (type == 0xFF) return;                     // empty / sentinel slot
    if (Index < 0) return;
    int total = Width * Height + 12;              // wear (12) + grid (W*H)
    if (Index >= total) return;

    // Grid stride: ITEM-raw is 0x44 (68) bytes per slot in 0.97k. Some tables
    // (e.g. char-list at DAT_07ea5298) use 0x44 too — same shape as inventory.
    BYTE* slot = Inv + Index * 0x44;

    // Copy item header + flags. 12 bytes is what 0.97k packets carry per slot
    // (type, level, dur, option, serial[8]).
    memcpy(slot, Item, 12);

    // Per-cell render-gate fields (per IDA sub_4E38B0 / RenderEquipment3D):
    //   slot+26 (Durability byte): high bit of Item[2]
    //   slot+27 (Option1 byte):    low bits of Item[3]
    //   slot+38 (Width grid coord, byte): rare item flag
    //   slot+62 (x grid pos), slot+63 (y grid pos)
    //   slot+56 (Key/dur-int gate, int): MUST be > 0 for the slot to render!
    //
    // Without slot+56 set, sub_4E38B0 line 60 `*(int*)(slot+0x38) > 0` fails →
    // the item is silently skipped from the render walk → invisible in grid.
    BYTE durability = Item[2];                    // raw durability byte
    if (durability == 0) durability = 1;          // ensure render gate
    *(int*)(slot + 56) = (int)durability;

    // Set grid x/y from Index. For wear slots (Index < 12), use slot index 0
    // for x and 0 for y (wear slots are special-cased downstream). For grid
    // slots, decompose into (Index-12) → (x = idx % Width, y = idx / Width).
    if (Index >= 12) {
        int gridIdx = Index - 12;
        slot[62] = (BYTE)(gridIdx % Width);
        slot[63] = (BYTE)(gridIdx / Width);
    } else {
        slot[62] = 0;
        slot[63] = 0;
    }

    // IDA InsertInventoryItem L77-80 (LABEL_59).
    if (Inv == Inventory) SortInventory((short*)Inv);
    // (Anti-tamper hash-table ref-count for !First — skipped per policy.)
    (void)First;
}

// FUN_0046fe00 @ 0x0046FE00 — DeleteJoint(int Type, DWORD Target, int SubType)
