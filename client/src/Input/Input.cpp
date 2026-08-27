// Input.cpp
// Sistema de input del cliente — análisis completo
#include "stdafx.h"
#include "Input/Input.h"

extern "C" { void DbgLogPublic(const char* msg); }
//
// Cubre: mouse (WM_MOUSE*), teclado (GetAsyncKeyState + WM_CHAR), IME/DBCS,
//        tabla de edge-detection de teclas, y la función de hit-test del numpad.
//
// NOTA: El WndProc (0x004149D0) tiene 4074 líneas (~80% HashTable obfuscation).
//       La lógica real de input representa solo ~10% del código, el resto es anti-tamper.
//
// ─────────────────────────────────────────────────────────────────────────────
// GLOBALS DE ESTADO DEL INPUT
// ─────────────────────────────────────────────────────────────────────────────
//
//   DAT_083a427c  g_MouseX             Posición X del mouse (0..639, normalizada a 640×480)
//   DAT_083a4278  g_MouseY             Posición Y del mouse (0..479)
//   DAT_083a4124  g_ClickFlag          1 = LButton pressed (cleared on release; no se setea si DblClick pending)
//   DAT_083a413c  g_DialogClose        1 = LButton released (set on WM_LBUTTONUP)
//                                      Se limpia si mouse se movió desde el down (055ca03c/40)
//   DAT_083a42c4  g_DblClickPending    1 = LButton held/down (previene double-fire de g_ClickFlag)
//   DAT_083a4299  g_DblClickFlag       1 = WM_LBUTTONDBLCLK (doble click)
//   DAT_055ca03c  g_MouseX_at_down     X del mouse cuando se presionó LButton (para detectar drag)
//   DAT_055ca040  g_MouseY_at_down     Y del mouse cuando se presionó LButton
//   DAT_055ca019  g_IME_Active         1 = IME en modo composición (WM_IME_START/ENDCOMPOSITION)
//
//   Nota: g_DialogClose se usa como "hubo un click sin drag" — las escenas lo leen
//   para confirmar selecciones (diferente de g_ClickFlag que es "está presionado ahora").
//
// ─────────────────────────────────────────────────────────────────────────────
// GLOBALS DEL BUFFER DE TEXTO
// ─────────────────────────────────────────────────────────────────────────────
//
//   DAT_07db8710  g_TextBuf[0]         Buffer de texto slot 0 (username/chat, 256 bytes)
//   DAT_07db8810  g_TextBuf[1]         Buffer de texto slot 1 (password, 256 bytes)
//   (fórmula: g_TextBuf_base + slot * 0x100)
//
//   DAT_07d780a8  g_TextLen[N]         int[N] — longitud actual de cada slot
//   DAT_00559c94  g_TextMaxLen[N]      int[N] — longitud máxima permitida por slot (de config)
//   DAT_07e11d78  g_ActiveSlot         Slot activo de input (Tab para rotar)
//
//   Flags de modo de input (todos en WM_CHAR guard):
//   DAT_00559c84  g_TextMode           General text input active (login screen / chat)
//   DAT_07e11d70  g_ChatMode           Chat input mode
//   DAT_07e11d71  g_IME_Mode           Korean/DBCS IME input active
//   DAT_07e11d72  g_DigitOnly          Modo solo dígitos (0-9) — PIN/second password
//   DAT_07e11d73  g_UppercaseOnly      Modo solo mayúsculas
//   DAT_083a7c24  g_UIScene            Si == 0x7E o 0x98: texto activo aunque otras flags == 0
//
//   Para Korean DBCS (WM_IME_COMPOSITION, 0x10F):
//   DAT_07e11cec  g_DBCS_Hi[slot*4]    Byte alto del carácter DBCS
//   DAT_07e11ced  g_DBCS_Lo[slot*4]    Byte bajo del carácter DBCS
//   DAT_07e11cee  g_DBCS_Null[slot*4]  Siempre 0 (null terminator)
//
//   DAT_055ca031  g_TextBufNearFull    1 = buffer a 1 byte del máximo
//   DAT_055ca034  g_DBCS_Counter       2=esperando trail byte, 1=byte suelto, 0=ASCII
//
// ─────────────────────────────────────────────────────────────────────────────
// TABLA DE EDGE-DETECTION DE TECLADO
// ─────────────────────────────────────────────────────────────────────────────
//
//   DAT_07e118ec  g_KeyState[256]      int[256] — estado de "ya procesé esta tecla"
//                 Indexado por VK code (0..255), 4 bytes cada entrada.
//                 0 = tecla libre o ya procesada
//                 1 = tecla presionada y aún no procesada (rising edge)
//
// 0x0047ec20  Key_IsJustPressed(int vk) → bool
//   Wrapper de GetAsyncKeyState con edge-detection por tecla.
//
//   GetAsyncKeyState(vk):
//     High bit set (0x8000) → tecla actualmente presionada
//     Low bit set           → fue presionada desde la última llamada
//
//   Lógica edge-detection:
//     Si tecla down (bit15):
//       Si g_KeyState[vk] == 0: → setear a 1, RETORNAR 1 (primera vez = "just pressed")
//       Si g_KeyState[vk] == 1: → RETORNAR 0 (ya procesada, no repetir)
//     Si tecla up:
//       g_KeyState[vk] = 0 (limpiar para próxima pulsación)
//       RETORNAR 0
//
//   Uso en game: por cada tecla de shortcut se llama una vez por frame.
//   Ejemplo: Key_IsJustPressed(0x2C) = F12 → toggle screenshot mode.
//
//   IMPORTANTE: No usar para movimiento continuo — usar GetAsyncKeyState directo.
uint Key_IsJustPressed(int vk);

