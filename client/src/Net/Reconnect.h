#pragma once
// Reconnect.h — reconexión automática al GameServer.
//
// DESVIACION (DLL Reconnect.cpp): el 0.97k, al cerrarse el socket en el juego,
// sólo avisa "Has sido desconectado".  Acá, si el cierre no lo pidió el
// jugador, el cliente vuelve a conectar al mismo GameServer, reenvía el login
// (F1/01), pide la lista (F3/00) y entra con el mismo personaje (F3/03), con
// una barra de progreso.  Si no lo logra en ReconnectTimeout, se rinde.

#include <windows.h>

class CReconnect {
public:
    // Datos para reconectar: el GameServer (no el ConnectServer) y la cuenta.
    void OnGameServerConnect(const char* ip, WORD port);
    void OnLoginSent(const char* account, const char* password);
    // El jugador eligió salir (menú Escape): un cierre posterior no reconecta.
    void MarkIntentional() { m_Status = STATUS_DISCONNECT; }

    // FD_CLOSE.  true si empezó a reconectar (el caller no muestra el aviso).
    bool OnConnectionLost();
    bool IsActive() const { return m_Status == STATUS_RECONNECT; }

    void Tick();     // una vez por frame (Game_MainLoop)
    void Render();   // después de RenderErrorMessage

    // Respuestas del server durante la reconexión.  true = las consumió y el
    // handler normal no debe correr (llevaría a char-select).
    void OnJoinServer();                 // F1/00: ya se puede mandar el login
    bool OnLoginResult(BYTE result);     // F1/01
    bool OnCharacterList();              // F3/00
    void OnCharacterInfo();              // F3/03: de vuelta en el juego
    // Salir del juego desde el menú mientras reconecta (DLL MenuExitGame).
    bool ExitIfActive();

private:
    enum Status { STATUS_NONE, STATUS_RECONNECT, STATUS_DISCONNECT };
    enum Progress { PROGRESS_CONNECT, PROGRESS_LOGIN, PROGRESS_CHAR_LIST, PROGRESS_CHAR_INFO };

    void SetProgress(Progress progress, DWORD retryWait);
    void GiveUp();
    void SendLogin();

    Status   m_Status = STATUS_NONE;
    Progress m_Progress = PROGRESS_CONNECT;
    DWORD    m_StartTime = 0;     // inicio de la reconexión (tope total)
    DWORD    m_StepTime = 0;      // último intento del paso actual
    DWORD    m_RetryWait = 0;     // espera antes de reintentar el paso
    bool     m_LoginSent = false;
    char     m_Address[16] = {};
    WORD     m_Port = 0;
    char     m_Account[11] = {};
    char     m_Password[11] = {};
    char     m_Name[11] = {};
};

extern CReconnect gReconnect;
