// Net_SocketEvents.cpp — eventos del socket que llegan por WSAAsyncSelect.
//
// IDA: WndProc (0x004149D0), case WM_USER (0x400). El mask de WSAAsyncSelect es
// 0x23 (FD_READ | FD_WRITE | FD_CLOSE); FD_CONNECT se atiende igual por el
// ConnectServer.

#include "stdafx.h"

extern "C" void CsmWatchdog(const char *tag);

// MSDN: wParam = socket, LOWORD(lParam) = evento, HIWORD(lParam) = error.
void Net_OnSocketEvent(WORD evt, WORD err)
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
        // Robustez: la conexión no-bloqueante completa suele señalizarse con
        // el primer FD_WRITE (el mask WSAAsyncSelect es 0x23, sin FD_CONNECT).
        // Mandamos el request de lista al ConnectServer acá también; el guard
        // g_ConnectServerRequested asegura que salga una sola vez.
        if (g_ConnectServerMode && !g_ConnectServerRequested) {
            extern void CS_SendPlain(const BYTE* data, int len);
            BYTE req[4] = { 0xC1, 0x04, 0xF4, 0x02 };
            CS_SendPlain(req, 4);
            g_ConnectServerRequested = 1;
        }
    }
    if (evt & 0x10) { // FD_CONNECT
        extern void CS_SendPlain(const BYTE* data, int len);
        extern void CreateSocket(const char* server, unsigned int port);
        if (err == 0 && g_ConnectServerMode && !g_ConnectServerRequested) {
            // Conectados al ConnectServer → pedir la lista de servers.
            // C1 04 F4 02 (PMSG_SERVER_LIST_RECV) — plano, sin encriptar.
            BYTE req[4] = { 0xC1, 0x04, 0xF4, 0x02 };
            CS_SendPlain(req, 4);
            g_ConnectServerRequested = 1;
        }
    }
    if (evt & 0x20) { // FD_CLOSE
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
        UIChatLogWindow_AddText((const char*)&DAT_083a7c5c, GlobalText[3], 1);
        if (SocketClientSocket != 0xffffffff) {
            closesocket((SOCKET)SocketClientSocket);
            SocketClientSocket = (DWORD)INVALID_SOCKET;
        }
    }
}
