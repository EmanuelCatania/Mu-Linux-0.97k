// globals.cpp — Definitions for all DAT_ global variables.
// In the original binary these are fixed addresses; here they are regular
// zero-initialized globals that the linker places wherever it chooses.
// To test as an overlay patch (DLL injection) these definitions would be
// replaced by pointer aliases to the original binary's addresses.

#include "stdafx.h"

// ── Named globals: defined in WinMain.cpp, only extern-declared here ─────────
// int g_GameState, HWND g_hWnd, HINSTANCE g_hInst, HDC g_hDC — see WinMain.cpp

// ── Low-address constants ─────────────────────────────────────────────────────
float    _DAT_00000010 = 0.0f;
float    _DAT_00000014 = 0.0f;
float    _DAT_00000018 = 0.0f;
float    _DAT_0000001c = 0.0f;
float    _DAT_00000020 = 0.0f;
float    _DAT_00000024 = 0.0f;
DWORD    DAT_00000010  = 0;
DWORD    DAT_00000014  = 0;
DWORD    DAT_00000018  = 0;
DWORD    DAT_0000001c  = 0;
DWORD    DAT_00000020  = 0;
DWORD    DAT_00000024  = 0;

// ── Game math / render constants ─────────────────────────────────────────────
float    _DAT_00552464 = 25.0f;
float    _DAT_0055246c = 395.0f;
float    _DAT_00552488 = 10.0f;
float    _DAT_005524bc = 0.004f;
float    _DAT_005524f0 = 100.0f;  // grid-to-world scale = TERRAIN_SCALE
DWORD    DAT_005524f0  = 0;
float    _DAT_005524f4 = 0.1f;
DWORD    DAT_005524f4  = 0;
float    _DAT_005524f8 = 0.01f;
DWORD    DAT_005524f8  = 0;
float    _DAT_005524fc = 20.0f;
DWORD    DAT_005524fc  = 0;
float    _DAT_00552500 = 0.001f;
DWORD    DAT_00552500  = 0;
float    _DAT_00552504 = 0.5f;
DWORD    DAT_00552504  = 0;
float    _DAT_00552530 = 0.8f;
DWORD    DAT_00552530  = 0;
float    _DAT_00552534 = 0.6f;
DWORD    DAT_00552534  = 0;
float    _DAT_00552538 = 0.399999976f;
DWORD    DAT_00552538  = 0;
float    _DAT_00552540 = 3.0f;
float    _DAT_005527d4 = 19.0f;   // Party HP bar row height (SecondPassword_HoverCheck)
DWORD    DAT_00552540  = 0;
float    _DAT_00552560 = 0.0f;
DWORD    DAT_00552560  = 0;
float    _DAT_0055256c = 1.0f;
DWORD    DAT_0055256c  = 0;
DWORD    DAT_00552580  = 0;
float    _DAT_00552594 = 0.01f;
DWORD    DAT_00552594  = 0;
float    _DAT_00552598 = 50.0f;
DWORD    DAT_00552598  = 0;
float    _DAT_00552660 = 5.0f;
DWORD    DAT_00552660  = 0;
float    _DAT_00552664 = 12.0f;
DWORD    DAT_00552664  = 0;
float    _DAT_005526dc = 0.0625f;
DWORD    DAT_005526dc  = 0;
float    _DAT_005526e0 = 16.0f;
DWORD    DAT_005526e0  = 0;
float    _DAT_005526e4 = 0.2f;
DWORD    DAT_005526e4  = 0;
float    _DAT_005526e8 = 0.9f;
DWORD    DAT_005526e8  = 0;
float    _DAT_00552834 = 15.0f;
DWORD    DAT_00552834  = 0;
// _DAT_00552838 = 1/480 (Y scale), _DAT_0055283c = 1/640 (X scale) — compile-time constants.
// ConvertX(x) = WindowWidth * x * _DAT_0055283c ; ConvertY(y) = WindowHeight * y * _DAT_00552838
// Used by FUN_005125a0 (RenderBitmap), FUN_00511950/80 (ConvertX/Y), Camera_SetupFrustum, etc.
float    _DAT_00552838 = 1.0f / 480.0f;
DWORD    DAT_00552838  = 0x3B088889;   // 1/480 as DWORD bits
float    _DAT_0055283c = 1.0f / 640.0f;
DWORD    DAT_0055283c  = 0x3ACCCCCD;   // 1/640 as DWORD bits (float bits)
DWORD    DAT_00552844  = 0;
DWORD    DAT_00552848  = 0x42B40000;  // 90.0f as DWORD bits
float    _DAT_00552848 = 90.0f;
float    _DAT_0055284c = 30.0f;
DWORD    DAT_0055284c  = 0;
DWORD    DAT_00552850  = 0;
float    _DAT_00552868 = 0.0001f;  // epsilon for near-zero float checks
DWORD    DAT_00552868  = 0x38D1B717;  // 0.0001f as DWORD bits
DWORD    DAT_0055286c  = 0x43B40000;  // 360.0f as DWORD bits
DWORD    DAT_00552874  = 0;
float   _DAT_0055287c  = 0.5625f;
DWORD    DAT_00552880  = 0;
float   _DAT_00552884  = 0.8671875f;
float   _DAT_00552888  = 0.0234375f;
float   _DAT_0055288c  = 0.2265625f;
float   _DAT_00552890  = 0.001f;
float    _DAT_005528b0 = 0.0174532942f;
DWORD    DAT_005528b0  = 0;
float    _DAT_005528b4 = 0.4f;
DWORD    DAT_005528b4  = 0;
float    _DAT_005528b8 = 0.3f;
DWORD    DAT_005528b8  = 0;
DWORD    DAT_005528dc  = 0;
float    _DAT_005528e0 = 0.002f;
DWORD    DAT_005528e0  = 0;
float    _DAT_005528f0 = 1.5f;
DWORD    DAT_005528f0  = 0;
float    _DAT_00552908 = 120.0f;
float    _DAT_005528fc = 110.0f;  // MoveJoint Z-offset constant
DWORD    DAT_00552908  = 0;
float    _DAT_0055290c = 60.0f;
DWORD    DAT_0055290c  = 0;
float    _DAT_00552910 = 0.03f;
DWORD    DAT_00552910  = 0;
float    _DAT_0055291c = -0.0005f;
DWORD    DAT_0055291c  = 0;
float    _DAT_00552920 = -0.0001f;
DWORD    DAT_00552920  = 0;
float    _DAT_00552928 = 0.7f;
DWORD    DAT_00552928  = 0;
float    _DAT_0055293c = 135.0f;
DWORD    DAT_0055293c  = 0;
float    _DAT_00552934 = 0.15f;
DWORD    DAT_00552934  = 0;
float    _DAT_00552974 = 190.0f;
DWORD    DAT_00552974  = 0;
float    _DAT_0055297c = 150.0f;
DWORD    DAT_0055297c  = 0;
float    _DAT_005529bc = 128.0f;
DWORD    DAT_005529bc  = 0;
float    _DAT_005529c0 = -0.3f;
DWORD    DAT_005529c0  = 0;
float    _DAT_005529e0 = 260.0f;
DWORD    DAT_005529e0  = 0;
float    _DAT_00552a00 = 0.1f;
DWORD    DAT_00552a00  = 0;
float    _DAT_00552a10 = 0.05f;
DWORD    DAT_00552a10  = 0;
DWORD    DAT_00552a1c  = 0;
float    _DAT_00552a48 = 0.0666666701f;
DWORD    DAT_00552a48  = 0;
float    _DAT_00552abc = 0.005f;
DWORD    DAT_00552abc  = 0;
float    _DAT_00552ac0 = 0.008f;
DWORD    DAT_00552ac0  = 0;
float    _DAT_00552adc = 0.75f;
DWORD    DAT_00552adc  = 0;
DWORD    DAT_00552b70  = 0;
float   _DAT_00552b6c  = 3600.0f;
float    _DAT_00552b7c = 0.00390625f;
DWORD    DAT_00552b7c  = 0;
float    _DAT_00552b88 = 0.0002f;
DWORD    DAT_00552b88  = 0;
float    _DAT_00552c00 = 0.45f;          // item spin speed
float    _DAT_00552c14 = 115.0f;
DWORD    DAT_00552c14  = 0;
float    _DAT_00552c24 = 285.0f;
DWORD    DAT_00552c24  = 0;
float    _DAT_00552ca4 = 350.0f;
DWORD    DAT_00552ca4  = 0;
float    _DAT_00552ca8 = 175.0f;
DWORD    DAT_00552ca8  = 0;
float    _DAT_00552cac = 245.0f;
DWORD    DAT_00552cac  = 0;
DWORD    DAT_00552cb0  = 0;
float    _DAT_00552cc4 = 0.0087266462f;  // 0.5*PI/180 — verificado bit-pattern 0x3C0EFA33 en binario original. Se usa para tan(FOV/2) en gluPerspective2 (FUN_00511220) y frustum (Camera_SetupFrustum). Antes era PI/180 (full) → PerspX/Y 1.92× más grandes → proyecciones name-labels clusterizadas al centro.
DWORD    DAT_00552cc4  = 0;
float    _DAT_00552d08 = 0.32f;
DWORD    DAT_00552d08  = 0;
float    _DAT_00552d0c = 0.588235259f;
DWORD    DAT_00552d0c  = 0;
// World → object-bucket grid factor (1/1600; celda = 1600 unidades de mundo)
float    _DAT_00552d20 = 6.25e-4f;   // 0.000625 = 1/1600
DWORD    DAT_00552d20  = 0;
float    _DAT_00552d38 = 97.5f;
DWORD    DAT_00552d38  = 0;
float    _DAT_00552d3c = 0.375f;
DWORD    DAT_00552d3c  = 0;
float    _DAT_00552d48 = 248.0f;
DWORD    DAT_00552d48  = 0;
float    _DAT_00552d4c = 14.0f;
DWORD    DAT_00552d4c  = 0;
DWORD    DAT_005538a0  = 0;

// ── Entity / render constants ─────────────────────────────────────────────────
// 16-byte XOR key table — usado por Crypto.cpp/Effect_Create.cpp/Net_PacketSession etc.
// Indexado como (&DAT_00559050)[i&0xf] o DAT_00559050[i%16].
BYTE     DAT_00559050[16] = {0};
float    _DAT_00559070 = 400.0f;  // Verlet physics damping/gravity scalar
DWORD    DAT_00559070  = 0;
// DAT_005590ac = g_bUseChatListBox. Default IDA = 1 (verificado: bytes en 0x5590ac
// = 01 00 00 00, seguidos de flt_5590B0/B4/B8 = 295/417/18 coords del input dialog).
// FIX 2026-07-19: estaba en 0 (una sesión previa lo bajó para tapar un doble-render
// que en realidad se resuelve con el skip de mode 1/2 en ChatLB_renderLine). Con =1,
// sub_480980 corre in-world → mensajes de sistema/GM salen ARRIBA-IZQUIERDA (no en el
// área del ChatListBox abajo). Fiel a IDA.
DWORD    DAT_005590ac  = 1;
// flt_5590B0 / flt_5590B4 / flt_5590B8 — layout de los 3 botones popup del
// ChatListBox (ver-chat / tamaño-historial / transparencia).
// Valores LEÍDOS DEL BINARIO en 0x5590B0 (12 bytes: 00 80 93 43 | 00 80 d0 43 |
// 00 00 90 41) → 295.0f, 417.0f, 18.0f.  Los usan sub_40E400 (hit-test, slot 26)
// y sub_40DEF0 (render, slot 24 vía el thunk sub_40D600).
float    DAT_005590b0  = 295.0f;   // X del primer botón
float    DAT_005590b4  = 417.0f;   // Y de los tres
float    DAT_005590b8  =  18.0f;   // separación horizontal entre botones
// Version (5 bytes) @ 0x0055961c: obfuscated as Version[i]-i-1 in login packet.
//
// HISTORICAL VALUE from original main.exe (MD5 eb95ac0785e40a7ad60c9ddb5d8bef34):
//   { 0x31, 0x3B, 0x3A, 0x35, 0x35 }  // unobfuscates to "09710"
//
// In the original distribution model the InfoEncoder.exe tool re-patches
// main.exe at this offset to match whatever ClientVersion is set in
// Encoder/MainInfo.ini.  Since we now build from source, we bake the value
// directly here.  The current MuEmu reference server
// (MuServer/GameServer/DATA/GameServerInfo - StartUp.dat: ServerVersion=0.97.11)
// expects "09711"; bytes are computed as target_char + (i + 1) so that
// (DAT[i] - i - 1) = "09711":
//     '0'+1=0x31, '9'+2=0x3B, '7'+3=0x3A, '1'+4=0x35, '1'+5=0x36
BYTE     DAT_0055961c[5]  = { 0x31, 0x3B, 0x3A, 0x35, 0x36 };
// Serial (16 bytes) @ 0x00559624: sent raw in login packet.
//
// HISTORICAL VALUE from original main.exe:
//   "b9xGyHABOa9m2NHY"
//
// Current value matches MuServer Encoder/MainInfo.ini ClientSerial=TbYehR2hFUPBKgZj.
// Server compares strict equality after CSM decode, so client and server must
// agree on this 16-byte string verbatim.
BYTE     DAT_00559624[16] = { 'T', 'b', 'Y', 'e', 'h', 'R', '2', 'h',
                              'F', 'U', 'P', 'B', 'K', 'g', 'Z', 'j' };
DWORD    DAT_00559678  = 0;
float    _DAT_00559680 = 0.0f;
DWORD    DAT_00559680  = 0;
DWORD    DAT_00559684  = 0;
char     DAT_005597a0  = 'p';               // PathFinder debug string sentinel
DWORD    DAT_005597c4  = 0;
float    _DAT_005597c8 = 1.0f;
DWORD    DAT_005597c8  = 0;
DWORD    DAT_0055987c  = 0;
DWORD    DAT_005599b0  = 0;
DWORD    DAT_005599e0  = 0;
DWORD    DAT_00559bf0  = 0;
// DAT_00559c4c/50 are defined below in the named hover/targeting section.
DWORD    DAT_00559c78  = 0xffffffff;
// 0x00559C7C — IDA `SetTextColor_0`: color del PREFIJO (nombre de guild) en la
// composición de burbujas de chat (`sub_47F360`).  Lo escriben `RenderBoolean`
// (3 sitios, según el sub-modo del mensaje) y `RenderPartyHP`.  Se define acá
// con el nombre de IDA; `DAT_00559c7c` es un macro-alias en globals.h.
extern "C" DWORD SetTextColor_0 = 0xffffffff;
char     DAT_00559b80[64] = "";   // GM/admin name filter string (Entity_FindNearby)
DWORD    DAT_00559c80  = 0;
DWORD    DAT_00559c84  = 0;
DWORD    DAT_00559c88  = 0;
DWORD    DAT_00559c8c  = 0;
DWORD    DAT_00559c90  = 0;
// InputTextMax[] lives at 0x00559c94 in the original binary as a contiguous
// int array (stride 4). Our port accidentally split it into 4 independent
// globals so `(&_DAT_00559c94)[1]` pointed at arbitrary adjacent storage —
// writes to the password slot went to limbo and WM_CHAR always saw max=0,
// silently rejecting every keypress. Fix: back all four aliases into one
// int[2] array so pointer arithmetic over `_DAT_00559c94` indexes the real
// array.
// 2026-05-04: cambio float& → int& en `_DAT_00559c94/c98`. Antes el alias
// era `float&`, así que `_DAT_00559c94 = 42` guardaba el bit pattern del
// FLOAT 42.0f (= 0x42280000 = 1109917696). El WM_CHAR / RenderInputText leen
// como int → ven 1109917696, fallan o caen al fallback maxLen=10.
// IDA original: `int InputTextMax[8]` — siempre se trata como int.
int      _InputTextMaxArr[8] = {0,0,0,0,0,0,0,0};
int&     _DAT_00559c94 = _InputTextMaxArr[0];
DWORD&   DAT_00559c94  = reinterpret_cast<DWORD&>(_InputTextMaxArr[0]);
int&     _DAT_00559c98 = _InputTextMaxArr[1];
DWORD&   DAT_00559c98  = reinterpret_cast<DWORD&>(_InputTextMaxArr[1]);
DWORD    DAT_00559cc4  = 0;
DWORD    DAT_00559ccc  = 0;
DWORD    DAT_00559cd0  = 0;
DWORD    DAT_00559cd4  = 0;
DWORD    DAT_00559cd8  = 0;
DWORD    DAT_00559d74  = 0;
DWORD    DAT_00559ef0  = 0;
DWORD    DAT_00559ef4  = 0;
DWORD    DAT_00559ef8  = 0;
char     DAT_00559f5f  = 0;
DWORD    DAT_0055a3e4  = 0xffffffff;  // hovered-skill index (-1 = none); IDA inits to -1
DWORD    DAT_0055a3e8[4] = {0};       // chaos-mix info (ReceiveTalk sub 3 copia 4 bytes acá; 0x55A3E8..0x55A3F4)
int      EventType     = 0;           // 0/1 tipo de EventWindow (ReceiveTalk sub 4/6)
int      g_NpcTalkActive = 0;         // 1 = mandamos un talk 0x30 y el server puede tener Interface.use=1 (shop/GoldenArcher/etc). Se limpia al mandar el close 0x31.
extern "C" { int g_bServerDivisionEnable = 0; int g_bServerDivisionAccept = 0; }  // ReceiveTalk sub 5
DWORD    DAT_0055a774  = 0;
DWORD    DAT_0055a778  = 0;
DWORD    DAT_0055a77c  = 0;
DWORD    DAT_0055a780  = 0;
char     s__s_file_not_found__0055a784[] = "%s file not found";
DWORD    DAT_0055a798  = 0;
char     DAT_0055a79c[] = "Data\\";    // texture/asset path prefix ("Data mode")
char     DAT_0055a7a4[] = "Data2\\";   // texture/asset path prefix ("Data2 / pak mode")
int      DAT_0055a7ac  = 0;
float    _DAT_0055a7c0 = 0.0f;
DWORD    DAT_0055a7c0  = 0;

// ── Game loop / scene state ───────────────────────────────────────────────────
// Static camera defaults from PE .data @ 0x0056154c:
//   CameraViewNear = 20.0f  (0x0056154c)
//   CameraViewFar  = 2000.0f (0x00561550)
//   CameraFOV      = 55.0f  (0x00561554)
// These are DWORDs in the variable table but the binary reads them as floats.
DWORD    DAT_0056154c  = 0x41A00000;  // 20.0f    CameraViewNear
DWORD    DAT_00561550  = 0x44FA0000;  // 2000.0f  CameraViewFar
DWORD    DAT_00561554  = 0x425C0000;  // 55.0f    CameraFOV (MoveMainCamera lo reescribe a 35.0)
float    DAT_005616b4  = 0.0f;        // CameraDistanceTarget (MoveMainCamera)
float    DAT_083a45d0  = 0.0f;        // CameraDistance (MoveMainCamera)
DWORD    DAT_0056156c  = 640;   // WindowWidth default
DWORD    DAT_00561570  = 480;   // WindowHeight default
DWORD    DAT_00561574  = 0;
// Buffer for server IP (writable — Config_ReadServerAddr fills it from server.cfg).
// DAT_005615b8 puntua a este buffer por defecto; si server.cfg existe, se sobreescribe
// con la IP del ConnectServer local.
char     g_ServerIPBuf[128] = "connect.muonline.co.kr";
char    *DAT_005615b8  = g_ServerIPBuf;
WORD     DAT_005615bc  = 55901;  // default MU port

// ── ConnectServer flow (2026-07-15) ──────────────────────────────────────────
// Cuando server.cfg tiene 2 líneas: línea 1 = ConnectServer (DAT_005615b8/bc),
// línea 2 = GameServer fallback (g_GameServerIP/Port). g_HasConnectServer activa
// el flujo original: conectar al CS → recibir lista+load (F4/04/F4/02) → al
// elegir server mandar F4/03 → redirect al GameServer → login.
int             g_HasConnectServer      = 0;  // server.cfg tiene 2 líneas
int             g_ConnectServerMode     = 0;  // 1 = socket actual habla con el CS
int             g_ConnectServerRequested = 0; // 1 = ya mandamos C1 04 F4 02 en esta conexión CS
char            g_GameServerIP[128]     = ""; // GameServer fallback (server.cfg línea 2)
unsigned short  g_GameServerPort        = 0;
// DAT_005615c0 = g_GameState (above)
// g_lpszMp3 @ 0x005615C4 — tabla de 6 punteros a las rutas de los BGM.
// NO son handles: son `char*`. Los DAT_005615c4..d8 son sus 6 elementos (cuarto
// caso del patron "DAT_ vecinos = una sola tabla"). Los consumen Game_MainLoop
// (seleccion de BGM por mapa) y StopMusic (0x513420), que recorre la tabla con
// bound `< 0x5615DC` = &g_lpszMp3[6].
//
// En nuestro build valian 0, asi que PlayMp3 recibia NULL y nunca sonaba nada.
//
// DESVIACION DELIBERADA: el binario apunta a nombres coreanos
// (data\music\ÁÖÁ¡.mp3 = taberna, ¹ÂÅ×¸¶ = MuTheme, ¼º´ç = catedral,
//  µ¥ºñ¾Æ½º = Devias, ³ë¸®¾Æ = Noria, ´øÁ¯ = Dungeon) que NO existen en
// bin/Client/Data/Music — nuestro pack de assets viene renombrado al ingles.
// Se apunta a los archivos reales. El mapeo de MuTheme -> Lorencia.mp3 lo
// confirma el DLL de inyeccion (Encoder/MapManager.txt: "THE LOGIN MUSIC IS
// Data\Music\MuTheme.mp3") y encaja con que Game_MainLoop use este mismo slot
// para el login (g_GameState == 2) y para Lorencia.
//
// ASSET FALTANTE: el track de la catedral de Devias (¼º´ç) no vino en el pack.
// Se deja el nombre apuntando a un archivo inexistente a proposito: PlayMp3
// hace fopen y sale sin tocar nada si falla, asi que dentro de la catedral
// sigue sonando Devias.mp3 sin corte. Si algun dia aparece el asset, funciona
// solo con dejarlo en Data\Music\Cathedral.mp3.
char*    g_lpszMp3[6] = {
    (char*)"Data\\Music\\Pub.mp3",        // [0] 0x5615C4 — taberna de Lorencia (HeroTile == 4)
    (char*)"Data\\Music\\Lorencia.mp3",   // [1] 0x5615C8 — MuTheme: login + Lorencia
    (char*)"Data\\Music\\Cathedral.mp3",  // [2] 0x5615CC — catedral de Devias  [FALTA EL ASSET]
    (char*)"Data\\Music\\Devias.mp3",     // [3] 0x5615D0 — Devias
    (char*)"Data\\Music\\Noria.mp3",      // [4] 0x5615D4 — Noria
    (char*)"Data\\Music\\Dungeon.mp3",    // [5] 0x5615D8 — Dungeon (World 1 y 5)
};
DWORD    DAT_005615e0  = 0;
DWORD    DAT_005615e8  = 0;

