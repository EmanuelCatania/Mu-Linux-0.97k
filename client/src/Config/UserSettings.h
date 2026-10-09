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
//   [Antilag] DeleteShadows DeleteObjects DeleteFloor DeleteSkills
//             DeleteStaticEffects DeleteDynamicEffects DeleteWings
//             DeleteHealthBar DeleteInterface DeleteWeather DeleteGlow
//             (0|1, ver eAntilag)
// [MiniMap] se integra con su sistema.

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

// DESVIACION (DLL OptionsMenu.cpp ApplyAntilagDefaults, WeaponView.cpp,
// HealthBar.cpp): el DLL apagaba funciones de render parcheando bytes; acá
// cada función consulta su opción.
enum eAntilag {
    ANTILAG_SHADOWS,          // BMD__RenderBodyShadow (0x00441F00)
    ANTILAG_OBJECTS,          // Terrain_Render (0x004FD800)
    ANTILAG_FLOOR,            // RenderTerrain (0x004F9AC0)
    ANTILAG_SKILLS,           // RenderJoints, RenderEffects, AddTerrainLight
    ANTILAG_STATIC_EFFECTS,   // RenderSprite (0x00479670)
    ANTILAG_DYNAMIC_EFFECTS,  // RenderParticles (0x00478C00)
    ANTILAG_WINGS,            // alas de RenderCharacter
    ANTILAG_HEALTH_BAR,       // barras de vida (UI/HealthBar.cpp)
    ANTILAG_INTERFACE,        // HUD, avisos y viewport 3D a pantalla completa
    // Propias (no estaban en el DLL; texto en ClientText):
    ANTILAG_WEATHER,          // RenderLeaves (0x0046CB70): hojas, lluvia, nieve
    ANTILAG_GLOW,             // brillo +N y del set completo
    MAX_ANTILAG
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
    bool GetDeleteHealthBar() const { return m_Antilag[ANTILAG_HEALTH_BAR]; }
    bool GetAntilag(eAntilag option) const { return m_Antilag[option]; }
    void SetAntilag(eAntilag option, bool enabled);
    // DESVIACION (DLL OptionsMenu PVPWithoutControl): atacar jugadores sin
    // Ctrl.  Como en el DLL no se guarda: vale hasta cerrar el cliente.
    bool GetPvPWithoutControl() const { return m_PvPWithoutControl; }
    void SetPvPWithoutControl(bool enabled) { m_PvPWithoutControl = enabled; }
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
    void SetWindow(bool windowMode, bool borderless, int resolution);
    // Escribe toda la sección [Font] y la marca presente.
    void SetFont(const UserFontSettings& font);
    // Borra [Font]: vuelve la fuente del binario (Arial según la resolución).
    void ResetFont();
    // Índice de la tabla para un tamaño, o -1 si no está.
    static int FindResolution(DWORD width, DWORD height);
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
    bool m_Antilag[MAX_ANTILAG] = {};
    bool m_PvPWithoutControl = false;
    int  m_Language = USER_LANG_DEFAULT;
    char m_IniPath[MAX_PATH] = {};
    char m_Username[11] = {};
    UserFontSettings m_Font;
};

extern CUserSettings gUserSettings;
