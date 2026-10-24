#include "stdafx.h"
#include "Platform/Win32/Win32Window.h"
#include "resource.h"

extern "C" void DbgLogPublic(const char* msg);

namespace Platform {
void Win32Window::ServiceLoadingEvents(unsigned maximum) {
    MSG message;
    for (unsigned i = 0; i < maximum && PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE); ++i) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

Win32Window& GetWin32Window()
{
    static Win32Window window;
    return window;
}
IWindow& GetWindow() { return GetWin32Window(); }

void Win32Window::BindDispatch(HINSTANCE instance, WNDPROC procedure)
{
    m_hInst = instance;
    m_WndProc = procedure;
}
void Win32Window::ApplySettings(const WindowSettings& settings)
{
    m_Width = settings.width;
    m_Height = settings.height;
    m_WindowMode = settings.windowMode;
    m_Borderless = settings.borderless;
}
void Win32Window::SetTitle(const char* title) { SetWindowTextW(m_hWnd, Wide(title).c_str()); }
void Win32Window::ShowAndFocus()
{
    ShowWindow(m_hWnd, SW_SHOW);
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
}
void Win32Window::CloseWindow() { SendMessageA(m_hWnd, WM_CLOSE, 0, 0); }

void Win32Window::RequestCloseApplication()
{
    // IDA: WndProc (0x004149D0): conservar el cierre síncrono de los loaders.
    SendMessageA(m_hWnd, WM_DESTROY, 0, 0);
}

void Win32Window::ChangeMode(const WindowSettings& settings)
{
    if (!m_hWnd) return;
    const bool wasFullscreen = !m_WindowMode;
    ApplySettings(settings);
    const bool windowMode = m_WindowMode;
    const bool borderless = m_Borderless;
    if (!windowMode) ApplyFullscreen();          // cambia el modo de video al tamaño nuevo
    else if (wasFullscreen) RestoreDisplay();

    // Mismo estilo y posición que Create.
    DWORD style, exStyle;
    if (!windowMode || borderless) {
        style = WS_POPUP | WS_VISIBLE;
        exStyle = windowMode ? (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) : WS_EX_APPWINDOW;
    } else {
        style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
        exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    }
    RECT rc = { 0, 0, (LONG)m_Width, (LONG)m_Height };
    AdjustWindowRect(&rc, style, FALSE);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int x = 0, y = 0;
    if (windowMode) {
        x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
        y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }
    SetWindowLongPtrA(m_hWnd, GWL_STYLE, style);
    SetWindowLongPtrA(m_hWnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(m_hWnd, NULL, x, y, w, h, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED);

}

void Win32Window::RestoreDisplay()
{
    if (!m_DisplayModeChanged) return;
    m_DisplayModeChanged = false;
    ChangeDisplaySettingsA(NULL, 0);
}

// DESVIACION respecto de IDA (WinMain 0x41E8A0 L394-418).
//
// El original busca un modo de video con ancho == WindowWidth, alto ==
// WindowHeight y 16 bits de color. En Windows 10/11 no hay modos de 16 bits, así
// que no encuentra ninguno y el "pantalla completa" queda como una ventana
// WS_POPUP en la esquina. Se usa el criterio del DLL
// (CWindow::ChangeDisplaySettingsFunction): la mayor profundidad de color que
// ofrezca el sistema.
void Win32Window::ApplyFullscreen()
{
    DEVMODEA dm = {};
    dm.dmSize = sizeof(dm);

    DWORD bestBpp = 0;
    for (int i = 0; EnumDisplaySettingsA(NULL, i, &dm); ++i) {
        if (dm.dmBitsPerPel > bestBpp) bestBpp = dm.dmBitsPerPel;
    }

    for (int i = 0; EnumDisplaySettingsA(NULL, i, &dm); ++i) {
        if (dm.dmPelsWidth  == m_Width &&
            dm.dmPelsHeight == m_Height &&
            dm.dmBitsPerPel == bestBpp) {
            if (ChangeDisplaySettingsA(&dm, 0) == DISP_CHANGE_SUCCESSFUL) {
                m_DisplayModeChanged = true;
                DbgLogPublic("Display: pantalla completa OK");
            } else {
                DbgLogPublic("Display: ChangeDisplaySettings FALLO, sigo en ventana");
            }
            return;
        }
    }

    // Igual que el DLL: si el modo pedido no existe, la ventana se crea igual.
    char line[128];
    wsprintfA(line, "Display: no hay modo %ux%u a %u bpp, sigo en ventana",
              m_Width, m_Height, bestBpp);
    DbgLogPublic(line);
}

// IDA: Window_Create (0x0041DFF0).
//   style:  CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS = 0x2B
//   clase:  "Dialog"; ventana WS_POPUP del tamaño configurado.
// La rama de pantalla completa es la del binario; el modo ventana es del DLL.
bool Win32Window::Create(const WindowSettings& settings)
{
    ApplySettings(settings);
    const HINSTANCE hInst = m_hInst;
    const WNDPROC wndProc = m_WndProc;

    WNDCLASSA wc     = {};
    wc.style         = 0x2B;            // CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS
    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = hInst;
    // Icono grande (Alt+Tab, barra de tareas) desde los recursos del exe.
    wc.hIcon         = LoadIconA(hInst, MAKEINTRESOURCEA(IDI_MAIN_ICON));
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "Dialog";        // lo usa el FindWindowA de instancia única
    RegisterClassA(&wc);

    if (!m_WindowMode) {
        // Pantalla completa, como el original: el cambio de modo va ANTES de
        // crear la ventana (en IDA, EnumDisplaySettings precede a StartWindow).
        ApplyFullscreen();
        m_hWnd = CreateWindowExA(
            WS_EX_APPWINDOW,
            "Dialog",
            "Mu Online",
            WS_POPUP | WS_VISIBLE,
            0, 0, m_Width, m_Height,
            NULL, NULL, hInst, NULL
        );
    } else {
        // Modo ventana (DLL, CWindow::StartWindow), con tres diferencias:
        //   - sin WS_EX_TOPMOST;
        //   - AdjustWindowRect para que el área de cliente mida exactamente
        //     ancho x alto (el DLL suma +26 a mano);
        //   - sin WS_THICKFRAME ni WS_MAXIMIZEBOX: la escala de layout se
        //     calcula una sola vez y un resize descuadraría todo.
        const DWORD style = m_Borderless
            ? (WS_POPUP | WS_VISIBLE)
            : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE);

        RECT rc = { 0, 0, (LONG)m_Width, (LONG)m_Height };
        AdjustWindowRect(&rc, style, FALSE);
        const int w = rc.right  - rc.left;
        const int h = rc.bottom - rc.top;

        // Centrada en el escritorio; si no entra, pegada arriba a la izquierda
        // para que la barra de título quede alcanzable.
        int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        m_hWnd = CreateWindowExA(
            WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
            "Dialog",
            "Mu Online",
            style,
            x, y, w, h,
            NULL, NULL, hInst, NULL
        );
    }

    if (!m_hWnd) return false;

    // El .ico trae una sola imagen de 48x48; pedir el tamaño chico del sistema
    // con LoadImage da un icono de barra de título más limpio que el escalado
    // de GDI (igual que el DLL en CWindow::ChangeWindowState).
    HICON hSmall = (HICON)LoadImageA(hInst, MAKEINTRESOURCEA(IDI_MAIN_ICON),
                                     IMAGE_ICON,
                                     GetSystemMetrics(SM_CXSMICON),
                                     GetSystemMetrics(SM_CYSMICON),
                                     LR_DEFAULTCOLOR);
    if (hSmall) SendMessageA(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
    return true;
}

} // namespace Platform
