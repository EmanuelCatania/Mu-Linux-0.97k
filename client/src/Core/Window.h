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
    // Tamaño LÓGICO del área de interfaz.  En una ventana 4:3 es el de la
    // ventana; en una más ancha (16:9...) es el 4:3 centrado de la misma alto,
    // así toda la interfaz (640x480) se escala igual en X e Y y no se estira.
    DWORD     GetWidth()       const { return m_UiWidth; }
    DWORD     GetHeight()      const { return m_Height; }
    // Tamaño real de la ventana en píxeles (viewport del mundo, capturas, GL).
    DWORD     GetRealWidth()   const { return m_Width; }
    DWORD     GetRealHeight()  const { return m_Height; }
    // Píxeles de la ventana a la izquierda del área de interfaz (0 en 4:3).
    int       GetUiOffsetX()   const { return m_UiOffsetX; }
    bool      IsWide()         const { return m_UiWidth != m_Width; }

    // DESVIACION (Fase 4, pantalla ancha): rectángulo de un viewport 3D dado en
    // la grilla 640x480, en píxeles reales.  Lo que toca el borde izquierdo
    // (x = 0) o el derecho (x + w = 640) llega hasta el borde de la ventana; lo
    // demás se ubica dentro del área de interfaz.  En 4:3 es el cálculo de siempre.
    void ViewportX(int x, int w, int& pixelX, int& pixelWidth) const;
    // Fracción del ancho real que ocupa un viewport de `width` unidades de 640.
    float ViewportFraction(int width) const;
    // Unidades de la grilla 640 que sobran a cada lado del área de interfaz.
    // Los paneles pegados al borde derecho de la ventana (inventario, personaje,
    // party...) suman esto a su x: su borde derecho cae en el de la ventana.
    int GetRightShift() const;
    int PanelX(int base) const { return base + GetRightShift(); }
    // Píxel real <-> columna de la grilla 640 (puede salir de 0..639 en los costados).
    int UiXFromPixel(int pixel) const;
    int PixelFromUiX(int ui) const;
    float     GetScreenRateX() const { return m_ScreenRateX; }
    float     GetScreenRateY() const { return m_ScreenRateY; }
    bool      IsWindowMode()   const { return m_WindowMode; }
    bool      IsBorderless()   const { return m_Borderless; }

private:
    DWORD m_LastTitleUpdate = 0;
    int m_TitleScene = -1;

    DWORD m_Width       = 640;
    DWORD m_Height      = 480;
    DWORD m_UiWidth     = 640;    // = m_Width salvo en pantalla ancha
    int   m_UiOffsetX   = 0;
    float m_ScreenRateX = 1.0f;   // ancho / 640
    float m_ScreenRateY = 1.0f;   // alto / 480

    // Default del DLL: en ventana y con bordes. Lo pisa Config.ini [Window].
    bool m_WindowMode = true;
    bool m_Borderless = false;

};

extern CWindow gWindow;
