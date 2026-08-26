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

static bool HUD_IsQuestPanelOpenRuntime(void);
static bool HUD_IsGoldenArcherPanelRuntime(void);
static bool HUD_IsInventoryFamilyActive(void);
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
extern "C" int g_MouseOnWindow = 0;

// IDA `Attacking` — estado del auto-ataque: -1 = ninguno, 1 = ataque iniciado
// desde Player_InputTick (L942), 2 = desde Attack (0x49CBF0 L1323).
// Lo resetean a -1 InitGame, ReceiveTeleport, CheckGate y varios paths de
// Attack. Con -1 (el default) el head-tracking hacia el mouse queda ACTIVO,
// que es el comportamiento normal fuera de combate.
extern "C" int g_Attacking = -1;

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
    g_MouseOnWindow = 0;

    int mx = (int)DAT_083a427c;
    int my = (int)DAT_083a4278;

    if (g_ChatLB_MouseOnWindow) { g_MouseOnWindow = 1; return; }

    // Bottom HUD (y >= 432) — per IDA Game_CharSelectTick:298.
    if (my >= 432) { g_MouseOnWindow = 1; return; }

    // Top quick buttons / small HUD icons rendered by Render_QuickButtons.
    if (HUD_IsGuildCreationRuntime()) {
        int btnX = (int)((float)DAT_07ea5b1c + _DAT_005524fc);
        int btnY = (int)((float)DAT_07ea5b20 + _DAT_00552ca4);
        if (mx >= btnX && mx < btnX + 70 && my >= btnY && my < btnY + 21) {
            g_MouseOnWindow = 1; return;
        }
        btnX += (int)_DAT_005524f0;
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

static void HUD_BottomBar_HitTest(void)
{
    if (DAT_005615c0 != 5) return;          // g_GameState: only in-game

    // 2026-04-30: detecta el click por flanco. DAT_083a413c se mantiene en 1 durante
    // multiple frames until some other handler consumes it. Without
    // edge-detection my toggle fires every frame while 413c==1, flipping
    // the panel back and forth and netting to no visible change.
    static DWORD s_prev413c = 0;
    DWORD curr413c = DAT_083a413c;
    bool risingEdge = (s_prev413c == 0) && (curr413c != 0);
    s_prev413c = curr413c;

    if (!risingEdge) return;

    int mx = (int)DAT_083a427c;             // 640-space mouse X
    int my = (int)DAT_083a4278;             // 480-space mouse Y

    bool consumed = false;

    // Party (348..372, 452..476)
    if (mx >= 348 && mx < 372 && my >= 452 && my < 476) {
        if (DAT_07eaa115) {
            DAT_07eaa115 = 0;
        } else {
            DAT_07eaa115 = 1;
            DAT_07eaa114 = 0; // guild list closes when party opens
            DAT_07eaa124 = 0; // guild creator closes when party opens
        }
        consumed = true;
    }
    // Character (379..403, 452..476)
    else if (mx >= 379 && mx < 403 && my >= 452 && my < 476) {
        DAT_07eaa116 = (DAT_07eaa116 == 0) ? (char)1 : (char)0;
        consumed = true;
    }
    // Inventory (410..434, 452..476)
    else if (mx >= 410 && mx < 434 && my >= 452 && my < 476) {
        DAT_07eaa117 = HUD_IsInventoryFamilyActive() ? (char)0 : (char)1;
        consumed = true;
    }
    // Guild (582..634, 459..477)
    else if (mx >= 582 && mx < 634 && my >= 459 && my < 477) {
        if (DAT_07eaa114 || DAT_07eaa124) {
            DAT_07eaa114 = 0;
            DAT_07eaa124 = 0;
        } else {
            DAT_07eaa114 = 1;
            DAT_07eaa115 = 0; // party list closes when guild opens
        }
        consumed = true;
    }

    if (consumed) {
        DAT_083a413c = '\0';                // consume the click
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
                wsprintfA(b, "CHATMODE ON [%s]  ime=%02X shop=%d inv=%d npcActive=%d",
                          tag, (BYTE)DAT_07e11d71, (int)DAT_07eaa118,
                          (int)DAT_07eaa117, g_NpcTalkActive);
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
    int kC = FUN_0047ec20(0x43); // 'C'  Character info
    int kV = FUN_0047ec20(0x56); // 'V'  Inventory (alt)
    int kI = FUN_0047ec20(0x49); // 'I'  Inventory
    int kG = FUN_0047ec20(0x47); // 'G'  Guild
    int kP = FUN_0047ec20(0x50); // 'P'  Party

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
    auto CloseNpcWindowsIfAny = [&]() {
        if (DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a || DAT_07eaa11b || DAT_07eaa128 || g_NpcTalkActive) {
            extern void __cdecl CloseInventoryRelatedWindows(void);
            CloseInventoryRelatedWindows();
            BYTE closePkt[4] = { 0xC1, 0x03, 0x31, 0 };
            g_NpcTalkActive = 0;
            if (DAT_055ca168 != 0xFFFFFFFF)
                send((SOCKET)DAT_055ca168, (const char*)closePkt, 3, 0);
            DbgLogPublic("HKT CLOSE-NPC (C/G/P panel)");
        }
    };

    if (kC) {
        if (DAT_07eaa116) DAT_07eaa116 = 0;
        else { CloseNpcWindowsIfAny(); DAT_07eaa116 = 1; }
    }
    if (kG) {
        if (DAT_07eaa114 || DAT_07eaa124) {
            DAT_07eaa114 = 0;
            DAT_07eaa124 = 0;
        }
        else {
            CloseNpcWindowsIfAny();
            DAT_07eaa114 = 1;
            DAT_07eaa115 = 0; // close Party
            // 2026-08-15 BUG-FIX (abrir el panel de guild con G desconectaba):
            // el opcode 0x52 pide Encrypt=0 en HackPacketCheck.txt, o sea frame
            // C1 plano. Enviarlo como C3 (Net_SendSmallPacket) hace que el
            // server responda "Packet encryption error" y cierre la sesión.
            const BYTE guildListPkt[3] = { 0xC1, 0x03, 0x52 };
            Net_SendC1Packet(guildListPkt, sizeof(guildListPkt));
        }
    }
    if (kP) {
        if (DAT_07eaa115) DAT_07eaa115 = 0;
        else {
            CloseNpcWindowsIfAny(); DAT_07eaa115 = 1; DAT_07eaa114 = 0; DAT_07eaa124 = 0; /* close Guild */
            // 2026-08-25 FIX (el panel de party se abria con los datos en 0):
            // la tecla P solo hacia el toggle LOCAL — nunca se pedia la lista,
            // asi que el 0x42 no llegaba y `Party[]`/`PartyNumber` quedaban con
            // lo que hubiera (normalmente ceros).
            //
            // El envio existia en `Chat_InputTick` (seccion 13 de IDA), pero esa
            // seccion se removio entera el 2026-05-08 al arreglar el
            // doble-toggle de C/V — y el toggle que quedo aca nunca lo repuso.
            // Al guild le paso lo mismo y se corrigio el 2026-08-15; el party
            // quedo pendiente.
            //
            // Per IDA: limpiar PartyNumber y pedir la lista. El 0x42 pide
            // Encrypt=0 (indice 66), o sea C1 plano — mandarlo como C3 haria que
            // el server cierre la conexion, igual que pasaba con el guild.
            PartyNumber = 0;
            const BYTE partyListPkt[3] = { 0xC1, 0x03, 0x42 };
            Net_SendC1Packet(partyListPkt, sizeof(partyListPkt));
        }
    }
    if (kV || kI) {
        // 2026-07-27 FIX: el gate era HUD_IsInventoryFamilyActive(), que incluye
        // CharacterOpened/Party/Guild → con el panel de Character abierto, tocar V
        // caía en la rama "cerrar todo" y cerraba TODOS los menús en vez de
        // togglear el inventario. Per IDA (Chat_InputTick sección 15) la tecla
        // I/V togglea InventoryOpened; al cerrar arrastra las ventanas de NPC.
        if (DAT_07eaa117 || DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a ||
            DAT_07eaa11b || DAT_07eaa128 || g_NpcTalkActive) {
            // 2026-07-27 FIX: con la tienda abierta, apretar I/V cerraba solo
            // InventoryOpened y dejaba la tienda abierta (y el server con
            // Interface.use=1 → no dejaba abrir otra). Ahora cierra toda la
            // familia de ventanas de NPC y avisa al server con el close 0x31,
            // igual que Escape / click-para-mover.
            bool hadNpcWindow = (DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a ||
                                 DAT_07eaa11b || DAT_07eaa128 || g_NpcTalkActive);
            extern void __cdecl CloseInventoryRelatedWindows(void);
            CloseInventoryRelatedWindows();     // limpia Shop/Warehouse/Mix/Trade + pools
            DAT_07eaa117 = 0;                    // InventoryOpened
            // (NO tocar CharacterOpened: I/V sólo maneja el inventario y las
            //  ventanas de NPC; el panel de Character lo togglea la tecla C.)
            if (hadNpcWindow) {
                BYTE closePkt[4] = { 0xC1, 0x03, 0x31, 0 };
                g_NpcTalkActive = 0;
                if (DAT_055ca168 != 0xFFFFFFFF)
                    send((SOCKET)DAT_055ca168, (const char*)closePkt, 3, 0);
            }
        } else {
            DAT_07eaa117 = 1;
        }
    }
}

// FUN_004acef0 — Player_InputTick (0x004acef0, 1688 lines)
//
// Procesador de input del jugador por frame. Se llama desde el camino de render del HUD/UI en cada frame.
// Responsibilities:
//   1. Cooldown gate (DAT_07e11d1c must be <= 0x1e)
//   2. Second-password auto-fill from hover entity name
//   3. Camera update + facing angle packet [0xC1][0x18][0x66]
//   4. Sub-tick via FUN_004ac140
//   5. Movement debounce (DAT_07e11d28 >= DAT_00559bec and !DAT_07e11dc0)
//   6. Animation state exit: swimming, normal walk, cancel
//   7. Click-on-character-select: slot copy + sends [0xC1][0x24] encrypted packet
//   8. Click-on-mob/player (SelectedCharacter): pathfind + FUN_00491c40
//   9. Click-on-NPC (SelectedNpc): alt pathfind
//  10. Click-on-special-object (SelectedOperate): pathfind + entity_type lookup
//  11. Ground click (ray cast FUN_004f9ac0 + FUN_004f8480): terrain check + pathfind
//  12. Entity state update FUN_0049cbf0
//  13. Hover terrain attribute → DAT_07e118e8
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
//   Char-select:  [0xC1][0x24] + slot data from entity+0x97 (0x44 bytes)

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

// 2026-07-25 (#2 shops): envía el request "hablar con NPC" (client→server 0x30,
// PMSG_NPC_TALK_RECV: [C1][05][30][idxHi][idxLo]). El server (CGNpcTalkRecv)
// responde con 0x30 ReceiveTalk (abre shop/baúl/chaos) + 0x31 lista de items.
// Mismo pipeline de encriptación que el attack in-range 0x15 (chain-XOR
// s_LoginKey + MuEmu::EncryptSend + send raw) que ya funciona end-to-end.
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);
static void SendNpcTalkRequest(WORD npcEntityId)
{
    unsigned char pkt[8];
    pkt[0] = 0xC1;
    pkt[1] = 0x05;
    pkt[2] = 0x30;
    pkt[3] = (unsigned char)((npcEntityId >> 8) & 0xFF);
    pkt[4] = (unsigned char)(npcEntityId & 0xFF);
    // FIX 2026-07-25: 0x30 (NPC talk) requiere C3 (encriptado + serial), no C1.
    // El server (HackPacketCheck.cpp:143 CheckPacketHack) cierra la conexión si
    // `lpInfo->Encrypt != encrypt` — cada opcode define su encriptación esperada.
    // Los moves (0x10) van C1, pero 0x30 exige C3 como el login. Net_SendSmallPacket
    // hace chain-XOR + serial-stomp + wrap C3 (mismo camino que el login F1).
    // NO pre-XORear: Net_SendSmallPacket ya aplica el chain-XOR internamente.
    Net_SendSmallPacket(pkt, 5);
    // 2026-07-27: marcar que hay un diálogo de NPC potencialmente abierto en el
    // server (Interface.use=1). Cubre shop/warehouse/chaos PERO también NPCs que
    // no setean flag local (Golden Archer, quest, etc.) — sin esto, cerrar esos
    // diálogos no mandaba el 0x31 y el server quedaba con Interface.use=1,
    // bloqueando abrir cualquier otra tienda.
    g_NpcTalkActive = 1;
    char ab[80];
    wsprintfA(ab, "PIT NPC-TALK send (C3): npcId=%d", (int)npcEntityId);
    DbgLogPublic(ab);
}

void __cdecl FUN_004acef0(void)
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
    if (DAT_005615c0 == 5) {
        DAT_07d78094 = (g_MouseOnWindow != 0) ? 1 : 0;
    }
    HUD_BottomBar_HitTest();

    // ── Guard: entity visibility / renderable flag ─────────────────────────────
    if (*(char*)((int)DAT_07abf5d8 + 0x2fd) != '\0')
        return;

    // ── Cooldown counter (HashTable obfuscation omitted) ─────────────────────
    // Original: HashTable manipulates DAT_07e11d1c; effective result is a decrement
    // then a range check.
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
    // pero `bec` se setea a `wpCount*3+4` cada vez que FUN_00491c40 envía un
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
                unsigned int moveOk = FUN_0043ea20(ent, '\x01');
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
                    // 2026-05-06: si hay action queued en c+0x2ed (set por
                    // click on mob/NPC con value 3=attack, 1=npc-talk, etc.),
                    // disparar Action(c, o) para procesar y mandar packet
                    // attack 0x15 / skill 0x19 / talk 0x30 al server.
                    //
                    // En IDA Action(c, o), tanto `c` como `o` son ENTITY ptr —
                    // c lee fields como c+0x2ED (action_queue), c+0x2F5 (attack
                    // pending), c+0x310 (target_idx). Para hero player, c y o
                    // son el mismo entity (DAT_07abf5d8). Anti-crash: validar
                    // que action queued sea uno de los cases válidos (1..5).
                    BYTE actionQueued = ent[0x2ed];
                    {
                        char wb[160];
                        wsprintfA(wb,
                            "PIT WALKER ARRIVE: 2ed=%d 559c70=%d 559ce8=%d hero=(%d,%d) anim=0x%02x",
                            (int)actionQueued, (int)DAT_00559c70,
                            (int)DAT_00559ce8,
                            (int)*(int*)(ent + 0x388), (int)*(int*)(ent + 0x38c),
                            (int)ent[0x105]);
                        DbgLogPublic(wb);
                    }
                    // 2026-05-06: en lugar del mini-attack inline (que ignoraba
                    // weapon-range, animation gates, position cache, etc),
                    // delegamos al port completo de Action() (FUN_0048d640
                    // case 2) SOLO para action=3 (attack). Otros valores de
                    // actionQueued (1=npc-talk, 2=pickup, 4=walk-final, 5=skill)
                    // dispararían cases 0,1,3,4 de Action() — esos NO están
                    // todavía completamente porteados y crashearon en runtime
                    // (user reportó AV addr=0x55D1D6 al moverse 2026-05-06).
                    if (actionQueued == 3) {
                        FUN_0048d640((DWORD)ent, (DWORD)ent);
                        // 2026-05-07: hard-clear queue post walker-arrival
                        // fire para que la SECONDARY TICK abajo no double-fire
                        // si Action() out-of-range no clean por sí sola.
                        *(unsigned char*)(ent + 0x2ed) = 0;
                    }
                    // 2026-07-25 (#2 shops): llegada a NPC (2ed==2) → mandar el
                    // request de talk 0x30. DAT_00559c70 = índice del NPC clickeado.
                    else if (actionQueued == 2) {
                        int npcIdx = (int)DAT_00559c70;
                        if (npcIdx >= 0 && npcIdx < 400) {
                            BYTE* npc = (BYTE*)(uintptr_t)DAT_07abf5d0 + npcIdx * 0x394;
                            if (npc[0] != 0)
                                SendNpcTalkRequest(*(WORD*)(npc + 0x1dc));
                        }
                        *(unsigned char*)(ent + 0x2ed) = 0;
                    }
                    // 2026-07-27: llegada al item (2ed==1) → mandar pickup 0x22.
                    else if (actionQueued == 1) {
                        // 2026-08-22: aca se releia SelectedItem EN VIVO al
                        // llegar.  IDA latchea el indice al hacer click
                        // (Player_InputTick L1278-1283: `ItemKey = v181;` junto
                        // con `c[749] = 1`) y `Action` (0x48D640) arma el pickup
                        // con ItemKey, no con SelectedItem.  Releerlo en vivo
                        // hace que, con varios items juntos, se levante el que
                        // este bajo el cursor AL LLEGAR y no el que se clickeo:
                        // el nombre flotante decia uno y entraba otro.
                        int itemSlotIdx = (int)ItemKey;
                        if (itemSlotIdx >= 0 && itemSlotIdx < 1000) {
                            BYTE* itemEnt = (BYTE*)&DAT_07e12840[0]
                                          + (uintptr_t)itemSlotIdx * 0x204;
                            if (itemEnt[72]) {   // active
                                unsigned short itemKey = (unsigned short)itemSlotIdx;
                                BYTE gp[6];
                                gp[0] = 0xC1; gp[1] = 0x05; gp[2] = 0x22;
                                gp[3] = (BYTE)((itemKey >> 8) & 0xFF);
                                gp[4] = (BYTE)(itemKey & 0xFF);
                                Net_SendSmallPacket(gp, 5);
                            }
                        }
                        *(unsigned char*)(ent + 0x2ed) = 0;
                    }
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

    // 2026-05-06: SECONDARY attack tick — si user click on mob already in
    // range, walker no se ejecuta (no path needed) y mi wireup arriba (que
    // está dentro del walker arrival branch) no dispara. Aquí check cada
    // tick si ent[+0x2ed]=3 y walker idle, dispara Action() completa.
    //
    // 2026-05-07 (v3) — FIRE-ONCE-PER-CLICK. Antes el SECONDARY TICK
    // disparaba Action() cada 200ms mientras 0x2ed==3 + walker idle. Esto
    // creaba auto-attack: usuario clickea mob → 0x2ed=3 → mientras user
    // sostenía el botón izq, safety guard skipea (bClickHeld=true), queue
    // queda armado, SECONDARY fires every 200ms → "ataca solo con hover".
    //
    // Fix: cada vez que ent[0x2ed] entra al value 3 (click nuevo o walker
    // arrival que lo dejó armado), permitir UN solo fire del Action() y
    // luego HARD-CLEAR el queue. La próxima fire requiere que el queue
    // caiga a 0 primero (que ahora pasa siempre tras el hard-clear) y
    // vuelva a 3 por un nuevo click.
    //
    // Crítico: hard-clear ent[0x2ed]=0 después del fire para que el
    // safety guard no necesite ejecutar (que skipea cuando bClickHeld=1).
    {
        unsigned char *ent = (unsigned char*)DAT_07abf5d8;
        if (ent && ent[0x2ed] == 3 && ent[0x356] == 0) {
            {
                char dbg[200];
                wsprintfA(dbg, "PIT SECONDARY TICK (1-shot): 2ed=%d c50=%d ce8=%d 4124=%d 42c4=%d 413c=%d bClickEdge=%d bClickHeld=%d",
                    (int)ent[0x2ed], (int)SelectedCharacter, (int)DAT_00559ce8,
                    (int)DAT_083a4124, (int)DAT_083a42c4, (int)DAT_083a413c,
                    0, 0);  // bClickEdge/bClickHeld read later — log raw flags
                DbgLogPublic(dbg);
            }
            FUN_0048d640((DWORD)ent, (DWORD)ent);
            // Limpia la cola de una después de disparar — elimina el auto-fire mientras
            // mouse held. Si Action() in-range ya lo limpió, este write
            // es no-op idempotente.
            ent[0x2ed] = 0;
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
        bool gatePass = (walkerIdle || DAT_00559bec <= DAT_07e11d28) && DAT_07e11dc0 == '\0';
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
    if ((walkerIdle || DAT_00559bec <= DAT_07e11d28) && DAT_07e11dc0 == '\0') {

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
        // Bug previo: ent[0x2ed] (3=attack/2=pickup/1=npc/4=walk-final/5=skill)
        // quedaba armado entre frames. Si user clickea entity → action set →
        // walker walks → Action() falla → no clear → secondary tick fires
        // Action() cada frame. User mueve mouse a otro entity → Action() puede
        // dispararse contra NUEVO target sin nuevo click. User reportó:
        // "ataca con hover incluso con NPCs" 2026-05-07.
        //
        // Clear cualquier valor del queue (no solo ==3) para cubrir todos los
        // action types. Walker activo (ent[0x356]>0) significa que el user
        // tiene movimiento en progreso → preservar queue para el walker-arrival.
        if (DAT_07abf5d8 && !bClickEdge && !bClickHeld) {
            BYTE* hero = (BYTE*)DAT_07abf5d8;
            if (hero[0x356] == 0 && hero[0x2ed] != 0) {
                hero[0x2ed] = 0;   // disarm stale action queue when idle + no click
            }
        }

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
        // bHoverActive=false (vía bHoverOrClick que también incluye DAT_083a42c4).
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
        // que no se consumió bien puede dejar este flag en 1, haciendo que
        // bHoverOrClick siga siendo true y ciertos paths (e.g. line 676
        // char-select packet) se disparen al pasar el mouse.
        if (!bClickHeld && !bClickLatched) {
            DAT_083a42c4 = 0;
        }
        bool bHoverOrClick = (DAT_083a42c4 != '\0') || bHoverActive;

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

        // ── Click on char-select entity ───────────────────────────────────────
        // Conditions: hover match, secondary target active, sub-state not 6
        if (DAT_00559c5c != '\0'
            && DAT_0055a7ac != 6
            && DAT_00559c58 == 1
            && SelectedCharacter != -1)
        {
            bHoverOrClick = true;
        }

        _DAT_07e11d50 = (DAT_05826e08 - _DAT_07e11d4c) * _DAT_00552890;

        if (_DAT_07e11d50 < _DAT_00552b6c && bHoverOrClick) {
            // Copia los datos del slot de char-select de entity+0x97 al buffer DAT_07e91350
            // (0x11 DWORDs = 68 bytes = char name/slot)
            if (!DAT_07eaa165) {
                DAT_07eaa165 = '\x01';

                unsigned char *slotSrc = (unsigned char*)DAT_07cf1ffc + 0x97;
                memcpy((void*)DAT_07e91350, slotSrc, 0x44);

                // Clear slot in entity
                *(short*)((unsigned char*)DAT_07cf1ffc + 0x97)  = -1;
                *(DWORD*)((unsigned char*)DAT_07cf1ffc + 0x98)   = 0;

                // Resetea el estado de la entidad y setea los flags de entrada al mundo
                FUN_0045c130((int)DAT_07abf5d8);
                FUN_0045c720((int)DAT_07abf5d8);
                DAT_07ea9800  = (DWORD)&DAT_07ea8410;
                DAT_07ea5b18  = 1;
                DAT_07e11e78  = 0;

                // Build [0xC1][0x24] char-select packet with slot data
                // (payload: slot index + name from DAT_07e91350 buffer, XOR-encoded)
                // El original arma un payload de varios bytes hasta 0x400 y después lo codifica con FUN_0053cc30.
                // Buffer layout: bytes 0,4,0x1a,0x1b are slot index fields.
                unsigned char *slotBuf = (unsigned char*)DAT_07e91350;
                unsigned char payload[8];
                memset(payload, 0, sizeof(payload));
                payload[0] = 0xC1;
                payload[1] = 1;            // len placeholder
                payload[2] = 0x24;
                payload[3] = slotBuf[0];   // slot index LSB  (DAT_07e91350 byte 0)
                payload[4] = slotBuf[4];   // field B          (DAT_07e91354)
                payload[5] = slotBuf[0x1a];// field C          (DAT_07e9136a)
                payload[6] = slotBuf[0x1b];// field D          (DAT_07e9136b)
                payload[7] = 0;

                // Aleatoriza el contador de secuencia y codifica vía FUN_0053cc30
                int rnd = rand();
                unsigned int rawLen = 7;
                int encLen = FUN_0053cc30(0, payload + 1, rawLen);
                if (encLen < 0x100) {
                    char outbuf[256];
                    outbuf[0] = (char)0xC3;
                    outbuf[1] = (char)(encLen + 2);
                    FUN_0053cc30((int)(outbuf + 2), payload + 1, rawLen);
                    SendPacket(outbuf, encLen + 2);
                } else {
                    char outbuf[256];
                    outbuf[0] = (char)0xC4;
                    outbuf[1] = (char)((encLen + 3) >> 8);
                    outbuf[2] = (char)(encLen + 3);
                    FUN_0053cc30((int)(outbuf + 3), payload + 1, rawLen);
                    SendPacket(outbuf, encLen + 3);
                }
            }
        }

        // NO resetear MouseUpdateTime acá. En 004ACEF0 el reset va adentro de
        // las ramas de acción concretas (por ejemplo LABEL_190 / camino fallido),
        // después de que un click fue aceptado. Resetearlo incondicionalmente en
        // este punto impedía que el contador de debounce llegara nunca al
        // umbral de SendMove mientras hubiera una ruta vieja presente.

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
                if (SelectedCharacter > -1 && bClickEdge) {
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

                    // 2026-05-07: simplified — siempre pathfind. Si target ya
                    // está en range, pathfind devuelve path corto/vacío y el
                    // walker llega rápido. Si está lejos, walker walks. Antes
                    // se gateaba por `pathOk = FUN_004830b0(...)`; si ese
                    // helper retornaba 0, nada se hacía Y el secondary tick
                    // disparaba Action() en place sin movimiento.
                    unsigned int ok2 = FUN_0043f3e0(srcX, srcY,
                                                     dstX, dstY,
                                                     ent + 0x354, 0.0f);
                    if ((char)ok2 != '\0') {
                        // Pathfind successful → start walker
                        FUN_00491c40((int)ent, (int)ent);
                    } else {
                        // Pathfind failed (target unreachable) → send move
                        // direct to current pos como fallback. Walker stays
                        // idle, secondary tick chequeará distance al firar
                        // Action() (case 2 con out-of-range branch).
                        char chk = FUN_0048ba70();
                        if (chk != '\0') {
                            Send_MovePacket_Player_legacy_stub();
                        }
                    }
                    goto end_tick_inc;
                }
            }
        }

        // ── Alt-target: NPC/item (SelectedNpc != -1) ────────────────────────
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

                    // 2026-07-25 (#2 shops): si el NPC ya está en rango de talk
                    // (server exige ±5 tiles — CGNpcTalkRecv L261), mandar el
                    // request 0x30 YA. Si está lejos, se encola (2ed=2) y el
                    // walker-arrival lo dispara al llegar.
                    {
                        int hgx = *(int*)(ent + 0x388), hgy = *(int*)(ent + 0x38c);
                        int ngx = *(int*)(tgtBase + 0x388), ngy = *(int*)(tgtBase + 0x38c);
                        int ddx = (hgx - ngx < 0) ? (ngx - hgx) : (hgx - ngx);
                        int ddy = (hgy - ngy < 0) ? (ngy - hgy) : (hgy - ngy);
                        if (((ddx > ddy) ? ddx : ddy) <= 4) {
                            SendNpcTalkRequest(*(WORD*)(tgtBase + 0x1dc));
                            *(unsigned char*)(ent + 0x2ed) = 0;  // no encolar walk
                            goto end_tick_inc;
                        }
                    }

                    int srcX = *(int*)(ent + 0x388);
                    int srcY = *(int*)(ent + 0x38c);
                    int dstX = *(int*)(tgtBase + 0x388);
                    int dstY = *(int*)(tgtBase + 0x38c);
                    // 2026-05-07 BUG-FIX: dst grid coords del NPC, NO el animTick.
                    DAT_07e016c0 = (DWORD)dstX;
                    DAT_07e016c4 = (DWORD)dstY;

                    unsigned int ok = FUN_0043f3e0(srcX, srcY,
                                                    dstX, dstY,
                                                    ent + 0x354, 0.0f);
                    if ((char)ok == '\0') {
                        Send_MovePacket_Player_legacy_stub();
                    } else {
                        FUN_00491c40((int)ent, (int)ent);
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

                // 2026-07-27: si el héroe ya está sobre/al lado del item, mandar
                // el pickup 0x22 AHORA (no hay walk → el arrival no dispara).
                // CGItemGetRecv: [C1][05][22][idxH][idxL], C3. El índice del pool
                // ES el map item index (el 0x20 handler guarda en pool[key*0x204]).
                {
                    int adx = srcX - dstX; if (adx < 0) adx = -adx;
                    int ady = srcY - dstY; if (ady < 0) ady = -ady;
                    if (adx <= 1 && ady <= 1) {
                        if (itemEnt[72]) {   // active
                            unsigned short itemKey = (unsigned short)itemSlotIdx;
                            BYTE gp[6];
                            gp[0] = 0xC1; gp[1] = 0x05; gp[2] = 0x22;
                            gp[3] = (BYTE)((itemKey >> 8) & 0xFF);
                            gp[4] = (BYTE)(itemKey & 0xFF);
                            Net_SendSmallPacket(gp, 5);
                        }
                        *(unsigned char*)(ent + 0x2ed) = 0;
                        goto end_tick_inc;
                    }
                }

                unsigned int ok = FUN_0043f3e0(srcX, srcY,
                                                dstX, dstY,
                                                ent + 0x354, 0.0f);
                if ((char)ok == '\0') {
                    Send_MovePacket_Player_legacy_stub();
                    *(unsigned char*)(ent + 0x2ed) = 0;
                } else {
                    FUN_00491c40((int)ent, (int)ent);
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
            // 2026-07-25 (#2 shops): si hay una ventana de NPC abierta (shop/
            // warehouse/chaos/trade) y el user clickea el MUNDO (fuera del panel),
            // cerrarla en vez de moverse.  Mandar el move con Interface.use=1 en
            // el server hace que lo rechace → desconexión.  Comportamiento MU:
            // clickear afuera cierra el diálogo del NPC.  El close 0x31 va en C1
            // (el talk 0x30 es C3, pero el close es C1 — HackPacketCheck).
            if (DAT_07eaa118 || DAT_07eaa119 || DAT_07eaa11a || DAT_07eaa11b || DAT_07eaa128 || g_NpcTalkActive) {
                // 2026-08-24 FIX (issue #15, "la segunda tienda ya no vende"): este
                // bloque disparaba tambien con el boton MANTENIDO, no solo con un
                // click nuevo. El talk 0x30 se manda al LLEGAR al NPC (actionQueued
                // == 2, mas arriba en este mismo tick) y deja `g_NpcTalkActive = 1`;
                // si el usuario venia sosteniendo el boton del click-to-move, el gate
                // lo veia activo en el MISMO tick y mandaba el close. El log lo
                // mostraba con el mismo milisegundo:
                //     [1748718953] PIT NPC-TALK send (C3): npcId=15
                //     [1748718953] PIT CLOSE-NPC (move): sending 0x31 close
                // El server procesaba talk (TargetShopNumber=15, Interface.use=1) y
                // acto seguido close, que hace `TargetShopNumber = -1` +
                // `Interface.use = 0` (NpcTalk.cpp:330-332). El cliente ya habia
                // recibido el 0x30 y mostraba la tienda, pero toda venta caia en el
                // `if (SHOP_RANGE(lpObj->TargetShopNumber) == 0) return;` de
                // CGItemSellRecv -> 0x33 con result 0. De ahi "la primera tienda
                // vende y la segunda no": dependia de si se solto el boton antes de
                // llegar caminando al NPC.
                //
                // Un boton que venia sostenido desde ANTES de que la ventana se
                // abriera no es "el usuario clickeo afuera". Sin edge no cerramos, y
                // tampoco movemos (que es lo que este bloque venia a evitar).
                if (!bClickEdge) goto end_tick_inc;
                extern void __cdecl CloseInventoryRelatedWindows(void);
                CloseInventoryRelatedWindows();          // limpia Shop/Warehouse/Mix/Trade + pools
                DAT_07eaa117 = 0;                         // InventoryOpened
                BYTE closePkt[4] = { 0xC1, 0x03, 0x31, 0 };
                {
                    char cb[96];
                    wsprintfA(cb, "PIT CLOSE-NPC (move): sock=%08X npcActive=%d sending 0x31 close",
                              (unsigned)DAT_055ca168, g_NpcTalkActive);
                    DbgLogPublic(cb);
                }
                g_NpcTalkActive = 0;
                if (DAT_055ca168 != 0xFFFFFFFF)
                    send((SOCKET)DAT_055ca168, (const char*)closePkt, 3, 0);
                goto end_tick_inc;                        // este click sólo cierra; no mueve
            }
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

                            unsigned int ok = FUN_0043f3e0(srcX, srcY,
                                                            DAT_07e016c0, DAT_07e016c4,
                                                            ent + 0x354, 0.0f);
                            { char d[80]; wsprintfA(d,
                                "PIT pathfind result ok=%d wp_count=%d",
                                (int)(char)ok, (int)*(unsigned char*)(ent + 0x356));
                              DbgLogPublic(d); }
                            if ((char)ok != '\0') {
                                *(unsigned char*)(ent + 0x2ed) = 0;
                                DbgLogPublic("PIT calling FUN_00491c40 (send move)");
                                FUN_00491c40((int)ent, (int)ent);
                                goto end_tick_inc;
                            }
                        }
                        DAT_07e11d28 = 0;
                    }
                }
            }
        } else if (bClickEdge) {
            // 2026-05-06: bClickEdge en vez de bHoverActive (mismo fix anti
            // spurious-from-stale-latch).
            // ── Special object target (SelectedOperate != -1 and char-class conditions) ──
            // BUG-FIX 2026-04-28: gate por click real + coords del tile.
            DAT_07e016c0 = (DWORD)(int)*(float*)&DAT_080ab288;
            DAT_07e016c4 = (DWORD)(int)*(float*)&DAT_080ab28c;

            // FUN_004f6c30 devuelve el atributo de terreno en la grilla calculada
            int attrIdx = FUN_004f6c30((int)DAT_07e016c0, (int)DAT_07e016c4);
            int iSrc = SelectedOperate;

            if (((unsigned char*)&DAT_0838bc70)[attrIdx] < 2
                && *(char*)(ent + 0x2ec) == '\0')
            {
                *(unsigned char*)(ent + 0x2ed) = 4;

                // Busca el tipo de entidad y el facing en la tabla de objetos especiales
                // Stride de la tabla: 3 int por entrada, en DAT_083a2378
                int tgtEntityPtr = ((int*)&DAT_083a2378)[iSrc * 3];
                DAT_07db8708  = (int)*(short*)(tgtEntityPtr + 2);
                _DAT_07e118e4 = *(DWORD*)(tgtEntityPtr + 0x24);

                int srcX = *(int*)(ent + 0x388);
                int srcY = *(int*)(ent + 0x38c);

                unsigned int ok = FUN_0043f3e0(srcX, (int)(float)srcY,
                                                DAT_07e016c0, DAT_07e016c4,
                                                ent + 0x354, 0.0f);
                if ((char)ok == '\0') {
                    Send_MovePacket_Player_legacy_stub();
                } else {
                    FUN_00491c40((int)ent, (int)ent);
                }
            }
        }
    } else {
        // Cooldown not ready: clear hover flags
        DAT_083a4124 = '\0';
        DAT_083a42c4 = '\0';
    }

end_tick_inc:
    DAT_07e11d28 = DAT_07e11d28 + 1;

end_tick:
    // ── Per-frame entity state update ─────────────────────────────────────────
    FUN_0049cbf0(DAT_07abf5d8);

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
