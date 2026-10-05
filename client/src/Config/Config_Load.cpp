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
//   0 -> 640x480   (WindowWidth=0x280, WindowHeight=0x1e0)
//   1 -> 800x600
//   2 -> 1024x768  (0x400 x 0x300)
//   3 -> 1280x1024 (0x500 x 0x400)
//   4 -> 1600x1200 (0x640 x 0x4b0)
//
// Config_ReadServerAddr @ 0x0041E800
//   Reads server IP from config.ini using key 0x75 ('u') and port using 0x70 ('p').
//   Result stored at: PTR_s_connect_muonline_co_kr_005615b8 (IP) and g_ServerPort (port).
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
#include "Config/ServerConfig.h"
#include "Config/UserSettings.h"

extern "C" void DbgLogPublic(const char* msg);

// Globals set by Config_Load
//
// La resolucion se escribe en WindowWidth / WindowHeight (DAT_0056156c/70),
// igual que el binario; ver el switch de resolucion, mas abajo.
DWORD g_SoundOn    = 1;      // DAT_?? (default 1 = sound on)
// g_MusicOn NO se define aca: es un macro-alias de m_MusicOnOff (0x055C9E3C),
// que vive en globals.cpp. Ver la nota en Config.h.
DWORD g_Resolution = 0;      // DAT_?? (default 0 = 640x480)
DWORD g_TextOut    = 0;      // DAT_?? (default 0)

