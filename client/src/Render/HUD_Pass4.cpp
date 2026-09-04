// HUD_Pass4.cpp — fourth and final batch of small/medium HUD ports.
//
// Functions ported (1:1 from IDA):
//   * Render_MacroTimer    (sub_4BF090)  — macro timer + match-countdown
//   * Render_MapLoadText   (sub_4BF2D0)  — match-mode banner during world 11..16
//   * Render_QuickButtons  (sub_4F5820)  — main HUD UI dispatcher
//   * sub_4E38B0           — inventory grid render (items + glow on chaos mix)
//   * RenderInputText      (sub_47F0B0)  — input field with cursor + IME pass
//   * RenderTipText        (sub_47F7F0)  — tooltip with bordered background
//   * CreateGuildMark      (sub_4F0100)  — composes guild mark texture from
//                                         the per-mark 8x8 nibble palette
//   * sub_47F360           — text glyph composition into Bitmaps[0].Buffer
//   * sub_47F4C0           — upload + render the composed bitmap quad
//
// The chat-listbox slot 22 (sub_40CE20) and slot 23 (sub_40D610) are also
// improved here — moved out of HUD_Pass4 into ChatListBox.cpp directly.
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <gl/GL.h>

extern "C" {
    void __cdecl RenderCharacterInfoWindow(int iPosX, int iPosY);
    void __cdecl RenderGuildList(int iPosX, int iPosY);
    void __cdecl RenderGuildCreation(int iPosX, int iPosY);
}

// ── Aliases: WM_CHAR + RenderInputText share storage via DAT_07db8710 /
// DAT_07d780a8 globals (matching the IDA original where InputText is at
// 0x07db8710 and InputLength at 0x07d780a8). See globals.h.
#define InputText   DAT_07db8710                    // char[10][256]
#define InputLength ((int*)(void*)DAT_07d780a8)     // 10 × DWORD lengths

// ── Local globals (kept here to avoid widening globals.h further) ───────────
extern "C" {
    int   MacroTime               = 0;
    int   m_iMatchCountDownType   = 0;
    DWORD m_dwMatchCountDownStart = 0;
    // m_iMatchTime vive en globals.cpp (0x00559CCC): lo escribe SetMatchInfo
    // desde el handler 0x9B.  Tenerlo aca como local dejaba al renderer
    // leyendo una copia que nadie escribia (2026-09-04).
    int   MixState                = 0;
    int   AlphaBlendType          = 0;

    // Input fields — IDA exposes 10 slots (chat + whisper-target + ...).
    // BUG-FIX 2026-05-04: previously this declared a separate `InputText[2][256]`
    // local storage and `int InputLength[2]`, which DESYNCED from WM_CHAR (which
    // writes to DAT_07db8710 / DAT_07d780a8). Net effect: typing in chat
    // appended to the global, but RenderInputText read this dead local → user
    // saw no characters appearing. Now both use the same storage.
    BYTE  InputTextHide[10]       = {0};
    char  InputTextIME[10][256]   = {{0}};
    // 2026-08-26 — mismo bug que el de arriba, que quedo a medias en 2026-05-04:
    // `InputIndex` e `InputFrame` eran copias LOCALES de dos globals reales, asi
    // que este archivo nunca veia lo que escribia el resto del cliente.
    //
    //   InputIndex = DAT_07e11d78 — indice del campo de input activo. Lo rota el
    //     Tab en WndProc (`DAT_07e11d78 = (DAT_07e11d78 + 1) % DAT_00559c88`) y
    //     lo lee `RenderInputText` (Chat.cpp) para saber en que campo va el
    //     caret. Con la copia local clavada en 0, el `_` se dibujaba SIEMPRE en
    //     el campo de chat aunque se estuviera escribiendo en el de whisper.
    //
    //   InputFrame = DAT_07e11d2c — contador del parpadeo del caret; Chat.cpp
    //     usa el mismo `% 2` sobre el global. Con dos contadores separados los
    //     dos caret parpadeaban desfasados.
    #define InputIndex   DAT_07e11d78
    #define InputFrame   DAT_07e11d2c

    // Guild mark colour palette (16 entries × DWORD ARGB).
    // 2026-08-25: esto era una copia LOCAL del array. El global real es
    // 0x7E11F34 (= DAT_07e11f34), que es el que lee `RenderGuildMark`
    // (0x4F02F0, nuestro RenderGuildMark_stub): `CreateGuildMark` llenaba esta
    // copia y el render leia el global, que quedaba en ceros — y encima estaba
    // declarado como UN DWORD, asi que indexarlo 0..15 desbordaba.
    // Ver [[global-partido-en-dos]].
    #define MarkColor DAT_07e11f34

    // Per-mark 8×8 nibble palette source (16 marks × 80 bytes — matches IDA
    // stride 80 for byte_7E919C5).  Storage opened here; the engine's mark
    // editor populates these slots.
    BYTE  byte_7E919C5_pool[16 * 80] = {0};
}

