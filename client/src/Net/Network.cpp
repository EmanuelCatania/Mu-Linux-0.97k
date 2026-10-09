// Network.cpp — CNetwork. Ver Network.h.

#include "stdafx.h"
#include "Net/Reconnect.h"
#include "Game/HeroVitals.h"
#include "Net/Ping.h"
#include "Net/Network.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"
#include "Net/PacketFrame.h"
#include "Net/ServerCharacterStats.h"
#include "UI/HealthBar.h"
#include "UI/MoveList.h"
#include "UI/EventTimer.h"
#include "UI/GoldenArcher.h"
#include "Item/ChaosMixRates.h"
#include "Item/Item_ServerValue.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" void CsmWatchdog(const char *tag);

CNetwork gNetwork;

void CNetwork::ResetCharacterData()
{
    gServerCharacterStats.Reset();
    gHeroVitals.Reset();
    gHealthBar.Clear();
    gChaosMixRates.Reset();
    GoldenArcher_ResetCharacter();
    // DSProtocol::DGCharacterInfoRecv reenvía M y las tablas para el personaje.
    gMoveList.Clear();
    ItemServerValue_ResetSession();
    gEventTimer.Close(); // El calendario pertenece a la misma conexión.
}

void CNetwork::ResetSessionData()
{
    ResetCharacterData();
    gPing.Reset();
    gEventTimer.Clear();
    GoldenArcher_ResetSession();
}

// Clave del chain-XOR. Es la misma que arma el binario antes de cada envío
// (los 32 bytes de `local_d58`).
const BYTE CNetwork::XorKey[32] = {
    0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
    0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
    0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
    0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
};

// ─────────────────────────────────────────────────────────────────────────────
// Eventos del socket — IDA WndProc (0x004149D0), case WM_USER
// ─────────────────────────────────────────────────────────────────────────────

void CNetwork::OnSocketEvent(WORD evt, WORD err)
{
    // Compatibilidad MuEmu: tras F4/03 redirect podemos reconectar a un
    // socket nuevo antes de que Windows entregue el FD_CLOSE del socket
    // viejo. Si procesamos ese evento tardío como si fuera del socket
    // actual, cerramos la sesión nueva inmediatamente.
    extern int  __fastcall CWsctlc_nRecv(void* ctx);
    extern int  __fastcall CWsctlc_FDWriteSend(int  ctx);
    extern void Net_ProcessPacket(void);
    if (evt & 0x01) { // FD_READ
        CWsctlc_nRecv((void*)(uintptr_t)SocketClient);
        CsmWatchdog("after-Recv");        // catches any future trample regression
        // Durante OpenWorld el pump de AccessModel reentra aca.
        // Se drena el socket (arriba) para que el server no cierre por
        // backpressure, pero NO se despachan los paquetes: quedan en la cola
        // y los procesa el frame siguiente, ya con los modelos cargados.
        // Sin esto, el 0x13 ViewportMonster creaba monstruos a medio cargar.
        if (!g_WorldLoading)
            Net_ProcessPacket();
    }
    if (evt & 0x02) { // FD_WRITE
        CWsctlc_FDWriteSend((int)(uintptr_t)SocketClient);
        // La conexión no-bloqueante completa suele señalizarse con el primer
        // FD_WRITE (el mask WSAAsyncSelect es 0x23, sin FD_CONNECT), así que el
        // pedido de lista al ConnectServer también sale de acá.
        RequestServerList();
    }
    if (evt & 0x10) { // FD_CONNECT
        if (err == 0)
            RequestServerList();
    }
    if (evt & 0x20) { // FD_CLOSE
        // DESVIACION (DLL Reconnect.cpp): en el juego, un cierre que no pidió
        // el jugador arranca la reconexión en vez de sólo avisar.
        const bool reconnecting = !m_ConnectServerMode && gReconnect.OnConnectionLost();
        ResetSessionData();
        // IDA WndProc @ 0x004149D0 case FD_CLOSE (original behaviour):
        //   UIChatLogWindow_AddText(strID, GlobalText[3], 1);
        //   CWsctlc::Close(&SocketClient);
        // ── No MessageBoxA, no PostQuitMessage. Solo log y cierre local.
        // El cierre del socket por logout limpio (F1/02/* iniciado por el
        // cliente) NO debe matar el proceso — el countdown de "Salir" o el
        // local-transition de "Seleccionar Servidor" se encargan de la
        // siguiente fase. Para FD_CLOSE no solicitado (server kick / red
        // caída) la UI de login mostrará el chat-log y el usuario verá
        // "Conexión cerrada" sin que el cliente se mate solo.
        extern void UIChatLogWindow_AddText(const char* strID, const char* msg, int color);
        if (!reconnecting)
            UIChatLogWindow_AddText((const char*)&DAT_083a7c5c, GlobalText[3], 1);
        if (SocketClientSocket != 0xffffffff) {
            closesocket((SOCKET)SocketClientSocket);
            SocketClientSocket = (DWORD)INVALID_SOCKET;
        }
    }
}

