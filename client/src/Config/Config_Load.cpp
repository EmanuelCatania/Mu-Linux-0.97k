// Config_Load.cpp
// Config_Load @ 0x0041E0A0
// Config_ReadServerAddr @ 0x0041E800
//
// Config_Load reads from two sources:
//
// 1. Windows Registry: HKEY_CURRENT_USER\SOFTWARE\Webzen\Mu\Config
//    Values:
//      (default)   -> version string (lpData_055c9ba0, 11 bytes)
//      SoundOnOff  -> DWORD -> g_SoundOn  (default: 1)
//      MusicOnOff  -> DWORD -> m_MusicOnOff @ 0x055C9E3C (default: 0)
//      Resolution  -> DWORD -> resolution index (default: 0)
//      TextOut     -> DWORD -> g_TextOut   (default: 0)
//
// 2. config.ini in current directory
//    [LOGIN]
//    Version=XXXXXXXX
//
// Resolution map (switch at end of Config_Load):
//   0 -> 640x480   (g_ScreenW=0x280, g_ScreenH=0x1e0)
//   1 -> 800x600
//   2 -> 1024x768  (0x400 x 0x300)
//   3 -> 1280x1024 (0x500 x 0x400)
//   4 -> 1600x1200 (0x640 x 0x4b0)
//
// Config_ReadServerAddr @ 0x0041E800
//   Reads server IP from config.ini using key 0x75 ('u') and port using 0x70 ('p').
//   Result stored at: PTR_s_connect_muonline_co_kr_005615b8 (IP) and DAT_005615bc (port).
//   Patchs.cpp overrides these:
//     MemoryCpy(0x00558ED8, serverIP, size);
//     SetWord(0x005615BC, serverPort);
//
// MuExe_IntegrityCheck @ 0x0041E560
//   Verifies presence of obfuscated game files:
//     "mu.xe"      (= mu.exe)
//     "mumsg.ll"   (= mumsg.dll)
//     "wz_zp.ll"   (= wz_zp.dll)
//     "message.tf" (= message.wtf)
//   Patchs.cpp bypasses the failure path:
//     SetByte(0x0041ECB5, 0xEB);  // Crack mu.exe check
//     SetByte(0x0041ED25, 0xEB);  // Crack OpenMainExe
//     SetByte(0x0041ED5E, 0xEB);  // Crack config.ini
//     SetByte(0x0041EFB5, 0xEB);  // Crack gg init

#include "stdafx.h"
#include "Config/Config.h"
#include "Net/MuEmu.h"

// Globals set by Config_Load
int   g_ScreenW    = 640;    // DAT_0056156c
int   g_ScreenH    = 480;    // DAT_00561570
DWORD g_SoundOn    = 1;      // DAT_?? (default 1 = sound on)
// g_MusicOn NO se define aca: es un macro-alias de m_MusicOnOff (0x055C9E3C),
// que vive en globals.cpp. Ver la nota en Config.h.
DWORD g_Resolution = 0;      // DAT_?? (default 0 = 640x480)
DWORD g_TextOut    = 0;      // DAT_?? (default 0)
float _DAT_055c9b70 = 1.0f;  // inverse screen width scale: g_ScreenW / 640.0  (UV normalization)
float _DAT_055c9b74 = 1.0f;  // inverse screen height scale: g_ScreenH / 480.0 (g_fScreenRate_y)
char  DAT_055c9bac[12] = {}; // config.ini [LOGIN] Version string (11 chars + null)

// Forward declarations
// Path_GetBasename  @ 0x00412BE0 — extracts filename from a full path/cmdline string
// FileVersion_Get   @ 0x00414500 — reads PE version info from a file (4 shorts out)
static int  Path_GetBasename(char* outBuf, char* fullPath);
static int  FileVersion_Get(LPCSTR filename, unsigned short outVer[4]);

