// HUD_Pass2.cpp — second batch of HUD ports.

//
// Functions in this TU (all 1:1 from IDA sub_xxxxxx of the original mu.exe):
//   * Render_ChatBox          (sub_4BE4F0)  — chat input box + scroll history
//   * Render_CharInfoPanel    (sub_4BC220)  — guild war / soccer score banner
//   * RenderEquipedHelperLife (sub_4BEC00)  — pet/helper life bar (top centre)
//   * RenderBrokenItem        (sub_4BE710)  — durability warnings (right side)
//   * RenderExperience        (sub_4BF990)  — XP bar + tooltip on hover
//
// Helpers ported here (because no callers existed yet):
//   * RenderBar       (sub_4BBDD0) — border + filled bar (used by 3 funcs)
//   * RenderNumber2D  (sub_5122F0) — bitmap-glyph integer renderer
//   * sub_47F6F0      — RenderText with shadow style (returns SIZE*)
//   * GetScreenWidth  (sub_4CB520) — UI panel-aware screen-width
//
// Helper STUBS (the real bodies are 600+ bytes each, port follow-up):
//   * RenderInputText (sub_47F0B0)
//   * RenderTipText   (sub_47F7F0)
//   * CreateGuildMark (sub_4F0100)
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <gl/GL.h>

extern "C" BYTE OffsetInventoryItems[];

// ── Globals only used in this TU ────────────────────────────────────────────
// Kept local because none of these existed in our globals.h yet and they're
// only referenced by the HUD passes in this file.

extern "C" {
    int  InputTextWidth      = 0;       // chat input field width in pixels
    BYTE m_bBlockWhisper     = 0;       // whisper-mute toggle (chat mute icon)
    int  TabInputEnable      = 0;       // tab-targeting input mode flag
    // EventWindowOpened/PartyOpened/GuildOpened/GuildCreatorOpened/
    // WarehouseOpened/ChaosMixOpened/ShopOpened/TradeOpened/InventoryOpened/
    // CharacterOpened are now #define aliases for DAT_07eaa11x in globals.h
    // (so IDA-ported toggle code and render gates share the same byte).
}

// 2026-08-22: `SummonLife` NO es DAT_07e11d28.  Una nota vieja lo aliaseaba
// ahí y colisionaba con el contador de debounce del walker
// (Player_InputTick.cpp), que lo incrementa cada frame — por eso la barra de HP
// del monstruo invocado se dibujaba siempre (el triángulo cyan).  El parche de
// entonces fue una variable local nueva, o sea el global quedó partido en dos.
// `ida_xrefs_to("SummonLife")` da la dirección real: **0x05826D24**
// (= DAT_05826d24), escrita por InitGame, ReceiveRevival (3 sitios) y el
// F3/0x20 de ProtocolCore, y leída por RenderEquipedHelperLife.  Ahora el alias
// apunta ahí, así que el reset de Recv_Revival lo ve este render.
// (Pendiente: portar el F3/0x20 `SummonLife = ReceiveBuffer[4]`, el único
//  productor del valor; hasta entonces queda en 0 y la barra no se dibuja.)
#define SummonLife               DAT_05826d24
#define g_bEventChipDialogEnable _g_bEventChipDialogEnable
#define byte_7E113E4         (*(char(*)[5][256])&DAT_07e113e4)
#define flt_7E118E4_PTR      (((char*)&DAT_07e113e4) + 5*256)  // one past last
#define byte_7E919BC         (*(char(*)[80])&DAT_07e919bc)     // single 80-byte slot view
#define dword_55C9CC4        DAT_00559cc4                      // chat scroll cursor
#define byte_7E11D6E         DAT_07e11d6e
#define ItemAttribute        ((ITEM_ATTRIBUTE*)DAT_07d78068)
extern "C" int DAT_07d78068;     // ItemAttribute base (ITEM_ATTRIBUTE*, stride 64)

// aTime_0 — string constant.  In IDA it lives in .rdata; replicate.
static const char aTime_0[] = "Time";

// dword_7EA51EC — end-pointer sentinel for byte_7E919BC name table walk.
// In our build the underlying array isn't sized; we approximate the walk
// limit at base + 80*100 (= 8000 bytes, 100 names) which is more than the
// game ever populates.
#define DWORD_7EA51EC_LIMIT  ((char*)&DAT_07e919bc + 80 * 100)