// ──────────────────────────────────────────────────────────────────────────
//  CameraWalk[6][6] — static waypoint table from PE @ 0x005615ec..0x0056167b
//  Each waypoint is 6 floats: pos[3] + angle[3] (degrees).
//  Read by MoveCamera (0x0051E4E0) + Game_SceneUpdate login init.
//  MUST BE CONTIGUOUS — MoveCamera indexes CameraWalk[i*6+k] off the base
//  pointer. Defining the six DAT_ names at f0/f4/f8/fc/00/... as separate
//  DWORDs lets MSVC scatter them → table read returns garbage and the
//  login-scene camera fly-through never activates.
//  Values recovered from `main.exe` (MD5 eb95ac0...) .data section.
// ──────────────────────────────────────────────────────────────────────────
float CameraWalk_005615ec[42] = {
    //  pos.x     pos.y     pos.z     ang.pitch  ang.yaw  ang.roll
    //  wp0: distant frontal
        0.0f,   -1000.0f,   500.0f,   -80.0f,    0.0f,     0.0f,
    //  wp1..wp4: closer frontal
        0.0f,   -1100.0f,   500.0f,   -80.0f,    0.0f,     0.0f,
        0.0f,   -1100.0f,   500.0f,   -80.0f,    0.0f,     0.0f,
        0.0f,   -1100.0f,   500.0f,   -80.0f,    0.0f,     0.0f,
        0.0f,   -1100.0f,   500.0f,   -80.0f,    0.0f,     0.0f,
    //  wp5: angled side (used by Scene_EnterWorld camera transition)
      200.0f,    -800.0f,   300.0f,   -80.0f,    0.0f,   -10.0f,
    //  wp6: angled side end (PE @0x0016167C) — completes the transition pan
      200.0f,    -900.0f,   300.0f,   -80.0f,    0.0f,   -10.0f,
};
// Legacy DAT_ names are references into the contiguous table.
DWORD& DAT_005615ec = *(DWORD*)&CameraWalk_005615ec[0];   // wp0.pos.x
DWORD& DAT_005615f0 = *(DWORD*)&CameraWalk_005615ec[1];   // wp0.pos.y
DWORD& DAT_005615f4 = *(DWORD*)&CameraWalk_005615ec[2];   // wp0.pos.z
DWORD& DAT_005615f8 = *(DWORD*)&CameraWalk_005615ec[3];   // wp0.ang.pitch
DWORD& DAT_005615fc = *(DWORD*)&CameraWalk_005615ec[4];   // wp0.ang.yaw
DWORD& DAT_00561600 = *(DWORD*)&CameraWalk_005615ec[5];   // wp0.ang.roll
DWORD& DAT_00561664 = *(DWORD*)&CameraWalk_005615ec[30];  // wp5.pos.x
DWORD& DAT_00561668 = *(DWORD*)&CameraWalk_005615ec[31];  // wp5.pos.y
DWORD& DAT_0056166c = *(DWORD*)&CameraWalk_005615ec[32];  // wp5.pos.z
DWORD& DAT_00561670 = *(DWORD*)&CameraWalk_005615ec[33];  // wp5.ang.pitch
DWORD& DAT_00561674 = *(DWORD*)&CameraWalk_005615ec[34];  // wp5.ang.yaw
DWORD& DAT_00561678 = *(DWORD*)&CameraWalk_005615ec[35];  // wp5.ang.roll
DWORD    DAT_00561694  = 0;
DWORD    DAT_00561698  = 0;
DWORD    DAT_005616a0  = 0;
// dialog Y / camera X — usados como SIGNED int en las animaciones de
// Game_SceneUpdate y Game_EnterWorldTick.  Si los declarás DWORD, la
// fórmula divide-by-3 (`(int)((long long)x * 0x55555555LL) >> 32`)
// overflowea con valores negativos → cX saltaba a 0x2AAAAAA4 → state
// machine oscilaba 0x18 ↔ 0x17 sin parar.
int      DAT_005616a4  = 0;
int      DAT_005616a8  = 0;
DWORD    DAT_005616ac  = 0;
DWORD    DAT_005616b0  = 0;
DWORD    DAT_005616b8  = 0;
DWORD    DAT_005617a0  = 0;
char     DAT_00561a30[]  = "%s";  // Scene_CharSelect — name label (top, bold)
DWORD    DAT_00561b04  = 0;
char     DAT_00561b70[8] = "OZJ";   // FUN_00529740 extension suffix (Data mode: .jpg→.OZJ)

// String literals (read-only — actual game strings come from the binary)
char     s_Local_Webzenlogo_jpg_00561774[]  = "Local/Webzenlogo.jpg";
char     s_Local_Everyone_jpg_0056178c[]    = "Local/Everyone.jpg";
char     s_Local_Loading01_jpg_00561a88[]   = "Local/Loading01.jpg";
char     s_Local_Loading02_jpg_00561a9c[]   = "Local/Loading02.jpg";
char     s_Local_Loading03_jpg_00561ab0[]   = "Local/Loading03.jpg";
char     s_connect_muonline_co_kr_005615b8[] = "connect.muonline.co.kr";
char     s_Dialog_005595e0[]                = "Dialog";
char     s_Hash_table_full______GetIndex_00558108[] = "Hash table full!!! GetIndex";
char     s_Hash_table_full______Insert_005580e8[] = "Hash table full!!! Insert";
char     s_Macro_Time_00559f30[]            = "Macro_Time";
char     s___2d_00559f44[]                  = " - ";
char     s___2d___00559f3c[]                = " -  ";
char     s_____2d_00559f4c[]                = "  - ";
char     s__d__d_00559f00[]                 = "%d/%d";
char     s__d__d_00559f1c[]                 = "%d / %d";
char     s__s__d_00561a34[]                 = "%s %d";
// Scene_CharSelect "account-blocked" warning lines. La tabla original de
// strings del 0.97k está stripped (ambos quedan todo-ceros en el PE), así que
// fijamos literales razonables en inglés para que el render muestre algo.
char     lpString_07d49c14[128]             = "Account is blocked.";
char     lpString_07d49d40[128]             = "Please contact the operator.";
char     lpString_00561a3c[64]              = {};  // server info line 1
char     lpString_00561a58[64]              = {};  // server info line 2
char     lpString_00561a68[64]              = {};  // server info line 3
char     s__s__d_Non_PVP___s_0056192c[]     = "%s%d Non PVP %s";
char     s__s__d_Non_PVP___s_00561940[]     = "%s%d Non PVP %s";
char     s__s__d_Non_PVP___s_00561954[]     = "%s%d Non PVP %s";
char     s__s__d__s_0056192c[]              = "%s%d %s";
char     s__s__d__s_00561940[]              = "%s%d %s";
char     s__s__d__s_00561954[]              = "%s%d %s";
char     s__s__d__s_00561968[]              = "%s%d %s";
char     s__s__d__s_00561974[]              = "%s%d %s";
char     s__s__d__s_00561980[]              = "%s%d %s";
char     s__s__s_00561914[]                 = "%s%s";
char     s__s__s_0056191c[]                 = "%s%s";
char     s__s__s_00561924[]                 = "%s%s";

// ── Misc game globals ─────────────────────────────────────────────────────────
// DAT_00583d8c es un alias de g_csQuest (ver globals.h).
DWORD    DAT_00583dac  = 0;
DWORD    DAT_00585e7c  = 0;
DWORD    DAT_0058c780  = 0;
DWORD    DAT_0058e1c4  = 0;
DWORD    DAT_0058e854  = 0;
DWORD    DAT_0058eee4  = 0;
DWORD    DAT_00590924  = 0;
// DAT_00590ac8, DAT_00590ac9, DAT_00590acc, DAT_00590ad0, DAT_00590ad8 —
// now macro aliases for g_EnableSound / g_Enable3DSound / SoundLoadCount /
// g_lpDS / wavefile respectively. See globals.h.

// ── Engine / Window handles ───────────────────────────────────────────────────
DWORD    DAT_055c9b40  = 0;   // g_EnableSound
DWORD    DAT_055c9b60  = 0;   // sound channel index offset
DWORD    DAT_055c9b70  = 0;
// _DAT_055c9b70 defined in Config_Load.cpp
DWORD    DAT_055c9b74  = 0;
// _DAT_055c9b74 = g_fScreenRate_y (overlaps DAT_055c9b74 as float)
DWORD    DAT_055c9b80  = 0;
// DAT_055c9bac defined in Config_Load.cpp as char[12]
// HashTable obfuscation. Original binary has a real hash-table object at
// 0x055c9bc8..0x055c9bd4 (contiguous). 40+ inlined callers deref the vtable
// at offset +0xC, and insert/lookup helpers read capacity at offset +0xC from
// the CONTEXT (&DAT_055c9bc8 + 0xC = DAT_055c9bd4).
//
// 2026-05-03: SAFE SENTINEL SLOT strategy. Previously capacity=0 caused the
// bd4-checks in caller sites to early-exit; sites that lacked the guard fell
// through to `puVar = NULL → *(NULL + 0x161)` AVs.
//
// New strategy: capacity = 1, hash function returns 0 (always slot 0), slot 0
// stores (key=0, value=&g_HashSentinelNode). g_HashSentinelNode is a 0x584-byte
// buffer that absorbs ALL anti-tamper ref-counts and XOR encryption writes:
//   - cVar5 = sentinel[0x161]  → reads byte at offset 353 of the buffer (valid)
//   - sentinel[0x161]++/-- → writes byte 353 of the buffer (valid)
//   - 0x584-byte XOR pass over sentinel → overwrites the buffer with garbage,
//     which is fine because nothing else reads it
// Real game data structures never get hit by anti-tamper writes because the
// hash never holds keys for them.
//
// The vtable function at offset +0xC is the hash function (called as
// `(*hash)(key)`). Returns 0 = slot index of the sentinel. Internal callers
// that loop over slots use this index safely (slot 0 has key=0 + value=
// sentinel, so the key-mismatch path drops out via the iteration limit
// without indexing out of bounds).
//
// External callers using HashTable_GetIndex (functions.h wrapper) get
// 0xFFFFFFFF instead, so their `if (idx != -1)` guard skips the subsequent
// FUN_00404280 lookup + NULL deref.
static unsigned int __cdecl HashFn_Sentinel(void*) { return 0; }
static void* g_FakeHashVtable[8] = {
    nullptr, nullptr, nullptr,
    (void*)HashFn_Sentinel,              // slot 3 / byte offset 0xC
    nullptr, nullptr, nullptr, nullptr
};

// 0x584-byte sentinel buffer. Anti-tamper paths read/write [+0x161] and the
// XOR pass mutates the entire buffer. Aligned to 4 so DWORD reads are clean.
__declspec(align(4)) static unsigned char g_HashSentinelNode[0x584] = {0};

// Single-slot hash arrays. Both point into static storage so we don't need
// to allocate. Slot 0 of g_HashValueArr is initialized to &sentinel; slot 0
// of g_HashKeyArr is left at 0 (matches any "key=0" lookup; for key!=0 the
// caller's key-check fails and the iteration limit drops out).
static DWORD g_HashValueArr[1] = { 0 };  // values (filled by ctor below)
static DWORD g_HashKeyArr  [1] = { 0 };  // keys

// Buffer layout (original binary): +0 vtable, +4 values, +8 keys, +0xC capacity.
DWORD    g_HashTableCtx[4] = { 0, 0, 0, 0 };
struct HashCtxInit_t {
    HashCtxInit_t() {
        g_HashValueArr[0]   = (DWORD)g_HashSentinelNode;
        g_HashTableCtx[0] = (DWORD)g_FakeHashVtable;        // vtable
        g_HashTableCtx[1] = (DWORD)g_HashValueArr;          // value array
        g_HashTableCtx[2] = (DWORD)g_HashKeyArr;            // key array
        g_HashTableCtx[3] = 1;                               // capacity = 1
    }
} g_HashCtxInitObj;
DWORD    DAT_055c9be0  = 0;
DWORD    DAT_055c9be4  = 0;
float    _DAT_055c9be4 = 0.0f;
DWORD    DAT_055c9be8  = 0;
DWORD    DAT_055c9bec  = 0;
DWORD    DAT_055c9bf0  = 0;
DWORD    DAT_055c9d00  = 0;
DWORD    DAT_055c9e04  = 0;
DWORD    DAT_055c9e44  = 0;
DWORD    DAT_055c9e48  = 0;
int      DAT_055c9e58[100] = {};   // RandomTable — lo siembra WinMain
DWORD    DAT_055c9ff0  = 0;  // HGLRC
DWORD    DAT_055c9ff4  = 0;
DWORD    DAT_055c9ff8  = 0;
// DAT_055c9ffc = g_hWnd (above)
// Font memory DC (GDI-only, DIB-backed). Set by Font_BuildLayout.
// NOT the window DC — that is g_hDC (DAT_055ca004) defined in WinMain.cpp.
HDC      DAT_055c9fec  = NULL;
// DAT_055ca000 = g_hInst (above)
// DAT_055ca004 = g_hDC — defined via macro in stdafx.h (no separate storage)
DWORD    DAT_055ca008  = 0;
DWORD    DAT_055ca00c  = 0;
DWORD    DAT_055ca010  = 0;
DWORD    DAT_055ca014  = 0;
DWORD    DAT_055ca018  = 0;
char     DAT_055ca019  = 0;
DWORD    DAT_055ca01c  = 0;
DWORD    DAT_055ca020  = 0;
DWORD    DAT_055ca028  = 0;
char     DAT_055ca031  = 0;
DWORD    DAT_055ca034  = 0;
DWORD    DAT_055ca038  = 0;
DWORD    DAT_055ca03c  = 0;
DWORD    DAT_055ca040  = 0;
DWORD    DAT_055ca050  = 0;
// Socket context struct — contiguous buffer at 0x055ca160 in original binary.
// Layout: +0 vtable/flags, +4 g_bGameServerConnected, +8 SOCKET,
//         +0xC send_buf[0x2000], +0x200C send_len, +0x2010 recv_buf[0x2000],
//         +0x4010 recv_index, +0x4014 dispatch_flag, +0x4018 padding,
//         +0x401C onwards: 300 packet slots × 0x2008 bytes each
//                          (slot = 4-byte flag + 4-byte len + 0x2000 data)
//         Total size = 0x401C + 300 * 0x2008 = 0x25BF04, round up to 0x260000.
// IDA confirms 300 slots / stride 0x2008 in sub_43DF90 + CWsctlc::GetReadMsg.
// Callers pass the literal 0x55ca160 as pointer — now redirected via macro in globals.h.
//
// BUG fixed: previously sized 0x4030 — slot 0 data area (offset 0x4024..0x6024)
// extended PAST the array by ~0x1FF4 bytes, so any packet >12 bytes scribbled
// into adjacent globals. DAT_05826c58 (Dec2 keys) was placed by linker right
// after DAT_055ca160's end, so every C3 packet trashed the decryption keys
// → C3 decode FAILED with checksum mismatch on every server response.
char     DAT_055ca160[0x260000] = {};
// Static init: socket field must start as INVALID_SOCKET (0xFFFFFFFF)
struct NetCtxInit_t { NetCtxInit_t() { *(SOCKET*)(DAT_055ca160 + 8) = INVALID_SOCKET; } } g_NetCtxInitObj;
DWORD    DAT_055cc16c  = 0;
DWORD    DAT_055ce174  = 0;

// ── Network / login state ─────────────────────────────────────────────────────
DWORD    DAT_05826bdc  = 0;
DWORD    DAT_05826c00  = 0;
DWORD    DAT_05826c04  = 0;
DWORD    DAT_05826c08  = 0;
// CSimpleModulus XOR key table @ 0x00562E48 (.rdata in the original binary).
// Used by CSimpleModulus_LoadEncryptionKey / LoadDecryptionKey (sub_53D1C0)
// to de-obfuscate the key DWORDs read from Enc1.dat / Dec2.dat.
// Extracted from main.exe rdata segment (MD5 eb95ac0785e40a7ad60c9ddb5d8bef34).
DWORD    DAT_00562e48[4] = {
    0x3F08A79B, 0xE25CC287, 0x93D27AB9, 0x20DEA7BF
};

// CSimpleModulus objects — 17 DWORDs each (68 bytes):
//   [0]      : vtable/header (unused by crypto math)
//   [1..4]   : ModKey[4]      (this+4  .. this+19)
//   [5..8]   : EncKey[4]      (this+20 .. this+35)
//   [9..12]  : DecKey[4]      (this+36 .. this+51)
//   [13..16] : XorKey[4]      (this+52 .. this+67)
// Loaded at WinMain startup from Data\Enc1.dat (CS) and Data\Dec2.dat (SC).
DWORD    DAT_05826c10[17] = {0};   // g_SimpleModulusCS (client→server encryption keys)
DWORD    DAT_05826c58[17] = {0};   // g_SimpleModulusSC (server→client decryption keys)
DWORD    DAT_05826c9c  = 0;
DWORD    DAT_05826ca0  = 0;  // HeroIndex
DWORD    DAT_05826ca4  = 0;
DWORD    DAT_05826ca8  = 0;
DWORD    DAT_05826cac  = 0;
DWORD    DAT_05826cb0  = 0;
DWORD    DAT_05826cb4  = 0;
DWORD    DAT_05826cc0  = 0;
DWORD    DAT_05826cc8  = 0;
char     DAT_05826cc9  = 0;
char     DAT_05826cd4[16]  = {0};
char     DAT_05826ceb  = 0;
DWORD    DAT_05826cec  = 0;
float    _DAT_05826cf4 = 0.0f;
DWORD    DAT_05826cf4  = 0;
DWORD    DAT_05826cf8  = 0;
DWORD    DAT_05826cf0  = 0;  // g_bGameServerConnected
DWORD    DAT_05826d08  = 0;
int      DAT_05826d04  = 0;
char     DAT_05826d14  = 0;   // Teleport (IDA @0x05826D14) — flag de gate/teleport en curso
DWORD    DAT_05826d18  = 0;   // cooldown de COMPRA en tienda (IDA dword_5826D18)
DWORD    DAT_05826d1c  = 0;   // cooldown de equipar/usar item (EnableUse)
DWORD    DAT_05826d20  = 0;
DWORD    DAT_05826d24  = 0;   // SummonLife (IDA @0x05826D24) — HP % de la mascota invocada
DWORD    DAT_05826d30  = 0;
char     DAT_05826d31  = 0;
char     DAT_05826d32  = 0;
char     DAT_05826d33  = 0;
DWORD    DAT_05826d78  = 0;
DWORD    DAT_05826dc8  = 0;
DWORD    DAT_05826df4  = 0;
DWORD    DAT_05826e04  = 0;
int      DAT_05826e08  = 0;  // WorldTime / g_AnimTick
float    g_AttackEffectMatrix_04D[3][4] = {};
float    g_AttackEffectMatrix_04D_Alt[3][4] = {};
float    g_AttackEffectMatrix_04D_Aux[3][4] = {};
DWORD    DAT_05826e0c  = 0;
DWORD    DAT_05826e10  = 0;
// BoneQuaternion @ 0x05826E18 — scratch de cuaterniones por hueso que llena
// BMD_Animation (0x440060 L157-166: `(char *)&unk_5826E18 + 16 * boneIdx`).
// MAX_BONES = 200, igual que g_BoneScratch → 200 x 16 bytes.
// 2026-08-21: estaba declarado como UN SOLO DWORD y el port además hacía
// `&DAT_05826e18 + boneIdx * 0x10` sobre un DWORD*, o sea 64 bytes de paso en
// vez de 16.  Cada frame de animación pisaba ~12 KB de globals vecinos.  Lo que
// se rompía dependía de qué caía al lado en el layout de BSS: con el layout de
// esta rama caía justo sobre los flags de paneles (0x07EAA114..117 =
// ShopOpened/PartyOpened/.../InventoryOpened) y los menús se abrían solos.
char     DAT_05826e18[200 * 0x10] = {0};
DWORD    DAT_05828d58  = 0;  // Models
void*    DAT_06f42a58  = nullptr;  // model memory pool

// BMD bounding-box scratch arrays (FUN_00442e60)
// Tablas scratch de BMD_CreateBoundingBox (0x442E60), la unica funcion del
// binario que las toca.  Se recorren UNA ENTRADA POR HUESO hasta numBones
// (= *(short*)(model+34)): word_77D87FC[bone] es el contador de vertices y
// flt_5827A98 / flt_6F42A5C son el max/min del bbox, 3 floats por hueso.
// 2026-08-22: estaban declaradas como escalares sueltos (2 y 4 bytes), asi que
// desde el hueso 1 en adelante escribian sobre los globals vecinos en cada
// carga de modelo — o sea al entrar al juego y en cada cambio de mapa.
// El vecino inmediato de word_77D87FC era DAT_07db870c (el flag de la lista de
// skills), que por eso aparecia abierta sola.  MAX_BONES = 200, igual que
// g_BoneScratch / BoneTransform / BoneQuaternion.
short    DAT_077d87fc[200]    = {0};   // contador de vertices por hueso
float    DAT_05827a98[200*3]  = {0};   // bbox max, 3 floats por hueso
float    DAT_06f42a5c[200*3]  = {0};   // bbox min, 3 floats por hueso

// UI name-list panel data (FUN_0051e240)
// DAT_083a430c — ahora macro dentro de DAT_083a42f8 (dialog button rects)
// 2026-05-08: DAT_083a44c4 IS g_lpszMessageBoxCustom — a 7-line × 0x26 byte
// dialog/message buffer used by CreateOkMessageBox, RenderErrorMessage,
// CSQuest dialogs, SecondPassword UI, UI_StatsPanel, etc. Previously declared
// as a separate single DWORD (line 1328) which made the crt_sprintf and
// stride-0x26 line writes spill into adjacent globals (heap stomp). Now
// sized properly as char[7 * 0x26] = 266 bytes; DAT_083a44ea is exposed as
// a macro alias projecting to offset +0x26 (line[1]) inside the buffer.
// (See definition lower in the file alongside the other DAT_083a4* symbols.)
byte     DAT_005618b8  = 0;
byte     DAT_005618bc  = 0;
byte     DAT_005618c0  = 0;
byte     DAT_005618c4  = 0;
char     s____s___005618c8[] = " %s ";

// ── Bone / skeleton data ──────────────────────────────────────────────────────
// Bone-matrix scratch buffer. In the original binary this region (0x06970a9c..
// ~0x0697163c) is a contiguous pool where Sprite_Draw / Entity_Render_3D write
// interpolated bone matrices (stride 0x30 bytes = float[3][4] per bone). Ghidra
// labels DAT_06970XXX were placed where specific slot addresses are referenced.
// We back all of them with a single char buffer and expose each sibling as a
// DWORD lvalue at the correct offset via macros (see globals.h), so:
//   &DAT_06970a9c  → g_BoneScratch + 0x000   (DWORD* into buffer)
//   &DAT_06970acc  → g_BoneScratch + 0x030
//   …
// This keeps addresses contiguous and makes the dynamic indexing
//   (float*)((char*)&DAT_06970a9c + boneIdx * 0x30)
// in Entity_Render_3D.cpp work byte-accurately.
//
// BUG-FIX 2026-07-17 (CRÍTICO — corrupción del preview char): este buffer es el
// `BoneTransform[MAX_BONES][3][4]` del original (MU 5.2 ZzzBMD.h: MAX_BONES=200 →
// 200*0x30 = 0x2580 bytes). Estaba dimensionado 0x1000 = SOLO 85 huesos. Cualquier
// modelo con >85 huesos (personajes/monstruos grandes) desbordaba el buffer, y como
// en globals.cpp queda inmediatamente ANTES de DAT_07abf050 (el preview char de
// char-select), el desborde pisaba entity+0 (Live/type) del preview → type=16247 →
// crash al animar/renderizar. Confirmado por map: g_BoneScratch@0x98b1b0 +0x1000 =
// DAT_07abf050@0x98c1b0 exacto. Fix: dimensionar al MAX_BONES real (200).
char     g_BoneScratch[200 * 0x30] = {0};   // = 0x2580 (BoneTransform[200][3][4])

