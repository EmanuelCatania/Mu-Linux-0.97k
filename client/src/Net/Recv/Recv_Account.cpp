// Recv_Account.cpp — paquetes del server: cuenta y conexión: login, lista de servers, redirect, logout.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Ping.h"
#include "UI/HealthBar.h"
#include "Net/Recv/NetRecv.h"
#include "Net/ServerCharacterStats.h"

// IDA: FUN_00433A80 ReceiveGGAuth. Pertenece al flujo de protocolo/autenticación,
// no a Party: un 0x73 sin cifrar solicita la respuesta cifrada F1/03/00/F1,
// mientras que un 0x73 cifrado lleva el puntero de recurso consumido por 53D5C0.
void ReceiveGGAuth97k(BYTE* packet, int size, bool encrypted)
{
    if (encrypted) {
        if (size >= 8) {
            char* resource = *(char**)(packet + 4);
            if (resource) Pipe_QueryResource(resource);
        }
        return;
    }

    const BYTE reply[6] = { 0xC1, 0x06, 0xF1, 0x03, 0x00, 0xF1 };
    gNetwork.Send(reply, sizeof(reply));
}

// ---------------------------------------------------------------------------
// F1/00 — ReceiveJoinServer  (@ 0x00424010)
// Server saluda tras conectar. Sub-byte Msg[4]: 1=OK, otro=fail.
// ---------------------------------------------------------------------------
void Recv_JoinServer(const BYTE* Msg)
{
    gPing.Reset();
    gServerCharacterStats.Reset();
    gHealthBar.Clear();
    if (Msg[4] == 1) {
        gPing.SetServer((SOCKET)SocketClientSocket);
        g_HeroKey      = (unsigned short)(Msg[6] | (Msg[5] << 8));
        // HeroKey (el que usa ClearCharacters vía OpenWorld) tiene que llevar el Key
        // real: con 0, ClearCharacters(0) conservaría las entidades con Key==0
        // (el Hero stale del slot 0) → fantasma renderizado + hover pegado.
        HeroKey   = g_HeroKey;
        CurrentProtocolState   = 2;
        DAT_083a7c14   = 2;          // login sub-state = CredentialInput
        PlayBuffer(27, 0, 0);
        NetLog("NET:    JoinServer OK: HeroKey=%d state→2/2", g_HeroKey);
        // ── F1/05 HWID (desviación MuEmu) ──────────────────────────────────────────
        // El server MuEmu requiere F1/05 SetHwid antes del F1/01 — sin él,
        // CheckHardwareID en Blacklist.cpp considera el HWID vacío como
        // blacklisted y devuelve code 05.
        //
        // Compatibilidad MuEmu: el deploy actual corta la sesión temprano si
        // tras F1/00 no recibe también F1/04 antes de F1/05/F1/01. Esto no
        // existe en el flujo Webzen original; queda aislado acá.
        //   Lang_Send(1);
        HWID_Send();
    } else {
        NetLog("NET:    JoinServer FAIL code=%d → SetErrorMessage(113)", Msg[4]);
        SetErrorMessage(113);        // "Connecting error"
    }
    // Version sanity: bytes 7..11 deben ser Version[i]-i-1. No bloqueante.
}

