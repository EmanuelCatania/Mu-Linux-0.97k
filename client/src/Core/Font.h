#pragma once
// Font.h — CFont: las fuentes GDI del cliente.
//
// Reúne lo que en el binario son tres handles sueltos que crea WinMain:
//   IDA: g_hFont (0x055CA00C), g_hFontBold (0x055CA010), g_hFontBig (0x055CA014)
// Se usan con gFont.GetFont(FONT_NORMAL / FONT_BOLD / FONT_BIG).
//
// El DC de memoria donde se rasteriza el texto (m_hFontDC, 0x055C9FEC) todavía
// vive aparte, con Font_BuildLayout.

#include <windows.h>

enum eFontTypes {
    FONT_NORMAL = 0,   // texto común
    FONT_BOLD,         // títulos y texto destacado
    FONT_BIG,          // el doble de alto, bold (nombre del modal de trade, timers)
    MAX_FONT_TYPES
};

class CFont {
public:
    // Crea las tres fuentes. El tamaño depende del ancho de la ventana, como en
    // el binario (WinMain, paso 15).
    void Create(DWORD windowWidth);

    // Libera las fuentes (IDA DestroyWindow 0x4145C0, L1-12).
    void Release();

    HFONT GetFont(eFontTypes type) const
    {
        return (type >= 0 && type < MAX_FONT_TYPES) ? m_Fonts[type] : nullptr;
    }

private:
    HFONT m_Fonts[MAX_FONT_TYPES] = {};
};

extern CFont gFont;