// ── Preview character entity (0x07abf050) ─────────────────────────────────────
// BUG-FIX 2026-07-17: buffer de entidad COMPLETO para el preview char de
// char-select. Antes era `DWORD DAT_07abf050 = 0` (4 bytes) y los campos
// _DAT_07abf05c…_DAT_07abf5cc estaban como globales sueltos → CreateCharacterPointer
// (escribe hasta +908) desbordaba sobre BSS adyacente y corrompía todo. Ahora un
// solo buffer de 0x580 (matching el spacing del original hasta el array 0x07abf5d0);
// los campos se acceden por macros (ver globals.h). Ver charselect-deferred-issues.
char     DAT_07abf050[0x580] = {0};
char     DAT_07d2b494[9000] = {};  // class name table (stride 300, 30 slots) — símbolo aparte
DWORD    DAT_07abf5d0  = 0;  // CharactersClient
int      DAT_07abf5d4  = 0;
char    *DAT_07abf5d8  = NULL;  // Hero
DWORD    DAT_07abf5dc  = 0;
DWORD    DAT_07abf5e0  = 0;
float    _DAT_07abf5e8 = 0.0f;
DWORD    DAT_07abf5e8  = 0;
// Particle pool — 3000 slots × 0x70 (112) bytes = 336000 bytes total.
// Original binary: 0x07abf5f0..0x07b11670 = 0x52080 bytes. /0x70 = 47999 slots.
// Pero MoveParticles_stub itera 3000 slots; usamos ese tamaño que ya existe en stubs.
// Antes era 1 byte → CreateParticle (FUN_00475220) tenía un overflow guard que
// retornaba 0 inmediatamente → NUNCA spawneaba lightning ELS=10/11, fire/smoke
// effects, weather particles, etc. — todo silenciado.
char     DAT_07abf5f0[3000 * 0x70] = {0};   // particle pool base
char     DAT_007abf06  = 0;
float    _DAT_007abf06 = 0.0f;

// ── Skill effects / player render pools ──────────────────────────────────────
DWORD    DAT_07b11698  = 0;
DWORD    DAT_07b116d0  = 0;
DWORD    DAT_07b27b08  = 0;
// BUG-FIX 2026-04-28: SkillEffect pool — 200 slots × 0x70 bytes (= 22400 bytes).
// Antes era DWORD; SkillEffect_Render iteraba más allá del símbolo → potencial AV
// (mitigado por AUTO-SKIP en SkillEffect_Render.cpp). Ahora con tamaño real
// podemos sacar el AUTO-SKIP y permitir el render real.
unsigned char DAT_07c5ab3c[200 * 0x70] = {};
// Joint/Trail/Blur shared render pool — 100 slots × 0x2f0 = 76800 bytes.
// IDA layout: each slot starts at offset 0; DAT_07c608b4 is the +12 "anchor"
// field within slot[0]. We back the entire pool here and project the anchor
// through a macro (see globals.h). Was declared as single DWORD = 0 → all
// joint / trail / blur loops walked random memory and crashed when any
// post-login non-zero byte was encountered.
char     g_RenderPool_07c608a8[100 * 0x2f0] = {};
DWORD    DAT_07c74ae0  = 0;
DWORD    DAT_07c74ae4  = 0;    // BGM track 1 enable flag
// Player render pool: 100 entries × 0x1BC bytes (= 444 bytes/slot).
// Original spans [0x07c74e68, 0x07c7fcc4). El walker v1 de Player_Render
// arranca en `&DAT_07c74f54` y lee offsets NEGATIVOS hasta -0xEC, accediendo
// a los primeros 0xEC bytes del slot. Por eso DAT_07c74f54 NO es el inicio
// real del pool — el inicio es 0xEC bytes antes (= DAT_07c74e68).
//
// 2026-05-07: re-allocated propiamente para que Player_Render funcione sin
// AUTO-SKIP. El pool real `g_PlayerRenderPool` cubre los 100 slots completos
// (incluyendo los 0xEC bytes de cabecera de cada slot que estaban antes de
// DAT_07c74f54). DAT_07c74f54 ahora es un alias dentro del array a offset
// 0xEC (= v1 anchor para slot 0).
char     g_PlayerRenderPool[100 * 0x1BC] = {};
DWORD*   DAT_07c74f54 = (DWORD*)(g_PlayerRenderPool + 0xEC);

// ── g_CharData XOR block ───────────────────────────────────────────────────────
void    *DAT_07cf1ffc  = nullptr;  // g_CharData pointer (0x584-byte XOR-encoded)

// ── UI / HUD data ─────────────────────────────────────────────────────────────
DWORD   _DAT_07e118e4  = 0;    // facing angle (float, movement packet)
DWORD    DAT_07e118e8  = 0;    // world/map type
DWORD    DAT_07e11d7c  = 0;    // countdown counter B
DWORD    DAT_07e11d8c  = 0;
DWORD    DAT_07e11d90  = 0;
DWORD    DAT_07e11d94  = 0;
DWORD    DAT_07e11d98  = 0;
DWORD    DAT_07e11da0  = 0;
DWORD    DAT_07e11db4  = 0;
DWORD    DAT_07e11db8  = 0;
int      DAT_07e11dbc  = 0;
char     DAT_07e11dc0  = 0;
DWORD    DAT_07e11dc4  = 0;   // NPC script: dialog-active flag (set 1 when dialog in progress)
DWORD    DAT_07e11dc8  = 0;   // NPC script: keepalive timer (GetTickCount at last send)
DWORD    DAT_07e11dcc  = 0;
DWORD    DAT_07e11de8  = 0;
DWORD    DAT_07e11e50  = 0;   // NPC script chat-log slot 0
DWORD    DAT_07e11e54  = 0;   // NPC script chat-log slot 1
DWORD    DAT_07e11e58  = 0;   // NPC script chat-log slot 2
DWORD    DAT_07e11e5c  = 0;   // NPC script chat-log slot 3
DWORD    DAT_07e11e78  = 0;
DWORD    DAT_07e11e98  = 0;
char     DAT_07e11e9c  = 0;
char     DAT_07e11d6e           = 0;
char     DAT_07d4ac7c[256]      = {};
char     DAT_07d4ada8[256]      = {};
// Scene_Login credential dialog + version footer. La tabla de strings del
// 0.97k está stripped: get_xrefs_to en Ghidra confirma que NADA escribe estos
// buffers en el binario (se renderizan vacíos). Rellenamos con defaults
// sensatos para que el panel muestre botones/texto legible.
char     lpString_07d4aed4[128] = "OK";          // botón OK del panel de credenciales
char     lpString_07d4b000[128] = "Exit";        // botón Exit/Cancel del panel
char     DAT_07d4b708[128]      = {};            // char name format string
char     DAT_07d4b12c[128]      = "Mu Online";                       // línea de versión 1 (centrada)
char     DAT_07d4b258[128]      = "Ver 0.97k";                       // línea de versión 2 (derecha)
char     DAT_07d4b384[128]      = "Copyright (C) 2003 Webzen Inc.";  // sprintf format sin args (izquierda)
char     lpString_07d4c518[128] = "Connecting...";
DWORD    DAT_07e127f8  = 0;
// El pool de items en el suelo es de 1000 entradas × 0x204 bytes (≈504 KB).
// El slot base es DAT_07e12840 + key*0x204; CreateItem escribe active@ip+72,
// model@ip+74, pos@ip+88 y el render (FUN_005038e0 en stubs_render_helpers.cpp)
// los lee en los mismos offsets (active@slot+72). Ambos alineados sobre
// DAT_07e12840.
unsigned char DAT_07e12840[1000 * 0x204] = {};
DWORD    DAT_07e12945  = 0;
char     DAT_07e91350[0x44]  = {0};
DWORD    DAT_07e91388  = 0;
byte     DAT_07e9138e  = 0;   // UI grid selected column
byte     DAT_07e9138f  = 0;   // UI grid selected row
DWORD    DAT_07e91394  = 0;
DWORD    DAT_07e913a8  = 0;
DWORD    DAT_07e91428  = 0;
DWORD    DAT_07e91784  = 0;
DWORD    DAT_07e91788  = 0;
DWORD    DAT_07e919b8  = 0;
// Tabla de nombres (guild members / buffs), stride 80 (0x50). En el binario
// va de 0x07E919BC al centinela dword_7EA51EC (0x07EA51EC) = 0x13C30 bytes.
// 2026-08-15: era un DWORD de 4 bytes y TODOS sus consumidores la indexan
// como `&DAT_07e919bc + N*80` — o sea leían fuera de rango. Ver la nota en
// globals.h.
char     DAT_07e919bc[0x13C30] = {};
BYTE     DAT_07ea5298[0x880] = {0};   // see globals.h
DWORD    DAT_07ea5b18  = 0;
DWORD    DAT_07ea5b1c  = 0;
DWORD    DAT_07ea5b20  = 0;
DWORD    DAT_07ea8410  = 0;
DWORD    DAT_07ea8414  = 0;
// Equip grid buffer — see globals.h header for layout rationale.  Each
// 68-byte row contains 8 ITEM cells at stride 544 bytes (so cells overlap
// across rows in a tiled layout).  Initialise all Type fields (every 56-th
// byte = byte at row-relative offset -56, but here we just zero-fill and
// stamp 0xFFFF in HUD_InitInventoryPools).
// unk_7EA9504 / unk_7EA9328 son POSICIONES DENTRO DEL INVENTARIO REAL, no un
// buffer aparte.  OffsetInventoryItems esta en 0x07EA8410 y la cuenta cierra
// exacta con slots de 0x44 y el campo Key en +56:
//     0x7EA9504 - 0x07EA8410 = 4340 = 63 * 0x44 + 56   -> slot 63, Key
//     0x7EA9328 - 0x07EA8410 = 3864 = 56 * 0x44 + 56   -> slot 56, Key
// El walker de FindQuestItemsInInven / FUN_004824c0 / FUN_00482850 recorre los
// slots 63..56 por afuera (paso -0x44) y por adentro salta de a -8 slots
// (paso -544), o sea cubre los 64 slots del grid 8x8.  Lo que lee en cada uno:
//     v7          = slot + 56  -> Key   ( > 0 = celda ocupada )
//     v7 - 56     = slot + 0   -> Type
//     v7 - 52     = slot + 4   -> Level
//
// 2026-08-22: acá había un `g_EquipGridBuf[0x12DC]` suelto, en cero, que nadie
// escribía nunca — los tres walkers recorrían memoria vacía y siempre reportaban
// "no tenés el item".  Por eso la lista de items de quest salía en rojo con el
// item en el inventario.  Ahora los dos punteros se reenraízan sobre el pool real.
int   *p_DAT_07ea9504_ = (int*)&OffsetInventoryItems[63 * 0x44 + 56];
int   *p_DAT_07ea9328_ = (int*)&OffsetInventoryItems[56 * 0x44 + 56];
DWORD    DAT_07ea9800  = 0;
DWORD    g_ItemMoveSourcePool = 0;
DWORD    g_ItemMoveTargetPool = 0;
DWORD    DAT_07ea9810  = 0;
DWORD    DAT_07ea9814  = 0;
float    _DAT_07ea9814 = 0.0f;
char     DAT_07ea9815  = 0;
char     DAT_07ea9816  = 0;
char     DAT_07ea9817  = 0;
unsigned int DAT_07ea9818  = 0;  // SecondPassword PIN bytes [4-7] (audit #8)
DWORD    DAT_07ea981c  = 0;
short    DAT_07ea981e  = 0;
DWORD    DAT_07ea982c  = 0;   // Screen3 panel origin X
DWORD    DAT_07ea9830  = 0;   // Screen3 panel origin Y
DWORD    DAT_07ea9834  = 0;
char     DAT_07ea983e  = 0;
DWORD    DAT_07eaa0d0  = 0;
DWORD    DAT_07eaa0d8  = 0;
DWORD    DAT_07eaa0e0  = 0;
DWORD    DAT_07eaa0e4  = 0;
DWORD    DAT_07eaa0f0  = 0;
DWORD    DAT_07eaa0f4  = 0;
int      DAT_07eaa0f8  = 0;   // item repair counter
DWORD    DAT_07eaa0fc  = 0;
char     DAT_07eaa0fd  = 0;
char     DAT_07eaa114  = 0;
char     DAT_07eaa115  = 0;
char     DAT_07eaa116  = 0;
char     DAT_07eaa117  = 0;
char     DAT_07eaa118  = 0;
char     DAT_07eaa119  = 0;
char     DAT_07eaa11a  = 0;
char     DAT_07eaa11b  = 0;
char     DAT_07eaa11c  = 0;
DWORD    DAT_07eaa124  = 0;
DWORD    DAT_07eaa128  = 0;
DWORD    DAT_07eaa13c  = 0;
DWORD    DAT_07eaa16c  = 0;
DWORD    DAT_07eaa144  = 0;
DWORD    DAT_07eaa14c  = 0;
DWORD    DAT_07eaa150  = 0;
DWORD    DAT_07eaa154  = 0;
DWORD    DAT_07eaa160  = 0;
char     DAT_07eaa165  = 0;
DWORD    DAT_07eaa168  = 0;   // TextureEnable (GL texture state cache)
char     DAT_07eaa179  = 0;
char     DAT_07eaa190  = 0;   // inventory drop error message ID string
// Frustum world corners: 5 × 3 floats at 0x07eab1b0
float    DAT_07eab1b0  = 0.0f;  // corner 0 X
float    DAT_07eab1b4  = 0.0f;  // corner 0 Y
float    DAT_07eab1b8  = 0.0f;  // corner 0 Z
float    DAT_07eab1bc  = 0.0f;  // corner 1 X
float    DAT_07eab1c0  = 0.0f;  // corner 1 Y
float    DAT_07eab1c4  = 0.0f;  // corner 1 Z
float    DAT_07eab1c8  = 0.0f;  // corner 2 X
float    DAT_07eab1cc  = 0.0f;  // corner 2 Y
float    DAT_07eab1d0  = 0.0f;  // corner 2 Z
float    DAT_07eab1d4  = 0.0f;  // corner 3 X
float    DAT_07eab1d8  = 0.0f;  // corner 3 Y
float    DAT_07eab1dc  = 0.0f;  // corner 3 Z
float    DAT_07eab1e0  = 0.0f;  // corner 4 X
float    DAT_07eab1e4  = 0.0f;  // corner 4 Y
float    DAT_07eab1e8  = 0.0f;  // corner 4 Z
DWORD    DAT_07eab1ec  = 0;
DWORD    DAT_07eab1f0  = 0;
DWORD    DAT_07eab1f4  = 0;
DWORD    DAT_07eab1f8  = 0;
// 2026-05-04: water-wave height table — 256×256 floats = 256KB.  Original
// binary placed it at 0x07EAB200; was a 4-byte DWORD here, so writes via
// `(char*)&DAT_07eab200 + (row*256+col)*4` (Terrain_Water.cpp + Terrain_Light.cpp)
// overflowed massively into adjacent BSS.
float    DAT_07eab200[256 * 256] = {};
DWORD    DAT_07eab24c  = 0;   // BackTerrainHeight array base
DWORD    DAT_07eab250  = 0;   // PrimaryTerrainLight array base
DWORD    DAT_07eeb200  = 0;
DWORD    DAT_07eeb204  = 0;
DWORD    DAT_07eeb208  = 0;
DWORD    DAT_07eeb20c  = 0;
DWORD    DAT_07eeb210  = 0;
float    DAT_07eeb214  = 0.0f;   // WaterMove — terrain water UV scroll offset (RenderTerrain)
// BUG-FIX 2026-05-01: era DWORD simple pero FUN_004f8ff0 (Frustum_IsVisible)
// lee 4 floats consecutivos desde cada array. Camera_SetMatrix también escribe
// los 4 corners. Sin contiguidad garantizada, el cull del frustum rechazaba
// TODOS los chunks (chunks_vis=0) y los objetos del .obj nunca se renderean.
float    DAT_07eeb218[4] = {0};   // frustum quad Y[4]
float    DAT_07eeb228[4] = {0};   // frustum quad X[4]
// BUG-FIX 2026-04-28: era DWORD simple pero FUN_00529360 escribe 256x256 RGB
// floats (= 196608 floats) usados como TerrainLight RGB ambiente.
float    DAT_07eeb238[256 * 256 * 3] = {};
DWORD    DAT_07feb238  = 0;
DWORD    DAT_07feb23c  = 0;
// 2026-04-28: tile pick corners buffer — 12 floats contiguos (4 vec3 corners
// del quad clickeado). Antes era 12 globals separados → no garantizado
// contiguo en memoria → glVertex3fv leía basura → AV en NVOGL.
float    g_TilePickBuf[12] = {};
// BUG-FIX 2026-04-28: TerrainNormal[256*256][3] float array — antes era DWORD.
// FUN_004f70b0 (CreateTerrainNormal) escribe 256*256*3 = 196608 floats acá.
float    DAT_07feb288[256 * 256 * 3] = {};

// ── Large game data arrays ────────────────────────────────────────────────────
// BUG-FIX 2026-04-28: estos cuatro estaban declarados como `DWORD` simple pero
// el código (FUN_004f6c60 Terrain_Clear y otros) los indexa hasta [65535].
// El IDA decomp expresa los accesos como `(int)&DAT_xxxx + iVar2` que MSVC
// compila como offset del símbolo → escribe fuera de bounds → AV/corruption.
// Cambiar a arrays explícitos del tamaño real evita el crash y elimina la
// corrupción silenciosa de globals adyacentes.
//   DAT_080ab2b4 = TileTex2[256*256] (BYTE)   — segundo índice de textura
//   DAT_080bb2b4 = TileTex1[256*256] (BYTE)   — primer índice de textura
//   DAT_0834b608 = TerrainHeight[256*256] (float) — altura de tile
//   DAT_0810b2cc = TerrainNoise[256*256] (float)  — ruido aleatorio init
unsigned char  DAT_080ab2b4[0x10000] = {};
unsigned char  DAT_080bb2b4[0x10000] = {};
// BUG-FIX 2026-04-28: BackTerrainHeight[256*256] float array — antes era DWORD.
float    DAT_080cb2cc[0x10000] = {};
float    DAT_0810b2cc[0x10000] = {};
float    g_TerrainTexCoord[8] = {};   // TerrainTextureCoord[4][2] (RenderTerrainFace/FaceTexture)
DWORD    DAT_0814b2dc  = 0;
BYTE     g_TerrainObjTable[0x328] = {};   // ambient terrain-object table (sub_4F7060); &DAT_081cb2ed=&[5]
// 2026-05-04: live per-tile lighting buffer — 256×256 tiles × 3 floats =
// 786432 bytes.  Was a 4-byte DWORD here, but FUN_004f95e0 writes via
// `(char*)&DAT_081cb608 + iVar2*12` (and analogous via cb60c, cb610) up to
// 786KB into adjacent BSS — that was the source of the DAT_07cf5600 "Oye!"
// corruption.  Original binary placed cb608/cb60c/cb610 as the three DWORDs
// of slot 0; the macros below project cb60c/cb610 into the same buffer.
float    DAT_081cb608[256 * 256 * 3] = {};
// BUG-FIX 2026-04-28: TerrainLightData[256*256][3] — antes era DWORD.
// FUN_004f71c0 (Terrain_FinalizeLighting) escribe 196608 floats acá.
float    DAT_0828b608[256 * 256 * 3] = {};
float    DAT_0834b608[0x10000] = {};
// Frustum plane normals: 5 planes × 3 floats
float    DAT_0838b7c4  = 0.0f;  // plane 0 normal X
float    DAT_0838b7c8  = 0.0f;  // plane 0 normal Y
float    DAT_0838b7cc  = 0.0f;  // plane 0 normal Z
float    DAT_0838b7d0  = 0.0f;  // plane 1 normal X
float    DAT_0838b7d4  = 0.0f;  // plane 1 normal Y
float    DAT_0838b7d8  = 0.0f;  // plane 1 normal Z
float    DAT_0838b7dc  = 0.0f;  // plane 2 normal X
float    DAT_0838b7e0  = 0.0f;  // plane 2 normal Y
float    DAT_0838b7e4  = 0.0f;  // plane 2 normal Z
float    DAT_0838b7e8  = 0.0f;  // plane 3 normal X
float    DAT_0838b7ec  = 0.0f;  // plane 3 normal Y
float    DAT_0838b7f0  = 0.0f;  // plane 3 normal Z
float    DAT_0838b7f4  = 0.0f;  // plane 4 (near) normal X
float    DAT_0838b7f8  = 0.0f;  // plane 4 (near) normal Y
float    DAT_0838b7fc  = 0.0f;  // plane 4 (near) normal Z
// BUG-FIX 2026-04-28: BMPHeader[1080] (BITMAPFILEHEADER + DIB header + palette)
// — antes era DWORD, OpenTerrainHeight escribe 1080 bytes acá.
unsigned char DAT_0838b800[1080] = {};
DWORD    DAT_0838bc44  = 0;
char     DAT_0838bc70[0x10000] = {};  // terrain walk flags (per-tile byte array, 256x256 grid)
DWORD    DAT_0839bc88  = 0;            // terrain-light double-buffer toggle (RenderTerrain)
DWORD    DAT_0839bc86  = 0;
DWORD    DAT_0839bc8c  = 0;
DWORD    DAT_0839bc90  = 0;
DWORD    DAT_0839bc94  = 0;
DWORD    DAT_0839bc98  = 0;
DWORD    DAT_0839bc9c  = 0;
DWORD    DAT_0839be18  = 0;