// ─────────────────────────────────────────────────────────────────────────────
// MANEJO DE MOUSE EN WndProc
// ─────────────────────────────────────────────────────────────────────────────
//
// WM_MOUSEMOVE (0x200):
//   g_MouseX = LOWORD(lParam) * 640 / g_ScreenW   (normalizar a 640×480)
//   g_MouseY = HIWORD(lParam) * 480 / g_ScreenH
//
// WM_LBUTTONDOWN (0x201):
//   [mucha obfuscación HashTable]
//   g_DialogClose = 0         (abrir nuevo click cancela el "close" anterior)
//   if (g_DblClickPending == 0):
//     g_ClickFlag = 1         (marcar click — solo si no hay click ya pending)
//   g_DblClickPending = 1
//
// WM_LBUTTONUP (0x202):
//   g_ClickFlag = 0
//   g_DialogClose = 1        (LButton up = "acción confirmada / dialog cerrado")
//   g_DblClickPending = 0
//   g_MouseX_at_down = g_MouseX   (guardar posición al soltar)
//   g_MouseY_at_down = g_MouseY
//   Nota: si mouse se movió desde WM_LBUTTONDOWN (055ca03c != g_MouseX o 055ca040 != g_MouseY),
//         g_DialogClose se limpia de nuevo (era un drag, no un click).
//
// WM_LBUTTONDBLCLK (0x203):
//   [obfuscación HashTable]
//   g_DblClickFlag = 1       (DAT_083a4299)
//
// WM_RBUTTONDOWN (0x204):
//   [obfuscación HashTable] — sin globals públicos identificados todavía
//
// ─────────────────────────────────────────────────────────────────────────────
// MANEJO DE TECLADO EN WndProc
// ─────────────────────────────────────────────────────────────────────────────
//
// WM_KEYDOWN/WM_KEYUP (0x100/0x101):
//   NO manejados en WndProc → van a DefWindowProcA.
//   El juego usa GetAsyncKeyState() en polling en cada frame.
//
// WM_SYSKEYDOWN/WM_SYSKEYUP (0x104/0x105):
//   Filtrados en el message loop de WinMain ANTES de DispatchMessage.
//   Resultado: Alt+F4, Alt+Tab, etc. no llegan al juego (ventana no Alt-cerrable en game).
//
// WM_CHAR (0x102) — Input de texto:
//   Guard: activo solo si alguna de las siguientes es true:
//     g_TextMode != 0  ||  g_ChatMode != 0  ||  g_IME_Mode != 0
//     || g_UIScene == 0x7E/0x98  ||  g_UppercaseOnly != 0
//   Si guard false: ignorado.
//
//   wParam == 8  (Backspace):
//     FUN_00541eab — mide longitud del último carácter (1 o 2 para DBCS)
//     Resta esa longitud de g_TextLen[g_ActiveSlot]
//     Null-termina en la nueva posición
//
//   wParam == 9  (Tab):
//     Si g_TextMode activo y hay múltiples slots (g_TextMaxSlots >= 2):
//       g_ActiveSlot = (g_ActiveSlot + 1) % slot_count  (rotar)
//     Si slot único y buffer vacío: toggle g_IME_Mode
//
//   wParam == 0xD (Enter):
//     Procesado aparte → DAT_083a4299 = 1 o acción de submit
//     (ver también WM_LBUTTONDBLCLK para char-select)
//
//   Char regular (0x20..0x7E + DBCS):
//     IME_GetAndClearState('\0', 3) — leer estado IME sin limpiar
//     Filtros por modo:
//       g_DigitOnly   → solo '0'..'9'
//       g_UppercaseOnly → solo 'A'..'Z' (FUN_00542457 = toupper)
//     DBCS: g_DBCS_Counter decrementado por byte; si era lead byte: counter = 2
//     Append a g_TextBuf[g_ActiveSlot]: DAT_07db8710 + slot * 0x100 + len
//     g_TextLen++ (max 255 / g_TextMaxLen[slot])
//     Si buffer lleno al borde (len == maxLen-1): g_TextBufNearFull = 1
//
// WM_IME_COMPOSITION (0x10F) — Korean DBCS:
//   g_DBCS_Hi[g_ActiveSlot*4]   = (char)(wParam >> 8)
//   g_DBCS_Lo[g_ActiveSlot*4]   = (char)(wParam)
//   g_DBCS_Null[g_ActiveSlot*4] = 0
//
// WM_IME_STARTCOMPOSITION (0x10D):  g_IME_Active = 1
// WM_IME_ENDCOMPOSITION   (0x10E):  g_IME_Active = 0
//
// ─────────────────────────────────────────────────────────────────────────────
// FUNCIONES AUXILIARES DE INPUT
// ─────────────────────────────────────────────────────────────────────────────

