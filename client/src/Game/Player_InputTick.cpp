#include "stdafx.h"
#include "Item/ContentCatalog.h"
#include "Render/Camera3D.h"
#include "UI/MiniMap.h"
#include "UI/EventTimer.h"
#include "UI/MoveList.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"
#include <winsock2.h>
#include <string.h>


extern "C" int __cdecl GetScreenWidth(void);
extern "C" void Net_SendNpcTalkClose(void);
extern "C" BOOL ChaosBoxRequestClose(void);

static bool HUD_IsQuestPanelOpenRuntime(void);
static bool HUD_IsGoldenArcherPanelRuntime(void);
static bool HUD_IsInventoryFamilyActive(void);
extern "C" int g_nGuildMemberCount;
static bool HUD_IsAnyRightPanelOpen(void);
static bool HUD_IsGuildCreationRuntime(void);
static bool HUD_IsGuildListRuntime(void);
static bool HUD_IsCharacterInfoRuntime(void);

// Bottom-bar HUD button hit-test.
// Los rectángulos son los mismos que usan los tooltips de hover y el
// resaltado de "panel abierto" en HUD_Pass5.cpp. Con un LButton release (no
// drag), toggles the corresponding panel flag.
//
// Gated by:
//   - sólo en estado in-game (SceneFlag == 5)
//   - released-click signal (DAT_083a413c) so holding doesn't repeat
//   - limpiar DAT_083a413c después de consumirlo, para que otra parte de la UI no lo maneje dos veces
//
// En el binario 0.97k original esto estaba inline dentro del ruido anti-tamper de
// sub_004B82xx; acá lo reimplementamos limpio.
//
// `MouseOnWindow` (IDA Player_InputTick:416,566 + sub_402F40:9) es un solo
// global del binario, 0x07D78094 (= DAT_07d78094): se setea cada frame si el
// mouse está sobre algún panel de UI abierto y gatea el GroundClick, para que
// clickear adentro de un panel no haga caminar al jugador. Lo escribe
// CheckInventory (sub_4E6550) con los seis rects de panel; g_MouseOnWindow es
// un alias.
#define g_MouseOnWindow DAT_07d78094

// IDA `Attacking` — estado del auto-ataque: -1 = ninguno, 1 = ataque iniciado
// desde Player_InputTick (L942), 2 = desde Attack (0x49CBF0 L1323).
// Lo resetean a -1 InitGame, ReceiveTeleport, CheckGate y varios paths de
// Attack. Con -1 (el default) el head-tracking hacia el mouse queda ACTIVO,
// que es el comportamiento normal fuera de combate.
// IDA `Attacking` vive en 0x00559C58 = Attacking (lo escriben InitGame,
// Player_InputTick L942 y Attack 0x49CBF0).  `g_Attacking` era una copia
// paralela que nadie escribia; se deja como alias de lectura para no romper
// declaraciones externas.
#define g_Attacking Attacking

// El ChatListBox publica su propio hit-test acá (definido en
// src/UI/ChatListBox.cpp). Su tick corre ANTES que esta función dentro del
// mismo frame, así que el latch está fresco. Sin esto, clickear dentro del
// recuadro del chat mandaría a caminar al personaje.
extern "C" int g_ChatLB_MouseOnWindow;

// Resetea y puebla MouseOnWindow al inicio del frame. La llama Player_InputTick.
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
        // IDA `sub_4E4760` L446 (igual que el render): el segundo botón va en
        // [origin+100, origin+170).
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

    // Skill bar expanded list (cells at y=370..411 cuando DAT_07db870c=1, el user
    // expandió el menú con click en el icono central): un click en una cell no
    // tiene que caer como move click.
    //
    // Chat_InputTick (corre ANTES) resetea DAT_07db870c a 0 cuando el user clickea
    // una cell, así que acá se usa un latch del frame anterior para que el click
    // "tail" siga viendo el menú como abierto.
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

// Cola comun de Chat_InputTick (0x4B14F0) al abrir guild/party/personaje y al
// cerrar el inventario, en sus dos versiones (teclas G/P/C/I/V L4921-6414 y
// botones de la barra L1116-2214):
//   TradeOpened      -> cancelar el trade (C3 0x3D)
//   WarehouseOpened  -> con EquipmentItem pendiente no se puede; si no, cerrar
//                       ventanas, devolver el item agarrado y mandar 0x82
//   ChaosMixOpened   -> 0x87 si la caja esta vacia y no hay item agarrado; si
//                       no, aviso 593
//   si no            -> apagar los paneles que no conviven y cerrar ventanas
//                       (CloseInventoryRelatedWindows ya manda el 0x31 de la
//                       tienda, fix del DLL)
// Devuelve false cuando la ventana del NPC no se pudo cerrar (la tecla apaga
enum HudPanelTail { TAIL_GUILD, TAIL_PARTY, TAIL_CHARACTER, TAIL_INVENTORY_CLOSE };
static bool HUD_PanelTail97k(HudPanelTail kind)
{
    extern void __cdecl CloseInventoryRelatedWindows(void);
    if (DAT_07eaa11b) {                                     // TradeOpened
        const BYTE pkt[3] = { 0xC1, 0x03, 0x3D };
        gNetwork.Send(pkt, sizeof(pkt));
        return true;
    }
    if (DAT_07eaa119) {                                     // WarehouseOpened
        if (DAT_07eaa165) return false;                     // EquipmentItem
        DAT_07eaa117 = 0;                                   // InventoryOpened
        CloseInventoryRelatedWindows();
        if ((int)DAT_07e91388 > 0) Item_ReturnPickedItem();
        const BYTE pkt[3] = { 0xC1, 0x03, 0x82 };
        gNetwork.SendC1(pkt, sizeof(pkt));
        return true;
    }
    if (DAT_07eaa11a)                                       // ChaosMixOpened
        return ChaosBoxRequestClose() != FALSE;             // aviso 593 incluido
    switch (kind) {
    case TAIL_GUILD:     DAT_07eaa117 = 0; DAT_07eaa116 = 0; break;
    case TAIL_PARTY:     DAT_07eaa117 = 0; DAT_07eaa11b = 0; DAT_07eaa116 = 0; break;
    case TAIL_CHARACTER: DAT_07eaa114 = 0; DAT_07eaa115 = 0; break;
    case TAIL_INVENTORY_CLOSE: DAT_07eaa117 = 0; break;
    }
    CloseInventoryRelatedWindows();
    return true;
}