// ── Scene / state machine vars ────────────────────────────────────────────────
float    _DAT_083a0210 = 0.0f;  // EarthQuake
DWORD    DAT_083a0210  = 0;
// ── Object-bucket grid ────────────────────────────────────────────────────────
// 16×16 chunk-cell grid. Each cell = {pad/scratch, head_ptr, tail_ptr, visible_flag}
// = 4 DWORDs = 16 bytes. Total = 16×16×16 = 4096 bytes = 0x1000.
// Maps to original 0x083a0218..0x083a1217. Macros in globals.h:
//   DAT_083a0218 = grid+0  (cell[0]+0  scratch)
//   DAT_083a021c = grid+4  (cell[0].head — Terrain_Render reads *chunk_ptr)
// Insert (FUN_004ff5a0) writes head at cell+4, tail at cell+8.
// Unload (FUN_004ffd50) walks puVar5=&DAT_083a0218 reading puVar5+8 as tail.
// Previously DAT_083a0218 was a separate orphan DWORD and DAT_083a021c was at
// grid+0 — the unload walker read 4096 bytes of unrelated BSS past the orphan
// DWORD and crashed when it hit a non-null garbage value (treated as a node).
char     g_ObjectBucketGrid[0x1000] = {0};
DWORD    DAT_083a1378  = 0;
DWORD    DAT_083a2e92  = 0;
DWORD    DAT_083a3ff0  = 0;
char     DAT_083a410c  = 0;
// ── DAT_083a4110 — mouse-ray endpoint world pos (3 floats) ───────────────────
// En el binario original Camera_MouseRay (FUN_005112f0) escribe out_ray[0..2]
// arrancando en 0x083a4110. Si se declara como UN único DWORD el linker no
// reserva los 12 bytes y los writes a out_ray[1]/[2] caen en globals vecinos.
DWORD    DAT_083a4110_arr[3] = {0};
DWORD&   DAT_083a4110 = DAT_083a4110_arr[0];
DWORD    DAT_083a4124  = 0;
DWORD    DAT_083a413c  = 0;
// ─── View/camera 3x4 matrix (48 bytes = 12 DWORDs) ───────────────────────────
// En el binario original 0x083a4140..0x083a416f es UN único buffer que
// FUN_005111d0 (GL_GetModelViewMatrix) llena con 3 filas × 4 floats.
// DAT_083a414c / DAT_083a415c / DAT_083a416c son los 4-th elementos de cada
// fila (offsets 0x0c, 0x1c, 0x2c) — no globales independientes.
// Si se declaran por separado el linker los reubica y FUN_005111d0
// desborda 48 bytes sobre globales vecinos (p.ej. DAT_083a7c49 Scene_Login
// init flag → loop de re-init cada frame).
DWORD    DAT_083a4140[12] = {0};
DWORD&   DAT_083a414c = DAT_083a4140[3];
DWORD&   DAT_083a415c = DAT_083a4140[7];
DWORD&   DAT_083a416c = DAT_083a4140[11];
// DAT_083a4174 — "Screen %02d %02d %02d %02d - %04d" screenshot filename buffer.
// Original declarado como DWORD pero crt_sprintf lo usa como char[]: la escritura
// real es ~26 chars. Reservamos el gap completo hasta DAT_083a4278 (0x104 bytes).
char     DAT_083a4174[0x104] = {0};
DWORD    DAT_083a4278  = 0;  // MouseY
DWORD    DAT_083a427c  = 0;  // MouseX
DWORD    DAT_083a4280  = 0;  // viewport width
// ── DAT_083a4284..428c — camera world position (3 floats) ────────────────────
// Camera_MouseRay (FUN_005112f0) escribe via FUN_004fa110(... &DAT_083a4284) los
// 3 floats consecutivos. DEBEN ser contiguos. Las definiciones en líneas 1318-
// 1320 (_DAT_083a4284..428c) ahora son referencias a este array para que las
// lecturas via float (camPos, etc.) vean lo que Camera_MouseRay escribió.
DWORD    DAT_083a4284_arr[3] = {0};
DWORD&   DAT_083a4284 = DAT_083a4284_arr[0];
DWORD&   DAT_083a4288 = DAT_083a4284_arr[1];
DWORD&   DAT_083a428c = DAT_083a4284_arr[2];
char     DAT_083a4299  = 0;
DWORD    DAT_083a429c  = 0;
DWORD    DAT_083a42a0  = 0;
DWORD    DAT_083a42a4  = 0;
DWORD    DAT_083a42a8  = 0;
DWORD    DAT_083a42ac  = 0;  // MouseRButton
DWORD    DAT_083a42b0  = 0;  // viewport height
// CameraAngle[3] @ 0x083a42b8 — 3 floats read by BeginOpengl via glRotatef and
// written by MoveCamera/Scene_CharSelect. Must be contiguous; DWORD aliases
// are provided so legacy sites that stored float-bits as DWORD still link.
// See bottom of file for the moved `float CameraAngle[3]` definition — here
// we emit the storage directly so memory layout matches the binary.
// (Defined in one place below via references — see CameraAngle_storage)
extern float    CameraAngle[3];         // defined later in this TU
DWORD&   DAT_083a42b8 = *(DWORD*)&CameraAngle[0];
DWORD&   DAT_083a42bc = *(DWORD*)&CameraAngle[1];
DWORD&   DAT_083a42c0 = *(DWORD*)&CameraAngle[2];
DWORD    DAT_083a42c4  = 0;  // MouseLButton
DWORD    DAT_083a42c8  = 0;  // viewport x offset
DWORD    DAT_083a42cc  = 0;  // viewport y offset
// CameraPosition[3] @ 0x083a42d4 — 3 floats read by BeginOpengl via glTranslatef
// and written by MoveCamera. Must be contiguous. DWORD views are aliases.
extern float    CameraPosition[3];      // defined later in this TU
float&  _DAT_083a42d4 = CameraPosition[0];
DWORD&   DAT_083a42d4 = *(DWORD*)&CameraPosition[0];
float&  _DAT_083a42d8 = CameraPosition[1];
DWORD&   DAT_083a42d8 = *(DWORD*)&CameraPosition[1];
float&  _DAT_083a42dc = CameraPosition[2];
DWORD&   DAT_083a42dc = *(DWORD*)&CameraPosition[2];
char     DAT_083a42e9  = 0;
char     DAT_083a42ea  = 0;  // FogEnable
char     DAT_083a42eb  = 0;  // auto-drop trigger flag (inventory)
DWORD    DAT_083a42ec  = 0;
DWORD    DAT_083a4320  = 0;
DWORD    DAT_083a4328  = 0;
// CurrentCameraPosition[3] @ 0x083a432c — 3 contiguous floats read by MoveCamera
// via `(float*)&DAT_083a432c`. Defining three separate DWORDs lets MSVC scatter
// them so index [1] and [2] would miss → camera position never integrates.
float    CurrentCameraPosition[3] = {0.0f, 0.0f, 0.0f};
DWORD&   DAT_083a432c  = *(DWORD*)&CurrentCameraPosition[0];
DWORD&   DAT_083a4330  = *(DWORD*)&CurrentCameraPosition[1];
float&  _DAT_083a4334  = CurrentCameraPosition[2];
DWORD&   DAT_083a4334  = *(DWORD*)&CurrentCameraPosition[2];
float    DAT_083a45d4  = 0.0f;  // Scene_Dispatch — login background animation angle (sin-product)
// ServerList buffer: 24 entries * stride 0x21e + padding. End bounds in the
// original code were absolute (0x83a7ac6, 0x83a7ada). We make it a contiguous
// buffer and the Scene_Login_ServerSelect loops use ((int)&DAT_083a45d8 + 0x34ee).
char     DAT_083a45d8[0x3600] = {};  // 13824 bytes covers the original 0x3502 span.
// Overlapping sub-symbols at the same address — accessed via macros in globals.h.

// Local server-name table (0x07D52C34). En el binario original es un buffer BSS
// que ReceiveServerList (F4/02) lee para el nombre de cada grupo (stride 300,
// indexado por ServerCode/20). Se puebla en runtime con el paquete F4/04
// (CCCustomServerListSend del ConnectServer: {WORD ServerCode; char Name[32]}).
// 24 grupos * 300 bytes de stride.
char     DAT_07d52c34[24 * 300] = {};
char     DAT_083a7ac8  = 0;
char     DAT_083a7acc  = 0;
// CurrentCameraAngle[3] @ 0x083a7ad0 — 3 floats (pitch, yaw, roll).
// Prior definition used `char` which would truncate float writes to 1 byte
// AND let MSVC scatter the three 1-byte slots away from each other →
// MoveCamera's `(float*)&DAT_083a7ad0 + i` reads garbage for i=1,2 and stomps
// adjacent globals when writing.
float    CurrentCameraAngle[3] = {0.0f, 0.0f, 0.0f};
float&   DAT_083a7ad0 = CurrentCameraAngle[0];
float&   DAT_083a7ad4 = CurrentCameraAngle[1];
float&   DAT_083a7ad8 = CurrentCameraAngle[2];
DWORD    DAT_083a7af4  = 0;
DWORD    DAT_083a7c00  = 0;
DWORD    DAT_083a7c10  = 0;
int      DAT_083a7c14  = 0;  // login sub-state
DWORD    DAT_083a7c18  = 0;
DWORD    DAT_083a7c1c  = 0;
DWORD    DAT_083a7c20  = 0;
DWORD    DAT_083a7c24  = 0;
DWORD    DAT_083a7c28  = 0;
DWORD    DAT_083a7c38  = 0;
DWORD    DAT_083a7c3c  = 0;
DWORD    DAT_083a7c40  = 0;  // cantidad de servers del F4/02 (ReceiveServerList)
DWORD    DAT_083a7c44  = 0;
// BUG-FIX: DAT_083a7c48 es char en el binario original (flag "connection check
// enable" leído como byte en Game_MainLoop). Declararlo DWORD hacía que
// `DAT_083a7c48 = 1` escribiese 4 bytes 01 00 00 00 y pisase DAT_083a7c49/4a/4b.
// Resultado: cada frame el init flag de Scene_Login (c49) volvía a 0, Scene_Login
// retornaba temprano y nada se dibujaba.
char     DAT_083a7c48  = 0;
char     DAT_083a7c49  = 0;
char     DAT_083a7c4a  = 0;
char     DAT_083a7c4b  = 0;
char     DAT_083a7c4c  = 0;
char     DAT_083a7c4d  = 0;
DWORD    DAT_083a7c50  = 0;
DWORD    DAT_083a7c54  = 0;
DWORD    DAT_083a7c58  = 0;
DWORD    DAT_083a7c5c  = 0;  // chat-log label for exit-countdown message (empty id)
DWORD    DAT_083a7c6c  = 0;
DWORD    DAT_083a7c70  = 0;
DWORD    DAT_083a7c74  = 0;
DWORD    DAT_083a7c78  = 0;
DWORD    DAT_083a7c7c  = 0;
DWORD    DAT_083a7c80  = 0;
DWORD    DAT_083a7c84  = 0;
DWORD    DAT_083a7c88  = 0;
DWORD    DAT_083a7c8c  = 0;
DWORD    DAT_083a7c90  = 0;
DWORD    DAT_083a7c94  = 0;
DWORD    DAT_083a7c98  = 0;
char     g_BitmapsRaw[0x13D30]  = {};  // Bitmaps table (1450 slots × 0x38 stride)
DWORD    DAT_083bb9d0  = 0;
DWORD    DAT_083bbb14  = 0;
char     lpBuffer_083bbb60[0x400] = {};  // named pipe write buffer
DWORD    DAT_083bbb64  = 0;
DWORD    DAT_083bbb68  = 0;
DWORD    DAT_083bbb6c  = 0;
DWORD    DAT_083bbb74  = 0;
HANDLE   lpTargetHandle_00563b58 = NULL;

// Input / player name buffers
char     DAT_007d29e5  = 0;
char     DAT_007eaa11  = 0;

// ── Additional globals needed by Game/Scene files ─────────────────────────────
// 2026-05-04: 64-byte canary padding around DAT_07e11d70..d72 (ChatMode/IME/
// DigitOnly).  These flags get corrupted to 0xFF every frame by an adjacent
// buffer-overflow we couldn't pinpoint yet.  Canaries should ABSORB the
// overflow if the writer is hitting bytes near d70/d71/d72.  If the canaries
// remain 0xCC after the corruption fires, the overflow is much further away.
char     g_PadBeforeChatMode[64] = { 0xCC };
char     DAT_07e11d70  = 0;
char     DAT_07e11d71  = 0;
char     DAT_07e11d72  = 0;
char     g_PadAfterChatMode[64]  = { 0xCC };
DWORD    DAT_07e11d78  = 0;
DWORD    DAT_07e11d30  = 0;
DWORD    DAT_07e11d1c  = 0;
DWORD    DAT_07e11d28  = 0;
DWORD    DAT_07e11d64  = 0;
float   _DAT_07e11d4c  = 0.0f;
float   _DAT_07e11d50  = 0.0f;
BYTE     DAT_07e113d8[40]  = {0};
// DAT_07e113d9 es DAT_07e113d8[1] en el binario original (flag per-slot de
// obfuscación: 0=plain, 1=password-mask, 2=hybrid). Separarlo en su propio
// global hacía que el init escribiera a otra address y Chat_DrawField siempre
// leyera 0 → password se mostraba en texto plano.
char&    DAT_07e113d9 = reinterpret_cast<char&>(DAT_07e113d8[1]);

DWORD    DAT_07e016c0  = 0;
DWORD    DAT_07e016c4  = 0;
DWORD    DAT_07e109c8  = 0;
// Historial de chat @ 0x07E113E4 — anillo de 5 entradas x 256 bytes.
// Lo confirma el propio binario: el walker de RenderChatInput termina en
// 0x07E118E4, y 0x7E118E4 - 0x7E113E4 = 0x500 = 5 * 256.
// 2026-08-21: estaba como un solo DWORD, y Chat_InputTick (flechas arriba/abajo)
// hace `memcpy((char*)&DAT_07e113e4 + idx * 0x100, ...)` — escritura de hasta
// 1280 bytes fuera del global.
char     DAT_07e113e4[5 * 256] = {};
// _DAT_07e118e4 already defined at line ~470

DWORD    DAT_07d78094  = 0;
BYTE     DAT_07d780a8[40]  = {0};
DWORD    DAT_07d780ac  = 0;
// Multi-slot input buffer (IDA: InputText[10][256] @ 0x07db8710).
// Slot 0 = chat / username, slot 1 = whisper-target / password.
// DAT_07db8810 is a #define alias for slot 1 in globals.h.
char     DAT_07db8710[10][256] = {{0}};
DWORD    DAT_07db8708  = 0;

// 2026-05-04: server-config globals (popullados por opcodes 0xDD/DE/DF).
// gPrintPlayer.MaxCharacterLevel del DLL source mapea a g_MaxCharacterLevel.
// Usado por RenderCharacterInfoWindow "Nivel: %d / %d".  Default 400 = cap
// vanilla 0.97k hasta que el server mande PMSG_CHARACTER_MAX_LEVEL_RECV.
extern "C" {
    DWORD    g_MaxCharacterLevel  = 400;
    WORD     g_CharDeleteMaxLevel = 10;
    BYTE     g_CharCreationEnable = 1;
}

void    *DAT_07cf1ff4  = NULL;

char     lpData_055c9ba0[12] = {0};

char     DAT_07d4c3ec[256] = {0};
char     DAT_07d4c644[256] = {0};
char     DAT_07d4c770[256] = {0};
DWORD    DAT_07d52c38  = 0;
char     DAT_07d530e8[256] = {0};
char     DAT_07d53214[256] = {0};

// Model data table base + entity vtable
// (DAT_05828d58 and DAT_05826e08 are defined above in their original sections)
// ── BoneVertex pool ──────────────────────────────────────────────────────────
// Transformed-vertex buffer used by Sprite_DrawBone (FUN_004404e0), BMD_DrawMesh
// and related paths. Layout: [mesh * 15000 + vert] * float[3] = 12 bytes stride.
// Capacity: 32 mesh/frame slots × 15000 verts × 12 B = 5.76 MB.
// DAT_0584621c is the base (slot 0, vert 0). DAT_05846224 is a Ghidra label at
// +8 B (the output start used by FUN_004404e0's pfOut). Both resolve via macros
// (see globals.h) to DWORD lvalues at the correct offsets within this buffer.
char     g_BoneVertexBuf[32 * 15000 * 12] = {0};  // 5,760,000 bytes
char     lpString_05826bfc[0x50] = {0};
char     lpString_05826cc0[0x50] = {0};
char     lpString_05826cc9[0x50] = {0};

// DAT_07e919bc, DAT_07abf5dc, DAT_07abf5d8, DAT_07eeb218/228 — defined above

// Temp bone position buffers
DWORD    DAT_07abf444[12] = {0};
DWORD    DAT_07abf3e4[12] = {0};
DWORD    DAT_07abf414[12] = {0};

// Animation distance / frequency constants
float   _DAT_00552650  = 4.0f;
float   _DAT_00552954  = 0.0015f;

// Teleport-anim pool — 100 slots × 0x70 bytes; see globals.h note (was 1 byte
// causing heap corruption in Entity_TeleportAnim).
char     DAT_07c80110[100 * 0x70] = {};

// Character/effect update pool — 1002 slots × 444 bytes = 0x6c660 (matches
// binario original 0x07c85890..0x07cf1ef0). Antes era 1 byte → FUN_004795c0
// (Effect_Spawn) tenía un AUTO-SKIP que saltaba la implementación entera y
// NO spawneaba NINGUNA partícula (glow +9, wing FX, weapon glows, lightning
// crackles — todo invisible). Buffer real ahora permite que el pool funcione.
char     DAT_07c85890[1002 * 0x1bc] = {0};
// DAT_07c85894 = mismo pool, offset +4 (Sound_Queue.cpp / Sigil_RenderAll).
// Lo dejamos como referencia al int en pool[4..7] para compartir storage.
int&     DAT_07c85894 = *reinterpret_cast<int*>(&DAT_07c85890[4]);

// Math / animation constants
float   _DAT_00552958 = 140.0f;
DWORD    DAT_00552958 = 0;
// _DAT_0055283c — X-scale constant (1/640); definido arriba con valor correcto.
// Esta definición fue 0.0f por error; reemplazada por la inicialización en la zona de video-scale.
float   _DAT_00552cb8 = 540.0f;
float   _DAT_00552cbc = 1190.0f;

// Server select input
DWORD    DAT_0056169c = 0;

// Char menu UI builder (FUN_004c3530)
int      DAT_07e11d20 = 0;
int      DAT_07e11d24 = 0;
char     lpString_07e90798[3000] = {};  // 30 slots * 100 bytes
int      DAT_07e91708[30] = {};  // TextListColor - 30 slots, igual que lpString
int      DAT_07ea7b10[30] = {};  // TextBold      - 30 slots, igual que lpString
char     DAT_07d329c4 = 0;
char     DAT_07d32af0 = 0;
char     DAT_07d34134 = 0;
char     DAT_07d34260 = 0;
char     DAT_07d358a4 = 0;
int      DAT_07d78068 = 0;
// 2026-05-08: backup MOVED to Render_Frame.cpp — adjacent placement next to
// DAT_07d78068 caused the corruption writer (2 consecutive int writes
// 0x00000001 + 0x00000000) to clobber both. Now lives in a different .obj.
int      DAT_07d78080 = 0;       // font height (set by resolution in WinMain)
// DAT_07e91530/534/53c/540 pasaron a ser macros sobre DAT_07e91528 (ver globals.h):
// en el binario son COLUMNAS de la misma tabla, no globals sueltos.
// UI text strings
char     DAT_0055a408[] = "";
char     DAT_0055a40c[] = "";
char     DAT_0055a410[] = "";
char     DAT_0055a414[] = "";
char     DAT_0055a418[] = "";
char     DAT_0055a41c[] = "";
char     DAT_0055a420[] = "";
char     DAT_0055a424[] = "";
char     DAT_0055a428[] = "";
char     DAT_0055a42c[] = "";
char     DAT_0055a430[] = "";
char     DAT_0055a434[] = "";
// RenderItemInfo string constants
// 2026-08-18: los SIETE son "\n" en el binario (leidos en 0x0055A4E0,
// 0x0055A4E4, 0x0055A570, 0x0055A5F4, 0x0055A5F0, 0x0055A5FC, 0x0055A640) —
// lineas separadoras de MEDIA altura, que es lo que cuenta SkipNum
// (DAT_07eaa158).  Estaban como cadena vacia, y como DrawItemInfoBox corta el
// conteo en la primera linea vacia, el slot 0 dejaba el tooltip sin dibujar.
char     DAT_0055a4e4[] = "\n";  // 0x0055A4E4 — separador tras la linea de precio
char     DAT_0055a570[] = "\n";  // 0x0055A570 — separador tras el nombre del item
char     DAT_0055a4e0[] = "\n";  // 0x0055A4E0 — separador de media altura (slot 0)
char     DAT_0055a5f4[] = "\n";  // 0x0055A5F4 — separador de media altura
char     DAT_0055a5f0[] = "\n";  // 0x0055A5F0 — separador de media altura (RenderItemInfo)
char     DAT_0055a5fc[] = "\n";  // 0x0055A5FC — separador de media altura (RenderRepairInfo)
char     DAT_0055a640[] = "\n";  // 0x0055A640 — separador de media altura (RenderRepairInfo, final)
char     DAT_0055a608[] = "";    // s__s__s format
char     DAT_0055a630[] = "";    // secondary stats line
char     DAT_07d3b40c[] = "";    // item level line format

// Weather particle system (FUN_0046cc80): DAT_07c5ab5c is the +0x20 alias
// of DAT_07c5ab3c, declared in globals.h; it has no standalone storage.
DWORD    DAT_07c74ae8  = 0;
DWORD    DAT_07c74aec  = 0;
int     _DAT_00559b9c  = 0;
float   _DAT_00552874  = 0.05f;
float   _DAT_00552880  = 0.0703125f;
float   _DAT_005529c8  = -0.1f;
DWORD    DAT_005529c8  = 0;
float   _DAT_00552a38  = 200000.0f;
float   _DAT_00552a3c  = 3.2f;
float   _DAT_00552a40  = 0.0005f;

// Mouse hover tick (FUN_004b0310)
DWORD    DAT_080ab288  = 0;
DWORD    DAT_080ab28c  = 0;
float    DAT_083a4130  = 0.0f;
float    DAT_083a4134  = 0.0f;
float    DAT_083a4138  = 0.0f;
int      DAT_07e11d5c  = 0;
// Hover targets: -1 = none. Mouse_Hover (FUN_004b0310) resets each frame to -1
// before priority-probing. En login/char-select Mouse_Hover no corre, así que
// el valor inicial debe ser -1 para que RenderCursor (FUN_004bffa0) muestre
// el cursor arrow por defecto en vez del item-cursor (bitmap 5 = mano abierta).
int      SelectedItem       = -1;  // DAT_00559c48
int      SelectedNpc        = -1;  // DAT_00559c4c
int      SelectedCharacter  = -1;  // DAT_00559c50
int      SelectedOperate    = -1;  // DAT_00559c54
int      DAT_00559c58  = -1;  // SelectedCharacter secondary
// 2026-05-06 BUG-FIX: m_bAutoAttack default = 1 (enabled). Per IDA
// Mouse_Hover (sub_4B0310:85), if !m_bAutoAttack the hover-target
// (DAT_00559c50 / SelectedCharacter) is reset to -1 every frame BEFORE the click handler reads
// it → click on mob fell through to ground-click handler. User reported
// "no atacaba a la primera, me costo empezar a atacar" 2026-05-06.
char     DAT_00559c5c  = 1;
int      DAT_00559c60  = 0;
int      DAT_00559c64  = 0;
int      DAT_00559c68  = 0;
int      DAT_00559c70  = 0;
int      DAT_00559ce8  = 0;
DWORD    DAT_00559bec  = 0;
// DAT_083a42ac already defined at line ~652
DWORD    DAT_083a42d0  = 0;
char     DAT_083a2370[0x960]  = {};   // render-state pool (0x960 bytes, stride 0xc, 128 slots)
// 2026-04-28: pool de boids (fish/butterfly/bird flocking).
// FUN_0043e680 itera 10 entries × stride 0x1bc = 0x1180 bytes. Antes era una
// dirección absoluta del binario original (0x083a2e90); declarada como array
// real para que el flocking algoritmo funcione 1:1 con el original.
char     DAT_083a2e90[10 * 0x1bc] = {};
DWORD    DAT_083a2378  = 0;

// Server select + char menu new globals
char     DAT_083a7c64[64] = {};
DWORD    DAT_083a7c68 = 0;

// Sprite entity pool (FUN_00478c00)
DWORD    DAT_07abf634 = 0;
float   _DAT_005528dc = 0.25f;
float   _DAT_00552940 = 0.005f;
float   _DAT_00552944 = 0.02f;   // Entity_UpdateRender sin period scale (case 0x15d)
float   _DAT_00552948 = 0.65f;   // Entity_UpdateRender sin bias
float   _DAT_0055294c = 0.35f;   // Entity_UpdateRender sin amplitude
float   _DAT_00552acc = -0.02f;
float   _DAT_00552914 = 0.02f;

