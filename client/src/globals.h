#pragma once
// globals.h — extern declarations for all Ghidra DAT_ globals.
// Los tipos se infieren del contexto; el prefijo _DAT_ = alias float de la misma dirección.
// NO incluir este header directo — entra vía stdafx.h.

// Reinterpret a DWORD global as float (for globals that have no _DAT_ float alias).
// Uso: Ff(DAT_xxx) para lecturas/escrituras float; Fi(DAT_xxx) para lecturas int.
#ifndef Ff
#define Ff(sym)  (*(float*)&(sym))
#define Fi(sym)  (*(int*)&(sym))
#endif

// ── Low-address constants (x87 FPU/hardware, Ghidra artifacts) ────────────────
// Aparecen cerca de la dirección 0 — se acceden como float vía el alias _DAT_.
extern float  _DAT_00000010;
extern float  _DAT_00000014;
extern float  _DAT_00000018;
extern float  _DAT_0000001c;
extern float  _DAT_00000020;
extern float  _DAT_00000024;
extern DWORD   DAT_00000010;
extern DWORD   DAT_00000014;
extern DWORD   DAT_00000018;
extern DWORD   DAT_0000001c;
extern DWORD   DAT_00000020;
extern DWORD   DAT_00000024;

// ── Game math / render constants (0x005524xx – 0x00552dxx) ────────────────────
// Casi todos se acceden como float vía el alias _DAT_; en algunos también hay acceso entero.
extern float  _DAT_00552464;
extern float  _DAT_0055246c;
extern float  _DAT_00552488;   // Z-bob oscillation phase
extern float  _DAT_005524bc;
extern float  _DAT_005524f0;   // grid-to-world scale factor
extern DWORD   DAT_005524f0;
extern float  _DAT_005524f4;
extern DWORD   DAT_005524f4;
extern float  _DAT_005524f8;
extern DWORD   DAT_005524f8;
extern float  _DAT_005524fc;
extern DWORD   DAT_005524fc;
extern float  _DAT_00552500;
extern DWORD   DAT_00552500;
extern float  _DAT_00552504;
extern DWORD   DAT_00552504;
extern float  _DAT_00552530;
extern DWORD   DAT_00552530;
extern float  _DAT_00552534;
extern DWORD   DAT_00552534;
extern float  _DAT_00552538;   // cloth grid uv scale factor
extern DWORD   DAT_00552538;
extern float  _DAT_00552540;   // particle dir scale (Matrix_TransformPoint branch)
extern float  _DAT_005527d4;   // Party HP bar row height in pixels (SecondPassword_HoverCheck)
extern DWORD   DAT_00552540;
extern float  _DAT_00552560;   // spring horizontal-vs-vertical angle threshold
extern DWORD   DAT_00552560;
extern float  _DAT_0055256c;
extern DWORD   DAT_0055256c;
extern DWORD   DAT_00552580;
extern float  _DAT_00552594;
extern DWORD   DAT_00552594;
extern float  _DAT_00552598;
extern DWORD   DAT_00552598;
extern float  _DAT_00552660;
extern DWORD   DAT_00552660;
extern float  _DAT_00552664;
extern DWORD   DAT_00552664;
extern float  _DAT_005526dc;
extern DWORD   DAT_005526dc;
extern float  _DAT_005526e0;   // Entity_GetMoveRate: speed for wyvern/dragon-class
extern DWORD   DAT_005526e0;
extern float  _DAT_005526e4;
extern DWORD   DAT_005526e4;
extern float  _DAT_005526e8;
extern DWORD   DAT_005526e8;
extern float  _DAT_00552834;
extern DWORD   DAT_00552834;
extern float  _DAT_00552838;   // 1/480 — Y scale constant
extern DWORD   DAT_00552838;
extern float  _DAT_0055283c;   // 1/640 — X scale constant
extern DWORD   DAT_0055283c;
extern DWORD   DAT_00552844;
extern DWORD   DAT_00552848;
extern float  _DAT_00552848;   // height offset — small entities in normal state
extern float  _DAT_0055284c;
extern DWORD   DAT_0055284c;
extern DWORD   DAT_00552850;
extern float  _DAT_00552868;
extern DWORD   DAT_00552868;
extern float  _DAT_00552874;   // alpha fade rate (weather particle)
extern float  _DAT_0055287c;   // Catmull-Rom weight t=0.25 (substep 1 X bias)
extern float  _DAT_00552880;
extern float  _DAT_00552884;   // Catmull-Rom weight (substep 0 main)
extern float  _DAT_00552888;   // Catmull-Rom weight (substep 0/2 cross)
extern float  _DAT_0055288c;   // Catmull-Rom weight (substep 0/1 lookahead)
extern DWORD   DAT_0055286c;
extern DWORD   DAT_00552874;
extern DWORD   DAT_00552880;
extern float  _DAT_00552890;   // movement speed scale (facing dt factor)
extern float  _DAT_005528b0;
extern DWORD   DAT_005528b0;
extern float  _DAT_005528b4;
extern DWORD   DAT_005528b4;
extern float  _DAT_005528b8;
extern DWORD   DAT_005528b8;
extern DWORD   DAT_005528dc;
extern float  _DAT_005528e0;   // Z-bob amplitude / fsin input
extern DWORD   DAT_005528e0;
extern float  _DAT_005528f0;
extern DWORD   DAT_005528f0;
extern float  _DAT_00552908;
extern DWORD   DAT_00552908;
extern float  _DAT_005528fc;  // MoveJoint Z-offset constant
extern float  _DAT_0055290c;
extern DWORD   DAT_0055290c;
extern float  _DAT_00552910;   // particle scale constant (gem/bead effects)
extern DWORD   DAT_00552910;
extern float  _DAT_0055291c;   // PathFinder spin-speed scale / Entity_Reset spin
extern DWORD   DAT_0055291c;
extern float  _DAT_00552920;   // BMD anim frame scale
extern DWORD   DAT_00552920;
extern float  _DAT_00552928;
extern DWORD   DAT_00552928;
extern float  _DAT_0055293c;   // particle Z offset constant
extern DWORD   DAT_0055293c;
extern float  _DAT_00552934;
extern DWORD   DAT_00552934;
extern float  _DAT_00552974;
extern DWORD   DAT_00552974;
extern float  _DAT_0055297c;
extern DWORD   DAT_0055297c;
extern float  _DAT_005529bc;
extern DWORD   DAT_005529bc;
extern float  _DAT_005529c0;   // particle velocity scale constant
extern DWORD   DAT_005529c0;
extern float  _DAT_005529c8;   // angle-to-radian scale (weather rotation)
extern DWORD   DAT_005529c8;
extern float  _DAT_005529e0;
extern DWORD   DAT_005529e0;
extern float  _DAT_00552a00;
extern DWORD   DAT_00552a00;
extern float  _DAT_00552a10;   // entity flash/tint ramp step multiplier
extern DWORD   DAT_00552a10;
extern DWORD   DAT_00552a1c;
extern float  _DAT_00552a38;   // butterfly/bubble max distance squared
extern float  _DAT_00552a3c;   // PI constant for angle flip (360°)
extern float  _DAT_00552a40;   // secondary fsin frequency (rain Y offset)
extern float  _DAT_00552a48;   // particle velocity cross-product scale
extern DWORD   DAT_00552a48;
extern float  _DAT_00552abc;   // particle rotation scale (wind effect)
extern DWORD   DAT_00552abc;
extern float  _DAT_00552ac0;   // particle size scale (large smoke/wind)
extern DWORD   DAT_00552ac0;
extern float  _DAT_00552adc;   // aspect ratio correction (float view of DAT_00552adc)
extern DWORD   DAT_00552adc;
extern DWORD   DAT_00552b70;
extern float  _DAT_00552b6c;   // facing delta threshold (max dt before skipping)
extern float  _DAT_00552b7c;
extern DWORD   DAT_00552b7c;
extern float  _DAT_00552b88;
extern DWORD   DAT_00552b88;
extern float  _DAT_00552c00;   // item spin speed (frames-to-degrees multiplier)
extern float  _DAT_00552c14;
extern DWORD   DAT_00552c14;
extern float  _DAT_00552c24;
extern DWORD   DAT_00552c24;
extern float  _DAT_00552ca4;
extern DWORD   DAT_00552ca4;
extern float  _DAT_00552ca8;
extern DWORD   DAT_00552ca8;
extern float  _DAT_00552cac;
extern DWORD   DAT_00552cac;
extern DWORD   DAT_00552cb0;
extern float  _DAT_00552cc4;   // PI/180 = 0.017453292f  (float view of DAT_00552cc4)
extern DWORD   DAT_00552cc4;
extern float  _DAT_00552d08;
extern DWORD   DAT_00552d08;
extern float  _DAT_00552d0c;
extern DWORD   DAT_00552d0c;
extern float  _DAT_00552d20;   // 1/1600 (world→object-bucket-grid factor)
extern DWORD   DAT_00552d20;
extern float  _DAT_00552d38;
extern DWORD   DAT_00552d38;
extern float  _DAT_00552d3c;
extern DWORD   DAT_00552d3c;
extern float  _DAT_00552d48;
extern DWORD   DAT_00552d48;
extern float  _DAT_00552d4c;
extern DWORD   DAT_00552d4c;
extern DWORD   DAT_005538a0;

// ── Entity / render constants (0x005590xx – 0x00559dxx) ───────────────────────
extern BYTE    DAT_00559050[16];   // 16-byte XOR key table (Crypto/anti-tamper)
extern float  _DAT_00559070;   // Verlet physics damping/gravity scalar
extern DWORD   DAT_00559070;
extern DWORD   DAT_005590ac;
// Layout de los 3 botones popup del ChatListBox — flt_5590B0/B4/B8 en IDA.
// Leídos del binario: 295.0 / 417.0 / 18.0 (ver comentario en globals.cpp).
extern float   DAT_005590b0;   // X del primer botón
extern float   DAT_005590b4;   // Y de los tres
extern float   DAT_005590b8;   // separación horizontal
extern BYTE    DAT_0055961c[5];    // Version — obfuscated as Version[i]-i-1 in login pkt
extern BYTE    DAT_00559624[16];   // Serial — sent raw in login pkt
extern DWORD   DAT_00559678;
extern float  _DAT_00559680;   // LOD factor
extern DWORD   DAT_00559680;
extern DWORD   DAT_00559684;
extern char    DAT_005597a0;    // PathFinder debug string (first char; take &)
extern DWORD   DAT_005597c4;
extern float  _DAT_005597c8;   // LOD scale: 1.0/1.2/1.4
extern DWORD   DAT_005597c8;
extern DWORD   DAT_0055987c;
extern DWORD   DAT_005599b0;
extern DWORD   DAT_005599e0;
extern DWORD   DAT_00559bf0;
extern int    _DAT_00559b9c;   // wind speed result (fsin * tick, *10)
extern DWORD   DAT_00559bec;   // movement cooldown threshold
extern int     DAT_00559c48;   // hover: item-on-ground index
extern int     DAT_00559c4c;   // hover: NPC index
extern int     DAT_00559c50;   // hover: entity (mob/player) index
extern int     DAT_00559c54;   // hover: special object index
extern int     DAT_00559c58;   // hover: secondary target index
extern char    DAT_00559c5c;   // hover enabled flag (0=disabled)
extern int     DAT_00559c60;   // equipped weapon type (right hand)
extern int     DAT_00559c64;   // equipped weapon type (left hand)
extern int     DAT_00559c68;   // equipped weapon type (crossbow/secondary)
extern int     DAT_00559c70;   // hover attack target (entity index copy)
extern DWORD   DAT_00559c78;
// 0x00559C7C — IDA `SetTextColor_0` (color del prefijo/guild en sub_47F360).
// Una sola memoria: el alias existe porque parte del port lo nombra DAT_ y
// otra parte SetTextColor_0.  Ver el comentario en globals.cpp.
#ifdef __cplusplus
extern "C" DWORD SetTextColor_0;
#else
extern DWORD SetTextColor_0;
#endif
#define DAT_00559c7c SetTextColor_0
extern DWORD   DAT_00559c80;
extern char    DAT_00559b80[];  // GM/admin name substring (Entity_FindNearby filter string)
extern DWORD   DAT_00559c84;
extern DWORD   DAT_00559c88;
extern DWORD   DAT_00559c8c;
extern DWORD   DAT_00559c90;
extern int     _InputTextMaxArr[8];
extern int&    _DAT_00559c94;
extern DWORD&  DAT_00559c94;
extern int&    _DAT_00559c98;
extern DWORD&  DAT_00559c98;
extern DWORD   DAT_00559bf1;   // chat/entity flag (allows type 3 show)
extern DWORD   DAT_00559cc4;   // chat history index (0..4)
extern DWORD   DAT_00559ce0;   // entity table iteration offset
extern DWORD   DAT_00559ccc;
extern DWORD   DAT_00559cd0;
extern DWORD   DAT_00559cd4;
extern DWORD   DAT_00559cd8;
extern int     DAT_00559ce8;   // hover target index copy (char-select slot or NPC)
extern DWORD   DAT_00559d74;
extern DWORD   DAT_00559ef0;
extern DWORD   DAT_00559ef4;
extern DWORD   DAT_00559ef8;
extern char    DAT_00559f5e;   // multi-select item result slot A
extern char    DAT_00559f5f;
extern DWORD   DAT_0055a3e8[4];   // chaos-mix info (ReceiveTalk sub 3)
extern int     EventType;         // EventWindow type (ReceiveTalk sub 4/6)
extern int     g_NpcTalkActive;
extern int     g_PartyPanelScratchX, g_PartyPanelScratchY;
extern int     g_GuildCreatorScratchX, g_GuildCreatorScratchY;   // 1 = talk 0x30 enviado, server puede tener Interface.use=1
extern "C" { extern int g_bServerDivisionEnable; extern int g_bServerDivisionAccept; }  // ReceiveTalk sub 5
extern DWORD   DAT_0055a3e4;
extern BYTE    DAT_0055a76c;    // unk_55A76C — terrain alpha-overlay present flag (never written in bin → 0)
extern BYTE    DAT_0055a770;    // BuxConvert 3-byte XOR key [0] (key spans +0,+1,+2)
extern DWORD   DAT_0055a774;
extern DWORD   DAT_0055a778;
extern DWORD   DAT_0055a77c;
extern DWORD   DAT_0055a780;
extern char    s__s_file_not_found__0055a784[];  // "%s file not found" format string
extern DWORD   DAT_0055a798;
extern char    DAT_0055a79c[];  // "Data\\"  path prefix
extern char    DAT_0055a7a4[];  // "Data2\\" path prefix
extern int     DAT_0055a7ac;   // g_GameSubState: 0=connecting 2=in-world 7=loading-map
extern float  _DAT_0055a7c0;
extern DWORD   DAT_0055a7c0;

// ── Game loop / scene state (0x005615xx – 0x005617xx) ─────────────────────────
// Nota: g_GameState (DAT_005615c0), g_hWnd y g_hDC se declaran en stdafx.h.
extern DWORD   DAT_0056154c;
extern DWORD   DAT_00561550;
extern DWORD   DAT_00561554;
extern float   DAT_005616b4;   // CameraDistanceTarget (MoveMainCamera smoothing)
extern float   DAT_083a45d0;   // CameraDistance (MoveMainCamera)
extern DWORD   DAT_0056156c;
extern DWORD   DAT_00561570;
extern DWORD   DAT_00561574;
extern char   *DAT_005615b8;   // server IP string ptr
#define PTR_s_connect_muonline_co_kr_005615b8  DAT_005615b8
extern WORD    DAT_005615bc;   // server port

// ── ConnectServer flow (2026-07-15) ──────────────────────────────────────────
extern int             g_HasConnectServer;       // server.cfg tiene 2 líneas → flujo CS
extern int             g_ConnectServerMode;      // 1 = socket actual habla con el ConnectServer
extern int             g_ConnectServerRequested; // 1 = ya mandamos C1 04 F4 02
extern char            g_GameServerIP[128];      // GameServer fallback (server.cfg línea 2)
extern unsigned short  g_GameServerPort;
// DAT_005615c0 = g_GameState (in stdafx.h)
// g_lpszMp3 @ 0x005615C4 — tabla de 6 rutas de BGM (ver globals.cpp).
// Los DAT_005615c4..d8 NO son handles independientes: son sus 6 elementos.
extern char*   g_lpszMp3[6];
#define DAT_005615c4     ((DWORD)(uintptr_t)g_lpszMp3[0])
#define DAT_005615c8     ((DWORD)(uintptr_t)g_lpszMp3[1])
#define DAT_005615cc     ((DWORD)(uintptr_t)g_lpszMp3[2])
#define DAT_005615d0     ((DWORD)(uintptr_t)g_lpszMp3[3])
#define DAT_005615d4     ((DWORD)(uintptr_t)g_lpszMp3[4])
#define DAT_005615d8     ((DWORD)(uintptr_t)g_lpszMp3[5])
#define PTR_DAT_005615c4 DAT_005615c4
#define PTR_DAT_005615c8 DAT_005615c8
#define PTR_DAT_005615cc DAT_005615cc
#define PTR_DAT_005615d0 DAT_005615d0
#define PTR_DAT_005615d4 DAT_005615d4
#define PTR_DAT_005615d8 DAT_005615d8
// DAT_005615dc @ 0x005615DC ES `g_iCurrentDialogScript` — el índice del diálogo
// de quest activo.  Lo escriben CSQuest::ShowDialogText (0x4017E7) y sub_51D840,
// y lo leen sub_401AF0 y UI_InGameMenu.  La etiqueta vieja ("login/scene misc
// flag") era una misidentificación.
// 2026-08-21: `g_iCurrentDialogScript` era un global APARTE en globals.cpp, así
// que el writer y el reader no se veían (mismo patrón que SetTextColor_0 /
// DAT_00559c7c).  Ahora es un alias.
extern DWORD   DAT_005615dc;
#define g_iCurrentDialogScript  (*(int*)&DAT_005615dc)
extern DWORD   DAT_005615e0;
extern DWORD   DAT_005615e8;   // fade timer
// CameraWalk[7][6] contiguous waypoint table (see globals.cpp for layout).
extern float   CameraWalk_005615ec[42];
extern DWORD&  DAT_005615ec;   // alias → CameraWalk[0]  (wp0.pos.x)
extern DWORD&  DAT_005615f0;   // alias → CameraWalk[1]
extern DWORD&  DAT_005615f4;   // alias → CameraWalk[2]
extern DWORD&  DAT_005615f8;   // alias → CameraWalk[3]
extern DWORD&  DAT_005615fc;   // alias → CameraWalk[4]
extern DWORD&  DAT_00561600;   // alias → CameraWalk[5]
extern DWORD&  DAT_00561664;   // alias → CameraWalk[30] (wp5.pos.x)
extern DWORD&  DAT_00561668;   // alias → CameraWalk[31]
extern DWORD&  DAT_0056166c;   // alias → CameraWalk[32]
extern DWORD&  DAT_00561670;   // alias → CameraWalk[33]
extern DWORD&  DAT_00561674;   // alias → CameraWalk[34]
extern DWORD&  DAT_00561678;   // alias → CameraWalk[35]
extern DWORD   DAT_00561694;
extern DWORD   DAT_00561698;
extern DWORD   DAT_005616a0;
extern int     DAT_005616a4;   // dialog Y (signed; ver globals.cpp)
extern int     DAT_005616a8;   // camera X (signed; ver globals.cpp)
extern DWORD   DAT_005616ac;
extern DWORD   DAT_005616b0;
extern DWORD   DAT_005616b8;
extern DWORD   DAT_005617a0;
extern char    DAT_00561a30[];   // "%s" format string used by Scene_CharSelect labels
extern DWORD   DAT_00561b04;
extern char    DAT_00561b70[]; // "OZJ" JPEG extension suffix
extern char    DAT_00561ba8[];  // OpenTGA extension suffix used in non-Data2 path (e.g. ".tga")

// Literales de string (datos de sólo lectura, etiquetas de Ghidra)
extern char    s_Local_Webzenlogo_jpg_00561774[];
extern char    s_Local_Everyone_jpg_0056178c[];
extern char    s_Local_Loading01_jpg_00561a88[];
extern char    s_Local_Loading02_jpg_00561a9c[];
extern char    s_Local_Loading03_jpg_00561ab0[];
extern char    s_connect_muonline_co_kr_005615b8[];
extern char    s_Dialog_005595e0[];
extern char    s_Hash_table_full______GetIndex_00558108[];
extern char    s_Hash_table_full______Insert_005580e8[];
extern char    s_Macro_Time_00559f30[];
extern char    s___2d_00559f44[];
extern char    s___2d___00559f3c[];
extern char    s_____2d_00559f4c[];
extern char    s__d__d_00559f00[];
extern char    s__d__d_00559f1c[];
extern char    s__s__d_00561a34[];
extern char    lpString_07d49c14[128];  // "Cannot create..." warning text
extern char    lpString_07d49d40[128];  // warning text line 2
extern char    lpString_00561a3c[64];   // server info line 1
extern char    lpString_00561a58[64];   // server info line 2
extern char    lpString_00561a68[64];   // server info line 3
extern char    s__s__d_Non_PVP___s_0056192c[];
extern char    s__s__d_Non_PVP___s_00561940[];
extern char    s__s__d_Non_PVP___s_00561954[];
extern char    s__s__d__s_0056192c[];
extern char    s__s__d__s_00561940[];
extern char    s__s__d__s_00561954[];
extern char    s__s__d__s_00561968[];
extern char    s__s__d__s_00561974[];
extern char    s__s__d__s_00561980[];
extern char    s__s__s_00561914[];
extern char    s__s__s_0056191c[];
extern char    s__s__s_00561924[];

// ── Misc game globals (0x00583dxx – 0x00590xxx) ───────────────────────────────
extern char    DAT_00583d8c[];  // quest manager object (~116KB)
extern DWORD   DAT_00583dac;
extern DWORD   DAT_00585e7c;
extern DWORD   DAT_0058c780;
extern DWORD   DAT_0058e1c4;
extern DWORD   DAT_0058e854;
extern DWORD   DAT_0058eee4;
extern DWORD   DAT_00590924;
// DAT_00590ac8 = g_EnableSound (sound master flag) — see declaration below in Sound section.
// DAT_00590ac9 = g_Enable3DSound (3D audio flag)    — see declaration below in Sound section.
// DAT_00590acc = SoundLoadCount                     — see declaration below in Sound section.
// DAT_00590ad0 = g_lpDS (IDirectSound*)             — see declaration below in Sound section.
// DAT_00590ad8 = wavefile (CWaveFile* being loaded) — see declaration below in Sound section.
#define DAT_00590ac8  g_EnableSound
#define DAT_00590ac9  g_Enable3DSound
#define DAT_00590acc  SoundLoadCount
#define DAT_00590ad0  g_lpDS
#define DAT_00590ad8  wavefile
// DAT_00583da8 = g_dwBufferBytes (current WAV data chunk size).
#define DAT_00583da8  g_dwBufferBytes

