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
#include "Platform/ClientSettings.h"
#include "Platform/Text.h"
#include "Platform/IFileSystem.h"
#include "Config/Config.h"
#include "Net/MuEmu.h"
#include "Config/ServerConfig.h"
#include "Config/UserSettings.h"
#include "Sound/SoundManager.h"

extern "C" void DbgLogPublic(const char* msg);

// Globals set by Config_Load
//
// La resolucion se escribe en WindowWidth / WindowHeight (DAT_0056156c/70),
// igual que el binario; ver el switch de resolucion, mas abajo.
DWORD g_SoundOn    = 1;      // DAT_?? (default 1 = sound on)
// La preferencia de música vive en CSound (IDA 0x055C9E3C).
DWORD g_Resolution = 0;      // DAT_?? (default 0 = 640x480)
DWORD g_TextOut    = 0;      // DAT_?? (default 0)

char  ConfigLoginVersion[12] = {}; // IDA: m_ExeVersion — config.ini [LOGIN] Version string

// Forward declarations
// Path_GetBasename  @ 0x00412BE0 — extracts filename from a full path/cmdline string
// FileVersion_Get   @ 0x00414500 — reads PE version info from a file (4 shorts out)


int Config_Load(void)
{
    // --- 1. Build path to config.ini in current directory ---
    //   GetCurrentDirectory(MAX_PATH, localBuf)
    //   if last char != '\\': append '\\'
    //   strcat(localBuf, "config.ini")
    //   -> stored in local_120 (stack buffer, MAX_PATH)
    char configPath[MAX_PATH];
    Platform::GetFileSystem().WorkingDirectory(MAX_PATH, configPath);
    int pathLen = (int)strlen(configPath);
    if (pathLen > 0 && configPath[pathLen - 1] != '\\')
    {
        configPath[pathLen]     = '\\';
        configPath[pathLen + 1] = '\0';
    }
    Platform::TextConcatSecure(configPath, MAX_PATH, "config.ini");

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
    Platform::GetFileSystem().ReadIniString("LOGIN", "Version", "", m_Version, 11, configPath);

    unsigned short versionWords[4] = {};
    if (Platform::GetClientSettings().ExecutableVersion(versionWords)) {
        Platform::TextFormat(ConfigLoginVersion, "%d.%02d", versionWords[0], versionWords[1]);
        if (versionWords[2]) {
            const char kLetterBase = 'a';   // IDA: word_559470
            char suffix[2] = { (char)(kLetterBase + versionWords[2] - 1), '\0' };
            Platform::TextConcatSecure(ConfigLoginVersion, sizeof(ConfigLoginVersion), suffix);
        }
    } else {
        Platform::TextCopy(ConfigLoginVersion, m_Version, sizeof(ConfigLoginVersion));
    }

    // --- 4. Registry: HKCU\SOFTWARE\Webzen\Mu\Config ---
    Platform::InitialClientSettings initial;
    std::memcpy(initial.username, lpData_055c9ba0, sizeof(initial.username));
    initial.sound = g_SoundOn; initial.music = gSound.GetMusicEnabled();
    initial.resolution = g_Resolution; initial.textOut = g_TextOut;
    Platform::GetClientSettings().ReadDefaults(initial, configPath);
    std::memcpy(lpData_055c9ba0, initial.username, sizeof(initial.username));
    g_SoundOn = initial.sound; gSound.SetMusicEnabled(initial.music);
    g_Resolution = initial.resolution; g_TextOut = initial.textOut;

    // --- 4b. Preferencias de Config.ini (DESVIACION, ver UserSettings.h) ---
    // Se aplican DESPUES del registro: lo que esté en Config.ini gana.
    // `configPath` es el mismo archivo (en Windows "config.ini" y "Config.ini"
    // son el mismo nombre).
    gUserSettings.Load(configPath);
    if (gUserSettings.GetEnableSound() >= 0) g_SoundOn    = (DWORD)gUserSettings.GetEnableSound();
    if (gUserSettings.GetEnableMusic() >= 0) gSound.SetMusicEnabled((DWORD)gUserSettings.GetEnableMusic());
    {
        const bool windowMode = gUserSettings.GetWindowMode() >= 0
                              ? gUserSettings.GetWindowMode() != 0 : gWindow.IsWindowMode();
        const bool borderless = gUserSettings.GetBorderless() >= 0
                              ? gUserSettings.GetBorderless() != 0 : gWindow.IsBorderless();
        gWindow.SetWindowMode(windowMode, borderless);
    }
    if (gUserSettings.GetUsername()[0] != 0)
        Platform::TextCopy((char*)lpData_055c9ba0, gUserSettings.GetUsername(), 11);

    // --- 5. Resolution -> screen dimensions ---
    //
    // IDA escribe WindowWidth/WindowHeight (DAT_0056156c/70) aca mismo
    // (0041E0A0 L121-138); el render entero (ventana, viewport, ortho,
    // proyeccion, mouse) lee esos dos.
    // Config.ini usa la tabla de resoluciones del DLL (ver UserSettings.h); si
    // no trae Resolution, manda el índice del registro con la tabla del binario.
    DWORD width = 640, height = 480;
    if (!CUserSettings::GetResolutionSize(gUserSettings.GetResolution(), &width, &height))
    {
        switch (g_Resolution)
        {
        default:
        case 0: width = 640;  height = 480;  break;
        case 1: width = 800;  height = 600;  break;
        case 2: width = 1024; height = 768;  break;
        case 3: width = 1280; height = 1024; break;
        case 4: width = 1600; height = 1200; break;
        }
    }

    // --- 6. Escalas de pixel -> layout lógico 640x480 ---
    // CWindow::SetResolution las calcula con las fórmulas de IDA (L144-145).
    gWindow.SetResolution(width, height);

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
        Platform::TextCopy(outIP, ServerConfig::ConnectServerIP, 128);
        *outPort = ServerConfig::ConnectServerPort;
        gNetwork.SetConnectServer(true);
    } else {
        Platform::TextCopy(outIP, ServerConfig::GameServerIP, 128);
        *outPort = ServerConfig::GameServerPort;
        gNetwork.SetConnectServer(false);
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
    Platform::TextFormat(line, "ServerConfig: %s=%s:%u GameServer=%s:%u version='%s'",
              gNetwork.HasConnectServer() ? "ConnectServer" : "GameServer(directo)",
              outIP, (unsigned)*outPort,
              ServerConfig::GameServerIP, (unsigned)ServerConfig::GameServerPort, v5);
    DbgLogPublic(line);
    return 1;
}
