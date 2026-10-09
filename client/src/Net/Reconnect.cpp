#include "stdafx.h"
#include "Net/Reconnect.h"
#include "Net/Network.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "Local/ClientText.h"

extern "C" void DbgLogPublic(const char* msg);
bool CreateSocketNoExit(const char* ip, unsigned int port);
void __cdecl DeleteCharacter(int key);

CReconnect gReconnect;

namespace {
// DLL MainInfo.ini ReconnectTime (default 300000): tope total para reconectar.
constexpr DWORD ReconnectTimeout = 300000;
constexpr DWORD ConnectRetry = 5000;      // entre intentos de conexión
constexpr DWORD LoginWait = 10000;        // F1/00 o respuesta del login
constexpr DWORD CharacterWait = 30000;    // lista o datos del personaje
constexpr int ErrorMessageReconnect = 0x1B;   // DLL ReconnectMainProc
}

void CReconnect::OnGameServerConnect(const char* ip, WORD port)
{
    if (!ip || !port) return;
    strncpy_s(m_Address, ip, _TRUNCATE);
    m_Port = port;
}

void CReconnect::OnLoginSent(const char* account, const char* password)
{
    memset(m_Account, 0, sizeof(m_Account));
    memset(m_Password, 0, sizeof(m_Password));
    memcpy(m_Account, account, strnlen(account, 10));
    memcpy(m_Password, password, strnlen(password, 10));
    // Un login nuevo (manual) vuelve a habilitar la reconexión.
    if (m_Status == STATUS_DISCONNECT) m_Status = STATUS_NONE;
}

void CReconnect::SetProgress(Progress progress, DWORD retryWait)
{
    m_Progress = progress;
    m_StepTime = GetTickCount();
    m_RetryWait = retryWait;
}

bool CReconnect::OnConnectionLost()
{
    if (m_Status == STATUS_RECONNECT) {
        // Se cayó un intento: volver a conectar.
        SetProgress(PROGRESS_CONNECT, ConnectRetry);
        return true;
    }
    if (m_Status == STATUS_DISCONNECT || SceneFlag != 5 || !m_Port || !m_Account[0] || !Hero)
        return false;

    m_Status = STATUS_RECONNECT;
    m_StartTime = GetTickCount();
    m_LoginSent = false;
    SetProgress(PROGRESS_CONNECT, 0);
    memset(m_Name, 0, sizeof(m_Name));
    memcpy(m_Name, (const char*)(uintptr_t)Hero + 0x1C1, 10);

    // DLL ReconnectViewportDestroy: las entidades vuelven con el viewport nuevo.
    // El héroe se conserva hasta que el F3/03 lo recree (el render lo usa).
    const BYTE* base = (const BYTE*)(uintptr_t)CharactersClient;
    for (int i = 0; base && i < 400; ++i) {
        const BYTE* c = base + i * 916;
        if (c[0] && (uintptr_t)c != (uintptr_t)Hero) DeleteCharacter(*(const WORD*)(c + 0x1DC));
    }
    PartyNumber = 0;
    DbgLogPublic("Reconnect: conexión perdida, reconectando");
    return true;
}

void CReconnect::GiveUp()
{
    m_Status = STATUS_DISCONNECT;
    DAT_083a7c24 = 0;   // ErrorMessage
    if (SocketClientSocket != 0xffffffff) {
        closesocket((SOCKET)SocketClientSocket);
        SocketClientSocket = (DWORD)INVALID_SOCKET;
    }
    extern void UIChatLogWindow_AddText(const char* strID, const char* msg, int color);
    UIChatLogWindow_AddText((const char*)&DAT_083a7c5c, GlobalText[3], 1);
    DbgLogPublic("Reconnect: sin éxito, se abandona");
}

void CReconnect::SendLogin()
{
    gNetwork.SendLogin(m_Account, m_Password);
    m_LoginSent = true;
    SetProgress(PROGRESS_LOGIN, LoginWait);
}