// Aliases.
#define byte_7E919C5    byte_7E919C5_pool
#define ItemAttribute   ((ITEM_ATTRIBUTE*)DAT_07d78068)
#define byte_7E11D6E    DAT_07e11d6e

// Decoded from C2:5A/C1:5C by the same guild-mark record table used by the
// viewport entity mapping (Character+474).
extern "C" const BYTE* Guild_GetMarkPixels(int row);
extern "C" bool GuildMark_EnsureRenderTarget(void);
extern "C" void GuildMark_InitializePalette(bool blend);

// External symbols already defined elsewhere.
// (UI flags are now #defined in globals.h to DAT_07eaa11x bytes.)
extern "C" int  GetScreenWidth(void);
extern "C" SIZE* __cdecl FUN_0047f6f0(int, int, const char*, int, char, int);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" void   __cdecl RenderBar(float, float, float, float, float, bool, bool);
extern "C" SIZE*  __cdecl RenderCenteredText(int, int, const char*);
extern "C" void   __cdecl RenderTipText(int, int, const char*);
// TradeInventoryStartX/Y now #defined in globals.h to DAT_07ea5290/528c.
extern "C" BYTE   OffsetMixItems[];
extern "C" BYTE   Inventory[];
extern "C" int*   g_pRenderText;
extern "C" DWORD  SetTextColor_0;
extern "C" int    InputTextWidth;
extern "C" int    RepairEnable_0;

#define TradeOpened   DAT_07eaa11b

static bool HUD_IsGuildCreationRuntime(void)
{
    return GuildCreatorOpened != 0;
}

static bool HUD_IsGuildListRuntime(void)
{
    return GuildOpened != 0;
}

static bool HUD_IsCharacterInfoRuntime(void)
{
    return CharacterOpened != 0;
}

// Constants.
static const char aMacroTime[] = "Macro Time";

// All 9 sub-panels are implemented in src/Render/HUD_Pass6.cpp.  sub_5126E0
// is also there.  Forward-declare the ones called from this TU.
extern "C" {
    void __cdecl sub_5126E0(int tex, float, float, float, float, int);
    void __cdecl RenderParty(int x, int y);
    void __cdecl RenderInventoryWindow(void);
    void __cdecl RenderTrade(void);
    void __cdecl RenderShopInterface(void);
    void __cdecl RenderChaosMix(void);
    void __cdecl RenderWarehouse(void);
    void __cdecl RenderEventWindow(void);
    void __cdecl RenderGoldenArcherWindow(void);
    void __cdecl RenderServerDivision(void);
    // GL state helpers — minor stubs, the real pipeline doesn't drive these
    // distinct alpha-blend modes in our build yet.
    void __cdecl EnableLightMap(void) {}
    void __cdecl EnableAlphaBlendMinus(void) {}
    void __cdecl EnableAlphaBlend2(void) {}
}

// =============================================================================
// Render_MacroTimer — sub_4BF090.  Two parts:
//   1. If MacroTime > 0, draw "Macro Time" label + 50-px progress bar at
//      ((screenW-50)/2, 392).
//   2. If m_iMatchCountDownType > 0 and elapsed < 30s, draw the countdown
//      text at (10, 372).  Text picked from GlobalText by type.
// =============================================================================
extern "C" int __cdecl Render_MacroTimer_(void);
int Render_MacroTimer_(void)
{
    if (MacroTime > 0) {
        float x = (640.0f - 50.0f) * 0.5f;
        int v8 = 50 * MacroTime / 100;
        EnableAlphaTest(true);
        UI_DrawText((int)x, 392, (char*)aMacroTime, 0, 1, 0);
        int n = lstrlenA(aMacroTime);
        GetTextExtentPointA(m_hFontDC, aMacroTime, n, &TextSize);
        TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);
        TextSize.cy = (LONG)((double)TextSize.cy / _DAT_055c9b74);
        float Bar = (float)v8;
        RenderBar(x, 404.0f, 50.0f, 2.0f, Bar, false, true);
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    int v1 = m_iMatchCountDownType;
    if (v1 > 0) {
        DWORD TickCount = GetTickCount();
        DWORD elapsed   = TickCount - m_dwMatchCountDownStart;
        if (elapsed <= 0x7530) {   // 30s
            GL_ResetState();          // DisableAlphaBlend
            EnableAlphaTest(false);
            m_dwBackColor = 0x80000000u;
            DWORD secs = elapsed / 0x3E8;
            m_dwTextColor = 0xFFFF8080u;   // -32640

            // GlobalText[431 + type] (300-byte stride ≈ "131435000 + 300*type").
            int idx = (v1 < 4 || v1 > 7) ? (431 + v1) : (612 + v1);
            const char* fmt = (idx >= 0 && idx < 1000) ? GlobalText[idx] : "%d";
            CHAR String[256];
            wsprintfA(String, fmt, 30 - (int)secs);
            UI_DrawText(10, 372, String, 0, 1, 0);
        } else {
            m_iMatchCountDownType = 0;
        }
    }
    return v1;
}

