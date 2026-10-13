// Window.cpp — CWindow. Ver Window.h.

#include "stdafx.h"
#include "Core/Font.h"
#include "Net/Ping.h"
#include "Core/Window.h"
#include "Entity/CharacterAttributeView.h"
#include "Local/ClientText.h"
#include "Platform/Win32/Win32Window.h"

CWindow gWindow;

// DESVIACION DLL: CWindow::ChangeWindowText, sin resets hasta integrar su HUD.
void CWindow::UpdateTitle()
{
    if (!Platform::GetWindow().IsCreated()) return;
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
    Platform::GetWindow().SetTitle(title);
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

void CWindow::ChangeMode(bool windowMode, bool borderless, DWORD width, DWORD height)
{
    if (!Platform::GetWindow().IsCreated()) return;
    SetResolution(width, height);
    SetWindowMode(windowMode, borderless);
    Platform::GetWindow().ChangeMode({width, height, windowMode, borderless});
    gFont.Reload(m_Width);
}

void CWindow::RestoreDisplay() { Platform::GetWindow().RestoreDisplay(); }
void CWindow::RequestCloseApplication() { Platform::GetWindow().RequestCloseApplication(); }
HWND CWindow::GetHwnd() const { return Platform::GetWin32Window().GetHwnd(); }
HINSTANCE CWindow::GetInstance() const { return Platform::GetWin32Window().GetInstance(); }

// IDA: Window_Create (0x0041DFF0). WndProc conserva el despacho existente.
bool CWindow::Create(HINSTANCE hInst, WNDPROC wndProc)
{
    Platform::GetWin32Window().BindDispatch(hInst, wndProc);
    return Platform::GetWindow().Create({m_Width, m_Height, m_WindowMode, m_Borderless});
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

    m_hDC = GetDC(GetHwnd());
    if (!m_hDC) return false;               // "OpenGL Get DC Error"
    int fmt = ChoosePixelFormat(m_hDC, &pfd);
    if (!fmt) return false;                 // "OpenGL Choose Pixel Format Error"
    if (!SetPixelFormat(m_hDC, fmt, &pfd)) return false;   // "OpenGL Set Pixel Format Error"

    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC) return false;               // "OpenGL Create Context Error"
    if (!wglMakeCurrent(m_hDC, m_hRC)) return false;       // "OpenGL Make Current Error"

    Platform::GetWindow().ShowAndFocus();
    return true;
}

// IDA: OpenGL_Release (0x0041AF20), la parte de ventana y video.
void CWindow::ReleaseOpenGL()
{
    wglMakeCurrent(NULL, NULL);     // "GL - Release Of DC And RC Failed"
    wglDeleteContext(m_hRC);        // "GL - Release Rendering Context Failed"
    DeleteDC(m_hDC);                // "GL - Release Device Context Failed"
    ReleaseDC(GetHwnd(), m_hDC);
    // Sólo se restaura el modo de video si lo cambiamos nosotros: hacerlo igual
    // en modo ventana hace parpadear el escritorio al salir (es el
    // FixDisplaySettingsOnClose del DLL).
    RestoreDisplay();
}