// RenderInputText / RenderTipText / CreateGuildMark are now ported in
// src/Render/HUD_Pass4.cpp — forward-declare them here so call sites in
// this TU resolve.
extern "C" void __cdecl RenderInputText(int x, int y, int slot);
extern "C" void __cdecl RenderTipText  (int x, int y, const char* text);
extern "C" void __cdecl CreateGuildMark(int markIndex, bool blend);

static bool HUD_IsQuestPanelOpenRuntime(void)
{
    return (g_csQuest != 0) &&
           // 2026-08-21: era 0x1C8FF.  El flag "panel de quest abierto" esta en
           // g_csQuest + 116863 (0x1C87F) — IDA lo usa asi en 9 sitios, uno de
           // ellos este mismo GetScreenWidth (0x4CB520 L103).  Con 0x1C8FF se
           // leia un byte 0x80 mas adelante.
           (*(BYTE*)((BYTE*)(uintptr_t)g_csQuest + 0x1C87F) != 0);
}

static bool HUD_IsGoldenArcherPanelRuntime(void)
{
    return (DAT_07eaa128 != 0 && DAT_07eaa128 != 3);
}

static bool HUD_IsInventorySidePairOpen(void)
{
    return ShopOpened || WarehouseOpened || ChaosMixOpened ||
           TradeOpened || EventWindowOpened;
}

static bool HUD_IsAnyRightPanelOpen(void)
{
    return InventoryOpened || CharacterOpened || PartyOpened ||
           GuildOpened || GuildCreatorOpened ||
           HUD_IsGoldenArcherPanelRuntime() ||
           HUD_IsQuestPanelOpenRuntime() ||
           (DAT_07eaa130 != '\0');
}

// ── Helper PORTS ─────────────────────────────────────────────────────────────

// sub_47F6F0 — RenderText with shadow / outline style (a5 selects style).
// Returns &TextSize so callers can chain reads of the dimension.
//
// Uses g_pRenderText vtable (CUIRenderText::RenderText, slot ?? in our build);
// our codebase doesn't expose g_pRenderText so we route through RenderText_1
// (FUN_0047f7a0) which is the same engine path with default style flags.
extern "C" SIZE* __cdecl FUN_0047f6f0(int x, int y, const char* lpString,
                                      int boxWidth, char style, int extraSize);
SIZE* __cdecl FUN_0047f6f0(int x, int y, const char* lpString,
                            int boxWidth, char style, int extraSize)
{
    if (!lpString) return NULL;
    if (!*lpString && !boxWidth) return NULL;

    // Forward to our existing RenderText_1.  The IDA original uses style
    // (0/1) + 1 to pick palette and box rendering; RenderText_1 does the
    // same with default style.  Keep the call signature compatible.
    FUN_0047f7a0(x, y, (char*)lpString, boxWidth, style ? 2 : 1, extraSize);

    int n = lstrlenA(lpString);
    GetTextExtentPointA(m_hFontDC, lpString, n, &TextSize);
    TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);
    TextSize.cy = (LONG)((double)TextSize.cy / _DAT_055c9b74);
    return &TextSize;
}

// RenderBar — sub_4BBDD0.  Border + filled progress bar, with optional
// 4-px screen-clip and red/green palette by Disabled flag.
extern "C" void __cdecl RenderBar(float x, float y, float Width, float Height,
                                  float Bar, bool Disabled, bool clipping);
void __cdecl RenderBar(float x, float y, float Width, float Height,
                        float Bar, bool Disabled, bool clipping)
{
    if (clipping) {
        if (x < 0.0f) x = 0.0f;
        if (y < 0.0f) y = 0.0f;
        if (x + Width  + 4.0f > 640.0f) x = 640.0f - (Width + 5.0f);
        if (y + Height + 4.0f > 433.0f) y = 433.0f - (Height + 5.0f);
    }
    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    FUN_005124c0(x + 1.0f, y + 1.0f, Width + 4.0f, Height + 4.0f);
    EnableAlphaBlend();
    if (Disabled) glColor3f(0.2f, 0.0f, 0.0f);
    else          glColor3f(0.0f, 0.2f, 0.2f);
    FUN_005124c0(x, y, Width + 4.0f, Height + 4.0f);
    if (Disabled) glColor3f(0.19607843f, 0.039215688f, 0.0f);
    else          glColor3f(0.0f, 0.19607843f, 0.19607843f);
    FUN_005124c0(x + 2.0f, y + 2.0f, Width, Height);
    if (Disabled) glColor3f(0.78431374f, 0.19607843f, 0.0f);
    else          glColor3f(0.0f, 0.78431374f, 0.19607843f);
    FUN_005124c0(x + 2.0f, y + 2.0f, Bar, Height);
    FUN_00511600();   // DisableAlphaBlend
}

