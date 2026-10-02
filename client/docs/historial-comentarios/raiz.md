# Historial de comentarios: `src/ (raíz)`

Comentarios de desarrollo movidos desde `src/ (raíz)` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/WinMain.cpp`

### Línea 39 — antes de `extern "C" void Chat_SendChatLine(const char* text);`

```cpp
// 2026-05-04: helper de src/UI/Chat_InputTick.cpp — envía una línea de chat
// escrita en InputText[0] (DAT_07db8710 slot 0) por WM_CHAR; la llama el
// handler de Enter en WndProc cuando InputEnable=1 y el buffer no está vacío.
```

### Línea 58 — antes de `static bool Chat_TryAssignMacro(const char* text)`

```cpp
// Chat_TryAssignMacro -- "/1 texto" guarda una macro en la tecla 1.
//
// DESVIACION DOCUMENTADA (2026-09-24).  El 0.97k NO tiene esto: sus macros
// salen unicamente de Data\\Macro.txt (OpenMacro 0x50F750) y no hay una sola
// escritura al array fuera de ese loader -- verificado con los xrefs de
// 0x07E0FFC8.  La asignacion por chat aparece recien en MU 5.2
// (ZzzInterface.cpp, CheckCommand): compara los dos primeros caracteres contra
// "/1".."/9" y "/0", y copia el texto desde el indice 3.
//
// Se porta de ahi, con dos diferencias deliberadas:
//   - 5.2 termina el slot con `MacroText[i][iTextSize-3] = NULL` donde
//     iTextSize quedo en el ULTIMO indice recorrido, asi que se come el ultimo
//     caracter del mensaje.  Aca se copia entero.
//   - la lista de comandos prohibidos (CheckMacroLimit) se compara contra los
//     literales, no contra GlobalText: los indices de esa tabla son los de 5.2
//     y no tienen por que coincidir con los del 0.97k.
//
// Devuelve true si la linea era una asignacion (y entonces no se envia).
```

### Línea 121 — antes de `static void Display_RestoreIfChanged(void)`

```cpp
// Devuelve el escritorio a su modo original, si fuimos nosotros los que lo
// cambiamos.  Tiene que ser idempotente y llamable desde un filtro de
// excepciones: la llama OpenGL_Release en el cierre normal Y
// DbgUnhandledException al crashear.
//
// Lo segundo NO es de adorno (reporte de 2026-09-27): en pantalla completa el
// filtro termina el proceso con EXCEPTION_EXECUTE_HANDLER, asi que
// OpenGL_Release no corre y al usuario le quedaba el escritorio clavado en la
// resolucion del juego hasta reiniciar.
```

### Línea 187 — antes de `static void Window_Create(HINSTANCE hInst)`

```cpp
// ── Window_Create @ 0x0041DFF0 ───────────────────────────────────────────────
//
// Registra WNDCLASSA y crea la ventana principal.
//   style:  CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS = 0x2B
//   exStyle: 0x40008 (WS_EX_APPWINDOW|WS_EX_TOPMOST)
//   class name: "Dialog" (s_Dialog_005595e0)
//   window style: WS_POPUP (0x80000000)
//   Dimensions: DAT_0056156c × DAT_00561570 (de ChangeDisplaySettings)
//
// 2026-09-27: se agrego el modo ventana (ver Config/Config.h).  Lo de
// arriba describe la rama `!g_WindowMode`, que es la del binario.
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 205 en `Window_Create` — antes de `wc.hIcon         = LoadIconA(hInst, MAKEINTRESOURCEA(IDI_MAIN_ICON));`

```cpp
    // Icono grande (Alt+Tab, barra de tareas).  Antes era
    // LoadIconA(NULL, "IDI_ICON1"): con hInstance NULL la API busca entre los
    // iconos PREDEFINIDOS del sistema, que son ordinales, asi que un nombre
    // propio nunca matchea y devolvia NULL.
```

### Línea 553 en `DbgLog` — antes de `static __declspec(thread) bool s_inside = false;`

```cpp
    // 2026-04-30: el guard IsDebuggerPresent fue removido porque la fuente
    // original del crash en VS (MuEmu::DumpHex emitiendo bytes binarios) ya
    // está silenciada bajo #ifdef MUEMU_TRACE.  Ahora podemos loggear bajo
    // VS sin riesgo de AV en KernelBase, y los traces son visibles en
    // debug.log.  El sanitizado a ASCII printable abajo sigue activo como
    // segunda red de seguridad.
```

### Línea 570 en `DbgLog` — antes de `char sanitized[510];`

```cpp
    // 2026-04-29: sanitize msg para que solo tenga ASCII printable. VS debugger
    // intercepta WriteFile en debug.log y crashea cuando el contenido tiene
    // caracteres no-printables (probablemente del hex dump de DumpHex que pasa
    // bytes binarios como string al format %s).
```

### Línea 586 en `DbgLog` — antes de `static HANDLE h = INVALID_HANDLE_VALUE;`

```cpp
        // 2026-04-29: NO usar OutputDebugStringA — VS debugger lo intercepta
        // y dispara una AV en KernelBase cuando el debug message contiene
        // bytes que VS no puede mostrar (probablemente el hex dump de
        // MuEmu::DumpHex que tiene caracteres binarios).
        // Solo escribir a archivo, con kill-switch.
```

### Línea 597 en `DbgLog` — antes de `DWORD w = 0;`

```cpp
        // 2026-05-01 (v2): VS debugger captura first-chance exceptions ANTES
        // que el SEH del programa, aún con __try/__except. La única forma de
        // evitar la pausa de VS es NO LLAMAR WriteFile cuando hay debugger.
        // Bajo debugger: solo OutputDebugStringA → Output Window de VS.
        // Sin debugger: WriteFile normal a debug.log.
```

### Línea 638 en `DbgUnhandledException` — antes de `static __declspec(thread) int s_handlerDepth = 0;`

```cpp
    // 2026-04-29: anti-recursion guard. Si DbgLog crashea (e.g., WriteFile AV),
    // el filter se reentra → recursion infinita → otra AV en KernelBase.
```

### Línea 805 en `WinMain` — antes de `Quest_InitializeStaticState();`

```cpp
    // CSQuest: en el binario el objeto lo construye `unknown_libname_1`
    // (0x401010), que esta en la tabla de inicializadores estaticos del CRT y
    // corre ANTES de WinMain.  Aca nadie lo llamaba, asi que `g_csQuest` se
    // quedaba en 0 y cualquier acceso al objeto escribia sobre la pagina cero
    // (crash 0xC0000005 con param1 = 0x1C848, que es el offset de la lista de
    // quests).  Los lectores viejos no reventaban porque estaban gateados con
    // `g_csQuest != 0`; los handlers de quest nuevos si.
```

### Línea 846 en `WinMain` — antes de `_CrtSetDbgFlag(flag);`

```cpp
        // DELAY_FREE_MEM removed: el crash es en sbh_alloc_block (Small Block
        // Heap, capa abajo del debug heap CRT) — _CrtCheckMemory no valida sbh.
        // Mantenemos el debug heap básico para tener guard bytes en allocs
        // grandes; UAF en sbh requiere otro approach (Application Verifier).
```

### Línea 855 en `WinMain` — antes de `_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);`

```cpp
        // 2026-04-29: NO prompt on assert/error. _vsnprintf_s falla con
        // __invoke_invalid_parameter en CRT debug y popup aparece en cada call
        // bad. Dejamos solo log en Output Window — el SilentInvalidParameterHandler
        // se encarga de no recursar.
```

### Línea 890 en `WinMain` — antes de `extern BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD *self, const char *fn);`

```cpp
    // 9: carga de claves CSimpleModulus — tiene que pasar ANTES de que se envíe
    // el primer paquete. Sin estas claves, el cuerpo de cada paquete C3/C4 va
    // encriptado con ceros, el server no lo puede desencriptar, y responde con
    // FD_CLOSE (el síntoma del diálogo vacío que vuelve al login de la corrida del 2026-04-24).
```

### Línea 901 en `WinMain` — antes de `OpenTextData();`

```cpp
    // 10b: localized string pool — Data/Local/Text.bmd → GlobalText[1000][300].
    // El WinMain original llama OpenTextData() antes del chequeo de versión/integridad
    // (ver el port de IDA en stubs.cpp:33019). Lo invocamos acá para que todo camino
    // de UI que lea GlobalText[N] (mensajes de error, confirmaciones, cuentas regresivas...)
    // datos reales en vez de vacíos.
```

### Línea 967 en `WinMain` — antes de `const DWORD kCharSet = DEFAULT_CHARSET;   // era 129 (HANGEUL_CHARSET)`

```cpp
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
```

### Línea 1030 en `WinMain` — antes de `extern void* CharacterMachine;`

```cpp
    // BUG-FIX 2026-05-01: tambien setear el global C++ `CharacterMachine`
    // (declarado en globals.cpp:2543 como nullptr separado). HUD_Pass5
    // (Render_HudPass_4BD650_) chequea `if (!CharacterMachine || !CharacterAttribute) return;`
    // Si solo seteamos DAT_07cf1ffc, el HUD nunca rendea HP/MP/skills.
```

### Línea 1039 en `WinMain` — antes de `extern void __cdecl PathContext_Create(void);   // src/Game/PathFinder.cpp`

```cpp
    // Contexto del pathfinder (DAT_05826df4).
    //
    // 2026-08-17: antes era `malloc(0x420)` + memset, que dejaba el vtable de la
    // cola de prioridad (+0x414) en NULL — por eso PATH_FindPath (PATH::FindPath)
    // no se podia usar. Ahora se construye igual que el binario
    // (0x0043F280..0x0043F2C7: reserva de 0x424 bytes, vtable y campos en cero).
    //
    // InitPath (0x0043F2D0) NO se llama aca: ya estaba portada en
    // stubs_externs.cpp y la llama OpenFont (World_Init) desde Scene_Intro,
    // igual que en el binario. Este ctor corre antes, que es el orden correcto.
```

### Línea 1052 en `WinMain` — antes de `DAT_055c9ff0 = (DWORD)ChatListBox_Construct();`

```cpp
    // 22: vtable object construction (font/UI system objects)
    // 2026-04-29: antes se había identificado DAT_055c9ff0 como HGLRC y se
    // inicializaba mal, como un buffer en cero; IDA muestra que es el objeto del
    // motor del chat-listbox, que construye sub_40C7D0 (ctor) sobre
    // operator_new(0x5C8). Sin el ctor la vtable queda nula y todos los
    // sitios de dispatch (Render_GameFrame +0x10, UIChatLogWindow_AddText +0x70,
    // CreateChat +0x70) crasheaban en silencio — por eso nunca aparecía el HUD
    // in-world. ChatListBox_Construct portea sub_40C5D0+sub_40C7D0 1:1.
```

### Línea 1061 en `WinMain` — antes de `DAT_055c9ff4 = (DWORD)ChatListBox_ConstructWhisper();`

```cpp
    // 2026-04-30: DAT_055c9ff4 es el segundo widget de chat (susurro / panel de
    // notificaciones, esquina superior derecha). El WinMain de IDA llama
    // operator_new(0xBC) + sub_40E990 (hermana de sub_40C7D0, con nodo de lista
    // más chico, 0x18, y 24 filas visibles). Sin construirlo bien la vtable
    // quedaba nula → el dispatch vtable[+0x14] desde SecondPassword_Screen1 crasheaba leyendo 0x14.
```

### Línea 1068 en `WinMain` — antes de `HUD_InitInventoryPools();`

```cpp
    // 2026-04-30: los slots vacíos del inventario tienen que tener Type=0xFFFF, no 0.
    // Los scanners de grilla del motor (Item_FindQuickSlotByCategory/sub_482850/sub_482E40) toman el 0
    // como un tipo de arma válido y matchean celdas vacías por error, lo que hace
    // que RenderItem3D se invoque sobre basura → el bug de render del triángulo cyan.
```

### Línea 1087 en `WinMain` — antes de `{`

```cpp
    // Class name table (DAT_07d2b494, stride 300, indexed por cidx = (klass&7) +
    // 4*(klass>>3)). En el original vive en GlobalText[20..] (cargado del .bmd de
    // texto); acá lo poblamos directo con los nombres para el create-panel
    // (RenderText 285,+200 lee `DAT_07d2b494 + class*300`) y para consistencia con
    // el char-list. 2026-07-17.
```

### Línea 1265 en `WndProc` — antes de `if (!g_WorldLoading)`

```cpp
            // 2026-09-02: durante OpenWorld el pump de AccessModel reentra aca.
            // Se drena el socket (arriba) para que el server no cierre por
            // backpressure, pero NO se despachan los paquetes: quedan en la cola
            // y los procesa el frame siguiente, ya con los modelos cargados.
            // Sin esto, el 0x13 ViewportMonster creaba monstruos a medio cargar.
```

### Línea 1441 en `WndProc` — antes de `if (GoldInputEnable) {`

```cpp
            // 2026-08-08 (baúl: guardar/sacar zen). Per IDA WndProc L2047-2051:
            //   InputIndex = 0;
            //   if (GoldInputEnable) { InputGold = atoi(InputText[0]); ... }
            // Sin esto InputGold quedaba siempre en 0 y el diálogo de zen del
            // baúl no tenía forma de saber cuánto tecleó el jugador.
            // GoldInputEnable = GoldInputEnable, InputGold = InputGold.
```

### Línea 1460 en `WndProc` — antes de `const bool guildDeleteCodeDialog =`

```cpp
            // 2026-05-04: toggle del chat in-game (per IDA WndProc:2011-2046).
            //   - state=5 (in-world)
            //   - input vacío + InputEnable=0  → abre el chat (InputEnable=1)
            //   - input vacío + InputEnable=1  → cierra el chat (InputEnable=0)
            //   - input con texto + InputEnable=1  → envía el chat y cierra
            //
            // IDA: WndProc reserva Enter cuando hay foco de Guild o cuando
            // ErrorMessage 126/152 está capturando un Personal Code; en esos
            // casos el input no puede abrir ni enviar chat.
```

