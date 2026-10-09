// UserSettings.cpp — preferencias del jugador desde Config.ini. Ver UserSettings.h.

#include "stdafx.h"
#include "Config/UserSettings.h"
#include <cerrno>
#include <climits>
#include <cctype>

extern "C" void DbgLogPublic(const char* msg);

CUserSettings gUserSettings;

namespace {

struct ResolutionSize { DWORD width; DWORD height; };

// Misma tabla que CWindow::iResolutionValues del DLL.
constexpr ResolutionSize kResolutions[MAX_USER_RESOLUTION] = {
    {  640,  480 }, {  800,  600 }, { 1024,  768 }, { 1280, 1024 },
    { 1280,  720 }, { 1366,  768 }, { 1600,  900 }, { 1920, 1080 },
};

// Ausente, truncado o inválido: no reemplazar el valor del registro/default.
bool TryReadInt(const char* section, const char* key, const char* iniPath, int& result)
{
    char value[64] = {};
    const DWORD length = GetPrivateProfileStringA(section, key, "", value, sizeof(value), iniPath);
    if (!length || length >= sizeof(value) - 1) return false;
    char* end;
    errno = 0;
    const long parsed = strtol(value, &end, 10);
    if (end == value || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX) return false;
    while (*end && isspace((unsigned char)*end)) ++end;
    if (*end) return false;
    result = (int)parsed;
    return true;
}

int ReadInt(const char* section, const char* key, const char* iniPath)
{
    int value;
    return TryReadInt(section, key, iniPath, value) ? value : -1;
}

int ReadRange(const char* section, const char* key, const char* iniPath,
              int low, int high, int fallback)
{
    int value;
    return TryReadInt(section, key, iniPath, value) && value >= low && value <= high
        ? value : fallback;
}

int ReadFlag(const char* section, const char* key, const char* iniPath)
{
    int value = ReadInt(section, key, iniPath);
    return value < 0 ? -1 : (value != 0);
}

const char* const AntilagKeys[MAX_ANTILAG] = {
    "DeleteShadows", "DeleteObjects", "DeleteFloor", "DeleteSkills",
    "DeleteStaticEffects", "DeleteDynamicEffects", "DeleteWings",
    "DeleteHealthBar", "DeleteInterface", "DeleteWeather", "DeleteGlow"
};
const char* AntilagKey(int option) { return AntilagKeys[option]; }

} // namespace

bool CUserSettings::GetResolutionSize(int index, DWORD* width, DWORD* height)
{
    if (index < 0 || index >= MAX_USER_RESOLUTION) return false;
    *width  = kResolutions[index].width;
    *height = kResolutions[index].height;
    return true;
}

