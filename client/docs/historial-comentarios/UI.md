# Historial de comentarios: `src/UI/`

Comentarios de desarrollo movidos desde `src/UI/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/UI/CharMenu_Build.cpp`

### Línea 10 — antes de `#define CHARMENU_ROW_MAX 28`

```cpp
// 2026-09-03 -- GUARDA DE RANGO (no esta en IDA).
// `DAT_07eaa154` es el numero de linea del panel y se usa como indice de TRES
// arrays de 30 entradas: `lpString_07e90798` (30 x 100 bytes), `DAT_07e91708`
// (color) y `DAT_07ea7b10` (negrita).  Aca se incrementa dentro de bucles cuya
// cantidad depende de los datos (skills del personaje, filas de stats), sin
// ningun tope, asi que un personaje con muchas entradas escribia mas alla de
// los 3000 bytes del buffer y sobre los globals vecinos.
// `RenderItemInfo` -- que llena las mismas tablas -- ya corta en 28; se replica
// ese criterio: al llegar al tope las lineas extra se pisan sobre la ultima en
// vez de desbordar.  El original no lo necesita porque alli el hueco de memoria
// que sigue al buffer es de relleno.
```

### Línea 43 en `HelpWindow_BuildTextList` — antes de `for (int i = firstLine; i < endLine; ++i, ++row) {`

```cpp
    // IDA recorre el bloque de GlobalText con un puntero de a 300 bytes hasta
    // GlobalText[endLine]; el port lo hacia contra la direccion absoluta del
    // binario (0x7D34134), que en este build no existe y se leia fuera.
```

### Línea 230 — antes de `extern "C" float Text_GetOrthoScaleX(void);   // src/stubs_externs.cpp`

```cpp
// FUN_0040fb70 @ 0x0040FB70 — RenderText del subclass de CUIRenderText, al que
// llega DrawItemInfoBox vía el dispatcher 0x0040F610.  Portamos la parte que
// define el layout: el offset de alineación (iSort) y el AVANCE VERTICAL que
// devuelve, que es lo que hace que cada línea quede donde va.
//
//   iSort 1 → izquierda con ancho fijo   2 → centrado   3 → derecha
//   retorna (cy / g_fScreenRate_y) / (text[0]=='\n' ? 2.0 : 1.0)
//
// DESVIACIÓN: el original rasteriza la línea a una textura de iBoxWidth px con
// TextOutA desplazado fVar4 px dentro de ella (0x0040FCD0).  Nosotros pintamos
// glifos directo en unidades del ortho, así que el desplazamiento se aplica
// sobre la x, convertido de píxeles a ortho con Text_GetOrthoScaleX().
// OJO (armadilla 1 de CLAUDE.md): stubs_bulk_misc.cpp ya define un
// `FUN_0040fb70` __fastcall que es un stub vacio (return 0.0f) y no lo llama
// nadie.  Para no crear dos simbolos con el mismo nombre y distinta firma,
// esta copia lleva otro nombre; el canonico va en el comentario.
// 2026-08-18 — FIX del ancho del recuadro.
//
// Este archivo convertia anchos de texto con g_fScreenRate_x, copiando la
// formula de IDA. En el binario eso es correcto porque su CUIRenderText recibe
// un ancho de referencia (640) y reescala la x internamente. NUESTRO stack de
// texto no hace eso: CUIRenderText_RenderText dibuja los glifos en unidades del ortho,
// convirtiendo con viewport/ortho (Text_PixelToOrthoScale).
//
// Al mezclar los dos factores, la CAJA quedaba dimensionada con un divisor y el
// TEXTO dibujado con otro: con 788 px de ancho la caja salia a 640/788 = 81%
// del texto y las lineas largas se desbordaban por la derecha.
//
// La altura no tenia el problema porque usa _DAT_055c9b74 en los dos lados
// (caja y avance por linea), asi que el factor se cancela.
//
// Es el mismo desvio ya documentado en HUD_Pass4.cpp:512 para el caret del
// input. Usamos la escala real del pipeline en todo lo que convierta anchos de
// TEXTO entre pixeles y layout.
```

### Línea 264 en `RenderHelpWindow`

```cpp
// src/stubs_externs.cpp
```

### Línea 485 — antes de `void __cdecl ItemHelp_RequireClass(int param_1)`

```cpp
// 2026-09-12: reescrita contra IDA.  La version anterior era inventada: leia
// `*(int*)(&DAT_07abf5d8 + 0x1bc)` (la direccion del PUNTERO al heroe + 0x1BC,
// no el heroe), trataba +0x38 como 4 "slots" de int y formateaba con
// DAT_0055a400/404, que estan vacios.
//
```

### Línea 566 — antes de `void __cdecl CharMenu_RenderStatRow(int column, unsigned char *format, int *value,`

```cpp
// 2026-09-17: el port anterior usaba el patron de ancho como formato y el
// formato como tabla de colores, y salteaba las filas en cero.
```

### Línea 601 — antes de `void __cdecl CharMenu_AppendSkillDesc(int param_1, int param_2, int param_3)`

```cpp
// IDA: sub_4C2D50 (0x004C2D50)
// 2026-09-12: era un no-op (la tabla vieja tenia direcciones literales del
// binario).  IDA usa GlobalText directamente: una linea por tipo, color 1, la
// dibuja con sub_4C2420(x, y, n, 0, 3, 0) y vuelve TextNum a 0.
```

### Línea 639 en `CharMenu_BuildStatRequirements` — antes de `if (DAT_00559fe0 == param_1) return;`

```cpp
    // El port anterior era una aproximacion con campos inventados.
```

## `src/UI/Chat.cpp`

### Línea 109 en `UI_RenderInputField`

```cpp
// 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
```

### Línea 121 en `UI_RenderInputField`

```cpp
// 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
```

### Línea 133 en `UI_RenderInputField`

```cpp
// 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
```

### Línea 164 en `UI_RenderInputField`

```cpp
// 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
```

### Línea 194 en `UI_RenderInputField` — antes de `ScaleGlobalTextSize();`

```cpp
  // BUG-FIX 2026-07-19 (el cursor `_` no se movía al escribir): acá había
  // `lVar9 = __ftol();` — llamada SIN argumentos, artefacto de Ghidra que lee
  // basura del tope de la pila x87. El IDA hace:
  //   TextSize.cx = (__int64)((double)TextSize.cx / g_fScreenRate_x);
  //   TextSize.cy = (__int64)((double)TextSize.cy / g_fScreenRate_y);
  // `lpsz_07e113d0` guarda el ANCHO del texto y es lo que posiciona el caret
  // más abajo (`(int)&lpsz_07e113d0->cx + param_1`). Con basura, el ancho
  // quedaba en 0/garbage → el `_` se dibujaba siempre al principio.
```

### Línea 254 en `UI_AddNotice` — antes de `if (!param_1) return;`

```cpp
    // IDA: CreateNotice (0x0047FAE0).  Aviso azul del centro: 6 slots de 264
    // bytes (texto en +0, color en +260).  Si el texto mide 256 px o mas se
    // parte con CutText: la primera mitad va al slot actual y la segunda al
    // siguiente, los dos con el mismo color.  (Antes se truncaba a 255 bytes
    // sin partir.)
```

### Línea 465 en `UI_RenderNotices` — antes de `int iVar1;`

```cpp
  // 2026-05-04: AUTO-SKIP removed. DAT_07db80d8 ahora propiamente sized
  // (6 slots × 0x108). Reemplazo el bound literal `< 0x7db8708` con count
  // explícito de 6 iteraciones.
```

### Línea 534 en `UI_RenderChatLogOverlay` — antes de `glColor3f(1.0f, 1.0f, 1.0f);`

```cpp
  // BUG-FIX: 0x3f800000 son los bits de 1.0f. Como int → 1065353216.0f.
```

### Línea 541 en `UI_RenderChatLogOverlay` — antes de `pbVar6 = (byte *)(DAT_07df9380 + iVar5 * 0x118);`

```cpp
    // BUG-FIX Ghidra: stride real del ring buffer = 280 bytes = 0x118 (confirmado
    // vs IDA sub_480980 y UIChatLogWindow_AddText). Ghidra decompiló el acceso
    // byte con *0x46 (= 70) porque el dword_7DF948C está tipado int[] — los
    // accesos DWORD con índice 0x46 sí dan 280 bytes, pero los accesos byte
    // necesitan *0x118.  Sin este fix, el renderer lee names/msgs del slot
    // equivocado (0, 70, 140, ...) en vez de (0, 280, 560, ...) y el texto
    // nunca aparece en pantalla (lo que pasaba con el countdown de Exit).
```

### Línea 576 en `UI_RenderChatLogOverlay` — antes de `{`

```cpp
      // BUG-FIX Ghidra: perdió los varargs de sprintf.  Port exacto de IDA:
      //   name  @ DAT_07df9380 + slot*280       (offset 0)
      //   msg   @ DAT_07df9380 + slot*280 + 11  (offset 0x0B)
      //   - Si los primeros 2 bytes del slot son 0x20 0x20 → "%s%s"  (sin dos puntos)
      //   - Si name[0] != 0                               → "%s: %s"
      //   - Si name[0] == 0                               → "%s"    (solo msg)
      // Las DAT_00559d4c/54/5c originales eran estos mismos formatos pero en
      // el binario aparecen como strings separados; en nuestro build están
      // declaradas como empty-string (char=0), así que usamos literales.
```

### Línea 601 en `UI_RenderChatLogOverlay` — antes de `*(LONG *)((char *)&DAT_07df9494 + iVar5 * 0x118) = local_108.cx;`

```cpp
      // BUG-FIX pointer-arith: &DAT_07df9494 ahora es int* (alias a offset
      // 0x114 del buffer), sumarle iVar5*0x118 como int* avanzaría 4x. Casteo
      // a char* antes de sumar el stride-en-bytes 0x118 para que el LONG
      // aterrice en el slot correcto.
```

### Línea 671 en `UI_TickHoverBubbles` — antes de `piVar7 = (int*)DAT_07e01720;`

```cpp
  // BUG-FIX 2026-04-28: pool real = DAT_07e01720[26 × 0x254].
  // Bounds end calculados a partir del array, no de la dirección absoluta.
```

## `src/UI/ChatListBox.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// ChatListBox.cpp — engine chat-widget object behind dword_55C9FF0.
//
// What this is
// ------------
// Port del objeto C++ que el mu.exe original crea con
//   operator new(0x5C8) → sub_40C7D0(this) → dword_55C9FF0 = this
// en WinMain @ 0x41F416-0x41F481. La identificación previa de
// dword_55C9FF0 como `HGLRC` era ERRÓNEA; es un objeto de widget de UI derivado
// cuya vtable vive en off_5525CC y cuyo constructor encadena a través
// del ctor base sub_40C5D0. Sin este objeto bien
// construido, los dispatches de Render_GameFrame (vtable+0x10) y de
// UIChatLogWindow_AddText (vtable+0x70) crasheaban en silencio y el
// HUD never appeared.
//
// Vtable layout (off_5525CC, 30 slots, all __thiscall)
// ----------------------------------------------------
//   slot  off  IDA addr   purpose                 (entries marked * are
//                                                  fully-rendered widget
//                                                  helpers — see notes)
//    0    +00  0x40DB80   ~CChatListBox(flag)       full
//    1    +04  0x40C670   set state              full
//    2    +08  0x40C6D0   set color1             full
//    3    +0C  0x40C6F0   set color2             full
//    4    +10  0x411920   render scroll frame    full (entry called
//                                                       from Render_GameFrame)
//    5    +14  0x40C710   tick / focus           full
//    6    +18  0x40DB40   nullsub                 full
//    7    +1C  0x411B60   maneja el input del scrollbar *completo, pero usa muchos
//                                                  GL globals; safe in our
//                                                  build (deps stubbed)
//    8    +20  0x403A30   nullsub                 full
//    9    +24  0x411A20   key-handler            re-uses FUN_00411a20
//                                                  already in stubs.cpp
//   10    +28  0x4118D0   clear list             full
//   11    +2C  0x410D70   nullsub                 full
//   12    +30  0x40CC50   scroll by N            full
//   13    +34  0x410D30   get visible count      full
//   14    +38  0x410D40   set visible count      full
//   15    +3C  0x410D40   set visible count (alt) full
//   16    +40  0x412150   incr scroll-step       full
//   17    +44  0x4122C0   trim oldest            full
//   18    +48  0x412320   recompute scrollbar    full
//   19    +4C  0x40CD80   count visible          full
//   20    +50  0x40CDD0   advance cursor         full
//   21    +54  0x40E230   hit-test del input     *usa sub_40C490 + globals
//   22    +58  0x40CE20   render del fondo del marco (ENORME) STUB — sólo delega a
//                                                  the counter increment;
//                                                  full GL render needs
//                                                  CUIRenderText + RenderBitmap
//                                                  port, que es su propio
//                                                  session.
//   23    +5C  0x40D610   render de línea (ENORME)     STUB — misma razón que el #22
//   24    +60  0x40D600   render footer thunk    full (thunk to sub_40DEF0)
//   25    +64  0x40E810   hover/click por línea   *completo (usa sub_40C490)
//   26    +68  0x40E400   per-frame button input *full
//   27    +6C  0x410D70   nullsub                 full
//   28    +70  0x40C940   AddText (entry called   full
//                          from UIChatLogWindow_AddText)
//   29    +74  0x40CD30   row visible?            full
//
// Linked list nodes
// -----------------
// Cada entrada de la lista de historial del chat (this[23] = campo+0x5C) es un
// byte block laid out as:
//   +0x00  fwd ptr (DWORD)
//   +0x04  back ptr (DWORD)
//   +0x08  data — sender 11 bytes + msg 257 bytes + flags (matches
//          original byte_55C95F8 staging buffer before AddText commits).
//   +0x10C msg-type DWORD (0..5, channel)
//   +0x114 cached text-extent cx (LONG)
// `delete__` recorre la lista doblemente enlazada y libera cada nodo.
//
// Ownership / lifetime
// --------------------
// El objeto vive en el heap, y su dueño es `dword_55C9FF0`. El destructor
// (sub_40DBA0) se invoca al cerrar el proceso vía el camino de ~Application;
// nuestro build por ahora lo filtra (el teardown de cierre vanilla no está
// ported).
//
// =============================================================================
```

### Línea 94 — antes de `int  __cdecl    ChatListBox_GetFocusState(DWORD* self);                // IDA: FUN_0040c68`

```cpp
// External helpers already implemented elsewhere in our build.  Linkage
// coincide con las definiciones que ya existen en stubs.cpp (C++, no extern "C").
```

### Línea 102 — antes de `static int Chat_BBoxHit(int x, int y, int w, int h, int mode)`

```cpp
// FUN_0040c490 — hit-test de bbox, port de sub_40C490 de IDA (lo referencian
// muchos helpers de UI). Vive acá porque stubs.cpp lo define detrás de
// bloques #if IDA_PORT_xxx que no están activados; volverlo
// siempre-activo chocaría con ésos, así que lo proveemos bajo una implementación única
// name and alias.
```

### Línea 144 — antes de `// ---------------------------------------------------------------------------`

```cpp
// En nuestro build dword_55C9B80 ya estaba declarado en globals.cpp (línea 423).
// Para no redefinirlo, tratamos el g_ChatListBox_FocusID_B local como un
// slot separado — los dos arrancan en 0, así que el comportamiento es idéntico para
// nuestro código, que no lee el original.
```

### Línea 173 en `ChatLB_renderBg`

```cpp
// slot 22 (stub)
```

### Línea 174 en `ChatLB_renderLine`

```cpp
// slot 23 (stub)
```

### Línea 236 — antes de `extern "C" void* ChatListBox_Construct(void)`

```cpp
// ---------------------------------------------------------------------------
// Construct — port 1:1 completo de sub_40C7D0(this), que primero llama a sub_40C5D0(this).
// Entrada pública: produce el objeto ya construido, listo para asignar a
// dword_55C9FF0. Reemplaza al `malloc(0x5c8)+memset` que hacía antes WinMain y
// left the vtable null.
// ---------------------------------------------------------------------------
```

### Línea 315 — antes de `extern "C" void* ChatListBox_ConstructWhisper(void)`

```cpp
// ===========================================================================
// ChatListBox_ConstructWhisper — port de sub_40E990, el SEGUNDO constructor
// de widget de chat (objeto más chico en DAT_055c9ff4, se usa para el input
// de destino de susurro + la lista chica de notificaciones). Misma estructura
// general que ChatListBox_Construct pero con:
//   * smaller list node size (0x18 instead of 0x120)
//   * 24 visible rows instead of 6
//   * position (460, 387), size (170, 250)
//   * vtable off_5526EC — 13 de los 30 slots DIFIEREN de off_5525CC.
//
// 2026-08-15: acá se instalaba `s_ChatLB_VTable` (la del chat).  Ese objeto es
// el que `RenderGuildList` usa para dibujar los miembros, así que el dispatch
// del slot 4 terminaba corriendo los métodos del CHAT sobre este objeto → AV.
// Ahora se instala `s_GuildLB_VTable` (definida al final del archivo, con los
// slots propios portados de IDA).
// ===========================================================================
```

### Línea 582 en `ChatLB_handleScrollIn` — antes de `if (((FnInt)vt[21])(self)) {`

```cpp
    // ── el slot 21 (hit-test del input) decide si el mouse está sobre este widget ──
    // IDA: `(*(...)(*(_DWORD *)this + 84))(this)` → +84 BYTES = entrada 21.
    // Corregido 2026-07-20: acá había vt[20] (= +80, advanceCursor).  Como
    // slot 21 es el ÚNICO que escribe self[48] (el flag "mouse cerca de la
    // barra de chat"), con el índice mal ese flag nunca se prendía y por lo
    // tanto ni el render (slot 24) ni el input (slot 26) de los 3 botones
    // popup llegaban a correr.
```

