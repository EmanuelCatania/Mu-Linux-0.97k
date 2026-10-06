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
// Alias a los globales reales (IDA): el teclado del PIN los comparte con
// SecondPassword_Screen9 (boton del candado), el drop del baul y el manejador 0x4E93A0.
#define dword_7EAA14C  DAT_07eaa14c
#define byte_7EAA1A4   DAT_07eaa1a4
#define byte_7EAA179   DAT_07eaa179
#define dword_7EA9814  DAT_07ea9814
#define word_7E91394   DAT_07e91394

// ── Globals required by these renderers ─────────────────────────────────────

// Inventory grid pool used by sub_482850 to count arrows.  IDA names this
// `dword_7EA9328` but our globals.h already declares DAT_07ea9328 as a
// sentinel int; expose our own backing array under a unique name with
// external linkage so HUD_Pass5.cpp's sub_482E40 can share the same pool.

// Public init helper called from WinMain.  Sets every ITEM slot's Type
// field (offset 0, WORD) to 0xFFFF so empty inventory cells aren't
// misinterpreted as type-0 matches by Item_FindQuickSlotByCategory and similar scanners.
extern "C" BYTE  Inventory[];
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
    // OffsetWarehouseItems también se inicializa: en ceros, Type=0 es un item
    // válido (Kris) en vez del marcador de celda vacía (0xFFFF).
    initPool(OffsetWarehouseItems,  120 * 0x44);

}