int Config_Load(void)
{
    // --- 1. Build path to config.ini in current directory ---
    //   GetCurrentDirectory(MAX_PATH, localBuf)
    //   if last char != '\\': append '\\'
    //   strcat(localBuf, "config.ini")
    //   -> stored in local_120 (stack buffer, MAX_PATH)
    char configPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, configPath);
    int pathLen = (int)strlen(configPath);
    if (pathLen > 0 && configPath[pathLen - 1] != '\\')
    {
        configPath[pathLen]     = '\\';
        configPath[pathLen + 1] = '\0';
    }
    strcat_s(configPath, MAX_PATH, "config.ini");

    // --- 2. Read [LOGIN] Version from config.ini ---
    //   GetPrivateProfileStringA("LOGIN", "Version", "", DAT_055c9bac, 11, configPath)
    //   Result: 10-char version string (e.g. "1.00h") at DAT_055c9bac
    GetPrivateProfileStringA("LOGIN", "Version", "", DAT_055c9bac, 11, configPath);

    // --- 3. Extract exe name + read PE version ---
    //   Path_GetBasename(exeNameBuf, GetCommandLineA())
    //     → strips path prefix and delimiters → "main.exe"
    //   FileVersion_Get("main.exe", versionWords[4])
    //     → GetFileVersionInfoA + VerQueryValueA("\\")
    //     → versionWords = [FileVer.MajorHi, MajorLo, MinorHi, MinorLo]
    //   Result presumably stored in a global for the version check at login
    char  exeNameBuf[MAX_PATH] = {};
    unsigned short versionWords[4] = {};
    Path_GetBasename(exeNameBuf, GetCommandLineA());
    FileVersion_Get(exeNameBuf, versionWords);

    // --- 4. Registry: HKCU\SOFTWARE\Webzen\Mu\Config ---
    HKEY hKey = NULL;
    if (RegCreateKeyExA(HKEY_CURRENT_USER,
        "SOFTWARE\\Webzen\\Mu\\Config",
        0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        // "ID" value — 11-byte username buffer -> lpData_055c9ba0 (m_ID)
        // Ghidra @ 0x0041e272: RegQueryValueExA(hKey, lpValueName_00559450, NULL, NULL,
        //                       (LPBYTE)0x055c9ba0, &DStack_330=0xb)
        // Usado luego por MoveLogInScene para prefilear DAT_07db8710 (InputText[0]=username).
        // ¡OJO! En el port anterior se escribía a DAT_055c9bac pisando la versión de config.ini.
        DWORD dwSize = 11;
        RegQueryValueExA(hKey, "ID", NULL, NULL, (LPBYTE)lpData_055c9ba0, &dwSize);

        dwSize = 4;
        if (RegQueryValueExA(hKey, "SoundOnOff", NULL, NULL, (LPBYTE)&g_SoundOn, &dwSize) != ERROR_SUCCESS)
            g_SoundOn = 1;

        dwSize = 4;
        if (RegQueryValueExA(hKey, "MusicOnOff", NULL, NULL, (LPBYTE)&g_MusicOn, &dwSize) != ERROR_SUCCESS)
            g_MusicOn = 0;

        dwSize = 4;
        if (RegQueryValueExA(hKey, "Resolution", NULL, NULL, (LPBYTE)&g_Resolution, &dwSize) != ERROR_SUCCESS)
            g_Resolution = 0;

        dwSize = 4;
        if (RegQueryValueExA(hKey, "TextOut", NULL, NULL, (LPBYTE)&g_TextOut, &dwSize) != ERROR_SUCCESS)
            g_TextOut = 0;

        RegCloseKey(hKey);
    }

    // --- 5. Resolution -> screen dimensions ---
    switch (g_Resolution)
    {
    default:
    case 0: g_ScreenW = 640;  g_ScreenH = 480;  break;  // DAT_0056156c/70
    case 1: g_ScreenW = 800;  g_ScreenH = 600;  break;
    case 2: g_ScreenW = 1024; g_ScreenH = 768;  break;
    case 3: g_ScreenW = 1280; g_ScreenH = 1024; break;
    case 4: g_ScreenW = 1600; g_ScreenH = 1200; break;
    }

    // --- 6. UV normalization scalar (used throughout renderer for 640x480-relative coords) ---
    //   _DAT_055c9b70 = (float)g_ScreenW * (1.0f / 640.0f)
    //   e.g. 640→1.0, 800→1.25, 1024→1.6, etc.
    _DAT_055c9b70 = (float)g_ScreenW * (1.0f / 640.0f);

    return 1;
}