// ---------------------------------------------------------------------------
// F1/01 — Login result  (inline en ProtocolCore, cases 0..0x11/0xC0..0xD2)
// Sub-byte Msg[4] → mapea a CurrentProtocolState = 20..38.
// ---------------------------------------------------------------------------
void Recv_LoginResult(const BYTE* Msg)
{
    DWORD state;
    switch (Msg[4]) {
        case 0x00: state = 21; break;
        case 0x01: state = 20; break;   // LogIn success → LogIn=2, CheckHack
        case 0x02: state = 22; break;
        case 0x03: state = 23; break;
        case 0x04: state = 24; break;
        case 0x05: state = 25; break;
        case 0x06: state = 26; break;   // version mismatch
        case 0x08: state = 28; break;
        case 0x09: state = 37; break;
        case 0x0A: state = 29; break;
        case 0x0B: state = 30; break;
        case 0x0C: state = 31; break;
        case 0x0D: state = 32; break;
        case 0x11: state = 38; break;
        case 0xC0: case 0xD0: state = 34; break;
        case 0xC1: case 0xD1: state = 35; break;
        case 0xC2: case 0xD2: state = 36; break;
        default:   state = 27; break;
    }
    DAT_05826cb0 = state;
    DAT_083a7c14 = 3;
    // Log del resultado de login crudo por intento.  Msg[4]=code server,
    // state=nuestro DAT_05826cb0.
    NetLog("NET:  → F1/01 LOGIN-RESULT code=0x%02X -> state=%u (t=%lu)",
           Msg[4], state, (unsigned long)GetTickCount());

    // DESVIACIÓN: auto-reintento del PRIMER login fallido con code 0x02.  Contra
    // MuEmu el 1er F1/01 se rechaza (0x02 = pass incorrecta) con datos IDÉNTICOS al
    // 2do intento que SÍ entra (0x01) — quirk del primer paquete C3, no es la
    // contraseña.  Reintentamos UNA sola vez simulando Enter (DAT_055ca038),
    // que hace que Game_SceneUpdate re-envíe las MISMAS credenciales.  Si la
    // pass fuera realmente incorrecta, el 2do intento también da 0x02 y ahí sí
    // se muestra el error.  Sólo 0x02 — no reintentamos banned/already-online/
    // version-mismatch para no enmascararlos.
    {
        static int s_loginAutoRetried = 0;
        if (Msg[4] == 0x02 && !s_loginAutoRetried) {
            s_loginAutoRetried = 1;
            DAT_055ca038 = 1;   // simula Enter → re-envía el login (mismas creds)
            NetLog("NET:    F1/01 AUTO-RETRY (first 0x02 = likely first-packet quirk)");
        } else {
            s_loginAutoRetried = 0;   // reset en éxito o en 2do fallo
        }
    }
}

