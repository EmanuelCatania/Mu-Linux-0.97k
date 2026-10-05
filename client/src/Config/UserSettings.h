#pragma once
// UserSettings.h — preferencias del jugador, leídas de Config.ini.
//
// DESVIACION: el 0.97k sólo lee estas opciones del registro (clave
// SOFTWARE\Webzen\Mu\Config, que escribía el launcher oficial). Acá se leen de
// `Config.ini` en la carpeta del cliente, con las MISMAS secciones y claves que
// usaba el Main.dll de inyección, así sirve el Config.ini que ya tienen los
// testers. Cuando una clave no está, manda el registro (y si tampoco está, el
// default del binario).
//
//   [Window]  WindowMode=0|1   Borderless=0|1   Resolution=0..7 (tabla de abajo)
//   [Sound]   EnableSound=0|1  EnableMusic=0|1  SoundLevel=0..9  MusicLevel=0..9
//   [User]    Username=        (precarga el campo de usuario del login)
//
// Las secciones [Font], [Antilag], [MiniMap] y [Language] del Config.ini del DLL
// se van a leer cuando se integren esos sistemas.

#include "stdafx.h"

// Índices de resolución del Config.ini del DLL (Enums.h / Window.cpp). Los
// cuatro primeros coinciden con los del registro del 0.97k; el 4 NO (en el
// registro es 1600x1200).
enum eUserResolution {
    RES_640x480 = 0,
    RES_800x600,
    RES_1024x768,
    RES_1280x1024,
    RES_1280x720,
    RES_1366x768,
    RES_1600x900,
    RES_1920x1080,
    MAX_USER_RESOLUTION
};

class CUserSettings {
public:
    // Lee `iniPath`. Lo que no está queda en -1 (o vacío) y no se aplica.
    void Load(const char* iniPath);

    // -1 = la clave no está en Config.ini.
    int GetWindowMode()  const { return m_WindowMode; }
    int GetBorderless()  const { return m_Borderless; }
    int GetResolution()  const { return m_Resolution; }
    int GetEnableSound() const { return m_EnableSound; }
    int GetEnableMusic() const { return m_EnableMusic; }
    int GetSoundLevel()  const { return m_SoundLevel; }
    int GetMusicLevel()  const { return m_MusicLevel; }
    const char* GetUsername() const { return m_Username; }

    // Ancho y alto de un índice de resolución; false si el índice no existe.
    static bool GetResolutionSize(int index, DWORD* width, DWORD* height);

private:
    int  m_WindowMode  = -1;
    int  m_Borderless  = -1;
    int  m_Resolution  = -1;
    int  m_EnableSound = -1;
    int  m_EnableMusic = -1;
    int  m_SoundLevel  = -1;
    int  m_MusicLevel  = -1;
    char m_Username[11] = {};
};

extern CUserSettings gUserSettings;