### Línea 729 en `ChatLB_handleScrollIn` — antes de `((FnVoid)vt[20])(self);`

```cpp
    // IDA sub_411B60 LABEL_76: `(*(...)(*(_DWORD *)this + 80))(this)`.
    // +80 BYTES = entrada 20 de la vtable = sub_40CDD0 (advanceCursor), NO la
    // 18 (+72 = recalcScroll).  Corregido 2026-07-20: advanceCursor es la que
    // deja self[25] apuntando al inicio de la ventana visible, que es lo que
    // consume el loop de abajo.
```

### Línea 757 — antes de `static int __fastcall ChatLB_keyHandler(DWORD* self) { return FUN_00411a20(self); }`

```cpp
// slot 9 — sub_411A20 — reusa el stub ya activado desde IDA que está en stubs.cpp.
```

### Línea 948 — antes de `#define InputText   DAT_07db8710`

```cpp
// slot 21 — sub_40E230 — input panel hit-test + right-click "copy sender
// name" support.  Full port:
//   * Si hay una fila con foco (this[28] != this[23]) Y el usuario
//     hizo click derecho, copia el nombre del remitente de esa fila (en +8 del payload del nodo)
//     a InputText[1] (el input de destino del susurro) y actualiza su longitud.
//   * Reset focused row to list head.
//   * Marca el diálogo de chat como "activo" (this[48] = 1) cuando el mouse está en la
//     banda del input de la fila inferior (160..480, 386..436); lo limpia cuando el mouse
//     is above y=416.
//   * Devuelve distinto de cero cuando el mouse está sobre la tira del encabezado del scroll del chat
//     (el botón de cerrar del borde derecho) O sobre una de las 3
//     bandas de toggle de canal de abajo (cuando está activo).
//
// 2026-07-20: flt_5590B0/B4/B8 YA tienen storage propio (ChatListBox_TabButtonsX/b4/b8 en
// globals.cpp, leídos del binario = 295 / 417 / 18).  El comentario anterior
// decía que "no existen en nuestro build" y por eso el segundo return usaba un
// rect inventado (160,420,60,16) que no cubría los botones — ver abajo.
// 2026-05-04: InputText/InputLength now alias DAT_07db8710 / DAT_07d780a8
// (ver HUD_Pass4.cpp). Usamos los mismos #define acá para compartir el storage
// with WM_CHAR + RenderInputText.
```

### Línea 995 en `ChatLB_hitTestInput` — antes de `if (self[48] == 1 && FUN_0040c490((int)ChatListBox_TabButtonsX, (int)ChatListBox_TabButton`

```cpp
    // Acá había un rect inventado (160, 420, 60, 16) de cuando no teníamos las
    // constantes.  Como no cubría x=295..349, al clickear un botón este hit-test
    // devolvía 0, slot 7 nunca despachaba el slot 26 y el click se perdía:
    // los botones se veían pero no respondían.
```

### Línea 1010 — antes de `extern "C" SIZE* __cdecl Text_MeasureBox(int x, int y, const char* lpString,`

```cpp
// slot 22 — sub_40CE20 — full chat-frame BG render.  When g_bUseChatListBox
// (g_bUseChatListBox) is enabled, draws:
//   * Alpha-tinted background quad (RenderColor at chat box bounds)
//   * Bitmap de la esquina superior (252) en el borde de arriba
//   * Vertical strip bitmaps (1281) tiling between top and bottom
//   * Bottom corner-piece bitmap (252)
//   * Two scrollbar arrows (1284) with hover hi-lighting (sub_40DCE0 — soft)
//   * Fondo (1283) y pulgar (1282) del scrollbar vertical cuando el contenido
//     excede la ventana visible — usa self[+0x90/+0x94/+0x98] para las
//     coordenadas del pulgar, que setea el slot 18 (recalcScroll).
//   * Active-channel tag header (CUIRenderText route → UI_DrawText in our
//     build).
// Incrementa self[46] (contador de frames) al final, para el parpadeo del cursor del slot 23.
//
// Casi todo este render depende de las constantes flt_55264C / flt_55256C / flt_552648 /
// flt_55265C / flt_552654 / flt_552658 / flt_552660 de la
// engine.  Those are 1-byte border thicknesses (typical: 1.0, 2.0, 3.0).
// Las aproximamos con literales suficientemente cercanos a los valores de IDA.
```

### Línea 1032 en `ChatLB_renderBg` — antes de `typedef void (__fastcall *FnVoidSelf)(DWORD*);`

```cpp
    // ── PORT FIEL de IDA sub_40CE20 (2026-07-20) ────────────────────────────
    // La versión anterior era una APROXIMACIÓN: su comentario decía "we
    // approximate with literals close enough to the IDA values".  De ahí venían
    // los 3 síntomas reportados: el recuadro salía casi transparente (alto del
    // fondo sin el +12 y sin el tope correcto), no había barra de desplazamiento
    // (nunca se llamaba a recalcScroll, así que self[36..40] quedaban en 0 y la
    // barra se dibujaba con geometría nula) y faltaban las flechas y el botón de
    // redimensionar (nunca se portaron).
    //
    // Constantes leídas del binario en 0x552644..0x552664 y 0x55256C:
    //   flt_552644=28  552648=7  55264C=2  552650=4  552654=21
    //   552658=8       55265C=22 552660=5  552664=12 55256C=1.0
    // Texturas: 252=Message_box2, 1280=nis_rsframe (grip), 1281=nis_vframe
    //   (bordes), 1282=nis_bar (thumb), 1283=nis_back (riel), 1284=nis_btnarrow.
```

### Línea 1173 en `ChatLB_renderBg` — antes de `char v40[256] = {0};`

```cpp
    // BUG-FIX 2026-05-01: header SOLO renderea cuando hay whisper target
    // activo (per IDA sub_40CE20 LABEL_37: chequea v40[0] tras sub_40E780).
    // Antes renderizaba siempre con string vacío → mostraba "palabra filtrada: "
    // (= GlobalText[754]) sin contexto.
    //
    // sub_40E780 (IDA): si this[200] (= self[50]) flag activo, concatena los
    // 5 whisper target names con separador. Sin flag → buffer queda vacío.
```

### Línea 1488 en `ChatLB_perFrameInput` — antes de `if (self[48] != 0 && DAT_00559c84 == 0) {`

```cpp
    // ── Los 3 botones popup del chat — PORT FIEL de IDA sub_40E400 ──────────
    // 2026-07-19: antes se salteaban por no tener las constantes de layout.
    // Leídas del binario en 0x5590B0..0x5590B8 (3 floats) → ahora viven en
    // globals.cpp como ChatListBox_TabButtonsX/b4/b8 = 295.0 / 417.0 / 18.0, compartidas
    // con el RENDER (slot 24 = ChatLB_renderFooter / IDA sub_40DEF0).
    // → tres rects de 16×16 en (295,417), (313,417) y (331,417). Caen en la
    // misma franja donde sub_4BE4F0 dibuja el input box (y≈415-422), por eso
    // el gate `!InputEnable` los oculta al abrir el recuadro de escribir.
    //
    // Gate de IDA: `if ( *((_DWORD *)this + 48) && !InputEnable )`
    //   self[48] (+192) NO es un puntero a método: es el flag "mouse cerca de
    //   la barra de chat" que escribe el slot 21 (ChatLB_hitTestInput) y que
    //   maneja el fade de los botones en sub_40DEF0.  El comentario anterior
    //   ("puntero a método / widget inicializado") era incorrecto.
```

### Línea 1566 en `ChatLB_AddText` — antes de `extern int __fastcall FUN_0040e730(void* This, int edx, char* param_1);`

```cpp
            // IDA sub_40C940: con el filtro de susurros activo solo pasan los que
            // coinciden (por remitente o por texto) con la lista del widget
            // (this+200, hasta 5 entradas de 256).  sub_40E730 devuelve 1 si la
            // lista esta vacia.  Con lista y m_bWhisperSound (0x07E11D80) suena
            // el aviso 38.  2026-09-12: estaba salteado ("soft-skipped").
```

### Línea 1608 en `ChatLB_AddText` — antes de `extern int __cdecl FUN_0040c2a0(LPCSTR, int, int, int, size_t, UINT, int);`

```cpp
        // IDA sub_40C940 L96-150: mensaje largo.  sub_40C2A0 lo parte en hasta
        // dos lineas de 180 px (buffers de 0x100); la primera va con el
        // remitente y la segunda con el nombre vacio.  Antes se truncaba a una.
```

### Línea 1672 — antes de `// MouseOnWindow — GLOBAL PARTIDO, corregido 2026-07-20.`

```cpp
// g_bUseChatListBox ya está definido en globals.cpp:192.
// En el original, /chatlistbox lo invierte en runtime. Nosotros sólo lo consumimos.
```

### Línea 1675 — antes de `extern "C" int MouseOnWindow = 0;`

```cpp
// MouseOnWindow — GLOBAL PARTIDO, corregido 2026-07-20.
// Este archivo definía su propio `MouseOnWindow` y lo escribía en el slot 7,
// pero NADIE lo leía: el flag que consume el resto del build (y en particular
// Player_InputTick, para no mandar al personaje a caminar cuando el click cae
// sobre una ventana) es `g_MouseOnWindow`, definido en Game/Player_InputTick.cpp.
// Resultado: clickear dentro del recuadro del chat hacía caminar al personaje.
//
// No se puede escribir g_MouseOnWindow directo desde acá: Player_InputTick lo
// resetea a 0 al principio de su propio tick (MouseOnWindow_Update), que corre
// DESPUÉS del tick del ChatListBox (Game_CharSelectTick: slot 5 en la línea 217,
// Player_InputTick en la 298).  Así que el slot 7 deja el resultado en este latch y
// MouseOnWindow_Update lo consulta.  El latch se reescribe entero en cada tick
// del widget, así que no se queda pegado.
```

### Línea 1691 — antes de `extern "C" void  __cdecl CreateGuildMark(int markIndex, bool blend);`

```cpp
// ===========================================================================
// ===  WIDGET DE LISTA DE GUILD (dword_55C9FF4, vtable off_5526EC)         ===
// ===========================================================================
//
// 2026-08-15.  El objeto de `dword_55C9FF4` (el que construye `sub_40E990`) NO
// comparte vtable con el chat principal: son DOS vtables distintas.
//
//   slot  off   off_5525CC (chat)  off_5526EC (guild)   ¿igual?
//    0    +00   0x40DB80           0x40EAC0             NO  dtor
//    9    +24   0x411A20           0x412180             NO
//   12    +30   0x40CC50           0x4119A0             NO  scrollByN
//   19    +4C   0x40CD80           0x410D50             NO  countVisible
//   20    +50   0x40CDD0           0x412470             NO  advanceCursor
//   21    +54   0x40E230           0x4124B0             NO  hitTest
//   22    +58   0x40CE20           0x40ED80             NO  renderBg
//   23    +5C   0x40D610           0x40EF10             NO  renderLine
//   24    +60   0x40D600           0x410D70 (nullsub)   NO  renderFooter
//   25    +64   0x40E810           0x40EAB0 (return 1)  NO
//   26    +68   0x40E400           0x40F320             NO  perFrameInput
//   28    +70   0x40C940           0x40EC20             NO  AddText/AddMember
//   29    +74   0x40CD30           0x4125F0             NO
//   (el resto coincide byte a byte)
//
// Nuestro `ChatListBox_ConstructWhisper` instalaba `s_ChatLB_VTable` en los dos
// objetos, asi que `RenderGuildList` -> slot 4 -> slots 20/22/23/24 corria los
// metodos del CHAT sobre el objeto de GUILD (otros offsets, otro layout de
// nodo) — de ahi el AV al abrir el panel teniendo miembros.  Aca esta la vtable
// propia, con los slots de la ruta de render y de datos portados 1:1.
//
// Nodo de la lista (0x18 bytes), tal como lo arma sub_40EC20:
//   +0x00 next . +0x04 prev
//   +0x08 name[11]  (strncpy 0xB desde el staging dword_55C9B60)
//   +0x13 flag de conexion (a3)
//   +0x14 numero de party, o 0xFF si no esta en party (a4)
// ===========================================================================
```

### Línea 2203 — antes de `// TextureScript_setScript @ 0x0040C170 (29 bytes) — thiscall: copia 4 bytes del parámetro`