void Render_MacroTimer(void) { Render_MacroTimer_(); }


// =============================================================================
// Render_MapLoadText — sub_4BF2D0.  Match-mode banner shown during PvP
// arena worlds (11..16) when m_byMatchType is set.  Three lines centred at
// (570, 345-358):
//   * monster-kill counter "X/Y" (when m_iMaxKillMonster != 0xFFFF)
//   * GlobalText[865]  ("Time")
//   * MM:SS:SS counter (turns red when minutes < 5)
// =============================================================================
extern "C" void __cdecl Render_MapLoadText_(void);
void Render_MapLoadText_(void)
{
    if (!m_byMatchType) return;
    if ((int)World < 11 || (int)World > 16) return;

    GL_ResetState();
    EnableAlphaTest(false);
    glColor3f(1.0f, 1.0f, 1.0f);
    float v1 = 345.0f;
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFF7700u;   // -16738561

    if ((m_byMatchType <= 2 || m_byMatchType == 5) && m_iMatchTime > 0) {
        if ((int)m_iMaxKillMonster != 0xFFFF) {
            CHAR String[256];
            int gtIdx = (m_byMatchType == 5) ? 866 : 864;
            wsprintfA(String, GlobalText[gtIdx], m_iKillMonster, m_iMaxKillMonster);
            SelectObject(m_hFontDC, g_hFontBold);
            m_dwTextColor = 0xFFFF7700u;
            RenderCenteredText(570, 345, String);
            v1 = 357.0f;
        }
        SelectObject(m_hFontDC, g_hFontBold);
        CHAR String[256];
        wsprintfA(String, GlobalText[865]);
        RenderCenteredText(570, (int)v1, String);

        float v2 = v1 + 10.0f;
        int v0 = m_iMatchTime / 60;
        wsprintfA(String, " %.2d :", v0);
        if (m_iMatchTime - 60 * v0 >= 0) {
            CHAR v3[12];
            wsprintfA(v3, " %.2d", m_iMatchTime - 60 * v0);
            strcat(String, v3);
        }
        if (v0 < 5) m_dwTextColor = 0xFF1FFFFFu;     // -14671617
        if (v0 < 15) {
            CHAR v3[12];
            // WorldTime is the engine animation tick (used as 60Hz seconds approx).
            wsprintfA(v3, ": %.2d", (int)((__int64)WorldTime % 60));
            strcat(String, v3);
        }
        // SelectObject g_hFontBig — we don't have a separate big font.
        SelectObject(m_hFontDC, g_hFontBold);
        RenderCenteredText(570, (int)v2, String);
    }
}

void Render_MapLoadText(void) { Render_MapLoadText_(); }


