// Camera_Login.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// Login_CameraUpdate @ 0x004F8EB0 — SIGNATURE MISMATCH.
// Real: void Login_CameraUpdate(float *entity_pos). Builds login-screen orbit camera:
// timer-driven rotation (FUN_004CB520 * DAT_0055283C * cos/sin constants),
// applies rotation matrix (FUN_004F9DB0 + FUN_004FA0B0) and writes to
// DAT_07EEB228/DAT_07EEB218 (login camera world positions).
// Stub kept void() until Scene_Login.cpp callers are updated.
void __cdecl Login_CameraUpdate(void) {
    // Wrapper: calls FUN_004f8eb0 with login-scene entity[0] world position (+0x10).
    FUN_004f8eb0((float *)(DAT_07abf5d0 + 0x10));
}

