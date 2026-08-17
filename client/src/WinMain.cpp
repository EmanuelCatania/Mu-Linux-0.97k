// WinMain.cpp
// WinMain @ 0x0041FC00  (1497 lines, completo)
// Window_Create @ 0x0041DFF0
// WndProc @ 0x004149D0  (4074 lines, ~80% anti-tamper — ver comentario abajo)
// OpenGL_Init @ 0x0041DE30
// OpenGL_Release @ 0x0041AF20
// GameGuard_Init @ 0x00406F20
// GameGuard_GetStatus @ 0x00406F60
// GameGuard_TickCheck @ 0x00406F90
// Net_Connect @ 0x0043DC70   (__thiscall, ctx=NetCtx @ ((int)(uintptr_t)DAT_055ca160))
// Net_Recv @ 0x0043DE30      (__thiscall)
//
// Globals clave:
//   g_GameState    @ 0x005615c0
//   g_hWnd         @ 0x055c9ffc
//   g_hDC          @ 0x055c9fec  (= DAT_055ca004 en decompile)
//   g_hRC          @ 0x055ca008
//   g_hInst        @ 0x055ca000
//   g_MouseX       @ 0x083a427c  (0..639, normalizado a 640x480)
//   g_MouseY       @ 0x083a4278
//   g_ScreenW      @ 0x0056156c
//   g_ScreenH      @ 0x00561570
//   Entity array base @ 0x07abf5d0  = DAT_055c9e48 + rand()%0x80 * 0x394  (RANDOMIZADO)
//   Player ptr     @ 0x07abf5d8  = entity array base (slot 0)
//   NetCtx         @ ((int)(uintptr_t)DAT_055ca160)
//   socket handle  @ 0x055ca168
//   buffer de envío @ 0x055ca16c  (máx 0x2001, cola de WSAEWOULDBLOCK)

#include "stdafx.h"
#include "Net/HWID.h"
#include "Scene/Scene.h"
#ifdef _DEBUG
#  define _CRTDBG_MAP_ALLOC
#  include <crtdbg.h>
#endif

// 2026-05-04: helper de src/UI/Chat_InputTick.cpp — envía una línea de chat
// escrita en InputText[0] (DAT_07db8710 slot 0) por WM_CHAR; la llama el
// handler de Enter en WndProc cuando InputEnable=1 y el buffer no está vacío.
extern "C" void Chat_SendChatLine(const char* text);
extern "C" DWORD g_ItemAttribute_Backup;   // src/globals.cpp — recovery pointer

// ── GLOBALS ───────────────────────────────────────────────────────────────────

int       g_GameState = 0;    // 0x005615c0
HWND      g_hWnd     = NULL;  // 0x055c9ffc
HINSTANCE g_hInst    = NULL;  // 0x055ca000
HDC       g_hDC      = NULL;  // 0x055ca004
HGLRC     g_hRC      = NULL;  // 0x055ca008

// Forward declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int  OpenGL_Init(void);
void OpenGL_Release(void);
static void GameGuard_Init(CHAR* hWnd);
static int  GameGuard_GetStatus(void);
static void GameGuard_TickCheck(void);
int  Config_Load(void);  // see Config/Config_Load.cpp

// ── Window_Create @ 0x0041DFF0 ───────────────────────────────────────────────
//
// Registra WNDCLASSA y crea la ventana principal.
//   style:  CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS = 0x2B
//   exStyle: 0x40008 (WS_EX_APPWINDOW|WS_EX_TOPMOST)
//   class name: "Dialog" (s_Dialog_005595e0)
//   window style: WS_POPUP (0x80000000)
//   Dimensions: DAT_0056156c × DAT_00561570 (de ChangeDisplaySettings)
// ─────────────────────────────────────────────────────────────────────────────
static void Window_Create(HINSTANCE hInst)
{
    WNDCLASSA wc     = {};
    wc.style         = 0x2B;            // CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIconA(NULL, "IDI_ICON1");
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "Dialog";
    RegisterClassA(&wc);

    // DAT_0056156c / DAT_00561570 los setean EnumDisplaySettings + ChangeDisplaySettingsA
    // (declarados en globals.h como DWORD)
    g_hWnd = CreateWindowExA(
        0x40008,                         // WS_EX_APPWINDOW|WS_EX_TOPMOST
        "Dialog",
        "Mu Online",
        WS_POPUP,
        0, 0, DAT_0056156c, DAT_00561570,
        NULL, NULL, hInst, NULL
    );
    // g_hWnd → DAT_055c9ffc
}

// ── OpenGL_Init @ 0x0041DE30 (completo, ~50 líneas) ─────────────────────────
//
// PIXELFORMATDESCRIPTOR: nSize=40, nVersion=1, dwFlags=0x25
//   (PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER), RGBA, 16-bit color, 16-bit depth
// Secuencia:
//   1. GetDC(g_hWnd)        → g_hDC (DAT_055ca004)   error: "OpenGL Get DC Error"
//   2. ChoosePixelFormat                               error: "OpenGL Choose Pixel Format Error"
//   3. SetPixelFormat                                  error: "OpenGL Set Pixel Format Error"
//   4. wglCreateContext(hDC)→ g_hRC (DAT_055ca008)   error: "OpenGL Create Context Error"
//   5. wglMakeCurrent(hDC, hRC)                       error: "OpenGL Make Current Error"
//   6. ShowWindow(g_hWnd, SW_SHOW=5) + SetForegroundWindow + SetFocus
//   7. return 1
// Cada error: FUN_00405540(log, errStr) + OpenGL_Release() + MessageBoxA + return 0
// ─────────────────────────────────────────────────────────────────────────────
static int OpenGL_Init(void)
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR); // 40 = 0x28
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // 0x25
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;

    g_hDC = GetDC(g_hWnd); // DAT_055ca004
    if (!g_hDC) {
        // FUN_00405540(DAT_055c9bf0, "OpenGL Get DC Error");
        // OpenGL_Release(); MessageBoxA(g_hWnd, ..., "IError", MB_OK);
        return 0;
    }
    int fmt = ChoosePixelFormat(g_hDC, &pfd);
    if (!fmt) { /* log "OpenGL Choose Pixel Format Error" + release */ return 0; }
    if (!SetPixelFormat(g_hDC, fmt, &pfd)) { /* log "OpenGL Set Pixel Format Error" + release */ return 0; }

    g_hRC = wglCreateContext(g_hDC); // DAT_055ca008
    if (!g_hRC) { /* log "OpenGL Create Context Error" + release */ return 0; }
    if (!wglMakeCurrent(g_hDC, g_hRC)) { /* log "OpenGL Make Current Error" + release */ return 0; }

    // NOTA: experimento previo con glFrontFace(GL_CW) removido. La apariencia
    // "X-ray" era en realidad causada por el bug NaN en BMD_DrawMesh (param
    // blendMesh reinterpretado como NaN → EnableAlphaBlend(GL_ONE,GL_ONE) con
    // depth-mask off), no por el winding. GL_CW rompía los quads 2D (UI/sky
    // en CCW quedaban culled). Dejamos el default GL_CCW.

    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);
    return 1;
}

// ── OpenGL_Release @ 0x0041AF20 (34 líneas, completo) ───────────────────────
//
// Libera el contexto OpenGL y restaura el modo de video.
// Llamado desde: WndProc(WM_USER+1), Window_FatalError, Scene_Dispatch (post re-auth).
//
// Secuencia:
//   1. wglMakeCurrent(NULL, NULL)
//        si falla: log "GL - Release Of DC And RC Failed"
//   2. wglDeleteContext(g_hRC)   → DAT_055ca008
//        si falla: log "GL - Release Rendering Context Failed"
//   3. DeleteDC(g_hDC)           → DAT_055ca004
//        si falla: log "GL - Release Device Context Failed"
//   4. ReleaseDC(g_hWnd, g_hDC)
//   5. ChangeDisplaySettingsA(NULL, 0)  — restaura modo de video original
//   6. ShowCursor(1)                    — muestra cursor del sistema
// ─────────────────────────────────────────────────────────────────────────────
void OpenGL_Release(void)
{
    // Libera DirectSound antes de desarmar OpenGL/la ventana. FreeDirectSound
    // is a no-op if g_EnableSound is FALSE, so safe to call unconditionally.
    FreeDirectSound();

    if (!wglMakeCurrent(NULL, NULL)) {
        // FUN_00405540(DAT_055c9bf0, "GL - Release Of DC And RC Failed");
    }
    if (!wglDeleteContext(g_hRC)) {
        // FUN_00405540(DAT_055c9bf0, "GL - Release Rendering Context Failed");
    }
    if (!DeleteDC(g_hDC)) {
        // FUN_00405540(DAT_055c9bf0, "GL - Release Device Context Failed");
    }
    ReleaseDC(g_hWnd, g_hDC);
    ChangeDisplaySettingsA(NULL, 0);
    ShowCursor(1);
}

// ── GameGuard_Init @ 0x00406F20 (15 líneas, completo) ────────────────────────
//
// Inicia nProtect GameGuard.
// param_1: HWND casteado a CHAR* (Ghidra lo tipea como CHAR*)
//   1. FUN_0053d410(param_1) → si retorno != DAT_005538a0: FUN_00406fd0()
//
// Patchs.cpp bypass:
//   SetByte(0x00406F36, 0xEB)  → JMP sobre check de init
//   SetByte(0x00406F5F, 0xEB)  → JMP sobre messagebox de error
//   SetByte(0x00406F9B, 0xEB)  → JMP sobre messagebox de error
// ─────────────────────────────────────────────────────────────────────────────
static void GameGuard_Init(CHAR* hWnd)
{
    // int result = FUN_0053d410(hWnd);  // npGameGuard::init(hwnd)
    // if (result != DAT_005538a0):
    //     FUN_00406fd0();               // GameGuard_InitFail() — messagebox + abort
    (void)hWnd;
}