// 0x0047edc0  IME_GetAndClearState(char clear_mode, byte save_mask) → bool
//   ImmGetContext(g_hWnd) → hIMC
//   ImmGetConversionStatus(hIMC, &conversion, &sentence)
//   Si hay estado IME activo (conversion != 0 || sentence != 0):
//     Si (save_mask & 1): DAT_07e11d94 = conversion
//     Si (save_mask & 2): DAT_00559cd8 = sentence
//     Si clear_mode != 0: ImmSetConversionStatus(hIMC, 0, 0)  — limpiar
//     Retorna 1 (había estado)
//   ImmReleaseContext(g_hWnd, hIMC)
//   DAT_07e11d98 = conversion   (siempre actualiza)
//   Retorna 0 (no había estado activo)
//   Usado en WM_CHAR para pausar IME durante composición ASCII.
bool IME_GetAndClearState(char clear_mode, byte save_mask);

// 0x004e9300  NumPad_HitTest(void) → int
//   Testea si g_MouseX/Y está sobre uno de los 11 botones del numpad del SecondPassword UI.
//   Layout del grid:
//     5 columnas × 2 filas (dígitos 0-9):
//       Cell (col, row): X = 0xDF + col * 0x28, Y = 0x96 + row * 0x28
//       Tamaño de celda: 0x28 × 0x28 (40×40 píxeles, en espacio 640×480)
//       Retorna índice lineal col + row*5 (0..9) si hit
//     Fila de botones extra (debajo de los 10 dígitos):
//       X: 0x109 + btn * 0x4E (step diferente)
//       Y: 0xEB..0xFF
//       Retorna 0x0B + índice del botón extra (máx index 0x1A4 en X)
//   Retorna -1 si el mouse no está sobre ningún botón.
int NumPad_HitTest(void);

