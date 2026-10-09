#pragma once
// Font.h — CFont: las fuentes GDI del cliente y la superficie donde se dibuja
// el texto.
//
// Reúne lo que en el binario son globales sueltas:
//   IDA: g_hFont (0x055CA00C), g_hFontBold (0x055CA010), g_hFontBig (0x055CA014)
//   IDA: m_hFontDC (0x055C9FEC) y ppvBits (0x055C9E4C), el DC de memoria y los
//        bits del DIB donde GDI rasteriza el texto antes de subirlo como textura.
// Se usan con gFont.GetFont(FONT_NORMAL / FONT_BOLD / FONT_BIG),
// gFont.GetTextDC() y gFont.GetTextBits().

#include <windows.h>

enum eFontTypes {
    FONT_NORMAL = 0,   // texto común
    FONT_BOLD,         // títulos y texto destacado
    FONT_BIG,          // el doble de alto, bold (nombre del modal de trade, timers)
    MAX_FONT_TYPES
};

class CFont {
public:
    // Crea las tres fuentes. Si Config.ini trae la sección [Font], se usa como
    // en el DLL (ver CUserSettings); si no, como el binario: Arial con el alto
    // según el ancho de la ventana (WinMain, paso 15).
    void Create(DWORD windowWidth);

    // DESVIACION (DLL Font.cpp ReloadFont): recrea las tres fuentes con la
    // configuración y la resolución actuales.  La superficie de texto no
    // cambia: su tamaño sale de FontInput.tga.
    void Reload(DWORD windowWidth);

    // Libera las fuentes y la superficie de texto (IDA DestroyWindow 0x4145C0).
    void Release();

    // IDA sub_50F5F0: DIB de 24 bits (width x height, de arriba hacia abajo) y
    // DC de memoria compatible con `hdc`, con fondo transparente.
    void CreateTextSurface(HDC hdc, LONG width, LONG height);

    HFONT GetFont(eFontTypes type) const
    {
        return (type >= 0 && type < MAX_FONT_TYPES) ? m_Fonts[type] : nullptr;
    }
    HDC   GetTextDC()   const { return m_hTextDC; }
    void* GetTextBits() const { return m_pTextBits; }

private:
    HFONT   m_Fonts[MAX_FONT_TYPES] = {};
    HDC     m_hTextDC     = NULL;
    HBITMAP m_hTextBitmap = NULL;
    void*   m_pTextBits   = nullptr;
};

extern CFont gFont;
