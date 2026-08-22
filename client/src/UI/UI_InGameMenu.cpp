// UI_InGameMenu.cpp — FUN_00514310 @ 0x00514310
// Secondary in-game UI state machine (DAT_083a7c24).
// Handles: escape menu, NPC shop, buy confirmation, item lists, server/char transitions.
// Called every frame from Game_MainLoop.
// Phantom stack params (in_stack_0x28..0x91fc) are SEH/obfuscation artefacts — ignored.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

#include "Net/MuEmu.h"
#include "Net/Net.h"  // 2026-05-05: Net_SendSmallPacket (proper C3 wrap with serial)

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);
extern "C" char byte_7E91790[];   // tabla de miembros del guild (stride 13)
extern "C" int  dword_5615E4;     // indice del miembro elegido para expulsar


extern char s_tradeRequestName[11];

extern "C" {
    void DbgLogPublic(const char* msg);
    // Char-list cache populated by Recv_CharList (Net_Process.cpp), replayed
    // here on JoinChar transition since server's F3/00 response post-JoinChar
    // is not consistently delivered.
    extern BYTE g_CharListCache[256];
    extern int  g_CharListCacheLen;
    void Recv_CharListReplay(const BYTE* Msg);
    // 0x81 PMSG_WAREHOUSE_MONEY_RECV (stubs_render_helpers.cpp)
    void Net_SendWarehouseMoney(BYTE type, DWORD money);
}
// g_iCurrentDialogScript es un macro sobre DAT_005615dc (globals.h)
// g_lpszDialogAnswer es un macro sobre DAT_083a4348 (globals.h): 10 x 38.

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

// XOR login key (32 bytes), used to encrypt outbound logout/transition packets.
// Same key as Net/Crypto.cpp login XOR.
static const BYTE s_xorKey[32] = {
    0xe7, 0x6d, 0x3a, 0x89, 0xbc, 0xb2, 0x9f, 0x73,
    0x23, 0xa8, 0xfe, 0xb6, 0x49, 0x5d, 0x39, 0x5d,
    0x8a, 0xcb, 0x63, 0x8d, 0xea, 0x7d, 0x2b, 0x5f,
    0xc3, 0xb1, 0xe9, 0x83, 0x29, 0x51, 0xe8, 0x56
};