void CReconnect::Tick()
{
    if (m_Status != STATUS_RECONNECT) return;
    DAT_083a7c24 = ErrorMessageReconnect;   // ErrorMessage, como el DLL
    const DWORD now = GetTickCount();
    if (now - m_StartTime > ReconnectTimeout) { GiveUp(); return; }
    if (now - m_StepTime < m_RetryWait) return;

    switch (m_Progress) {
    case PROGRESS_CONNECT:
        if (CreateSocketNoExit(m_Address, m_Port)) {
            m_LoginSent = false;
            SetProgress(PROGRESS_LOGIN, LoginWait);   // el login sale con el F1/00
        } else {
            SetProgress(PROGRESS_CONNECT, ConnectRetry);
        }
        break;
    case PROGRESS_LOGIN:
        if (!m_LoginSent) { SendLogin(); break; }      // no llegó el F1/00
        // Sin respuesta: empezar de nuevo.
        if (SocketClientSocket != 0xffffffff) {
            closesocket((SOCKET)SocketClientSocket);
            SocketClientSocket = (DWORD)INVALID_SOCKET;
        }
        SetProgress(PROGRESS_CONNECT, ConnectRetry);
        break;
    default:
        if (SocketClientSocket != 0xffffffff) {
            closesocket((SOCKET)SocketClientSocket);
            SocketClientSocket = (DWORD)INVALID_SOCKET;
        }
        SetProgress(PROGRESS_CONNECT, ConnectRetry);
        break;
    }
}

void CReconnect::OnJoinServer()
{
    if (m_Status == STATUS_RECONNECT && m_Progress == PROGRESS_LOGIN && !m_LoginSent) SendLogin();
}

bool CReconnect::OnLoginResult(BYTE result)
{
    if (m_Status != STATUS_RECONNECT) return false;
    if (m_Progress != PROGRESS_LOGIN) return true;
    if (result == 1) {
        const BYTE pkt[4] = { 0xC1, 0x04, 0xF3, 0x00 };   // pedir la lista
        gNetwork.Send(pkt, sizeof(pkt));
        SetProgress(PROGRESS_CHAR_LIST, CharacterWait);
    } else if (result == 3) {
        // La sesión vieja sigue abierta en el server: reintentar el login.
        m_LoginSent = false;
        SetProgress(PROGRESS_LOGIN, LoginWait);
    } else {
        GiveUp();
    }
    return true;
}

bool CReconnect::OnCharacterList()
{
    if (m_Status != STATUS_RECONNECT) return false;
    if (m_Progress == PROGRESS_CHAR_LIST) {
        // Mismo pedido que char-select (Game_CharSelectTick): F3/03 con el nombre.
        BYTE pkt[14] = { 0xC1, 0x0E, 0xF3, 0x03 };
        memcpy(pkt + 4, m_Name, 10);
        gNetwork.SendC1(pkt, sizeof(pkt));
        SetProgress(PROGRESS_CHAR_INFO, CharacterWait);
    }
    return true;
}

void CReconnect::OnCharacterInfo()
{
    if (m_Status != STATUS_RECONNECT) return;
    m_Status = STATUS_NONE;
    DAT_083a7c24 = 0;   // ErrorMessage
    DbgLogPublic("Reconnect: de vuelta en el juego");
}

bool CReconnect::ExitIfActive()
{
    if (m_Status != STATUS_RECONNECT) return false;
    SendMessageA(gWindow.GetHwnd(), WM_DESTROY, 0, 0);
    return true;
}

// DLL ReconnectDrawInterface: barra roja sobre fondo negro, centrada, con el
// paso actual.
void CReconnect::Render()
{
    if (m_Status != STATUS_RECONNECT) return;
    constexpr float Width = 150.0f, Height = 18.0f;
    const float x = 320.0f - Width / 2.0f, y = 240.0f - Height / 2.0f;
    float progress = (GetTickCount() - m_StartTime) * Width / (float)ReconnectTimeout;
    if (progress > Width) progress = Width;
    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    GL_DrawRect(x, y, Width + 10.0f, Height);
    glColor3f(1.0f, 0.0f, 0.0f);
    GL_DrawRect(x + 5.0f, y + 5.0f, progress, Height - 10.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    GL_ResetState();

    static const ClientTextId Steps[] = {
        ClientTextId::ReconnectConnecting, ClientTextId::ReconnectLogin,
        ClientTextId::ReconnectCharList, ClientTextId::ReconnectCharInfo };
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    EnableAlphaTest(true);
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderText((int)x, (int)y + 5, const_cast<char*>(gClientText.Get(Steps[m_Progress])),
               (int)(Width * gWindow.GetWidth() / 640), 1, nullptr);
    m_dwTextColor = color;
    m_dwBackColor = back;
    GL_ResetState();
}