// ─────────────────────────────────────────────────────────────────────────────
// POLLING DE TECLADO (durante el game loop)
// ─────────────────────────────────────────────────────────────────────────────
//
// El juego NO usa WM_KEYDOWN para control de juego.
// En su lugar, Game_SceneUpdate y otras funciones llaman directamente
// GetAsyncKeyState() en cada frame para las teclas de acción.
//
// Ejemplos documentados:
//   Game_MainLoop / Game_SceneUpdate:
//     GetAsyncKeyState(VK_SHIFT) → if high bit: toggle screenshot debug mode
//     Key_IsJustPressed(VK_F12)  → toggle screenshot mode (DAT_083a42ec ^= 1)
//
//   Login Scene (Game_SceneUpdate):
//     Clicks de mouse en áreas de botón como hit-test manual
//     (g_MouseX in [x1..x2] && g_MouseY in [y1..y2]) && g_ClickFlag == 1
//
//   Chat (in-game):
//     GetAsyncKeyState(VK_RETURN) o similar → abrir/cerrar chat
//     g_ChatMode = 1/0 para activar/desactivar entrada de texto
//
// ─────────────────────────────────────────────────────────────────────────────
// FLUJO COMPLETO DE UN CLICK EN EL MUNDO (in-game walk)
// ─────────────────────────────────────────────────────────────────────────────
//
//   1. WM_MOUSEMOVE: g_MouseX/Y actualizados cada evento
//   2. WM_LBUTTONDOWN: g_ClickFlag = 1, guardar posición
//   3. Game_SceneUpdate (g_GameState==5, g_GameSubState==2):
//      a. Lee g_ClickFlag
//      b. Camera_MouseRay(g_MouseX, g_MouseY) → ray en world space
//      c. Ray × terrain heightmap → world position (x, y)
//      d. World → grid: gridX = world_x / DAT_005524f0, gridY = world_y / DAT_005524f0
//      e. Si terreno walkable (GetTileType(gridX, gridY) bit0 == 1):
//           Enviar C1 0xD7 [gridX][gridY] al servidor (client walk request)
//      f. Actualizar entity+0x306/307 del jugador (predicción de cliente)
//   4. Servidor responde con C1 0x10 → PacketHandler_0x10 → Entity_MoveAlongPath
//
// ─────────────────────────────────────────────────────────────────────────────
// NOTAS DE PORTING (60fps / WebGL / Android)
// ─────────────────────────────────────────────────────────────────────────────
//
// - Mouse click flag (g_ClickFlag): ya es frame-rate independent (set/clear por events).
// - GetAsyncKeyState polling: en WebGL usar KeyboardEvent + estado en tabla similar a g_KeyState.
// - g_MouseX/Y normalizados a 640×480: referenciar SIEMPRE en ese espacio, no en resolución real.
// - DBCS / Korean IME: fuera de scope para primera versión del port.
// - Android: touch → simular WM_LBUTTONDOWN/UP con tap, touch drag → WM_MOUSEMOVE.
// - ChatMode text input: reemplazar con native IME de browser/Android.
//
// ─────────────────────────────────────────────────────────────────────────────
// RESUMEN DE ADDRESSES
// ─────────────────────────────────────────────────────────────────────────────
//
//   0x004149D0  WndProc           — 4074 líneas, ~80% anti-tamper. Ver secciones arriba.
//   0x0047ec20  Key_IsJustPressed — GetAsyncKeyState + edge-detection
//   0x0047edc0  IME_GetAndClear   — pausa/lee estado IME via Imm* API
//   0x004e9300  NumPad_HitTest    — hit-test del numpad SecondPassword
//   0x0047fae0  Chat_PushMessage  — añade mensaje al buffer de chat scroll (no es input)
//   0x00480620  Chat_Scroll       — scroll periódico del chat (no es input)


