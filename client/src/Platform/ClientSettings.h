#pragma once
#include "Platform/IClientSettings.h"
#include "Platform/PlatformHeaders.h"
#include "Platform/IFileSystem.h"
#include "Platform/Text.h"
#include <cstdlib>
namespace Platform {
namespace ClientSettingsDetail {
#ifdef _WIN32
static int Path_GetBasename(char* outBuf, char* fullPath)
{
    // Find last '\\' in fullPath
    char* lastSlash = fullPath;
    char* p = fullPath;
    while (p) {
        char* next = strchr(p + 1, '\\');
        lastSlash = p;
        p = next;
    }

    // If at least one '\\' exists, skip past it
    char* src = (strchr(fullPath, '\\') != nullptr) ? lastSlash + 1 : lastSlash;

    // Copy src → outBuf (mimics 4-byte word copy + remainder)
    size_t len = strlen(src) + 1;
    memcpy(outBuf, src, len);

    // Null-terminate at first delimiter
    for (char* q = outBuf; *q; ++q) {
        char c = *q;
        if (c == ' ' || c == '"' || c == '/' || c == '\\') {
            *q = '\0';
            break;
        }
    }
    return 1;
}

// -----------------------------------------------------------------------
// FileVersion_Get @ 0x00414500  (__cdecl)
//
// Reads the PE version resource from the given file.
// Uses Win32: GetFileVersionInfoSizeA → GetFileVersionInfoA →
//             VerQueryValueA("\\") → VS_FIXEDFILEINFO
//
// outVer[0] = HIWORD(FileVersionMS)  — major
// outVer[1] = LOWORD(FileVersionMS)  — minor
// outVer[2] = HIWORD(FileVersionLS)  — build
// outVer[3] = LOWORD(FileVersionLS)  — revision
//
// Returns 1 on success, 0 on failure.
// Uses operator_new / operator_delete for the info buffer.
// lpSubBlock_005592d0 = "\\" (root query, retrieves VS_FIXEDFILEINFO).
// -----------------------------------------------------------------------
static int FileVersion_Get(LPCSTR filename, unsigned short outVer[4])
{
    DWORD  dummy;
    DWORD  dwLen = GetFileVersionInfoSizeA(filename, &dummy);
    if (dwLen == 0) return 0;

    BYTE* lpData = new BYTE[dwLen];
    if (!GetFileVersionInfoA(filename, 0, dwLen, lpData)) {
        delete[] lpData;
        return 0;
    }

    VS_FIXEDFILEINFO* pInfo = nullptr;
    UINT  infoLen = 0;
    if (!VerQueryValueA(lpData, "\\", (LPVOID*)&pInfo, &infoLen)) {
        delete[] lpData;
        return 0;
    }

    outVer[0] = (unsigned short)(pInfo->dwFileVersionMS >> 16);
    outVer[1] = (unsigned short)(pInfo->dwFileVersionMS & 0xFFFF);
    outVer[2] = (unsigned short)(pInfo->dwFileVersionLS >> 16);
    outVer[3] = (unsigned short)(pInfo->dwFileVersionLS & 0xFFFF);

    delete[] lpData;
    return 1;
}

#endif
class ClientSettings final : public IClientSettings {
public:
    bool ExecutableVersion(unsigned short words[4]) override {
#ifdef _WIN32
        char path[MAX_PATH]{};
        return Path_GetBasename(path, GetCommandLineA()) && FileVersion_Get(path, words);
#else
        // Versión de este cliente, equivalente al FILEVERSION de resource.rc.
        words[0] = 0; words[1] = 97; words[2] = 11; words[3] = 0;
        return true;
#endif
    }
    void ReadDefaults(InitialClientSettings& settings, const char* iniPath) override {
#ifdef _WIN32
    HKEY hKey = NULL;
    if (RegCreateKeyExA(HKEY_CURRENT_USER,
        "SOFTWARE\\Webzen\\Mu\\Config",
        0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        // "ID" value — 11-byte username buffer -> settings.username (m_ID)
        // Ghidra @ 0x0041e272: RegQueryValueExA(hKey, lpValueName_00559450, NULL, NULL,
        //                       (LPBYTE)0x055c9ba0, &DStack_330=0xb)
        // Usado luego por MoveLogInScene para prefilear DAT_07db8710 (InputText[0]=username).
        DWORD dwSize = 11;
        RegQueryValueExA(hKey, "ID", NULL, NULL, (LPBYTE)settings.username, &dwSize);

        dwSize = 4;
        if (RegQueryValueExA(hKey, "SoundOnOff", NULL, NULL, (LPBYTE)&settings.sound, &dwSize) != ERROR_SUCCESS)
            settings.sound = 1;

        dwSize = 4;
        DWORD musicEnabled = settings.music;
        if (RegQueryValueExA(hKey, "MusicOnOff", NULL, NULL, (LPBYTE)&musicEnabled, &dwSize) != ERROR_SUCCESS)
            musicEnabled = 0;
        settings.music = musicEnabled;

        dwSize = 4;
        if (RegQueryValueExA(hKey, "Resolution", NULL, NULL, (LPBYTE)&settings.resolution, &dwSize) != ERROR_SUCCESS)
            settings.resolution = 0;

        dwSize = 4;
        if (RegQueryValueExA(hKey, "TextOut", NULL, NULL, (LPBYTE)&settings.textOut, &dwSize) != ERROR_SUCCESS)
            settings.textOut = 0;

        RegCloseKey(hKey);
    }

        (void)iniPath;
#else
        // Linux toma las preferencias del mismo Config.ini, sin registro emulado.
        GetFileSystem().ReadIniString("User", "Username", "", settings.username, sizeof(settings.username), iniPath);
        auto read = [&](const char* section, const char* key, std::uint32_t fallback, unsigned maximum) {
            char text[32]{};
            GetFileSystem().ReadIniString(section, key, "", text, sizeof(text), iniPath);
            char* end = nullptr;
            const long value = std::strtol(text, &end, 10);
            return end != text && *end == 0 && value >= 0 && value <= maximum ? static_cast<std::uint32_t>(value) : fallback;
        };
        settings.sound = read("Sound", "EnableSound", settings.sound, 1);
        settings.music = read("Sound", "EnableMusic", settings.music, 1);
        settings.resolution = read("Window", "Resolution", settings.resolution, 4);
#endif
    }
};
}
inline IClientSettings& GetClientSettings() { static ClientSettingsDetail::ClientSettings settings; return settings; }
}