// ── Engine / Window handles (0x055c9xxx – 0x055caxxx) ─────────────────────────
// Nota: g_hWnd (DAT_055c9ffc) y g_hDC (DAT_055c9fec) están en stdafx.h.
extern char    lpData_055c9ba0[12];     // version string buffer (login packet)
extern DWORD   DAT_055c9b40;   // g_EnableSound
extern DWORD   DAT_055c9b60;   // sound channel index offset
extern DWORD   DAT_055c9b70;
extern float  _DAT_055c9b70;
extern DWORD   DAT_055c9b74;
extern float  _DAT_055c9b74;   // g_fScreenRate_y
extern DWORD   DAT_055c9b80;
extern char    DAT_055c9bac[12]; // config.ini [LOGIN] Version string
// Contexto de la ofuscación por HashTable — buffer contiguo (el binario original tiene el
// 4 fields physically contiguous at 0x055c9bc8..0x055c9bd4). Callers do
// `&DAT_055c9bc8 + 0xC` esperando la capacidad; las macros superponen el buffer para que
// holds. See globals.cpp for init.
extern DWORD   g_HashTableCtx[4];
#define DAT_055c9bc8  (g_HashTableCtx[0])   // vtable pointer
#define DAT_055c9bcc  (g_HashTableCtx[1])   // values array
#define DAT_055c9bd0  (g_HashTableCtx[2])   // keys array
#define DAT_055c9bd4  (g_HashTableCtx[3])   // capacity
extern DWORD   DAT_055c9be0;
extern DWORD   DAT_055c9be4;
extern float  _DAT_055c9be4;
extern DWORD   DAT_055c9be8;
extern DWORD   DAT_055c9bec;
extern DWORD   DAT_055c9bf0;
extern DWORD   DAT_055c9d00;
extern DWORD   DAT_055c9e04;
extern DWORD   DAT_055c9e44;
extern DWORD   DAT_055c9e48;
// RandomTable @ 0x055C9E58 — 100 enteros (`rand() % 360`) que siembra WinMain
// (0x41E8A0 L522-525).  Sólo lo usa Entity_Render (0x5038E0) para repartir en
// círculo las monedas del montón de Zen del suelo.
// 2026-08-21: estaba declarado como un único DWORD = 0 y nadie lo sembraba, así
// que los ángulos y radios salían todos 0 y las monedas se apilaban en un mismo
// punto (el "Zen sin sprite expandido").
extern int     DAT_055c9e58[100];
#define RandomTable  DAT_055c9e58
extern DWORD   DAT_055c9ff0;   // HGLRC (OpenGL context)
extern DWORD   DAT_055c9ff4;
extern DWORD   DAT_055c9ff8;
extern HINSTANCE DAT_055ca000;  // g_hInst (also in stdafx.h as g_hInst)
// DAT_055ca004 = g_hDC — macro in stdafx.h; no separate storage.
extern DWORD   DAT_055ca008;
extern DWORD   DAT_055ca00c;
extern DWORD   DAT_055ca010;
extern DWORD   DAT_055ca014;
extern DWORD   DAT_055ca018;
extern char    DAT_055ca019;
extern DWORD   DAT_055ca01c;
extern DWORD   DAT_055ca020;
extern DWORD   DAT_055ca028;
extern char    DAT_055ca031;
extern DWORD   DAT_055ca034;
extern DWORD   DAT_055ca038;
extern DWORD   DAT_055ca03c;
extern DWORD   DAT_055ca040;
extern DWORD   DAT_055ca050;
// Socket context struct (contiguous buffer, ~0x4030 bytes). Callers of
// FUN_0043db30/Net_Connect/Net_Disconnect pass the base address directly
// (originalmente el literal 0x55ca160). Los campos se superponen al buffer:
extern char    DAT_055ca160[0x260000]; // socket context object base (struct + 300-slot recv queue)
#define DAT_055ca164  (*(DWORD*)(DAT_055ca160 + 4))    // g_bGameServerConnected
#define DAT_055ca168  (*(SOCKET*)(DAT_055ca160 + 8))   // socket handle
#define DAT_055ca16c  ((char*)(DAT_055ca160 + 0xC))    // send/recv buffer
extern DWORD   DAT_055cc16c;
extern DWORD   DAT_055ce174;

// ── Network / login state (0x05826xxx) ────────────────────────────────────────
extern DWORD   DAT_05826bdc;
extern DWORD   DAT_05826c00;
extern DWORD   DAT_05826c04;
extern DWORD   DAT_05826c08;
extern DWORD   DAT_00562e48[4];   // CSimpleModulus XOR-deobfuscate key table
extern DWORD   DAT_05826c10[17];  // g_SimpleModulusCS (17 DWORDs = 68 bytes)
extern DWORD   DAT_05826c58[17];  // g_SimpleModulusSC (17 DWORDs = 68 bytes)
extern DWORD   DAT_05826c9c;
extern DWORD   DAT_05826ca0;   // HeroIndex (random 0..399 picked by ReceiveJoinMapServer)
extern DWORD   DAT_05826ca4;
extern DWORD   DAT_05826ca8;
extern DWORD   DAT_05826cac;
extern DWORD   DAT_05826cb0;   // server response code (0x0b=LoginOK, 0x14=char-list)
extern DWORD   DAT_05826cb4;
extern DWORD   DAT_05826cc0;
extern DWORD   DAT_05826cc8;
extern char    DAT_05826cc9;
extern char    DAT_05826cd4[16];       // username copy destination (needs >= 11 bytes)
extern char    DAT_05826ceb;
extern DWORD   DAT_05826cec;
extern DWORD   DAT_05826cf0;   // g_bGameServerConnected (1=connected, 0=disconnected)
extern float  _DAT_05826cf4;
extern DWORD   DAT_05826cf4;
extern DWORD   DAT_05826cf8;
extern DWORD   DAT_05826d08;   // chat rate-limit counter (max 0x46 = 70 ticks)
extern int     DAT_05826d04;   // teleport / consumable use flag (runtime global)
extern char    DAT_05826adc[0x50]; // last-sent chat message buffer (rate-limit compare)
extern char    DAT_05826d14;   // NPC script: keepalive-sent flag
extern DWORD   DAT_05826d1c;
extern DWORD   DAT_05826d20;
extern DWORD   DAT_05826d24;
extern DWORD   DAT_05826d30;
extern char    DAT_05826d31;
extern char    DAT_05826d32;
extern char    DAT_05826d33;
extern DWORD   DAT_05826d78;
extern DWORD   DAT_05826dc8;
extern DWORD   DAT_05826df4;
extern DWORD   DAT_05826e04;
// DAT_05826e08 se declara más abajo, con su comentario, en la sección "Entity / animation tick globals"
extern DWORD   DAT_05826e0c;
extern DWORD   DAT_05826e10;
extern char    DAT_05826e18[200 * 0x10];   // BoneQuaternion scratch (200 huesos x 16B)
// DAT_05828d58 declared below in "Model data table" section

// ── Bone / skeleton data (0x06970xxx) ─────────────────────────────────────────
// Bone-matrix scratch pool (see globals.cpp). All DAT_06970XXX siblings alias
// slots en el mismo buffer, con stride 0x30 entre slots de hueso adyacentes.
// Los offsets de abajo se calculan relativos a 0x06970a9c (la etiqueta de Ghidra que
// fija el slot 0). Cada macro produce un lvalue DWORD, así que `&DAT_06970XXX` devuelve
// un puntero al inicio del slot y se mantiene la semántica vieja de leer/escribir DWORD.
extern char    g_BoneScratch[200 * 0x30];   // 2026-07-17: MAX_BONES=200 (era 0x1000=85, desbordaba al preview char)
#define DAT_06970a9c   (*(DWORD*)(g_BoneScratch + 0x000))  // root bone slot
#define DAT_06970acc   (*(DWORD*)(g_BoneScratch + 0x030))
#define DAT_06970afc   (*(DWORD*)(g_BoneScratch + 0x060))
#define DAT_06970b2c   (*(DWORD*)(g_BoneScratch + 0x090))
#define DAT_06970b5c   (*(DWORD*)(g_BoneScratch + 0x0c0))
#define DAT_06970bbc   (*(DWORD*)(g_BoneScratch + 0x120))
#define DAT_06970bec   (*(DWORD*)(g_BoneScratch + 0x150))
#define DAT_06970c1c   (*(DWORD*)(g_BoneScratch + 0x180))
#define DAT_06970c4c   (*(DWORD*)(g_BoneScratch + 0x1b0))
#define DAT_06970c7c   (*(DWORD*)(g_BoneScratch + 0x1e0))
#define DAT_06970d0c   (*(DWORD*)(g_BoneScratch + 0x270))
#define DAT_06970d6c   (*(DWORD*)(g_BoneScratch + 0x2d0))
#define DAT_06970e2c   (*(DWORD*)(g_BoneScratch + 0x390))
#define DAT_06970e8c   (*(DWORD*)(g_BoneScratch + 0x3f0))
#define DAT_06970eec   (*(DWORD*)(g_BoneScratch + 0x450))
#define DAT_0697106c   (*(DWORD*)(g_BoneScratch + 0x5d0))
#define DAT_0697139c   (*(DWORD*)(g_BoneScratch + 0x900))
#define DAT_0697154c   (*(DWORD*)(g_BoneScratch + 0xab0))
#define DAT_0697157c   (*(DWORD*)(g_BoneScratch + 0xae0))
#define DAT_0697160c   (*(DWORD*)(g_BoneScratch + 0xb70))

// ── Preview character entity (0x07abf050) ─────────────────────────────────────
// BUG-FIX 2026-07-17: DAT_07abf050 ES el struct de entidad del PREVIEW char de
// char-select (creado por FUN_0045adc0/CreateCharacterPointer con model 0xab).
// Es un entity struct COMPLETO (stride 0x394; el original lo espacia 0x580 hasta
// el array en 0x07abf5d0). Estaba declarado como un DWORD de 4 bytes, así que
// CreateCharacterPointer (que escribe hasta +908) desbordaba ~900 bytes sobre los
// globales BSS adyacentes → corrupción → type@+2 basura (16247) → crash al animar/
// renderizar el preview (MoveCharacter/RenderCharacter). Los símbolos _DAT_07abf05c
// … _DAT_07abf5cc eran CAMPOS de esta entidad mal-separados por Ghidra; ahora son
// macros que proyectan dentro del buffer. Ver globals.cpp.
extern char    DAT_07abf050[0x580];
#define _DAT_07abf05c (*(float*)(DAT_07abf050 + 0x00c))
#define  DAT_07abf05c (*(DWORD*)(DAT_07abf050 + 0x00c))
#define _DAT_07abf060 (*(float*)(DAT_07abf050 + 0x010))
#define  DAT_07abf060 (*(DWORD*)(DAT_07abf050 + 0x010))
#define _DAT_07abf064 (*(float*)(DAT_07abf050 + 0x014))
#define  DAT_07abf064 (*(DWORD*)(DAT_07abf050 + 0x014))
#define _DAT_07abf068 (*(float*)(DAT_07abf050 + 0x018))
#define  DAT_07abf068 (*(DWORD*)(DAT_07abf050 + 0x018))
#define _DAT_07abf06c (*(float*)(DAT_07abf050 + 0x01c))
#define  DAT_07abf06c (*(DWORD*)(DAT_07abf050 + 0x01c))
#define _DAT_07abf070 (*(float*)(DAT_07abf050 + 0x020))
#define  DAT_07abf070 (*(DWORD*)(DAT_07abf050 + 0x020))
#define  DAT_07abf0d4 (*(DWORD*)(DAT_07abf050 + 0x084))
#define _DAT_07abf138 (*(float*)(DAT_07abf050 + 0x0e8))
#define  DAT_07abf138 (*(DWORD*)(DAT_07abf050 + 0x0e8))
#define _DAT_07abf13c (*(float*)(DAT_07abf050 + 0x0ec))
#define  DAT_07abf13c (*(DWORD*)(DAT_07abf050 + 0x0ec))
#define _DAT_07abf140 (*(float*)(DAT_07abf050 + 0x0f0))
#define  DAT_07abf140 (*(DWORD*)(DAT_07abf050 + 0x0f0))
#define  DAT_07abf20c (*(DWORD*)(DAT_07abf050 + 0x1bc))
extern char    DAT_07d2b494[];  // class name table (stride 300, 30 slots)
#define _DAT_07abf5c4 (*(float*)(DAT_07abf050 + 0x574))  // preview entity field (BodyOrigin x)
#define _DAT_07abf5c8 (*(float*)(DAT_07abf050 + 0x578))  // preview entity field (BodyOrigin y)
#define _DAT_07abf5cc (*(float*)(DAT_07abf050 + 0x57c))  // preview entity field (BodyOrigin z)
extern DWORD   DAT_07abf5d0;   // entity array base (stride 0x394)
extern int     DAT_07abf5d4;   // visible entity counter (reset each frame)
extern char   *DAT_07abf5d8;   // local player entity ptr
extern DWORD   DAT_07abf5dc;
extern DWORD   DAT_07abf5e0;
extern float  _DAT_07abf5e8;   // extra flag (reset 0 each frame)
extern DWORD   DAT_07abf5e8;
extern char    DAT_07abf5f0[3000 * 0x70];   // particle pool — 3000 slots × 112B
extern char    DAT_007abf06;   // (low byte flag)
extern float  _DAT_007abf06;

// ── Skill effects pool (0x07b1xxxx – 0x07c7xxxx) ─────────────────────────────
extern DWORD   DAT_07b11698;
extern DWORD   DAT_07b116d0;
extern DWORD   DAT_07b27b08;
extern unsigned char DAT_07c5ab3c[200 * 0x70];   // SkillEffect pool: 200 slots × 0x70 bytes
// El unk_7C5AB5C de IDA no es una segunda alocación: es el +0x20 dentro del
// pool de efectos 0x7C5AB3C. Weather_Update lo recorre con stride de 0x70 bytes.
// Mantenerlo como global separado desplaza ese recorrido al pool de blur.
#define DAT_07c5ab5c (*(DWORD*)(DAT_07c5ab3c + 0x20))
// RenderCharacter (00456770) captures these bone transforms for action kind
// 0x4D; AttackEffect (00445230) consume los dos primeros en un tick posterior.
extern float   g_AttackEffectMatrix_04D[3][4];
extern float   g_AttackEffectMatrix_04D_Alt[3][4];
extern float   g_AttackEffectMatrix_04D_Aux[3][4];
// Pool de render de joints/trails/blur — ver globals.cpp. La base es el array
// `g_RenderPool_07c608a8`; DAT_07c608b4 es el campo ancla +12 del slot[0].
extern char    g_RenderPool_07c608a8[100 * 0x2f0];
#define DAT_07c608b4  (*(DWORD*)(&g_RenderPool_07c608a8[12]))
extern DWORD   DAT_07c74ae0;
extern DWORD   DAT_07c74ae4;   // weather intensity target (interpolated)
extern DWORD   DAT_07c74ae8;   // rain Y offset (driven by fsin)
extern DWORD   DAT_07c74aec;   // rain position counter (0..2000)

// ── Player render pool ─────────────────────────────────────────────────────────
// 2026-05-07: re-allocado propiamente. v1 walker de Player_Render arranca en
// DAT_07c74f54 y lee offsets NEGATIVOS hasta -0xEC. El pool REAL es
// g_PlayerRenderPool[100 × 0x1BC] cubriendo todos los slots; DAT_07c74f54 es
// pointer alias a offset +0xEC (donde v1 vive en cada slot iter).
extern char    g_PlayerRenderPool[100 * 0x1BC];
extern DWORD*  DAT_07c74f54;   // = g_PlayerRenderPool + 0xEC (v1 anchor)

// ── g_CharData XOR-encoded block (0x07cfxxxx) ─────────────────────────────────
extern void   *DAT_07cf1ff4;   // g_CharData sub-pointer (XOR-encoded)
extern void   *DAT_07cf1ffc;   // g_CharData pointer (XOR-encoded, 0x584 bytes)

// ── Player input / targeting state (0x07e01xxx – 0x07e11xxx) ─────────────────
extern DWORD   DAT_07e016c0;   // target grid X (from ray cast or hover click)
extern DWORD   DAT_07e016c4;   // target grid Y
extern DWORD   DAT_07e109c8;   // hover NPC entity index (for pathfind target)
extern DWORD   DAT_07db8708;   // hover target entity type (from entity+2)
extern char    DAT_07e113e4[5 * 256];   // historial de chat: 5 entradas x 256B

// ── UI / HUD data (0x07e1xxxx – 0x07eaxxxx) ──────────────────────────────────
extern DWORD  _DAT_07e118e4;   // player facing angle (float, sent in movement packets)
extern DWORD   DAT_07e118e8;   // world/map type
extern char    DAT_07e11d70;   // char-select init flag
extern char    DAT_07e11d71;   // char-select flag B
extern char    DAT_07e11d72;   // char-select flag C
extern int     DAT_07e11d74;   // gold / currency (checked vs 0x2faf081)
extern DWORD   DAT_07e11d78;   // login field active flag
extern DWORD   DAT_07e11d7c;   // countdown counter B
extern DWORD   DAT_07e11d1c;   // per-frame cooldown tick counter
extern DWORD   DAT_07e11d28;   // movement debounce step counter
extern DWORD   DAT_07e11d30;   // frame counter (anti-tamper)
extern int     DAT_07e11d5c;   // cursor sprite size/frame index
extern DWORD   DAT_07e11d64;   // terrain walk flag (0=walkable 1=blocked)
extern float  _DAT_07e11d4c;   // facing tick reference (last frame time)
extern float  _DAT_07e11d50;   // facing dt (delta * speed scale)
extern BYTE    DAT_07e113d8[40];   // world-enter counter (memset writes 40 bytes)
extern char&   DAT_07e113d9;   // alias de DAT_07e113d8[1] (password-mask flag)
extern DWORD   DAT_07e11d8c;
extern DWORD   DAT_07e11d90;
extern DWORD   DAT_07e11d94;
extern DWORD   DAT_07e11d98;
extern DWORD   DAT_07e11da0;
extern DWORD   DAT_07e11db4;
extern DWORD   DAT_07e11db8;   // movement step count (must be >0x27 for facing packet)
extern int     DAT_07e11dbc;   // last facing tick time
extern char    DAT_07e11dc0;   // movement lock flag B
extern DWORD   DAT_07e11dc4;   // NPC script: dialog-active flag
extern DWORD   DAT_07e11dc8;   // NPC script: keepalive timer (GetTickCount at last send)
extern DWORD   DAT_07e11dcc;
extern DWORD   DAT_07e11e50;   // NPC script chat-log slot 0
extern DWORD   DAT_07e11e54;   // NPC script chat-log slot 1
extern DWORD   DAT_07e11e58;   // NPC script chat-log slot 2
extern DWORD   DAT_07e11e5c;   // NPC script chat-log slot 3
extern DWORD   DAT_07e11de8;
extern DWORD   DAT_07e11e78;   // target entity index (active selection)
extern DWORD   DAT_07e11e98;   // party HP bars (stride 0x24)
extern char    DAT_07e11e9c;   // party slot 0 name base (stride 0x24; +0x1c=entity idx, +0x20=pos ptr)
extern char    DAT_07e11d6e;            // flag reset each login frame
extern DWORD   DAT_07d552e4;            // UI inventory/item selection state
extern DWORD   DAT_07d78094;            // UI click/selection flag
extern BYTE    DAT_07d780a8[40];         // username field length (memset writes 40 bytes)
extern DWORD   DAT_07d780ac;            // password field length
// IDA: InputText[10][256] @ 0x07db8710. Slot 0 = username/chat, slot 1 =
// contraseña/destino-del-susurro. DAT_07db8810 es el alias +0x100 del slot 1 que usan
// los caminos de login/susurro; el mismo storage respalda a RenderInputText.
extern char    DAT_07db8710[10][256];    // input slot table (10 × 256 bytes)
#define DAT_07db8810   (DAT_07db8710[1]) // alias: +0x100 = slot 1
extern char    DAT_07d4c3ec[256];       // connecting status string buffer
extern char    DAT_07d4c644[256];       // char name send buffer
extern char    DAT_07d4c770[256];       // char name confirm buffer
extern DWORD   DAT_07d52c38;            // server select render flag
extern char    DAT_07d530e8[256];       // server group name buffer
extern char    DAT_07d53214[256];       // server channel name buffer
extern char    DAT_07d4ac7c[256];       // username display buffer
extern char    DAT_07d4ada8[256];       // password display buffer
extern char    lpString_07d4aed4[128];  // OK button text
extern char    lpString_07d4b000[128];  // Exit/Cancel button text
extern char    DAT_07d4b708[128];       // char name format string
extern char    DAT_07d4b12c[128];       // version string 1
extern char    DAT_07d4b258[128];       // version string 2
extern char    DAT_07d4b384[128];       // version string 3 format
extern char    lpString_07d4c518[128];  // "Connecting..." string
extern DWORD   DAT_07e127f8;
extern unsigned char DAT_07e12840[1000 * 0x204];   // GroundItem pool: 1000 × 0x204 bytes
extern DWORD   DAT_07e12945;   // sprite render loop base (stride 0x204)
extern char    DAT_07e91350[0x44];      // pPickedItem — sizeof(ITEM) = 0x44
extern DWORD   DAT_07e91388;
extern byte    DAT_07e9138e;   // UI grid selected column (byte, FUN_004cd3b0)
extern byte    DAT_07e9138f;   // UI grid selected row (byte, FUN_004cd3b0)
extern DWORD   DAT_07e91394;
extern DWORD   DAT_07e913a8;
extern DWORD   DAT_07e91428;
extern DWORD   DAT_07e91784;
extern DWORD   DAT_07e91788;
extern DWORD   DAT_07e919b8;
// Tabla de nombres stride 80 (guild members / buffs). NO es un DWORD: los
// consumidores (HUD_Pass2, HUD_Pass3, SecondPassword, SMD_Parser y el
// handler 0x52 de Net_Process) la recorren con `&DAT_07e919bc + N*80`.
extern char    DAT_07e919bc[0x13C30];
// Pools de índices de nombres de personaje — cada uno es un buffer de 0x880 bytes (32 entradas × 0x44).
// Lo recorre FUN_004cba60 (CharPreview_Reset) con stride 0x44.
extern BYTE    DAT_07ea5298[0x880];
extern DWORD   DAT_07ea5b18;
extern DWORD   DAT_07ea5b1c;
extern DWORD   DAT_07ea5b20;
extern DWORD   DAT_07ea8410;
extern DWORD   DAT_07ea8414;
// Equipment grid buffer.  IDA layout:
//   FUN_00482be0 walks `&unk_7EA9504` outer-stride -68 (= -17 dwords) for 7
//   filas, y stride interno -544 (= -136 dwords) para 8 columnas, leyendo
//   `*(short*)(v7 - 56)` (Type) y `*v7` (Qty). El externo va desde
//   unk_7EA9504 down to unk_7EA9328 (= 7*68 = 476 bytes back).  Inner
//   avanza 8 celdas × 544 bytes = 4352 bytes hacia atrás desde la posición externa.
//   O sea el buffer se extiende al menos desde (unk_7EA9504 - 4828) hasta
//   unk_7EA9504, totalling 4828 bytes.
//
// Acá exponemos un único buffer contiguo y reenraizamos DAT_07ea9504 y
// DAT_07ea9328 sobre los EXTREMOS de puntero dentro de ese buffer (vía accesores
// inline en stubs.cpp). Así `&DAT_07ea9504` == fin-del-buffer y el recorrido
// reads valid ITEM-shaped memory throughout.
extern BYTE    g_EquipGridBuf[0x12DC];           // 4828 bytes
extern int    *p_DAT_07ea9504_;                  // &g_EquipGridBuf[end-4]
extern int    *p_DAT_07ea9328_;                  // &g_EquipGridBuf[end-0x1E0]
#define DAT_07ea9504  (*p_DAT_07ea9504_)
#define DAT_07ea9328  (*p_DAT_07ea9328_)
extern DWORD   DAT_07ea9800;
extern DWORD   g_ItemMoveSourcePool;
extern DWORD   g_ItemMoveTargetPool;
extern DWORD   DAT_07ea9804;   // NPC shop context A
extern DWORD   DAT_07ea9808;   // NPC shop context B
extern DWORD   DAT_07ea980c;   // NPC shop context C
extern DWORD   DAT_07ea9810;
extern DWORD   DAT_07ea9814;
extern float  _DAT_07ea9814;
extern char    DAT_07ea9815;
extern char    DAT_07ea9816;
extern char    DAT_07ea9817;
extern unsigned int DAT_07ea9818;  // SecondPassword PIN bytes [4-7] — widened from char (audit #8)
extern DWORD   DAT_07ea981c;
extern short   DAT_07ea981e;   // second-password shuffle state (short, FUN_004e9250)
extern DWORD   DAT_07ea982c;   // Screen3 panel origin X
extern DWORD   DAT_07ea9830;   // Screen3 panel origin Y
extern DWORD   DAT_07ea9834;
extern char    DAT_07ea983e;
extern DWORD   DAT_07eaa0d0;
extern DWORD   DAT_07eaa0d8;
extern DWORD   DAT_07eaa0e0;
extern DWORD   DAT_07eaa0e4;
extern DWORD   DAT_07eaa0f0;
extern DWORD   DAT_07eaa0f4;
extern int     DAT_07eaa0f8;   // CharData_RecalcDurability accumulator (count of items needing repair)
extern DWORD   DAT_07eaa0fc;
extern char    DAT_07eaa0fd;
extern DWORD   DAT_07eaa104;   // shop/item selection A
extern DWORD   DAT_07eaa108;   // shop/item selection B
extern char    DAT_07eaa114;
extern char    DAT_07eaa115;
extern char    DAT_07eaa116;
extern char    DAT_07eaa117;
extern char    DAT_07eaa118;
extern char    DAT_07eaa119;
extern char    DAT_07eaa11a;
extern char    DAT_07eaa11b;
extern char    DAT_07eaa11c;
extern DWORD   DAT_07eaa124;
extern DWORD   DAT_07eaa128;
extern DWORD   DAT_07eaa13c;
extern DWORD   DAT_07eaa16c;   // guild join/select current selection index
extern DWORD   DAT_07eaa144;
extern DWORD   DAT_07eaa148;   // shop/item selection C
extern DWORD   DAT_07eaa14c;
extern DWORD   DAT_07eaa150;
extern DWORD   DAT_07eaa154;
extern DWORD   DAT_07eaa160;
extern char    DAT_07eaa165;   // char-select packet-sent flag
extern DWORD   DAT_07eaa168;   // TextureEnable (GL texture state cache)
extern char    DAT_07eaa179;
extern char    DAT_07eaa190;   // inventory drop error message ID string
// Frustum world corners: 5 corners × 3 floats = 15 DWORDs at 0x07eab1b0
extern float   DAT_07eab1b0;   // corner 0 X (apex)
extern float   DAT_07eab1b4;   // corner 0 Y
extern float   DAT_07eab1b8;   // corner 0 Z
extern float   DAT_07eab1bc;   // corner 1 X (top-left)
extern float   DAT_07eab1c0;   // corner 1 Y
extern float   DAT_07eab1c4;   // corner 1 Z
extern float   DAT_07eab1c8;   // corner 2 X (top-right)
extern float   DAT_07eab1cc;   // corner 2 Y
extern float   DAT_07eab1d0;   // corner 2 Z
extern float   DAT_07eab1d4;   // corner 3 X (bot-right)
extern float   DAT_07eab1d8;   // corner 3 Y
extern float   DAT_07eab1dc;   // corner 3 Z
extern float   DAT_07eab1e0;   // corner 4 X (bot-left)
extern float   DAT_07eab1e4;   // corner 4 Y
extern float   DAT_07eab1e8;   // corner 4 Z
extern DWORD   DAT_07eab1ec;
extern DWORD   DAT_07eab1f0;
extern DWORD   DAT_07eab1f4;
extern DWORD   DAT_07eab1f8;
extern float   DAT_07eab200[256 * 256];  // water-wave heights (row*256+col floats)
extern DWORD   DAT_07eab24c;   // BackTerrainHeight array base
extern DWORD   DAT_07eab250;   // MISLABEL: NO es PrimaryTerrainLight (ese es DAT_081cb608 @0x081CB608). Global muerto/no asignado.
extern DWORD   DAT_07eeb200;
extern DWORD   DAT_07eeb204;
extern DWORD   DAT_07eeb208;
extern DWORD   DAT_07eeb20c;
extern DWORD   DAT_07eeb210;
extern float   DAT_07eeb214;      // WaterMove — terrain water UV scroll offset (RenderTerrain)
extern float   DAT_07eeb218[4];   // frustum quad Y[4] (per FUN_004f8ff0)
extern float   DAT_07eeb228[4];   // frustum quad X[4]
extern float   DAT_07eeb238[256 * 256 * 3];   // TerrainLight RGB ambient buffer (256x256 RGB floats)
extern DWORD   DAT_07feb238;
extern DWORD   DAT_07feb23c;
extern float   g_TerrainTexCoord[8];   // TerrainTextureCoord[4][2] — UVs per tile face (RenderTerrainFace/FaceTexture)
// 2026-04-28: tile pick corners buffer (12 floats contiguous, 4 vec3 corners)
extern float   g_TilePickBuf[12];
#define DAT_07feb258  (g_TilePickBuf[0])
#define DAT_07feb25c  (g_TilePickBuf[1])
#define DAT_07feb260  (g_TilePickBuf[2])
#define _DAT_07feb264 (g_TilePickBuf[3])
#define _DAT_07feb268 (g_TilePickBuf[4])
#define _DAT_07feb26c (g_TilePickBuf[5])
#define _DAT_07feb270 (g_TilePickBuf[6])
#define _DAT_07feb274 (g_TilePickBuf[7])
#define _DAT_07feb278 (g_TilePickBuf[8])
#define _DAT_07feb27c (g_TilePickBuf[9])
#define _DAT_07feb280 (g_TilePickBuf[10])
#define _DAT_07feb284 (g_TilePickBuf[11])
extern float   DAT_07feb288[256 * 256 * 3];   // TerrainNormal[256*256][3] vertex normals