// Botones de la barra inferior.  En el binario esto vive dentro de
// `Chat_InputTick` (0x4B14F0); aca corre desde Player_InputTick, que se ejecuta
// antes de la logica de click al mundo.
//
// Como en IDA: inventario y personaje conviven (por eso GetScreenWidth
// devuelve 260 justo para esa combinacion); el click se toma de
// `MouseLButtonPush` (DAT_083a4124) y se CONSUME poniendolo en 0, que es lo
// que evita el auto-repeat; y toda la fila se apaga mientras hay un modal.
//
// Rects (IDA L1130, L1455, L1775, L2078):
//   guild      (582..634, 459..477)
//   party      (348..372, 452..476)
//   personaje  (379..403, 452..476)
//   inventario (410..434, 452..476)
extern "C" void HUD_BottomBarButtons_HitTest(void);
void HUD_BottomBarButtons_HitTest(void)
{
    if (SceneFlag != 5) return;          // SceneFlag: only in-game
    if (!IsClickPushed()) return;

    // Gate de IDA L1116-1128: con un modal o el creador de guild abiertos, la
    // fila entera de botones no responde.  (`GuildInputEnable` del original no
    // existe en este arbol; GuildCreatorOpened cubre el mismo estado.)
    if (DAT_07eaa11b ||                      // TradeOpened
        DAT_07eaa124 ||                      // GuildCreatorOpened
        DAT_083a7c24 == 126 ||               // ErrorMessage: expulsar del guild
        DAT_083a7c24 == 152 ||
        _g_bEventChipDialogEnable ||
        ServerDivisionOpened ||                      // g_bServerDivisionEnable
        HUD_IsQuestPanelOpenRuntime())
        return;

    const int mx = (int)DAT_083a427c;       // 640-space mouse X
    const int my = (int)DAT_083a4278;       // 480-space mouse Y

    // -- Guild --
    if (mx >= 582 && mx < 634 && my >= 459 && my < 477) {
        DAT_083a4124 = 0;                    // MouseLButtonPush = 0
        DAT_07eaa115 = 0;                    // PartyOpened = 0
        if (DAT_07eaa114) {                  // GuildOpened
            DAT_07eaa114 = 0;
        } else {
            // 0x52 pide Encrypt=0 en HackPacketCheck.txt -> frame C1 plano.
            const BYTE guildListPkt[3] = { 0xC1, 0x03, 0x52 };
            gNetwork.SendC1(guildListPkt, sizeof(guildListPkt));
            g_nGuildMemberCount = -1;
            DAT_07eaa114 = 1;
            HUD_PanelTail97k(TAIL_GUILD);
        }
        PlayBuffer(0x19, 0, 0);            // IDA LABEL_123: en las dos ramas
        PlayBuffer(0x1c, 0, 0);
        return;
    }

    // -- Party --
    if (mx >= 348 && mx < 372 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        DAT_07eaa114 = 0;                    // GuildOpened
        if (PartyOpened) {
            PartyOpened = 0;
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            PartyNumber = 0;
            const BYTE partyListPkt[3] = { 0xC1, 0x03, 0x42 };
            gNetwork.SendC1(partyListPkt, sizeof(partyListPkt));
            PartyOpened = 1;
            HUD_PanelTail97k(TAIL_PARTY);
        }
        return;
    }

    // -- Personaje --
    if (mx >= 379 && mx < 403 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        if (DAT_07eaa116) {                  // CharacterOpened
            DAT_07eaa116 = 0;
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            DAT_07eaa116 = 1;
            HUD_PanelTail97k(TAIL_CHARACTER);
        }
        return;
    }

    // -- Inventario --
    if (mx >= 410 && mx < 434 && my >= 452 && my < 476) {
        DAT_083a4124 = 0;
        if (!DAT_07eaa117) {                 // InventoryOpened
            DAT_07eaa117 = 1;
            DAT_07eaa114 = 0;                // GuildOpened = 0
            DAT_07eaa115 = 0;                // PartyOpened = 0
        } else {
            HUD_PanelTail97k(TAIL_INVENTORY_CLOSE);
        }
        return;
    }
}