// UI_InGameMenu state machine (FUN_00514310)
DWORD    DAT_083a7c04  = 0;
DWORD    DAT_083a7c08  = 0;
char     DAT_083a7c09  = 0;
DWORD    DAT_083a7c0c  = 0;
DWORD    DAT_083a7c2c  = 0;
// DAT_083a42fc — ahora macro dentro de DAT_083a42f8 (dialog button rects)
DWORD    DAT_083a4324  = 0;
// see comment near DAT_083a44ea — sized as 7×0x26 message-box-custom buffer.
char     DAT_083a44c4[7 * 0x26] = {0};
char     DAT_07d566d0  = 0;   // fallback char name string
char     s__d___s_005580b0[] = "%d %s";
DWORD    DAT_005615dc  = 0;
int      DAT_07e11d74  = 0;
DWORD    DAT_07ea9804  = 0;
DWORD    DAT_07ea9808  = 0;
DWORD    DAT_07ea980c  = 0;
char     DAT_00559f5e  = 0;
DWORD    DAT_07d552e4  = 0;
DWORD    DAT_07eaa104  = 0;
DWORD    DAT_07eaa108  = 0;
DWORD    DAT_07eaa148  = 0;

// Chat globals added to globals.h in prior session (from FUN_004b14f0 analysis)
// DAT_00559bf1 = byte_559BF1 = toggle "Ver chat on/off" (tecla F2).
// FIX 2026-07-19: default IDA = 1 (verificado: byte en 0x559BF1 = 0x01), estaba en 0.
// Con 0, el chat normal (canal 3) se descartaba en DOS lugares:
//   - ChatLB_AddText:    `else if (kind == 3) return;`  → ni se agregaba a la lista
//   - ChatLB_renderLine: `if (!DAT_00559bf1 && msgType==3) return 0;` → no se dibujaba
// Resultado: los mensajes de chat de jugadores nunca aparecían.
DWORD    DAT_00559bf1  = 1;
DWORD    DAT_00559ce0  = 0;
char     DAT_05826adc[0x50] = {};

// Chat ring buffer (FUN_00480980 renderer, UIChatLogWindow_AddText writer).
// BUG-FIX (blue countdown / chat render): DAT_07df938b, DAT_07df948c y
// DAT_07df9494 son ALIASES a offsets 0x0B / 0x10C / 0x114 del slot 0 dentro de
// este mismo buffer en el binario original. Ghidra los recuperó como globales
// independientes; con almacenamiento separado, UIChatLogWindow_AddText escribe
// en DAT_07df9380 pero FUN_00480980 leía las variables sueltas (siempre 0) y
// el guard `msg[0] != '\0'` fallaba → el texto jamás aparecía. Se convierten
// a macros en globals.h que resuelven al byte/DWORD real del buffer.
char     DAT_07df9380[0x77 * 0x118]  = {0};
DWORD    DAT_07e11970  = 0;
DWORD    DAT_07e11974  = 0;
DWORD    DAT_07e11984  = 0;
DWORD    DAT_07e1198c  = 0;
DWORD    DAT_07e119f4  = 0;
DWORD    DAT_07e11a34  = 0;
char     DAT_07db870c  = 0;
DWORD    DAT_07ea840c  = 0;
DWORD    DAT_07ea8408  = 0;
// BUG-FIX 2026-04-28: macro hotkey table — 10 slots × 0x100 bytes.
// Era char (1 byte). OpenMacro escribe a [0x07e0ffc8 .. 0x07e109c8] = 2560 bytes.
char     DAT_07e0ffc8[10 * 0x100] = {};
char     DAT_005592dc  = 0;
DWORD    DAT_005592d8  = 0;
DWORD    DAT_005592d4  = 0;
char     DAT_07d5391c  = 0;
char     DAT_07d3d284  = 0;
char     DAT_07d3d3b0  = 0;
char     DAT_07d3cdd4  = 0;
// Chat command parser name buffers (FUN_004942e0)
char     DAT_07d3cb7c  = 0;
char     DAT_07d3cca8  = 0;
char     DAT_07d3c924  = 0;
char     DAT_07d3c6cc  = 0;
char     DAT_07d3bfc4  = 0;
char     DAT_07d3c0f0  = 0;
char     DAT_07d3d608  = 0;
char     DAT_07d3d734  = 0;
DWORD    DAT_07e11dac  = 0;
char     DAT_07eaa132  = 0;
DWORD    lpDefault_00583d88 = 0;
char     DAT_07d55410  = 0;
DWORD    DAT_07ea9848  = 0;
char     DAT_07eaa134  = 0;   // RepairEnable_0

// ── Chat / UI countdown timers ────────────────────────────────────────────────
int      DAT_00559cdc  = 300;
int      DAT_00559ce4  = 0x96;
// DAT_07e11dd0 = byte_7E11DD0: buffer de TEXTO del aviso periódico.
// MoveNotices (IDA 0x47FCB0) hace cada 300 frames `CreateNotice(byte_7E11DD0, 0)`,
// y CreateNotice hace `lstrlenA` + `strcpy` sobre él (hasta 256 bytes).
// FIX 2026-07-19: estaba declarado como UN SOLO char → esas lecturas se iban a
// los globals adyacentes y renderizaban basura como aviso dorado (el usuario veía
// mensajes dorados con una sola letra "D" que el cliente original NO mostraba).
// Ahora es un buffer propio, zero-init → aviso vacío (no se dibuja) hasta que
// se porte el handler que lo llena (probablemente el F3/E6 periódico del server,
// que trae los textos de evento tipo "Devil Square").
char     DAT_07e11dd0[256] = {0};
char     DAT_07e11dd8  = 0;
char     DAT_07e11ddc  = 0;
// Chat ring buffers
// 2026-05-04: enlarge to actual slot pool size — IDA loop in FUN_0047fce0
// walks 6 slots × 0x108 stride. Antes era single byte → AUTO-SKIP.
char     DAT_07db80d8[6 * 0x108]  = {0};   // system chat buffer (6 slots × 0x108)
// DAT_07db81dc was the flag byte alias inside slot 0 (+0x104). Now resolves
// to DAT_07db80d8[0x104] via macro in globals.h.
int      DAT_07e11d9c  = 0;
int      DAT_07e11da4  = 0;
// Text-extent work vars
LPSIZE   lpsz_07e113d0 = NULL;
int     _DAT_07e113d4  = 0;
DWORD    DAT_07e11d2c  = 0;
char     DAT_07e11cec  = 0;
// String constants
char     lpString_00559d3c = 0;
char     lpString_00559d40 = 0;
char     DAT_00559d4c  = 0;
char     DAT_00559d54  = 0;
char     DAT_00559d5c  = 0;

// ── Guild system (opcodes 0x90-0x99) ─────────────────────────────────────────
// DAT_07eaa117 — defined above (char, line 537)
// DAT_07eaa116 — defined above (char, line 536)
// DAT_07eaa128 — defined above (DWORD, line 544)
int      DAT_07eaa12c  = 0;
// DAT_07eaa108 — defined above (DWORD, line 894)
BYTE     DAT_07ea97c0[64] = {};
char     DAT_07e11d73  = 0;
BYTE     DAT_07d5b680  = 0;
BYTE     DAT_07d5b7ac  = 0;
BYTE     DAT_07d5c10c  = 0;
BYTE     DAT_07d5c238  = 0;
BYTE     DAT_07d5b8d8  = 0;
BYTE     DAT_07d6813c  = 0;
char     param_2_07d68268 = 0;
BYTE     DAT_07d58ea8  = 0;
char     param_2_07d58fd4 = 0;
DWORD    _DAT_00559f58 = 0;
WORD     DAT_00559f5c  = 0;
// _DAT_00559c94 — defined above (float, line 184)
// DAT_00559c84  — defined above (DWORD, line 180)
// DAT_00559c88  — defined above (DWORD, line 181)
// Guild UI state
// DAT_083a4324  — defined above (DWORD, line 882)
// DAT_083a44c4  — defined above (DWORD, line 883)
DWORD    DAT_083a42f8[10] = {};
// DAT_083a7c24  — defined above (DWORD, line 689)
// DAT_083a7c28  — defined above (DWORD, line 690)
int      DAT_083a7c30  = 0;
int      DAT_083a7c34  = 0;
DWORD    DAT_083a7af8  = 0;
DWORD    DAT_083a7afc  = 0;
WORD     DAT_083a7b00  = 0;
DWORD    DAT_083a7b04  = 0;
DWORD    DAT_083a7b08  = 0;
DWORD    DAT_083a7b0c  = 0;
char     DAT_07d59358  = 0;
char     param_2_07d59484  = 0;
char     DAT_07d5ba04  = 0;
char     DAT_07d5bfe0  = 0;
char *   PTR_DAT_005618a0  = nullptr;
char     param_2_005618a4  = 0;
char     param_2_005618a8  = 0;
char     param_2_005618b0  = 0;
char     param_2_005618b4  = 0;
char     lpString_07d68bc8[300] = {};
char     lpString_07d68cf4[300] = {};
char     lpString_07d68970[300] = {};
char     lpString_07d68a9c[300] = {};
char     param_2_07d68e20  = 0;
char     param_2_07d68f4c  = 0;
char     param_2_07d69078  = 0;

// ── Pre-compile missing globals (found by diff scan) ─────────────────────────
float    _DAT_0055264c = 2.0f;
float    _DAT_005528a0 = 0.0f;
float    _DAT_005529fc = 18.0f;
char     DAT_00558128[256]  = {};   // debug log format/data block
char     DAT_0055a7c4  = 1;         // compressed-assets flag (alias: g_tex_ext_mode); 1=Data mode (plain Data\ folder, no pak)
DWORD    _DAT_07e016f0 = 0;         // FPS tick timer (DWORD milliseconds)
char     DAT_083a1218[0x1158]  = {};  // Butterfles OBJECT array (10 entries × 0x1BC stride = 0x1158 bytes)
// Float aliases for view/projection matrix — MUST alias the array slots
// (see DAT_083a4140[12] above). Definidos como referencias para compartir
// memoria con DAT_083a4140[3/7/11] y evitar stomping.
float&   _DAT_083a414c = *reinterpret_cast<float*>(&DAT_083a4140[3]);
float&   _DAT_083a415c = *reinterpret_cast<float*>(&DAT_083a4140[7]);
float&   _DAT_083a416c = *reinterpret_cast<float*>(&DAT_083a4140[11]);
// _DAT_083a4284..428c — float aliases sobre el mismo array DWORD que escribe
// Camera_MouseRay (ver DAT_083a4284_arr arriba). Antes eran floats separados,
// causando que las lecturas via _DAT_xxx vieran 0/basura mientras los writes
// via DAT_xxx (DWORD) iban a otra memoria → mouse-ray rayO siempre incorrecto.
float&   _DAT_083a4284 = *reinterpret_cast<float*>(&DAT_083a4284_arr[0]);
float&   _DAT_083a4288 = *reinterpret_cast<float*>(&DAT_083a4284_arr[1]);
float&   _DAT_083a428c = *reinterpret_cast<float*>(&DAT_083a4284_arr[2]);
float    _DAT_083a42a4 = 0.0f;
float    _DAT_083a42a8 = 0.0f;
float    _DAT_083a4294 = 0.0f;  // projection center Y
// Bone transform scratch buffer (3x4 row-major matrix, 12 floats / 48 bytes).
// Must be CONTIGUOUS — AngleMatrix/BMD_Animation write all 12 floats starting
// at &DAT_06989c9c assuming array layout. Defining the three DAT_ names as
// separate floats would let MSVC scatter them anywhere → memory corruption.
// Instead: one backing array, individual names as references at offsets 0/5/10.
float    DAT_06989c9c_matrix[12] = {0};
float&   DAT_06989c9c  = DAT_06989c9c_matrix[0];   // matrix[0][0] @ +0x00
float&  _DAT_06989cb0  = DAT_06989c9c_matrix[5];   // matrix[1][1] @ +0x14
float&  _DAT_06989cc4  = DAT_06989c9c_matrix[10];  // matrix[2][2] @ +0x28
// ── Bone normal-intensity pool ───────────────────────────────────────────────
// Per-normal scalar intensity used for per-vertex lighting.
// Layout: [mesh * 15000 + normal] * float = 4 bytes stride.
// Capacity: 32 mesh slots × 15000 normals × 4 B = 1.92 MB.
// DAT_077e298c resolves via macro (globals.h) to a DWORD lvalue at offset 0.
char     g_BoneNormalBuf[32 * 15000 *  4] = {0};  // 1,920,000 bytes
// DAT_05846224 is an alias label 8 bytes into g_BoneVertexBuf (see globals.h).
// CharMenu_Build sub-function globals
int      DAT_07eaa158  = 0;
char     DAT_0055a400[64] = {};
char     DAT_0055a404[64] = {};
// Tabla de requisitos de stats @ 0x07E91528 — 12 filas x 10 ints (480 bytes).
// sub_4C2E20 escribe `dword_7E91528[10*i]`, `dword_7E91530[10*i]`, etc. (paso de
// fila = 40 bytes) y sub_4C2C10 la recorre con `v9 += 10`.
// 2026-08-21: estaba como `int[12]` mas nueve globals sueltos para las columnas,
// y el port hacia `&DAT_07e91530 + i * 4` sobre un int* (64 bytes de paso) —
// escritura fuera de rango de ~176 bytes cada vez que se arma el menu de
// personaje.  Mismo patron que BoneQuaternion.
int      DAT_07e91528[12 * 10] = {};
char     DAT_07d359d0  = 0;
int      DAT_00559fe0  = -1;

// UI_StatsPanel (FUN_0051af50) globals
float   _DAT_00552854 = 85.0f;
float   _DAT_00552a2c = 35.0f;
float   _DAT_00552ae4 = 0.03125f;
float   _DAT_00552d40 = 213.0f;
float   _DAT_00552d44 = 0.0078125f;
// DAT_00559c78 — defined above (DWORD, line 177); using 0xffffffff as initial value there
// DAT_00559c80 — defined above (DWORD, line 179)
// DAT_00559c8c — defined above (DWORD, line 182)
// DAT_00559c5c — defined above (char, line 858)
char     DAT_07e11d80  = 0;
char     DAT_07d29d24  = 0;
char     DAT_07d46e60  = 0;
char     DAT_07d486fc[300] = {};
char     DAT_07d48828  = 0;
char     DAT_07d48f30[300] = {};
char     DAT_07d493e0  = 0;
char     DAT_07d4950c  = 0;
char     DAT_07d49638  = 0;
char     DAT_07d49764  = 0;
char     DAT_07d5fa78  = 0;
char     DAT_07d698ac  = 0;
char     DAT_07d699d8  = 0;
char     DAT_07d69b04  = 0;
char     DAT_07d69c30  = 0;
// DAT_083a4304 — ahora macro dentro de DAT_083a42f8 (dialog button rects)
char     DAT_083a4348[10][1][38] = {};   // g_lpszDialogAnswer (10 x 38 = 380)
// DAT_083a7c08 — defined above (DWORD, line 877)
// DAT_083a7c09 — defined above (char, line 879)
// DAT_083a7c0c — defined above (DWORD, line 880)
// DAT_083a4124 — defined above (DWORD, line 634)
// _DAT_00552cac — defined above (float, line 141)
// Options submenu (0x96) toggle labels — rendered by UI_StatsPanel FUN_0051af50
// L152-175. El render llama crt_sprintf(buf, s__s_On_...) sin argumentos, por
// lo que el string debe ser literal (sin %s). Los 4 slots corresponden a los
// toggles m_bAutoAttack (DAT_00559c5c) y m_bWhisperSound (DAT_07e11d80), NO
// son master-sound/master-music toggles:
//   m_bAutoAttack    — auto-ataque en combate (Combat.cpp / Player_InputTick)
//   m_bWhisperSound  — notificación sonora de whisper (Font_Text.cpp config bits)
char     s__s_On_0056184c[32]  = "Auto-ataque : On";
char     s__s_Off_00561854[32] = "Auto-ataque : Off";
char     s__s_On_0056185c[32]  = "Susurros : On";
char     s__s_Off_00561864[32] = "Susurros : Off";
char     lpString_0056186c[32] = {};
// In the original binary these are 300-byte slots inside a GlobalText[] pool
// loaded from Data/Local/Text.bmd by sub_479830 (@0x00479830).
// Los placeholders en español son fallback — OpenTextData() los sobrescribe
// con GlobalText[381..385,388] al arrancar (ver src/Local/Text_Data.cpp).
// Layout: stride 300 (0x12c).
//
// Orden real verificado en UI_StatsPanel.cpp FUN_0051af50 state 0x6e:
//   07d45ba0 → SIEMPRE button 0 (Y=0x41 center)           = "Salir del juego"
//   07d45ccc → button 1 en charselect/ingame (Y=0x5f)     = "Ir a otro servidor"
//   07d45df8 → button 2 en ingame (Y=0x7d)                = "Ir a otro personaje"
//   07d46050 → button {3/2/1} ingame/charsel/login (Y variable) + título 0x96 = "Opciones"
//   07d45f24 → SIEMPRE último (final Y=0x7d/0x9b/0xb9)    = "Cancelar"
//   07d463d4 → estado 0x96 slot 3 (Y=0x7d)                = "Cancelar" (close submenu)
char     lpString_07d45ba0[300] = "Salir del juego";         // 0x6e btn 0 (Exit)
char     lpString_07d45ccc[300] = "Ir a otro servidor";      // 0x6e btn 1 ingame/charsel
char     lpString_07d45df8[300] = "Ir a otro personaje";     // 0x6e btn 2 ingame
char     lpString_07d45f24[300] = "Cancelar";                // 0x6e último (Close)
char     lpString_07d46050[300] = "Opciones";                // 0x6e btn Options + título 0x96
char     lpString_07d463d4[300] = "Cancelar";                // 0x96 submenu close
char     lpString_07d4662c[300] = {};
char     lpString_07d46758[300] = {};
char     lpString_07d46884[300] = {};
char     lpString_07d469b0[300] = {};
char     lpString_07d46adc[300] = {};
char     lpString_07d46c08[300] = {};
char     lpString_07d46d34[300] = {};
char     lpString_07d46f8c[300] = {};
char     lpString_07d470b8[300] = {};
char     lpString_07d471e4[300] = {};
char     lpString_07d47310[300] = {};
char     lpString_07d4743c[300] = {};
char     lpString_07d47568[300] = {};
char     lpString_07d47694[300] = {};
char     lpString_07d477c0[300] = {};
char     lpString_07d478ec[300] = {};
char     lpString_07d47a18[300] = {};
char     lpString_07d47b44[300] = {};
char     lpString_07d47c70[300] = {};
char     lpString_07d47d9c[300] = {};
char     lpString_07d47ec8[300] = {};
char     lpString_07d47ff4[300] = {};
char     lpString_07d48120[300] = {};
char     lpString_07d4824c[300] = {};
char     lpString_07d48378[300] = {};
char     lpString_07d484a4[300] = {};
char     lpString_07d485d0[300] = {};
char     lpString_07d48954[300] = {};
char     lpString_07d48a80[300] = {};
char     lpString_07d48bac[300] = {};
char     lpString_07d48cd8[300] = {};
char     lpString_07d48e04[300] = {};
char     lpString_07d4905c[300] = {};
char     lpString_07d49188[300] = {};
char     lpString_07d492b4[300] = {};
char     lpString_07d49890[300] = {};
char     lpString_07d499bc[300] = {};
char     lpString_07d49ae8[300] = {};
char     lpString_07d49e6c[300] = {};
char     lpString_07d49f98[300] = {};
char     lpString_07d4a0c4[300] = {};
char     lpString_07d4a1f0[300] = {};
char     lpString_07d4a31c[300] = {};
char     lpString_07d4a448[300] = {};
char     lpString_07d4a574[300] = {};
char     lpString_07d4a6a0[300] = {};
char     lpString_07d558c0[300] = {};
char     lpString_07d559ec[300] = {};
char     lpString_07d55b18[300] = {};
char     lpString_07d5f94c[300] = {};

// ── GL_State cached state ─────────────────────────────────────────────────────
int     DAT_083a412c = 0;
char    DAT_083a411d = 0;
char    DAT_083a4125 = 0;
int     DAT_083a42f4 = 0;
int     DAT_083a42f0 = 0;
// ── Float constants ───────────────────────────────────────────────────────────
float   _DAT_00552580 = 0.0f;
float   _DAT_00552850 = 400.0f;
float   _DAT_00552878 = 80.0f;
float   _DAT_00552ce8 = 0.017453292f;  // π/180
float   _DAT_00552ce0 = 0.5f;          // half-angle factor for EulerToQuat (FUN_004fa1d0).
                                       // IDA sub_4FA1D0 shows literal `a1[k] * 0.5` — quaternion
                                       // half-angle. Input Euler angles are already in RADIANS
                                       // (AngleMatrix path uses _DAT_00552ce8=π/180, different const).
                                       // Previously set to π/180 by mistake, which made every bone
                                       // rotation ≈ 0 → characters rendered with wrong orientation.
