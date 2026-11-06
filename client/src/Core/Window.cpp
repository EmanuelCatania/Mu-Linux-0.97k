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
#include "Render/Gfx.h"
#include "Render/RenderBackend.h"
#ifdef MU_RENDER_WEBGPU
#include "Render/Backend/WebGPU/Device.h"
#include "Render/Backend/WebGPU/Renderer.h"
#endif

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
    const auto viewport = Ui::MapViewport(x, w, (int)m_Width, (int)m_UiWidth, m_UiOffsetX);
    pixelX = viewport.x;
    pixelWidth = viewport.width;
}

float CWindow::ViewportFraction(int width) const
{
    if (!IsWide()) return (float)width * (1.0f / 640.0f);
    int x, w;
    ViewportX(0, width, x, w);
    return (float)w / (float)m_Width;
}

int CWindow::GetRightShift() const
{
    if (!IsWide() || m_UiWidth == 0) return 0;
    return (int)((long long)m_UiOffsetX * 640 / (long long)m_UiWidth);
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
    Platform::GetWindow().ChangeMode({width, height, windowMode, borderless, !RenderBackend::IsWebGPU()});
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU()) WebGPU::Configure(width, height, gUserSettings.GetVSync());
#endif
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
    // DESVIACION: calidad de texturas elegida por el jugador, antes de subir ninguna.
    Gfx::SetTextureQuality(gUserSettings.GetTextureQuality() >= 1, gUserSettings.GetTextureQuality() >= 2);
    Gfx::SetMultisample(gUserSettings.Multisample());
#ifdef MU_RENDER_WEBGPU
    // DESVIACION: backend de render elegido en Config.ini ([Window] RenderBackend).
    RenderBackend::active = gUserSettings.GetRenderBackend() == RenderBackendPreference::OpenGL
        ? RenderBackend::Kind::OpenGL : RenderBackend::Kind::WebGPU;
#endif
    return Platform::GetWindow().Create({m_Width, m_Height, m_WindowMode, m_Borderless, !RenderBackend::IsWebGPU(), Gfx::Multisample()});
}

#ifdef MU_RENDER_WEBGPU
namespace {
WebGPU::Api WebGPUApi(RenderBackendPreference preference)
{
    switch (preference) {
    case RenderBackendPreference::D3D12: return WebGPU::Api::D3D12;
    case RenderBackendPreference::D3D11: return WebGPU::Api::D3D11;
    case RenderBackendPreference::Vulkan: return WebGPU::Api::Vulkan;
    default: return WebGPU::Api::Auto;
    }
}

// Dispositivo WebGPU sobre la ventana. Si no se puede crear, la ventana se
// recrea con contexto OpenGL y el cliente sigue con ese backend.
bool InitWebGPU(DWORD width, DWORD height, bool windowMode, bool borderless)
{
    const WebGPU::Surface surface{Platform::NativeWindow(), Platform::NativeInstance()};
    if (WebGPU::Initialize(WebGPUApi(gUserSettings.GetRenderBackend()), surface, width, height, gUserSettings.GetVSync()))
        return true;
    CErrorReport_Write(&DAT_055c9bf0, "> WebGPU no disponible: se usa OpenGL");
    RenderBackend::active = RenderBackend::Kind::OpenGL;
    Platform::GetWindow().Shutdown();
    return Platform::GetWindow().Create({width, height, windowMode, borderless, true, Gfx::Multisample()});
}
}
#endif

// IDA: OpenGL_Init (0x0041DE30): mostrar la ventana después de activar el contexto.
bool CWindow::InitOpenGL()
{
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU() && !InitWebGPU(m_Width, m_Height, m_WindowMode, m_Borderless)) return false;
    if (RenderBackend::IsWebGPU()) {
        BufferedDraw::Initialize();
        gFrameLimiter.SetFpsLimit(gUserSettings.GetFpsLimit());
        gFrameLimiter.SetRenderDecoupled(gUserSettings.GetRenderDecoupled());
        Platform::GetWindow().ShowAndFocus();
        return true;
    }
#endif
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
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU()) WebGPU::Release();
    else
#endif
    Platform::GetGLContext().Release();
    RestoreDisplay();
}

void CWindow::Present()
{
    BufferedDraw::Suspend();
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU()) { WebGPU::SubmitFrame(); return; }
#endif
    Platform::GetGLContext().Present();
}

bool CWindow::SetSwapInterval(bool vsync)
{
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU()) return WebGPU::Configure(m_Width, m_Height, vsync);
#endif
    return Platform::GetGLContext().SetSwapInterval(vsync ? 1 : 0);
}
HDC CWindow::GetHdc() const { return Platform::NativeDC(); }
void* CWindow::RenderIdentity() const
{
#ifdef MU_RENDER_WEBGPU
    if (RenderBackend::IsWebGPU()) return WebGPU::GetDevice();
#endif
    return Platform::GetGLContext().CurrentIdentity();
}

HGLRC CWindow::GetGLContext() const { return Platform::NativeGLContext(); }
