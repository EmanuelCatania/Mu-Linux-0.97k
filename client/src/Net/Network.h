#pragma once
// Network.h — CNetwork: eventos del socket, envío de paquetes y estado de la
// conexión (ConnectServer / GameServer).
//
// Reúne lo que estaba repartido entre WinMain (el case WM_USER del WndProc),
// Game_SceneUpdate (los emisores C1/C3/C4) y Net_Process (el envío plano al
// ConnectServer). El socket en sí sigue siendo el CWsctlc del binario
// (SocketClient, CreateSocket, CWsctlc_*).
//
//   IDA: WndProc (0x004149D0) case WM_USER — eventos de WSAAsyncSelect
//   IDA: CWsctlc::sSend / la cola de WSAEWOULDBLOCK (0x055CA16C, máx 0x2001)
//
// Formato de entrada de todos los Send*: [C1][len][opcode][payload] en claro.

#include <windows.h>

class CNetwork {
public:
    // Clave del chain-XOR del protocolo (32 bytes). La aplica el cliente sobre
    // pkt[3..len-1] y el server la revierte en CPacketManager::XorData.
    static const BYTE XorKey[32];

    // ── Eventos del socket (WSAAsyncSelect, WM_USER) ────────────────────────
    // evt = LOWORD(lParam), err = HIWORD(lParam).
    void OnSocketEvent(WORD evt, WORD err);

    // ── Envío al GameServer ─────────────────────────────────────────────────
    // Eligen el frame según PacketFrame (tabla de HackPacketCheck.txt); el
    // nombre sólo indica la preferencia del caller cuando el server acepta
    // cualquiera de los dos.
    void Send(const BYTE* pkt, int len);     // prefiere C3 (encriptado + serial)
    void SendC1(const BYTE* pkt, int len);   // prefiere C1 (plano)
    void SendLarge(const BYTE* pkt, int len);// C4, para paquetes de más de 255 bytes

    // Bytes ya enmarcados: aplica el cifrado de MuEmu y encola el resto si el
    // socket devuelve WSAEWOULDBLOCK. 0 = ok, -1 = error.
    int SendRaw(const char* buf, int len);

    // Envío plano al ConnectServer (no usa el cifrado de MuEmu).
    void SendToConnectServer(const BYTE* data, int len);

    // ── Estado de la conexión ───────────────────────────────────────────────
    // ServerConfig trae ConnectServer: el login arranca por ahí y el F4/03
    // redirige al GameServer.
    bool HasConnectServer() const { return m_HasConnectServer; }
    void SetConnectServer(bool enabled) { m_HasConnectServer = enabled; }

    // El socket actual habla con el ConnectServer.
    bool IsConnectServerMode() const { return m_ConnectServerMode; }
    void BeginConnectServerSession();   // modo CS, lista todavía sin pedir
    void EndConnectServerSession();     // vuelta al GameServer

private:
    void SendFrameC1(const BYTE* pkt, int len);
    void SendFrameC3(const BYTE* pkt, int len);
    void SendResolved(const BYTE* pkt, int len, bool chosenC3, const char* who);
    void RequestServerList();   // C1 04 F4 02, una sola vez por sesión de CS

    bool m_HasConnectServer = false;
    bool m_ConnectServerMode = false;
    bool m_ServerListRequested = false;
};

extern CNetwork gNetwork;
