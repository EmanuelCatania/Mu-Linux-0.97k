// HUD_Pass3.cpp — third batch of HUD ports.
//
// Functions ported in this TU (1:1 from IDA):
//   * RenderNumArrow   (sub_4BF540)  — quiver / arrow-count overlay
//   * sub_4BCD20       — gauge + RenderNumber2D for a stat from CharacterAttribute
//   * sub_4F6050       — inventory grid render dispatch (4 panels)
//   * sub_4EB070       — pet-stats panel (helper / golem name + skill icons)
//   * RenderBoolean    (sub_480E00)  — per-entry floating-number renderer
//
// Ported support helpers:
//   * RenderCenteredText (sub_514270)   — small wrapper around RenderText
//   * sub_482850         — count arrows of equipped type in inventory grid
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <gl/GL.h>

// sub_4E38B0 / sub_47F360 / sub_47F4C0 are now ported in HUD_Pass4.cpp.
// sub_4E9300 is ported in HUD_Pass6.cpp.
extern "C" int __cdecl sub_4E38B0(float a1, float a2, float x, int a4,
                                   float sx, int a6);

static float PointerBitsAsFloat(const void* pointer)
{
    DWORD bits = (DWORD)(uintptr_t)pointer;
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}
extern "C" int __cdecl sub_4E9300_(void);
#define sub_4E9300 sub_4E9300_

// ── Globals required by these renderers ─────────────────────────────────────

// Inventory grid pool used by sub_482850 to count arrows.  IDA names this
// `dword_7EA9328` but our globals.h already declares DAT_07ea9328 as a
// sentinel int; expose our own backing array under a unique name with
// external linkage so HUD_Pass5.cpp's sub_482E40 can share the same pool.
extern "C" DWORD g_InventoryGridPool[2176] = {0};

// Public init helper called from WinMain.  Sets every ITEM slot's Type
// field (offset 0, WORD) to 0xFFFF so empty inventory cells aren't
// misinterpreted as type-0 matches by FUN_00482be0 and similar scanners.
extern "C" BYTE  Inventory[];
extern "C" void  DbgLogPublic(const char*);
extern "C" BYTE  OffsetInventoryItems[];
extern "C" BYTE  OffsetTradeItems[];
extern "C" BYTE  OffsetMixItems[];
extern "C" BYTE  ShopItems[];   // pool dedicado de la tienda (definido abajo)
extern "C" BYTE  OffsetWarehouseItems[];   // pool del baúl (definido abajo)
extern "C" void HUD_InitInventoryPools(void)
{
    // ITEM struct stride 0x44 = 68 bytes.  Each pool is 64 slots × 68 b.
    auto initPool = [](BYTE* base, size_t totalBytes) {
        for (size_t i = 0; i < totalBytes; i += 0x44) {
            *(WORD*)(base + i) = 0xFFFF;   // Type field → empty marker
        }
    };
    initPool(Inventory,             64 * 0x44);
    initPool(OffsetInventoryItems,  64 * 0x44);
    initPool(OffsetTradeItems,      64 * 0x44);
    initPool(OffsetMixItems,        64 * 0x44);
    initPool(ShopItems,             120 * 0x44);
    // 2026-07-27 FIX (baúl abre "lleno" de Kris sin data): OffsetWarehouseItems
    // era el ÚNICO pool que no se inicializaba → quedaba todo en ceros, y Type=0
    // es un item válido (Kris) en vez del marcador de celda vacía (0xFFFF) → las
    // 120 celdas del baúl mostraban un Kris fantasma con durabilidad 0/20.
    initPool(OffsetWarehouseItems,  120 * 0x44);
    // g_InventoryGridPool stride 136 (= 0x88), 64 slots ≈ 8704 b.
    BYTE* gp = (BYTE*)g_InventoryGridPool;
    for (size_t i = 0; i < sizeof(g_InventoryGridPool); i += 0x88) {
        *(WORD*)(gp + i) = 0xFFFF;
    }

    // 2026-08-22: acá se estampaba Type=0xFFFF sobre un `g_EquipGridBuf` suelto
    // para que sus celdas vacías no dieran falso positivo.  Ese buffer no existía
    // en el binario: unk_7EA9504 / unk_7EA9328 son posiciones dentro del
    // inventario real (ver globals.cpp).  Con los punteros ya reenraizados, este
    // loop escribiría 0xFFFF encima del Type de slots REALES.  Removido — el
    // inventario ya lo inicializa initPool() unas líneas más arriba.
}

