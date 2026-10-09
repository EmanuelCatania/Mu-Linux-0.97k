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
//   [Font]    FontName FontHeight FontBold FontItalic FontCharset FontWidth
//             FontUnderline FontQuality FontStrikeOut  (ver UserFontSettings)
//
//   [Language] LangSelection=Eng|Spn|Por  (Text/Dialog y textos del cliente)
//
// [Antilag] DeleteHealthBar=0|1 oculta las barras de monstruos.
// El resto de [Antilag] y [MiniMap] se integra con sus sistemas.

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

// Valores del F1/04 del server (PMSG_SET_LANG_RECV, LANGUAGE_*).
enum eUserLanguage {
    USER_LANG_DEFAULT = -1,   // sin selección: archivos del 0.97k (Text.bmd)
    USER_LANG_ENGLISH = 0,
    USER_LANG_SPANISH,
    USER_LANG_PORTUGUESE,
    MAX_USER_LANGUAGE
};

// Config.ini [Font], con los mismos defaults que el DLL (Font.cpp) para las
// claves que falten. `present` es false si la sección no existe: entonces
// CFont usa la fuente del binario (Arial, alto según la resolución).
struct UserFontSettings {
    bool present      = false;
    char faceName[32] = "Verdana";
    int  height       = 13;                       // tope 25, como el DLL
    int  bold         = 0;
    int  italic       = 0;
    int  charset      = DEFAULT_CHARSET;
    int  width        = 0;
    int  underline    = 0;
    int  quality      = NONANTIALIASED_QUALITY;
    int  strikeOut    = 0;
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
    bool GetDeleteHealthBar() const { return m_DeleteHealthBar; }
    const char* GetUsername() const { return m_Username; }
    int GetLanguage() const { return m_Language; }
    // "Eng", "Spn" o "Por"; nullptr para USER_LANG_DEFAULT.
    static const char* GetLanguageSuffix(int language);
    const UserFontSettings& GetFont() const { return m_Font; }

    // Cambios desde el menú de opciones: actualizan el valor y lo escriben en
    // el mismo Config.ini que se leyó.
    void SetLanguage(int language);
    void SetSoundLevel(int level);
    void SetMusicLevel(int level);
    void SaveInt(const char* section, const char* key, int value) const;

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
    bool m_DeleteHealthBar = false;
    int  m_Language = USER_LANG_DEFAULT;
    char m_IniPath[MAX_PATH] = {};
    char m_Username[11] = {};
    UserFontSettings m_Font;
};

extern CUserSettings gUserSettings;