// ── Large game data arrays (0x080xxxxx – 0x083bxxxx) ─────────────────────────
extern DWORD   DAT_080ab288;   // cursor screen X (float)
extern DWORD   DAT_080ab28c;   // cursor screen Y (float)
extern unsigned char  DAT_080ab2b4[0x10000];   // TileTex2[256*256] tile texture index 2
extern unsigned char  DAT_080bb2b4[0x10000];   // TileTex1[256*256] tile texture index 1
extern float   DAT_080cb2cc[0x10000];          // BackTerrainHeight[256*256] tile height
extern float   DAT_0810b2cc[0x10000];          // TerrainNoise[256*256] random noise init
extern DWORD   DAT_0814b2dc;
// Ambient terrain-object table — sub_4F7060 (FUN_004f7060) walks 100 records de
// stride 8 leyendo campos en [-5..+1].  Backing zeroado con prefijo de 5 bytes
// para que las lecturas [-5] queden in-bounds; vacío en nuestro build (el loader
// de objetos .obj no está wireado) → el loop no spawnea nada.
// &DAT_081cb2ed == &g_TerrainObjTable[5].
extern BYTE    g_TerrainObjTable[0x328];
#define DAT_081cb2ed  (g_TerrainObjTable[5])
// 2026-05-04: buffer de iluminación por tile, vivo — dimensionado para 256×256 tiles × 3
// floats. Antes era un DWORD de 4 bytes; las macros de abajo proyectan DAT_081cb60c/610
// en los campos del 2do/3er slot, como el layout contiguo del binario original.
extern float   DAT_081cb608[256 * 256 * 3];
extern float   DAT_0828b608[256 * 256 * 3];   // TerrainLightData[256*256][3] post-normal lighting
// cb60c/cb610 son el 2do/3er DWORD del slot 0 de cb608 (y análogamente para
// 0828b608). El código los accede como `(&DAT_*60c)[iVar2*3]` (lee el 2do campo
// del slot iVar2) o `(char*)&DAT_*60c + iVar2*12` (escribe en el 2do campo).
#define DAT_081cb60c   (*(DWORD*)((char*)&DAT_081cb608[0] + 4))
#define DAT_081cb610   (*(DWORD*)((char*)&DAT_081cb608[0] + 8))
#define DAT_0828b60c   (*(DWORD*)((char*)&DAT_0828b608[0] + 4))
#define DAT_0828b610   (*(DWORD*)((char*)&DAT_0828b608[0] + 8))
extern float   DAT_0834b608[0x10000];          // TerrainMappingAlpha[256*256] — blend factor layer1↔layer2 (IDA/DLL; NO es height)
extern float   DAT_0838b7c4;  // frustum plane 0 normal X
extern float   DAT_0838b7c8;  // frustum plane 0 normal Y
extern float   DAT_0838b7cc;  // frustum plane 0 normal Z
extern float   DAT_0838b7d0;  // frustum plane 1 normal X
extern float   DAT_0838b7d4;  // frustum plane 1 normal Y
extern float   DAT_0838b7d8;  // frustum plane 1 normal Z
extern float   DAT_0838b7dc;  // frustum plane 2 normal X
extern float   DAT_0838b7e0;  // frustum plane 2 normal Y
extern float   DAT_0838b7e4;  // frustum plane 2 normal Z
extern float   DAT_0838b7e8;  // frustum plane 3 normal X
extern float   DAT_0838b7ec;  // frustum plane 3 normal Y
extern float   DAT_0838b7f0;  // frustum plane 3 normal Z
extern float   DAT_0838b7f4;  // frustum plane 4 (near) normal X
extern float   DAT_0838b7f8;  // frustum plane 4 (near) normal Y
extern float   DAT_0838b7fc;  // frustum plane 4 (near) normal Z
extern unsigned char DAT_0838b800[1080];   // BMPHeader (BITMAPFILEHEADER + DIB + palette)
extern DWORD   DAT_0838bc44;    // TerrainFlag (RenderTerrain pass selector: 0=base, 2=alpha overlay)
extern char    DAT_0838bc70[];  // char array (monster name/flag table)
extern DWORD   DAT_0839bc88;    // terrain-light double-buffer toggle (RenderTerrain ^=1 cada frame)
extern DWORD   DAT_0839bc86;
extern DWORD   DAT_0839bc8c;
extern DWORD   DAT_0839bc90;
extern DWORD   DAT_0839bc94;
extern DWORD   DAT_0839bc98;
extern DWORD   DAT_0839bc9c;
extern DWORD   DAT_0839be18;

// ── Scene / state machine vars (0x083a0xxx – 0x083bxxxx) ─────────────────────
extern char    DAT_083a2370[0x960];   // Entity render-state pool (stride 0xc, 128 slots, 0x960 bytes)
#define DAT_083a2cd0 DAT_083a2370[0x960-1]  // end-marker alias
extern char    DAT_083a2e90[10 * 0x1bc];   // Boids pool: 10 entries × 0x1bc bytes
extern DWORD   DAT_083a2378;   // hover special-object entity table (stride 3*int, ~24 entries)
extern float  _DAT_083a0210;
extern DWORD   DAT_083a0210;
// Object-bucket grid (see globals.cpp). 16×16 cells × 16 B = 0x1000.
// Cell layout (matches original binary 0x083a0218..0x083a1217):
//   cell+0  → alias DAT_083a0218 (base de la celda, también usada como scratch por el walker de descarga FUN_004ffd50)
//   cell+4  → DAT_083a021c  puntero head (Terrain_Render lo lee vía *chunk_ptr)
//   cell+8  → puntero tail  (el insert de FUN_004ff5a0 appendea acá; la descarga arranca el recorrido desde acá)
//   cell+12 → visibility flag (Terrain_Render writes *(chunk_ptr+8))
// IMPORTANTE: g_ObjectBucketGrid[0] representa la dirección 0x083a0218, así que DAT_083a0218 está
// en el offset 0 y DAT_083a021c en el +4. Antes DAT_083a021c estaba en el offset 0
// y DAT_083a0218 era un DWORD aparte — eso hacía que la descarga (`puVar5 = &DAT_083a0218;
// while (head = *(puVar5+8)) ...`) leyera BSS sin inicializar adyacente al DWORD huérfano,
// y crasheara con el primer valor basura no nulo.
extern char    g_ObjectBucketGrid[0x1000];
#define DAT_083a0218  (*(DWORD*)(g_ObjectBucketGrid + 0))
#define DAT_083a021c  (*(DWORD*)(g_ObjectBucketGrid + 4))
extern DWORD   DAT_083a1378;
extern DWORD   DAT_083a2e92;
extern DWORD   DAT_083a3ff0;
extern char    DAT_083a410c;   // Scene_Intro: normal-path flag
// DAT_083a4110_arr — 3-DWORD storage para mouse-ray endpoint (Camera_MouseRay
// escribe out_ray[0..2]). DAT_083a4110 es referencia al [0].
extern DWORD   DAT_083a4110_arr[3];
extern DWORD&  DAT_083a4110;
extern DWORD   DAT_083a4124;  // MouseLButtonPush flag — VALORES VÁLIDOS: {0,1}
// Helper de defensa: usar IsClickPushed() en vez de truthy-check `&& DAT_083a4124`.
// Algún stub stompea DAT_083a4124 con un valor wild (0x4397355A observado) que
// pasaría como verdadero y dispara phantom clicks. La igualdad estricta == 1
// suprime el síntoma mientras buscamos al culpable.
static inline bool IsClickPushed() { return DAT_083a4124 == 1; }
extern float   DAT_083a4130;   // cursor billboard screen X
extern float   DAT_083a4134;   // cursor billboard screen Y
extern float   DAT_083a4138;   // terrain pick Z result
extern DWORD   DAT_083a413c;
// DAT_083a4140 es un buffer de 48 bytes (matrix 3×4 floats) — ver globals.cpp.
// DAT_083a414c / DAT_083a415c / DAT_083a416c son referencias a índices 3/7/11
// del mismo array; comparten memoria para que FUN_005111d0 (que escribe 12
// DWORDs) no desborde sobre globales vecinos.
extern DWORD   DAT_083a4140[12];
extern DWORD&  DAT_083a414c;
extern DWORD&  DAT_083a415c;
extern DWORD&  DAT_083a416c;
extern char    DAT_083a4174[0x104];   // screenshot name buf (26 chars+)
extern DWORD   DAT_083a4278;
extern DWORD   DAT_083a427c;
extern DWORD   DAT_083a4280;   // viewport width (pixels)
// DAT_083a4284_arr — 3-DWORD storage para camera world position (escrito por
// Camera_MouseRay vía FUN_004fa110(...&DAT_083a4284)). _DAT_083a4284..428c son
// aliases float al mismo storage.
extern DWORD   DAT_083a4284_arr[3];
extern DWORD&  DAT_083a4284;
extern DWORD&  DAT_083a4288;
extern DWORD&  DAT_083a428c;
extern char    DAT_083a4299;
extern DWORD   DAT_083a429c;
extern DWORD   DAT_083a42a0;
extern DWORD   DAT_083a42a4;
extern DWORD   DAT_083a42a8;
extern DWORD   DAT_083a42ac;   // UI flag: mouse button state
extern DWORD   DAT_083a42b0;   // viewport height (pixels)
// CameraAngle/CameraPosition contiguous — DAT_ names are aliases (see globals.cpp)
extern DWORD&  DAT_083a42b8;   // alias → CameraAngle[0]
extern DWORD&  DAT_083a42bc;   // alias → CameraAngle[1]
extern DWORD&  DAT_083a42c0;   // alias → CameraAngle[2]
extern DWORD   DAT_083a42c4;
extern DWORD   DAT_083a42c8;   // viewport x offset
extern DWORD   DAT_083a42cc;   // viewport y offset
extern DWORD   DAT_083a42d0;   // UI flag: right-click / context menu
extern float&  _DAT_083a42d4;  // alias → CameraPosition[0]
extern DWORD&  DAT_083a42d4;   // alias → CameraPosition[0] (DWORD view)
extern float&  _DAT_083a42d8;  // alias → CameraPosition[1]
extern DWORD&  DAT_083a42d8;   // alias → CameraPosition[1] (DWORD view)
extern float&  _DAT_083a42dc;  // alias → CameraPosition[2]
extern DWORD&  DAT_083a42dc;   // alias → CameraPosition[2] (DWORD view)
extern char    DAT_083a42e9;
extern char    DAT_083a42ea;   // reset to 0 each frame in Scene_Loading
extern char    DAT_083a42eb;   // auto-drop trigger flag (inventory)
extern DWORD   DAT_083a42ec;
// DAT_083a42fc  — macro alias dentro de DAT_083a42f8 (ver bloque de dialog button rects)
extern DWORD   DAT_083a4320;
// DAT_083a4324 @ 0x083A4324 ES `g_iNumLineMessageBoxCustom` — la cantidad de
// lineas que SeparateTextIntoLines dejo en el cuadro de dialogo.
// Confirmado en el disassembly de CSQuest::ShowDialogText 0x401806:
// `a3 24 43 3a 08` = `mov [0x083A4324], eax` justo despues del
// SeparateTextIntoLines.  La etiqueta vieja ("shop item list display count")
// era una misidentificacion.
extern DWORD   DAT_083a4324;
#define g_iNumLineMessageBoxCustom  (*(int*)&DAT_083a4324)
extern DWORD   DAT_083a4328;
// CurrentCameraPosition[3] contiguous (see globals.cpp).
extern float   CurrentCameraPosition[3];
extern DWORD&  DAT_083a432c;   // alias → CurrentCameraPosition[0]
extern DWORD&  DAT_083a4330;   // alias → CurrentCameraPosition[1]
extern float&  _DAT_083a4334;  // alias → CurrentCameraPosition[2]
extern DWORD&  DAT_083a4334;   // alias → CurrentCameraPosition[2] (DWORD view)
extern char    DAT_083a44c4[7 * 0x26];   // g_lpszMessageBoxCustom — 7×38-byte dialog buffer
extern float   DAT_083a45d4;   // login background animation angle (Scene_Dispatch)
extern char    DAT_083a45d8[0x3600];   // server list buffer (stride 0x21e, 24 entries); overlaps sub-symbols below
#define DAT_083a45ec   (*(DWORD*)(DAT_083a45d8 + 0x14))   // offset 0x083a45ec
#define DAT_083a45ed   (*(char *)(DAT_083a45d8 + 0x15))   // offset 0x083a45ed
#define DAT_083a4604   (*(DWORD*)(DAT_083a45d8 + 0x2c))   // offset 0x083a4604
#define DAT_083a4606   (*(WORD *)(DAT_083a45d8 + 0x2e))   // offset 0x083a4606
// Name-field aliases usados por ReceiveServerList (F4/02). Ambos caen dentro
// del array DAT_083a45d8[0x3600]: slot 23 (0x30B2) y slot 24 (0x32D0).
#define DAT_083a768a   (DAT_083a45d8 + 0x30B2)   // name field of display slot 23
#define DAT_083a78a8   (DAT_083a45d8 + 0x32D0)   // name field of display slot 24 (event)
// Local server-name table poblada por F4/04 (custom list); stride 300 por grupo.
extern char    DAT_07d52c34[24 * 300];

// Login/scene init flags (byte-sized)
extern char    DAT_083a7ac8;
extern char    DAT_083a7acc;
// CurrentCameraAngle[3] contiguous (see globals.cpp).
extern float   CurrentCameraAngle[3];
extern float&  DAT_083a7ad0;   // alias → CurrentCameraAngle[0]
extern float&  DAT_083a7ad4;   // alias → CurrentCameraAngle[1]
extern float&  DAT_083a7ad8;   // alias → CurrentCameraAngle[2]
extern DWORD   DAT_083a7af4;   // fade-in flag (1 = fading in)
// ── FUN_004fdc00 (Object_RenderUpdate) full IDA port activation ──────────────
// Activa el port completo de sub_4FDC00 en stubs_IDA_ports.cpp y desactiva el
// minimal de stubs_linker.cpp. Aliases IDA→DAT que el port full necesita.
#define IDA_PORT_004FDC00 1

// ── FUN_00445230 (AttackEffect) full IDA port activation ─────────────────────
// 2026-08-16: `AttackEffect` es la que spawnea los efectos VISUALES de los
// skills (CreateEffect 191/200/201/223/240/241/568/1210/1211/1271, CreateJoint
// 1253...). El port fiel de IDA (2043 lineas) vivia en stubs_IDA_ports.cpp pero
// su gate nunca se definio, asi que se compilaba una version PARCIAL de 374
// lineas en stubs_misc2.cpp -> Lightning no mostraba nada y Evil Spirit /
// Inferno salian incompletos. Activado el port completo.
// El ruido anti-tamper crudo (CErrorReport::Write/aHashTableFullG, hash table
// FUN_004041e0/FUN_00403f80 con otras firmas, delete__, PACKET_ENCRYPT, `Models`)
// quedo neutralizado con shims locales al inicio del bloque gated en
// stubs_IDA_ports.cpp (con sus #undef al final). Ver CLAUDE.md 2026-08-16.
#define IDA_PORT_00445230 1
#define CameraWalkCut       DAT_083a7af4
#define CurrentCameraCount  DAT_005615e8
extern DWORD   DAT_083a7c00;
extern DWORD   DAT_083a7c04;   // in-game UI context A
extern DWORD   DAT_083a7c08;   // in-game UI context B
extern char    DAT_083a7c09;   // in-game UI flag byte
// DAT_083a7c0c @ 0x083A7C0C ES `g_iNumAnswer`.  Disassembly 0x40182E:
// `89 35 0c 7c 3a 08` = `mov [0x083A7C0C], esi`.
extern DWORD   DAT_083a7c0c;
#define g_iNumAnswer  (*(int*)&DAT_083a7c0c)
extern DWORD   DAT_083a7c10;
extern int     DAT_083a7c14;   // login sub-state machine
extern DWORD   DAT_083a7c18;
extern DWORD   DAT_083a7c1c;
extern DWORD   DAT_083a7c20;
extern DWORD   DAT_083a7c24;   // secondary in-game UI state machine
extern DWORD   DAT_083a7c28;   // queued next UI state (applied at end of tick)
extern DWORD   DAT_083a7c2c;   // selected item column (set by state 0x8f click)
extern DWORD   DAT_083a7c38;
extern DWORD   DAT_083a7c3c;
extern DWORD   DAT_083a7c40;
extern DWORD   DAT_083a7c44;
extern char    DAT_083a7c48;   // connection-check-enable flag (byte, NO DWORD)
extern char    DAT_083a7c49;   // Scene_Login init flag
extern char    DAT_083a7c4a;   // Scene_Loading init flag
extern char    DAT_083a7c4b;
extern char    DAT_083a7c4c;
extern char    DAT_083a7c4d;
extern DWORD   DAT_083a7c50;
extern DWORD   DAT_083a7c54;
extern DWORD   DAT_083a7c58;
extern DWORD   DAT_083a7c5c;   // chat-log label for exit-countdown
extern char    DAT_083a7c64[64];  // server IP string buffer (filled on server select)
extern DWORD   DAT_083a7c68;
extern DWORD   DAT_083a7c6c;
extern DWORD   DAT_083a7c70;
extern DWORD   DAT_083a7c74;
extern DWORD   DAT_083a7c78;
extern DWORD   DAT_083a7c7c;
extern DWORD   DAT_083a7c80;
extern DWORD   DAT_083a7c84;
extern DWORD   DAT_083a7c88;
extern DWORD   DAT_083a7c8c;
extern DWORD   DAT_083a7c90;
extern DWORD   DAT_083a7c94;
extern DWORD   DAT_083a7c98;
// ── Bitmaps table (contiguous, 1450 entries × 0x38 stride = 0x13D30 bytes) ───
// Original address range: 0x083A7CA0 .. 0x083BB9D0
// Slot layout: +0x00 active, +0x20 width(f), +0x24 height(f), +0x28 bpp(b),
//              +0x2C gl_handle(u), +0x30 ref_count(b), +0x34 pixels(ptr)
extern char    g_BitmapsRaw[0x13D30];