extern "C" {
    int   m_Resolution           = 4;

    // Pet panel state (sub_4EB070).
    int   dword_7EAA14C          = 0;
    BYTE  byte_7EAA1A4           = 0;
    BYTE  byte_7EAA179           = 0;
    char  dword_7EA9814[64]      = {0};
    short word_7E91394[16]       = {0};
    short unk_55A6FC             = 0x002A;
    float flt_83A7ACC[8]         = {0,0,0,0,0,0,0,0};

    // Floating-numbers / chat bubble per-entry runtime state.
    BYTE  byte_7E11DE0           = 0;
    DWORD dword_7E11DA8          = 0;

    // SetTextColor_0 (= 0x00559C7C) YA NO se define acá.  2026-08-15: estaba
    // duplicado — esta copia era la que escribían RenderBoolean_IDA y
    // RenderPartyHP, mientras que el LECTOR real (FUN_0047f360, stubs_game.cpp)
    // leía `DAT_00559c7c`, otro global distinto que quedaba en 0.  Resultado: el
    // prefijo de guild de la burbuja de chat se pintaba con color 0x00000000
    // (transparente/negro).  Ahora vive en globals.cpp y `DAT_00559c7c` es un
    // alias del mismo símbolo (ver globals.h).  Verificado en IDA:
    // xrefs de 0x559C7C = { sub_47F360 (lee), RenderBoolean ×3 (escribe),
    // RenderPartyHP (escribe) } — o sea UNA sola memoria.

    // Inventory pools.
    // 2026-07-25 (#2 shops): Inventory era [64*68] pero el pool de TIENDA es un
    // overlay 8×15 (120 slots) que arranca en &Inventory[32].WalkSpeed y llega
    // hasta ~Inventory+10348 (HUD_Pass3:382 sub_4E38B0 + Net_Process ShopInsert).
    // Con 64 slots el render leía OOB (shop salía vacío) y poblarlo corrompería
    // memoria. Agrandado a 160 slots (10880 bytes) para cubrir el grid completo.
    BYTE  Inventory[160 * 68]            = {0};
    BYTE  OffsetInventoryItems[64 * 68]   = {0};
    BYTE  OffsetTradeItems[64 * 68]       = {0};
    BYTE  OffsetMixItems[64 * 68]         = {0};
    // 2026-05-08: warehouse grid is 8x15 = 120 slots * 0x44 = 8160 bytes.
    // Used by FUN_004d23b0 (Inventory click handler) when WarehouseOpened.
    BYTE  OffsetWarehouseItems[120 * 68]  = {0};
    // 2026-07-27: pool de la TIENDA (8×15 = 120 slots). ANTES era un overlay
    // dentro de Inventory (&Inventory[32].WalkSpeed) — como en el binario
    // original esas direcciones son distintas, en nuestro build el overlay
    // colisionaba con otros usos de Inventory[32] (scratch de coords de paneles,
    // loops de trade, etc.) que lo pisaban cada frame → "tienda abre vacía"
    // intermitente (diag SHOPREND: populate 15/15 y 1s después occ=0 SIN ningún
    // paquete de red). Con array propio el pool es inmune a ese aliasing.
    BYTE  ShopItems[120 * 68]             = {0};

    // 2026-04-30: Inventory/Trade panel origin globals — unified with the
    // IDA-side DAT_ addresses that RenderEquipment3D_stub / RenderItem3D
    // already read.  The Ghidra-era phantoms (InventoryStartX = 380 etc.)
    // were independent of DAT_07ea5288 → equipment items rendered at
    // x≈0 while the panel frame rendered at x=450.
    int   dword_7EAA0CC          = 0;
    int   dword_7EAA0C8          = 0;

    // g_hFontBig — large-font HFONT used by the pet panel.  Use g_hFontBold
    // as the closest existing handle until a dedicated big font is created.
}

// Render-text engine stub — must be at namespace scope (extern "C" with
// initialiser inside another extern "C" was tripping MSVC).
static int g_RenderTextStubObj[16] = {0};
extern "C" int*  g_pRenderText = g_RenderTextStubObj;

// Externs for globals defined in HUD_Pass2.cpp.
// (UI flags are now #defined in globals.h to DAT_07eaa11x bytes.)
extern "C" int  GetScreenWidth(void);
void __cdecl FUN_0047f4c0(int a1, int a2, float Width, float Height, int a5, int a6, float a7, int a8);

// Forward decl for the helper defined in HUD_Pass2.cpp.
extern "C" SIZE* __cdecl FUN_0047f6f0(int x, int y, const char* lpString,
                                      int boxWidth, char style, int extraSize);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" void   __cdecl RenderTipText(int, int, const char*);

// g_hFontBig fallback alias.
#define g_hFontBig  (g_hFontBold ? g_hFontBold : g_hFont)

// Aliases.  ItemAttribute already defined in HUD_Pass2 via a #define on
// DAT_07d78068 — but #defines don't propagate across TUs.  Re-establish here.
extern "C" int DAT_07d78068;
#define ItemAttribute            ((ITEM_ATTRIBUTE*)DAT_07d78068)