### Línea 1528 en `WndProc` — antes de `const bool guildDeleteCodeDialog =`

```cpp
        // Carácter imprimible — se agrega si hay algún modo de input activo.
        // El gate de InputEnable cubre login + chat; GuildInputEnable y los
        // modales 126/152 comparten el buffer, pero preservan un foco distinto
        // y no deben abrir el chat.
        // 2026-08-08: el diálogo de zen del baúl abre con InputEnable=0 +
        // GoldInputEnable=1 (sub_4EB5D0), así que con el gate viejo (sólo
        // InputEnable) NO se podía tipear nada. Per IDA WndProc L1953-1963 el
        // gate es `InputEnable || GoldInputEnable || …` y con GoldInputEnable
        // sólo se aceptan dígitos '0'..'9'.
```

## `src/functions.h`

### Línea 3 — antes de `#include <intrin.h>   // _ReturnAddress (used por HashTable_GetIndex defensive log)`

```cpp
// 2026-09-25: se borraron 40 declaraciones FUN_ huerfanas de este header --
// sin definicion, sin un solo call site y casi todas con la firma generica
// inventada (int, int, int, int).  Sus direcciones ya tienen su funcion real
// declarada con el nombre de IDA (RenderParty, RenderGuildList, RenderTrade,
// Camera_SetupFrustum, OpenTextData, ...), asi que lo unico que hacian era
// ofrecer un SEGUNDO simbolo C++ para la misma direccion del binario: la forma
// exacta en que quedo muerto el puente de AccessModel.
```

### Línea 14 — antes de `// ── HashTable / ref-count obfuscation ─────────────────────────────────────────`

```cpp
// Kayito canonical name index (from main.exe.idb, 2026-01-03):
```

### Línea 50 — antes de `inline unsigned int HashTable_GetIndex(void* /*ctx_ptr*/, void* /*key*/) {`

```cpp
// HashTable_GetIndex — dispatch via vtable at (MAIN_HASH_CLASS+0xC)
// Returns slot index, or 0xFFFFFFFF if not found.
//
// DEFENSIVE (silencioso): anti-tamper invocado desde sistemas per-frame.  Un
// ctx/vtable corrupto producía un fn() a basura imposible de rastrear.
// Validamos y retornamos 0xFFFFFFFF (== "no encontrado") en lugar de morir.
// NO se loggea desde acá — hacerlo re-entra DbgLog cuando Windows bombea
// mensajes durante la escritura, y termina en stack smash.  Si se sospecha
// que la tabla hash está rota, mirar MAIN_HASH_CLASS en el debugger.
// 2026-05-03: SAFE STUB. Always return 0xFFFFFFFF (= "not found") so all
// callers' `if (idx != 0xffffffff) ...` guards skip the subsequent deref.
// Previously this called the vtable's hash function (g_FakeHashVtable[3] =
// HashFn_Sentinel returning 0) — but HashTable_GetNode (HashTable_GetValue) then
// did its own key-match check and returned NULL when the slot's key didn't
// match the lookup key, causing NULL deref in callers like Game_MainLoop:158.
// Returning -1 here makes the table appear empty to callers, which IS our
// desired semantics (no anti-tamper data is actually stored).
```

### Línea 351 — antes de `void  __cdecl Particle_RenderAll(void); // IDA: FUN_0046be40`

```cpp
// 2026-05-07: Particle_Render real es void per IDA mu97k-src-IDA/raw/
// 0046BE40_Particle_Render.c. La firma anterior (6 args) era erronea — el
// llamador en Game_RenderTick lo invoca sin args.
```

### Línea 359 — antes de `void* __cdecl CreateJoint(int, float *, float *, float *, unsigned int, int, float, short,`

```cpp
// Compatibility bridge used only by stubs_IDA_ports.cpp.
```

### Línea 380 — antes de `void  __cdecl ItemConvert(int, int, int);`

```cpp
// Render_DrawSprite / Render_DrawSpritePool / CheckSprites — implemented in src/stubs.cpp (Character/Effect pool)
```

### Línea 384 — antes de `int   __fastcall Stats_CalcBase(int characterMachine);                // Stats_CalcBase (a`

```cpp
// Stat helpers — ported 2026-05-02. Signatures match IDA decompile.
```

### Línea 435 — antes de `void  __cdecl Action(DWORD c, DWORD o);`

```cpp
// IDA: Action (0x0048D640)
// Despachador real de acciones (pickup/equip/attack/skill/walk) basado en
// `*(c+749)` queue. Implementación en stubs.cpp.
// IDA: FUN_0048d640
```

### Línea 456 — antes de `DWORD __cdecl FUN_00494520(void *buf, char flag);`

```cpp
// IDA sub_494520(texto, 1): valida/consume el buffer de texto; 0 = seguir.
// 2026-09-24: la firma tenia un tercer parametro inventado (el estado de
// Enter); los dos unicos call sites son el tick de macros y pasan 2 args.
```

### Línea 551 — antes de `void  __cdecl Font_CreateTextDib(int dc);`

```cpp
// FUN_0050f7a0 NO es "Map_Unload": es sub_50F7A0, el envio de opciones F3/30.
// Portada como SaveOptionsToServer97k en UI/UI_InGameMenu.cpp (2026-09-21).
// FUN_0050f5f0 @ 0x0050F5F0 (IDA)
```

### Línea 809 — antes de `void  __cdecl FUN_004d23b0(char *origin_x, int origin_y, short *inv_base,`

```cpp
// 2026-05-08: real signature per IDA `004D23B0_sub_4D23B0.c` (Inventory grid
// render + click dispatcher). See Item/Item_ClickHandler.cpp for the port.
```

### Línea 847 en `mbclen`

```cpp
// IsLeadByte — already in stubs.cpp
```

### Línea 893 — antes de `int   __cdecl GetItemCount(int siType, int iLevel);  // 0x00482FF0`

```cpp
// ── Item inventory helpers (from Offsets.h) ───────────────────────────────────
// GetItemCount (0x00482FF0) y GetItemSlot (0x00482D70) se implementan en
// Item/Item_LegacyHelpers.cpp.  Aca habia dos #define que mapeaban sus FUN_ a
// esos nombres; nadie los usaba y eran una trampa: los bloques IDA-only de
// stubs_IDA_ports.cpp DEFINEN esos FUN_, asi que al activar su gate el define
// los convertia en una redefinicion de la funcion real.
```

### Línea 921 — antes de `void  __cdecl SetPlayerShock(int entity, int type); // IDA: SetPlayerShock (0x00444B60)`

```cpp
// ── Character animation/attack helpers (Kayito names, called from large stubs) ──
// SetPlayerAttack — already declared above (line 266) with 4 args: (int, int, int, int)
```

### Línea 936 — antes de `#define GetToken    TextParser_GetToken`

```cpp
// ── Token parser aliases ──────────────────────────────────────────────────────
// GetToken = TextParser_GetToken — lee el proximo token del archivo abierto y lo deja en
// TokenString (= TextParserTokenString, IDA @0x07CF1EF0).
// OJO: hay DOS tokenizers con buffers DISTINTOS y el arbol los tenia mezclados
// (corregido 2026-08-22):
//   GetToken        0x0047A1F0 -> TokenString    0x07CF1EF0  (Item/Monster/Skill/
//                                                             NPC/Gate/Filter.txt)
//   Parse_NextToken 0x0050E2C0 -> ParserTokenString   0x083A3FF4  (OpenWorldModels)
```

### Línea 1151 — antes de `void* __cdecl FUN_0053e8c0(void *param_1);                              // GG encrypted st`

```cpp
// FUN_0053d890 — implemented in GameGuard_Init2.cpp
```

### Línea 1242 — antes de `void  __cdecl FUN_00466300(float *param_1); // IDA: FUN_00466300`

```cpp
// Compatibility bridge used only by stubs_IDA_ports.cpp.
```

### Línea 1245 — antes de `void  __cdecl FUN_0046b980(int param_1); // IDA: FUN_0046b980`

```cpp
// Compatibility bridge used only by stubs_IDA_ports.cpp.
```

## `src/ghidra_compat.h`

### Línea 1 — antes de `#pragma once`

```cpp
// ghidra_compat.h
//
// Macros que el decompile de Ghidra / Hex-Rays da por existentes y que el port
// necesita para compilar sus salidas tal cual.
//
// Antes cada archivo las redefinía por su cuenta con su propio `#ifndef`: había
// 15 copias de `qmemcpy`, 14 de `delete__` y 11-12 de cada accesor de palabra,
// repartidas en 17 archivos. Eso rompía el refactor B3: al mover una función a
// otro módulo dejaba de ver las macros de su archivo de origen.
//
// Se incluye desde `stdafx.h`, así que está disponible en todo el proyecto.
// Los `#ifndef` se conservan para no chocar con los `#define` locales que aún
// queden en los .cpp mientras se van limpiando.
```

## `src/globals.cpp`

### Línea 167

```cpp
// 0.5*PI/180 — verificado bit-pattern 0x3C0EFA33 en binario original. Se usa para tan(FOV/2) en gluPerspective2 (GL_SetPerspective) y frustum (Camera_SetupFrustum). Antes era PI/180 (full) → PerspX/Y 1.92× más grandes → proyecciones name-labels clusterizadas al centro.
```

### Línea 190

```cpp
// compatibility alias for stubs_IDA_ports.cpp
```

### Línea 193 — antes de `DWORD    g_bUseChatListBox  = 1; // IDA: g_bUseChatListBox (0x005590AC)`

```cpp
// g_bUseChatListBox. Default IDA = 1 (verificado: bytes en 0x5590ac
// = 01 00 00 00, seguidos de flt_5590B0/B4/B8 = 295/417/18 coords del input dialog).
// FIX 2026-07-19: estaba en 0 (una sesión previa lo bajó para tapar un doble-render
// que en realidad se resuelve con el skip de mode 1/2 en ChatLB_renderLine). Con =1,
// sub_480980 corre in-world → mensajes de sistema/GM salen ARRIBA-IZQUIERDA (no en el
// área del ChatListBox abajo). Fiel a IDA.
// IDA: g_bUseChatListBox (0x005590AC)
```

### Línea 266 — antes de `int      _InputTextMaxArr[8] = {0,0,0,0,0,0,0,0};`

```cpp
// InputTextMax[] lives at 0x00559c94 in the original binary as a contiguous
// int array (stride 4). Our port accidentally split it into 4 independent
// globals so `(&InputTextMax)[1]` pointed at arbitrary adjacent storage —
// writes to the password slot went to limbo and WM_CHAR always saw max=0,
// silently rejecting every keypress. Fix: back all four aliases into one
// int[2] array so pointer arithmetic over `InputTextMax` indexes the real
// array.
// 2026-05-04: cambio float& → int& en `InputTextMax/c98`. Antes el alias
// era `float&`, así que `InputTextMax = 42` guardaba el bit pattern del
// FLOAT 42.0f (= 0x42280000 = 1109917696). El WM_CHAR / RenderInputText leen
// como int → ven 1109917696, fallan o caen al fallback maxLen=10.
// IDA original: `int InputTextMax[8]` — siempre se trata como int.
```

### Línea 280 — antes de `int&     _DAT_00559c98 = _InputTextMaxArr[1];`

```cpp
// 2026-09-25: el alias DWORD de InputTextMax[0] se elimino al renombrar
// DAT_00559C94: era el MISMO dato con otro tipo, o sea un puente duplicado.
```

### Línea 334 — antes de `int             g_HasConnectServer      = 0;  // server.cfg tiene 2 líneas`

```cpp
// ── ConnectServer flow (2026-07-15) ──────────────────────────────────────────
// Cuando server.cfg tiene 2 líneas: línea 1 = ConnectServer (szServerIpAddress/bc),
// línea 2 = GameServer fallback (g_GameServerIP/Port). g_HasConnectServer activa
// el flujo original: conectar al CS → recibir lista+load (F4/04/F4/02) → al
// elegir server mandar F4/03 → redirect al GameServer → login.
```

### Línea 492 — antes de `static unsigned int __cdecl HashFn_Sentinel(void*) { return 0; }`

```cpp
// ConfigLoginVersion (IDA: m_ExeVersion) defined in Config_Load.cpp as char[12]
// HashTable obfuscation. Original binary has a real hash-table object at
// 0x055c9bc8..0x055c9bd4 (contiguous). 40+ inlined callers deref the vtable
// at offset +0xC, and insert/lookup helpers read capacity at offset +0xC from
// the CONTEXT (&MAIN_HASH_CLASS + 0xC = DAT_055c9bd4).
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
// HashTable_GetNode lookup + NULL deref.
```

### Línea 585 — antes de `char     SocketClient[0x260000] = {};`

```cpp
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
// into adjacent globals. g_SimpleModulusSC (Dec2 keys) was placed by linker right
// after SocketClient's end, so every C3 packet trashed the decryption keys
// → C3 decode FAILED with checksum mismatch on every server response.
// IDA: SocketClient (0x055CA160)
```

### Línea 685 — antes de `char     DAT_05826e18[200 * 0x10] = {0};`

```cpp
// BoneQuaternion @ 0x05826E18 — scratch de cuaterniones por hueso que llena
// BMD_Animation (0x440060 L157-166: `(char *)&unk_5826E18 + 16 * boneIdx`).
// MAX_BONES = 200, igual que g_BoneScratch → 200 x 16 bytes.
// 2026-08-21: estaba declarado como UN SOLO DWORD y el port además hacía
// `&DAT_05826e18 + boneIdx * 0x10` sobre un DWORD*, o sea 64 bytes de paso en
// vez de 16.  Cada frame de animación pisaba ~12 KB de globals vecinos.  Lo que
// se rompía dependía de qué caía al lado en el layout de BSS: con el layout de
// esta rama caía justo sobre los flags de paneles (0x07EAA114..117 =
// ShopOpened/PartyOpened/.../InventoryOpened) y los menús se abrían solos.
```

### Línea 698 — antes de `short    DAT_077d87fc[200]    = {0};   // contador de vertices por hueso`