// Hotkeys de UI por frame para el HUD in-game (C/V/I).
// En el binario 0.97k original, los botones de la barra inferior (y estos
// atajos de teclado) invierten los flags de un byte DAT_07eaa11x que gatean cada
// bloque de render del HUD. La rutina dedicada que hacía esto estaba enterrada en
// 0x004B8xxx anti-tamper hash-table noise; this clean reimplementation
// cubre el mismo comportamiento observable para el usuario.
//
// Se suprime mientras haya algún modo de entrada de texto activo (chat, IME, texto de login)
// para que tipear letras en el chat no invierta paneles sin querer.
// Guard defensivo: GuildInputEnable/d71 (ChatMode/IME) aparecen corrompidos a
// 0xFF poco después de abrir el inventario (escritor no encontrado); se clampea
// a 0 cualquier valor fuera de {0,1} y se loguean las primeras ocurrencias.
extern char g_PadBeforeChatMode[64];
extern char g_PadAfterChatMode[64];
static void ClampChatModeIME(const char* tag)
{
    static int s_logs = 0;
    BYTE chat = (BYTE)GuildInputEnable;
    BYTE ime  = (BYTE)DAT_07e11d71;
    if (chat > 1 || ime > 1) {
        if (s_logs < 8) {
            s_logs++;
        }
        if (chat > 1) GuildInputEnable = 0;
        if (ime  > 1) DAT_07e11d71 = 0;
    }

    // Detector de la transición 0→1 de ChatMode: el clamp de arriba sólo atrapa
    // valores >1, pero un `1` espurio abre la caja de chat/whisper con un carácter
    // suelto. Se loguea el punto del frame donde se encendió para ubicar al escritor.
    {
        static BYTE s_prevChat = 0;
        BYTE now = (BYTE)GuildInputEnable;
        if (now == 1 && s_prevChat == 0) {
            static int s_onLogs = 0;
            if (s_onLogs < 12) {
                s_onLogs++;
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
    return (GoldenArcherOpenType != 0 && GoldenArcherOpenType != 3);
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
           (ServerDivisionOpened != 0) ||   // ServerDivisionOpened
           HUD_IsQuestPanelOpenRuntime();
}

static bool HUD_IsAnyRightPanelOpen(void)
{
    return HUD_IsInventoryFamilyActive() ||
           (DAT_07eaa115 != 0) ||   // PartyOpened
           (DAT_07eaa114 != 0) ||   // GuildOpened
           gEventTimer.IsOpen();    // DESVIACION: panel H
}

// DESVIACION (MU 5.2 NewUIHotKey.cpp, DLL Controller.cpp): Espacio levanta el
// primer item visible a menos de 300 unidades del héroe.  Como en el 5.2, se
// pide directo (g_bAutoGetItem) en vez de caminar hasta él; el DLL no lo hace
// y sólo funcionaba con el item al lado.
extern bool g_bAutoGetItem;
static void HUD_PickUpNearestItem(void)
{
    if (!Hero || (int)DAT_07e91388 > 0) return;                 // item en el cursor
    if ((int)DAT_083a427c >= GetScreenWidth() || (int)DAT_083a4278 >= 429) return;
    const float heroX = *(const float*)(Hero + 16);
    const float heroY = *(const float*)(Hero + 20);
    for (int i = 0; i < 1000; ++i) {
        const BYTE* item = &DAT_07e12840[0] + (size_t)i * 0x204;
        if (!item[72] || !item[424]) continue;                   // vivo y visible
        const float dx = *(const float*)(item + 88) - heroX;
        const float dy = *(const float*)(item + 92) - heroY;
        if (dx * dx + dy * dy >= 300.0f * 300.0f) continue;
        *(unsigned char*)(Hero + 0x2ed) = 1;                     // MOVEMENT_GET
        ItemKey = (DWORD)i;
        TargetX = (DWORD)(int)(*(const float*)(item + 88) / 100.0f);
        TargetY = (DWORD)(int)(*(const float*)(item + 92) / 100.0f);
        g_bAutoGetItem = true;
        Action((DWORD)Hero, (DWORD)Hero);
        g_bAutoGetItem = false;
        *(unsigned char*)(Hero + 0x2ed) = 0;
        return;
    }
}

static void HUD_HotkeyTick(void)
{
    ClampChatModeIME("HKT_enter");
    if (GuildInputEnable != '\0') return;  // g_ChatMode
    if (DAT_00559c84 != '\0') return;  // g_TextMode (login / dialog text)
    if (DAT_07e11d71 != '\0') return;  // g_IME_Mode
    // IDA Chat_InputTick L3976-3985: ANTES de mirar cualquier hotkey de panel,
    // el original corta la funcion entera con esta lista:
    //
    //   TradeOpened || GuildCreatorOpened || GuildInputEnable
    //   || ErrorMessage == 126 || ErrorMessage == 152
    //   || g_bEventChipDialogEnable
    //   || *(BYTE*)(g_csQuest + 116863) == 1
    //   || g_bServerDivisionEnable
    //
    // Con g_bEventChipDialogEnable (Golden Archer) abierto, la V dibujaría el
    // inventario encima del panel (los dos van a x=450) y al cerrarlo
    // CloseInventoryRelatedWindows limpiaría el flag sin avisarle al server, que
    // después rechaza /move. Con el gate, esa ventana solo se cierra por su X o
    // caminando, que son los caminos que mandan el 0x31.
    if (DAT_07eaa11b ||                      // TradeOpened
        DAT_07eaa124 ||                      // GuildCreatorOpened
        DAT_083a7c24 == 126 ||               // ErrorMessage: expulsar del guild
        DAT_083a7c24 == 152 ||
        _g_bEventChipDialogEnable ||         // Golden Archer / chip de evento
        ServerDivisionOpened ||                      // g_bServerDivisionEnable
        HUD_IsQuestPanelOpenRuntime())
        return;

    // Cada llamada a Key_IsJustPressed tiene efectos secundarios de detección por flanco, así que
    // capturamos los resultados antes de combinarlos (V o I invierten el inventario).
    int kC = PressKey(0x43); // 'C'  Character info
    int kV = PressKey(0x56); // 'V'  Inventory (alt)
    int kI = PressKey(0x49); // 'I'  Inventory
    int kG = PressKey(0x47); // 'G'  Guild
    int kP = PressKey(0x50); // 'P'  Party
    // DESVIACION (DLL Controller.cpp): H abre los horarios recibidos del server.
    if (PressKey('H')) gEventTimer.Toggle();
    // DESVIACION (DLL Controller.cpp): menú M con destinos recibidos del server.
    if (PressKey('M')) gMoveList.Toggle();
    // DESVIACION (DLL Controller.cpp): Tab abre el mapa.
    if (PressKey(VK_TAB)) gMiniMap.Toggle();
    if (PressKey(VK_SPACE)) HUD_PickUpNearestItem();
    // DESVIACION (DLL Controller.cpp): F10 activa la cámara 3D, F11 la restaura.
    if (PressKey(VK_F10)) gCamera3D.Toggle();
    if (PressKey(VK_F11)) gCamera3D.Restore();

    // IDA Chat_InputTick L4921-6414.  Al abrir con tecla, si la ventana del
    // NPC no se pudo cerrar (baul con EquipmentItem, Chaos con items) el panel
    // queda cerrado.  Los sonidos 25/28 salen al cerrar y al abrir el
    // inventario.
    if (kG) {
        DAT_07eaa115 = 0;                    // PartyOpened
        if (DAT_07eaa114) {
            DAT_07eaa114 = 0;
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            // 0x52: Encrypt=0 en HackPacketCheck.txt -> C1 plano.
            const BYTE guildListPkt[3] = { 0xC1, 0x03, 0x52 };
            gNetwork.SendC1(guildListPkt, sizeof(guildListPkt));
            g_nGuildMemberCount = -1;
            DAT_07eaa114 = 1;
            if (!HUD_PanelTail97k(TAIL_GUILD)) DAT_07eaa114 = 0;
        }
    }
    if (kP) {
        DAT_07eaa114 = 0;                    // GuildOpened
        if (PartyOpened) {
            PartyOpened = 0;
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            PartyNumber = 0;
            const BYTE partyListPkt[3] = { 0xC1, 0x03, 0x42 };
            gNetwork.SendC1(partyListPkt, sizeof(partyListPkt));
            PartyOpened = 1;
            if (!HUD_PanelTail97k(TAIL_PARTY)) PartyOpened = 0;
        }
    }
    if (kC) {
        if (DAT_07eaa116) {
            DAT_07eaa116 = 0;
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            DAT_07eaa116 = 1;
            if (!HUD_PanelTail97k(TAIL_CHARACTER)) DAT_07eaa116 = 0;
        }
    }
    if (kV || kI) {
        if (!DAT_07eaa117) {                 // InventoryOpened
            DAT_07eaa11b = 0;                // IDA: TradeOpened = 0 al abrir
            DAT_07eaa117 = 1;
            DAT_07eaa114 = 0;                // GuildOpened
            DAT_07eaa115 = 0;                // PartyOpened
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
        } else {
            HUD_PanelTail97k(TAIL_INVENTORY_CLOSE);
        }
    }
}

// IDA: Player_InputTick — Player_InputTick (0x004acef0, 1688 lines)
//
// Procesador de input del jugador por frame. Se llama desde el camino de render del HUD/UI en cada frame.
// Responsibilities:
//   1. Cooldown gate (DAT_07e11d1c must be <= 0x1e)
//   2. Second-password auto-fill from hover entity name
//   3. Camera update + facing angle packet [0xC1][0x18][0x66]
//   4. Sub-tick via CheckGate
//   5. Movement debounce (DAT_07e11d28 >= DAT_00559bec and !DAT_07e11dc0)
//   6. Animation state exit: swimming, normal walk, cancel
//   7. Click sobre mob/jugador (SelectedCharacter): pathfind + Combat_SendMovePathPacket
//   8. Click sobre NPC (SelectedNpc): pathfind alternativo
//   9. Click sobre objeto especial (SelectedOperate): pathfind + lookup de entity_type
//  10. Click en suelo (ray cast RenderTerrain + RenderTerrainTile): chequeo de terreno + pathfind
//  11. Actualización de estado de entidad: Combat_DispatchHeroSkillAttack
//  12. Atributo de terreno bajo el cursor → DAT_07e118e8
//
// Los bloques de ofuscación por HashTable repartidos por la función (~70 % del código) se omiten
// (ver docs/analisis/hashtable-anti-tamper.md).
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

// Paquete ya enmarcado (chain-XOR aplicado): lo manda CNetwork.
static void SendPacket(const char *buf, unsigned int len)
{
    gNetwork.SendRaw(buf, (int)len);
}

// IDA: Player_InputTick
void __cdecl Player_ProcessInput(void)
{
    // El procesamiento de hotkeys de UI va PRIMERO, para que los toggles funcionen
    // incluso cuando los gates de abajo saldrían temprano (p.ej. durante un cooldown).
    HUD_HotkeyTick();
    // Poblar el flag MouseOnWindow (per IDA) ANTES de la lógica de GroundClick,
    // así clickear adentro de un panel no hace caminar al jugador.
    MouseOnWindow_Update();
    // DAT_07d78094 y g_MouseOnWindow son la misma memoria (la escribe
    // CheckInventory por los paneles): no copiar uno en el otro acá.
    // Los botones de la barra inferior se atienden desde `Chat_InputTick`
    // (0x4B14F0), que es donde los tiene el binario.

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
        && MouseRButtonPush != '\0')
    {
        unsigned char *hoverEntity = (unsigned char*)(DAT_07abf5d0 + SelectedCharacter * 0x394);
        unsigned char *nameSrc     = hoverEntity + 0x1c1;

        // Copy name into password buffer
        memcpy(DAT_07db8810, nameSrc, 0x40);

        // El slot del historial es dword_559CC4 (0..4), NO SelectedCharacter.
        // IDA Player_InputTick L344-349:
        //     v11 = dword_559CC4;
        //     v12 = &byte_7E113E4[256 * v11];
        int histSlot = (int)DAT_00559cc4;
        if (histSlot < 0 || histSlot > 4) histSlot = 0;
        memcpy((void*)(DAT_07e113e4 + histSlot * 0x100), nameSrc, 0x40);

        // Pone en cero el buffer de contraseña y después copia de vuelta desde el slot preparado
        memset(DAT_07db8810, 0, 0x40 * sizeof(DWORD));
        memcpy(DAT_07db8810, (void*)(DAT_07e113e4 + histSlot * 0x100), 0x40);

        // Setea el largo de la contraseña y dispara el BGM 0x19
        DAT_07d780ac = (DWORD)strlen((char*)DAT_07db8810);
        PlayBuffer(0x19, 0, 0);
    }

    // ── Head-tracking hacia el mouse (IDA Player_InputTick L353-385) ──────
    //
    // Layout de la entidad, confirmado con la struct de MU 5.2 (`w_ObjectInfo.h`):
    //     +28 Angle[3]      +40 HeadAngle[3]      +52 HeadTargetAngle[3]
    bool bHeadTrackActive = false;
    float fHalfScreenW = 320.0f;
    {
        unsigned char* ent = (unsigned char*)DAT_07abf5d8;
        if (ent) {
            fHalfScreenW = (float)(GetScreenWidth() / 2);      // GetScreenWidth() / 2
            const float mouseX = (float)(int)DAT_083a427c;
            const float mouseY = (float)(int)DAT_083a4278;

            // Ángulo del mouse respecto del centro del viewport, llevado al marco
            // del cuerpo y clampeado a [120, 240]: la cabeza sólo gira ~±60°.
            const float angMouse = CreateAngle(fHalfScreenW, 180.0f, mouseX, mouseY);
            int v16 = (int)((int)(angMouse + *(float*)(ent + 36)) + 315) % 360;
            if (v16 >= 120) { if (v16 > 240) v16 = 240; }
            else            { v16 = 120; }

            *(DWORD*)(ent + 60) = 0;                          // HeadTargetAngle[2]

            // IDA L374: el tracking se apaga durante el auto-ataque y con la
            // animación de muerte (62).
            if ((DAT_07e11e18 == 0 || g_Attacking == -1 || World == 6)
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
    CheckGate();

    // ── WALKER (corre cada tick, INDEPENDIENTE de gates) ─────────────────────
    // Va fuera del gate de debounce `bec <= d28` (que se setea a `wpCount*3+4` en
    // cada envío de camino) y ARRIBA del gate `DAT_07d78094` (hover sobre la skill
    // bar): el walker se ejecuta SIEMPRE; solo el envío de packets y el
    // procesamiento de clicks nuevos van gateados.
    {
        unsigned char *ent = (unsigned char*)DAT_07abf5d8;
        if (*(unsigned char*)(ent + 0x78) & 0x20) {
            SetPlayerStop((int)ent);
        } else {
            // El chequeo isIdle va ANTES de SetPlayerWalk: si está idle (sin path activo)
            // no hay que setear la acción de caminar cada frame (resetearía el frame
            // counter de la animación).
            // IDA gatea todo este walker con Hero+748. Los contadores de waypoint
            // son internos a MovePath y no hay que usarlos para enganchar la posición
            // de mundo mientras el runner de camino está inactivo.
            bool isIdle = (ent[748] == 0);
            if (!isIdle) {
                SetPlayerWalk((int)ent);
            }
            if (!isIdle) {
                unsigned int moveOk = Entity_AdvancePath(ent, '\x01');
                if ((char)moveOk == '\0') {
                    MoveCharacterPosition((int)ent);
                } else {
                    // Al llegar al destino se resetean wp_count + cur_wp para que isIdle sea true
                    // en el frame siguiente.
                    *(unsigned char*)(ent + 0x354) = 0;   // cur_wp
                    *(unsigned char*)(ent + 0x355) = 0;   // substep
                    *(unsigned char*)(ent + 0x356) = 0;   // wp_count
                    *(unsigned char*)(ent + 0x305) = 0;   // move_pending:
                    // sin esto isIdle queda false → walker sigue ejecutando
                    // SetPlayerWalk cada frame → action=walk persistente.
                    *(unsigned char*)(ent + 0x2ec) = 0;
                    // IDA Player_InputTick L399: `*(_BYTE *)(v0 + 748) = 0;`
                    // Es la UNICA escritura a +748 que tiene esa funcion en el
                    // binario: sin ella el walker nunca se apaga.
                    *(unsigned char*)(ent + 748) = 0;
                    SetPlayerStop((int)ent);
                    // IDA L401: `dword_7E11DBC = (__int64)*(float *)(v0 + 36);`
                    // — es el FACING del héroe, no un timestamp.
                    DAT_07e11dbc = (int)*(float*)(ent + 36);
                    DAT_07e11db8 = 0;
                    // IDA L397-403: al terminar el camino, Action(c, c) con
                    // la cola que haya (0 = nada).  El port despachaba por
                    // tipo de cola con atajos propios (talk directo, pickup,
                    // solo ataque) y dejaba la cola 4 a un tick secundario.
                    Action((DWORD)ent, (DWORD)ent);
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
                            (int)CreateAngle(mouseX, mouseY, fHalfScreenW, 180.0f);

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
                            // y un paquete mal formado desconecta.
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
                        WORD targetId = *(WORD*)(tgt + 0x1dc);
                        extern float __cdecl CreateAngle(float, float, float, float);
                        float ex = *(float*)(ent + 0x10);
                        float ey = *(float*)(ent + 0x14);
                        float ttx = *(float*)(tgt + 0x10);
                        float tty = *(float*)(tgt + 0x14);
                        float facing = CreateAngle(ex, ey, ttx, tty);
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
                            pkt[i] ^= pkt[i - 1] ^ CNetwork::XorKey[i & 0x1f];
                        }
                        MuEmu::EncryptSend(pkt, 7);
                        if (SocketClientSocket != 0xFFFFFFFF) {
                            ::send(SocketClientSocket, (const char*)pkt, 7, 0);
                        }
                        SetPlayerAttack((int)ent, 0, 0, 0);
                    }
                }
            }
            ent[0x2ed] = 0;  // consume attack mode
        }
    }
    #endif // obsolete inline mini-attack disabled

    // ── Salida temprana si el movimiento está bloqueado o la UI activa (post-walker) ──
    // El gate va DESPUÉS del walker, así el walker siempre avanza incluso con
    // DAT_07d78094 seteado (mouse sobre la barra de skills).
    if (DAT_07e11d30 != 0 || g_MouseOnWindow != 0)
        goto end_tick;

    // ── Movement debounce gate (controla envío de packets/clicks, NO walker) ─
    // Desviación: con el walker idle (wp_count == 0) los clicks se aceptan de
    // inmediato; mientras se mueve se mantiene el gate por tiempo para no spamear
    // el server con paths intermedios.
    bool walkerIdle = (((unsigned char*)DAT_07abf5d8)[0x356] == 0);
    // DAT_07e11dc0 ("movement lock flag B"): según IDA solo lo escriben Attack
    // (0x49CC50) y Chat_InputTick (0x4B6630). Attack es stub vacío en nuestro build
    // y el port de Chat_InputTick no tiene ese write, así que su valor fiel es 0;
    // se fuerza acá porque se observó corrompido (-44) por un buffer adyacente.
    // Revisar al portar Attack.
    DAT_07e11dc0 = 0;
    if ((DAT_00559bec <= DAT_07e11d28) && DAT_07e11dc0 == '\0') {

        // Un click = un GroundClick.
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
        // Trackea si el evento de click ABAJO pasó sobre una ventana. Si sí, todo el
        // ciclo del click (ARRIBA/soltar) también se trata como "click de panel",
        // aunque el usuario haya movido el mouse al mundo antes de soltar.
        static bool s_clickStartedOnWindow = false;

        // Al entrar al mundo (SceneFlag == 5) se consumen los click flags que quedaron
        // del "Enter" de CharSelect (si no, el héroe atacaría al primer mob visible).
        {
            static int s_lastGameState = -1;
            int curState = (int)SceneFlag;
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
                    Attacking = -1;
                    DAT_00559c70 = -1;
                    DAT_00559ce8 = -1;
                    // Clear hero action queue (+0x2ED) so secondary tick
                    // doesn't fire Action() with garbage. NO tocar +0x2EC —
                    // ése es un state flag (is_moving / in_action) que server
                    // maneja, no un "alive" flag (per IDA ReceiveAction:20
                    // setea a 0 durante acciones de entidades vivas).
                    //
                    // También se resetean anim_state a idle (1) y el path state: el héroe hereda
                    // el anim_state del preview de CharSelect.
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
                    // NO wipear el entity pool aquí: Player_InputTick corre DESPUÉS de que F3/03
                    // JoinMapServer pobló el pool con los viewport spawns (0x12/0x13). El wipe se
                    // hace en el handler F3/03 de Net_Process, ANTES de cargar OpenWorld.
                }
                s_lastGameState = curState;
            }
        }

        // Semántica per IDA Player_InputTick:
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

        // Si cambia el target de hover con el click mantenido (p.ej. muere el mob A y
        // el mouse pasa a B), es un intent nuevo: se resetea el ciclo. Sólo con
        // bClickHeld; un latch colgado más un cambio de hover atacaría al pasar el mouse.
        static int s_lastHoverMob = -1;
        int curHoverMob = (int)SelectedCharacter;
        if (curHoverMob != -1 && curHoverMob != s_lastHoverMob && bClickHeld) {
            // Cambió el objetivo bajo el mouse Y el mouse está efectivamente apretado → intención de arrastre.
            s_clickCycleConsumed = false;
        }
        s_lastHoverMob = curHoverMob;

        // Detectar el flanco DOWN del click: cada click nuevo dispara un ciclo nuevo,
        // aunque sea sobre el mismo mob.
        static bool s_prevClickHeld = false;
        if (bClickHeld && !s_prevClickHeld) {
            // Rising edge: new click started
            s_clickCycleConsumed = false;
        }
        s_prevClickHeld = bClickHeld;

        // bClickEdge = bMousePush: el push pulse YA es one-shot (como en IDA). Después
        // del click handler se CONSUME (DAT_083a4124 = 0).
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
        // attack-arm (dispararía ataques al pasar el mouse).
        bool bClickEdge = bMousePush;
        // Solo consumir el push pulse si el click NO está sobre una ventana de UI: el
        // click handler de ESA ventana (FUN_004d23b0, llamado más tarde desde
        // RenderInventoryWindow / RenderShopInterface / etc.) necesita ver
        // `DAT_083a4124 == 1`. `g_MouseOnWindow` lo setea HUD_HitTest_AllWindows según
        // las flags de panel abierto + bounding box.
        if (bMousePush && !g_MouseOnWindow) {
            DAT_083a4124 = 0;
        }

        // Hard-consume del latch POP también, por si quedó stale.
        // Mismo razonamiento: solo consumir si NO estamos sobre UI.
        if (bClickLatched && !g_MouseOnWindow) {
            DAT_083a413c = 0;
        }

        // Durante los primeros 10 frames in-world, bClickEdge=false: cubre un
        // DAT_083a4124=1 colgado de la transición CharSelect → World.
        {
            static int s_inWorldFramesEdge = 0;
            if (SceneFlag == 5) s_inWorldFramesEdge++;
            else                   s_inWorldFramesEdge = 0;
            if (s_inWorldFramesEdge < 10) {
                bClickEdge = false;
            }
        }

        // IDA Player_InputTick:599 sólo sigue con el combate si `m_bAutoAttack &&
        // Attacking==1 && SelectedCharacter!=-1` o hay click (v32); si no, sale.
        bool bHoverActive = false;
        // IDA Player_InputTick L590-598 no tiene ningun concepto de "ciclo
        // consumido":
        //     v32 = 0;
        //     if ( MouseLButtonPush ) { MouseLButtonPush = 0; v32 = 1; }
        //     if ( MouseLButton )     { v32 = 1; }      // MANTENIDO
        // o sea con el boton apretado el click se procesa en CADA apertura
        // del gate, y el throttle es el propio MouseUpdateTimeMax -- que ya
        // gatea todo este bloque. No agregar `&& !s_clickCycleConsumed`: como
        // `MouseUpdateTime = 0` vive en el camino de procesar el click, el contador
        // no se resetearía nunca.
        if (bClickHeld || bClickLatched) {
            bHoverActive = true;
            s_clickCycleConsumed = true;
            // NO consumir DAT_083a4124 cuando el mouse está sobre
            // un panel — el render-phase de RenderCharacterInfoWindow / etc.
            // necesita ese flag para detectar clicks en sus botones (X close,
            // [+] stat add, etc.).  Solo consumir cuando el click ES para el ground
            // (mouse fuera de panels).
            if (!g_MouseOnWindow) {
                DAT_083a4124 = '\0';
                DAT_083a413c = '\0';   // consume ambos flags
            }
        }
        // Si el mouse está sobre un panel (HUD bottom, skill expanded list, panels
        // right-side), bHoverActive=false para que los alt-targets (hover target, NPC
        // click, tertiary) NO procesen el click como movement. También se consume
        // DAT_083a42c4 y se resetean SelectedCharacter/4c/48 (hover targets), para que
        // un hover target stale no dispare pathfind.
        if (g_MouseOnWindow || s_clickStartedOnWindow) {
            bHoverActive = false;
            // Consume los flags de click para que no se propaguen al tick
            // alt-target processing. Panel handlers se registraron via
            // Chat_InputTick (corre antes); ya no necesitamos los flags.
            DAT_083a4124 = '\0';
            DAT_083a413c = '\0';
            DAT_083a42c4 = 0;
        }

        // Si el user NO está clickeando activamente (no held, no latched), forzar
        // DAT_083a42c4=0 también, para que un click mal consumido no deje el flag activo.
        if (!bClickHeld && !bClickLatched) {
            DAT_083a42c4 = 0;
        }

        // IDA 0x004ACEF0 L599-604: si NO hay click (v32 = MouseLButtonPush ||
        // MouseLButton) y el auto-ataque no esta enganchado, el original sale
        // por LABEL_390 SIN tocar MouseUpdateTime:
        //     if ( (!m_bAutoAttack || World == 6 || Attacking != 1
        //           || SelectedCharacter == -1) && !v32 )
        //     { flt_7E11D50 = 0.0; flt_7E11D4C = WorldTime; goto LABEL_390; }
        //
        // Faltaba, y es la causa de "hay que clickear varias veces para
        // caminar".  El `MouseUpdateTime = 0` de LABEL_190 (mas abajo) corria
        // en CADA frame en que el gate de debounce estaba abierto, aunque no
        // hubiera click, asi que el contador quedaba en diente de sierra
        // 0..max en vez de saturar en max.  Solo el frame exacto en que
        // llegaba a max aceptaba un click: con MouseUpdateTimeMax = 3*wp+4
        // (31 tras una ruta de 9 waypoints) eso es 1 de cada 31 frames.
        {
            const bool bHasClick = (bMousePush || bClickHeld || bClickLatched);
            const bool bAutoAttackEngaged = (m_bAutoAttack != 0)
                                         && (World != 6)
                                         && (g_Attacking == 1)
                                         && (SelectedCharacter != -1);
            if (!bAutoAttackEngaged && !bHasClick) {
                _DAT_07e11d50 = 0.0f;
                _DAT_07e11d4c = DAT_05826e08;      // WorldTime
                goto end_tick_inc;                 // IDA: goto LABEL_390
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
                // DESVIACION DELIBERADA: la excepcion de IDA es 0x4E..0x50
                // (78..80 = stop/walk/run_TwoHandTwo).  La extendemos a
                // 0x4C (76) para cubrir tambien 76 y 77, Pegasus_fly y
                // Pegasus_fly_weapon -- el Dinorant volando en Tarkan e
                // Icarus.
                //
                // El rango bloqueante 0x22..0x5B empieza en attack_fist (34),
                // o sea esta pensado para no aceptar clicks durante un ataque.
                // Las acciones de caminar quedan debajo (las de montura son 32
                // y 33), y las tres de TwoHandTwo caen dentro solo por quedar
                // numeradas despues de los ataques -- de ahi que las exceptuen
                // a mano.  Con 76/77 se olvidaron, y son del mismo tipo:
                // animaciones de desplazamiento, no de ataque.
                //
                // Con 76/77 bloqueadas el camino no se encadena y SetPlayerStop pasa un
                // frame por la pose de parado cada ~1.08 s.  Es el mismo artefacto
                // que Webzen describe en 5.2 ("애니메이션 튀는거", la animacion
                // salta) y que alla resolvieron dejando de usar 76/77.
                // Extender la excepcion es mas acotado: conserva la eleccion de
                // accion de SetPlayerWalk y solo destraba el input.
                if (act < 0x4C || act > 0x50)
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
            unsigned int canAct = CheckAttack();
            // Desviación: CheckAttack retorna 0 cuando no hay entidad bajo el mouse
            // (SelectedCharacter == -1); con bHoverActive (click real) se pasa igual, para
            // que el click al suelo llegue al pathfind. Los handlers internos siguen
            // gateados por SelectedCharacter/4c/48/54 != -1.
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
                // Filtro: mobs MUERTOS no son targeteables (ver abajo).
                // IDA 0x004ACEF0 L590-603 no exige el flanco:
                //   v32 = MouseLButtonPush || MouseLButton;
                //   if ((!m_bAutoAttack || World == 6 || Attacking != 1 ||
                //        SelectedCharacter == -1) && !v32) goto LABEL_390;
                // O sea con el boton MANTENIDO se sigue atacando (el gate de
                // animacion de mas arriba marca el ritmo), y con m_bAutoAttack
                // el ataque continua al soltar mientras el objetivo siga
                // vivo (sub_4B0310 lo mantiene fijo).
                const bool bAutoAttackGoOn = m_bAutoAttack != 0          // m_bAutoAttack
                                          && World != 6           // World
                                          && (int)Attacking == 1;     // Attacking
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
                    BYTE* hoverEnt = (BYTE*)(uintptr_t)DAT_07abf5d0
                                   + (uintptr_t)SelectedCharacter * 0x394;
                    // Muerto = +0x2FD (IDA ReceiveDie:18). No usar +0x2EC (state flag que el
                    // server también pone en 0 en ReceiveAction) ni +0x34E (es SafeZone: dejaría
                    // sin targetear a todo NPC del pueblo).
                    if (hoverEnt[0x2FD] != 0) {
                        // Muerto — limpia el estado de hover para que el click siguiente no quede
                        // pegado al cadáver, y sale.
                        SelectedCharacter = -1;
                        SelectedOperate = -1;
                        Attacking = 0;
                        DAT_00559c70 = -1;
                        goto end_tick_inc;
                    }
                    int tgtEntityBase = (int)(DAT_07abf5d0 + SelectedCharacter * 0x394);
                    int dstX = *(int*)(tgtEntityBase + 0x388);
                    int dstY = *(int*)(tgtEntityBase + 0x38c);

                    DAT_00559ce8 = SelectedCharacter;
                    Attacking  = 1;
                    *(unsigned char*)(ent + 0x2ed) = 3;
                    // Destino = grid del MOB target (no DAT_05826e08, que es el tick counter).
                    TargetX = (DWORD)dstX;
                    TargetY = (DWORD)dstY;

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
                        Action((DWORD)(uintptr_t)ent,
                                                   (DWORD)(uintptr_t)ent);
                        goto end_tick_inc;
                    }
                    {
                        const char* const CM = (const char*)(uintptr_t)DAT_07cf1ffc;
                        const int lh = CM ? ItemBehaviorType(*(const short*)(CM + 536)) : -1;  // IDA: v265
                        const int rh = CM ? ItemBehaviorType(*(const short*)(CM + 604)) : -1;  // IDA: v264
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
                    Action((DWORD)(uintptr_t)ent,
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
        // En IDA los dos son `if` HERMANOS y el de SelectedOperate va primero:
        //     if ( SelectedOperate != -1 ) { ... }
        //     if ( SelectedNpc != -1 )     { ... }
        // Objeto interactuable bajo el cursor (sillas, bancos, barandas,
        // orbes de Noria).  IDA 0x4ACEF0 L1133-1195.
        //
        // En IDA la cadena es secuencial y SelectedOperate se chequea ANTES del ramo
        // de movimiento por terreno (no sólo con Shift):
        //     if ( SelectedOperate != -1 ) { ... goto LABEL_340/LABEL_312; }
        //     ...
        //     if ( GetAsyncKeyState(16) >> 8 != 0x80 ) { RenderTerrain(1); ... }
        // y las dos salidas del bloque saltan al final del tick, o sea
        // tienen PRECEDENCIA sobre el click al suelo.
        // IDA L590-598: las ramas de mobiliario, NPC e item corren con
        //     v32 = MouseLButtonPush || MouseLButton
        // o sea con el boton MANTENIDO tambien, igual que el click al suelo.
        const bool bClickNow = bClickEdge || DAT_083a42c4 != 0;   // MouseLButton
        if (SelectedOperate != -1 && bClickNow) {
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
                // TargetX/TargetY salen de la POSICION DEL OBJETO, no del tile bajo el
                // cursor.  IDA L1138:
                //     TargetX = (__int64)(o->Position[0] * 0.01);
                //     TargetY = (__int64)(o->Position[1] * 0.01);
                TargetX = (DWORD)(int)(*(float*)(tgtEntityPtr + 0x10) * 0.01f);
                TargetY = (DWORD)(int)(*(float*)(tgtEntityPtr + 0x14) * 0.01f);

                const int attrIdx = TERRAIN_INDEX((int)TargetX, (int)TargetY);
                if (((unsigned char*)&DAT_0838bc70)[attrIdx] < 2
                    && *(char*)(ent + 0x2ec) == 0)
                {
                    *(unsigned char*)(ent + 0x2ed) = 4;   // MOVEMENT_OPERATE
                    DAT_07db8708  = (int)*(short*)(tgtEntityPtr + 2);
                    _DAT_07e118e4 = *(DWORD*)(tgtEntityPtr + 0x24);

                    const int srcX = *(int*)(ent + 0x388);
                    const int srcY = *(int*)(ent + 0x38c);
                    unsigned int ok = Path_FindRoute(srcX, srcY,
                                                     TargetX, TargetY,
                                                     ent + 0x354, 0.0f);
                    if ((char)ok == 0) {
                        // LABEL_312: sin camino (ya estamos al lado) ->
                        // ejecutar la accion ahora.  El port mandaba otro
                        // paquete de movimiento y NUNCA llamaba a Action,
                        // asi que sentarse no se disparaba nunca.
                        Action((DWORD)ent, (DWORD)ent);
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
            if (SelectedNpc != -1 && bClickNow) {
                // Click sobre un NPC: setea el objetivo de movimiento y pathfindea (gateado por
                // un click real, bClickEdge, no bHoverActive).
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
                    // Destino = grid del NPC.
                    TargetX = (DWORD)dstX;
                    TargetY = (DWORD)dstY;

                    unsigned int ok = Path_FindRoute(srcX, srcY,
                                                    dstX, dstY,
                                                    ent + 0x354, 0.0f);
                    if ((char)ok == '\0') {
                        // IDA L1210: sin camino -> LABEL_312 (Action manda el 0x30).
                        Action((DWORD)ent, (DWORD)ent);
                    } else {
                        Combat_SendMovePathPacket((int)ent, (int)ent);
                    }
                    goto end_tick_inc;
                }
            }

            // ── Tertiary target (SelectedItem) ───────────────────────────────
            if (SelectedItem != -1 && bClickNow) {
                // bClickEdge, no bHoverActive (igual que el handler de ataque).
                *(unsigned char*)(ent + 0x2ed) = 1;
                ItemKey = (DWORD)SelectedItem;   // latch, IDA L1281
                // SelectedItem es índice del pool de items del suelo (DAT_07e12840, stride
                // 0x204), NO del pool de personajes (DAT_07abf5d0, stride 0x394). El tile del
                // item = worldXY/100.
                int itemSlotIdx = (int)SelectedItem;
                BYTE* itemEnt = (BYTE*)&DAT_07e12840[0]
                              + (uintptr_t)itemSlotIdx * 0x204;
                // La posición world del item la escribe CreateItem en ip+88/92 (el render la
                // lee en v1+16 = ip+72+16 = ip+88); itemEnt = ip.
                int dstX = (int)(*(float*)(itemEnt + 88) / 100.0f);
                int dstY = (int)(*(float*)(itemEnt + 92) / 100.0f);
                TargetX = (DWORD)dstX;
                TargetY = (DWORD)dstY;
                int srcX = *(int*)(ent + 0x388);
                int srcY = *(int*)(ent + 0x38c);

                unsigned int ok = Path_FindRoute(srcX, srcY,
                                                dstX, dstY,
                                                ent + 0x354, 0.0f);
                if ((char)ok == '\0') {
                    // IDA L1318: sin camino -> Action y cola en 0.
                    Action((DWORD)ent, (DWORD)ent);
                    *(unsigned char*)(ent + 0x2ed) = 0;
                } else {
                    Combat_SendMovePathPacket((int)ent, (int)ent);
                }
                goto end_tick_inc;
            }

            // ── Ground click: ray cast → terrain check → pathfind ────────────
            // Como el original, MANTENER el botón camina de forma continua.
            // MoveHero @ 0x004ACEF0:
            //     bVar32 = MouseLButtonPush != false;
            //     if (bVar32) MouseLButtonPush = false;      // consume el flanco
            //     bVar31 = MouseLButton != false || bVar32;  // ESTADO SOSTENIDO || flanco
            //     if (MouseLButton == false && !bVar32) { ...sale sin mover... }
            // La repetición la limita el debounce (DAT_00559bec <= DAT_07e11d28), igual
            // que el original la limita con MouseUpdateTimeMax <= MouseUpdateTime.
            //
            // Se lee DAT_083a42c4 EN VIVO, no la copia bClickHeld: más arriba se limpian
            // los flags cuando el cursor pasa sobre una ventana, y con la copia el hold
            // seguiría recalculando el destino un frame más (como la cámara sigue al
            // héroe, el destino huiría con ella).
            if (DAT_083a42c4 == 0 && !bClickEdge) goto end_tick_inc;
            // Per IDA Player_InputTick:416,566: bloquear el ground click cuando el mouse
            // está sobre cualquier panel abierto (MouseOnWindow=1).
            if (g_MouseOnWindow) goto end_tick_inc;
            // IDA: el click al mundo NO cierra ventanas de NPC acá. Lo hace
            // SendMove (0x491C40) al mandar el movimiento: el personaje camina
            // y la ventana se cierra con su paquete (Combat.cpp,
            // SendMove_CloseWindows97k). MuEmu no rechaza el 0x10 con la
            // interfaz abierta (CGMoveRecv no la chequea).
            // También se bloquea si el click se inició sobre una ventana (p.ej. una
            // skill cell que Chat_InputTick ya consumió).
            if (s_clickStartedOnWindow) goto end_tick_inc;
            // Hard gate: si la skill expanded list estuvo abierta este frame O el
            // anterior, ningún ground click vale (race entre el reset de Chat_InputTick
            // y este chequeo).
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
            {
                SHORT shift = GetAsyncKeyState(0x10);
                bool shiftHeld = ((char)((unsigned short)shift >> 8) == -0x80);
                if (!shiftHeld) {
                    // Reset del closest-hit sentinel ANTES de cada scan: si DAT_083a4120 (t_max)
                    // quedó de un frame previo, CollisionDetectLineToFace rechaza todos los hits.
                    extern void Map_InitRayCast(void);
                    Map_InitRayCast();
                    DAT_07eab1fc = 0;             // reset hit flag
                    RenderTerrain('\x01');         // iterate tiles + raycast

                    char cHit = (DAT_07eab1fc != 0) ? '\x01' : '\0';

                    if (cHit != '\0') {
                        // DAT_080ab288/28c ya viene en grid coords (e.g. 218.0): el picker
                        // (RenderTerrain) hace la conversión interna con _DAT_005524f0. Cast directo a
                        // int, sin dividir por 100.
                        float pickWX = *(float*)&DAT_080ab288;
                        float pickWY = *(float*)&DAT_080ab28c;
                        TargetX = (DWORD)(int)pickWX;
                        TargetY = (DWORD)(int)pickWY;
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
                        int terrIdx = TargetX + TargetY * 0x100;
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

                            // IDA 0x4ACEF0 L1344-1381, dos gates que faltaban.
                            //
                            // 1) Mientras el heroe camina (c+748), el inicio
                            //    cacheado (c+904/908) tiene que estar a menos de
                            //    2 tiles de su posicion real; si no, se saltea el
                            //    click (solo ++MouseUpdateTime).  Sin esto el
                            //    port recalculaba la ruta desde un tile que el
                            //    heroe ya habia dejado y mandaba un movimiento
                            //    que arranca en otro lado.
                            // 2) Si el destino es el tile donde ya arranca la
                            //    ruta y sigue caminando, no se re-rutea: solo
                            //    MouseUpdateTime = 0 (IDA LABEL_389).
                            {
                                const int heroGX = (int)(*(float*)(ent + 0x10) * 0.01f);
                                const int heroGY = (int)(*(float*)(ent + 0x14) * 0.01f);
                                const bool bMoving = (*(unsigned char*)(ent + 0x2ec) != 0);
                                if (bMoving) {
                                    if (abs(srcX - heroGX) >= 2) goto end_tick_inc;
                                    if (abs(srcY - heroGY) >= 2) goto end_tick_inc;
                                }
                                if (bMoving &&
                                    srcX == (int)TargetX && srcY == (int)TargetY) {
                                    DAT_07e11d28 = 0;          // IDA LABEL_389
                                    goto end_tick_inc;
                                }
                            }

                            unsigned int ok = Path_FindRoute(srcX, srcY,
                                                            TargetX, TargetY,
                                                            ent + 0x354, 0.0f);
                            if ((char)ok != '\0') {
                                *(unsigned char*)(ent + 0x2ed) = 0;
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
    // IDA Player_InputTick (0x4ACEF0 L586-588) NO borra los flags de click cuando
    // el gate de debounce bloquea:
    //     if ( MouseUpdateTime < MouseUpdateTimeMax || byte_7E11DC0 )
    //         goto LABEL_390;            // == ++MouseUpdateTime; salir
    // Los dos flags solo se limpian en el anti-AFK de L607-611 (boton
    // sostenido 3600 s). O sea el click PENDIENTE sobrevive al bloqueo y lo
    // procesa el primer tick que pase el gate.

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
            DAT_07e118e8 = ((unsigned char*)&TerrainMappingLayer1)[idx];
        }
    }
}