// ---------------------------------------------------------------------------
// F1/02 — ReceiveLogOut  (@ 0x004247D0)
// Respuesta del server al packet C1/05/F1/02/<sub> que envía UI_InGameMenu
// (botones Salir / Ir-a-otro-server / Ir-a-otro-char).  Sub-byte Msg[4]:
//   0 = Exit:   WM_DESTROY (cierra el proceso).
//   1 = JoinChar (volver a char-select sin reconectar): mantiene socket,
//       SceneFlag=4, descarga in-game scene, pide F3/00 char list.
//   2 = JoinSrv (volver a server-select): cierra socket, SceneFlag=2,
//       reset login init flags, vuelve a Scene_Login_ServerSelect.
//
// IDA hace además un ack-resend del packet cuando bEncrypted=false (echo),
// pero para nuestro flujo MuEmu-encrypted basta con la transición de estado.
// ---------------------------------------------------------------------------
void Recv_LogOut(const BYTE* Msg)
{
    gServerCharacterStats.Reset();
    gHealthBar.Clear();
    BYTE sub = Msg[4];
    NetLog("NET:    F1/02 LogOut sub=%d gs=%d", sub, (int)SceneFlag);

    if (sub == 0) {
        // Exit: WM_DESTROY → WndProc cleanup → PostQuitMessage
        SendMessageA(gWindow.GetHwnd(), WM_DESTROY, 0, 0);
        return;
    }

    if (sub == 1) {
        // JoinChar — back to char-select, keep connection open
        if (SceneFlag == 5) {
            StopMusic();
            AllStopSound();
            Item_ReturnPickedItem();              // CharPreview_Refresh
            ReleaseMainData();
        }
        SceneFlag = 4;                // SceneFlag = CharSelect
        DAT_083a7c14 = 0;                // sub-state reset (will be set to 0x14/0x15 by EnterWorldTick init)
        DAT_083a7c18 = 0;
        DAT_05826cb0 = 50;               // CurrentProtocolState
        // RESET de las guardas de init de escena. EnterWorldTick (state=4) y
        // CharSelectTick (state=5) tienen guardas de init (IDA: DAT_083a7c4b/4c) que
        // sólo permiten re-inicializar la escena la PRIMERA vez. Sin reset, el
        // substate DAT_083a7c14 heredado del primer login (0x1c, post-OK-click)
        // dispararía un F3/03 select-char inmediato → el server respondería con
        // JoinMapServer → el cliente recargaría el mapa en vez de mostrar char-select.
        CharSelectSceneInitialized = 0;
        DAT_083a7c4c = 0;                // CharSelect per-tick init
        DAT_083a7c10 = 0;                // IDA: EnableMainRender (0x083A7C10)
        DAT_083a4299 = 0;                // double-click flag
        DAT_083a4124 = 0;                // single-click flag
        DAT_005616ac = -1;               // selected slot
        DAT_005616b0 = -1;               // creation slot
        // Manda el pedido de lista de personajes F3/00 vía gNetwork.Send para que
        // salga como C3 (encriptado) con el contador de serial correcto + chain-XOR. Los
        // envíos C1 planos, sin serial, el server los descartaba en silencio en
        // CheckSerial (SocketManager.cpp:328-330) — lee DecBuff[1] como
        // DecSerial y rechaza el paquete si se rompe la monotonía.
        BYTE pkt[4] = { 0xC1, 0x04, 0xF3, 0x00 };
        NetLog("NET:    F3/00 char-list request (post-JoinChar)");
        gNetwork.Send(pkt, 4);

        // ── Cola de ReceiveLogOut ──────────────────────────────────────────────────
        // IDA 0x4247D0 LABEL_117: DESPUÉS del send, la rama sub==1 hace
        // `CurrentProtocolState = 0` e `InitGame()`, igual que la rama sub==2.
        // El `World = -1` de InitGame importa porque nuestro RequestTerrainHeight
        // (Terrain_Utils.cpp) gatea con `World < 0` en vez del `SceneFlag != 5` del
        // original — una desviación deliberada por el orden del JoinMapServer. Con el
        // World anterior y su heightmap todavía cargado, CreateCharacterPointer le
        // daría a cada personaje del char-select la altura del terreno de ese mapa.
        DAT_05826cb0 = 0;                // CurrentProtocolState
        InitGame();
        return;
    }

    if (sub == 2) {
        // JoinSrv — back to login/server-select, close socket
        if (SceneFlag == 5) {
            StopMusic();
            AllStopSound();
            Item_ReturnPickedItem();
            ReleaseMainData();
        }
        CWsctlc_Close((int)(uintptr_t)SocketClient);  // Net_Disconnect (close socket)
        ReleaseCharacterSceneData(); // IDA: ReleaseCharacterSceneData (0x005102C0)
        SceneFlag = 2;                // SceneFlag = Login
        DAT_083a7c14 = 0;                // sub-state = ServerSelect
        DAT_083a7c18 = 0;
        DAT_05826cb0 = 0;                // CurrentProtocolState
        DAT_083a7c48 = 0;                // ConnectionCheckEnable
        DAT_083a7c49 = 0;                // InitLogIn
        CharSelectSceneInitialized = 0;
        DAT_083a7c4c = 0;                // InitMainScene
        DAT_083a7c10 = 0;                // IDA: EnableMainRender (0x083A7C10)
        InitGame();
        return;
    }
}

