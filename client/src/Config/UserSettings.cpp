// UserSettings.cpp — preferencias del jugador desde Config.ini. Ver UserSettings.h.

#include "stdafx.h"
#include "Config/UserSettings.h"

extern "C" void DbgLogPublic(const char* msg);

CUserSettings gUserSettings;

namespace {

struct ResolutionSize { DWORD width; DWORD height; };

// Misma tabla que CWindow::iResolutionValues del DLL.
constexpr ResolutionSize kResolutions[MAX_USER_RESOLUTION] = {
    {  640,  480 }, {  800,  600 }, { 1024,  768 }, { 1280, 1024 },
    { 1280,  720 }, { 1366,  768 }, { 1600,  900 }, { 1920, 1080 },
};

// -1 si la clave no existe (GetPrivateProfileInt no distingue "no está" de 0).
int ReadInt(const char* section, const char* key, const char* iniPath)
{
    char value[16] = {};
    GetPrivateProfileStringA(section, key, "", value, sizeof(value), iniPath);
    if (value[0] == 0) return -1;
    return atoi(value);
}

int ReadFlag(const char* section, const char* key, const char* iniPath)
{
    int value = ReadInt(section, key, iniPath);
    return value < 0 ? -1 : (value != 0);
}

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
    m_WindowMode  = ReadFlag("Window", "WindowMode", iniPath);
    m_Borderless  = ReadFlag("Window", "Borderless", iniPath);
    m_Resolution  = ReadInt ("Window", "Resolution", iniPath);
    if (m_Resolution >= MAX_USER_RESOLUTION) {
        char line[96];
        wsprintfA(line, "Config.ini: Resolution=%d IGNORADO (fuera de 0..%d)",
                  m_Resolution, MAX_USER_RESOLUTION - 1);
        DbgLogPublic(line);
        m_Resolution = -1;
    }

    m_EnableSound = ReadFlag("Sound", "EnableSound", iniPath);
    m_EnableMusic = ReadFlag("Sound", "EnableMusic", iniPath);
    m_SoundLevel  = ReadInt ("Sound", "SoundLevel",  iniPath);
    m_MusicLevel  = ReadInt ("Sound", "MusicLevel",  iniPath);

    GetPrivateProfileStringA("User", "Username", "", m_Username, sizeof(m_Username), iniPath);

    // [Font]: si la sección existe, cada clave que falte toma el default del DLL.
    char section[8] = {};
    if (GetPrivateProfileSectionA("Font", section, sizeof(section), iniPath) > 0) {
        UserFontSettings& f = m_Font;
        f.present = true;
        GetPrivateProfileStringA("Font", "FontName", "Verdana", f.faceName, sizeof(f.faceName), iniPath);
        f.height    = GetPrivateProfileIntA("Font", "FontHeight",    13, iniPath);
        if (f.height > 25) f.height = 25;
        f.bold      = GetPrivateProfileIntA("Font", "FontBold",      0, iniPath);
        f.italic    = GetPrivateProfileIntA("Font", "FontItalic",    0, iniPath);
        f.charset   = GetPrivateProfileIntA("Font", "FontCharset",   DEFAULT_CHARSET, iniPath);
        f.width     = GetPrivateProfileIntA("Font", "FontWidth",     0, iniPath);
        f.underline = GetPrivateProfileIntA("Font", "FontUnderline", 0, iniPath);
        f.quality   = GetPrivateProfileIntA("Font", "FontQuality",   NONANTIALIASED_QUALITY, iniPath);
        f.strikeOut = GetPrivateProfileIntA("Font", "FontStrikeOut", 0, iniPath);
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
