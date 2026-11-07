#pragma once
// Config.h - Configuration loading
//
// Config_Load      @ 0x0041E0A0  - reads registry + config.ini
// Config_ReadServerAddr @ 0x0041E800  - reads IP/port from config.ini

#include "stdafx.h"

// Load all config (registry + config.ini).
// Registry key: HKCU\SOFTWARE\Webzen\Mu\Config
//   SoundOnOff  -> g_SoundOn
//   MusicOnOff  -> g_MusicOn
//   Resolution  -> sets g_ScreenW / g_ScreenH:
//     0=640x480  1=800x600  2=1024x768  3=1280x1024  4=1600x1200
//   TextOut     -> g_TextOut
// config.ini [LOGIN] Version=
// Returns 1 on success, 0 on failure.
// @ 0x0041E0A0
int  Config_Load(void);

// Server IP and port: DESVIACION, salen de Config/ServerConfig.h (el original
// los lee de config.ini). Stores results in szServerIpAddress and g_ServerPort.
// @ 0x0041E800
int  Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort);

// Known globals (set by Config_Load):
// La resolucion va a WindowWidth / WindowHeight (DAT_0056156c/70, globals.h);
// no declarar g_ScreenW / g_ScreenH propias.
extern DWORD g_SoundOn;    // lpData_055c9fe8  (1 = sound on)
// Música: gSound.GetMusicEnabled()/SetMusicEnabled().
extern DWORD g_Resolution; // lpData_055c9e38 (0-4)
extern DWORD g_TextOut;    // lpData_055ca044

// -- Modo ventana (DESVIACION DELIBERADA) ------------------------------------
// El 0.97k sólo corre a pantalla completa con un modo de video de 16 bits, que en
// Windows 10/11 no existe. Se porta el modo ventana del DLL; el estado vive en
// CWindow (Core/Window.h) y lo configura Config.ini [Window].