// Input_IsKeyJustPressed @ 0x0047ec20 — Input_GetKeyDown (Key_IsJustPressed)
// Returns 1 on the first frame a key goes down (edge trigger), 0 otherwise.
//
// BUG-FIX CRÍTICO (ESC-flicker): la versión de Ghidra terminaba con
//   `return uVar2 & 0xffffff00;`
// donde `uVar2 = CONCAT22(extraout_var, SVar1)` — y `extraout_var` nunca se
// inicializa (warning C4700). Cuando la tecla estaba UP, el valor devuelto
// era basura de EAX (restos del `MOV AX, <GAK>`), a veces ≠0 → el llamador
// lo interpretaba como "tecla recién presionada" y el menú ESC flipeaba
// ~16 veces/seg sin tocar nada. IDA (`PressKey`) siempre devuelve 0 en
// cualquier path que no sea el edge-trigger.
// IDA: FUN_0047ec20
int __cdecl Input_IsKeyJustPressed(int param_1)
{
  SHORT SVar1 = GetAsyncKeyState(param_1);
  // &DAT_07e118ec + param_1*4: byte offset correcto para el slot DWORD
  // (disasm @ 0x0047ec35 = MOV EAX,[ESI*0x4 + 0x7e118ec]). Necesario castear
  // la base a char* porque DAT_07e118ec es DWORD[256].
  DWORD* slot = (DWORD*)((char*)&DAT_07e118ec + param_1 * 4);
  if (((unsigned short)SVar1 >> 8) == 0x80) {
    if (*slot == 0) {
      *slot = 1;
      return 1;
    }
  } else {
    *slot = 0;
  }
  return 0;
}