// ---------------------------------------------------------------------------
// F4/04 — CustomServerList (ConnectServer CCCustomServerListSend)
// Pobla la tabla local de nombres DAT_07d52c34 (stride 300, indexada por
// ServerCode/20) que ReceiveServerList (F4/02) lee para el nombre de cada
// grupo. Formato (C2 header):
//   [C2][sizeHB][sizeLB][F4][04][countHB][countLB] + count × {WORD ServerCode; char Name[32]}
//   → entry stride 34 bytes.
// ---------------------------------------------------------------------------
void Recv_CustomServerList(const BYTE* Msg)
{
    int count = (Msg[5] << 8) | Msg[6];   // SET_NUMBERHB/LB → HB en [5], LB en [6]
    const BYTE* p = Msg + 7;
    for (int i = 0; i < count && i < 24; i++) {
        unsigned short serverCode = (unsigned short)(p[0] | (p[1] << 8));
        const char* name = (const char*)(p + 2);
        int group = serverCode / 20;
        if (group >= 0 && group < 24)
            lstrcpynA(DAT_07d52c34 + 300 * group, name, 32);  // deja hueco null-term
        p += 34;
    }
    NetLog("NET:  → F4/04 CustomServerList count=%d (names populated)", count);
}

// ---------------------------------------------------------------------------
// F4/02 — ReceiveServerList  (@ 0x00423E10)  — port FIEL desde IDA.
// Lista de game servers publicada por el ConnectServer. Cada entrada trae
// {WORD ServerCode; BYTE UserTotal; pad} (stride 4). UserTotal (0..100) es el
// load que Scene_Login_ServerSelect Pass 4 dibuja como cuadros llenos.
// El nombre de cada grupo se copia desde DAT_07d52c34 (poblada por F4/04).
// Layout del display array (DAT_083a45d8, stride 542 por slot):
//   +0x14 num_channels · +0x2c channel_id (stride 26) · +0x2e load (stride 26)
// Slot destino = 23 - (ServerCode/20)  (o 24 para el grupo evento 12).
// ---------------------------------------------------------------------------
void Recv_ServerList(const BYTE* Msg)
{
    unsigned char count = Msg[5];
    DAT_083a7c40 = count;

    // Clear num_channels (+0x14) de todos los slots (25). Repuebla desde el
    // paquete; esto también elimina la entrada estática "MuServer" del slot 0.
    for (int s = 0; s < 25; s++)
        *(unsigned char*)(DAT_083a45d8 + 542 * s + 0x14) = 0;

    if (count == 0)
        return;

    const BYTE* entry = Msg + 6;
    for (int i = 0; i < count; i++, entry += 4) {
        unsigned short serverCode = (unsigned short)(entry[0] | (entry[1] << 8));
        unsigned char  load       = entry[2];
        int group = serverCode / 20;
        int slot;
        if (group == 12) {
            slot = 24;
            // grupo evento: nombre desde GlobalText[559] (si existe)
            if (GlobalText[559]) lstrcpynA(DAT_083a78a8, (const char*)GlobalText[559], 20);
        } else {
            slot = 23 - group;
            if (slot < 0 || slot > 24) continue;
            // nombre desde la tabla local poblada por F4/04
            lstrcpynA(DAT_083a45d8 + 542 * slot, DAT_07d52c34 + 300 * group, 20);
        }
        char* slotBase = DAT_083a45d8 + 542 * slot;
        unsigned char chan = *(unsigned char*)(slotBase + 0x14);   // num_channels actual
        *(unsigned short*)(slotBase + 0x2c + 26 * chan) = serverCode;  // channel_id
        *(unsigned char*) (slotBase + 0x2e + 26 * chan) = load;        // load (UserTotal)
        *(unsigned char*) (slotBase + 0x14) = chan + 1;               // ++num_channels
    }
    NetLog("NET:  → F4/02 ServerList parsed count=%d (loads applied)", count);
}

// ---------------------------------------------------------------------------
// F4/03 — Server redirect: cerrar socket, reconnect a (IpAddr, port)
// ---------------------------------------------------------------------------
void Recv_Redirect(const BYTE* Msg)
{
    char IpAddr[16] = {0};
    // Msg+4..Msg+19 = IP string (16 bytes), Msg+20..21 = port
    for (int i = 0; i < 15 && Msg[4 + i]; ++i) IpAddr[i] = (char)Msg[4 + i];
    unsigned short port = (unsigned short)(Msg[20] | (Msg[21] << 8));

    NetLog("NET:    Redirect → Disconnect + Connect(%s:%d)", IpAddr, port);
    // Salimos del modo ConnectServer: el socket nuevo habla con el GameServer,
    // que responderá con JoinServer (F1/00) y arranca el login normal.
    // Reactivamos la capa MuEmu (byte-XOR) que el GameServer sí usa.
    gNetwork.EndConnectServerSession();
    MuEmu::SetActive(true);
    CWsctlc_Close((int)(uintptr_t)SocketClient);   // Net_Disconnect
    CreateSocket(IpAddr, port);                   // Net_Connect
    g_bGameServerConnected = 1;                              // g_bGameServerConnected
}

