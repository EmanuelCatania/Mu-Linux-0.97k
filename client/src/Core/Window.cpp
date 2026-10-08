// Window.cpp — CWindow. Ver Window.h.

#include "stdafx.h"
#include "Net/Ping.h"
#include "Core/Window.h"
#include "Entity/CharacterAttributeView.h"
#include "Local/ClientText.h"
#include "resource.h"

extern "C" void DbgLogPublic(const char* msg);

CWindow gWindow;

// DESVIACION DLL: CWindow::ChangeWindowText, sin resets hasta integrar su HUD.
void CWindow::UpdateTitle()
{
    if (!m_hWnd) return;
    const DWORD now = GetTickCount();
    if (m_TitleScene == SceneFlag && now - m_LastTitleUpdate < 1000) return;
    m_TitleScene = SceneFlag;
    m_LastTitleUpdate = now;
    char title[160] = "Mu Online";
    if (SceneFlag == 5 && CharacterAttribute) {
        const CharacterAttributeView attributes((void*)(uintptr_t)CharacterAttribute);
        DWORD milliseconds;
        char ping[32] = "--";
        if (gPing.GetMilliseconds(milliseconds))
            sprintf_s(ping, "%lu ms", milliseconds);
        sprintf_s(title, gClientText.Get(ClientTextId::WindowTitle),
                  attributes.Name(), GlobalText[161], (unsigned int)attributes.Level(), ping, FPS);
    }
    SetWindowTextA(m_hWnd, title);
}


void CWindow::SetResolution(DWORD width, DWORD height)
{
    m_Width  = width;
    m_Height = height;

    // IDA Config_Load (0x0041E0A0 L144-145):
    //     g_fScreenRate_x = (double)WindowWidth  * 0.0015625;      // = /640
    //     g_fScreenRate_y = (double)WindowHeight * 0.0020833334;   // = /480
    // Se calculan una sola vez: la ventana no se redimensiona (sin WM_SIZE).
    m_ScreenRateX = (float)((double)(int)m_Width  * 0.0015625);
    m_ScreenRateY = (float)((double)(int)m_Height * 0.0020833334);
}

void CWindow::SetWindowMode(bool windowMode, bool borderless)
{
    m_WindowMode = windowMode;
    m_Borderless = borderless;
}

void CWindow::RestoreDisplay()
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
void CWindow::ApplyFullscreen()
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
bool CWindow::Create(HINSTANCE hInst, WNDPROC wndProc)
{
    m_hInst = hInst;

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

// IDA: OpenGL_Init (0x0041DE30).
// PIXELFORMATDESCRIPTOR: PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
// RGBA, 16 bits de color y 16 de profundidad.
bool CWindow::InitOpenGL()
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // 0x25
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;

    m_hDC = GetDC(m_hWnd);
    if (!m_hDC) return false;               // "OpenGL Get DC Error"
    int fmt = ChoosePixelFormat(m_hDC, &pfd);
    if (!fmt) return false;                 // "OpenGL Choose Pixel Format Error"
    if (!SetPixelFormat(m_hDC, fmt, &pfd)) return false;   // "OpenGL Set Pixel Format Error"

    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC) return false;               // "OpenGL Create Context Error"
    if (!wglMakeCurrent(m_hDC, m_hRC)) return false;       // "OpenGL Make Current Error"

    ShowWindow(m_hWnd, SW_SHOW);
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
    return true;
}

// IDA: OpenGL_Release (0x0041AF20), la parte de ventana y video.
void CWindow::ReleaseOpenGL()
{
    wglMakeCurrent(NULL, NULL);     // "GL - Release Of DC And RC Failed"
    wglDeleteContext(m_hRC);        // "GL - Release Rendering Context Failed"
    DeleteDC(m_hDC);                // "GL - Release Device Context Failed"
    ReleaseDC(m_hWnd, m_hDC);
    // Sólo se restaura el modo de video si lo cambiamos nosotros: hacerlo igual
    // en modo ventana hace parpadear el escritorio al salir (es el
    // FixDisplaySettingsOnClose del DLL).
    RestoreDisplay();
}