// Accesores retrocompatibles al slot[0] — el código usa (&DAT_xxx)[offset]
// para indexar la tabla contigua con aritmética de punteros.
#define DAT_083a7ca0  (*(DWORD*) &g_BitmapsRaw[0x00])
#define DAT_083a7cc0  (*(DWORD*) &g_BitmapsRaw[0x20])
#define DAT_083a7cc4  (*(DWORD*) &g_BitmapsRaw[0x24])
#define DAT_083a7cc8  (*(char*)  &g_BitmapsRaw[0x28])
#define DAT_083a7ccc  (*(DWORD*) &g_BitmapsRaw[0x2C])
#define DAT_083a7cd0  (*(char*)  &g_BitmapsRaw[0x30])
#define DAT_083a7cd4  (*(DWORD*) &g_BitmapsRaw[0x34])
extern DWORD   DAT_083bb9d0;
extern DWORD   DAT_083bbb14;
extern char    lpBuffer_083bbb60[0x400]; // named pipe write buffer (FUN_005403a0)
extern DWORD   DAT_083bbb64;             // named pipe bytes written count
extern DWORD   DAT_083bbb68;
extern DWORD   DAT_083bbb6c;
extern DWORD   DAT_083bbb74;

// ── Named pipe handle (0x00563b58) ────────────────────────────────────────────
extern HANDLE  lpTargetHandle_00563b58;  // handle to named pipe (MuHelper/plugin IPC)

// ── Model data table (0x05828d58, stride 0xbc per entity_type) ────────────────
extern DWORD   DAT_05828d58;   // model data array base; slot = DAT_05828d58 + type*0xbc
extern void*   DAT_06f42a58;   // model memory pool base (allocated once in Model_LoadItems)
// BoneVertex pool (see globals.cpp). Backed by g_BoneVertexBuf.
// DAT_0584621c = base del slot 0; &DAT_0584621c es DWORD*, así que el código existente
// like (float*)(&DAT_0584621c + vertIdx * 3) does k*12-byte stride correctly
// (DWORD=4 × 3 = 12 B per vertex). (char*)&DAT_0584621c yields a byte pointer.
extern char    g_BoneVertexBuf[32 * 15000 * 12];
#define DAT_0584621c  (*(DWORD*)(g_BoneVertexBuf + 0))
// DAT_05846224 es una etiqueta de Ghidra 8 bytes adentro del mismo buffer (el inicio de pfOut
// de FUN_004404e0: pfDst = pfOut - 2 = &DAT_0584621c + local_64). Las escrituras abarcan 12 bytes desde pfDst.
#define DAT_05846224  (*(DWORD*)(g_BoneVertexBuf + 8))

// ── BMD bounding-box scratch arrays (FUN_00442e60 — BMD_ComputeBounds) ────────
extern short   DAT_077d87fc;   // vertex ref-count table base (short per bone, 256 max)
extern DWORD   DAT_05827a98;   // bbox max[0] base (float array: 3 floats × bone, stride 0xc)
extern DWORD   DAT_05827a9c;   // bbox max[1] base
extern DWORD   DAT_05827aa0;   // bbox max[2] base
extern DWORD   DAT_06f42a5c;   // bbox min[0] base
extern DWORD   DAT_06f42a60;   // bbox min[1] base
extern DWORD   DAT_06f42a64;   // bbox min[2] base

// ── UI name-list panel data (FUN_0051e240) ────────────────────────────────────
// DAT_083a430c  — macro alias dentro de DAT_083a42f8 (ver bloque de dialog button rects)
// 2026-05-08: alias por macro que proyecta en el offset +0x26 (line[1]) dentro del
// properly-sized DAT_083a44c4 buffer (g_lpszMessageBoxCustom).
#define DAT_083a44ea   (DAT_083a44c4[0x26])
extern byte    DAT_005618b8;       // item class name format ptr A
extern byte    DAT_005618bc;       // item class name format ptr B
extern byte    DAT_005618c0;       // item class name format ptr C
extern byte    DAT_005618c4;       // item class name format ptr D
extern char    s____s___005618c8[];// list entry format string (" %s " etc.)

// ── Entity / animation tick globals ───────────────────────────────────────────
extern int     DAT_05826e08;   // g_AnimTick — global frame/animation counter
// DAT_05826d31 — declared above (line 416)
extern char    DAT_05826d33;   // guild name comparison flag B
extern char    lpString_05826bfc[0x50]; // player guild name A (wide->char cache)
extern char    lpString_05826cc0[0x50]; // player guild name B
extern char    lpString_05826cc9[0x50]; // player guild name C

// DAT_07e919bc — base del array de info de guild (indexado por slot de party) — declarado arriba como DWORD
// DAT_07abf5dc — player/entity slot count — declared above as DWORD
// DAT_07abf5d8 — entity_array[0] base pointer — declared above as char*
// DAT_07eeb218/228 — login camera Y/X screen-pos — declared above as DWORD

// ── Buffers temporales de posición de huesos (usados en el traspaso de animación) ──
extern DWORD   DAT_07abf444[12]; // temp bone matrix buffer A
extern DWORD   DAT_07abf3e4[12]; // temp bone matrix buffer B
extern DWORD   DAT_07abf414[12]; // temp bone matrix buffer C

// ── Animation distance / frequency constants ───────────────────────────────────
extern float  _DAT_00552650;   // animation LOD cutoff distance
extern float  _DAT_00552954;   // sin animation frequency constant

// ── Teleport-anim pool (0x07c80110, stride 0x70, 100 slots) ──────────────────
// Sized via IDA bound: (0x7c82cd0 - 0x7c80110) / 0x70 = 0x2BC0 / 0x70 = 100.
// Antes era `extern char DAT_07c80110;` (1 byte) — Entity_TeleportAnim en stubs.cpp:3324
// recorría los 100 slots escribiendo 0x70 bytes por paso → corrupción de heap en el primer
// teleport. Now properly sized.
extern char    DAT_07c80110[100 * 0x70];   // active-flag at +0; slot stride 0x70

// ── Character/effect update pool (0x07c85890, stride 0x1BC, 1002 slots) ──────
extern char    DAT_07c85890[1002 * 0x1bc];  // pool array (was 1 byte → AUTO-SKIP)
extern int&    DAT_07c85894;   // alias = pool[+4] (Sound_Queue / Sigil_RenderAll)

// ── Math / animation constants ────────────────────────────────────────────────
extern float  _DAT_00552958;   // teleport-anim height offset
extern DWORD   DAT_00552958;
// _DAT_0055283c extern declarado arriba (1/640, X scale constant)
extern float  _DAT_00552cb8;   // login camera X-rotation speed
extern float  _DAT_00552cbc;   // login camera Y-rotation speed

// Input / player name buffers
extern char    DAT_007d29e5;
extern char    DAT_007eaa11;

// ── Sprite entity pool (FUN_00478c00 — decoration/item-drop sprite render) ────
extern DWORD   DAT_07abf634;       // sprite entity pool base (stride 0x70, limit 0x7b116b3)
extern float  _DAT_005528dc;       // UV step per sprite-sheet column (1/N cols)
extern float  _DAT_00552940;       // UV row base offset for sprite-sheet
extern float  _DAT_00552944;       // Entity_UpdateRender sin period scale (0x15d)
extern float  _DAT_00552948;       // Entity_UpdateRender sin bias (0x15d)
extern float  _DAT_0055294c;       // Entity_UpdateRender sin amplitude (0x15d)
extern float  _DAT_00552acc;       // anim speed scale constant A
extern float  _DAT_00552914;       // anim speed scale constant B

// ── Server select input (FUN_0051e7e0) ───────────────────────────────────────
extern DWORD   DAT_0056169c;        // selected channel (port % 0x14 + 1)

// ── Char menu UI builder (FUN_004c3530) ───────────────────────────────────────
extern int     DAT_07e11d20;        // UI mode: 1=class-list-A, 2=class-list-B, 3=stats
extern int     DAT_07e11d24;        // character class/subtype ID (range 0..0x1FF)
extern char    lpString_07e90798[]; // string list buffer (100 bytes per entry, ~30 slots)
extern int     DAT_07e91708[30];    // TextListColor @0x07E91708 - color por linea
extern int     DAT_07ea7b10[30];    // TextBold      @0x07EA7B10 - negrita por linea
extern char    DAT_07d329c4;        // class name table A (first entry base)
extern char    DAT_07d32af0;        // class info list A (stride 300, limit 0x7d34134)
extern char    DAT_07d34134;        // class name table B (first entry base)
extern char    DAT_07d34260;        // class info list B (stride 300, limit 0x7d358a4)
extern char    DAT_07d358a4;        // class name / info buffer C
extern int     DAT_07d78068;        // character data base (indexed by class_id * 0x40)
// ── Posición del item que se está arrastrando ───────────────────────────────
// 2026-07-20.  En IDA esto vive en `Inventory[32].Type` (el pool de shop/trade-in,
// slot 32, campo Type en offset 0) — lo usan sub_47D410 (preview de stats) y el
// render del footprint durante el drag.
//
// Nuestro build lo estaba guardando en DAT_07ea9844, que es OTRA cosa: en el
// binario esa dirección se escribe SOLO como byte (`mov byte ptr [7EA9844], bl`
// en 0x4D2586 y `..., 0` en 0x4D1D8B) y se lee únicamente en Scene_MapTick, para
// pasarla como 4º parámetro (bSell) a RenderItemInfo/RenderRepairInfo.
// Al meterle índices de slot, cualquier slot != 0 dejaba el byte bajo en no-cero
// → el tooltip mostraba el PRECIO DE VENTA sin estar en una tienda.
extern "C" BYTE Inventory[];
#define ItemPickedPos  (*(int*)&Inventory[32 * 68])   // Inventory[32].Type

// 2026-05-08: ItemAttribute base address backup (recovery copy).
// Usar ItemAttribute_Base() en vez de leer DAT_07d78068 directo cuando
// computing attrBase = type*0x40 + base — DAT_07d78068 is being clobbered
// a 0x1 por algún escritor desconocido, y provoca un AV en el tooltip / RenderBrokenItem.
extern "C" DWORD g_ItemAttribute_Backup;
static __forceinline unsigned int ItemAttribute_Base(void) {
    unsigned int p = (unsigned int)DAT_07d78068;
    if (p < 0x100000u || p >= 0x80000000u) {
        p = g_ItemAttribute_Backup;
        if (p >= 0x100000u && p < 0x80000000u) {
            DAT_07d78068 = (int)p;
        } else {
            return 0;   // truly uninitialized
        }
    }
    return p;
}
extern int     DAT_07d78080;        // font height (set by resolution in WinMain step 15)
// DAT_07e91530/534/53c/540 — columnas 2/3/5/6 de la fila 0 de DAT_07e91528
// (ver la declaracion de la tabla mas abajo).
#define DAT_07e91530   (DAT_07e91528[2])
#define DAT_07e91534   (DAT_07e91528[3])
#define DAT_07e91538   (DAT_07e91528[4])
#define DAT_07e9153c   (DAT_07e91528[5])
#define DAT_07e91540   (DAT_07e91528[6])
#define DAT_07e91544   (DAT_07e91528[7])
#define DAT_07e91548   (DAT_07e91528[8])
#define DAT_07e9154c   (DAT_07e91528[9])
// Strings de formato de la UI (datos de sólo lectura en el binario)
extern char    DAT_0055a408[];
extern char    DAT_0055a40c[];
extern char    DAT_0055a410[];
extern char    DAT_0055a414[];
extern char    DAT_0055a418[];
extern char    DAT_0055a41c[];
extern char    DAT_0055a420[];
extern char    DAT_0055a424[];
extern char    DAT_0055a428[];
extern char    DAT_0055a42c[];
extern char    DAT_0055a430[];
extern char    DAT_0055a434[];
// RenderItemInfo string constants
extern char    DAT_0055a4e0[];   // item name format string
extern char    DAT_0055a5f4[];   // item stats header format string
extern char    DAT_0055a4e4[];   // salto de linea — separador tras el precio
extern char    DAT_0055a570[];   // salto de linea — separador tras el nombre
extern char    DAT_0055a5f0[];   // salto de linea — separador de media altura
extern char    DAT_0055a5fc[];   // salto de linea — separador de media altura
extern char    DAT_0055a640[];   // salto de linea — separador de media altura
extern char    DAT_0055a608[];   // s__s__s format
extern char    DAT_0055a630[];   // secondary stats line
extern char    DAT_07d3b40c[];   // item level line format

// ── Misc low-address globals ──────────────────────────────────────────────────
// (07abf06, 07d29e5, 07eaa11 — byte flags or array references)

// ── Chat ring buffer (FUN_00480980 renderer / UIChatLogWindow_AddText) ──────
// Único buffer real: 0x77 entries × 0x118 bytes stride.
// Per-slot layout:
//   +0x000..+0x00A  sender name (11 bytes)
//   +0x00B..+0x10B  message text (257 bytes)
//   +0x10C          type/channel DWORD (0..5)
//   +0x114          cached text-extent cx (LONG)
// DAT_07df938b, DAT_07df948c, DAT_07df9494 en el binario original son ALIASES
// dentro de este buffer a los offsets 0x0B / 0x10C / 0x114 del slot 0. Ghidra
// los recuperó como globales independientes; sin este fix el writer escribía
// al buffer y el reader leía las variables sueltas (siempre 0), por eso el
// countdown azul del Exit no aparecía en pantalla.
extern char    DAT_07df9380[0x77 * 0x118];
// Alias del primer byte del campo msg (slot 0). Las fórmulas
// `&DAT_07df938b + slot*0x118` del decompile de Ghidra resuelven al byte
// correcto gracias a que éste es un lvalue dentro del buffer real.
#define DAT_07df938b (DAT_07df9380[0x0B])
// Alias del DWORD type/channel (slot 0). Mantiene el tipo int para que los
// accesos `(&DAT_07df948c)[slot*0x46]` del decompile avancen 0x118 bytes por
// slot (0x46 DWORDs = 280 bytes).
#define DAT_07df948c (*(int*)&DAT_07df9380[0x10C])
// Alias del LONG cached cx (slot 0). Los call-sites usan
// `*(LONG*)(&DAT_07df9494 + slot*0x118)` — ese cast de pointer-arith a char*
// se hace en el call-site, acá sólo exponemos el DWORD base.
#define DAT_07df9494 (*(int*)&DAT_07df9380[0x114])

// Key debounce for scroll/nav
extern DWORD   DAT_07e11970;           // PageUp debounce
extern DWORD   DAT_07e11974;           // PageDown debounce
extern DWORD   DAT_07e11984;           // Up-arrow debounce
extern DWORD   DAT_07e1198c;           // Down-arrow debounce
extern DWORD   DAT_07e119f4;           // 'B' key debounce
extern DWORD   DAT_07e11a34;           // 'R' key debounce

// Item/hotkey assignment UI
extern char    DAT_07db870c;           // item-selection toggle (char)
extern DWORD   DAT_07ea840c;           // UI cursor / hotkey X position
extern DWORD   DAT_07ea8408;           // UI cursor / hotkey Y position

// Chat input channel buffers (9 channels × 0x100 bytes)
extern char    DAT_07e0ffc8[10 * 0x100];   // macro hotkey table: 10 slots × 256 bytes
extern char    DAT_07e108c8;           // whisper-target buffer (channel 9 / separate)

// Player name (for chat name-match / tab-complete)
extern char    DAT_005592dc;           // player name string (char array)
extern DWORD   DAT_005592d8;           // name utility ptr A
extern DWORD   DAT_005592d4;           // name utility ptr B

// Text input / command buffers
extern char    DAT_07d5391c;           // current in-game text input buffer (char array)
extern char    DAT_07d3d284;           // "/whisper" command string
extern char    DAT_07d3d3b0;           // "/pvp" command string
extern char    DAT_07d3cdd4;           // GM command string
// Chat command parser name buffers (FUN_004942e0)
extern char    DAT_07d3cb7c;           // player name buffer 1 (chat cmd parser)
extern char    DAT_07d3cca8;           // player name buffer 2 (chat cmd parser)
extern char    DAT_07d3c924;           // player name buffer 3 (chat cmd parser)
extern char    DAT_07d3c6cc;           // player name buffer 4 (chat cmd parser)
extern char    DAT_07d3bfc4;           // player name buffer 5 (chat cmd parser)
extern char    DAT_07d3c0f0;           // player name buffer 6 (chat cmd parser)
extern char    DAT_07d3d608;           // whisper target display buffer
extern char    DAT_07d3d734;           // pvp-toggle result message buffer
extern DWORD   DAT_07e11dac;           // command-result flag (1=whisper, 0=pvp-off)

// B-key toggle guards
extern char    DAT_07eaa130;           // guard flag for tab/key inputs
extern char    DAT_07eaa132;           // guard for B-key toggle enable
extern char    DAT_07eaa134;           // RepairEnable_0

// Screenshot / error dialog context
extern DWORD   lpDefault_00583d88;     // default context for FUN_00480620
extern char    DAT_07d55410;           // error/info message for FUN_00480620

// Connection-check context
extern DWORD   DAT_07ea9848;           // connection context for FUN_004e3d60

// DAT_05826ceb — rolling packet counter byte (declared above near 0x05826xxx block)

// ── Timers de cuenta regresiva de chat/UI (los usan Sound_Queue.cpp + stubs.cpp) ──
extern int     DAT_00559cdc;           // system-message scroll timer (reset to 300 on Chat_AddMessage)
extern int     DAT_00559ce4;           // player-chat scroll timer   (reset to 0x96 on UI_ShowDialog)
// Strings que Sound_Countdown1 / Sound_Countdown2 vuelven a mostrar al vencer el timer:
extern char    DAT_07e11dd0[256];      // byte_7E11DD0: texto del aviso periódico (MoveNotices/CreateNotice)
extern char    DAT_07e11dd8;           // player-chat periodic refresh msg  (arg2 of FUN_00480620)
extern char    DAT_07e11ddc;           // player-chat periodic refresh label (arg1 of FUN_00480620)

// ── Chat ring buffers ─────────────────────────────────────────────────────────
// System message buffer (FUN_0047fae0 / Chat_DrawMessages)
extern char    DAT_07db80d8[6 * 0x108];   // system chat ring buffer (6 slots × 0x108 bytes)
#define        DAT_07db81dc   (DAT_07db80d8[0x104])  // flag byte alias (slot 0 +0x104)

// 2026-05-04: server-config globals (popullados por opcodes 0xDD/DE/DF).
extern "C" {
    extern DWORD g_MaxCharacterLevel;     // 0xDF — cap de level (default 400)
    extern WORD  g_CharDeleteMaxLevel;    // 0xDD — max level que permite delete
    extern BYTE  g_CharCreationEnable;    // 0xDE — permite crear chars
}
extern int     DAT_07e11d9c;           // system message count (max 5, i.e. 6 entries 0..5)
// Player chat / notification buffer (FUN_00480620 / UI_DrawPlayerList)
// Entry stride: 0x118 bytes (= 0x46 × sizeof(DWORD), Ghidra type convention)
// Layout per entry: sender[0..0xA] + msg[0xB..0x10B] + type[0x10C]
extern int     DAT_07e11da4;           // player chat entry count (max 0x77 = 119 entries)

// ── Chat text-extent work variables (Chat_DrawField / Chat_DrawEntry) ─────────
extern LPSIZE  lpsz_07e113d0;          // last GetTextExtentPointA result (cx)
extern int    _DAT_07e113d4;           // last GetTextExtentPointA result (cy)
extern DWORD   DAT_07e11d2c;           // char-index/step counter (draw-field loop)
extern char    DAT_07e11cec;           // text field char-buffer base (stride 4)

// ── Chat string constants (displayed on login/cursor events) ──────────────────
extern char    lpString_00559d3c;      // cursor/input prompt string A
extern char    lpString_00559d40;      // cursor/input prompt string B (empty/null)
extern char    DAT_00559d4c;           // player-list format: sender+space+msg fmt string
extern char    DAT_00559d54;           // s__s___s: " %s - %s" style format string
extern char    DAT_00559d5c;           // player-list alternate format string