```cpp
// BMD bounding-box scratch arrays (BMD_CreateBoundingBox)
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
```

### Línea 713 — antes de `byte     DAT_005618b8  = 0;`

```cpp
// UI name-list panel data (ShowCheckBox)
// DAT_083a430c — ahora macro dentro de DAT_083a42f8 (dialog button rects)
// 2026-05-08: DAT_083a44c4 IS g_lpszMessageBoxCustom — a 7-line × 0x26 byte
// dialog/message buffer used by CreateOkMessageBox, RenderErrorMessage,
// CSQuest dialogs, SecondPassword UI, UI_StatsPanel, etc. Previously declared
// as a separate single DWORD (line 1328) which made the crt_sprintf and
// stride-0x26 line writes spill into adjacent globals (heap stomp). Now
// sized properly as char[7 * 0x26] = 266 bytes; DAT_083a44ea is exposed as
// a macro alias projecting to offset +0x26 (line[1]) inside the buffer.
// (See definition lower in the file alongside the other DAT_083a4* symbols.)
```

### Línea 729 — antes de `char     g_BoneScratch[200 * 0x30] = {0};   // = 0x2580 (BoneTransform[200][3][4])`

```cpp
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
```

### Línea 753 — antes de `char     DAT_07abf050[0x580] = {0};`

```cpp
// ── Preview character entity (0x07abf050) ─────────────────────────────────────
// BUG-FIX 2026-07-17: buffer de entidad COMPLETO para el preview char de
// char-select. Antes era `DWORD DAT_07abf050 = 0` (4 bytes) y los campos
// _DAT_07abf05c…_DAT_07abf5cc estaban como globales sueltos → CreateCharacterPointer
// (escribe hasta +908) desbordaba sobre BSS adyacente y corrompía todo. Ahora un
// solo buffer de 0x580 (matching el spacing del original hasta el array 0x07abf5d0);
// los campos se acceden por macros (ver globals.h). Ver charselect-deferred-issues.
```

### Línea 769 — antes de `char     DAT_07abf5f0[3000 * 0x70] = {0};   // particle pool base`

```cpp
// Particle pool — 3000 slots × 0x70 (112) bytes = 336000 bytes total.
// Original binary: 0x07abf5f0..0x07b11670 = 0x52080 bytes. /0x70 = 47999 slots.
// Pero MoveParticles itera 3000 slots; usamos ese tamaño que ya existe en stubs.
// Antes era 1 byte → CreateParticle (Particle_Spawn) tenía un overflow guard que
// retornaba 0 inmediatamente → NUNCA spawneaba lightning ELS=10/11, fire/smoke
// effects, weather particles, etc. — todo silenciado.
```

### Línea 783 — antes de `unsigned char DAT_07c5ab3c[200 * 0x70] = {};`

```cpp
// BUG-FIX 2026-04-28: SkillEffect pool — 200 slots × 0x70 bytes (= 22400 bytes).
// Antes era DWORD; SkillEffect_Render iteraba más allá del símbolo → potencial AV
// (mitigado por AUTO-SKIP en SkillEffect_Render.cpp). Ahora con tamaño real
// podemos sacar el AUTO-SKIP y permitir el render real.
```

### Línea 788 — antes de `char     g_RenderPool_07c608a8[100 * 0x2f0] = {};`

```cpp
// Joint/Trail/Blur shared render pool — 100 slots × 0x2f0 = 76800 bytes.
// IDA layout: each slot starts at offset 0; DAT_07c608b4 is the +12 "anchor"
// field within slot[0]. We back the entire pool here and project the anchor
// through a macro (see globals.h). Was declared as single DWORD = 0 → all
// joint / trail / blur loops walked random memory and crashed when any
// post-login non-zero byte was encountered.
```

### Línea 797 — antes de `char     g_PlayerRenderPool[100 * 0x1BC] = {};`

```cpp
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
```

### Línea 841 — antes de `char     lpString_07d4aed4[128] = "OK";          // botón OK del panel de credenciales`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[450]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4ac7c[256]      = {};
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[451]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4ada8[256]      = {};
// Scene_Login credential dialog + version footer. La tabla de strings del
// 0.97k está stripped: get_xrefs_to en Ghidra confirma que NADA escribe estos
// buffers en el binario (se renderizan vacíos). Rellenamos con defaults
// sensatos para que el panel muestre botones/texto legible.
```

### Línea 851 — antes de `char     lpString_07d4c518[128] = "Connecting...";`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[459]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4b708[128]      = {};            // char name format string
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[454]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4b12c[128]      = "Mu Online";                       // línea de versión 1 (centrada)
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[455]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4b258[128]      = "Ver 0.97k";                       // línea de versión 2 (derecha)
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[456]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4b384[128]      = "Copyright (C) 2003 Webzen Inc.";  // sprintf format sin args (izquierda)
```

### Línea 861 — antes de `unsigned char DAT_07e12840[1000 * 0x204] = {};`

```cpp
// El pool de items en el suelo es de 1000 entradas × 0x204 bytes (≈504 KB).
// El slot base es DAT_07e12840 + key*0x204; CreateItem escribe active@ip+72,
// model@ip+74, pos@ip+88 y el render (Entity_Render en stubs_render_helpers.cpp)
// los lee en los mismos offsets (active@slot+72). Ambos alineados sobre
// DAT_07e12840.
```

### Línea 879 — antes de `char     DAT_07e919bc[0x13C30] = {};`

```cpp
// Tabla de nombres (guild members / buffs), stride 80 (0x50). En el binario
// va de 0x07E919BC al centinela dword_7EA51EC (0x07EA51EC) = 0x13C30 bytes.
// 2026-08-15: era un DWORD de 4 bytes y TODOS sus consumidores la indexan
// como `&DAT_07e919bc + N*80` — o sea leían fuera de rango. Ver la nota en
// globals.h.
```

### Línea 891 — antes de `int   *p_DAT_07ea9504_ = (int*)&OffsetInventoryItems[63 * 0x44 + 56];`

```cpp
//
// 2026-08-22: acá había un `g_EquipGridBuf[0x12DC]` suelto, en cero, que nadie
// escribía nunca — los tres walkers recorrían memoria vacía y siempre reportaban
// "no tenés el item".  Por eso la lista de items de quest salía en rojo con el
// item en el inventario.  Ahora los dos punteros se reenraízan sobre el pool real.
```

### Línea 982 — antes de `float    DAT_07eab200[256 * 256] = {};`

```cpp
// 2026-05-04: water-wave height table — 256×256 floats = 256KB.  Original
// binary placed it at 0x07EAB200; was a 4-byte DWORD here, so writes via
// `(char*)&DAT_07eab200 + (row*256+col)*4` (Terrain_Water.cpp + Terrain_Light.cpp)
// overflowed massively into adjacent BSS.
```

### Línea 988 — antes de `DWORD    FrustrumFaceD  = 0;`

```cpp
DWORD    DAT_07eab250  = 0;   // PrimaryTerrainLight array base
```

### Línea 995 — antes de `float    FrustrumY[4] = {0};   // frustum quad Y[4]`

```cpp
// BUG-FIX 2026-05-01: era DWORD simple pero TestFrustrum2D (Frustum_IsVisible)
// lee 4 floats consecutivos desde cada array. Camera_SetMatrix también escribe
// los 4 corners. Sin contiguidad garantizada, el cull del frustum rechazaba
// TODOS los chunks (chunks_vis=0) y los objetos del .obj nunca se renderean.
```

### Línea 1001 — antes de `float    DAT_07eeb238[256 * 256 * 3] = {};`

```cpp
// BUG-FIX 2026-04-28: era DWORD simple pero OpenJpegBuffer escribe 256x256 RGB
// floats (= 196608 floats) usados como TerrainLight RGB ambiente.
```

### Línea 1006 — antes de `float    g_TilePickBuf[12] = {};`

```cpp
// 2026-04-28: tile pick corners buffer — 12 floats contiguos (4 vec3 corners
// del quad clickeado). Antes era 12 globals separados → no garantizado
// contiguo en memoria → glVertex3fv leía basura → AV en NVOGL.
```

### Línea 1010 — antes de `float    DAT_07feb288[256 * 256 * 3] = {};`

```cpp
// BUG-FIX 2026-04-28: TerrainNormal[256*256][3] float array — antes era DWORD.
// FUN_004f70b0 (CreateTerrainNormal) escribe 256*256*3 = 196608 floats acá.
```

### Línea 1014 — antes de `unsigned char  TerrainMappingLayer2[0x10000] = {};`

```cpp
// ── Large game data arrays ────────────────────────────────────────────────────
// BUG-FIX 2026-04-28: estos cuatro estaban declarados como `DWORD` simple pero
// el código (InitTerrainMappingLayer Terrain_Clear y otros) los indexa hasta [65535].
// El IDA decomp expresa los accesos como `(int)&DAT_xxxx + iVar2` que MSVC
// compila como offset del símbolo → escribe fuera de bounds → AV/corruption.
// Cambiar a arrays explícitos del tamaño real evita el crash y elimina la
// corrupción silenciosa de globals adyacentes.
//   TerrainMappingLayer2 = TileTex2[256*256] (BYTE)   — segundo índice de textura
//   TerrainMappingLayer1 = TileTex1[256*256] (BYTE)   — primer índice de textura
//   TerrainMappingAlpha = TerrainHeight[256*256] (float) — altura de tile
//   DAT_0810b2cc = TerrainNoise[256*256] (float)  — ruido aleatorio init
```

### Línea 1027 — antes de `float    DAT_080cb2cc[0x10000] = {};`

```cpp
// BUG-FIX 2026-04-28: BackTerrainHeight[256*256] float array — antes era DWORD.
```

### Línea 1033 — antes de `float    DAT_081cb608[256 * 256 * 3] = {};`

```cpp
// 2026-05-04: live per-tile lighting buffer — 256×256 tiles × 3 floats =
// 786432 bytes.  Was a 4-byte DWORD here, but Terrain_Water writes via
// `(char*)&DAT_081cb608 + iVar2*12` (and analogous via cb60c, cb610) up to
// 786KB into adjacent BSS — that was the source of the GateAttribute "Oye!"
// corruption.  Original binary placed cb608/cb60c/cb610 as the three DWORDs
// of slot 0; the macros below project cb60c/cb610 into the same buffer.
```

### Línea 1040 — antes de `float    DAT_0828b608[256 * 256 * 3] = {};`

```cpp
// BUG-FIX 2026-04-28: TerrainLightData[256*256][3] — antes era DWORD.
// FUN_004f71c0 (Terrain_FinalizeLighting) escribe 196608 floats acá.
```

### Línea 1060 — antes de `unsigned char DAT_0838b800[1080] = {};`

```cpp
// BUG-FIX 2026-04-28: BMPHeader[1080] (BITMAPFILEHEADER + DIB header + palette)
// — antes era DWORD, OpenTerrainHeight escribe 1080 bytes acá.
```

### Línea 1077 — antes de `char     g_ObjectBucketGrid[0x1000] = {0};`

```cpp
// Previously DAT_083a0218 was a separate orphan DWORD and DAT_083a021c was at
// grid+0 — the unload walker read 4096 bytes of unrelated BSS past the orphan
// DWORD and crashed when it hit a non-null garbage value (treated as a node).
```

### Línea 1210 — antes de `char     DAT_083a7c48  = 0;`

```cpp
// BUG-FIX: DAT_083a7c48 es char en el binario original (flag "connection check
// enable" leído como byte en Game_MainLoop). Declararlo DWORD hacía que
// `DAT_083a7c48 = 1` escribiese 4 bytes 01 00 00 00 y pisase DAT_083a7c49/4a/4b.
// Resultado: cada frame el init flag de Scene_Login (c49) volvía a 0, Scene_Login
// retornaba temprano y nada se dibujaba.
```

### Línea 1252 — antes de `char     g_PadBeforeChatMode[64] = { 0xCC };`

```cpp
// ── Additional globals needed by Game/Scene files ─────────────────────────────
// 2026-05-04: 64-byte canary padding around GuildInputEnable..d72 (ChatMode/IME/
// DigitOnly).  These flags get corrupted to 0xFF every frame by an adjacent
// buffer-overflow we couldn't pinpoint yet.  Canaries should ABSORB the
// overflow if the writer is hitting bytes near d70/d71/d72.  If the canaries
// remain 0xCC after the corruption fires, the overflow is much further away.
```

### Línea 1280 — antes de `char     DAT_07e113e4[5 * 256] = {};`

```cpp
// Historial de chat @ 0x07E113E4 — anillo de 5 entradas x 256 bytes.
// Lo confirma el propio binario: el walker de RenderChatInput termina en
// 0x07E118E4, y 0x7E118E4 - 0x7E113E4 = 0x500 = 5 * 256.
// 2026-08-21: estaba como un solo DWORD, y Chat_InputTick (flechas arriba/abajo)
// hace `memcpy((char*)&DAT_07e113e4 + idx * 0x100, ...)` — escritura de hasta
// 1280 bytes fuera del global.
```

### Línea 1291 — antes de `char     DAT_07db8710[10][256] = {{0}};`

```cpp
// DAT_07d780ac ELIMINADO (2026-08-26): en el binario es `InputLength[1]`, o sea
// los bytes +4..+7 de DAT_07d780a8, no una variable aparte. Ahora es un macro
// en globals.h que proyecta dentro del array. Ver la nota alli.
// Multi-slot input buffer (IDA: InputText[10][256] @ 0x07db8710).
// Slot 0 = chat / username, slot 1 = whisper-target / password.
// DAT_07db8810 is a #define alias for slot 1 in globals.h.
```

### Línea 1300 — antes de `extern "C" {`

```cpp
// 2026-05-04: server-config globals (popullados por opcodes 0xDD/DE/DF).
// gPrintPlayer.MaxCharacterLevel del DLL source mapea a g_MaxCharacterLevel.
// Usado por RenderCharacterInfoWindow "Nivel: %d / %d".  Default 400 = cap
// vanilla 0.97k hasta que el server mande PMSG_CHARACTER_MAX_LEVEL_RECV.
```

