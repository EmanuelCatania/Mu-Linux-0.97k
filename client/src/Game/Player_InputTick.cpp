#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"
#include <winsock2.h>
#include <string.h>

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

extern "C" void DbgLogPublic(const char*);
extern "C" int __cdecl GetScreenWidth(void);
extern "C" void Net_SendNpcTalkClose(void);
extern "C" BOOL ChaosBoxRequestClose(void);

static bool HUD_IsQuestPanelOpenRuntime(void);
static bool HUD_IsGoldenArcherPanelRuntime(void);
static bool HUD_IsInventoryFamilyActive(void);
static bool HUD_CloseNpcWindowsIfAny(void);
extern "C" int g_nGuildMemberCount;
static bool HUD_IsAnyRightPanelOpen(void);
static bool HUD_IsGuildCreationRuntime(void);
static bool HUD_IsGuildListRuntime(void);
static bool HUD_IsCharacterInfoRuntime(void);

// 2026-04-30: Bottom-bar HUD button hit-test.
// Los rectángulos salen de HUD_Pass5.cpp:264-296 (las mismas coordenadas que ya
// se usan para los tooltips de hover y el resaltado de "panel abierto"). Con un
// LButton release (no drag), toggles the corresponding panel flag.
//
// Gated by:
//   - sólo en estado in-game (g_GameState == 5)
//   - released-click signal (DAT_083a413c) so holding doesn't repeat
//   - limpiar DAT_083a413c después de consumirlo, para que otra parte de la UI no lo maneje dos veces
//
// En el binario 0.97k original esto estaba inline dentro del ruido anti-tamper de
// sub_004B82xx; acá lo reimplementamos limpio porque el camino click→toggle
// es lo que realmente hace funcionar los botones del HUD.
// 2026-05-04: MouseOnWindow (per IDA Player_InputTick:416,566 + sub_402F40:9):
// flag que se setea en cada frame si el mouse está sobre algún panel de UI abierto. Se usa para gatear
// el GroundClick / walker de movimiento, así clickear adentro de un panel no hace
// que el jugador camine hacia esa posición de pantalla.
// 2026-09-09: `MouseOnWindow` es UN SOLO global del binario, 0x07D78094 (=
// DAT_07d78094).  Estaba partido en dos: el port fiel de `sub_4E6550`
// (CheckInventory) escribia DAT_07d78094 con los seis rects de panel
// -- inventario, tienda, baul, ChaosMix, trade y ventana de evento -- y el gate
// de `Attack` (IDA L1330) leia este `g_MouseOnWindow`, una reimplementacion
// propia que NO cubre ninguno de esos seis.  Ahora es un alias del global real.
#define g_MouseOnWindow DAT_07d78094

// IDA `Attacking` — estado del auto-ataque: -1 = ninguno, 1 = ataque iniciado
// desde Player_InputTick (L942), 2 = desde Attack (0x49CBF0 L1323).
// Lo resetean a -1 InitGame, ReceiveTeleport, CheckGate y varios paths de
// Attack. Con -1 (el default) el head-tracking hacia el mouse queda ACTIVO,
// que es el comportamiento normal fuera de combate.
// IDA `Attacking` vive en 0x00559C58 = DAT_00559c58 (lo escriben InitGame,
// Player_InputTick L942 y Attack 0x49CBF0).  `g_Attacking` era una copia
// paralela que nadie escribia; se deja como alias de lectura para no romper
// declaraciones externas.
#define g_Attacking DAT_00559c58

// 2026-07-20: el ChatListBox publica su propio hit-test acá (definido en
// src/UI/ChatListBox.cpp).  Su tick (slot 5 → slot 7) corre ANTES que esta
// función dentro del mismo frame (Game_CharSelectTick líneas 217 y 298), así
// que el latch está fresco.  Sin esto, clickear dentro del recuadro del chat
// mandaba a caminar al personaje: el slot 7 escribía un `MouseOnWindow` local
// de ChatListBox.cpp que no leía nadie.
extern "C" int g_ChatLB_MouseOnWindow;

// Resetea y puebla MouseOnWindow al inicio del frame. La llama FUN_004acef0.
static void MouseOnWindow_Update(void)
{
    // Sin reset: el valor del frame lo fija `Game_CharSelectTick` (IDA L298,
    // `MouseOnWindow = MouseY > 431`) y a partir de ahi los productores solo
    // SUMAN -- este, `sub_4E6550` y el widget de chat.  El reset que habia aca
    // borraba el aporte de los paneles si corria despues de CheckInventory.

    int mx = (int)DAT_083a427c;
    int my = (int)DAT_083a4278;

    if (g_ChatLB_MouseOnWindow) { g_MouseOnWindow = 1; return; }

    // El selector local de Chaos (ErrorMessage 143) es un modal de 213x210;
    // el original bloquea el click de mundo mientras está presente.  Sin este
    // gate, sus botones pueden atravesar hacia los paneles o el mapa.
    if (DAT_083a7c24 == 143 &&
        mx >= 213 && mx < 426 && my >= 120 && my < 330) {
        g_MouseOnWindow = 1;
        return;
    }

    // IDA: RenderErrorMessage dibuja los modales 126/152 desde (213,60)
    // hasta (426,124), incluidos el campo de Personal Code y sus botones.
    // Ese diálogo es modal: su click no puede seguir al recorrido de mundo.
    if ((DAT_083a7c24 == 126 || DAT_083a7c24 == 152) &&
        mx >= 213 && mx < 426 && my >= 60 && my < 124) {
        g_MouseOnWindow = 1;
        return;
    }

    // Bottom HUD (y >= 432) — per IDA Game_CharSelectTick:298.
    if (my >= 432) { g_MouseOnWindow = 1; return; }

    // Top quick buttons / small HUD icons rendered by Render_QuickButtons.
    if (HUD_IsGuildCreationRuntime()) {
        // IDA: estos rectángulos comparten el origen guardado por
        // RenderGuildCreation. El port ya no usa Inventory[32] para ese
        // scratch, porque dicho slot pertenece al pool de tienda.
        int btnX = (int)((float)g_GuildCreatorScratchX + _DAT_005524fc);
        int btnY = (int)((float)g_GuildCreatorScratchY + _DAT_00552ca4);
        if (mx >= btnX && mx < btnX + 70 && my >= btnY && my < btnY + 21) {
            g_MouseOnWindow = 1; return;
        }
        // 2026-08-26: mismo fix que el render — IDA `sub_4E4760` L446 pone el
        // segundo boton en [origin+100, origin+170), no en +20+100.
        btnX = (int)((float)g_GuildCreatorScratchX + 100.0f);
        if (mx >= btnX && mx < btnX + 70 && my >= btnY && my < btnY + 21) {
            g_MouseOnWindow = 1; return;
        }
    }

    if (HUD_IsGuildListRuntime()) {
        int iconX = (int)((float)DAT_07e91788 + _DAT_00552464);
        int iconY = (int)((float)DAT_07e91784 + _DAT_0055246c);
        if (mx >= iconX && mx < iconX + 24 && my >= iconY && my < iconY + 24) {
            g_MouseOnWindow = 1; return;
        }
    }

    if (HUD_IsCharacterInfoRuntime()) {
        int iconX = (int)DAT_07ea982c + 25;
        int iconY = (int)DAT_07ea9830 + 395;
        if (mx >= iconX && mx < iconX + 24 && my >= iconY && my < iconY + 24) {
            g_MouseOnWindow = 1; return;
        }
    }

    // 2026-05-05: Skill bar expanded list (cells at y=370..411 cuando
    // DAT_07db870c=1, el user expandió el menú con click en el icono central).
    // Sin este check, click en una skill cell del menú expandido cae en zona
    // libre del world → Player_InputTick lo procesa como move click → hero
    // camina al lugar del cell además de cambiar la skill.
    //
    // 2026-05-05 (followup): Chat_InputTick (corre ANTES) resetea DAT_07db870c
    // a 0 cuando user clickea una cell. Entonces cuando llegamos acá, el flag
    // ya cambió. Usamos un latch del frame anterior para que el click "tail"
    // siga viendo el menú como abierto.
    static char s_lastMenuOpen = 0;
    char menuOpen = (DAT_07db870c != '\0') ? (char)1 : (char)0;
    if ((menuOpen || s_lastMenuOpen) && my >= 370 && my < 412) {
        // Cell range x es variable por slot count, pero conservadoramente el
        // skill list expandido vive entre x=180..460 (centrado en 320 con ~10
        // slots × 32 px).
        if (mx >= 180 && mx < 460) {
            g_MouseOnWindow = 1;
            s_lastMenuOpen = menuOpen;
            return;
        }
    }
    s_lastMenuOpen = menuOpen;

    // Captura de UI consciente de los paneles. El cliente original usa el mismo helper
    // de "ancho de pantalla" que el HUD para decidir dónde empieza el área de click libre al mundo.
    // That collapses:
    //   inventory only                   -> 450..640
    //   character/party/guild/guild ui  -> 450..640
    //   inventory + side panel pair     -> 260..640
    // en vez de mantener acá una lista paralela de rectángulos.
    if (HUD_IsAnyRightPanelOpen()) {
        int panelStartX = GetScreenWidth();
        if (panelStartX < 640) {
            if (mx >= panelStartX && mx < 640 && my >= 0 && my < 433) {
                g_MouseOnWindow = 1; return;
            }
        }
    }
}

// Render/Chat original: both the bottom HUD icon and P execute this same
// Party toggle.  Opening discards only the displayed count and immediately
// asks the server for an authoritative 0x42; it never clears Party rows or
// changes membership locally.
static void Party_ToggleAndRefresh(void)
{
    GuildOpened = 0;
    GuildCreatorOpened = 0;
    if (PartyOpened) {
        PartyOpened = 0;
        FUN_00404bc0(0x19, 0, 0);
        FUN_00404bc0(0x1c, 0, 0);
        return;
    }

    PartyNumber = 0;
    // IDA Chat_InputTick L5639-5807: abrir el party cierra inventario y personaje.
    DAT_07eaa117 = 0;   // InventoryOpened
    DAT_07eaa116 = 0;   // CharacterOpened
    const BYTE partyListPkt[3] = { 0xC1, 0x03, 0x42 };
    Net_SendC1Packet(partyListPkt, sizeof(partyListPkt));
    PartyOpened = 1;
}

static bool HUD_CloseNpcWindowsIfAny(void)
{
    if (DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a || DAT_07eaa11b || DAT_07eaa128) {
        const bool wasChaos = (DAT_07eaa11a != 0);
        if (wasChaos) {
            // 0x87 ACK performs the close; never expose another NPC panel
            // while the Chaos interface remains server-active.
            ChaosBoxRequestClose();
            return false;
        }
        extern void __cdecl CloseInventoryRelatedWindows(void);
        CloseInventoryRelatedWindows();
        Net_SendNpcTalkClose();
        DbgLogPublic("HKT CLOSE-NPC (C/G/P panel)");
    }
    return true;
}


// Cierra la familia de ventanas de inventario/NPC.  Es la rama de cierre que
// comparten la tecla I/V y el boton de la barra inferior (IDA Chat_InputTick
// 0x4B14F0 L2078-2260: TradeOpened -> cancelar trade; WarehouseOpened -> close
// 0x82; ChaosMixOpened -> close 0x87; si no, InventoryOpened = 0).
static void HUD_CloseInventoryFamilyFromUI(void)
{
    if (DAT_07eaa11a) {              // ChaosMixOpened: el ACK del 0x87 cierra
        ChaosBoxRequestClose();
        return;
    }
    const bool hadNpcWindow = (DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11b ||
                               DAT_07eaa128);
    extern void __cdecl CloseInventoryRelatedWindows(void);
    CloseInventoryRelatedWindows();
    DAT_07eaa117 = 0;                // InventoryOpened
    if (hadNpcWindow) {
        Net_SendNpcTalkClose();
    }
}

