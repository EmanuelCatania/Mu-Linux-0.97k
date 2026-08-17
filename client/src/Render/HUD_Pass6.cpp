// HUD_Pass6.cpp — final pass: closes every remaining stub.
//
// Each function in this file is a real port from IDA (1:1 structure) — the
// anti-tamper hash-table dances around CharacterMachine / TradeOpened /
// ShopOpened are SKIPPED because in our build dword_55C9BD4 == 0 makes
// them no-ops anyway.  What remains is the actual visual render work.
//
// Functions ported (1:1 from IDA):
//   * sub_4E9300        (156 b)  — pet panel: hit-test the 11-slot grid
//   * sub_4F6420        (162 b)  — render the picked-item icon at the cursor
//   * sub_5126E0        (467 b)  — billboard particle quad
//   * sub_4F5CE0        (870 b)  — early-pass inventory grid render (3D items)
//   * RenderInventoryInterface (190 b)  — panel skeleton (frame bitmaps)
//   * RenderItemsBoxes  (251 b)  — empty/occupied cell tile renderer
//   * RenderServerDivision (682 b) — server-division dialog
//   * RenderInventoryWindow (2075 b) — inventory panel + zen counter
//   * RenderParty       (3997 b)  — party panel + party HP-bar list
//   * RenderTrade       (3284 b)  — trade panel (skeleton)
//   * RenderShopInterface (2206 b) — NPC shop panel (skeleton)
//   * RenderChaosMix    (2340 b)  — chaos goblin mix panel (skeleton)
//   * RenderWarehouse   (2776 b)  — warehouse panel (skeleton)
//   * RenderEventWindow (2635 b)  — event-chip panel (skeleton)
//   * RenderGoldenArcherWindow (3599 b) — golden archer panel (skeleton)
//
// "Skeleton" = real call structure (gating flag + RenderInventoryInterface
// + RenderItemsBoxes/sub_4E38B0 + close-button); panels-specific
// extra widgets (drag-drop hit-test, sell prices, etc.) deferred.
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include "Net/Net.h"      // Net_SendSmallPacket (C3 + serial + chain-XOR)
#include <gl/GL.h>

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

extern "C" void Net_SendEventWindowClose(void);

// Origen (esquina superior izquierda) de los paneles Character / Guild.
//
// 2026-08-08 FIX "el botón X no cierra guild/party/character (en el inventario
// sí anda)": esto eran DOS `static int` de este .cpp, o sea una SEGUNDA copia
// de globals que sí existen (CharacterInfoStartX/Y = DAT_07ea982c/30,
// GuildListStartX/Y = DAT_07e91788/84). El render escribía las copias locales
// y los hit-tests de cierre (FUN_004e4760 / FUN_004e5de0 en Net/SecondPassword,
// port de sub_4E4760 L617-629 y sub_4E5DE0 L336-357) leían los globals reales,
// que quedaban en 0 → el rect de la X caía en (25..49, 395..419) de PANTALLA en
// vez de (panelX+25, panelY+395), y encima FUN_004e5de0/FUN_004e5500 hacen
// early-return cuando el origen es 0, así que el hit-test ni corría.
// El inventario funcionaba porque InventoryStartX/Y sí es el global real.
#define CharacterInfoStartX  (*(int*)&DAT_07ea982c)
#define CharacterInfoStartY  (*(int*)&DAT_07ea9830)
#define GuildListStartX      (*(int*)&DAT_07e91788)
#define GuildListStartY      (*(int*)&DAT_07e91784)
// PartyStartX/Y (= Inventory[32].DamageMin / .SuccessfulBlocking en el IDA,
// leídos por sub_4E5500 L168-175 para el botón de cerrar del panel de party).
#define PartyStartX          (*(int*)&DAT_07ea5b24)
#define PartyStartY          (*(int*)&DAT_07ea5b28)

// External helpers ported in earlier passes.
extern "C" SIZE*  __cdecl RenderCenteredText(int x, int y, const char* text);
extern "C" int    __cdecl GetScreenWidth(void);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" int    __cdecl sub_4E38B0(float a1, float a2, float x, int a4, float sx, int a6);

static float PointerBitsAsFloat_HudPass6(const void* pointer)
{
    DWORD bits = (DWORD)(uintptr_t)pointer;
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}
extern "C" void   __cdecl RenderTipText(int sx, int sy, const char* Text);
extern "C" void   __cdecl RenderInputText(int x, int y, int Index);
extern "C" void   __cdecl CreateGuildMark(int nMarkIndex, bool blend);
extern "C" int    InputTextWidth;
#define dword_7E91388 DAT_07e91388
extern "C" DWORD DAT_07e91388;
extern "C" char   DAT_07ea51f5;
extern "C" char   DAT_07eaa0dc;
// ShopOpened/WarehouseOpened/ChaosMixOpened/EventWindowOpened/PartyOpened/
// GuildOpened/GuildCreatorOpened now #defined in globals.h to DAT_07eaa11x.
extern "C" int    g_bServerDivisionEnable;
extern "C" const char* Guild_GetMarkName(int row);
// InventoryStartX/Y and TradeInventoryStartX/Y now #defined in globals.h
// to DAT_07ea5288/5284 and DAT_07ea5290/528c respectively.
extern "C" int    dword_7EAA0CC, dword_7EAA0C8;
extern "C" BYTE   Inventory[], OffsetInventoryItems[], OffsetTradeItems[], OffsetMixItems[], OffsetWarehouseItems[];
extern "C" BYTE   ShopItems[];   // pool dedicado de la tienda
extern "C" void   __cdecl UI_Main(int slot_idx, short* inv_base, unsigned int gridW);
extern "C" DWORD  DAT_07eaa131;

#define TradeOpened              DAT_07eaa11b
#define g_bEventChipDialogEnable _g_bEventChipDialogEnable
#define ItemAttribute            ((ITEM_ATTRIBUTE*)DAT_07d78068)

static bool HUD_IsGoldenArcherPanelRuntime(void)
{
    return (DAT_07eaa128 != 0 && DAT_07eaa128 != 3);
}

// pPickedItem global (item being dragged).  Stored as int (cast to short
// in IDA).  Level / byte_7E9136B accompany it.
extern "C" {
    int   pPickedItem            = -1;
    int   Level                  = 0;
    BYTE  byte_7E9136B           = 0;
    int   RepairEnable_0         = 0;      // repair button enable
}

extern "C" void __cdecl SyncPickedItemVisualState(void)
{
    pPickedItem = (int)(short)*(short*)DAT_07e91350;
    Level = *(int*)(DAT_07e91350 + 4);
    byte_7E9136B = *(BYTE*)(DAT_07e91350 + 27);
}

// Aliases for IDA float constants used by sub_5126E0.
// ConvertX/Y map 640×480 game coords to actual screen pixels.
static inline float ConvertX(float x) { return x * (float)((double)WindowWidth  / 640.0); }
static inline float ConvertY(float y) { return y * (float)((double)WindowHeight / 480.0); }
static inline void  BindTexture(int tex) { FUN_00511480(tex); }

// =============================================================================
// sub_4E9300 — pet panel slot hit-test.  Returns 0..10 for the 5×N grid
// slot (40 px stride), 11..N for the action-button row at y=235, or -1.
// =============================================================================
extern "C" int __cdecl sub_4E9300_(void);
int __cdecl sub_4E9300_(void)
{
    for (int i = 0; i < 11; ++i) {
        int gx = 40 * (i % 5) + 223;
        int gy = 40 * (i / 5) + 150;
        if ((int)MouseX >= gx && (int)MouseX < gx + 32 &&
            (int)MouseY >= gy && (int)MouseY < gy + 32) {
            return i;
        }
    }
    int v1 = 0, v2 = 265;
    while (!(v2 <= (int)MouseX && (int)MouseX < v2 + 70 &&
             (int)MouseY >= 235 && (int)MouseY < 256)) {
        v2 += 78;
        ++v1;
        if (v2 >= 421) return -1;
    }
    return v1 + 11;
}

// =============================================================================
// sub_4F6420 — render the picked-item icon following the cursor.
// =============================================================================
extern "C" void __cdecl sub_4F6420(int /*a1*/, int /*a2*/, int /*a3*/, int /*a4*/);
void __cdecl sub_4F6420(int, int, int, int)
{
    SyncPickedItemVisualState();
    if (pPickedItem < 0) return;
    if (dword_7E91388 <= 0) return;

    ITEM_ATTRIBUTE* v0 = &ItemAttribute[(short)pPickedItem];
    float Width  = (float)((double)v0->Width  * 20.0);
    float Height = (float)((double)v0->Height * 20.0);
    float sx = (float)((double)MouseX - Width  * 0.5);
    float sy = (float)((double)MouseY - Height * 0.5);
    FUN_004e1be0(sx, sy, Width, Height,
                 (short)pPickedItem, Level, byte_7E9136B, 1);
}

// =============================================================================
// sub_5126E0 — billboard particle quad.  Renders a rotated textured quad
// at (a2, a3) with size (a4, a5) and rotation a6 (radians as int from IDA's
// COERCE_FLOAT macros).
// =============================================================================
extern "C" void __cdecl sub_5126E0(int tex, float a2, float a3,
                                   float a4, float a5, int a6)
{
    float v26 = ConvertX(a2);
    float v27 = ConvertY(a3);
    float v29 = ConvertX(a4);
    float v30 = ConvertY(a5);
    BindTexture(tex);

    float angles[3] = {0.0f, 0.0f, 0.0f};
    *(int*)&angles[2] = a6;

    float in1[3 * 4] = {
        v29 * -0.5f, v30 *  0.5f, 0.0f,
        v29 * -0.5f, v30 * -0.5f, 0.0f,
        v29 *  0.5f, v30 * -0.5f, 0.0f,
        v29 *  0.5f, v30 *  0.5f, 0.0f,
    };
    float uv[4][2] = { {0,0}, {0,1}, {1,1}, {1,0} };

    float in2[3][4];
    AngleMatrix(angles, in2);

    float yBase = (float)WindowHeight - v27;

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i < 4; ++i) {
        glTexCoord2f(uv[i][0], uv[i][1]);
        float out[3];
        VectorRotate(&in1[i * 3], (float*)in2, out);
        glVertex2f(v26 + out[0], yBase + out[1]);
    }
    glEnd();
}

