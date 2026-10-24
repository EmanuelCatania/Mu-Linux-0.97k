#include <cstdint>
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
#include "Render/Gfx.h"
#include "Config/UserSettings.h"
#include "Platform/Text.h"
#include "Platform/ITimer.h"
#include "Item/ContentCatalog.h"
#include "Item/Item_ServerValue.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include "UI/EventTimer.h"

extern "C" {
    void __cdecl RenderCharacterInfoWindow(int iPosX, int iPosY);
    void __cdecl RenderGuildList(int iPosX, int iPosY);
    void __cdecl RenderGuildCreation(int iPosX, int iPosY);
}

// ── Aliases: WM_CHAR + RenderInputText share storage via DAT_07db8710 /
// DAT_07d780a8 globals (matching the IDA original where InputText is at
// 0x07db8710 and InputLength at 0x07d780a8). See globals.h.
// MacroTime es 0x07E11D7C (IDA lo decrementa en Game_MainLoop y lo pone en
// 100 al disparar una macro): tiene que ser el global, no una variable propia.
#define MacroTime   DAT_07e11d7c

#define InputText   DAT_07db8710                    // char[10][256]
#define InputLength ((int*)(void*)DAT_07d780a8)     // 10 × DWORD lengths

// ── Local globals (kept here to avoid widening globals.h further) ───────────
extern "C" {
    // 0x07E11D8C / 0x07E11D90.  Los escribe StartMatchCountDown (0x47EC00),
    // que atiende el opcode 0x92; declarados en globals.h para que el handler
    // los vea.
    int   m_iMatchCountDownType   = 0;
    DWORD m_dwMatchCountDownStart = 0;
    // m_iMatchTime vive en globals.cpp (0x00559CCC): lo escribe SetMatchInfo
    // desde el handler 0x9B.
    // MixState vive en globals.h como alias de DAT_07eaa140 (0x07EAA140): lo
    // escriben Item_ClickHandler al mandar el mix (= 1) y el handler del 0x86
    // (0 o 2), y lo leen la animacion de la caja de Chaos de abajo y el
    // `++MixState` que la hace avanzar.
    int   AlphaBlendType          = 0;

    // Input fields — IDA exposes 10 slots (chat + whisper-target + ...).
    // InputText / InputLength son los globals DAT_07db8710 / DAT_07d780a8 que
    // escribe WM_CHAR (ver los aliases de arriba), no storage local.
    BYTE  InputTextHide[10]       = {0};
    char  InputTextIME[10][256]   = {{0}};
    // `InputIndex` e `InputFrame` son los globals reales, no copias locales:
    //
    //   InputIndex = DAT_07e11d78 — indice del campo de input activo. Lo rota el
    //     Tab en WndProc (`DAT_07e11d78 = (DAT_07e11d78 + 1) % InputNumber`) y
    //     lo leen `RenderInputText` y Chat.cpp para saber en que campo va el
    //     caret.
    //
    //   InputFrame = DAT_07e11d2c — contador del parpadeo del caret; Chat.cpp
    //     usa el mismo `% 2` sobre el global.
    #define InputIndex   DAT_07e11d78
    #define InputFrame   DAT_07e11d2c

    // Guild mark colour palette (16 entries × DWORD ARGB).
    // Es el global real 0x7E11F34 (= DAT_07e11f34), el que lee `RenderGuildMark`
    // (0x4F02F0) y llena `CreateGuildMark`.
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
extern "C" SIZE* __cdecl Text_MeasureBox(int, int, const char*, int, char, int);
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

// Implementaciones reales de los tres modos de blend (ver el bloque de abajo).
void GL_EnableLightMap(void);
void GL_SetBlendSrcAlpha(void);
void __stdcall GL_SetBlendInvSrcColor(void);

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
    // Modos de blend que usa la cola de RenderTipText (sub_47F7F0) para dejar
    // el estado GL como estaba.  Los tres eran cuerpos VACIOS con el comentario
    // "the real pipeline doesn't drive these modes yet" -- falso: las tres
    // implementaciones existen y el port duplicado de UI_Tooltip.cpp ya las
    // usaba.  Con los cuerpos vacios, un tooltip dibujado sobre blend 1, 4 o 5
    // dejaba el estado GL sin restaurar.
    //   EnableLightMap        0x00511890 -> GL_EnableLightMap    (GL_State.cpp)
    //   EnableAlphaBlendMinus 0x00511790 -> GL_SetBlendSrcAlpha  (GL_State.cpp)
    //   EnableAlphaBlend2     0x00511810 -> GL_SetBlendInvSrcColor (Render_SpriteHelpers.cpp)
    void __cdecl EnableLightMap(void)        { GL_EnableLightMap(); }
    void __cdecl EnableAlphaBlendMinus(void) { GL_SetBlendSrcAlpha(); }
    void __cdecl EnableAlphaBlend2(void)     { GL_SetBlendInvSrcColor(); }
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
        float x = (float)(((double)GetScreenWidth() - 50.0) * 0.5);
        int v8 = 50 * MacroTime / 100;
        EnableAlphaTest(true);
        UI_DrawText((int)x, 392, (char*)aMacroTime, 0, 1, 0, sizeof(aMacroTime));
        int n = Platform::TextLength(aMacroTime);
        gFont.MeasureText(gFont.GetTextDC(), aMacroTime, n, &TextSize);
        TextSize.cx = (LONG)((double)TextSize.cx / gWindow.GetScreenRateX());
        TextSize.cy = (LONG)((double)TextSize.cy / gWindow.GetScreenRateY());
        float Bar = (float)v8;
        RenderBar(x, 404.0f, 50.0f, 2.0f, Bar, false, true);
    }
    Gfx::Color3f(1.0f, 1.0f, 1.0f);

    int v1 = m_iMatchCountDownType;
    if (v1 > 0) {
        DWORD TickCount = Platform::GetTimer().Ticks();
        DWORD elapsed   = TickCount - m_dwMatchCountDownStart;
        if (elapsed <= 0x7530) {   // 30s
            GL_ResetState();          // DisableAlphaBlend
            EnableAlphaTest(false);
            m_dwBackColor = 0x80000000u;
            DWORD secs = elapsed / 0x3E8;
            m_dwTextColor = 0xFFFF8080u;   // -32640

            // Las dos tablas del original son `300*type + 131435000` y
            // `300*type + 131489300`.  Con GlobalText en 0x07D29D24 y stride 300
            // eso da GlobalText[639 + type] y GlobalText[820 + type] -- el port
            // tenia 431/612, que apuntan a textos de otra cosa.  Para Blood
            // Castle el tipo 5 es GlobalText[825] = "Infiltracion al Blood
            // Castle (en %d segundos)".
            int idx = (v1 < 4 || v1 > 7) ? (639 + v1) : (820 + v1);
            const char* fmt = (idx >= 0 && idx < 1000) ? GlobalText[idx] : "%d";
            CHAR String[256];
            Platform::TextFormat(String, fmt, 30 - (int)secs);
            UI_DrawText(10, 372, String, 0, 1, 0, sizeof(String));
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
    if ((int)gMapManager.GetCurrentMap() < 11 || (int)gMapManager.GetCurrentMap() > 16) return;

    GL_ResetState();
    EnableAlphaTest(false);
    Gfx::Color3f(1.0f, 1.0f, 1.0f);
    float v1 = 345.0f;
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFF7700u;   // -16738561

    if ((m_byMatchType <= 2 || m_byMatchType == 5) && m_iMatchTime > 0) {
        if ((int)m_iMaxKillMonster != 0xFFFF) {
            CHAR String[256];
            int gtIdx = (m_byMatchType == 5) ? 866 : 864;
            Platform::TextFormat(String, GlobalText[gtIdx], m_iKillMonster, m_iMaxKillMonster);
            gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
            m_dwTextColor = 0xFFFF7700u;
            RenderCenteredText(570, 345, String);
            v1 = 357.0f;
        }
        gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
        CHAR String[256];
        Platform::TextFormat(String, GlobalText[865]);
        RenderCenteredText(570, (int)v1, String);

        float v2 = v1 + 10.0f;
        int v0 = m_iMatchTime / 60;
        Platform::TextFormat(String, " %.2d :", v0);
        if (m_iMatchTime - 60 * v0 >= 0) {
            CHAR v3[12];
            Platform::TextFormat(v3, " %.2d", m_iMatchTime - 60 * v0);
            strcat(String, v3);
        }
        if (v0 < 5) m_dwTextColor = 0xFF1FFFFFu;     // -14671617
        if (v0 < 15) {
            CHAR v3[12];
            // WorldTime is the engine animation tick (used as 60Hz seconds approx).
            Platform::TextFormat(v3, ": %.2d", (int)((std::int64_t)WorldTime % 60));
            strcat(String, v3);
        }
        // IDA usa g_hFontBig (0x055CA014) para el reloj: es la bold al DOBLE de
        // altura.  El port ponia bold porque `g_hFontBig` era un alias local que
        // apuntaba justamente a la bold; ahora sale de globals.h.
        gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BIG));
        RenderCenteredText(570, (int)v2, String);
    }
}