// RenderNumber2D — sub_5122F0.  Renders an integer using bitmap glyph 1
// (digits 0-9 packed left-to-right at u-step 0.0625).  Width spacing factor
// 0.7 packs digits closely.
extern "C" double __cdecl RenderNumber2D(float x, float y, int Num,
                                          float Width, float Height);
double __cdecl RenderNumber2D(float x, float y, int Num,
                               float Width, float Height)
{
    char Buffer[32];
    _itoa(Num, Buffer, 10);
    int n = (int)strlen(Buffer);
    float xa = x - (double)(n) * Width * 0.5;
    if (n <= 0) return xa;
    int i = 0;
    do {
        double u = (double)(Buffer[i] - '0') * 0.0625;
        FUN_005125a0(1, xa, y, Width, Height, (float)u, 0.0f, 0.0625f, 0.5f, 1, 1);
        ++i;
        xa = Width * 0.69999999f + xa;
    } while (i < n);
    return xa;
}

// GetScreenWidth — sub_4CB520.  Returns the X coord at which the bottom-
// HUD's "right edge" sits, depending on which UI panels are open.
//   * Inventory + (Character | Shop | Warehouse | ChaosMix | Trade | Event)
//                                                        → 260
//   * Inventory only                                     → 450
//   * Character / Party / Guild / GuildCreator           → 450
//   * Quest panel (g_csQuest+0x1C87F == 1)               → 450
//   * ServerDivision panel                               → 450
//   * default                                            → 640
//
// The original interleaves the result with the same anti-tamper hash-table
// dance every other render does (refs ShopOpened / TradeOpened).  We skip
// it: the path is benign in our build.
extern "C" int __cdecl GetScreenWidth(void);
int __cdecl GetScreenWidth(void)
{
    if (InventoryOpened) {
        if (CharacterOpened) return 260;
        if (HUD_IsInventorySidePairOpen()) return 260;
        return 450;
    }
    if (HUD_IsAnyRightPanelOpen()) return 450;
    return 640;
}

// =============================================================================
// Render_ChatBox — sub_4BE4F0.
// Two background panels (texture 248/249) at y=415 + chat input fields
// (whisper target + main text) + scrolled chat-history (5 entries) +
// whisper-mute icon.
// =============================================================================
extern "C" void __cdecl Render_ChatBox_(void);
void Render_ChatBox_(void)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, g_hFont ? g_hFont : g_hFontBold);

    if (!(InputEnable || DAT_07e11d71)) return;

    FUN_00511600();   // DisableAlphaBlend
    FUN_005125a0(248, 186.0f, 415.0f, 134.0f, 29.0f, 0.0f, 0.0f, 0.51953125f, 0.90625f, 1, 1);
    FUN_005125a0(249, 320.0f, 415.0f, 134.0f, 29.0f, 0.0f, 0.0f, 0.51953125f, 0.90625f, 1, 1);
    EnableAlphaTest(true);

    InputTextWidth = 180 * (int)WindowWidth / 0x280;
    if ((int)(180 * WindowWidth / 0x280) > 256) InputTextWidth = 256;
    RenderInputText(191, 422, 0);

    InputTextWidth = 50 * (int)WindowWidth / 0x280;
    RenderInputText(376, 422, 1);

    int v0 = (int)dword_55C9CC4;
    InputTextWidth = 256;
    m_dwTextColor = (DWORD)-3618616;     // 0xFFC8C8C8 ish
    m_dwBackColor = 1677721600u;          // 0x64000000

    // Walk byte_7E113E4 entries (stride 256 bytes, 5 entries).
    int v1 = 0;
    char* v2 = (char*)&DAT_07e113e4;
    char* end = (char*)flt_7E118E4_PTR;
    while (v2 < end) {
        if (v1 != v0) {
            FUN_0047f7a0(376, 15 * (v1 - v0) + 422, v2, 0, 1, 0);
            int n = lstrlenA(v2);
            GetTextExtentPointA(m_hFontDC, v2, n, &TextSize);
            TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);
            v0 = (int)dword_55C9CC4;
            TextSize.cy = (LONG)((double)TextSize.cy / _DAT_055c9b74);
        }
        v2 += 256;
        ++v1;
    }

    if (DAT_00559bf0 != 0) {
        FUN_005125a0(239, 432.0f, 420.0f, 15.0f, 16.0f, 0.0f, 0.0f, 0.9375f, 1.0f, 1, 1);
    }
}

