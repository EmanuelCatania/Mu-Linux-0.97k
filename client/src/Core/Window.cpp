// Window.cpp — CWindow. Ver Window.h.

#include "stdafx.h"
#include "Platform/Text.h"
#include "Platform/ITimer.h"
#include "Core/Font.h"
#include "Net/Ping.h"
#include "Core/Window.h"
#include "Entity/CharacterAttributeView.h"
#include "Local/ClientText.h"
#include "Platform/NativeBridge.h"
#include "Platform/IWindow.h"
#include "Platform/IGLContext.h"
#include "Config/UserSettings.h"
#include "Game/FrameLimiter.h"

CWindow gWindow;

// DESVIACION DLL: CWindow::ChangeWindowText, sin resets hasta integrar su HUD.
void CWindow::UpdateTitle()
{
    if (!Platform::GetWindow().IsCreated()) return;
    const DWORD now = Platform::GetTimer().Ticks();
    if (m_TitleScene == SceneFlag && now - m_LastTitleUpdate < 1000) return;
    m_TitleScene = SceneFlag;
    m_LastTitleUpdate = now;
    char title[160] = "Mu Online";
    if (SceneFlag == 5 && CharacterAttribute) {
        const CharacterAttributeView attributes((void*)(uintptr_t)CharacterAttribute);
        DWORD milliseconds;
        char ping[32] = "--";
        if (gPing.GetMilliseconds(milliseconds))
            Platform::TextFormatSecure(ping, "%lu ms", milliseconds);
        Platform::TextFormatSecure(title, gClientText.Get(ClientTextId::WindowTitle),
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
HWND CWindow::GetHwnd() const { return Platform::NativeWindow(); }
HINSTANCE CWindow::GetInstance() const { return Platform::NativeInstance(); }

// IDA: Window_Create (0x0041DFF0). WndProc conserva el despacho existente.
bool CWindow::Create(HINSTANCE hInst, WNDPROC wndProc)
{
    Platform::BindNativeDispatch(hInst, wndProc);
    return Platform::GetWindow().Create({m_Width, m_Height, m_WindowMode, m_Borderless});
}

// IDA: OpenGL_Init (0x0041DE30): mostrar la ventana después de activar el contexto.
bool CWindow::InitOpenGL()
{
    if (!Platform::GetGLContext().Initialize()) return false;
    // DESVIACION: preferencias de presentación por la interfaz de plataforma.
    if (!Platform::GetGLContext().SetSwapInterval(gUserSettings.GetVSync() ? 1 : 0))
        CErrorReport_Write(&DAT_055c9bf0, "> No se pudo aplicar Window.VSync");
    gFrameLimiter.SetFpsLimit(gUserSettings.GetFpsLimit());
    Platform::GetWindow().ShowAndFocus();
    return true;
}

// IDA: OpenGL_Release (0x0041AF20): restaurar pantalla después de liberar GL.
void CWindow::ReleaseOpenGL()
{
    Platform::GetGLContext().Release();
    RestoreDisplay();
}

void CWindow::Present() { Platform::GetGLContext().Present(); }
HDC CWindow::GetHdc() const { return Platform::NativeDC(); }
HGLRC CWindow::GetGLContext() const { return Platform::NativeGLContext(); }