// F1/01 — armado que antes estaba en Game_SceneUpdate (envío del login).
// Layout PMSG_CONNECT_ACCOUNT_SEND: cuenta y contraseña con el XOR de 3 bytes
// (PacketArgumentEncrypt), TickCount, ClientVersion ofuscada y Serial.
void CNetwork::SendLogin(const char* account, const char* password)
{
    BYTE user[10] = {}, pass[10] = {};
    memcpy(user, account, strnlen(account, 10));
    memcpy(pass, password, strnlen(password, 10));
    static const BYTE kArgXor[3] = { 0xFC, 0xCF, 0xAB };
    BYTE pkt[64] = {0};
    pkt[0] = 0xC1;
    pkt[1] = 0x01;     // placeholder — serial stomp overwrites
    pkt[2] = 0xF1;     // head (plaintext)
    pkt[3] = 0x01;     // subh (plaintext) — CGConnectAccountRecv dispatch
    int  pos = 4;

    // account[10] — XOR with 3-byte rotating mask (PacketArgumentEncrypt).
    for (int i = 0; i < 10; i++) {
        pkt[pos + i] = (BYTE)(user[i] ^ kArgXor[i % 3]);
    }
    pos += 10;

    // password[10] — same XOR.
    for (int i = 0; i < 10; i++) {
        pkt[pos + i] = (BYTE)(pass[i] ^ kArgXor[i % 3]);
    }
    pos += 10;

    // TickCount (4 bytes LE) — plaintext.
    DWORD tick = GetTickCount();
    memcpy(pkt + pos, &tick, 4);
    pos += 4;

    // ClientVersion[5] — obfuscated as (v[i] - i - 1), plaintext after.
    for (int i = 0; i < 5; i++) {
        pkt[pos + i] = (BYTE)(Version[i] - (char)i - 1);
    }
    pos += 5;

    // ClientSerial[16] — raw copy, plaintext.
    memcpy(pkt + pos, Serial, 16);
    pos += 16;
    // pkt[1] := serial is set later by gNetwork.Send

    // ── LoginKey chain XOR ──────────────────────────────────────
    // Se aplica en gNetwork.Send / gNetwork.SendLarge, para TODOS los
    // paquetes salientes (F1/05 HWID incluido), igual que el companion
    // (Protocol.cpp ExtractPacket).
    int totalLen = pos;

    // Compute CRC and build final send buffer
    int crc = CSimpleModulus_Encode(0, pkt + 1, totalLen - 1);
    if (crc < 0x100) {
        gNetwork.Send(pkt, totalLen);
    } else {
        gNetwork.SendLarge(pkt, totalLen);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ConnectServer
// ─────────────────────────────────────────────────────────────────────────────

void CNetwork::BeginConnectServerSession()
{
    ResetSessionData();
    m_ConnectServerMode = true;
    m_ServerListRequested = false;
}

void CNetwork::EndConnectServerSession()
{
    m_ConnectServerMode = false;
    m_ServerListRequested = false;
}

// C1 04 F4 02 (PMSG_SERVER_LIST_RECV), plano. Sale una sola vez por sesión.
void CNetwork::RequestServerList()
{
    if (!m_ConnectServerMode || m_ServerListRequested) return;
    const BYTE req[4] = { 0xC1, 0x04, 0xF4, 0x02 };
    SendToConnectServer(req, 4);
    m_ServerListRequested = true;
}

// Envío plano (sin MuEmu ni serial) al ConnectServer, que no usa la
// encriptación del GameServer: sus pedidos (C1 04 F4 02 / C1 06 F4 03) van
// crudos. Mismo patrón de cola por WSAEWOULDBLOCK que CServerSelWin en el
// binario.
void CNetwork::SendToConnectServer(const BYTE* data, int len)
{
    if (SocketClientSocket == 0xffffffff) return;
    unsigned int remain = (unsigned int)len;
    int off = 0;
    while ((int)remain > 0) {
        int r = ::send(SocketClientSocket, (const char*)data + off, (int)remain, 0);
        if (r == -1) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                if (SocketClientSendBufferLength + (int)remain < 0x2001) {
                    memcpy((char*)SocketClientSendBuffer + SocketClientSendBufferLength, data + off, remain);
                    SocketClientSendBufferLength += (int)remain;
                } else {
                    Net_Disconnect((int)(uintptr_t)SocketClient);
                }
            } else {
                Net_Disconnect((int)(uintptr_t)SocketClient);
            }
            return;
        }
        if (r == 0) return;
        remain -= (unsigned int)r;
        off += r;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Envío al GameServer
// ─────────────────────────────────────────────────────────────────────────────

// Send len bytes from buf over the game socket with WSAEWOULDBLOCK fallback.
// Returns 0 on success, -1 on hard error.
//
// MuEmu integration: the whole buffer is HackCheck-encrypted in place
// (symmetric byte stream cipher — see MuEmu.h) before hitting the wire.
// The server blindly decrypts every inbound byte, so sending plaintext
// results in a "Protocol header error" immediate disconnect.
int CNetwork::SendRaw(const char* buf, int len)
{
    if (SocketClientSocket == 0xffffffff) return 0;

    // Copy to a mutable scratch buffer so we can encrypt in place without
    // clobbering caller state (some callers reuse `buf` across retries).
    static BYTE s_scratch[0x2000];
    if (len <= 0 || (int)sizeof(s_scratch) < len) return -1;
    memcpy(s_scratch, buf, len);
    MuEmu::EncryptSend(s_scratch, len);

    int sent = 0;
    int rem  = len;
    do {
        int n = send(SocketClientSocket, (const char*)s_scratch + sent, rem, 0);
        if (n == -1) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                if (SocketClientSendBufferLength + (len - sent) < 0x2001) {
                    // Queue the still-unsent ENCRYPTED tail — never re-queue
                    // plaintext or the prefix we already transmitted.
                    memcpy(SocketClientSendBuffer + SocketClientSendBufferLength,
                           s_scratch + sent, (len - sent));
                    SocketClientSendBufferLength += (len - sent);
                } else {
                    Net_Disconnect(((int)(uintptr_t)SocketClient));
                }
            } else {
                Net_Disconnect(((int)(uintptr_t)SocketClient));
            }
            return -1;
        }
        if (n == 0) break;
        if (SocketClientLogPrint) FUN_0043de60();
        sent += n;
        rem  -= n;
    } while (rem > 0);
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Emisores de paquetes al GameServer
// ─────────────────────────────────────────────────────────────────────────────
// `Net_SendFrameC1` y `Net_SendFrameC3` arman cada frame; `gNetwork.Send`
// y `gNetwork.SendC1` son las entradas publicas y **eligen el frame solas**
// consultando PacketFrame_WantsEncrypt (tabla portada de HackPacketCheck.txt).
//
// Antes cada call site elegia el frame a mano y equivocarse desconectaba sin
// aviso ("Packet encryption error" -> CloseClient). Ahora el call site puede
// llamar a cualquiera de las dos: si el server pide el otro frame, se corrige
// y queda registrado en debug.log.
//
// Los dos trabajan sobre una COPIA del buffer del caller. Antes el chain-XOR y
// el serial se escribian sobre el buffer original, lo que ademas de sorprender
// al caller haria imposible re-enmarcar (el XOR quedaria aplicado dos veces).

// Net_SendFrameC1 — frame plano.
//
// El chain-XOR se aplica igual que en los C3: el server hace `XorData` sobre
// TODO frame C1 (`CPacketManager::ExtractPacket` -> `XorData(size-1, 2)`), asi
// que hay que cifrar `pkt[3..len-1]`. Para paquetes de 3 bytes el bucle no
// itera (no hay payload), que es justo el caso de los pedidos de lista.
//
// A diferencia del C3, aca NO se pisa `pkt[1]` con el serial: los frames C1 no
// lo llevan y `CSerialCheck` no los valida.
void CNetwork::SendFrameC1(const BYTE* pkt, int totalLen)
{
    BYTE buf[0x402];
    memcpy(buf, pkt, totalLen);
    for (int i = 3; i < totalLen; i++) {
        buf[i] ^= buf[i - 1] ^ XorKey[i & 0x1f];
    }
    SendRaw((char*)buf, totalLen);
}

// Net_SendFrameC3 — frame encriptado: chain-XOR + serial + CSimpleModulus.
//
// El companion (Mu-linux-97K Source/Client/Main/Protocol.cpp:983) llama
// gPacketManager.ExtractPacket(EncBuff) DENTRO de DataSend antes del CSM
// encrypt, y ExtractPacket aplica XorData sobre todos los bytes del paquete.
// El server (GameServer/PacketManager.cpp XorData) lo reversa en sentido
// inverso. Sin este chain el server ve basura (subh corrupto): para F1/05 HWID
// veia subh=0x7D en vez de 0x05, nunca despachaba CGSetHwidRecv, y
// lpObj->HardwareID quedaba "" -> F1/01 LoginResult code=05.
//
// El chain va ANTES del serial-stomp, en el mismo orden que el companion.
//
// Serial: CProtocol::DataSend pisa el byte de tamano con el contador rodante
// (g_byPacketSerialSend / DAT_05826ceb) justo antes de encriptar, asi que el
// primer byte DESENCRIPTADO del lado del server es el serial, no el tamano.
// El server lo lee como DecSerial y exige la secuencia 0,1,2,... via
// CheckSerial; cualquier salto -> CloseClient.
void CNetwork::SendFrameC3(const BYTE* pkt, int totalLen)
{
    BYTE plain[0x402];
    char buf[0x402];
    memcpy(plain, pkt, totalLen);

    for (int i = 3; i < totalLen; i++) {
        plain[i] ^= plain[i - 1] ^ XorKey[i & 0x1f];
    }

    plain[1] = DAT_05826ceb++;           // el byte de tamano pasa a ser el serial
    int  bodyLen = totalLen - 1;
    int  encLen  = CSimpleModulus_Encode(0, plain + 1, bodyLen);
    int  total   = encLen + 2;
    buf[0] = (char)0xC3;
    buf[1] = (char)total;
    CSimpleModulus_Encode((int)(buf + 2), plain + 1, bodyLen);
    SendRaw(buf, total);
}

// Elige el frame segun la tabla del server y avisa cuando corrige al caller.
// `chosenC3` es lo que pidio el call site; se respeta salvo que el server exija
// lo contrario.
void CNetwork::SendResolved(const BYTE* pkt, int totalLen, bool chosenC3, const char* who)
{
    if (!pkt || totalLen < 3 || totalLen > 0x400) return;

    const BYTE opcode = pkt[2];
    const BYTE subop  = (totalLen > 3) ? pkt[3] : 0;
    const int  want   = PacketFrame_WantsEncrypt(opcode, subop);

    bool useC3 = chosenC3;

    if (want == PACKETFRAME_PLAIN || want == PACKETFRAME_ENCRYPTED) {
        const bool serverWantsC3 = (want == PACKETFRAME_ENCRYPTED);
        if (serverWantsC3 != chosenC3) {
            useC3 = serverWantsC3;
        }
    }
    else if (want == PACKETFRAME_UNKNOWN) {
        // El server no tiene fila para este par: responde "Packet unknown
        // error" y cierra. Se manda igual (no queremos cambiar que un paquete
        // salga o no), pero queda el aviso para no perder una hora buscando
        // una desconexion sin causa aparente.
        char line[160];
        wsprintfA(line,
                  "PacketFrame: 0x%02X/%02X no figura en HackPacketCheck.txt "
                  "-> el server va a cerrar la conexion (%s)", opcode, subop, who);
        DbgLogPublic(line);
    }
    // PACKETFRAME_ANY (0xF3): sirve cualquiera, se respeta lo que pidio el caller.

    if (useC3) SendFrameC3(pkt, totalLen);
    else       SendFrameC1(pkt, totalLen);
}

// Entrada publica para los call sites que esperan un frame ENCRIPTADO (C3).
void CNetwork::Send(const BYTE* pkt, int totalLen)
{
    SendResolved(pkt, totalLen, true, "Send");
}

// Entrada publica para los call sites que esperan un frame PLANO (C1).
void CNetwork::SendC1(const BYTE* pkt, int totalLen)
{
    SendResolved(pkt, totalLen, false, "SendC1");
}

// Send a large packet: input is plaintext [0xC1][len][opcode][payload].
// Encrypts pkt[1..totalLen-1] and wraps with [0xC4][hi][lo] framing.
// Same serial-stomp rationale as gNetwork.Send above (companion uses
// EncBuff[2] for C4 — the byte right after the 2-byte wire size), but our
// input format has 1-byte size at pkt[1] so we stomp there for consistency.
void CNetwork::SendLarge(const BYTE* pkt, int totalLen)
{
    char buf[0x802];

    // Chain XOR universal — ver gNetwork.Send arriba para detalles.
    // Mismo formato de input (pkt[0]=C1 placeholder, pkt[1]=size, pkt[2]=head,
    // pkt[3]=subh) → chain desde i=3 igual que el path C3.
    {
        BYTE* mp = (BYTE*)pkt;
        for (int i = 3; i < totalLen; i++) {
            mp[i] ^= mp[i - 1] ^ XorKey[i & 0x1f];
        }
    }

    ((BYTE*)pkt)[1] = DAT_05826ceb++;    // replace size byte with serial
    int  bodyLen = totalLen - 1;
    int  encLen  = CSimpleModulus_Encode(0, (unsigned char*)(pkt + 1), bodyLen);
    int  total   = encLen + 3;
    buf[0] = (char)0xC4;
    buf[1] = (char)((total + ((total >> 31) & 0xff)) >> 8);
    buf[2] = (char)total;
    CSimpleModulus_Encode((int)(buf + 3), (unsigned char*)(pkt + 1), bodyLen);
    SendRaw(buf, total);
}
