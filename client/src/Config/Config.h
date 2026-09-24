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
// Stores results in PTR_s_connect_muonline_co_kr_005615b8 and DAT_005615bc.
// @ 0x0041E800
int  Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort);

// Known globals (set by Config_Load):
// g_ScreenW / g_ScreenH REMOVIDAS (2026-08-26). Eran variables propias de
// Config_Load.cpp que duplicaban WindowWidth / WindowHeight (DAT_0056156c/70,
// declaradas en globals.h) sin ninguna sincronizacion: el render usaba las
// segundas y nada copiaba un par al otro. Usar WindowWidth / WindowHeight.
extern DWORD g_SoundOn;    // lpData_055c9fe8  (1 = sound on)
// g_MusicOn es un ALIAS del unico global del binario, m_MusicOnOff @ 0x055C9E3C
// (definido en globals.cpp). No declarar una variable propia aca: hasta 2026-08-17
// habia dos memorias distintas — esta se escribia y la de Music.cpp se leia — y
// por eso PlayMp3 salia siempre por el early-return.
extern DWORD m_MusicOnOff;
#define g_MusicOn m_MusicOnOff   // 0x055C9E3C  (0 = musica apagada)
extern DWORD g_Resolution; // lpData_055c9e38 (0-4)
extern DWORD g_TextOut;    // lpData_055ca044

// -- Overrides de server.cfg (DESVIACION DOCUMENTADA, 2026-09-24) ------------
// El 0.97k solo lee estas opciones del registro (la clave Config de Webzen/Mu),
// que es donde las deja el launcher oficial.  Sin launcher no hay forma de
// mandar el cliente preconfigurado, asi que `server.cfg` acepta las mismas
// tres claves y, cuando estan, ganan sobre el registro.
//   MusicOnOff=0|1   SoundOnOff=0|1   Resolution=0..4  (o "800x600")
// -1 = la clave no estaba en el archivo -> se respeta el registro.
extern int g_CfgMusicOnOff;
extern int g_CfgSoundOnOff;
extern int g_CfgResolution;