// ── Guild system globals (opcodes 0x90-0x99) ──────────────────────────────────
// DAT_07eaa117 — declared above as char (line 613)
// DAT_07eaa116 — declared above as char (line 612)
// DAT_07eaa128 — declared above as DWORD (line 620)
extern int     DAT_07eaa12c;           // guild member ID / count
// DAT_07eaa108 — guild UI flag — declared above as DWORD (line 609)
extern BYTE    DAT_07ea97c0[64];       // guild entity pool (zeroed on stage 3, 0x40 bytes)
extern char    DAT_07e11d73;           // char-select flag D (set 1 when guild stage==3)
// Guild message string buffers (shown via ShowGuildMessage / FUN_0051d6f0):
extern BYTE    DAT_07d5b680;           // guild create result msg 1
extern BYTE    DAT_07d5b7ac;           // guild create result msg 2
extern BYTE    DAT_07d5c10c;           // guild create result msg 3
extern BYTE    DAT_07d5c238;           // guild create result msg 4
extern BYTE    DAT_07d5b8d8;           // guild create result msg 5
extern BYTE    DAT_07d6813c;           // guild join OK message
extern char    param_2_07d68268;       // guild join error format string (wsprintfA %d)
extern BYTE    DAT_07d58ea8;           // guild request OK message
extern char    param_2_07d58fd4;       // guild request error format string (wsprintfA %d)
// Posición objetivo del guild (la setean los opcodes 0x94 / 0x96):
extern DWORD   _DAT_00559f58;          // guild target tile X (4-byte, overlapping)
extern WORD    DAT_00559f5c;           // guild target tile Y (2-byte)
// Campos del sub-estado de login (también los escribe la rama 3 del guild 0x94):
// _DAT_00559c94 — declared above as float (line 202)
// DAT_00559c84  — declared above as DWORD (line 198)
// DAT_00559c88  — declared above as DWORD (line 199)
// ── Estado de la UI de guild (bloque 0x083a, usado por ShowGuildMessage / la lista de miembros) ──
// DAT_083a4324  — declared above as DWORD (line 749)
// DAT_083a44c4  — declared above as DWORD (line 755)
// ── Dialog button rects (0x083A42F8, 2 entradas × 5 ints = 0x28 bytes) ───────
// Layout por entrada: [0]=bitmapId-240 (1..4) [1]=x [2]=y [3]=width [4]=height.
// Escrito por CreateOkMessageBox/CreateDialogInterface/ShowCheckBox/sub_51D9E0/
// sub_51DA80 (todos hacen `memset(&unk_83A42F8, 0, 0x28)` -> la region es
// EXACTAMENTE 40 bytes) y leido por RenderErrorMessage (&unk_83A4304, o sea el
// campo width de la entrada 0, avanzando de a 5 ints) y por el hit-test de
// UI_InGameMenu (&unk_83A42FC = campo x).
//
// 2026-08-08 FIX (botones Yes/No del cartel de venta invisibles): en el binario
// original 42F8 / 42FC / 4304 / 430C son OFFSETS DENTRO DE ESTA MISMA REGION,
// pero aca estaban declarados como CUATRO globals independientes -> los
// writers poblaban DAT_083a42f8[]/DAT_083a430c[] y los readers leian
// &DAT_083a42fc / &DAT_083a4304, que eran otra memoria (ceros) -> el gate
// `1 <= id <= 4` nunca pasaba y no se dibujaba ningun boton.
extern DWORD   DAT_083a42f8[10];       // UI panel descriptor array (set by guild funcs)
#define DAT_083a42fc   (DAT_083a42f8[1])                 // entry0.x
#define DAT_083a4304   (*(int*)&DAT_083a42f8[3])         // entry0.width
#define DAT_083a430c   ((DWORD*)&DAT_083a42f8[5])        // entry1 base
// DAT_083a7c24  — declared above as DWORD (line 780)
// DAT_083a7c28  — declared above as DWORD (line 781)
extern int     DAT_083a7c30;           // guild member count for UI
extern int     DAT_083a7c34;           // guild UI auxiliary param
extern DWORD   DAT_083a7af8;           // guild member list array base (stride 0x18/member)

// Columnas de datos de miembro de guild (parte del mismo array de stride 0x18 que arranca en DAT_083a7af8):
extern DWORD   DAT_083a7afc;           // guild member name continuation (bytes 4-7)
extern WORD    DAT_083a7b00;           // guild member class/level (bytes 8-9)
extern DWORD   DAT_083a7b04;           // guild member kills
extern DWORD   DAT_083a7b08;           // guild member deaths
extern DWORD   DAT_083a7b0c;           // guild member score

// Guild leaderboard UI strings (FUN_0051ddf0 / FUN_0051db00):
extern char    DAT_07d59358;           // guild panel title string ("Guild War Score" etc.)
extern char    param_2_07d59484;       // format string for current player name ("%s")
extern char    DAT_07d5ba04;           // column headers array (stride 300, ~5 entries, ends at 0x7d5bfe0)
extern char    DAT_07d5bfe0;           // last column header entry
extern char *  PTR_DAT_005618a0;       // pointer to rank format string (e.g. "You: %d")
extern char    param_2_005618a4;       // rank number format string ("%d.")
extern char    param_2_005618a8;       // kills column format string ("%d")
extern char    param_2_005618b0;       // deaths column format string ("%d")
extern char    param_2_005618b4;       // score column format string ("%d")
extern char    lpString_07d68bc8[300];      // guild win title string (no guild state)
extern char    lpString_07d68cf4[300];      // guild win subtitle (no guild state)
extern char    lpString_07d68970[300];      // guild win title string (guild state)
extern char    lpString_07d68a9c[300];      // guild win subtitle (guild state)
extern char    param_2_07d68e20;       // score total format string
extern char    param_2_07d68f4c;       // opponent deaths format string ("%d")
extern char    param_2_07d69078;       // own kills format string ("%d")

// ── Pre-compile missing globals (found by diff scan) ─────────────────────────

// Game math constants missing from 0x005524xx–0x00552dxx section:
extern float  _DAT_0055264c;   // cursor offset constant (cursor draw X/Y subtract)
extern float  _DAT_005528a0;   // float constant 0.0 (guard for division / degenerate-length check)
extern float  _DAT_005529fc;   // cursor Y-effect offset (used in animated cursor draw)

// Hash table / debug string (0x00558xxx):
extern char    DAT_00558128[];  // debug log data block / format string

// Compressed-assets flag (0x0055a7c4):
extern char    DAT_0055a7c4;   // 0=plain files, 1=compressed OZJ/OZT; alias: g_tex_ext_mode
#define g_tex_ext_mode  DAT_0055a7c4

// Misc.cpp / Entity gravity constants:
extern float   _DAT_00552570;
extern float   _DAT_005527d0;
extern float   _DAT_00552a28;

// FPS timer (0x07e016f0) — DWORD millisecond timestamp, accessed via _DAT_ Ghidra alias:
extern DWORD   _DAT_07e016f0;  // last FPS tick timestamp (timeGetTime value)

// Sound emitter pool base (0x083a1218) — referenced in stubs.cpp Sound_SpawnEmitter:
extern char    DAT_083a1218[0x1158];   // Butterfles OBJECT array (10 entries × 0x1BC stride)

// Float aliases for view/projection matrix globals (0x083a4xxx):
// DWORD counterparts (DAT_083a414c etc.) are already declared above.
extern float& _DAT_083a414c;   // alias → DAT_083a4140[3]
extern float& _DAT_083a415c;   // alias → DAT_083a4140[7]
extern float& _DAT_083a416c;   // alias → DAT_083a4140[11]
extern float& _DAT_083a4284;   // camera position X (alias → DAT_083a4284_arr[0])
extern float& _DAT_083a4288;   // camera position Y (alias → DAT_083a4284_arr[1])
extern float& _DAT_083a428c;   // camera position Z (alias → DAT_083a4284_arr[2])
extern float  _DAT_083a42a4;   // projection scale X (float alias of DAT_083a42a4)
extern float  _DAT_083a42a8;   // projection scale Y (float alias of DAT_083a42a8)
extern float  _DAT_083a4294;   // projection center Y (float)

// Bone transform scratch buffer (0x06989xxx) — 3x4 row-major matrix (12 floats).
// Respaldado por DAT_06989c9c_matrix (contiguo). DAT_06989c9c es la dirección de
// matrix[0]; los dos alias _DAT apuntan a los elementos [5] y [10] de la diagonal.
// El código que pasa `&DAT_06989c9c` como float* obtiene un puntero adentro del array,
// so (&DAT_06989c9c)[N] reaches matrix[N] correctly.
extern float   DAT_06989c9c_matrix[12];
extern float&  DAT_06989c9c;   // alias → DAT_06989c9c_matrix[0]
extern float& _DAT_06989cb0;   // alias → DAT_06989c9c_matrix[5]
extern float& _DAT_06989cc4;   // alias → DAT_06989c9c_matrix[10]

// Bone normal-intensity pool (see globals.cpp). Backed by g_BoneNormalBuf.
// &DAT_077e298c es DWORD*, así que (float*)(&DAT_077e298c + meshIdx * 15000)
// avanza k*4 bytes por float (correcto para un buffer escalar por normal).
extern char    g_BoneNormalBuf[32 * 15000 * 4];
#define DAT_077e298c  (*(DWORD*)(g_BoneNormalBuf + 0))

// DAT_05846224 — declared above as #define aliasing g_BoneVertexBuf + 8.

// ── CharMenu_Build sub-function globals ──────────────────────────────────────
extern int     DAT_07eaa158;           // secondary text-entry counter (CharMenu_AppendSkillReq)
extern char    DAT_0055a400[];         // skill-req format string A (met)
extern char    DAT_0055a404[];         // skill-req format string B (not met)
// Tabla de requisitos de stats @ 0x07E91528 — 12 filas x 10 ints.  Los simbolos
// DAT_07e9152c/530/534/538/53c/540/544/548/54c son las COLUMNAS 1..9 de la fila 0
// (macros de arriba), no globals independientes.  Fila i, columna c =
// DAT_07e91528[10*i + c].
extern int     DAT_07e91528[12 * 10];
#define DAT_07e9152c   (DAT_07e91528[1])
extern char    DAT_07d359d0;           // skill description string table base (slot 0, stride ~0x138)
extern int     DAT_00559fe0;           // class-data cache guard (last built class_id)

// ── UI_StatsPanel (FUN_0051af50) globals ─────────────────────────────────────
// Float step/scale constants (0x00552xxx)
extern float  _DAT_00552854;   // row height / step in entity loop
extern float  _DAT_00552a2c;   // button width for entity panel
extern float  _DAT_00552ae4;   // UV scale V  (Texture_Draw2D)
extern float  _DAT_00552d40;   // panel X offset
extern float  _DAT_00552d44;   // UV scale U  (Texture_Draw2D)
// Dialog text color control
extern DWORD   DAT_00559c78;   // current text color ABGR (0xffffffff = white)
// DAT_00559c80 — declared above as DWORD (line 197)
// DAT_00559c8c — declared above as DWORD (line 200)
// Toggle flags
extern char    DAT_00559c5c;   // sound-effect toggle (0=off, non-zero=on)
extern char    DAT_07e11d80;   // music toggle (0=off, non-zero=on)
// Entity/level data
extern char    DAT_07d29d24;   // class name table base (stride 300)
// Format strings for numeric dialogs
extern char    DAT_07d46e60;   // login account name format
extern char    DAT_07d486fc[300]; // party/trade request status format
extern char    DAT_07d48828;   // second-password error format
extern char    DAT_07d48f30[300]; // party/trade request secondary format
extern char    DAT_07d493e0;   // duel offer sub-text format
extern char    DAT_07d4950c;   // trade/duel request line 1
extern char    DAT_07d49638;   // trade accept text
extern char    DAT_07d49764;   // duel accept text
// Vendor/NPC name tables
extern char    DAT_07d5fa78;   // vendor item name list (stride 300, 2 entries)
extern char    DAT_07d698ac;   // NPC/vendor info line A
extern char    DAT_07d699d8;   // NPC/vendor info line B
extern char    DAT_07d69b04;   // NPC entry 0 line A
extern char    DAT_07d69c30;   // NPC entry 0 line B
// DAT_083a4304  — macro alias dentro de DAT_083a42f8 (ver bloque de dialog button rects)
// DAT_083a4348 @ 0x083A4348 ES `g_lpszDialogAnswer` — las 10 respuestas del
// dialogo, 38 (0x26) bytes cada una.  Disassembly 0x401812:
// `bf 48 43 3a 08` = `mov edi, 0x083A4348` seguido de `mov ecx, 5Fh; rep stosd`
// = 0x5F dwords = 380 bytes = 10 * 38, que fija el tamano real.
extern char    DAT_083a4348[10][1][38];
#define g_lpszDialogAnswer  DAT_083a4348
// Guild message decode state
// DAT_083a7c08 — declared above as DWORD (line 772)
extern char    DAT_083a7c09;   // guild dialog flag B
// DAT_083a7c0c — declared above as DWORD (line 774)
// Zona del cursor / hit-test (también se usa en otros paneles)
// DAT_083a4124 — declared above as DWORD (line 713)
// Y del cursor de selección en la lista de servers (también se usa en el panel 0x96)
extern float  _DAT_00552cac;   // panel cursor bound Y-min
// String literals at fixed addresses (Ghidra: s__s_xxx_address)
extern char    s__s_On_0056184c[];    // "Sound: On"  / toggle-on string
extern char    s__s_Off_00561854[];   // "Sound: Off" / toggle-off string
extern char    s__s_On_0056185c[];    // "Music: On"
extern char    s__s_Off_00561864[];   // "Music: Off"
extern char    lpString_0056186c[];   // "Cancel" or empty string at 0x56186c
// Error/message string table (in-game dialogs, ~0x07d4xxxx range)
extern char    lpString_07d45ba0[300]; // dialog string 0x6e/0 (menu title)
extern char    lpString_07d45ccc[300]; // dialog string 0x6e/1
extern char    lpString_07d45df8[300]; // dialog string 0x6e/2
extern char    lpString_07d45f24[300]; // dialog string 0x6e/3
extern char    lpString_07d46050[300]; // dialog string 0x6e/4 / 0x96 header
extern char    lpString_07d463d4[300];// options panel string 0x96/3 (Cancelar)
extern char    lpString_07d4662c[300];     // dialog string 0x7a
extern char    lpString_07d46758[300];     // dialog string 0x7b
extern char    lpString_07d46884[300];     // dialog string 0x73/line0
extern char    lpString_07d469b0[300];     // dialog string 0x73/line1
extern char    lpString_07d46adc[300];     // dialog string 0x36/line0
extern char    lpString_07d46c08[300];     // dialog string 0x36/line1
extern char    lpString_07d46d34[300];     // dialog string 0x37
extern char    lpString_07d46f8c[300];     // dialog string 0x72/line1
extern char    lpString_07d470b8[300];     // dialog string 0x84/line0
extern char    lpString_07d471e4[300];     // dialog string 0x84/line1
extern char    lpString_07d47310[300];     // dialog string 0x8a/0x3a/0x90
extern char    lpString_07d4743c[300];     // dialog string 0x71
extern char    lpString_07d47568[300];     // dialog string 0x64 (decimal 100)
extern char    lpString_07d47694[300];     // dialog string 0x65
extern char    lpString_07d477c0[300];     // dialog string 0x1a/0x70/line0
extern char    lpString_07d478ec[300];     // dialog string 0x1a/0x70/line1
extern char    lpString_07d47a18[300];     // dialog string 0x15/0x47
extern char    lpString_07d47b44[300];     // dialog string 0x1b
extern char    lpString_07d47c70[300];     // dialog string 0x1c
extern char    lpString_07d47d9c[300];     // dialog string 0x1d
extern char    lpString_07d47ec8[300];     // dialog string 0x1e
extern char    lpString_07d47ff4[300];     // dialog string 0x1f
extern char    lpString_07d48120[300];     // dialog string 0x20
extern char    lpString_07d4824c[300];     // dialog string 0x16/0x3f/0x41/0x45
extern char    lpString_07d48378[300];     // dialog string 0x17
extern char    lpString_07d484a4[300];     // dialog string 0x18
extern char    lpString_07d485d0[300];     // dialog string 0x19
extern char    lpString_07d48954[300];     // dialog string 0x74 case 0
extern char    lpString_07d48a80[300];     // dialog string 0x74 case 1
extern char    lpString_07d48bac[300];     // dialog string 0x74 case 2
extern char    lpString_07d48cd8[300];     // dialog string 0x75
extern char    lpString_07d48e04[300];     // dialog string 0x76
extern char    lpString_07d4905c[300];     // dialog string 0x7f
extern char    lpString_07d49188[300];     // dialog string 0x7e/line0
extern char    lpString_07d492b4[300];     // dialog string 0x7e/line1
extern char    lpString_07d49890[300];     // dialog string 0x25
extern char    lpString_07d499bc[300];     // dialog string 0x85
extern char    lpString_07d49ae8[300];     // dialog string 0x26
extern char    lpString_07d49e6c[300];     // dialog string 0x3a case 0
extern char    lpString_07d49f98[300];     // dialog string 0x3a case 3
extern char    lpString_07d4a0c4[300];     // dialog string 0x86
extern char    lpString_07d4a1f0[300];     // dialog string 0x87
extern char    lpString_07d4a31c[300];     // dialog string 0x88
extern char    lpString_07d4a448[300];     // dialog string 0x98/line0
extern char    lpString_07d4a574[300];     // dialog string 0x98/line1
extern char    lpString_07d4a6a0[300];     // dialog string 0x89
extern char    lpString_07d558c0[300];     // dialog string 0x22
extern char    lpString_07d559ec[300];     // dialog string 0x23
extern char    lpString_07d55b18[300];     // dialog string 0x24
extern char    lpString_07d5f94c[300];     // dialog string 0x8f (guild membership)

// ── GL_State cached state globals ────────────────────────────────────────────
extern int     DAT_083a412c;   // current blend mode (0=off 2=SrcOver 3=Add 4=SrcAlpha)
extern char    DAT_083a411d;   // **AlphaTestEnable** cache (glEnable/Disable 0xBC0). El comentario viejo decia 'fog' y es FALSO: ver FUN_00511590/680.
extern char    DAT_083a4125;   // **TextureEnable** cache (glEnable/Disable 0xDE1 = GL_TEXTURE_2D). El comentario viejo decia 'depth test' y es FALSO.
extern int     DAT_083a42f4;   // screenshot active flag
extern int     DAT_083a42f0;   // screenshot counter (mod 10000)
// ── Float constants (overlapping globals) ────────────────────────────────────
extern float   _DAT_00552580;  // 0.0f guard constant
extern float   _DAT_00552850;  // particle proximity threshold (squared)
extern float   _DAT_00552878;  // particle half-way distance
extern float   _DAT_00552ce8;  // degrees-to-radians (π/180 = 0.017453f)
extern float   _DAT_00552ce0;  // half-angle factor 0.5 (used in FUN_004fa1d0 EulerToQuat; input already radians)
extern float   _DAT_00552cf0;  // 1.0f (used in quaternion→matrix)
extern float   _DAT_00552cf8;  // π/2 (used in SLERP degenerate case)
extern float   _DAT_00552d00;  // SLERP near-parallel threshold (~1e-3)
extern float   _DAT_00552a1c;  // animation speed multiplier
// ── Effect pool ──────────────────────────────────────────────────────────────
extern char    DAT_07b11670[200 * 0x1bc];   // Effect pool: 200 × 0x1bc (IDA: 0x07B11670..0x07B27150)
// ── Monster_Data string literals ─────────────────────────────────────────────
extern char    s_Data2_MonsterSetBase2_txt_00561530[];
extern char    s_Data2_Monster__0055e06c[];  // "Data2/Monster/"
extern char    s_swordbasic_smd_0055e07c[];  // "swordbasic.smd"
extern char    DAT_0055dff0;
extern char    DAT_0055df70;
extern char    DAT_0055dee0;
extern char    DAT_0055dec0;
extern char    DAT_0055de84;
extern char    DAT_0055de10;
extern FILE   *DAT_07d7806c;   // file handle (MonsterSetBase2 parser)
extern char    DAT_005580ac[]; // "rb" fopen mode string
extern char    DAT_00558090;   // Quest XOR decrypt key[0] (3-byte key used by FUN_00401120)
extern char    DAT_07cf1ef0;   // sentinel compare buffer
extern char    DAT_00559088;   // sentinel "END" string base
extern int     DAT_07d7807c;   // spawn slot index
extern char    s_Data_Monster__0055ddf8[];  // "Data/Monster/"
extern char    s_Monster_0055de08[];        // "Monster"
extern char    s_Monster__0055ddec[];       // "Monster/"
// ── Monster sound filename strings (DAT_0055d…) ──────────────────────────────
extern char    DAT_0055ddd4;  extern char    DAT_0055ddbc;
extern char    DAT_0055dda0;  extern char    DAT_0055dd84;
extern char    DAT_0055dd68;  extern char    DAT_0055dd50;
extern char    DAT_0055dd38;  extern char    DAT_0055dd1c;
extern char    DAT_0055dd00;  extern char    DAT_0055dce4;
extern char    DAT_0055dccc;  extern char    DAT_0055dcb0;
extern char    DAT_0055dc94;  extern char    DAT_0055dc7c;
extern char    DAT_0055dc60;  extern char    DAT_0055dc44;
extern char    DAT_0055dc24;  extern char    DAT_0055dc04;
extern char    DAT_0055dbe4;  extern char    DAT_0055dbcc;
extern char    DAT_0055dbb4;  extern char    DAT_0055db98;
extern char    DAT_0055db7c;  extern char    DAT_0055db60;
extern char    DAT_0055db44;  extern char    DAT_0055db28;
extern char    DAT_0055db08;  extern char    DAT_0055dae8;
extern char    DAT_0055dac8;  extern char    DAT_0055dab0;
extern char    DAT_0055da98;  extern char    DAT_0055da7c;
extern char    DAT_0055da5c;  extern char    DAT_0055da3c;
extern char    DAT_0055da24;  extern char    DAT_0055da0c;
extern char    DAT_0055d9f0;  extern char    DAT_0055d9d4;
extern char    DAT_0055d9b8;  extern char    DAT_0055d9a0;
extern char    DAT_0055d988;  extern char    DAT_0055d96c;
extern char    DAT_0055d950;  extern char    DAT_0055d934;
extern char    DAT_0055d91c;  extern char    DAT_0055d904;
extern char    DAT_0055d8e8;  extern char    DAT_0055d8cc;
extern char    DAT_0055d8b0;  extern char    DAT_0055d894;
extern char    DAT_0055d878;  extern char    DAT_0055d858;
extern char    DAT_0055d844;  extern char    DAT_0055d830;
extern char    DAT_0055d818;  extern char    DAT_0055d800;
extern char    DAT_0055d7e8;  extern char    DAT_0055d7cc;
extern char    DAT_0055d7b0;  extern char    DAT_0055d794;
extern char    DAT_0055d778;  extern char    DAT_0055d758;
extern char    DAT_0055d738;  extern char    DAT_0055d718;
extern char    DAT_0055d6ac;  extern char    DAT_0055d694;
extern char    DAT_0055d678;  extern char    DAT_0055d65c;
extern char    DAT_0055d6fc;  extern char    DAT_0055d6e0;
extern char    DAT_0055d6c4;  extern char    DAT_0055d644;
extern char    DAT_0055d62c;  extern char    DAT_0055d610;
extern char    DAT_0055d5f4;  extern char    DAT_0055d5d8;
extern char    DAT_0055d5bc;  extern char    DAT_0055d5a0;
extern char    DAT_0055d580;  extern char    DAT_0055d560;
extern char    DAT_0055d540;  extern char    DAT_0055d528;
extern char    DAT_0055d50c;  extern char    DAT_0055d4f0;
extern char    DAT_0055d4d8;  extern char    DAT_0055d4c0;
extern char    DAT_0055d4a4;  extern char    DAT_0055d488;
extern char    DAT_0055d46c;  extern char    DAT_0055d450;
extern char    DAT_0055d434;  extern char    DAT_0055d414;
extern char    DAT_0055d3f4;  extern char    DAT_0055d3d4;
extern char    DAT_0055d3bc;  extern char    DAT_0055d3a4;
extern char    DAT_0055d388;  extern char    DAT_0055d36c;
extern char    DAT_0055d350;  extern char    DAT_0055d338;
extern char    DAT_0055d320;  extern char    DAT_0055d304;
extern char    DAT_0055d2e8;  extern char    DAT_0055d2cc;
extern char    DAT_0055d2b0;  extern char    DAT_0055d298;
extern char    DAT_0055d280;  extern char    DAT_0055d264;
extern char    DAT_0055d24c;  extern char    DAT_0055d234;
extern char    DAT_0055d218;  extern char    DAT_0055d1fc;
extern char    DAT_0055d1e0;  extern char    DAT_0055d1c8;
extern char    DAT_0055d1b0;  extern char    DAT_0055d194;
extern char    DAT_0055d178;  extern char    DAT_0055d15c;
extern char    DAT_0055d144;  extern char    DAT_0055d12c;
extern char    DAT_0055d114;  extern char    DAT_0055d0f8;
extern char    DAT_0055d0dc;  extern char    DAT_0055d0c0;
extern char    DAT_0055d0a8;  extern char    DAT_0055d08c;
// ── Monster sound s_ named strings (0055cxxx range) ──────────────────────────
extern char    s_Data_Sound_iron1_wav_0055d074[];
extern char    s_Data_Sound_iron_attack1_wav_0055d058[];
extern char    s_Data_Sound_jaikan1_wav_0055d040[];
extern char    s_Data_Sound_jaikan2_wav_0055d028[];
extern char    s_Data_Sound_jaikan_attack1_wav_0055d008[];
extern char    s_Data_Sound_jaikan_attack2_wav_0055cfe8[];
extern char    s_Data_Sound_jaikan_die_wav_0055cfcc[];
extern char    s_Monster_bv01_2_jpg_0055cfb8[];
extern char    s_Monster_bv02_2_jpg_0055cfa4[];
extern char    s_Data_Sound_blood1_wav_0055cf8c[];
extern char    s_Data_Sound_blood_attack1_wav_0055cf6c[];
extern char    s_Data_Sound_blood_attack2_wav_0055cf4c[];
extern char    s_Data_Sound_blood_die_wav_0055cf30[];
extern char    s_Data_Sound_death1_wav_0055cf18[];
extern char    s_Data_Sound_death_attack1_wav_0055cef8[];
extern char    s_Data_Sound_death_die_wav_0055cedc[];
extern char    s_Data_Sound_mutant1_wav_0055cec4[];
extern char    s_Data_Sound_mutant2_wav_0055ceac[];
extern char    s_Data_Sound_mutant_attack1_wav_0055ce8c[];
extern char    s_Data_Sound_mOrcArcherAttack1_wav_0055ce68[];
extern char    s_Data_Sound_mOrcCapAttack1_wav_0055ce48[];
extern char    s_Data_Sound_mCursedKing1_wav_0055ce2c[];
extern char    s_Data_Sound_mCursedKing2_wav_0055ce10[];
extern char    s_Data_Sound_mCursedKingDie1_wav_0055cdf0[];
extern char    s_Monster_iui02_tga_0055cddc[];
extern char    s_Monster_iui03_tga_0055cdc8[];
extern char    s_Data_Sound_mMolt1_wav_0055cd50[];
extern char    s_Data_Sound_mMoltAttack1_wav_0055cd34[];
extern char    s_Data_Sound_mMoltDie_wav_0055cd1c[];
extern char    s_Data_Sound_mMegaCrust1_wav_0055cdac[];
extern char    s_Data_Sound_mMegaCrustAttack1_wav_0055cd88[];
extern char    s_Data_Sound_mMegaCrustDie_wav_0055cd68[];
extern char    s_Data_Sound_mAlquamosAttack1_wav_0055ccfc[];
extern char    s_Data_Sound_mAlquamosDie_wav_0055cce0[];
extern char    s_Data_Sound_mRainner1_wav_0055ccc4[];
extern char    s_Data_Sound_mRainnerAttack1_wav_0055cca4[];
extern char    s_Data_Sound_mRainnerDie_wav_0055cc88[];
extern char    s_Data_Sound_mPhantom1_wav_0055cc6c[];
extern char    s_Data_Sound_mPhantomAttack1_wav_0055cc4c[];
extern char    s_Data_Sound_mPhantomDie_wav_0055cc30[];
extern char    s_Data_Sound_mDrakan1_wav_0055cc18[];
extern char    s_Data_Sound_mDrakanAttack1_wav_0055cbf8[];
extern char    s_Data_Sound_mDrakanDie_wav_0055cbdc[];
extern char    s_Data_Sound_mPhoenix1_wav_0055cbc0[];
extern char    s_Data_Sound_mPhoenixAttack1_wav_0055cba0[];
extern char    s_Data_Sound_mMagicSkull_wav_0055cb84[];
extern char    s_Data_Sound_mBullDie_wav_0055cb6c[];
extern char    s_Data_Sound_mBlackSkullDie_wav_0055cb4c[];
extern char    s_Data_Sound_mBlackSkullAttack_wav_0055cb28[];
extern char    s_Data_Sound_mGhaintOrgerDie_wav_0055cb08[];
extern char    s_Data_Sound_mRedSkull_wav_0055caec[];
extern char    s_Data_Sound_mRedSkullDie_wav_0055cad0[];
extern char    s_Data_Sound_mRedSkullAttack_wav_0055cab0[];