```cpp
// ── TextureScript_setScript — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2214 — antes de `char __fastcall TextureScriptParsing_parsingTScript(void* ecx, void* /*edx*/, DWORD* param`

```cpp
// ── TextureScriptParsing_parsingTScript — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2222 — antes de `int __cdecl FUN_0040c2a0(LPCSTR text, int dst, int width, int maxLines,`

```cpp
// ── FUN_0040c2a0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// IDA: sub_40C2A0 (0x0040C2A0) — parte un texto en hasta `maxLines` lineas de
// `width` px (espacio 640).  Cada linea va a `dst + n*lineSize` (con
// `reverse == 1` se llenan de la ultima a la primera).  `firstIndent` le resta
// ancho solo a la primera.  Busca el corte por biseccion sobre la cantidad de
// caracteres y lo alinea a caracter multibyte con _mbclen.  Devuelve la
// cantidad de lineas escritas.  Antes era un stub `return 0`.
```

### Línea 2284 — antes de `int FUN_0040c480()`

```cpp
// ── FUN_0040c480 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c480 @ 0x0040C480 (12 bytes) — increment ref counter
// FUN_0040c480 (IDA-activated, was Ghidra stub)
```

### Línea 2292 — antes de `void __fastcall FUN_0040c500(void* ecx, void* /*edx*/, int param_1, int param_2, int param`

```cpp
// ── FUN_0040c500 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2298 — antes de `void __fastcall ChatListBox_DequeueFront(int param_1) {`

```cpp
// ── ChatListBox_DequeueFront — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2316 — antes de `void* __fastcall FUN_0040c5d0(void* param_1) {`

```cpp
// ── FUN_0040c5d0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2322 — antes de `void __fastcall FUN_0040c670(int ecx, int /*edx*/, int param_1) {`

```cpp
// ── FUN_0040c670 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2328 — antes de `// FUN_0040c680 @ 0x0040C680 (4 bytes) — getter thiscall: devuelve this->field_0x24`

```cpp
// ── FUN_0040c680 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2331 — antes de `int __cdecl ChatListBox_GetFocusState(DWORD *_this)`

```cpp
// FUN_0040c680 @ 0x0040C680 (4 bytes) — getter thiscall: devuelve this->field_0x24
// FUN_0040c680 (IDA-activated, was Ghidra stub)
// IDA: FUN_0040c680
```

### Línea 2339 — antes de `int __cdecl FUN_0040c680(DWORD *_this)`

```cpp
// Compatibility entry point retained solely for stubs_IDA_ports.cpp.
// IDA: FUN_0040c680
```

### Línea 2346 — antes de `void __fastcall FUN_0040c6b0(int ecx, int /*edx*/, int param_1, int param_2) {`

```cpp
// ── FUN_0040c6b0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2353 — antes de `// FUN_0040c6d0 @ 0x0040C6D0 (24 bytes) — thiscall: setea 3 campos en +0x3c, +0x44 y +0x48`

```cpp
// ── FUN_0040c6d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2363 — antes de `void __fastcall FUN_0040c6f0(int ecx, int /*edx*/, int p1, int p2, int p3) {`

```cpp
// ── FUN_0040c6f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2371 — antes de `int __fastcall FUN_0040c710(void* ecx, void* /*edx*/, int param_1) {`

```cpp
// ── FUN_0040c710 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2378 — antes de `int __cdecl FUN_0040c930(int a1)`

```cpp
// ── FUN_0040c930 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c930 @ 0x0040C930 (11 bytes) — refcount increment at +0x114
// FUN_0040c930 (IDA-activated, was Ghidra stub)
```

### Línea 2390 — antes de `int __cdecl FUN_0040cdd0(DWORD *_this)`

```cpp
// ── FUN_0040cdd0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_0040cdd0 @ 0x0040CDD0 (71 bytes) — skip ahead in linked list
// FUN_0040cdd0 (IDA-activated, was Ghidra stub)
```

### Línea 2422 — antes de `void __fastcall FUN_0040d550(void* param_1) { (void)param_1; }`

```cpp
// ── FUN_0040d550 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2426 — antes de `void __fastcall FUN_00410a90(int *param_1) {`

```cpp
// ── FUN_00410a90 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2433 — antes de `void __fastcall FUN_00410ab0(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_00410ab0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2440 — antes de `void __fastcall FUN_00410ad0(void *This) {`

```cpp
// ── FUN_00410ad0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2447 — antes de `void __fastcall FUN_00410de0(void *This, int /*edx*/, int *param_1, int *param_2, int *par`

```cpp
// ── FUN_00410de0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2461 — antes de `void __fastcall FUN_00410e30(int ecx, int /*edx*/, int *param_1) {`

```cpp
// ── FUN_00410e30 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2469 — antes de `// FUN_00410e40 @ 0x00410E40 (14 bytes) — thiscall: copia el valor de *(this+4)->first`

```cpp
// ── FUN_00410e40 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2477 — antes de `void __cdecl FUN_00410e50(void* self, DWORD* param_1, int* param_2) {`

```cpp
// ── FUN_00410e50 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2485 — antes de `void __cdecl FUN_00411360(void* self, int* param_1, int* param_2) {`

```cpp
// ── FUN_00411360 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2492 — antes de `// FUN_004113a0 @ 0x004113A0 (62 bytes) — BST lower_bound (find >= key)`

```cpp
// ── FUN_004113a0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2514 — antes de `void __fastcall FUN_004113e0(void *This, int /*edx*/, int *param_1, int *param_2) {`

```cpp
// ── FUN_004113e0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2534 — antes de `void __cdecl FUN_00411460(void* self, DWORD* param_1, int param_2, int* param_3, int* para`

```cpp
// ── FUN_00411460 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2542 — antes de `void __fastcall FUN_00411700(void *This, int /*edx*/, int param_1) {`

```cpp
// ── FUN_00411700 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2569 — antes de `void __fastcall FUN_00411760(void *This, int /*edx*/, int *param_1) {`

```cpp
// ── FUN_00411760 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 2596 — antes de `void __fastcall FUN_00411820(int ecx, int /*edx*/, int *param_1, BYTE *param_2) {`

```cpp
// ── FUN_00411820 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2603 — antes de `void __cdecl FUN_00411840(int param_1, int param_2) {`

```cpp
// ── FUN_00411840 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2611 — antes de `void __fastcall FUN_00411920(int* param_1) {`

```cpp
// ── FUN_00411920 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 2619 — antes de `int __cdecl FUN_004119a0(DWORD *_this, int a2)`

```cpp
// ── FUN_004119a0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_004119a0 @ 0x004119A0 (~29 lines) — ListBox_ScrollUp: decrements scroll offset
// (this+0x88) en param_1, clampeado a [0, itemCount - visibleCount]. Chequea la capacidad
// en this+0x80 o this+0x60 según el flag de modo en this+0x74.
// FUN_004119a0 (IDA-activated, was Ghidra stub)
```

### Línea 2662 — antes de `int __cdecl FUN_00411a20(DWORD *_this)`

```cpp
// ── FUN_00411a20 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00411a20 @ 0x00411A20 (~66 lines) — ListBox_HandleInput: processes key events
// (7=click, 0xC=page-scroll, 0xD/0xE=selection up/down) on a doubly-linked item list.
// Ajusta la posición del scroll vía vtable[0x30] y actualiza el puntero al ítem seleccionado en this[0x1C].
// FUN_00411a20 (IDA-activated, was Ghidra stub)
```

## `src/UI/Chat_Bubbles.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 21 — antes de `int __cdecl FUN_0047fed0(int lvl, const char* name) {`

```cpp
// Antes era un stub `return 1` sin parametros y sin callers.
```

### Línea 108 en `CreateChat` — antes de `if (*(BYTE *)entity == 0) return;              // Object.Live   → +0`

```cpp
    // Guard: entity must be alive and visible
    // BUG-FIX 2026-07-19: offsets de entidad equivocados (mismo problema que
    // AssignChat). IDA CreateChat @0x481BA0:
    //     if ( *(_BYTE *)Owner && *(_BYTE *)(Owner + 352) )
    //     v5 = *(unsigned __int8 *)(Owner + 746);          // PK
    //     if ( *(_BYTE *)(Owner + 132) == 4 ) v5 = 0;      // Kind
```

### Línea 125 en `CreateChat` — antes de `char *pool_base = DAT_07e016f8;`

```cpp
    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 en CreateChat+0x86): el bound era
    // la dirección LITERAL del binario original (`POOL_END = 0x7e0ffc8`) y la
    // base era `&DAT_07e016f8`, que estaba declarado como un char de 1 BYTE.
    // El walk se paseaba por memoria ajena hasta reventar en
    // `*(DWORD*)(slot + 0x234)`. Hasta ahora no se notaba porque AssignChat
    // nunca matcheaba (offsets de entidad mal) → CreateChat era código muerto.
    // Pool real: base 0x7E016F8, stride 596, fin 0x7E0FFC8 → (0xE8D0)/596 = 100.
```

### Línea 189 en `CreateChat` — antes de `// Copy ID (name) — siempre`

```cpp
    // PORT FIEL a IDA CreateChat @0x481BA0 found-existing path (2026-07-25):
    // Antes hacíamos el shift text1→text2, re-seteábamos owner y limpiábamos
    // text1 INCONDICIONALMENTE.  Pero Target_Render llama esto cada frame con
    // Text="" mientras hacés hover sobre un NPC → el manoseo per-frame de las
    // líneas hacía que el nombre se dibujara solapado varias veces.
    // IDA: para Text vacío hace SOLO `v6[10]=10` (refresca timer); el shift +
    // owner + set-text SOLO ocurren cuando Text NO está vacío.
```

### Línea 235 en `AssignChat` — antes de `DWORD base = DAT_07abf5d0;  // CharactersClient`

```cpp
    // 0x00482090 — Find character by ID, create chat bubble
    //
    // BUG-FIX 2026-07-19 (LA BURBUJA NUNCA APARECÍA): los offsets estaban mal.
    // IDA AssignChat @0x482090:
    //     if ( *(_BYTE *)v4 && *(_BYTE *)(v4 + 132) == 1 )
    //   → activo en **+0** (byte), kind en **+132 (0x84)** (byte).
    // Nosotros leíamos activo en +0x04 y kind como SHORT en +0x02 — pero +0x02
    // es `entity_type` (390 para jugadores, per CLAUDE.md), así que
    // `*(short*)(c+2) == 1` NUNCA era cierto → el pass 1 no matcheaba jamás y
    // no se creaba ninguna burbuja.
    // Entity stride 0x394 (=916, coincide con IDA). ID string en +0x1C1 (=449).
```

## `src/UI/Chat_CommandParser.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/UI/Chat_InputTick.cpp`

### Línea 72 en `Chat_SendPacket` — antes de `memcpy((char*)SocketClientSendBuffer + SocketClientSendBufferLength, pkt, len);`

```cpp
                    // BUG-FIX 2026-05-03: was `(BYTE*)0x055ca16c + 4` — literal
                    // source-binary address (unmapped in our build → AV on first
                    // WSAEWOULDBLOCK retry). The other 5 sites of this same
                    // pattern (Game_*Tick, Party, Player_InputTick) all use the
                    // SocketClientSendBuffer macro (= SocketClient + 0xC). Match them.
```

### Línea 109 en `SendRaw3`

```cpp
// BUG-FIX 2026-05-03: was literal 0x055ca16c
```

### Línea 260 — antes de `extern "C" void Chat_SendChatLine(const char* text)`

```cpp
// 2026-05-04 — Public helper: send a chat line typed into InputText[0]
// (DAT_07db8710 slot 0) by the WM_CHAR handler. Mirrors the inline
// build/send logic at Chat_InputTick lines 580-614 (channel-0 path)
// without depending on the per-frame FUN_00494520 polling. Packet:
//   [0xC1][len][..XOR-encoded text..]
// `text` must be NUL-terminated, length capped at 0x3c chars (matching
// the IDA original's truncation).
```

### Línea 342 en `Chat_TrySendGuildRequest` — antes de `memcpy(ChatWhisperID, whisperTarget, 10);`

```cpp
        // IDA WndProc (0x41D954, tras el send del susurro): ChatWhisperID =
        // InputText[1][0..9], con '\0' en [10].  Lo usa el aviso del 0x0C
        // ("no esta conectado") como remitente.  2026-09-12.
```

### Línea 349 en `Chat_TrySendGuildRequest` — antes de `if (DAT_07abf5d8) {`

```cpp
        // BUG-FIX 2026-07-19 (nuestros mensajes no llegaban): el campo name[10]
        // quedaba en CEROS y el server los descartaba en silencio.
```

### Línea 361 en `Chat_TrySendGuildRequest` — antes de `for (int xi = 3; xi < pktLen; ++xi)`

```cpp
    // BUG-FIX: era un XOR simple `pkt[i] ^= key[i]`. El server (XorData en
    // PacketManager.cpp) reversa el CHAIN-XOR, así que el cliente debe usar
    // `pkt[i] ^= pkt[i-1] ^ key[i]` — igual que el resto de los C1 (movimiento,
    // enter-world, char-select). Con el XOR simple el texto llegaba ilegible.
```

### Línea 424 en `Chat_InputTick` — antes de `const char *tableName = DAT_07df9380 + slot * 0x118;`

```cpp
            // Con los aliases DAT_07df938b/948c/9494 apuntando al buffer real,
            // &DAT_07df948c y &DAT_07df9494 son int*, asi que la aritmetica se
            // hace casteando a char* antes de sumar el stride en bytes.
            //
            // 2026-09-03 FIX: tres de los cuatro punteros usaban `slot * 0x46`
            // sobre char*, o sea un paso de 0x46 BYTES.  El stride real de la
            // tabla es **0x118** -- lo dice su propia declaracion
            // (`char DAT_07df9380[0x77 * 0x118]`) y lo usan todos los accesos de
            // Chat.cpp.  El 0x46 viene de los sitios donde el indice se aplica a
            // un `int*` (`(&DAT_07df948c)[i * 0x46]`, y 0x46*4 == 0x118): al
            // copiar el multiplicador a un contexto de bytes el paso quedaba 4x
            // corto y el bucle releia las primeras ~20 filas en vez de recorrer
            // las 78.  Misma familia que el bug del pool de clima, pero sin
            // salirse del buffer: no corrompe, devuelve la fila equivocada.
```

### Línea 554 en `Chat_InputTick` — antes de `HUD_BottomBarButtons_HitTest();`

```cpp
            // 2026-09-04 -- estaban portados como "class-tab buttons" con los
            // rects correctos pero el cuerpo mal: los tres compartian el helper
            // `ClassTab_HandleClick`, que hacia `GuildOpened = 1` y
            // `g_nGuildMemberCount = -1` SIEMPRE (son del boton de guild, IDA
            // L1196-1199) y mandaba paquetes inventados 0xF3..0xFB en vez de los
            // reales (0x52 guild, 0x42 party, 0x82/0x87 para cerrar
            // warehouse/chaos).  De ahi que party y personaje abrieran el panel
            // de guild.  Ademas consumian el click (`MouseLButtonPush = 0`)
            // antes de que llegara el hit-test bueno.
            //
```

### Línea 654 en `Chat_InputTick` — antes de `if ((DAT_00559c84 == 0) && (DAT_07e11d71 == 0))`

```cpp
            // 2026-09-04 BUG-FIX: el gate miraba VK_NUMPAD0 (0x60) en vez de
            // VK_MENU (18 = ALT), y las llamadas no pasaban el numero.
```

### Línea 838 en `Chat_InputTick`

```cpp
// BUG-FIX 2026-05-03: was literal 0x055ca16c
```

### Línea 863 en `Chat_InputTick` — antes de `if (DAT_00559c84 != 0) return;   // g_TextMode  (input de texto activo)`

```cpp
    // ── GATE: hotkeys de letra solo con el input de chat CERRADO ────────────
    // PORT FIEL de IDA Chat_InputTick @ 0x4B6575..0x4B65DB — son CINCO tests
    // encadenados, cada uno con `jnz` al epílogo (0x4BB807/0x4BB80D/0x4BB924/
    // 0x4BB928), no uno solo:
    //     8a 15 84 9c 55 00   mov  dl, byte_559C84    ; g_TextMode
    //     8a 15 71 1d e1 07   mov  dl, byte_7E11D71   ; g_IME_Mode
    //        ... test byte_7EAA11B                    ; TradeOpened
    //     a0 24 a1 ea 07      mov  al, byte_7EAA124   ; GuildCreatorOpened
    //     a0 70 1d e1 07      mov  al, byte_7E11D70   ; g_ChatMode
    // Todo lo que sigue (B @0x4BAD60, R, y las llamadas a sub_482BE0 de Q/W/E
    // en 0x4B666B/0x4B6CE8/0x4B739C/0x4B7A8D) está DESPUÉS de esos saltos.
    // (Las llamadas sub_43D8A0/sub_4041E0 intercaladas son ruido anti-tamper
    // de hash-table — omitidas per policy del proyecto.)
    //
    // BUG QUE ARREGLA (2026-07-19): en teclado español/latino `@` es AltGr+Q, y
    // `GetAsyncKeyState` ve la Q presionada → tipear `@` (o 'q'/'w'/'e', 'b',
    // 'r') en el chat disparaba el quick-use del hotbar y mandaba un
    // PMSG_ITEM_USE_RECV → el server cerraba la conexión.
    //
    // 1er intento porteó SOLO `g_ChatMode` y no alcanzó: al abrir el chat el
    // que se pone en 1 es **`g_TextMode` (DAT_00559c84)** — es el `InputEnable`
    // del log de WM_CHAR. Es también el flag que ya usaba `HUD_HotkeyTick`
    // (Player_InputTick.cpp), por eso las teclas de inventario SÍ quedaban
    // bloqueadas y estas no.
```

### Línea 1016 en `Chat_InputTick` — antes de `}`

```cpp
    // ── 12-15. C/V/I/G/P key handlers — REMOVED ────────────────────────────
    // 2026-05-08 (b): These keys are already handled by Player_InputTick
    // (`HUD_HotkeyTick` in src/Game/Player_InputTick.cpp:251-287) using the
    // edge-triggered helper PressKey. Adding duplicate handlers here
    // caused a DOUBLE-TOGGLE bug: pressing C played sound (Chat_InputTick set
    // CharacterOpened=1, played sound) but Player_InputTick toggled it back
    // to 0 in the same frame → net result = closed.
    //
    // Player_InputTick's pure-toggle handler is sufficient for visible-panel
    // gameplay. The packet-send side effects (guild/party 0x52/0x42 list
    // request, warehouse close 0x82) of the IDA Chat_InputTick path require
    // server context which is not yet wired.
```

## `src/UI/Chat_Log.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 38 en `UIChatLogWindow_AddText` — antes de `if (!label) label = "";`

```cpp
    // BUG-FIX 2026-08-17: el dispatch de abajo estaba gateado con `&& label`, y
    // el handler del notice 0x0D type=1 (Net_Process) llama con label = nullptr.
    // Resultado: los mensajes del server (incluido el contador "You will quit
    // game in N second(s)") nunca entraban a la lista del listbox, así que
    // in-game no se veían — sólo aparecían al pasar a char-select, donde los
    // dibuja el otro sink (sub_480980, gateado a g_bUseChatListBox||state!=5).
    // En IDA 0x480620 el dispatch es la PRIMERA sentencia y es incondicional;
    // el original nunca pasa NULL (usa cadena vacía). Normalizamos acá, que
    // además evita el lstrcpynA con origen NULL de más abajo.
```

### Línea 64 en `UIChatLogWindow_AddText` — antes de `char firstID = label ? *label : 0;`

```cpp
    // FIX 2026-07-19: estaba INVERTIDO (skipeaba mode ∈ {1,2,6,...}). El ring lo
    // consume sub_480980 (notificaciones login/char-select); con el gate invertido
    // los mensajes de sistema/GM/whisper con sender iban al ring equivocado.
```

## `src/UI/Chat_Send.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 31 en `SendChat` — antes de `if ((int)ChatTime > 0x32) return;`

```cpp
    // Rate limit — `ChatTime` es el GLOBAL 0x05826D08 (= ChatTime), que
    // `Game_MainLoop` decrementa un tick por frame (IDA 0x5262D9-0x5262EA):
    //     if ( ChatTime > 0 ) --ChatTime;
    //
    // 2026-08-12 BUG-FIX: acá era un `static int s_ChatTime` local, y NADIE lo
    // decrementaba. Después del primer mensaje quedaba clavado en 70, así que
    // `if (> 50) return` bloqueaba **todo** el chat posterior de la sesión.
    // Síntoma reportado: el primer `/move <mapa>` funcionaba y los siguientes
    // no hacían nada — el jugador se quedaba en el destino del primero y
    // parecía que el comando "recordaba" el mapa anterior.
    // Lo usan además WndProc y Chat_InputTick, que ya leían el global real.
```

### Línea 87 en `SendChat` — antes de `DAT_07e11dac = 1;`

```cpp
                // 2026-08-12 BUG-FIX: acá había DOS `static bool` distintos —
                // uno para "on" y otro para "off" — así que el toggle no
                // cambiaba nada y nadie podía leer el estado.
```

## `src/UI/Quest_Legacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.
```

### Línea 47 — antes de `char __fastcall FUN_00403150(void *pThis, int /*edx*/, char a2, char a3)`

```cpp
// sub_403150 @ 0x00403150 (438 bytes) — lista de items que pide la quest.
//
// Devuelve 1 si el personaje TIENE todos los items requeridos, 0 si falta
// alguno.  sub_403320 usa ese retorno para pintar el boton de "Proceder" en
// claro o en gris; sub_4BFDE0 la llama con a3 = 0 para dibujar los modelos 3D
// de los items en el panel.
//
//   a2 = estado esperado (se compara contra This + 0x1C882)
//   a3 = 0 -> dibuja los items en 3D;  != 0 -> dibuja "Nombre x N" como texto
//
// 2026-08-22: portada desde el DISASSEMBLY, no desde el decompile.  Hex-Rays
// emite "positive sp value has been detected, the output may be wrong" para
// esta funcion y pierde los parametros (lee Buffer[92] y v18 sin inicializar),
// asi que el decompile no sirve.  El disassembly, en cambio, sale limpio.
//
// Layout de la entrada de quest (18 bytes, arranca en pQuest + 40 + 18*i):
//   +0  categoria del item      -> nType = categoria * 32 + indice
//   +1  indice del item
//   +2  cantidad pedida
//   +4  flags de disponibilidad por clase (se indexa con This[4])
//   -1  (= pQuest + 39 + 18*i) 1 = la entrada pide un item
```

### Línea 114 en `FUN_00403150` — antes de `#define QUEST_ITEM_PREVIEW_DLL_FIX 1`

```cpp
            // DESVIACION DELIBERADA (pedido del usuario, 2026-09-20).
            //
            // IDA pasa Level = -1 aca (`push 0FFFFFFFFh` en 0x004032A5).  Rio
            // abajo, RenderObjectScreen extrae el +N con `(Level >> 3) & 0xF`,
            // y para -1 eso da 15: la vista previa se dibuja como si el item
            // fuera +15, o sea con el doble render del glow (flags 0x44/0x48).
            //
            // El DLL de inyeccion parchea exactamente ese byte --
            // `SetByte(0x004032A8, 0x0)`, "Fix Quest Item Preview" en
            // Patchs.cpp:100 -- para que el Level sea 0.  Replicamos el parche
            // con el mismo valor.  Poner QUEST_ITEM_PREVIEW_DLL_FIX en 0
            // devuelve el comportamiento de IDA.
```

### Línea 135 — antes de `void __fastcall FUN_00403320(void* param_1) {`

```cpp
// FUN_00403320 @ 0x00403320 (955 bytes) — ventana de quest del NPC
// Port fiel del decompile.  2026-08-21: acá había un resumen que sólo dibujaba
// el fondo y dejaba el resto como comentarios ("stub: full render logic
// omitted"); ni siquiera llamaba a FUN_00402ff0, que es la que dibuja el texto
// del diálogo y las respuestas.  Con el flag del panel prendido, GetScreenWidth
// angostaba el viewport a 450 y esa franja quedaba en negro.
//
// Desviación: la llamada a sub_403150 (que dibuja la lista de items pedidos por
// la quest y devuelve si están todos en el inventario) queda pendiente — su
// decompile sale con "positive sp value has been detected, the output may be
// wrong" y Hex-Rays perdió los parámetros.  Acá se asume "cumple" para el color
// del botón; lo único que cambia es que el botón sale habilitado y la lista de
// items no se dibuja.
```

### Línea 212 en `FUN_00403320` — antes de `char* npcName = getMonsterName(*(unsigned char*)(questBase + 12));`

```cpp
    // getMonsterName devuelve NULL mientras MonsterScript no esté parseada
    // (ver la nota de CLAUDE.md); el original no lo contempla.
```

### Línea 225 — antes de `void __fastcall FUN_00403a40(void* param_1);`

```cpp
// FUN_00403f30 @ 0x00403F30 (30 bytes) — dispatcher del render de quest.
// 2026-08-21: no existía y nadie lo llamaba.  sub_4F5820 (Render_QuickButtons_)
// lo invoca entre RenderGoldenArcherWindow y RenderServerDivision; sin eso el
// panel de quest nunca se dibujaba aunque su flag estuviera prendido.
```

### Línea 277 en `FUN_00403a40` — antes de `FUN_00402ff0((int)(uintptr_t)param_1);`

```cpp
    // 2026-08-21: faltaba el render del texto del dialogo + respuestas.
```

## `src/UI/RenderItemInfo.cpp`

### Línea 7 — antes de `extern "C" void __cdecl RenderItemInfo_impl(void*, void*, void*, int);`

```cpp
// 2026-05-08: SEH wrapper. Múltiples crashes recurrentes en este path
// (addr=0x74F3DBCC en ucrtbase.dll desde snprintf con `%s` reading bogus
// itemName pointer). Aunque agregamos guards extensivos, hay paths
// internos que pueden seguir tropezando con punteros corruptos. SEH
// silencia cualquier AV interno en lugar de matar el proceso — la
// tooltip simplemente no aparece esa frame.
```

### Línea 204 en `BuildInventorySpecialNameLine` — antes de `default: snprintf(dst, dstSize, "%s", p->Name); break;`

```cpp
        // El switch del binario (0x004C4DB2) llega hasta el case 0xC (12).
        // El case 13 -> GlobalText[117] era un injerto de version posterior.
        // levels 14/15 REMOVIDOS 2026-07-20: variantes de Box of Luck de
        // versiones posteriores; usaban GlobalText[1650]/[1651], fuera de las
        // 1000 filas.  Los niveles 0..12 (hasta "Box of Kundun +5") son validos
        // y quedan intactos.
```

### Línea 259 en `BuildInventorySpecialNameLine` — antes de `if (type == ITEM_POTION_BASE + 9) {`

```cpp
    // REMOVIDO 2026-07-21 — HELPER+20 (type 436) no existe en el item.bmd del
    // 0.97k (slot vacio): rama muerta, nunca disparaba.  Graft de version nueva.
    // REMOVIDO 2026-07-21 — HELPER+107 (type 523 >=512): tipo imposible.
```

### Línea 269 en `BuildInventorySpecialNameLine` — antes de `const char* skillName = (const char*)((char*)&SkillAttribute + 8 * (5 * (int)level + 150))`

```cpp
        // IDA (RenderItemInfo case 395): `v315 = 8 * (5 * Level + 150);`
        // → entrada 30+Level de SkillAttribute, que tiene stride 40, con el
        // nombre en el offset 0.  2026-08-21: el port usaba stride 300 + 4 y
        // leía hasta 10 KB fuera de la tabla (que son 2560 bytes en total).
```

### Línea 300 en `BuildInventorySpecialNameLine` — antes de `if (type == ITEM_WING_BASE + 32 || type == ITEM_WING_BASE + 33 || type == ITEM_WING_BASE +`

```cpp
    // REMOVIDO 2026-07-21 — HELPER+4/+5 (types 420/421) son slots vacios.
    // REMOVIDO 2026-07-21 — HELPER+30 (type 446) es slot vacio.
    // REMOVIDO 2026-07-21 — POTION+28 (type 476) es slot vacio.
```

### Línea 367 — antes de `static int GetInventorySpecialNameColor(ITEM* ip)`

```cpp
// Color del NOMBRE del item (slot 1) — port literal de RenderItemInfo
// @0x004C4650, bloque 0x004C4750..0x004C4820 (variable `local_8c` del decompile).
//
// 2026-08-18: lo que habia aca era una invencion de ~110 lineas con listas de
// tipos de versiones POSTERIORES del MU (Chaos Card, Ancient sets, alas de
// 3er nivel...), y ademas devolvia un color 9 que NO EXISTE: DrawItemInfoBox
// solo mapea 0..6, cualquier otro valor cae al `default` del switch y HEREDA
// el color de la linea anterior.  El binario 0.97k es mucho mas corto:
//
//   Type in {0x1CD, 0x1CE, 0x18F, 0x1D0, 0x1D6}   -> 3 (dorado)
//   Type in {0xAA, 0x13, 0x92}                    -> 6 (magenta)
//   Type in {0x1D1, 0x1D2, 0x1D3, 0x1B0, 0x1B1}   -> 3
//   si no hay excellent (SpecialNum==0 || (Option1 & 0x3F)==0):
//        level > 6 -> 3        si no -> (SpecialNum != 0) ? 1 : 0
//   con excellent                                 -> 4 (verde)
//
//   y al final, para 0x183..0x186 (que caen dentro del rango de arriba pero
//   tienen regla propia):
//        level < 7 -> (SpecialNum != 0) ? 1 : 0   si no -> 3
//
// `level` es (ip->Level >> 3) & 0xF, igual que `local_88` en el decompile.
```

### Línea 444 — antes de `// AUDITORIA 2026-07-20 — FormatInventoryTooltipTime REMOVIDA (10 sitios).`

```cpp
// REMOVIDA 2026-07-20 — GetInventoryTooltipAddOptionData, junto con sus 5
// callers.  Pertenecia al sistema de items por PERIODO de versiones
// posteriores (mismo bloque que FormatInventoryTooltipTime, ya removida):
//   · su switch solo cubria tipos 520..535, imposibles en una tabla de 512;
//   · su fuente de datos, GetInventoryItemAddOption, lee
//     Data\Local\ItemAddOption.bmd, archivo que NO EXISTE en el 0.97k —
//     verificado en todo el proyecto.  Sin el, s_loaded queda en false y la
//     funcion devolvia false siempre.
// La struct INVENTORY_ITEM_ADD_OPTION lleva un campo m_Time (vencimiento),
// que es la firma de ese sistema.
```

### Línea 455 — antes de `// Formatea un entero con separador de miles ("4700" -> "4,700"), que es lo que`

```cpp
// AUDITORIA 2026-07-20 — FormatInventoryTooltipTime REMOVIDA (10 sitios).
// Formateaba "tiempo restante" (dia/hora/minuto/segundo) para items con
// vencimiento, una feature MUY posterior al 0.97k.  Tres pruebas de que es
// injerto:
//   1. NO tenia un solo caller — ya era codigo muerto.
//   2. Usaba GlobalText[2298..2301] y [2308], fuera de las 1000 filas del
//      Text.bmd (o sea que el original no tiene esas etiquetas siquiera).
//   3. La tabla que la alimentaba (GetInventoryTooltipAddOptionData) solo
//      matchea tipos 520..535, y el item.bmd del 0.97k tiene 512 entradas:
//      esos tipos no existen ni pueden existir.
```

### Línea 561 en `AppendInventorySpecialTooltipLines` — antes de `case ITEM_HELPER_BASE + 41:`

```cpp
    // AUDITORIA 2026-07-20 — case ITEM_HELPER_BASE + 38 (type 454, Large Mana
    // Potion) REMOVIDO.  Usaba GlobalText[926], que en el Text.bmd del 0.97k es
    // "Antilag" (un string del MENU DE OPCIONES: 924 "Reiniciar fuente",
    // 925 "Volver", 926 "Antilag", 927 "Eliminar Sombras"), y GlobalText[2207],
    // que esta fuera de las 1000 filas del archivo.
    // El item existe, pero el case entero es de otra version: sin el, el tipo
    // cae al camino generico de pociones y muestra "Numero de items" como debe.
    // REMOVIDOS 2026-07-20 (4 sitios) — cases HELPER+49..53, types 465..469:
    // Devil's Eye, Devil's Key, Devil's Invitation, Remedy of Love y Rena.
    // Los cuatro bloques emitian UNICAMENTE indices fuera de las 1000 filas del
    // Text.bmd ([2397]/[2398]/[2399]/[1665]) y cortaban con `return`: no
    // dibujaban nada y ademas tapaban lo que hubiera mas abajo, igual que los
    // interceptores de las joyas.
    // REMOVIDO 2026-07-20 — case HELPER+40 (type 456, Antidote).  Emitia solo
    // GlobalText[2232] y [3088], ambos fuera de las 1000 filas del Text.bmd,
    // y cortaba con `return`.
```

### Línea 578 en `AppendInventorySpecialTooltipLines` — antes de `addFmt(GlobalText[88], 20, C_BLUE);`

```cpp
        // [2248]/[3088] removidos 2026-07-21: fuera de rango.  Se conservan
        // las lineas de buff [88]/[89] (Ale da daño adicional al consumirse).
```

### Línea 583 en `AppendInventorySpecialTooltipLines` — antes de `default:`

```cpp
    // REMOVIDOS 2026-07-20 (8 sitios) — cases HELPER+43 y +44 (types 459 Box
    // of Luck y 460 Heart).  Cuatro indices cada uno ([2256]/[2257]/[2297]/
    // [2567]/[2568]), TODOS fuera de las 1000 filas, y cortaban con `return`:
    // no dibujaban nada y ademas tapaban lo de mas abajo.
    // REMOVIDO 2026-07-20 — case HELPER+45 (type 461, Jewel of Bless).  Segundo
    // interceptor del mismo tipo que el de 462..464: cortaba con `return` antes
    // de que Bless llegara a su descripcion real ([572]).  Sus tres indices
    // ([2258]/[2297]/[2566]) estan fuera de las 1000 filas → salia vacio.
    // ── SCROLLS: descripciones REMOVIDAS 2026-07-21 ──────────────────────────
    // Bloques HELPER+64..76/+69/+70/+71..75 y POTION+42..44: emitian
    // descripciones con GlobalText>=1000 (verificado: RenderItemInfo 0x4C4650
    // NO referencia ninguno; los reales [571]/[69] SI tienen xref).  +69/+70
    // tenian ademas logica de PORTAL (coords del Hero), pero en 0.97k son
    // spells de mago (Ice/Teleport), no warp scrolls.  Sin estos bloques los
    // scrolls muestran nombre + requisitos + linea de clase (RequireClass, ya
    // funcional).  Si el cliente de referencia mostrara la skill que enseña el
    // scroll, es feature aparte (lookup a SkillAttribute como WING+11).
```

### Línea 604 en `AppendInventorySpecialTooltipLines` — antes de `switch (type) {`

```cpp
    // REMOVIDO 2026-07-20 (3 sitios) — ESTE bloque era el que dejaba mudas a las
    // joyas.  Interceptaba los tipos 462..464 (Jewel of Soul, Zen, Jewel of
    // Life) y cortaba con `return` ANTES de que llegaran a sus descripciones
    // reales, mas abajo en esta misma funcion ([573] Soul, [621] Life).
    // Y lo que emitia salia vacio: GlobalText[2259] y [2270] estan fuera de
    // las 1000 filas del Text.bmd.  Resultado: solo el nombre.
```

### Línea 614 en `AppendInventorySpecialTooltipLines`

```cpp
// [3088] removido 2026-07-21: fuera de rango
```

### Línea 615 en `AppendInventorySpecialTooltipLines` — antes de `case ITEM_HELPER_BASE + 55:`

```cpp
    // AUDITORIA 2026-07-20 — REMOVIDOS los cases de las pociones 448..452
    // (Apple, Small/Medium/Large Healing, Small Mana).  Son items REALES, pero
    // los cinco usaban GlobalText[1181]/[1917]/[1918]/[1919], todos fuera de las
    // 1000 filas del Text.bmd → cadena vacia, y el `return` cortaba el resto.
    // Por eso la Large Healing no mostraba NADA.  Sin estos cases caen al camino
    // generico de conteo, igual que el resto de las pociones.
    // AUDITORIA 2026-07-20 — case ITEM_HELPER_BASE + 37 (type 453, Medium
    // Mana Potion) REMOVIDO: mezclaba GlobalText[70] ("Vida", incorrecto en
    // una pocion de mana) con 10 indices fuera de las 1000 filas del Text.bmd
    // ([1860]/[1861]/[1867]..[1870]...).  Cae al camino generico como sus
    // hermanas Small (452) y Large (454).
    // RESTAURADO 2026-07-20 — items de la quest de evolucion: 471 Scroll of
    // Emperor, 472 Broken Sword, 473 Tear of Elf, 474 Soul Shard of Wizard.
    // No se venden ni se depositan.  El binario emite este par CONSECUTIVO:
    // en 0x4C5D42 el push de GlobalText[733] va inmediatamente despues del
    // sprintf de GlobalText[731] — mismo run de lineas.
    // (Antes [733] colgaba por error del bloque de Box of Luck, donde el
    //  cliente de referencia no la muestra en ninguna de sus 11 variantes.)
```

### Línea 640 en `AppendInventorySpecialTooltipLines` — antes de `default:`

```cpp
    // REMOVIDO 2026-07-20 — cases HELPER+54..58 (types 470..474).  El 470 es
    // Jewel of Creation: este bloque lo interceptaba y cortaba antes de su
    // descripcion real ([619]).  Usaba GlobalText[2511]/[2510], fuera de rango.
```

### Línea 647 en `AppendInventorySpecialTooltipLines` — antes de `// REMOVIDO 2026-07-20: colgaba de los tipos 513..516, que NO EXISTEN — el`

```cpp
    // (switch vacio removido 2026-07-21: sus cases eran grafts inexistentes)
```

### Línea 649 en `AppendInventorySpecialTooltipLines` — antes de `if (type >= ITEM_POTION_BASE + 70 && type <= ITEM_POTION_BASE + 71) {`

```cpp
    // REMOVIDO 2026-07-20: colgaba de los tipos 513..516, que NO EXISTEN — el
    // item.bmd del 0.97k tiene 512 entradas.  Los indices ([730]/[731]/[732])
    // son validos, pero estaban aplicados a items imposibles.
```

### Línea 680 en `AppendInventorySpecialTooltipLines` — antes de `if (level == 1)`

```cpp
        // levels 2/3 ([1099]/[1291]) removidos 2026-07-21: fuera de rango.
```

### Línea 695 en `AppendInventorySpecialTooltipLines` — antes de `if (level == 13)`

```cpp
        // REMOVIDA 2026-07-20 — GlobalText[733] "No puede ser vendido." se emitia
        // INCONDICIONALMENTE para todas las variantes de Box of Luck.  Probadas
        // las 11 contra el cliente de referencia: ninguna la muestra.
        //
        // OJO / PENDIENTE: la cadena NO es un injerto — el binario la usa en
        // RenderItemInfo, en UN solo sitio (0x4C5D42).  O sea que pertenece a
        // otro item y nos quedamos sin emitirla en ningun lado.  Falta ubicar su
        // guard real desasmando alrededor de 0x4C5D42 y re-colgarla ahi.
```

### Línea 739 en `AppendInventorySpecialTooltipLines` — antes de `if (type == ITEM_HELPER_BASE + 0) {`

```cpp
    // REMOVIDO 2026-07-21 — bloque escrito como "alas" (384+32..34) pero que en
    // realidad cae sobre los tipos 416, 417 y 418: Guardian Angel, Imp y Horn
    // of Uniria.  Les emitia GlobalText[571] ("Tiralo al suelo y podras recibir
    // zen o items"), que es la descripcion de la Box of Luck, y cortaba con
    // `return` antes del bloque propio de los pets que viene justo abajo
    // (`if (type == ITEM_HELPER_BASE + 0)` etc.).
    // Las alas de verdad son 384..390; 384+32 ya se pasa de ese rango.
```

### Línea 760 en `AppendInventorySpecialTooltipLines` — antes de `if (type == 433) {`

```cpp
    // SIMPLIFICADO 2026-07-21 — quitado HELPER+30 (type 446, slot vacio); solo
    // queda 433 (Blood Bone), item real.  El absorb del 446 (10+level) era
    // codigo muerto.
```

### Línea 803 en `AppendInventorySpecialTooltipLines` — antes de `if (level == 0)`

```cpp
        // level 1 ([1236]) removido 2026-07-21: fuera de rango (sin Dark Lord
        // en 0.97k, Loch's Feather no tiene tier).
```

### Línea 811 en `AppendInventorySpecialTooltipLines` — antes de `char line[256];`

```cpp
        // Fruit (431): descripcion = stat que sube + "Incrementa 1~3 puntos".
        // Removidos 2026-07-21: 2do switch ([1910]), linea [1908], caso level 4
        // ([1900]) y el bloque "equipable por Soul Master" — grafts (el 0.97k
        // solo tiene 4 stats: Ene/Vit/Agi/Fue = levels 0..3).
```

### Línea 826 en `AppendInventorySpecialTooltipLines` — antes de `if (type == ITEM_HELPER_BASE + 16 || type == ITEM_HELPER_BASE + 17) {`

```cpp
    // REMOVIDO 2026-07-20: emitia GlobalText[69] ("Numero de items") para los
    // mismos rangos que ya cubre AppendInventoryDurabilityTooltipLines, asi que
    // la linea salia DOS VECES (visible en Large Mana Potion).
```

### Línea 867 en `AppendInventorySpecialTooltipLines` — antes de `// Aca habia un 'switch (type)' con los cases HELPER+110..113 (types 526..529),`

```cpp
    // REMOVIDO 2026-07-21 — WING+130..135 (types 514..519 >=512): tipos
    // imposibles.  "Absorb wings" de una version con mas de 512 items.
    // REMOVIDO 2026-07-21 — HELPER+42 (type 458, Town Portal Scroll).  Emitia 7
    // lineas de GlobalText[976..982] (filas VACIAS en el Text.bmd) + [3088]
    // graft.  Confirmado contra el cliente de referencia: Town Portal NO tiene
    // descripcion (las filas 974..985 quedaron vacias para completar a futuro).
```

### Línea 874 en `AppendInventorySpecialTooltipLines` — antes de `}`

```cpp
    // Aca habia un `switch (type)` con los cases HELPER+110..113 (types 526..529),
    // removidos 2026-07-20 por inexistentes (la tabla del item.bmd tiene 512
    // entradas).  El switch quedaba solo con `default: break;` — warning C4065.
```

### Línea 879 — antes de `static void AppendInventoryDurabilityTooltipLines(ITEM* ip, ITEM_ATTRIBUTE* p, unsigned in`

```cpp
// ── RESTAURADA 2026-07-21 ────────────────────────────────────────────────────
// Esta funcion se perdio por una edicion POR NUMERO DE LINEA que borro su
// definicion dejando vivas las 2 llamadas (error C3861 en 1786 y 1921).
// Recuperada del backup del hilo paralelo y con las correcciones de la sesion
// 2026-07-20 REAPLICADAS, esta vez ancladas por texto.
// LECCION: en este archivo, editar por numero de linea es una bomba.
```

### Línea 967 en `AppendInventoryDurabilityTooltipLines` — antes de `} else if (type >= ITEM_HELPER_BASE && type <= ITEM_HELPER_BASE + 7) {`

```cpp
        // REMOVIDAS 2026-07-20 (2 sitios): ramas INALCANZABLES.  Cubrian los tipos
        // 448/449/450 (Apple, Small y Medium Healing Potion), que ya matchean la
        // PRIMERA condicion de esta cadena else-if (448..456).  Encima usaban
        // GlobalText[1181], fuera de las 1000 filas del Text.bmd.
        //
        // REMOVIDA tambien la rama HELPER+37 (453, Medium Mana Potion): le ponia
        // GlobalText[70] = "Vida: %d".  En el binario [70] tiene UNA sola
        // referencia en RenderItemInfo (0x4C6875) y [175] ("Mana: %d") NO la usa
        // RenderItemInfo — solo el tooltip de skills (sub_4C9730).  La pocion de
        // mana no lleva ninguna de las dos lineas; ademas la Small (452) y la
        // Large (454) tampoco la mostraban.
```

### Línea 991 en `AppendInventoryDurabilityTooltipLines` — antes de `} else if (type == ITEM_POTION_BASE + 100) {`

```cpp
        // REMOVIDAS 2026-07-20 (6 sitios):
        //  · tipos 462..464 (Jewel of Soul, Zen, Jewel of Life): usaban
        //    GlobalText[2260], fuera de rango.  Las joyas NO se acumulan en el
        //    0.97k — verificado contra el cliente de referencia, que no muestra
        //    conteo en ninguna.  Su texto es la DESCRIPCION, que ya emite
        //    AppendInventorySpecialTooltipLines ([572]/[573]/[621]/[619]/[574]).
        //  · tipos 541..543, 501, 477 y 537: no existen en el item.bmd del 0.97k
        //    (541/543/537 pasan las 512 entradas de la tabla; 501 y 477 son
        //    slots vacios).  Usaban [2260]/[2296]/[3105]/[3106], fuera de rango.
```

### Línea 1083 en `AppendInventoryLateBonusTooltipLines` — antes de `}`

```cpp
    // REMOVIDO 2026-07-20: emisor DUPLICADO de GlobalText[574] para el tipo 399
    // (Jewel of Chaos).  La descripcion ya la emite el bloque de joyas de
    // AppendInventorySpecialTooltipLines (junto a [572] Bless, [573] Soul,
    // [621] Life, [619] Creation), que ademas corta con `return`.  Al estar
    // tambien aca, la linea "Es utilizado para combinar items" salia DOS VECES.
```

### Línea 1286 — antes de `static void AppendInventoryRequirementTooltipLines(ITEM* ip, ITEM_ATTRIBUTE* pAttr)`

```cpp
// Requisitos de stat del tooltip.
//
// 2026-08-22 FIX ("las Leather Gloves piden 80 de fuerza"): estas lineas leian
// `pAttr->Require*`, o sea el valor CRUDO de la fila de ItemAttribute, que NO es
// el requisito final — es el coeficiente que escala `ItemConvert` (0x0047B910
// L227-234) con el nivel del item:
//
//     Require = 3 * attr.Require * (attr.Level + 3 * itemLevel) / 100 + 20
//
// (en el decompile la division sale como la constante magica 4123168605 >> 37,
//  que es exactamente `* 3 / 100`; la forma legible esta en sub_4C2E20 L188).
// Para las Leather Gloves +0 eso da 20, no 80.  IDA lee los CUATRO requisitos de
// la INSTANCIA — `ip->RequireStrength` L1459, `ip->RequireDexterity` L1684,
// `ip->RequireLevel` L1906, `ip->RequireEnergy` L2353 — que es donde
// `ItemConvert` deja el valor ya escalado.
//
// La nota vieja (2026-08-18) decia que leer de `ip` no mostraba ninguna linea;
// eso ya no aplica: `ItemData_FillStats` siembra la instancia con el crudo antes
// de que `ItemConvert` la recalcule, asi que el campo nunca queda en 0.
// Los indices de GlobalText ya estaban bien: 0x49=73 fuerza, 0x4B=75 agilidad,
// 0x4C=76 nivel, 0x4D=77 energia (verificado en 0x004C6xxx).
```

### Línea 1394 en `AppendInventoryRequireClassLines` — antes de `if (byRequireClass == 1)`

```cpp
            // FIX 2026-07-20 — los nombres de tier 2 estaban CORRIDOS UNO.
            // Volcado del Text.bmd desencriptado:
            //   20 Dark Wizard   21 Dark Knight  22 Fairy Elf  23 Magic Gladiator
            //   24 Soul Master   25 Blade Knight 26 Muse Elf
            //   27/28 "Reservation: job"  (el MG no tiene tier 2)
            // Se usaba 25 para el mago evolucionado, y por eso un Grand Soul
            // Armor decia "Blade Knight" en lugar de "Soul Master".
            //
            // Los tier 3 (1668..1671) NO existen en 0.97k y ademas caen FUERA
            // de GlobalText[1000][300]: leerlos es un desborde del array.
            // El Text.bmd tiene exactamente 1000 filas (300000 bytes).
```

### Línea 1436 — antes de `#define RENDERITEMINFO_FIEL 1`

```cpp
// IDA: RenderItemInfo (0x004C4650)
// ═════════════════════════════════════════════════════════════════════════════
// RenderItemInfo — port fiel de IDA (0x004C4650), 2026-09-12.
//
// Reescrita en el orden exacto del decompile (raw 004C4650, ~2800 lineas de
// las que ~60 % es hash-table anti-tamper).  La version anterior
// (RenderItemInfo_impl, mas arriba) esta armada por helpers y mezcla ramas del
// 5.2 con las del 0.97k; se conserva detras de RENDERITEMINFO_FIEL para poder
// comparar las dos en el juego.
//
// Contrato de la lista de texto (igual que en el binario):
//   TextList      = lpString_07e90798 (30 x 100)   TextNum = DAT_07eaa154
//   TextListColor = DAT_07e91708                   TextBold = DAT_07ea7b10
//   SkipNum (dword_7EAA158) = DAT_07eaa158 (lineas de media altura)
// Unica desviacion: RII_Line devuelve un scratch si el indice se pasa de 30,
// para no escribir fuera del buffer (el original no tiene tope).
// ═════════════════════════════════════════════════════════════════════════════
```

### Línea 1923 en `RenderRepairInfo` — antes de `unsigned int attrBaseOK = ItemAttribute_Base();`

```cpp
    // 2026-05-08: defensive — si nos llaman antes de que WinMain initialice
    // el ItemAttribute table (DAT_07d78068), o si DAT_07d78068 fue clobbered
    // a 0x1 por un writer desconocido, ItemAttribute_Base() recupera del
    // backup. Si tampoco es válido, saltamos.
```

### Línea 1946 en `RenderRepairInfo` — antes de `for (int i = 0; i < 30; ++i)`

```cpp
    // BUG-FIX 2026-05-03: was `< 0x7e91350` (absolute end bound from source binary).
    // In our build lpString_07e90798 is linker-placed; literal address is junk.
```

### Línea 1982 en `RenderRepairInfo` — antes de `if (ShopOpened != 0 && DAT_07eaa154 < 28) {`

```cpp
    // REMOVIDO 2026-07-21 — bloque de "repair gold" dentro de RenderItemInfo.
    // Llamaba a ConvertRepairGold escribiendo en `lpString_07e90798` = SLOT 0,
    // o sea la linea que va ARRIBA del nombre del item.  Sintoma: cualquier
    // item con curDur < maxDur mostraba un numero suelto encima del nombre
    // ("1" en Guardian Angel/Imp, "5,200" en Horn of Dinorant), y los que
    // estaban full (Horn of Uniria, 255/255) no mostraban nada.
    //
    // NO ES DEL ORIGINAL: ConvertRepairGold (0x4C3EF0) tiene exactamente 3
    // xrefs en el binario — dos en sub_4C4080 (0x4C4206 y 0x4C42D2) y uno en
    // RenderRepairInfo (0x4C8F28).  NINGUNO en RenderItemInfo (0x4C4650).
    // El precio de reparacion lo calcula RenderRepairInfo, que es la otra rama
    // del dispatch de Scene_MapTick.
    // ── Slot: item NAME line (with +N suffix when level > 0) ────────────────
    // ── Precio (bloque LAB_004c4a61, 0x004C4A61..0x004C4CED) ────────────────
    // 2026-08-18: este bloque estaba AL FINAL del tooltip y con color/negrita
    // propios.  En el binario va ACA, entre el separador del slot 0 y el nombre
    // del item, y hereda el MISMO color que el nombre (local_8c) con negrita.
    //
    //   if (ShopOpened) {
    //       precio = ItemValue(ip, Sell);
    //       sprintf(linea, GlobalText[Sell ? 0x3e : 0x3f], precioFormateado);
    //       TextListColor[TextNum] = local_8c;   TextBold[TextNum] = 1;
    //       TextNum++;
    //       sprintf(linea, DAT_0055a4e4);        // separador de media altura
    //       TextNum++;  SkipNum++;
    //   }
    //
    // El gate real es ShopOpened (en el decompile aparece como `cStack_71`, que
    // es su valor desofuscado tras el bloque de hash-table anti-tamper).  El
    // port usaba `param_4` (bSell) como gate y ademas deducia compra-vs-venta
    // comparando el puntero del item contra el rango del pool de la tienda;
    // el binario lo decide con `Sell` a secas.
    // 2026-09-12: el modo de ItemValue estaba invertido.  IDA L522-557:
    //   if (Sell) { ItemValue(ip, 0) ... GlobalText[62] }
    //   else      { ItemValue(ip, 1) ... GlobalText[63] }
    // El segundo argumento NO es `Sell`: 0 = precio completo (el mismo que se
    // cobra al comprar, sub_4D23B0 L416), 1 = precio de venta.
```

### Línea 2100 en `RenderRepairInfo` — antes de `// ── Damage range — for weapons (slot+0x18 = DamageMin, +0x1C = Max).`

```cpp
        // 2026-08-18: TODAS las stats de abajo se leian de `it` (la INSTANCIA
        // del item), pero viven en `p` — la fila de ItemAttribute indexada por
        // tipo.  `p` estaba declarado aca y no se usaba.  Resultado: Defense,
        // DamageMin/Max, MagicDefense y las velocidades salian 0 y sus lineas
        // NO se emitian; en el tooltip solo sobrevivia la durabilidad.
        // De `it` solo salen los campos de la instancia (Option1, Level,
        // SpecialNum, Durability actual).
```

### Línea 2194 en `RenderRepairInfo` — antes de `AppendInventoryRequirementTooltipLines(it, p);`

```cpp
        // ORDEN 2026-07-20: la línea "Puede ser equipado por X" va JUSTO DESPUÉS
        // de los requisitos, ANTES del bloque de opciones excellent.  Antes se
        // emitía última.  Verificado contra la salida del cliente de referencia
        // (mismo binario que tenemos en IDA): requisitos → clase → excellent.
```

### Línea 2246 en `RenderRepairInfo` — antes de `unsigned int attrBaseOK_ = ItemAttribute_Base();`

```cpp
    // 2026-05-08: same defensive guards as RenderItemInfo (sibling function).
    // Use the backup-aware accessor to recover DAT_07d78068 if clobbered.
```

### Línea 2271 en `RenderRepairInfo` — antes de `for (int i = 0; i < 30; ++i)`

```cpp
    // BUG-FIX 2026-05-03: was `< 0x7e91350` (absolute end bound from source binary).
```

### Línea 2305 en `RenderRepairInfo` — antes de `char repairGold[64] = "0";`

```cpp
    // IDA RenderRepairInfo: RepairEnable_0 = 1 con el item sano y = 2 con el
    // item dañado; el 2 es el martillo animado de RenderCursor, y Scene_MapTick
    // lo vuelve a 1 cada frame.  Estas escrituras se habian quitado el
    // 2026-05-08 porque el "fix" de Scene_MapTick de entonces las trababa;
    // desde que Scene_MapTick normaliza como IDA (2026-09-12) no hace falta.
    // IDA L133-150: la linea del costo es sprintf(GlobalText[238], Buffer), con
    // Buffer = ConvertRepairGold(...) si el item esta danado y "0" (0x55A5F8)
    // si esta sano; color = tier (v12), en negrita.
    // 2026-09-12: el port formateaba GlobalText[238] ("Costo de reparacion: %s",
    // alias DAT_07d3b40c) SIN argumento -- de ahi el "%s" en basura -- y
    // escribia el precio en lpString+64, en medio de la linea anterior.
```

### Línea 2319 en `RenderRepairInfo` — antes de `int gold = Item_CalculateValue((void*)param_3, 2);`

```cpp
        // BUG-FIX 2026-04-26 (audit #3): same ItemValue/ConvertRepairGold pair.
```

## `src/UI/UI_DialogInterface.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.
```

### Línea 5 — antes de `#include "stdafx.h"`

```cpp
// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.
```

## `src/UI/UI_GuildLegacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 47 — antes de `int __cdecl RenderMatchScore(void)`

```cpp
// 2026-09-07: estaba neutralizada con un `return 0` al entrar ("AUTO-SKIP:
// absolute end-bound loop"), asi que el cartel salia con el marco y el boton OK
// pero VACIO.  Reescrita contra IDA con los bucles acotados por contador.
//
```

### Línea 216 en `GuildMemberList_Update` — antes de `int nMembers = count;`

```cpp
    // Copy member list data: count * 0x18 bytes into DAT_083a7af8
    // 2026-09-03: la tabla tiene 11 registros (0x108 bytes, ver globals.h).
    // IDA no acota `count` porque alli el hueco es exactamente ese; aca el
    // clamp evita que un `count` grande escriba sobre los globals vecinos.
```

### Línea 244 — antes de `int __cdecl ItemList_Select(int param_1) {`

```cpp
// ItemList_Select @ 0x0051D840 — ItemList_Select(slot)
// Selects character slot `slot` for the in-game item/skill list display.
// Sets DAT_005615dc, populates DAT_083a4324 and skill/item display arrays,
// then transitions UI state to 0x8e.
// 2026-08-21: era un stub que salteaba el texto ("requires DAT_07cf5608 char
// data arrays not yet mapped").  La tabla ya esta reconciliada (DIALOG_SCRIPT
// en globals.h), asi que ahora es el port fiel de sub_51D840: arma el cuadro de
// dialogo desde g_DialogScript[a1] igual que CSQuest::ShowDialogText, mas el
// memset de los rects de boton y ErrorMessage = 142.
```

## `src/UI/UI_GuildMark.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extraído de stubs_game.cpp; se conserva la trazabilidad IDA en los comentarios de las funciones.
```

### Línea 85 en `RenderGuildMark` — antes de `if (p5 < 0 || p5 > 15) return;`

```cpp
    // 2026-08-25: `DAT_07e11f34` ahora es el array de 16 que realmente es, asi
    // que se indexa directo (antes `(&DAT_07e11f34)[p5]` sobre un unico DWORD
    // leia hasta 60 bytes del vecino).
```

## `src/UI/UI_IME.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/UI/UI_InGameMenu.cpp`

### Línea 12 — antes de `#include "Net/Net.h"`

```cpp
// 2026-05-05: Net_SendSmallPacket (proper C3 wrap with serial)
```

### Línea 17 — antes de `extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);`

```cpp
// 2026-08-25: los opcodes del trade con Encrypt=0 necesitan C1 PLANO.
```

### Línea 30 — antes de `void Net_SendWarehouseMoney(BYTE type, DWORD money);`

```cpp
    // 0x81 PMSG_WAREHOUSE_MONEY_RECV (stubs_render_helpers.cpp)
```

### Línea 49 — antes de `static void SaveOptionsToServer97k(void)`

```cpp
// 2026-09-21: faltaba entera.  `functions.h` la tenia declarada como
// "Map_Unload" (mal identificada) y ni estaba definida ni la llamaba nadie, asi
// que el cliente NUNCA mandaba el F3/30: las teclas de skill (Ctrl+numero) se
// perdian al volver a entrar.  El handler de entrada (F3/30, ReceiveOption) ya
// estaba y era fiel; faltaba esta mitad.
//
```

### Línea 154 en `UI_InGameMenu` — antes de `int escHit = PressKey(27);  // PressKey(27)`

```cpp
    // IDA UI_InGameMenu (0x514310) L534-572.  2026-09-18: fiel.  El port
    // cerraba primero los paneles abiertos (y mandaba 0x31), cancelaba los
    // carteles Si/No 151/153 y abria el menu sobre los carteles de desconexion;
    // nada de eso esta en el binario.
```

### Línea 214 en `UI_InGameMenu` — antes de `case 0x1a:`

```cpp
    // ── Quit game (fiel al IDA) ───────────────────────────────────────────
    // Codes 0x1a (result=6) / 0x1c (result=8 "demasiados intentos") /
    // 0x70/0x71 (fallo de conexión). El IDA (0x514310 L1396-1414) muestra el
    // cartel y SOLO hace SendMessageA(WM_DESTROY) cuando hacés click en el botón
    // OK (rect 284-354 × 98-119) — no cierra directo. El cartel persiste hasta
    // el click. BUG-FIX 2026-07-14: gateamos el WM_DESTROY al click en OK.
```

### Línea 320 en `UI_InGameMenu` — antes de `if (SceneFlag == 5)                      // IDA L1070: sub_50F7A0()`

```cpp
                            //
                            // 2026-09-16: el port hacia toda la transicion aca en
                            // el acto (el comentario decia que MuEmu no contesta
                            // F1/02/02, y es falso: User.cpp:2347
                            // GCCloseClientSend(2) tras CloseCount).  Por eso no
                            // habia cuenta regresiva.
```

### Línea 352 en `UI_InGameMenu` — antes de `SaveOptionsToServer97k();       // IDA L1080: sub_50F7A0()`

```cpp
                            // 2026-05-05 (final): JoinChar — back to char-select.
                            //
                            // Análisis del pcap+server log: server tarda ~5
                            // segundos en procesar el F1/02/01 (tick async).
                            // Durante ese delay Connected sigue OBJECT_ONLINE, y
                            // CGCharacterListRecv early-returns. Por eso el flow
                            // viejo (mandar F1/02/01 + F3/00 en mismo tick)
                            // hacía que server ignorara nuestro F3/00.
                            //
                            // Flow correcto:
                            //   1. Cliente manda F1/02/01 (C3-wrapped)
                            //   2. ESPERAMOS sin transition local
                            //   3. Server tick procesa CloseCount=1 →
                            //      CharacterGameClose → Connected=OBJECT_LOGGED
                            //      → manda F1/02/01 ack
                            //   4. Cliente recibe F1/02 ack → Recv_LogOut runs:
                            //      transitions state=4 + manda F3/00 (ahora
                            //      server YA está OBJECT_LOGGED → procesa OK)
                            //   5. Server GDCharacterListSend → DataServer →
                            //      F3/00 char-list fresca
                            //   6. Cliente Recv_CharList puebla slots 0-4
```

### Línea 405 en `UI_InGameMenu` — antes de `case 0x72:`

```cpp
    // Antes este case corria SIN gate de click: el cartel se cerraba solo en
    // el frame siguiente y nunca se mandaba el borrado (no habia ningun envio
    // de F3/02 en el arbol).
```

### Línea 456 en `UI_InGameMenu` — antes de `case 0x74:`

```cpp
    // ── Zen input dialog (ErrorMessage 116) — baúl / trade ─────────────────
    // 2026-08-08 PORT (antes: rama inventada que llamaba SecondPassword_Shuffle, o sea el
    // shuffle del teclado numérico del PIN, y hacía `goto tail` INCONDICIONAL →
    // el cartel se auto-dismisseaba el frame siguiente y nunca se enviaba nada).
    //
    // Per IDA 0x514310 L1400-1406: para ErrorMessage==116 el rect del botón OK
    // NO cuenta — la única confirmación es Enter (byte_55CA038). Y L1420-1432:
    // si InputGold > 50.000.000 se muestra el cartel 118 y se resetea el input.
    //
    // StorageGoldFlag (StorageGoldFlag) lo setea quien abrió el diálogo:
    //   0 = guardar zen en el baúl     (sub_4EB5D0 case 0)
    //   1 = sacar zen del baúl         (sub_4EB5D0 case 1)
    //   2 = poner zen en el trade      (sub_4EB7F0)
    // Warehouse: C1:81 [type][money:4]. Trade: C1:3A [padding][money:4].
```

### Línea 585 en `UI_InGameMenu` — antes de `case 0x7e:`

```cpp
    // ── 126 — confirmar salida/expulsión/disolución de Guild ──────────────
    // 2026-08-15: acá había un bloque que limpiaba los buffers de usuario y
    // password (`DAT_07db8710`/`DAT_07db8810`).  Eso NO es lo que hace el
    // binario: IDA `UI_InGameMenu` L3343 `case 126:` arma y envía el paquete de
    // expulsión, y NUNCA toca InputText[1] en toda la función (grep sobre el
    // decompile: 0 ocurrencias).  El limpiar-input es la rama de CANCELAR
    // (`case 126: case 152:` del segundo switch, L2706), que ya está más abajo.
    //
    // PMSG_GUILD_DELETE_RECV (MuEmu Guild.h:205):
    //     [C1][0x17][0x53][name:10][PersonalCode:10]      sizeof = 23
    // `name` sale de la tabla de 13 bytes por miembro `byte_7E91790`, indexada
    // por `dword_5615E4` (lo fija el click de `sub_40F320`). Para un miembro
    // normal esa fila sólo puede ser la propia; para el Master puede ser la de
    // otro miembro o la propia. El servidor decide si corresponde expulsar,
    // abandonar o disolver. `PersonalCode` es el valor de InputText[0].
    //
    // HackPacketCheck índice 83 → Encrypt = 0 ⇒ frame C1 + chain-XOR, SIN
    // serial (ver "Desconexiones: serial de packets y Encrypt=1" en CLAUDE.md).
    // El original hace el XOR en dos pasadas (3..12 y 13..22) porque appendea el
    // PersonalCode después; como la cadena es secuencial y los rangos son
    // disjuntos y contiguos, una sola pasada 3..22 da el mismo resultado.
```

### Línea 648 en `UI_InGameMenu` — antes de `// IDA UI_InGameMenu (0x514310) L3857-3963: carteles 139 (CreateOkMessageBox),`

```cpp
    // ── 0x8b / 0x8c / 0x9a — REMOVIDO 2026-08-26 ─────────────────────────
    // Acá había un case agrupado etiquetado "NPC shop item list" que terminaba
    // en `goto tail` INCONDICIONAL. `tail` hace `ErrorMessage = NextErrorMessage`,
    // o sea limpiaba el estado en el mismo frame, antes de que
    // `RenderInformation -> RenderErrorMessage` alcanzara a dibujarlo.
    //
    // Los tres estados son message boxes, no una lista de tienda:
    //   0x8b (139) — CreateOkMessageBox      (0x0051D6F0)
    //   0x8c (140) — GuildMemberList_Update            (ranking de Devil Square, lista)
    //   0x9a (154) — GuildMemberList_Add            (ranking de Devil Square, 1 fila)
    //
    // Y el switch de IDA (raw 00514310) NO tiene case para ninguno: 139, 140,
    // 141, 142 y 154 se agrupan en una rama propia (L1381) que sólo dismissea
    // al clickear un botón. Sin case, caen al `default` de abajo, que ya
    // persiste hasta el click en OK o Enter — que es el comportamiento fiel.
    //
    // Sintoma que tenia: el cartel del tiempo de los eventos (click derecho
    // sobre "Devil's Invitation" / "Cloak of Invisibility") no aparecia nunca,
    // aunque el paquete iba y el server respondia bien.
    //
    // La geometria que usaba tampoco salia de ningun lado: filas de 16 px desde
    // y=0x2c entre x=0x6a y x=0x16a, contra las dos filas de 35 px en y=180/265
    // (x 245-395) que el original usa para el estado 143.
```

### Línea 672 en `UI_InGameMenu` — antes de `case 0x8b:`

```cpp
    // IDA UI_InGameMenu (0x514310) L3857-3963: carteles 139 (CreateOkMessageBox),
    // 140, 141 (CreateDialogInterface, con paginas) y 154.  2026-09-18: fiel.
    // El port terminaba el 141 con un `goto tail` incondicional, que cierra el
    // cartel en el primer frame (boton Explicacion del Golden Archer).
```

### Línea 765 en `UI_InGameMenu` — antes de `int cur  = g_iCurrentDialogScript;`

```cpp
                    //
                    // 2026-08-21: el port decidia si cerrar comparando el TEXTO
                    // de la respuesta contra GlobalText[609] (invencion), y le
                    // pasaba a ItemList_Select el indice de RESPUESTA en vez del
                    // link.  Con la tabla ya reconciliada se puede hacer lo que
                    // hace el binario.
```

### Línea 871 en `UI_InGameMenu` — antes de `case 0x97:`

```cpp
    // ── Yes/No checkbox (sell/drop confirm) — ErrorMessage 151 ─────────────
    // 2026-07-27 FIX: el port anterior trataba 0x97 como una lista de respuestas
    // de NPC (ItemList_Select), que dismisseaba el cartel al instante sin setear la
    // respuesta → el sell-confirm quedaba colgado con el item agarrado (tooltip
    // pegado, "todo raro"). ErrorMessage 151 es un cartel Yes/No. Port IDA
    // UI_InGameMenu L1798-1856: hit-test de los 2 botones (DAT_083a42f8, stride
    // 5 ints [id][x][y][w][h]; Yes=btn0 id1, No=btn1 id3, render en +213/+100) y
    // seteo de DAT_00559f5e = 1 (Yes) / 2 (No), que el drop-dispatcher
    // (Inventory_DropDispatch) consume para enviar/cancelar el sell.
```

### Línea 922 en `UI_InGameMenu` — antes de `Net_SendSmallPacket(pkt, sizeof(pkt));`

```cpp
        // 2026-08-25 FIX: la rama de RECHAZO mandaba el paquete con `::send`
        // crudo — C1 sin encriptar — cuando el 0x41 pide Encrypt=1
        // (HackPacketCheck.txt indice 65). El server lo rechaza con "Packet
        // encryption error" y CIERRA la conexion, o sea rechazar una invitacion
        // de party desconectaba. Aceptar ya iba bien por C3.
        //
        //   struct PMSG_PARTY_REQUEST_RESULT_RECV {   // Party.h:19
        //       PBMSG_HEAD header;   // C1 : 6 : 0x41
        //       BYTE result;         // +3
        //       BYTE index[2];       // +4, +5  (index[0] = byte ALTO)
        //   };
```

### Línea 965 en `UI_InGameMenu` — antes de `case 0x98:`

```cpp
    // Antes corria sin gate: el cartel se cerraba solo y nunca se mandaba nada.
```

### Línea 1002 en `UI_InGameMenu` — antes de `case 0x99:`

```cpp
    // ANTES: este case estaba portado como "multi-select item list" y terminaba
    // en un `goto tail` INCONDICIONAL.  `tail` hace ErrorMessage =
    // NextErrorMessage, asi que el cartel se cerraba al frame siguiente de
    // abrirse y la fruta nunca se podia usar.  Mismo patron que el case
    // 0x8b/0x8c/0x9a que se removio el 2026-08-26.
    //
```

### Línea 1066 en `UI_InGameMenu` — antes de `{`

```cpp
        // BUG-FIX 2026-07-14: los estados no manejados (incluidos los códigos de
        // ErrorMessage del login — 0x16 wrong-pass, 0x65 sin-pass, etc., que
        // comparten el global DAT_083a7c24) NO deben caer al `tail` cada frame.
        // El `tail` hace `DAT_083a7c24 = DAT_083a7c28` (dismissea el cartel) +
        // PlayBuffer(25) — antes corría incondicionalmente → el cartel de error
        // del login se borraba 1 frame después de aparecer (con el "sonido de
        // click" que reportó el usuario). El dismiss debe pasar SOLO al click en
        // el botón OK, que RenderErrorMessage (RenderErrorMessage default box path)
        // dibuja en (284,98)-(354,119). Con click → tail (dismiss). Sin click →
        // el cartel persiste, fiel al original.
```

### Línea 1084 en `UI_InGameMenu` — antes de `if (!okClick && IsClickPushed()) {`

```cpp
            // 2026-08-26: el rect fijo de arriba sirve para los carteles del
            // login, pero no para los que arma `CreateOkMessageBox` (139) y
            // compania: esos traen su propio descriptor de boton en
            // DAT_083a42f8 (5 ints por entrada: bitmapId-240, x, y, w, h) y
            // `RenderErrorMessage` los dibuja en (x + _DAT_00552d40, y + _DAT_0055290c)
            // = (x+213, y+60). Para el 139 el descriptor es {1, 71, 140, 70, 21},
            // o sea el OK cae en (284..354, 200..221) — 100 px mas abajo que el
            // rect fijo, asi que con el mouse no se podia cerrar (solo con Enter).
            //
            // Se aceptan los dos rects en vez de reemplazar uno por el otro: el
            // original usa el descriptor, pero el rect fijo cubre los estados
            // del login que hoy dependen de el.
```

## `src/UI/UI_LegacyExterns.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 132 en `FUN_004c9730_old` — antes de `if (a3 < 0 || a3 >= 60 || !CharacterAttribute) return;`

```cpp
    // 2026-05-05: SIMPLIFIED safe version. La versión completa hacía 10+
    // sprintf_s con GlobalText[N] format strings. Si cualquier slot de
    // GlobalText estaba sin cargar/corrupted (e.g. tenía "%s" donde el código
    // pasa un int), sprintf interpretaba el int como char* → AV crash on
    // hover. Esta versión solo muestra el nombre del skill (sin damage,
    // mana, distance lines) hasta que GlobalText loader esté validado.
    //
    // Bounds check: a3 (slot index hovered) debe ser 0..63 para evitar OOB
    // read en CharacterAttribute[87 + a3].
```

### Línea 342 en `RenderSkillTooltip` — antes de `auto  TextListN     = [](int i) -> char* { return lpString_07e90798 + i * 100; };`

```cpp
    // ── Build TextList lines ────────────────────────────────────────────────
    // Estructura del binario (0x004C97xx, y el volcado de IDA que quedo en
    // FUN_004c9730_old bajo `#if 0`):
    //   slot 0 = "\n"                       (separador de media altura)
    //   slot 1 = nombre de la skill         color 1 (azul claro), NEGRITA
    //   slot 2 = "\n"
    //   slot 3+ = dano / rango / mana / AG  color 0 (blanco)
    //   opcional "no puede usarla"          color 5 (blanco con franja)
    //   ultimo  = "\n"
    //
    // 2026-08-18: antes esto pintaba su PROPIA caja (cuarta reimplementacion
    // inventada del tooltip, con colores ARGB y textos en ingles hardcodeados).
    // Ahora usa lpString_07e90798 + CharMenu_RenderTextList, que es lo que hace el binario
    // — misma rutina que el tooltip de item y el menu de personaje.
```

### Línea 443 — antes de `void __cdecl UI_DrawText(int param_1, int param_2, char *param_3, int param_4, int param_5`

```cpp
// FUN_0047F7A0 @ 0x0047F7A0 (IDA)
// UI_DrawText — Text_Draw(x, y, text, maxw, iSort, extra)
// Draws text via Font vtable dispatch (CUIRenderText_RenderText) if non-empty or maxw!=0.
// Return type is void per functions.h declaration.
//
// 2026-05-04: BUG-FIX — el flag `param_5` (iSort) determinaba alineación:
//   1 = left-align (default)
//   2 = center within [x, x+maxw]
// Antes era `(void)param_5` → todo render quedaba left-aligned. El menú C
// (RenderText con centered=1 sobre maxw=70-150) renderizaba "mago", "[Soul
// Master]", "Fuerza:1000" etc. pegados a la izquierda en lugar de centrados.
// IDA original delega al `CUIRenderText::RenderText` que maneja iSort
// internamente; lo replicamos inline acá.
// IDA: FUN_0047F7A0
```

### Línea 460 en `UI_DrawText` — antes de `{`

```cpp
    // 2026-05-08: bug-fix — múltiples crashes en ucrtbase.dll's strlen/lstrlenA
    // venían de pasar punteros pequeños (< 0x100000) tipo 0x2A00 (= type*64
    // con DAT_07d78068=0). Validar el puntero antes de cualquier strlen.
    // Range: heap user-space [0x100000..0x80000000). rdata strings in our
    // exe live in [0x004XXXXX..0x00500000) — also valid.
```

### Línea 477 en `UI_DrawText` — antes de `if (param_5 >= 2 && param_4 > 0 && DAT_055c9fec) {`

```cpp
    // Aplicar centrado dentro del box [param_1, param_1+param_4].
    // param_1 y param_4 están en unidades del ortho; el extent de GDI viene en
    // píxeles de framebuffer.  Text_MeasureOrthoWidth hace la conversión (es el
    // equivalente correcto del `sz.cx / g_fScreenRate_x` de IDA para nuestro
    // pipeline).  Sin ella el texto quedaba descentrado hacia la izquierda.
    // 2026-08-26 — MEZCLA DE ESPACIOS. `param_4` (iBoxWidth) llega en PIXELES:
    // los callers lo calculan como `N * WindowWidth / 640`, que es lo que hace
    // el original (p.ej. RenderCharacterInfoWindow 0x4ECC60 L279:
    // `RenderText(iPosX + 35, iPosY + 12, Buffer, 120 * WindowWidth / 0x280, ...)`).
    // Ese hardcode es correcto y se conserva.
    //
    // Pero `param_1` es LOGICO y `Text_MeasureOrthoWidth` devuelve LOGICO, asi
    // que el centrado mezclaba las dos unidades. A 640x480 no se notaba porque
    // `N * 640 / 640 == N` y la escala vale 1.0; a 1024 el ancho de caja salia
    // 1.6x mas grande que la medida del texto y el centrado se corria a la
    // derecha — el sintoma de "textos corridos respecto de sus labels" en el
    // menu de personaje (C).
    //
    //   param_4  -> pixel -> / g_fScreenRate_x -> logico
    //   textW    -> logico (Text_MeasureOrthoWidth ya divide)
    //   x        -> logico + logico  -> lo convierte CUIRenderText_RenderText
```

### Línea 516 en `FindTextA` — antes de `if (iVar5 < 1) return 0;`

```cpp
    // BUG-FIX 2026-07-17: patrón vacío = no-match. El IDA devuelve 1 para patrón
    // vacío, pero eso solo es "correcto" porque en el original los strings de filtro
    // están cargados (no vacíos). Varios de esos globals llegan vacíos en runtime en
    // nuestro build (ej. GlobalText[457/458] si el Text.bmd no tiene esas filas) →
    // FindText(nombre,"")=1 rechazaba TODO nombre en el create-char. Un patrón vacío
    // no debe matchear nada.
```

### Línea 563 — antes de `static void Text_PixelToOrthoScale(float* outX, float* outY)`

```cpp
// CUIRenderText_RenderText @ 0x0040F610 — CUIRenderText::RenderText (vtable dispatcher)
// Original IDA: `(*(vtable[0]+4))(this, x, y, text, ...)` — thiscall through
// CUIRenderText->pSubclass->vtable[1]. El subclass se instancia en OpenFont
// (0x0050f690) vía sub_40F570 y puede ser tipo-0 (simple, TGA font) o tipo-1
// (compleja, bitmap font procesada). El subclass tipo-0 usa `Bitmaps[0/1]`
// (Interface/FontInput.tga) para renderizar cada glyph como un quad.
//
// Port: implementación autónoma usando wglUseFontBitmapsA sobre la HFONT
// GDI ya seleccionada en m_hFontDC. Genera 256 display lists a partir de la
// fuente GDI y emite el texto con glCallLists. Suficiente para UI (login,
// char-select, chat) — el look no matchea la fuente TGA original píxel a
// píxel pero los textos aparecen en su posición con el color y layout
// correctos, que era lo que faltaba.
//
// Coordenadas: callers pasan Y con origen TOP-LEFT (convención game). La GL
// ortho es `gluOrtho2D(0, vw, 0, vh)` Y-bottom (ver GL_Begin2D en
// Render/GL_2D.cpp). GL_DrawTexture Y-flipa via `vh - param_3`. Acá hacemos
// lo mismo para raster pos.
//
// Color: DAT_00559c78 es el COLORREF GDI (0x00BBGGRR). Se convierte a glColor.
// El Alpha del byte alto (cuando está seteado, ej 0xffd2e6ff) se respeta.
// ── Escala "píxeles de framebuffer" → "unidades del ortho 2D" ───────────────
// 2026-07-20.  El punto que hacía fallar todos los recuadros de texto:
//
//   · La GEOMETRÍA 2D (RenderColor / RenderBitmap) se emite en unidades del
//     ortho, que es `gluOrtho2D(0, DAT_0056156c, 0, DAT_00561570)`, y la GPU la
//     estira hasta el viewport.  Un quad de ancho W se ve W * (viewport/ortho).
//   · Los GLIFOS, en cambio, los pinta wglUseFontBitmaps + glBitmap, que hace
//     un blit 1:1 EN PÍXELES DE FRAMEBUFFER desde el raster position.  NO se
//     estiran.  Y `GetTextExtentPointA` mide en esos mismos píxeles.
//
// O sea que el ancho del texto y el ancho de su fondo viven en unidades
// distintas, y hay que dividir el extent por la relación viewport/ortho para
// que el recuadro cubra exactamente las letras.
//
// Deliberadamente NO usamos `g_fScreenRate_x` (_DAT_055c9b70) ni
// `Screen_ToGLX` para esto: son dos fuentes de escala que en nuestro build
// están DESINCRONIZADAS.  `g_fScreenRate_x` sale de `g_ScreenW` (Config_Load),
// mientras que el ortho y el viewport salen de `DAT_0056156c` — y son dos
// variables separadas (globals.cpp:303 vs Config_Load.cpp:49), donde nada
// copia una a la otra.  Preguntarle a OpenGL por su viewport es la única
// fuente que no puede desincronizarse, y sigue siendo correcta si algún día
// se unifican esos globals.
// 2026-08-26: esto derivaba la escala del viewport de OpenGL
// (`viewport / WindowWidth`) para esquivar a `g_fScreenRate_x`, que en ese
// momento estaba desincronizado. Pero el viewport se setea con ESE MISMO global
// (`GL_Begin2D`: `glViewport(0,0,vw,vh)` con `vw = WindowWidth`), asi que la
// division daba 1.0 por construccion: no compensaba nada.
//
// Con las escalas globales ya correctas, la conversion pixel -> logico es
// exactamente `g_fScreenRate_x/y`, que es lo que usa el binario en sus seis
// sitios (`TextSize.cx / g_fScreenRate_x`). Se mantiene el nombre y la firma
// para no tocar los cuatro consumidores; lo que cambia es de donde sale el
// factor.
```

### Línea 623 — antes de `extern "C" float Text_GetOrthoScaleX(void)`

```cpp
// Ancho del texto EN UNIDADES DEL ORTHO (que es donde vive todo el layout).
// Es el equivalente correcto, para nuestro pipeline, del `sz.cx /
// g_fScreenRate_x` que hace IDA en RenderText (0x47F650) y RenderTipText
// (0x47F7F0).
// Escala pixeles-de-framebuffer -> unidades del ortho en las que dibuja el
// stack de texto (la misma que aplica CUIRenderText_RenderText a los glifos).
//
// OJO, NO es lo mismo que g_fScreenRate_x: ese es el factor del BINARIO, que
// pasa a espacio-640 porque su CUIRenderText reescala internamente. En nuestro
// pipeline el reescalado lo hace el ortho, asi que un ancho medido con
// GetTextExtentPointA hay que dividirlo por ESTE factor para mezclarlo con el
// layout. Usar los dos mezclados deja la caja y el texto a escalas distintas
// (ver el fix del ancho del tooltip en CharMenu_Build.cpp).
```

### Línea 847 en `CUIRenderText_RenderText` — antes de `HFONT hFont = (HFONT)GetCurrentObject(hFontDC, OBJ_FONT);`

```cpp
    // ── FUENTE ACTIVA (fix 2026-07-20) ──────────────────────────────────────
    // Acá había `hFont = DAT_055ca00c` HARDCODEADO (la fuente regular), pero
    // los callers seleccionan fuentes DISTINTAS en este mismo DC antes de
    // llamarnos: g_hFontBold (chat, HUD_Pass2/3/4, sub_40CE20 vía
    // CUIRenderText::SetFont) y g_hFontBig (HUD_Pass3:486, doble tamaño).
    //
    // Consecuencia doble:
    //   1. Los glifos salían SIEMPRE en regular — bold y big nunca se veían.
    //   2. El recuadro de fondo se medía con GetTextExtentPointA usando la
    //      fuente que el caller seleccionó (bold/big = más ancha) mientras las
    //      letras se dibujaban en regular (más angosta) → el fondo excedía al
    //      texto.  Y cuando el caller sí había dejado la regular, calzaba.
    //      De ahí el "a veces sobra, a veces no" que quedaba después de
    //      arreglar la escala viewport/ortho.
    //
    // Ahora la fuente sale del DC (que es la que efectivamente usó el caller
    // para medir), y cacheamos las display lists POR fuente.
```

### Línea 868 en `CUIRenderText_RenderText` — antes de `struct FontLists { HGLRC rc; HFONT font; GLuint base; int ascent; };`

```cpp
    // Cache de hasta 4 fuentes (regular / bold / big / repuesto).  Antes era
    // una sola entrada, así que alternar fuentes entre llamadas habría
    // reconstruido 256 display lists en CADA llamada.
```

### Línea 938 en `CUIRenderText_RenderText` — antes de `extern DWORD DAT_0056156c;  // ancho del ortho 2D`

```cpp
    // 2026-08-21: sin esto, al llegar al borde el quad de fondo se dibujaba
    // (glVertex2f se clipea normal) pero los glifos no, porque glRasterPos
    // fuera del viewport invalida la posición y glCallLists no emite nada →
    // quedaba un recuadro negro vacío en el borde.
```

### Línea 961 en `CUIRenderText_RenderText` — antes de `SIZE tot = {0, 0};`

```cpp
        // Antes se dividia por Text_PixelToOrthoScale, que valia 1.0 por
        // construccion; a 640x480 el resultado numerico no cambia.
```

### Línea 985 en `CUIRenderText_RenderText` — antes de `GLfloat curColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};`

```cpp
    // Color base desde DAT_00559c78 (COLORREF 0x00BBGGRR + opcional alpha en
    // byte 3).  Con marcadores presentes esto es solo el color del PRIMER tramo;
    // el resto sale de markers[].fg (ver el loop de abajo).
    //
    // 2026-05-04: respetar el alpha que el CALLER setea via glColor4f(...,α)
    // antes de RenderText. Antes pisábamos con `glColor4ub(R,G,B,A)` y se
    // perdía el cross-fade del banner clase/zona (los dos textos siempre a
    // α=1 → solapaban). Multiplicamos los alphas para que tanto el del
    // texto-color (DAT_00559c78 byte 3) como el del caller respeten su
    // contribución.
```

### Línea 999 en `CUIRenderText_RenderText` — antes de `GLfloat callerR = curColor[0], callerG = curColor[1], callerB = curColor[2];`

```cpp
    // 2026-08-21: el color del caller (glColor3f) también MODULA el RGB, no
    // sólo el alpha.  En el binario el subclass por defecto de CUIRenderText
    // (sub_410AF0, g_iRenderTextType != 1) hace TextOut a un DIB, copia los
    // píxeles a una textura pintándolos con m_dwTextColor / m_dwBackColor
    // (sub_40F6C0) y dibuja el quad con RenderBitmap (0x5125A0) — que NO llama
    // a glColor, así que la textura sale modulada por el color que dejó el
    // caller.  O sea: color final = m_dwTextColor × glColor.
    //
    // Sin esto, los `glColor3f` de RenderItemName (0x4C9E70) no hacían nada y
    // los nombres del suelo salían todos con m_dwTextColor: el Zen sin su
    // dorado y los items +N sin su color por nivel.
```

### Línea 1018 en `CUIRenderText_RenderText` — antes de `{`

```cpp
    // ── FONDO + GLIFOS, POR TRAMOS DE COLOR ──────────────────────────────────
    // Sin marcadores hay un único tramo con (m_dwTextColor, m_dwBackColor), o
    // sea exactamente el comportamiento previo.
    //
    // El original (sub_4105F0) resuelve el color por COLUMNA de píxel: busca el
    // último marcador cuyo pixelX sea menor que la columna.  Como pixelX es el
    // extent del prefijo, ese corte cae justo en el borde del carácter, así que
    // partir por charIndex — que es lo natural para un renderer de glifos — da
    // el mismo resultado.
    //
    // FONDO: 2026-07-19 — faltaba por completo.  En el original el subclass
    // pinta el fondo (m_dwBackColor / bg del marcador) en los píxeles NO-texto
    // del tramo; por eso los mensajes del chat salían sin su recuadro.
    // Formato 0xAABBGGRR igual que el color de texto.  Alpha 0 = sin fondo.
```

### Línea 1068 en `CUIRenderText_RenderText` — antes de `int descent = (int)bsz.cy - s_ascent;`

```cpp
                // glRasterPos deja el ORIGEN DEL GLIFO en la baseline, así que el
                // texto ocupa [rasterY - descent, rasterY + ascent]. Antes usaba
                // `rasterY-2 .. rasterY+cy-2`, que corría la caja hacia arriba y
                // dejaba aire abajo. descent = cy - ascent.
```

## `src/UI/UI_LegacyGameHelpers.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 47 — antes de `void __cdecl Item_ReturnPickedItem(void)`

```cpp
// 2026-09-11: reescrita contra IDA.  La version anterior escribia en bases
// DAT_ corridas 0x38 que en este build son OTRA memoria (no los Offset*Items),
// borraba con memset un buffer de 64 celdas que el original no toca, tomaba la
// posicion de ItemPickedPos y no soltaba el item de la mano: el item "levantado
// y devuelto" nunca volvia a su celda.
//
```

### Línea 109 — antes de `char __cdecl SelectSkillByHotkey(int a1)`

```cpp
// SelectSkillByHotkey @ 0x004B0E80 — SelectSkillByHotkey(int number)
//
// Elige la skill activa a partir del numero de hotkey que el jugador acaba de
// apretar.  Recorre las 20 ranuras de skill y, para la que tenga asignado ese
// numero, escribe su indice en `Hero + 913` (la skill en uso).
//
//   for (i = 0; i < 20; i++) {
//       if (CharacterAttribute[i + 87] &&
//           CharacterAttribute[(SelectedHero << 6) + i + 215] == a1) {
//           Hero[913] = i;  found = 1;
//       }
//       if (m_bAutoAttack && World != 6) {
//           v9 = CharacterAttribute[Hero[913] + 87];
//           if (v9 == 6 || v9 == 15) { SelectedCharacter = -1; Attacking = -1; }
//       }
//   }
//
// 2026-09-04 -- BUG-FIX ("asigno el skill con Ctrl+N pero al apretar el numero
// no cambia").  El port tenia la firma `void SelectSkillByHotkey(void)`: Ghidra perdio
// el argumento (viaja en registro) y quien lo porteo comparo la tabla de
// asignaciones contra la CONSTANTE 1 en vez de contra el numero apretado.  O sea
// solo podia seleccionar la skill asignada al 1 -- y como los dos call sites
// llamaban sin argumento, cualquier tecla 0..9 hacia lo mismo.
//
// Los globals si estaban bien mapeados (verificado con ida_get_function):
// SelectedHero = 0x5616AC, m_bAutoAttack = 0x559C5C, Attacking = 0x559C58,
// CharacterAttribute = 0x7CF1FF4;  +87 = tipo de skill, +215 = numero de hotkey
// (la tabla es por personaje: SelectedHero << 6).
// IDA: sub_4B0E80 (0x004B0E80)
```

## `src/UI/UI_LegacyText.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 42 — antes de `int __cdecl SeparateTextIntoLines(const char *lpszText, char *lpszSeparated, int iMaxLine,`

```cpp
// SeparateTextIntoLines @ 0x0051D600 -- corta un texto en lineas de ancho fijo.
//
// 2026-09-20: reescrita 1:1 contra el raw.  La anterior era una aproximacion
// con dos reglas propias:
//   (a) rebobinaba al ultimo espacio si caia en la mitad final de la linea
//       (`lastSpace > maxChars/2`).  El binario rebobina solo si el espacio
//       esta dentro de los ultimos min(iLineSize/2, 10) caracteres, o sea es
//       mas estricto: parte un poco antes y las lineas salen mas cortas.
//   (b) cuando no rebobinaba, cortaba a los 10 caracteres.  El binario no
//       corta ahi: parte a lo ancho de la linea, sin rebobinar.
//
// Medido: para prosa normal las dos dan el mismo resultado, porque siempre hay
// un espacio en la mitad final y la rama (b) no llega a correr.  La diferencia
// aparece con palabras largas sin espacios (URLs, nombres pegados), donde la
// version vieja cortaba a 10 caracteres.  O sea esto es fidelidad, no el
// arreglo de ningun sintoma reportado.
//
// Detalles fieles que importan: avanza por caracteres MBCS (_mbclen; con el
// locale "C" que usa este build devuelve siempre 1, igual que la version por
// bytes), el terminador de cada linea se escribe ANTES de saltar al slot
// siguiente, y el retorno es `indiceDeLinea + 1` -- nunca 0 para texto no
// vacio, cosa que la version vieja si podia devolver.
```

## `src/UI/UI_LegacyWidgetSystem.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 152 en `Widget_CtorBase` — antes de `*(const void **)param_1 = (const void *)g_ClothVTable;`

```cpp
    // 2026-08-11 — vtable. IDA hace `*(_DWORD *)this = &off_552520;` y varias
    // rutinas la usan por indirección; con el campo sin inicializar se ejecuta
    // basura (crash 0xC0000005 param0=8 al entrar al mundo, desde
    // `DeleteCloth`/DeleteCloth que llama vtable[0](3)).
    //
    // Leída del binario original (`Cliente armado/main.exe`, MD5 eb95ac…):
    //     off_552520 = { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
    // De esas cuatro sólo `sub_408FF0` está portada (y es un stub vacío); las
    // otras tres no existen en este build. Se instala una vtable bien formada
    // con no-ops para que el objeto sea válido y las indirecciones no salten a
    // basura. TODO: portar 0x0045AAA0 (dtor), 0x00408780, 0x004089B0 y el
    // cuerpo real de 0x00408FF0 (render de la tela).
```

### Línea 188 — antes de `int __cdecl Sound_UpdateChannel3D_Tick(int *param_1, float dt);`

```cpp
// IDA: Widget_CheckState (0x00408900)
// Widget_CheckState(widget, hash, flags)
// __thiscall in original (this=widget via ECX). Calls Sound_UpdateChannel3D_Tick `flags` times,
// returns 0 if any fails, 1 if all pass. Sound_UpdateChannel3D_Tick is a void stub → always return 1.
// Port FIEL de IDA `sub_408900` (Hex-Rays perdió el `this`, que viaja en ECX):
//     v2 = 0;
//     if (a2 <= 0) return 1;
//     while (sub_408940(a1)) { if (++v2 >= a2) return 1; }
//     return 0;
// `hash` son los BITS del dt (0x3ba3d70a = 0.005f) y `flags` el nº de
// iteraciones. 2026-08-11: era un stub que devolvía 1 SIN ejecutar la
// simulación, así que los nodos de la tela nunca se movían.
```

### Línea 223 en `GridSpring_Create` — antes de `*(float *)(thiz + 0x18) = *(float*)&p4;   // a4`

```cpp
    // 2026-08-11 FIX: los tres campos de abajo estaban CORRIDOS un parámetro.
    // IDA `sub_408130` L78-89:
    //     this[6] (+0x18) = a4        ← nuestro port ponía p3
    //     this[7] (+0x1C) = a5        ← ponía p4
    //     this[8] (+0x20) = a8        ← ponía p5
    //     this[9] (+0x24) = a9        ← ok
    // El de +0x20 es el ANCHO de la grilla: con p5 (0.0 en la llamada de la
    // capa) todos los nodos quedaban en la misma columna.
```

### Línea 233 en `GridSpring_Create` — antes de `*(int   *)(thiz + 0x28) = p6;             // cols`

```cpp
    // 2026-08-11 FIX: IDA `sub_408130` L80-90 guarda
    //     this[10] (+0x28) = a6   ← 6º param
    //     this[11] (+0x2c) = a7   ← 7º param  (nuestro port ponía p3, el 3º)
    //     this[12] (+0x30) = a7 * a6
    // Con p3 en +0x2c la grilla quedaba de 19 filas sobre 10x10 → el render
    // (sub_408FF0 / sub_4091D0, que leen +0x28 y +0x2c) se iba de rango.
```

### Línea 332 en `GridSpring_Create` — antes de `nx = out_col[0]; ny = out_col[1]; nz = out_col[2];`

```cpp
                // 2026-08-11 FIX: BMD__TransformPosition LEE del 3er arg (Pos)
                // y ESCRIBE en el 4º (WorldPos). El port leía de vuelta
                // `out_pos` — la ENTRADA sin transformar — y descartaba el
                // resultado, así que la malla quedaba en espacio local en vez
                // de en la posición del personaje.
```

### Línea 340 en `GridSpring_Create` — antes de `int ni = W * row + col;`

```cpp
            // IDA `v31 = v30 + i * v29` con v29 = this[10] = W (columnas).
            // 2026-08-11: era `H * row + col`. Coincide sólo cuando W == H
            // (la capa del MG es 10x10); los demás cloths quedaban barajados.
```

### Línea 348 en `GridSpring_Create` — antes de `BYTE vert_flag = (((*(unsigned int *)(thiz + 0x14) & 0x300) != 0x100)) ? 4 : 0;`

```cpp
    // 2026-08-11: las diagonales recibían `vert_flag` (4) en vez de v63 (1),
    // o sea entraban al solver de rango y nunca al de igualdad.
```

## `src/UI/UI_LegacyWidgets.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_bulk_misc.cpp.
```

### Línea 31 — antes de `void __stdcall FUN_0040dce0(int p1, int p2, float p3, float p4, float p5, float p6, DWORD `

```cpp
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║ STUBS FANTASMA — NO SON LA IMPLEMENTACIÓN VIVA (2026-07-20)              ║
// ║ FUN_0040dce0 / FUN_0040def0 son el render de los 3 botones popup del     ║
// ║ ChatListBox.  Están PORTADOS en src/UI/ChatListBox.cpp como              ║
// ║ ChatLB_DrawButton / ChatLB_renderFooter (slot 24 de la vtable, al que    ║
// ║ IDA llega por el thunk sub_40D600).  Estos dos cuerpos quedan vacíos     ║
// ║ porque no los llama nadie: los únicos xrefs en IDA son sub_40DEF0 y      ║
// ║ sub_40CE20 (slot 22, el frame/BG, todavía stub).  Si algún día se        ║
// ║ portea slot 22, que llame al helper de ChatListBox.cpp — no a esto.      ║
// ╚══════════════════════════════════════════════════════════════════════════╝
```

### Línea 104 — antes de `void __cdecl FUN_004104b0(LONG _this, char *Source)`

```cpp
// FUN_004104b0 @ 0x004104B0 (~111 lines) — Font_ParseColorMarkup: scans string for \x02
// escape codes, builds segment table (up to 0x11 segments) at this+0x24, strips markup
// bytes from output, measures each segment via GetTextExtentPointA. Calls FUN_004102e0
// per segment.
// FUN_004104b0 (IDA-activated, was Ghidra stub)
```

### Línea 286 — antes de `void Game_DestroyWindow(void) {`

```cpp
// Game_DestroyWindow @ 0x004145C0 (IDA `DestroyWindow`, 1039 bytes) — la limpieza
// de salida.  Su unico llamador es el final de WinMain (0x42207B), despues del
// bucle de mensajes.
//
// Se porta SOLO lo que tiene efecto observable o es trivialmente seguro.  Lo
// que sigue queda fuera A PROPOSITO:
//
//  - Los dos bloques de hash-table (re-encriptado de SkillAttribute y de
//    CharacterMachine): anti-tamper, fuera por politica del proyecto.
//
//  - `SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_iScreenSaverOldValue, 0, 0)`
//    y el `SystemParametersInfoA(0x61, 0, ...)`.  **Portarlos seria un bug, no
//    una mejora**: el lado de ARRANQUE que guarda el valor viejo vive bajo
//    `IDA_PORT_00422074`, que no esta definido, asi que nunca desactivamos el
//    salvapantallas y `g_iScreenSaverOldValue` vale 0.  Restaurar ese 0 le
//    DESACTIVARIA el salvapantallas al usuario de forma permanente.  Si algun
//    dia se activa ese gate, estas dos lineas vuelven junto con el.
//
//  - Los frees masivos (`BMD::Release` de los modelos 160..962, `UnloadImage`
//    de las 1450 texturas, ModelsDump, RendomMemoryDump, SkillAttribute,
//    CharacterMachine, GateAttribute).  El proceso termina inmediatamente
//    despues, asi que el SO los reclama igual; y nuestros pools no tienen los
//    mismos tamanos que el binario, con lo cual un recorrido a ciegas puede
//    crashear al cerrar.  Un crash de salida es peor que una limpieza que no
//    hace falta.
//
//  - `g_pRenderText`: en este arbol es un objeto stub (`g_RenderTextStubObj`),
//    no un objeto con vtable real, asi que no tiene destructor que llamar.
```

### Línea 355 — antes de `void __cdecl    FUN_00407970(void *_this);   // dtor de nodo   (0x407970, __cdecl)`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// Sistema de tela (capa del MG) — destructores. Port 2026-08-11.
//
// Las vtables se leyeron del binario original (`Cliente armado/main.exe`,
// MD5 eb95ac0785e40a7ad60c9ddb5d8bef34), porque su contenido es data y no
// aparece en los decompiles:
//     off_552520  (widget) = { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
//     DAT_005524e8 (nodo)  = { 0x00408680 }
// ─────────────────────────────────────────────────────────────────────────────
```

## `src/UI/UI_SkillHotkeys.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 8 — antes de `int __stdcall FindHotKey(int Skill) {`

```cpp
// FindHotKey @ 0x004B1170 (~202 lines)
// Looks up a skill ID in the CharacterMachine hotkey table via MAIN_HASH_CLASS.
// Returns hotkey slot index (0..19), or -1 if not found.
// Original wraps access in anti-tamper encrypt/decrypt; we skip that.
// Ghidra: CharacterAttribute->Skill[iVar5+4] == unaff_retaddr (phantom param = Skill)
// Real access: *(BYTE*)(DAT_07cf1ff4 + 0x57 + iVar5) == Skill
// IDA: FindHotKey @ 0x004B1170 — int __cdecl FindHotKey(int Skill)
//   v17 = 0;                                        // <- valor por defecto
//   v4  = 0;
//   while ( *(unsigned __int8 *)(CharacterAttribute + v4 + 87) != Skill )
//     if ( ++v4 >= 20 ) goto LABEL_22;
//   v17 = v4;
// LABEL_22:
//   return v17;
//
// 2026-09-01 FIX — devolvia **-1** cuando el skill no esta en los 20 slots;
// IDA devuelve **0** (el inicializador de v17, que el camino de no-encontrado
// nunca pisa).  Consecuencia real medida en el path de flechas:
//   MoveCharacter (6 sitios) -> CreateArrows(c, o, 0, FindHotKey(skill), ...)
//   -> CreateArrow -> CreateEffect(..., SkillIndex, Skill)
//   -> CreateEffect prologo: `i[133] = (BYTE)SkillIndex`  (= 0xFF con -1)
//   -> sub_466440 (0x00466440, llamado por MoveEffect en cada tick del
//      proyectil) hace `CharacterAttribute[ i[133] + 87 ]`, o sea
//      CharacterAttribute[342] — FUERA del array de 20 skills (87..106).
// Ese byte basura se compara contra 51/52 y, cuando cae en 52, dispara
// `CreateJoint(1249, ..., SubType 6, ...)` (la espiral de Penetration) en CADA
// flecha, de cualquier skill de Elf.  Tambien envenena
// `sub_45FEC0(i[133], ...)`.  Con 0 el indice vuelve a caer dentro del array.
// Ningun caller del arbol distingue -1 (verificado): nadie compara el retorno
// contra -1 ni contra < 0.
// IDA: FindHotKey (0x004B1170)
```

### Línea 68 en `RenderSkillIcon` — antes de `if (iIndex < 0 || iIndex >= 60) return;`

```cpp
    // Read skill ID from CharacterAttribute->Skill[iIndex + 4]
    // CharacterAttribute = DAT_07cf1ff4, Skill array starts at offset +0x57
    // Actually the Ghidra accesses Skill[unaff_retaddr + 4] where unaff_retaddr = iIndex
    // 2026-05-05: bounds check on iIndex (passed by caller, can be Hero[913]
    // garbage). Without this, reading CA[0x57+iIndex] overflows CA buffer
    // → garbage skillId → OOB on subsequent SkillAttribute reads → crash.
```

### Línea 84 en `RenderSkillIcon` — antes de `if (skillId == 0x2f) {`

```cpp
    // If skill is 0x2f (Helper summon) and helper type is not Dark Horse (0x332) or Dark Spirit (0x333),
    // tint the icon reddish
    // IDA sub_4BB940 L91-97: el skill 47 (se usa montado) sale rojizo si el
    // heroe no tiene Uniria (818) ni Dinorant (819) en el slot de helper
    // (Hero + 696 = c+0x2B8).  2026-09-12: estaba comentado como "cosmetico".
```

## `src/UI/UI_StatsPanel.cpp`

### Línea 42 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 63 en `RenderErrorMessage`

```cpp
// BUG-FIX bits → float
```

### Línea 66 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 138 en `RenderErrorMessage`

```cpp
// BUG-FIX bits → float
```

### Línea 141 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 213 en `RenderErrorMessage` — antes de `piVar7 = &DAT_083a4304;`

```cpp
      // BUG-FIX 2026-05-03: was `(int)piVar7 < 0x83a432c` (literal end addr from
      // source binary). The real iteration count is 2 (button rects: stride 5
      // ints, IDA bound 0x83a432c - 0x83a4304 = 0x28 = 2 × 20 bytes).
```

### Línea 219 en `RenderErrorMessage` — antes de `local_d4 = (float)piVar7[1];`

```cpp
          // 2026-08-26: el ANCHO estaba como `*(float*)piVar7`, o sea
          // reinterpretando los bits, mientras que el ALTO de la linea de al
          // lado convertia con `(float)`. Los dos salen del mismo descriptor de
          // ints (CreateOkMessageBox escribe `v1[3] = 70; v1[4] = 21;`), asi que
          // los dos tienen que convertir. Reinterpretado, el 70 daba 9.8e-44:
          // ancho cero y boton invisible — el cartel de "OK" no se podia cerrar
          // con el mouse. Misma familia que los bugs de `(float)(uintptr_t)`,
          // con la mezcla de estilos dentro de la misma expresion como pista.
```

### Línea 237 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 243 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 253 en `RenderErrorMessage`

```cpp
// BUG-FIX bits → float
```

### Línea 257 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 263 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 273 en `RenderErrorMessage`

```cpp
// BUG-FIX bits → float
```

### Línea 277 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 281 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 347 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 362 en `RenderErrorMessage` — antes de `piVar7 = &DAT_083a4304;`

```cpp
      // BUG-FIX 2026-05-03: same hardcoded address bound as line 213 — 2 button
      // rects (stride 5 ints, total 0x28 bytes / 0x14 stride = 2 entries).
```

### Línea 856 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 874 en `RenderErrorMessage`

```cpp
// BUG-FIX: 0x3f800000 son los bits de 1.0f
```

### Línea 915 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 940 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 946 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

### Línea 954 en `RenderErrorMessage`

```cpp
// BUG-FIX
```

## `src/UI/UI_TextBitmap.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 19 en `Font_RenderTextToBitmap` — antes de `int prefixWidth = bmpWidth;`

```cpp
    // FIDELIDAD 2026-08-15: IDA inicializa `sz.cx = a1` ANTES del check de '\n'
    // (sub_47F360 L21-23).  Si el texto empieza en '\n' la función saltea todo el
    // TextOut y sz.cx queda en a1, así que el split de color nunca se cruza y la
    // fila entera se pinta con SetTextColor_0.  Antes arrancábamos en 0 (=todo
    // m_dwTextColor), que es el caso opuesto.
```

### Línea 67 en `Font_RenderTextToBitmap` — antes de `if (!dstRow || !srcRow) {`

```cpp
    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 addr=FUN_0047f360+0x14B, param1=0):
    // el loop leía `*src` con `srcRow = ppvBits_055c9e4c` en NULL. `ppvBits` es
    // el puntero a los bits del DIB de la fuente (lo crea Font_BuildLayout vía
    // CreateDIBSection, Font_Layout.cpp:39); si esa init no corrió todavía queda
    // en nullptr. Hasta ahora no se notaba porque esta función solo se alcanza
    // desde RenderBoolean (burbujas de chat), que era código muerto — el pool
    // estaba partido en 3 globals y nunca tenía slots activos.
    // Mismo guard que ya usa HUD_Pass4.cpp:663 para este idéntico pixel-copy.
```

### Línea 95 en `Font_RenderTextToBitmap` — antes de `color = SetTextColor_0;`

```cpp
                            // 0x00559C7C — IDA `SetTextColor_0`: color del prefijo
                            // (nombre de guild).  2026-08-15: esto leía el global
                            // `DAT_00559c7c`, que en nuestro build era una memoria
                            // SEPARADA de `SetTextColor_0` (la que sí escriben
                            // RenderBoolean/RenderPartyHP) y quedaba en 0 → el
                            // [guild] salía transparente.  Unificados en globals.h.
```

### Línea 119 — antes de `void __cdecl Font_RenderBitmapText(int a1, int a2, float Width, float Height, int a5, int `

```cpp
// FUN_0047f4c0 @ 0x0047F4C0 — Font_RenderBitmapText (~64 lines)
// glTexImage2D uploads Bitmaps[0xd]. Clamps to screen bounds. RenderBitmap.
// Font_RenderBitmapText (IDA-activated, was Ghidra stub)
```

### Línea 200 en `Font_RenderBitmapText`

```cpp
// stubs_externs.cpp
```

## `src/UI/UI_Tooltip.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 10

```cpp
//
// 2026-09-26: aca habia una copia bajo el nombre RenderTipText.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.
```
