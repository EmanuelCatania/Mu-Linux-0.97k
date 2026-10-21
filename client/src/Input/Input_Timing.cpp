// Input_Timing.cpp

#include "stdafx.h"
#include "Platform/ITimer.h"
#include "Platform/Text.h"
#include "Local/ClientText.h"
#include "globals.h"
#include "functions.h"

namespace { bool showFps=false; }
void ToggleFpsCounter() { showFps=!showFps; }

// IDA: RenderDebugWindow @ 0x004C14E0 — FPS_TickReset.
// Selects the UI font into the DC, resets text color to white (0xFFFFFFFF)
// and background to black (0xFF000000). On each 1-second interval resets
// the FPS counter (DAT_07E11DCC = 0) and updates the timestamp.
// IDA: RenderDebugWindow
void __cdecl UI_UpdateFpsCounter(void) {
    gFont.SelectFont(gFont.GetTextDC(), (HGDIOBJ)(uintptr_t)((DWORD)(uintptr_t)gFont.GetFont(FONT_NORMAL)));
    DAT_00559c78 = 0xffffffff;
    SetBackgroundTextColor = 0xff000000;
    // DESVIACION: F12 muestra/oculta el contador bajo los avisos superiores
    // del lado izquierdo. El dibujo no consulta teclas ni avanza SIM.
    if (showFps) {
        char text[32];
        Platform::TextFormatSecure(text,gClientText.Get(ClientTextId::RenderFpsOverlay),FPS);
        RenderText(4,28,text,(int)(100*gWindow.GetWidth()/640),1,nullptr);
    }
    DWORD DVar1 = Platform::GetTimer().MultimediaTicks();
    if (999 < DVar1 - (DWORD)_DAT_07e016f0) {
        _DAT_07e016f0 = Platform::GetTimer().MultimediaTicks();
        TotalPacketSize = 0;
    }
}