// ── Effect_Create float constants ────────────────────────────────────────────
extern float   _DAT_005524ec;
extern float   _DAT_0055253c;
extern float   _DAT_00552828;
extern float   _DAT_00552830;  // death particle anim frame upper bound
extern float   _DAT_00552844;
extern float   _DAT_005528e8;
extern float   _DAT_0055295c;
extern float   _DAT_00552960;
extern float   _DAT_00552968;
extern float   _DAT_00552970;
extern float   _DAT_00552978;
extern float   _DAT_00552980;
extern float   _DAT_00552984;
extern float   _DAT_00552988;
extern float   _DAT_0055298c;
// ── Weather particle pool (40 slots × 0x1bc bytes = 0x4560 bytes) ────────────
// BUG-FIX 2026-05-04: antes los 30+ globals DAT_0839bc?? eran chars sueltos
// en BSS, pero Weather_Update y FUN_0043e680 los acceden con stride 0x1bc
// (slot stride) o 0x6f (int stride = 0x1bc/4). Sin un buffer contiguo, escribir
// a slot 1+ corrompe globals adyacentes; leer slot 1+ leía garbage o causaba AV
// (visible como crash en RenderNumArrow al entrar al mundo, addr=0x004BF712,
// param1=0x0839BCB0). Ahora ALL son macros que indexan dentro de un único
// buffer contiguo.
extern char    g_WeatherSlotPool[40 * 0x1bc];
#define DAT_0839bcb0  g_WeatherSlotPool[0x000]                              // slot[0] active flag (char)
#define DAT_0839bcb2  (*(short        *)&g_WeatherSlotPool[0x002])          // slot[0] type (short, stride 0xde × short)
#define DAT_0839bcb4  g_WeatherSlotPool[0x004]                              // slot[0] mode flag (char)
#define DAT_0839bcbc  (*(unsigned int *)&g_WeatherSlotPool[0x00c])          // slot[0] angular speed (int/float)
#define DAT_0839bcc0  (*(unsigned int *)&g_WeatherSlotPool[0x010])          // slot[0] pos_x (int/float)
#define DAT_0839bcc4  (*(unsigned int *)&g_WeatherSlotPool[0x014])          // slot[0] pos_y (int/float)
#define DAT_0839bcc8  (*(unsigned int *)&g_WeatherSlotPool[0x018])          // slot[0] pos_z (int/float)
#define DAT_0839bccc  (*(unsigned int *)&g_WeatherSlotPool[0x01c])          // slot[0] vel_x (int/float)
#define DAT_0839bcd0  (*(unsigned int *)&g_WeatherSlotPool[0x020])          // slot[0] vel_y (int/float)
#define DAT_0839bcd4  (*(unsigned int *)&g_WeatherSlotPool[0x024])          // slot[0] phase (int/float)
#define DAT_0839bd08  (*(unsigned int *)&g_WeatherSlotPool[0x058])          // slot[0] color R or -1 (int)
#define DAT_0839bd10  (*(unsigned int *)&g_WeatherSlotPool[0x060])          // slot[0] color G/intensity (int)
#define DAT_0839bd14  (*(unsigned int *)&g_WeatherSlotPool[0x064])          // slot[0] color B (int)
#define DAT_0839bd30  (*(unsigned int *)&g_WeatherSlotPool[0x080])          // slot[0] rotation angle (int/float)
#define DAT_0839bd40  g_WeatherSlotPool[0x090]                              // slot[0] direction vector base (3 floats)
#define DAT_0839bd70  g_WeatherSlotPool[0x0c0]                              // slot[0] cached head pos X (float byte addr)
#define DAT_0839bd74  g_WeatherSlotPool[0x0c4]                              // slot[0] cached head pos Y
#define DAT_0839bd78  g_WeatherSlotPool[0x0c8]                              // slot[0] velocity Z (float)
#define DAT_0839bd7c  (*(unsigned int *)&g_WeatherSlotPool[0x0cc])          // slot[0] scale (int/float)
#define DAT_0839bd84  g_WeatherSlotPool[0x0d4]                              // slot[0] size (float byte addr)
#define DAT_0839bd88  g_WeatherSlotPool[0x0d8]                              // slot[0] altitude limit (float byte addr)
#define DAT_0839bd8c  (*(unsigned int *)&g_WeatherSlotPool[0x0dc])          // slot[0] render flag (int)
#define DAT_0839bd98  g_WeatherSlotPool[0x0e8]                              // slot[0] color R float byte addr
#define DAT_0839bd9c  g_WeatherSlotPool[0x0ec]                              // slot[0] color G float byte addr
#define DAT_0839bda0  g_WeatherSlotPool[0x0f0]                              // slot[0] color B float byte addr
#define DAT_0839bdb4  g_WeatherSlotPool[0x104]                              // slot[0] state byte (cloud life phase)
#define DAT_0839bdb5  g_WeatherSlotPool[0x105]                              // slot[0] blend mode byte
#define DAT_0839bdb6  g_WeatherSlotPool[0x106]                              // slot[0] field at +0x106
#define DAT_0839bdb8  g_WeatherSlotPool[0x108]                              // slot[0] rotation float byte addr
#define DAT_0839bdbc  g_WeatherSlotPool[0x10c]                              // slot[0] field at +0x10c
#define DAT_0839be11  g_WeatherSlotPool[0x161]                              // slot[0] byte field
#define DAT_0839be14  (*(unsigned int *)&g_WeatherSlotPool[0x164])          // slot[0] alpha fade (int/float)
// ── Weather float constants ───────────────────────────────────────────────────
extern float   _DAT_0055285c;  // weather float constant
extern float   _DAT_0055286c;  // weather float constant
extern float   _DAT_005528e4;  // weather float constant
extern float   _DAT_00552900;  // weather z-offset constant
extern float   _DAT_00552ab4;  // weather float constant
extern float   _DAT_00552ab8;  // weather float constant
extern float   _DAT_00552d24;  // weather float constant
extern float   _DAT_00552d28;  // weather float constant

// ── Camera / viewport globals ─────────────────────────────────────────────────
extern DWORD   DAT_0055a7b0;   // viewport param 2 (y offset)
extern DWORD   DAT_0055a7b4;   // viewport param 1 (x offset)
extern DWORD   DAT_0055a7b8;   // viewport param 3 (width)
extern float   _DAT_0055a7bc;  // viewport height (float)

// ── GL_State cache ────────────────────────────────────────────────────────────
extern char    DAT_083a411c;   // GL state cache byte
extern char    DAT_083a411e;   // GL state cache byte
extern float   DAT_083a4120;   // Map_InitRayCast running closest-hit ray-t (was DWORD; widened to match IDA flt_83A4120 — audit #7)
extern DWORD   DAT_083a42e8;   // GL state cache dword
#define DAT_0055a7bc _DAT_0055a7bc

// ── Fog / projection globals ──────────────────────────────────────────────────
extern float   DAT_00561558;   // fog start distance
extern DWORD   DAT_0056155c;   // fog color pointer / float start

// ── Perspective constant ──────────────────────────────────────────────────────
extern float   _DAT_00552d34;  // perspective scale constant

// ── Joint pool ────────────────────────────────────────────────────────────────
extern char    DAT_07b27150[500 * 0x9d8];   // Joint pool: 500 × 0x9d8 (IDA: 0x07B27150..0x07C5AB30)

// ── Joint_Create float constants ─────────────────────────────────────────────
extern float   _DAT_00552a14;  // joint float constant
extern float   _DAT_00552658;  // joint float constant
extern float   _DAT_005526d8;  // joint float constant
extern float   _DAT_00552a44;  // joint float constant
extern float   _DAT_00552a34;  // joint float constant
extern float   _DAT_005529ac;  // joint float constant

// ── Timer globals ─────────────────────────────────────────────────────────────
extern DWORD   DAT_05826e14;   // FPS counter B
extern DWORD   DAT_05826df0;   // last frame timestamp
extern float   _DAT_005528a8;  // 1/1000 ms-to-s scale
extern float   _DAT_00552898;  // 5.0 second FPS window
extern float   _DAT_00552890;  // delta-time scale
extern DWORD   DAT_05826e00;   // FPS accumulator ring buffer
extern float   _DAT_0055979c;  // delta time per frame
extern DWORD   DAT_05826dfc;   // previous frame timestamp
extern float   _DAT_05826df8;  // smoothed FPS value

// ── Music.cpp globals ─────────────────────────────────────────────────────────
extern DWORD   m_MusicOnOff;            // 0x055C9E3C — flag on/off de la musica (ver globals.cpp)
extern char    DAT_055c9d04[256];       // 0x055C9D04 — Mp3FileName: track en reproduccion
extern char    s_MuPlayer_00559110[];   // "MuPlayer" string
extern char    s_MuPlayer_exe_00559154[]; // "MuPlayer.exe" string
extern char    s_MuPlayer_exe__s_00559130[]; // "MuPlayer.exe %s" format
extern char    s_StopMp3_cmd_0055911c[];    // stop command string
extern char    s_PlayMp3_cmd_00559140[];    // play command string

// ── Sound_DS3D globals ────────────────────────────────────────────────────────
extern DWORD   DAT_0058443c;   // DS3D state dword

// ── Net_Connect globals ───────────────────────────────────────────────────────
extern DWORD   DAT_055ca15c;   // net connect state dword
extern char    s_Failed_to_connect__00559688[]; // error string

// ── Net_PacketSession globals ─────────────────────────────────────────────────
extern BYTE    DAT_07ea8448[0x1100];   // 64-slot × 0x44 inventory ref grid (sized properly)
// Char-select entry pools — IDA layout:
//   DAT_07ea5b68 .. 0x07ea7b48 (= 8160 bytes = 116 slots × 0x44 stride)
//   DAT_07ea9880 .. 0x07eaa100 (= 2176 bytes =  32 slots × 0x44 stride)
// Cada slot es el registro del panel de char-select. En nuestro build estaban
// declarados como DWORDs sueltos, lo que hacía que FUN_004cba60 (CharPreview_Reset)
// recorriera mucho más allá del final, en memoria random → AV.
extern BYTE    DAT_07ea5b68[0x1FE0];   // 8160 bytes
extern BYTE    DAT_07ea9880[0x0880];   // 2176 bytes
extern DWORD   DAT_07eaa0e8;
extern BYTE    DAT_07ea7b88[0x880];     // see DAT_07ea5298 above
extern DWORD   DAT_07e11f34;   // MarkColor[16] — guild mark color palette (uint ARGB)
extern BYTE    DAT_07e11f78[0x880];
extern BYTE    DAT_07ea52d0[0x880];
extern BYTE    DAT_07ea7bc0[0x880];
extern BYTE    DAT_07e11fb0[0x880];
extern DWORD   DAT_055c9b7c;
extern DWORD   DAT_07eaa164;

// ── Entity_Init globals ───────────────────────────────────────────────────────
extern char    DAT_00559b74[];  // "rb" fopen mode
extern DWORD   DAT_00559b70;
extern char    DAT_00559b50[];
extern char    s__4d__4d_30__4d__4d__1_00559b58[]; // format string

// ── Scene_CharPreview globals ─────────────────────────────────────────────────
extern DWORD   DAT_07e91354;
extern char    DAT_07e9136a;   // picked item durability/option byte
extern char    DAT_07e9136b;
extern DWORD   DAT_07ea5244;
extern DWORD   DAT_07ea5240;
extern char    DAT_07ea525b;

// ── Effect_Tick globals ───────────────────────────────────────────────────────
extern float   _DAT_00552aac;  // effect tick float constant
extern char    DAT_07c74ec8[40 * 0x1bc];   // Fade-effect pool: 40 × 0x1bc
extern char    DAT_07c82cdc[63 * 0x70];   // Flare effect pool: 63 × 0x70 bytes
extern char    DAT_07c80128[100 * 0x70];   // Spark-effect pool: 100 × 0x70

// ── Terrain_Light globals ─────────────────────────────────────────────────────
extern DWORD   DAT_0839bc84;
extern float   _DAT_00552a08;  // terrain light float constant
// 2026-05-04: cb60c / 0828b60c / cb610 / 0828b610 NO son globals separados —
// son el 2do/3er DWORD del slot 0 de cb608 / 0828b608. Las macros están
// definidas junto a las declaraciones de cb608 / 0828b608, más arriba en este header.

// ── Scene_Resources string literals ──────────────────────────────────────────
// Nombres base de BMD/SMD que consumen FUN_005060b0 / FUN_00505e90 en Scene_Resources.
extern char    DAT_0055e834[8];     // "Ship"
extern char    DAT_005606ac[8];     // "Logo"
extern char    DAT_005607c0[8];     // "Face"
extern char    DAT_005606e8[32];    // SMD basename (stub)
extern char    DAT_005606d0[32];    // SMD basename (stub)
extern char    DAT_005607c8[32];    // SMD basename (stub)
extern char    DAT_0056085c[32];    // SMD basename (stub)
extern DWORD   DAT_00560694;    // Map_Load block-read descriptor (format/context for FUN_00543274)
extern char    s_Logo_Webzenlogo_jpg_005606b0[];
extern char    s_Logo_Title_jpg_005606bc[];
extern char    s_Interface_GFx_Interface_jpg_005607c4[];
extern char    s_Interface_GFx_Interface2_jpg[];
extern char    s_Data_Interface_jpg[];
extern char    s_Data_Interface2_jpg[];
extern char    s_Object1_Object1_jpg[];
extern char    s_warrior_bmd[];
extern char    s_main_bmd[];
extern char    s_fairy_bmd[];

// ── Scene_Resources strings ───────────────────────────────────────────────────
extern char    s_Logo_0Account_new_tga_005607a8[];
extern char    s_Logo_0On_Botton_jpg_00560794[];
extern char    s_Logo_0On_Botton2_jpg_0056077c[];
extern char    s_Logo_0Text_Box_jpg_00560768[];
extern char    s_Logo_0New_Account01_tga_00560750[];
extern char    s_Logo_0New_Account02_tga_00560738[];
extern char    s_Logo_0Box_jpg_00560728[];
extern char    s_Interface_Progress_Back_jpg_0056070c[];
extern char    s_Interface_Progress_jpg_005606f4[];
extern char    s_Data2_Object1__0055f120[];
extern char    s_ship_smd_0055ee54[];
extern char    s_Data2_Logo__005606dc[];
extern char    s_mu_smd_005606c8[];
extern char    s_sun_smd_005606b4[];
extern char    s_Data_Object1__0055f360[];
extern char    s_Data_Logo__005606a0[];
extern char    s_Object1__0055f354[];
extern char    s_Logo__00560698[];
extern char    s_Logo_Interface01_tga_00560a4c[];
extern char    s_Logo_Interface02_tga_00560a34[];
extern char    s_Logo_Interface03_tga_00560a1c[];
extern char    s_Logo_Interface04_tga_00560a04[];
extern char    s_Logo_New_Character201_tga_005609e8[];
extern char    s_Logo_New_Character202_jpg_005609cc[];
extern char    s_Logo_Delete01_tga_005609b8[];
extern char    s_Logo_Delete02_jpg_005609a4[];
extern char    s_Logo_Ok01_tga_00560994[];
extern char    s_Logo_Ok02_jpg_00560984[];
extern char    s_Logo_New_Character01_tga_00560968[];
extern char    s_Logo_New_Character02_tga_0056094c[];
extern char    s_Logo_New_Character_Cancel_jpg_0056092c[];
extern char    s_Logo_New_Character_Ok_jpg_00560910[];
extern char    s_Logo_New_Character001_jpg_005608f4[];
extern char    s_Logo_New_Character002_jpg_005608d8[];
extern char    s_Logo_New_Character003_jpg_005608bc[];
extern char    s_Logo_New_Character004_jpg_005608a0[];
extern char    s_main_smd_00560894[];
extern char    s_warrior_smd_00560828[];
extern char    s_fairy_smd_0055c438[];

// ── Entity_Spawn ──────────────────────────────────────────────────────────────
// DAT_0838bc70 redeclarado como array de char (no DWORD)
// (definido en globals.cpp como char DAT_0838bc70[])

// ── Additional misc globals ───────────────────────────────────────────────────
// DAT_07e016f0 — misma dirección que _DAT_07e016f0 (ya declarado arriba)
#define DAT_07e016f0  _DAT_07e016f0
// DAT_07e01720 -> macro sobre el pool (ver abajo)   // Tooltip/bubble pool: 26 × 0x254 bytes
extern BYTE    DAT_0055984c[9];  // bone index table for glitter effect (Entity_UpdateRender)
extern DWORD   DAT_07e118ec[256];  // key-state edge-trigger table
extern DWORD   DAT_07e11aac;
extern DWORD   DAT_07e11ab0;
extern DWORD   DAT_07e11ab4;
extern DWORD   DAT_07e11ab8;
extern DWORD   DAT_07e11da8;
extern DWORD   DAT_07e12858;
extern DWORD   DAT_07ea5284;
extern DWORD   DAT_07ea5288;
extern DWORD   DAT_07ea9844;
extern char    DAT_083a2f78[10 * 0x1bc];   // Ambient particle pool: 10 slots × 0x1bc bytes
extern float   _DAT_00590af0;
extern float   DAT_00590af4;   // 3D sound sin component (updated by Sound_UpdateChannel3D_Tick)
extern float   DAT_00590af8;   // 3D sound cos component (updated by Sound_UpdateChannel3D_Tick)
extern float   DAT_00590afc;   // cloth wind Z
extern float   _DAT_00559068;  // gravedad 9.8
extern float   _DAT_0055906c;  // dt fijo 0.0025
extern LPBYTE  lpData_055ca044;
extern char   *lpText_07d63aec;
extern DWORD   DAT_00561740;
extern DWORD   DAT_00561744;
extern DWORD   DAT_00561748;
extern DWORD   DAT_0056174c;
extern char    s_WEBZEN_0056176c[];
extern char    s_WebZen_0056175c[];
extern char    s_Webzen_00561764[];
extern DWORD   DAT_00561750;
extern char    s_Webzen_00561754[];

// ── Chat.cpp missing globals ──────────────────────────────────────────────────
extern char    DAT_07db8714;         // input field buffer byte +2
extern char    DAT_07db8716;         // input field buffer byte +4 (char)
extern DWORD   DAT_07db8718;         // input field buffer dword +8 (char-select PIN data base)
extern char    DAT_07d73104[20000];  // word-filter table (1000 × 20 bytes, Filter.bmd)