// =============================================================================
// Render_QuickButtons — sub_4F5820.  HUD UI dispatcher.  Calls 9 sub-panels
// in sequence (party / inventory / trade / shop / chaos-mix / warehouse /
// event / golden-archer / server-division) plus a quest-state refresh.
// All 9 are stubs at the moment — when they get ported individually their
// visuals appear without touching this dispatcher.
// =============================================================================
extern "C" void __cdecl Render_QuickButtons_(void);
void Render_QuickButtons_(void)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_ResetState();
    m_dwTextColor = 0xFFFF8080u;
    // 2026-08-08 FIX "el panel de Character (C) se ve negro si se abre despues
    // del inventario": esto era `GetScreenWidth()`, que devuelve 260 cuando
    // Inventory+Character estan abiertos a la vez -> el panel de Character se
    // dibujaba ENCIMA del inventario (que tambien va a 260) y la franja 450..640
    // quedaba sin pintar = rectangulo negro. En el binario los 3 paneles del
    // lado derecho son CONSTANTES 0x1C2 (=450), no el ancho del viewport:
    //   sub_4F5820+0x39  push 0 / push 1C2h / call RenderGuildCreation
    //   sub_4F5820+0x253 push 0 / push 1C2h / call RenderGuildList
    //   sub_4F5820+0x379 push 0 / push 1C2h / call RenderCharacterInfoWindow
    // (GetScreenWidth solo recorta el viewport 3D, no posiciona paneles.)
    const int panelStartX = 450;

    // IDA: FUN_004F5820 compone el panel antes de sus botones compartidos. La
    // textura inferior del panel alcanza y=433 y, si se dibuja después, cubre
    // por completo los quads de OK/CANCEL aunque sus hit-tests continúen vivos.
    if (HUD_IsGuildCreationRuntime())  RenderGuildCreation(panelStartX, 0);

    if (HUD_IsGuildCreationRuntime()) {
        // IDA: RenderGuildCreation guarda el origen 450,0 antes de que el
        // dispatcher pinte los botones compartidos. El port los dibujaba antes
        // de inicializar su scratch y por eso quedaban fuera del panel.
        g_GuildCreatorScratchX = panelStartX;
        g_GuildCreatorScratchY = 0;
        // 2026-08-26 FIX "los botones OK/CANCEL salen abajo a la izquierda, fuera
        // del panel": el origen era `DAT_07ea5b1c/20` (= Inventory[32].Level/Part),
        // el scratch que el port ABANDONO el 2026-07-27 al mover el origen del
        // creador a `g_GuildCreatorScratchX/Y` (Inventory[32] es el slot 0 del pool
        // de la tienda y lo estaba pisando). Quedo en 0, asi que los botones se
        // dibujaban en (0+20, 0+350) absoluto — abajo a la izquierda — mientras los
        // El hit-test FUN_004e4760 ya usaba el origen bueno: se dibujaban en un
        // lado y se clickeaban en otro.
        // Los tres offsets coinciden con esos hit-tests: +20/+350 y +100 el segundo.
        // Es el tercer hermano del fix del 2026-08-08 b (GuildList y CharacterInfo
        // ya habian pasado a sus globals reales; este quedo sin actualizar).
        float btnX = (float)g_GuildCreatorScratchX + _DAT_005524fc;
        float btnY = (float)g_GuildCreatorScratchY + _DAT_00552ca4;
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 2.1875f, 0.65625f, 1, 1);
        int tex = ((int)MouseX >= (int)btnX && (int)MouseX < (int)(btnX + 70.0f) &&
                   (int)MouseY >= (int)btnY && (int)MouseY < (int)(btnY + 21.0f)) ? 0xF2 : 0xF1;
        GL_DrawTexture(tex, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);

        // 2026-08-26 FIX (CANCEL sobresalia del panel): el segundo boton
        // arranca en +100 desde el origen del panel, NO en +20+100.
        // IDA `sub_4E4760` L446-447 da los dos rects:
        //     boton 1 (crear):  [origin+20 , origin+90 )   ancho 70
        //     boton 2 (cancel): [origin+100, origin+170)   ancho 70
        // El port partia del primero (+20) y le sumaba `_DAT_005524f0` (que
        // ademas es TERRAIN_SCALE, no un offset de UI: valia 100 de casualidad),
        // dejando el boton en +120..+190. Como el panel mide 190 de ancho, ese
        // rect terminaba exactamente en el borde y se veia sobresalir; el
        // desalineado contra el hit-test crecia con la resolucion.
        // No es mezcla de espacios: los dos estaban en logico. Es el offset.
        btnX = (float)g_GuildCreatorScratchX + 100.0f;
        GL_DrawTexture(0x118, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 2.1875f, 0.65625f, 1, 1);
        tex = ((int)MouseX >= (int)btnX && (int)MouseX < (int)(btnX + 70.0f) &&
               (int)MouseY >= (int)btnY && (int)MouseY < (int)(btnY + 21.0f)) ? 0xF4 : 0xF3;
        GL_DrawTexture(tex, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);
    }

    if (HUD_IsGuildListRuntime()) {
        float iconX = (float)DAT_07e91788 + _DAT_00552464;
        float iconY = (float)DAT_07e91784 + _DAT_0055246c;
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, iconX, iconY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        GL_DrawTexture(RepairEnable_0 ? 287 : 286,
                     iconX, iconY, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((int)MouseX >= (int)iconX && (int)MouseX < (int)(iconX + 24.0f) &&
            (int)MouseY >= (int)iconY && (int)MouseY < (int)(iconY + 24.0f)) {
            SelectObject(m_hFontDC, g_hFont);
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)iconX, (int)iconY - 12, GlobalText[233]);
        }
    }

    if (HUD_IsCharacterInfoRuntime()) {
        float iconX = (float)DAT_07ea982c + 25.0f;
        float iconY = (float)DAT_07ea9830 + 395.0f;
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, iconX, iconY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        GL_DrawTexture(280, iconX, iconY, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((int)MouseX >= (int)iconX && (int)MouseX < (int)(iconX + 24.0f) &&
            (int)MouseY >= (int)iconY && (int)MouseY < (int)(iconY + 24.0f)) {
            SelectObject(m_hFontDC, g_hFont);
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)iconX, (int)iconY - 13, GlobalText[225]);
        }
    }

    // 2026-05-04: en el original (sub_4F5820) hay 3 `if` sites antes/alrededor
    // de RenderParty que llaman a RenderCharacterInfoWindow/RenderGuildList/
    // RenderGuildCreation cuando los flags correspondientes están seteados.
    // Sin ellos, los menús C/G renderizan como rectángulo negro afuera del
    // 3D viewport (que es 450 wide cuando esos flags están on).
    if (HUD_IsCharacterInfoRuntime())  RenderCharacterInfoWindow(panelStartX, 0);
    if (HUD_IsGuildListRuntime())      RenderGuildList(panelStartX, 0);

    RenderParty(450, 0);
    RenderInventoryWindow();
    RenderTrade();
    RenderShopInterface();
    RenderChaosMix();
    RenderWarehouse();
    RenderEventWindow();
    RenderGoldenArcherWindow();

    // 2026-08-21: faltaba el render del panel de quest.  IDA sub_4F5820 L38:
    // `sub_403F30((_BYTE *)g_csQuest);` entre RenderGoldenArcherWindow y
    // RenderServerDivision.  Sin esto, con el flag del panel prendido
    // GetScreenWidth angostaba el viewport a 450 y esa franja quedaba negra.
    FUN_00403f30((void*)(uintptr_t)g_csQuest);

    // ── In-world drop dispatcher (2026-05-08) ────────────────────────────────
    // Mirrors what Net_PacketSession.cpp:284 does for state=4 char-select but
    // for in-world. Gated internally on dword_7E91388 > 0 (= player carrying
    // an item picked up via FUN_004d23b0 inside RenderInventoryWindow). This
    // is the function that builds and SENDS the 0x24 PMSG_ITEM_MOVE_RECV
    // packet via SendRequestEquipmentItem_stub → Net_SendSmallPacket (C3).
    FUN_004df410(0, 0);

    RenderServerDivision();
}

