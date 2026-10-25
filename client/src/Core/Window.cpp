// Window.cpp — CWindow. Ver Window.h.

#include "stdafx.h"
#include "Entity/EntityName.h"
#include "Render/RenderProfile.h"
#include "Render/TerrainDraw.h"
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
#include "Render/BufferedDraw.h"
#include "Render/MeshDraw.h"
#include "Render/GlyphAtlas.h"

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
                  (Hero ? EntityName::Get(Hero) : attributes.Name()), GlobalText[161], (unsigned int)attributes.Level(), ping, FPS);
    }
    Platform::GetWindow().SetTitle(title);
}


void CWindow::SetResolution(DWORD width, DWORD height)
{
    m_Width  = width;
    m_Height = height;

    // DESVIACION (Fase 4, pantalla ancha): con más de 7:5 la interfaz ocupa un
    // 4:3 centrado del alto de la ventana en vez de estirarse a todo el ancho.
    m_UiWidth = m_Width;
    m_UiOffsetX = 0;
    if (m_Height > 0 && (unsigned long long)m_Width * 5 > (unsigned long long)m_Height * 7) {
        m_UiWidth = (DWORD)(((unsigned long long)m_Height * 640 + 240) / 480);
        m_UiOffsetX = (int)(m_Width - m_UiWidth) / 2;
    }

    // IDA Config_Load (0x0041E0A0 L144-145):
    //     g_fScreenRate_x = (double)WindowWidth  * 0.0015625;      // = /640
    //     g_fScreenRate_y = (double)WindowHeight * 0.0020833334;   // = /480
    // Se calculan una sola vez: la ventana no se redimensiona (sin WM_SIZE).
    // Con el ancho lógico, en pantalla ancha las dos escalas son iguales.
    m_ScreenRateX = (float)((double)(int)m_UiWidth * 0.0015625);
    m_ScreenRateY = (float)((double)(int)m_Height * 0.0020833334);
}

void CWindow::ViewportX(int x, int w, int& pixelX, int& pixelWidth) const
{
    if (!IsWide()) {
        pixelX = (int)((unsigned)m_Width * (unsigned)x / 640u);
        pixelWidth = (int)((unsigned)m_Width * (unsigned)w / 640u);
        return;
    }
    const int left = x <= 0 ? 0 : m_UiOffsetX + (int)((long long)m_UiWidth * x / 640);
    const int right = (x + w >= 640) ? (int)m_Width : m_UiOffsetX + (int)((long long)m_UiWidth * (x + w) / 640);
    pixelX = left;
    pixelWidth = right - left;
}

float CWindow::ViewportFraction(int width) const
{
    if (!IsWide()) return (float)width * (1.0f / 640.0f);
    int x, w;
    ViewportX(0, width, x, w);
    return (float)w / (float)m_Width;
}

int CWindow::UiXFromPixel(int pixel) const
{
    if (!IsWide()) return (int)((long long)pixel * 640 / (m_Width ? m_Width : 640));
    return (int)((long long)(pixel - m_UiOffsetX) * 640 / (long long)m_UiWidth);
}

int CWindow::PixelFromUiX(int ui) const
{
    if (!IsWide()) return (int)((long long)ui * m_Width / 640);
    return m_UiOffsetX + (int)((long long)ui * m_UiWidth / 640);
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
    if (!Platform::GetGLContext().Initialize() || !BufferedDraw::Initialize()) {
        TerrainDraw::Release();
        MeshDraw::Release();
        BufferedDraw::Release();
        Platform::GetGLContext().Release();
        RestoreDisplay();
        Platform::GetWindow().ShowError(gClientText.Get(ClientTextId::RenderGLUnavailable),"Mu Online",false,true);
        return false;
    }
    // DESVIACION: preferencias de presentación por la interfaz de plataforma.
    if (!Platform::GetGLContext().SetSwapInterval(gUserSettings.GetVSync() ? 1 : 0))
        CErrorReport_Write(&DAT_055c9bf0, "> No se pudo aplicar Window.VSync");
    gFrameLimiter.SetFpsLimit(gUserSettings.GetFpsLimit());
    gFrameLimiter.SetRenderDecoupled(gUserSettings.GetRenderDecoupled());
    Platform::GetWindow().ShowAndFocus();
    return true;
}

// IDA: OpenGL_Release (0x0041AF20): restaurar pantalla después de liberar GL.
void CWindow::ReleaseOpenGL()
{
    RenderProfile::ResetContext();
    TerrainDraw::Release();
    MeshDraw::Release();
    GlyphAtlas::Release();
    BufferedDraw::Release();
    Platform::GetGLContext().Release();
    RestoreDisplay();
}

void CWindow::Present() { BufferedDraw::Suspend(); Platform::GetGLContext().Present(); }
HDC CWindow::GetHdc() const { return Platform::NativeDC(); }
HGLRC CWindow::GetGLContext() const { return Platform::NativeGLContext(); }