### Línea 1314 — antes de `DWORD    DAT_07d52c38  = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[470]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4c3ec[256] = {0};
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[472]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4c644[256] = {0};
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[473]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d4c770[256] = {0};
```

### Línea 1321 — antes de `// Model data table base + entity vtable`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[563]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d530e8[256] = {0};
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[564]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d53214[256] = {0};
```

### Línea 1356 — antes de `char     DAT_07c85890[1002 * 0x1bc] = {0};`

```cpp
// Character/effect update pool — 1002 slots × 444 bytes = 0x6c660 (matches
// binario original 0x07c85890..0x07cf1ef0). Antes era 1 byte → CreateSprite
// (Effect_Spawn) tenía un AUTO-SKIP que saltaba la implementación entera y
// NO spawneaba NINGUNA partícula (glow +9, wing FX, weapon glows, lightning
// crackles — todo invisible). Buffer real ahora permite que el pool funcione.
```

### Línea 1383 — antes de `int      DAT_07d78068 = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[120]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d329c4 = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[121]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d32af0 = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[140]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d34134 = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[141]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d34260 = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[160]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d358a4 = 0;
```

### Línea 1394 — antes de `int      FontHeight = 0;`

```cpp
// 2026-05-08: backup MOVED to Render_Frame.cpp — adjacent placement next to
// DAT_07d78068 caused the corruption writer (2 consecutive int writes
// 0x00000001 + 0x00000000) to clobber both. Now lives in a different .obj.
// FontHeight — alto de la fuente, lo calcula WinMain segun la resolucion
// (12 en 640x480, 13 en 800, 14 en 1024, 15 en 1280+) y lo leen RenderBoolean
// (0x00480E00) y sub_480C60.
//
// 2026-09-26: era FontHeight y convivia con una variable FontHeight aparte
// fijada en 14.  WinMain escribia esta y TODO el render leia la otra, asi que
// el tamano calculado por resolucion no llegaba a ningun lado.
// IDA: FontHeight (0x07D78080)
```

### Línea 1421 — antes de `char     DAT_0055a4e4[] = "\n";  // 0x0055A4E4 — separador tras la linea de precio`

```cpp
// RenderItemInfo string constants
// 2026-08-18: los SIETE son "\n" en el binario (leidos en 0x0055A4E0,
// 0x0055A4E4, 0x0055A570, 0x0055A5F4, 0x0055A5F0, 0x0055A5FC, 0x0055A640) —
// lineas separadoras de MEDIA altura, que es lo que cuenta SkipNum
// (DAT_07eaa158).  Estaban como cadena vacia, y como DrawItemInfoBox corta el
// conteo en la primera linea vacia, el slot 0 dejaba el tooltip sin dibujar.
```

### Línea 1436 — antes de `// Weather particle system (MoveLeaves): DAT_07c5ab5c is the +0x20 alias`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[238]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3b40c[] = "";    // item level line format
```

### Línea 1468 — antes de `char     m_bAutoAttack  = 1;`

```cpp
// 2026-05-06 BUG-FIX: m_bAutoAttack default = 1 (enabled). Per IDA
// Mouse_Hover (sub_4B0310:85), if !m_bAutoAttack the hover-target
// (DAT_00559c50 / SelectedCharacter) is reset to -1 every frame BEFORE the click handler reads
// it → click on mob fell through to ground-click handler. User reported
// "no atacaba a la primera, me costo empezar a atacar" 2026-05-06.
```

### Línea 1489 — antes de `char     DAT_083a2e90[10 * 0x1bc] = {};`

```cpp
// 2026-04-28: pool de boids (fish/butterfly/bird flocking).
// Particle_PathUpdate itera 10 entries × stride 0x1bc = 0x1180 bytes. Antes era una
// dirección absoluta del binario original (0x083a2e90); declarada como array
// real para que el flocking algoritmo funcione 1:1 con el original.
```

### Línea 1520 — antes de `char     s__d___s_005580b0[] = "%d %s";`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[609]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d566d0  = 0;   // fallback char name string
```

### Línea 1534 — antes de `DWORD    DAT_00559bf1  = 1;`

```cpp
// Chat globals added to globals.h in prior session (from Chat_InputTick analysis)
// DAT_00559bf1 = byte_559BF1 = toggle "Ver chat on/off" (tecla F2).
// FIX 2026-07-19: default IDA = 1 (verificado: byte en 0x559BF1 = 0x01), estaba en 0.
// Con 0, el chat normal (canal 3) se descartaba en DOS lugares:
//   - ChatLB_AddText:    `else if (kind == 3) return;`  → ni se agregaba a la lista
//   - ChatLB_renderLine: `if (!DAT_00559bf1 && msgType==3) return 0;` → no se dibujaba
// Resultado: los mensajes de chat de jugadores nunca aparecían.
```

### Línea 1545 — antes de `char     DAT_07df9380[0x77 * 0x118]  = {0};`

```cpp
// Chat ring buffer (UI_RenderChatLogOverlay renderer, UIChatLogWindow_AddText writer).
// BUG-FIX (blue countdown / chat render): DAT_07df938b, DAT_07df948c y
// DAT_07df9494 son ALIASES a offsets 0x0B / 0x10C / 0x114 del slot 0 dentro de
// este mismo buffer en el binario original. Ghidra los recuperó como globales
// independientes; con almacenamiento separado, UIChatLogWindow_AddText escribe
// en DAT_07df9380 pero UI_RenderChatLogOverlay leía las variables sueltas (siempre 0) y
// el guard `msg[0] != '\0'` fallaba → el texto jamás aparecía. Se convierten
// a macros en globals.h que resuelven al byte/DWORD real del buffer.
```

### Línea 1563 — antes de `char     MacroText[10 * 0x100] = {};`

```cpp
// BUG-FIX 2026-04-28: macro hotkey table — 10 slots × 0x100 bytes.
// Era char (1 byte). OpenMacro escribe a [0x07e0ffc8 .. 0x07e109c8] = 2560 bytes.
```

### Línea 1570 — antes de `DWORD    DAT_07e11dac  = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[264]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3d284  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[265]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3d3b0  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[260]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3cdd4  = 0;
// Chat command parser name buffers (Chat_ValidateCommandName)
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[258]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3cb7c  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[259]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3cca8  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[256]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3c924  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[254]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3c6cc  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[248]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3bfc4  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[249]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3c0f0  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[267]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3d608  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[268]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d3d734  = 0;
```

### Línea 1596 — antes de `DWORD    DAT_07ea9848  = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[593]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d55410  = 0;
```

### Línea 1604 — antes de `char     DAT_07e11dd0[256] = {0};`

```cpp
// DAT_07e11dd0 = byte_7E11DD0: buffer de TEXTO del aviso periódico.
// MoveNotices (IDA 0x47FCB0) hace cada 300 frames `CreateNotice(byte_7E11DD0, 0)`,
// y CreateNotice hace `lstrlenA` + `strcpy` sobre él (hasta 256 bytes).
// FIX 2026-07-19: estaba declarado como UN SOLO char → esas lecturas se iban a
// los globals adyacentes y renderizaban basura como aviso dorado (el usuario veía
// mensajes dorados con una sola letra "D" que el cliente original NO mostraba).
// Ahora es un buffer propio, zero-init → aviso vacío (no se dibuja) hasta que
// se porte el handler que lo llena (probablemente el F3/E6 periódico del server,
// que trae los textos de evento tipo "Devil Square").
```

### Línea 1614 — antes de `char     DAT_07e11dd8[4]  = {0};`

```cpp
// DAT_07e11dd8 = strText (0x07E11DD8) y DAT_07e11ddc = byte_7E11DDC: los dos
// argumentos de la llamada periodica de Chat_TickMessageTimer (0x480950).
// En TODO el binario cada uno tiene UN SOLO xref, que es justamente esa
// lectura: nadie los escribe nunca, o sea son cadenas vacias permanentes.
// Eso es a proposito -- ver la nota en Chat_TickMessageTimer.
//
// Tienen que ser BUFFERS, no un char suelto: se pasan como `const char*` y se
// recorren con strlen, asi que un unico byte no garantiza terminador propio y
// la lectura se mete en el global que el linker haya puesto al lado.  De ahi
// salia el mensaje fantasma con un caracter raro que se vio en 2026-07-27.
// Es el mismo bug que ya se habia corregido en DAT_07e11dd0 (aviso dorado con
// una sola letra), aca sin corregir.  Los tamanos son los huecos reales del
// binario: dd8..ddc = 4 bytes, ddc..de8 = 12.
```

### Línea 1634 — antes de `char     DAT_07db80d8[6 * 0x108]  = {0};   // system chat buffer (6 slots × 0x108)`

```cpp
// Chat ring buffers
// 2026-05-04: enlarge to actual slot pool size — IDA loop in UI_RenderNotices
// walks 6 slots × 0x108 stride. Antes era single byte → AUTO-SKIP.
```

### Línea 1646 — antes de `char     DAT_07e11cec[10 * 4] = {0};`

```cpp
// 2026-09-03: tabla de 10 punteros del buffer de composicion del IME.
// Era un `char` suelto y WinMain lo escribia con `slot * 4` (slot clampeado a
// 0..9), o sea 36 bytes fuera; Chat.cpp lo lee como `LPCSTR*`.
```

### Línea 1665 — antes de `char     param_2_07d68268 = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[677]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d5b680  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[678]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d5b7ac  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[686]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d5c10c  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[687]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d5c238  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[679]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d5b8d8  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[850]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d6813c  = 0;
```

### Línea 1678 — antes de `char     param_2_07d58fd4 = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[643]. Ver el bloque de alias al final de globals.h.
// BYTE     DAT_07d58ea8  = 0;
```

### Línea 1694 — antes de `BYTE     DAT_083a7af8[GUILD_MEMBER_TABLE_BYTES] = {0};`

```cpp
// 2026-09-03 FIX -- tabla de miembros de guild (UI_GuildLegacy).
// Eran SEIS escalares sueltos (24 bytes en total = una sola entrada), pero el
// binario los trata como un array de registros de 0x18 bytes:
//   +0x00 name[10]  (DWORD+DWORD+WORD)   +0x0C, +0x10, +0x14  DWORDs
// `GuildMemberList_Set` copia `count * 0x18` bytes desde el paquete y el render
// lee `base + iMod*0x18`, asi que con mas de un miembro se escribia/leia sobre
// los globals vecinos.  El hueco real en el binario va de 0x083A7AF8 al
// siguiente global conocido (0x083A7C00) = 0x108 bytes = 11 entradas.
```

### Línea 1703 — antes de `char     param_2_07d59484  = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[647]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d59358  = 0;
```

### Línea 1706 — antes de `char *   PTR_DAT_005618a0  = nullptr;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[680]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d5ba04  = 0;
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[685]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d5bfe0  = 0;
```

### Línea 1737 — antes de `float&   _CameraRayOriginX = *reinterpret_cast<float*>(&CameraRayOriginX_arr[0]); // _DAT_`

```cpp
// _CameraRayOriginX..428c — float aliases sobre el mismo array DWORD que escribe
// Camera_MouseRay (ver CameraRayOriginX_arr arriba). Antes eran floats separados,
// causando que las lecturas via _DAT_xxx vieran 0/basura mientras los writes
// via DAT_xxx (DWORD) iban a otra memoria → mouse-ray rayO siempre incorrecto.
```

### Línea 1767 — antes de `int      DAT_07e91528[12 * 10] = {};`

```cpp
// Tabla de requisitos de stats @ 0x07E91528 — 12 filas x 10 ints (480 bytes).
// sub_4C2E20 escribe `dword_7E91528[10*i]`, `dword_7E91530[10*i]`, etc. (paso de
// fila = 40 bytes) y sub_4C2C10 la recorre con `v9 += 10`.
// 2026-08-21: estaba como `int[12]` mas nueve globals sueltos para las columnas,
// y el port hacia `&DAT_07e91530 + i * 4` sobre un int* (64 bytes de paso) —
// escritura fuera de rango de ~176 bytes cada vez que se arma el menu de
// personaje.  Mismo patron que BoneQuaternion.
```

### Línea 1775 — antes de `int      DAT_00559fe0  = -1;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[161]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d359d0  = 0;
```

### Línea 1790 — antes de `char     DAT_07d486fc[300] = {};`

```cpp
// 2026-09-03: DAT_07d29d24 pasa a ser un alias de GlobalText (ver globals.h).
// Era un `char` suelto recorrido con `&DAT_07d29d24 + i * 300`, y sus dos
// lectores usan indices ~601-607 (nombres de clase): leian ~180 KB fuera del
// global y le pasaban el resultado a lstrlenA / crt_sprintf.
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[397]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d46e60  = 0;
```

### Línea 1799 — antes de `char     DAT_07d4950c  = 0;`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[429]. Ver el bloque de alias al final de globals.h.
// char     DAT_07d493e0  = 0;
```

### Línea 1910 en `HashFn_Sentinel` — antes de `float   _DAT_00552cf0 = 1.0f;          // 1.0 (quaternion normalization)`

```cpp
                                       // IDA sub_4FA1D0 shows literal `a1[k] * 0.5` — quaternion
                                       // half-angle. Input Euler angles are already in RADIANS
                                       // (AngleMatrix path uses Math_DegreesToRadians=π/180, different const).
                                       // Previously set to π/180 by mistake, which made every bone
                                       // rotation ≈ 0 → characters rendered with wrong orientation.
```

### Línea 1919 — antes de `char    DAT_07b11670[200 * 0x1bc] = {};   // 200 slots: (0x07B27150-0x07B11670)/0x1bc (IDA`

```cpp
// ── Effect pool ───────────────────────────────────────────────────────────────
// 2026-04-28: effect pool — Effect_TickAll itera 200 slots × 0x1bc bytes.
```

### Línea 1934 — antes de `char    bBuxCode[3] = { (char)0xFC, (char)0xCF, (char)0xAB };`

