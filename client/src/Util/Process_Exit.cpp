// Process_Exit.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// ExitProgram @ 0x004F6CB0 — ExitProgram
// Shows the fatal error message at lpText_07d2aa08 then destroys the main window.
void __cdecl ExitProgram(void)
{
    MessageBoxA(g_hWnd, lpText_07d2aa08, NULL, 0);
    SendMessageA(g_hWnd, 2, 0, 0);  // WM_DESTROY
}