void Render_MapLoadText(void) { Render_MapLoadText_(); }


// =============================================================================
// Render_QuickButtons — sub_4F5820.  HUD UI dispatcher.  Calls 9 sub-panels
// in sequence (party / inventory / trade / shop / chaos-mix / warehouse /
// event / golden-archer / server-division) plus a quest-state refresh.
// =============================================================================
extern "C" void __cdecl Render_QuickButtons_(void);
void Render_QuickButtons_(void)
{
    Gfx::Color3f(1.0f, 1.0f, 1.0f);
    GL_ResetState();
    m_dwTextColor = 0xFFFF8080u;
    // Los 3 paneles del lado derecho van en x CONSTANTE 0x1C2 (=450), no en
    // `GetScreenWidth()` (que devuelve 260 con Inventory+Character abiertos):
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
        // El origen de los botones OK/CANCEL es `g_GuildCreatorScratchX/Y`, el mismo
        // que usa el hit-test SecondPassword_Screen1 (no `DAT_07ea5b1c/20` =
        // Inventory[32].Level/Part, que es el slot 0 del pool de la tienda).
        // Los tres offsets coinciden con esos hit-tests: +20/+350 y +100 el segundo.
        float btnX = (float)g_GuildCreatorScratchX + _DAT_005524fc;
        float btnY = (float)g_GuildCreatorScratchY + _DAT_00552ca4;
        Gfx::Color3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 2.1875f, 0.65625f, 1, 1);
        int tex = ((int)MouseX >= (int)btnX && (int)MouseX < (int)(btnX + 70.0f) &&
                   (int)MouseY >= (int)btnY && (int)MouseY < (int)(btnY + 21.0f)) ? 0xF2 : 0xF1;
        GL_DrawTexture(tex, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);

        // El segundo boton arranca en +100 desde el origen del panel, NO en +20+100.
        // IDA `sub_4E4760` L446-447 da los dos rects:
        //     boton 1 (crear):  [origin+20 , origin+90 )   ancho 70
        //     boton 2 (cancel): [origin+100, origin+170)   ancho 70
        // (`_DAT_005524f0` es TERRAIN_SCALE, no un offset de UI.)
        btnX = (float)g_GuildCreatorScratchX + 100.0f;
        GL_DrawTexture(0x118, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 2.1875f, 0.65625f, 1, 1);
        tex = ((int)MouseX >= (int)btnX && (int)MouseX < (int)(btnX + 70.0f) &&
               (int)MouseY >= (int)btnY && (int)MouseY < (int)(btnY + 21.0f)) ? 0xF4 : 0xF3;
        GL_DrawTexture(tex, btnX, btnY, 70.0f, 21.0f, 0.0f, 0.0f, 0.546875f, 0.65625f, 1, 1);
    }

    if (HUD_IsGuildListRuntime()) {
        float iconX = (float)DAT_07e91788 + _DAT_00552464;
        float iconY = (float)DAT_07e91784 + _DAT_0055246c;
        Gfx::Color3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, iconX, iconY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        GL_DrawTexture(RepairEnable_0 ? 287 : 286,
                     iconX, iconY, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((int)MouseX >= (int)iconX && (int)MouseX < (int)(iconX + 24.0f) &&
            (int)MouseY >= (int)iconY && (int)MouseY < (int)(iconY + 24.0f)) {
            gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)iconX, (int)iconY - 12, GlobalText[233]);
        }
    }

    if (HUD_IsCharacterInfoRuntime()) {
        float iconX = (float)DAT_07ea982c + 25.0f;
        float iconY = (float)DAT_07ea9830 + 395.0f;
        Gfx::Color3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(0x118, iconX, iconY, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        GL_DrawTexture(280, iconX, iconY, 24.0f, 24.0f,
                     0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
        if ((int)MouseX >= (int)iconX && (int)MouseX < (int)(iconX + 24.0f) &&
            (int)MouseY >= (int)iconY && (int)MouseY < (int)(iconY + 24.0f)) {
            gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            RenderTipText((int)iconX, (int)iconY - 13, GlobalText[225]);
        }
    }

    // En el original (sub_4F5820) hay 3 `if` sites antes/alrededor de RenderParty
    // que llaman a RenderCharacterInfoWindow/RenderGuildList/RenderGuildCreation
    // cuando los flags correspondientes están seteados.
    // El ORDEN importa -- los tres paneles se dibujan en el mismo x=450 y el
    // ultimo tapa a los anteriores.  IDA sub_4F5820:
    //     if (GuildCreatorOpened) RenderGuildCreation(450, 0);
    //     else if (GuildOpened)   RenderGuildList(450, 0);
    //     RenderParty(450, 0);
    //     if (CharacterOpened)    RenderCharacterInfoWindow(450, 0);
    if (HUD_IsGuildListRuntime())      RenderGuildList(panelStartX, 0);

    RenderParty(450, 0);

    if (HUD_IsCharacterInfoRuntime())  RenderCharacterInfoWindow(panelStartX, 0);
    // DESVIACION: panel H de horarios en la misma columna (se alterna con los anteriores).
    gEventTimer.RenderPanel();
    RenderInventoryWindow();
    RenderTrade();
    RenderShopInterface();
    RenderChaosMix();
    RenderWarehouse();
    RenderEventWindow();
    RenderGoldenArcherWindow();

    // Render del panel de quest.  IDA sub_4F5820 L38:
    // `sub_403F30((_BYTE *)g_csQuest);` entre RenderGoldenArcherWindow y
    // RenderServerDivision.
    FUN_00403f30((void*)(uintptr_t)g_csQuest);

    // El drop se despacha sólo desde UpdateWindowsMouse en SIM (IDA).
    RenderServerDivision();
}

void Render_QuickButtons(void) { Render_QuickButtons_(); }


// =============================================================================
namespace {
struct MixCellSample { float green=0.4f, red=0.6f, size=5.0f, x=0, y=0; };
struct GridSample { int state=0; MixCellSample cells[32]; };
GridSample gridSamples[2][7];
int gridCount[2] = {}, gridCursor[2] = {};
}
void HUD_BeginGridPass(int pass) { gridCursor[pass] = 0; }
// DESVIACION: reproducir en SIM la multiplicidad de ambas pasadas de grillas.
void HUD_TickGridPresentation() {
    gridCount[0] = gridCount[1] = 0;
    if (SceneFlag != 5 || !DAT_083a7c10 || DAT_07e11d1c > 30 ||
        gUserSettings.GetAntilag(ANTILAG_INTERFACE)) return;
    bool pools[7]; int count=0;
    if (InventoryOpened) pools[count++]=false;
    if (ShopOpened) pools[count++]=false;
    if (TradeOpened) { pools[count++]=false; pools[count++]=false; }
    if (WarehouseOpened) pools[count++]=false;
    if (ChaosMixOpened) pools[count++]=true;
    if (EventWindowOpened) pools[count++]=true;
    for (int pass=0; pass<2; ++pass) {
        if (pass==0 && GoldenArcherOpenType && GoldenArcherOpenType != 3) continue;
        gridCount[pass]=count;
        for (int grid=0; grid<count; ++grid) {
            GridSample &sample=gridSamples[pass][grid];
            sample.state=MixState;
            if (pass==1 && pools[grid] && MixState>0 && MixState<51) {
                for (int cell=0; cell<32; ++cell) {
                    if (*(short *)(OffsetMixItems + cell*68)==-1) continue;
                    MixCellSample &c=sample.cells[cell];
                    c.green=(float)(rand()%4+4)*0.1f;
                    c.red=(float)(rand()%6+6)*0.1f;
                    c.size=(float)(rand()%10+5);
                    c.x=(float)(rand()%20); c.y=(float)(rand()%20);
                }
            }
            if (MixState>1) ++MixState;
        }
    }
}

// sub_4E38B0 — inventory grid render.  Walks an N×M grid of 68-byte ITEM
// records and renders each occupied cell either as a 3D mesh (`a6 != 0`)
// via RenderItem3D, or as a 2D quantity glyph (RenderNumber2D) for arrows.
//
// Special case for OffsetMixItems during MixState animation: spawns three
// glow particles via sub_5126E0 (HUD_Pass6.cpp).
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
    const int pass = (BYTE)a6 ? 1 : 0;
    const int grid = gridCursor[pass]++;
    const GridSample *sample = grid < gridCount[pass] ? &gridSamples[pass][grid] : nullptr;
    const int mixState = sample ? sample->state : (int)MixState;
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
                        if (*(int *)&AsItem(v6_addr)->Key > 0) {
                            if ((BYTE)a6) {
                                // DESVIACION (DLL ItemStack.cpp RenderItemNumber, 0x004E398C):
                                // también los items que el server declara apilables.
                                const int kind = ItemBehaviorType(v7);   // 0.97.20
                                const bool stack = (kind >= 448 && kind <= 456) ||
                                    ItemStack_GetMaxStack(v7, (AsItem(v6_addr)->Level >> 3) & 0xF) != 0;
                                if (stack && AsItem(v6_addr)->Durability > 1) {
                                    Gfx::Color3f(1.0f, 0.9f, 0.7f);
                                    float h = sxa + 13.0f;
                                    RenderNumber2D(h, sy,
                                                   AsItem(v6_addr)->Durability,
                                                   9.0f, 10.0f);
                                }
                            } else {
                                ITEM_ATTRIBUTE* v8 = &ItemAttribute[v7];
                                float Height = (float)((double)v8->Height * 20.0);
                                float Width  = (float)((double)v8->Width  * 20.0);
                                RenderItem3D(sxa, sy, Width, Height, v7,
                                             AsItem(v6_addr)->Level,
                                             AsItem(v6_addr)->Option1, 0, 0);
                            }
                            ++v23;
                        }

                        if ((BYTE)a6 && (BYTE*)(uintptr_t)x_addr == OffsetMixItems
                            && mixState > 0 && mixState < 51)
                        {
                            EnableAlphaBlend();
                            const int cell = (int)((v6_addr - x_addr) / 68);
                            const MixCellSample c = sample && cell < 32 ? sample->cells[cell] : MixCellSample{};
                            float green = c.green;
                            float red = c.red;
                            Gfx::Color3f(red, green, 0.2f);
                            int seed = (int)((double)((std::int64_t)WorldTime % 100) * 20.0);
                            float v20 = c.size;
                            float v28 = c.x + sxa;
                            float sxb = c.y + sy;
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
                Gfx::Color3f(0.0f, 1.0f, 1.0f);
                float sxe = (float)((double)ItemAttribute[v11->Type].Width * 20.0);
                float xa  = (float)((double)(20 * v11->x) + (double)TradeInventoryStartX + 15.0);
                float v36 = (float)(sin((double)WorldTime * 0.015) +
                                    (double)(20 * v11->y) +
                                    (double)TradeInventoryStartY + 70.0);
                float y_pos = v36 + 5.0f;
                GL_DrawTexture(9, xa, y_pos, 24.0f, 24.0f, 0.0f, 0.40000001f, 1.0f, 1.0f, 1, 1);
                Gfx::Color3f(1.0f, 1.0f, 1.0f);
                gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
                m_dwTextColor = 0xFFFFFFFFu;
                m_dwBackColor = 0xFF000052u;
                int boxW = (int)((double)gWindow.GetWidth() * sxe * 0.0015625);
                RenderText((int)xa, (int)v36, GlobalText[370], boxW, 0, 0);
            }
            ++v11;
        }
    }

    return mixState;
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
    // DESBORDE DE PILA (2026-10-02, issue #75): estas dos ramas copiaban
    // strlen(InputText[Index]) bytes en Text[260] SIN cota -- la tercera rama ya
    // usaba strncpy, o sea el limite estaba contemplado y faltaba aca.
    // InputText es [10][256] contiguo, asi que un slot que quede sin NUL (p.ej.
    // strncpy(InputText[1], src, 0x100) con origen de 256 bytes) hace que strlen
    // siga hacia los slots siguientes y devuelva hasta 2560.  Como esta funcion
    // se inlinea dentro de Scene_Login con /O2, el desborde pisaba la cookie /GS
    // de Scene_Login: MSVC reporta eso con __fastfail, que mata el proceso sin
    // pasar por el filtro de excepciones -- de ahi los cierres sin cartel ni log.
    const size_t kMaxText = sizeof(Text) - 1;
    if (hide == 1) {
        size_t len = strlen(InputText[Index]);
        if (len > kMaxText) len = kMaxText;
        for (size_t i = 0; i < len; ++i) Text[i] = '*';
        Text[len] = 0;
    } else if (hide == 2) {
        const char* src = InputText[Index];
        size_t len = strlen(src);
        if (len > kMaxText) len = kMaxText;
        size_t i = 0;
        for (; i < 7 && i < len; ++i) Text[i] = src[i];
        for (; i < len; ++i) Text[i] = '*';
        Text[i] = 0;
    } else {
        strncpy(Text, InputText[Index], sizeof(Text) - 1);
    }

    int v7 = InputTextWidth;
    UI_DrawText(x, y, Text, InputTextWidth, 1, 0, sizeof(Text));
    int n = Platform::TextLength(Text);
    gFont.MeasureText(gFont.GetTextDC(), Text, n, &TextSize);
    if (v7 > 0 && TextSize.cx > v7) TextSize.cx = v7;

    // IDA (RenderInputText 0x47F0B0) posiciona el caret con el TextSize ya
    // dividido, o sea en espacio lógico (UI_DrawText -> CUIRenderText_RenderText
    // convierte lógico -> físico con g_fScreenRate_x/y, igual que `sub_410AF0`).
    //   ancho medido -> píxel -> / g_fScreenRate_x -> lógico -> + x (lógico)
    TextSize.cx = (LONG)((double)TextSize.cx / gWindow.GetScreenRateX());
    TextSize.cy = (LONG)((double)TextSize.cy / gWindow.GetScreenRateY());

    if (Index == InputIndex) {
        int v9 = InputFrame % 2;
        if (v9 == 0) {
            const char* ime = InputTextIME[Index];
            if (strlen(ime) != 0) {
                if (InputTextHide[Index] == 1) {
                    UI_DrawText(x + TextSize.cx, y, (char*)"**", 0, 1, 0, 3);
                    gFont.MeasureText(gFont.GetTextDC(), "**", 2, &TextSize);
                } else {
                    UI_DrawText(x + TextSize.cx, y, (char*)ime, 0, 1, 0, sizeof(InputTextIME[Index]));
                    gFont.MeasureText(gFont.GetTextDC(), ime, Platform::TextLength(ime), &TextSize);
                }
            } else {
                UI_DrawText(x + TextSize.cx, y, (char*)"_", 0, 1, 0, 2);
                gFont.MeasureText(gFont.GetTextDC(), "_", 1, &TextSize);
            }
            TextSize.cx = (LONG)((double)TextSize.cx / gWindow.GetScreenRateX());
            TextSize.cy = (LONG)((double)TextSize.cy / gWindow.GetScreenRateY());
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
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL) ? gFont.GetFont(FONT_NORMAL) : gFont.GetFont(FONT_BOLD));
    int n = Platform::TextLength(Text);
    gFont.MeasureText(gFont.GetTextDC(), Text, n, &sz);
    int prevBlend = AlphaBlendType;

    EnableAlphaTest(true);
    Gfx::Color4f(0.0f, 0.0f, 0.0f, 1.0f);

    float fy   = (float)sy;
    float fx   = (float)sx;
    float boxX = fx - 2.0f;     // ≈ flt_55264C constant
    float boxY = fy - 1.0f;     // ≈ flt_552540 constant
    float W    = (float)((double)sz.cx / gWindow.GetScreenRateX() + 4.0);
    float H    = (float)((double)sz.cy / gWindow.GetScreenRateY() + 4.0);

    // 4 thin border strips (top, left, right, bottom).
    GL_DrawRect(boxX, boxY, W, 1.0f);
    GL_DrawRect(boxX, boxY, 1.0f, H);
    GL_DrawRect(boxX + W - 1.0f, boxY, 1.0f, H);
    GL_DrawRect(boxX, boxY + H - 1.0f, W, 1.0f);

    Gfx::Color4f(0.0f, 0.0f, 0.0f, 0.8f);
    // Filled black panel inside the border.
    GL_DrawRect(boxX + 1.0f, boxY + 1.0f, W - 2.0f, H - 2.0f);

    Gfx::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    // CACHE (2026-10-03): este glEnable no tocaba DAT_083a4125, asi que el GL
    // quedaba texturado con el cache diciendo lo contrario y el proximo
    // DisableTexture salteaba su glDisable.  Mismo patron que la fuga de blend
    // que volvia traslucidos a los monstruos de Atlans.
    Gfx::Enable(Gfx::Cap::Texture2D);   DAT_083a4125 = 1;
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFFFFFFu;
    UI_DrawText(sx, sy, (char*)Text, 0, 1, 0, (size_t)n + 1);

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

    Gfx::BindTexture(Bitmaps[34].TextureNumber);
    Gfx::TexImage2D(Bitmaps[34].Components, Width, Height, Gfx::Format::RGBA, Bitmaps[34].Buffer);
}


