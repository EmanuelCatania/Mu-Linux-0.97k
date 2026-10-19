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
// WndProc vive en Core/WndProc.cpp; su orden de despacho se conserva.
// La ventana nativa y su contexto OpenGL viven en Platform/Win32.

#include "Platform/PlatformHeaders.h"

class CWindow {
public:
    // Registra la clase, aplica el modo de video si es pantalla completa y crea
    // la ventana. Devuelve false si CreateWindowEx falla.
    bool Create(HINSTANCE hInst, WNDPROC wndProc);

    // Crea el contexto OpenGL sobre la ventana y la muestra.
    bool InitOpenGL();

    // Libera el contexto OpenGL y restaura el modo de video si se cambió.
    void ReleaseOpenGL();

    // Presenta el frame en el mismo punto que el SwapBuffers original.
    void Present();

    // Devuelve el escritorio a su modo original si lo cambiamos nosotros.
    // Idempotente y llamable desde un filtro de excepciones.
    void RestoreDisplay();

    // Cierre síncrono de la aplicación, también para errores de carga.
    void RequestCloseApplication();

    // Título del DLL: personaje, nivel, ping y FPS durante el juego.
    void UpdateTitle();

    // Tamaño del área de dibujo y escala respecto del layout lógico 640x480.
    void SetResolution(DWORD width, DWORD height);
    void SetWindowMode(bool windowMode, bool borderless);

    // DESVIACION (DLL Window.cpp ChangeWindowState): cambia modo y tamaño con
    // la ventana ya creada, desde el menú de opciones.
    void ChangeMode(bool windowMode, bool borderless, DWORD width, DWORD height);

    HWND      GetHwnd()        const;
    HINSTANCE GetInstance()    const;
    HDC       GetHdc()         const;
    HGLRC     GetGLContext()   const;
    DWORD     GetWidth()       const { return m_Width; }
    DWORD     GetHeight()      const { return m_Height; }
    float     GetScreenRateX() const { return m_ScreenRateX; }
    float     GetScreenRateY() const { return m_ScreenRateY; }
    bool      IsWindowMode()   const { return m_WindowMode; }
    bool      IsBorderless()   const { return m_Borderless; }

private:
    DWORD m_LastTitleUpdate = 0;
    int m_TitleScene = -1;

    DWORD m_Width       = 640;
    DWORD m_Height      = 480;
    float m_ScreenRateX = 1.0f;   // ancho / 640
    float m_ScreenRateY = 1.0f;   // alto / 480

    // Default del DLL: en ventana y con bordes. Lo pisa Config.ini [Window].
    bool m_WindowMode = true;
    bool m_Borderless = false;

};

extern CWindow gWindow;
