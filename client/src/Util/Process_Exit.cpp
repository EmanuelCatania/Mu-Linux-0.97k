// Process_Exit.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// ExitProgram @ 0x004F6CB0 — ExitProgram
// Shows the fatal error message at lpText_07d2aa08 then destroys the main window.
void __cdecl ExitProgram(void)
{
    MessageBoxA(gWindow.GetHwnd(), lpText_07d2aa08, NULL, 0);
    SendMessageA(gWindow.GetHwnd(), 2, 0, 0);  // WM_DESTROY
}