```cpp
// bBuxCode @ 0x00558090 — la clave XOR de 3 bytes de BuxConvert_1 (0x401120),
// la que descifra Quest.bmd.  Leida del binario: FC CF AB — la misma que usa
// BuxConvert_0 (DAT_00559bb4), pero es otra copia en otra direccion.
// 2026-08-21: estaba declarada como UN char = 0, asi que BuxConvert_1
// (IDA: BuxConvert_1) hacia
// `(&bBuxCode)[i % 3]` sobre un cero y dos bytes de globals vecinos: el
// script de quests quedaba sin descifrar.  De ahi que el nombre del NPC saliera
// equivocado (getMonsterName de un tipo basura) y el texto de la quest vacio.
// IDA: bBuxCode (0x00558090)
```

### Línea 2085 — antes de `alignas(16) char g_WeatherSlotPool[40 * 0x1bc] = {0};`

```cpp
// ── Weather particle pool (40 × 0x1bc = 0x4560 bytes) ────────────────────────
// BUG-FIX 2026-05-04: backing buffer único. Antes los 30+ DAT_0839bc?? eran
// chars sueltos en BSS, lo que causaba AV cuando Weather_Update walked slots
// 1..39 con stride 0x1bc. Ver globals.h para los macros de field accessors.
```

### Línea 2100 — antes de `int     DAT_0055a7b0   = -1;   // tipo de objeto que se derrumba`

```cpp
// ── Camera / viewport globals ─────────────────────────────────────────────────
// SetActionObject (0x4FA5C0) / MoveObject_Special (0x4FA5F0): animacion de
// derrumbe de la puerta del evento.  En el binario los cuatro arrancan en -1
// (bytes .data en 0x0055A7B0: FF FF FF FF x3 + 00 00 80 BF) y los guards de
// MoveObject_Special son comparaciones CON SIGNO (`unk_55A7B4 < 0`).
// 2026-09-04: estaban como DWORD inicializados en 0, o sea (a) los `< 0` nunca
// se cumplian y (b) en el primer frame de Lorencia (World == 0 == DAT_0055a7b4)
// cualquier objeto de tipo 0 entraba al bloque de derrumbe: se ocultaba, se
// giraba a 90 grados y se disparaba AddTerrainAttributeRange(13,70,3,6,8,0).
```

### Línea 2127 — antes de `char    DAT_07b27150[500 * 0x9d8] = {};   // 500 slots: (0x07C5AB30-0x07B27150)/0x9d8 = 12`

```cpp
// ── Joint pool ────────────────────────────────────────────────────────────────
// 2026-04-28: joint pool — Joint_TickAll itera 500 slots × 0x9d8 bytes (~1.2MB).
```

### Línea 2188 — antes de `DWORD   DAT_07e11f34[16] = {0};  // MarkColor[16] — paleta de la marca (ARGB)`

```cpp
// DAT_07ea7b88: alias de OffsetTradeItems (globals.h)
// 2026-08-25: el comentario decia "MarkColor[16]" y estaba declarado como UN
// DWORD. `CreateGuildMark` (0x4F0100) escribe los 16 colores y
// `RenderGuildMark` (0x4F02F0) indexa `MarkColor[p5]` con p5 en 0..15, o sea 60
// bytes de desborde sobre el vecino en BSS. `MarkColor[1] = 0xFF000000` es
// justamente el valor que aparecia en el crash del editor de marca
// (0xC0000005 leyendo 0xFF000000 dentro del driver GL).
// El hueco hasta DAT_07e11f78 es de 68 bytes, asi que los 16 entran.
```

### Línea 2225 — antes de `char    DAT_07c74ec8[40 * 0x1bc] = {};`

```cpp
// 2026-04-28: fade-effect pool — Effect_TickFade itera 40 slots × 0x1bc bytes.
```

### Línea 2227 — antes de `char    DAT_07c82cdc[63 * 0x70] = {};`

```cpp
// 2026-04-28: flare effect pool — Effect_TickFlare itera 63 slots × 0x70 bytes.
```

### Línea 2229 — antes de `char    DAT_07c80128[100 * 0x70] = {};`

```cpp
// 2026-04-28: spark-effect pool — Effect_TickSpark itera 100 slots × 0x70 bytes.
```

### Línea 2235 — antes de `// ── Scene_Resources string literals ──────────────────────────────────────────`

```cpp
// 2026-05-04: cb60c/cb610 and 0828b60c/610 are NOT separate globals — in the
// original binary they're the 2nd/3rd DWORDs of slot 0 of cb608/0828b608.
// Code uses `(&DAT_081cb60c)[iVar2*3]` to access slot iVar2's 2nd field, and
// `(char*)&DAT_081cb60c + iVar2*12` to write to it.  We promote them to
// macros that project into the actual buffer.  See globals.h.
```

### Línea 2241 — antes de `char    DAT_0055e834[8]   = "Ship";          // → Data\Object1\Ship01.bmd (login ship)`

```cpp
// ── Scene_Resources string literals ──────────────────────────────────────────
// Originally at .rdata in the binary; their content must match the actual
// asset file basenames on disk or Scene_LoadAccountResources / ...CharSelectResources
// construct malformed paths (e.g. "Data\\Object1\\01.bmd" instead of "Ship01.bmd").
// Ship/Logo/Face are BMD basenames used by AccessModel;
// the three SMD entries (Korean-named background/face assets) are only consumed
// by OpenModel which is stubbed in this port — kept as empty strings so any
// sprintf(%s, "") produces harmless paths without crashing.
```

### Línea 2316 — antes de `DWORD   KeyState[256] = {0};`

```cpp
// 2026-04-28: tooltip/bubble pool — UI_TickTooltips (UI_TickHoverBubbles) itera 26
// slots × 0x254 bytes (stride 0x95 DWORDs). Antes era DWORD simple → AV.
// 2026-07-19: `DAT_07e01720` YA NO es un array propio — es el pool de burbujas
// de chat proyectado a +40. Ver DAT_07e016f8 más abajo. La declaración vieja
// (26 slots sueltos) convivía con `DAT_07e016f8` como char de 1 byte, y
// CreateChat caminaba ESE char con stride 596 → AV.
// Key-state table para PressKey (PressKey / Key_IsJustPressed).
// La función indexa como `*(DWORD*)((char*)&KeyState + vkey*4)`, o sea
// 256 entradas DWORD (1024 bytes) — una por código VK. En IDA es una tabla
// al símbolo dword_7E118EC. Si se deja como DWORD single, cualquier tecla
// con vkey>=1 pisa globals adyacentes y el edge-trigger queda corrupto
// (ESC=27 pisa 108 bytes hacia adelante).
```

### Línea 2338 — antes de `char    DAT_083a2f78[10 * 0x1bc] = {};`

```cpp
// 2026-04-28: Ambient particle pool — Ambient_ParticleUpdate itera 10 slots
// × 0x1bc bytes (stride 0x6f DWORDs). Antes era DWORD simple → AV al spawnear.
```

### Línea 2360 — antes de `DWORD   DAT_00561740   = 0x20;      // " "`

```cpp
// BUG-FIX 2026-07-17: strings reservados de sub_513570 (name-filter). En el binario
// original son patrones bloqueados (espacio, DBCS coreano, punto); estaban en 0 (string
// vacío) → FindText(nombre,"") devuelve 1 → TODO nombre se rechazaba con "palabras
// restringidas". Valores reales de IDA (bytes little-endian): 0x561740=" ",
// 0x561744="\xA1\xA1", 0x561748=".", 0x56174c="\xA1\xA4", 0x561750="\xA1\xAD".
```

### Línea 2473 — antes de `float   _DAT_005527d0   = 6.0f;    // MoveItems: decaimiento de la velocidad Z por frame`

```cpp
// 2026-08-21: los dos valían un valor inventado.  Leídos del binario:
// 0x5527D0 = 40 C0 00 00 → 6.0f  y  0x552A28 = C1 20 00 00 → -10.0f.
```

### Línea 2524 — antes de `char    DAT_00559bb4[3] = { (char)0xFC, (char)0xCF, (char)0xAB };`

```cpp
// BuxConvert_0 (BuxConvert_0) indexes (&DAT_00559bb4)[i % 3] — so this must be
// a 3-byte array, not a scalar.  Previously declared as a single char, which
// made the XOR cipher pick up whatever two bytes happened to be adjacent in
// memory, scrambling every Text.bmd / Filter.bmd / Dialog.bmd decode.
```

### Línea 2548 — antes de `char    DAT_07eaa1a0  = 0;   // UI message label A`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[580]. Ver el bloque de alias al final de globals.h.
// char    DAT_07d544d4  = 0;   // error string – case 0 wrong PIN
```

### Línea 2551 — antes de `char    DAT_07eaa198  = 0;   // UI message label B`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[581]. Ver el bloque de alias al final de globals.h.
// char    DAT_07d54600  = 0;   // error string – auth fail
```

### Línea 2554 — antes de `char    DAT_07eaa19c  = 0;   // UI message label C`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[600]. Ver el bloque de alias al final de globals.h.
// char    DAT_07d55c44  = 0;   // error string – case 0xfffffff8/0xfffffffe
```

### Línea 2563 — antes de `char    DAT_07ea51ec[8]  = {0};   // GuildName`

```cpp
// 2026-08-25: NO son "PIN entry" — esa etiqueta mentia. Son los buffers del
// editor de creacion de GUILD, y estaban declarados como escalares de 1-4 bytes
// mientras el codigo los recorre como arrays:
//   RenderGuildCreation lee `mark[gx + gy*8]` con gx,gy en 0..7 -> 64 bytes
//   sobre un `char`, o sea 63 bytes de desborde en CADA frame del editor.
// Layout del binario (contiguo, verificado contra el vecino DAT_07ea5240 que
// deja 75 bytes de espacio):
//   0x7EA51EC  GuildName[8]   (IDA lo lee como 2 DWORDs: +0 y +4)
//   0x7EA51F5  GuildMark[64]  (1 byte por celda de la grilla 8x8)
```

### Línea 2577 — antes de `short   DAT_00559f5a  = 0;   // second-password level check B`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[896]. Ver el bloque de alias al final de globals.h.
// char    DAT_07d6b724  = 0;   // Error message: "no item in slot"
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[854]. Ver el bloque de alias al final de globals.h.
// char    DAT_07d685ec  = 0;   // Error message: "invalid slot"
```

### Línea 2627 — antes de `double _DAT_005529d8  = 12.5;`

```cpp
// 2026-09-26: los tres son DOUBLES de 8 bytes en el binario y estaban
// declarados como float, o sea se leian los 4 bytes bajos de cada uno.
// Bytes reales (ida_get_bytes 0x5529D8, 32): la region intercala
//   0x5529D8 double 12.5 | 0x5529E0 float 260.0 | 0x5529E4 padding
//   0x5529E8 double 1/180 | 0x5529F0 double PI
// Los usa MoveEffect case 244 (Rageful Blow): el producto PI*(1/180) daba
// -1.8e12 en vez de 0.01745, asi que el seno del arco del arma era basura,
// y el 12.5 en 0 hacia que el test `v356 != 12.5` fuera SIEMPRE cierto -> el
// arma solo subia (+8/frame) y nunca bajaba.
```

### Línea 2729 en `HashFn_Sentinel` — antes de `// bBuxCode de BuxConvert_1 (0x004F6EB0) -- la copia que usa OpenTerrainAttribute.`

```cpp
                                  // 2026-08-23: estaba en 0 con el comentario
                                  // "never written in bin -> 0".  Nadie lo
                                  // escribe —un solo xref, la lectura en
                                  // RenderTerrain— pero es constante de .data y
                                  // en el binario vale 1 (ida_get_bytes
                                  // 0x0055A76C -> 01 00 00 00).  Con 0 el
                                  // overlay no se dibujaba en ningun mapa.
```

### Línea 2747 — antes de `// IDA: bBuxCode (0x0055A770)`

```cpp
// 2026-09-26: era UN solo byte, y la funcion indexa [i % 3] -- los otros dos
// salian de los globals vecinos en BSS.  Hoy son cero y por eso el XOR queda
// neutro, pero cualquier cambio de layout los volveria basura y romperia el
// terreno de golpe (el patron del diff de .map).  Ahora son tres bytes propios.
```

### Línea 2754 — antes de `_SkillAttrEntry SkillAttribute = {};   // skill attribute table base @ 0x07D29D20`

```cpp
// ── SkillAttribute table ──────────────────────────────────────────────────────
// DAT_07e118e8 (HeroTile) already defined as DWORD above (~line 495)
```

### Línea 2760 — antes de `char   DAT_0814b2e0[0x80000] = {};    // grass-wind/water-wave double buffer`

```cpp
// Grass-wind / water-wave ping-pong buffer: sub_4F98C0 (setup) y sub_4F9A30
// (smoothing) escriben `&DAT_0814b2e0 + 0x40000*toggle` → 2 buffers de 0x40000
// (256×256 DWORDs c/u) = 0x80000.  Antes era 1 char → 512KB de heap stomp en
// cada frame al wirear RenderTerrain (CLAUDE.md "hardcoded-address" pattern).
```

### Línea 2865 — antes de `// Batch 18 — InitGame / ReceiveChat globals`

```cpp
// m_dwTextColor / m_dwBackColor NO son globals separados: en IDA son EXACTAMENTE
// 0x559c78 / 0x559c80 (= DAT_00559c78 / SetBackgroundTextColor). Verificado por disasm
// (sub_40D610 @0x40D734: `mov [0x559c78], 0xffff9664`, y sub_480980 idéntico).
// Estaban declarados aparte → todo el código que setea m_dwTextColor (HUD_Pass1/2/3,
// ChatListBox render) escribía a un global que el render de texto (CUIRenderText_RenderText, lee
// DAT_00559c78) NUNCA leía → colores perdidos = texto blanco. Ahora son macros
// (globals.h) que apuntan al global real. Ver [[charselect-deferred-issues]].
// g_lpszMessageBoxCustom es un alias de DAT_083a44c4 (ver globals.h).
// m_hFontDC ahora es macro sobre DAT_055c9fec (ver globals.h)
// g_hFontBold es ahora un alias de DAT_055ca0xx (ver globals.h).
```