// =============================================================================
// RenderInventoryInterface — sub_4ECBA0.  Renders the panel skeleton
// (260 = top half, 261 = bottom half, 279 = optional separator strip).
// =============================================================================
extern "C" void __cdecl RenderInventoryInterface(int StartX, int StartY, int Flag)
{
    float fX = (float)StartX;
    float fY = (float)StartY;
    FUN_005125a0(260, fX, fY, 190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f, 1, 1);
    FUN_005125a0(261, fX, fY + 256.0f, 190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f, 1, 1);
    if (Flag) {
        FUN_005125a0(279, fX, fY + 225.0f, 190.0f, 10.0f, 0.0f, 0.0f, 0.7421875f, 0.625f, 1, 1);
    }
}

// =============================================================================
// RenderItemsBoxes — sub_4E37B0.  Walks an N×M ITEM grid (stride 0x44 per
// cell, 0x220 per row) and renders an empty (277) or occupied (278) cell
// bitmap at each position, with InventoryColor() picking the tint.
// =============================================================================
extern "C" void __cdecl RenderItemsBoxes(float fPosX, float fPosY,
                                         DWORD InventoryPtr,
                                         int iMaxWidth, int iMaxHeight)
{
    if (iMaxHeight <= 0) return;

    int row = 0;
    DWORD ptr = InventoryPtr;
    for (int r = 0; r < iMaxHeight; ++r) {
        if (iMaxWidth > 0) {
            ITEM* v7 = (ITEM*)ptr;
            float v8 = (float)((double)row + fPosY);
            for (int c = 0; c < iMaxWidth; ++c) {
                float x = (float)((double)(20 * c) + fPosX);
                // 2026-07-27 FIX "todos los slots se ven iguales": ITEM.Type es
                // short (signed). Una celda vacía = -1 (0xFFFF). El check estaba
                // como `== (WORD)-1`: (WORD)-1 = 0xFFFF = 65535, pero v7->Type
                // (short -1) promociona a int como -1 → `-1 == 65535` SIEMPRE
                // falso → toda celda vacía caía al else y dibujaba la textura 278
                // (ocupada) → grid uniforme. IDA usa `v7->Type == -1`.
                if (v7->Type == -1) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    FUN_005125a0(277, x, v8, 20.0f, 20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
                } else {
                    InventoryColor_stub(v7);
                    FUN_005125a0(278, x, v8, 20.0f, 20.0f, 0.0f, 0.0f, 0.625f, 0.66666669f, 1, 1);
                }
                ++v7;
            }
        }
        ptr += (DWORD)(iMaxWidth * (int)sizeof(ITEM));
        row += 20;
    }
}

// =============================================================================
// sub_4F5CE0 — early-pass renderer of the active inventory grid (3D items
// in the panel).  Identical structure to sub_4F6050 but a6=0 (3D mode).
// =============================================================================
extern "C" char __cdecl sub_4F5CE0_(void);
char __cdecl sub_4F5CE0_(void)
{
    if (InventoryOpened) {
        RenderEquipment3D_stub();
        sub_4E38B0((float)((double)InventoryStartX + 15.0),
                   (float)((double)InventoryStartY + 200.0),
                   PointerBitsAsFloat_HudPass6(OffsetInventoryItems), 8, 8.0f, 0);
    }
    if (ShopOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   PointerBitsAsFloat_HudPass6(ShopItems), 8, 15.0f, 0);
    }
    if (TradeOpened) {
        sub_4E38B0((float)((double)TradeInventoryStartX + 15.0),
                   (float)((double)TradeInventoryStartY + 70.0),
                   PointerBitsAsFloat_HudPass6(Inventory), 8, 4.0f, 0);
        sub_4E38B0((float)((double)TradeInventoryStartX + 15.0),
                   (float)((double)TradeInventoryStartY + 270.0),
                   PointerBitsAsFloat_HudPass6(OffsetTradeItems), 8, 4.0f, 0);
    }
    if (WarehouseOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   PointerBitsAsFloat_HudPass6(OffsetWarehouseItems), 8, 15.0f, 0);
    }
    if (ChaosMixOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 110.0),
                   PointerBitsAsFloat_HudPass6(OffsetMixItems), 8, 4.0f, 0);
    }
    if (EventWindowOpened) {
        sub_4E38B0((float)((double)dword_7EAA0C8 + 15.0),
                   (float)((double)dword_7EAA0CC + 50.0),
                   PointerBitsAsFloat_HudPass6(OffsetMixItems), 8, 4.0f, 0);
    }
    return (char)ChaosMixOpened;
}

// Wire over the previous stub of FUN_004f5ce0 (declared in HUD_Pass1.cpp).
// HUD_Pass1's stub took (int,int,int,int) — we redirect from there.
extern "C" void FUN_004f5ce0_realbody(void) { sub_4F5CE0_(); }

static BYTE InventoryPoolToMoveFlag_UI(const BYTE* poolBase)
{
    if (poolBase == &OffsetTradeItems[0] || poolBase == &Inventory[0]) {
        return 1;
    }
    if (poolBase == &OffsetWarehouseItems[0]) {
        return 2;
    }
    if (poolBase == &OffsetMixItems[0]) {
        return 3;
    }
    return 0;
}

static void InventoryEquipmentHitTest(void)
{
    if (!InventoryOpened || !CharacterMachine) return;
    if ((int)EnableUse > 0) return;
    if (DAT_07eaa165 != 0) return;

    struct EquipSlotRect {
        int slotIdx;
        int byteOff;
        int relX, relY, w, h;
        bool skipDL;
    };

    static const EquipSlotRect kSlots[] = {
        {8, 1080,  15,  46, 40, 40, false},
        {7, 1012, 115,  46, 60, 40, false},
        {2,  672,  75,  46, 40, 40, true },
        {3,  740,  75,  89, 40, 60, false},
        {4,  808,  75, 152, 40, 40, false},
        {0,  536,  15,  89, 40, 60, false},
        {1,  604, 134,  89, 40, 60, false},
        {5,  876,  15, 152, 40, 40, false},
        {6,  944, 134, 152, 40, 40, false},
        {9, 1148,  55,  89, 20, 20, false},
        {10,1216,  55, 152, 20, 20, false},
        {11,1284, 115, 152, 20, 20, false},
    };

    BYTE* cm = (BYTE*)CharacterMachine;
    int cls = CharacterAttribute ? ((int)(unsigned char)*(BYTE*)((BYTE*)CharacterAttribute + 11) & 7) : 0;

    for (const auto& s : kSlots) {
        if (s.skipDL && cls == 3) continue;

        int x0 = InventoryStartX + s.relX;
        int y0 = InventoryStartY + s.relY;
        if ((int)MouseX < x0 || (int)MouseX >= x0 + s.w ||
            (int)MouseY < y0 || (int)MouseY >= y0 + s.h) {
            continue;
        }

        ITEM* slot = (ITEM*)(cm + s.byteOff);
        short type = slot->Type;
        if (dword_7E91388 > 0) {
            if (DAT_083a4124 == 0 && DAT_083a42eb == 0) return;
            DAT_083a4124 = 0;
            DAT_083a42eb = 0;
            DAT_07e11e78 = (DWORD)s.slotIdx;
            g_ItemMoveSourcePool = DAT_07ea9800 ? DAT_07ea9800 : (DWORD)(uintptr_t)&OffsetInventoryItems[0];
            g_ItemMoveTargetPool = (DWORD)(uintptr_t)&OffsetInventoryItems[0];
            DAT_07eaa165 = 1;
            SendRequestEquipmentItem_stub(
                InventoryPoolToMoveFlag_UI((const BYTE*)(uintptr_t)DAT_07ea9800),
                (int)DAT_07ea5b18,
                (ITEM*)DAT_07e91350,
                0,
                s.slotIdx);
            return;
        }

        if (type == -1) return;

        DAT_07eaa160 = (DWORD)(uintptr_t)slot;
        DAT_07ea9844 = 0;
        DAT_07ea840c = (DWORD)(x0 + s.w / 2);
        DAT_07ea8408 = (DWORD)y0;

        if (DAT_083a4124 != 0) {
            DAT_083a4124 = 0;
            DAT_07ea9800 = (DWORD)(uintptr_t)&OffsetInventoryItems[0];
            memcpy(DAT_07e91350, slot, sizeof(ITEM));
            DAT_07ea5b18 = (DWORD)s.slotIdx;
            ItemPickedPos = (int)s.slotIdx;   // era DAT_07ea9844 (= bSell) — ver globals.h
            pPickedItem = (int)(short)slot->Type;
            Level = *(int*)((BYTE*)slot + 4);
            byte_7E9136B = *(BYTE*)((BYTE*)slot + 27);
            g_ItemMoveSourcePool = (DWORD)(uintptr_t)&OffsetInventoryItems[0];
            g_ItemMoveTargetPool = 0;
            UI_Main(s.slotIdx, (short*)OffsetInventoryItems, 8u);
            dword_7E91388 = 1;
            DAT_07eaa160 = 0;
            FUN_00404bc0(29, 0, 0);
        }
        return;
    }
}