// ── GameGuard_GetStatus @ 0x00406F60 (13 líneas, completo) ──────────────────
//
// Comprueba si el proceso de GameGuard sigue vivo.
//   if (DAT_083bbb14 == NULL): return 0   // handle de GG nulo → bypass activo
//   else: return GameGuard_QueryProcess(DAT_083bbb14)
//           → GetExitCodeProcess(ggHandle)
//           → OpenEventA(ggEventName)   — channel de eventos GG
//
// Patch bypass (Patchs.cpp): DAT_083bbb14 = NULL → siempre retorna 0
// ─────────────────────────────────────────────────────────────────────────────
static int GameGuard_GetStatus(void)
{
    // if (DAT_083bbb14 == NULL) return 0;
    // return FUN_0053d470(DAT_083bbb14);  // GameGuard_QueryProcess
    return 0;
}

// ── GameGuard_TickCheck @ 0x00406F90 (14 líneas, completo) ──────────────────
//
// Watchdog del GameGuard, llamado desde WM_TIMER (id=1000, period=20000ms).
//   int status = GameGuard_GetStatus()
//   if (status != DAT_005538a0):
//       MessageBoxA(g_hWnd, errorStr, "IError", MB_OK)
//       Window_FatalError(NULL, '\0')
//
// Patch bypass: DAT_005538a0 = 0 y DAT_083bbb14 = NULL
//   → GameGuard_GetStatus() retorna 0 == DAT_005538a0 → check siempre pasa
// ─────────────────────────────────────────────────────────────────────────────
static void GameGuard_TickCheck(void)
{
    // int status = GameGuard_GetStatus();
    // if (status != DAT_005538a0):
    //     MessageBoxA(g_hWnd, DAT_007d29e50_errStr, "IError", MB_OK);
    //     Window_FatalError(NULL, '\0');
}

// ── Net_Connect @ 0x0043DC70 (51 líneas, __thiscall) ─────────────────────────
//
// Conecta el socket al servidor.
//   this+0  = hWnd (ventana que recibe WM_USER con lParam de WSAAsyncSelect)
//   this+8  = SOCKET
//   param_2 = char* serverIP
//   param_3 = UINT wMsg  (el mensaje Windows para WSAAsyncSelect; WinMain pasa WM_USER=0x400)
//   param_4 = ushort port
//
// Secuencia:
//   1. u_long addr = inet_addr(param_2)
//      if (addr == INADDR_NONE):
//          HOSTENT* h = gethostbyname(param_2)
//          if (!h): return -1   // falló el DNS
//          addr = *(u_long*)h->h_addr_list[0]
//   2. SOCKADDR_IN sa; sa.sin_family=AF_INET; sa.sin_port=htons(param_4); sa.sin_addr=addr
//   3. this+8 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
//      if (INVALID_SOCKET): return -1
//   4. connect(this+8, &sa, sizeof(sa))
//      if error != WSAEWOULDBLOCK: return -1   // (0x2733 = ok, sigue conectando)
//   5. WSAAsyncSelect(this+8, this+0, param_3, FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE)
//      FD_READ=1, FD_WRITE=2, FD_CONNECT=0x10, FD_CLOSE=0x20 → lEvent=0x23
//   6. return 0
// ─────────────────────────────────────────────────────────────────────────────

// ── Net_Recv @ 0x0043DE30 (78 líneas, __thiscall) ────────────────────────────
//
// Lee datos del socket al ring buffer de recepción.
//   this → NetCtx @ ((int)(uintptr_t)DAT_055ca160)
//   Buffer: this+0x2010, capacidad 0x2000; puntero de llenado: this+0x4010
//
// Secuencia:
//   1. recv(socket, ctx+0x2010+fill, 0x2000-fill, 0) → bytes recibidos
//      if SOCKET_ERROR: error de red → return
//   2. fill += bytes_received
//   3. Dispatch loop: mientras fill >= mínimo de cabecera:
//        byte[0] == 0xC1 || 0xC3:  pktLen = byte[1]
//        byte[0] == 0xC2 || 0xC4:  pktLen = byte[1]*256 + byte[2]
//        if fill < pktLen: break   // paquete incompleto, esperar
//        FUN_0043df90(ctx, ptr, pktLen)  // Net_ProcessPacket(ctx, data, len)
//        ptr += pktLen; fill -= pktLen
//   4. Si quedan bytes (paquete parcial): memmove(ctx+0x2010, ptr, fill)
//      ctx+0x4010 = fill
// ─────────────────────────────────────────────────────────────────────────────