#define byte_7E11D6E             DAT_07e11d6e
#define byte_7E919BC             DAT_07e919bc
#define ppvBits                  ppvBits_055c9e4c
#define g_pRenderText            g_pRenderText                       // exposed above
#define dword_7EAA14C_alias      dword_7EAA14C
#define m_Resolution_alias       m_Resolution

// PACKET_ENCRYPT — used by GetScreenWidth (HUD_Pass2) and sub_4F6050; both
// are anti-tamper noise calling into the hash-table.  Stub if unimplemented.
extern "C" void __cdecl PACKET_ENCRYPT(void* /*ctx*/, void* /*key*/) {}

// =============================================================================
// RenderCenteredText — sub_514270.  Centre `pszText` at iPos_x.
// =============================================================================
extern "C" SIZE* __cdecl RenderCenteredText(int iPos_x, int iPos_y,
                                             const char* pszText);
SIZE* __cdecl RenderCenteredText(int iPos_x, int iPos_y, const char* pszText)
{
    SIZE sz = {0,0};
    int n = lstrlenA(pszText);
    GetTextExtentPointA(m_hFontDC, pszText, n, &sz);
    int adjustedX = iPos_x - ((640 * sz.cx / (int)WindowWidth) >> 1);
    RenderText(adjustedX, iPos_y, (char*)pszText, 0, 0, 0);
    return &TextSize;
}


// =============================================================================
// sub_482850 — count arrows in inventory matching the equipped bow/crossbow
// type.  Returns the matching item count.
//
//   if (CharacterAttribute->Class & 7 == 2 /* elf */) {
//       v5 = mainSlotItemType  (CharacterMachine+536, short)
//       v28 = helperSlotItemType (CharacterMachine+604, short)
//       if (v28 in [128..134] || v28 == 145) match = 143  (xbow bolts)
//       else if (v5 in [136..142] or [144..159]) match = 135 (arrows)
//       else match = v28
//       count items[type==match && quantity>0] in 8x8 grid (dword_7EA9504..
//       dword_7EA9328, 136 dwords/row stride backwards).
//   }
//   return count.
// =============================================================================
extern "C" int __cdecl FUN_00482850_(void);   // we prefix to avoid clash with stubs.cpp
int __cdecl FUN_00482850_(void)
{
    if (!CharacterMachine || !CharacterAttribute) return 0;

    // Anti-tamper inner block — skipped.

    if (((*(BYTE*)((BYTE*)CharacterAttribute + 11)) & 7) != 2) return 0;

    int v5  = *(short*)((BYTE*)CharacterMachine + 536);
    int v28 = *(short*)((BYTE*)CharacterMachine + 604);

    int match;
    if ((v28 >= 128 && v28 < 135) || v28 == 145) {
        match = 143;
    } else if ((v5 >= 136 && v5 < 143) || (v5 >= 144 && v5 < 160)) {
        match = 135;
    } else {
        match = v28;
    }

    int count = 0;
    // Walk the inventory 8x8 grid backwards (matches IDA).
    int* base = (int*)&g_InventoryGridPool[0];
    int* end_ptr  = base + 119;     // (&unk_7EA9504 - &unk_7EA9328) / 4 = 119 dwords
    int* row = end_ptr;
    while (row >= base) {
        int* cell = row;
        for (int i = 0; i < 8; ++i) {
            // type stored as int16 at cell-28 dwords (= -112 bytes from row)
            // — preserving the IDA pointer-arithmetic.
            short type = *((short*)cell - 28);
            int   qty  = *cell;
            if (type == match && qty > 0) ++count;
            cell -= 136;
        }
        row -= 17;
    }
    return count;
}


// =============================================================================
// RenderNumArrow — sub_4BF540.  Two-line overlay near the right edge of the
// HUD when an arrow-using item (143 = bow / 135 = crossbow) is equipped.
//   Line 1: arrow count for slot 536  (uses GlobalText[351])
//   Line 2: arrow count for slot 604  (uses GlobalText[352])
// Y offset is resolution-dependent (m_Resolution case).
// =============================================================================
extern "C" bool __cdecl RenderNumArrow_(void);
bool __cdecl RenderNumArrow_(void)
{
    bool drewSomething = false;
    float v27 = (PartyNumber > 0 && !PartyOpened) ? 50.0f : 0.0f;

    int baseY = 10;
    switch (m_Resolution) {
        case 0: baseY = 90; break;
        case 1: baseY = 75; break;
        case 2: baseY = 65; break;
        case 3: baseY = 58; break;
        case 4: baseY = 10; break;
    }

    SelectObject(m_hFontDC, g_hFont ? g_hFont : g_hFontBold);

    if (!CharacterMachine) return drewSomething;

    // Anti-tamper inner block (CharacterMachine decrypt) — skipped.

    if (*(WORD*)((BYTE*)CharacterMachine + 536) == 143) {
        int screenW = 640;
        int v10 = *(unsigned char*)((BYTE*)CharacterMachine + 562);
        float v25 = (float)((double)screenW - (double)v27 - 10.0);
        int v11 = FUN_00482850_();
        if (v10 > 0 || v11 > 0) {
            CHAR String[100];
            wsprintfA(String, GlobalText[351], v10, v11);
            EnableAlphaTest(true);
            int n = lstrlenA(String);
            SIZE sz = {0,0};
            GetTextExtentPointA(m_hFontDC, String, n, &sz);
            FUN_0047f6f0((int)v25, baseY, String, 0, 0, 0);
            drewSomething = true;
        }
    }

    if (*(WORD*)((BYTE*)CharacterMachine + 604) == 135) {
        int screenW = 640;
        int v13 = *(unsigned char*)((BYTE*)CharacterMachine + 630);
        float v26 = (float)((double)screenW - (double)v27 - 10.0);
        int v14 = FUN_00482850_();
        if (v13 > 0 || v14 > 0) {
            CHAR String[100];
            wsprintfA(String, GlobalText[352], v13, v14);
            EnableAlphaTest(true);
            int n = lstrlenA(String);
            SIZE sz = {0,0};
            GetTextExtentPointA(m_hFontDC, String, n, &sz);
            FUN_0047f6f0((int)v26, baseY + 12, String, 0, 0, 0);
            drewSomething = true;
        }
    }

    // Symmetric ref-count decrement — skipped.
    return drewSomething;
}

