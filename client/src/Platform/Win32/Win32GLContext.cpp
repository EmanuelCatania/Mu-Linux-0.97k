#include "stdafx.h"
#include "Render/Backend/GL/LegacyGL.h"
#include "Platform/Win32/Win32GLContext.h"
#include "Platform/Win32/Win32Window.h"
#include "Platform/GLFunctions.h"

namespace Platform {
namespace {
bool ValidWGL(PROC address) {
    const auto value=reinterpret_cast<INT_PTR>(address);
    return address && value!=1 && value!=2 && value!=3 && value!=-1;
}
GLProc ResolveGL(const char* name) {
    auto address=wglGetProcAddress(name);
    if (!ValidWGL(address)) address=GetProcAddress(GetModuleHandleA("opengl32.dll"),name);
    return reinterpret_cast<GLProc>(address);
}
}

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
    if (m_hRC) return wglMakeCurrent(m_hDC,m_hRC)!=FALSE;
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;

    m_hDC = GetDC(GetWin32Window().GetHwnd());
    if (!m_hDC) return false;
    auto fail=[this]() { Release(); return false; };
    // El formato de una ventana sólo puede fijarse una vez; admitir reinicio.
    if (!GetPixelFormat(m_hDC)) {
        const int fmt=ChoosePixelFormat(m_hDC,&pfd);
        if (!fmt || !SetPixelFormat(m_hDC,fmt,&pfd)) return fail();
    }
    // DESVIACION: WGL necesita un contexto temporal para resolver su extensión
    // de creación. El contexto de juego siempre solicita 3.3 compatibility.
    m_hRC=wglCreateContext(m_hDC);
    if (!m_hRC || !wglMakeCurrent(m_hDC,m_hRC)) return fail();
    const auto address=wglGetProcAddress("wglCreateContextAttribsARB");
    if (!ValidWGL(address)) return fail();
    using CreateContext=HGLRC (WINAPI*)(HDC,HGLRC,const int*);
    constexpr int ContextMajor=0x2091,ContextMinor=0x2092,ContextProfile=0x9126;
    constexpr int CompatibilityProfile=0x00000002;
    const int attributes[]={ContextMajor,3,ContextMinor,3,ContextProfile,CompatibilityProfile,0};
    const HGLRC modern=reinterpret_cast<CreateContext>(address)(m_hDC,nullptr,attributes);
    if (!modern) return fail();
    if (!wglMakeCurrent(m_hDC,modern)) { wglDeleteContext(modern); return fail(); }
    wglDeleteContext(m_hRC);
    m_hRC=modern;
    if (!LoadGL33(ResolveGL)) { OutputDebugStringA(GL33Error()); return fail(); }
    OutputDebugStringA("OpenGL 3.3 compatibility inicializado\n");
    return true;
}

// IDA: OpenGL_Release (0x0041AF20), la parte de ventana y video.
void Win32GLContext::Release()
{
    if (m_hRC) {
        if (wglGetCurrentContext()==m_hRC) wglMakeCurrent(nullptr,nullptr);
        wglDeleteContext(m_hRC);
        m_hRC=nullptr;
        ResetGL33();
    }
    // GetDC entrega un DC prestado: corresponde ReleaseDC, no DeleteDC.
    if (m_hDC) { ReleaseDC(GetWin32Window().GetHwnd(),m_hDC); m_hDC=nullptr; }
}

// DESVIACION: extensión WGL opcional; el controlador puede no ofrecerla.
bool Win32GLContext::SetSwapInterval(int interval)
{
    if (!m_hRC || (interval != 0 && interval != 1)) return false;
    const auto address = wglGetProcAddress("wglSwapIntervalEXT");
    if (!ValidWGL(address)) return false;
    using SwapInterval = BOOL (WINAPI*)(int);
    return reinterpret_cast<SwapInterval>(address)(interval) != FALSE;
}

void Win32GLContext::Present()
{
    SwapBuffers(m_hDC);
}
} // namespace Platform