// Build a C1-framed packet, XOR it with the login key, encode it via
// FUN_0053cc30, then send it.  Same send+WSAEWOULDBLOCK queue pattern
// used throughout Net_Process.cpp.
static void SendLoginPacket(BYTE *payload, int payloadLen)
{
    // XOR-encrypt payload
    for (int i = 0; i < payloadLen; ++i)
        payload[i] ^= s_xorKey[i & 0x1f];

    // Encode and send
    FUN_0053cc30(0, payload, payloadLen);

    int sent = send(DAT_055ca168, (char *)payload, payloadLen, 0);
    if (sent == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
    {
        // Queue into overflow buffer (max 0x2001 bytes at DAT_055ca16c).
        // Other send paths append directly at DAT_055ca16c + queuedBytes;
        // the extra +4 here leaves a gap and desynchronises popup/login sends.
        BYTE *qbuf = (BYTE *)DAT_055ca16c;
        DWORD q    = *(DWORD *)((char *)&DAT_055ca160 + 0x0c); // queued byte count
        if (q + (DWORD)payloadLen <= 0x2001)
        {
            memcpy(qbuf + q, payload, payloadLen);
            *(DWORD *)((char *)&DAT_055ca160 + 0x0c) += payloadLen;
        }
    }
}

// ---------------------------------------------------------------------------
// FUN_00514310 — UI_InGameMenu
// ---------------------------------------------------------------------------
void __cdecl FUN_00514310(void)
{
    // Mouse coordinates (updated by input layer)
    int mouseX = (int)DAT_083a427c;
    int mouseY = (int)DAT_083a4278;

    DWORD state = DAT_083a7c24;
    DWORD pending = DAT_083a7c28;

    // ── ESC key toggle (IDA 00514310 lines 534-572) ─────────────────────────
    // v2 = PressKey(27);  // VK_ESCAPE
    // if (v2 && !EquipmentItem) {
    //     if (ErrorMessage == 110 || 150) { close → NextErrorMessage }
    //     else if (!ErrorMessage) ErrorMessage = 110;   // open ESC menu
    //     PlayBuffer(25);
    // }
    // DAT_083a7c24 ≡ ErrorMessage, estado 0x6e (110) = menu abierto.
    // EquipmentItem es el ítem agarrado con el mouse; cuando hay uno, ESC no
    // abre el menú (se usa para soltar el ítem). En login EquipmentItem=NULL
    // siempre, así que podemos omitir ese guard.
    int escHit = FUN_0047ec20(27);  // VK_ESCAPE (1 si fue just-pressed)
    if (escHit) {
        bool didToggle = false;
        // 2026-07-27: válvula de escape para los diálogos Yes/No (151) y pet
        // rename (153). Si el hit-test de sus botones no pega, el cartel dejaba
        // el juego trabado sin forma de cerrarlo. Escape = cancelar (No).
        if (state == 0x97 || state == 0x99) {
            extern char DAT_00559f5e;
            DAT_00559f5e = 2;                 // respuesta = No / cancelar
            DAT_083a7c24 = DAT_083a7c28;      // ErrorMessage = NextErrorMessage
            DAT_083a7c28 = 0;
            FUN_00404bc0(0x19, 0, 0);
            DbgLogPublic("ESC: cartel Yes/No cancelado");
            return;
        }
        if (state == 0x6e || state == 0x96) {
            // Cerrar menú: volver al estado anterior (NextErrorMessage).
            DAT_083a7c24 = DAT_083a7c28;
            DAT_083a7c28 = 0;
            state = DAT_083a7c24;
            didToggle = true;
        }
        // 2026-05-08: BUG-FIX UX — si hay paneles abiertos (inventario,
        // character, party, etc), ESC los cierra PRIMERO sin abrir el
        // menú in-game. Esto evita que ESC accidentalmente abra el menú
        // (cuyo botón "Salir" tiene hit-test overlapando con la zona
        // de equipos del inventario → user clickea equip → logout).
        // Comportamiento convencional Mu Online.
        else if (DAT_005615c0 == 5 &&
                 (InventoryOpened || CharacterOpened ||
                  PartyOpened || GuildOpened ||
                  WarehouseOpened || ChaosMixOpened ||
                  TradeOpened || ShopOpened || DAT_07eaa128 || g_NpcTalkActive))
        {
            // 2026-07-25 (#2 shops): ANTES sólo cerraba inventory/character/
            // party/guild y "diferia" shop/warehouse/trade → la tienda quedaba
            // abierta al apretar Escape. Ahora también las cierra.
            // 2026-07-27: g_NpcTalkActive cubre NPCs que no setean flag local
            // (Golden Archer, quest, etc.) — el server igual tiene Interface.use=1
            // y hay que mandarle el close 0x31 o no deja abrir otra tienda.
            bool hadNpcWindow = (ShopOpened || WarehouseOpened ||
                                 ChaosMixOpened || TradeOpened || DAT_07eaa128 || g_NpcTalkActive);
            InventoryOpened = '\0';
            CharacterOpened = '\0';
            PartyOpened     = '\0';
            GuildOpened     = '\0';
            if (hadNpcWindow) {
                // Limpia ShopOpened/Warehouse/ChaosMix/Trade/Event + pools.
                CloseInventoryRelatedWindows();
                // Avisar al server que cerramos el diálogo del NPC
                // (CGNpcTalkCloseRecv, head 0x31).
                // FIX 2026-07-25: el talk 0x30 requiere C3, pero el close 0x31
                // enviado en C3 desconectaba → 0x31 requiere C1.  Lo mandamos
                // como C1 plano por el send() hookeado (MuEmu byte-encrypt), igual
                // que los moves 0x10.  Sin body no hace falta chain-XOR.
                BYTE closePkt[4] = { 0xC1, 0x03, 0x31, 0 };
                {
                    char cb[96];
                    wsprintfA(cb, "ESC CLOSE-NPC: sock=%08X npcActive=%d sending 0x31 close",
                              (unsigned)DAT_055ca168, g_NpcTalkActive);
                    DbgLogPublic(cb);
                }
                g_NpcTalkActive = 0;
                if (DAT_055ca168 != 0xFFFFFFFF)
                    send((SOCKET)DAT_055ca168, (const char*)closePkt, 3, 0);
            }
            didToggle = true;
        }
        else if (state == 0 || state == 0x71 || state == 0x70 ||
                   state == 0x1a || state == 0x1c) {
            // BUG-FIX 2026-04-28: estados 0x71/0x70 son "Connection lost" / quit
            // que se setean en bg cuando el server desconecta — antes anulaban
            // el menú ESC. Permitir abrirlo igual sobre estos estados.
            DAT_083a7c24 = 0x6e;
            state = 0x6e;
            didToggle = true;
        }
        // IDA L571: PlayBuffer(25) tras el toggle (incluye cerrar y abrir).
        if (didToggle) FUN_00404bc0(0x19, 0, 0);
    }

    // ── Exit-countdown (IDA 00514310 L536-550) ──────────────────────────────
    // Cuando el botón Exit del login setea DAT_083a7c1c=1 y DAT_083a7c20=50,
    // este bloque decrementa el contador cada frame. Al llegar a 0 manda
    // WM_DESTROY a la ventana principal → WndProc limpia y PostQuitMessage.
    // Sin este bloque el botón Exit del login sólo bajaba el diálogo sin
    // terminar el proceso.
    if (DAT_083a7c1c) {
        DWORD v3 = DAT_083a7c20;
        if ((v3 % 10) == 0) {
            // IDA L541-542: sprintf(Buffer, GlobalText[380], v3/10);
            //               UIChatLogWindow_AddText(byte_83A7C5C, Buffer, 1);
            // GlobalText[380] contains the language-specific format string
            // (e.g. "Saldrás del juego en %d segundos."), loaded from
            // Data/Local/Text.bmd by OpenTextData() at boot.
            char buf[300];
            wsprintfA(buf, GlobalText[380], (int)(v3 / 10));
            UIChatLogWindow_AddText((const char*)&DAT_083a7c5c, buf, 1);
        }
        DAT_083a7c20 = v3 - 1;
        if ((int)(v3 - 1) <= 0) {
            DAT_083a7c1c = 0;
            SendMessageA(g_hWnd, WM_DESTROY, 0, 0);
        }
    }

    // ── Fast exits ──────────────────────────────────────────────────────────
    if (state == 0)
        return;

    // ── Outer state switch ──────────────────────────────────────────────────
    switch (state)
    {
    // ── Quit game (fiel al IDA) ───────────────────────────────────────────
    // Codes 0x1a (result=6) / 0x1c (result=8 "demasiados intentos") /
    // 0x70/0x71 (fallo de conexión). El IDA (0x514310 L1396-1414) muestra el
    // cartel y SOLO hace SendMessageA(WM_DESTROY) cuando hacés click en el botón
    // OK (rect 284-354 × 98-119) — no cierra directo. El cartel persiste hasta
    // el click. BUG-FIX 2026-07-14: gateamos el WM_DESTROY al click en OK.
    case 0x1a:
    case 0x1c:
    case 0x70:
    case 0x71:
    {
        // OK vía click en el botón (284-354 × 98-119) O tecla Enter
        // (DAT_055ca038). Consumimos el flag de Enter para que el login-trigger
        // de Game_SceneUpdate (que corre DESPUÉS) no lo tome como reintento.
        bool enterHit = (DAT_055ca038 != '\0');
        bool okClick = (mouseX >= 284 && mouseX < 354 &&
                        mouseY >= 98 && mouseY < 119 && IsClickPushed());
        if (okClick || enterHit) {
            DAT_083a4124 = 0;
            DAT_055ca038 = '\0';                     // consume Enter
            SendMessageA(g_hWnd, WM_DESTROY, 0, 0);  // cierra el cliente
            DAT_083a7c24 = DAT_083a7c28;             // ErrorMessage = NextErrorMessage
            DAT_083a7c28 = 0;
        }
        return;   // persiste hasta OK (click o Enter)
    }

    // ── Escape / in-game menu ────────────────────────────────────────────
    // N buttons (3/4/5 según g_GameState) en X=[0x103..0x17b], Y=(3*i+6)*10.
    //
    // Render order (UI_StatsPanel FUN_0051af50):
    //   Login (state 2, 3 btns):     Salir • Opciones • Cancelar
    //   CharSelect (state 4, 4 btns): Salir • IrOtroSrv • Opciones • Cancelar
    //   Ingame (state 5, 5 btns):     Salir • IrOtroSrv • IrOtroChar • Opciones • Cancelar
    //
    // Click routing (IDA 0x00514310 L600-1320): el índice i tiene acción distinta
    // según g_GameState — case 0 siempre Exit, case 1 Options(login)|JoinServer(cs/in),
    // case 2 Close(login)|Options(cs)|JoinChar(in), case 3 Close(cs)|Options(in),
    // case 4 Close(in).
    case 0x6e:
    {
        if (mouseX >= 0x103 && mouseX <= 0x17b && IsClickPushed())
        {
            DWORD gs = DAT_005615c0;              // g_GameState
            int btnCount = (gs == 5) ? 5 : ((gs == 4) ? 4 : 3);

            for (int i = 0; i < btnCount; ++i)
            {
                int btnY = (3 * i + 6) * 10;
                if (mouseY >= btnY && mouseY < btnY + 0x16)
                {
                    DAT_083a4124 = 0;  // consume click

                    // Decide acción semántica según (estado, índice).
                    // semantic: 0=Exit 1=JoinSrv 2=JoinChar 3=Options 4=Close
                    int action;
                    if (gs == 5) {                 // ingame
                        const int map[5] = { 0, 1, 2, 3, 4 };
                        action = map[i];
                    } else if (gs == 4) {          // charselect
                        const int map[4] = { 0, 1, 3, 4 };
                        action = map[i];
                    } else {                       // login / loading
                        const int map[3] = { 0, 3, 4 };
                        action = map[i];
                    }

                    switch (action)
                    {
                    case 0:  // Exit / Salir del juego
                    {
                        // IDA 00514310 L605-825: el packet F1/02/00 SOLO se manda
                        // cuando estamos in-game (gs==5). En login/char-select
                        // (gs==2/4) la rama de L823-825 solo cierra el socket y
                        // arranca el countdown — no envía nada al server.
                        if (DAT_005615c0 == 5) {
                            BYTE pkt[8] = { 0xC1, 0x05, 0xF1, 0x02, 0x00, 0x00, 0x00, 0x00 };
                            SendLoginPacket(pkt, 5);
                        }
                        if (DAT_055ca168 != 0xffffffff) {
                            closesocket((SOCKET)DAT_055ca168);
                            DAT_055ca168 = (DWORD)INVALID_SOCKET;
                        }
                        DAT_083a7c1c = 1;
                        DAT_083a7c20 = 50;   // 50 ticks = 5 s at 10 FPS refresh
                    }
                    break;

                    // ── case 1/2: el menú-índice NO coincide con el sub-byte F1/02 ──
                    // Protocolo F1/02: sub=0 Exit, sub=1 JoinChar, sub=2 JoinSrv.
                    // Layout del menú según g_GameState:
                    //   gs=2 login    : { Options, Close }
                    //                   case1=Options  case2=Close
                    //   gs=4 cs       : { Exit, JoinSrv, Options, Close }
                    //                   case1=JoinSrv  case2=Options  case3=Close
                    //   gs=5 ingame   : { Exit, JoinSrv, JoinChar, Options, Close }
                    //                   case1=JoinSrv  case2=JoinChar case3=Options
                    //
                    // MuEmu NO hace echo de F1/02 (server cierra el socket o ignora),
                    // por lo que la transición de escena se hace localmente acá
                    // emulando lo que ReceiveLogOut@0x004247D0 haría con la respuesta.
                    case 1:
                    {
                        if (DAT_005615c0 == 4 || DAT_005615c0 == 5) {
                            // JoinSrv → F1/02/02
                            BYTE pkt[8] = { 0xC1, 0x05, 0xF1, 0x02, 0x02, 0x00, 0x00, 0x00 };
                            SendLoginPacket(pkt, 5);
                            // Transición local — emula ReceiveLogOut sub=2 (IDA 0x004247D0):
                            //   if (gs==5) { StopMusic; AllStopSound; sub_4CD3B0; ReleaseMainData; }
                            //   CWsctlc::Close
                            //   ReleaseCharacterSceneData()  ← libera modelos/texturas char-select
                            //   g_GameState = 2
                            //   InitLogIn=0; InitCharacterScene=0; InitMainScene=0;
                            //   EnableMainRender=0; CurrentProtocolState=0;
                            //   InitGame()                  ← reset estado de sesión
                            if (DAT_005615c0 == 5) {
                                StopMusic();
                                AllStopSound();
                                FUN_004cd3b0();
                                ReleaseMainData();
                            }
                            FUN_0043dc90((int)(uintptr_t)DAT_055ca160);  // Net_Disconnect
                            FUN_005102c0();       // ReleaseCharacterSceneData — saca preview
                            DAT_005615c0   = 2;   // g_GameState = Login
                            DAT_083a7c14  = 0;   // sub-state = ServerSelect
                            DAT_083a7c18  = 0;
                            DAT_05826cb0 = 0;   // CurrentProtocolState
                            // Reset init guards: cuando el usuario re-loguee y vuelva
                            // a char-select, las funciones init re-cargan los assets.
                            DAT_083a7c48 = 0;   // ConnectionCheckEnable
                            DAT_083a7c49 = 0;   // InitLogIn  → fuerza Scene_Login init
                            DAT_083a7c4b = 0;   // InitCharacterScene → fuerza OpenCharacterSceneData
                            DAT_083a7c4c = 0;   // InitMainScene
                            DAT_083a7c4d = 0;   // EnableMainRender / warning flag
                            InitGame();           // reset estado de juego
                        } else if (DAT_005615c0 == 2) {
                            // Login: case 1 = Options
                            DAT_083a7c28 = 0x96;
                        }
                    }
                    break;

                    case 2:
                    {
                        if (DAT_005615c0 == 5) {
                            // 2026-05-05 (final): JoinChar — back to char-select.
                            //
                            // Análisis del pcap+server log: server tarda ~5
                            // segundos en procesar el F1/02/01 (tick async).
                            // Durante ese delay Connected sigue OBJECT_ONLINE, y
                            // CGCharacterListRecv early-returns. Por eso el flow
                            // viejo (mandar F1/02/01 + F3/00 en mismo tick)
                            // hacía que server ignorara nuestro F3/00.
                            //
                            // Flow correcto:
                            //   1. Cliente manda F1/02/01 (C3-wrapped)
                            //   2. ESPERAMOS sin transition local
                            //   3. Server tick procesa CloseCount=1 →
                            //      CharacterGameClose → Connected=OBJECT_LOGGED
                            //      → manda F1/02/01 ack
                            //   4. Cliente recibe F1/02 ack → Recv_LogOut runs:
                            //      transitions state=4 + manda F3/00 (ahora
                            //      server YA está OBJECT_LOGGED → procesa OK)
                            //   5. Server GDCharacterListSend → DataServer →
                            //      F3/00 char-list fresca
                            //   6. Cliente Recv_CharList puebla slots 0-4
                            DbgLogPublic("JoinChar: send F1/02/01 (waiting for server ack)");
                            BYTE pkt[5] = { 0xC1, 0x05, 0xF1, 0x02, 0x01 };
                            Net_SendSmallPacket(pkt, 5);
                            // NO transición local. NO F3/00 send. Dejamos
                            // que Recv_LogOut maneje todo cuando llegue el ack.
                        } else if (DAT_005615c0 == 4) {
                            // Char-select: case 2 = Options
                            DAT_083a7c28 = 0x96;
                        } else {
                            // Login: case 2 = Close
                            DAT_083a7c28 = 0;
                        }
                    }
                    break;

                    case 3:  // Opciones → abre submenú 0x96
                        DAT_083a7c28 = 0x96;
                        break;

                    case 4:  // Cancelar → cerrar menú
                        DAT_083a7c28 = 0;
                        break;
                    }
                    goto tail;
                }
            }
        }
        // No click on a button — stay in state 0x6e
        return;
    }

    // ── Return to char select ────────────────────────────────────────────
    case 0x72:
        DAT_083a7c14 = 0x18;
        DAT_083a7c18 = 0x15;
        // SIN PlayBuffer aca: IDA tiene DOS `case 114`, y solo el del switch del
        // boton de cerrar (L2183, nuestro segundo switch) reproduce el 27. El de
        // este switch principal (L2811) llama sub_513C10 y no suena. Tenerlo en
        // los dos hacia sonar el clic dos veces al volver a char-select.
        // TODO(port): este case ademas ejecuta el cuerpo del segundo switch en
        // vez de sub_513C10 — divergencia estructural preexistente, fuera del
        // alcance de la pasada de audio.
        // Close NPC UI
        FUN_0047ec60(0);
        DAT_00559c94 = (DWORD)0x2a;
        DAT_00559c88 = 2;
        DAT_07e11d72 = 0;
        DAT_00559c84 = 0;
        goto tail;

    // ── Zen input dialog (ErrorMessage 116) — baúl / trade ─────────────────
    // 2026-08-08 PORT (antes: rama inventada que llamaba FUN_004e9250, o sea el
    // shuffle del teclado numérico del PIN, y hacía `goto tail` INCONDICIONAL →
    // el cartel se auto-dismisseaba el frame siguiente y nunca se enviaba nada).
    //
    // Per IDA 0x514310 L1400-1406: para ErrorMessage==116 el rect del botón OK
    // NO cuenta — la única confirmación es Enter (byte_55CA038). Y L1420-1432:
    // si InputGold > 50.000.000 se muestra el cartel 118 y se resetea el input.
    //
    // StorageGoldFlag (DAT_07eaa108) lo setea quien abrió el diálogo:
    //   0 = guardar zen en el baúl     (sub_4EB5D0 case 0)
    //   1 = sacar zen del baúl         (sub_4EB5D0 case 1)
    //   2 = poner zen en el trade      (sub_4EB7F0)
    // Warehouse: C1:81 [type][money:4]. Trade: C1:3B [money:4].
    case 0x74:
    {
        bool enterHit = (DAT_055ca038 != '\0');
        if (!enterHit) return;          // el cartel persiste hasta Enter
        DAT_055ca038 = '\0';            // consumimos Enter

        int gold = (int)DAT_07e11d74;   // InputGold (lo llena WndProc con atoi)

        if (gold > 50000000) {
            // NextErrorMessage = 118 ("cantidad demasiado grande") + reset input
            DAT_083a7c28 = 118;
            FUN_0047ec60(0);            // ClearInput(0)
            DAT_00559c94 = (DWORD)42;   // InputTextMax[0]
            DAT_00559c88 = 2;           // InputNumber
            DAT_07e11d72 = 0;           // GoldInputEnable
            DAT_00559c84 = 0;           // InputEnable
            DAT_07e11d28 = 0;           // MouseUpdateTime
            DAT_00559bec = 6;           // MouseUpdateTimeMax
            FUN_00404bc0(0x19, 0, 0);   // PlayBuffer(25)
            return;
        }

        if (DAT_07eaa108 == 2) {
            // PMSG_TRADE_MONEY_RECV (MuEmu Trade.h). The amount is little-
            // endian, matching the server's DWORD field.
            BYTE pkt[7] = { 0xC1, 0x07, 0x3B, 0, 0, 0, 0 };
            memcpy(pkt + 3, &gold, sizeof(DWORD));
            Net_SendSmallPacket(pkt, sizeof(pkt));
        } else if (gold > 0) {
            Net_SendWarehouseMoney((BYTE)(DAT_07eaa108 & 1), (DWORD)gold);
        }

        FUN_0047ec60(0);                // ClearInput(0)
        DAT_00559c94 = (DWORD)42;
        DAT_00559c88 = 2;
        DAT_07e11d72 = 0;               // GoldInputEnable = 0
        DAT_00559c84 = 0;               // InputEnable = 0
        DAT_07e11d74 = 0;               // InputGold = 0
        goto tail;
    }

    // ── Level-up reward ───────────────────────────────────────────────────
    case 0x75:
    {
        // Check reward flag at CharData+0x152
        BYTE *charData = (BYTE *)DAT_07cf1ffc;
        if (charData && charData[0x152] != 0)
        {
            // Send variable packet with reward ACK
            BYTE pkt[6] = { 0xC1, 0x06, 0xF3, 0x10,
                            (BYTE)(DAT_07eaa0d8 & 0xff),
                            (BYTE)((DAT_07eaa0d8 >> 8) & 0xff) };
            SendLoginPacket(pkt, 6);
        }
        goto tail;
    }

    // Guild invitation (ProtocolCore 0x50 -> ErrorMessage 119). The source
    // client emits PMSG_GUILD_RESULT_RECV C1:51 with the original inviter key.
    // Button geometry is the standard two-button dialog: left=yes, right=no.
    case 0x77:
    {
        const bool yes = mouseX >= 234 && mouseX < 304 &&
                         mouseY >= 98 && mouseY < 119 && IsClickPushed();
        const bool no  = mouseX >= 334 && mouseX < 404 &&
                         mouseY >= 98 && mouseY < 119 && IsClickPushed();
        if (!yes && !no)
            return;

        DAT_083a4124 = 0;
        const WORD key = (WORD)DAT_07eaa0d8;
        // 0x51 pide Encrypt=0 (C1 plano). Acá el chain-XOR sí importa: el
        // paquete tiene payload, y el server lo descifra con XorData.
        BYTE pkt[6] = { 0xC1, 0x06, 0x51, (BYTE)(yes ? 1 : 0),
                        (BYTE)(key >> 8), (BYTE)key };
        Net_SendC1Packet(pkt, sizeof(pkt));
        goto tail;
    }

    // ── Send 4-byte packet ────────────────────────────────────────────────
    case 0x79:
    {
        BYTE pkt[4] = { 0xC1, 0x04, 0x30, 0x00 };
        SendLoginPacket(pkt, 4);
        goto tail;
    }

    // ── 126 — confirmar EXPULSAR a un miembro del guild ───────────────────
    // 2026-08-15: acá había un bloque que limpiaba los buffers de usuario y
    // password (`DAT_07db8710`/`DAT_07db8810`).  Eso NO es lo que hace el
    // binario: IDA `UI_InGameMenu` L3343 `case 126:` arma y envía el paquete de
    // expulsión, y NUNCA toca InputText[1] en toda la función (grep sobre el
    // decompile: 0 ocurrencias).  El limpiar-input es la rama de CANCELAR
    // (`case 126: case 152:` del segundo switch, L2706), que ya está más abajo.
    //
    // PMSG_GUILD_DELETE_RECV (MuEmu Guild.h:205):
    //     [C1][0x17][0x53][name:10][PersonalCode:10]      sizeof = 23
    // `name` sale de la tabla de 13 bytes por miembro `byte_7E91790`, indexada
    // por `dword_5615E4` (lo setea el click en el botón de expulsar, sub_40F320
    // → nuestro GuildLB_perFrameInput).  `PersonalCode` es lo que el jugador
    // tipeó en el diálogo (InputText[0] = DAT_07db8710).
    //
    // HackPacketCheck índice 83 → Encrypt = 0 ⇒ frame C1 + chain-XOR, SIN
    // serial (ver "Desconexiones: serial de packets y Encrypt=1" en CLAUDE.md).
    // El original hace el XOR en dos pasadas (3..12 y 13..22) porque appendea el
    // PersonalCode después; como la cadena es secuencial y los rangos son
    // disjuntos y contiguos, una sola pasada 3..22 da el mismo resultado.
    case 0x7e:
    {
        // Gate de botones.  IDA L2142-2168: para ErrorMessage 126 y 152 los dos
        // botones NO están en el rect estándar (234/334) sino corridos:
        //   Sí  → x ∈ [323, 363), y ∈ [98, 119)
        //   No  → x ∈ [373, 413), y ∈ [98, 119)
        // Sin este gate el `case` corría en CADA frame mientras el cartel
        // estuviera abierto → spam del paquete 0x53 → desconexión.
        const bool yes = mouseX >= 323 && mouseX < 363 &&
                         mouseY >= 98 && mouseY < 119 && IsClickPushed();
        if (!yes) {
            // El "No" lo atiende el SEGUNDO switch (su gate es el rect
            // x ∈ [0x175, 0x185], que es el mismo botón derecho); ahí el
            // `case 0x7e` limpia el input y cae al `tail` (dismiss).  Salimos
            // con `break` para que ese switch lo vea.  Sin click en ninguno de
            // los dos, `return`: el cartel persiste.
            const bool no = mouseX >= 0x175 && mouseX <= 0x185 && IsClickPushed();
            if (no) break;
            return;
        }
        DAT_083a4124 = 0;

        int memberIdx = dword_5615E4;
        if (memberIdx < 0 || memberIdx >= 0x22C / 13) goto tail;

        BYTE pkt[23];
        memset(pkt, 0, sizeof(pkt));
        pkt[0] = 0xC1;
        pkt[1] = 23;
        pkt[2] = 0x53;
        memcpy(pkt + 3,  &byte_7E91790[13 * memberIdx], 10);
        memcpy(pkt + 13, (const char*)DAT_07db8710, 10);
        Net_SendC1Packet(pkt, 23);
        goto tail;
    }

    // ── Send 4-byte packet + flush ────────────────────────────────────────
    case 0x80:
    {
        // PMSG_TRADE_RESPONSE_RECV (MuEmu Trade.h). Dialog 128 is the
        // original trade-request Yes/No prompt, not a F1 session action.
        const bool yes = mouseX >= 234 && mouseX < 304 &&
                         mouseY >= 98 && mouseY < 119 && IsClickPushed();
        const bool no  = mouseX >= 334 && mouseX < 404 &&
                         mouseY >= 98 && mouseY < 119 && IsClickPushed();
        if (!yes && !no)
            return;

        DAT_083a4124 = 0;
        BYTE pkt[20] = { 0xC1, 0x14, 0x37, (BYTE)(yes ? 1 : 0) };
        memcpy(pkt + 4, s_tradeRequestName, 10);
        Net_SendSmallPacket(pkt, sizeof(pkt));
        goto tail;
    }

    // ── NPC shop item list — display / navigation ─────────────────────────
    case 0x8b:
    case 0x8c:
    case 0x9a:
    {
        // Iterate shop item list (stride 5, up to DAT_083a4324 entries)
        // Requires click (MouseLButtonPush) — not hover.
        int *piVar10 = (int *)&DAT_083a42fc;
        int  count   = (int)DAT_083a4324;
        for (int i = 0; i < count; ++i)
        {
            // Hit-test each row
            int rowY = 0x2c + i * 0x10;
            if (mouseY >= rowY && mouseY < rowY + 0x10 &&
                mouseX >= 0x6a  && mouseX <= 0x16a && IsClickPushed())
            {
                DAT_083a4124 = 0;
                DAT_083a7c2c = (DWORD)i;
                DAT_083a7c28 = 0x8f;
                goto tail;
            }
        }
        goto tail;
    }

    // ── State 141 (0x8d) — Yes/No dialog with paging arrows ──────────────
    // Per IDA L3867-3963. The 7-line message-box buffer (g_lpszMessageBoxCustom)
    // shows up to 7 rows of GlobalText starting at offset (300 * unk_83A7C04).
    // unk_83A7C08 is the current page index (0..byte_83A7C09[0]).
    //
    // Left arrow at (249,202)-(264,222): decrement page (--unk_83A7C08, clamp 0)
    // Right arrow at (377,202)-(392,222): increment page (++unk_83A7C08, clamp max)
    // Both repaginate via SeparateTextIntoLines into g_lpszMessageBoxCustom.
    //
    // 2026-05-08: ported faithfully. Existing 0x8d-0x8e fall-through to shop
    // handler was wrong — caused NPC Yes/No dialogs to misroute.
    case 0x8d:
    {
        // Iterate button-rect array (DAT_083a42fc, stride 5 ints, count =
        // g_iNumLineMessageBoxCustom). Set hover/click state per button.
        int*  base = (int*)&DAT_083a42fc;
        int  *btn  = base;
        int   nBtns = (int)g_iNumLineMessageBoxCustom;
        bool  clickConsumed = false;
        for (int i = 0; i < nBtns; ++i, btn += 5) {
            if (btn[-1] >= 1) {
                if (mouseX < btn[0] + 213 || mouseY < btn[1] + 60 ||
                    mouseX > btn[0] + btn[2] + 213 ||
                    mouseY > btn[3] + btn[1] + 60)
                {
                    btn[-1] = 1;       // normal
                } else {
                    btn[-1] = 2;       // hover
                    if (IsClickPushed()) {
                        DAT_083a4124 = 0;
                        clickConsumed = true;
                    }
                }
            }
        }

        // Left arrow click: previous page
        if (mouseX >= 249 && mouseY >= 202 && mouseX <= 264 && mouseY <= 222 &&
            IsClickPushed())
        {
            int newIdx = (int)DAT_083a7c08 - 1;
            if (newIdx < 0) newIdx = 0;
            DAT_083a7c08 = (DWORD)newIdx;
            // Repaginate: SeparateTextIntoLines from
            //   GlobalText[unk_83A7C04 + newIdx]  (300 * idx + 0x07D29D24)
            const char* src = (const char*)
                ((uintptr_t)&GlobalText[0][0] + 300 * (DAT_083a7c04 + newIdx));
            g_iNumLineMessageBoxCustom = SeparateTextIntoLines(
                src, &DAT_083a44c4[0], 7, 38);
            DAT_083a4124 = 0;
            goto tail;
        }

        // Right arrow click: next page
        if (mouseX >= 377 && mouseY >= 202 && mouseX <= 392 && mouseY <= 222 &&
            IsClickPushed())
        {
            int newIdx = (int)DAT_083a7c08 + 1;
            int maxIdx = (int)DAT_083a7c09;
            if (newIdx > maxIdx) newIdx = maxIdx;
            DAT_083a7c08 = (DWORD)newIdx;
            const char* src = (const char*)
                ((uintptr_t)&GlobalText[0][0] + 300 * (DAT_083a7c04 + newIdx));
            g_iNumLineMessageBoxCustom = SeparateTextIntoLines(
                src, &DAT_083a44c4[0], 7, 38);
            DAT_083a4124 = 0;
            goto tail;
        }

        // Click on body (not on arrows): close the dialog
        if (clickConsumed) {
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
            DAT_083a7c24 = DAT_083a7c28;
            DAT_083a7c28 = 0;
            FUN_00404bc0(0x19, 0, 0);
        }
        goto tail;
    }

    // ── State 142 (0x8e) — NPC multi-answer dialog ──────────────────────
    // Per IDA L3891-3936. NPC shows up to N answer lines at fixed Y positions.
    // Click on answer N → look up DialogScript.LinkForAnswer[N] →
    //   if < 0: pop NextErrorMessage as new ErrorMessage (close dialog)
    //   if >= 0: invoke sub_51D840(link) to advance dialog tree.
    //
    // Layout (per 97k IDA / render companion):
    //   v350 = 18 * (7 - g_iNumAnswer - g_iNumLineMessageBoxCustom) / 2 + 66;
    //   v351 = v350 + 18 * g_iNumLineMessageBoxCustom;
    //   answer_idx = (mouseY - v351) / 18 if mouseY in [v351..v351+18*g_iNumAnswer)
    //   AND |556 - mouseX| <= 106 AND clicked.
    case 0x8e:
    {
        if (!IsClickPushed()) goto tail;
        int numAnswer = g_iNumAnswer;
        int numLines  = g_iNumLineMessageBoxCustom;
        int v350 = 18 * (7 - numAnswer - numLines) / 2 + 66;
        int v351 = v350 + 18 * numLines;
        if (mouseY < v351) goto tail;
        int v352 = mouseY - v351;
        int absDx = mouseX - 556;
        if (absDx < 0) absDx = -absDx;
        if (v352 < 18 * numAnswer && absDx <= 106) {
            int v353 = v352 / 18;
            if (v353 >= 0) {
                if (numAnswer <= 1) numAnswer = 1;   // IDA L3910-3912
                if (v353 < numAnswer) {
                    DAT_083a4124 = 0;
                    DAT_07e11d28 = 0;
                    DAT_00559bec = 6;
                    PlayBuffer(25, 0, 0);   // IDA L3917: un solo sonido, el 25

                    // IDA L3918-3931:
                    //   v355 = v353 + (g_iCurrentDialogScript << 8);
                    //   if (g_DialogScript[0].m_iLinkForAnswer[v355] < 0) { cerrar }
                    //   else { sub_51D840(m_iLinkForAnswer[v355]); }
                    // `<< 8` son 256 ints = 0x400 bytes = el stride de entrada,
                    // o sea `g_DialogScript[curScript].m_iLinkForAnswer[v353]`.
                    //
                    // 2026-08-21: el port decidia si cerrar comparando el TEXTO
                    // de la respuesta contra GlobalText[609] (invencion), y le
                    // pasaba a FUN_0051d840 el indice de RESPUESTA en vez del
                    // link.  Con la tabla ya reconciliada se puede hacer lo que
                    // hace el binario.
                    int cur  = g_iCurrentDialogScript;
                    int link = -1;
                    if (cur >= 0 && cur < DIALOG_SCRIPT_COUNT && v353 < 10)
                        link = g_DialogScript[cur].m_iLinkForAnswer[v353];

                    if (link < 0) {
                        int prev = (int)DAT_083a7c28;      // NextErrorMessage
                        DAT_083a7c28 = 0;
                        DAT_083a7c24 = (DWORD)prev;        // ErrorMessage
                    } else {
                        DAT_083a7c24 = DAT_083a7c28;
                        DAT_083a7c28 = 0;
                        FUN_0051d840(link);
                    }
                }
            }
        }
        goto tail;
    }

    // ── Item column click ─────────────────────────────────────────────────
    case 0x8f:
        // Range test for column X position — requires click.
        if (mouseX >= 0x6a && mouseX <= 0x16a && IsClickPushed())
        {
            DAT_083a4124 = 0;
            int col = (mouseX - 0x6a) / 0x28;
            DAT_083a7c2c = (DWORD)col;
            FUN_0051d840((int)DAT_083a7c2c);
        }
        goto tail;

    // ── Options submenu (0x96) — 4 buttons ────────────────────────────────
    // IDA 0x00514310 case 150 (L1336-1377):
    //   i=0: Back to ESC menu (NextErrorMessage=110 if ErrorMessage still nonzero,
    //        else ErrorMessage=110).
    //   i=1: toggle sound effect (m_bAutoAttack == DAT_00559c5c)
    //   i=2: toggle music (m_bWhisperSound == DAT_07e11d80)
    //   i=3: close submenu → ErrorMessage = NextErrorMessage; NextErrorMessage = 0.
    // Buttons at X∈[0x104..0x17c], Y = 10*(3*i+3) = 30,60,90,120; height 22.
    case 0x96:
    {
        for (int i = 0; i < 4; ++i)
        {
            int btnY = (3 * i + 3) * 10;
            if (mouseX >= 0x104 && mouseX < 0x17c &&
                mouseY >= btnY && mouseY < btnY + 22 && IsClickPushed())
            {
                DAT_083a4124 = 0;
                switch (i)
                {
                case 0:  // Back → ESC menu (stash para pop en Cancel)
                    DAT_083a7c28 = 0x6e;
                    break;
                case 1:  // Toggle sound
                    DAT_00559c5c ^= 1;
                    break;
                case 2:  // Toggle music
                    DAT_07e11d80 ^= 1;
                    break;
                case 3:  // Close — swap NextErrorMessage → ErrorMessage
                {
                    DWORD pop = DAT_083a7c28;
                    DAT_083a7c28 = 0;
                    DAT_083a7c24 = pop;
                }
                break;
                }
                // NO goto tail — case 0x96 maneja sus transiciones internamente,
                // pero sí reproducimos el click sound (IDA L837 PlayBuffer(25))
                // que en el original viene del LABEL_75 común.
                FUN_00404bc0(0x19, 0, 0);
                return;
            }
        }
        return;
    }

    // ── Yes/No checkbox (sell/drop confirm) — ErrorMessage 151 ─────────────
    // 2026-07-27 FIX: el port anterior trataba 0x97 como una lista de respuestas
    // de NPC (FUN_0051d840), que dismisseaba el cartel al instante sin setear la
    // respuesta → el sell-confirm quedaba colgado con el item agarrado (tooltip
    // pegado, "todo raro"). ErrorMessage 151 es un cartel Yes/No. Port IDA
    // UI_InGameMenu L1798-1856: hit-test de los 2 botones (DAT_083a42f8, stride
    // 5 ints [id][x][y][w][h]; Yes=btn0 id1, No=btn1 id3, render en +213/+100) y
    // seteo de DAT_00559f5e = 1 (Yes) / 2 (No), que el drop-dispatcher
    // (FUN_004df410) consume para enviar/cancelar el sell.
    case 0x97:
    {
        int clickResult[2] = { -1, -1 };
        int* rect = (int*)&DAT_083a42f8;     // &id del botón 0
        for (int bi = 0; bi < 2; ++bi) {
            int id = bi * 2 + 1;             // 1 (Yes), 3 (No)
            rect[0] = id;
            int bx = rect[1], by = rect[2], bw = rect[3], bh = rect[4];
            if (bx + 213 <= mouseX && by + 100 <= mouseY &&
                mouseX <= bx + bw + 213 && mouseY <= by + bh + 100) {
                if (IsClickPushed()) clickResult[bi] = bi;
                rect[0] = id + 1;            // highlight (hover bitmap)
            }
            rect += 5;
        }
        DAT_083a4124 = 0;                    // consume MouseLButtonPush

        if (clickResult[0] == 0 || clickResult[1] == 1) {
            DAT_00559f5e = (clickResult[1] == 1) ? (char)2 : (char)1;  // No=2, Yes=1
            DAT_083a7c24 = DAT_083a7c28;     // ErrorMessage = NextErrorMessage
            DAT_083a7c28 = 0;
            FUN_00404bc0(0x19, 0, 0);        // PlayBuffer(25)
        }
        return;
    }

    // ── Clear input buffers ───────────────────────────────────────────────
    // Party invitation (ErrorMessage 120).  UI_InGameMenu @ 00514310
    // sends C3(C1:06:41:01:key) for the primary button and the plain
    // C1:06:41:00:key form when Escape/secondary cancel is used.
    case 120:
    {
        const bool accept = mouseX >= 234 && mouseX < 304 &&
                            mouseY >= 98  && mouseY < 119 && IsClickPushed();
        const bool reject = escHit ||
                            (mouseX >= 334 && mouseX < 404 &&
                             mouseY >= 98  && mouseY < 119 && IsClickPushed());
        if (!accept && !reject) return;

        const WORD partyKey = (WORD)DAT_07eaa0e4;
        BYTE pkt[6] = { 0xC1, 0x06, 0x41, (BYTE)(accept ? 1 : 0),
                        (BYTE)(partyKey >> 8), (BYTE)partyKey };
        if (accept) {
            Net_SendSmallPacket(pkt, sizeof(pkt));
        } else if (DAT_055ca168 != 0xFFFFFFFF) {
            ::send(DAT_055ca168, (const char*)pkt, sizeof(pkt), 0);
        }

        DAT_083a4124 = 0;
        DAT_083a7c24 = DAT_083a7c28;
        DAT_083a7c28 = 0;
        FUN_00404bc0(0x19, 0, 0);
        DbgLogPublic(accept ? "PARTY: invitation accepted (0x41/C3)"
                            : "PARTY: invitation rejected (0x41/C1)");
        return;
    }

    case 0x98:
        memset(DAT_07db8710, 0, 0x100);
        *(DWORD*)DAT_07d780a8 = 0;
        DAT_07d552e4  = 0;
        DAT_07d780ac  = 0;
        goto tail;

    // ── Multi-select item list (2-slot result) ────────────────────────────
    case 0x99:
    {
        // Requires click — not hover.
        int *piVar = (int *)&DAT_083a42fc;
        int  count = (int)DAT_083a4324;
        for (int i = 0; i < count; ++i)
        {
            int rowY = 0x2c + i * 0x10;
            if (mouseY >= rowY && mouseY < rowY + 0x10 &&
                mouseX >= 0x6a  && mouseX <= 0x16a && IsClickPushed())
            {
                DAT_083a4124 = 0;
                DAT_00559f5e = (char)i;
                DAT_00559f5f = (char)DAT_083a7c2c;
                goto tail;
            }
        }
        goto tail;
    }

    default:
        // BUG-FIX 2026-07-14: los estados no manejados (incluidos los códigos de
        // ErrorMessage del login — 0x16 wrong-pass, 0x65 sin-pass, etc., que
        // comparten el global DAT_083a7c24) NO deben caer al `tail` cada frame.
        // El `tail` hace `DAT_083a7c24 = DAT_083a7c28` (dismissea el cartel) +
        // PlayBuffer(25) — antes corría incondicionalmente → el cartel de error
        // del login se borraba 1 frame después de aparecer (con el "sonido de
        // click" que reportó el usuario). El dismiss debe pasar SOLO al click en
        // el botón OK, que RenderErrorMessage (FUN_0051af50 default box path)
        // dibuja en (284,98)-(354,119). Con click → tail (dismiss). Sin click →
        // el cartel persiste, fiel al original.
        {
            // OK vía click en el botón O tecla Enter (DAT_055ca038). El Enter se
            // consume acá para que el login-trigger de Game_SceneUpdate no lo
            // tome como reintento (fiel al IDA 0x514310 L1391).
            bool enterHit = (DAT_055ca038 != '\0');
            bool okClick = (mouseX >= 284 && mouseX < 354 &&
                            mouseY >= 98 && mouseY < 119 && IsClickPushed());
            if (okClick || enterHit) {
                DAT_083a4124 = 0;     // consume click
                DAT_055ca038 = '\0';  // consume Enter
                goto tail;            // dismiss: shift ErrorMessage + PlayBuffer(25)
            }
        }
        return;
    }

    // ── Second outer switch: close-button hit test at X=0x175 ────────────
    // This runs after the main switch for some states that also have a
    // close button overlay. Requires click — not hover.
    if (mouseX >= 0x175 && mouseX <= 0x185 && IsClickPushed())
    {
        DAT_083a4124 = 0;
        switch (state)
        {
        case 0x72:
            DAT_083a7c14 = 0x18;
            DAT_083a7c18 = 0x15;
            FUN_00404bc0(0x1b, 0, 0);
            break;

        case 0x77:
        {
            BYTE hi = (BYTE)((DAT_07eaa0d8 >> 8) & 0xff);
            BYTE lo = (BYTE)(DAT_07eaa0d8 & 0xff);
            BYTE pkt[6] = { 0xC1, 0x06, 0x36, 0x00, lo, hi };
            SendLoginPacket(pkt, 6);
        }
        break;

        case 0x79:
        {
            BYTE pkt[4] = { 0xC1, 0x04, 0x30, 0x00 };
            SendLoginPacket(pkt, 4);
        }
        break;

        case 0x7e:
        case 0x98:
            memset(DAT_07db8710, 0, 0x100);
            *(DWORD*)DAT_07d780a8 = 0;
            DAT_07d552e4  = 0;
            break;

        case 0x80:
        {
            BYTE pkt[4] = { 0xC1, 0x04, 0xF1, 0x03 };
            SendLoginPacket(pkt, 4);
            FUN_00423db0();
            FUN_00423db0();
        }
        break;

        default:
            break;
        }
    }

tail:
    // ── Apply queued state transition ────────────────────────────────────
    // Pattern: DAT_083a7c24 = DAT_083a7c28; DAT_083a7c28 = 0
    DAT_083a7c24  = DAT_083a7c28;
    DAT_083a7c28  = 0;
    DAT_07e11d28  = 0;
    DAT_00559bec  = 6;
    FUN_00404bc0(0x19, 0, 0);
}