void Render_ChatBox(void) { Render_ChatBox_(); }


// =============================================================================
// Render_CharInfoPanel — sub_4BC220.  Top-centre banner used during guild
// war or soccer events.  Displays:
//   * Hero's guild mark + score (line 1)
//   * Enemy guild mark + score (line 2)
//   * Soccer time when SoccerTime > 0
//
// Most of this is dormant in non-event play (EnableGuildWar=0,
// SoccerObserver=0, SoccerTime=0 by default in our build).
//
// Note: the IDA decomp shows several `&dword_7EA51EC` comparisons used to
// terminate the byte_7E919BC name-table scan.  We approximate with a
// 100-name (8000-byte) bound — see DWORD_7EA51EC_LIMIT.
// =============================================================================

// 2026-08-22: `SoccerTime` y `SoccerObserver` NO son DAT_07e11e10 / DAT_07e11e14.
// `ida_xrefs_to` da 0x05826C08 y 0x05826D33; de 0x07E11E10 el unico xref en todo
// el binario es sub_494520 (el bloque anti-tamper de IME/RC4), o sea el alias
// viejo era inventado.  Mientras estuvo mal, los escritores (InitGame y ahora
// los handlers F3/22 y F3/23) y este lector estaban en memorias distintas, asi
// que el reloj del evento y el marcador nunca se dibujaban.  Ver
// [[global-partido-en-dos]].
#define DAT_07e11e10_alias  DAT_05826c08   // SoccerTime
#define DAT_07e11e14_alias  DAT_05826d33   // SoccerObserver

