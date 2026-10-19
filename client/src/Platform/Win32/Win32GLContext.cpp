#include "stdafx.h"
#include "Platform/Win32/Win32GLContext.h"
#include "Platform/Win32/Win32Window.h"

namespace Platform {
Win32GLContext& GetWin32GLContext()
{
    static Win32GLContext context;
    return context;
}
void* Win32GLContext::CurrentIdentity() const { return wglGetCurrentContext(); }
IGLContext& GetGLContext() { return GetWin32GLContext(); }

// IDA: OpenGL_Init (0x0041DE30).
// PIXELFORMATDESCRIPTOR: PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
// RGBA, 16 bits de color y 16 de profundidad.
bool Win32GLContext::Initialize()
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // 0x25
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;

    m_hDC = GetDC(GetWin32Window().GetHwnd());
    if (!m_hDC) return false;               // "OpenGL Get DC Error"
    int fmt = ChoosePixelFormat(m_hDC, &pfd);
    if (!fmt) return false;                 // "OpenGL Choose Pixel Format Error"
    if (!SetPixelFormat(m_hDC, fmt, &pfd)) return false;   // "OpenGL Set Pixel Format Error"

    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC) return false;               // "OpenGL Create Context Error"
    if (!wglMakeCurrent(m_hDC, m_hRC)) return false;       // "OpenGL Make Current Error"

    return true;
}

// IDA: OpenGL_Release (0x0041AF20), la parte de ventana y video.
void Win32GLContext::Release()
{
    wglMakeCurrent(NULL, NULL);     // "GL - Release Of DC And RC Failed"
    wglDeleteContext(m_hRC);        // "GL - Release Rendering Context Failed"
    DeleteDC(m_hDC);                // "GL - Release Device Context Failed"
    ReleaseDC(GetWin32Window().GetHwnd(), m_hDC);
}

// DESVIACION: extensión WGL opcional; el controlador puede no ofrecerla.
bool Win32GLContext::SetSwapInterval(int interval)
{
    if (!m_hRC || (interval != 0 && interval != 1)) return false;
    const auto address = wglGetProcAddress("wglSwapIntervalEXT");
    const auto value = reinterpret_cast<INT_PTR>(address);
    if (!address || value == 1 || value == 2 || value == 3 || value == -1) return false;
    using SwapInterval = BOOL (WINAPI*)(int);
    return reinterpret_cast<SwapInterval>(address)(interval) != FALSE;
}

void Win32GLContext::Present()
{
    SwapBuffers(m_hDC);
}
} // namespace Platform