void CUserSettings::Load(const char* iniPath)
{
    // Una segunda carga no debe conservar la fuente ni preferencias ausentes.
    *this = CUserSettings{};
    strcpy_s(m_IniPath, iniPath);
    for (int i = 0; i < MAX_ANTILAG; ++i)
        m_Antilag[i] = ReadFlag("Antilag", AntilagKey(i), iniPath) > 0;
    m_WindowMode  = ReadFlag("Window", "WindowMode", iniPath);
    m_Borderless  = ReadFlag("Window", "Borderless", iniPath);
    m_Resolution  = ReadInt ("Window", "Resolution", iniPath);
    if (m_Resolution < -1 || m_Resolution >= MAX_USER_RESOLUTION) {
        char line[96];
        wsprintfA(line, "Config.ini: Resolution=%d IGNORADO (fuera de 0..%d)",
                  m_Resolution, MAX_USER_RESOLUTION - 1);
        DbgLogPublic(line);
        m_Resolution = -1;
    }

    m_EnableSound = ReadFlag("Sound", "EnableSound", iniPath);
    m_EnableMusic = ReadFlag("Sound", "EnableMusic", iniPath);
    m_SoundLevel  = ReadRange("Sound", "SoundLevel", iniPath, 0, 9, -1);
    m_MusicLevel  = ReadRange("Sound", "MusicLevel", iniPath, 0, 9, -1);

    GetPrivateProfileStringA("User", "Username", "", m_Username, sizeof(m_Username), iniPath);

    char language[8] = {};
    GetPrivateProfileStringA("Language", "LangSelection", "", language, sizeof(language), iniPath);
    for (int i = 0; language[0] && i < MAX_USER_LANGUAGE; ++i)
        if (_stricmp(language, GetLanguageSuffix(i)) == 0) m_Language = i;
    if (language[0] && m_Language == USER_LANG_DEFAULT) {
        char line[96];
        wsprintfA(line, "Config.ini: LangSelection=%s IGNORADO (Eng, Spn o Por)", language);
        DbgLogPublic(line);
    }

    // [Font]: si la sección existe, cada clave que falte toma el default del DLL.
    char section[8] = {};
    if (GetPrivateProfileSectionA("Font", section, sizeof(section), iniPath) > 0) {
        UserFontSettings& f = m_Font;
        f.present = true;
        GetPrivateProfileStringA("Font", "FontName", "Verdana", f.faceName, sizeof(f.faceName), iniPath);
        int height;
        if (TryReadInt("Font", "FontHeight", iniPath, height))
            f.height = height < -25 ? -25 : (height > 25 ? 25 : height);
        f.bold = ReadFlag("Font", "FontBold", iniPath) > 0;
        f.italic = ReadFlag("Font", "FontItalic", iniPath) > 0;
        f.charset   = ReadRange("Font", "FontCharset", iniPath, 0, 255, DEFAULT_CHARSET);
        f.width     = ReadRange("Font", "FontWidth", iniPath, -25, 25, 0);
        f.underline = ReadFlag("Font", "FontUnderline", iniPath) > 0;
        f.quality   = ReadRange("Font", "FontQuality", iniPath, 0, CLEARTYPE_NATURAL_QUALITY, NONANTIALIASED_QUALITY);
        f.strikeOut = ReadFlag("Font", "FontStrikeOut", iniPath) > 0;
    }

    char line[160];
    wsprintfA(line, "Config.ini: WindowMode=%d Borderless=%d Resolution=%d "
                    "EnableSound=%d EnableMusic=%d (-1 = no está)",
              m_WindowMode, m_Borderless, m_Resolution, m_EnableSound, m_EnableMusic);
    DbgLogPublic(line);
    if (m_Font.present) {
        wsprintfA(line, "Config.ini: Font='%s' %d (bold=%d quality=%d)",
                  m_Font.faceName, m_Font.height, m_Font.bold, m_Font.quality);
    } else {
        wsprintfA(line, "Config.ini: sin [Font], se usa la fuente del binario (Arial)");
    }
    DbgLogPublic(line);
}

const char* CUserSettings::GetLanguageSuffix(int language)
{
    static const char* const Suffix[MAX_USER_LANGUAGE] = { "Eng", "Spn", "Por" };
    return language >= 0 && language < MAX_USER_LANGUAGE ? Suffix[language] : nullptr;
}

void CUserSettings::SaveInt(const char* section, const char* key, int value) const
{
    if (!m_IniPath[0]) return;
    char text[16];
    wsprintfA(text, "%d", value);
    WritePrivateProfileStringA(section, key, text, m_IniPath);
}

void CUserSettings::SetLanguage(int language)
{
    if (language < USER_LANG_DEFAULT || language >= MAX_USER_LANGUAGE) return;
    m_Language = language;
    // Sin selección se borra la clave: vuelven los archivos del 0.97k.
    if (m_IniPath[0])
        WritePrivateProfileStringA("Language", "LangSelection", GetLanguageSuffix(language), m_IniPath);
}

void CUserSettings::SetSoundLevel(int level)
{
    m_SoundLevel = level;
    SaveInt("Sound", "SoundLevel", level);
}

void CUserSettings::SetMusicLevel(int level)
{
    m_MusicLevel = level;
    SaveInt("Sound", "MusicLevel", level);
}

void CUserSettings::SetAntilag(eAntilag option, bool enabled)
{
    if (option < 0 || option >= MAX_ANTILAG) return;
    m_Antilag[option] = enabled;
    SaveInt("Antilag", AntilagKey(option), enabled ? 1 : 0);
}

void CUserSettings::SetWindow(bool windowMode, bool borderless, int resolution)
{
    m_WindowMode = windowMode ? 1 : 0;
    m_Borderless = borderless ? 1 : 0;
    m_Resolution = resolution;
    SaveInt("Window", "WindowMode", m_WindowMode);
    SaveInt("Window", "Borderless", m_Borderless);
    if (resolution >= 0) SaveInt("Window", "Resolution", resolution);
}

int CUserSettings::FindResolution(DWORD width, DWORD height)
{
    for (int i = 0; i < MAX_USER_RESOLUTION; ++i)
        if (kResolutions[i].width == width && kResolutions[i].height == height) return i;
    return -1;
}