// Input_ClearState @ 0x0047ec60 — Input_ClearAll
// Resets global input state: clears active slot and key tables.
// param_1 != 0: also clears slot 1 (password buffer); 0: preserves slot 1.
// IDA: FUN_0047ec60
void __cdecl Input_ClearState(int param_1)
{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;

  DAT_07e11d78 = 0;
  DAT_00559c90 = 0xffffffff;
  iVar2 = 0;
  do {
    if ((iVar2 != 1) || (param_1 != 0)) {
      // stride 0x100 (256 bytes per slot) — Ghidra mis-read as 0x40
      puVar3 = (undefined4*)((char*)DAT_07db8710 + iVar2 * 0x100);
      for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
      DAT_07d780a8[iVar2] = 0;
      DAT_07e113d8[iVar2] = 0;
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 < 10);
  return;
}


// Input_ProcessFunctionKeys @ 0x004c04a0 — Input_ProcessFunctionKeys
// Handles F1(0x70)-F4(0x73) toggle keys each frame.
// IDA: FUN_004c04a0
void Input_ProcessFunctionKeys(void)
{
  byte bVar1;
  char cVar2;
  SHORT SVar3;
  uint uVar4;
  byte *pbVar5;
  undefined4 uVar6;
  undefined4 uVar7;

  SVar3 = GetAsyncKeyState(0x70);
  if ((char)((ushort)SVar3 >> 8) == -0x80) {
    if (DAT_07e11aac == 0) {
      DAT_07e11aac = 1;
      if (DAT_07e11d20 == 3) {
        DAT_07e11d20 = 0;
      }
      else {
        DAT_07e11d20 = (DAT_07e11d20 + 1) % 3;
      }
      FUN_00404bc0(0x19,0,0);
    }
  }
  else {
    DAT_07e11aac = 0;
  }
  SVar3 = GetAsyncKeyState(0x71);
  if ((char)((ushort)SVar3 >> 8) == -0x80) {
    if (DAT_07e11ab0 == 0) {
      DAT_00559bf1 = DAT_00559bf1 == '\0';
      DAT_07e11ab0 = 1;
      FUN_00404bc0(0x19,0,0);
      // vtable[+0x30] = slot 12 = ChatLB_scrollByN (__fastcall this+n).
      if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
          DWORD* obj = (DWORD*)DAT_055c9ff0;
          void** vt  = (void**)*obj;
          typedef int (__fastcall *FnScroll)(DWORD*, int /*edx*/, int);
          ((FnScroll)vt[12])(obj, 0, 0);
      }
    }
  }
  else {
    DAT_07e11ab0 = 0;
  }
  SVar3 = GetAsyncKeyState(0x72);
  if ((char)((ushort)SVar3 >> 8) == -0x80) {
    if (DAT_07e11ab4 == 0) {
      DAT_07e11ab4 = 1;
      DAT_00559bf0 = DAT_00559bf0 == '\0';
      FUN_00404bc0(0x19,0,0);
    }
  }
  else {
    DAT_07e11ab4 = 0;
  }
  SVar3 = GetAsyncKeyState(0x73);
  if ((char)((ushort)SVar3 >> 8) != -0x80) {
    DAT_07e11ab8 = 0;
    return;
  }
  if (DAT_07e11ab8 != 0) {
    return;
  }
  DAT_07e11ab8 = 1;
  FUN_0040e330(DAT_055c9ff0);
  if (DAT_005590ac != 1) {
    uVar7 = 0x51;
    uVar6 = 0xfffffff6;
    goto LAB_004c06e6;
  }
  if (DAT_07eaa117 == '\0') {
LAB_004c06d6:
    uVar7 = 0x1a4;
    uVar6 = 0xba;
  }
  else {
    if (DAT_07eaa116 == '\0') {
      FUN_0043d8a0(&DAT_055c9bc8,&DAT_07eaa11b);
      cVar2 = DAT_07eaa11b;
      uVar4 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_07eaa11b);
      if (uVar4 != 0xffffffff) {
        pbVar5 = (byte *)FUN_00404280(&DAT_055c9bc8,&DAT_07eaa11b);
        bVar1 = pbVar5[1];
        pbVar5[1] = bVar1 - 1;
        if ((byte)(bVar1 - 1) == 0) {
          FUN_00423710(pbVar5,&DAT_07eaa11b);
        }
      }
      if ((((cVar2 == '\0') && (DAT_07eaa119 == '\0')) && (DAT_07eaa11a == '\0')) &&
         (DAT_07eaa11c == '\0')) {
        FUN_0043d8a0(&DAT_055c9bc8,&DAT_07eaa118);
        cVar2 = DAT_07eaa118;
        uVar4 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_07eaa118);
        if (uVar4 != 0xffffffff) {
          pbVar5 = (byte *)FUN_00404280(&DAT_055c9bc8,&DAT_07eaa118);
          bVar1 = pbVar5[1];
          pbVar5[1] = bVar1 - 1;
          if ((byte)(bVar1 - 1) == 0) {
            FUN_00423710(pbVar5,&DAT_07eaa118);
          }
        }
        if (cVar2 == '\0') goto LAB_004c06d6;
      }
    }
    uVar7 = 0x1a4;
    uVar6 = 0;
  }
LAB_004c06e6:
  FUN_0040c690((void*)(uintptr_t)DAT_055c9ff0,uVar6,uVar7);
  FUN_00404bc0(0x19,0,0);
  return;
}
