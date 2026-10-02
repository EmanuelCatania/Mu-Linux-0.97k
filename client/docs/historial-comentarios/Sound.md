# Historial de comentarios: `src/Sound/`

Comentarios de desarrollo movidos desde `src/Sound/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Sound/Music.cpp`

### Línea 27 — antes de `#ifndef qmemcpy`

```cpp
// NOTA(refactor B3): `qmemcpy` viene del decompile de Ghidra y el proyecto lo
// define suelto en 14 archivos. Conviene centralizarlo en un header comun.
```

### Línea 106 — antes de `int __fastcall FUN_00412180(int* param_1) {`

```cpp
// ── FUN_00412180 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 115 — antes de `void __cdecl FUN_004124d0(void *a1, const void *a2)`

```cpp
// ── FUN_004124d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_004124d0 @ 0x004124D0 (24 bytes) — memcpy 0x4a dwords (296 bytes)
// FUN_004124d0 (IDA-activated, was Ghidra stub)
```

### Línea 126 — antes de `void __fastcall FUN_004124f0(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_004124f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 133 — antes de `void __fastcall FUN_00412510(DWORD* param_1) {`

```cpp
// ── FUN_00412510 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 141 — antes de `void __fastcall FUN_004125f0(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_004125f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 148 — antes de `void __fastcall FUN_00412610(DWORD* param_1) {`

```cpp
// ── FUN_00412610 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 155 — antes de `void FUN_00412700(void) { FUN_0053d430((BYTE *)&g_GameGuardGameName); }`

```cpp
// ── FUN_00412700 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00412700 @ 0x00412700 (12 bytes) — NO es "string cleanup": es el wrapper que
// arranca GameGuard, `return PreInitNPGameMon("Mu")`.
//
// En el binario NO lo llama nadie desde codigo: su unico xref es de DATOS, desde la
// tabla de inicializadores dinamicos del CRT en 0x00558010. O sea corre ANTES de
// WinMain, via el thunk FUN_004126F0, que ademas registra el release con atexit.
// Por eso el splash de nProtect aparecia antes de que existiera la ventana del juego.
//
// Nuestro build no replica esa tabla, asi que esta funcion queda sin callers y
// GameGuard nunca arranca — que es lo que queremos (ver CLAUDE.md).
```

### Línea 168 — antes de `void FUN_00412710(void) {}`

```cpp
// ── FUN_00412710 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 172 — antes de `void FUN_00412780(void) { PacketCipher_Initialize((void *)0x055ca0a0); }`

```cpp
// ── FUN_00412780 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 176 — antes de `void FUN_00412790(void) {}`

```cpp
// ── FUN_00412790 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 180 — antes de `void FUN_004127c0(void) { FUN_00405240_init((void *)0x055c9bf0); }`

```cpp
// ── FUN_004127c0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 184 — antes de `void FUN_004127d0(void) {}`

```cpp
// ── FUN_004127d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

## `src/Sound/Sound_DS3D.cpp`

### Línea 140 — antes de `static void __cdecl FUN_00404e60_impl(int param_1) {`

```cpp
// ── FUN_00404e60_impl — helper local movido desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 161 — antes de `void __fastcall waveIO__dtor(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── waveIO__dtor — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 168 — antes de `void __fastcall FUN_00405260(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_00405260 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 175 — antes de `// FUN_00405280 @ 0x00405280 (11 bytes)`

```cpp
// ── FUN_00405280 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 11-byte: vtable + chain ─────────────────────────────────────────────────
```

### Línea 178 — antes de `int __cdecl FUN_00405280(HANDLE *_this)`

```cpp
// FUN_00405280 @ 0x00405280 (11 bytes)
// FUN_00405280 (IDA-activated, was Ghidra stub)
```

### Línea 187 — antes de `void __stdcall FUN_00405340(void) {`

```cpp
// ── FUN_00405340 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 196 — antes de `// CErrorReport::WriteDebugInfoStr @ 0x00405500 (IDA: CErrorReport__WriteDebugInfoStr).`

```cpp
// ── CErrorReport_WriteDebugInfoStr — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 52-byte ─────────────────────────────────────────────────────────────────
```

### Línea 207 — antes de `void __cdecl CErrorReport_Write(void*,const char*,...)            {} // debug log — kept a`

```cpp
// ── CErrorReport_Write — movida desde stubs_render_helpers.cpp (refactor B3) ──
```

## `src/Sound/Sound_Queue.cpp`

### Línea 3 — antes de `#include "stdafx.h"`

```cpp
//
// Sound_UpdateQueue — processes the pending sound-play queue each frame.
//
// The queue is a flat array starting at DAT_07c85894 with stride 0x6f (int-words),
// so each entry is 0x1bc = 444 bytes. The array ends at 0x7CF1EF4.
// Each entry layout (relative to piVar2, which points at field +4):
//   piVar2[-1]  (byte)   — active flag (non-zero = pending)
//   piVar2[0]   (int)    — sound type:
//                            0 = GL_SetBlendAdditive (play once)
//                            1 = GL_SetBlendSrcAlpha (play looped / 3D)
//                            2 = GL_SetBlendSrcOver(1) (play with param)
//   piVar2[...]          — remaining sound params (passed to Render_DrawSprite)
//
// After dispatching the play call, Render_DrawSprite is invoked on the entry
// (likely to advance or clear the queue slot).
//
// Sub-functions:
//   GL_SetBlendAdditive — Sound_Play (type 0: one-shot)
//   GL_SetBlendSrcAlpha — Sound_PlayLoop (type 1: looped/3D)
//   GL_SetBlendSrcOver — Sound_PlayParam (type 2: param variant)
//   Render_DrawSprite — Sound_Queue_Advance / clear slot
```

### Línea 27 — antes de `void __cdecl Render_DrawSpritePool(void)`

```cpp
// Render_DrawSpritePool = RenderSprites (verificado vía Ghidra). Recorre el effect pool
// y por cada slot activo:
//   - dispatch GL state según blend mode en +4 (GL_SetBlendAdditive/90/80)
//   - llama Render_DrawSprite (RenderSprite) para dibujar el quad
//   - clear active flag
// Llamada desde Scene_CharSelect.cpp:325 (mal-comentada como "Portal_Render"),
// Scene_Login.cpp:100. Es la función que dibuja TODOS los sprites/glows/sparkles
// del pool (glow +9 set, wing FX, weapon FX, particles, etc.)
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODO el render — ahora itera
// por índice acotado a 1002 slots.
// IDA: RenderSprites
```

### Línea 55 — antes de `/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */`

```cpp
//
// Sound_Queue_Advance — per-frame update for a sound queue slot.
//
// Fades the slot's volume field (+0x108) up or down based on the
// direction flag at +0x160:
//   0 — fade out: subtract _DAT_005524f4 per frame; clamp to 0.2 (0x3e4ccccd)
//   1 — fade in:  add    _DAT_005524f4 per frame; clamp to 1.0 (0x3f800000)
//
// After adjusting volume, calls RenderSprite_0 to submit the sound update:
//   channel  = *(short*)(param_1 + 2)
//   position = (float*)(param_1 + 0x10)
//   volume   = slot_volume * channel_volume_R * channel_volume_G (from DAT_083a7cc0/DAT_083a7cc4)
//   roll-off = *(float*)(param_1 + 0xe8)
//   distance = *(float*)(param_1 + 0x24)
//
// Globals:
//   _DAT_005524f4  — fade step delta
//   _DAT_005526e4  — minimum fade-out floor (0.2)
//   _DAT_0055256c  — float 1.0
//   DAT_083a7cc0   — per-channel volume table (R component, stride 0x38)
//   DAT_083a7cc4   — per-channel volume table (G component, stride 0x38)
```

### Línea 86 en `Render_DrawSprite` — antes de `if (*(char *)(param_1 + 0x160) == '\0') {`

```cpp
  // BUG-FIX 2026-04-27: esta función NO es Sound_Queue_Advance — es **RenderSprite**
  // (verificado vía Ghidra, addr 0x00479670 → "RenderSprite"). El comentario y el
  // archivo Sound_Queue.cpp tenían el nombre wrong. Saca cada slot del effect pool
  // y lo dibuja como billboard cuadrado vía RenderSprite_0 (Sprite_DrawTexturedQuad).
  // El return; previo bloqueaba TODO el render de sprites del juego (glow +9, wing
  // FX, weapon sparkles, lightning, particles) — combinado con el AUTO-SKIP del
  // pool en CreateSprite, NADA spawneaba ni se dibujaba.
  // DAT_083a7cc0/cc4 = Bitmaps[type] tabla de texturas (stride 0x38), fields +0/+4
  // = width/height usados para scale del quad.
```

### Línea 148 en `Chat_TickMessageTimer` — antes de `if (bVar1) {`

```cpp
  // Verificado runtime 2026-07-19: el tick dispara cada 150 frames y agrega la
  // linea vacia que hace scrollear el historial superior-izquierdo.
```

### Línea 152 en `Chat_TickMessageTimer` — antes de `UIChatLogWindow_AddText(DAT_07e11ddc, DAT_07e11dd8, 0);`

```cpp
    // Este es el ENVEJECEDOR del historial de chat, no un "mensaje periodico".
    //
    // IDA 0x480950 llama incondicionalmente con strText (0x07E11DD8) y
    // byte_7E11DDC (0x07E11DDC), y a esos dos globals NO LOS ESCRIBE NADIE en
    // todo el binario: tienen un unico xref cada uno, que es esta misma
    // lectura.  O sea son cadenas VACIAS siempre, y ese es el punto.
    //
    // El primer branch de ChatLB_AddText (sub_40C940) es justamente
    // `if (!*src && !*msg)`: recorre la lista y hace ++nodo[+0x114] en cada
    // entrada.  El render de la linea (slot 23) lee ese contador y empuja la
    // fila hacia arriba, dejando de dibujarla cuando pasa el tope de filas
    // visibles.  O sea la caducidad del historial la produce esta llamada,
    // cada 150 frames.  MoveNotices (0x47FCB0) es el mismo patron para los
    // avisos: CreateNotice(byte_7E11DD0, 0) cada 300, con otro buffer que
    // tampoco escribe nadie.
    //
    // 2026-07-27 esto se habia gateado con un chequeo de "texto imprimible"
    // para tapar un mensaje fantasma con un caracter raro.  El sintoma era
    // real pero la causa era otra: DAT_07e11dd8/ddc estaban declarados como un
    // char suelto, y leerlos como cadena se iba a los globals vecinos.  Con el
    // gate puesto, el caso normal (buffers vacios) no llama nunca y el
    // historial deja de avanzar: solo se movia cuando llegaban mensajes
    // nuevos.  Los buffers ya estan bien dimensionados en globals.cpp, asi que
    // la llamada vuelve a ser incondicional como en IDA.
```

### Línea 182

```cpp
// CreateBug — implemented in src/stubs.cpp (Sound_SpawnEmitter, cleaner version)
```