// Botones de la barra inferior.  En el binario esto vive dentro de
// `Chat_InputTick` (0x4B14F0); aca corre desde Player_InputTick, que se ejecuta
// antes de la logica de click al mundo.
//
// 2026-09-04 -- reescrito contra IDA.  La version anterior era una
// reimplementacion ("clean reimplementation ... covers the same observable
// behavior") con tres desviaciones:
//   * el boton de inventario gateaba con `HUD_IsInventoryFamilyActive()`, que
//     incluye CharacterOpened -> con el panel de personaje abierto, clickear
//     inventario lo CERRABA en vez de abrirlo.  En el original los dos paneles
//     conviven (por eso GetScreenWidth devuelve 260 justo para esa combinacion).
//   * usaba `DAT_083a413c` (latch de click soltado) con deteccion de flanco
//     propia; IDA usa `MouseLButtonPush` (DAT_083a4124) y lo CONSUME poniendolo
//     en 0, que es lo que evita el auto-repeat.
//   * le faltaban los sonidos 25/28 al cerrar, el `PartyOpened = 0` del boton de
//     guild, el `GuildOpened = 0; PartyOpened = 0` al abrir inventario, y el
//     gate de entrada que apaga toda la fila mientras hay un modal abierto.
//
// Rects (IDA L1130, L1455, L1775, L2078):
//   guild      (582..634, 459..477)
//   party      (348..372, 452..476)
//   personaje  (379..403, 452..476)
//   inventario (410..434, 452..476)
extern "C" void HUD_BottomBarButtons_HitTest(void);
void HUD_BottomBarButtons_HitTest(void)
{
    if (DAT_005615c0 != 5) return;          // g_GameState: only in-game
    if (!IsClickPushed()) return;

    // Gate de IDA L1116-1128: con un modal o el creador de guild abiertos, la
    // fila entera de botones no responde.  (`GuildInputEnable` del original no
    // existe en este arbol; GuildCreatorOpened cubre el mismo estado.)
    if (DAT_07eaa11b ||                      // TradeOpened
        DAT_07eaa124 ||                      // GuildCreatorOpened
        DAT_083a7c24 == 126 ||               // ErrorMessage: expulsar del guild
        DAT_083a7c24 == 152 ||
        _g_bEventChipDialogEnable ||
        DAT_07eaa130 ||                      // g_bServerDivisionEnable
        HUD_IsQuestPanelOpenRuntime())
        return;

    const int mx = (int)DAT_083a427c;       // 640-space mouse X
    const int my = (int)DAT_083a4278;       // 480-space mouse Y

    // ── Guild ────────────────────────────────────────────────────────────────
    if (mx >= 582 && mx < 634 && my >= 459 && my < 477) {
        DAT_083a4124 = 0;                    // MouseLButtonPush = 0
        DAT_07eaa115 = 0;                    // PartyOpened = 0
        if (DAT_07eaa114) {                  // GuildOpened
            DAT_07eaa114 = 0;
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
        } else {
            if (!HUD_CloseNpcWindowsIfAny()) return;
            // 0x52 pide Encrypt=0 en HackPacketCheck.txt -> frame C1 plano.
            const BYTE guildListPkt[3] = { 0xC1, 0x03, 0x52 };
            Net_SendC1Packet(guildListPkt, sizeof(guildListPkt));
            g_nGuildMemberCount = -1;
            DAT_07eaa114 = 1;
        }
        return;
    }

    // ── Party ────────────────────────────────────────────────────────────────
    if (mx >= 348 && mx < 372 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        if (!DAT_07eaa115 && !HUD_CloseNpcWindowsIfAny()) return;
        Party_ToggleAndRefresh();            // ya hace GuildOpened=0 + sonidos
        return;
    }

    // ── Personaje ────────────────────────────────────────────────────────────
    // IDA no toca ningun otro flag aca: Character convive con Inventory.
    if (mx >= 379 && mx < 403 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        if (DAT_07eaa116) {                  // CharacterOpened
            DAT_07eaa116 = 0;
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
        } else if (HUD_CloseNpcWindowsIfAny()) {
            DAT_07eaa116 = 1;
        }
        return;
    }

    // ── Inventario ───────────────────────────────────────────────────────────
    if (mx >= 410 && mx < 434 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        if (!DAT_07eaa117) {                 // InventoryOpened
            DAT_07eaa117 = 1;
            DAT_07eaa114 = 0;                // GuildOpened = 0
            DAT_07eaa115 = 0;                // PartyOpened = 0
        } else {
            HUD_CloseInventoryFamilyFromUI();
        }
        return;
    }
}

// 2026-04-30: hotkeys de UI por frame para el HUD in-game (C/V/I).
// En el binario 0.97k original, los botones de la barra inferior (y estos
// atajos de teclado) invierten los flags de un byte DAT_07eaa11x que gatean cada
// bloque de render del HUD. La rutina dedicada que hacía esto estaba enterrada en
// 0x004B8xxx anti-tamper hash-table noise; this clean reimplementation
// cubre el mismo comportamiento observable para el usuario.
//
// Se suprime mientras haya algún modo de entrada de texto activo (chat, IME, texto de login)
// para que tipear letras en el chat no invierta paneles sin querer.
// 2026-05-04: defensive guard — DAT_07e11d70/d71 (ChatMode/IME) get corrupted
// a 0xFF (-1 con signo) por ALGÚN código, poco después de abrir el inventario. El
// writer is hard to find via grep (no literal -1 store).  As a defense, clamp
// cualquier valor que no sea {0,1} a 0 al inicio de cada llamada a Player_InputTick Y logueamos
// las primeras veces que vemos la corrupción, para poder encontrar la fuente.
extern char g_PadBeforeChatMode[64];
extern char g_PadAfterChatMode[64];
static void ClampChatModeIME(const char* tag)
{
    static int s_logs = 0;
    BYTE chat = (BYTE)DAT_07e11d70;
    BYTE ime  = (BYTE)DAT_07e11d71;
    if (chat > 1 || ime > 1) {
        if (s_logs < 8) {
            s_logs++;
            // Muestrea el primer/último byte de cada canario para saber por qué lado desbordó.
            char b[260];
            wsprintfA(b,
                "ChatMode/IME CORRUPT[%s]: chat=%02X ime=%02X  "
                "padBefore[0]=%02X padBefore[63]=%02X  padAfter[0]=%02X padAfter[63]=%02X",
                tag, chat, ime,
                (BYTE)g_PadBeforeChatMode[0], (BYTE)g_PadBeforeChatMode[63],
                (BYTE)g_PadAfterChatMode[0],  (BYTE)g_PadAfterChatMode[63]);
            DbgLogPublic(b);
        }
        if (chat > 1) DAT_07e11d70 = 0;
        if (ime  > 1) DAT_07e11d71 = 0;
    }

    // 2026-07-27: detector de la transición 0→1 de ChatMode. El clamp de arriba
    // sólo atrapa valores >1, pero un `1` espurio es un valor VÁLIDO ("chat on")
    // → abre la caja de chat/whisper con un carácter suelto (el bug del "whisper
    // con la letra l" que aparece cada tanto, típicamente tras abrir tiendas).
    // Logueamos el tag del punto del frame donde se encendió para ubicar al
    // escritor real.
    {
        static BYTE s_prevChat = 0;
        BYTE now = (BYTE)DAT_07e11d70;
        if (now == 1 && s_prevChat == 0) {
            static int s_onLogs = 0;
            if (s_onLogs < 12) {
                s_onLogs++;
                char b[160];
                wsprintfA(b, "CHATMODE ON [%s]  ime=%02X shop=%d inv=%d",
                          tag, (BYTE)DAT_07e11d71, (int)DAT_07eaa118,
                          (int)DAT_07eaa117);
                DbgLogPublic(b);
            }
        }
        s_prevChat = now;
    }
}

// Alias público para que otras unidades de traducción puedan llamar al bisect.
extern "C" void Bisect_ChatMode(const char* tag) { ClampChatModeIME(tag); }

static bool HUD_IsQuestPanelOpenRuntime(void)
{
    return (g_csQuest != 0) &&
           (*(char*)((uintptr_t)g_csQuest + 0x1c87f) != 0);
}

static bool HUD_IsGoldenArcherPanelRuntime(void)
{
    return (DAT_07eaa128 != 0 && DAT_07eaa128 != 3);
}

static bool HUD_IsGuildCreationRuntime(void)
{
    return DAT_07eaa124 != 0;
}

static bool HUD_IsGuildListRuntime(void)
{
    return DAT_07eaa114 != 0;
}

static bool HUD_IsCharacterInfoRuntime(void)
{
    return DAT_07eaa116 != 0;
}

static bool HUD_IsInventoryFamilyActive(void)
{
    return (DAT_07eaa117 != 0) ||   // InventoryOpened
           (DAT_07eaa116 != 0) ||   // CharacterOpened
           (DAT_07eaa118 != 0) ||   // ShopOpened
           (DAT_07eaa119 != 0) ||   // WarehouseOpened
           (DAT_07eaa11a != 0) ||   // ChaosMixOpened
           (DAT_07eaa11b != 0) ||   // TradeOpened
           (DAT_07eaa11c != 0) ||   // EventWindowOpened
           (DAT_07eaa124 != 0) ||   // GuildCreatorOpened
           HUD_IsGoldenArcherPanelRuntime() ||
           (DAT_07eaa130 != 0) ||   // ServerDivisionOpened
           HUD_IsQuestPanelOpenRuntime();
}

static bool HUD_IsAnyRightPanelOpen(void)
{
    return HUD_IsInventoryFamilyActive() ||
           (DAT_07eaa115 != 0) ||   // PartyOpened
           (DAT_07eaa114 != 0);     // GuildOpened
}

static void HUD_HotkeyTick(void)
{
    ClampChatModeIME("HKT_enter");
    if (DAT_07e11d70 != '\0') return;  // g_ChatMode
    if (DAT_00559c84 != '\0') return;  // g_TextMode (login / dialog text)
    if (DAT_07e11d71 != '\0') return;  // g_IME_Mode
    // 2026-08-21: con la ventana de quest abierta el original NO deja tocar los
    // hotkeys de panel (IDA Chat_InputTick L3978-3985 corta con
    // `*(BYTE*)(g_csQuest + 116863) == 1`).  Sin este gate se podia abrir el
    // inventario encima del panel de quest — los dos se dibujan en x=450.
    if (HUD_IsQuestPanelOpenRuntime()) return;

    // Cada llamada a Key_IsJustPressed tiene efectos secundarios de detección por flanco, así que
    // capturamos los resultados antes de combinarlos (V o I invierten el inventario).
    int kC = Input_IsKeyJustPressed(0x43); // 'C'  Character info
    int kV = Input_IsKeyJustPressed(0x56); // 'V'  Inventory (alt)
    int kI = Input_IsKeyJustPressed(0x49); // 'I'  Inventory
    int kG = Input_IsKeyJustPressed(0x47); // 'G'  Guild
    int kP = Input_IsKeyJustPressed(0x50); // 'P'  Party

    // Toggle pattern matches IDA Chat_InputTick (sub_4B14F0):
    //   tecla C → si CharacterOpened: 0; si no: 1 (con el paquete de tab de clase).
    //   tecla G → si GuildOpened: 0; si no: 1 (con limpieza de party + paquete).
    //   tecla P → si PartyOpened: 0; si no: 1 (con limpieza de guild + paquete).
    //   kV/kI    → if InventoryOpened: 0 (close inventory + clear shop/etc);
    //              else: 1 (open inventory).
    // Abrir G también pide la lista autoritativa de miembros del guild. MuEmu
    // maneja C1:03:52 en CGGuildListRecv; el resultado es el frame C2:52
    // decoded by Net_Process.
    // 2026-07-27 FIX (tienda "vacía" al abrir Character/Party/Guild): el panel
    // de inventario se mueve a x=260 cuando CharacterOpened||PartyOpened
    // (HUD_Pass6:440), que es EXACTAMENTE donde se dibuja el panel de la tienda
    // (dword_7EAA0C8=260) → el inventario quedaba encima de la tienda y parecía
    // vacía (en realidad los datos estaban intactos: diag SHOPREND occ=56).
    // En MU los paneles izquierdos (Character / Shop / Warehouse) son mutuamente
    // excluyentes: abrir C/G/P cierra la ventana del NPC (y avisa al server con
    // el close 0x31, como ya hacen I/V y Escape).
    // IDA Chat_InputTick L4921-5826: al CERRAR con G/P/C suenan 25 y 28 (P ya
    // los tiene en Party_ToggleAndRefresh); G apaga el party antes de mirar
    // su propio flag.  2026-09-14: faltaban los sonidos y el PartyOpened = 0.
    if (kC) {
        if (DAT_07eaa116) {
            DAT_07eaa116 = 0;
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
        }
        else if (HUD_CloseNpcWindowsIfAny()) { DAT_07eaa116 = 1; }
    }
    if (kG) {
        DAT_07eaa115 = 0;   // PartyOpened (IDA L4928)
        if (DAT_07eaa114 || DAT_07eaa124) {
            DAT_07eaa114 = 0;
            DAT_07eaa124 = 0;
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
        }
        else {
            if (!HUD_CloseNpcWindowsIfAny()) return;
            DAT_07eaa114 = 1;
            DAT_07eaa115 = 0; // close Party
            // IDA Chat_InputTick L5275-5277: abrir el guild cierra inventario y
            // personaje (se dibujan en la misma franja x=450).
            DAT_07eaa117 = 0; // InventoryOpened
            DAT_07eaa116 = 0; // CharacterOpened
            // 2026-08-15 BUG-FIX (abrir el panel de guild con G desconectaba):
            // el opcode 0x52 pide Encrypt=0 en HackPacketCheck.txt, o sea frame
            // C1 plano. Enviarlo como C3 (Net_SendSmallPacket) hace que el
            // server responda "Packet encryption error" y cierre la sesión.
            const BYTE guildListPkt[3] = { 0xC1, 0x03, 0x52 };
            Net_SendC1Packet(guildListPkt, sizeof(guildListPkt));
        }
    }
    if (kP) {
        if (!PartyOpened && !HUD_CloseNpcWindowsIfAny()) return;
        Party_ToggleAndRefresh();
    }
    if (kV || kI) {
        // 2026-07-27 FIX: el gate era HUD_IsInventoryFamilyActive(), que incluye
        // CharacterOpened/Party/Guild → con el panel de Character abierto, tocar V
        // caía en la rama "cerrar todo" y cerraba TODOS los menús en vez de
        // togglear el inventario. Per IDA (Chat_InputTick sección 15) la tecla
        // I/V togglea InventoryOpened; al cerrar arrastra las ventanas de NPC.
        if (DAT_07eaa117 || DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a ||
            DAT_07eaa11b || DAT_07eaa128) {
            // 2026-07-27 FIX: con la tienda abierta, apretar I/V cerraba solo
            // InventoryOpened y dejaba la tienda abierta (y el server con
            // Interface.use=1 → no dejaba abrir otra). Ahora cierra toda la
            // familia de ventanas de NPC y avisa al server con el close 0x31,
            // igual que Escape / click-para-mover.
            // (NO tocar CharacterOpened: I/V sólo maneja el inventario y las
            //  ventanas de NPC; el panel de Character lo togglea la tecla C.)
            HUD_CloseInventoryFamilyFromUI();
        } else {
            // IDA L6323-6327: al abrir apaga guild y party y suenan 25 y 28.
            DAT_07eaa117 = 1;
            DAT_07eaa114 = 0;   // GuildOpened
            DAT_07eaa115 = 0;   // PartyOpened
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
        }
    }
}