void Render_QuickButtons(void) { Render_QuickButtons_(); }


// =============================================================================
// sub_4E38B0 — inventory grid render.  Walks an N×M grid of 68-byte ITEM
// records and renders each occupied cell either as a 3D mesh (`a6 != 0`)
// via RenderItem3D, or as a 2D quantity glyph (RenderNumber2D) for arrows.
//
// Special case for OffsetMixItems during MixState animation: spawns three
// glow particles via sub_5126E0 (stubbed).
//
// Special case for the Inventory pool: cells with Color == 99 get a quest
// icon overlay (bitmap 9) bobbing with WorldTime, plus GlobalText[370]
// label rendered in white.
// =============================================================================
extern "C" int __cdecl sub_4E38B0(float a1, float a2, float x, int a4,
                                  float sx, int a6);
int __cdecl sub_4E38B0(float a1, float a2, float x_param, int a4,
                       float sx, int a6)
{
    // IDA receives the inventory-base pointer in the x87 float argument slot.
    // It is a bit-pattern carrier, not a numeric float: a C cast turns most
    // heap pointers into 0 (or undefined), while the original uses the bits.
    DWORD x_addr;
    memcpy(&x_addr, &x_param, sizeof(x_addr));
    int v23 = 0;
    if ((int)sx <= 0) goto check_quest_overlay;

    {
        DWORD v6_addr = x_addr;
        int v22 = 0;
        int v27 = (int)sx;
        DWORD v26 = x_addr;

        do {
            if (a4 > 0) {
                int v21 = 0;
                DWORD v24 = v6_addr;
                int v25 = a4;
                float sy = (float)((double)v22 + a2);

                do {
                    short v7 = *(WORD*)v6_addr;
                    float sxa = (float)((double)v21 + a1);
                    if (v7 != (short)0xFFFF) {
                        if (*(int*)(v6_addr + 0x38) > 0) {
                            if ((BYTE)a6) {
                                if (v7 >= 448 && v7 <= 456 &&
                                    *(BYTE*)(v6_addr + 0x1A) > 1) {
                                    glColor3f(1.0f, 0.9f, 0.7f);
                                    float h = sxa + 13.0f;
                                    RenderNumber2D(h, sy,
                                                   *(unsigned char*)(v6_addr + 26),
                                                   9.0f, 10.0f);
                                }
                            } else {
                                ITEM_ATTRIBUTE* v8 = &ItemAttribute[v7];
                                float Height = (float)((double)v8->Height * 20.0);
                                float Width  = (float)((double)v8->Width  * 20.0);
                                FUN_004e1be0(sxa, sy, Width, Height, v7,
                                             *(int*)(v6_addr + 4),
                                             *(unsigned char*)(v6_addr + 27), 0);
                            }
                            ++v23;
                        }

                        if ((BYTE)a6 && (BYTE*)(uintptr_t)x_addr == OffsetMixItems
                            && MixState > 0 && MixState < 51)
                        {
                            EnableAlphaBlend();
                            float green = (float)((double)(rand() % 4 + 4) * 0.1);
                            float red   = (float)((double)(rand() % 6 + 6) * 0.1);
                            glColor3f(red, green, 0.2f);
                            int seed = (int)((double)((__int64)WorldTime % 100) * 20.0);
                            float v20 = (float)(rand() % 10 + 5);
                            float v28 = (float)(rand() % 20) + sxa;
                            float sxb = (float)(rand() % 20) + sy;
                            sub_5126E0(1230, v28, sxb, v20, v20, 0);
                            sub_5126E0(1230, v28, sxb, v20, v20, seed);
                            sub_5126E0(1231, v28, sxb, v20 * 3.0f, v20 * 3.0f, seed);
                            sub_5126E0(1150, v28, sxb, v20 * 6.0f, v20 * 6.0f, 0);
                            GL_ResetState();
                            v6_addr = v24;
                        }
                    }
                    v6_addr += 68;
                    v24 = v6_addr;
                    v21 += 20;
                    --v25;
                } while (v25);
            }
            v6_addr = v26 + 544;
            v22 += 20;
            v26 += 544;
            --v27;
        } while (v27);
    }

check_quest_overlay:
    if ((BYTE*)(uintptr_t)x_addr == Inventory) {
        ITEM* v11 = (ITEM*)Inventory;
        ITEM* end = (ITEM*)(Inventory + 32 * sizeof(ITEM));
        while (v11 < end) {
            if (v11->Type != (WORD)-1 && v11->Key && v11->Color == 99) {
                glColor3f(0.0f, 1.0f, 1.0f);
                float sxe = (float)((double)ItemAttribute[v11->Type].Width * 20.0);
                float xa  = (float)((double)(20 * v11->x) + (double)TradeInventoryStartX + 15.0);
                float v36 = (float)(sin((double)WorldTime * 0.015) +
                                    (double)(20 * v11->y) +
                                    (double)TradeInventoryStartY + 70.0);
                float y_pos = v36 + 5.0f;
                GL_DrawTexture(9, xa, y_pos, 24.0f, 24.0f, 0.0f, 0.40000001f, 1.0f, 1.0f, 1, 1);
                glColor3f(1.0f, 1.0f, 1.0f);
                SelectObject(m_hFontDC, g_hFontBold);
                m_dwTextColor = 0xFFFFFFFFu;
                m_dwBackColor = 0xFF000052u;
                int boxW = (int)((double)WindowWidth * sxe * 0.0015625);
                RenderText((int)xa, (int)v36, GlobalText[370], boxW, 0, 0);
            }
            ++v11;
        }
    }

    if (MixState > 1) return ++MixState;
    return MixState;
}


