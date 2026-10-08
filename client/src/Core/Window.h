#pragma once
// Window.h — CWindow: ventana principal, modo de video y contexto OpenGL.
//
// Reúne lo que en el binario son globales sueltas y funciones de WinMain:
//   IDA: g_hWnd (0x055C9FFC), g_hInst (0x055CA000), g_hDC (0x055CA004),
//        g_hRC (0x055CA008), WindowWidth (0x0056156C), WindowHeight (0x00561570),
//        g_fScreenRate_x/y (0x055C9B70/74)
//   IDA: StartWindow / Window_Create (0x0041DFF0), OpenGL_Init (0x0041DE30),
//        KillGLWindow / OpenGL_Release (0x0041AF20)
//
// El resto del cliente lee el estado con los getters de `gWindow`; sólo
// Config_Load (resolución y modo) y esta clase lo escriben.
//
// El WndProc todavía vive en WinMain.cpp: mezcla input, red y chat, y se va a
// repartir entre esos módulos cuando se armen.

#include <windows.h>

class CWindow {
public:
    // Registra la clase, aplica el modo de video si es pantalla completa y crea
    // la ventana. Devuelve false si CreateWindowEx falla.
    bool Create(HINSTANCE hInst, WNDPROC wndProc);

    // Crea el contexto OpenGL sobre la ventana y la muestra.
    bool InitOpenGL();

    // Libera el contexto OpenGL y restaura el modo de video si se cambió.
    void ReleaseOpenGL();

    // Devuelve el escritorio a su modo original si lo cambiamos nosotros.
    // Idempotente y llamable desde un filtro de excepciones.
    void RestoreDisplay();

    // Título del DLL: personaje, nivel, ping y FPS durante el juego.
    void UpdateTitle();

    // Tamaño del área de dibujo y escala respecto del layout lógico 640x480.
    void SetResolution(DWORD width, DWORD height);
    void SetWindowMode(bool windowMode, bool borderless);

    HWND      GetHwnd()        const { return m_hWnd; }
    HINSTANCE GetInstance()    const { return m_hInst; }
    HDC       GetHdc()         const { return m_hDC; }
    HGLRC     GetGLContext()   const { return m_hRC; }
    DWORD     GetWidth()       const { return m_Width; }
    DWORD     GetHeight()      const { return m_Height; }
    float     GetScreenRateX() const { return m_ScreenRateX; }
    float     GetScreenRateY() const { return m_ScreenRateY; }
    bool      IsWindowMode()   const { return m_WindowMode; }
    bool      IsBorderless()   const { return m_Borderless; }

private:
    void ApplyFullscreen();

    DWORD m_LastTitleUpdate = 0;
    int m_TitleScene = -1;

    HWND      m_hWnd  = NULL;
    HINSTANCE m_hInst = NULL;
    HDC       m_hDC   = NULL;
    HGLRC     m_hRC   = NULL;

    DWORD m_Width       = 640;
    DWORD m_Height      = 480;
    float m_ScreenRateX = 1.0f;   // ancho / 640
    float m_ScreenRateY = 1.0f;   // alto / 480

    // Default del DLL: en ventana y con bordes. Lo pisa Config.ini [Window].
    bool m_WindowMode = true;
    bool m_Borderless = false;

    // true sólo si ApplyFullscreen cambió el modo de video.
    bool m_DisplayModeChanged = false;
};

extern CWindow gWindow;