// AntiTamper_HashMaintain_A → RenderNumArrow.
void AntiTamper_HashMaintain_A(void) { RenderNumArrow_(); }


// =============================================================================
// sub_4BCD20 — right-side stat gauge (vertical bar 15×N at x=551 + 2-digit
// counter at (571, 467)) showing some stat from CharacterAttribute+36 over
// CharacterAttribute+38.  When the mouse hovers the gauge, GlobalText[214]
// is shown via RenderTipText at (546, 419).
//
// Without CharacterAttribute populated this becomes a no-op.
// =============================================================================
extern "C" void __cdecl Render_HudPass_4BCD20_(void);
void Render_HudPass_4BCD20_(void)
{
    if (!CharacterMachine || !CharacterAttribute) return;

    // Anti-tamper #1 — skipped.

    int v3 = *(unsigned short*)((BYTE*)CharacterAttribute + 36);
    int v23 = *(unsigned short*)((BYTE*)CharacterAttribute + 38);
    if (v23 == 0) v23 = 1;

    // Anti-tamper #2 — skipped.

    int barHeight = 36 * v3 / v23;
    if (barHeight < 0) barHeight = 0;
    if (barHeight > 36) barHeight = 36;
    float Heightb = (float)barHeight;
    float vHeight = Heightb * 0.015625f;
    float y = 473.0f - Heightb;
    FUN_005125a0(257, 551.0f, y, 15.0f, Heightb, 0.0f, 0.0f, 0.9375f, vHeight, 1, 1);

    RenderNumber2D(571.0f, 467.0f, v3, 9.0f, 10.0f);

    if ((int)MouseX >= 551 && (int)MouseX < 566 &&
        (int)MouseY >= 437 && (int)MouseY < 473) {
        CHAR Buffer[100];
        wsprintfA(Buffer, GlobalText[214], v3, v23);
        RenderTipText(546, 419, Buffer);
    }
}

// AntiTamper_HashMaintain_D → sub_4BCD20.
void AntiTamper_HashMaintain_D(void) { Render_HudPass_4BCD20_(); }


// =============================================================================
// sub_4F6050 — inventory-grid render dispatch.  Calls sub_4E38B0 four times
// to draw the active grid panel (Inventory / Shop / Trade / Warehouse /
// ChaosMix) using its own offset ints and 8×N item array.
//
// The IDA decomp interleaves heavy hash-table ref-count noise on
// ShopOpened / TradeOpened — those serve only to satisfy anti-tamper hash
// tracking and are skipped here.
//
// Without sub_4E38B0 ported the panels won't show inventory items, but the
// rest of the HUD is unaffected.
// =============================================================================
// 2026-05-08 NOTE: previously had FUN_004d23b0/FUN_004df410 hook here.
// That was wrong — sub_4F6050 (this fn) is NOT called in-world. The actual
// per-frame in-world inventory render is RenderInventoryWindow (sub_4F0A50)
// invoked from Render_QuickButtons_ (sub_4F5820), HUD_Pass4.cpp:246. The
// click-handler hook lives there now (HUD_Pass6.cpp:RenderInventoryWindow).