float   _DAT_00552cf0 = 1.0f;          // 1.0 (quaternion normalization)
float   _DAT_00552cf8 = 1.5707963f;    // π/2 (SLERP degenerate)
float   _DAT_00552d00 = 0.001f;        // SLERP near-parallel epsilon
float   _DAT_00552a1c = 1.0f;
// ── Effect pool ───────────────────────────────────────────────────────────────
// 2026-04-28: effect pool — Effect_TickAll itera 200 slots × 0x1bc bytes.
char    DAT_07b11670[200 * 0x1bc] = {};   // 200 slots: (0x07B27150-0x07B11670)/0x1bc (IDA bound unk_7B27178 = poolEnd+40)
// ── Monster_Data string literals ─────────────────────────────────────────────
char    s_Data2_MonsterSetBase2_txt_00561530[] = "Data2/MonsterSetBase2.txt";
char    s_Data2_Monster__0055e06c[] = "Data2/Monster/";
char    s_swordbasic_smd_0055e07c[] = "swordbasic.smd";
char    DAT_0055dff0 = 0;
char    DAT_0055df70 = 0;
char    DAT_0055dee0 = 0;
char    DAT_0055dec0 = 0;
char    DAT_0055de84 = 0;
char    DAT_0055de10 = 0;
FILE   *DAT_07d7806c = nullptr;
char    DAT_005580ac[] = "rb";  // binary read mode string at 0x005580ac
// bBuxCode @ 0x00558090 — la clave XOR de 3 bytes de BuxConvert_1 (0x401120),
// la que descifra Quest.bmd.  Leida del binario: FC CF AB — la misma que usa
// BuxConvert_0 (DAT_00559bb4), pero es otra copia en otra direccion.
// 2026-08-21: estaba declarada como UN char = 0, asi que FUN_00401120 hacia
// `(&DAT_00558090)[i % 3]` sobre un cero y dos bytes de globals vecinos: el
// script de quests quedaba sin descifrar.  De ahi que el nombre del NPC saliera
// equivocado (getMonsterName de un tipo basura) y el texto de la quest vacio.
char    DAT_00558090[3] = { (char)0xFC, (char)0xCF, (char)0xAB };
char    DAT_07cf1ef0[256] = {};   // TokenString (IDA @0x07CF1EF0) — buffer de GetToken (0x47A1F0)
char    DAT_00559088 = 0;
int     DAT_07d7807c = 0;
char    s_Data_Monster__0055ddf8[] = "Data/Monster/";
char    s_Monster_0055de08[] = "Monster";
char    s_Monster__0055ddec[] = "Monster/";
// ── Monster sound filename strings ───────────────────────────────────────────
char    DAT_0055ddd4 = 0;  char    DAT_0055ddbc = 0;
char    DAT_0055dda0 = 0;  char    DAT_0055dd84 = 0;
char    DAT_0055dd68 = 0;  char    DAT_0055dd50 = 0;
char    DAT_0055dd38 = 0;  char    DAT_0055dd1c = 0;
char    DAT_0055dd00 = 0;  char    DAT_0055dce4 = 0;
char    DAT_0055dccc = 0;  char    DAT_0055dcb0 = 0;
char    DAT_0055dc94 = 0;  char    DAT_0055dc7c = 0;
char    DAT_0055dc60 = 0;  char    DAT_0055dc44 = 0;
char    DAT_0055dc24 = 0;  char    DAT_0055dc04 = 0;
char    DAT_0055dbe4 = 0;  char    DAT_0055dbcc = 0;
char    DAT_0055dbb4 = 0;  char    DAT_0055db98 = 0;
char    DAT_0055db7c = 0;  char    DAT_0055db60 = 0;
char    DAT_0055db44 = 0;  char    DAT_0055db28 = 0;
char    DAT_0055db08 = 0;  char    DAT_0055dae8 = 0;
char    DAT_0055dac8 = 0;  char    DAT_0055dab0 = 0;
char    DAT_0055da98 = 0;  char    DAT_0055da7c = 0;
char    DAT_0055da5c = 0;  char    DAT_0055da3c = 0;
char    DAT_0055da24 = 0;  char    DAT_0055da0c = 0;
char    DAT_0055d9f0 = 0;  char    DAT_0055d9d4 = 0;
char    DAT_0055d9b8 = 0;  char    DAT_0055d9a0 = 0;
char    DAT_0055d988 = 0;  char    DAT_0055d96c = 0;
char    DAT_0055d950 = 0;  char    DAT_0055d934 = 0;
char    DAT_0055d91c = 0;  char    DAT_0055d904 = 0;
char    DAT_0055d8e8 = 0;  char    DAT_0055d8cc = 0;
char    DAT_0055d8b0 = 0;  char    DAT_0055d894 = 0;
char    DAT_0055d878 = 0;  char    DAT_0055d858 = 0;
char    DAT_0055d844 = 0;  char    DAT_0055d830 = 0;
char    DAT_0055d818 = 0;  char    DAT_0055d800 = 0;
char    DAT_0055d7e8 = 0;  char    DAT_0055d7cc = 0;
char    DAT_0055d7b0 = 0;  char    DAT_0055d794 = 0;
char    DAT_0055d778 = 0;  char    DAT_0055d758 = 0;
char    DAT_0055d738 = 0;  char    DAT_0055d718 = 0;
char    DAT_0055d6ac = 0;  char    DAT_0055d694 = 0;
char    DAT_0055d678 = 0;  char    DAT_0055d65c = 0;
char    DAT_0055d6fc = 0;  char    DAT_0055d6e0 = 0;
char    DAT_0055d6c4 = 0;  char    DAT_0055d644 = 0;
char    DAT_0055d62c = 0;  char    DAT_0055d610 = 0;
char    DAT_0055d5f4 = 0;  char    DAT_0055d5d8 = 0;
char    DAT_0055d5bc = 0;  char    DAT_0055d5a0 = 0;
char    DAT_0055d580 = 0;  char    DAT_0055d560 = 0;
char    DAT_0055d540 = 0;  char    DAT_0055d528 = 0;
char    DAT_0055d50c = 0;  char    DAT_0055d4f0 = 0;
char    DAT_0055d4d8 = 0;  char    DAT_0055d4c0 = 0;
char    DAT_0055d4a4 = 0;  char    DAT_0055d488 = 0;
char    DAT_0055d46c = 0;  char    DAT_0055d450 = 0;
char    DAT_0055d434 = 0;  char    DAT_0055d414 = 0;
char    DAT_0055d3f4 = 0;  char    DAT_0055d3d4 = 0;
char    DAT_0055d3bc = 0;  char    DAT_0055d3a4 = 0;
char    DAT_0055d388 = 0;  char    DAT_0055d36c = 0;
char    DAT_0055d350 = 0;  char    DAT_0055d338 = 0;
char    DAT_0055d320 = 0;  char    DAT_0055d304 = 0;
char    DAT_0055d2e8 = 0;  char    DAT_0055d2cc = 0;
char    DAT_0055d2b0 = 0;  char    DAT_0055d298 = 0;
char    DAT_0055d280 = 0;  char    DAT_0055d264 = 0;
char    DAT_0055d24c = 0;  char    DAT_0055d234 = 0;
char    DAT_0055d218 = 0;  char    DAT_0055d1fc = 0;
char    DAT_0055d1e0 = 0;  char    DAT_0055d1c8 = 0;
char    DAT_0055d1b0 = 0;  char    DAT_0055d194 = 0;
char    DAT_0055d178 = 0;  char    DAT_0055d15c = 0;
char    DAT_0055d144 = 0;  char    DAT_0055d12c = 0;
char    DAT_0055d114 = 0;  char    DAT_0055d0f8 = 0;
char    DAT_0055d0dc = 0;  char    DAT_0055d0c0 = 0;
char    DAT_0055d0a8 = 0;  char    DAT_0055d08c = 0;
// ── Monster s_ named sound strings ───────────────────────────────────────────
char    s_Data_Sound_iron1_wav_0055d074[] = "Data/Sound/iron1.wav";
char    s_Data_Sound_iron_attack1_wav_0055d058[] = "Data/Sound/iron_attack1.wav";
char    s_Data_Sound_jaikan1_wav_0055d040[] = "Data/Sound/jaikan1.wav";
char    s_Data_Sound_jaikan2_wav_0055d028[] = "Data/Sound/jaikan2.wav";
char    s_Data_Sound_jaikan_attack1_wav_0055d008[] = "Data/Sound/jaikan_attack1.wav";
char    s_Data_Sound_jaikan_attack2_wav_0055cfe8[] = "Data/Sound/jaikan_attack2.wav";
char    s_Data_Sound_jaikan_die_wav_0055cfcc[] = "Data/Sound/jaikan_die.wav";
char    s_Monster_bv01_2_jpg_0055cfb8[] = "Monster/bv01_2.jpg";
char    s_Monster_bv02_2_jpg_0055cfa4[] = "Monster/bv02_2.jpg";
char    s_Data_Sound_blood1_wav_0055cf8c[] = "Data/Sound/blood1.wav";
char    s_Data_Sound_blood_attack1_wav_0055cf6c[] = "Data/Sound/blood_attack1.wav";
char    s_Data_Sound_blood_attack2_wav_0055cf4c[] = "Data/Sound/blood_attack2.wav";
char    s_Data_Sound_blood_die_wav_0055cf30[] = "Data/Sound/blood_die.wav";
char    s_Data_Sound_death1_wav_0055cf18[] = "Data/Sound/death1.wav";
char    s_Data_Sound_death_attack1_wav_0055cef8[] = "Data/Sound/death_attack1.wav";
char    s_Data_Sound_death_die_wav_0055cedc[] = "Data/Sound/death_die.wav";
char    s_Data_Sound_mutant1_wav_0055cec4[] = "Data/Sound/mutant1.wav";
char    s_Data_Sound_mutant2_wav_0055ceac[] = "Data/Sound/mutant2.wav";
char    s_Data_Sound_mutant_attack1_wav_0055ce8c[] = "Data/Sound/mutant_attack1.wav";
char    s_Data_Sound_mOrcArcherAttack1_wav_0055ce68[] = "Data/Sound/mOrcArcherAttack1.wav";
char    s_Data_Sound_mOrcCapAttack1_wav_0055ce48[] = "Data/Sound/mOrcCapAttack1.wav";
char    s_Data_Sound_mCursedKing1_wav_0055ce2c[] = "Data/Sound/mCursedKing1.wav";
char    s_Data_Sound_mCursedKing2_wav_0055ce10[] = "Data/Sound/mCursedKing2.wav";
char    s_Data_Sound_mCursedKingDie1_wav_0055cdf0[] = "Data/Sound/mCursedKingDie1.wav";
char    s_Monster_iui02_tga_0055cddc[] = "Monster/iui02.tga";
char    s_Monster_iui03_tga_0055cdc8[] = "Monster/iui03.tga";
char    s_Data_Sound_mMolt1_wav_0055cd50[] = "Data/Sound/mMolt1.wav";
char    s_Data_Sound_mMoltAttack1_wav_0055cd34[] = "Data/Sound/mMoltAttack1.wav";
char    s_Data_Sound_mMoltDie_wav_0055cd1c[] = "Data/Sound/mMoltDie.wav";
char    s_Data_Sound_mMegaCrust1_wav_0055cdac[] = "Data/Sound/mMegaCrust1.wav";
char    s_Data_Sound_mMegaCrustAttack1_wav_0055cd88[] = "Data/Sound/mMegaCrustAttack1.wav";
char    s_Data_Sound_mMegaCrustDie_wav_0055cd68[] = "Data/Sound/mMegaCrustDie.wav";
char    s_Data_Sound_mAlquamosAttack1_wav_0055ccfc[] = "Data/Sound/mAlquamosAttack1.wav";
char    s_Data_Sound_mAlquamosDie_wav_0055cce0[] = "Data/Sound/mAlquamosDie.wav";
char    s_Data_Sound_mRainner1_wav_0055ccc4[] = "Data/Sound/mRainner1.wav";
char    s_Data_Sound_mRainnerAttack1_wav_0055cca4[] = "Data/Sound/mRainnerAttack1.wav";
char    s_Data_Sound_mRainnerDie_wav_0055cc88[] = "Data/Sound/mRainnerDie.wav";
char    s_Data_Sound_mPhantom1_wav_0055cc6c[] = "Data/Sound/mPhantom1.wav";
char    s_Data_Sound_mPhantomAttack1_wav_0055cc4c[] = "Data/Sound/mPhantomAttack1.wav";
char    s_Data_Sound_mPhantomDie_wav_0055cc30[] = "Data/Sound/mPhantomDie.wav";
char    s_Data_Sound_mDrakan1_wav_0055cc18[] = "Data/Sound/mDrakan1.wav";
char    s_Data_Sound_mDrakanAttack1_wav_0055cbf8[] = "Data/Sound/mDrakanAttack1.wav";
char    s_Data_Sound_mDrakanDie_wav_0055cbdc[] = "Data/Sound/mDrakanDie.wav";
char    s_Data_Sound_mPhoenix1_wav_0055cbc0[] = "Data/Sound/mPhoenix1.wav";
char    s_Data_Sound_mPhoenixAttack1_wav_0055cba0[] = "Data/Sound/mPhoenixAttack1.wav";
char    s_Data_Sound_mMagicSkull_wav_0055cb84[] = "Data/Sound/mMagicSkull.wav";
char    s_Data_Sound_mBullDie_wav_0055cb6c[] = "Data/Sound/mBullDie.wav";
char    s_Data_Sound_mBlackSkullDie_wav_0055cb4c[] = "Data/Sound/mBlackSkullDie.wav";
char    s_Data_Sound_mBlackSkullAttack_wav_0055cb28[] = "Data/Sound/mBlackSkullAttack.wav";
char    s_Data_Sound_mGhaintOrgerDie_wav_0055cb08[] = "Data/Sound/mGhaintOrgerDie.wav";
char    s_Data_Sound_mRedSkull_wav_0055caec[] = "Data/Sound/mRedSkull.wav";
char    s_Data_Sound_mRedSkullDie_wav_0055cad0[] = "Data/Sound/mRedSkullDie.wav";
char    s_Data_Sound_mRedSkullAttack_wav_0055cab0[] = "Data/Sound/mRedSkullAttack.wav";

// ── Effect_Create float constants ────────────────────────────────────────────
float   _DAT_005524ec = 180.0f;
float   _DAT_0055253c = 0.0174532924f;
float   _DAT_00552828 = -5.0f;
float   _DAT_00552830 = 9.0f;  // death particle anim frame upper bound
float   _DAT_00552844 = 45.0f;
float   _DAT_005528e8 = 500.0f;
float   _DAT_0055295c = 0.04f;
float   _DAT_00552960 = 2.0f;
float   _DAT_00552968 = 2.0f;
float   _DAT_00552970 = 280.0f;
float   _DAT_00552978 = 330.0f;
float   _DAT_00552980 = 130.0f;
float   _DAT_00552984 = -30.0f;
float   _DAT_00552988 = -15.0f;
float   _DAT_0055298c = -50.0f;
// ── Weather particle pool (40 × 0x1bc = 0x4560 bytes) ────────────────────────
// BUG-FIX 2026-05-04: backing buffer único. Antes los 30+ DAT_0839bc?? eran
// chars sueltos en BSS, lo que causaba AV cuando Weather_Update walked slots
// 1..39 con stride 0x1bc. Ver globals.h para los macros de field accessors.
alignas(16) char g_WeatherSlotPool[40 * 0x1bc] = {0};
// ── Weather float constants ───────────────────────────────────────────────────
float   _DAT_0055285c = 200.0f;
float   _DAT_0055286c = 360.0f;
float   _DAT_005528e4 = 40.0f;
float   _DAT_00552900 = 300.0f;
float   _DAT_00552ab4 = 600.0f;
float   _DAT_00552ab8 = 1000.0f;
float   _DAT_00552d24 = 1500.0f;
float   _DAT_00552d28 = 0.0003f;

// ── Camera / viewport globals ─────────────────────────────────────────────────
DWORD   DAT_0055a7b0   = 0;
DWORD   DAT_0055a7b4   = 0;
DWORD   DAT_0055a7b8   = 0;
float   _DAT_0055a7bc  = 0.0f;

// ── GL_State cache ────────────────────────────────────────────────────────────
char    DAT_083a411c   = 0;
char    DAT_083a411e   = 0;
float   DAT_083a4120   = 0.0f; // Map_InitRayCast running closest-hit ray-t (audit #7)
DWORD   DAT_083a42e8   = 0;

// ── Fog / projection globals ──────────────────────────────────────────────────
float   DAT_00561558   = 0.0f;
DWORD   DAT_0056155c   = 0;

// ── Perspective constant ──────────────────────────────────────────────────────
float   _DAT_00552d34  = 1.4f;

// ── Joint pool ────────────────────────────────────────────────────────────────
// 2026-04-28: joint pool — Joint_TickAll itera 500 slots × 0x9d8 bytes (~1.2MB).
char    DAT_07b27150[500 * 0x9d8] = {};   // 500 slots: (0x07C5AB30-0x07B27150)/0x9d8 = 1260000/2520 (IDA ItemDrop_Render: 0x7B27B08..0x7C5B4E8, ancla +0x9B8)

// ── Joint_Create float constants ─────────────────────────────────────────────
float   _DAT_00552a14  = -0.5f;
float   _DAT_00552658  = 8.0f;
float   _DAT_005526d8  = -1.0f;
float   _DAT_00552a44  = -0.0174532924f;
float   _DAT_00552a34  = -4.0f;
float   _DAT_005529ac  = -2.0f;

// ── Timer globals ─────────────────────────────────────────────────────────────
DWORD   DAT_05826e14   = 0;
DWORD   DAT_05826df0   = 0;
float   _DAT_005528a8  = 0.001f;  // 1/1000 ms-to-s
float   _DAT_00552898  = 5.0f;    // 5.0 second FPS window
// _DAT_00552890 — defined above (float, line 80)
DWORD   DAT_05826e00   = 0;
float   _DAT_0055979c  = 0.0f;    // delta time per frame
DWORD   DAT_05826dfc   = 0;
float   _DAT_05826df8  = 0.0f;    // smoothed FPS value

// ── Music.cpp globals ─────────────────────────────────────────────────────────
// m_MusicOnOff @ 0x055C9E3C — flag on/off de la musica (BOOL, NO un puntero a
// datos como decia la etiqueta vieja `lpData_055c9e3c`). Lo escribe Config_Load
// leyendo HKCU\SOFTWARE\Webzen\Mu\Config -> "MusicOnOff"; lo leen PlayMp3 y
// StopMp3. Default 0 (musica apagada) = fiel a IDA 0x0041E0A0 L81.
// Antes estaba partido en dos memorias (`g_MusicOn` escrita + `lpData_055c9e3c`
// leida) que nunca se veian, asi que la musica jamas arrancaba.
DWORD   m_MusicOnOff                = 0;
// Mp3FileName @ 0x055C9D04 — nombre del track en reproduccion. En el binario es
// un buffer de string (PlayMp3 hace strcpy del path completo, ~25 chars); estaba
// declarado como UN char, asi que la copia pisaba los globals de al lado.
char    DAT_055c9d04[256]           = {};
char    s_MuPlayer_00559110[]       = "MuPlayer";
char    s_MuPlayer_exe_00559154[]   = "MuPlayer.exe";
char    s_MuPlayer_exe__s_00559130[] = "MuPlayer.exe %s";
char    s_StopMp3_cmd_0055911c[]    = ">StopMp3<";
char    s_PlayMp3_cmd_00559140[]    = ">PlayMp3<";

// ── Sound_DS3D globals ────────────────────────────────────────────────────────
DWORD   DAT_0058443c   = 0;

// ── Net_Connect globals ───────────────────────────────────────────────────────
DWORD   DAT_055ca15c   = 0;
char    s_Failed_to_connect__00559688[] = "Failed to connect.";

// ── Net_PacketSession globals ─────────────────────────────────────────────────
// DAT_07ea8448 was a single DWORD; FILL_GRID(&DAT_07ea8448) and the
// Net_PacketSession reset loop both write 0x1100 bytes into it (= 64 slots ×
// 0x44 stride matching the IDA bound 0x7ea9548 - 0x7ea8448 = 0x1100). Sized
// properly to avoid heap corruption when in-game inventory grids fill.
BYTE    DAT_07ea8448[0x1100] = {0};
BYTE    DAT_07ea5b68[0x1FE0] = {0};   // see globals.h
BYTE    DAT_07ea9880[0x0880] = {0};
DWORD   DAT_07eaa0e8   = 0;
BYTE    DAT_07ea7b88[0x880] = {0};
// 2026-08-25: el comentario decia "MarkColor[16]" y estaba declarado como UN
// DWORD. `CreateGuildMark` (0x4F0100) escribe los 16 colores y
// `RenderGuildMark` (0x4F02F0) indexa `MarkColor[p5]` con p5 en 0..15, o sea 60
// bytes de desborde sobre el vecino en BSS. `MarkColor[1] = 0xFF000000` es
// justamente el valor que aparecia en el crash del editor de marca
// (0xC0000005 leyendo 0xFF000000 dentro del driver GL).
// El hueco hasta DAT_07e11f78 es de 68 bytes, asi que los 16 entran.
DWORD   DAT_07e11f34[16] = {0};  // MarkColor[16] — paleta de la marca (ARGB)
BYTE    DAT_07e11f78[0x880] = {0};
BYTE    DAT_07ea52d0[0x880] = {0};
BYTE    DAT_07ea7bc0[0x880] = {0};
BYTE    DAT_07e11fb0[0x880] = {0};
DWORD   DAT_055c9b7c   = 0;
DWORD   DAT_07eaa164   = 0;

// ── Entity_Init globals ───────────────────────────────────────────────────────
char    DAT_00559b74[] = "rb";  // binary read mode string at 0x00559b74
DWORD   DAT_00559b70   = 0;
char    DAT_00559b50[64] = {};
char    s__4d__4d_30__4d__4d__1_00559b58[] = "%4d/%4d[0] %4d/%4d[1]";

// ── Scene_CharPreview globals ─────────────────────────────────────────────────
DWORD   DAT_07e91354   = 0;
char    DAT_07e9136a   = 0;  // picked item durability/option byte
char    DAT_07e9136b   = 0;
DWORD   DAT_07ea5244   = 0;
DWORD   DAT_07ea5240   = 0;
char    DAT_07ea525b   = 0;

// ── Effect_Tick globals ───────────────────────────────────────────────────────
float   _DAT_00552aac  = 0.0833333358f;
// 2026-04-28: fade-effect pool — Effect_TickFade itera 40 slots × 0x1bc bytes.
char    DAT_07c74ec8[40 * 0x1bc] = {};
// 2026-04-28: flare effect pool — Effect_TickFlare itera 63 slots × 0x70 bytes.
char    DAT_07c82cdc[63 * 0x70] = {};
// 2026-04-28: spark-effect pool — Effect_TickSpark itera 100 slots × 0x70 bytes.
char    DAT_07c80128[100 * 0x70] = {};

// ── Terrain_Light globals ─────────────────────────────────────────────────────
DWORD   DAT_0839bc84   = 0;
float   _DAT_00552a08  = 0.003f;
// 2026-05-04: cb60c/cb610 and 0828b60c/610 are NOT separate globals — in the
// original binary they're the 2nd/3rd DWORDs of slot 0 of cb608/0828b608.
// Code uses `(&DAT_081cb60c)[iVar2*3]` to access slot iVar2's 2nd field, and
// `(char*)&DAT_081cb60c + iVar2*12` to write to it.  We promote them to
// macros that project into the actual buffer.  See globals.h.

// ── Scene_Resources string literals ──────────────────────────────────────────
// Originally at .rdata in the binary; their content must match the actual
// asset file basenames on disk or Scene_LoadAccountResources / ...CharSelectResources
// construct malformed paths (e.g. "Data\\Object1\\01.bmd" instead of "Ship01.bmd").
// Ship/Logo/Face are BMD basenames used by FUN_005060b0;
// the three SMD entries (Korean-named background/face assets) are only consumed
// by FUN_00505e90 which is stubbed in this port — kept as empty strings so any
// sprintf(%s, "") produces harmless paths without crashing.
char    DAT_0055e834[8]   = "Ship";          // → Data\Object1\Ship01.bmd (login ship)
char    DAT_005606ac[8]   = "Logo";          // → Data\Logo\Logo0N.bmd   (login logos 1..4)
char    DAT_005607c0[8]   = "Face";          // → Data\Logo\Face0N.bmd   (char-select faces)
char    DAT_005606e8[32]  = "background.smd";  // SMD basename, slot 0xa0 (stub)
char    DAT_005606d0[32]  = "background2.smd"; // SMD basename, slot 0xa1 (stub)
char    DAT_005607c8[32]  = "swordsman_face.smd"; // SMD basename, slot 0xad (stub)
char    DAT_0056085c[32]  = "wizard_face.smd";    // SMD basename, slot 0xaa (stub)
char    s_Logo_Webzenlogo_jpg_005606b0[]         = "Logo/Webzenlogo.jpg";
char    s_Logo_Title_jpg_005606bc[]              = "Logo/Title.jpg";
char    s_Interface_GFx_Interface_jpg_005607c4[] = "Interface/GFx/Interface.jpg";
char    s_Interface_GFx_Interface2_jpg[]         = "Interface/GFx/Interface2.jpg";
char    s_Data_Interface_jpg[]                   = "Data/Interface.jpg";
char    s_Data_Interface2_jpg[]                  = "Data/Interface2.jpg";
char    s_Object1_Object1_jpg[]                  = "Object1/Object1.jpg";
char    s_warrior_bmd[]                          = "warrior.bmd";
char    s_main_bmd[]                             = "main.bmd";
char    s_fairy_bmd[]                            = "fairy.bmd";