// =============================================================================
// RenderInputText — sub_47F0B0.  Renders an input field at (x, y) using
// InputText[Index], with optional masking via InputTextHide[Index]
// (1 = full mask "*", 2 = mask after the 7th char).  When this slot is the
// active input (Index == InputIndex), draws a blinking cursor "_" or the
// current IME composition string after the text.
// =============================================================================
extern "C" void __cdecl RenderInputText(int x, int y, int Index)
{
    if (Index < 0 || Index >= 10) return;

    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;

    CHAR Text[260] = {0};
    BYTE hide = InputTextHide[Index];
    if (hide == 1) {
        size_t len = strlen(InputText[Index]);
        for (size_t i = 0; i < len; ++i) Text[i] = '*';
        Text[len] = 0;
    } else if (hide == 2) {
        const char* src = InputText[Index];
        size_t len = strlen(src);
        size_t i = 0;
        for (; i < 7 && i < len; ++i) Text[i] = src[i];
        for (; i < len; ++i) Text[i] = '*';
        Text[i] = 0;
    } else {
        strncpy(Text, InputText[Index], sizeof(Text) - 1);
    }

    int v7 = InputTextWidth;
    UI_DrawText(x, y, Text, InputTextWidth, 1, 0);
    int n = lstrlenA(Text);
    GetTextExtentPointA(m_hFontDC, Text, n, &TextSize);
    if (v7 > 0 && TextSize.cx > v7) TextSize.cx = v7;

    // 2026-08-26: acá había un workaround. En 2026-07-19 se detectó que el
    // caret quedaba corto (~80% del largo) y se lo compensó guardando el ancho
    // SIN dividir, porque en ese momento `UI_DrawText` -> `FUN_0040f610`
    // dibujaba en píxeles crudos: la mitad "lógico -> físico" del pipeline no
    // existía, así que un offset en espacio-640 se dibujaba como si fuera píxel.
    //
    // Esa mitad ya está implementada (FUN_0040f610 convierte con
    // g_fScreenRate_x/y, igual que `sub_410AF0` en el binario), así que el
    // workaround quedó obsoleto y ahora es él quien descoloca el caret: sumaba
    // un ancho en PÍXELES a una `x` en LÓGICO.
    //
    // IDA (RenderInputText 0x47F0B0) posiciona el caret con el TextSize ya
    // dividido, o sea en espacio lógico, que es lo que se restaura acá.
    //   ancho medido -> píxel -> / g_fScreenRate_x -> lógico -> + x (lógico)
    TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);
    TextSize.cy = (LONG)((double)TextSize.cy / _DAT_055c9b74);

    if (Index == InputIndex) {
        int v9 = InputFrame % 2;
        ++InputFrame;
        if (v9 == 0) {
            const char* ime = InputTextIME[Index];
            if (strlen(ime) != 0) {
                if (InputTextHide[Index] == 1) {
                    UI_DrawText(x + TextSize.cx, y, (char*)"**", 0, 1, 0);
                    GetTextExtentPointA(m_hFontDC, "**", 2, &TextSize);
                } else {
                    UI_DrawText(x + TextSize.cx, y, (char*)ime, 0, 1, 0);
                    GetTextExtentPointA(m_hFontDC, ime, lstrlenA(ime), &TextSize);
                }
            } else {
                UI_DrawText(x + TextSize.cx, y, (char*)"_", 0, 1, 0);
                GetTextExtentPointA(m_hFontDC, "_", 1, &TextSize);
            }
            TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);
            TextSize.cy = (LONG)((double)TextSize.cy / _DAT_055c9b74);
        }
    }
}