extern "C" void __cdecl Render_HudPass_4F6050_(void);
void Render_HudPass_4F6050_(void)
{
    bool draw = false;
    if (!InventoryOpened) {
        // Decide based on Shop / Warehouse / ChaosMix / Trade / Event flags.
        if (ShopOpened || WarehouseOpened || ChaosMixOpened || TradeOpened || EventWindowOpened) {
            draw = true;
        }
    } else {
        draw = true;
    }
    if (!draw) return;

    EnableAlphaTest(true);
    if (InventoryOpened) {
        sub_4E38B0((float)((double)InventoryStartX + 15.0),
                   (float)((double)InventoryStartY + 200.0),
                   PointerBitsAsFloat(OffsetInventoryItems), 8, 8.0f, 1);
    }
    if (ShopOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   // IDA pushes `Inventory.WalkSpeed + 0x880`, which in our
                   // ITEM layout is exactly `&Inventory[32].WalkSpeed`.
                   PointerBitsAsFloat(ShopItems), 8, 15.0f, 1);
    }
    if (TradeOpened) {
        sub_4E38B0((float)((double)TradeInventoryStartX + 15.0),
                   (float)((double)TradeInventoryStartY + 70.0),
                   PointerBitsAsFloat(Inventory), 8, 4.0f, 1);
        sub_4E38B0((float)((double)TradeInventoryStartX + 15.0),
                   (float)((double)TradeInventoryStartY + 270.0),
                   PointerBitsAsFloat(OffsetTradeItems), 8, 4.0f, 1);
    }
    if (WarehouseOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   PointerBitsAsFloat(OffsetWarehouseItems), 8, 15.0f, 1);
    }
    if (ChaosMixOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 110.0),
                   PointerBitsAsFloat(OffsetMixItems), 8, 4.0f, 1);
    }
    if (EventWindowOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   PointerBitsAsFloat(OffsetMixItems), 8, 4.0f, 1);
    }
}

void Render_HudPass_4F6050(void) { Render_HudPass_4F6050_(); }