// ── Scene_Resources strings ───────────────────────────────────────────────────
char    s_Logo_0Account_new_tga_005607a8[]       = "Logo\\0Account_new.tga";
char    s_Logo_0On_Botton_jpg_00560794[]         = "Logo\\0On_Botton.jpg";
char    s_Logo_0On_Botton2_jpg_0056077c[]        = "Logo\\0On_Botton2.jpg";
char    s_Logo_0Text_Box_jpg_00560768[]          = "Logo\\0Text_Box.jpg";
char    s_Logo_0New_Account01_tga_00560750[]     = "Logo\\0New_Account01.tga";
char    s_Logo_0New_Account02_tga_00560738[]     = "Logo\\0New_Account02.tga";
char    s_Logo_0Box_jpg_00560728[]               = "Logo\\0Box.jpg";
char    s_Interface_Progress_Back_jpg_0056070c[] = "Interface\\Progress_Back.jpg";
char    s_Interface_Progress_jpg_005606f4[]      = "Interface\\Progress.jpg";
char    s_Data2_Object1__0055f120[]              = "Data2\\Object1\\";
char    s_ship_smd_0055ee54[]                    = "ship.smd";
char    s_Data2_Logo__005606dc[]                 = "Data2\\Logo\\";
char    s_mu_smd_005606c8[]                      = "mu.smd";
char    s_sun_smd_005606b4[]                     = "sun.smd";
char    s_Data_Object1__0055f360[]               = "Data\\Object1\\";
char    s_Data_Logo__005606a0[]                  = "Data\\Logo\\";
char    s_Object1__0055f354[]                    = "Object1\\";
char    s_Logo__00560698[]                       = "Logo\\";
char    s_Logo_Interface01_tga_00560a4c[]        = "Logo\\Interface01.tga";
char    s_Logo_Interface02_tga_00560a34[]        = "Logo\\Interface02.tga";
char    s_Logo_Interface03_tga_00560a1c[]        = "Logo\\Interface03.tga";
char    s_Logo_Interface04_tga_00560a04[]        = "Logo\\Interface04.tga";
char    s_Logo_New_Character201_tga_005609e8[]   = "Logo\\New_Character201.tga";
char    s_Logo_New_Character202_jpg_005609cc[]   = "Logo\\New_Character202.jpg";
char    s_Logo_Delete01_tga_005609b8[]           = "Logo\\Delete01.tga";
char    s_Logo_Delete02_jpg_005609a4[]           = "Logo\\Delete02.jpg";
char    s_Logo_Ok01_tga_00560994[]               = "Logo\\Ok01.tga";
char    s_Logo_Ok02_jpg_00560984[]               = "Logo\\Ok02.jpg";
char    s_Logo_New_Character01_tga_00560968[]    = "Logo\\New_Character01.tga";
char    s_Logo_New_Character02_tga_0056094c[]    = "Logo\\New_Character02.tga";
char    s_Logo_New_Character_Cancel_jpg_0056092c[] = "Logo\\New_Character_Cancel.jpg";
char    s_Logo_New_Character_Ok_jpg_00560910[]   = "Logo\\New_Character_Ok.jpg";
char    s_Logo_New_Character001_jpg_005608f4[]   = "Logo\\New_Character001.jpg";
char    s_Logo_New_Character002_jpg_005608d8[]   = "Logo\\New_Character002.jpg";
char    s_Logo_New_Character003_jpg_005608bc[]   = "Logo\\New_Character003.jpg";
char    s_Logo_New_Character004_jpg_005608a0[]   = "Logo\\New_Character004.jpg";
char    s_main_smd_00560894[]                    = "main.smd";
char    s_warrior_smd_00560828[]                 = "warrior.smd";
char    s_fairy_smd_0055c438[]                   = "fairy.smd";

// ── Additional misc globals ───────────────────────────────────────────────────
// DAT_07e016f0 — same address as _DAT_07e016f0 above (line 997); alias defined in globals.h
BYTE    DAT_0055984c[9] = {};  // bone index table for glitter effect
// 2026-04-28: tooltip/bubble pool — UI_TickTooltips (FUN_004821a0) itera 26
// slots × 0x254 bytes (stride 0x95 DWORDs). Antes era DWORD simple → AV.
// 2026-07-19: `DAT_07e01720` YA NO es un array propio — es el pool de burbujas
// de chat proyectado a +40. Ver DAT_07e016f8 más abajo. La declaración vieja
// (26 slots sueltos) convivía con `DAT_07e016f8` como char de 1 byte, y
// CreateChat caminaba ESE char con stride 596 → AV.
// Key-state table para FUN_0047ec20 (PressKey / Key_IsJustPressed).
// La función indexa como `*(DWORD*)((char*)&DAT_07e118ec + vkey*4)`, o sea
// 256 entradas DWORD (1024 bytes) — una por código VK. En IDA es una tabla
// al símbolo dword_7E118EC. Si se deja como DWORD single, cualquier tecla
// con vkey>=1 pisa globals adyacentes y el edge-trigger queda corrupto
// (ESC=27 pisa 108 bytes hacia adelante).
DWORD   DAT_07e118ec[256] = {0};
DWORD   DAT_07e11aac   = 0;
DWORD   DAT_07e11ab0   = 0;
DWORD   DAT_07e11ab4   = 0;
DWORD   DAT_07e11ab8   = 0;
DWORD   DAT_07e11da8   = 0;
DWORD   DAT_07e12858   = 0;
DWORD   DAT_07ea5284   = 0;
DWORD   DAT_07ea5288   = 0;
DWORD   DAT_07ea9844   = 0;
// 2026-04-28: Ambient particle pool — Ambient_ParticleUpdate itera 10 slots
// × 0x1bc bytes (stride 0x6f DWORDs). Antes era DWORD simple → AV al spawnear.
char    DAT_083a2f78[10 * 0x1bc] = {};
float   _DAT_00590af0  = 0.0f;
float   DAT_00590af4   = 0.0f;
float   DAT_00590af8   = 0.0f;
float   DAT_00590afc   = 0.0f;  // cloth wind Z (flt_590AFC; el binario nunca lo escribe)
float   _DAT_00559068  = 9.8f;      // gravedad   (flt_559068, leído del binario)
float   _DAT_0055906c  = 0.0025f;   // dt fijo    (flt_55906C, leído del binario)
LPBYTE  lpData_055ca044 = NULL;
char   *lpText_07d63aec = NULL;
// BUG-FIX 2026-07-17: strings reservados de sub_513570 (name-filter). En el binario
// original son patrones bloqueados (espacio, DBCS coreano, punto); estaban en 0 (string
// vacío) → FindText(nombre,"") devuelve 1 → TODO nombre se rechazaba con "palabras
// restringidas". Valores reales de IDA (bytes little-endian): 0x561740=" ",
// 0x561744="\xA1\xA1", 0x561748=".", 0x56174c="\xA1\xA4", 0x561750="\xA1\xAD".
DWORD   DAT_00561740   = 0x20;      // " "
DWORD   DAT_00561744   = 0xA1A1;    // "\xA1\xA1" (DBCS)
DWORD   DAT_00561748   = 0x2E;      // "."
DWORD   DAT_0056174c   = 0xA4A1;    // "\xA1\xA4" (DBCS)
char    s_WEBZEN_0056176c[]  = "WEBZEN";
char    s_WebZen_0056175c[]  = "WebZen";
char    s_Webzen_00561764[]  = "Webzen";
DWORD   DAT_00561750   = 0xADA1;    // "\xA1\xAD" (DBCS)
char    s_Webzen_00561754[]  = "Webzen";

// ── Chat.cpp missing globals ──────────────────────────────────────────────────
char    DAT_07db8714    = 0;
char    DAT_07db8716    = 0;
DWORD   DAT_07db8718    = 0;   // PIN data base for char-select second-password
// Word-filter table (banned chat keywords), 1000 entries × 20 bytes = 20000 bytes.
// Loaded at boot by FUN_00479b30("Data/Local/Filter.bmd") — 20000 bytes XOR'd
// with BuxConvert_0 (3-byte key FC CF AB), preceded by a 4-byte ring checksum
// (seed 0x7cfa00, magic 15997).  Empty first byte terminates the valid range.
char    DAT_07d73104[20000] = {};
// GlobalText[] — 1000 × 300 byte localized string table, loaded at boot from
// Data/Local/Text.bmd by OpenTextData().  Each row holds one null-terminated
// entry; row index matches the original binary's string IDs.
char    GlobalText[GLOBALTEXT_ROWS][300] = {};   // ver nota en globals.h
// DAT_07d4b4b0/5dc = GlobalText[457]/[458] — ahora macros en globals.h (name-filter).
DWORD   DAT_07db8070    = 0;
int     DAT_07d78074    = 0;         // command table A count (name-filter)
int     DAT_07d78070    = 0;         // command table B count (word-filter)
// Name-filter table (banned character names), 1000 entries × 20 bytes = 20000 bytes.
// Loaded at boot by FUN_00479e50("Data/Local/FilterName.bmd") — seed 0x578200,
// magic 11201, same BuxConvert_0 XOR cipher.
char    DAT_07d27610[20000] = {};
float   _DAT_00552950   = 2.5f;     // lightning speed constant

// ── GameGuard globals (GameGuard_Init2.cpp) ───────────────────────────────────
DWORD   DAT_083bbb0c    = 0;    // GG child process ID
HANDLE  DAT_083bbb10    = NULL; // GG child process handle
DWORD   DAT_083bbaf0    = 0;    // GG error state flag
DWORD   DAT_083bbaf4    = 0;    // main thread ID
DWORD   DAT_083bbaf8    = 10000;// event timeout (ms)
// DAT_083bbb14 — defined above (DWORD, line 725)
char    DAT_00562e5c     = 0;
char    DAT_00562e58     = 0;
char    lpString1_083bb9e0[0x104] = {0};  // GG log/data directory path
char    lpSubBlock_005592d0[4]    = "\\"; // path separator
char    DAT_005633dc     = 0;
char    DAT_005633cc     = 0;
char    DAT_00563338     = 0;
char    DAT_00563308     = 0;
char    DAT_005632fc     = 0;
char    DAT_005632e0     = 0;
char    DAT_005632d0     = 0;
char    DAT_005632ac     = 0;
char    DAT_0056329c     = 0;
char    DAT_00563298     = 0;
char    DAT_00563284     = 0;
char    DAT_00563270     = 0;
char    DAT_00563254     = 0;
char    DAT_0056323c     = 0;
char    DAT_0056322c     = 0;
char    DAT_0056321c     = 0;
char    DAT_00563218     = 0;
char    DAT_00563208     = 0;
char    DAT_00563204     = 0;
char    DAT_00563200     = 0;
char    DAT_005631fc     = 0;
char    DAT_005631f8     = 0;
char    DAT_005631e0     = 0;
char    DAT_005631dc     = 0;
char    DAT_005631d0     = 0;
char    DAT_005631b4     = 0;
char    DAT_00563198     = 0;
char    DAT_0056318c     = 0;
char    PTR_DAT_00563180 = 0;
char    DAT_00563170     = 0;
char    DAT_00563158     = 0;
char    DAT_0056314c     = 0;
char    DAT_0056313c     = 0;
char    DAT_00563128     = 0;
char    PTR_DAT_00563110 = 0;
char    DAT_005630f8     = 0;
char    DAT_005630e4     = 0;
char    DAT_005630d4     = 0;
char    DAT_005630d0     = 0;
char    DAT_005630c0     = 0;
char    DAT_005630a4     = 0;
char    DAT_005630a0     = 0;
char    DAT_0056308c     = 0;
char    DAT_00563088     = 0;
char    DAT_00563064     = 0;
char    DAT_00563040     = 0;
char    DAT_0056301c     = 0;
char    DAT_00563018     = 0;
char    DAT_00562ff8     = 0;
char    DAT_00562ff0[8]  = {0};
char    DAT_00562fdc     = 0;
char    DAT_00562fc4     = 0;
char    DAT_00562fc0     = 0;
char    DAT_00562fa8     = 0;
char    DAT_00562f98     = 0;
char    DAT_00562f94     = 0;
char    DAT_00562f7c     = 0;
char    DAT_00562f60     = 0;
char    DAT_00562f5c     = 0;
char    DAT_00562f3c     = 0;
char    DAT_0056337c     = 0;

// ── Misc.cpp / Entity gravity globals ────────────────────────────────────────
float   _DAT_00552570   = -0.2f;   // gravity min clamp
// 2026-08-21: los dos valían un valor inventado.  Leídos del binario:
// 0x5527D0 = 40 C0 00 00 → 6.0f  y  0x552A28 = C1 20 00 00 → -10.0f.
float   _DAT_005527d0   = 6.0f;    // MoveItems: decaimiento de la velocidad Z por frame
float   _DAT_00552a28   = -10.0f;  // MoveItems: giro del item mientras cae


// ── Terrain map globals (FUN_004f6f90, FUN_004ffe70, FUN_004f7270) ────────────
// DAT_083a0218 is now a macro alias into g_ObjectBucketGrid[0] (see line ~811).
float   _DAT_00552b70 = 0.00392156886f; // height scale

// ── Terrain tile pick / normal buffer globals ─────────────────────────────────
// Estos son ALIASES dentro de g_TilePickBuf[12] declarado arriba para garantizar
// que los 12 floats están contiguos (consumidos como vec3 corners por
// glVertex3fv y por las cross-product helpers).
DWORD   DAT_07eab1fc  = 0;   // tile pick result flag
DWORD   DAT_07e11d44  = 0;   // combat target select sub-mode

// ── Model slot index globals ──────────────────────────────────────────────────
DWORD   DAT_083a4104  = 0;
DWORD   DAT_083a4108  = 0;

// ── Terrain culling globals ───────────────────────────────────────────────────
short   _DAT_0838b60a = 0;
DWORD   _DAT_0838b614 = 0;
DWORD   _DAT_0838b710 = 0;
// _DAT_005528a8 already defined above (terrain: reuse same address)

// ── Model loader format strings ───────────────────────────────────────────────
char    s__s_bmd_0055a7f8[]    = "%s.bmd";
char    s__s0_d_bmd_0055a7ec[] = "%s0%d.bmd";
char    s__s_d_bmd_0055a7e0[]  = "%s%d.bmd";

// ── Item/Skill/Gate data loader globals ──────────────────────────────────────
int     DAT_07cf1ff0    = 0;       // item record shadow array base
int     DAT_07cf1ff8    = 0;       // skill record shadow array base
int     DAT_07d29d20    = 0;       // skill/gate data array base
DWORD   DAT_07cf5600    = 0;       // gate data array base — malloc'd in WinMain
// dialog data array — original ocupaba 0x07cf5608..0x07d27608 (0x32000 bytes).
// Declaramos un buffer real de ese tamaño en DWORDs para que &DAT_07cf5608 apunte a memoria válida.
DWORD   DAT_07cf5608[(DIALOG_SCRIPT_COUNT * 0x400) / 4] = {};   // g_DialogScript (200 x 0x400 = 200 KB)
int     DAT_07d78078    = 0;       // NPC name count (EditMonsterNumber)
// MonsterScript / DAT_07cf2000 / DAT_07cf2001 son la MISMA tabla en 0x07CF2000
// (verificado en IDA: getMonsterName lee `MonsterScript` = 0x07CF2000, y
// NPCName_Load escribe `&DAT_07cf2000` = 0x07CF2000).  Antes estaban declarados
// como TRES globals de 1 byte separados → el loader escribia en uno y
// getMonsterName leia de otro → nombres de NPC/mob vacios.  Ahora es UNA tabla
// real: N entradas × 0x36 (Type[0], Name[1..32], Level, Attribute).
// Definida abajo como MonsterScript[]; DAT_07cf2000/2001 son macros a ella.
void   *ppvBits_055c9e4c = nullptr; // DIB bitmap pointer
DWORD   DAT_01c5e200    = 0x01c5e200;  // item BMD checksum seed A (literal = su propia dirección original)
DWORD   DAT_00b43000    = 0x00b43000;  // skill BMD checksum seed B
// FUN_00479910 (BuxConvert_0) indexes (&DAT_00559bb4)[i % 3] — so this must be
// a 3-byte array, not a scalar.  Previously declared as a single char, which
// made the XOR cipher pick up whatever two bytes happened to be adjacent in
// memory, scrambling every Text.bmd / Filter.bmd / Dialog.bmd decode.
char    DAT_00559bb4[3] = { (char)0xFC, (char)0xCF, (char)0xAB };
// Error message format strings
char    s__s___File_not_exist__00558094[] = "%s - File not exist.";
char    s__s___File_corrupted__00559bd4[] = "%s - File corrupted.";
// File open mode strings
char    DAT_005597d4[] = "wb";  // binary write mode at 0x005597d4

// ── SecondPassword Screen2 entity list globals ────────────────────────────────
DWORD   DAT_07e11e80  = 0;   // char-select entity name table base (stride 0x24)
DWORD   DAT_07ea5b24  = 0;   // Screen2 panel origin X
DWORD   DAT_07ea5b28  = 0;   // Screen2 panel origin Y

// ── SecondPassword UI sub-handler globals ─────────────────────────────────────
DWORD   DAT_07eaa0c8  = 0;   // SecondPassword dialog origin X (pixel)
DWORD   DAT_07eaa0cc  = 0;   // SecondPassword dialog origin Y (pixel)
DWORD   DAT_07eaa140  = 0;   // SecondPassword timeout/retry counter
DWORD   DAT_07eaa131  = 0;   // SecondPassword checkbox/toggle state
DWORD   DAT_07eaa138  = 0;   // RepairEnable (low byte)
DWORD   DAT_07ea5290  = 0;   // SecondPassword alt-panel origin X
DWORD   DAT_07ea528c  = 0;   // SecondPassword alt-panel origin Y
char    DAT_07d544d4  = 0;   // error string – case 0 wrong PIN
char    DAT_07eaa1a0  = 0;   // UI message label A
char    DAT_07d54600  = 0;   // error string – auth fail
char    DAT_07eaa198  = 0;   // UI message label B
char    DAT_07d55c44  = 0;   // error string – case 0xfffffff8/0xfffffffe
char    DAT_07eaa19c  = 0;   // UI message label C
int     DAT_0055a3f8  = 0;   // auth mode param A
int     DAT_0055a3fc  = 0;   // auth mode param B

// ── SecondPassword Screen5/6/7 additional globals ────────────────────────────
DWORD   DAT_07eaa120  = 0;   // SecondPassword_Screen5 mode
char    DAT_07eaa0dc  = 0;   // SecondPassword selected grid index
// 2026-08-25: NO son "PIN entry" — esa etiqueta mentia. Son los buffers del
// editor de creacion de GUILD, y estaban declarados como escalares de 1-4 bytes
// mientras el codigo los recorre como arrays:
//   RenderGuildCreation lee `mark[gx + gy*8]` con gx,gy en 0..7 -> 64 bytes
//   sobre un `char`, o sea 63 bytes de desborde en CADA frame del editor.
// Layout del binario (contiguo, verificado contra el vecino DAT_07ea5240 que
// deja 75 bytes de espacio):
//   0x7EA51EC  GuildName[8]   (IDA lo lee como 2 DWORDs: +0 y +4)
//   0x7EA51F5  GuildMark[64]  (1 byte por celda de la grilla 8x8)
char    DAT_07ea51ec[8]  = {0};   // GuildName
char    DAT_07ea51f5[64] = {0};   // GuildMark (grilla 8x8)
float  _DAT_00552c20  = 425.0f; // Screen5 button X upper bound
float  _DAT_00552c1c  = 33.0f; // Screen5 button height
float  _DAT_00552c28  = 210.0f; // Screen5 button Y base
char    DAT_07d6b724  = 0;   // Error message: "no item in slot"
char    DAT_07d685ec  = 0;   // Error message: "invalid slot"
short   DAT_00559f5a  = 0;   // second-password level check B
int     DAT_00559f80  = 0;   // level threshold array base
int     DAT_00559f84  = 0;   // level threshold array upper
int     DAT_00559f60  = 0;   // level range lower array
int     DAT_00559f64  = 0;   // level range upper array
// DAT_07ea7b88 — declared above as DWORD (line 1404)
char    DAT_07ea5b30  = 0;   // second-password char-slot list base

// ── BMD_DrawMesh / BMD_DrawBoneSlot_Anim buffers ─────────────────────────────
// LightTransform RGB buffer (float R,G,B per vertex). Stride 12 B per vertex.
// Capacity: 32 mesh slots × 15000 verts × 12 B = 5.76 MB.
// Backed by g_BoneLightBuf; DAT_060db65c is a 1-byte-typed lvalue macro
// (globals.h) so that &DAT_060db65c + k yields byte-level arithmetic, matching
// the Ghidra-decompiled accesses (&DAT_060db65c + iVar2 * 12, + meshIdx * 180000).
char    g_BoneLightBuf[32 * 15000 * 12] = {0};  // 5,760,000 bytes
// Chrome UV scratch (per-mesh, {U,V} pairs indexed by vertex, up to 15000 verts).
// Written + read within the same mesh by BMD_DrawMesh chrome pre-loop / tri loop.
float   g_ChromeUVBuf[15000 * 2] = {0};   // 120,000 bytes
// Transformed-normal buffer for chrome env-map (3 floats/normal, 180000 B/mesh,
// 32 mesh slots — parallel to g_BoneVertexBuf). Written by FUN_004404e0's normal
// loop, read by BMD_DrawMesh chrome pre-loop. Sin esto, las normales quedaban en
// cero → el env-map chrome colapsaba a un texel → armas/glow chrome como barra.
char    g_BoneChromeNormalBuf[32 * 15000 * 12] = {0};  // 5,760,000 bytes
float  _DAT_00552544  = 0.99f; // alpha threshold (alpha < this → use alpha channel path)
float  _DAT_005528c0  = 0.00024f; // chrome U scale factor
float  _DAT_005528c4  = 0.007f; // sin period scale for vertex deformation
float  _DAT_00552644  = 28.0f; // sin amplitude for vertex deformation

// ── MoveEffect (FUN_00466ad0) constants ──────────────────────────────────────
float  _DAT_005524a8  = 27.0f;
float  _DAT_00552864  = 270.0f;
float  _DAT_00552990  = -0.4f;
float  _DAT_00552994  = 250.0f;
float  _DAT_00552998  = 0.0031f;
float  _DAT_0055299c  = 15.37f;
float  _DAT_005529a0  = 2.0943952f;
float  _DAT_005529a4  = 0.17f;
float  _DAT_005529a8  = 24.0f;
float  _DAT_005529b0  = -200.0f;
float  _DAT_005529b4  = 1.1f;
float  _DAT_005529b8  = 32.0f;
float  _DAT_005529c4  = 700.0f;
float  _DAT_005529cc  = 0.015f;
float  _DAT_005529d0  = -0.01f;
float  _DAT_005529d4  = 0.333333343f;
float  _DAT_005529d8  = 0.0f;
float  _DAT_005529e8  = 3.1249046e-25f;
float  _DAT_005529f0  = -5773911433377342244919403216988274688.0f;
float  _DAT_005529f8  = 72.0f;
float  _DAT_00552a04  = 24.0f;
float  _DAT_00552a0c  = 0.04f;
float  _DAT_00552a18  = 1.8f;
float  _DAT_00552a20  = 0.0333333351f;
float  _DAT_00552938  = 70.0f;  // MoveJoint trig scale (mode 1)
float  _DAT_00552a54  = 0.18f;  // MoveJoint particle angle scale
float  _DAT_00552a8c  = 0.6666667f;  // MoveJoint color fade rate A
float  _DAT_00552a90  = 0.7692308f;  // MoveJoint color fade rate B
float  _DAT_00552a94  = 0.03065f;  // MoveJoint trig freq A
float  _DAT_00552a98  = 0.024f;  // MoveJoint trig scale B
float  _DAT_00552aa4  = 0.025f;  // MoveJoint HP-bar scale factor
float  _DAT_00552a9c  = 0.0613f;  // FUN_00473d90 ring trig scale X
float  _DAT_00552aa0  = 0.048f;  // FUN_00473d90 ring trig scale Y
float  _DAT_00552aa8  = 0.1113f;  // FUN_00473d90 ring trig scale Z
char   DAT_00559b78[7] = {};   // MoveEffect byte lookup table A
char   DAT_00559b7f[7] = {};   // MoveEffect byte lookup table B