// Modo ventana (ver Config.h). Default del DLL: en ventana y con bordes; lo
// pisa Config.ini [Window]. El binario original no tiene estas opciones.
int g_WindowMode = 1;
int g_Borderless = 0;
// g_fScreenRate_x / _y — escala pixel -> layout 640x480. Las calcula
// Config_Load desde WindowWidth/WindowHeight; el 1.0f es solo el valor
// previo a esa llamada (y el correcto para 640x480).
float _DAT_055c9b70 = 1.0f;  // g_fScreenRate_x
float _DAT_055c9b74 = 1.0f;  // g_fScreenRate_y
char  ConfigLoginVersion[12] = {}; // IDA: m_ExeVersion — config.ini [LOGIN] Version string

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

    // --- 2/3. Version que muestra el pie del login (m_ExeVersion) ---
    //
    // IDA 0x41E0A0 L38-75.  El valor del ini es SOLO el fallback: la fuente
    // real es el recurso VERSIONINFO del propio exe.
    //
    //   GetPrivateProfileStringA("LOGIN","Version", "", m_Version, 11, ini);
    //   if (!GetFileNameOfFilePath(lpszFile, GetCommandLineA()))
    //        m_ExeVersion = m_Version;
    //   else if (GetFileVersion(lpszFile, wVersion)) {
    //        sprintf(m_ExeVersion, "%d.%02d", wVersion[0], wVersion[1]);
    //        if (wVersion[2])
    //            strcat(m_ExeVersion, (char)(word_559470 + wVersion[2] - 1));
    //   } else m_ExeVersion = m_Version;
    //
    // word_559470 = 'a' (0x61, leido del binario).  El main.exe original
    // declara FileVersion 0.97.11.0, asi que sale "0.97" + la letra 11 = 'k':
    // de ahi viene el nombre "0.97k".
    char m_Version[12] = {};   // IDA: m_Version (solo lo usa este bloque)
    GetPrivateProfileStringA("LOGIN", "Version", "", m_Version, 11, configPath);

    char  exeNameBuf[MAX_PATH] = {};
    unsigned short versionWords[4] = {};
    if (!Path_GetBasename(exeNameBuf, GetCommandLineA())) {
        lstrcpynA(ConfigLoginVersion, m_Version, sizeof(ConfigLoginVersion));
    } else if (FileVersion_Get(exeNameBuf, versionWords)) {
        wsprintfA(ConfigLoginVersion, "%d.%02d", versionWords[0], versionWords[1]);
        if (versionWords[2]) {
            const char kLetterBase = 'a';   // IDA: word_559470
            char suffix[2] = { (char)(kLetterBase + versionWords[2] - 1), '\0' };
            strcat_s(ConfigLoginVersion, sizeof(ConfigLoginVersion), suffix);
        }
    } else {
        lstrcpynA(ConfigLoginVersion, m_Version, sizeof(ConfigLoginVersion));
    }

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

    // --- 4b. Preferencias de Config.ini (DESVIACION, ver UserSettings.h) ---
    // Se aplican DESPUES del registro: lo que esté en Config.ini gana.
    // `configPath` es el mismo archivo (en Windows "config.ini" y "Config.ini"
    // son el mismo nombre).
    gUserSettings.Load(configPath);
    if (gUserSettings.GetEnableSound() >= 0) g_SoundOn    = (DWORD)gUserSettings.GetEnableSound();
    if (gUserSettings.GetEnableMusic() >= 0) g_MusicOn    = (DWORD)gUserSettings.GetEnableMusic();
    if (gUserSettings.GetWindowMode()  >= 0) g_WindowMode = gUserSettings.GetWindowMode();
    if (gUserSettings.GetBorderless()  >= 0) g_Borderless = gUserSettings.GetBorderless();
    if (gUserSettings.GetUsername()[0] != 0)
        lstrcpynA((char*)lpData_055c9ba0, gUserSettings.GetUsername(), 11);

    // --- 5. Resolution -> screen dimensions ---
    //
    // IDA escribe WindowWidth/WindowHeight (DAT_0056156c/70) aca mismo
    // (0041E0A0 L121-138); el render entero (ventana, viewport, ortho,
    // proyeccion, mouse) lee esos dos.
    // Config.ini usa la tabla de resoluciones del DLL (ver UserSettings.h); si
    // no trae Resolution, manda el índice del registro con la tabla del binario.
    if (!CUserSettings::GetResolutionSize(gUserSettings.GetResolution(),
                                          &WindowWidth, &WindowHeight))
    {
        switch (g_Resolution)
        {
        default:
        case 0: WindowWidth = 640;  WindowHeight = 480;  break;
        case 1: WindowWidth = 800;  WindowHeight = 600;  break;
        case 2: WindowWidth = 1024; WindowHeight = 768;  break;
        case 3: WindowWidth = 1280; WindowHeight = 1024; break;
        case 4: WindowWidth = 1600; WindowHeight = 1200; break;
        }
    }

    // --- 6. Escalas de pixel -> layout logico 640x480 ---
    //
    // IDA (0041E0A0 L144-145):
    //     g_fScreenRate_x = (double)WindowWidth  * 0.0015625;      // = /640
    //     g_fScreenRate_y = (double)WindowHeight * 0.0020833334;   // = /480
    //
    // Lifecycle: se calculan UNA vez, aca. Es lo que hace el binario — el
    // unico escritor de estas dos variables en todo el decompile es esta
    // funcion. No se agrega recalculo por resize: la ventana es WS_POPUP y el
    // WndProc no maneja WM_SIZE / WM_SIZING / WM_DISPLAYCHANGE.
    g_fScreenRate_x = (float)((double)(int)WindowWidth  * 0.0015625);
    g_fScreenRate_y = (float)((double)(int)WindowHeight * 0.0020833334);

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
//     g_ServerPort = port
//
// Helpers:
//   Config_ReadByEncKey @ 0x0041e450 — reads config.ini value by obfuscated key byte
//   str_to_ushort       @ 0x0054261d — atoi variant returning unsigned short
// -----------------------------------------------------------------------
// DESVIACION: el original lee IP y puerto de config.ini con claves ofuscadas
// (o de la línea de comandos que arma el launcher). Acá la conexión y la
// identidad del server vienen compiladas en Config/ServerConfig.h, como en
// MU 5.2. Ver ese archivo para apuntar a otro server.
//
// Con ConnectServerPort != 0 se usa el flujo ConnectServer: outIP/outPort
// apuntan al ConnectServer y el GameServer queda como fallback. Con 0 se
// conecta directo al GameServer.
//
// Además deja el serial y la versión que viajan en el login F1/01, y deriva la
// clave de encriptación de MuEmu.
int Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort)
{
    (void)pConfig; (void)lpCmdLine;
    if (outIP == nullptr || outPort == nullptr) return 0;

    if (ServerConfig::ConnectServerPort != 0) {
        lstrcpynA(outIP, ServerConfig::ConnectServerIP, 128);
        *outPort = ServerConfig::ConnectServerPort;
        lstrcpynA(g_GameServerIP, ServerConfig::GameServerIP, sizeof(g_GameServerIP));
        g_GameServerPort   = ServerConfig::GameServerPort;
        g_HasConnectServer = 1;
    } else {
        lstrcpynA(outIP, ServerConfig::GameServerIP, 128);
        *outPort = ServerConfig::GameServerPort;
        g_HasConnectServer = 0;
    }

    // Serial del login: el server compara 16 bytes contra su m_ServerSerial[17],
    // relleno de ceros (Protocol.cpp:1193).
    memset(Serial, 0, sizeof(Serial));
    int serLen = (int)strlen(ServerConfig::ServerSerial);
    if (serLen > (int)sizeof(Serial)) serLen = (int)sizeof(Serial);
    memcpy(Serial, ServerConfig::ServerSerial, serLen);

    // Versión del login: "0.97.11" se condensa con los índices 0,2,3,5,6, igual
    // que CServerInfo::ReadStartupInfo; "09711" se toma tal cual. El paquete la
    // lleva ofuscada: el cliente guarda v[i] + i + 1 (Protocol.cpp:1186).
    const char* ver  = ServerConfig::ClientVersion;
    char        v5[6] = { 0 };
    int         vlen = (int)strlen(ver);
    if (vlen >= 7 && ver[1] == '.') {
        v5[0] = ver[0]; v5[1] = ver[2]; v5[2] = ver[3];
        v5[3] = ver[5]; v5[4] = ver[6];
    } else if (vlen >= 5) {
        memcpy(v5, ver, 5);
    }
    if (v5[0] != 0) {
        for (int i = 0; i < 5; i++)
            Version[i] = (BYTE)(v5[i] + i + 1);
    }

    MuEmu::InitKeys(ServerConfig::CustomerName, ServerConfig::ServerSerial);

    char line[256];
    wsprintfA(line, "ServerConfig: %s=%s:%u GameServer=%s:%u version='%s'",
              g_HasConnectServer ? "ConnectServer" : "GameServer(directo)",
              outIP, (unsigned)*outPort,
              ServerConfig::GameServerIP, (unsigned)ServerConfig::GameServerPort, v5);
    DbgLogPublic(line);
    return 1;
}