// IDA: FUN_004acef0 — Player_InputTick (0x004acef0, 1688 lines)
//
// Procesador de input del jugador por frame. Se llama desde el camino de render del HUD/UI en cada frame.
// Responsibilities:
//   1. Cooldown gate (DAT_07e11d1c must be <= 0x1e)
//   2. Second-password auto-fill from hover entity name
//   3. Camera update + facing angle packet [0xC1][0x18][0x66]
//   4. Sub-tick via FUN_004ac140
//   5. Movement debounce (DAT_07e11d28 >= DAT_00559bec and !DAT_07e11dc0)
//   6. Animation state exit: swimming, normal walk, cancel
//   7. Click sobre mob/jugador (SelectedCharacter): pathfind + Combat_SendMovePathPacket
//   8. Click sobre NPC (SelectedNpc): pathfind alternativo
//   9. Click sobre objeto especial (SelectedOperate): pathfind + lookup de entity_type
//  10. Click en suelo (ray cast FUN_004f9ac0 + FUN_004f8480): chequeo de terreno + pathfind
//  11. Actualización de estado de entidad: Combat_DispatchHeroSkillAttack
//  12. Atributo de terreno bajo el cursor → DAT_07e118e8
//
// Los bloques de ofuscación por HashTable repartidos por la función (~70 % del código) se omiten
// per project policy (see CLAUDE.md §Anti-tamper).
//
// Key globals:
//   DAT_07abf5d8          — local player entity ptr
//   DAT_07abf5d0          — entity array base (stride 0x394)
//   DAT_07cf1ffc          — g_CharData (XOR-encoded char-select data block)
//   DAT_05826e08          — frame counter (float-compatible tick)
//   _DAT_00552890         — speed scale (dt multiplier)
//   _DAT_00552b6c         — max facing dt threshold
//   _DAT_00552928         — min entity speed for facing packet
//   DAT_00559bec          — movement debounce threshold
//   DAT_07e11d28          — movement debounce counter
//   DAT_07e11db8          — contador de pasos (tiene que ser > 0x27 para el facing)
//   SelectedCharacter          — hover mob/player entity index
//   SelectedNpc          — hover NPC entity index
//   SelectedOperate          — hover special-object index
//   SelectedItem          — hover ground item index
//   DAT_083a2378          — special-object entity pointer table (stride 3*int)
//
// Packet formats (all XOR-encoded before send):
//   Facing:       [0xC1][0x07][0x0F] + encoded direction byte
//   Walk/swim:    [0xC1][0x11] + grid_x,grid_y

// Helper: manda el buffer por el socket, con fallback a la cola de WSAEWOULDBLOCK
// BUG-FIX 2026-04-29: server log mostró `[SocketManager] Protocol header
// error (Header: 41)` — 0x41 = lo que el server-side decrypt produce cuando
// recibe nuestros bytes plain como si fueran cipher. Causa: este helper
// NUNCA llamaba MuEmu::EncryptSend. Server con ENCRYPT_STATE=1 decripta todo
// el stream → packets que mandamos plain salen como garbage → kick.
// El primer F3/03 (CharSelect) funcionaba porque va por OTRO path con
// EncryptSend (Game_CharSelectTick). Movimiento y swim packets desde aquí
// rompían la sesión.
static void SendPacket(const char *buf, unsigned int len)
{
    if (DAT_055ca168 == 0xffffffff)
        return;

    // Encriptar antes de enviar — capa MuEmu byte-XOR (HackCheck.cpp).
    BYTE wireBuf[0x800];
    if ((int)len > (int)sizeof(wireBuf)) return;
    memcpy(wireBuf, buf, len);
    MuEmu::EncryptSend(wireBuf, (int)len);
    buf = (const char*)wireBuf;

    int sent = 0;
    unsigned int remaining = len;
    while ((int)remaining > 0) {
        int r = send(DAT_055ca168, buf + sent, remaining, 0);
        if (r == -1) {
            int err = WSAGetLastError();
            if (err == 0x2733 /*WSAEWOULDBLOCK*/) {
                if ((int)(DAT_055cc16c + len) < 0x2001) {
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, buf, len);
                    DAT_055cc16c += len;
                } else {
                    Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                }
            } else {
                Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
            }
            break;
        }
        if (r == 0) break;
        if (DAT_055ce174 != 0)
            FUN_0043de60();
        sent += r;
        remaining -= r;
    }
}