// =============================================================================
// sub_4EB070 — pet stats / customise panel (when dword_7EAA14C != 0).
// Renders:
//   * Centred frame (5 row bitmaps 251 stacked between two cap rows 252)
//   * Header text (RenderCenteredText) — picked by mode
//   * Subtitle text (mode → 695/696/697)
//   * Editable name field (string + cursor)
//   * 11 skill icons (5×2 grid + extra slot 10) with counts in word_7E91394[]
//   * Two action buttons at y=239
// Currently the IDA panel only fires when a pet UI mode is active.
// =============================================================================
extern "C" void __cdecl Render_HudPass_4EB070_(void);
void Render_HudPass_4EB070_(void)
{
    if (!dword_7EAA14C) return;

    int v20 = sub_4E9300();
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_005125a0(252, 213.0f, 100.0f, 213.0f, 5.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);

    int Width = 105;
    for (int v1 = 4; v1; --v1) {
        FUN_005125a0(251, 213.0f, (float)Width, 213.0f, 40.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);
        Width += 40;
    }
    FUN_005125a0(252, 213.0f, (float)Width, 213.0f, 5.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);

    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFFC4C4u;   // -15164

    int v2 = dword_7EAA14C - 1;
    if (v2 < 0)      v2 = 0;
    else if (v2 > 4) v2 = 4;
    if (dword_7EAA14C == 6) v2 = 1;

    // Original: RenderCenteredText(320, 110, (const char *)(300 * v2 + 131450300));
    // Those addresses point into GlobalText[] (300-byte stride, base ≈ GlobalText[≈438+v2]).
    // Calling with the literal addresses would crash; route through GlobalText
    // assuming the indices exist; otherwise emit empty strings.
    {
        int idx = 438 + v2;
        const char* hdr = (idx >= 0 && idx < 1000) ? GlobalText[idx] : "";
        RenderCenteredText(320, 110, hdr);
    }
    int v3;
    switch (dword_7EAA14C) {
        case 2: case 3: case 6: v3 = 697; break;
        case 5:                 v3 = 696; break;
        default:                v3 = 695; break;
    }
    {
        int idx = 421 + (v3 - 695);   // 695..697 → 421..423
        const char* sub = (idx >= 0 && idx < 1000) ? GlobalText[idx] : "";
        RenderCenteredText(320, 122, sub);
    }

    glColor3f(0.30000001f, 0.30000001f, 0.30000001f);
    int v4 = 4;
    if (dword_7EAA14C == 3 || dword_7EAA14C == 2 || dword_7EAA14C == 6) {
        v4 = (int)flt_83A7ACC[0];
    }
    float Widtha = (v4 > 4) ? (float)((double)(v4 - 4) * 13.0 + 52.0) : 52.0f;
    float v21 = Widtha * 0.5f;
    float x = 320.0f - v21;
    FUN_005125a0(253, x, 134.0f, Widtha, 16.0f, 0.0f, 0.0f, 0.625f, 0.5625f, 1, 1);
    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, g_hFont ? g_hFont : g_hFontBold);
    m_dwBackColor = 0;

    CHAR String[29];
    String[0] = (char)byte_7EAA1A4;
    memset(&String[1], 0, sizeof(String) - 1);
    m_dwTextColor = 0xFFC44400u;   // -3899264
    SelectObject(m_hFontDC, g_hFontBig);
    int v5 = 0;
    int len = (int)strlen(dword_7EA9814);
    if (len > 0) {
        short v6 = unk_55A6FC;
        do {
            ++v5;
            *(short*)&String[strlen(String)] = v6;
        } while (v5 < len);
    }
    RenderText(v4 - (int)(v21 - 320.0f), 132, String, 0, 0, 0);

    // Skill grid: 11 entries (slots 0..10) — slot 10 is special.
    for (int v7 = 0; v7 < 11; ++v7) {
        BYTE v8 = 0;
        if (v20 == v7) {
            if (byte_7EAA179) v8 = 1;
            m_dwTextColor = 0xFFFF0084u;   // -16726844
            glColor3f(1.0f, 1.0f, 0.80000001f);
        } else {
            m_dwTextColor = 0xFFC44400u;
            glColor3f(0.85000002f, 0.85000002f, 0.64999998f);
        }
        int v22 = 38 * (v7 / 5) + 154;
        float v24 = (float)v22;
        float v25_x = (float)(40 * (v7 % 5) + 223);
        FUN_005125a0(277, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);

        if (v7 == 10) {
            int v9 = (v20 == 10) ? ((v8 != 0) + 1) : 0;
            FUN_005125a0(v9 + 254, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
        } else {
            int v10 = (v20 == v7 && v8) ? 278 : 277;
            FUN_005125a0(v10, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
            CHAR pszText[32];
            wsprintfA(pszText, "%d", word_7E91394[v7]);
            m_dwBackColor = 0;
            RenderText(40 * (v7 % 5) + 235, v22 - (v8 != 0) + 6, pszText, 0, 0, 0);
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    // Two action buttons at y=239: 70×21 px, textures 241/243 with hover
    // highlight when v20 in {11, 12}.
    int v11 = 265;
    DWORD textures[2] = { 241, 243 };
    for (int v12 = 0; v12 < 2; ++v12) {
        FUN_005125a0(textures[v12] + (v20 == v12 + 11 ? 1 : 0),
                     (float)v11, 239.0f, 70.0f, 21.0f,
                     0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);
        v11 += 78;
    }
}

void Render_HudPass_4EB070(void) { Render_HudPass_4EB070_(); }


// =============================================================================
// RenderBoolean — sub_480E00.  Per-floating-number entry renderer.  IDA
// body is huge (3081 bytes) and depends on the CUIRenderText vtable's [+8]
// flag (mode 1 → big offscreen-bitmap path; else → glColor + RenderColor +
// CUIRenderText::RenderText path).
//
// In our build g_pRenderText points to a stub object where [+8] = 0 — the
// "else" branch wins.  We port that branch byte-for-byte EXCEPT the actual
// text-glyph composition is delegated to RenderText_1 / FUN_0047f7a0
// (which already drives the same downstream font pipeline our build has).
//
// Entry layout (offsets from `c`, in bytes):
//   +0    sender ID (string)
//   +24   guild prefix (string)
//   +36   msg-type byte (0..5)
//   +37   sub-mode byte (0..2)
//   +44   primary text (string)
//   +300  wrapped overflow text (string)
//   +556  text-fade timer 1
//   +560  text-fade timer 2
//   +568  cached screen X
//   +572  cached screen Y
//   +576  text width  (cx)
//   +580  text height (cy)
// =============================================================================
// ⚠️ CÓDIGO MUERTO — NO USAR (verificado 2026-07-19)
// Esta es una copia VIEJA de sub_480E00. El dispatcher real `FUN_00480e00`
// delega en `RenderBoolean_IDA` (más abajo en este mismo archivo), que es la
// que se mantiene. Esta copia NO tiene los fixes aplicados (en particular el
// de `*(float*)&TextSize.cx` vs `(float)TextSize.cx` en las llamadas a
// FUN_0047f4c0, que era lo que impedía que se dibujara la burbuja de chat).
// Grep confirma 0 callers de código: solo la referencian comentarios.
// Mismo patrón de función fantasma que ya costó caro con OpenSMDFile —
// candidata a borrar en una pasada de limpieza (tarea B3 del roadmap).
void __cdecl RenderBoolean(int x, int y, DWORD c)
{
    if (!c) return;

    EnableAlphaTest(true);
    glColor3f(1.0f, 1.0f, 1.0f);

    LONG cx = *(int*)(c + 576);
    LONG cy = *(int*)(c + 580);
    TextSize.cx = cx;
    TextSize.cy = cy;

    // Pick palette by msg-type at +36.
    BYTE kind = *(BYTE*)(c + 36);
    switch (kind) {
        case 0:  m_dwTextColor = 0xFFF0FFE6u; break;   // -983146
        case 1:  m_dwTextColor = 0xFFFF7724u; break;   // -34716
        case 2:  m_dwTextColor = 0xFFFFB44Cu; break;   // -19316
        case 3:  m_dwTextColor = 0xFFFFDCE8u; break;   // -9016
        case 4:  m_dwTextColor = 0xFF3CCBFFu; break;   // -12806401
        case 5:  m_dwTextColor = 0xFF1DE3FFu; break;   // -14790401
        default: m_dwTextColor = 0xFF0000FFu; break;   // -16776961
    }

    // Background bg colour by sub-mode at +37.
    BYTE mode = *(BYTE*)(c + 37);
    DWORD bg;
    switch (mode) {
        case 0:  bg = 0x96143214u; SetTextColor_0 = 0xFFFFC8DCu; break;
        case 1:  bg = 0x96000000u; SetTextColor_0 = 0xFF005228u; break;
        default: bg = 0x96000000u; SetTextColor_0 = 0xFF0000FFu; break;
    }
    m_dwBackColor = bg;

    // Hover: when mouse is over the entry rect AND the entry is from a
    // different sender than Hero, swap text/back colours for a flicker.
    int rectX = *(int*)(c + 568);
    int rectY = *(int*)(c + 572);
    int rectW = 640 * (int)cx / (int)WindowWidth;
    int rectH = 480 * (int)cy / (int)WindowHeight;
    if ((int)MouseX >= rectX && (int)MouseX < rectX + rectW &&
        (int)MouseY >= rectY && (int)MouseY < rectY + rectH &&
        InputEnable && Hero && *(BYTE*)((BYTE*)(uintptr_t)Hero + 846) &&
        strcmp((const char*)c, (const char*)((BYTE*)(uintptr_t)Hero + 449)) &&
        (int)(dword_7E11DA8 % 6) < 3)
    {
        DWORD swap = m_dwTextColor;
        m_dwTextColor = bg;
        m_dwBackColor = swap;
    }

    // Compose label: "[guild] sender" + main text (and wrapped tail).
    CHAR a4[256] = {0};
    BYTE pulse = byte_7E11DE0;
    a4[0] = (char)pulse;
    if (*(BYTE*)(c + 24)) {
        BYTE styleByte = *(BYTE*)(c + 37) - 14;   // (kind != 0 → -14, etc.)
        // 'small' is a Windows SDK keyword (RPC IDL) — use 'marker' instead.
        char marker[3]; marker[0] = 2; marker[1] = (char)styleByte; marker[2] = 0;
        strcat(a4, marker);
        marker[1] = -16;
        strcat(a4, (const char*)(c + 24));
        strcat(a4, marker);
    }
    strcat(a4, (const char*)c);

    // Background quad first.
    FUN_005124c0((float)x, (float)y,
                 (float)rectW, (float)rectH);

    // Render the composed text via RenderText_1 (our font pipeline alias).
    FUN_0047f7a0(x, y, a4, 0, 1, 0);

    // Wrapped overflow text on the next line(s).
    int v20 = *(int*)(c + 560);
    if (v20 > 0) {
        m_dwTextColor = (v20 < 10) ? 0xFFFFFFA6u : 0xFFC8C8C8u;
        FUN_0047f7a0(x, y + FontHeight + 1, (char*)(c + 300), 0, 1, 0);

        int v22 = *(int*)(c + 556);
        m_dwTextColor = (v22 < 10) ? 0xFFFFFFA6u : 0xFFC8C8C8u;
        FUN_0047f7a0(x, y + 2 * FontHeight + 1, (char*)(c + 44), 0, 1, 0);
    } else {
        int v24 = *(int*)(c + 556);
        if (v24 > 0) {
            m_dwTextColor = (v24 < 10) ? 0xFFFFFFA6u : 0xFFC8C8C8u;
            FUN_0047f7a0(x, y + FontHeight + 1, (char*)(c + 44), 0, 1, 0);
        }
    }
}

static void RenderBoolean_IDA(int x, int y, DWORD c)
{
    if (!c) return;

    EnableAlphaTest(true);
    glColor3f(1.0f, 1.0f, 1.0f);

    int texW = 1;
    int texH = 1;
    LONG cx = *(int*)(c + 576);
    LONG cy = *(int*)(c + 580);
    TextSize.cx = cx;
    TextSize.cy = cy;

    while (texW < cx && texW < 256) texW *= 2;
    while (texH < cy && texH < 256) texH *= 2;

    // 2026-08-15: los 6 colores estaban mal transcritos (el decompile los muestra
    // como decimales con signo).  Valores de IDA RenderBoolean L121-144:
    //   -983146=0xFFF0FF96  -34716=0xFFFF7864  -19316=0xFFFFB48C
    //   -9016=0xFFFFDCC8  -12806401=0xFF3C96FF  -14790401=0xFF1E50FF
    //   default -16776961=0xFF0000FF
    BYTE kind = *(BYTE*)(c + 36);
    switch (kind) {
        case 0:  m_dwTextColor = 0xFFF0FF96u; break;
        case 1:  m_dwTextColor = 0xFFFF7864u; break;
        case 2:  m_dwTextColor = 0xFFFFB48Cu; break;
        case 3:  m_dwTextColor = 0xFFFFDCC8u; break;
        case 4:  m_dwTextColor = 0xFF3C96FFu; break;
        case 5:  m_dwTextColor = 0xFF1E50FFu; break;
        default: m_dwTextColor = 0xFF0000FFu; break;
    }

    byte_7E11D6E = 1;
    int drawX = x * (int)WindowWidth / 640;
    int drawY = y * (int)WindowHeight / 480;
    if (FontHeight > 32) FontHeight = 32;

    auto ClearFontRows = [&](int rows) {
        if (rows <= 0 || !ppvBits) return;
        int rowBytes = 3 * TextSize.cx;
        char* row = (char*)ppvBits;
        for (int i = 0; i < rows; ++i) {
            memset(row, 0, 4 * (rowBytes >> 2));
            memset(&row[4 * (rowBytes >> 2)], 0, rowBytes & 3);
            row += 1536;
        }
    };
    ClearFontRows(FontHeight);

    // 2026-08-15: constantes corregidas contra IDA RenderBoolean L180-195.
    //   mode 0: back=-1773129196=0x96503214  SetTextColor_0=-14116=0xFFFFC8DC
    //   mode 1: back=-1778359236=0x9600643C  SetTextColor_0=-16711736=0xFF00FFC8
    //   otros : back=-1778384796=0x96000064  SetTextColor_0=-16776961=0xFF0000FF
    BYTE mode = *(BYTE*)(c + 37);
    DWORD bg;
    switch (mode) {
        case 0:  bg = 0x96503214u; SetTextColor_0 = 0xFFFFC8DCu; break;
        case 1:  bg = 0x9600643Cu; SetTextColor_0 = 0xFF00FFC8u; break;
        default: bg = 0x96000064u; SetTextColor_0 = 0xFF0000FFu; break;
    }
    m_dwBackColor = bg;

    int rectX = *(int*)(c + 568);
    int rectY = *(int*)(c + 572);
    int rectW = 640 * (int)cx / (int)WindowWidth;
    int rectH = 480 * (int)cy / (int)WindowHeight;
    if ((int)MouseX >= rectX && (int)MouseX < rectX + rectW &&
        (int)MouseY >= rectY && (int)MouseY < rectY + rectH &&
        InputEnable && Hero && *(BYTE*)((BYTE*)(uintptr_t)Hero + 846) &&
        strcmp((const char*)c, (const char*)((BYTE*)(uintptr_t)Hero + 449)) &&
        (int)(dword_7E11DA8 % 6) < 3)
    {
        m_dwBackColor = m_dwTextColor;
        m_dwTextColor = bg;
    }

    FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)c, texW, 0, 0, 0, 0, (LPCSTR)(c + 24));
    FUN_0047f4c0(drawX, drawY, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);

    // IDA L214-224: -1778372066=0x9600321E · -1778384846=0x96000032 ·
    //               -1775100406=0x96321E0A
    if (mode) {
        m_dwBackColor = (mode == 1) ? 0x9600321Eu : 0x96000032u;
    } else {
        m_dwBackColor = 0x96321E0Au;
    }

    // IDA: m_dwTextColor = -3613466 (0xFFC8DCE6), y -2134319898 (0x80C8DCE6)
    // cuando al mensaje le quedan menos de 10 ticks de vida (fade-out).
    int fade2 = *(int*)(c + 560);
    if (fade2 > 0) {
        m_dwTextColor = (fade2 < 10) ? 0x80C8DCE6u : 0xFFC8DCE6u;
        ClearFontRows(FontHeight);
        FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)(c + 300), texW, 0, 0, 0, 0, 0);
        FUN_0047f4c0(drawX, drawY + FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);

        int fade1 = *(int*)(c + 556);
        m_dwTextColor = (fade1 < 10) ? 0x80C8DCE6u : 0xFFC8DCE6u;
        ClearFontRows(FontHeight);
        FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)(c + 44), texW, 0, 0, 0, 0, 0);
        FUN_0047f4c0(drawX, drawY + 2 * FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);
    } else {
        int fade1 = *(int*)(c + 556);
        if (fade1 > 0) {
            m_dwTextColor = (fade1 < 10) ? 0x80C8DCE6u : 0xFFC8DCE6u;
            ClearFontRows(FontHeight);
            FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)(c + 44), texW, 0, 0, 0, 0, 0);
            FUN_0047f4c0(drawX, drawY + FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);
        }
    }
}

// IDA's name was FUN_00480e00.  functions.h declares it with C++ linkage;
// match that here (no extern "C") to avoid the redeclaration conflict.
void __cdecl FUN_00480e00(int xx, int yy, int eb)
{
    RenderBoolean_IDA(xx, yy, (DWORD)eb);
}