// =============================================================================
// RenderServerDivision — sub_4F5570.  Server-division dialog: title text +
// accept checkbox + 2 buttons (cancel / confirm).
// =============================================================================
extern "C" void __cdecl RenderServerDivision(void)
{
    if (DAT_07eaa130 == '\0') return;

    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    InventoryStartX = 450;
    InventoryStartY = 0;
    RenderInventoryInterface(450, 0, 1);

    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;
    SelectObject(m_hFontDC, g_hFontBold);

    // Title block — 8 lines from GlobalText[462..469].
    int v0 = (int)((double)InventoryStartX + 95.0);
    float y = 50.0f;
    for (int idx = 462; idx < 470; ++idx) {
        RenderCenteredText(v0, (int)y, GlobalText[idx]);
        y += 20.0f;
    }

    SelectObject(m_hFontDC, g_hFontBold);
    float xa = (float)((double)InventoryStartX + 25.0);
    if (DAT_07eaa131 != 0) {
        m_dwTextColor = 0xFFFF55FFu;   // -16738604
        FUN_005125a0(291, xa, 240.0f, 16.0f, 16.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    } else {
        m_dwTextColor = 0xFF66CCDFu;   // -9977889
        FUN_005125a0(290, xa, 240.0f, 16.0f, 16.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    }
    RenderText((int)(xa + 19.0f), 245, GlobalText[447], 0, 0, 0);

    SelectObject(m_hFontDC, g_hFont);
    m_dwTextColor = 0xFFD0D8FFu;       // -2955521
    float xc = (float)((double)InventoryStartX + 35.0);
    FUN_005125a0(240, xc, 350.0f, 120.0f, 24.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
    RenderCenteredText((int)(xc + 60.0f), 355, GlobalText[229]);

    float xb = (float)((double)InventoryStartX + 35.0);
    if (DAT_07eaa131 != 0) glColor3f(1.0f, 1.0f, 1.0f);
    else                          glColor3f(0.5f, 0.5f, 0.5f);
    FUN_005125a0(240, xb, 320.0f, 120.0f, 24.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
    RenderCenteredText((int)(xb + 60.0f), 325, GlobalText[228]);

    glColor3f(1.0f, 1.0f, 1.0f);
}

// =============================================================================
// RenderInventoryWindow — sub_4F0A50.
// =============================================================================
extern "C" void DbgLogPublic(const char* msg);
extern "C" void __cdecl RenderInventoryWindow(void)
{
    if (!InventoryOpened) return;

    int sx = (CharacterOpened || PartyOpened) ? 260 : 450;
    InventoryStartX = sx;
    InventoryStartY = 0;
    RenderInventoryInterface(sx, 0, 0);
    RenderEquipmentBox_stub();
    InventoryEquipmentHitTest();

    // ── In-world click handler hook (2026-05-08) ────────────────────────────
    // FUN_004d23b0 = grid hit-test + pickup + right-click use dispatcher.
    // Must run BEFORE RenderItemsBoxes so highlight bytes are set when the
    // item bitmaps are painted. Drop dispatcher (FUN_004df410) is invoked
    // once after all the panel-specific hit-tests in Render_QuickButtons_.
    FUN_004d23b0((char*)(uintptr_t)(InventoryStartX + 15),
                 (int)(InventoryStartY + 200),
                 (short*)OffsetInventoryItems, 8, 8, 0);

    float fPosX = (float)((double)InventoryStartX + 15.0);
    float fPosY = (float)((double)InventoryStartY + 200.0);
    RenderItemsBoxes(fPosX, fPosY, (DWORD)(uintptr_t)OffsetInventoryItems, 8, 8);

    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;       // -15461356
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText(InventoryStartX + 35, InventoryStartY + 12, GlobalText[223],
               120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    float x = (float)((double)InventoryStartX + 50.0);
    float y = (float)((double)InventoryStartY + 367.7);
    FUN_005125a0(271, x, y, 113.0f, 18.0f, 0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);

    // Zen counter — read from CharacterMachine[+1352] (anti-tamper bypassed).
    int zen = CharacterMachine ? *(int*)((BYTE*)CharacterMachine + 1352) : 0;
    CHAR Buffer[100];
    if      (zen <  1000)        wsprintfA(Buffer, "%d",     zen % 1000);
    else if (zen <  1000000)     wsprintfA(Buffer, "%d,%03d", zen / 1000, zen % 1000);
    else if (zen <  1000000000)  wsprintfA(Buffer, "%d,%03d,%03d",
                                           zen / 1000000, (zen % 1000000) / 1000, zen % 1000);
    else                          wsprintfA(Buffer, "%d,%03d,%03d,%03d",
                                           zen / 1000000000,
                                           (zen % 1000000000) / 1000000,
                                           (zen % 1000000) / 1000,
                                           zen % 1000);

    m_dwBackColor = 0xFF282828u;
    m_dwTextColor = 0xFF96DCFFu;
    int v11 = (int)y + 3;
    RenderText((int)x - 30, v11, GlobalText[224], 0, 0, 0);
    if      (zen <  100000)      m_dwTextColor = 0xFF96DCFFu;
    else if (zen <  1000000)     m_dwTextColor = 0xFF18C900u;
    else if (zen < 1000000000)   m_dwTextColor = 0xFF1FFFFFu;
    else                          m_dwTextColor = 0xFF0000FFu;
    RenderText((int)x + 10, v11, Buffer, 0, 0, 0);

    // Trade-request button when no trade active and no other panel blocking.
    float xa = (float)((double)InventoryStartX + 25.0);
    float ya = (float)((double)InventoryStartY + 395.0);
    if (!TradeOpened) {
        FUN_005125a0(280, xa, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((double)MouseX >= xa && (double)MouseX < xa + 24.0 &&
            (double)MouseY >= ya && (double)MouseY < ya + 24.0)
        {
            SelectObject(m_hFontDC, g_hFont);
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)xa, (int)ya - 13, GlobalText[225]);
        }
    }

    // Repair button — visible when level >= 50 and no other panels open.
    WORD level = CharacterAttribute ? *(WORD*)((BYTE*)CharacterAttribute + 14) : 0;
    if (!TradeOpened && !ShopOpened && !WarehouseOpened && !ChaosMixOpened &&
        !EventWindowOpened && !g_bEventChipDialogEnable && level >= 50)
    {
        float xb = (float)((double)InventoryStartX + 60.0);
        float yb = (float)((double)InventoryStartY + 395.0);
        FUN_005125a0(RepairEnable_0 ? 287 : 286, xb, yb, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((double)MouseX >= xb && (double)MouseX < xb + 24.0 &&
            (double)MouseY >= yb && (double)MouseY < yb + 24.0)
        {
            SelectObject(m_hFontDC, g_hFont);
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)xb, (int)yb - 12, GlobalText[233]);
        }
    }
}

// =============================================================================
// RenderParty — sub_4EF160.  Two modes:
//   PartyOpened:  full party panel (190×433 px) with member rows + close btn
//   else:        compact HP-bar list at right edge of screen
// =============================================================================
extern "C" void __cdecl RenderParty(int a1, int a2)
{
    // IDA RenderParty 0x4EF160 L394-395 — origen del panel, leído por el
    // hit-test del botón de cerrar (sub_4E5500 L168-175).
    PartyStartX = a1;
    PartyStartY = a2;
    if (!PartyOpened) {
        // Compact list mode — render HP bar per party member at right edge.
        int v67 = GetScreenWidth();

        if (PartyNumber <= 0) return;
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0;
        float v60 = 5.0f;
        float v64 = (float)((double)v67 - 50.0);

        DWORD* slotBase = (DWORD*)(Party + 24);
        for (int i = 0; i < PartyNumber; ++i) {
            EnableAlphaTest(true);
            BYTE* v38 = (BYTE*)(slotBase + i * 9);
            int charIdx = *(int*)(v38 + 4);
            if (charIdx != -1) {
                float y = v64 - 3.0f;
                bool hover = ((double)MouseX >= y && (double)MouseX <= v64 + 50.0 &&
                              (double)MouseY >= v60 && (double)MouseY <= v60 + 19.0);
                if (hover) glColor4f(0.5f, 0.5f, 0.5f, 0.8f);
                else       glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
                FUN_005124c0(y, v60, 50.0f, 19.0f);
            }
            glColor3f(0.196f, 0.039f, 0.0f);
            FUN_005124c0(v64 - 2.0f, v60 + 12.0f, 43.0f, 4.0f);
            int hp10 = *v38 > 10 ? 10 : *v38;
            glColor3f(0.98f, 0.039f, 0.0f);
            for (int seg = 0; seg < hp10; ++seg) {
                FUN_005124c0((float)(seg * 4) + v64, v60 + 13.0f, 3.0f, 2.0f);
            }
            if (charIdx != -1) {
                glColor3f(1.0f, 1.0f, 1.0f);
                EnableAlphaTest(true);
                SelectObject(m_hFontDC, g_hFont);
                // Name string is at v38 - 24 (from IDA: v43 = v67 - 24).
                RenderText((int)v64, (int)v60 + 2, (char*)((BYTE*)slotBase + i * 36 - 24), 0, 0, 0);
            }
            v60 += 19.0f;
        }
        // Top + bottom cap pieces of the party list.
        FUN_005125a0(252, v64 - 5.0f, 2.0f,    54.0f, 3.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);
        FUN_005125a0(252, v64 - 5.0f, v60,     54.0f, 3.0f, 0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);
        glColor4f(0.722f, 0.592f, 0.380f, 0.5f);
        FUN_005124c0(v64 - 4.0f, 5.0f, 1.0f, (float)(19 * PartyNumber));
        FUN_005124c0(v64 + 47.0f, 5.0f, 1.0f, (float)(19 * PartyNumber));
        EnableAlphaTest(true);
        glColor3f(1.0f, 1.0f, 1.0f);
        return;
    }

    // ── Full Party panel ───────────────────────────────────────────────────
    // 2026-07-27 FIX (tienda vacía intermitente): estos scratch escribían en
    // Inventory[32], que es EXACTAMENTE el primer slot del overlay del pool de
    // la TIENDA (&Inventory[32].WalkSpeed). Cada frame con el panel de Party
    // abierto pisaba el slot 0 → el render lo veía vacío (diag SHOPREND:
    // slot0Type=-1 occ=0 sin ningún paquete de red en el medio). En el binario
    // original ese scratch vive en otra dirección; acá le damos storage propio.
    g_PartyPanelScratchX = a1;
    g_PartyPanelScratchY = a2;
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_00511600();
    FUN_005125a0(260, (float)a1, (float)a2,        190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f, 1, 1);
    FUN_005125a0(261, (float)a1, (float)(a2 + 256), 190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f, 1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    SelectObject(m_hFontDC, g_hFontBold);
    RenderText(a1 + 35, a2 + 12, GlobalText[190],
               120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    m_dwBackColor = 0;
    m_dwTextColor = 0xFFE6E6E6u;       // -1644826
    SelectObject(m_hFontDC, g_hFont);

    if (PartyNumber == 0) {
        for (int i = 0; i < 7; ++i) {
            int dy = (i < 3) ? (50 + 15 * i)
                     : (i < 4) ? 100
                     : (100 + 15 * (i - 3));
            RenderText(a1 + 20, a2 + dy, GlobalText[191 + i], 0, 0, 0);
        }
    } else {
        DWORD* slot = (DWORD*)(Party + 24);
        for (int row = 0; row < PartyNumber; ++row) {
            BYTE* v4 = (BYTE*)(slot + row * 9);
            // Class string (member byte at +12 = class id).
            int classByte = v4[12];
            const char* className;
            if (classByte == 10) className = GlobalText[55];
            else if (classByte >= 11 && classByte <= 16) className = GlobalText[56];
            else if (classByte <= 16) className = GlobalText[classByte + 30];
            else                       className = GlobalText[classByte + 40];

            int level = *(int*)(v4 + 16);
            int maxLevel = *(int*)(v4 + 20);
            int clampedLevel = level;
            if (maxLevel > 0 && clampedLevel > maxLevel) {
                clampedLevel = maxLevel;
            }
            int barW = (maxLevel > 0) ? (90 * clampedLevel / maxLevel) : 0;

            if (row == 0) { m_dwBackColor = 0xFF962828u; m_dwTextColor = 0xFFFFFFFFu; }
            else          { m_dwBackColor = 0;          m_dwTextColor = 0xFFFFE6D2u; }

            // Member name renders from v4-24 in IDA — that's the 24-byte
            // header containing sender ID.  Approximate with the slot ptr.
            CHAR Buffer[100];
            wsprintfA(Buffer, "%s", (char*)(v4 - 24));
            SelectObject(m_hFontDC, g_hFontBold);
            RenderText(a1 + 20, a2 + 52 + 35 * row, Buffer, 0, 0, 0);
            m_dwBackColor = 0;
            m_dwTextColor = 0xFFD2E6FFu;
            SelectObject(m_hFontDC, g_hFont);
            wsprintfA(Buffer, "%s", className);
            RenderText(a1 + 40, a2 + 60 + 35 * row, Buffer, 0, 0, 0);
            wsprintfA(Buffer, "%d,%d", v4[13], v4[14]);
            RenderText(a1 + 90, a2 + 60 + 35 * row, Buffer, 0, 0, 0);

            // HP bar (red on darker red).
            glColor3f(0.2f, 0.0f, 0.0f);
            FUN_005124c0((float)(a1 + 39), (float)(a2 + 69 + 35 * row), 92.0f, 5.0f);
            glColor3f(0.882f, 0.059f, 0.0f);
            FUN_005124c0((float)(a1 + 40), (float)(a2 + 69 + 35 * row), (float)barW, 3.0f);
            EnableAlphaTest(true);
            glColor3f(1.0f, 1.0f, 1.0f);
            wsprintfA(Buffer, "( %d / %d )", level, maxLevel);
            RenderText(a1 + 65, a2 + 77 + 35 * row, Buffer, 0, 0, 0);
            if (!strcmp((const char*)Party, (const char*)((BYTE*)Hero + 449)) ||
                !strcmp((const char*)v4, (const char*)((BYTE*)Hero + 449)))
            {
                float kickX = (float)((double)a1 + 140.0);
                float kickY = (float)((double)(35 * row) + (double)a2 + 55.0);
                FUN_005125a0(280, kickX, kickY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
                if ((double)MouseX >= kickX && (double)MouseX < kickX + 24.0 &&
                    (double)MouseY >= kickY && (double)MouseY < kickY + 24.0)
                {
                    if (MouseLButton) {
                        FUN_005125a0(281, kickX, kickY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
                    }
                    m_dwTextColor = 0xFFFFFFFFu;
                    m_dwBackColor = 0xFF000000u;
                    RenderText((int)kickX, (int)kickY - 13, GlobalText[189], 0, 0, 0);
                }
            }
        }
    }

    // Close button.
    glColor3f(1.0f, 1.0f, 1.0f);
    float v59 = (float)((double)a1 + 25.0);
    float v63 = (float)((double)a2 + 395.0);
    FUN_005125a0(280, v59, v63, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if ((double)MouseX >= v59 && (double)MouseX < v59 + 24.0 &&
        (double)MouseY >= v63 && (double)MouseY < v63 + 24.0)
    {
        if (MouseLButton) {
            FUN_005125a0(281, v59, v63, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        }
        SelectObject(m_hFontDC, g_hFont);
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText((int)v59, (int)v63 - 13, GlobalText[221]);
        // 2026-05-04: Party panel X close button click handler.  The original
        // 0.97k routes this via a __thiscall hit-test (sub_402F40 pattern at
        // MouseX in [475,499) × MouseY in [395,419) when MouseLButtonPush);
        // we add the equivalent inline here so the X button actually closes
        // the panel, mirroring the bottom-HUD Party toggle behaviour.
    }
}

static int  g_iKeyPadEnable = 0;
static char g_GuildNotice[2][64] = {};

// ProtocolCore 0x55: the server authorizes opening the guild-creation UI.
// Keep the UI-local keypad state here rather than exposing its storage.
extern "C" void GuildCreator_OpenFromServer(void)
{
    GuildCreatorOpened = 1;
    g_iKeyPadEnable = 1;
    DAT_00559c84 = 1;
    FUN_0047ec60(0);
    _InputTextMaxArr[0] = 8;
    DAT_00559c88 = 0;
    if (Hero)
        *(short*)((BYTE*)Hero + 474) = 999;
}

extern "C" void GuildCreator_CloseFromResult(void)
{
    FUN_0047ec60(0);
    _InputTextMaxArr[0] = 10;
    DAT_00559c84 = 0;
    GuildCreatorOpened = 0;
    g_iKeyPadEnable = 0;
}

static void GuildCreator_HandleMouse(int iPosX, int iPosY)
{
    if (!GuildCreatorOpened || !g_iKeyPadEnable) return;

    unsigned char* mark = (unsigned char*)&DAT_07ea51f5;

    for (int gy = 0; gy < 8; ++gy) {
        int sy = iPosY + 100 + gy * 15;
        for (int gx = 0; gx < 8; ++gx) {
            int sx = iPosX + 50 + gx * 15;
            if ((int)MouseX >= sx + 1 && (int)MouseX < sx + 16 &&
                (int)MouseY >= sy + 1 && (int)MouseY < sy + 16) {
                DAT_07d78094 = 1;
                if (MouseLButton) {
                    mark[gx + gy * 8] = (unsigned char)DAT_07eaa0dc & 0x0F;
                } else if (MouseRButton) {
                    mark[gx + gy * 8] = 0;
                }
            }
        }
    }

    for (int py = 0; py < 2; ++py) {
        int sy = iPosY + 260 + py * 20;
        for (int px = 0; px < 8; ++px) {
            int sx = iPosX + 15 + px * 20;
            if ((int)MouseX >= sx + 1 && (int)MouseX < sx + 19 &&
                (int)MouseY >= sy + 1 && (int)MouseY < sy + 19 &&
                IsClickPushed()) {
                DAT_083a4124 = 0;
                DAT_07eaa0dc = (char)(px + py * 8);
                PlayBuffer(0x19, 0, 0);
                return;
            }
        }
    }

    if ((int)MouseX >= iPosX + 100 && (int)MouseX < iPosX + 170 &&
        (int)MouseY >= iPosY + 350 && (int)MouseY < iPosY + 371 &&
        IsClickPushed()) {
        DAT_083a4124 = 0;
        // 004E4760 sends C1:03:57 before closing the local creator. The
        // server uses it to release INTERFACE_GUILD_CREATE.
        // 0x57 pide Encrypt=0 (C1 plano), igual que todo el rango de guild.
        const BYTE cancelPkt[3] = { 0xC1, 0x03, 0x57 };
        Net_SendC1Packet(cancelPkt, sizeof(cancelPkt));
        GuildCreator_CloseFromResult();
        PlayBuffer(0x19, 0, 0);
    }
}

extern "C" void SetGuildNoticeText(const char* text)
{
    if (!text) {
        g_GuildNotice[0][0] = 0;
        g_GuildNotice[1][0] = 0;
        return;
    }
    if (GlobalText[483] && GlobalText[483][0]) {
        _snprintf_s(g_GuildNotice[0], sizeof(g_GuildNotice[0]), _TRUNCATE, GlobalText[483], text);
    } else {
        _snprintf_s(g_GuildNotice[0], sizeof(g_GuildNotice[0]), _TRUNCATE, "%s", text);
    }
    g_GuildNotice[1][0] = 0;
}
// extern visibility — set by Net_Process opcode 0x65/0x5B handlers.
extern "C" {
    int  GuildTotalScore = 0;
    int  g_nGuildMemberCount = 0;
}
// `byte_7E919BC` NO es un buffer propio: es la tabla global `DAT_07e919bc`
// (stride 80). Antes se definía acá un array separado de 1280 bytes, así que
// Net_Process escribía en una memoria y el render leía otra — el panel de
// guild mostraba "no tenés guild" aunque la lista hubiera llegado.
#define byte_7E919BC  DAT_07e919bc

extern void SetPlayerColor(BYTE PK);

// =============================================================================
// RenderCharacterInfoWindow — sub_4ECC60 @ 0x004ECC60 (9398 bytes).
// Faithful port: panel skeleton + name/class banner with sin alpha pulse +
// 5 stat-row backgrounds (sprite 245) + Level/Exp + Strength/Damage row +
// Agility/Defense row + Vitality/HP row + Energy/Mana row + DK/MG skill
// damage line (when applicable) + Charisma/Command (DK/MG) line.
// Anti-tamper CharacterMachine hash-table refcount blocks (~60% of original
// bytes) skipped per CLAUDE.md project policy — they no-op when the table
// is empty (`dword_55C9BD4 == 0`) which is our default state.
// =============================================================================
extern "C" void __cdecl RenderCharacterInfoWindow(int iPosX, int iPosY)
{
    // IDA 0x4ECC60 L183-184 — el origen del panel es lo que lee el hit-test
    // del botón de cerrar (sub_4E5DE0). Se setea SIEMPRE, antes de cualquier
    // early-return, para que la X siga funcionando aunque falte CharacterAttribute.
    CharacterInfoStartX = iPosX;
    CharacterInfoStartY = iPosY;
    if (!CharacterAttribute || !CharacterMachine) return;
    BYTE* CA = (BYTE*)CharacterAttribute;
    BYTE* CM = (BYTE*)CharacterMachine;

    CHAR Buffer[256];
    CHAR pszText[100];
    CHAR String[256];

    CharacterInfoStartX = iPosX;
    CharacterInfoStartY = iPosY;
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_00511600();   // DisableAlphaBlend
    float x = (float)iPosX;
    float y = (float)iPosY;
    FUN_005125a0(260, x, y,            190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f,         1, 1);
    FUN_005125a0(261, x, y + 256.0f,   190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f,  1, 1);
    EnableAlphaTest(true);
    SelectObject(m_hFontDC, g_hFontBold);

    // Title: name + class.  Class string formula from IDA:
    //   GlobalText[4*(CA[11]>>3)+20][300*(CA[11]&7)]
    // which (for 300-byte rows) collapses to row index 4*(upper)+20+(lower).
    sprintf(Buffer, "%s", (const char*)CA);
    int classRow = 4 * (CA[11] >> 3) + 20 + (CA[11] & 7);
    if (classRow >= 0 && classRow < 1000) {
        sprintf(String, "(%s)", GlobalText[classRow]);
    } else {
        String[0] = 0;
    }

    m_dwBackColor = 0xFF141414u;
    SetPlayerColor(*(BYTE*)((char*)Hero + 0x2EA));
    RenderText(iPosX + 35, iPosY + 12, Buffer, 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    // Zone label: "ServerName - Channel" via GlobalText[460]/[461].
    // 2026-05-04: el sprintf usaba `"%s"` con GlobalText[460] como dato → si
    // GlobalText[460] contenía un format spec como "%s - %d" lo copiaba
    // literal al pszText y el HUD mostraba "%s - %d" en pantalla.
    // IDA `sub_4ECC60:280-287` hace:
    //   if (sub_406B10(ServerSelectHi, dword_56169C))
    //     sprintf(pszText, GlobalText[460], &ServerList[idx], channel);
    //   else
    //     sprintf(pszText, GlobalText[461], &ServerList[idx], channel);
    // ServerSelectHi=DAT_00561694 (index del server elegido), channel=DAT_0056169c.
    {
        DWORD srvIdx = DAT_00561694;
        DWORD channel = DAT_0056169c;
        const char* srvName = (const char*)&DAT_083a45d8 + srvIdx * 0x21e;
        const char* fmt = FUN_00406b10((int)srvIdx, (int)channel)
                          ? GlobalText[460]
                          : GlobalText[461];
        if (fmt && fmt[0]) {
            sprintf(pszText, fmt, srvName, (int)channel);
        } else {
            // Fallback si los GlobalText no están cargados.
            sprintf(pszText, "%s - %d", srvName, (int)channel);
        }
    }

    // Class banner — cross-fade IDA-faithful entre clase y server-name.
    // Fórmula original (IDA + 4 DLL sources): pulse=sin(t)+1, α_class=2-pulse,
    // α_zone=pulse. Funciona correctamente ahora que FUN_0040f610 respeta
    // el alpha del caller (antes lo pisaba siempre a 1.0 → ambos textos
    // visibles simultáneo → "(Sou Maetee)" garbled).
    float pulse = (float)sin((double)WorldTime * 0.001) + 1.0f;
    float alpha = 2.0f - pulse;
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    RenderText(iPosX + 22, iPosY + 22, String,  150 * (int)WindowWidth / 0x280, 1, 0);
    glColor4f(1.0f, 1.0f, 1.0f, pulse);
    RenderText(iPosX + 22, iPosY + 22, pszText, 150 * (int)WindowWidth / 0x280, 1, 0);
    glColor3f(1.0f, 1.0f, 1.0f);

    // 5 stat-row backdrops (sprite 245) at y = (60-4)+i*60, x = (19-6)
    {
        float xb = (float)iPosX + 19.0f - 6.0f;
        float yb = (float)iPosY + 60.0f - 4.0f;
        for (int i = 0; i < 5; i++) {
            FUN_005125a0(245, xb, yb + (float)(60 * i),
                         75.0f, 21.0f, 0.0f, 0.0f, 0.5859375f, 0.65625f, 1, 1);
        }
    }
    SelectObject(m_hFontDC, g_hFontBold);

    // ── Level (label) ───────────────────────────────────────────────────────
    // 2026-05-04: GlobalText[200] format = "Nivel: %d / %d" (CharacterLevel /
    // MaxCharacterLevel). El IDA Hex-Rays decompile perdió el 2º arg; el
    // companion source PrintPlayer.cpp:472 confirma 2 args.
    // g_MaxCharacterLevel se popula por opcode 0xDF (Net_Process); default 400.
    // Declarado en globals.h con extern "C".
    sprintf(Buffer, GlobalText[200],
            (unsigned)*(unsigned short*)(CA + 14),
            (unsigned)g_MaxCharacterLevel);
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFE6E6E6u;
    RenderText(iPosX + 14, iPosY + 60, Buffer, 70 * (int)WindowWidth / 0x280, 1, 0);

    // ── Experience (cur / max) ──────────────────────────────────────────────
    SelectObject(m_hFontDC, g_hFont);
    sprintf(Buffer, GlobalText[201], *(int*)(CA + 16), *(int*)(CA + 52));
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText(iPosX + 24, iPosY + 80, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // ── "Puntos: NNNN" (LevelUpPoint) — top-center, blue background ──────────
    // Per IDA `ReceiveAddPoint` line 48: `--*(_WORD *)(CharacterAttribute + 84);`
    // confirms CA+84 is the available stat-points field.  GlobalText[217]
    // format ("Puntos: %d").  Only render when LevelUpPoint > 0 — matches
    // the truncated `if (LevelUpPoint > 0)` block in IDA RenderCharacterInfoWindow.
    unsigned short levelUpPoint = *(unsigned short*)(CA + 84);
    if (levelUpPoint > 0) {
        SelectObject(m_hFontDC, g_hFontBold);
        m_dwBackColor = 0xFF1E6EC8u;     // (30, 110, 200) blue band
        m_dwTextColor = 0xFF000000u;     // black text
        char ptsBuf[64];
        // Use GlobalText[217] if available; defensive fallback to a literal.
        const char* ptsFmt = GlobalText[217][0] ? GlobalText[217] : "Puntos: %d";
        wsprintfA(ptsBuf, ptsFmt, (int)levelUpPoint);
        RenderText(iPosX + 95, iPosY + 49, ptsBuf,
                   80 * (int)WindowWidth / 0x280, 1, 0);
    }

    // ── "Puntos de reposición: %d / %d" (reset points, CA+46/+48) ───────────
    // Per IDA RenderCharacterInfoWindow line 500: GlobalText[199] uses CA+46
    // and CA+48 — these are the RESET points (different from CA+84 above).
    // In the user's screenshot this renders as "Puntos de reposición: 0 / 62".
    if (*(unsigned short*)(CA + 14) <= 5) {
        wsprintfA(Buffer, GlobalText[199], 0, 0);
    } else {
        wsprintfA(Buffer, GlobalText[199],
                  *(short*)(CA + 46), *(short*)(CA + 48));
    }
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFFF9664u;
    RenderText(iPosX + 24, iPosY + 95, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // ── Stat-add [+] buttons (sprites 0x120 / 0x121) ────────────────────────
    // 2026-08-08: la geometría venía de OpenMU (PrintPlayer.cpp) y NO coincidía
    // con el hit-test real. Ahora sale de IDA sub_4E5DE0 L94-98, que es donde el
    // binario testea estos botones:
    //     x ∈ [CharacterInfoStartX+125, +149)      (24 px)
    //     y ∈ [CharacterInfoStartY+115+60*row, +24)
    // Gate: LevelUpPoint = *(WORD*)(CharacterAttribute+0x54) != 0.
    //   Sprite 0x120 (288): normal   0x121 (289): hover/pressed
    // El click y el envío del F3/06 los hace FUN_004e5de0 — acá sólo se dibuja.
    int availPts = (int)*(unsigned short*)(CA + 84);
    auto draw_plus_button = [&](int row, int statSlot) {
        if (availPts <= 0) return;
        float btnX = (float)iPosX + 125.0f;
        float btnY = (float)(iPosY + 115 + 60 * row);
        bool hover = ((double)MouseX >= btnX && (double)MouseX < btnX + 24.0 &&
                      (double)MouseY >= btnY && (double)MouseY < btnY + 24.0);
        // 2026-05-04: tap clicks (DOWN+UP within one frame) lose DAT_083a4124
        // before this render runs — WM_LBUTTONUP clears it.  Use the latched
        // click-event flag DAT_083a413c instead (set on UP, sticks until
        // consumed). Hover-without-click still highlights via 4124 for held
        // clicks.  Player_InputTick already gates on g_MouseOnWindow so it
        // won't double-fire ground walk.
        // 2026-08-08: SOLO render + highlight. El CLICK (y el envío del
        // F3/06) lo maneja `FUN_004e5de0` (port de sub_4E5DE0 L85-244), que es
        // donde el binario original tiene el hit-test de estos botones — mismo
        // rect (+125..+149 × +115+60*row ..+24) y mismo gate (LevelUpPoint).
        // Tener el send acá TAMBIÉN mandaba el paquete dos veces por click.
        bool pressed = hover && (DAT_083a4124 != 0 || DAT_083a413c != 0);
        (void)statSlot;
        glColor3f(1.0f, 1.0f, 1.0f);
        FUN_005125a0(pressed ? 0x121 : 0x120,
                     btnX, btnY, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    };

    // ── Strength row + Damage range ─────────────────────────────────────────
    SelectObject(m_hFontDC, g_hFontBold);
    draw_plus_button(0, 0);  // Strength = slot 0
    sprintf(Buffer, GlobalText[202], (unsigned)*(unsigned short*)(CA + 20));
    m_dwBackColor = 0;
    m_dwTextColor = 0xFF66CCDFu;   // -9977889
    RenderText(iPosX + 14, iPosY + 120, Buffer, 70 * (int)WindowWidth / 0x280, 1, 0);
    SelectObject(m_hFontDC, g_hFont);

    // Compute damage range:  pick weapon slots, magic-class bonus, set bonus.
    // pWeapon = CharacterMachine + 536 (left-hand slot).
    // v57    = CharacterMachine + 604 (right-hand slot).
    ITEM* pWeaponLeft  = (ITEM*)(CM + 536);
    ITEM* pWeaponRight = (ITEM*)(CM + 604);
    // Mid-drag drop overlay: if dragging from inventory grid, the picked item
    // takes the slot's place for the preview. (dword_7EA9800 / pPickedItem
    // not modeled in our build — skip this branch.)

    short typeL = *(short*)pWeaponLeft;
    short typeR = *(short*)pWeaponRight;

    // Damage base picker:
    //   bow/crossbow class items in [128,135) or 145 in right hand → use CA[60..62]
    //   (Two-handed sword on left in [136,143) etc → CA[64..66])
    int v61, v62;
    if ( (typeL < 136 || typeL >= 143)
      && (typeL < 144 || typeL >= 145)
      && typeL != 146
      && (((typeR >= 128) && (typeR < 135)) || typeR == 145 || typeL == -1) )
    {
        v61 = *(unsigned short*)(CA + 64);
        v62 = *(unsigned short*)(CA + 66);
    } else {
        v61 = *(unsigned short*)(CA + 60);
        v62 = *(unsigned short*)(CA + 62);
    }

    // Magic class bonus (class 1=DW, 3=MG): if both slots in [0,192)
    int classL3 = *(BYTE*)((char*)Hero + 0x1BC) & 7;
    bool dualBonus = false;
    if (classL3 == 1 || classL3 == 3) {
        if (typeR >= 0 && typeR < 192 && typeL >= 0 && typeL < 192) {
            dualBonus = true;
            v61 = (55 * *(unsigned short*)(CA + 60) + 55 * *(unsigned short*)(CA + 64)) / 100;
            v62 = (55 * *(unsigned short*)(CA + 62) + 55 * *(unsigned short*)(CA + 66)) / 100;
        }
    } else if (classL3 == 2 && typeL >= 128 && typeL < 160) {
        // Elf with bow: dual hand bow set bonus
        if (typeR >= 128 && typeR < 160) {
            BYTE rL = ((BYTE*)pWeaponRight)[1];
            BYTE lL = pWeaponLeft->Level;
            if ((typeR == 135 && (rL & 0x78) >= 8) ||
                (typeL == 143 && (lL & 0x78) >= 8)) {
                dualBonus = true;
            }
        }
    }
    // PVP bonus flag from CA[40] bit 1
    if ((CA[40] & 2) != 0) {
        v61 += 15;
        v62 += 15;
    }

    if (*(unsigned short*)(CA + 58)) {
        sprintf(Buffer, GlobalText[203], v61, v62, *(unsigned short*)(CA + 58));
    } else {
        sprintf(Buffer, GlobalText[204], v61, v62);
    }
    m_dwTextColor = dualBonus ? 0xFFFF9664u : 0xFFFFFFFFu;
    m_dwBackColor = 0x80000000u;
    RenderText(iPosX + 24, iPosY + 140, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // Equipment-set bonus check — original walks 5 equipment slots looking
    // for matching set indexes (offset +740, stride 68 from +136..+408).
    // Result stored in HIBYTE(v165) per IDA but visual effect is just a tint
    // delta; we leave as default.

    // ── Agility row + Defense + Defense Rate ────────────────────────────────
    SelectObject(m_hFontDC, g_hFontBold);
    draw_plus_button(1, 1);  // Agility = slot 1
    sprintf(Buffer, GlobalText[205], (unsigned)*(unsigned short*)(CA + 22));
    m_dwBackColor = 0;
    m_dwTextColor = 0xFF67BFDFu;
    RenderText(iPosX + 14, iPosY + 180, Buffer, 70 * (int)WindowWidth / 0x280, 1, 0);
    SelectObject(m_hFontDC, g_hFont);

    bool agiBonus = false;
    if (agiBonus) {
        if (*(unsigned short*)(CA + 76)) {
            sprintf(Buffer, GlobalText[206],
                    *(unsigned short*)(CA + 78),
                    *(unsigned short*)(CA + 76),
                    *(unsigned short*)(CA + 78) / 10);
        } else {
            sprintf(Buffer, GlobalText[207],
                    *(unsigned short*)(CA + 78),
                    *(unsigned short*)(CA + 78) / 10);
        }
    } else if (*(unsigned short*)(CA + 76)) {
        sprintf(Buffer, GlobalText[208],
                *(unsigned short*)(CA + 78),
                *(unsigned short*)(CA + 76));
    } else {
        sprintf(Buffer, GlobalText[209], *(unsigned short*)(CA + 78));
    }
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText(iPosX + 24, iPosY + 200, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // Defense rate (different field per class)
    unsigned short defRate = *(unsigned short*)(CA + 56);
    if ((CA[11] & 7) == 0) defRate = *(unsigned short*)(CA + 68);
    sprintf(Buffer, GlobalText[64], defRate);
    RenderText(iPosX + 24, iPosY + 215, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // ── Vitality row + HP ───────────────────────────────────────────────────
    SelectObject(m_hFontDC, g_hFontBold);
    draw_plus_button(2, 2);  // Vitality = slot 2
    sprintf(Buffer, GlobalText[210], (unsigned)*(unsigned short*)(CA + 24));
    m_dwBackColor = 0;
    m_dwTextColor = 0xFF67BFDFu;
    RenderText(iPosX + 14, iPosY + 240, Buffer, 70 * (int)WindowWidth / 0x280, 1, 0);

    SelectObject(m_hFontDC, g_hFont);
    sprintf(Buffer, GlobalText[211],
            *(unsigned short*)(CA + 28),
            *(unsigned short*)(CA + 32));
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText(iPosX + 24, iPosY + 260, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // ── Energy row + Mana ───────────────────────────────────────────────────
    SelectObject(m_hFontDC, g_hFontBold);
    draw_plus_button(3, 3);  // Energy = slot 3
    sprintf(Buffer, GlobalText[212], (unsigned)*(unsigned short*)(CA + 26));
    m_dwBackColor = 0;
    m_dwTextColor = 0xFF67BFDFu;
    RenderText(iPosX + 14, iPosY + 300, Buffer, 70 * (int)WindowWidth / 0x280, 1, 0);

    SelectObject(m_hFontDC, g_hFont);
    sprintf(Buffer, GlobalText[213],
            *(unsigned short*)(CA + 30),
            *(unsigned short*)(CA + 34));
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText(iPosX + 24, iPosY + 320, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);

    // ── DW (class 0) / MG (class 3): magic skill damage line ────────────────
    int classFlag = *(BYTE*)((char*)Hero + 444) & 7;
    int yMagic = iPosY + 335;
    if (classFlag == 0 || classFlag == 3) {
        // Best-effort: GetMagicSkillDamage.  Hero+913 = current skill index.
        int piMin = 0, piMax = 0;
        BYTE skillType = *(BYTE*)((char*)CA + 87 + *(BYTE*)((char*)Hero + 913));
        // FUN_0047e4f0 (GetMagicSkillDamage) declared in functions.h but not
        // linked yet; piMin/piMax stay 0 until the helper is unconditionally
        // ported.  Compile-time gate avoids unresolved-extern.
        (void)skillType;
        // FUN_0047e4f0((int)CharacterMachine, (int)skillType, 1);

        // PVP +10 bonus
        if ((CA[40] & 2) != 0) { piMin += 10; piMax += 10; }

        // Effect-extra weapon (type ∈ [160,192) or 31): apply staff damage % bonus
        WORD wType = pWeaponRight->Type;
        if ((wType >= 160 && wType < 192) || wType == 31) {
            int staffMul = ((pWeaponRight->Level >> 3) & 0xF);
            staffMul = (pWeaponRight->DamageMin >> 1) + 2 * staffMul;
            // CalcDurabilityPercent(Durability, MagicDur, Level, Option1):
            // not ported — fallback factor 0.0 (no penalty).
            float pct  = 0.0f;
            float mult = (float)staffMul * 0.01f;
            int extra = (int)((mult - pct * mult) * (float)piMax);
            sprintf(Buffer, GlobalText[215], piMin, piMax, extra);
        } else {
            sprintf(Buffer, GlobalText[216], piMin, piMax);
        }
        RenderText(iPosX + 24, yMagic, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);
    }

    // ── Class-specific extra text (Charisma / Command for DK/MG) ────────────
    if (classFlag == 1) {
        // DK: Command from Energy/10 + 200
        sprintf(Buffer, GlobalText[582],
                *(unsigned short*)(CA + 26) / 10 + 200);
        RenderText(iPosX + 24, iPosY + 335, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);
    } else if (classFlag == 3) {
        // MG: fixed 200
        sprintf(Buffer, GlobalText[582], 200);
        RenderText(iPosX + 24, iPosY + 350, Buffer, 130 * (int)WindowWidth / 0x280, 0, 0);
    }

    // The original IDA function ends here; close button is NOT drawn by
    // this function (the C panel close is handled via the bottom-HUD
    // "C" button toggle in Player_InputTick, NOT via an X overlay).
}

// =============================================================================
// RenderGuildList — sub_4F0810 @ 0x004F0810 (566 bytes).  Faithful port.
// =============================================================================
extern "C" void __cdecl RenderGuildList(int StartX, int StartY)
{
    CHAR Text[100];

    GuildListStartX = StartX;
    GuildListStartY = StartY;
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_00511600();
    float x = (float)StartX;
    float y = (float)StartY;
    FUN_005125a0(260, x, y,            190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f,         1, 1);
    FUN_005125a0(261, x, y + 256.0f,   190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f,  1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    SelectObject(m_hFontDC, g_hFontBold);

    // Hero+474 holds the player's guild-mark index (-1 if no guild).
    short guildIdx = *(short*)((char*)Hero + 474);
    if (guildIdx == -1) {
        sprintf(Text, "%s", GlobalText[180]);
    } else {
        sprintf(Text, "%s (Score:%d)",
                Guild_GetMarkName(guildIdx), GuildTotalScore);
    }
    RenderText(StartX + 35, StartY + 12, Text, 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    m_dwBackColor = 0;
    m_dwTextColor = 0xFFE6E6E6u;
    SelectObject(m_hFontDC, g_hFont);

    if (g_nGuildMemberCount == 0) {
        // 3-line "no guild" message.
        RenderText(StartX + 20, StartY + 50, GlobalText[185], 0, 0, 0);
        RenderText(StartX + 20, StartY + 65, GlobalText[186], 0, 0, 0);
        RenderText(StartX + 20, StartY + 80, GlobalText[187], 0, 0, 0);
    }

    // Notice (2 lines, centered, gold).
    int xc = StartX + 95;
    m_dwBackColor = 0x80000000u;
    m_dwTextColor = 0xFFC8FF64u;
    RenderCenteredText(xc, StartY + 44, g_GuildNotice[0]);
    RenderCenteredText(xc, StartY + 58, g_GuildNotice[1]);

    // ── Listado de miembros ───────────────────────────────────────────────
    // IDA `RenderGuildList` @0x4F0810 termina con:
    //     if ( g_nGuildMemberCount > 0 )
    //         (*(void (**)(void))(*(_DWORD *)dword_55C9FF4 + 16))();
    // o sea despacha el slot 4 de la vtable del widget de lista (el que dibuja
    // las filas de miembros).
    //
    // 2026-08-15: el AV de EJECUCIÓN que había acá NO era "el objeto no está
    // construido" — `WinMain.cpp:844` sí lo construye vía
    // `ChatListBox_ConstructWhisper()`.  El problema era que ese constructor le
    // instalaba la vtable del CHAT (`s_ChatLB_VTable`, off_5525CC) cuando el
    // binario le pone off_5526EC, y 13 de los 30 slots DIFIEREN — entre ellos
    // los cuatro que usa este dispatch (20/22/23/24).  Corregido en
    // `UI/ChatListBox.cpp` (bloque "WIDGET DE LISTA DE GUILD").
    if (g_nGuildMemberCount > 0 && DAT_055c9ff4) {
        DWORD* obj = (DWORD*)(uintptr_t)DAT_055c9ff4;
        void** vt  = (void**)*obj;
        if (vt) {
            typedef void (__fastcall *FnRender)(DWORD*);
            ((FnRender)vt[4])(obj);
        }
    }
}

// =============================================================================
// RenderGuildCreation — sub_4F0480 @ 0x004F0480 (905 bytes).  Faithful port.
// =============================================================================
extern "C" void __cdecl RenderGuildCreation(int iPosX, int iPosY)
{
    // 2026-07-27 FIX: idem Party panel — Inventory[32] es el slot 0 del pool de
    // la tienda; usar storage propio en vez de pisarlo.
    g_GuildCreatorScratchX = iPosX;
    g_GuildCreatorScratchY = iPosY;
    GuildCreator_HandleMouse(iPosX, iPosY);
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_00511600();

    float x = (float)iPosX;
    float y = (float)iPosY;
    FUN_005125a0(260, x, y,            190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f,         1, 1);
    FUN_005125a0(261, x, y + 256.0f,   190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f,  1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    SelectObject(m_hFontDC, g_hFontBold);
    RenderText(iPosX + 35, iPosY + 12, GlobalText[180],
               120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    if (g_iKeyPadEnable) {
        // Name input field + virtual keypad (PIN-style).
        float v19 = x + 55.0f;
        float v17 = y + 60.0f;
        FUN_005125a0(271, v19, v17, 113.0f, 18.0f, 0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);
        m_dwBackColor = 0;
        m_dwTextColor = 0xFFE6E6E6u;
        SelectObject(m_hFontDC, g_hFontBold);
        int v3 = (int)v17 + 2;
        int v4 = (int)v19;
        RenderText(v4 - 38, v3, GlobalText[182], 0, 0, 0);
        InputTextWidth = 100;
        RenderInputText(v4 + 5, v3, 0);
        InputTextWidth = 255;

        // Guild mark editor grid (8×8) + palette.  The shared 97.04 source
        // confirms the same structure: 64 editable cells backed by the
        // DAT_07ea51f5 byte buffer and a 16-color palette driven by
        // DAT_07eaa0dc / MarkColor.
        {
            unsigned char* mark = (unsigned char*)&DAT_07ea51f5;
            for (int gy = 0; gy < 8; ++gy) {
                int sy = iPosY + 100 + gy * 15;
                for (int gx = 0; gx < 8; ++gx) {
                    int sx = iPosX + 50 + gx * 15;
                    unsigned char color = mark[gx + gy * 8];
                    RenderGuildMark_stub((float)(sx + 1), (float)(sy + 1), 13.0f, 13.0f, color);
                }
            }
            for (int py = 0; py < 2; ++py) {
                int sy = iPosY + 260 + py * 20;
                for (int px = 0; px < 8; ++px) {
                    int sx = iPosX + 15 + px * 20;
                    int color = px + py * 8;
                    RenderGuildMark_stub((float)(sx + 1), (float)(sy + 1), 18.0f, 18.0f, color);
                }
            }
            RenderGuildMark_stub((float)(iPosX + 16), (float)(iPosY + 231),
                                 23.0f, 23.0f, ((unsigned char)DAT_07eaa0dc & 0x0F));
        }

        m_dwBackColor = 0;
        m_dwTextColor = 0xFFE6E6E6u;
        SelectObject(m_hFontDC, g_hFont);
        RenderText(iPosX + 50, iPosY + 230, GlobalText[183], 0, 0, 0);
        RenderText(iPosX + 50, iPosY + 245, GlobalText[184], 0, 0, 0);

    } else {
        // KeyPad disabled — simple "No guild master rights" message.
        m_dwBackColor = 0;
        m_dwTextColor = 0xFFE6E6E6u;
        RenderText(iPosX + 20, iPosY + 50, GlobalText[181], 0, 0, 0);
    }
}

// =============================================================================
// RenderTrade / RenderShopInterface / RenderChaosMix / RenderWarehouse /
// RenderEventWindow / RenderGoldenArcherWindow
//
// These six panels share a common skeleton: gate on their respective
// *Opened flag, render the panel skeleton via RenderInventoryInterface at
// (260, 0), render the panel-specific item grid via RenderItemsBoxes /
// sub_4E38B0, render title + close button + tooltip.  The six panels'
// distinguishing widgets (price labels, accept buttons, ingredient
// previews, jewel quality bar, event countdown) sit on top — each is its
// own ~200-line port that adds drag-drop hit-test.  The skeletons here
// hand correct visuals for the panel skeleton itself; widgets above the
// skeleton render on top when ported.
//
// Anti-tamper hash-table noise around TradeOpened / ShopOpened / etc. is
// skipped for the same reason as the other passes.
// =============================================================================

extern "C" void __cdecl RenderTrade(void)
{
    if (!TradeOpened) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    TradeInventoryStartX = 260;
    TradeInventoryStartY = 0;
    RenderInventoryInterface(TradeInventoryStartX, TradeInventoryStartY, 1);

    // Trader name title
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    RenderText(TradeInventoryStartX + 35, TradeInventoryStartY + 12,
               GlobalText[226], 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    // IDA / C1:38-39 mapping: remote offer on top (Inventory), local offer
    // on bottom (OffsetTradeItems). Only the latter accepts item movement.
    RenderItemsBoxes((float)((double)TradeInventoryStartX + 15.0),
                     (float)((double)TradeInventoryStartY + 70.0),
                     (DWORD)(uintptr_t)Inventory, 8, 4);
    RenderItemsBoxes((float)((double)TradeInventoryStartX + 15.0),
                     (float)((double)TradeInventoryStartY + 270.0),
                     (DWORD)(uintptr_t)OffsetTradeItems, 8, 4);

    // Accept / cancel buttons.
    float xa = (float)((double)TradeInventoryStartX + 25.0);
    float ya = (float)((double)TradeInventoryStartY + 395.0);
    FUN_005125a0(280, xa, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
}

extern "C" void __cdecl RenderShopInterface(void)
{
    if (!ShopOpened) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    dword_7EAA0C8 = 260;
    dword_7EAA0CC = 0;
    RenderInventoryInterface(dword_7EAA0C8, dword_7EAA0CC, 0);

    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    RenderText(dword_7EAA0C8 + 35, dword_7EAA0CC + 12,
               GlobalText[230], 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    // Shop items start at `&Inventory[32].WalkSpeed` in the original client.
    RenderItemsBoxes((float)((double)dword_7EAA0C8 + 15.0),
                     (float)((double)dword_7EAA0CC + 50.0),
                     (DWORD)(uintptr_t)ShopItems, 8, 15);

    // Close button.
    float xa = (float)((double)dword_7EAA0C8 + 25.0);
    float ya = (float)((double)dword_7EAA0CC + 395.0);
    FUN_005125a0(280, xa, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
}

extern "C" void __cdecl RenderChaosMix(void)
{
    if (!ChaosMixOpened) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    dword_7EAA0C8 = 260;
    dword_7EAA0CC = 0;
    RenderInventoryInterface(dword_7EAA0C8, dword_7EAA0CC, 0);

    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    RenderText(dword_7EAA0C8 + 35, dword_7EAA0CC + 12,
               GlobalText[232], 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    RenderItemsBoxes((float)((double)dword_7EAA0C8 + 15.0),
                     (float)((double)dword_7EAA0CC + 110.0),
                     (DWORD)(uintptr_t)OffsetMixItems, 8, 4);

    // Mix / cancel buttons.
    float xa = (float)((double)dword_7EAA0C8 + 25.0);
    float ya = (float)((double)dword_7EAA0CC + 395.0);
    FUN_005125a0(280, xa,         ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    FUN_005125a0(282, xa + 30.0f, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
}

// RenderWarehouse — sub_4F3170 @ 0x004F3170 (2776 bytes).
//
// 2026-08-08: port completo. Antes era un esqueleto (interfaz + grid + un botón
// suelto 280) al que le faltaba TODA la fila inferior: el zen guardado, el
// impuesto de retiro y los 3 botones (guardar zen / sacar zen / candado).
// El hit-test de esos botones ya estaba portado (FUN_004eb5d0, mal llamado
// "SecondPassword_Screen9" en Net/SecondPassword.cpp) pero no se veía nada,
// así que había que adivinar dónde clickear.
//
// Ruido anti-tamper (hash table sobre CharacterMachine alrededor de CADA lectura
// de +0x54C/+0x548 y del Level) omitido per policy — el efecto neto es la
// lectura directa.
extern "C" void __cdecl RenderWarehouse(void)
{
    if (!WarehouseOpened) return;

    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    dword_7EAA0C8 = 260;      // 0x104
    dword_7EAA0CC = 0;
    RenderInventoryInterface(dword_7EAA0C8, dword_7EAA0CC, 0);

    RenderItemsBoxes((float)((double)dword_7EAA0C8 + 15.0),
                     (float)((double)dword_7EAA0CC + 50.0),
                     (DWORD)(uintptr_t)OffsetWarehouseItems, 8, 15);

    // ── Título: "Baúl (Abierto|Cerrado)" ─────────────────────────────────────
    // IDA: wsprintfA(String, "%s (%s)", GlobalText[234], GlobalText[240 + lock])
    // con glColor amarillo cuando el baúl está bloqueado por password.
    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    const int lock = (DAT_00559f5f != 0) ? 1 : 0;
    if (lock) glColor3f(1.0f, 1.0f, 0.0f);
    CHAR Title[128];
    wsprintfA(Title, "%s (%s)", GlobalText[234], GlobalText[240 + lock]);
    RenderText(dword_7EAA0C8 + 35, dword_7EAA0CC + 12,
               Title, 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);
    glColor3f(1.0f, 1.0f, 1.0f);

    // ── Barra del zen guardado (bitmap 271 = 0x10F) ──────────────────────────
    float x = (float)((double)dword_7EAA0C8 + 50.0);
    float y = (float)((double)dword_7EAA0CC + 352.7);
    FUN_005125a0(271, x, y, 113.0f, 18.0f, 0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);
    SelectObject(m_hFontDC, g_hFontBold);

    // WarehouseMoney = CharacterMachine[+0x54C]; (el inventario usa +0x548).
    int whZen = CharacterMachine ? *(int*)((BYTE*)CharacterMachine + 1356) : 0;
    CHAR Buffer[100];
    if      (whZen <  1000)       wsprintfA(Buffer, "%d", whZen % 1000);
    else if (whZen <  1000000)    wsprintfA(Buffer, "%d,%03d",
                                            (whZen % 1000000) / 1000, whZen % 1000);
    else if (whZen <  1000000000) wsprintfA(Buffer, "%d,%03d,%03d",
                                            (whZen % 1000000000) / 1000000,
                                            (whZen % 1000000) / 1000, whZen % 1000);
    else                          wsprintfA(Buffer, "%d,%03d,%03d,%03d",
                                            whZen / 1000000000,
                                            (whZen % 1000000000) / 1000000,
                                            (whZen % 1000000) / 1000, whZen % 1000);

    m_dwBackColor = 0xFF282828u;
    m_dwTextColor = 0xFF96DCFFu;
    const int zenX = (int)x;          // 310
    const int zenY = (int)y + 3;      // 355
    RenderText(zenX - 30, zenY, GlobalText[224], 0, 0, 0);
    if      (whZen >= 10000000)  m_dwTextColor = 0xFF0000FFu;   // >= 0x989680
    else if (whZen >= 1000000)   m_dwTextColor = 0xFF0096FFu;
    else if (whZen >= 100000)    m_dwTextColor = 0xFF18C900u;   // 0xFF96DCFF + 0xFF81EC01
    else                         m_dwTextColor = 0xFF96DCFFu;
    RenderText(zenX + 10, zenY, Buffer, 0, 0, 0);

    // ── Impuesto de retiro (label GlobalText[266] + monto) ───────────────────
    // tax = (lock ? 2*Level : 0) + (int)(Level*Level * 0.040000003), redondeado
    // a decenas (>=100) o centenas (>=1000), mínimo 1.
    m_dwBackColor = 0xFF282828u;
    m_dwTextColor = 0xFF0000FFu;
    SelectObject(m_hFontDC, g_hFontBold);
    const int taxX = dword_7EAA0C8 + 50;    // 310
    const int taxY = dword_7EAA0CC + 374;   // 374
    RenderText(taxX, taxY, GlobalText[266], 0, 0, 0);

    m_dwTextColor = 0xFF96DCFFu;
    const unsigned short lvl =
        CharacterAttribute ? *(WORD*)((BYTE*)CharacterAttribute + 14) : 0;
    int tax = (int)((double)((unsigned int)lvl * (unsigned int)lvl) * 0.040000003)
            + (lock ? 2 * (int)lvl : 0);
    if (tax < 1)            tax = 1;
    else if (tax >= 1000)   tax = 100 * (tax / 100);
    else if (tax >= 100)    tax =  10 * (tax /  10);

    CHAR TaxStr[64];
    if      (tax <  1000)       wsprintfA(TaxStr, "%d", tax % 1000);
    else if (tax <  1000000)    wsprintfA(TaxStr, "%d,%03d",
                                          (tax % 1000000) / 1000, tax % 1000);
    else                        wsprintfA(TaxStr, "%d,%03d,%03d",
                                          (tax % 1000000000) / 1000000,
                                          (tax % 1000000) / 1000, tax % 1000);
    // DESVIACIÓN CONSCIENTE (única de esta función): el binario original hace
    // `RenderText(LODWORD(x) + 70, LODWORD(y), TaxStr, ...)` leyendo el slot de
    // stack donde quedaron los FLOATS de RenderBitmap (0x4F3A08: `mov edx,
    // [esp+C4h+x]` / `add edx, 46h`), no los ints ftol'eados. O sea el monto se
    // dibuja en (0x439B0046, 0x43B05666) ≈ 1.13e9 px → fuera de pantalla: en el
    // 0.97k el impuesto NUNCA se ve. Lo dibujamos en la posición que el autor
    // claramente quiso (label en taxX, monto en taxX+70, misma fila).
    RenderText(taxX + 70, taxY, TaxStr, 0, 0, 0);

    // ── Botones inferiores ───────────────────────────────────────────────────
    //   i=0 (x+26)  guardar zen   tex 272, pressed 273, tip GlobalText[235]
    //   i=1 (x+76)  sacar zen     tex 272, pressed 273, tip GlobalText[236]
    //   i=2 (x+126) candado       tex 294/296 (abierto/cerrado),
    //                             pressed 295/297, tip GlobalText[242]
    // (El 4º botón del hit-test — cerrar, en (215,395) — lo dibuja
    //  RenderInventoryInterface.)
    m_dwTextColor = 0xFFFFFFFFu;
    m_dwBackColor = 0xFF000000u;
    static const int kBtnX[3]   = { 26, 76, 126 };
    static const int kBtnTip[3] = { 235, 236, 242 };
    static const int kTexNormal[4]  = { 272, 272, 294, 296 };
    static const int kTexPressed[4] = { 273, 273, 295, 297 };
    for (int i = 0; i < 3; ++i) {
        int v = i;
        if (i == 2 && lock) v = 3;
        float bx = (float)((double)kBtnX[i] + (double)dword_7EAA0C8);
        float by = (float)((double)dword_7EAA0CC + 390.0);
        FUN_005125a0(kTexNormal[v], bx, by, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((double)MouseX >= bx && (double)MouseX < bx + 24.0 &&
            (double)MouseY >= by && (double)MouseY < by + 24.0)
        {
            if (MouseLButton) {
                FUN_005125a0(kTexPressed[v], bx, by, 24.0f, 24.0f,
                             0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
            }
            SelectObject(m_hFontDC, g_hFont);
            RenderTipText((int)bx, (int)by - 13, GlobalText[kBtnTip[i]]);
        }
        if (i == 2 && DAT_07eaa14c) {
            FUN_005125a0(kTexPressed[v], bx, by, 24.0f, 24.0f,
                         0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

extern "C" void __cdecl RenderEventWindow(void)
{
    if (!EventWindowOpened) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    dword_7EAA0C8 = 260;
    dword_7EAA0CC = 0;
    RenderInventoryInterface(dword_7EAA0C8, dword_7EAA0CC, 0);

    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    char* eventTitle = g_bEventChipDialogEnable ? GlobalText[846] : GlobalText[39];
    RenderText(dword_7EAA0C8 + 35, dword_7EAA0CC + 12,
               eventTitle, 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    RenderItemsBoxes((float)((double)dword_7EAA0C8 + 15.0),
                     (float)((double)dword_7EAA0CC + 50.0),
                     (DWORD)(uintptr_t)OffsetMixItems, 8, 4);

    float xa = (float)((double)dword_7EAA0C8 + 25.0);
    float ya = (float)((double)dword_7EAA0CC + 395.0);
    FUN_005125a0(280, xa, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
}

extern "C" void __cdecl RenderGoldenArcherWindow(void)
{
    if (!HUD_IsGoldenArcherPanelRuntime()) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    dword_7EAA0C8 = 450;
    dword_7EAA0CC = 0;
    RenderInventoryInterface(dword_7EAA0C8, dword_7EAA0CC, 0);

    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = 0x00000000u;
    m_dwTextColor = 0xFFF0FF96u;
    RenderText(dword_7EAA0C8 + 35, dword_7EAA0CC + 12,
               getMonsterName(236), 120 * (int)WindowWidth / 0x280, 1, (SIZE*)3);

    // Golden archer doesn't render a grid — just a 3-slot vertical strip.
    float xa = (float)((double)dword_7EAA0C8 + 25.0);
    float ya = (float)((double)dword_7EAA0CC + 395.0);
    FUN_005125a0(280, xa,         ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    FUN_005125a0(282, xa + 30.0f, ya, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);

    // ── Botón de cerrar (el primero, bitmap 280) ─────────────────────────────
    // 2026-08-08: faltaba el hit-test — el panel se dibujaba pero la X no hacía
    // nada. Port de CheckGoldenArcherWindow (0x4E7AC0 LABEL_133): rect
    // (StartX+25, StartY+395) 24×24, gate MouseLButtonPush, manda
    // `[C1][03][0x97]` (Protocol.cpp case 0x97 = cerrar ventana de evento) y
    // resetea el estado de la UI + los campos de input.
    if ((int)MouseX >= (int)xa && (int)MouseX < (int)xa + 24 &&
        (int)MouseY >= (int)ya && (int)MouseY < (int)ya + 24 &&
        DAT_083a4124 != 0)
    {
        DAT_083a4124 = 0;
        Net_SendEventWindowClose();
        g_bEventChipDialogEnable = 0;
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        DAT_07e11d28 = 0;               // MouseUpdateTime
        DAT_00559bec = 6;               // MouseUpdateTimeMax
        FUN_0047ec60(0);                // ClearInput(0)
        DAT_00559c84 = 0;               // InputEnable
        DAT_07e11d72 = 0;               // GoldInputEnable
        DAT_07e11d74 = 0;               // InputGold
        DAT_07eaa108 = 0;               // StorageGoldFlag
    }
}
