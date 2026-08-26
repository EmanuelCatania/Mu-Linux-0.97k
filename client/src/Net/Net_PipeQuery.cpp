// Net_PipeQuery.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0053ed00 @ 0x0053ed00 — Pipe_SendQuery
// If manager's first byte is non-zero (pipe open), sends opcode 0x616.
uint __cdecl FUN_0053ed00(void* mgr, char* name)
{
    if (mgr == NULL || *(char *)mgr == '\0') return 0;
    FUN_005404a0((LPCVOID)0x8b1, 0x616, name);
    return 1;
}