// ── WinMain @ 0x0041FC00 (1497 líneas, completo) ─────────────────────────────
//
// SECUENCIA REAL DE INICIALIZACIÓN (del decompile, líneas 1-560):
//
//   1.  FUN_00406af0()                          — app init / anti-tamper setup
//   2.  HashTable tracking de DAT_055ca01c
//   3.  FUN_00510fe0()                          — world pre-init
//   4.  MuExe_IntegrityCheck()                  — verifica integridad del .exe
//       FileVersion_Get() → formatea string de versión
//   5.  FUN_004065f0(local_3d4)                 — anti-tamper continued
//   6.  Config_ReadServerAddr(this, cmdLine, &DAT_055c9e04, &port)
//         → PTR_s_connect_muonline_co_kr_005615b8 = &DAT_055c9e04
//         → DAT_005615bc = port
//       si cmdline == "-1":
//         WinExec(serverAddr, SW_SHOW)           — lanza otra instancia
//   7.  FindWindowA("Dialog", windowTitle)
//         → si existe: SendMessageA(hWnd, WM_CLOSE, 0, 0)   — mata instancia previa
//   8.  App_SingleInstanceCheck() @ 0x00412cd0  — mutex / named pipe check
//   9.  DataFile_LoadEnc(&DAT_05826c10, "Data_Enc1.dat")  @ 0x0053d180
//       DataFile_LoadDec(&DAT_05826c58, "Data_Dec2.dat")  @ 0x0053d1a0
//  10.  Config_Load()  — lee config.ini + registry
//         si retorna 0: log "config_ini_read_error" + abort
//  11.  EnumDisplaySettings loop:
//         operator_new(count * 0x94)   — array de DEVMODEA
//         busca modo con: dmBitsPerPel==16, dmPelsWidth==DAT_0056156c, dmPelsHeight==DAT_00561570
//         ChangeDisplaySettingsA(mode, CDS_FULLSCREEN)
//  12.  Window_Create(hInst)   → DAT_055c9ffc = hWnd
//  13.  OpenGL_Init()          → si retorna 0: abort
//  14.  GameGuard_Init((CHAR*)hWnd)   ← nota: HWND casteado a CHAR*
//  15.  Font size por resolución (DAT_07d78080):
//         0x280 (640)  → 0x0c
//         0x320 (800)  → 0x0d
//         0x400 (1024) → 0x0e
//         0x500 (1280) → 0x0f
//       CreateFontA(0,0,0,0,400,...)   → DAT_055ca00c  (normal)
//       CreateFontA(0,0,0,0,700,...) ×2 → DAT_055ca010, DAT_055ca014  (bold)
//  16.  FUN_0054283e(0, DAT_055c9d00)  — WSAStartup(0x0202, &wsaData)
//  17.  SetTimer(hWnd, 1000, 20000, NULL)  — GameGuard watchdog (20s)
//  18.  srand(FUN_00542762(NULL))       — time() seed; + obfuscación rand()
//
//  19.  BUFFER ALLOCATIONS:
//         DAT_07cf5600 = new(900)             → cleared 0xe1*4 bytes
//         DAT_07d29d20 = new(0xa00)           → cleared 0x280*4 bytes
//         DAT_07cf1ff8 = new(0xa00)           → cleared 0x280*4 bytes
//         DAT_055c9e44 = new(0x18000)         → DAT_07d78068 = base + rand()%0x400 * 0x40
//                                               cleared 0x2000*4 bytes
//         DAT_07cf1ff0 = new(0x8000)          → cleared 0x2000*4 bytes
//         DAT_055c9e48 = new(0x764d4)         → DAT_07abf5d0 = base + rand()%0x80 * 0x394
//                                               ← ENTITY ARRAY (BASE RANDOMIZADA CADA LAUNCH)
//                                               cleared 0x166b5*4 bytes
//         local_9ac    = new(0x584)           → DAT_07cf1ffc (HashTable object)
//                                               cleared 0x161*4 bytes
//
//  20.  DAT_07cf1ff4 = DAT_07cf1ffc
//       FUN_0047d3d0(DAT_07cf1ffc)  — HashTable_Init
//  21.  DAT_07abf5d8 = DAT_07abf5d0  — player ptr = entity array base (slot 0)
//  22.  vtable constructions:
//         new(0x5c8) → FUN_0040c7d0 → DAT_055c9ff0
//         new(0xbc)  → FUN_0040e990 → DAT_055c9ff4
//         new(0xc)   → FUN_0040f500 → DAT_055c9ff8
//  23.  SystemParametersInfoA(0x61, 1, ...)  — ajusta velocidad del ratón
//
//  ── MESSAGE LOOP ────────────────────────────────────────────────────────────
//   PeekMessageA loop (non-blocking):
//     Si mensaje disponible: GetMessageA → si WM_QUIT: salir
//       Filtra WM_SYSKEYDOWN/UP antes de DispatchMessageA
//     Si no hay mensaje: Scene_Dispatch(g_hDC)
//
//  ── ANTI-TAMPER ─────────────────────────────────────────────────────────────
//   ~60 bloques unreachable (dead code). 63 phantom stack params (anti-tamper stack padding).
//   HashTable tracking de DAT_055ca01c, DAT_055ca028, etc. entre cada paso.
// ─────────────────────────────────────────────────────────────────────────────
// ── TRACING DE DEBUG (temporal, para diagnosticar la pantalla negra) ──────────
extern "C" void DbgLogPublic(const char* msg);
extern "C" void CsmWatchdog(const char *tag);
static void DbgLog(const char* msg)
{
    // 2026-04-30: el guard IsDebuggerPresent fue removido porque la fuente
    // original del crash en VS (MuEmu::DumpHex emitiendo bytes binarios) ya
    // está silenciada bajo #ifdef MUEMU_TRACE.  Ahora podemos loggear bajo
    // VS sin riesgo de AV en KernelBase, y los traces son visibles en
    // debug.log.  El sanitizado a ASCII printable abajo sigue activo como
    // segunda red de seguridad.
    static __declspec(thread) bool s_inside = false;
    if (s_inside) return;
    s_inside = true;

    static volatile LONG s_killed = 0;
    if (s_killed) { s_inside = false; return; }

    char buf[600];
    const char* safeMsg = msg ? msg : "(null)";
    DWORD t = GetTickCount();

    // 2026-04-29: sanitize msg para que solo tenga ASCII printable. VS debugger
    // intercepta WriteFile en debug.log y crashea cuando el contenido tiene
    // caracteres no-printables (probablemente del hex dump de DumpHex que pasa
    // bytes binarios como string al format %s).
    char sanitized[510];
    {
        int j = 0;
        for (int i = 0; safeMsg[i] && j < (int)sizeof(sanitized) - 1; ++i) {
            unsigned char c = (unsigned char)safeMsg[i];
            sanitized[j++] = (c >= 0x20 && c < 0x7F) || c == '\t' ? c : '?';
        }
        sanitized[j] = 0;
    }

    int n = wsprintfA(buf, "[%u] %s\r\n", t, sanitized);
    if (n > 0) {
        // 2026-04-29: NO usar OutputDebugStringA — VS debugger lo intercepta
        // y dispara una AV en KernelBase cuando el debug message contiene
        // bytes que VS no puede mostrar (probablemente el hex dump de
        // MuEmu::DumpHex que tiene caracteres binarios).
        // Solo escribir a archivo, con kill-switch.
        static HANDLE h = INVALID_HANDLE_VALUE;
        if (h == INVALID_HANDLE_VALUE) {
            h = CreateFileA("debug.log", GENERIC_WRITE, FILE_SHARE_READ,
                            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (h == INVALID_HANDLE_VALUE) { s_killed = 1; s_inside = false; return; }
        }
        // 2026-05-01 (v2): VS debugger captura first-chance exceptions ANTES
        // que el SEH del programa, aún con __try/__except. La única forma de
        // evitar la pausa de VS es NO LLAMAR WriteFile cuando hay debugger.
        // Bajo debugger: solo OutputDebugStringA → Output Window de VS.
        // Sin debugger: WriteFile normal a debug.log.
        DWORD w = 0;
        if (IsDebuggerPresent()) {
            OutputDebugStringA(buf);
        } else {
            if (!WriteFile(h, buf, n, &w, NULL)) {
                s_killed = 1;
            }
        }
    }
    s_inside = false;
}

extern "C" void DbgLogPublic(const char* msg) { DbgLog(msg); }

// ── HEAP CHECKPOINT ───────────────────────────────────────────────────────────
// Llama _CrtCheckMemory() para validar el heap.  Si está corrupto:
//  1) loguea "HEAP_CORRUPT at <tag>" con el primer tag que falla,
//  2) dispara DebugBreak() para detener en VS2022 con stack limpia.
// El chequeo inmediatamente posterior al culpable es el que falla, así que el
// tag apunta al intervalo [prev_tag, this_tag].
extern "C" void ChkHeapPublic(const char* tag)
{
#ifdef _DEBUG
    if (!_CrtCheckMemory()) {
        char b[192];
        _snprintf_s(b, sizeof(b), _TRUNCATE, "HEAP_CORRUPT at %s", tag ? tag : "(null)");
        DbgLog(b);
        DebugBreak();
    }
#else
    (void)tag;
#endif
}

static LONG WINAPI DbgUnhandledException(EXCEPTION_POINTERS* ep)
{
    // 2026-04-29: anti-recursion guard. Si DbgLog crashea (e.g., WriteFile AV),
    // el filter se reentra → recursion infinita → otra AV en KernelBase.
    static __declspec(thread) int s_handlerDepth = 0;
    if (s_handlerDepth > 0) return EXCEPTION_CONTINUE_SEARCH;
    s_handlerDepth++;

    char line1[256], line2[256], full[600];
    __try {
        _snprintf_s(line1, sizeof(line1), _TRUNCATE,
            "CRASH: code=0x%08X  addr=0x%p",
            ep->ExceptionRecord->ExceptionCode,
            ep->ExceptionRecord->ExceptionAddress);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        line1[0] = 0;
    }
    line2[0] = 0;
    __try {
        if (ep && ep->ExceptionRecord && ep->ExceptionRecord->NumberParameters >= 2) {
            _snprintf_s(line2, sizeof(line2), _TRUNCATE,
                "param0=%u  param1=0x%p",
                (unsigned)ep->ExceptionRecord->ExceptionInformation[0],
                (void*)ep->ExceptionRecord->ExceptionInformation[1]);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { line2[0] = 0; }
    __try { DbgLog(line1); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (line2[0]) __try { DbgLog(line2); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    // ── Stack trace desde el contexto del crash ──────────────────────────────
    __try {
        if (ep && ep->ContextRecord) {
            CONTEXT* ctx = ep->ContextRecord;
            char trace[800];
            int n = 0;
            n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                "STACK_PC: 0x%p (esp=0x%p ebp=0x%p)",
                (void*)ctx->Eip, (void*)ctx->Esp, (void*)ctx->Ebp);
            DbgLog(trace);

            // Volcado de registros — los valores de los argumentos suelen ir en eax/ecx/edx
            _snprintf_s(trace, sizeof(trace), _TRUNCATE,
                "REGS: eax=%08X ecx=%08X edx=%08X ebx=%08X esi=%08X edi=%08X",
                (unsigned)ctx->Eax, (unsigned)ctx->Ecx, (unsigned)ctx->Edx,
                (unsigned)ctx->Ebx, (unsigned)ctx->Esi, (unsigned)ctx->Edi);
            DbgLog(trace);

            // ── Recorrido de frames por EBP (direcciones de retorno reales) ─────
            // Por cada frame: prev_ebp = *ebp, return_addr = *(ebp+4),
            // then ebp = prev_ebp.
            n = _snprintf_s(trace, sizeof(trace), _TRUNCATE, "STACK_FRAMES:");
            DWORD ebp = ctx->Ebp;
            DWORD prev_ebp_value = 0;
            for (int frame = 0; frame < 12 && n < (int)sizeof(trace) - 16; ++frame) {
                __try {
                    if (ebp < 0x10000 || ebp > 0x7FFFFFFF) break;
                    if (ebp & 3) break; // misaligned
                    DWORD ret = *(DWORD*)(ebp + 4);
                    DWORD next = *(DWORD*)(ebp);
                    if (next != 0 && next <= ebp) break; // sanity: ebp must grow upward
                    n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                        " 0x%X", ret);
                    prev_ebp_value = ebp;
                    ebp = next;
                    if (ebp == prev_ebp_value) break;
                } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            }
            DbgLog(trace);

            // ── Escaneo crudo del stack (250 dwords desde esp): atrapa frames FPO/sin-EBP ──
            // El loop emite hasta ~25 direcciones candidatas (filtradas al código del juego).
            int dumped = 0;
            n = _snprintf_s(trace, sizeof(trace), _TRUNCATE, "STACK_SCAN:");
            DWORD esp = ctx->Esp;
            for (int i = 0; i < 250 && n < (int)sizeof(trace) - 16 && dumped < 25; ++i) {
                __try {
                    DWORD val = *(DWORD*)(esp + i * 4);
                    if (val >= 0x401000 && val < 0x600000) {
                        n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                            " %X:%X", esp + i*4, val);
                        dumped++;
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            }
            DbgLog(trace);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    __try {
        _snprintf_s(full, sizeof(full), _TRUNCATE, "%s\n%s", line1, line2);
        MessageBoxA(NULL, full, "Crash", MB_OK | MB_ICONERROR);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    s_handlerDepth--;
    return EXCEPTION_EXECUTE_HANDLER;
}

extern int Config_Load(void);
extern int Config_ReadServerAddr(void* pConfig, char* lpCmdLine, char* outIP, unsigned short* outPort);
extern "C" void* ChatListBox_Construct(void);         // src/UI/ChatListBox.cpp
extern "C" void* ChatListBox_ConstructWhisper(void);  // src/UI/ChatListBox.cpp
extern "C" void  HUD_InitInventoryPools(void);        // src/Render/HUD_Pass3.cpp

// Silent CRT invalid_parameter handler — sin esto, snprintf con format
// inválido (NULL fmt, etc.) recursa infinitamente vía ucrtbased y crashea.
static void __cdecl SilentInvalidParameterHandler(
    const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{
    // No-op: deja que la función con argumento inválido retorne -1.
}

// Hook de reportes del CRT — captura los asserts de corrupción de heap y los escribe en debug.log.
// `_CRTDBG_CHECK_ALWAYS_DF` hace que cada alloc/free valide el heap; al detectar
// algo, el hook se dispara con un mensaje multilínea con los datos del bloque
// culpable (tamaño, tipo, dirección, archivo:línea del alloc original).
static int __cdecl HeapAssertReportHook(int reportType, char* msg, int* retVal)
{
    if (msg) {
        char buf[600];
        int n = 0;
        for (int i = 0; msg[i] && n < (int)sizeof(buf) - 1; ++i) {
            if (msg[i] == '\r' || msg[i] == '\n') {
                if (n > 0 && buf[n-1] != ' ') buf[n++] = ' ';
                continue;
            }
            buf[n++] = msg[i];
        }
        buf[n] = 0;
        char full[680];
        _snprintf_s(full, sizeof(full), _TRUNCATE,
            "*** CRT_HEAP type=%d: %s", reportType, buf);
        DbgLog(full);
    }
    if (retVal) *retVal = 0;   // continue, no debug break
    return TRUE;               // we handled it
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    // Todos los paths de datos del original son relativos (p.ej. Data\\Skill\\Fire01.bmd).
    // El binario reconstruido lo puede lanzar un debugger o una app con el
    // workspace como directorio actual, a diferencia del launcher retail. Anclamos
    // el proceso al directorio de main.exe antes de que corra cualquier loader de recursos.
    {
        char exePath[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
            char* slash = strrchr(exePath, '\\');
            if (slash) {
                *slash = '\0';
                SetCurrentDirectoryA(exePath);
            }
        }
    }
    _set_invalid_parameter_handler(SilentInvalidParameterHandler);
    SetUnhandledExceptionFilter(DbgUnhandledException);
#ifdef _DEBUG
    // DIAG: CRT debug heap con guard bytes. La corrupción se detecta en el
    // siguiente alloc/free que toque el bloque dañado — rápido sin O(n^2) de
    // CHECK_ALWAYS. Si el detector no lo atrapa a tiempo, subir a
    // _CRTDBG_CHECK_EVERY_1024_DF (valida todo el heap cada 1024 ops).
    {
        int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        flag |= _CRTDBG_ALLOC_MEM_DF;          // debug heap con guard bytes
        flag |= _CRTDBG_CHECK_EVERY_1024_DF;   // validate every 1024 ops
        // DELAY_FREE_MEM removed: el crash es en sbh_alloc_block (Small Block
        // Heap, capa abajo del debug heap CRT) — _CrtCheckMemory no valida sbh.
        // Mantenemos el debug heap básico para tener guard bytes en allocs
        // grandes; UAF en sbh requiere otro approach (Application Verifier).
        _CrtSetDbgFlag(flag);

        // Instala un hook de reportes del CRT para que los asserts de corrupción
        // de heap queden logueados en debug.log (visible sin debugger).
        _CrtSetReportHook(HeapAssertReportHook);
        // 2026-04-29: NO prompt on assert/error. _vsnprintf_s falla con
        // __invoke_invalid_parameter en CRT debug y popup aparece en cada call
        // bad. Dejamos solo log en Output Window — el SilentInvalidParameterHandler
        // se encarga de no recursar.
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_DEBUG);
        DbgLog("CRT heap debug: ALLOC_MEM + CHECK_EVERY_1024 habilitados (LEAK_CHECK off)");
    }
#endif
    DbgLog("== WinMain entry ==");
    g_hInst = hInst;

    // 1-5: anti-tamper init, world pre-init, integrity check
    // FUN_00406af0();
    // FUN_00510fe0();
    // MuExe_IntegrityCheck();
    // FUN_004065f0(local_3d4);

    // 6: Config_ReadServerAddr — lee server.cfg ("IP PORT" o "IP:PORT") y
    // sobreescribe el buffer al que apunta PTR_s_connect_muonline_co_kr_005615b8.
    // Si server.cfg no existe, se mantiene "connect.muonline.co.kr" (falla el connect).
    {
        extern char g_ServerIPBuf[128];
        unsigned short cfgPort = DAT_005615bc;
        if (Config_ReadServerAddr(NULL, lpCmdLine, g_ServerIPBuf, &cfgPort)) {
            DAT_005615b8 = g_ServerIPBuf;
            DAT_005615bc = cfgPort;
            DbgLog("server.cfg: overrode server IP/port");
        }
        {
            char b[160];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "CS-DIAG WinMain: g_HasConnectServer=%d line1=%s:%d gsIP=%s gsPort=%d",
                g_HasConnectServer, g_ServerIPBuf, (int)DAT_005615bc,
                g_GameServerIP, (int)g_GameServerPort);
            DbgLog(b);
        }
    }

    // 7-8: single instance check
    // HWND hExisting = FindWindowA("Dialog", windowTitle);
    // if (hExisting): SendMessageA(hExisting, WM_CLOSE, 0, 0);
    // App_SingleInstanceCheck();

    // 9: carga de claves CSimpleModulus — tiene que pasar ANTES de que se envíe
    // el primer paquete. Sin estas claves, el cuerpo de cada paquete C3/C4 va
    // encriptado con ceros, el server no lo puede desencriptar, y responde con
    // FD_CLOSE (el síntoma del diálogo vacío que vuelve al login de la corrida del 2026-04-24).
    extern BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD *self, const char *fn);
    extern BOOL __cdecl CSimpleModulus_LoadDecryptionKey(DWORD *self, const char *fn);
    DbgLog("before CSimpleModulus::LoadEncryptionKey");
    BOOL okEnc = CSimpleModulus_LoadEncryptionKey(DAT_05826c10, "Data\\Enc1.dat");
    BOOL okDec = CSimpleModulus_LoadDecryptionKey(DAT_05826c58, "Data\\Dec2.dat");
    {
        char b[256];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "CSimpleModulus load: Enc1=%d Dec2=%d", okEnc, okDec);
        DbgLog(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "POST-LOAD Dec2 mod=[%08X %08X %08X %08X] addr=%p",
                    DAT_05826c58[1], DAT_05826c58[2], DAT_05826c58[3], DAT_05826c58[4],
                    (void*)DAT_05826c58);
        DbgLog(b);
    }

    // 10: config
    DbgLog("before Config_Load");
    if (!Config_Load()) { DbgLog("Config_Load FAILED"); return 0; }
    DbgLog("after Config_Load");

    // 10b: localized string pool — Data/Local/Text.bmd → GlobalText[1000][300].
    // El WinMain original llama OpenTextData() antes del chequeo de versión/integridad
    // (ver el port de IDA en stubs.cpp:33019). Lo invocamos acá para que todo camino
    // de UI que lea GlobalText[N] (mensajes de error, confirmaciones, cuentas regresivas...)
    // datos reales en vez de vacíos.
    DbgLog("before OpenTextData");
    OpenTextData();
    DbgLog("after OpenTextData");

    // 11: modo de video — pone los valores por defecto si no está configurado
    if (DAT_0056156c == 0) DAT_0056156c = 640;
    if (DAT_00561570 == 0) DAT_00561570 = 480;
    // ChangeDisplaySettings completo omitido (por ahora corre en ventana)

    // 12-13: window + OpenGL
    DbgLog("before Window_Create");
    Window_Create(hInst);
    if (!g_hWnd) { DbgLog("Window_Create FAILED"); return 0; }
    DbgLog("after Window_Create");
    DbgLog("before OpenGL_Init");
    if (!OpenGL_Init()) { DbgLog("OpenGL_Init FAILED"); return 0; }
    DbgLog("after OpenGL_Init");

    // 14: GameGuard — HWND casteado a CHAR*
    GameGuard_Init((CHAR*)g_hWnd);

    // 14a: Compute hardware fingerprint (F1/05 SetHwid payload).
    // Lo exige el chequeo de blacklist del GameServer de MuEmu antes del login F1/01.
    HWID_Init();

    // 14b: DirectSound8 init — creates g_lpDS, primary buffer w/ DSBCAPS_CTRL3D,
    // QueryInterface IID_IDirectSound3DListener, SetFormat 22050/16/stereo,
    // y pone en cero todos los arrays por slot. Setea g_EnableSound=TRUE si tuvo éxito
    // (que es a lo que DAT_00590ac8 aliasa ahora vía macro).
    {
        HRESULT hrDS = InitDirectSound(g_hWnd);
        DbgLog(SUCCEEDED(hrDS) ? "InitDirectSound OK" : "InitDirectSound FAILED");
    }

    // 14c: STATIC SERVER LIST — workaround hasta que Net_ProcessPacket implemente
    // el dispatcher F1/02 (server list, 1824 líneas en el original). Sin esto,
    // DAT_083a45d8 queda en ceros y Scene_Login_ServerSelect no dibuja botones.
    //
    // Layout por entrada (stride 0x21e):
    //   +0x00  char[20]  name
    //   +0x14  byte      num_channels   (el PASS 3 saltea la entrada si es 0)
    //   +0x15  byte      pvp_flag       (auto-set en PASS 2 si name[...]=='2')
    //   Canales stride 0x1a a partir de +0x1c:
    //     +0x2c  ushort  channel_id     (display index = id%20+1)
    //     +0x2e  byte    load           (<100=LOW verde, 0x80+=FULL rojo, si no NORMAL)
    {
        char* srv0 = DAT_083a45d8;                           // entry index 0
        memset(srv0, 0, 0x21e);
        lstrcpynA(srv0, "MuServer", 20);                     // +0x00 name
        srv0[0x14] = 1;                                      // +0x14 num_channels
        *(unsigned short*)(srv0 + 0x2c) = 1;                 // channel[0] id → "1"
        // Load byte = ocupación del server (0..100). El original lo recibe por
        // red (ReceiveServerList @0x423E10: byte_83A4606 = ReceiveBuffera[2]) y
        // con 0 cuentas logeadas llega 0 → barra vacía. Como conectamos directo
        // al GameServer (sin flujo ConnectServer F4/02), usamos la lista estática
        // e inicializamos load=0 para replicar ese estado "server vacío". El
        // render (Scene_Login_ServerSelect Pass 4) dibuja load/5 cuadros llenos:
        // 0 → 0 llenos + 20 vacíos = barra vacía, igual que el binario original.
        srv0[0x2e] = 0;                                      // channel[0] load = 0 (vacío)
        DbgLog("Static server list populated: MuServer [1 ch]");
    }

    // 15: fuentes — el tamaño depende de la resolución
    {
        int fontSize = 0x0c;  // default for 640x480
        if (DAT_0056156c == 0x320) fontSize = 0x0d;       // 800
        else if (DAT_0056156c == 0x400) fontSize = 0x0e;   // 1024
        else if (DAT_0056156c >= 0x500) fontSize = 0x0f;   // 1280+
        DAT_07d78080 = fontSize;
        // CHARSET — DESVIACIÓN DELIBERADA del binario (2026-07-20).
        // Acá había 129 = HANGEUL_CHARSET, que es lo que usa el cliente coreano
        // original porque su Text.bmd es coreano.  El nuestro es ESPAÑOL en
        // Windows-1252, y con HANGEUL_CHARSET la GDI trata los bytes 0x81..0xFE
        // como lead-byte de una secuencia DBCS: se come el carácter siguiente.
        // Por eso se veía "da? o" (daño: 0xF1 + 'o' consumidos como par) y
        // "?xito" (éxito).  Afecta a medir Y a dibujar, así que también
        // desalineaba los recuadros de fondo.
        // En el original esto sale de `g_dwCharSet`, que el cliente elige según
        // el idioma; para datos en 1252 el equivalente es DEFAULT_CHARSET.
        const DWORD kCharSet = DEFAULT_CHARSET;   // era 129 (HANGEUL_CHARSET)
        DAT_055ca00c = (DWORD)CreateFontA(fontSize, 0, 0, 0, 400, 0, 0, 0, kCharSet, 0, 0, 0, 0, "Arial");
        DAT_055ca010 = (DWORD)CreateFontA(fontSize, 0, 0, 0, 700, 0, 0, 0, kCharSet, 0, 0, 0, 0, "Arial");
        DAT_055ca014 = (DWORD)CreateFontA(fontSize, 0, 0, 0, 700, 0, 0, 0, kCharSet, 0, 0, 0, 0, "Arial");
    }

    // 16: WSAStartup — obligatorio antes de cualquier operación de socket
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    // 17: GameGuard watchdog timer (20s period)
    SetTimer(g_hWnd, 1000, 20000, NULL);

    // 18: seed RNG
    srand((unsigned int)time(NULL));

    // 19: alocación de buffers (tamaños sacados del decompile)
    DAT_07cf5600 = (DWORD)malloc(900);     memset((void*)DAT_07cf5600, 0, 0xe1 * 4);
    DAT_07d29d20 = (int)malloc(0xa00);     memset((void*)DAT_07d29d20, 0, 0x280 * 4);
    DAT_07cf1ff8 = (int)malloc(0xa00);     memset((void*)DAT_07cf1ff8, 0, 0x280 * 4);

    // ItemAttribute — base aleatorizada (anti-tamper, pero igual necesitamos el alloc)
    DAT_055c9e44 = (DWORD)malloc(0x18000);
    DAT_07d78068 = DAT_055c9e44 + (rand() % 0x400) * 0x40;
    memset((void*)DAT_07d78068, 0, 0x2000 * 4);
    // 2026-05-08: puntero de respaldo que usan los helpers del tooltip para
    // recuperarse cuando algún escritor desconocido corrompe DAT_07d78068 a 0x1.
    // (g_ItemAttribute_Backup se declara más abajo a nivel de archivo con extern "C").
    g_ItemAttribute_Backup = (DWORD)DAT_07d78068;

    DAT_07cf1ff0 = (int)malloc(0x8000);
    memset((void*)DAT_07cf1ff0, 0, 0x2000 * 4);

    // Array de entidades — CRÍTICO, todo lo accede
    DAT_055c9e48 = (DWORD)malloc(0x764d4);
    DAT_07abf5d0 = DAT_055c9e48 + (rand() % 0x80) * 0x394;
    memset((void*)DAT_07abf5d0, 0, 0x166b5 * 4);

    // CharacterMachine / HashTable
    {
        void* ht = malloc(0x584);
        memset(ht, 0, 0x161 * 4);
        DAT_07cf1ffc = ht;
    }

    // 20-21: CharacterAttribute = CharacterMachine; Hero = entity array slot 0
    DAT_07cf1ff4 = DAT_07cf1ffc;
    // BUG-FIX 2026-05-01: tambien setear el global C++ `CharacterMachine`
    // (declarado en globals.cpp:2543 como nullptr separado). HUD_Pass5
    // (Render_HudPass_4BD650_) chequea `if (!CharacterMachine || !CharacterAttribute) return;`
    // Si solo seteamos DAT_07cf1ffc, el HUD nunca rendea HP/MP/skills.
    extern void* CharacterMachine;
    CharacterMachine = DAT_07cf1ffc;
    // FUN_0047d3d0(DAT_07cf1ffc);   // HashTable_Init — TODO: implement
    DAT_07abf5d8 = (char*)DAT_07abf5d0;  // player ptr = entity array slot 0

    // Contexto del pathfinder (DAT_05826df4).
    //
    // 2026-08-17: antes era `malloc(0x420)` + memset, que dejaba el vtable de la
    // cola de prioridad (+0x414) en NULL — por eso FUN_0043f500 (PATH::FindPath)
    // no se podia usar. Ahora se construye igual que el binario
    // (0x0043F280..0x0043F2C7: reserva de 0x424 bytes, vtable y campos en cero).
    //
    // InitPath (0x0043F2D0) NO se llama aca: ya estaba portada en
    // stubs_externs.cpp y la llama FUN_0050f690 (World_Init) desde Scene_Intro,
    // igual que en el binario. Este ctor corre antes, que es el orden correcto.
    extern void __cdecl PathContext_Create(void);   // src/Game/PathFinder.cpp
    PathContext_Create();

    // 22: vtable object construction (font/UI system objects)
    // 2026-04-29: antes se había identificado DAT_055c9ff0 como HGLRC y se
    // inicializaba mal, como un buffer en cero; IDA muestra que es el objeto del
    // motor del chat-listbox, que construye sub_40C7D0 (ctor) sobre
    // operator_new(0x5C8). Sin el ctor la vtable queda nula y todos los
    // sitios de dispatch (Render_GameFrame +0x10, UIChatLogWindow_AddText +0x70,
    // CreateChat +0x70) crasheaban en silencio — por eso nunca aparecía el HUD
    // in-world. ChatListBox_Construct portea sub_40C5D0+sub_40C7D0 1:1.
    DAT_055c9ff0 = (DWORD)ChatListBox_Construct();
    // 2026-04-30: DAT_055c9ff4 es el segundo widget de chat (susurro / panel de
    // notificaciones, esquina superior derecha). El WinMain de IDA llama
    // operator_new(0xBC) + sub_40E990 (hermana de sub_40C7D0, con nodo de lista
    // más chico, 0x18, y 24 filas visibles). Sin construirlo bien la vtable
    // quedaba nula → el dispatch vtable[+0x14] desde FUN_004e4760 crasheaba leyendo 0x14.
    DAT_055c9ff4 = (DWORD)ChatListBox_ConstructWhisper();

    // 2026-04-30: los slots vacíos del inventario tienen que tener Type=0xFFFF, no 0.
    // Los scanners de grilla del motor (FUN_00482be0/sub_482850/sub_482E40) toman el 0
    // como un tipo de arma válido y matchean celdas vacías por error, lo que hace
    // que RenderItem3D se invoque sobre basura → el bug de render del triángulo cyan.
    HUD_InitInventoryPools();
    DAT_055c9ff8 = (DWORD)malloc(0xc);   memset((void*)DAT_055c9ff8, 0, 0xc);

    // Fallback de la tabla de modelos: FUN_00506170 (Model_LoadItems) sólo corre
    // durante la carga del mapa (state=5). Los spawns de entidades de la escena de
    // login deferencian DAT_05828d58 + etype*0xbc + offset y crashean si es NULL.
    // Alocamos un fallback en cero, grande como para etype hasta ~0x300.
    if (DAT_05828d58 == 0) {
        size_t modelPoolSz = 0x5b334;
        DAT_06f42a58 = malloc(modelPoolSz);
        memset(DAT_06f42a58, 0, modelPoolSz);
        DAT_05828d58 = (DWORD)DAT_06f42a58;
    }

    // Class name table (DAT_07d2b494, stride 300, indexed por cidx = (klass&7) +
    // 4*(klass>>3)). En el original vive en GlobalText[20..] (cargado del .bmd de
    // texto); acá lo poblamos directo con los nombres para el create-panel
    // (RenderText 285,+200 lee `DAT_07d2b494 + class*300`) y para consistencia con
    // el char-list. 2026-07-17.
    {
        extern char DAT_07d2b494[9000];
        static const char* kCN[7] = {
            "Dark Wizard", "Dark Knight", "Fairy Elf", "Magic Gladiator",
            "Soul Master", "Blade Knight", "Muse Elf"
        };
        for (int i = 0; i < 7; ++i)
            lstrcpynA(DAT_07d2b494 + i * 300, kCN[i], 300);
    }

    // 23: mouse speed
    // SystemParametersInfoA(0x61, 1, NULL, 0);  // SPI_SETMOUSESPEED

    // 24: Set initial game state → Intro (Webzen logo)
    DbgLog("allocs done, entering message loop");
    g_GameState = 1;  // DAT_005615c0: 1=Intro, 2=Login, 3=Loading, 4=CharSelect, 5=InGame

    // ── MESSAGE LOOP ─────────────────────────────────────────────────────────
    // PeekMessage (non-blocking): cuando no hay mensajes → Scene_Dispatch
    MSG msg = {};
    for (;;)
    {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessageA(&msg, NULL, 0, 0))
                break; // WM_QUIT

            // WM_SYSKEYDOWN/UP no pasan a DispatchMessage (para evitar Alt+key del sistema)
            if (msg.message != WM_SYSKEYDOWN && msg.message != WM_SYSKEYUP) {
                // TranslateMessage convierte WM_KEYDOWN → WM_CHAR. Sin esto
                // el WndProc nunca recibe los caracteres tipeados — faltaba
                // y por eso el input de login/chat parecía "muerto".
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        else
        {
            // ── CLICK-FLAG CLAMP (global) ───────────────────────────────────
            // DAT_083a4124 (MouseLButtonPush) sólo debe valer {0,1} (lo
            // settea WndProc en LBUTTONDOWN/UP).  En char-select aparecía
            // 0x4397355A cada frame → phantom clicks en TODAS las escenas
            // (login/server-select/char-select/in-game) que lo leen como
            // truthy.  Algún stub stompea via buffer overflow; mientras
            // localizo al culpable, este clamp suprime el wild value antes
            // del frame.  Misma protección para DAT_083a4299 (DBLCLK flag).
            if (DAT_083a4124 != 0 && DAT_083a4124 != 1) {
                static unsigned s_corruptCnt = 0;
                if (s_corruptCnt < 30) {
                    s_corruptCnt++;
                    char dbg[120];
                    wsprintfA(dbg, "CLK-CLAMP: DAT_083a4124=0x%08X t=%u",
                        (unsigned)DAT_083a4124, GetTickCount());
                    DbgLogPublic(dbg);
                }
                DAT_083a4124 = 0;
            }
            if ((BYTE)DAT_083a4299 != 0 && (BYTE)DAT_083a4299 != 1) {
                DAT_083a4299 = 0;
            }
            // No hay mensajes pendientes: ejecutar frame
            Scene_Dispatch(g_hDC);
        }
    }

    DbgLog("message loop exit");

    // ── Cola de DestroyWindow (0x004145C0) ──────────────────────────────────
    // El binario cierra el reproductor externo al salir; sin esto MuPlayer.exe
    // queda vivo reproduciendo despues de cerrar el cliente.
    //   WindowA = FindWindowA(0, "MuPlayer");
    //   if (WindowA) SendMessageA(WindowA, 2 /*WM_CLOSE*/, 0, 0);
    // El resto de DestroyWindow (liberar fonts, modelos, texturas, el hash de
    // SkillAttribute/CharacterMachine) es cleanup de memoria previo a terminar
    // el proceso y no esta portado.
    {
        HWND hMuPlayer = FindWindowA(NULL, "MuPlayer");
        if (hMuPlayer) {
            SendMessageA(hMuPlayer, WM_CLOSE, 0, 0);
            DbgLog("MuPlayer cerrado");
        }
    }

    return (int)msg.wParam;
}

// ── WndProc @ 0x004149D0 (4074 líneas, ~80% anti-tamper) ────────────────────
//
// __stdcall 4 params (RET 0x10). Todas las rutas terminan en DefWindowProcA.
//
// ── DISPATCH PASS 1 (mensajes 0x02..0x113+) ─────────────────────────────────
//
//   uMsg == 0x113 → WM_TIMER @ 0x00414db9 (antes del jump table)
//   uMsg in [0x2..0x20] → jump table 1 @ 0x41DD55:
//     WM_DESTROY(02), WM_ACTIVATE(06), WM_CLOSE(10), WM_SETCURSOR(20), otros
//   uMsg > 0x113 → checks adicionales:
//     uMsg == WM_USER (0x400) → sub-dispatch por wParam:
//       wParam==1:    Net_Recv()                — datos listos (WSAAsyncSelect FD_READ)
//       wParam==2:    Net_Send()                — buffer libre (FD_WRITE)
//       wParam==0x20: Screenshot() + Net_Connect() — socket conectado (FD_CONNECT)
//     uMsg == WM_USER+1 (0x401) → OpenGL_Release()   — shutdown message
//     uMsg == 0x3111            → envía paquete 0xC1/0xF1 (re-auth)
//     default → DefWindowProcA
//
// ── DISPATCH PASS 2 (mouse/IME, desde dentro del body de WM_USER) ───────────
//
//   Recarga uMsg @ 0x0041971a y despacha de nuevo:
//   uMsg == WM_LBUTTONUP  (0x202) @ 0x00419bf4 — suelta click
//   uMsg == WM_IME_COMPOSITION (0x10F) @ 0x0041a7bd — Korean DBCS chat input:
//     DAT_07e11cec[DAT_07e11d78 * 4]   = high byte
//     DAT_07e11ced[DAT_07e11d78 * 4]   = low byte
//     DAT_07e11cee[DAT_07e11d78 * 4]   = 0 (null terminator)
//   uMsg == WM_MOUSEMOVE  (0x200) @ 0x00419816 — normaliza coords a 640×480:
//     g_MouseX = LOWORD(lParam) * 640 / g_ScreenW  → DAT_083a427c
//     g_MouseY = HIWORD(lParam) * 480 / g_ScreenH  → DAT_083a4278
//   uMsg == WM_LBUTTONDOWN (0x201) @ 0x00419859
//   uMsg - 0x203 in [0..0x7F] → jump table 2 @ 0x41DD88:
//     WM_RBUTTONDOWN(204), WM_RBUTTONUP(205), WM_MOUSEWHEEL(20A), etc.
//
// ── TECLADO ─────────────────────────────────────────────────────────────────
//   WM_KEYDOWN/WM_KEYUP/WM_CHAR: NO manejados → DefWindowProcA
//   Movimiento/acción: GetAsyncKeyState() desde Game_SceneUpdate (polled)
//   WM_IME_COMPOSITION (0x10F): manejado (chat coreano)
//   IME toggle: IME_SetConversion @ 0x0047ED80 — ImmSetConversionStatus wrapper
//
// ── WM_TIMER ─────────────────────────────────────────────────────────────────
//   id=1000 (period=20000ms):
//     → GameGuard_TickCheck() — comprueba que el proceso GG sigue vivo
//     si DAT_05826cf0 != 0 (conectado): envía keep-alive 0xC1/0x0E
//
// ── GLOBALS CLAVE ────────────────────────────────────────────────────────────
//   DAT_083a427c = g_MouseX  (0..639, normalizado 640×480)
//   DAT_083a4278 = g_MouseY  (0..479)
//   DAT_083a413c = mouse-moved flag
//   DAT_083a4299 = flag de ventana activa (0 al desactivarse)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_ACTIVATE:
        // DAT_083a4299 = (LOWORD(wParam) != WA_INACTIVE) ? 1 : 0;
        break;

    case WM_SETCURSOR:
        // SetCursor(NULL) in-game para ocultar cursor del sistema.
        // Sin esto el cursor de Windows tapa al sprite de cursor del MU.
        SetCursor(NULL);
        return TRUE;  // we handled it; prevent DefWindowProc from setting arrow

    case WM_TIMER:
        if (wParam == 1000)
            GameGuard_TickCheck();
        break;

    case WM_USER:          // 0x400 — WSAAsyncSelect events
    {
        // MSDN: wParam = socket handle, LOWORD(lParam) = event code.
        // El código anterior hacía el switch sobre wParam, que está mal; va lParam.event.
        WORD evt = LOWORD(lParam);
        WORD err = HIWORD(lParam);
        {
            char dbg[96];
            wsprintfA(dbg, "NET: WM_USER wParam=0x%X lParam=0x%X evt=0x%X err=%u",
                      (unsigned)wParam, (unsigned)lParam, (unsigned)evt, (unsigned)err);
            DbgLog(dbg);
        }
        // Compatibilidad MuEmu: tras F4/03 redirect podemos reconectar a un
        // socket nuevo antes de que Windows entregue el FD_CLOSE del socket
        // viejo. Si procesamos ese evento tardío como si fuera del socket
        // actual, cerramos la sesión nueva inmediatamente.
        extern int  __fastcall FUN_0043de70(void* ctx);
        extern int  __fastcall FUN_0043ddd0(int  ctx);
        extern void Net_ProcessPacket(void);
        if (evt & 0x01) { // FD_READ
            FUN_0043de70((void*)(uintptr_t)DAT_055ca160);
            CsmWatchdog("after-Recv");        // catches any future trample regression
            Net_ProcessPacket();
        }
        if (evt & 0x02) { // FD_WRITE
            FUN_0043ddd0((int)(uintptr_t)DAT_055ca160);
            // Robustez: la conexión no-bloqueante completa suele señalizarse con
            // el primer FD_WRITE (el mask WSAAsyncSelect es 0x23, sin FD_CONNECT).
            // Mandamos el request de lista al ConnectServer acá también; el guard
            // g_ConnectServerRequested asegura que salga una sola vez.
            if (g_ConnectServerMode && !g_ConnectServerRequested) {
                extern void CS_SendPlain(const BYTE* data, int len);
                BYTE req[4] = { 0xC1, 0x04, 0xF4, 0x02 };
                CS_SendPlain(req, 4);
                g_ConnectServerRequested = 1;
                DbgLog("NET: CS mode (FD_WRITE) → sent C1 04 F4 02 (server-list request)");
            }
        }
        if (evt & 0x10) { // FD_CONNECT
            DbgLog("NET: FD_CONNECT fired (socket ready for I/O)");
            extern void CS_SendPlain(const BYTE* data, int len);
            extern void FUN_00423920(const char* server, unsigned int port);
            if (err == 0 && g_ConnectServerMode && !g_ConnectServerRequested) {
                // Conectados al ConnectServer → pedir la lista de servers.
                // C1 04 F4 02 (PMSG_SERVER_LIST_RECV) — plano, sin encriptar.
                BYTE req[4] = { 0xC1, 0x04, 0xF4, 0x02 };
                CS_SendPlain(req, 4);
                g_ConnectServerRequested = 1;
                DbgLog("NET: CS mode → sent C1 04 F4 02 (server-list request)");
            } else if (err != 0 && g_ConnectServerMode &&
                       g_HasConnectServer && g_GameServerPort != 0) {
                // El ConnectServer no respondió → fallback: GameServer directo.
                DbgLog("NET: ConnectServer unreachable → fallback to GameServer");
                g_ConnectServerMode      = 0;
                g_ConnectServerRequested = 0;
                MuEmu::SetActive(true);   // GameServer sí usa la encriptación MuEmu
                if (DAT_055ca168 != 0xffffffff) {
                    closesocket((SOCKET)DAT_055ca168);
                    DAT_055ca168 = (DWORD)INVALID_SOCKET;
                }
                FUN_00423920(g_GameServerIP, g_GameServerPort);
            }
        }
        if (evt & 0x20) { // FD_CLOSE
            DbgLog("NET: FD_CLOSE fired (remote closed socket)");
            // Fallback ConnectServer: si seguimos en modo CS cuando llega el
            // FD_CLOSE, la conexión al ConnectServer se cayó ANTES del redirect
            // (connection refused, CS caído, o cierre inesperado). Con el mask
            // 0x23 (sin FD_CONNECT) un "refused" llega por acá. Reconectamos al
            // GameServer directo para no dejar al usuario sin poder loguear.
            // (Tras un redirect exitoso Recv_Redirect ya puso mode=0, así que
            // este FD_CLOSE del socket viejo NO dispara el fallback.)
            if (g_ConnectServerMode && g_HasConnectServer && g_GameServerPort != 0) {
                extern void FUN_00423920(const char* server, unsigned int port);
                DbgLog("NET: CS dropped before redirect → fallback to GameServer");
                g_ConnectServerMode      = 0;
                g_ConnectServerRequested = 0;
                MuEmu::SetActive(true);   // GameServer sí usa la encriptación MuEmu
                if (DAT_055ca168 != 0xffffffff) {
                    closesocket((SOCKET)DAT_055ca168);
                    DAT_055ca168 = (DWORD)INVALID_SOCKET;
                }
                FUN_00423920(g_GameServerIP, g_GameServerPort);
                break;   // no mostrar "conexión cerrada"; estamos reconectando
            }
            // IDA WndProc @ 0x004149D0 case FD_CLOSE (original behaviour):
            //   UIChatLogWindow_AddText(strID, GlobalText[3], 1);
            //   CWsctlc::Close(&SocketClient);
            // ── No MessageBoxA, no PostQuitMessage. Solo log y cierre local.
            // El cierre del socket por logout limpio (F1/02/* iniciado por el
            // cliente) NO debe matar el proceso — el countdown de "Salir" o el
            // local-transition de "Seleccionar Servidor" se encargan de la
            // siguiente fase. Para FD_CLOSE no solicitado (server kick / red
            // caída) la UI de login mostrará el chat-log y el usuario verá
            // "Conexión cerrada" sin que el cliente se mate solo.
            extern void UIChatLogWindow_AddText(const char* strID, const char* msg, int color);
            UIChatLogWindow_AddText((const char*)&DAT_083a7c5c, GlobalText[3], 1);
            if (DAT_055ca168 != 0xffffffff) {
                closesocket((SOCKET)DAT_055ca168);
                DAT_055ca168 = (DWORD)INVALID_SOCKET;
            }
        }
    }
        // Pass 2: mouse/IME dispatch desde aquí (Ghidra lo muestra como fall-through)
        break;

    case WM_USER + 1:      // 0x401 — shutdown
        OpenGL_Release();
        break;

    case WM_MOUSEMOVE:     // 0x200
        // g_MouseX = LOWORD(lParam) * 640 / g_ScreenW  → DAT_083a427c
        // g_MouseY = HIWORD(lParam) * 480 / g_ScreenH  → DAT_083a4278
        {
            DWORD sw = DAT_0056156c ? DAT_0056156c : 640;
            DWORD sh = DAT_00561570 ? DAT_00561570 : 480;
            DAT_083a427c = ((DWORD)(short)LOWORD(lParam) * 640) / sw;
            DAT_083a4278 = ((DWORD)(short)HIWORD(lParam) * 480) / sh;
        }
        break;

    case WM_LBUTTONDOWN:   // 0x201
        {
            char dbg[96];
            wsprintfA(dbg, "WM_LBUTTONDOWN @ (%d,%d) c4_pending=%d t=%u",
                (int)DAT_083a427c, (int)DAT_083a4278,
                (int)DAT_083a42c4, GetTickCount());
            DbgLogPublic(dbg);
        }
        DAT_083a413c = 0;                    // cancelar "dialog close" anterior
        if (DAT_083a42c4 == 0) {
            DAT_083a4124 = 1;                // g_ClickFlag = 1
        }
        DAT_083a42c4 = 1;                    // g_DblClickPending = 1
        DAT_055ca03c = DAT_083a427c;         // snapshot posición al down
        DAT_055ca040 = DAT_083a4278;
        break;

    case WM_LBUTTONUP:     // 0x202
        {
            char dbg[96];
            wsprintfA(dbg, "WM_LBUTTONUP   @ (%d,%d) t=%u",
                (int)DAT_083a427c, (int)DAT_083a4278, GetTickCount());
            DbgLogPublic(dbg);
        }
        DAT_083a4124 = 0;                    // g_ClickFlag = 0
        // Si no hubo drag desde el down, marcar "dialog close / click confirmado"
        if (DAT_055ca03c == DAT_083a427c && DAT_055ca040 == DAT_083a4278) {
            DAT_083a413c = 1;
        } else {
            DAT_083a413c = 0;                // fue drag
        }
        DAT_083a42c4 = 0;                    // clear pending
        DAT_055ca03c = DAT_083a427c;
        DAT_055ca040 = DAT_083a4278;
        break;

    case WM_LBUTTONDBLCLK: // 0x203
        {
            char dbg[96];
            wsprintfA(dbg, "WM_LBUTTONDBLCLK @ (%d,%d) t=%u",
                (int)DAT_083a427c, (int)DAT_083a4278, GetTickCount());
            DbgLogPublic(dbg);
        }
        DAT_083a4299 = 1;                    // g_DblClickFlag
        break;

    case WM_RBUTTONDOWN:   // 0x204
        // 004149D0 WndProc: MouseRButtonPop = 0; if (!MouseRButton)
        // MouseRButtonPush = 1; MouseRButton = 1.  Attack (0049CBF0)
    // consume esos dos flags para arrancar y sostener el casteo de un skill.
        if (DAT_083a42ac == 0) {
            DAT_083a42d0 = 1;                // MouseRButtonPush
        }
        DAT_083a42ac = 1;                    // MouseRButton
        {
            char dbg[128];
            wsprintfA(dbg, "INPUT RMB down @ (%d,%d) push=%u held=%u",
                (int)DAT_083a427c, (int)DAT_083a4278,
                (unsigned)DAT_083a42d0, (unsigned)DAT_083a42ac);
            DbgLogPublic(dbg);
        }
        break;

    case WM_RBUTTONUP:     // 0x205
        // El original limpia Push y suelta MouseRButton (también setea el flag
        // Pop aparte, que hoy ningún camino de gameplay compilado lee).
        DAT_083a42d0 = 0;
        DAT_083a42ac = 0;
        DbgLogPublic("INPUT RMB up");
        break;

    case WM_MOUSEWHEEL:    // 0x20A
        break;

    // 0x10F = WM_IME_COMPOSITION — chat coreano DBCS
    //   DAT_07e11cec[DAT_07e11d78*4] = HIWORD(wParam) >> 8
    //   DAT_07e11ced[DAT_07e11d78*4] = LOWORD(wParam) & 0xFF
    //   toggle de IME vía IME_SetConversion @ 0x0047ED80
    case WM_IME_STARTCOMPOSITION:  // 0x10D
        DAT_055ca019 = 1;
        break;
    case WM_IME_ENDCOMPOSITION:    // 0x10E
        DAT_055ca019 = 0;
        break;

    // WM_CHAR — entrada de texto para usuario/contraseña/chat/zen/guild.
    // Portado del WndProc de IDA @ 0x004149D0 líneas 1890-2010.
    // Antes no se manejaba (→ DefWindowProcA), lo que descartaba cada tecla
    // en la pantalla de login, así que no se podía tipear usuario/contraseña.
    //
    // Globals mapping (IDA → ours):
    //   InputIndex            → DAT_07e11d78   (active slot 0..9)
    //   InputLength[i]        → ((DWORD*)DAT_07d780a8)[i]
    //   InputText[i][j]       → DAT_07db8710 + i*0x100 + j
    //   InputEnable           → DAT_00559c84
    //   InputNumber           → DAT_00559c88
    //   InputTextMax[0]       → _DAT_00559c94 (alias floatizado del DWORD 0x559c94)
    //   byte_55CA019 (IME)    → DAT_055ca019
    //   byte_55CA038 (Enter)  → DAT_055ca038   (lo lee el disparador de login de Game_SceneUpdate)
    case WM_CHAR:          // 0x102
    {
        // Diagnóstico: confirma que WM_CHAR está llegando a la ventana y loguea
        // el estado de los flags del subsistema de input, para poder distinguir
        // "la tecla se perdió" de "la tecla se ignoró".
        {
            char dbg[128];
            // 2026-05-04: leer MaxLen como el int crudo de _InputTextMaxArr[slot]
            // — `(int)_DAT_00559c94` castearía el VALOR float (siempre 0 para
            // patrones de bits de enteros chicos), en vez de reinterpretar los bits.
            wsprintfA(dbg,
                "WM_CHAR: wParam=0x%02X slot=%u len=%u InputEnable=%u InputNumber=%u MaxLen=%d",
                (unsigned)wParam,
                (unsigned)DAT_07e11d78,
                (unsigned)((DWORD*)DAT_07d780a8)[DAT_07e11d78 & 0x0F],
                (unsigned)DAT_00559c84,
                (unsigned)DAT_00559c88,
                _InputTextMaxArr[DAT_07e11d78 & 0x0F]);
            DbgLog(dbg);
        }

        // Clampea el slot a [0,9] para evitar OOB si el índice por slot es basura.
        DWORD  slot = DAT_07e11d78 & 0x0F;
        if (slot > 9) slot = 0;
        DWORD* lens = (DWORD*)DAT_07d780a8;
        int    len  = (int)lens[slot];
        char*  buf  = (char*)DAT_07db8710 + slot * 0x100;

        if (!DAT_055ca019) {
            // Limpia el buffer de composición del IME para este slot (stride 4)
            *((char*)&DAT_07e11cec + slot * 4) = 0;
        }

        if (wParam == 8) {           // Backspace
            if (len > 0) {
                lens[slot] = (DWORD)(len - 1);
                buf[len - 1] = 0;
            }
            break;
        }
        if (wParam == 9) {           // Tab — rotate active slot
            if (DAT_00559c84 && DAT_00559c88 > 1) {
                DAT_07e11d78 = (DAT_07e11d78 + 1) % DAT_00559c88;
                FUN_00404bc0(0x19, 0, 0);   // PlayBuffer(25) — click sfx
            }
            break;
        }
        if (wParam == 0xD) {         // Enter — signals login/submit + chat toggle
            DAT_055ca038 = 1;

            // 2026-08-08 (baúl: guardar/sacar zen). Per IDA WndProc L2047-2051:
            //   InputIndex = 0;
            //   if (GoldInputEnable) { InputGold = atoi(InputText[0]); ... }
            // Sin esto InputGold quedaba siempre en 0 y el diálogo de zen del
            // baúl no tenía forma de saber cuánto tecleó el jugador.
            // GoldInputEnable = DAT_07e11d72, InputGold = DAT_07e11d74.
            if (DAT_07e11d72) {
                DAT_07e11d78 = 0;                     // InputIndex = 0
                char* goldBuf = (char*)DAT_07db8710;  // InputText[0]
                goldBuf[0xFF] = 0;
                DAT_07e11d74 = atoi(goldBuf);
                // LABEL_591: limpia el slot y cierra el input (el envío lo hace
                // UI_InGameMenu case 116 leyendo DAT_055ca038 el frame siguiente).
                memset(goldBuf, 0, 0x100);
                lens[0] = 0;
                DAT_00559c84 = 0;                     // InputEnable = 0
                break;
            }

            // 2026-05-04: toggle del chat in-game (per IDA WndProc:2011-2046).
            //   - state=5 (in-world)
            //   - input vacío + InputEnable=0  → abre el chat (InputEnable=1)
            //   - input vacío + InputEnable=1  → cierra el chat (InputEnable=0)
            //   - input con texto + InputEnable=1  → envía el chat y cierra
            //
            // NOTA: el gateo por ErrorMessage (126/152/139/142/140) y los demás
            // estados de diálogo no están portados del todo — lo relajamos al estado de gameplay.
            if (DAT_005615c0 == 5) {
                bool empty = (lens[slot] == 0);
                if (empty) {
                    if (DAT_00559c84) {
                        // Cierra el input de chat.
                        DAT_00559c84 = 0;
                    } else {
                        // Abre el input de chat — inicializa los campos de texto.
                        // Per IDA Game.cpp:4326-4327: InputTextMax[0]=42, [1]=10.
                        _InputTextMaxArr[0] = 42;               // chat msg max
                        _InputTextMaxArr[1] = 10;               // whisper target name max
                        // InputTextHide[0] = 0 — no modelado (sólo para contraseña)
                        DAT_00559c88 = 2;                       // InputNumber = 2 (chat + whisper target)
                        // GoldInputEnable = 0 — ya está en 0 en el juego normal
                        DAT_00559c84 = 1;                       // InputEnable = 1
                        DAT_07e11d78 = 0;                       // InputIndex = 0
                    }
                    FUN_00404bc0(0x19, 0, 0);   // click sfx
                } else if (DAT_00559c84) {
                    // Con texto + chat abierto → envía la línea (slot 0), limpia y
                    // cierra. Refleja las líneas 2046-2120 del WndProc de IDA condensadas
                    // en un solo camino (el original de IDA separa /whisper +
                    // /pvp + comandos de GM, pero por ahora los reenviamos al
                    // loop de canales de Chat_InputTick; el envío crudo anda
                    // bien para el caso del chat normal).
                    char* line = (char*)DAT_07db8710 + slot * 0x100;
                    Chat_SendChatLine(line);

                    // Limpia el slot de input y su longitud.
                    memset(line, 0, 0x100);
                    lens[slot] = 0;
                    // Cierra el chat (coincide con el comportamiento "enviar + autocerrar";
                    // la queja del usuario era que Enter-con-texto NO
                    // close).
                    DAT_00559c84 = 0;
                    FUN_00404bc0(0x19, 0, 0);   // click sfx
                }
            }
            break;
        }
        // Carácter imprimible — se agrega si hay algún modo de input activo.
        // El gate de InputEnable cubre login + chat; los otros flags (Tab/Gold/Guild)
        // son diálogos in-game que todavía no cableamos, pero alcanza para
        // que la pantalla de login acepte tipeo.
        // 2026-08-08: el diálogo de zen del baúl abre con InputEnable=0 +
        // GoldInputEnable=1 (sub_4EB5D0), así que con el gate viejo (sólo
        // InputEnable) NO se podía tipear nada. Per IDA WndProc L1953-1963 el
        // gate es `InputEnable || GoldInputEnable || …` y con GoldInputEnable
        // sólo se aceptan dígitos '0'..'9'.
        if (DAT_00559c84 || DAT_07e11d72) {
            BYTE c = (BYTE)wParam;
            if (DAT_07e11d72 && (c < '0' || c > '9')) break;
            // RANGO ACEPTADO — DESVIACIÓN DELIBERADA, hermana del charset de
            // CreateFontA (ver WinMain paso 15).
            // Acá había `c < 0x7F`, o sea ASCII puro: por eso no se podía
            // escribir 'ñ' (0xF1 en Windows-1252) ni vocales acentuadas.
            // El original acepta 0x20..0x7E y manda los bytes >= 0x81 por el
            // camino DBCS (lead byte + trail byte), porque su entrada es
            // coreana.  Nuestro texto es Latin-1 de un solo byte, así que ese
            // camino los destrozaría igual que HANGEUL_CHARSET destrozaba el
            // render.  Los aceptamos como carácter simple.
            // 0x7F es DEL: se sigue descartando.
            if (c >= 0x20 && c != 0x7F) {
                // InputTextMax[8] @ 0x00559c94 — leído como int desde el array
                // tipado (NO vía la referencia aliasada a float, que castea el valor).
                int maxLen = _InputTextMaxArr[slot];
                if (maxLen <= 0) maxLen = 10;  // login-default fallback
                if (len < maxLen) {
                    buf[len]   = (char)c;
                    lens[slot] = (DWORD)(len + 1);
                }
            }
        }
        break;
    }

    default:
        break;
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

// ── Window_FatalError @ 0x004129F0 (~15 líneas, completo) ───────────────────
//
// Llamado desde GameGuard_TickCheck cuando GG muere o su canal de eventos falla.
//   1. DAT_055ca020 = 1                    — g_FatalError flag
//   2. crt_sprintf(local_64, DAT_07d29e50) — formatea mensaje de error (buf 100 bytes)
//   3. MessageBoxA(g_hWnd, local_64, "IError", MB_OK)
//   4. SendMessageA(g_hWnd, WM_DESTROY, 0, 0)
//   5. si param_2 != 0:
//        SendMessageA(param_1, 0x2B1A, 0, 0)  — custom GG shutdown msg
//        SendMessageA(param_1, WM_CLOSE, 0, 0)
//
// Patch bypass: DAT_083bbb14=NULL + DAT_005538a0=0
//   → GameGuard_GetStatus() retorna 0 == DAT_005538a0 → Window_FatalError nunca llamada
// ─────────────────────────────────────────────────────────────────────────────
void Window_FatalError(HWND param_1, char param_2)
{
    CHAR local_64[100];
    // DAT_055ca020 = 1;
    // crt_sprintf(local_64, (byte*)&DAT_07d29e50);
    // MessageBoxA(g_hWnd, local_64, "IError", MB_OK);
    // SendMessageA(g_hWnd, WM_DESTROY, 0, 0);
    if (param_2 != '\0') {
        // SendMessageA(param_1, 0x2B1A, 0, 0);
        // SendMessageA(param_1, WM_CLOSE, 0, 0);
    }
    (void)local_64;
    (void)param_1;
    (void)param_2;
}
