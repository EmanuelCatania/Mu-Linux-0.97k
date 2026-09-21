// Net_PipeQuery.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_0053ed00 (0x0053ED00)
// If manager's first byte is non-zero (pipe open), sends opcode 0x616.
uint __cdecl Pipe_SendQuery(void* mgr, char* name)
{
    if (mgr == NULL || *(char *)mgr == '\0') return 0;
    FUN_005404a0((LPCVOID)0x8b1, 0x616, name);
    return 1;
}