// -----------------------------------------------------------------------
// Path_GetBasename @ 0x00412BE0  (__cdecl)
//
// Extracts the filename component from a full path or command-line string.
// Finds the LAST '\\' in fullPath, advances past it, copies the result
// into outBuf (word-aligned memcpy), then null-terminates at the first
// ' ', '"', '/', or '\\' character.
//
// Example:
//   "C:\Mu\main.exe" → "main.exe"
//   "\"C:\Mu\main.exe\" -arg" → "main.exe"
//
// Called from Config_Load to extract the exe name for FileVersion_Get.
// -----------------------------------------------------------------------
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
// Uses operator_new / operator_delete (FUN_0054158c) for the info buffer.
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

// -----------------------------------------------------------------------
// Config_ReadServerAddr @ 0x0041E800  (__thiscall)
//
// Reads server IP and port from config.ini using encrypted key bytes.
// Called from WinMain after Config_Load.
//
// Decompiled flow (fully recovered):
//   1. Config_ReadByEncKey(this, configPath, 0x75, localBuf)
//        key 0x75 = 'u' (obfuscated) → reads IP string into localBuf[256]
//      If fails → return 0
//   2. memcpy(outIP, localBuf, strlen(localBuf)+1)  [word-aligned loop]
//      Copies IP string to outIP
//   3. Config_ReadByEncKey(NULL, configPath, 0x70, localBuf)
//        key 0x70 = 'p' (obfuscated) → reads port string into localBuf
//      If fails → return 0
//   4. *outPort = str_to_ushort(localBuf)    (@ 0x0054261d)
//      Converts port string to unsigned short
//   5. return 1
//
// Parameters:
//   this      — config object context (used by Config_ReadByEncKey)
//   configPath— path to config.ini (param_1, passed as command line string)
//   outIP     — destination buffer for IP string (param_2)
//   outPort   — destination for port as ushort (param_3)
//
// Called from WinMain:
//   Config_ReadServerAddr(this, param_3, &DAT_055c9e04, &port)
//   If success:
//     PTR_s_connect_muonline_co_kr_005615b8 = &DAT_055c9e04  (server IP)
//     DAT_005615bc = port
//
// Helpers:
//   Config_ReadByEncKey @ 0x0041e450 — reads config.ini value by obfuscated key byte
//   str_to_ushort       @ 0x0054261d — atoi variant returning unsigned short
// -----------------------------------------------------------------------
// Lee una línea "IP PORT" (o "IP:PORT") desde `server.cfg` en el directorio
// actual. Reemplaza el lector original de config.ini encriptado (clave 0x75/0x70)
// hasta que portemos Config_ReadByEncKey. Así el usuario puede apuntar al
// ConnectServer local sin inyectar DLL ni recompilar.
//
// Formato aceptado (primera línea no vacía que empiece con dígito):
//     127.0.0.1 44405
//     127.0.0.1:44405
//
// Además acepta líneas `clave=valor` con la identidad del server, que es de
// donde sale la clave de encriptación de MuEmu (2026-08-26):
//     CustomerName=MuLinux
//     ServerSerial=TbYehR2hFUPBKgZj
// Tienen que coincidir con `GameServerInfo - StartUp.dat` del GameServer; si
// faltan, se usan los valores por defecto de MuEmu.cpp. Ver MuEmu.h.
//
// Retorna 1 si encontró IP+puerto válidos (y los escribió en outIP/outPort),
// 0 en caso contrario (el caller mantiene los valores por defecto —
// s_connect_muonline_co_kr_005615b8 / DAT_005615bc).
int Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort)
{
    (void)pConfig; (void)lpCmdLine;
    if (outIP == nullptr || outPort == nullptr) return 0;

    FILE* fp = fopen("server.cfg", "r");
    if (fp == nullptr) fp = fopen("Server.cfg", "r");
    if (fp == nullptr) return 0;

    // server.cfg puede tener UNA o DOS líneas "IP PORT":
    //   1 línea  → conexión directa al GameServer (comportamiento clásico).
    //   2 líneas → línea 1 = ConnectServer (lista + load vía F4/02/04),
    //              línea 2 = GameServer fallback (si el ConnectServer no responde
    //              o el redirect F4/03 no llega). Activa el flujo ConnectServer.
    char line[256];
    int  found  = 0;   // ¿leímos al menos la línea 1?
    int  nAddr  = 0;   // cuántas direcciones válidas leímos

    // Identidad del server para derivar la clave de encriptación de MuEmu
    // (líneas `clave=valor`). Ver MuEmu.h.
    char cfgCustomerName[64] = "";
    char cfgServerSerial[64] = "";

    while (fgets(line, sizeof(line), fp) != nullptr) {
        // Skip leading whitespace
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '\r' || *p == '#' || *p == ';')
            continue;

        // --- Líneas `clave=valor` (CustomerName / ServerSerial) --------------
        // Se procesan antes que el parseo de IP porque no empiezan con dígito y
        // el parser de abajo las descartaría en silencio.
        {
            char* eq = strchr(p, '=');
            if (eq != nullptr) {
                // Clave: recortar espacios de los dos lados.
                char key[64];
                int  keyLen = (int)(eq - p);
                while (keyLen > 0 && (p[keyLen - 1] == ' ' || p[keyLen - 1] == '\t'))
                    keyLen--;
                if (keyLen > (int)sizeof(key) - 1) keyLen = (int)sizeof(key) - 1;
                memcpy(key, p, keyLen);
                key[keyLen] = 0;

                // Valor: recortar espacios iniciales y el fin de línea.
                char* val = eq + 1;
                while (*val == ' ' || *val == '\t') val++;
                int valLen = (int)strlen(val);
                while (valLen > 0 && (val[valLen - 1] == '\n' || val[valLen - 1] == '\r' ||
                                      val[valLen - 1] == ' '  || val[valLen - 1] == '\t'))
                    valLen--;
                val[valLen] = 0;

                if (_stricmp(key, "CustomerName") == 0)
                    lstrcpynA(cfgCustomerName, val, sizeof(cfgCustomerName));
                else if (_stricmp(key, "ServerSerial") == 0) {
                    lstrcpynA(cfgServerSerial, val, sizeof(cfgServerSerial));

                    // El serial no sirve solo para derivar la clave: el server
                    // TAMBIEN lo valida en el login F1/01
                    // (Protocol.cpp:1193, memcmp contra m_ServerSerial ->
                    // GCConnectAccountSend(6) si no coincide). Asi que el mismo
                    // valor tiene que ir al buffer que se manda en el paquete,
                    // o la clave saldria bien y el login fallaria igual.
                    // Se rellena con ceros porque el server compara 16 bytes
                    // contra su m_ServerSerial[17], que tambien viene en cero.
                    memset(DAT_00559624, 0, sizeof(DAT_00559624));
                    int serLen = (int)strlen(cfgServerSerial);
                    if (serLen > (int)sizeof(DAT_00559624)) serLen = (int)sizeof(DAT_00559624);
                    memcpy(DAT_00559624, cfgServerSerial, serLen);
                }

                continue;   // nunca es una dirección
            }
        }

        // Extract IP (digits + dots)
        char ipBuf[64];
        int  ipLen = 0;
        while (ipLen < 63 && p[ipLen] != '\0' &&
               p[ipLen] != ' ' && p[ipLen] != '\t' &&
               p[ipLen] != ':' && p[ipLen] != '\n' && p[ipLen] != '\r') {
            ipBuf[ipLen] = p[ipLen];
            ipLen++;
        }
        ipBuf[ipLen] = '\0';
        if (ipLen == 0) continue;

        p += ipLen;
        while (*p == ' ' || *p == '\t' || *p == ':') p++;

        int port = atoi(p);
        if (port <= 0 || port > 65535) continue;

        if (nAddr == 0) {
            // Línea 1 → destino primario (ConnectServer si hay línea 2).
            memcpy(outIP, ipBuf, ipLen + 1);
            *outPort = (unsigned short)port;
            found = 1;
            nAddr++;
        } else if (nAddr == 1) {
            // Línea 2 → GameServer fallback + activa el flujo ConnectServer.
            memcpy(g_GameServerIP, ipBuf, ipLen + 1);
            g_GameServerPort  = (unsigned short)port;
            g_HasConnectServer = 1;
            nAddr++;
        }
        // Sin `break`: las direcciones extra se ignoran, pero seguimos leyendo
        // para no perder líneas `clave=valor` que vengan después de las IPs.
    }
    fclose(fp);

    // Derivar la clave de MuEmu si server.cfg trajo la identidad del server.
    // Sin esas líneas quedan los valores por defecto (CustomerName="MuLinux"),
    // que es el comportamiento que tenía el cliente antes de esto.
    if (cfgCustomerName[0] != 0 || cfgServerSerial[0] != 0)
        MuEmu::InitKeys(cfgCustomerName, cfgServerSerial);

    return found;
}