### Línea 2952 — antes de `// ── MoveParticles camera shake globals ──────────────────────────────────────`

```cpp
// 2026-09-07: era un buffer aparte; en realidad es GlobalText[474]. Ver el bloque de alias al final de globals.h.
// char   DAT_07d4c89c       = 0;     // "Not enough mana" message string
```

### Línea 3017 — antes de `char   DAT_07e016f8[100 * 0x254] = {};`

```cpp
// Chat bubble pool (base 0x07E016F8, stride 0x254, ~96 slots)
// ── Pool de burbujas de chat (CreateChat 0x481BA0 / MoveChat 0x4821A0) ───────
// En el binario: base `unk_7E016F8`, stride 596 (0x254), fin `unk_7E0FFC8`.
//   (0x7E0FFC8 - 0x7E016F8) / 596 = **100 slots**.
// `unk_7E01720` NO es otro pool: es base + 40 (el campo timer1), que es donde
// MoveChat arranca su walk. Por eso ahora es una macro (ver globals.h).
// Antes: DAT_07e016f8 era un char de 1 byte y DAT_07e01720 un array separado
// de 26 slots → CreateChat caminaba 59600 bytes sobre globals adyacentes.
```

### Línea 3043 — antes de `// g_hFont es ahora un alias de DAT_055ca0xx (ver globals.h).`

```cpp
// 2026-07-19: DAT_07e01924 era un TERCER buffer separado para el MISMO pool de
// burbujas. En el binario 0x7E01924 = 0x7E016F8 + 0x22C (campo disp1 del slot 0).
// CreateChat escribia en DAT_07e016f8 y RenderBooleans leia aca -> nunca se
// dibujaba nada. Ahora es una macro sobre el pool unico (ver globals.h).
```

### Línea 3062 — antes de `int g_PartyPanelScratchX = 0, g_PartyPanelScratchY = 0;`

```cpp
// 2026-07-27: scratch de coordenadas de los paneles Party / GuildCreation.
// Antes se guardaban en Inventory[32], que es el slot 0 del overlay del pool
// de la TIENDA → lo pisaban cada frame (tienda vacía intermitente).
```

## `src/globals.h`

### Línea 194 — antes de `extern BYTE (&DAT_00559050)[16];`

```cpp
// DAT_00559050 is retained only as a reference alias for stubs_IDA_ports.cpp,
// which is intentionally preserved as IDA infrastructure.
```

### Línea 304 — antes de `extern int             g_HasConnectServer;       // server.cfg tiene 2 líneas → flujo CS`

```cpp
// ── ConnectServer flow (2026-07-15) ──────────────────────────────────────────
```

### Línea 326 — antes de `extern DWORD   DAT_005615dc;`

```cpp
// DAT_005615dc @ 0x005615DC ES `g_iCurrentDialogScript` — el índice del diálogo
// de quest activo.  Lo escriben CSQuest::ShowDialogText (0x4017E7) y sub_51D840,
// y lo leen sub_401AF0 y UI_InGameMenu.  La etiqueta vieja ("login/scene misc
// flag") era una misidentificación.
// 2026-08-21: `g_iCurrentDialogScript` era un global APARTE en globals.cpp, así
// que el writer y el reader no se veían (mismo patrón que SetTextColor_0 /
// DAT_00559c7c).  Ahora es un alias.
```

### Línea 400 — antes de `#define DAT_00583d8c   g_csQuest`

```cpp
// ── Misc game globals (0x00583dxx – 0x00590xxx) ───────────────────────────────
// DAT_00583d8c @ 0x00583D8C ES `g_csQuest` — el PUNTERO al objeto CSQuest, no
// el objeto.  Lo confirman los xrefs: ReceiveQuestHistory / State / Result
// referencian 0x583D8C y el decompile los muestra como `g_csQuest`; el objeto
// es `unk_567500` (= DAT_00567500), que es a donde apunta.
// 2026-08-21: acá estaba declarado como un objeto de 0x1D000 bytes, o sea
// convivían DOS objetos CSQuest: éste (donde cargaba Quest.bmd) y DAT_00567500
// (al que apunta g_csQuest).  Todos los consumidores ya lo usan como puntero
// (`(uintptr_t)DAT_00583d8c + 0x1c87f`), así que el alias los arregla a todos.
```

### Línea 460 — antes de `extern int     DAT_055c9e58[100];`

```cpp
// RandomTable @ 0x055C9E58 — 100 enteros (`rand() % 360`) que siembra WinMain
// (0x41E8A0 L522-525).  Sólo lo usa Entity_Render (0x5038E0) para repartir en
// círculo las monedas del montón de Zen del suelo.
// 2026-08-21: estaba declarado como un único DWORD = 0 y nadie lo sembraba, así
// que los ángulos y radios salían todos 0 y las monedas se apilaban en un mismo
// punto (el "Zen sin sprite expandido").
```

### Línea 525 — antes de `#define DAT_05826d04 Teleport`

```cpp
// Teleport es 0x05826D14 (ida_xrefs_to: ReceiveTeleport, Attack, CheckGate,
// Skills_PacketHandler, sub_482BE0, sub_4D23B0...).  Hasta 2026-09-12 Attack
// escribia un global aparte (DAT_05826d04) y el resto leia Teleport: el
// gate del Town Portal no se enteraba del teleport en curso.
```

### Línea 538

```cpp
// 2026-09-17: eran dos globals; los usos escribian uno y los handlers el otro
```

### Línea 574

```cpp
// 2026-07-17: MAX_BONES=200 (era 0x1000=85, desbordaba al preview char)
```

### Línea 596 — antes de `extern char    DAT_07abf050[0x580];`

```cpp
// ── Preview character entity (0x07abf050) ─────────────────────────────────────
// BUG-FIX 2026-07-17: DAT_07abf050 ES el struct de entidad del PREVIEW char de
// char-select (creado por CreateCharacterPointer/CreateCharacterPointer con model 0xab).
// Es un entity struct COMPLETO (stride 0x394; el original lo espacia 0x580 hasta
// el array en 0x07abf5d0). Estaba declarado como un DWORD de 4 bytes, así que
// CreateCharacterPointer (que escribe hasta +908) desbordaba ~900 bytes sobre los
// globales BSS adyacentes → corrupción → type@+2 basura (16247) → crash al animar/
// renderizar el preview (MoveCharacter/RenderCharacter). Los símbolos _DAT_07abf05c
// … _DAT_07abf5cc eran CAMPOS de esta entidad mal-separados por Ghidra; ahora son
// macros que proyectan dentro del buffer. Ver globals.cpp.
```

### Línea 670 — antes de `extern char    g_PlayerRenderPool[100 * 0x1BC];`

```cpp
// ── Player render pool ─────────────────────────────────────────────────────────
// 2026-05-07: re-allocado propiamente. v1 walker de Player_Render arranca en
// DAT_07c74f54 y lee offsets NEGATIVOS hasta -0xEC. El pool REAL es
// g_PlayerRenderPool[100 × 0x1BC] cubriendo todos los slots; DAT_07c74f54 es
// pointer alias a offset +0xEC (donde v1 vive en cada slot iter).
```

### Línea 736 — antes de `#define DAT_07d780ac   (*(DWORD*)&DAT_07d780a8[4])`

```cpp
// InputLength[1] — el largo del campo 1 (destino del susurro).
//
// 2026-08-26: esto era un `DWORD` SEPARADO mientras que `WM_CHAR` lee el largo
// como `((DWORD*)DAT_07d780a8)[slot]`, o sea los bytes +4..+7 del array. En el
// binario las dos cosas son la misma memoria (0x07D780A8 es InputLength[10] y
// 0x07D780AC es su elemento 1); en el port eran dos, y los SEIS sitios que
// escriben el largo del susurro (historial de flechas, tab-complete, click
// derecho sobre un jugador, cierre del menu) le escribian a la copia huerfana
// — nadie la leia nunca.
//
// Sintoma: al usar las flechas del historial, el buffer se limpiaba pero el
// largo quedaba con el valor viejo, asi que lo que se tipeaba despues entraba
// detras de N bytes vacios y a los 10 caracteres se bloqueaba. Cerrar y
// reabrir el chat lo destrababa porque ese camino si resetea el array.
//
// El buffer companero (DAT_07db8810) ya se habia unificado como alias del slot
// 1; esto es la mitad que habia quedado afuera.
```

### Línea 889 — antes de `extern DWORD   FrustrumFaceD;   // IDA: DAT_07eeb200 (0x07EEB200)`

```cpp
                               // DAT_081cb608 @0x081CB608). Global muerto, sin
                               // usos desde 2026-08-23 — no reintroducirlo.
```

### Línea 903 — antes de `extern float   g_TilePickBuf[12];`

```cpp
// 2026-04-28: tile pick corners buffer (12 floats contiguous, 4 vec3 corners)
```

### Línea 934 — antes de `extern float   DAT_081cb608[256 * 256 * 3];`

```cpp
// 2026-05-04: buffer de iluminación por tile, vivo — dimensionado para 256×256 tiles × 3
// floats. Antes era un DWORD de 4 bytes; las macros de abajo proyectan DAT_081cb60c/610
// en los campos del 2do/3er slot, como el layout contiguo del binario original.
```

### Línea 978 — antes de `#define DAT_083a2378  (*(DWORD*)&DAT_083a2370[8])`

```cpp
// `Operates` de IDA = 0x083A2378 = DAT_083a2370 + 8, o sea el campo [2] (puntero
// al objeto) de la ENTRADA 0 de la lista de objetos interactuables.  Los
// consumidores lo indexan `Operates[3 * SelectedOperate]`, que salta de entrada
// en entrada (3 DWORDs = los 12 bytes de stride).
//
// 2026-09-04 FIX: estaba declarado como un DWORD SUELTO inicializado en 0 -- otra
// memoria distinta de la lista.  `((int*)&DAT_083a2378)[i*3]` leia entonces ese
// global y lo que le siguiera; para SelectedOperate == 0 devolvia 0 y
// `*(short*)(0 + 2)` crasheaba leyendo la direccion 2 (visto: CRASH addr=...
// param1=0x00000002 al pasar el mouse por una silla).
```

### Línea 991 — antes de `extern char    g_ObjectBucketGrid[0x1000];`

```cpp
// Object-bucket grid (see globals.cpp). 16×16 cells × 16 B = 0x1000.
// Cell layout (matches original binary 0x083a0218..0x083a1217):
//   cell+0  → alias DAT_083a0218 (base de la celda, también usada como scratch por el walker de descarga DeleteObjects)
//   cell+4  → DAT_083a021c  puntero head (Terrain_Render lo lee vía *chunk_ptr)
//   cell+8  → puntero tail  (el insert de CreateObject appendea acá; la descarga arranca el recorrido desde acá)
//   cell+12 → visibility flag (Terrain_Render writes *(chunk_ptr+8))
// IMPORTANTE: g_ObjectBucketGrid[0] representa la dirección 0x083a0218, así que DAT_083a0218 está
// en el offset 0 y DAT_083a021c en el +4. Antes DAT_083a021c estaba en el offset 0
// y DAT_083a0218 era un DWORD aparte — eso hacía que la descarga (`puVar5 = &DAT_083a0218;
// while (head = *(puVar5+8)) ...`) leyera BSS sin inicializar adyacente al DWORD huérfano,
// y crasheara con el primer valor basura no nulo.
```

### Línea 1107 — antes de `#define IDA_PORT_004FDC00 1`

```cpp
// ── FUN_004fdc00 (Object_RenderUpdate) full IDA port activation ──────────────
// Activa el port completo de sub_4FDC00 en stubs_IDA_ports.cpp y desactiva el
// minimal de stubs_linker.cpp. Aliases IDA→DAT que el port full necesita.
```

### Línea 1112 — antes de `#define IDA_PORT_00445230 1`

```cpp
// ── AttackEffect full IDA port activation ─────────────────────
// 2026-08-16: `AttackEffect` es la que spawnea los efectos VISUALES de los
// skills (CreateEffect 191/200/201/223/240/241/568/1210/1211/1271, CreateJoint
// 1253...). El port fiel de IDA (2043 lineas) vivia en stubs_IDA_ports.cpp pero
// su gate nunca se definio, asi que se compilaba una version PARCIAL de 374
// lineas en stubs_misc2.cpp -> Lightning no mostraba nada y Evil Spirit /
// Inferno salian incompletos. Activado el port completo.
// El ruido anti-tamper crudo (CErrorReport::Write/aHashTableFullG, hash table
// FUN_004041e0/HashTable_Insert con otras firmas, delete__, PACKET_ENCRYPT, `Models`)
// quedo neutralizado con shims locales al inicio del bloque gated en
// stubs_IDA_ports.cpp (con sus #undef al final). Ver CLAUDE.md 2026-08-16.
```

### Línea 1224 — antes de `#define DAT_083a44ea   (DAT_083a44c4[0x26])`

```cpp
// ── UI name-list panel data (ShowCheckBox) ────────────────────────────────────
// DAT_083a430c  — macro alias dentro de DAT_083a42f8 (ver bloque de dialog button rects)
// 2026-05-08: alias por macro que proyecta en el offset +0x26 (line[1]) dentro del
// properly-sized DAT_083a44c4 buffer (g_lpszMessageBoxCustom).
```

### Línea 1260 — antes de `extern char    DAT_07c80110[100 * 0x70];   // active-flag at +0; slot stride 0x70`

```cpp
// ── Teleport-anim pool (0x07c80110, stride 0x70, 100 slots) ──────────────────
// Sized via IDA bound: (0x7c82cd0 - 0x7c80110) / 0x70 = 0x2BC0 / 0x70 = 100.
// Antes era `extern char DAT_07c80110;` (1 byte) — Entity_TeleportAnim en stubs.cpp:3324
// recorría los 100 slots escribiendo 0x70 bytes por paso → corrupción de heap en el primer
// teleport. Now properly sized.
```

### Línea 1307 — antes de `extern "C" BYTE Inventory[];`

```cpp
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
```

### Línea 1367 — antes de `extern char    DAT_07df9380[0x77 * 0x118];`