// ── Map_LoadObjectModels (0x0050c4d0) ─────────────────────────────────────────
FILE*  DAT_083a40fc       = nullptr;      // file handle for custom-map object list
char   DAT_083a3ff4[256]  = {};           // token read buffer
char   DAT_083a4100       = 0;            // Lorencia models-loaded flag
// Icarus water-tile name table — 32 entries × 0x38 bytes (= 0x700 bytes total).
// Was 1 byte → Scene_Objects.cpp case 7 (Icarus map) wrote 32 filename strings
// past the global → heap corruption on map enter. IDA bound: 0x83a91d8 - 0x83a8ad8 = 0x700.
char   DAT_083a8ad8[32 * 0x38] = {0};

// ── BMD_SkinUpdate (0x0040b630) ──────────────────────────────────────────────
int    DAT_00590c10       = 0;            // skinned vertex count
float  DAT_055c4068       = 0.0f;         // bone world-vertex array base
float  DAT_00593008       = 0.0f;         // source vertex position array base
short  DAT_00590c32       = 0;            // parent-bone index array base
float  _DAT_0055259c      = 1.0f;         // global mesh scale factor
float  DAT_055c4038       = 0.0f;         // bone world-position matrix base
float  DAT_055c4098       = 0.0f;         // bone world-pos X
float  DAT_055c409c       = 0.0f;         // bone world-pos Y
float  DAT_055c40a0       = 0.0f;         // bone world-pos Z
int    DAT_00593968       = 0;            // triangle count
BYTE   DAT_0059396a       = 0;            // material name array base (stride 0x20)
float  DAT_00608c70       = 0.0f;         // triangle vertex data base
short  DAT_0080a428       = 0;            // UV index count per mesh group
short  DAT_0080a42a       = 0;            // normal index count per mesh group
short  DAT_0080a42c       = 0;            // vertex index count per mesh group
short  DAT_008097a0       = 0;            // material group count accumulator
BYTE   DAT_008097a2       = 0;            // material name table base
float  DAT_0080a424       = 0.0f;         // packed vertex buffer base
short  DAT_00879588       = 0;            // vertex index table base
short  DAT_00896a48       = 0;            // UV index table base
short  DAT_00831530       = 0;            // UV name/linked-vert table base
float  DAT_00831534       = 0.0f;         // UV coordinate data base
float  DAT_0083153c       = 0.0f;         // UV extra/Z channel base
short  DAT_00831540       = 0;            // UV linked vertex index table base
float  DAT_00862270       = 0.0f;         // normal data base
float  DAT_00862274       = 0.0f;         // normal Y channel base
short  DAT_008b3f08       = 0;            // normal index table base

// ── Parse_NextToken (0x0050e2c0) ─────────────────────────────────────────────
int    _DAT_083a40f4      = 0;            // last token type code
float  _DAT_083a40f8      = 0.0f;         // last numeric token value

// ── RenderObjectScreen (0x004e13a0) ──────────────────────────────────────────
float  _DAT_07ea952c      = 0.0f;         // item render: rotation X offset
float  _DAT_07ea9530      = 0.0f;         // item render: rotation Y
float  _DAT_07ea9534      = 0.0f;         // item render: rotation Z
short  _DAT_07ea9512      = 0;            // item render: resolved model type index
float  _DAT_07ea9538      = 0.0f;         // item render: HeadAngle[0]
int    _DAT_07ea9618      = 0;            // item render: state flag A
int    _DAT_07ea961c      = 0;            // item render: state flag B
short  DAT_07ea9616       = 0;            // item render: state flag C

// ── Filter / name-filter system globals ──────────────────────────────────────
DWORD  DAT_007cfa00       = 0x007cfa00;  // word-filter BMD checksum seed (literal)
DWORD  DAT_00578200       = 0x00578200;  // name-filter BMD checksum seed (literal)
char   lpText_07d2aa08[256] = {};  // fatal-error message string (shown by ExitProgram)

// ── BuxConvert XOR key and misc ───────────────────────────────────────────────
BYTE   DAT_0055a76c       = 1;    // unk_55A76C — gate de la 2da pasada del terreno
                                  // (TerrainFlag=2, la capa de billboards de
                                  // pasto/arena que se mueve con el viento).
                                  // 2026-08-23: estaba en 0 con el comentario
                                  // "never written in bin -> 0".  Nadie lo
                                  // escribe —un solo xref, la lectura en
                                  // RenderTerrain— pero es constante de .data y
                                  // en el binario vale 1 (ida_get_bytes
                                  // 0x0055A76C -> 01 00 00 00).  Con 0 el
                                  // overlay no se dibujaba en ningun mapa.
BYTE   DAT_0055a770       = 0;    // BuxConvert key byte [0]  (+1, +2 are adjacent bytes)

// ── SkillAttribute table ──────────────────────────────────────────────────────
// DAT_07e118e8 (HeroTile) already defined as DWORD above (~line 495)
_SkillAttrEntry SkillAttribute = {};   // skill attribute table base @ 0x07D29D20
LPVOID DAT_07abf164       = nullptr;   // character extra BMD heap buffer
char   DAT_07c82cd0       = 0;        // floating label pool base
char   DAT_0814b6e0       = 0;        // water wave buffer A
// Grass-wind / water-wave ping-pong buffer: sub_4F98C0 (setup) y sub_4F9A30
// (smoothing) escriben `&DAT_0814b2e0 + 0x40000*toggle` → 2 buffers de 0x40000
// (256×256 DWORDs c/u) = 0x80000.  Antes era 1 char → 512KB de heap stomp en
// cada frame al wirear RenderTerrain (CLAUDE.md "hardcoded-address" pattern).
char   DAT_0814b2e0[0x80000] = {};    // grass-wind/water-wave double buffer
char   DAT_00561ba8[8]    = "OZT";   // OpenTGA extension suffix (Data mode: .tga→.OZT)
DWORD  DAT_00560694       = 0;    // Map_Load block-read descriptor

// ── Small-function batch globals ─────────────────────────────────────────────
BYTE   DAT_00567500[0x1C900] = {0};  // Quest table base — see globals.h
DWORD  DAT_00590b00       = 0;       // Sound device context
DWORD  DAT_055c9b78       = 0;       // RefCount / tick counter

int    PTR_LAB_00552460   = 0;
int    PTR_LAB_005524e8   = 0;
int    PTR_FUN_00552508   = 0;
int    PTR_LAB_005527e4   = 0;
int    PTR_LAB_005524b8   = 0;
int    PTR_FUN_005524c0   = 0;
int    PTR_FUN_005524c4   = 0;
int    PTR_FUN_005524d8   = 0;
int    PTR_LAB_00552810   = 0;
int    PTR_FUN_005527e0   = 0;
int    PTR_FUN_0055389c   = 0;

// IDA off_XXXX vtable/pointer-literal symbols, renamed by port.py to DAT_xxxxxxxx.
// (parallel to PTR_LAB_/PTR_FUN_ above; port.py uses the DAT_ form)
DWORD  DAT_00552460 = 0;
DWORD  DAT_005524b4 = 0;
DWORD  DAT_005524b8 = 0;
DWORD  DAT_005524c0 = 0;
DWORD  DAT_005524c4 = 0;
DWORD  DAT_005524c8 = 0;
DWORD  DAT_005524d8 = 0;
DWORD  DAT_005524e8 = 0;
DWORD  DAT_00552508 = 0;
DWORD  DAT_00552514 = 0;
DWORD  DAT_00552548 = 0;
DWORD  DAT_00552568 = 0;
DWORD  DAT_00552574 = 0;
DWORD  DAT_00552588 = 0;
DWORD  DAT_005525a0 = 0;
DWORD  DAT_005525c8 = 0;
DWORD  DAT_00552668 = 0;
DWORD  DAT_00552760 = 0;
DWORD  DAT_005527e0 = 0;
DWORD  DAT_005527e4 = 0;
DWORD  DAT_005527f8 = 0;
DWORD  DAT_00552810 = 0;
DWORD  DAT_0055389c = 0;

// DAT_07e11e54 already defined above (line ~532)
HANDLE lpTargetHandle_00563b5c = INVALID_HANDLE_VALUE;
HANDLE hEvent             = NULL;
LPVOID lpParameter        = NULL;
char   lpWindowName_0055910c[64] = {};
DWORD  g_csQuest          = 0;

BYTE   m_byMatchType      = 0;
int    m_iMatchTimeMax    = 0;
int    m_iMaxKillMonster  = 0;
int    m_iKillMonster     = 0;

BOOL   g_EnableSound      = FALSE;
bool   g_Enable3DSound    = false;
LPDIRECTSOUND           g_lpDS            = NULL;
LPDIRECTSOUND3DLISTENER g_lpDS3DListener  = NULL;
void*  wavefile           = NULL;
DWORD  g_dwBufferBytes    = 0;
// DirectSound buffer tables
int                    MaxBufferChannel[420] = {};
int                    BufferChannel[420]    = {};
bool                   Enable3DSound[420]    = {};
int                    SoundLoadCount         = 0;
char                   BufferName[420][0x40] = {};
LPDIRECTSOUNDBUFFER    g_lpDSBuffer[420][4]   = {};
LPDIRECTSOUND3DBUFFER  g_lpDS3DBuffer[420][4] = {};
DWORD                  Object3DSound[420][4]  = {};

// Tabla de nombres NPC/mob: 512 entradas × 0x36 bytes (ver nota en 0x07CF2000).
BYTE   MonsterScript[512 * 0x36] = {};
char   WhisperRegistID[28][4] = {};

// DAT_07c608b8 — see globals.h. Defined as macro into g_RenderPool_07c608a8.
// (The standalone declaration was a single int that backed nothing — MoveBlurs
//  walked past it. Now the pool array is the storage and the macro projects.)
// Butterfles is now a macro alias for DAT_083a1218 (same address 0x083a1218)

// ── Batch 3 globals ──────────────────────────────────────────────────────────
void  *DAT_055c9b98       = NULL;   // RB-tree sentinel (NIL)
float  CameraAngle[3]     = {0};
float  CameraPosition[3]  = {0};
int    _g_bEventChipDialogEnable = 0;
// InventoryOpened/CharacterOpened/etc. are now #define aliases for
// DAT_07eaa117/116/etc. — see globals.h. Storage is the byte-sized DAT_
// globals defined above (lines ~745-753).
FILE  *SMDFile             = NULL;
// lpDefault_00583d88 already defined above (line ~1014)
DWORD  DAT_00563e00       = 0;
// DAT_00560694 already defined above (line ~1922)

// Missing globals (compilation fixes)
DWORD  DAT_07c82cf4[0xAF0] = {};   // terrain alpha bitmap pool
DWORD  DAT_0055339c       = 0;
// m_dwTextColor / m_dwBackColor NO son globals separados: en IDA son EXACTAMENTE
// 0x559c78 / 0x559c80 (= DAT_00559c78 / DAT_00559c80). Verificado por disasm
// (sub_40D610 @0x40D734: `mov [0x559c78], 0xffff9664`, y sub_480980 idéntico).
// Estaban declarados aparte → todo el código que setea m_dwTextColor (HUD_Pass1/2/3,
// ChatListBox render) escribía a un global que el render de texto (FUN_0040f610, lee
// DAT_00559c78) NUNCA leía → colores perdidos = texto blanco. Ahora son macros
// (globals.h) que apuntan al global real. Ver [[charselect-deferred-issues]].
// g_lpszMessageBoxCustom es un alias de DAT_083a44c4 (ver globals.h).
// m_hFontDC ahora es macro sobre DAT_055c9fec (ver globals.h)
HFONT  g_hFontBold        = NULL;

// Batch 18 — InitGame / ReceiveChat globals
DWORD  EnableUse          = 0;
int    DAT_07e11998       = -1;
// DAT_07e11d28 already defined above (line ~805)
int    DAT_07e11e10       = 0;
int    DAT_07e11e14       = 0;
int    DAT_07e11994       = -1;
int    DAT_07e11990       = -1;
// DAT_07e1198c already defined above (line ~994)
int    DAT_07e11988       = -1;
// DAT_07e11984 already defined above (line ~993)
int    DAT_07e11e18       = 1;
// DAT_07e11d24 already defined above (line ~873)
// DAT_07e11d1c already defined above (line ~804)
int    DAT_07e11d18       = 0;
int    DAT_07e11d14       = 0;
BYTE   DAT_00559c6d       = 0xFF;
short  DAT_07e11e1c       = 0;
short  DAT_07e11e20       = -1;
short  DAT_07e11e22       = -1;
short  DAT_07e11e24       = -1;
BYTE   DAT_07e11e26       = 0;
int    DAT_07e11980       = 0;

// Batch 19 — SendCheck globals
DWORD  DAT_07e11d10       = 0;
BYTE   DAT_07db8600       = 0;
BYTE   DAT_05826cfc       = 0;
DWORD  DAT_05826d00       = 0;

// Batch 20 — OpenNpc, MoveCamera, RenderEquipment3D, RenderItems3D, FUN_0043ce50, LookAtTarget
// Only truly new globals:
float  _DAT_005524a0      = 55.0f;  // equip pendant X offset
float  _DAT_00552c18      = 46.0f;  // equip slot Y offset (helm row)
float  _DAT_00552c10      = 75.0f;  // equip slot X offset (pants/gloves/boots col)
float  _DAT_00552c0c      = 89.0f;  // equip slot Y offset (mid row)
float  _DAT_00552c08      = 152.0f;  // equip slot Y offset (ring/boots row)
float  _DAT_00552c04      = 134.0f;  // equip slot X offset (weapon_L/ring_R col)
float  _DAT_005527dc      = 13.0f;  // arrow count X offset
float  DAT_083a7adc[6]    = {0};   // CurrentCameraWalkDelta[6]
float  _DAT_00552a30      = 0.166666672f;  // lerp speed (camera smoothing)
void*  g_LoginSceneObjects[9] = {0}; // sky, ship1, wave1, ship2, wave2, ship3, wave3, banner, sun

// Batch 22 — AttackStage, CreateArrow
// g_iLimitAttackTime @ 0x00559858 — umbral del contador de ataque (c+757) que
// gatea TODA la seccion de skills de MoveCharacter (0x449900 L840):
//     if (*(BYTE*)(c + 757) >= g_iLimitAttackTime) { ...skills..., hit sounds }
// El valor inicial del .data del binario es 15 (leido: 0f 00 00 00); despues lo
// reescribe AttackStage (0x448930) con 5 o 15 segun el skill.
// Estaba en 0, con lo cual el gate era `0 >= 0` = siempre cierto y el bloque
// corria en CADA frame para CADA entidad: en char-select eso disparaba cientos
// de PlayBuffer(rand()%7+50) = el ruido de golpes (eBlow/eShortBlow).
int    DAT_00559858       = 15;    // g_iLimitAttackTime
DWORD  DAT_05826d10       = 0;     // CurrentSkill (current skill ID for arrow/projectile)
float  _DAT_00552904      = 1400.0f;  // sin/cos offset multiplier (sword trail radius)
float  _DAT_005528f8      = 145.0f;  // sin/cos offset multiplier (slash projectile)
float  _DAT_005528f4      = 0.54f;  // combo animation offset constant

// Batch 21 — RenderSkillIcon, CheckMixRecipe, MoveObjects, CreateArrows, CollisionDetectLineToMesh
DWORD  DAT_083a3fec       = 0;     // visible object counter (MoveObjects)
DWORD  DAT_07eaa178       = 0;     // mix recipe: socket flag byte
DWORD  DAT_07eaa170       = 0;     // mix recipe: wing socket option value
int    DAT_07eaa174       = 0;     // mix recipe: combined value score (capped at 0x5a)
char   DAT_00559c6c       = 0;     // last rendered hotkey char
float  _DAT_00552ab0      = 7.5f;  // arrow angle offset (5-way shot small)
float  _DAT_00552584      = 22.5f;  // arrow angle offset (5-way shot large)
float  _DAT_00552a4c      = 26.0f;  // RenderNumber2D Y offset

// ── UseSkillWizard dependencies ──────────────────────────────────────────────
DWORD  DAT_07d780a0       = 0;     // MovementSkillTarget (index into CharactersClient)
DWORD  DAT_07d7809c       = 0;     // skill slot index (current selected skill slot)
char   DAT_00559d94[8]    = "webzen";  // GM name string (anti-impersonation check 1)
char   DAT_00559d9c[8]    = "webzen";  // GM name string (anti-impersonation check 2)

// ── SkillElf dependencies ────────────────────────────────────────────────────
char   DAT_00559db4       = 0;     // GM name check string (part of "webzen" pattern)
char   DAT_07e11dfc       = 0;     // chat log widget ID string (for AddText)
char   DAT_07d4c89c       = 0;     // "Not enough mana" message string

// ── MoveParticles camera shake globals ──────────────────────────────────────
float  DAT_07c800f8       = 0.0f;  // camera shake accumulator X
float  DAT_07c800fc       = 0.0f;  // camera shake accumulator Y
float  DAT_07c80100       = 0.0f;  // camera shake accumulator Z
float  DAT_07c80104       = 0.0f;  // camera shake velocity X
float  DAT_07c80108       = 0.0f;  // camera shake velocity Y

// ── MoveParticles float constants ───────────────────────────────────────────
float  _DAT_00552a60      = 0.0111111114f;
float  _DAT_00552a7c      = 0.08f;
float  _DAT_00552ac4      = 0.09f;
float  _DAT_00552ac8      = 0.07f;
// _DAT_00552acc — already defined above (line ~964)
float  _DAT_00552ad0      = 0.87266463f;
float  _DAT_00552ad4      = 0.222222224f;
float  _DAT_00552ad8      = 0.0872664601f;
// _DAT_00552adc — already defined above (line ~141)
float  _DAT_00552ae0      = 0.17453292f;
float  _DAT_00552ae8      = 0.9090909f;
float  _DAT_00552aec      = 0.6145098f;
float  _DAT_00552af0      = 0.980392158f;
float  _DAT_00552af4      = 0.42f;
float  _DAT_00552af8      = 0.0416666679f;
float  _DAT_00552afc      = 0.333f;
float  _DAT_00552b00      = 0.572f;
float  _DAT_00552b04      = 0.725f;
float  _DAT_00552b08      = 0.00174532924f;
float  _DAT_00552b0c      = 0.0002f;
float  _DAT_00552b10      = 0.3137255f;
float  _DAT_00552b14      = 0.394901961f;
float  _DAT_00552b18      = 0.470588237f;
float  _DAT_00552b1c      = 0.125f;
float  _DAT_00552b20      = 6.4f;
float  _DAT_00552b24      = 1.05f;
float  _DAT_00552b28      = 0.2f;
float  _DAT_00552b2c      = 0.98f;
float  _DAT_00552b30      = 0.000400000019f;
float  _DAT_00552b34      = 0.7407407f;
float  _DAT_00552b38      = 0.0001f;
float  _DAT_00552b3c      = 0.095f;
float  _DAT_00552b40      = 0.85f;
float  _DAT_00552b44      = 0.95f;
float  _DAT_00552b48      = -1.7f;
float  _DAT_00552b4c      = 1.7f;
float  _DAT_00552b50      = -0.6f;
float  _DAT_00552b54      = 0.0006f;

// Additional math/render constants
float  _DAT_00552860      = 57.29578f;  // 180/pi (radians to degrees)
float  _DAT_00552b9c      = 0.015625f;  // 1/64 (terrain UV step)
float  _DAT_00552cb4      = 64.0f;      // terrain tile size

// CheckArrow chat string globals (runtime-initialized by resource loader)
char   DAT_07e11df4       = 0;
char   DAT_07e11df8       = 0;
char   DAT_07d3c348       = 0;

// Skill selection
char   DAT_07d78098       = 0;

// Chat bubble pool (base 0x07E016F8, stride 0x254, ~96 slots)
// ── Pool de burbujas de chat (CreateChat 0x481BA0 / MoveChat 0x4821A0) ───────
// En el binario: base `unk_7E016F8`, stride 596 (0x254), fin `unk_7E0FFC8`.
//   (0x7E0FFC8 - 0x7E016F8) / 596 = **100 slots**.
// `unk_7E01720` NO es otro pool: es base + 40 (el campo timer1), que es donde
// MoveChat arranca su walk. Por eso ahora es una macro (ver globals.h).
// Antes: DAT_07e016f8 era un char de 1 byte y DAT_07e01720 un array separado
// de 26 slots → CreateChat caminaba 59600 bytes sobre globals adyacentes.
char   DAT_07e016f8[100 * 0x254] = {};
// Guild mark bracket strings (initialized by resource loader)
char   DAT_00559d60       = 0;  // guild mark prefix string "["
short  DAT_00559d64       = 0;  // guild mark suffix 2-byte "]"
char   DAT_00559d66       = 0;  // guild mark suffix trailing

// ── HUD render globals (Phase-2 port) ────────────────────────────────────────
int    PartyNumber           = 0;
BYTE   Party[2048]           = {0};
int    EnableGuildWar        = 0;
int    HeroSoccerTeam        = 0;
int    GuildWarScore[2]      = {0, 0};
char   GuildWarName[80]      = {0};
char   SoccerTeamName[2][80] = {{0}, {0}};

// 2026-07-19: DAT_07e01924 era un TERCER buffer separado para el MISMO pool de
// burbujas. En el binario 0x7E01924 = 0x7E016F8 + 0x22C (campo disp1 del slot 0).
// CreateChat escribia en DAT_07e016f8 y RenderBooleans leia aca -> nunca se
// dibujaba nada. Ahora es una macro sobre el pool unico (ver globals.h).

HFONT  g_hFont               = NULL;
int    FontHeight            = 14;
SIZE   TextSize              = {0, 0};

// dword_55C9BC8 hash-table state lives in g_HashTableCtx[4] (line ~438);
// macros DAT_055c9bc8/cc/d0/d4 alias it.  Do NOT re-define here.

void  *CharacterMachine      = nullptr;
// CharacterAttribute is a #define alias of DAT_07cf1ff4 — no storage here.

// DAT_07e11d6e is already defined at line ~676 as char (matches header).

// 2026-07-27: scratch de coordenadas de los paneles Party / GuildCreation.
// Antes se guardaban en Inventory[32], que es el slot 0 del overlay del pool
// de la TIENDA → lo pisaban cada frame (tienda vacía intermitente).
int g_PartyPanelScratchX = 0, g_PartyPanelScratchY = 0;
int g_GuildCreatorScratchX = 0, g_GuildCreatorScratchY = 0;