extern "C" char __cdecl Render_CharInfoPanel_(void);
char Render_CharInfoPanel_(void)
{
    FUN_00511600();   // DisableAlphaBlend
    m_dwBackColor = 0x80000000u;

    char ret = 0;
    float v24_y0 = 4.0f;
    float x = 0.0f;

    if (EnableGuildWar && Hero && *(WORD*)((BYTE*)(uintptr_t)Hero + 474) != 0xFFFF) {
        EnableAlphaTest(true);
        x = 320.0f - 20.0f;
        SelectObject(m_hFontDC, g_hFontBold);
        int v20 = *(short*)((BYTE*)(uintptr_t)Hero + 474);
        m_dwTextColor = HeroSoccerTeam != 0 ? 0xFFFF5901 : 0xFF008000u;
        CreateGuildMark(v20, 1);
        FUN_005125a0(34, x, 4.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);

        CHAR String[100];
        wsprintfA(String, "%d", GuildWarScore[0]);
        int iPos_x = (int)x + 13;
        FUN_0047f7a0(iPos_x, 4, String, 0, 1, 0);
        int n = lstrlenA(String);
        GetTextExtentPointA(m_hFontDC, String, n, &TextSize);

        const char* v1 = (char*)&DAT_07e919bc + 80 * v20;
        int v2 = (int)x + 30;
        FUN_0047f7a0(v2, 4, (char*)v1, 0, 1, 0);

        // Walk the name table to find the GuildWarName slot index.
        int v23 = 0;
        const char* v22 = (char*)&DAT_07e919bc;
        m_dwTextColor = HeroSoccerTeam != 0 ? 0xFF008000u : 0xFFFF9560;
        int v5 = 0;
        while (strcmp(v22, GuildWarName) != 0) {
            bool keep = (int)(v22 + 80) < (int)DWORD_7EA51EC_LIMIT;
            ++v23;
            v22 += 80;
            if (!keep) { v5 = 0; goto LABEL_7; }
        }
        v5 = v23;
LABEL_7:
        CreateGuildMark(v5, 1);
        FUN_005125a0(34, x, 19.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
        wsprintfA(String, "%d", GuildWarScore[1]);
        FUN_0047f7a0(iPos_x, 19, String, 0, 1, 0);
        FUN_0047f7a0(v2, 19, GuildWarName, 0, 1, 0);
    }

    if (DAT_07e11e14_alias) {  // SoccerObserver
        EnableAlphaTest(true);
        v24_y0 = 4.0f;
        x = 320.0f - 20.0f;
        SelectObject(m_hFontDC, g_hFontBold);
        m_dwTextColor = 0xFF008000u;
        CreateGuildMark(0, 1);
        FUN_005125a0(34, x, 4.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);

        CHAR String[100];
        wsprintfA(String, "%d", GuildWarScore[0]);
        int v9 = (int)x;
        int v10 = v9 + 13;
        FUN_0047f7a0(v10, 4, String, 0, 1, 0);
        int iPos_xa = v9 + 30;
        FUN_0047f7a0(iPos_xa, 4, SoccerTeamName[0], 0, 1, 0);

        m_dwTextColor = 0xFFFF9560;
        CreateGuildMark(0, 1);
        FUN_005125a0(34, x, 19.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
        wsprintfA(String, "%d", GuildWarScore[1]);
        FUN_0047f7a0(v10, 19, String, 0, 1, 0);
        FUN_0047f7a0(iPos_xa, 19, SoccerTeamName[1], 0, 1, 0);
    }

    if (DAT_07e11e10_alias > 0) {   // SoccerTime
        CHAR String[100];
        wsprintfA(String, "%d:%d",
                  DAT_07e11e10_alias / 60, DAT_07e11e10_alias % 60);
        m_dwTextColor = 0xFF969500u;
        int v15 = (int)v24_y0;
        int v16 = (int)x + 100;
        FUN_0047f7a0(v16, v15, (char*)aTime_0, 0, 1, 0);
        FUN_0047f7a0(v16, v15 + 13, String, 0, 1, 0);
    }

    return ret;
}

void Render_CharInfoPanel(void) { Render_CharInfoPanel_(); }


// =============================================================================
// RenderEquipedHelperLife — sub_4BEC00.
// Pet/helper life bar drawn at top-centre when a pet item (type 816..819)
// is equipped.  Also renders SummonLife bar at top-right.
//
// Reads CharacterMachine[+1106] (helper HP percentage) — wrapped in the
// usual anti-tamper hash table refresh.  In our build CharacterMachine is
// NULL so the inner block is skipped.
// =============================================================================
extern "C" int __cdecl RenderEquipedHelperLife_(bool a2);
// (Aca vivian IsHudHelperType / IsHudHelperSlotOccupied / ResolveHudHelperName /
//  ResolveHudHelperItemSource: una cadena de fallbacks inventada por el port que
//  buscaba el pet en el slot de ITEM.  IDA resuelve todo con `Hero + 0x2B8`, asi
//  que quedaron sin uso.  `IsHudHelperType` ademas aceptaba tipos que no existen
//  en el 0.97k — MODEL_HELPER + 37/64/65/67/80/106/123 son pets de versiones
//  posteriores; el binario gatea solo el rango 816..819.)

int RenderEquipedHelperLife_(bool a2)
{
    // 2026-08-22 FIX (la barra del pet no se dibujaba): el gate leia el SLOT DE
    // ITEM (`CharacterMachine + 536 + 68*8`) a traves de una cadena de fallbacks
    // inventada por el port.  IDA (0x4BEC00, verificado en el disassembly del
    // prologo) lee la ENTIDAD del heroe:
    //     mov ax, [eax+2B8h]   ; Hero + 696 = tipo del helper
    //     cmp ax, 330h / 333h  ; 816..819
    // Ese campo lo escriben SetCharacterClass, ChangeCharacterExt y el handler
    // 0x25.  El slot de item se usa SOLO para la vida y para el nombre por
    // defecto.
    //
    // (`+0x2B8` esta etiquetado como `char_class` en la tabla de offsets de
    //  CLAUDE.md — es falso, es el tipo del helper/pet equipado.)
    int retY = 15;                       // `mov esi, 0Fh` del prologo

    const BYTE* hero = (const BYTE*)DAT_07abf5d8;
    const WORD helperType = hero ? *(const WORD*)(hero + 0x2B8) : 0;

    DWORD backupBgTextColor = DAT_00559c80;
    DWORD backupTextColor   = DAT_00559c78;

    if (helperType >= 816 && helperType <= 819) {
        const float posY = a2 ? 24.0f : 4.0f;

        // x = GetScreenWidth() - 50.0 - (PartyNumber > 0 ? 50.0 : 0.0) - 15.0
        // (flt_552598 = 50.0 y flt_552834 = 15.0, leidos del binario).  Queda
        // arriba a la DERECHA; el port anterior la centraba en pantalla.
        const float x = (float)GetScreenWidth()
                      - 50.0f
                      - ((PartyNumber > 0) ? 50.0f : 0.0f)
                      - 15.0f;

        // Vida = durabilidad del item del slot helper (CharacterMachine + 1106
        // = wear slot 8 en +1080, campo Durability en +26).  Escala sobre 255.
        BYTE life = 0;
        if (CharacterMachine) life = *(const BYTE*)((const BYTE*)CharacterMachine + 1106);
        const int bar = 50 * (int)life / 255;

        const char* text = "";
        if (helperType == 818)      text = GlobalText[355];
        else if (helperType == 819) text = GlobalText[354];
        else if (ItemAttribute)     text = ItemAttribute[helperType - 400].Name;
        if (!text) text = "";

        SelectObject(m_hFontDC, g_hFont);
        EnableAlphaTest(true);
        DAT_00559c80 = 0x80000000u;
        DAT_00559c78 = 0xFFFFFFFFu;

        // IDA centra el texto en el rango [x, x+50] (sub_47F6F0 recibe x en st0
        // y x+50 como 2do arg).  Nuestra reimplementacion de esa funcion toma
        // (x, y, texto), asi que el centrado se calcula aca.
        SIZE ts = { 0, 0 };
        GetTextExtentPointA(m_hFontDC, text, lstrlenA(text), &ts);
        ts.cx = (LONG)((double)ts.cx / g_fScreenRate_x);
        int textX = (int)x + (50 - ts.cx) / 2;
        if (textX < 0) textX = 0;
        FUN_0047f6f0(textX, (int)posY, text, 0, 0, 0);

        RenderBar(x, posY + 12.0f, 50.0f, 2.0f, (float)bar, false, true);
        glColor3f(1.0f, 1.0f, 1.0f);

        retY = (int)posY + 23;
    }

    // Barra de la mascota INVOCADA por skill.  Es un bloque independiente del de
    // arriba: otra posicion, otra fuente de vida (SummonLife, escala 0..100) y
    // coordenadas FIJAS (y=4 el texto, y=16 la barra) — no encadena con posY.
    if (SummonLife) {
        const float xs = (float)GetScreenWidth() - 50.0f - 150.0f;

        SelectObject(m_hFontDC, g_hFont);
        EnableAlphaTest(true);
        DAT_00559c80 = 0x80000000u;
        DAT_00559c78 = 0xFFFFFFFFu;

        const char* summonText = GlobalText[356] ? GlobalText[356] : "";
        SIZE ts = { 0, 0 };
        GetTextExtentPointA(m_hFontDC, summonText, lstrlenA(summonText), &ts);
        ts.cx = (LONG)((double)ts.cx / g_fScreenRate_x);
        int textX = (int)xs + (50 - ts.cx) / 2;
        if (textX < 0) textX = 0;
        FUN_0047f6f0(textX, 4, summonText, 0, 0, 0);

        RenderBar(xs, 16.0f, 50.0f, 2.0f, (float)(50 * (int)SummonLife / 100), false, true);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    DAT_00559c80 = backupBgTextColor;
    DAT_00559c78 = backupTextColor;
    return retY;
}

void Render_CharPartyInfo(void)
{
    // sub_4BEC00 = RenderEquipedHelperLife.  Original signature has a bool
    // arg (`a2`) that selects the Y offset (4 vs 24).  Our call site in
    // Render_GameFrame doesn't propagate the bool — pass false.
    RenderEquipedHelperLife_(false);
}


// =============================================================================
// RenderBrokenItem — sub_4BE710.
// Walks the equipped item slots (8 slots × 68 bytes at CharacterMachine+536)
// and renders a "Name (cur/max)" durability label at top-right when the
// durability is at or below specific thresholds (red/orange/yellow palette).
// Surrounding the slot walk are anti-tamper hash-table dances on
// CharacterMachine and TradeOpened.
//
// In our build CharacterMachine=NULL; the entire walk is gated.  When
// CharacterMachine is later populated by the engine, this function will
// produce the right labels because the byte-pattern matches.
// =============================================================================
extern "C" void __cdecl RenderBrokenItem_(int a1);
void RenderBrokenItem_(int a1)
{
    bool tradeOpenedSnap = (TradeOpened != 0);
    if (tradeOpenedSnap) return;
    if (!CharacterMachine) return;

    float v32 = (PartyNumber > 0 && !PartyOpened) ? 50.0f : 0.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);

    float screenW = 640.0f;
    float v38 = screenW - v32 - 10.0f;
    float v37 = (float)a1;

    int v34 = 0;
    int v33 = 0;
    while (v33 < 816) {
        m_dwBackColor = 0x80000000u;

        // Anti-tamper inner block — skipped (CharacterMachine path).

        WORD v12 = *(WORD*)((BYTE*)CharacterMachine + v33 + 536);
        DWORD v13 = (DWORD)((BYTE*)CharacterMachine + v33 + 536);

        if (v12 != 0xFFFF) {
            // 2026-05-08: usar ItemAttribute_Base() para recuperar de
            // corrupción de DAT_07d78068 a 0x1.
            unsigned int attrBase = ItemAttribute_Base();
            if (attrBase == 0) { v33 += 68; continue; }
            ITEM_ATTRIBUTE* v19 = (ITEM_ATTRIBUTE*)(attrBase + (unsigned)v12 * 64);

            // Skip excluded item types: 135, 143, 416..419
            if (v12 != 135 && v12 != 143 && (v12 < 416 || v12 > 419)) {
                int level = (*(int*)(v13 + 4) >> 3) & 0xF;
                int v30 = (int)FUN_004c45c0((void*)v13, (int)v19, level);
                if (*(WORD*)v13 == 426) v30 = 200;
                BYTE v21 = *(BYTE*)(v13 + 26);

                if (v21 == 0) {
                    *(BYTE*)(v13 + 64) = 8;
                    m_dwTextColor = 0xFFFFFFFFu;
                    m_dwBackColor = 0x800A0AFFu;
                } else {
                    double v22 = (double)v21;
                    double v23 = (double)v30;
                    if (v22 <= v23 * 0.2) {
                        *(BYTE*)(v13 + 64) = 7;
                        m_dwTextColor = 0xFF0A0AFFu;
                    } else if (v22 <= v23 * 0.30000001) {
                        *(BYTE*)(v13 + 64) = 6;
                        m_dwTextColor = 0xFF009CFFu;
                    } else if (v22 <= v23 * 0.5) {
                        *(BYTE*)(v13 + 64) = 5;
                        m_dwTextColor = 0xFF00E4FFu;
                    } else {
                        // Item OK — skip render.
                        v33 += 68;
                        continue;
                    }
                }

                CHAR Buffer[100];
                // 2026-05-08: defensive — v19->Name is `char[30]` inline,
                // address = v19. Crash at addr 0x74F3DBCC param1=0x2A01 came
                // from wsprintfA reading bogus v19->Name. Validate v19 is in
                // heap range before reading.
                if ((uintptr_t)v19 < 0x100000 || (uintptr_t)v19 >= 0x80000000) {
                    v33 += 68; continue;   // bogus pointer — skip slot
                }
                // Defensive: copy Name to local buffer with a hard byte cap to
                // avoid reading past end of struct (Name field is 30 bytes).
                char nameLocal[31];
                __try {
                    int ni = 0;
                    for (; ni < 30; ++ni) {
                        char c = v19->Name[ni];
                        nameLocal[ni] = c;
                        if (c == '\0') break;
                    }
                    nameLocal[30] = '\0';
                    if (ni == 30) nameLocal[30] = '\0';
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    nameLocal[0] = '\0';
                }
                wsprintfA(Buffer, "%s (%d/%d)", nameLocal, v21, v30);
                byte_7E11D6E = 1;
                FUN_0047f6f0((int)v38, v34 + (int)v37, Buffer, 0, 0, 0);
                v34 += 12;
            }
        }
        v33 += 68;
    }

    m_dwBackColor = 0x80000000u;
}

// Old name kept for the call from Render_GameFrame.  IDA shows it takes
// `int a1` (the y-coordinate baseline) but our caller passed nothing —
// default to 0.
void Render_CharNameTags(void)
{
    RenderBrokenItem_(0);
}


// =============================================================================
// RenderExperience — sub_4BF990.
// Bottom-HUD XP bar.  Computes progress as
//   curExp = CharacterAttribute[+16]
//   maxExp = CharacterAttribute[+52]
//   prevExp = formula based on prev level (cubic)
//   ratio = (curExp - prevExp) / (maxExp - prevExp) * 10
// then renders a 198-px-wide bar at (221, 439) and the integer ratio (0..9)
// via RenderNumber2D at (425, 434).  Mouse hover over the bar shows a
// tooltip with the absolute exp values.
// =============================================================================
extern "C" void __cdecl RenderExperience_(void);
void RenderExperience_(void)
{
    // Anti-tamper inner block — skipped.

    void* v3 = CharacterAttribute;
    if (!v3) return;

    if (*(WORD*)((BYTE*)v3 + 0xE) == 0) return;

    int dwPriorExperience = 0;
    int wPriorLevel = (int)*(unsigned short*)((BYTE*)v3 + 0xE) - 1;
    if (wPriorLevel > 0) {
        dwPriorExperience = 10 * wPriorLevel * wPriorLevel * (wPriorLevel + 9);
        if (wPriorLevel > 255) {
            int curLvl = (int)*(unsigned short*)((BYTE*)v3 + 0xE);
            dwPriorExperience += 1000 * (curLvl - 256) * (curLvl - 256) * (curLvl - 256 + 9);
        }
    }

    int fNeedExp = *(int*)((BYTE*)v3 + 52) - dwPriorExperience;
    int fExp     = *(int*)((BYTE*)v3 + 16) - dwPriorExperience;
    if (fNeedExp <= 0) return;

    double fExpBarNum = (double)fExp / (double)fNeedExp * 10.0;
    int iExp = (int)fExpBarNum;
    if (iExp >= 9)      iExp = 9;
    else if (iExp < 0)  iExp = 0;

    glColor3f(0.92f, 0.80f, 0.34f);
    double width = (fExpBarNum - (double)iExp) * 198.0;
    if (width >= 198.0) FUN_005124c0(221.0f, 439.0f, 198.0f, 4.0f);
    else if (width >= 0.0) FUN_005124c0(221.0f, 439.0f, (float)width, 4.0f);
    else FUN_005124c0(221.0f, 439.0f, 0.0f, 4.0f);

    EnableAlphaTest(true);
    glColor3f(0.91f, 0.81f, 0.60f);
    RenderNumber2D(425.0f, 434.0f, iExp, 9.0f, 10.0f);
    FUN_00511600();
    glColor3f(1.0f, 1.0f, 1.0f);

    // Hover tooltip when mouse is over the bar (219..421, 439..445).
    double mx = (double)MouseX;
    double my = (double)MouseY;
    if (mx >= 219.0 && mx < 421.0 && my >= 439.0 && my < 445.0) {
        CHAR Buffer[100];
        wsprintfA(Buffer, GlobalText[357],
                  *(int*)((BYTE*)v3 + 16),
                  *(int*)((BYTE*)v3 + 52));
        RenderTipText(223, 424, Buffer);
    }

    // Anti-tamper outer ref-count decrement — skipped (same gate).
}

// AntiTamper_HashMaintain_B was the old (mislabeled) symbol for
// RenderExperience.  Re-route the existing call site.
void AntiTamper_HashMaintain_B(void) { RenderExperience_(); }