// ---------------------------------------------------------------------------
// F4/05 — Vuelta a estado Connecting
// ---------------------------------------------------------------------------
void Recv_BackToConnecting(void)
{
    DAT_05826cb0 = 1;
    DAT_083a7c14 = 1;
}

// 0xF1
void NetRecv_F1(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    switch (sub) {
        case 0x00:
            NetLog("NET:  → F1/00 JoinServer result=%d hero=%d",
                   Msg[4], Msg[5] | (Msg[6] << 8));
            Recv_JoinServer(Msg);
            break;
        case 0x01:
            NetLog("NET:  → F1/01 LoginResult code=%02X", Msg[4]);
            Recv_LoginResult(Msg);
            break;
        case 0x02:
            NetLog("NET:  → F1/02 LogOut sub=%02X", Msg[4]);
            Recv_LogOut(Msg);
            break;
        default:
            NetLog("NET:  → F1/%02X unhandled", sub);
            break;
    }
}

// 0xF4
void NetRecv_F4(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    switch (sub) {
        case 0x02:
            NetLog("NET:  → F4/02 ServerList count=%d", Msg[5]);
            Recv_ServerList(Msg);
            break;
        case 0x04:
            NetLog("NET:  → F4/04 CustomServerList");
            Recv_CustomServerList(Msg);
            break;
        case 0x03:
            NetLog("NET:  → F4/03 Redirect ip=%.15s port=%d",
                   (const char*)(Msg + 4), Msg[20] | (Msg[21] << 8));
            Recv_Redirect(Msg);
            break;
        case 0x05:
            NetLog("NET:  → F4/05 BackToConnecting");
            Recv_BackToConnecting();
            break;
        default:
            NetLog("NET:  → F4/%02X unhandled", sub);
            break;
    }
}

// 0x0E
void NetRecv_0E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // LiveClient ACK (server confirma keepalive)
    NetLog("NET:  → 0x0E LiveClient ACK");
}

// 0x03
void NetRecv_03(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // HackPacketCheck.txt del server marca el opcode 3 (MainCheck) con
    // Encrypt=1: el ACK tiene que salir C3.  Mandado como C1 plano (con el
    // MuEmu byte-XOR) el server ve encrypt=0 → "Packet encryption error"
    // (Index: 3, Value: -1, Encrypt: [0][1]) → CloseClient.
    //
    // gNetwork.Send wrap correcto: chain-XOR + serial counter
    // + SimpleModulus + envelope C3.
    NetLog("NET:  → op=0x03 MainCheck challenge size=%d, sending ACK (C3)", Size);
    if (SocketClientSocket != 0xffffffff && Size >= 3) {
        BYTE ack[16];
        int ackSize = Size > (int)sizeof(ack) ? (int)sizeof(ack) : Size;
        ack[0] = 0xC1;
        ack[1] = (BYTE)ackSize;
        ack[2] = 0x03;
        // Devuelve la clave como eco (el server no valida cuando MainChecksum=0)
        if (Size > 3) {
            memcpy(ack + 3, Msg + 3, ackSize - 3);
        }
        gNetwork.Send(ack, ackSize);
    }
}

// 0x73
void NetRecv_73(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: FUN_00433A80 ReceiveGGAuth (no pertenece a Party).
    NetLog("NET:  → 0x73 GGAuth size=%d encrypted=%d", Size, (int)bEncrypted);
    ReceiveGGAuth97k(Msg, Size, bEncrypted);
}