// IDA: FUN_004acef0
void __cdecl Player_ProcessInput(void)
{
    // 2026-04-30: el procesamiento de hotkeys de UI va PRIMERO, para que los toggles funcionen incluso
    // cuando los gates de abajo saldrían temprano (p.ej. durante un cooldown).
    HUD_HotkeyTick();
    // 2026-05-04: poblar el flag MouseOnWindow (per IDA) ANTES de la lógica de
    // GroundClick, así clickear adentro de un panel no hace caminar al jugador.
    MouseOnWindow_Update();
    // 2026-05-20: in-game unificamos el latch viejo de hover con la captura de UI
    // capture result. Inventory / character / chat render paths still poke
    // DAT_07d78094 directo y lo puede dejar pegado, lo que bloquea el movimiento
    // even when the mouse is no longer over a panel. For world input, only
    // sólo debería importar la captura de UI del frame actual.
    // 2026-09-09: aca habia `DAT_07d78094 = (g_MouseOnWindow != 0) ? 1 : 0;`,
    // que in-game PISABA el flag que ya habia puesto CheckInventory por los
    // paneles.  Sintoma: con la Chaos Machine abierta, mover items disparaba el
    // camino de ataque -> sin mana -> busca pocion -> cartel GlobalText[474]
    // ("Los items no pueden ser utilizados mientras usas el baul o durante
    // trade").  Ahora los dos nombres son la misma memoria y no hay que copiar.
    // 2026-09-04: los botones de la barra inferior se atienden desde
    // `Chat_InputTick` (0x4B14F0), que es donde los tiene el binario.

    // ── Guard: entity visibility / renderable flag ─────────────────────────────
    if (*(char*)((int)DAT_07abf5d8 + 0x2fd) != '\0')
        return;

    // ── LoadingWorld cooldown ────────────────────────────────────────────────
    // IDA 0.97K Player_InputTick @ 0x004ACF9B..0x004AD04D: once per input
    // tick, LoadingWorld is decremented while positive, then the resulting
    // value is compared with 30. CheckGate @ 0x004AC140 subsequently requires
    // this same global to be exactly zero.  Omitting the decrement left the
    // value written by ReceiveTeleport (30) permanently nonzero, preventing
    // every subsequent automatic map gate from ever sending C3:06:1C.
    if (DAT_07e11d1c > 0)
        --DAT_07e11d1c;

    if (DAT_07e11d1c > 0x1e)
        return;

    // ── Second-password auto-fill from hover entity name ──────────────────────
    // Si el modo de segunda contraseña está activo, la entidad bajo el mouse está muerta y el índice de hover es válido:
    //   copy entity name (entity+0x1c1) to password buffer DAT_07db8810,
    //   después poner en cero y volver a copiar vía el buffer intermedio DAT_07e113e4.
    if (DAT_00559c84 != '\0'
        && *(char*)((int)DAT_07abf5d8 + 0x34e) != '\0'
        && SelectedCharacter != -1
        && DAT_083a42d0 != '\0')
    {
        unsigned char *hoverEntity = (unsigned char*)(DAT_07abf5d0 + SelectedCharacter * 0x394);
        unsigned char *nameSrc     = hoverEntity + 0x1c1;

        // Copy name into password buffer
        memcpy(DAT_07db8810, nameSrc, 0x40);

        // El slot del historial es dword_559CC4 (0..4), NO SelectedCharacter.
        // IDA Player_InputTick L344-349:
        //     v11 = dword_559CC4;
        //     v12 = &byte_7E113E4[256 * v11];
        // 2026-08-21: el port indexaba con SelectedCharacter (SelectedCharacter, que
        // llega hasta 399) sobre una tabla de 5 entradas → escribía 0x40 bytes
        // hasta ~100 KB fuera del global cada vez que se hacía click derecho
        // sobre un jugador con el chat abierto.
        int histSlot = (int)DAT_00559cc4;
        if (histSlot < 0 || histSlot > 4) histSlot = 0;
        memcpy((void*)(DAT_07e113e4 + histSlot * 0x100), nameSrc, 0x40);

        // Pone en cero el buffer de contraseña y después copia de vuelta desde el slot preparado
        memset(DAT_07db8810, 0, 0x40 * sizeof(DWORD));
        memcpy(DAT_07db8810, (void*)(DAT_07e113e4 + histSlot * 0x100), 0x40);

        // Setea el largo de la contraseña y dispara el BGM 0x19
        DAT_07d780ac = (DWORD)strlen((char*)DAT_07db8810);
        FUN_00404bc0(0x19, 0, 0);
    }

    // ── Head-tracking hacia el mouse (IDA Player_InputTick L353-385) ──────
    //
    // Layout de la entidad, confirmado con la struct de MU 5.2 (`w_ObjectInfo.h`):
    //     +28 Angle[3]      +40 HeadAngle[3]      +52 HeadTargetAngle[3]
    //
    // La cadena ya estaba entera salvo el PRODUCTOR:
    //   · `MoveCharacterVisual` (0x4520C0) interpola cada frame
    //       HeadAngle[j] = TurnAngle2(HeadAngle[j], HeadTargetAngle[j],
    //                                 FarAngle(HeadAngle[j], HeadTargetAngle[j]) * 0.2)
    //   · `BMD_Animation` (0x440060) rota el hueso de la cabeza con
    //     HeadAngle[0]/[1] (grados → radianes, * 0.017453294).
    // Nadie escribía HeadTargetAngle del héroe, así que quedaba en 0 y el pj
    // miraba siempre al frente de su cuerpo.
    //
    // El port anterior había degradado justo las dos líneas del cálculo:
    // `FUN_004cb520()` — que es **GetScreenWidth**, no "frame time" — con el
    // resultado descartado, y `FUN_0043e050(0,0,0,0)` (CreateAngle) con ceros.
    bool bHeadTrackActive = false;
    float fHalfScreenW = 320.0f;
    {
        unsigned char* ent = (unsigned char*)DAT_07abf5d8;
        if (ent) {
            fHalfScreenW = (float)(FUN_004cb520() / 2);      // GetScreenWidth() / 2
            const float mouseX = (float)(int)DAT_083a427c;
            const float mouseY = (float)(int)DAT_083a4278;

            // Ángulo del mouse respecto del centro del viewport, llevado al marco
            // del cuerpo y clampeado a [120, 240]: la cabeza sólo gira ~±60°.
            const float angMouse = FUN_0043e050(fHalfScreenW, 180.0f, mouseX, mouseY);
            int v16 = (int)((int)(angMouse + *(float*)(ent + 36)) + 315) % 360;
            if (v16 >= 120) { if (v16 > 240) v16 = 240; }
            else            { v16 = 120; }

            *(DWORD*)(ent + 60) = 0;                          // HeadTargetAngle[2]

            // IDA L374: el tracking se apaga durante el auto-ataque y con la
            // animación de muerte (62).
            if ((DAT_07e11e18 == 0 || g_Attacking == -1 || DAT_0055a7ac == 6)
                && ent[261] != 62)
            {
                bHeadTrackActive = true;
                *(float*)(ent + 52) = (float)((v16 + 180) % 360);   // yaw
                *(float*)(ent + 56) = (float)(180 - (int)DAT_083a4278) * 0.050000001f; // pitch
            }
            else
            {
                *(DWORD*)(ent + 52) = 0;
                *(DWORD*)(ent + 56) = 0;
            }
        }
    }

    // ── Sub-tick (handles animation transitions etc.) ────────────────────────
    FUN_004ac140();

    // ── WALKER (corre cada tick, INDEPENDIENTE de gates) ─────────────────────
    // BUG-FIX 2026-05-01: el walker estaba adentro del gate `bec <= d28`,
    // pero `bec` se setea a `wpCount*3+4` cada vez que Combat_SendMovePathPacket envía un
    // packet de movimiento. Para wpCount=5 → bec=19 ticks (760 ms). Eso
    // throttleaba el walker a 1.3 calls/sec — el hero "se movía por zonas".
    //
    // 2026-05-05: además debe ir ARRIBA del gate `DAT_07d78094` (que se setea
    // cuando user hover sobre skill bar). Sin esto, hover sobre skill detenía
    // el walker mid-path → hero parado en el lugar pero anim de walk seguía
    // corriendo. El walker debe ejecutarse SIEMPRE; solo el envío de packets
    // y el procesamiento de NEW clicks debe gated.
    {
        unsigned char *ent = (unsigned char*)DAT_07abf5d8;
        if (*(unsigned char*)(ent + 0x78) & 0x20) {
            FUN_004430c0((int)ent);
        } else {
            // BUG-FIX 2026-05-03: el chequeo isIdle DEBE ir ANTES de FUN_00443930.
            // Si está idle (sin path activo), NO queremos que FUN_00443930 setee
            // walk action (action 0x0d) cada frame. Antes el orden era:
            //   FUN_00443930 (set walk) → check isIdle → si idle: set 1 (idle)
            // → action cambia walk↔idle cada frame → frame counter reset cada
            // tick → render frozen en frame 0.
            // IDA gatea todo este walker con Hero+748. Los contadores de waypoint
            // son internos a MovePath y no hay que usarlos para enganchar la posición
            // de mundo mientras el runner de camino está inactivo.
            bool isIdle = (ent[748] == 0);
            if (!isIdle) {
                FUN_00443930((int)ent);
            }
            if (!isIdle) {
                unsigned int moveOk = Entity_AdvancePath(ent, '\x01');
                if ((char)moveOk == '\0') {
                    FUN_00454ba0((int)ent);
                } else {
                    // BUG-FIX 2026-05-03: al llegar al destino, resetear
                    // wp_count + cur_wp para que isIdle (línea 319) sea true
                    // en el frame siguiente. Sin esto, isIdle queda en false
                    // (wp_count != 0), el walker sigue corriendo cada frame
                    // ejecutando FUN_00443930 (sets walk action) → FUN_004430c0
                    // (sets idle action) → frame counter reset cada tick →
                    // player FROZEN en pose de walk frame 0.
                    *(unsigned char*)(ent + 0x354) = 0;   // cur_wp
                    *(unsigned char*)(ent + 0x355) = 0;   // substep
                    *(unsigned char*)(ent + 0x356) = 0;   // wp_count
                    *(unsigned char*)(ent + 0x305) = 0;   // 2026-05-05: move_pending,
                    // sin esto isIdle queda false → walker sigue ejecutando
                    // FUN_00443930 cada frame → action=walk persistente.
                    *(unsigned char*)(ent + 0x2ec) = 0;
                    FUN_004430c0((int)ent);
                    // IDA L401: `dword_7E11DBC = (__int64)*(float *)(v0 + 36);`
                    // — es el FACING del héroe, no un timestamp. El port tenía
                    // `DAT_05826e08` (WorldTime), que dejaba basura en el campo
                    // que después lee la rotación por octante.
                    DAT_07e11dbc = (int)*(float*)(ent + 36);
                    DAT_07e11db8 = 0;
                    Send_MovePacket_Player_legacy_stub();
                    // IDA L397-403: al terminar el camino, Action(c, c) con
                    // la cola que haya (0 = nada).  El port despachaba por
                    // tipo de cola con atajos propios (talk directo, pickup,
                    // solo ataque) y dejaba la cola 4 a un tick secundario.
                    Combat_ProcessQueuedAction((DWORD)ent, (DWORD)ent);
                }
            }
            else
            {
                // ── Rotación del cuerpo hacia el mouse (IDA L419-440) ────────
                // Sólo cuando el héroe está PARADO (sin path activo) — por eso
                // vive en el `else` de `isIdle`, igual que el binario, que lo
                // pone en el `else if (!EditFlag)` del walker.
                //
                // El cuerpo no sigue al mouse de forma continua: se compara el
                // OCTANTE (360/8 = 45°, de ahí el * 1/45 = 0.022222223) del
                // facing actual contra el del mouse, y sólo si cambió se rota.
                // Además espera 40 ticks (~1.6 s a 25 fps) entre rotaciones.
                ++DAT_07e11db8;
                if (DAT_07e11db8 >= 40 && !g_MouseOnWindow && !ent[765])
                {
                    const BYTE act = ent[261];
                    // 139/140/133/135 = animaciones de gate/teleport: no rotar.
                    if (act != 139 && act != 140 && act != 133 && act != 135
                        && bHeadTrackActive && !ent[757])
                    {
                        DAT_07e11db8 = 0;
                        const float mouseX = (float)(int)DAT_083a427c;
                        const float mouseY = (float)(int)DAT_083a4278;
                        const int   angToMouse =
                            (int)FUN_0043e050(mouseX, mouseY, fHalfScreenW, 180.0f);

                        const float curFacing = *(float*)(ent + 36);
                        const int   curOct = (int)((curFacing + 22.5f) * 0.022222223f + 1.0f) & 7;

                        DAT_07e11dbc = (405 - angToMouse) % 360;
                        const float newFacing = (float)DAT_07e11dbc;
                        const int   newOct = (int)((newFacing + 22.5f) * 0.022222223f + 1.0f) & 7;

                        if (curOct != newOct) {
                            *(float*)(ent + 36) = newFacing;
                            // TODO(paquete): el binario avisa acá al server con
                            // PMSG_ACTION_RECV (`C1:18`, Protocol.h:55 —
                            // {BYTE dir; BYTE action; BYTE index[2]}) llevando el
                            // octante como `dir`. El layout exacto del buffer en
                            // el decompile está entrelazado con el ruido de
                            // hash-table (la key de 32 bytes v233..v256), así que
                            // queda sin enviar hasta confirmarlo por disassembly:
                            // sólo afecta a que OTROS jugadores vean la rotación,
                            // y un paquete mal formado desconecta (ver la entrada
                            // "Desconexiones: serial de packets" de CLAUDE.md).
                        }
                    }
                }
            }
        }
    }

    // ── obsolete inline mini-attack disabled below ──────────────────────────
    #if 0
    {
        unsigned char *ent = (unsigned char*)DAT_07abf5d8;
        if (ent && ent[0x2ed] == 3 && ent[0x356] == 0) {
            int targetIdx = (int)DAT_00559ce8;
            if (targetIdx >= 0 && targetIdx < 400) {
                unsigned char *tgt = (unsigned char*)(DAT_07abf5d0 + (uintptr_t)targetIdx * 0x394);
                if (tgt[0] != 0 && tgt[0x34e] == 0) {
                    int hxg = (int)*(int*)(ent + 0x388);
                    int hyg = (int)*(int*)(ent + 0x38c);
                    int txg = (int)*(int*)(tgt + 0x388);
                    int tyg = (int)*(int*)(tgt + 0x38c);
                    int dx = (hxg - txg < 0) ? -(hxg - txg) : (hxg - txg);
                    int dy = (hyg - tyg < 0) ? -(hyg - tyg) : (hyg - tyg);
                    int cheb = (dx > dy) ? dx : dy;
                    if (cheb <= 2) {
                        static const unsigned char s_LoginKey[32] = {
                            0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                            0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                            0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                            0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
                        };
                        WORD targetId = *(WORD*)(tgt + 0x1dc);
                        extern float __cdecl FUN_0043e050(float, float, float, float);
                        float ex = *(float*)(ent + 0x10);
                        float ey = *(float*)(ent + 0x14);
                        float ttx = *(float*)(tgt + 0x10);
                        float tty = *(float*)(tgt + 0x14);
                        float facing = FUN_0043e050(ex, ey, ttx, tty);
                        *(float*)(ent + 0x24) = facing;
                        int dirCode = ((int)((facing + 22.5f) * (1.0f / 45.0f))) & 7;
                        unsigned char pkt[8];
                        pkt[0] = 0xC1;
                        pkt[1] = 0x07;
                        pkt[2] = 0x15;
                        pkt[3] = (unsigned char)((targetId >> 8) & 0xFF);
                        pkt[4] = (unsigned char)(targetId & 0xFF);
                        pkt[5] = 0x64;
                        pkt[6] = (unsigned char)dirCode;
                        for (int i = 3; i < 7; ++i) {
                            pkt[i] ^= pkt[i - 1] ^ s_LoginKey[i & 0x1f];
                        }
                        MuEmu::EncryptSend(pkt, 7);
                        if (DAT_055ca168 != 0xFFFFFFFF) {
                            ::send(DAT_055ca168, (const char*)pkt, 7, 0);
                        }
                        FUN_00444410((int)ent, 0, 0, 0);
                        char ab[96];
                        wsprintfA(ab, "PIT ATTACK IN-RANGE: tgtIdx=%d tgtId=%d dir=%d cheb=%d",
                                  targetIdx, (int)targetId, dirCode, cheb);
                        DbgLogPublic(ab);
                    }
                }
            }
            ent[0x2ed] = 0;  // consume attack mode
        }
    }
    #endif // obsolete inline mini-attack disabled

    // ── Salida temprana si el movimiento está bloqueado o la UI activa (post-walker) ──
    // 2026-05-05: el gate se movió a DESPUÉS del walker, así el walker siempre avanza
    // incluso con DAT_07d78094 seteado (mouse sobre la barra de skills). Sin esto,
    // hover over skill icon froze hero mid-walk with looping anim.
    // [DIAG TEMP #2] por qué se bloquea el input (gate 717) en un frame con click. REMOVER al cerrar #2.
    if (DAT_083a4124 || DAT_083a42c4 || DAT_083a413c) {
        char d[220]; wsprintfA(d,
            "MOVEBLOCK editFlag=%d mouseOnWin=%d invOpen=%d skillMenu=%d mouse=(%d,%d) -> %s",
            (int)DAT_07e11d30, (int)g_MouseOnWindow, (int)DAT_07eaa117, (int)DAT_07db870c,
            (int)DAT_083a427c, (int)DAT_083a4278,
            (DAT_07e11d30 != 0 || g_MouseOnWindow != 0) ? "BLOCKED@717" : "reaches-clickblock");
        DbgLogPublic(d);
    }
    if (DAT_07e11d30 != 0 || g_MouseOnWindow != 0)
        goto end_tick;

    // ── Movement debounce gate (controla envío de packets/clicks, NO walker) ─
    // 2026-05-03: relax el gate cuando el walker está idle (wp_count == 0).
    // Antes el gate era estrictamente time-based (~1.2 sec entre clicks).
    // Si user clickea rápidamente, los clicks se descartaban silenciosamente.
    // Ahora: si idle, aceptar clicks de inmediato; si moviendo, mantener el
    // gate original para no spamear el server con paths intermedios.
    bool walkerIdle = (((unsigned char*)DAT_07abf5d8)[0x356] == 0);
    // [DIAG TEMP #2c] inputs del gate de debounce (736) en frame con click. REMOVER al cerrar #2.
    if (DAT_083a4124 || DAT_083a42c4 || DAT_083a413c) {
        bool gatePass = DAT_00559bec <= DAT_07e11d28 && DAT_07e11dc0 == '\0';
        char dg[200]; wsprintfA(dg,
            "MOVEDEB invOpen=%d walkerIdle=%d 559bec=%d 11d28=%d 11dc0=%d wpcnt=%d -> %s",
            (int)DAT_07eaa117, (int)walkerIdle, (int)DAT_00559bec, (int)DAT_07e11d28,
            (int)DAT_07e11dc0, (int)((unsigned char*)DAT_07abf5d8)[0x356],
            gatePass ? "PASS" : "BLOCKED@debounce");
        DbgLogPublic(dg);
    }
    // [FIX #2 2026-06-30] DAT_07e11dc0 ("movement lock flag B") — per IDA solo lo
    // escriben Attack (0x49CC50) y Chat_InputTick (0x4B6630). Attack es stub vacío
    // en nuestro build y el port de Chat_InputTick omitió ese write, así que NADA
    // lo setea legítimamente → su valor fiel es 0. El runtime mostró -44 (corrupción
    // de un buffer adyacente que toggle con el inventario: cerrado=-44 bloqueaba el
    // gate de debounce). Forzamos 0 acá hasta portar Attack (que reimplementaría el
    // lock real) y/o encontrar el corruptor. Sin esto, el héroe no caminaba con el
    // inventario cerrado en Devias.
    DAT_07e11dc0 = 0;
    if ((DAT_00559bec <= DAT_07e11d28) && DAT_07e11dc0 == '\0') {

        // BUG-FIX 2026-04-30 (v2): un click = un GroundClick.
        //
        // Race condition entre WM_LBUTTONUP y este tick (PIT corre a 25 Hz / 40 ms).
        // Tres casos a manejar:
        //   1. Held click (DOWN > 40 ms): primer tick ve 4124=1 → procesar.
        //      Ticks siguientes mientras held: ya no debería dispararse otra vez.
        //   2. Tap rápido (DOWN+UP < 40 ms): nunca vemos 4124=1, solo 413c=1.
        //   3. Tras un Held: el UP setea 413c=1 — lo cual VOLVERÍA a disparar
        //      en el tick siguiente si solo miramos los flags directos.
        //
        // Solución: edge-guard que cubre un ciclo entero (DOWN→UP→idle).
        // El ciclo se abre con cualquier flag activo y se cierra cuando todo
        // queda idle (sin click held, sin latch, sin pending).
        static bool s_clickCycleConsumed = false;
        // 2026-05-05: trackea si el evento de click ABAJO pasó sobre una ventana.
        // Si sí, todo el ciclo del click (ARRIBA/soltar) también tiene que tratarse
        // como "click de panel" — aunque el usuario haya movido el mouse al mundo antes de soltar.
        // Sin esto, un click en un ícono de skill seguido de una deriva del mouse al
        // mundo antes de soltar disparaba un movimiento por el flanco de subida.
        static bool s_clickStartedOnWindow = false;

        // 2026-05-07 BUG-FIX: detectar entry a in-world (g_GameState == 5)
        // y CONSUMIR los click flags stale del CharSelect click "Enter".
        // Sin esto, el latch DAT_083a413c=1 del click final en CharSelect
        // queda set al primer frame in-world → bClickEdge fires sin click
        // real → mob attack handler dispara → hero ataca al primer mob
        // visible al spawn. User reportó "aparece el char atacando cuando
        // entro al mundo sin haber clickeado nada" 2026-05-07.
        //
        // 2026-05-07 (followup): además wipear el entity pool DAT_07abf5d0
        // (excepto hero slot) porque CharSelect dejaba los slots de los chars
        // disponibles activos (slot[0]=1) con sus nombres en +0x1C1. Cuando
        // entrábamos al mundo, hover detect en FUN_004afdc0 leía esos slots
        // como entidades válidas y Target_Render mostraba sus nombres como
        // si fueran NPCs/players del mundo. User reportó "leo los nombres de
        // los personajes del select character" 2026-05-07.
        {
            static int s_lastGameState = -1;
            int curState = (int)DAT_005615c0;
            if (curState != s_lastGameState) {
                if (curState == 5) {
                    // Entró al mundo en este frame — consume cualquier flag de click viejo.
                    DAT_083a4124 = '\0';
                    DAT_083a413c = '\0';
                    DAT_083a42c4 = 0;
                    // Limpia los objetivos de hover para evitar arrastre de estado del char-select.
                    SelectedCharacter = -1;
                    SelectedNpc = -1;
                    SelectedItem = -1;
                    SelectedOperate = -1;
                    DAT_00559c58 = -1;
                    DAT_00559c70 = -1;
                    DAT_00559ce8 = -1;
                    // Clear hero action queue (+0x2ED) so secondary tick
                    // doesn't fire Action() with garbage. NO tocar +0x2EC —
                    // ése es un state flag (is_moving / in_action) que server
                    // maneja, no un "alive" flag (per IDA ReceiveAction:20
                    // setea a 0 durante acciones de entidades vivas).
                    //
                    // 2026-05-07: también resetear anim_state a idle (1) y
                    // path state. El hero entity hereda anim_state stale del
                    // CharSelect (donde se anima el preview en walk/idle) y
                    // sin reset queda walking-in-place al spawn del mundo.
                    if (DAT_07abf5d8) {
                        BYTE* hero = (BYTE*)DAT_07abf5d8;
                        hero[0x2ed] = 0;            // action queue
                        hero[0x105] = 1;            // anim_state = idle
                        hero[0x106] = 1;            // anim_state_prev
                        *(float*)(hero + 0x108) = 0.0f;  // anim frame
                        hero[0x354] = 0;            // path_current_wp
                        hero[0x355] = 0;            // path_substep
                        hero[0x356] = 0;            // path_wp_count
                        hero[0x305] = 0;            // move_pending
                    }
                    // 2026-05-07: NO wipear el entity pool aquí — Player_InputTick
                    // corre DESPUÉS que F3/03 JoinMapServer ya pobló el pool con
                    // viewport spawns (0x12/0x13). Wipearlo aquí borraba los mobs
                    // recién spawneados → user veía mundo vacío con hero walking
                    // in place. El wipe se hace en Net_Process F3/03 handler
                    // ANTES del OpenWorld load, donde es safe.
                }
                s_lastGameState = curState;
            }
        }

        // 2026-05-07 (FINAL): semántica per IDA Player_InputTick:
        //   DAT_083a4124 = MouseLButtonPush (DOWN pulse, ONE-SHOT)
        //                  Lo setea WndProc en WM_LBUTTONDOWN si DAT_083a42c4==0.
        //                  IDA consume: `if (Push) { Push=0; v32=1; }`.
        //   DAT_083a42c4 = MouseLButton (estado de mantenido: se setea al bajar, se limpia al soltar)
        //   DAT_083a413c = MouseLButtonPop (set on UP no-drag)
        //
        // PUSH es one-shot — set en DOWN, consumed por nosotros aquí, no se
        // dispara again hasta el next DOWN.
        // HELD es continuous — refleja el real-time mouse button state.
        // POP es UP-edge — set en UP no-drag, consumed por dialog buttons etc.
        //
        // BUG previo: nuestro IsClickPushed() retornaba DAT_083a4124==1 que
        // permanecía true durante todo el hold. NO consumíamos en
        // InputTick → cada frame veía push=true → cualquier reset de
        // s_prevAnyClick disparaba edge espurio en hover.
        bool bMousePush    = (DAT_083a4124 == 1);   // DOWN pulse this tick
        bool bClickHeld    = (DAT_083a42c4 != 0);   // real-time held state
        bool bClickLatched = (DAT_083a413c == 1);   // UP no-drag latch
        bool bAnyPending   = bClickHeld || bClickLatched || bMousePush;

        if (!bAnyPending) {
            // Idle: cierra ciclo previo, listo para uno nuevo.
            s_clickCycleConsumed = false;
            s_clickStartedOnWindow = false;
        }

        // Capture: si el click recién empezó (DOWN edge) y mouse está sobre
        // window, marcar para todo el ciclo.
        if (bClickHeld && !s_clickCycleConsumed && g_MouseOnWindow) {
            s_clickStartedOnWindow = true;
        }

        // 2026-05-06: detectar cambio de hover target durante click held.
        // Si user click on mob A → mob A muere → user mueve mouse a mob B
        // sin liberar click, debería ser un new intent (new attack on B).
        // El cycle-consumed bloquea, así que reset cycle si target cambia.
        //
        // 2026-05-06 (followup): GATED POR bClickHeld. Sin esto, un
        // bClickLatched colgado (DAT_083a413c=1) más cualquier cambio de
        // hover (= mouse pasando sobre mob) reseteaba el cycle → bHoverActive
        // se volvía true sin click real → attack disparaba al pasar el mouse
        // sobre un mob. User reportó: "si le paso el mouse por encima ataca,
        // no si le hago click" 2026-05-06.
        static int s_lastHoverMob = -1;
        int curHoverMob = (int)SelectedCharacter;
        if (curHoverMob != -1 && curHoverMob != s_lastHoverMob && bClickHeld) {
            // Cambió el objetivo bajo el mouse Y el mouse está efectivamente apretado → intención de arrastre.
            s_clickCycleConsumed = false;
        }
        s_lastHoverMob = curHoverMob;

        // 2026-05-06: detectar DOWN edge del click. Cada nuevo click DEBE
        // disparar nuevo cycle (incluso si user click rapidamente sobre el
        // mismo mob varias veces). Antes user tenía que mover mouse para que
        // funcionara cada attack — muy molesto.
        static bool s_prevClickHeld = false;
        if (bClickHeld && !s_prevClickHeld) {
            // Rising edge: new click started
            s_clickCycleConsumed = false;
        }
        s_prevClickHeld = bClickHeld;

        // 2026-05-07 FINAL (matching IDA semantics):
        //   bClickEdge = bMousePush — el push pulse YA es one-shot per IDA.
        //   Después del click handler, lo CONSUMIMOS (DAT_083a4124 = 0).
        //   Próximos frames: push=0 hasta el next DOWN. NO se dispara en
        //   hover, NO se dispara espurio.
        //
        // Push semantics:
        //   - DOWN: WndProc sets DAT_083a4124=1, DAT_083a42c4=1.
        //   - InputTick this frame: bMousePush=true → process → consume (=0).
        //   - Held: 4124=0 (consumed), 42c4=1 (still held).
        //   - UP: WndProc clears 4124 (already 0), 42c4=0, sets 413c=1.
        //   - Next InputTick: bMousePush=false, all clean.
        //
        // El bClickLatched (POP/UP-edge) sigue funcionando como respaldo para
        // dialogs/menus que necesiten detectar UP. NO se usa aquí para
        // attack-arm (eso causaba el hover-attack bug).
        bool bClickEdge = bMousePush;
        // 2026-05-08 BUG-FIX MAYÚSCULO: solo consumir el push pulse si el
        // click NO está sobre una UI window. Si el cursor está sobre el
        // inventario / character panel / shop / etc., el click handler de
        // ESA window (FUN_004d23b0 invocado más tarde en el render pipeline
        // desde RenderInventoryWindow / RenderShopInterface / etc.) necesita
        // ver `DAT_083a4124 == 1` para detectar y procesar el click. Si lo
        // consumimos acá, el handler del UI ve 0 y el pickup/use jamás
        // dispara — síntoma observado: hovers funcionan (la rama hover de
        // FUN_004d23b0 no usa el flag), pero ningún click consigue mover ni
        // consumir items. `g_MouseOnWindow` lo setea HUD_HitTest_AllWindows
        // (líneas 65-90) basado en las flags de panel abierto + bounding box.
        if (bMousePush && !g_MouseOnWindow) {
            DAT_083a4124 = 0;
        }

        // Hard-consume del latch POP también, por si quedó stale.
        // Mismo razonamiento: solo consumir si NO estamos sobre UI.
        if (bClickLatched && !g_MouseOnWindow) {
            DAT_083a413c = 0;
        }

        // 2026-05-07: durante los primeros 10 frames in-world, force bClickEdge=false.
        // Cubre el caso de WndProc dejando DAT_083a4124=1 colgado durante la
        // transición CharSelect → World, o cualquier edge espurio causado por
        // race condition en la inicialización del input system.
        {
            static int s_inWorldFramesEdge = 0;
            if (DAT_005615c0 == 5) s_inWorldFramesEdge++;
            else                   s_inWorldFramesEdge = 0;
            if (s_inWorldFramesEdge < 10) {
                bClickEdge = false;
            }
        }

        // 2026-05-07 SAFETY (mejorada): clear ent[0x2ed] (action queue) cuando:
        //   - NO hubo click edge este frame
        //   - El user NO está sosteniendo el botón izquierdo (bClickHeld=false)
        //   - Walker está idle (ent[0x356]==0) — sin path activo
        //
        // Match IDA Player_InputTick:599 que requires `m_bAutoAttack && Attacking==1
        // && SelectedCharacter!=-1` AND v32 (current click) para continuar combat.
        // Sin alguna de esas, bail (= no attack/action).
        //
        bool bHoverActive = false;
        if ((bClickHeld || bClickLatched) && !s_clickCycleConsumed) {
            bHoverActive = true;
            s_clickCycleConsumed = true;
            // 2026-05-04: NO consumir DAT_083a4124 cuando el mouse está sobre
            // un panel — el render-phase de RenderCharacterInfoWindow / etc.
            // necesita ese flag para detectar clicks en sus botones (X close,
            // [+] stat add, etc.).  Si lo consumimos acá, los handlers de
            // panel ven `pressed=false` y nunca disparan.  Solo consumir
            // cuando el click ES para el ground (mouse fuera de panels).
            if (!g_MouseOnWindow) {
                DAT_083a4124 = '\0';
                DAT_083a413c = '\0';   // consume ambos flags
            }
        }
        // 2026-05-05: Si el mouse está sobre un panel (HUD bottom, skill
        // expanded list, panels right-side), forzar bHoverActive=false para
        // que los alt-targets (hover target, NPC click, tertiary) NO procesen
        // el click como movement. Antes user click en skill icon → bHoverActive
        // permanecía true → disparaba pathfind a stale hover target → hero
        // caminaba al lugar de un NPC/monster cercano.
        //
        // 2026-05-05 (followup): TAMBIÉN consume DAT_083a42c4 y reset
        // SelectedCharacter/4c/48 (hover targets) cuando hay click sobre window.
        // Sin esto, un hover target stale (NPC bajo el cursor del frame
        // anterior) hacía que líneas 757/801/831 dispararan pathfind aunque
        // bHoverActive=false, aunque DAT_083a42c4 conserve el latch del click.
        if (g_MouseOnWindow || s_clickStartedOnWindow) {
            bHoverActive = false;
            // Consume los flags de click para que no se propaguen al tick
            // alt-target processing. Panel handlers se registraron via
            // Chat_InputTick (corre antes); ya no necesitamos los flags.
            DAT_083a4124 = '\0';
            DAT_083a413c = '\0';
            DAT_083a42c4 = 0;
        }

        // [DIAG TEMP #2b] inputs de bHoverActive + hover-targets en frame con click. REMOVER al cerrar #2.
        if (bClickHeld || bClickLatched || bMousePush) {
            char dh[256]; wsprintfA(dh,
                "MOVEHOVER invOpen=%d push=%d held=%d latch=%d cycCons=%d startWin=%d mouseOnWin=%d hovActive=%d c50=%d c4c=%d c54=%d",
                (int)DAT_07eaa117, (int)bMousePush, (int)bClickHeld, (int)bClickLatched,
                (int)s_clickCycleConsumed, (int)s_clickStartedOnWindow, (int)g_MouseOnWindow,
                (int)bHoverActive, (int)SelectedCharacter, (int)SelectedNpc, (int)SelectedOperate);
            DbgLogPublic(dh);
        }

        // 2026-05-05: si el user NO está clickeando activamente (no held, no
        // latched), forzar DAT_083a42c4=0 también. Sin esto, un click anterior
        // que no se consumió bien puede dejar este flag activo después de
        // soltar el botón.
        if (!bClickHeld && !bClickLatched) {
            DAT_083a42c4 = 0;
        }
        // [DIAG 2026-04-28] Una vez por segundo: loguea todo lo que afecta al caminar
        {
            static DWORD s_lastDiag = 0;
            DWORD now = GetTickCount();
            if (now - s_lastDiag > 1000) {
                s_lastDiag = now;
                unsigned char *ent_dbg = (unsigned char*)DAT_07abf5d8;
                if (ent_dbg) {
                    char dbg[320];
                    float wx = *(float*)(ent_dbg + 0x10);
                    float wy = *(float*)(ent_dbg + 0x14);
                    float wz = *(float*)(ent_dbg + 0x18);
                    float fa = *(float*)(ent_dbg + 0x24);
                    int wxi = (int)wx, wyi = (int)wy, wzi = (int)wz, fai = (int)fa;
                    int wxf = (int)((wx - wxi) * 100), wyf = (int)((wy - wyi) * 100);
                    int wzf = (int)((wz - wzi) * 100), faf = (int)((fa - fai) * 10);
                    wsprintfA(dbg,
                        "PIT click=%d hov=%d tgt=%d,%d cwp=%d,%d wp=%d/%d "
                        "move=%d 2ec=%d ANIM=0x%02x|0x%02x WPOS=(%d.%02d,%d.%02d,%d.%02d) FACE=%d.%d "
                        "h50=%d h4c=%d h48=%d c70=%d",
                        (int)DAT_083a4124, (int)bHoverActive,
                        (int)ent_dbg[0x306], (int)ent_dbg[0x307],
                        (int)*(int*)(ent_dbg + 0x388), (int)*(int*)(ent_dbg + 0x38c),
                        (int)ent_dbg[0x354], (int)ent_dbg[0x356],
                        (int)ent_dbg[0x305], (int)ent_dbg[0x2ec],
                        (int)ent_dbg[0x105], (int)ent_dbg[0x106],
                        wxi, wxf, wyi, wyf, wzi, wzf, fai, faf,
                        (int)SelectedCharacter, (int)SelectedNpc,
                        (int)SelectedItem, (int)DAT_00559c70);
                    DbgLogPublic(dbg);
                }
            }
        }


        // (Walker movido arriba del gate — ya corrió al inicio del tick.)
        unsigned char *ent = (unsigned char*)DAT_07abf5d8;

        _DAT_07e11d50 = (DAT_05826e08 - _DAT_07e11d4c) * _DAT_00552890;

        // IDA 0x004ACEF0 L613-625: ANTES de procesar el click, el original lo
        // descarta mientras el heroe no puede actuar:
        //   accion 130 (golpeado), c+124 == 1 o 2, alpha (c+360) < 0.7, o una
        //   animacion de ataque/skill (34..91) — salvo las 78..80.
        // Faltaba: spameando clicks durante el golpe se re-disparaba Action y
        // el ataque se reiniciaba (doble golpe en la misma animacion).
        if (ent) {
            const unsigned char act = ent[261];
            const unsigned char st  = ent[124];
            if (act == 0x82 || st == 1 || st == 2 ||
                *(float*)(ent + 360) < 0.69999999f ||
                (act >= 0x22 && act <= 0x5B)) {
                if (act < 0x4E || act > 0x50)
                    goto end_tick_inc;                 // IDA: goto LABEL_390
            }
        }

        // IDA 0x004ACEF0 LABEL_190 (raw L716-718):
        //     LABEL_190: v86 = *(_BYTE *)(v34 + 846);   // SafeZone
        //                MouseUpdateTime = 0;
        //                if ( !v86 && CheckAttack() ) ...
        // El reset es INCONDICIONAL y es el punto de merge de todo el bloque de
        // accion del click (ataque a mob, ground click, operate).  Junto con el
        // gate `MouseUpdateTimeMax <= MouseUpdateTime` de mas arriba es el
        // debounce real del original: SendMove (0x00491C40 L128-146) deja
        // MouseUpdateTimeMax = 0 si la ruta tiene <= 2 waypoints (click corto,
        // sin throttle) y 3*wp+4 si es mas larga, asi que mientras se camina una
        // ruta larga no se acepta otro click.
        //
        // La nota anterior decia "NO resetear aca"; era incorrecta.  Sin este
        // reset el contador crecia sin techo, el gate quedaba abierto en todos
        // los frames y con el boton mantenido el bloque corria dos veces por
        // tick: el primer paso armaba el ataque (ent[0x2ed]=3 + ruta al mob) y
        // el segundo caia en el ground click, repathfindeaba al tile del cursor
        // y pisaba la ruta.  Como el heroe quedaba siempre caminando,
        // `ent[0x2ed]==3 && ent[0x356]==0` nunca se cumplia, Action nunca corria
        // y el cursor parpadeaba entre ataque y movimiento.
        DAT_07e11d28 = 0;

        // ── Movement/attack packet for swimming anim ─────────────────────────
        // Si la entidad está viva y CanAct y en movimiento de nado:
        if (*(char*)(ent + 0x34e) == '\0') {
            unsigned int canAct = FUN_00483160();
            // BUG-FIX 2026-04-28: FUN_00483160 (CheckAttack) retorna 0 cuando
            // no hay entidad bajo el mouse (SelectedCharacter == -1). El gate
            // original solo dejaba pasar entity-hover-clicks → ground-click
            // (clic en el suelo sin hover de entidad) NUNCA disparaba el
            // pathfind → hero no se movía nunca.
            // El IDA original probablemente separaba ground-click fuera de
            // este gate; aquí relajamos: si bHoverActive (click real), pasar
            // aunque canAct=0. Los handlers internos siguen gateados por
            // SelectedCharacter/4c/48/54 != -1, así que no disparan spurio.
            if ((char)canAct != '\0' || bHoverActive) {
                if (*(char*)(ent + 0x2ec) != '\0'
                    && *(char*)(ent + 0x2ed) == '\0'
                    && (*(unsigned char*)(ent + 0x1bc) & 7) == 2)
                {
                    short animA = *(short*)((unsigned char*)DAT_07cf1ffc + 0x86);
                    short animB = *(short*)((unsigned char*)DAT_07cf1ffc + 0x97);

                    // Anim ranges for movement packet [0xC1][0x11]
                    bool sendMovePkt =
                        (animA > 0x87 && animA < 0x8f)
                     || (animA > 0x8f && animA < 0xa0)
                     || (animB > 0x7f && animB < 0x87)
                     || (animB == 0x91);

                    if (sendMovePkt) {
                        // Build [0xC1][0x11] movement packet
                        // Payload: grid_x (entity+0x388), grid_y (entity+0x38c)
                        unsigned char movePkt[6];
                        movePkt[0] = 0xC1;
                        movePkt[1] = 1;     // len placeholder
                        movePkt[2] = 0x11;
                        movePkt[3] = (unsigned char)(*(int*)(ent + 0x388));  // src_x
                        movePkt[4] = (unsigned char)(*(int*)(ent + 0x38c));  // src_y
                        movePkt[5] = 0;
                        SendPacket((char*)movePkt, 6);
                    }
                }

                // ── Hover entity attack (SelectedCharacter valid) ─────────────────
                // 2026-05-06 (final): GATEAR POR bClickEdge (rising edge del
                // mouse press, capturado tanto desde bClickHeld como del
                // latch DAT_083a413c). Match IDA Player_InputTick que usa
                // `MouseLButton` raw — el ataque se arma SOLO en el frame
                // exacto donde el botón pasa de released → pressed.
                //
                // Bug original: bHoverActive era TRUE mientras bClickLatched
                // estuviera set (entre frames antes de consumirse), aunque
                // el user NO estuviera apretando el mouse. Cualquier cambio
                // de hover target durante esa ventana → attack disparaba al
                // pasar el mouse sobre un mob. User reportó: "si le paso el
                // mouse por encima ataca, no si le hago click".
                //
                // Con bClickEdge: dispara una sola vez por click. Sin posibili-
                // dad de spuriarse por latches stale, hover changes, etc.
                //
                // Filtro adicional: mobs MUERTOS (entity[+0x34e]==1) no son
                // targeteables.
                // 2026-09-16: IDA 0x004ACEF0 L590-603 no exige el flanco:
                //   v32 = MouseLButtonPush || MouseLButton;
                //   if ((!m_bAutoAttack || World == 6 || Attacking != 1 ||
                //        SelectedCharacter == -1) && !v32) goto LABEL_390;
                // O sea con el boton MANTENIDO se sigue atacando (el gate de
                // animacion de mas arriba marca el ritmo), y con m_bAutoAttack
                // el ataque continua al soltar mientras el objetivo siga
                // vivo (sub_4B0310 lo mantiene fijo).  Antes solo pegaba en el
                // frame del click.
                const bool bAutoAttackGoOn = DAT_00559c5c != 0          // m_bAutoAttack
                                          && DAT_0055a7ac != 6           // World
                                          && (int)DAT_00559c58 == 1;     // Attacking
                if (SelectedCharacter > -1 && (bClickEdge || bClickHeld || bAutoAttackGoOn)) {
                    // IDA Player.cpp (0x004ACEF0) gates the character-attack
                    // path with CheckAttack before it reaches Action().  Action
                    // itself intentionally sends 0x15 without rechecking it.
                    // Keep the earlier bHoverActive relaxation only for ground
                    // movement; applying it here allowed neutral players to be
                    // queued as attack targets without Ctrl after Guild War.
                    if ((char)canAct == '\0') {
                        goto end_tick_inc;
                    }
                    {
                        // 2026-05-07 diag — keep until hover bug resolved.
                        char dbg[200];
                        wsprintfA(dbg, "PIT MOB CLICK FIRED: c50=%d bMousePush=%d bClickHeld=%d bClickLatched=%d 4124=%d 42c4=%d 413c=%d",
                            (int)SelectedCharacter, (int)bMousePush,
                            (int)bClickHeld, (int)bClickLatched,
                            (int)DAT_083a4124, (int)DAT_083a42c4, (int)DAT_083a413c);
                        DbgLogPublic(dbg);
                    }
                    BYTE* hoverEnt = (BYTE*)(uintptr_t)DAT_07abf5d0
                                   + (uintptr_t)SelectedCharacter * 0x394;
                    // 2026-05-07: dead check usa SOLO 0x2FD per IDA ReceiveDie:18.
                    // El check viejo `0x2EC == 0` era WRONG: 0x2EC es un "state"
                    // flag que el server setea a 0 también en ReceiveAction y
                    // otros casos NO-muerte (per IDA ReceiveAction:20). Usarlo
                    // como "dead" filter rechazaba mobs vivos → click handler no
                    // armaba attack. User reportó que el ataque "a veces" no
                    // disparaba (cuando el mob estaba en mid-action).
                    // 2026-08-10: sacado el `|| hoverEnt[0x34e] != 0`. +0x34E es
                    // SafeZone, no dead: incluirlo volvía NO-targeteable a todo
                    // NPC parado en zona segura (o sea todos los del pueblo).
                    if (hoverEnt[0x2FD] != 0) {
                        // Muerto — limpia el estado de hover para que el click siguiente no quede
                        // pegado al cadáver, y sale.
                        SelectedCharacter = -1;
                        SelectedOperate = -1;
                        DAT_00559c58 = 0;
                        DAT_00559c70 = -1;
                        goto end_tick_inc;
                    }
                    int tgtEntityBase = (int)(DAT_07abf5d0 + SelectedCharacter * 0x394);
                    int dstX = *(int*)(tgtEntityBase + 0x388);
                    int dstY = *(int*)(tgtEntityBase + 0x38c);

                    DAT_00559ce8 = SelectedCharacter;
                    DAT_00559c58  = 1;
                    *(unsigned char*)(ent + 0x2ed) = 3;
                    // 2026-05-07 BUG-FIX: dst grid coords son del MOB target,
                    // NO `DAT_05826e08` (eso es g_AnimTick, tick counter).
                    // El bug viejo asignaba el tick counter como grid coord,
                    // entonces pathfind iba a un tile aleatorio basado en
                    // frame number → user reportó que click far mob no movía
                    // al hero pero hacía attack animation in place.
                    DAT_07e016c0 = (DWORD)dstX;
                    DAT_07e016c4 = (DWORD)dstY;

                    DAT_07db8708    = (int)*(short*)(tgtEntityBase + 2);
                    _DAT_07e118e4   = *(DWORD*)(tgtEntityBase + 0x24);
                    DAT_00559c70    = SelectedCharacter;

                    // Pathfind to hover target
                    int srcX = *(int*)(ent + 0x388);
                    int srcY = *(int*)(ent + 0x38c);

                    // IDA 0x004ACEF0 L986-988: `if (!CheckWall(hx, hy, TargetX,
                    // TargetY)) goto LABEL_390;` — con una pared entre el heroe y
                    // el objetivo no se camina ni se ataca (el objetivo y la cola
                    // ya quedaron fijados arriba, igual que en el original).
                    // Faltaba: el heroe salia a caminar o pegaba a traves de la
                    // pared y el server descartaba el golpe.
                    if (!Path_IsLineClear(srcX, srcY, dstX, dstY))
                        goto end_tick_inc;

                    // IDA 0x004ACEF0 L1027-1131 — tres salidas, no dos:
                    //
                    //   if ( !PathFinding2(hx, hy, TargetX, TargetY, c + 852, 0.0) )
                    //   {                                   // ya adyacente / sin ruta
                    //       if ( !CheckArrow() ) return;
                    //       Action(c, c);  goto LABEL_390;
                    //   }
                    //   ...
                    //   if ( v265 >= 136 && v265 < 143 || v265 >= 144 && v265 < 160
                    //     || v264 >= 128 && v264 < 135 || v264 == 145 )
                    //       v148 = c;                       // ARMA A DISTANCIA
                    //   else
                    //   {
                    //       v148 = c;
                    //       if ( *(_BYTE *)(c + 747) != 9 )
                    //       {
                    //           SendMove(c, c);             // MELEE: caminar
                    //           goto LABEL_390;
                    //       }
                    //   }
                    //   if ( !CheckArrow() ) return;
                    //   Action(v148, v148);                 // dispara desde donde esta
                    //   goto LABEL_390;
                    //
                    // v265/v264 son los mismos tipos de item que lee Action para
                    // elegir Range (CharacterMachine + 536 / + 604), y los mismos
                    // del gate de L762.  O sea: con arco o ballesta el original NO
                    // manda el paquete de movimiento aunque exista ruta -- llama
                    // Action directo y ahi el gate de distancia usa Range = 6.0.
                    //
                    // El port mandaba SIEMPRE a caminar cuando habia ruta, y en la
                    // rama sin ruta mandaba un move en vez de Action: de ahi que la
                    // elfa se acercara al cuerpo a cuerpo para poder pegar.
                    unsigned int ok2 = Path_FindRoute(srcX, srcY,
                                                     dstX, dstY,
                                                     ent + 0x354, 0.0f);
                    if ((char)ok2 == 0) {
                        // Sin ruta (ya esta al lado, o inalcanzable) -> atacar.
                        if (Combat_CheckArrowRequirement() == 0)
                            goto end_tick;              // IDA: return (sin ++MouseUpdateTime)
                        Combat_ProcessQueuedAction((DWORD)(uintptr_t)ent,
                                                   (DWORD)(uintptr_t)ent);
                        goto end_tick_inc;
                    }
                    {
                        const char* const CM = (const char*)(uintptr_t)DAT_07cf1ffc;
                        const int lh = CM ? *(const short*)(CM + 536) : -1;  // IDA: v265
                        const int rh = CM ? *(const short*)(CM + 604) : -1;  // IDA: v264
                        const bool bRanged = (lh >= 136 && lh < 143)
                                          || (lh >= 144 && lh < 160)
                                          || (rh >= 128 && rh < 135)
                                          || (rh == 145);
                        if (!bRanged && *(unsigned char*)(ent + 747) != 9) {
                            Combat_SendMovePathPacket((int)ent, (int)ent);   // IDA: SendMove
                            goto end_tick_inc;
                        }
                    }
                    if (Combat_CheckArrowRequirement() == 0)
                        goto end_tick;                  // IDA: return
                    Combat_ProcessQueuedAction((DWORD)(uintptr_t)ent,
                                               (DWORD)(uintptr_t)ent);
                    goto end_tick_inc;
                }

                // IDA 0x004ACEF0 L719-1132: el bloque `if (!SafeZone && CheckAttack())`
                // es EXCLUYENTE — todas sus salidas hacen `goto LABEL_390`; nunca cae al
                // click de NPC / item / suelo que viene despues.
                //
                // Nuestro port le agrego `&& bClickEdge` al gate de la rama del mob, asi
                // que con el boton MANTENIDO esa rama no se tomaba y la ejecucion seguia
                // hasta el ground click, que repathfindeaba al tile del cursor y mandaba
                // un move: por eso el heroe caminaba hasta donde estaba el monstruo
                // despues de matarlo.
                //
                // El `|| bHoverActive` de mas arriba es una relajacion del port para que
                // el ground click funcione cuando CheckAttack() da 0 (sin objetivo), asi
                // que la exclusividad se aplica SOLO con canAct != 0, que es el gate real
                // del binario.
                if ((char)canAct != 0)
                    goto end_tick_inc;
            }
        }

        // ── Alt-target: NPC/item (SelectedNpc != -1) ────────────────────────
        // 2026-09-04 (b): este bloque estaba DENTRO del guard
        //     if (SelectedOperate == -1 || (montado && !SafeZone)) { ... }
        // que envuelve las ramas de NPC / item / click al suelo.  O sea cuando SI
        // habia objeto seleccionado, el guard saltaba todo el bloque -- incluido el
        // propio manejo del operate.  Medido en debug.log: en el frame del click
        // `MOVEHOVER ... c54=23` no lo seguia ni `PIT GroundClick!` ni la sonda.
        // En IDA los dos son `if` HERMANOS y el de SelectedOperate va primero:
        //     if ( SelectedOperate != -1 ) { ... }
        //     if ( SelectedNpc != -1 )     { ... }
        // Objeto interactuable bajo el cursor (sillas, bancos, barandas,
        // orbes de Noria).  IDA 0x4ACEF0 L1133-1195.
        //
        // 2026-09-04 FIX: este bloque estaba como `else if (bClickEdge)`
        // del `if (!shiftHeld)` de abajo, o sea SOLO corria con Shift
        // apretado.  En IDA la cadena es secuencial y SelectedOperate se
        // chequea ANTES del ramo de movimiento por terreno:
        //     if ( SelectedOperate != -1 ) { ... goto LABEL_340/LABEL_312; }
        //     ...
        //     if ( GetAsyncKeyState(16) >> 8 != 0x80 ) { RenderTerrain(1); ... }
        // y las dos salidas del bloque saltan al final del tick, o sea
        // tienen PRECEDENCIA sobre el click al suelo.
        if (SelectedOperate != -1 && bClickEdge) {
            // Gate de montura (IDA L1135): solo se opera si NO se va
            // montado, o si se esta en zona segura.
            const unsigned short helper = *(unsigned short*)(ent + 0x2b8);
            const bool mountOk = ((helper != 818 && helper != 819)
                                  || *(unsigned char*)(ent + 0x34e) != 0);
            const int iSrc = SelectedOperate;
            // Bound check (no esta en IDA): SelectedOperate viene del
            // picker del frame anterior.
            const int nOper = (int)(sizeof(DAT_083a2370) / 0xc);
            const int tgtEntityPtr = (iSrc >= 0 && iSrc < nOper)
                                   ? ((int*)&DAT_083a2378)[iSrc * 3] : 0;

            if (mountOk && tgtEntityPtr != 0) {
                // 2026-09-04 FIX: TargetX/TargetY salen de la POSICION
                // DEL OBJETO, no del tile bajo el cursor.  IDA L1138:
                //     TargetX = (__int64)(o->Position[0] * 0.01);
                //     TargetY = (__int64)(o->Position[1] * 0.01);
                DAT_07e016c0 = (DWORD)(int)(*(float*)(tgtEntityPtr + 0x10) * 0.01f);
                DAT_07e016c4 = (DWORD)(int)(*(float*)(tgtEntityPtr + 0x14) * 0.01f);

                const int attrIdx = FUN_004f6c30((int)DAT_07e016c0, (int)DAT_07e016c4);
                if (((unsigned char*)&DAT_0838bc70)[attrIdx] < 2
                    && *(char*)(ent + 0x2ec) == 0)
                {
                    *(unsigned char*)(ent + 0x2ed) = 4;   // MOVEMENT_OPERATE
                    DAT_07db8708  = (int)*(short*)(tgtEntityPtr + 2);
                    _DAT_07e118e4 = *(DWORD*)(tgtEntityPtr + 0x24);

                    const int srcX = *(int*)(ent + 0x388);
                    const int srcY = *(int*)(ent + 0x38c);
                    unsigned int ok = Path_FindRoute(srcX, srcY,
                                                     DAT_07e016c0, DAT_07e016c4,
                                                     ent + 0x354, 0.0f);
                    if ((char)ok == 0) {
                        // LABEL_312: sin camino (ya estamos al lado) ->
                        // ejecutar la accion ahora.  El port mandaba otro
                        // paquete de movimiento y NUNCA llamaba a Action,
                        // asi que sentarse no se disparaba nunca.
                        Combat_ProcessQueuedAction((DWORD)ent, (DWORD)ent);
                    } else {
                        // LABEL_340: hay camino -> caminar hasta el objeto.
                        Combat_SendMovePathPacket((int)ent, (int)ent);
                    }
                    goto end_tick_inc;
                }
            }
        }

        if (SelectedOperate == -1
            || ((*(short*)(ent + 0x2b8) == 0x332 || *(short*)(ent + 0x2b8) == 0x333)
                && *(char*)(ent + 0x34e) == '\0'))
        {
            if (SelectedNpc != -1 && bClickEdge) {
                // 2026-05-06: bClickEdge en vez de bHoverActive — mismo fix
                // que el attack handler arriba para evitar disparos por
                // bClickLatched stale + cambio de hover.
                // Click sobre un NPC: setea el objetivo de movimiento y pathfindea (gateado por un click real)
                if (DAT_07eaa118 == '\0' && DAT_07eaa119 == '\0') {
                    *(unsigned char*)(ent + 0x2ed) = 2;
                    DAT_00559ce8 = SelectedNpc;
                    int tgtBase = (int)(DAT_07abf5d0 + SelectedNpc * 0x394);
                    DAT_07db8708  = (int)*(short*)(tgtBase + 2);
                    _DAT_07e118e4 = *(DWORD*)(tgtBase + 0x24);
                    DAT_00559c70  = SelectedNpc;

                    int srcX = *(int*)(ent + 0x388);
                    int srcY = *(int*)(ent + 0x38c);
                    int dstX = *(int*)(tgtBase + 0x388);
                    int dstY = *(int*)(tgtBase + 0x38c);
                    // 2026-05-07 BUG-FIX: dst grid coords del NPC, NO el animTick.
                    DAT_07e016c0 = (DWORD)dstX;
                    DAT_07e016c4 = (DWORD)dstY;

                    unsigned int ok = Path_FindRoute(srcX, srcY,
                                                    dstX, dstY,
                                                    ent + 0x354, 0.0f);
                    if ((char)ok == '\0') {
                        // IDA L1210: sin camino -> LABEL_312 (Action manda el 0x30).
                        Combat_ProcessQueuedAction((DWORD)ent, (DWORD)ent);
                    } else {
                        Combat_SendMovePathPacket((int)ent, (int)ent);
                    }
                    goto end_tick_inc;
                }
            }

            // ── Tertiary target (SelectedItem) ───────────────────────────────
            if (SelectedItem != -1 && bClickEdge) {
                // 2026-05-06: bClickEdge en vez de bHoverActive (mismo fix).
                *(unsigned char*)(ent + 0x2ed) = 1;
                ItemKey = (DWORD)SelectedItem;   // latch, IDA L1281
                // 2026-07-27 BUG-FIX: SelectedItem es índice del pool de items
                // del suelo (DAT_07e12840, stride 0x204), NO del pool de
                // personajes (DAT_07abf5d0, stride 0x394). El port anterior leía
                // el destino del pool equivocado → coords basura → el héroe
                // caminaba a cualquier lado. El tile del item = worldXY/100
                // (world = base+16/20).
                int itemSlotIdx = (int)SelectedItem;
                BYTE* itemEnt = (BYTE*)&DAT_07e12840[0]
                              + (uintptr_t)itemSlotIdx * 0x204;
                // 2026-07-27 BUG-FIX: la posición world del item la escribe
                // CreateItem en ip+88/92 (no ip+16). Leer ip+16 daba (0,0) → el
                // héroe caminaba al origen del mundo (nada). El render lee la pos
                // en v1+16 = ip+72+16 = ip+88; el item-base (itemEnt) = ip, así
                // que la pos está en itemEnt+88/92.
                int dstX = (int)(*(float*)(itemEnt + 88) / 100.0f);
                int dstY = (int)(*(float*)(itemEnt + 92) / 100.0f);
                DAT_07e016c0 = (DWORD)dstX;
                DAT_07e016c4 = (DWORD)dstY;
                int srcX = *(int*)(ent + 0x388);
                int srcY = *(int*)(ent + 0x38c);

                unsigned int ok = Path_FindRoute(srcX, srcY,
                                                dstX, dstY,
                                                ent + 0x354, 0.0f);
                if ((char)ok == '\0') {
                    // IDA L1318: sin camino -> Action y cola en 0.
                    Combat_ProcessQueuedAction((DWORD)ent, (DWORD)ent);
                    *(unsigned char*)(ent + 0x2ed) = 0;
                } else {
                    Combat_SendMovePathPacket((int)ent, (int)ent);
                }
                goto end_tick_inc;
            }

            // ── Ground click: ray cast → terrain check → pathfind ────────────
            // 2026-08-17 — REVERTIDO el gate one-shot de 2026-04-28.
            //
            // El gate era `if (!bHoverActive) goto end_tick_inc;`, y bHoverActive
            // es one-shot (lo cierra el latch s_clickCycleConsumed en la línea
            // ~1183). Eso convertía MANTENER el botón en un click único: el héroe
            // daba un paso y se plantaba.
            //
            // El original NO hace eso. MoveHero @ 0x004ACEF0:
            //     bVar32 = MouseLButtonPush != false;
            //     if (bVar32) MouseLButtonPush = false;      // consume el flanco
            //     bVar31 = MouseLButton != false || bVar32;  // ESTADO SOSTENIDO || flanco
            //     if (MouseLButton == false && !bVar32) { ...sale sin mover... }
            // bVar31 — lo que habilita el movimiento — es el estado en tiempo real
            // del botón O el flanco de bajada. Mantener el botón camina de forma
            // continua: es el comportamiento clásico del MU.
            //
            // El comentario del fix viejo decía "sin esto el hero seguía al mouse
            // continuamente sin click": seguir al mouse mientras el botón está
            // apretado ES lo correcto. El bug real era que DAT_083a42c4 quedaba
            // pegado en 1 tras soltar (de ahí el "sin click"); hoy WndProc lo
            // mantiene bien (WinMain.cpp:1182-1204), así que la causa ya no existe.
            //
            // La repetición la limita el debounce de la línea ~955
            // (DAT_00559bec <= DAT_07e11d28), igual que el original la limita con
            // MouseUpdateTimeMax <= MouseUpdateTime. Los gates de UI de abajo
            // (g_MouseOnWindow, s_clickStartedOnWindow) siguen intactos.
            //
            // 2026-08-17 (b): leer DAT_083a42c4 EN VIVO, no la copia bClickHeld
            // capturada en la línea ~1059. Las líneas ~1214-1216 limpian los flags
            // de click cuando el cursor pasa a estar sobre una ventana, y con la
            // copia vieja ese limpiado no tenía efecto hasta el frame siguiente:
            // manteniendo el botón y arrastrando el cursor sobre la UI, el ground
            // click seguía recalculando destino desde el píxel bajo el cursor y,
            // como la cámara sigue al héroe, el destino huía con ella → caminata
            // infinita. Con el estado en vivo el hold se corta en el acto, igual
            // que el original, que lee MouseLButton directo y no una copia.
            if (DAT_083a42c4 == 0 && !bClickEdge) goto end_tick_inc;
            // 2026-05-04: per IDA Player_InputTick:416,566 — block ground click
            // cuando el mouse está sobre cualquier panel abierto (MouseOnWindow=1). Sin
            // esto, clickear el botón [+] de stats o la X de cerrar del panel también
            // hacía caminar al jugador hacia esa posición de pantalla.
            if (g_MouseOnWindow) goto end_tick_inc;
            // IDA: el click al mundo NO cierra ventanas de NPC acá. Lo hace
            // SendMove (0x491C40) al mandar el movimiento: el personaje camina
            // y la ventana se cierra con su paquete (Combat.cpp,
            // SendMove_CloseWindows97k). MuEmu no rechaza el 0x10 con la
            // interfaz abierta (CGMoveRecv no la chequea).
            // 2026-05-05: También bloquear si el click se inició sobre window
            // (caso: user click skill cell, Chat_InputTick consume y resetea
            // DAT_07db870c → siguiente frame g_MouseOnWindow=0 pero el click
            // tail aún propagating como bHoverActive=true).
            if (s_clickStartedOnWindow) goto end_tick_inc;
            // 2026-05-05: hard gate — si la skill expanded list estuvo abierta
            // este frame O el frame anterior, ningún ground click vale.
            // Cubre el race entre Chat_InputTick reset y Player_InputTick check.
            {
                static char s_lastSkillMenu = 0;
                char skillMenuNow = (DAT_07db870c != '\0') ? (char)1 : (char)0;
                bool skillMenuActive = (skillMenuNow || s_lastSkillMenu);
                s_lastSkillMenu = skillMenuNow;
                if (skillMenuActive) {
                    int my = (int)DAT_083a4278;
                    int mx = (int)DAT_083a427c;
                    // si mouse está cerca de la skill bar zone, ignorar click
                    if (my >= 350 && my < 460 && mx >= 180 && mx < 460) {
                        goto end_tick_inc;
                    }
                }
            }
            { char d[64]; wsprintfA(d, "PIT GroundClick! 559c4c=%d c48=%d c54=%d",
                (int)SelectedNpc, (int)SelectedItem, (int)SelectedOperate);
              DbgLogPublic(d); }
            {
                SHORT shift = GetAsyncKeyState(0x10);
                bool shiftHeld = ((char)((unsigned short)shift >> 8) == -0x80);
                if (!shiftHeld) {
                    // BUG-FIX 2026-04-29: reset closest-hit sentinel ANTES de
                    // cada scan. Sin esto, FUN_00512d40 rechaza todos los hits
                    // si DAT_083a4120 (t_max) quedó stale de un frame previo.
                    extern void FUN_00512d30(void);
                    FUN_00512d30();
                    DAT_07eab1fc = 0;             // reset hit flag
                    FUN_004f9ac0('\x01');         // iterate tiles + raycast

                    char cHit = (DAT_07eab1fc != 0) ? '\x01' : '\0';

                    { char d[128]; wsprintfA(d,
                        "PIT pickRay hit=%d picked=(%d,%d) DAT_080ab288=%08x",
                        (int)cHit,
                        (int)*(float*)&DAT_080ab288, (int)*(float*)&DAT_080ab28c,
                        DAT_080ab288);
                      DbgLogPublic(d); }

                    if (cHit != '\0') {
                        // BUG-FIX 2026-04-30: el "fix 2026-04-28" estaba MAL.
                        // En realidad DAT_080ab288/28c YA viene en grid coords
                        // (e.g. 218.0) — el picker (FUN_004f9ac0) hace la
                        // conversión interna con _DAT_005524f0.  Dividir otra
                        // vez por 100 producía siempre gridX=2 gridY=0 (218/100
                        // → 2 truncado) y bloqueaba el movimiento porque
                        // pathfind iba siempre al mismo destino imposible.
                        //
                        // Evidencia del log: pickWX=218.0 (grid 218), no 21800.
                        // Cast directo a int.
                        float pickWX = *(float*)&DAT_080ab288;
                        float pickWY = *(float*)&DAT_080ab28c;
                        DAT_07e016c0 = (DWORD)(int)pickWX;
                        DAT_07e016c4 = (DWORD)(int)pickWY;
                        { char d[128]; wsprintfA(d,
                            "PIT pickGrid wx=%d wy=%d gridX=%d gridY=%d",
                            (int)pickWX, (int)pickWY,
                            (int)DAT_07e016c0, (int)DAT_07e016c4);
                          DbgLogPublic(d); }

                        // DAT_07e11d64 es `DontMove` (0x07E11D64 en el binario), NO un
                        // "walkable": es COSMETICO, sólo elige el sprite del cursor
                        // (10 = prohibido / 3 = mover) en el render del puntero. No
                        // bloquea nada, ni acá ni en el original — que también lo usa
                        // sólo para eso (3 xrefs: dos escrituras en MoveHero y una
                        // lectura en el dibujo del cursor).
                        //
                        // MoveHero @ 0x004ACEF0 lo calcula con un operador coma:
                        //     if ((TerrainWall[idx] < 8) ||
                        //        (DontMove = true, (TerrainWall[idx] & 0x20) == 0x20)) {
                        //         DontMove = false;
                        //     }
                        // o sea DontMove = true  <=>  attr >= 8 && !(attr & 0x20),
                        // que es exactamente lo que hace la forma de abajo. Es fiel;
                        // el nombre "walkability" del comentario viejo confundía.
                        int terrIdx = DAT_07e016c0 + DAT_07e016c4 * 0x100;
                        unsigned char terrAttr = ((unsigned char*)&DAT_0838bc70)[terrIdx];
                        if (terrAttr < 8 || (terrAttr & 0x20) == 0x20)
                            DAT_07e11d64 = 0;   // DontMove = false
                        else
                            DAT_07e11d64 = 1;   // DontMove = true

                        // La rama de piso de 004ACEF0 no rechaza una animación de
                        // acción/ataque activa. Resuelve el click sobre el terreno
                        // y reemplaza la acción pendiente por un movimiento.
                        // El gate viejo del DLL companion sobre +0x2EC y la acción
                        // ranges made a completed cast permanently block all
                        // subsequent ground clicks.
                        {
                            int srcX = *(int*)(ent + 0x388);
                            int srcY = *(int*)(ent + 0x38c);

                            { char d[160]; wsprintfA(d,
                                "PIT pathfind src=(%d,%d) dst=(%d,%d) terrAttr=%02X dontMove=%d",
                                srcX, srcY, (int)DAT_07e016c0, (int)DAT_07e016c4,
                                terrAttr, (int)DAT_07e11d64);
                              DbgLogPublic(d); }

                            unsigned int ok = Path_FindRoute(srcX, srcY,
                                                            DAT_07e016c0, DAT_07e016c4,
                                                            ent + 0x354, 0.0f);
                            { char d[80]; wsprintfA(d,
                                "PIT pathfind result ok=%d wp_count=%d",
                                (int)(char)ok, (int)*(unsigned char*)(ent + 0x356));
                              DbgLogPublic(d); }
                            if ((char)ok != '\0') {
                                *(unsigned char*)(ent + 0x2ed) = 0;
                                DbgLogPublic("PIT calling Combat_SendMovePathPacket (send move)");
                                Combat_SendMovePathPacket((int)ent, (int)ent);
                                goto end_tick_inc;
                            }
                        }
                        DAT_07e11d28 = 0;
                    }
                }
            }
                }
    }
    // 2026-09-02 FIX (hay que clickear varias veces para caminar): aca habia
    // un `else` que, cuando el gate de debounce bloqueaba, hacia
    //     MouseLButtonPush = 0; MouseLButton = 0;
    //
    // IDA Player_InputTick (0x4ACEF0 L586-588) NO borra nada en ese camino:
    //     if ( MouseUpdateTime < MouseUpdateTimeMax || byte_7E11DC0 )
    //         goto LABEL_390;            // == ++MouseUpdateTime; salir
    // Los dos flags solo se limpian en el anti-AFK de L607-611 (boton
    // sostenido 3600 s). O sea el click PENDIENTE sobrevive al bloqueo y lo
    // procesa el primer tick que pase el gate.
    //
    // Al borrarlos se perdia el click: con el boton sostenido, el primer tick
    // bloqueado mataba MouseLButton y el caminar continuo se cortaba; con un
    // click corto se perdia el pulso entero y habia que volver a clickear.
    // Medido en debug.log: 161 WM_LBUTTONDOWN -> solo 57 GroundClick.

end_tick_inc:
    DAT_07e11d28 = DAT_07e11d28 + 1;

end_tick:
    // ── Per-frame entity state update ─────────────────────────────────────────
    Combat_DispatchHeroSkillAttack(DAT_07abf5d8);

    // ── HeroTile: atributo de terreno bajo el HÉROE ───────────────────────────
    // IDA Player_InputTick L570-582:
    //     v227 = (__int64)*(float *)(Hero + 16) / 100
    //          + (((__int64)*(float *)(Hero + 20) / 100) << 8);
    //     clamp [0, 0xFFFF]
    //     HeroTile = TerrainMappingLayer1[v227];
    //
    // 2026-08-16: el port tenía DOS errores acá y por eso `HeroTile` era basura:
    //   1. Leía `DAT_05826e08` (**WorldTime**) en AMBOS ejes, no la posición del
    //      héroe. El comentario lo admitía ("simplified").
    //   2. Componía el índice invertido (`gy + gx*256` en vez de `gx + gy*256`).
    //
    // `HeroTile` es lo que gatea el **techo transparente**: `MoveObjects`
    // (0x4FDC00) pone AlphaTarget=0 en los objetos de techo cuando el héroe
    // entra bajo uno — Lorencia (World 0) tipos 125/126 con HeroTile==4, y
    // Devias (World 2) tipos 81/82/96/98/99 con HeroTile==3 o >=10. Ese bloque
    // YA estaba portado y activo; sólo recibía un HeroTile sin sentido, así que
    // el techo nunca se volvía transparente y tapaba al personaje.
    // Lo leen además Render_Frame (2da pasada de SkillEffect_Render) y
    // Scene_CharSelect_Nav.
    {
        const char* hero = (const char*)DAT_07abf5d8;
        if (hero) {
            int gx = (int)(*(const float*)(hero + 16)) / 100;   // world X → celda
            int gy = (int)(*(const float*)(hero + 20)) / 100;   // world Y → celda
            int idx = gx + (gy << 8);
            if (idx < 0)       idx = 0;
            if (idx > 0xffff)  idx = 0xffff;
            DAT_07e118e8 = ((unsigned char*)&DAT_080bb2b4)[idx];
        }
    }
}