extern "C" {
    int   m_Resolution           = 4;

    // (Aca se definian dword_7EAA14C, byte_7EAA1A4, byte_7EAA179, dword_7EA9814
    //  y word_7E91394 como variables PROPIAS de este archivo — memorias aparte
    //  de los globales reales.  El modo del teclado del PIN se leia siempre 0,
    //  asi que el panel del candado del baul nunca se dibujaba.  Ahora son
    //  alias; ver el bloque de abajo.)
    short unk_55A6FC             = 0x002A;   // IDA: unk_55A6FC = '*'
    // IDA: flt_83A7ACC es el arreglo de la CAMARA (lo escribe MoveCamera).
    // Lo consume ChatListBox copiando sus bits a InputTextMax, que es la
    // rareza del binario documentada ahi.
    float flt_83A7ACC[8]         = {0,0,0,0,0,0,0,0};

    // Floating-numbers / chat bubble per-entry runtime state.
    BYTE  byte_7E11DE0           = 0;
    DWORD dword_7E11DA8          = 0;

    // SetTextColor_0 (= 0x00559C7C) NO se define acá: vive en globals.cpp y
    // `DAT_00559c7c` es un alias del mismo símbolo (ver globals.h).  Verificado en
    // IDA: xrefs de 0x559C7C = { sub_47F360 (lee), RenderBoolean ×3 (escribe),
    // RenderPartyHP (escribe) } — o sea UNA sola memoria.

    // Inventory pools.
    // Inventory tiene 160 slots (10880 bytes): el tamaño viene de cuando la tienda
    // era un overlay 8×15 dentro de Inventory (desde &Inventory[32].WalkSpeed);
    // hoy la tienda usa su propio pool (ShopItems).
    BYTE  Inventory[160 * 68]            = {0};
    BYTE  OffsetInventoryItems[64 * 68]   = {0};
    BYTE  OffsetTradeItems[64 * 68]       = {0};
    BYTE  OffsetMixItems[64 * 68]         = {0};
    // Warehouse grid is 8x15 = 120 slots * 0x44 = 8160 bytes.
    // Used by FUN_004d23b0 (Inventory click handler) when WarehouseOpened.
    BYTE  OffsetWarehouseItems[120 * 68]  = {0};
    // Pool de la TIENDA (8×15 = 120 slots), con array propio: en el binario
    // original son direcciones distintas, y como overlay dentro de Inventory
    // (&Inventory[32].WalkSpeed) colisionaría con otros usos de Inventory[32]
    // (scratch de coords de paneles, loops de trade, etc.).
    BYTE  ShopItems[120 * 68]             = {0};

    // Inventory/Trade panel origin globals — unificados con las direcciones DAT_
    // del lado IDA que ya leen RenderEquipment3D / RenderItem3D.
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
void __cdecl Font_RenderBitmapText(int a1, int a2, float Width, float Height, int a5, int a6, float a7, int a8);

// Forward decl for the helper defined in HUD_Pass2.cpp.
extern "C" SIZE* __cdecl Text_MeasureBox(int x, int y, const char* lpString,
                                      int boxWidth, char style, int extraSize);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" void   __cdecl RenderTipText(int, int, const char*);

// g_hFontBig fallback alias.
// g_hFontBig lo publica globals.h como alias de DAT_055ca014 (la fuente
// de doble altura que crea WinMain).  Antes este alias local lo mandaba
// a la bold, por eso los textos "grandes" salian del tamano normal.

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

// PACKET_ENCRYPT se implementa en Net/Crypto.cpp (0x00404040).

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
    int adjustedX = iPos_x - ((640 * sz.cx / (int)gWindow.GetWidth()) >> 1);
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
extern "C" int __cdecl FUN_00482850_(void);   // sufijo `_` para no chocar con otros FUN_00482850
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
    // Recorre la grilla 8x8 del inventario hacia atras, igual que IDA.
    // unk_7EA9328 y unk_7EA9504 son posiciones dentro de OffsetInventoryItems — ver
    // la derivacion en globals.cpp.
    int* base = &DAT_07ea9328;      // slot 56, campo Key
    int* row  = &DAT_07ea9504;      // slot 63, campo Key  (= base + 119 dwords)
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

    SelectObject(m_hFontDC, gFont.GetFont(FONT_NORMAL) ? gFont.GetFont(FONT_NORMAL) : gFont.GetFont(FONT_BOLD));

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
            Text_MeasureBox((int)v25, baseY, String, 0, 0, 0);
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
            Text_MeasureBox((int)v26, baseY + 12, String, 0, 0, 0);
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

    // DESVIACION NECESARIA (barra de AG).
    // `sub_4BCD20` dibuja con RenderBitmap / RenderNumber2D / RenderTipText, que
    // no arman matrices propias: dependen de la ortho de `BeginBitmap`.  Y el
    // pase anterior (`sub_4BD650`) TERMINA con `EndBitmap`.
    //
    // En el binario eso funciona por accidente: `EndBitmap` (0x5124B0) hace dos
    // `glPopMatrix` seguidos SIN cambiar de modo, o sea los dos caen sobre
    // MODELVIEW y la PROJECTION ortho que empujo `BeginBitmap` queda activa (a
    // costa de desbordar esa pila, que es el GL_STACK_OVERFLOW 0x503 conocido).
    // Nuestro `GL_End2D` esta balanceado a proposito, asi que al salir de
    // sub_4BD650 la proyeccion vuelve a la perspectiva 3D y todo lo que dibuja
    // esta funcion caeria fuera de pantalla.
    GL_Begin2D();

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
    GL_DrawTexture(257, 551.0f, y, 15.0f, Heightb, 0.0f, 0.0f, 0.9375f, vHeight, 1, 1);

    RenderNumber2D(571.0f, 467.0f, v3, 9.0f, 10.0f);

    if ((int)MouseX >= 551 && (int)MouseX < 566 &&
        (int)MouseY >= 437 && (int)MouseY < 473) {
        CHAR Buffer[100];
        wsprintfA(Buffer, GlobalText[214], v3, v23);
        RenderTipText(546, 419, Buffer);
    }

    GL_End2D();
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
// =============================================================================
// sub_4F6050 (esta función) NO se llama in-world: el render por frame del
// inventario in-world es RenderInventoryWindow (sub_4F0A50), invocado desde
// Render_QuickButtons_ (sub_4F5820, HUD_Pass4.cpp). El hook del click-handler
// (FUN_004d23b0) vive en HUD_Pass6.cpp:RenderInventoryWindow.

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
    GL_DrawTexture(252, 213.0f, 100.0f, 213.0f, 5.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);

    int Width = 105;
    for (int v1 = 4; v1; --v1) {
        GL_DrawTexture(251, 213.0f, (float)Width, 213.0f, 40.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);
        Width += 40;
    }
    GL_DrawTexture(252, 213.0f, (float)Width, 213.0f, 5.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);

    SelectObject(m_hFontDC, gFont.GetFont(FONT_BOLD));
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
        // IDA: 300 * v2 + 131450300, y GlobalText esta en 131243300 (0x07D29D24)
        // -> (131450300 - 131243300) / 300 = 690.  Antes decia 438.
        int idx = 690 + v2;
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
        // IDA: 300 * v3 + 131243300 -> GlobalText[v3] directo.  Antes restaba 274.
        int idx = v3;
        const char* sub = (idx >= 0 && idx < 1000) ? GlobalText[idx] : "";
        RenderCenteredText(320, 122, sub);
    }

    glColor3f(0.30000001f, 0.30000001f, 0.30000001f);
    // Largo del campo: 4 para el PIN, 7 para el codigo personal.
    // DESVIACION: el binario lee LODWORD(flt_83A7ACC[0]), que es el arreglo de
    // la CAMARA (lo escribe MoveCamera como floats) — o sea toma los bits de un
    // float como largo.  Usamos el largo real que valida el server
    // (gObjCheckPersonalCode compara 7 caracteres).
    int v4 = 4;
    if (dword_7EAA14C == 3 || dword_7EAA14C == 2 || dword_7EAA14C == 6) {
        v4 = 7;
    }
    float Widtha = (v4 > 4) ? (float)((double)(v4 - 4) * 13.0 + 52.0) : 52.0f;
    float v21 = Widtha * 0.5f;
    float x = 320.0f - v21;
    GL_DrawTexture(253, x, 134.0f, Widtha, 16.0f, 0.0f, 0.0f, 0.625f, 0.5625f, 1, 1);
    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, gFont.GetFont(FONT_NORMAL) ? gFont.GetFont(FONT_NORMAL) : gFont.GetFont(FONT_BOLD));
    m_dwBackColor = 0;

    CHAR String[29];
    String[0] = (char)byte_7EAA1A4;
    memset(&String[1], 0, sizeof(String) - 1);
    m_dwTextColor = 0xFFC44400u;   // -3899264
    SelectObject(m_hFontDC, gFont.GetFont(FONT_BIG));
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
        GL_DrawTexture(277, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);

        if (v7 == 10) {
            int v9 = (v20 == 10) ? ((v8 != 0) + 1) : 0;
            GL_DrawTexture(v9 + 254, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
        } else {
            int v10 = (v20 == v7 && v8) ? 278 : 277;
            GL_DrawTexture(v10, v25_x, v24, 32.0f, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
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
        GL_DrawTexture(textures[v12] + (v20 == v12 + 11 ? 1 : 0),
                     (float)v11, 239.0f, 70.0f, 21.0f,
                     0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);
        v11 += 78;
    }
}

void Render_HudPass_4EB070(void) { Render_HudPass_4EB070_(); }


// =============================================================================
// RenderBoolean — sub_480E00.
// =============================================================================

void __cdecl RenderBoolean(int x, int y, DWORD c)
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

    // Colores de IDA RenderBoolean L121-144 (el decompile los muestra como
    // decimales con signo):
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
    int drawX = x * (int)gWindow.GetWidth() / 640;
    int drawY = y * (int)gWindow.GetHeight() / 480;
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

    // Constantes de IDA RenderBoolean L180-195.
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
    int rectW = 640 * (int)cx / (int)gWindow.GetWidth();
    int rectH = 480 * (int)cy / (int)gWindow.GetHeight();
    if ((int)MouseX >= rectX && (int)MouseX < rectX + rectW &&
        (int)MouseY >= rectY && (int)MouseY < rectY + rectH &&
        InputEnable && Hero && *(BYTE*)((BYTE*)(uintptr_t)Hero + 846) &&
        strcmp((const char*)c, (const char*)((BYTE*)(uintptr_t)Hero + 449)) &&
        (int)(dword_7E11DA8 % 6) < 3)
    {
        m_dwBackColor = m_dwTextColor;
        m_dwTextColor = bg;
    }

    Font_RenderTextToBitmap(TextSize.cx, FontHeight, (LPCSTR)c, texW, 0, 0, 0, 0, (LPCSTR)(c + 24));
    Font_RenderBitmapText(drawX, drawY, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);

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
        Font_RenderTextToBitmap(TextSize.cx, FontHeight, (LPCSTR)(c + 300), texW, 0, 0, 0, 0, 0);
        Font_RenderBitmapText(drawX, drawY + FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);

        int fade1 = *(int*)(c + 556);
        m_dwTextColor = (fade1 < 10) ? 0x80C8DCE6u : 0xFFC8DCE6u;
        ClearFontRows(FontHeight);
        Font_RenderTextToBitmap(TextSize.cx, FontHeight, (LPCSTR)(c + 44), texW, 0, 0, 0, 0, 0);
        Font_RenderBitmapText(drawX, drawY + 2 * FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);
    } else {
        int fade1 = *(int*)(c + 556);
        if (fade1 > 0) {
            m_dwTextColor = (fade1 < 10) ? 0x80C8DCE6u : 0xFFC8DCE6u;
            ClearFontRows(FontHeight);
            Font_RenderTextToBitmap(TextSize.cx, FontHeight, (LPCSTR)(c + 44), texW, 0, 0, 0, 0, 0);
            Font_RenderBitmapText(drawX, drawY + FontHeight, *(float*)&TextSize.cx, *(float*)&FontHeight, texW, texH, 0.0f, 640);
        }
    }
}