```cpp
// ── Chat ring buffer (UI_RenderChatLogOverlay renderer / UIChatLogWindow_AddText) ──────
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
```

### Línea 1408 — antes de `#define DAT_07e108c8   (MacroText[0x900])`

```cpp
// Fila 9 de la tabla de arriba (0x07E0FFC8 + 0x900 = 0x07E108C8), no un global
// aparte: es el buffer del destinatario de susurro y lo llenan FUN_00494520,
// strcmp/strlen/memcpy.  2026-08-22: estaba declarado como UN char, o sea
// escribia 255 bytes sobre los globals vecinos.
```

### Línea 1435 — antes de `#define ServerDivisionOpened   (*(char*)&g_bServerDivisionEnable)`

```cpp
// B-key toggle guards
// 0x07EAA130 ES g_bServerDivisionEnable (confirmado por xrefs de IDA:
// ReceiveTalk, SendMove, sub_492F10, Chat_InputTick, GetScreenWidth,
// RenderServerDivision).  2026-08-22: estaba partido en dos variables —
// el writer era g_bServerDivisionEnable (Net_Process, ReceiveTalk sub 5) y
// los readers ServerDivisionOpened, que nadie seteaba nunca.  Por eso el panel de
// division de servidor no se dibujaba y GetScreenWidth no lo contaba.
// El byte 1 de ese int es 0x07EAA131, que ya se usa asi en stubs_externs.
```

### Línea 1456 — antes de `extern int     DAT_00559cdc;           // system-message scroll timer (reset to 300 on Cha`

```cpp
// ── Timers de cuenta regresiva de chat/UI (los usan Sound_Queue.cpp + stubs.cpp) ──
```

### Línea 1472 — antes de `extern "C" {`

```cpp
// 2026-05-04: server-config globals (popullados por opcodes 0xDD/DE/DF).
```

### Línea 1518 — antes de `extern DWORD   DAT_083a42f8[10];       // UI panel descriptor array (set by guild funcs)`

```cpp
// Campos del sub-estado de login (también los escribe la rama 3 del guild 0x94):
// InputTextMax — declared above as float (line 202)
// DAT_00559c84  — declared above as DWORD (line 198)
// InputNumber  — declared above as DWORD (line 199)
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
```

### Línea 1549 — antes de `extern char    LockInputStatus; // IDA: DAT_07e11d6f (0x07E11D6F)`

```cpp
// ── Declaraciones perdidas al restaurar globals.h desde git (2026-09-03) ──────
// Estos globals ya existian en globals.cpp; sus `extern` estaban entre los
// cambios sin commitear del header.
```

### Línea 1560 — antes de `#define GUILD_MEMBER_TABLE_BYTES  0x108`

```cpp
// Tabla de miembros de guild: 11 registros de 0x18 bytes (0x083A7AF8..0x083A7C00).
// 2026-09-03: eran SEIS escalares sueltos (24 bytes = una sola entrada) mientras
// `GuildMemberList_Set` copia `count * 0x18` bytes y el render lee
// `base + iMod*0x18`; con mas de un miembro se escribia sobre los globals vecinos.
```

### Línea 1614 — antes de `extern char    DAT_083a1218[0x1158];   // Butterfles OBJECT array (10 entries × 0x1BC stri`

```cpp
// Sound emitter pool base (0x083a1218) — referenced in stubs.cpp Sound_SpawnEmitter:
```

### Línea 1944 — antes de `extern char    g_WeatherSlotPool[40 * 0x1bc];`

```cpp
// ── Weather particle pool (40 slots × 0x1bc bytes = 0x4560 bytes) ────────────
// BUG-FIX 2026-05-04: antes los 30+ globals DAT_0839bc?? eran chars sueltos
// en BSS, pero Weather_Update y Particle_PathUpdate los acceden con stride 0x1bc
// (slot stride) o 0x6f (int stride = 0x1bc/4). Sin un buffer contiguo, escribir
// a slot 1+ corrompe globals adyacentes; leer slot 1+ leía garbage o causaba AV
// (visible como crash en RenderNumArrow al entrar al mundo, addr=0x004BF712,
// param1=0x0839BCB0). Ahora ALL son macros que indexan dentro de un único
// buffer contiguo.
```

### Línea 2064 — antes de `#define DAT_07ea5298   (*(BYTE(*)[0x880])(Inventory))`

```cpp
// Alias de CAMPO sobre los pools de items (verificado en el desensamblado de
// CloseInventoryRelatedWindows 0x4CBD36-0x4CBD9C y los errores de
// ida_get_function): en el binario no son copias sino el mismo pool abordado
// desde otro campo.  Los bucles originales escriben Type en `ptr - 0x38` y
// Key en `ptr`, o sea DAT_x + 0x38 = Key del slot 0.  Antes eran arrays
// propios: todo lo que se escribia ahi no llegaba a los pools reales.
//   0x07EA5298 Inventory              0x07EA52D0 Inventory.Key
//   0x07EA7B88 OffsetTradeItems       0x07EA7BC0 OffsetTradeItems.Key
//   0x07EA9880 OffsetMixItems.Key     0x07EA8448 OffsetInventoryItems.Key
//   0x07EA5B68 Key del pool de 0x07EA5B30 (baul; en IDA tambien la tienda)
//   0x07E11FB0 Key de word_7E11F78 (trade del otro jugador)
// Son lvalues de array: `&`, la aritmetica y el decay a BYTE* funcionan igual.
```

### Línea 2114 — antes de `// ── Scene_Resources string literals ──────────────────────────────────────────`

```cpp
// 2026-05-04: cb60c / 0828b60c / cb610 / 0828b610 NO son globals separados —
// son el 2do/3er DWORD del slot 0 de cb608 / 0828b608. Las macros están
// definidas junto a las declaraciones de cb608 / 0828b608, más arriba en este header.
```

### Línea 2243 — antes de `#define DAT_07d29d24 (GlobalText[0][0])`

```cpp
// BUG-FIX 2026-07-17: DAT_07d4b4b0/5dc son GlobalText[457]/[458] (name-filter blocked
// words, cargados de Text.bmd). Estaban como chars sueltos =0 (string vacío) → FindText
// devolvía 1 → nombres rechazados. `&DAT_07d4b4b0` ahora = GlobalText[457].
// DAT_07d29d24: base de la tabla de nombres de clase, recorrida con
// `&DAT_07d29d24 + i * 300`.  Es una fila de GlobalText -- sus dos lectores
// (UI_StatsPanel y SecondPassword) usan indices ~601-607, que en Text.bmd son
// los nombres de clase.  Estaba declarada como un `char` suelto, asi que esas
// lecturas se iban ~180 KB fuera del global y terminaban en lstrlenA.
```

### Línea 2289 — antes de `#define DIALOG_SCRIPT_COUNT   200`

```cpp
//
// 2026-08-21: los cuatro campos estaban como globals ESCALARES sueltos y los
// consumidores hacían `&DAT_07cf5734 + idx * 0x400` sobre punteros tipados
// (paso 4x) — lecturas fuera de rango garantizadas.  Ahora hay una sola tabla.
```

### Línea 2371 — antes de `extern char    DAT_07ea51ec[8];    // GuildName`

```cpp
// 2026-08-25: buffers del editor de creacion de GUILD (la etiqueta "PIN entry"
// era falsa). Estaban como escalares y el render los recorre como arrays —
// 64 bytes de mark sobre un `char`. Ver la nota en globals.cpp.
```

### Línea 2385 — antes de `#define DAT_00559f60   (m_iDevilSquareLimitLevel[0][0])`

```cpp
// 2026-09-07: DAT_00559f60 / DAT_00559f64 SON m_iDevilSquareLimitLevel.
// Verificado con ida_get_function: m_iDevilSquareLimitLevel = 0x00559F60 y
// m_iBloodCastleLimitLevel = 0x00559F80 (32 bytes despues = 4 niveles x 2 int).
// Estaban partidos en dos: el handler del 0x8E llenaba el array C y
// `SecondPassword_Screen5` leia `(&DAT_00559f60)[i*2]`, un int suelto -> el chequeo de
// nivel del Devil Square comparaba contra basura de los globals vecinos.
```

### Línea 2519 — antes de `extern BYTE  OffsetInventoryItems[];`

```cpp
// ── Pools de items y atributos de terreno ────────────────────────────────────
// Centralizadas acá por el refactor B3 (2026-08-16). Antes cada .cpp las
// redeclaraba con su propio `extern`, y eso rompia el movimiento de funciones
// entre modulos: la funcion movida dejaba de ver la global de su archivo.
//
// Los cuatro pools de items son grids de slots ITEM (stride 0x44). Ojo: su
// indice de celda es `slot - 12`; los 12 wear slots NO viven aca sino en
// `CharacterMachine + 536 + 68*slot` (ver la entrada de 2026-08-08 g).
```

### Línea 2537 — antes de `struct _SkillAttrEntry { char Raw[2560]; };`

```cpp
// 2026-08-21: antes era un único bloque de 300 bytes, así que cualquier índice
// leía fuera del objeto.
```

### Línea 2557 — antes de `extern BYTE    DAT_00567500[0x1C900];   // Quest table base (was DWORD)`

```cpp
// ── Small-function batch globals ─────────────────────────────────────────────
// Base de la tabla de quests — IDA la trata como un buffer de ~0x1C900 bytes. El código en
// stubs.cpp escribe en &DAT_00567500 + 0x1C8F8..+0x1C8FD (líneas 26586-26589)
// y HUD_Pass2:GetScreenWidth lee `g_csQuest + 0x1C8FF`. Un DWORD de 4 bytes
// acá significa que esos accesos caen más allá del final de nuestros globals →
// AVs aleatorios. Lo exponemos como array de BYTE del tamaño correcto.
```

### Línea 2614 en `ItemAttribute_Base` — antes de `extern DWORD   g_csQuest;         // Quest system state (0=inactive)`

```cpp
                                        // recibe PreInitNPGameMon: "Mu". NO es un nombre de
                                        // ventana: el cliente no crea ninguna ventana de
                                        // GameGuard (ver CLAUDE.md, seccion GameGuard).
```

### Línea 2668 — antes de `#define _g_bEventChipDialogEnable (*(int*)&GoldenArcherOpenType)`

```cpp
// Quest/NPC window
// g_bEventChipDialogEnable es 0x07EAA128 (GoldenArcherOpenType).  Hasta 2026-09-11 era
// un global aparte: el 0x94 lo escribia y el panel del Golden Archer leia
// GoldenArcherOpenType, asi que nunca se enteraba.
```

### Línea 2674 — antes de `#define InventoryOpened    DAT_07eaa117`

```cpp
// 2026-04-30: los flags de los paneles de UI ahora aliasan los bytes reales DAT_07eaa11x (per
// el Offsets.h del proyecto companion de IDA, líneas 59-69). Las direcciones de la época de
// Ghidra 0x07e5ba84/88 para InventoryOpened/CharacterOpened eran misidentificaciones — los
// flags reales están en DAT_07eaa116..11c (de un byte). Usamos #define para que tanto
// el código de toggle portado de IDA (escribe DAT_07eaa117) como los gates
// de render del HUD (leen `if (InventoryOpened)`) peguen en el mismo byte de memoria.
```

### Línea 2691 — antes de `#define InventoryStartX      DAT_07ea5288`

```cpp
// 2026-04-30: Inventory/Trade panel origin coords. Same unification pattern
// que los flags *Opened de arriba — IDA escribe/lee vía DAT_07ea5284..5290 y
// los pases de render del HUD usan los nombres de C++. Forzarlos a la misma memoria
// fixes the "panel right, items left" misalignment.
```

### Línea 2718 — antes de `#define g_lpszMessageBoxCustom  ((char (*)[0x26])DAT_083a44c4)`

```cpp
// g_iNumAnswer — alias de DAT_083a7c0c (ver arriba).
// g_iNumLineMessageBoxCustom — alias de DAT_083a4324 (ver arriba).
// g_lpszMessageBoxCustom — alias de DAT_083a44c4 (el buffer real de 7 lineas x
// 0x26).  2026-08-21: era un array de 16 PUNTEROS en NULL, o sea los writers
// (ShowDialogText, CreateOkMessageBox) llenaban DAT_083a44c4 y los readers
// (sub_402FF0) leian punteros nulos → el texto del dialogo de quest nunca se
// dibujaba.  Sexto global partido en dos de este subsistema.
```

### Línea 2726 — antes de `#define m_hFontDC  DAT_055c9fec`

```cpp
// g_iCurrentDialogScript — alias de DAT_005615dc (ver arriba).
// g_lpszDialogAnswer — alias de DAT_083a4348 (ver arriba).
// 2026-07-19: m_hFontDC NO es un global aparte — en IDA sub_50F5F0 hace
// `m_hFontDC = CreateCompatibleDC(hdc)` y ese mismo DC es DAT_055c9fec (el font
// memory DC, declarado en stdafx.h). Tenerlos separados dejaba m_hFontDC en NULL
// para siempre (125 usos, 0 asignaciones) -> GetTextExtentPoint32A fallaba.
```

### Línea 2733 — antes de `#define g_hFontBold  ((HFONT)(uintptr_t)DAT_055ca010)`

```cpp
// 2026-09-04 FIX -- las tres fuentes estaban PARTIDAS EN DOS.  WinMain crea los
// handles en DAT_055ca00c / 010 / 014 (normal / bold / big, esta ultima al doble
// de altura), pero `g_hFont` y `g_hFontBold` estaban declaradas como HFONT
// APARTE que nadie asignaba -- quedaban en NULL, asi que los ~80
// `SelectObject(m_hFontDC, g_hFontBold)` del arbol no cambiaban de fuente.
// IDA: g_hFont = 0x055CA00C, g_hFontBold = 0x055CA010, g_hFontBig = 0x055CA014.
```

### Línea 2901 — antes de `extern int     PartyNumber;          // 0x07EAA0E0 — count of valid party slots`

