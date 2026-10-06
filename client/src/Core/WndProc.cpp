// WndProc.cpp — procedimiento de la ventana principal.
//
// Despacha los mensajes de la ventana; la red y el input viven en sus módulos:
//   gNetwork.OnSocketEvent      (Net/Network.cpp)  — WM_USER
//   Input_OnWindowMessage  (Input/Input_WndProc.cpp)   — mouse, IME y WM_CHAR

#include "stdafx.h"

void OpenGL_Release(void);        // WinMain.cpp
void GameGuard_TickCheck(void);   // WinMain.cpp
void Input_OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

// ── WndProc @ 0x004149D0 (4074 líneas, ~80% anti-tamper) ────────────────────
//
// __stdcall 4 params (RET 0x10). Todas las rutas terminan en DefWindowProcA.
//
// ── DISPATCH PASS 1 (mensajes 0x02..0x113+) ─────────────────────────────────
//
//   uMsg == 0x113 → WM_TIMER @ 0x00414db9 (antes del jump table)
//   uMsg in [0x2..0x20] → jump table 1 @ 0x41DD55:
//     WM_DESTROY(02), WM_ACTIVATE(06), WM_CLOSE(10), WM_SETCURSOR(20), otros
//   uMsg > 0x113 → checks adicionales:
//     uMsg == WM_USER (0x400) → sub-dispatch por wParam:
//       wParam==1:    Net_Recv()                — datos listos (WSAAsyncSelect FD_READ)
//       wParam==2:    Net_Send()                — buffer libre (FD_WRITE)
//       wParam==0x20: Screenshot() + Net_Connect() — socket conectado (FD_CONNECT)
//     uMsg == WM_USER+1 (0x401) → OpenGL_Release()   — shutdown message
//     uMsg == 0x3111            → envía paquete 0xC1/0xF1 (re-auth)
//     default → DefWindowProcA
//
// ── DISPATCH PASS 2 (mouse/IME, desde dentro del body de WM_USER) ───────────
//
//   Recarga uMsg @ 0x0041971a y despacha de nuevo:
//   uMsg == WM_LBUTTONUP  (0x202) @ 0x00419bf4 — suelta click
//   uMsg == WM_IME_COMPOSITION (0x10F) @ 0x0041a7bd — Korean DBCS chat input:
//     DAT_07e11cec[DAT_07e11d78 * 4]   = high byte
//     DAT_07e11ced[DAT_07e11d78 * 4]   = low byte
//     DAT_07e11cee[DAT_07e11d78 * 4]   = 0 (null terminator)
//   uMsg == WM_MOUSEMOVE  (0x200) @ 0x00419816 — normaliza coords a 640×480:
//     g_MouseX = LOWORD(lParam) * 640 / g_ScreenW  → DAT_083a427c
//     g_MouseY = HIWORD(lParam) * 480 / g_ScreenH  → DAT_083a4278
//   uMsg == WM_LBUTTONDOWN (0x201) @ 0x00419859
//   uMsg - 0x203 in [0..0x7F] → jump table 2 @ 0x41DD88:
//     WM_RBUTTONDOWN(204), WM_RBUTTONUP(205), WM_MOUSEWHEEL(20A), etc.
//
// ── TECLADO ─────────────────────────────────────────────────────────────────
//   WM_KEYDOWN/WM_KEYUP/WM_CHAR: NO manejados → DefWindowProcA
//   Movimiento/acción: GetAsyncKeyState() desde Game_SceneUpdate (polled)
//   WM_IME_COMPOSITION (0x10F): manejado (chat coreano)
//   IME toggle: IME_SetConversion @ 0x0047ED80 — ImmSetConversionStatus wrapper
//
// ── WM_TIMER ─────────────────────────────────────────────────────────────────
//   id=1000 (period=20000ms):
//     → GameGuard_TickCheck() — comprueba que el proceso GG sigue vivo
//     si g_bGameServerConnected != 0 (conectado): envía keep-alive 0xC1/0x0E
//
// ── GLOBALS CLAVE ────────────────────────────────────────────────────────────
//   DAT_083a427c = g_MouseX  (0..639, normalizado 640×480)
//   DAT_083a4278 = g_MouseY  (0..479)
//   DAT_083a413c = mouse-moved flag
//   DAT_083a4299 = flag de ventana activa (0 al desactivarse)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        // IDA WndProc (0x4149D0) case WM_DESTROY: marca el cierre, corta la
        // conexión, libera los 420 buffers de sonido y llama a KillGLWindow
        // (OpenGL_Release: DirectSound, contexto OpenGL y modo de video). Sin
        // esto, al cerrar en pantalla completa el escritorio quedaba en la
        // resolución del juego.
        DAT_055ca018 = 1;                                   // Destroy
        CWsctlc_Close((int)(uintptr_t)SocketClient);
        for (int buffer = 0; buffer < 420; ++buffer)
            Sound_ReleaseBuffer(buffer);
        OpenGL_Release();
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_ACTIVATE:
        // DAT_083a4299 = (LOWORD(wParam) != WA_INACTIVE) ? 1 : 0;
        break;

    case WM_SETCURSOR:
        // SetCursor(NULL) in-game para ocultar cursor del sistema.
        // Sin esto el cursor de Windows tapa al sprite de cursor del MU.
        SetCursor(NULL);
        return TRUE;  // we handled it; prevent DefWindowProc from setting arrow

    case WM_TIMER:
        if (wParam == 1000)
            GameGuard_TickCheck();
        break;

    case WM_USER:          // 0x400 — eventos de WSAAsyncSelect
        gNetwork.OnSocketEvent(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_USER + 1:      // 0x401 — shutdown
        OpenGL_Release();
        break;

    default:
        // Mouse, IME y WM_CHAR.
        Input_OnWindowMessage(uMsg, wParam, lParam);
        break;
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}
