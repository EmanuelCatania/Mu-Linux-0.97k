// Input_Timing.cpp

#include "stdafx.h"
#include "Render/RenderProfile.h"
#include "Render/TerrainDraw.h"
#include "Render/Materials.h"
#include "Platform/IInput.h"
#include "Platform/ITimer.h"
#include "Platform/Text.h"
#include "Local/ClientText.h"
#include "globals.h"
#include "functions.h"

namespace { bool showFps=false; }
void ToggleFpsCounter() {
    // DESVIACION: Shift+F12 alterna R1; F12 solo conserva el contador liviano.
    if(Platform::GetInput().KeyState(VK_MENU)&0x8000) {
        // DESVIACION R3b: Ctrl+Alt+F12 compara dos pasadas frente a composición.
        if(Platform::GetInput().KeyState(VK_CONTROL)&0x8000)TerrainDraw::ToggleComposition();
        else TerrainDraw::Toggle();showFps=true;
    }
    else if((Platform::GetInput().KeyState(VK_CONTROL)&0x8000)&&(Platform::GetInput().KeyState(VK_SHIFT)&0x8000)) { Materials::Toggle();showFps=true; }
    else if(Platform::GetInput().KeyState(VK_SHIFT)&0x8000) { RenderProfile::Toggle();showFps=true; }
    else showFps=!showFps;
}

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
    if(showFps && RenderProfile::Requested()) {
        char profile[160];Platform::TextFormatSecure(profile,"%s",RenderProfile::Summary());
        RenderText(4,40,profile,(int)(620*gWindow.GetWidth()/640),1,nullptr);
    }
    if(showFps && TerrainDraw::Requested()) {
        char terrain[160];Platform::TextFormatSecure(terrain,"%s",TerrainDraw::Status());
        RenderText(4,52,terrain,(int)(620*gWindow.GetWidth()/640),1,nullptr);
    }
    if(showFps && Materials::Enabled())RenderText(4,64,"Materiales: experimental",(int)(620*gWindow.GetWidth()/640),1,nullptr);
    DWORD DVar1 = Platform::GetTimer().MultimediaTicks();
    if (999 < DVar1 - (DWORD)_DAT_07e016f0) {
        _DAT_07e016f0 = Platform::GetTimer().MultimediaTicks();
        TotalPacketSize = 0;
    }
}