```cpp
// ── HUD render globals (Phase-2 port) ────────────────────────────────────────
// Agregados el 2026-04-29 para el port de RenderPartyHP / RenderMainFrameWindow /
// RenderBooleans / Render_HotbarItems3D from IDA.
//
// Estado de party / soccer / guild war — binding mínimo para que los renderers
// sigan funcionando antes de que se porten los sistemas completos de party/guild.
// Los valores por defecto en 0 hacen que todos esos caminos tomen sus ramas de
// salida temprana, así que el juego corre igual con los ports incompletos.
```

### Línea 2951 — antes de `#define DAT_07d329c4       (GlobalText[120][0])`

```cpp
// ─── Filas de GlobalText que el port habia partido en globals sueltos ────────
// 2026-09-07.  El pool de textos vive en `GlobalText[1000][300]` con base
// 0x07D29D24 (verificado: 0x07D4B4B0 == GlobalText[457]).  Estos simbolos caen
// EXACTAMENTE en multiplos de 300 desde esa base, o sea son filas del pool, no
// buffers propios.  Estaban declarados como `char`/`BYTE` sueltos (1 byte) y
// nadie los llenaba, asi que todo texto que pasara por ellos salia VACIO.
//
// Sintoma que lo destapo: el cartel del Devil Square salia sin texto.  La sonda
// OKBOX mostro `CreateOkMessageBox` recibiendo "" desde sub_4E6C40, que en IDA
// llama con GlobalText[677] / [686] / [687] / [854].
//
// Mismo patron que DAT_081cb60c: un macro que proyecta dentro del array real,
// asi `&DAT_x` sigue siendo un `char*` a la fila.
```

## `src/resource.h`

### Línea 3 — antes de `#define IDI_MAIN_ICON   101`

```cpp
// IDs de los recursos de src/resource.rc.
//
// IDI_MAIN_ICON TIENE que estar #definido aca: un simbolo sin definir en un
// .rc no es un error, el compilador de recursos lo toma como NOMBRE DE CADENA
// y el grupo de icono termina llamandose "IDI_MAIN_ICON" en vez de tener un
// ordinal.  Asi estaba hasta 2026-09-27, y por eso LoadIcon con
// MAKEINTRESOURCE devolvia NULL (busca ordinales) y la ventana salia con el
// icono generico.  Se puede comprobar en el .exe ya linkeado: el directorio
// de recursos muestra RT_GROUP_ICON -> "IDI_MAIN_ICON" en vez de -> #101.
```

## `src/stdafx.h`

### Línea 18 — antes de `#ifdef _DEBUG`

```cpp
// CRT debug heap — enable BEFORE stdlib.h to track all malloc/new sites
// 2026-05-03: investigando crash heap corruption (addr 0x0054B54C, 0x005488CC).
// _CRTDBG_MAP_ALLOC redirige new/malloc al debug heap con file/line tracking.
// _CRTDBG_CHECK_ALWAYS_DF (set en WinMain) hace que CADA alloc valide TODO el
// heap antes/después → captura la corrupción en cuanto pasa.
```

## `src/structs.h`

### Línea 431 — antes de `#define ServerListCount     DAT_083a7c40     // BYTE (use as char)`

```cpp
// 2026-08-22: `ServerNumber` es un nombre INVENTADO del port y describe mal el
// campo.  `0x083A7C40` es la **cantidad de servidores** que trajo el F4/02
// (`ReceiveServerList` L15: `unk_83A7C40 = ReceiveBuffer[5]`), y `Game_SceneUpdate`
// lo pone en 0 al entrar al login.  Nadie lo LEE — se guarda y no se consume.
//
// El "numero de server" que decide con que nombre/color se dibuja cada rectangulo
// del select-server es OTRA cosa: es `group = ServerCode / 20` (0..N), que indexa
// la tabla de nombres `DAT_07d52c34` (stride 300) — ver `Recv_ServerList`.  El
// grupo 12 es un caso especial: usa `GlobalText[559]` y va al slot 24.
```

### Línea 465 — antes de `#define PrimaryTerrainLight  ((float(*)[3])&DAT_081cb608[0])  // float[256*256][3]`

```cpp
// 2026-08-23 FIX: esto apuntaba a `DAT_07eab250`, que es un DWORD de 4 bytes.
// El propio `globals.h:837` ya documentaba el mislabel ("NO es
// PrimaryTerrainLight (ese es DAT_081cb608)") pero el macro nunca se corrigio.
// Consecuencias, las dos graves:
//   1. Los 23 call sites de `AddTerrainLight(..., PrimaryTerrainLight[0])`
//      escribian la luz dinamica en un global muerto, asi que el fuego, las
//      antorchas y los efectos NUNCA iluminaban — el render lee DAT_081cb608.
//   2. `AddTerrainLight` indexa `Buffer[768*y + 3*x]` con x,y hasta 255, o sea
//      escribia hasta ~786 KB pasado un DWORD: desborde masivo sobre BSS.
// Y como nadie resetea el buffer muerto, la luz se acumulaba sin techo (medido:
// el tile del fuego llego a 64.0 y subiendo). `Terrain_Water` (0x4F95E0) si
// resetea DAT_081cb608 por frame, que es lo que acota la acumulacion.
```

### Línea 478 — antes de `#define VectorRotate         Vector_Rotate`

```cpp
// Functions (map companion-project names → FUN_ addresses from functions.h):
// 2026-09-26 FIX (origen del Aqua Beam detras del pj): este alias apuntaba a
// Vector_InverseRotate, que es el port de VectorIRotate (0x4FA110) -- la
// TRANSPUESTA, o sea la rotacion INVERSA.  El VectorRotate del binario es
// 0x4FA0B0 y es row-major:
//     0x4FA0B0 VectorRotate   out[i] = m[4i+0]*x + m[4i+1]*y + m[4i+2]*z
//     0x4FA110 VectorIRotate  out[i] = m[i]*x + m[4+i]*y + m[8+i]*z
// Con el alias mal, todo offset calculado con VectorRotate se rotaba por -yaw:
// sub_4451C0 (el origen del Aqua Beam, MoveCharacter case 12) lo ponia espejado
// respecto del frente del personaje.  Los otros 113 call sites del arbol llaman
// Vector_Rotate directo y por eso siempre estuvieron bien.
```

### Línea 490 — antes de `void __cdecl AddTerrainLight(float xf, float yf, float *Light, int Range, float *Buffer); `

```cpp
// 2026-08-23 FIX (el fuego no iluminaba): esto aliaseaba `AddTerrainLight` a
// `AddTerrainLightClip`, que es OTRA funcion del binario.
//   AddTerrainLight     0x004F76C0  sin clamp superior  · decenas de callers
//   AddTerrainLightClip 0x004F7800  clampea a 1.0       · UN caller (0x4C0E59)
// Con el alias, toda la luz dinamica (fuego, antorchas, efectos) quedaba cortada
// en 1.0 y no llegaba a saturar — de ahi que el fuego se dibujara pero sin
// resplandor.  El port correcto de 0x4F76C0 ya existia como `AddTerrainLight`
// (Render/SMD_Parser.cpp), mal etiquetado en functions.h como "Terrain_SetHeight
// or similar"; ese nombre inventado es lo que llevo a crear este alias.
//
```

## Retoques sueltos (referencias a números de línea y a archivos)

Comentarios de una línea corregidos fuera de los bloques de arriba: se quitó la
referencia a un número de línea que ya no corresponde, o se corrigió el archivo o
el tipo nombrado. La línea indica dónde estaba en `fase/1`.

### `src/globals.cpp`, línea 1658

```cpp
// DAT_07eaa117 — defined above (char, line 537)
```

### `src/globals.cpp`, línea 1659

```cpp
// DAT_07eaa116 — defined above (char, line 536)
```

### `src/globals.cpp`, línea 1660

```cpp
// GoldenArcherOpenType — defined above (DWORD, line 544)
```

### `src/globals.cpp`, línea 1662

```cpp
// StorageGoldFlag — defined above (DWORD, line 894)
```

### `src/globals.cpp`, línea 1683

```cpp
// InputTextMax — defined above (float, line 184)
```

### `src/globals.cpp`, línea 1684

```cpp
// DAT_00559c84  — defined above (DWORD, line 180)
```

### `src/globals.cpp`, línea 1685

```cpp
// InputNumber  — defined above (DWORD, line 181)
```

### `src/globals.cpp`, línea 1687

```cpp
// DAT_083a4324  — defined above (DWORD, line 882)
```

### `src/globals.cpp`, línea 1688

```cpp
// DAT_083a44c4  — defined above (DWORD, line 883)
```

### `src/globals.cpp`, línea 1690

```cpp
// DAT_083a7c24  — defined above (DWORD, line 689)
```

### `src/globals.cpp`, línea 1691

```cpp
// DAT_083a7c28  — defined above (DWORD, line 690)
```

### `src/globals.cpp`, línea 1785

```cpp
// DAT_00559c78 — defined above (DWORD, line 177); using 0xffffffff as initial value there
```

### `src/globals.cpp`, línea 1786

```cpp
// SetBackgroundTextColor — defined above (DWORD, line 179)
```

### `src/globals.cpp`, línea 1787

```cpp
// DAT_00559c8c — defined above (DWORD, line 182)
```

### `src/globals.cpp`, línea 1788

```cpp
// m_bAutoAttack — defined above (char, line 858)
```

### `src/globals.cpp`, línea 1811

```cpp
// DAT_083a7c08 — defined above (DWORD, line 877)
```

### `src/globals.cpp`, línea 1812

```cpp
// DAT_083a7c09 — defined above (char, line 879)
```

### `src/globals.cpp`, línea 1813

```cpp
// DAT_083a7c0c — defined above (DWORD, line 880)
```

### `src/globals.cpp`, línea 1814

```cpp
// DAT_083a4124 — defined above (DWORD, line 634)
```

### `src/globals.cpp`, línea 1815

```cpp
// _DAT_00552cac — defined above (float, line 141)
```

### `src/globals.cpp`, línea 2145

```cpp
// _DAT_00552890 — defined above (float, line 80)
```

### `src/globals.cpp`, línea 2309

```cpp
// DAT_07e016f0 — same address as _DAT_07e016f0 above (line 997); alias defined in globals.h
```

### `src/globals.cpp`, línea 2398

```cpp
// ── GameGuard globals (GameGuard_Init2.cpp) ───────────────────────────────────
```

### `src/globals.cpp`, línea 2404

```cpp
// DAT_083bbb14 — defined above (DWORD, line 725)
```

### `src/globals.cpp`, línea 2587

```cpp
// DAT_07ea7b88 — declared above as DWORD (line 1404)
```

### `src/globals.h`, línea 1237

```cpp
// DAT_05826d31 — declared above (line 416)
```

### `src/globals.h`, línea 1498

```cpp
// DAT_07eaa117 — declared above as char (line 613)
```

### `src/globals.h`, línea 1499

```cpp
// DAT_07eaa116 — declared above as char (line 612)
```

### `src/globals.h`, línea 1500

```cpp
// GoldenArcherOpenType — declared above as DWORD (line 620)
```

### `src/globals.h`, línea 1502

```cpp
// StorageGoldFlag — guild UI flag — declared above as DWORD (line 609)
```

### `src/globals.h`, línea 1543

```cpp
// DAT_083a7c24  — declared above as DWORD (line 780)
```

### `src/globals.h`, línea 1544

```cpp
// DAT_083a7c28  — declared above as DWORD (line 781)
```

### `src/globals.h`, línea 1669

```cpp
// SetBackgroundTextColor — declared above as DWORD (line 197)
```

### `src/globals.h`, línea 1670

```cpp
// DAT_00559c8c — declared above as DWORD (line 200)
```

### `src/globals.h`, línea 1698

```cpp
// DAT_083a7c08 — declared above as DWORD (line 772)
```

### `src/globals.h`, línea 1700

```cpp
// DAT_083a7c0c — declared above as DWORD (line 774)
```

### `src/globals.h`, línea 1702

```cpp
// DAT_083a4124 — declared above as DWORD (line 713)
```

### `src/globals.h`, línea 2393

```cpp
// DAT_07ea7b88 — declared above as DWORD (line 1474)
```

### `src/globals.h`, línea 2605

```cpp
extern DWORD   DAT_07e11e50;       // _PartyNumber (already in line 574 - reuse)
```

### `src/functions.h`, línea 101

```cpp
// Packet_DecryptDword signature — canonical (void*,void*) at line 25 above
```

### `src/functions.h`, línea 868

```cpp
// OpenModel — same as Monster_RegisterBMD above (int first arg), see line 653
```

### `src/functions.h`, línea 983

```cpp
// void  __fastcall FUN_00412510(void *This);                           // GG module deinit — duplicate, correct DWORD* version at line 1276
```

### `src/functions.h`, línea 984

```cpp
// void  __fastcall FUN_00412610(void *This);                           // GG module2 deinit — duplicate, correct DWORD* version at line 1277
```

### `src/functions.h`, línea 1063

```cpp
// int   __fastcall FUN_00409f30(void *This, int p1, int p2, int p3, char p4); // open file — duplicate, correct 6-param version at line 1156
```

### `src/functions.h`, línea 1064

```cpp
// void  __fastcall FUN_0040a300(void *This, int p1);                   // cleanup after open — duplicate, correct 3-param version at line 1159
```

### `src/functions.h`, línea 1299

```cpp
// LoadWaveFile declared above (line 1088) with real signature — real impl in src/Sound/Sound.cpp.
```

### `src/functions.h`, línea 1302

```cpp
// InventoryColor = InventoryColor (declared at line 1301)
```

### `src/functions.h`, línea 1303

```cpp
// RenderEquipmentPart3D = RenderEquipmentPart3D (declared at line 1303)
```

### `src/functions.h`, línea 1305

```cpp
// DisableAlphaBlend = GL_ResetState (declared at line 578)
```

### `src/functions.h`, línea 1308

```cpp
// RenderBitmap = GL_DrawTexture (declared at line 601)
```

### `src/functions.h`, línea 1327

```cpp
// CreateAngle (already declared line 207; Ghidra shows 4 floats → float — cast in caller)
```
