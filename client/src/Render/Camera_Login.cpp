// Camera_Login.cpp
// Cámara de la escena de login (Login_CameraUpdate).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// FUN_004F8EB0 @ 0x004F8EB0 — Login_CameraUpdate; wrapper signature differs.
// Real: void Login_CameraUpdate(float *entity_pos). Builds login-screen orbit camera:
// timer-driven rotation (FUN_004CB520 * DAT_0055283C * cos/sin constants),
// applies rotation matrix (FUN_004F9DB0 + FUN_004FA0B0) and writes to
// DAT_07EEB228/DAT_07EEB218 (login camera world positions).
// Se mantiene void(): los callers (Game_SceneUpdate / Game_EnterWorldTick) no pasan la posición.
void __cdecl Login_CameraUpdate(void) {
    // Wrapper: calls CreateFrustrum2D with login-scene entity[0] world position (+0x10).
    CreateFrustrum2D((float *)(DAT_07abf5d0 + 0x10));
}

