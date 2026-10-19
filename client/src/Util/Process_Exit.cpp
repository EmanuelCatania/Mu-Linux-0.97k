// Process_Exit.cpp

#include "stdafx.h"
#include "Platform/IWindow.h"
#include "globals.h"
#include "functions.h"

// ExitProgram @ 0x004F6CB0 — ExitProgram
// Shows the fatal error message at lpText_07d2aa08 then destroys the main window.
void __cdecl ExitProgram(void)
{
    Platform::GetWindow().ShowError(lpText_07d2aa08, NULL, true, false);
    gWindow.RequestCloseApplication();  // WM_DESTROY
}

