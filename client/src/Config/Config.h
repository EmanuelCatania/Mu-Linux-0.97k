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

// Read server IP and port from config.ini.
// Stores results in PTR_s_connect_muonline_co_kr_005615b8 and g_ServerPort.
// @ 0x0041E800
int  Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort);

// Known globals (set by Config_Load):
// La resolucion va a WindowWidth / WindowHeight (DAT_0056156c/70, globals.h);
// no declarar g_ScreenW / g_ScreenH propias.
extern DWORD g_SoundOn;    // lpData_055c9fe8  (1 = sound on)
// g_MusicOn es un ALIAS del unico global del binario, m_MusicOnOff @ 0x055C9E3C
// (definido en globals.cpp). No declarar una variable propia aca: Music.cpp lee
// m_MusicOnOff y tiene que ser la misma memoria.
extern DWORD m_MusicOnOff;
#define g_MusicOn m_MusicOnOff   // 0x055C9E3C  (0 = musica apagada)
extern DWORD g_Resolution; // lpData_055c9e38 (0-4)
extern DWORD g_TextOut;    // lpData_055ca044

// -- Overrides de server.cfg (DESVIACION DOCUMENTADA) ------------------------
// El 0.97k solo lee estas opciones del registro (la clave Config de Webzen/Mu).
// `server.cfg` acepta las mismas tres claves y, cuando estan, ganan sobre el
// registro (permite distribuir el cliente preconfigurado sin launcher).
//   MusicOnOff=0|1   SoundOnOff=0|1   Resolution=0..4  (o "800x600")
// -1 = la clave no estaba en el archivo -> se respeta el registro.
extern int g_CfgMusicOnOff;
extern int g_CfgSoundOnOff;
extern int g_CfgResolution;

// -- Modo ventana (DESVIACION DELIBERADA) ------------------------------------
// El 0.97k solo corre a pantalla completa con un modo de video de 16 bits
// (WinMain + StartWindow, WS_POPUP en (0,0)), que en Windows 10/11 no existe.
// Se porta el modo ventana del DLL de inyeccion (Source/Client/Main/Window.cpp,
// CWindow::StartWindow + ChangeDisplaySettingsFunction), que ademas elige el
// modo de video por la mayor profundidad de color disponible.
//   WindowMode=0|1   (1 = ventana, default;  0 = pantalla completa)
//   Borderless=0|1   (solo en modo ventana: sin barra de titulo ni borde)
// Las dos salen de server.cfg, como el resto de los overrides de arriba.
extern int g_WindowMode;   // 1 = ventana
extern int g_Borderless;   // 1 = ventana sin bordes
