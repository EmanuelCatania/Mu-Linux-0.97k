#pragma once
// Scene.h - Game state machine declarations
//
// g_GameState (DAT_005615c0) controls which scene renders each frame:
//
//   1 = Scene_Intro    - Webzen logo + Everyone (ESRB) image
//   2 = Scene_Login    - Login screen (handled via Game_MainLoop)
//   3 = Scene_Loading  - Loading01-03 images, transitions to state 5
//   4 = Scene_CharSelect - Character selection (via Game_MainLoop)
//   5 = InGame         - Actual gameplay (via Game_MainLoop)
//
// State transitions (confirmed from decompilation):
//   Scene_Loading -> sets g_GameState = 5 when done

#include "../stdafx.h"

// Called once per frame from the WinMain message loop.
// Routes to Scene_Intro, Game_MainLoop (Login/CharSelect/InGame),
// or Scene_Loading based on g_GameState.
// @ 0x005274A8
void Scene_Dispatch(HDC hDC);

// State 1: Webzen logo + Everyone.jpg
// @ 0x005137A0
void Scene_Intro(HDC hDC);

// State 3: Loading01-03.jpg splash, then sets g_GameState = 5
// @ 0x00524B20
void Scene_Loading(HDC hDC);

// States 2 (Login), 4 (CharSelect), 5 (InGame) all route here.
// @ 0x00525D40
void Game_MainLoop(HDC hDC);

// Called from Game_MainLoop: renders the login screen UI
// @ 0x00521630
unsigned int Scene_Login(void);

// Called from Game_MainLoop: renders character selection UI
// @ 0x00523B30
int Scene_CharSelect(void);