// =============================================================================
// RenderTipText — sub_47F7F0.  Tooltip with an inset border (4 thin
// RenderColor strips) + black drop shadow + black main panel + foreground
// text via CUIRenderText.  Restores the previous AlphaBlendType at exit.
//
// We approximate the layout exactly but render the foreground text via
// UI_DrawText (the CUIRenderText path doesn't run in our build).
// =============================================================================
extern "C" void __cdecl RenderTipText(int sx, int sy, const char* Text)
{
    SIZE sz = {0,0};
    SelectObject(m_hFontDC, g_hFont ? g_hFont : g_hFontBold);
    int n = lstrlenA(Text);
    GetTextExtentPointA(m_hFontDC, Text, n, &sz);
    int prevBlend = AlphaBlendType;

    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

    float fy   = (float)sy;
    float fx   = (float)sx;
    float boxX = fx - 2.0f;     // ≈ flt_55264C constant
    float boxY = fy - 1.0f;     // ≈ flt_552540 constant
    float W    = (float)((double)sz.cx / g_fScreenRate_x + 4.0);
    float H    = (float)((double)sz.cy / _DAT_055c9b74 + 4.0);

    // 4 thin border strips (top, left, right, bottom).
    GL_DrawRect(boxX, boxY, W, 1.0f);
    GL_DrawRect(boxX, boxY, 1.0f, H);
    GL_DrawRect(boxX + W - 1.0f, boxY, 1.0f, H);
    GL_DrawRect(boxX, boxY + H - 1.0f, W, 1.0f);

    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    // Filled black panel inside the border.
    GL_DrawRect(boxX + 1.0f, boxY + 1.0f, W - 2.0f, H - 2.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFFFFFFu;
    UI_DrawText(sx, sy, (char*)Text, 0, 1, 0);

    switch (prevBlend) {
        case 1: EnableLightMap();        break;
        case 2: EnableAlphaTest(true);   break;
        case 3: EnableAlphaBlend();      break;
        case 4: EnableAlphaBlendMinus(); break;
        case 5: EnableAlphaBlend2();     break;
        default: GL_ResetState();         break;
    }
}


// =============================================================================
// CreateGuildMark — sub_4F0100.  Composes guild-mark texture #34 from the
// 8×8 nibble palette stored in byte_7E919C5_pool[mark*80..mark*80+79].
// Each nibble (0..15) indexes into MarkColor[16] which the function also
// (re-)initialises with the standard 16-colour ARGB palette.  When `blend`
// is true, MarkColor[0] is the alpha-zero entry; otherwise alpha=128 for
// 50% transparency on background-coloured pixels.
// =============================================================================
extern "C" void __cdecl CreateGuildMark(int nMarkIndex, bool blend)
{
    if (!GuildMark_EnsureRenderTarget()) return;

    int Width  = (int)Bitmaps[34].Width;
    int Height = (int)Bitmaps[34].Height;
    BYTE* Buffer = Bitmaps[34].Buffer;
    if (!Buffer || Width <= 0 || Height <= 0) return;

    GuildMark_InitializePalette(blend);

    if (nMarkIndex < 0) return;
    const BYTE* v5 = Guild_GetMarkPixels(nMarkIndex);
    if (v5 == nullptr) return;

    BYTE* outRow = Buffer;
    for (int y = 0; y < Height; ++y) {
        for (int x_idx = 0; x_idx < Width; ++x_idx) {
            BYTE nibble = *v5++;
            *(DWORD*)outRow = MarkColor[nibble & 0xF];
            outRow += 4;
        }
    }

    glBindTexture(GL_TEXTURE_2D, Bitmaps[34].TextureNumber);
    glTexImage2D(GL_TEXTURE_2D, 0, Bitmaps[34].Components,
                 Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 Bitmaps[34].Buffer);
}


// =============================================================================
// sub_47F360 — text glyph composition.  Takes target Bitmaps[0].Buffer
// (a row-major 4-bytes-per-pixel offscreen surface, stride 1024 bytes) and
// composes a coloured glyph block from the GDI greyscale render of `a3`
// (and optional prefix `lpString`) into it.  Used by RenderBoolean's
// CUIRenderText-mode-1 path.
//
// Pixels in the surface that come from the prefix are tinted with
// SetTextColor_0; the rest get m_dwTextColor; black background gets
// m_dwBackColor (or 0 past column a8).
//
// In our build Bitmaps[0] / ppvBits are not initialised by the engine
// (font surface), so this path effectively no-ops; preserved structure for
// fidelity.
// =============================================================================
extern "C" int __cdecl sub_47F360(int a1, int a2, LPCSTR a3, int a4, int a5,
                                  int x, int a7, int a8, LPCSTR lpString)
{
    (void)a4; (void)a5; (void)a7;
    int v9  = a2;
    int v10 = a1;
    SIZE sz = {a1, a2};
    if (!a3 || *a3 == 10) return (int)Bitmaps[0].Height;

    if (lpString) {
        int n = lstrlenA(lpString);
        GetTextExtentPointA(m_hFontDC, lpString, n, &sz);
        TextOutA(m_hFontDC, x, 0, lpString, (int)strlen(lpString));
    } else {
        sz.cx = 0;
    }
    SetTextColor(m_hFontDC, RGB(0xFF, 0xFF, 0xFF));
    TextOutA(m_hFontDC, x + sz.cx, 0, a3, (int)strlen(a3));

    if (!a8) a8 = v10;
    int Height = (int)Bitmaps[0].Height;
    if (v9 > Height) v9 = Height;

    if (v9 > 0 && Bitmaps[0].Buffer && ppvBits_055c9e4c) {
        BYTE* dst = Bitmaps[0].Buffer;
        const BYTE* src = (const BYTE*)ppvBits_055c9e4c;
        int rows = v9;
        do {
            BYTE* dRow = dst;
            const BYTE* sRow = src;
            for (int col = 0; col < v10; ++col) {
                DWORD value;
                if (*sRow) {
                    value = (col >= sz.cx) ? m_dwTextColor : SetTextColor_0;
                } else {
                    value = (col >= a8) ? 0u : m_dwBackColor;
                }
                *(DWORD*)dRow = value;
                sRow += 3;     // RGB triplet input
                dRow += 4;     // BGRA output
            }
            src += 1536;       // 512×3 = 1536 bytes per source row
            dst += 1024;       // 256×4 = 1024 bytes per dest row
            --rows;
        } while (rows);
    }
    return Height;
}


// =============================================================================
// sub_47F4C0 — upload + render the composed bitmap quad at (a1, a2) with
// (Width, Height) screen dimensions, clamping to the screen rect (with
// special HUD bottom-bar handling at byte_7E11D6E).
//
// Uploads Bitmaps[0] via glTexImage2D (the same surface populated by
// sub_47F360), then RenderBitmap(0, ...) with computed UV.  In our build
// Bitmaps[0].Buffer is NULL until the engine creates the font surface;
// glTexImage2D with NULL data silently fails — call sites accept this.
// =============================================================================
extern "C" void __cdecl sub_47F4C0(int a1, int a2, float Width, float Height,
                                   int a5, int a6, float a7, int a8)
{
    (void)a5; (void)a6;
    if (Bitmaps[0].Buffer) {
        glBindTexture(GL_TEXTURE_2D, Bitmaps[0].TextureNumber);
        glTexImage2D(GL_TEXTURE_2D, 0, Bitmaps[0].Components,
                     (int)Bitmaps[0].Width, (int)Bitmaps[0].Height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, Bitmaps[0].Buffer);
    }

    int x_pos = (a1 < 0) ? 0 : a1;
    if ((BYTE)a7) {
        DWORD wScale = 640u * (DWORD)Width / WindowWidth;
        if ((int)(wScale + x_pos) > a8) x_pos = a8 - (int)wScale;
    } else if ((int)Width + x_pos > (int)WindowWidth) {
        x_pos = (int)WindowWidth - (int)Width;
    }

    int y_pos = a2;
    if (DAT_07e11d6e) {
        if (a2 < 0) y_pos = 0;
        if ((BYTE)a7) {
            DWORD hScale = 480u * (DWORD)Height / WindowHeight;
            if ((int)(hScale + y_pos) > 0x1B1) y_pos = 433 - (int)hScale;
        } else {
            int v12 = (int)WindowHeight - (47 * (int)WindowHeight) / 640;
            if ((int)Height + y_pos > v12) y_pos = v12 - (int)Height;
        }
    }

    float bw = (Width  + 0.01f) / Bitmaps[0].Width;
    float bh = (Height + 0.01f) / Bitmaps[0].Height;
    GL_DrawTexture(0, (float)x_pos, (float)y_pos, Width, Height,
                 0.0f, 0.0f, bw, bh, 0, (char)(BYTE)a7);
}