// ── GlobalText[] localized string pool ────────────────────────────────────────
// 1000 slots × 300 bytes, los carga al arranque OpenTextData() desde
// Data/Local/Text.bmd (XOR con la clave de 3 bytes FC CF AB vía BuxConvert_0).
// Entries referenced across the codebase as strings like "Saldrás del juego
// en %d segundos." (index 380), error messages, UI labels, etc.
// GlobalText — el Text.bmd del 0.97k tiene EXACTAMENTE 1000 filas de 300 bytes
// (300000 bytes, verificado sobre el archivo).  Pero nuestro port heredo ~217
// indices DISTINTOS >= 1000 (hasta el 3305) de versiones posteriores, que con
// el array declarado [1000][300] eran LECTURAS FUERA DE RANGO: hasta ~690 KB
// pasado el final, entregadas a snprintf como si fueran cadenas.
// Se agranda el array a 4000 filas y se dejan las de arriba en cero: esas
// entradas devuelven cadena vacia y las rutinas de tooltip las saltean solas
// (todas chequean text[0]).  El loader sigue leyendo solo las 1000 reales.
#define GLOBALTEXT_ROWS_FILE   1000    // filas que trae el Text.bmd
#define GLOBALTEXT_ROWS        4000    // filas reservadas (el resto queda vacio)
extern char    GlobalText[GLOBALTEXT_ROWS][300];
void __cdecl OpenTextData(void);
int  __cdecl LoadTextData_Bin(const char *FileName);
// BUG-FIX 2026-07-17: DAT_07d4b4b0/5dc son GlobalText[457]/[458] (name-filter blocked
// words, cargados de Text.bmd). Estaban como chars sueltos =0 (string vacío) → FindText
// devolvía 1 → nombres rechazados. `&DAT_07d4b4b0` ahora = GlobalText[457].
#define DAT_07d4b4b0 (GlobalText[457][0])
#define DAT_07d4b5dc (GlobalText[458][0])
extern DWORD   DAT_07db8070;         // chat buffer extended
extern int     DAT_07d78074;         // command table A count (Chat_DrawEntry)
extern int     DAT_07d78070;         // command table B count (Chat_DrawEntry)
extern char    DAT_07d27610[20000];  // name-filter table (1000 × 20 bytes, FilterName.bmd)
extern float   _DAT_00552950;        // speed constant (Ambient_Particles lightning speed)

// ── Terrain map globals ───────────────────────────────────────────────────────
// DAT_083a0218 ahora se declara como alias por macro dentro de g_ObjectBucketGrid (ver arriba).
extern float   _DAT_00552b70;  // terrain height scale constant

// ── Terrain tile pick / normal computation globals ────────────────────────────
// (DAT_07feb25c..DAT_07feb284 são macros — ver g_TilePickBuf arriba)
extern DWORD   DAT_07eab1fc;   // tile pick result flag
extern DWORD   DAT_07e11d44;   // combat target select sub-mode

// ── Model slot index globals ──────────────────────────────────────────────────
extern DWORD   DAT_083a4104;   // model slot active flag
extern DWORD   DAT_083a4108;   // model slot current index

// ── Terrain culling globals ───────────────────────────────────────────────────
extern short   _DAT_0838b60a;  // terrain horizon/clip value
extern DWORD   _DAT_0838b614;  // terrain view scale (float bits)
extern DWORD   _DAT_0838b710;  // terrain LOD scale (float bits)
// _DAT_005528a8 ya declarado arriba (1/1000, de ms a s, misma dirección)

// ── Model loader format strings ───────────────────────────────────────────────
extern char    s__s_bmd_0055a7f8[];    // "%s.bmd"
extern char    s__s0_d_bmd_0055a7ec[]; // "%s0%d.bmd"
extern char    s__s_d_bmd_0055a7e0[];  // "%s%d.bmd"

// ── Item/Skill/Gate data loader globals ──────────────────────────────────────
extern int     DAT_07cf1ff0;       // item record shadow array base (mirrors DAT_07d78068)
extern int     DAT_07cf1ff8;       // skill record shadow array base
extern int     DAT_07d29d20;       // skill/gate data array base
extern DWORD   DAT_07cf5600;       // gate data array base (stride 9 bytes, 100 entries) — malloc'd in WinMain
// ── g_DialogScript @ 0x07CF5608 — 200 entradas x 0x400 bytes ────────────────
// Layout verificado contra el disassembly de CSQuest::ShowDialogText (0x4017E0):
//   0x4017F1  shl eax, 0Ah            → stride de entrada = 0x400
//   0x4017F6  add eax, g_DialogScript → m_lpszText en +0x000
//   0x401834  cmp [ecx+0x07CF5734]    → m_iNumAnswer      en +0x12C
//   0x40184D  add eax, 0x07CF5788     → m_lpszAnswer      en +0x180, paso 0x40
// y contra los xrefs de sub_401AF0 / UI_InGameMenu para los otros dos campos:
//   0x07CF5738 = m_iLinkForAnswer   (+0x130)  — índice del diálogo siguiente
//   0x07CF5760 = m_iReturnForAnswer (+0x158)  — acción de la respuesta (1/2/3)
// Cierra exacto: 0x180 + 10*0x40 = 0x400, y 0x130 + 10*4 = 0x158 + 10*4 = 0x180.
//
// 2026-08-21: los cuatro campos estaban como globals ESCALARES sueltos y los
// consumidores hacían `&DAT_07cf5734 + idx * 0x400` sobre punteros tipados
// (paso 4x) — lecturas fuera de rango garantizadas.  Ahora hay una sola tabla.
#define DIALOG_SCRIPT_COUNT   200
struct DIALOG_SCRIPT {
    /*+0x000*/ char m_lpszText[300];
    /*+0x12C*/ int  m_iNumAnswer;
    /*+0x130*/ int  m_iLinkForAnswer[10];
    /*+0x158*/ int  m_iReturnForAnswer[10];
    /*+0x180*/ char m_lpszAnswer[10][0x40];
};
extern DWORD   DAT_07cf5608[];     // base real de la tabla (DIALOG_SCRIPT_COUNT * 0x400)
#define g_DialogScript  ((DIALOG_SCRIPT*)DAT_07cf5608)
static_assert(sizeof(DIALOG_SCRIPT)                        == 0x400, "DIALOG_SCRIPT: stride de entrada");
static_assert(offsetof(DIALOG_SCRIPT, m_iNumAnswer)        == 0x12C, "DIALOG_SCRIPT: m_iNumAnswer");
static_assert(offsetof(DIALOG_SCRIPT, m_iLinkForAnswer)    == 0x130, "DIALOG_SCRIPT: m_iLinkForAnswer");
static_assert(offsetof(DIALOG_SCRIPT, m_iReturnForAnswer)  == 0x158, "DIALOG_SCRIPT: m_iReturnForAnswer");
static_assert(offsetof(DIALOG_SCRIPT, m_lpszAnswer)        == 0x180, "DIALOG_SCRIPT: m_lpszAnswer");
// Los cuatro DAT_ viejos son los campos de la ENTRADA 0, no globals aparte.
#define DAT_07cf5734   (g_DialogScript[0].m_iNumAnswer)
#define DAT_07cf5738   (g_DialogScript[0].m_iLinkForAnswer[0])
#define DAT_07cf5760   (g_DialogScript[0].m_iReturnForAnswer[0])
#define DAT_07cf5788   (g_DialogScript[0].m_lpszAnswer[0][0])
extern char    DAT_07d566d0;       // fallback char name string (no-char placeholder)
extern char    s__d___s_005580b0[];// "%d %s" format string for char-select display
extern int     DAT_07d78078;       // NPC name count
// DAT_07cf2000 / DAT_07cf2001: alias a MonsterScript (misma tabla 0x07CF2000).
// Ver nota en globals.cpp.  &DAT_07cf2000 = base de la tabla; DAT_07cf2001 =
// primer byte del campo Name de la entrada 0.
#define DAT_07cf2000   (MonsterScript[0])
#define DAT_07cf2001   (MonsterScript[1])
extern void   *ppvBits_055c9e4c;   // DIB section bitmap pointer (Font_BuildLayout)
extern DWORD   DAT_01c5e200;       // item/skill BMD checksum seed A
extern DWORD   DAT_00b43000;       // item/skill BMD checksum seed B
extern char    DAT_00559bb4[3];    // XOR cipher key for FUN_00479910 (0xFC,0xCF,0xAB)
// Error message format strings
extern char    s__s___File_not_exist__00558094[];  // "%s - File not exist."
extern char    s__s___File_corrupted__00559bd4[];  // "%s - File corrupted."
// File open modes
extern char    DAT_005597d4[];  // "wb" (binary write mode for FUN_005430f0 callers)

// ── SecondPassword Screen2 entity list globals ────────────────────────────────
extern DWORD   DAT_07e11e80;   // char-select entity name table base (stride 0x24 per slot)
extern DWORD   DAT_07ea5b24;   // Screen2 panel origin X
extern DWORD   DAT_07ea5b28;   // Screen2 panel origin Y

// ── SecondPassword UI sub-handler globals (FUN_004e8b70 / 004e9050 / 004eb5d0 / 004e6550) ──
extern DWORD   DAT_07eaa0c8;   // SecondPassword dialog origin X (pixel)
extern DWORD   DAT_07eaa0cc;   // SecondPassword dialog origin Y (pixel)
extern DWORD   DAT_07eaa140;   // SecondPassword timeout/retry counter (0 = no timeout)
extern DWORD   DAT_07eaa131;   // SecondPassword checkbox/toggle state
extern DWORD   DAT_07eaa138;   // RepairEnable (low byte cleared on teleport)
extern DWORD   DAT_07ea5290;   // SecondPassword alt-panel origin X
extern DWORD   DAT_07ea528c;   // SecondPassword alt-panel origin Y
// Entradas de la tabla de strings que usa el switch de FUN_004e9050:
extern char    DAT_07d544d4;   // error string for case 0 (second password wrong)
extern char    DAT_07eaa1a0;   // UI message label A (FUN_00480620 arg1)
extern char    DAT_07d54600;   // error string for auth-fail case
extern char    DAT_07eaa198;   // UI message label B
extern char    DAT_07d55c44;   // error string for case 0xfffffff8/0xfffffffe
extern char    DAT_07eaa19c;   // UI message label C
// DAT_0055a3f8 / DAT_0055a3fc — iVar1/iVar3 defaults for FUN_004e3db0 non-1/7/0xb/8 cases
extern int     DAT_0055a3f8;   // auth mode param A
extern int     DAT_0055a3fc;   // auth mode param B

// ── SecondPassword Screen5/6/7 additional globals ────────────────────────────
extern DWORD   DAT_07eaa120;   // SecondPassword_Screen5 mode (0=normal, 1=equip-select)
extern char    DAT_07eaa0dc;   // SecondPassword selected grid index (byte)
extern char    DAT_07ea51ec;   // PIN entry char prefix buffer (6 bytes, null-term)
extern DWORD   DAT_07ea51f0;   // PIN entry packed data
extern char    DAT_07ea51f5;   // PIN grid selection buffer base (multi-byte, stride 1)
extern float  _DAT_00552c20;   // Screen5 button X upper bound
extern float  _DAT_00552c1c;   // Screen5 button height
extern float  _DAT_00552c28;   // Screen5 button Y base
extern char    DAT_07d6b724;   // Error message: "no item in slot"
extern char    DAT_07d685ec;   // Error message: "invalid slot" (FUN_004e6c40)
extern short   DAT_00559f5a;   // second-password level check B (short)
extern int     DAT_00559f80;   // level threshold array base (index by slot)
extern int     DAT_00559f84;   // level threshold array upper (index by slot)
extern int     DAT_00559f60;   // level range lower array (index by slot)
extern int     DAT_00559f64;   // level range upper array (index by slot)
// DAT_07ea7b88 — declared above as DWORD (line 1474)
extern char    DAT_07ea5b30;   // second-password char-slot list base

// BMD_DrawMesh / BMD_DrawBoneSlot_Anim (FUN_00440d50 / FUN_004414d0) buffers
// LightTransform pool (see globals.cpp). Backed by g_BoneLightBuf.
// DAT_060db65c es un lvalue char en el offset 0 del buffer; &DAT_060db65c + k avanza
// k bytes (aritmética de bytes) — lo exigen los accesos de BMD_DrawMesh.cpp.
extern char    g_BoneLightBuf[32 * 15000 * 12];
#define DAT_060db65c  (*(char*)(g_BoneLightBuf + 0))
// Chrome UV scratch — backed by g_ChromeUVBuf. &DAT_05828d5c = &g_ChromeUVBuf[0]
// (float*). (&DAT_05828d5c)[i] = g_ChromeUVBuf[i]; +1 float = V slot (old d60).
extern float   g_ChromeUVBuf[15000 * 2];
#define DAT_05828d5c  (g_ChromeUVBuf[0])
// Transformed-normal buffer — backed by g_BoneChromeNormalBuf. DAT_06f433bc = base
// (destino de escritura de VectorRotate), DAT_06f433c0 = base+4 (lectura del pre-loop de chrome; pfVar8[-1]
// = normal.x en base, [0] = normal.y en base+4, [1] = normal.z en base+8).
extern char    g_BoneChromeNormalBuf[32 * 15000 * 12];
#define DAT_06f433bc  (*(char*)(g_BoneChromeNormalBuf + 0))
#define DAT_06f433c0  (*(float*)(g_BoneChromeNormalBuf + 4))

// _DAT_ float constants missing from earlier declarations
extern float   _DAT_00552544;  // alpha threshold (alpha < this → use alpha channel path)
extern float   _DAT_005528c0;  // chrome U scale factor
extern float   _DAT_005528c4;  // sin period scale for vertex deformation (FUN_004414d0)
extern float   _DAT_00552644;  // sin amplitude for vertex deformation (FUN_004414d0)

// ── MoveEffect (FUN_00466ad0) constants (0x005524a8–0x00552a20 range) ─────────
extern float   _DAT_005524a8;  // move effect float constant
extern float   _DAT_00552864;  // move effect float constant
extern float   _DAT_00552990;  // move effect float constant
extern float   _DAT_00552994;  // move effect float constant
extern float   _DAT_00552998;  // move effect float constant
extern float   _DAT_0055299c;  // move effect float constant
extern float   _DAT_005529a0;  // move effect float constant
extern float   _DAT_005529a4;  // move effect float constant
extern float   _DAT_005529a8;  // move effect float constant
extern float   _DAT_005529b0;  // move effect float constant
extern float   _DAT_005529b4;  // move effect float constant
extern float   _DAT_005529b8;  // move effect float constant
extern float   _DAT_005529c4;  // move effect float constant
extern float   _DAT_005529cc;  // move effect float constant
extern float   _DAT_005529d0;  // move effect float constant
extern float   _DAT_005529d4;  // move effect float constant
extern float   _DAT_005529d8;  // move effect float constant
extern float   _DAT_005529e8;  // move effect float constant
extern float   _DAT_005529f0;  // move effect float constant
extern float   _DAT_005529f8;  // move effect float constant
extern float   _DAT_00552a04;  // move effect float constant
extern float   _DAT_00552a0c;  // move effect float constant
extern float   _DAT_00552a18;  // move effect float constant
extern float   _DAT_00552a20;  // move effect float constant
extern float   _DAT_00552938;  // MoveJoint trig scale (mode 1)
extern float   _DAT_00552a54;  // MoveJoint particle angle scale
extern float   _DAT_00552a8c;  // MoveJoint color fade rate A
extern float   _DAT_00552a90;  // MoveJoint color fade rate B
extern float   _DAT_00552a94;  // MoveJoint trig freq A
extern float   _DAT_00552a98;  // MoveJoint trig scale B
extern float   _DAT_00552aa4;  // MoveJoint HP-bar scale factor
extern float   _DAT_00552a9c;  // FUN_00473d90 ring trig scale X
extern float   _DAT_00552aa0;  // FUN_00473d90 ring trig scale Y
extern float   _DAT_00552aa8;  // FUN_00473d90 ring trig scale Z
extern char    DAT_00559b78[]; // MoveEffect byte lookup table A (7 entries)
extern char    DAT_00559b7f[]; // MoveEffect byte lookup table B (7 entries)

// ── Map_LoadObjectModels (0x0050c4d0) ─────────────────────────────────────────
extern FILE*   DAT_083a40fc;          // file handle for custom-map object list
extern char    DAT_083a3ff4[256];     // token read buffer (Parse_NextToken output)
extern int     _DAT_083a40f4;         // Parse_NextToken: last token type code
extern float   _DAT_083a40f8;         // Parse_NextToken: last numeric value
extern char    DAT_083a4100;          // flag: Lorencia object models already loaded
extern char    DAT_083a8ad8[32 * 0x38];  // Icarus water-tile name table (32 × 0x38)
// DAT_0055e714 — format string for custom map object file path (zone index)
// DAT_005580ac — declared above (file open mode char)
// DAT_083a410c, DAT_0055a7c4, DAT_0055a7ac, DAT_05828d58 — declared above

// ── BMD_SkinUpdate (0x0040b630) ──────────────────────────────────────────────
extern int     DAT_00590c10;          // skinned vertex count
extern float   DAT_055c4068;          // bone output world-vertex array base   (stride 0x1b floats)
extern float   DAT_00593008;          // source vertex position array          (stride 3 floats)
extern short   DAT_00590c32;          // parent-bone index array               (stride 0x11 shorts)
extern float   _DAT_0055259c;         // global mesh scale factor
extern float   DAT_055c4038;          // bone world-position matrix array base (stride 0x6c bytes)
extern float   DAT_055c4098;          // bone world-pos X component            (stride 0x1b)
extern float   DAT_055c409c;          // bone world-pos Y component
extern float   DAT_055c40a0;          // bone world-pos Z component
extern int     DAT_00593968;          // triangle count
extern BYTE    DAT_0059396a;          // material name array base (stride 0x20, parallel to triangle data)
extern float   DAT_00608c70;          // triangle vertex data (9 floats/vert: bone,x,y,z,nx,ny,nz,u,v)
extern short   DAT_0080a428;          // UV index count per mesh group
extern short   DAT_0080a42a;          // normal index count per mesh group
extern short   DAT_0080a42c;          // vertex index count per mesh group
extern short   DAT_008097a0;          // material group count (build loop accumulator)
extern BYTE    DAT_008097a2;          // material name table base (stride 0x20 bytes per entry)
extern float   DAT_0080a424;          // packed vertex buffer (stride 0x31be9 floats per mesh group)
extern short   DAT_00879588;          // vertex index table   (stride 0x637d2 per mesh group)
extern short   DAT_00896a48;          // UV index table       (same stride)
extern short   DAT_00831530;          // UV name/linked-vert table
extern float   DAT_00831534;          // UV coordinate data   (stride 0x31be9)
extern float   DAT_0083153c;          // UV extra/Z channel
extern short   DAT_00831540;          // UV linked vertex index table
extern float   DAT_00862270;          // normal data          (stride 0x31be9)
extern float   DAT_00862274;          // normal Y channel
extern short   DAT_008b3f08;          // normal index table   (stride 0x637d2)

// ── RenderObjectScreen (0x004e13a0) ──────────────────────────────────────────
// Estos globals guardan la pose de render por frame del item/objeto que se está mostrando.
extern float   _DAT_07ea952c;         // item render: rotation X offset
extern float   _DAT_07ea9530;         // item render: rotation Y
extern float   _DAT_07ea9534;         // item render: rotation Z
extern short   _DAT_07ea9512;         // item render: resolved model type index
extern float   _DAT_07ea9538;         // item render: HeadAngle[0] (used by BMD_Animation)
extern int     _DAT_07ea9618;         // item render: state flag A
extern int     _DAT_07ea961c;         // item render: state flag B
extern short   DAT_07ea9616;          // item render: state flag C

// ── Filter / name-filter system globals ──────────────────────────────────────
extern DWORD   DAT_007cfa00;    // word-filter BMD checksum seed
extern DWORD   DAT_00578200;    // name-filter BMD checksum seed
extern char    lpText_07d2aa08[];  // fatal-error message string (shown by ExitProgram)

// ── HeroTile — terrain tile type under the hero character ─────────────────────
// DAT_07e118e8 ya declarado como DWORD cerca de la línea 546 (tipo de mundo/mapa)
#define HeroTile ((int)DAT_07e118e8)
// World = g_GameSubState = DAT_0055a7ac (already declared above)
#ifndef World
#define World    g_GameSubState

// ── Pools de items y atributos de terreno ────────────────────────────────────
// Centralizadas acá por el refactor B3 (2026-08-16). Antes cada .cpp las
// redeclaraba con su propio `extern`, y eso rompia el movimiento de funciones
// entre modulos: la funcion movida dejaba de ver la global de su archivo.
//
// Los cuatro pools de items son grids de slots ITEM (stride 0x44). Ojo: su
// indice de celda es `slot - 12`; los 12 wear slots NO viven aca sino en
// `CharacterMachine + 536 + 68*slot` (ver la entrada de 2026-08-08 g).
extern BYTE  OffsetInventoryItems[];
extern BYTE  OffsetTradeItems[];
extern BYTE  OffsetWarehouseItems[];
extern BYTE  OffsetMixItems[];
// TerrainWall = &DAT_0838bc70 (256x256 flags de tile: bit0 safezone, bit1
// ocupado por entidad, bit3 no-caminable).
extern unsigned char* TerrainWall;

#endif

// ── SkillAttribute @ 0x07D29D20 — 64 entradas × 40 bytes ─────────────────────
// WinMain reserva 0xA00 (= 2560) bytes y OpenSkillScript / el anti-tamper copian
// de a 0x28 (40) bytes por entrada, así que la stride es 40 — no 300.  Los
// consumidores indexan por BYTE: `&SkillAttribute[8 * (5*Level + 150)]`
// (= entrada 30+Level), con el nombre en el offset 0 de la entrada.
// 2026-08-21: antes era un único bloque de 300 bytes, así que cualquier índice
// leía fuera del objeto.
struct _SkillAttrEntry { char Raw[2560]; };
extern _SkillAttrEntry SkillAttribute;  // base of table @ 0x07D29D20

// ── CharactersClient extra BMD heap buffer ────────────────────────────────────
extern LPVOID  DAT_07abf164;   // extra BMD data heap block for character models

// ── FloatingLabel pool ────────────────────────────────────────────────────────
extern char    DAT_07c82cd0;   // floating label pool base (stride 0x70, limit 0x7c8588f)

// ── Water wave simulation buffers ─────────────────────────────────────────────
extern char    DAT_0814b6e0;   // water wave buffer A (256×256 ints, 2 banks × 0x40000 bytes)
extern char    DAT_0814b2e0[0x80000];   // grass-wind/water-wave double buffer (2×0x40000, sub_4F98C0/4F9A30)

// ── Small-function batch globals ─────────────────────────────────────────────
// Base de la tabla de quests — IDA la trata como un buffer de ~0x1C900 bytes. El código en
// stubs.cpp escribe en &DAT_00567500 + 0x1C8F8..+0x1C8FD (líneas 26586-26589)
// y HUD_Pass2:GetScreenWidth lee `g_csQuest + 0x1C8FF`. Un DWORD de 4 bytes
// acá significa que esos accesos caen más allá del final de nuestros globals →
// AVs aleatorios. Lo exponemos como array de BYTE del tamaño correcto.
extern BYTE    DAT_00567500[0x1C900];   // Quest table base (was DWORD)
extern DWORD   DAT_00590b00;       // Sound device context
extern DWORD   DAT_055c9b78;       // RefCount / tick counter

extern int     PTR_LAB_00552460;   // vtable for FUN_00403310
extern int     PTR_LAB_005524e8;   // vtable for FUN_00407970
extern int     PTR_FUN_00552508;   // vtable for FUN_00407de0
extern int     PTR_LAB_005527e4;   // vtable for FUN_0040f690
extern int     PTR_LAB_005524b8;   // vtable for FUN_00403ef0
extern int     PTR_FUN_005524c0;   // vtable for CWaveFile
extern int     PTR_FUN_005524c4;   // vtable for CErrorReport
extern int     PTR_FUN_005524d8;   // vtable for HashTable
extern int     PTR_LAB_00552810;   // vtable for StreamBase2
extern int     PTR_FUN_005527e0;   // vtable for Stream deinit
extern int     PTR_FUN_0055389c;   // vtable for GameGuard class

// Símbolos de vtable/literal-puntero off_XXXX de IDA, renombrados por port.py a DAT_xxxxxxxx.
// (en paralelo a los PTR_LAB_/PTR_FUN_ de arriba; port.py usa la forma DAT_)
extern DWORD   DAT_00552460;
extern DWORD   DAT_005524b4;
extern DWORD   DAT_005524b8;
extern DWORD   DAT_005524c0;
extern DWORD   DAT_005524c4;
extern DWORD   DAT_005524c8;
extern DWORD   DAT_005524d8;
extern DWORD   DAT_005524e8;
extern DWORD   DAT_00552508;
extern DWORD   DAT_00552514;
extern DWORD   DAT_00552548;
extern DWORD   DAT_00552568;
extern DWORD   DAT_00552574;
extern DWORD   DAT_00552588;
extern DWORD   DAT_005525a0;
extern DWORD   DAT_005525c8;
extern DWORD   DAT_00552668;
extern DWORD   DAT_00552760;
extern DWORD   DAT_005527e0;
extern DWORD   DAT_005527e4;
extern DWORD   DAT_005527f8;
extern DWORD   DAT_00552810;
extern DWORD   DAT_0055389c;

extern DWORD   DAT_07e11e50;       // _PartyNumber (already in line 574 - reuse)
extern DWORD   DAT_07e11e54;       // _PartyKey

extern HANDLE  lpTargetHandle_00563b5c; // GameGuard pipe handle B
extern HANDLE  hEvent;                  // GameGuard sync event
extern LPVOID  lpParameter;             // GameGuard thread param
extern char    lpWindowName_0055910c[]; // GameGuard window class name

extern DWORD   g_csQuest;         // Quest system state (0=inactive)

// Match/DevilSquare info
extern BYTE    m_byMatchType;
extern int     m_iMatchTimeMax;
extern int     m_iMaxKillMonster;
extern int     m_iKillMonster;

// Sound system (DirectSound8 pipeline — companion-project layout, 420 slots)
extern BOOL    g_EnableSound;                         // DAT_00590ac8 — master on/off
extern bool    g_Enable3DSound;                       // DAT_00590AC9 — 3D audio enabled
extern LPDIRECTSOUND          g_lpDS;                 // DAT_00590AD0 — main device
extern LPDIRECTSOUND3DLISTENER g_lpDS3DListener;      // primary-buffer listener
extern void*   wavefile;                              // DAT_00590AD8 — CWaveFile* currently being loaded
extern DWORD   g_dwBufferBytes;                       // DAT_00583DA8 — last WAV data chunk size

// Tablas por slot (stride 4, 420 entradas cada una — contiguas en la sección .data original):
extern int     MaxBufferChannel[420];                 // 0x0058E1C4 — # active channels (>0 means "loaded")
extern int     BufferChannel[420];                    // rotating channel index (0..MaxBufferChannel-1)
extern bool    Enable3DSound[420];                    // 0x00590924 — per-slot 3D flag
extern int     SoundLoadCount;                        // 0x00590ACC — running count of loaded sounds
extern char    BufferName[420][0x40];                 // per-slot filename (64 bytes)

extern LPDIRECTSOUNDBUFFER   g_lpDSBuffer  [420][4];  // 0x0058C780 — secondary buffers (up to 4 instances per slot)
extern LPDIRECTSOUND3DBUFFER g_lpDS3DBuffer[420][4];  // 0x0058EEE4 — parallel 3D-buffer array
extern DWORD   Object3DSound[420][4];                 // payload (entity id bound to each channel)

// Music

// Entity data
extern BYTE    MonsterScript[512 * 0x36];  // NPC/mob name table (stride 0x36; Type[0], Name[1..32])
extern char    WhisperRegistID[][4]; // whisper ID array (stride 4, 0x1b*4+2 = 0x6e bytes)

// Blur/trail system
// DAT_07c608b8 — campo int en el +16 del slot[0] de g_RenderPool_07c608a8 (ver arriba).
#define DAT_07c608b8  (*(int*)(&g_RenderPool_07c608a8[16]))

// Objetos mariposa/bicho — alias de DAT_083a1218 (misma dirección 0x083a1218 en el original)
#define Butterfles  (*(DWORD*)DAT_083a1218)

// ── Batch 3 globals ──────────────────────────────────────────────────────────
// BST / std::map sentinel
extern void   *DAT_055c9b98;       // RB-tree sentinel node (NIL)

// Camera
extern float   CameraAngle[3];     // DAT_083a42e0 (3 floats)
extern float   CameraPosition[3];  // DAT_083a42d4 (3 floats)

// Quest/NPC window
extern int     _g_bEventChipDialogEnable; // DAT_07e5ba80

// 2026-04-30: los flags de los paneles de UI ahora aliasan los bytes reales DAT_07eaa11x (per
// el Offsets.h del proyecto companion de IDA, líneas 59-69). Las direcciones de la época de
// Ghidra 0x07e5ba84/88 para InventoryOpened/CharacterOpened eran misidentificaciones — los
// flags reales están en DAT_07eaa116..11c (de un byte). Usamos #define para que tanto
// el código de toggle portado de IDA (escribe DAT_07eaa117) como los gates
// de render del HUD (leen `if (InventoryOpened)`) peguen en el mismo byte de memoria.
#define InventoryOpened    DAT_07eaa117
#define CharacterOpened    DAT_07eaa116
#define GuildOpened        DAT_07eaa114
#define PartyOpened        DAT_07eaa115
#define ShopOpened         DAT_07eaa118
#define WarehouseOpened    DAT_07eaa119
#define ChaosMixOpened     DAT_07eaa11a
#define TradeOpened        DAT_07eaa11b
#define EventWindowOpened  DAT_07eaa11c
#define GuildCreatorOpened DAT_07eaa124

// 2026-04-30: Inventory/Trade panel origin coords. Same unification pattern
// que los flags *Opened de arriba — IDA escribe/lee vía DAT_07ea5284..5290 y
// los pases de render del HUD usan los nombres de C++. Forzarlos a la misma memoria
// fixes the "panel right, items left" misalignment.
#define InventoryStartX      DAT_07ea5288
#define InventoryStartY      DAT_07ea5284
#define TradeInventoryStartX DAT_07ea5290
#define TradeInventoryStartY DAT_07ea528c

// SMD parser
extern FILE   *SMDFile;             // DAT_0055c0a0

// Codec / JPEG internal refs (function pointers stored as addresses)
// lpDefault_00583d88 already declared above (line ~1036) as DWORD
extern DWORD   DAT_00563e00;       // codec state A
// DAT_00560694 already declared above (line ~1616) as DWORD

// Missing globals (compilation fixes)
extern DWORD   DAT_07c82cf4[0xAF0];       // terrain alpha bitmap pool (0x2BC0 bytes = 100 slots * 112 bytes)
#define DAT_07c858b4 DAT_07c82cf4[0xAF0-1]   // end-marker alias
extern DWORD   DAT_0055339c;       // JPEG natural order table
// m_dwTextColor / m_dwBackColor SON DAT_00559c78 / DAT_00559c80 (mismo global en IDA:
// 0x559c78 / 0x559c80, verificado por disasm de sub_40D610 y sub_480980). El render de
// texto (FUN_0040f610) lee DAT_00559c78; sin esta unificación los seteos de m_dwTextColor
// (HUD passes + ChatListBox) se perdían → texto blanco. DAT_00559c78/80 declarados arriba.
#define m_dwTextColor  DAT_00559c78
#define m_dwBackColor  DAT_00559c80
// g_iNumAnswer — alias de DAT_083a7c0c (ver arriba).
// g_iNumLineMessageBoxCustom — alias de DAT_083a4324 (ver arriba).
extern char   *g_lpszMessageBoxCustom[16]; // Quest dialog text lines
// g_iCurrentDialogScript — alias de DAT_005615dc (ver arriba).
// g_lpszDialogAnswer — alias de DAT_083a4348 (ver arriba).
// 2026-07-19: m_hFontDC NO es un global aparte — en IDA sub_50F5F0 hace
// `m_hFontDC = CreateCompatibleDC(hdc)` y ese mismo DC es DAT_055c9fec (el font
// memory DC, declarado en stdafx.h). Tenerlos separados dejaba m_hFontDC en NULL
// para siempre (125 usos, 0 asignaciones) -> GetTextExtentPoint32A fallaba.
#define m_hFontDC  DAT_055c9fec
extern HFONT   g_hFontBold;        // Bold font handle

// Batch 18 — InitGame / ReceiveChat globals
extern DWORD   EnableUse;          // item use enabled flag
extern int     DAT_07e11998;       // SendGetItem
// DAT_07e11d28 already declared above (line ~554) as DWORD
extern int     DAT_07e11e10;       // SoccerTime
extern int     DAT_07e11e14;       // SoccerObserver
extern int     DAT_07e11994;       // SelectedNpc
extern int     DAT_07e11990;       // SelectedOperate
// DAT_07e1198c already declared above (line ~1003) as DWORD
extern int     DAT_07e11988;       // SelectedItem
// DAT_07e11984 already declared above (line ~1002) as DWORD
extern int     DAT_07e11e18;       // m_bAutoAttack
extern int     DAT_07e11d24;       // _CheckInventory
// DAT_07e11d1c already declared above (line ~553) as DWORD
extern int     DAT_07e11d18;       // RepairEnable_0
extern int     DAT_07e11d14;       // RepairEnable
extern BYTE    DAT_00559c6d;       // UI alpha/state byte
extern short   DAT_07e11e1c;       // _g_shEventChipCount
extern short   DAT_07e11e20;       // g_shMutoNumber[0]
extern short   DAT_07e11e22;       // g_shMutoNumber[1]
extern short   DAT_07e11e24;       // g_shMutoNumber[2]
extern BYTE    DAT_07e11e26;       // m_bWhisperSound
extern int     DAT_07e11980;       // SceneFlag

// Batch 19 — SendCheck globals
extern DWORD   DAT_07e11d10;       // g_bGameServerConnected
extern BYTE    DAT_07db8600;       // g_byPacketSerialSend
extern BYTE    DAT_05826cfc;       // SendCheck first-call flag
extern DWORD   DAT_05826d00;       // SendCheck first-call tickCount

// Batch 20 — OpenNpc, MoveCamera, RenderEquipment3D, RenderItems3D, FUN_0043ce50, LookAtTarget
// Sólo los globals realmente nuevos (los que no estaban declarados arriba):
extern float   _DAT_005524a0;      // equip pendant X offset
extern float   _DAT_00552c18;      // equip slot Y offset (helm row)
extern float   _DAT_00552c10;      // equip slot X offset (pants/gloves/boots col)
extern float   _DAT_00552c0c;      // equip slot Y offset (mid row)
extern float   _DAT_00552c08;      // equip slot Y offset (ring/boots row)
extern float   _DAT_00552c04;      // equip slot X offset (weapon_L/ring_R col)
extern float   _DAT_005527dc;      // arrow count X offset
extern float   DAT_083a7adc[];     // CurrentCameraWalkDelta[6] (pos delta + angle delta)
extern float   _DAT_00552a30;      // lerp speed (camera smoothing)
extern void*   g_LoginSceneObjects[9]; // exact CreateObject pointers for login bg: sky, 3 ships, 3 waves, banner, sun

// Batch 22 — AttackStage, CreateArrow
extern int     DAT_00559858;       // g_iLimitAttackTime (attack frame limit counter)
extern DWORD   DAT_05826d10;       // CurrentSkill (current skill ID for arrow/projectile)
extern float   _DAT_00552904;      // sin/cos offset multiplier (sword trail radius)
extern float   _DAT_005528f8;      // sin/cos offset multiplier (slash projectile)
extern float   _DAT_005528f4;      // combo animation offset constant

// Batch 21 — RenderSkillIcon, CheckMixRecipe, MoveObjects, CreateArrows, CollisionDetectLineToMesh
extern DWORD   DAT_083a3fec;       // visible object counter (MoveObjects)
extern DWORD   DAT_07eaa178;       // mix recipe: socket flag byte
extern DWORD   DAT_07eaa170;       // mix recipe: wing socket option value
extern int     DAT_07eaa174;       // mix recipe: combined value score (capped at 0x5a)
extern char    DAT_00559c6c;       // last rendered hotkey char
extern float   _DAT_00552ab0;      // arrow angle offset (5-way shot small)
extern float   _DAT_00552584;      // arrow angle offset (5-way shot large)
extern float   _DAT_00552a4c;      // RenderNumber2D Y offset

// ── UseSkillWizard dependencies ──────────────────────────────────────────────
extern DWORD   DAT_07d780a0;       // MovementSkillTarget (index into CharactersClient)
extern DWORD   DAT_07d7809c;       // skill slot index (current selected skill slot)
extern char    DAT_00559d94[8];    // GM name string "webzen" (anti-impersonation check 1)
extern char    DAT_00559d9c[8];    // GM name string "webzen" (anti-impersonation check 2)

// ── SkillElf dependencies ────────────────────────────────────────────────────
extern char    DAT_00559db4;       // GM name check string (part of "webzen" pattern)
extern char    DAT_07e11dfc;       // chat log widget ID string (for AddText)
extern char    DAT_07d4c89c;       // "Not enough mana" message string

// ── MoveParticles camera shake globals (0x07c800f8..0x07c8010c) ─────────────
extern float   DAT_07c800f8;       // camera shake accumulator X
extern float   DAT_07c800fc;       // camera shake accumulator Y
extern float   DAT_07c80100;       // camera shake accumulator Z
extern float   DAT_07c80104;       // camera shake velocity X
extern float   DAT_07c80108;       // camera shake velocity Y (DAT_07c80104+4, loop end 0x7c8010c)

// ── MoveParticles float constants ────────────────────────────────────────────
extern float  _DAT_00552a60;       // particle damping factor (type 0x4ab sub 9)
extern float  _DAT_00552a7c;       // particle angle increment (type 0x4c5)
extern float  _DAT_00552ac4;       // particle scale delta (type 0x565 sub!=0)
extern float  _DAT_00552ac8;       // particle scale delta (type 0x565 sub==0)
// _DAT_00552acc — already declared above (line ~958)
extern float  _DAT_00552ad0;       // sin frequency (type 0x599)
extern float  _DAT_00552ad4;       // gravity factor (type 0x598)
extern float  _DAT_00552ad8;       // sin frequency (type 0x4cf)
// _DAT_00552adc — already declared above (line ~151)
extern float  _DAT_00552ae0;       // sin frequency (type 0x4ce/0x4d0/0x4d2)
extern float  _DAT_00552ae8;       // color decay (type 0x4e1 sub 4)
extern float  _DAT_00552aec;       // green channel factor (type 0x4c4 sub 9)
extern float  _DAT_00552af0;       // red channel factor (type 0x4c4 sub 9)
extern float  _DAT_00552af4;       // velocity damping (type 0x4c4 sub 8)
extern float  _DAT_00552af8;       // lifetime-to-light factor (type 0x4c4 sub 8)
extern float  _DAT_00552afc;       // blue channel factor (type 0x4c4 sub 7)
extern float  _DAT_00552b00;       // green channel factor (type 0x4c4 sub 7)
extern float  _DAT_00552b04;       // red channel factor (type 0x4c4 sub 7)
extern float  _DAT_00552b08;       // sin frequency (type 0x4c4 sub 6)
extern float  _DAT_00552b0c;       // sin frequency (type 0x4f4/0x4c4 sub 6)
extern float  _DAT_00552b10;       // blue tint factor (type 0x4c4 sub 4)
extern float  _DAT_00552b14;       // green tint factor (type 0x4c4 sub 4)
extern float  _DAT_00552b18;       // red tint factor (type 0x4c4 sub 4)
extern float  _DAT_00552b1c;       // lifetime-to-light factor (type 0x4c4 sub 0/3/4/7)
extern float  _DAT_00552b20;       // velocity Y adjust (type 0x4b0 sub 1/5)
extern float  _DAT_00552b24;       // velocity damping (type 0x4ac)
extern float  _DAT_00552b28;       // lifetime-to-light factor (type 0x4a6/0x4d2/0x498)
extern float  _DAT_00552b2c;       // velocity damping (type 0x4ab/0x47e)
extern float  _DAT_00552b30;       // random offset scale (type 0x47e)
extern float  _DAT_00552b34;       // color decay (type 0x47e sub 4)
extern float  _DAT_00552b38;       // scale decay factor (type 0x47e sub 4)
extern float  _DAT_00552b3c;       // gravity random scale (type 0x47e sub 4)
extern float  _DAT_00552b40;       // velocity damping (type 0x47e sub 3)
extern float  _DAT_00552b44;       // velocity damping general (multiple types)
extern float  _DAT_00552b48;       // camera shake clamp lower
extern float  _DAT_00552b4c;       // camera shake clamp upper
extern float  _DAT_00552b50;       // camera shake bounce / terrain bounce factor
extern float  _DAT_00552b54;       // camera shake random scale

// ── Additional math/render constants (added for CreateAngle/Terrain_RenderTexturedObject) ──
extern float  _DAT_00552860;       // 57.29578f = 180/pi (radians to degrees)
extern DWORD   DAT_00552860;
extern float  _DAT_00552b9c;       // 0.015625f = 1/64 (terrain UV step)
extern DWORD   DAT_00552b9c;
extern float  _DAT_00552cb4;       // 64.0f (terrain tile size)
extern DWORD   DAT_00552cb4;

// ── CheckArrow / CharSelect chat message string globals ──
extern char    DAT_07e11df4;        // arrow-required error string ID
extern char    DAT_07e11df8;        // bolt-required error string ID
extern char    DAT_07d3c348;        // empty chat message string

// ── Skill selection globals ──
extern char    DAT_07d78098;        // skill mode flag (0=direct, nonzero=from attribute)
extern DWORD   DAT_00559c84;        // InputEnable flag (already declared above, kept for reference)

// ── CharSelect packet globals ──
extern DWORD   DAT_005616ac;        // SelectedHero index (already declared above, kept for reference)

// ── Chat bubble pool ──
// Pool base = 0x07E016F8; stride 0x254 per entry; ~96 slots.
// Per-slot layout:
//   +0x000 char[24]  ID (sender name)
//   +0x018 char[12]  guild mark string  ("[GuildName]")
//   +0x024 char      color index
//   +0x025 char      guild team byte
//   +0x028 int       timer1 (tick counter for line 1)
//   +0x02c char[256] text line 1
//   +0x12c char[256] text line 2
//   +0x22c int       display timer 1
//   +0x230 int       display timer 2
//   +0x234 DWORD     owner CHARACTER* pointer
extern char    DAT_07e016f8[100 * 0x254];   // chat bubble pool: 100 slots x 596B
// unk_7E01720 = base + 40 (campo timer1). MoveChat arranca su walk ahi.
#define DAT_07e01720  (&DAT_07e016f8[40])
extern char    DAT_00559d60;        // guild mark prefix string (e.g. "[")
extern short   DAT_00559d64;        // guild mark suffix 2-byte (e.g. "]")
extern char    DAT_00559d66;        // guild mark suffix trailing byte

// ── HUD render globals (Phase-2 port) ────────────────────────────────────────
// Agregados el 2026-04-29 para el port de RenderPartyHP / RenderMainFrameWindow /
// RenderBooleans / Render_HotbarItems3D from IDA.
//
// Estado de party / soccer / guild war — binding mínimo para que los renderers
// sigan funcionando antes de que se porten los sistemas completos de party/guild.
// Los valores por defecto en 0 hacen que todos esos caminos tomen sus ramas de
// salida temprana, así que el juego corre igual con los ports incompletos.
extern int     PartyNumber;          // 0x07EAA0E0 — count of valid party slots
extern BYTE    Party[2048];          // 0x07E11E80 — Party struct array (stride 36, header 24)
extern int     EnableGuildWar;       // GW active flag
extern int     HeroSoccerTeam;       // 0/1 — hero's soccer team index
extern int     GuildWarScore[2];     // [self, enemy]
extern char    GuildWarName[80];     // enemy guild name
extern char    SoccerTeamName[2][80];// team names

// Float-text floating-numbers entry pool (RenderBooleans iterator).
// Layout per entry (596 bytes = 149 dwords):
//   +0x000 (DWORD)  tipo/flag (se lee como `*((int*)v0 - 2)` y `*((int*)v0 - 131)`)
//   +0x008 (DWORD)  owner ptr (entity*)
//   +0x010 (3*float) world position fallback
//   +0x224 (DWORD)  extent de texto cx (lo setea sub_480C60)
//   +0x228 (DWORD)  text-extent cy
//   +0x22C..       text strings: at +44, +300, +24, +base
//   +0x22C (int)   countdown-1
//   +0x230 (int)   countdown-2
// Pool spans 0x07E01924..0x07E101F4 (~60 KB, ~100 entries).
#define DAT_07e01924  (&DAT_07e016f8[0x22C])   // = base + disp1 (mismo pool)

// Globals de fuente / medición de texto que consumen sub_480C60 y el HUD
// renderers (HFONT object handles + DC + computed dimensions).
extern HFONT   g_hFont;              // primary plain font handle
extern int     FontHeight;           // pixel height of g_hFont
extern SIZE    TextSize;             // shared scratch SIZE for text extent

// Anti-tamper hash-table state — already declared above as g_HashTableCtx[4]
// con las macros DAT_055c9bc8/cc/d0/d4. Sólo referenciarlos; NO volver a hacerles extern.

// CharacterMachine — the encrypted player attribute buffer (size 0x584).
// Los desencripta in situ el bloque anti-tamper interno de RenderMainFrameWindow.
extern void   *CharacterMachine;     // pointer to encrypted CHARACTER struct
// CharacterAttribute aliases DAT_07cf1ff4 (already declared at line ~569).
#define CharacterAttribute  DAT_07cf1ff4

// DAT_07e11d6e (flag de UI sucia) ya está declarado en la línea 620 como `char`.

